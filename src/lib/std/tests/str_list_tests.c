#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../str_list.h"

// 添加一个辅助函数来显示列表的详细状态
void debug_print_list(const char* operation, str_list_t *list) {
    printf("DEBUG [%s]: ", operation);
    if (!list) {
        printf("NULL list\n");
        return;
    }
    printf("size=%zu, capacity=%zu, scalar_from_string=%s, content=", 
           list->size, list->capacity, list->scalar_from_string ? "true" : "false");
    str_list_print(list);
    printf("\n");
}

// 测试基本字符串列表操作
void test_basic_operations() {
    printf("Testing basic string list operations...\n");
    
    // 创建空列表
    str_list_t *list = str_list_create();
    assert(list != NULL);
    assert(str_list_empty(list) == true);
    assert(str_list_size(list) == 0);
    assert(list->scalar_from_string == false);
    debug_print_list("after create", list);
    
    // 测试 push_inplace
    assert(str_list_push_inplace(list, "hello") == 0);
    assert(str_list_size(list) == 1);
    assert(str_list_empty(list) == false);
    assert(strcmp(str_list_get(list, 0), "hello") == 0);
    assert(list->scalar_from_string == false);
    debug_print_list("after push 'hello'", list);
    
    assert(str_list_push_inplace(list, "world") == 0);
    assert(str_list_size(list) == 2);
    assert(strcmp(str_list_get(list, 1), "world") == 0);
    debug_print_list("after push 'world'", list);
    
    // 测试 pop
    char *popped = str_list_pop(list);
    assert(popped != NULL);
    assert(strcmp(popped, "world") == 0);
    assert(str_list_size(list) == 1);
    assert(list->scalar_from_string == false);
    debug_print_list("after pop", list);
    free(popped);
    
    // 测试 pop_inplace
    assert(str_list_push_inplace(list, "test") == 0);
    debug_print_list("before pop_inplace", list);
    assert(str_list_pop_inplace(list) == 0);
    assert(str_list_size(list) == 1);
    assert(list->scalar_from_string == false);
    debug_print_list("after pop_inplace", list);
    
    // 测试 replace_inplace
    assert(str_list_replace_inplace(list, 0, "replaced") == 0);
    assert(strcmp(str_list_get(list, 0), "replaced") == 0);
    assert(list->scalar_from_string == false);
    debug_print_list("after replace_inplace", list);
    
    str_list_destroy(list);
    printf("✓ Basic operations tests passed!\n\n");
}

// 测试 add_inplace 和 remove 操作
void test_add_remove_operations() {
    printf("Testing add_inplace and remove operations...\n");
    
    str_list_t *list = str_list_create();
    assert(list != NULL);
    
    // 添加多个元素
    assert(str_list_push_inplace(list, "a") == 0);
    assert(str_list_push_inplace(list, "b") == 0);
    assert(str_list_push_inplace(list, "c") == 0);
    debug_print_list("after adding a,b,c", list);
    
    // 在开头添加
    assert(str_list_add_inplace(list, 0, "x") == 0);
    assert(str_list_size(list) == 4);
    assert(strcmp(str_list_get(list, 0), "x") == 0);
    assert(strcmp(str_list_get(list, 1), "a") == 0);
    debug_print_list("after add_inplace at 0", list);
    
    // 在中间添加
    assert(str_list_add_inplace(list, 2, "y") == 0);
    assert(str_list_size(list) == 5);
    assert(strcmp(str_list_get(list, 2), "y") == 0);
    assert(strcmp(str_list_get(list, 3), "b") == 0);
    debug_print_list("after add_inplace at 2", list);
    
    // 测试 remove (返回值)
    char *removed = str_list_remove(list, 2);
    assert(removed != NULL);
    assert(strcmp(removed, "y") == 0);
    assert(str_list_size(list) == 4);
    assert(strcmp(str_list_get(list, 2), "b") == 0);
    debug_print_list("after remove at 2", list);
    free(removed);
    
    // 测试 remove_inplace
    assert(str_list_remove_inplace(list, 0) == 0);
    assert(str_list_size(list) == 3);
    assert(strcmp(str_list_get(list, 0), "a") == 0);
    debug_print_list("after remove_inplace at 0", list);
    
    str_list_destroy(list);
    printf("✓ Add/remove operations tests passed!\n\n");
}

