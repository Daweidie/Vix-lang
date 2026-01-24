//io.c
//包括了linux arm64和x86_64的系统调用实现
#ifdef __linux__
    #ifdef __x86_64__
        #define SYS_WRITE 1
        #define SYS_EXIT  60
        static long syscall1(long n, long a1) {
            unsigned long ret;
            __asm__ __volatile__ (
                "syscall"
                : "=a"(ret)
                : "a"(n), "D"(a1)
                : "rcx", "r11", "memory"
            );
            return ret;
        }
        
        static long syscall3(long n, long a1, long a2, long a3) {
            unsigned long ret;
            __asm__ __volatile__ (
                "syscall"
                : "=a"(ret)
                : "a"(n), "D"(a1), "S"(a2), "d"(a3)
                : "rcx", "r11", "memory"
            );
            return ret;
        }
    #elif __aarch64__
        #define SYS_WRITE 64
        #define SYS_EXIT  93
        
        static long syscall1(long n, long a1) {
            unsigned long ret;
            __asm__ __volatile__ (
                "svc #0"
                : "=r"(ret)
                : "r"(n), "r"(a1)
                : "memory"
            );
            return ret;
        }
        
        static long syscall3(long n, long a1, long a2, long a3) {
            unsigned long ret;
            __asm__ __volatile__ (
                "svc #0"
                : "=r"(ret)
                : "r"(n), "r"(a1), "r"(a2), "r"(a3)
                : "memory"
            );
            return ret;
        }
    #else
        #error "Unsupported architecture"
    #endif
#elif _WIN32
    // Win系统需要更多工作来实现无C库运行时
    // 这里简化处理，仍然链接kernel32
    #include <windows.h>
#else
    #error "Unsupported platform"
#endif
static long strlen(const char* str) {
    long len = 0;
    while (str[len]) len++;
    return len;
}
void vix_print(const char* str) {
#ifdef __linux__
    long len = strlen(str);
    syscall3(SYS_WRITE, 1, (long)str, len);
    syscall3(SYS_WRITE, 1, (long)"\n", 1);
#elif _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written;
    WriteConsoleA(hOut, str, (DWORD)strlen(str), &written, NULL);
    WriteConsoleA(hOut, "\n", 1, &written, NULL);
#endif
}
void vix_exit(int code) {
#ifdef __linux__
    syscall1(SYS_EXIT, code);
#elif _WIN32
    ExitProcess(code);
#endif
}
int main(void);
#ifdef __linux__
void _start(void) {
    int rc = main();
    vix_exit(rc);
}
#elif _WIN32
#endif
