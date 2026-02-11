#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../list.h"

// 测试整数列表
void test_int_list() {
    printf("Testing integer list operations...\n");
    
    // 创建整数列表
    list_t *list = list_create(sizeof(int));
    assert(list != NULL);
    assert(list_empty(list) == true);
    assert(list_size(list) == 0);
    
    // 测试 push_back
    int val1 = 10, val2 = 20, val3 = 30;
    assert(list_push_back(list, &val1) == 0);
    assert(list_size(list) == 1);
    assert(list_empty(list) == false);
    
    assert(list_push_back(list, &val2) == 0);
    assert(list_size(list) == 2);
    
    // 测试 get
    int *result = (int*)list_get(list, 0);
    assert(result != NULL);
    assert(*result == 10);
    
    result = (int*)list_get(list, 1);
    assert(result != NULL);
    assert(*result == 20);
    
    // 测试 push_front
    assert(list_push_front(list, &val3) == 0);
    assert(list_size(list) == 3);
    
    result = (int*)list_get(list, 0);
    assert(*result == 30);
    result = (int*)list_get(list, 1);
    assert(*result == 10);
    result = (int*)list_get(list, 2);
    assert(*result == 20);
    
    // 测试 pop_back
    void *popped = list_pop_back(list);
    assert(popped != NULL);
    assert(*(int*)popped == 20);
    assert(list_size(list) == 2);
    free(popped);
    
    // 测试 pop_front
    popped = list_pop_front(list);
    assert(popped != NULL);
    assert(*(int*)popped == 30);
    assert(list_size(list) == 1);
    free(popped);
    
    // 测试 set
    int new_val = 99;
    assert(list_set(list, 0, &new_val) == 0);
    result = (int*)list_get(list, 0);
    assert(*result == 99);
    
    // 测试 insert
    int insert_val = 55;
    assert(list_insert(list, 0, &insert_val) == 0);
    assert(list_size(list) == 2);
    result = (int*)list_get(list, 0);
    assert(*result == 55);
    result = (int*)list_get(list, 1);
    assert(*result == 99);
    
    // 测试 remove
    popped = list_remove(list, 0);
    assert(popped != NULL);
    assert(*(int*)popped == 55);
    assert(list_size(list) == 1);
    free(popped);
    
    // 测试 clear
    list_clear(list);
    assert(list_size(list) == 0);
    assert(list_empty(list) == true);
    
    // 重新添加一些元素并测试 reserve
    assert(list_push_back(list, &val1) == 0);
    assert(list_push_back(list, &val2) == 0);
    assert(list_reserve(list, 10) == 0);
    assert(list->capacity >= 10);
    
    list_destroy(list);
    printf("✓ Integer list tests passed!\n\n");
}

// 测试字符串列表
void test_string_list() {
    printf("Testing string list operations...\n");
    
    list_t *list = list_create(sizeof(char*));
    assert(list != NULL);
    
    char *str1 = "hello";
    char *str2 = "world";
    char *str3 = "vix";
    
    // 添加字符串
    assert(list_push_back(list, &str1) == 0);
    assert(list_push_back(list, &str2) == 0);
    assert(list_push_front(list, &str3) == 0);
    
    assert(list_size(list) == 3);
    
    // 验证内容
    char **result = (char**)list_get(list, 0);
    assert(strcmp(*result, "vix") == 0);
    
    result = (char**)list_get(list, 1);
    assert(strcmp(*result, "hello") == 0);
    
    result = (char**)list_get(list, 2);
    assert(strcmp(*result, "world") == 0);
    
    // 测试 pop
    void *popped = list_pop_back(list);
    assert(popped != NULL);
    assert(strcmp(*(char**)popped, "world") == 0);
    free(popped);
    
    assert(list_size(list) == 2);
    
    list_destroy(list);
    printf("✓ String list tests passed!\n\n");
}

// 测试边界情况
void test_edge_cases() {
    printf("Testing edge cases...\n");
    
    list_t *list = list_create(sizeof(int));
    assert(list != NULL);
    
    // 测试空列表操作
    assert(list_get(list, 0) == NULL);
    assert(list_pop_back(list) == NULL);
    assert(list_pop_front(list) == NULL);
    assert(list_remove(list, 0) == NULL);
    
    // 测试无效索引
    int val = 42;
    assert(list_push_back(list, &val) == 0);
    assert(list_get(list, 1) == NULL);
    assert(list_remove(list, 1) == NULL);
    assert(list_set(list, 1, &val) == -1);
    assert(list_insert(list, 2, &val) == -1);
    
    // 测试 NULL 参数
    assert(list_push_back(NULL, &val) == -1);
    assert(list_push_back(list, NULL) == -1);
    
    list_destroy(list);
    
    // 测试零大小元素
    list_t *invalid_list = list_create(0);
    assert(invalid_list == NULL);
    
    printf("✓ Edge case tests passed!\n\n");
}

// 测试内存管理
void test_memory_management() {
    printf("Testing memory management...\n");
    
    list_t *list = list_create(sizeof(int));
    assert(list != NULL);
    
    // 添加大量元素
    for (int i = 0; i < 100; i++) {
        assert(list_push_back(list, &i) == 0);
    }
    
    assert(list_size(list) == 100);
    
    // 验证所有元素
    for (int i = 0; i < 100; i++) {
        int *val = (int*)list_get(list, i);
        assert(val != NULL);
        assert(*val == i);
    }
    
    // 清空并重新填充
    list_clear(list);
    assert(list_size(list) == 0);
    
    int val = 123;
    assert(list_push_back(list, &val) == 0);
    assert(list_size(list) == 1);
    
    list_destroy(list);
    printf("✓ Memory management tests passed!\n\n");
}

int main() {
    printf("Running List Library Tests\n");
    printf("=========================\n\n");
    
    test_int_list();
    test_string_list();
    test_edge_cases();
    test_memory_management();
    
    printf("All tests passed! ✓\n");
    return 0;
}