// 测试从字符串初始化和赋值操作
void test_string_assignment() {
    printf("Testing string assignment operations...\n");
    
    // 从单个字符串创建列表
    str_list_t *list = str_list_from_string("single");
    assert(list != NULL);
    assert(str_list_size(list) == 1);
    assert(strcmp(str_list_get(list, 0), "single") == 0);
    assert(list->scalar_from_string == true);
    debug_print_list("after from_string 'single'", list);
    
    // 赋值操作
    assert(str_list_assign_string(list, "assigned") == 0);
    assert(str_list_size(list) == 1);
    assert(strcmp(str_list_get(list, 0), "assigned") == 0);
    assert(list->scalar_from_string == true);
    debug_print_list("after assign_string 'assigned'", list);
    
    // 添加更多元素后 scalar_from_string 应该变为 false
    assert(str_list_push_inplace(list, "extra") == 0);
    assert(list->scalar_from_string == false);
    debug_print_list("after push 'extra'", list);
    
    // 再次赋值应该恢复 scalar_from_string
    assert(str_list_assign_string(list, "reset") == 0);
    assert(str_list_size(list) == 1);
    assert(list->scalar_from_string == true);
    debug_print_list("after assign_string 'reset'", list);
    
    str_list_destroy(list);
    printf("✓ String assignment tests passed!\n\n");
}

// 测试打印功能
void test_print_functionality() {
    printf("Testing print functionality...\n");
    
    // 测试 scalar_from_string 为 true 的情况
    str_list_t *list = str_list_from_string("hello");
    printf("Scalar list output: ");
    str_list_print(list);
    printf("\n");
    // 应该只输出 "hello" 而不是 "[hello]"
    
    // 测试普通列表
    assert(str_list_push_inplace(list, "world") == 0);
    printf("Regular list output: ");
    str_list_print(list);
    printf("\n");
    // 应该输出 "[hello, world]"
    
    str_list_destroy(list);
    printf("✓ Print functionality tests passed!\n\n");
}

// 测试边界情况
void test_edge_cases() {
    printf("Testing edge cases...\n");
    
    // NULL 参数测试
    assert(str_list_create() != NULL);
    assert(str_list_from_string(NULL) == NULL);
    
    str_list_t *list = str_list_create();
    assert(list != NULL);
    
    // 无效索引测试
    assert(str_list_get(list, 0) == NULL);
    assert(str_list_remove(list, 0) == NULL);
    assert(str_list_remove_inplace(list, 0) == -1);
    assert(str_list_replace_inplace(list, 0, "test") == -1);
    
    // 添加一个元素后测试边界
    assert(str_list_push_inplace(list, "test") == 0);
    debug_print_list("after adding 'test'", list);
    assert(str_list_get(list, 1) == NULL);
    assert(str_list_remove(list, 1) == NULL);
    assert(str_list_remove_inplace(list, 1) == -1);
    
    // NULL 字符串测试
    assert(str_list_push_inplace(list, NULL) == -1);
    assert(str_list_add_inplace(list, 0, NULL) == -1);
    assert(str_list_assign_string(list, NULL) == -1);
    
    str_list_destroy(list);
    printf("✓ Edge case tests passed!\n\n");
}

int main() {
    printf("Running String List Library Tests\n");
    printf("=================================\n\n");
    
    test_basic_operations();
    test_add_remove_operations();
    test_string_assignment();
    test_print_functionality();
    test_edge_cases();
    
    printf("All string list tests passed! ✓\n");
    return 0;
}