#include "str_list.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// 初始化字符串列表
str_list_t* str_list_create(void) {
    str_list_t *list = (str_list_t*)malloc(sizeof(str_list_t));
    if (!list) {
        return NULL;
    }
    
    list->data = NULL;
    list->size = 0;
    list->capacity = 0;
    list->scalar_from_string = false;
    
    return list;
}

// 从单个字符串初始化列表（设置scalar_from_string标志）
str_list_t* str_list_from_string(const char *str) {
    if (!str) {
        return NULL;
    }
    
    str_list_t *list = str_list_create();
    if (!list) {
        return NULL;
    }
    
    // 扩容到至少1个元素的容量
    if (list->capacity == 0) {
        size_t new_capacity = 4;
        char **new_data = (char**)realloc(list->data, new_capacity * sizeof(char*));
        if (!new_data) {
            free(list);
            return NULL;
        }
        list->data = new_data;
        list->capacity = new_capacity;
    }
    
    // 复制字符串
    char *new_str = strdup(str);
    if (!new_str) {
        free(list->data);
        free(list);
        return NULL;
    }
    
    list->data[0] = new_str;
    list->size = 1;
    list->scalar_from_string = true;
    
    return list;
}

// 释放字符串列表内存
void str_list_destroy(str_list_t *list) {
    if (!list) {
        return;
    }
    
    if (list->data) {
        for (size_t i = 0; i < list->size; i++) {
            free(list->data[i]);
        }
        free(list->data);
    }
    free(list);
}

// 获取列表大小
size_t str_list_size(const str_list_t *list) {
    if (!list) {
        return 0;
    }
    return list->size;
}

// 检查列表是否为空
bool str_list_empty(const str_list_t *list) {
    return str_list_size(list) == 0;
}

// 扩容列表（内部使用）
static int str_list_grow(str_list_t *list) {
    if (!list) {
        return -1;
    }
    
    size_t new_capacity = list->capacity == 0 ? 4 : list->capacity * 2;
    char **new_data = (char**)realloc(list->data, new_capacity * sizeof(char*));
    if (!new_data) {
        return -1;
    }
    
    list->data = new_data;
    list->capacity = new_capacity;
    return 0;
}

// 在指定位置添加元素（inplace操作）
int str_list_add_inplace(str_list_t *list, size_t idx, const char *val) {
    if (!list || !val || idx > list->size) {
        return -1;
    }
    
    // 如果容量不足，扩容
    if (list->size >= list->capacity) {
        if (str_list_grow(list) != 0) {
            return -1;
        }
    }
    
    // 将idx及之后的元素向后移动一位
    for (size_t i = list->size; i > idx; i--) {
        list->data[i] = list->data[i - 1];
    }
    
    // 复制字符串
    char *new_str = strdup(val);
    if (!new_str) {
        // 如果插入失败，需要将元素移回原位
        for (size_t i = idx; i < list->size; i++) {
            list->data[i] = list->data[i + 1];
        }
        return -1;
    }
    
    list->data[idx] = new_str;
    list->size++;
    list->scalar_from_string = false;
    
    return 0;
}

// 移除并返回指定位置的元素
char* str_list_remove(str_list_t *list, size_t idx) {
    if (!list || idx >= list->size) {
        return NULL;
    }
    
    char *element = list->data[idx];
    
    // 将idx之后的元素向前移动一位
    for (size_t i = idx; i < list->size - 1; i++) {
        list->data[i] = list->data[i + 1];
    }
    
    list->size--;
    list->scalar_from_string = false;
    
    return element;
}

// 移除指定位置的元素（inplace操作，返回列表本身）
int str_list_remove_inplace(str_list_t *list, size_t idx) {
    if (!list || idx >= list->size) {
        return -1;
    }
    
    // 释放要删除的字符串
    free(list->data[idx]);
    
    // 将idx之后的元素向前移动一位
    for (size_t i = idx; i < list->size - 1; i++) {
        list->data[i] = list->data[i + 1];
    }
    
    list->size--;
    list->scalar_from_string = false;
    
    return 0;
}

// 在末尾添加元素（inplace操作）
int str_list_push_inplace(str_list_t *list, const char *val) {
    if (!list || !val) {
        return -1;
    }
    
    // 如果容量不足，扩容
    if (list->size >= list->capacity) {
        if (str_list_grow(list) != 0) {
            return -1;
        }
    }
    
    // 复制字符串
    char *new_str = strdup(val);
    if (!new_str) {
        return -1;
    }
    
    list->data[list->size] = new_str;
    list->size++;
    list->scalar_from_string = false;
    
    return 0;
}

