#ifndef VIX_STR_LIST_H
#define VIX_STR_LIST_H

#include <stddef.h>
#include <stdbool.h>

// 字符串列表结构体定义
typedef struct {
    char **data;        // 指向字符串指针数组的指针
    size_t size;        // 当前元素数量
    size_t capacity;    // 当前容量
    bool scalar_from_string; // 标记是否从单个字符串转换而来（用于输出格式）
} str_list_t;

// 初始化字符串列表
str_list_t* str_list_create(void);

// 从单个字符串初始化列表（设置scalar_from_string标志）
str_list_t* str_list_from_string(const char *str);

// 释放字符串列表内存
void str_list_destroy(str_list_t *list);

// 获取列表大小
size_t str_list_size(const str_list_t *list);

// 检查列表是否为空
bool str_list_empty(const str_list_t *list);

// 在指定位置添加元素（inplace操作）
int str_list_add_inplace(str_list_t *list, size_t idx, const char *val);

// 移除并返回指定位置的元素
char* str_list_remove(str_list_t *list, size_t idx);

// 移除指定位置的元素（inplace操作，返回列表本身）
int str_list_remove_inplace(str_list_t *list, size_t idx);

// 在末尾添加元素（inplace操作）
int str_list_push_inplace(str_list_t *list, const char *val);

// 在末尾添加元素并返回添加的值
char* str_list_push(str_list_t *list, const char *val);

// 移除并返回末尾元素
char* str_list_pop(str_list_t *list);

// 移除末尾元素（inplace操作，返回列表本身）
int str_list_pop_inplace(str_list_t *list);

// 替换指定位置的元素（inplace操作）
int str_list_replace_inplace(str_list_t *list, size_t idx, const char *val);

// 赋值操作：将列表设置为包含单个字符串
int str_list_assign_string(str_list_t *list, const char *str);

// 获取指定索引的元素
const char* str_list_get(const str_list_t *list, size_t index);

// 设置指定索引的元素（内部使用，不重置scalar_from_string标志）
int str_list_set_internal(str_list_t *list, size_t index, const char *element);

// 清空列表
void str_list_clear(str_list_t *list);

// 扩容列表到指定容量
int str_list_reserve(str_list_t *list, size_t new_capacity);

// 打印列表（根据scalar_from_string标志决定输出格式）
void str_list_print(const str_list_t *list);

#endif // VIX_STR_LIST_H