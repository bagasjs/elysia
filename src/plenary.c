#include "plenary.h"
#include <errno.h>

Proc cmd_exec_async(CMD cmd)
{
    if(cmd.count < 1) {
        return INVALID_PROC;
    }
    String_Builder sb = {0};
    cmd_to_string(cmd, &sb);
    TRACELOG(LOG_INFO, "CMD: %s", sb.data);
    sb_append_null(&sb);
    sb_free(&sb);
    memset(&sb, 0, sizeof(sb));

#ifdef _WIN32
    // https://docs.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output

    STARTUPINFO siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(siStartInfo));
    siStartInfo.cb = sizeof(STARTUPINFO);
    // NOTE: theoretically setting NULL to std handles should not be a problem
    // https://docs.microsoft.com/en-us/windows/console/getstdhandle?redirectedfrom=MSDN#attachdetach-behavior
    // TODO: check for errors in GetStdHandle
    siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    siStartInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    siStartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    // TODO: use a more reliable rendering of the command instead of cmd_render
    // cmd_render is for logging primarily
    cmd_render(cmd, &sb);
    sb_append_null(&sb);
    BOOL bSuccess = CreateProcessA(NULL, sb.items, NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo);
    sb_free(sb);

    if (!bSuccess) {
        TRACELOG(LOG_ERROR, "Could not create child process: %lu", GetLastError());
        return INVALID_PROC;
    }

    CloseHandle(piProcInfo.hThread);

    return piProcInfo.hProcess;
#else
    pid_t cpid = fork();
    if (cpid < 0) {
        TRACELOG(LOG_ERROR, "Could not fork child process: %s", strerror(errno));
        return INVALID_PROC;
    }

    if (cpid == 0) {
        // NOTE: This leaks a bit of memory in the child process.
        // But do we actually care? It's a one off leak anyway...
        CMD cmd_null = {0};
        da_append_many(&cmd_null, cmd.data, cmd.count);
        cmd_append(&cmd_null, NULL);

        if (execvp(cmd.data[0], (char * const*) cmd_null.data) < 0) {
            TRACELOG(LOG_ERROR, "Could not exec child process: %s", strerror(errno));
            exit(1);
        }
        PL_ASSERT(0 && "unreachable");
    }

    return cpid;
#endif
}

bool proc_wait(Proc proc)
{
    if (proc == INVALID_PROC) return false;

#ifdef _WIN32
    DWORD result = WaitForSingleObject(
            proc,    // HANDLE hHandle,
            INFINITE // DWORD  dwMilliseconds
            );

    if (result == WAIT_FAILED) {
        TRACELOG(LOG_ERROR, "could not wait on child process: %lu", GetLastError());
        return false;
    }

    DWORD exit_status;
    if (!GetExitCodeProcess(proc, &exit_status)) {
        TRACELOG(LOG_ERROR, "could not get process exit code: %lu", GetLastError());
        return false;
    }

    if (exit_status != 0) {
        TRACELOG(LOG_ERROR, "command exited with exit code %lu", exit_status);
        return false;
    }

    CloseHandle(proc);

    return true;
#else
    for (;;) {
        int wstatus = 0;
        if (waitpid(proc, &wstatus, 0) < 0) {
            TRACELOG(LOG_ERROR, "could not wait on command (pid %d): %s", proc, strerror(errno));
            return false;
        }

        if (WIFEXITED(wstatus)) {
            int exit_status = WEXITSTATUS(wstatus);
            if (exit_status != 0) {
                TRACELOG(LOG_ERROR, "command exited with exit code %d", exit_status);
                return false;
            }

            break;
        }

        if (WIFSIGNALED(wstatus)) {
            TRACELOG(LOG_ERROR, "command process was terminated by %s", strsignal(WTERMSIG(wstatus)));
            return false;
        }
    }
    return true;
#endif
}

bool cmd_exec(CMD cmd)
{
    Proc p = cmd_exec_async(cmd);
    if (p == INVALID_PROC) return false;
    return proc_wait(p);
}

void cmd_to_string(CMD cmd, String_Builder *render)
{
    for (size_t i = 0; i < cmd.count; ++i) {
        const char *arg = cmd.data[i];
        if (arg == NULL) break;
        if (i > 0) sb_append_cstr(render, " ");
        if (!strchr(arg, ' ')) {
            sb_append_cstr(render, arg);
        } else {
            da_append(render, '\'');
            sb_append_cstr(render, arg);
            da_append(render, '\'');
        }
    }
    sb_append_null(render);
}

void pl_tracelog(Log_Level level, const char *fmt, ...)
{
    FILE *out = level == LOG_ERROR ? stderr : stdout;
    switch (level) {
        case LOG_ERROR:
            fprintf(out, "[ERROR] ");
            break;
        case LOG_WARN:
            fprintf(out, "[WARN] ");
            break;
        case LOG_INFO:
            fprintf(out, "[INFO] ");
            break;
        default:
            break;
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(out, fmt, args);
    va_end(args);
    fprintf(out, "\n");
}