// 在末尾添加元素并返回添加的值
char* str_list_push(str_list_t *list, const char *val) {
    if (!list || !val) {
        return NULL;
    }
    
    // 如果容量不足，扩容
    if (list->size >= list->capacity) {
        if (str_list_grow(list) != 0) {
            return NULL;
        }
    }
    
    // 复制字符串
    char *new_str = strdup(val);
    if (!new_str) {
        return NULL;
    }
    
    list->data[list->size] = new_str;
    list->size++;
    list->scalar_from_string = false;
    
    return new_str;
}

// 移除并返回末尾元素
char* str_list_pop(str_list_t *list) {
    if (!list || list->size == 0) {
        return NULL;
    }
    
    char *element = list->data[list->size - 1];
    list->size--;
    list->scalar_from_string = false;
    
    return element;
}

// 移除末尾元素（inplace操作，返回列表本身）
int str_list_pop_inplace(str_list_t *list) {
    if (!list || list->size == 0) {
        return -1;
    }
    
    // 释放末尾字符串
    free(list->data[list->size - 1]);
    list->size--;
    list->scalar_from_string = false;
    
    return 0;
}

// 替换指定位置的元素（inplace操作）
int str_list_replace_inplace(str_list_t *list, size_t idx, const char *val) {
    if (!list || idx >= list->size || !val) {
        return -1;
    }
    
    // 释放旧字符串
    free(list->data[idx]);
    
    // 复制新字符串
    char *new_str = strdup(val);
    if (!new_str) {
        return -1;
    }
    
    list->data[idx] = new_str;
    list->scalar_from_string = false;
    
    return 0;
}

// 赋值操作：将列表设置为包含单个字符串
int str_list_assign_string(str_list_t *list, const char *str) {
    if (!list || !str) {
        return -1;
    }
    
    // 清空现有内容
    str_list_clear(list);
    
    // 扩容到至少1个元素的容量
    if (list->capacity == 0) {
        size_t new_capacity = 4;
        char **new_data = (char**)realloc(list->data, new_capacity * sizeof(char*));
        if (!new_data) {
            return -1;
        }
        list->data = new_data;
        list->capacity = new_capacity;
    }
    
    // 复制字符串
    char *new_str = strdup(str);
    if (!new_str) {
        return -1;
    }
    
    list->data[0] = new_str;
    list->size = 1;
    list->scalar_from_string = true;
    
    return 0;
}

// 获取指定索引的元素
const char* str_list_get(const str_list_t *list, size_t index) {
    if (!list || index >= list->size) {
        return NULL;
    }
    
    return list->data[index];
}

// 设置指定索引的元素（内部使用，不重置scalar_from_string标志）
int str_list_set_internal(str_list_t *list, size_t index, const char *element) {
    if (!list || index >= list->size || !element) {
        return -1;
    }
    
    // 释放旧字符串
    free(list->data[index]);
    
    // 复制新字符串
    char *new_str = strdup(element);
    if (!new_str) {
        return -1;
    }
    
    list->data[index] = new_str;
    
    return 0;
}

// 清空列表
void str_list_clear(str_list_t *list) {
    if (!list) {
        return;
    }
    
    // 释放所有字符串的内存
    for (size_t i = 0; i < list->size; i++) {
        free(list->data[i]);
    }
    
    list->size = 0;
    list->scalar_from_string = false;
}

// 扩容列表到指定容量
int str_list_reserve(str_list_t *list, size_t new_capacity) {
    if (!list || new_capacity <= list->capacity) {
        return 0;
    }
    
    char **new_data = (char**)realloc(list->data, new_capacity * sizeof(char*));
    if (!new_data) {
        return -1;
    }
    
    list->data = new_data;
    list->capacity = new_capacity;
    return 0;
}

// 打印列表（根据scalar_from_string标志决定输出格式）
void str_list_print(const str_list_t *list) {
    if (!list) {
        return;
    }
    
    if (list->scalar_from_string && list->size == 1) {
        printf("%s", list->data[0]);
    } else {
        printf("[");
        for (size_t i = 0; i < list->size; ++i) { 
            if (i) printf(", "); 
            printf("%s", list->data[i]); 
        }
        printf("]");
    }
}