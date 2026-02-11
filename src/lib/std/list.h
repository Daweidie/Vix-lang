#ifndef VIX_LIST_H
#define VIX_LIST_H

#include <stddef.h>
#include <stdbool.h>

// 列表结构体定义
typedef struct {
    void **data;        // 指向元素指针数组的指针
    size_t size;        // 当前元素数量
    size_t capacity;    // 当前容量
    size_t element_size; // 单个元素的大小（用于复制）
} list_t;

// 初始化列表
list_t* list_create(size_t element_size);

// 释放列表内存
void list_destroy(list_t *list);

// 获取列表大小
size_t list_size(const list_t *list);

// 检查列表是否为空
bool list_empty(const list_t *list);

// 在末尾添加元素
int list_push_back(list_t *list, const void *element);

// 在开头添加元素
int list_push_front(list_t *list, const void *element);

// 移除并返回末尾元素
void* list_pop_back(list_t *list);

// 移除并返回开头元素
void* list_pop_front(list_t *list);

// 获取指定索引的元素
void* list_get(const list_t *list, size_t index);

// 设置指定索引的元素
int list_set(list_t *list, size_t index, const void *element);

// 在指定位置插入元素
int list_insert(list_t *list, size_t index, const void *element);

// 移除指定位置的元素
void* list_remove(list_t *list, size_t index);

// 清空列表
void list_clear(list_t *list);

// 扩容列表
int list_reserve(list_t *list, size_t new_capacity);

#endif // VIX_LIST_H