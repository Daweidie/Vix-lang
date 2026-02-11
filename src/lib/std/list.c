#include "list.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// 初始化列表
list_t* list_create(size_t element_size) {
    if (element_size == 0) {
        return NULL;
    }
    
    list_t *list = (list_t*)malloc(sizeof(list_t));
    if (!list) {
        return NULL;
    }
    
    list->data = NULL;
    list->size = 0;
    list->capacity = 0;
    list->element_size = element_size;
    
    return list;
}

// 释放列表内存
void list_destroy(list_t *list) {
    if (!list) {
        return;
    }
    
    if (list->data) {
        free(list->data);
    }
    free(list);
}

// 获取列表大小
size_t list_size(const list_t *list) {
    if (!list) {
        return 0;
    }
    return list->size;
}

// 检查列表是否为空
bool list_empty(const list_t *list) {
    return list_size(list) == 0;
}

// 扩容列表（内部使用）
static int list_grow(list_t *list) {
    if (!list) {
        return -1;
    }
    
    size_t new_capacity = list->capacity == 0 ? 4 : list->capacity * 2;
    void **new_data = (void**)realloc(list->data, new_capacity * sizeof(void*));
    if (!new_data) {
        return -1;
    }
    
    list->data = new_data;
    list->capacity = new_capacity;
    return 0;
}

// 在末尾添加元素
int list_push_back(list_t *list, const void *element) {
    if (!list || !element) {
        return -1;
    }
    
    // 如果容量不足，扩容
    if (list->size >= list->capacity) {
        if (list_grow(list) != 0) {
            return -1;
        }
    }
    
    // 分配内存并复制元素
    void *new_element = malloc(list->element_size);
    if (!new_element) {
        return -1;
    }
    
    memcpy(new_element, element, list->element_size);
    list->data[list->size] = new_element;
    list->size++;
    
    return 0;
}

// 在开头添加元素
int list_push_front(list_t *list, const void *element) {
    if (!list || !element) {
        return -1;
    }
    
    // 如果容量不足，扩容
    if (list->size >= list->capacity) {
        if (list_grow(list) != 0) {
            return -1;
        }
    }
    
    // 将所有元素向后移动一位
    for (size_t i = list->size; i > 0; i--) {
        list->data[i] = list->data[i - 1];
    }
    
    // 分配内存并复制元素
    void *new_element = malloc(list->element_size);
    if (!new_element) {
        return -1;
    }
    
    memcpy(new_element, element, list->element_size);
    list->data[0] = new_element;
    list->size++;
    
    return 0;
}

// 移除并返回末尾元素
void* list_pop_back(list_t *list) {
    if (!list || list->size == 0) {
        return NULL;
    }
    
    void *element = list->data[list->size - 1];
    list->size--;
    return element;
}

// 移除并返回开头元素
void* list_pop_front(list_t *list) {
    if (!list || list->size == 0) {
        return NULL;
    }
    
    void *element = list->data[0];
    
    // 将所有元素向前移动一位
    for (size_t i = 0; i < list->size - 1; i++) {
        list->data[i] = list->data[i + 1];
    }
    
    list->size--;
    return element;
}

// 获取指定索引的元素
void* list_get(const list_t *list, size_t index) {
    if (!list || index >= list->size) {
        return NULL;
    }
    
    return list->data[index];
}

// 设置指定索引的元素
int list_set(list_t *list, size_t index, const void *element) {
    if (!list || index >= list->size || !element) {
        return -1;
    }
    
    // 释放旧元素
    free(list->data[index]);
    
    // 分配新内存并复制元素
    void *new_element = malloc(list->element_size);
    if (!new_element) {
        return -1;
    }
    
    memcpy(new_element, element, list->element_size);
    list->data[index] = new_element;
    
    return 0;
}

// 在指定位置插入元素
int list_insert(list_t *list, size_t index, const void *element) {
    if (!list || index > list->size || !element) {
        return -1;
    }
    
    // 如果容量不足，扩容
    if (list->size >= list->capacity) {
        if (list_grow(list) != 0) {
            return -1;
        }
    }
    
    // 将index及之后的元素向后移动一位
    for (size_t i = list->size; i > index; i--) {
        list->data[i] = list->data[i - 1];
    }
    
    // 分配内存并复制元素
    void *new_element = malloc(list->element_size);
    if (!new_element) {
        return -1;
    }
    
    memcpy(new_element, element, list->element_size);
    list->data[index] = new_element;
    list->size++;
    
    return 0;
}

// 移除指定位置的元素
void* list_remove(list_t *list, size_t index) {
    if (!list || index >= list->size) {
        return NULL;
    }
    
    void *element = list->data[index];
    
    // 将index之后的元素向前移动一位
    for (size_t i = index; i < list->size - 1; i++) {
        list->data[i] = list->data[i + 1];
    }
    
    list->size--;
    return element;
}

// 清空列表
void list_clear(list_t *list) {
    if (!list) {
        return;
    }
    
    // 释放所有元素的内存
    for (size_t i = 0; i < list->size; i++) {
        free(list->data[i]);
    }
    
    list->size = 0;
}

// 扩容列表到指定容量
int list_reserve(list_t *list, size_t new_capacity) {
    if (!list || new_capacity <= list->capacity) {
        return 0;
    }
    
    void **new_data = (void**)realloc(list->data, new_capacity * sizeof(void*));
    if (!new_data) {
        return -1;
    }
    
    list->data = new_data;
    list->capacity = new_capacity;
    return 0;
}