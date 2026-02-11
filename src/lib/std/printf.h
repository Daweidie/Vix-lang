#ifndef PRINTF_H
#define PRINTF_H

// 不依赖C标准库的printf函数声明
void print(const char* str);
void println(const char* str);
int printf(const char* format, ...);

#endif // PRINTF_H