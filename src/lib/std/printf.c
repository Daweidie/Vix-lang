// 不依赖C标准库的printf实现
// 适用于Linux x86_64系统

// 系统调用函数
static void sys_write(int fd, const char* buf, int count) {
    __asm__ volatile (
        "syscall"
        :
        : "a"(1), "D"(fd), "S"(buf), "d"(count)
        : "rcx", "r11", "memory"
    );
}

// 字符串长度计算
static int my_strlen(const char* s) {
    int i = 0;
    while (s[i] != '\0') {
        i++;
    }
    return i;
}

// 反转字符串
static void reverse_string(char* str, int len) {
    int i = 0, j = len - 1;
    while (i < j) {
        char temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}

// 整数转字符串
static int int_to_string(int num, char* str) {
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return 1;
    }
    
    int negative = 0;
    if (num < 0) {
        negative = 1;
        num = -num;
    }
    
    int i = 0;
    while (num > 0) {
        str[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    if (negative) {
        str[i++] = '-';
    }
    
    str[i] = '\0';
    reverse_string(str, i);
    return i;
}

// 无符号整数转字符串
static int uint_to_string(unsigned int num, char* str) {
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return 1;
    }
    
    int i = 0;
    while (num > 0) {
        str[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    str[i] = '\0';
    reverse_string(str, i);
    return i;
}

// 十六进制转字符串
static int hex_to_string(unsigned int num, char* str, int uppercase) {
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return 1;
    }
    
    char hex_chars_lower[] = "0123456789abcdef";
    char hex_chars_upper[] = "0123456789ABCDEF";
    char* hex_chars = uppercase ? hex_chars_upper : hex_chars_lower;
    
    int i = 0;
    while (num > 0) {
        str[i++] = hex_chars[num & 0xF];
        num >>= 4;
    }
    
    str[i] = '\0';
    reverse_string(str, i);
    return i;
}

// 手动实现的va_list（简化版）
typedef char* va_list;

#define va_start(ap, last_arg) (ap = (char*)&last_arg + ((sizeof(last_arg) + 7) & ~7))
#define va_arg(ap, type) (*(type*)((ap += 8) - 8))
#define va_end(ap) ((void)0)

// 核心printf函数
int printf(const char* format, ...) {
    if (!format) return 0;
    
    va_list args;
    va_start(args, format);
    
    int total_written = 0;
    char buffer[32]; // 足够大的临时缓冲区
    
    const char* p = format;
    while (*p) {
        if (*p != '%') {
            // 普通字符，直接输出
            sys_write(1, p, 1);
            total_written++;
            p++;
            continue;
        }
        
        // 处理格式说明符
        p++; // 跳过'%'
        if (!*p) break; // 如果%是最后一个字符
        
        switch (*p) {
            case 'd': 
            case 'i': {
                int val = va_arg(args, int);
                int len = int_to_string(val, buffer);
                sys_write(1, buffer, len);
                total_written += len;
                break;
            }
            
            case 'u': {
                unsigned int val = va_arg(args, unsigned int);
                int len = uint_to_string(val, buffer);
                sys_write(1, buffer, len);
                total_written += len;
                break;
            }
            
            case 'x': {
                unsigned int val = va_arg(args, unsigned int);
                int len = hex_to_string(val, buffer, 0);
                sys_write(1, buffer, len);
                total_written += len;
                break;
            }
            
            case 'X': {
                unsigned int val = va_arg(args, unsigned int);
                int len = hex_to_string(val, buffer, 1);
                sys_write(1, buffer, len);
                total_written += len;
                break;
            }
            
            case 'c': {
                char val = (char)va_arg(args, int); // char会被提升为int
                sys_write(1, &val, 1);
                total_written++;
                break;
            }
            
            case 's': {
                char* str = va_arg(args, char*);
                if (str) {
                    int len = my_strlen(str);
                    sys_write(1, str, len);
                    total_written += len;
                } else {
                    sys_write(1, "(null)", 6);
                    total_written += 6;
                }
                break;
            }
            
            case '%': {
                sys_write(1, "%", 1);
                total_written++;
                break;
            }
            
            default: {
                // 未知格式，原样输出
                sys_write(1, "%", 1);
                sys_write(1, p, 1);
                total_written += 2;
                break;
            }
        }
        p++;
    }
    
    va_end(args);
    return total_written;
}

// 简化版本的print（只支持字符串）
void print(const char* str) {
    if (str) {
        sys_write(1, str, my_strlen(str));
    }
}

// 带换行的print
void println(const char* str) {
    print(str);
    sys_write(1, "\n", 1);
}