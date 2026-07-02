#include <stddef.h>
#include <stdlib.h>
#include <string.h>

int vix_array_len(void *arr) {
  if (arr == NULL)
    return 0;
  int *header = (int *)((char *)arr - 8);
  return *header;
}

void *vix_array_push_i32(void *arr, int val) {
  int old_len = vix_array_len(arr);
  int new_len = old_len + 1;
  void *base = (arr == NULL) ? NULL : (void *)((char *)arr - 8);
  size_t data_bytes = (size_t)new_len * sizeof(int);
  size_t total_bytes = 8 + data_bytes;
  void *new_block = realloc(base, total_bytes);
  if (new_block == NULL)
    return NULL;
  *(int *)new_block = new_len;
  int *data = (int *)((char *)new_block + 8);
  data[old_len] = val;
  return (void *)((char *)new_block + 8);
}

void *vix_array_push_ptr(void *arr, void *val) {
  int old_len = vix_array_len(arr);
  int new_len = old_len + 1;
  void *base = (arr == NULL) ? NULL : (void *)((char *)arr - 8);
  size_t data_bytes = (size_t)new_len * sizeof(void *);
  size_t total_bytes = 8 + data_bytes;
  void *new_block = realloc(base, total_bytes);
  if (new_block == NULL)
    return NULL;
  *(int *)new_block = new_len;
  void **data = (void **)((char *)new_block + 8);
  data[old_len] = val;
  return (void *)((char *)new_block + 8);
}

void *vix_array_push_bytes(void *arr, void *val, size_t elem_size) {
  if (elem_size == 0)
    return arr;
  int old_len = vix_array_len(arr);
  int new_len = old_len + 1;
  void *base = (arr == NULL) ? NULL : (void *)((char *)arr - 8);
  size_t data_bytes = (size_t)new_len * elem_size;
  size_t total_bytes = 8 + data_bytes;
  void *new_block = realloc(base, total_bytes);
  if (new_block == NULL)
    return NULL;
  *(int *)new_block = new_len;
  char *data = (char *)new_block + 8;
  memcpy(data + ((size_t)old_len * elem_size), val, elem_size);
  return (void *)data;
}

void *vix_string_concat(const char *a, const char *b) {
  if (a == NULL)
    a = "";
  if (b == NULL)
    b = "";
  size_t len_a = strlen(a);
  size_t len_b = strlen(b);
  size_t total = len_a + len_b + 1;
  char *result = (char *)malloc(total);
  if (result == NULL)
    return NULL;
  memcpy(result, a, len_a);
  memcpy(result + len_a, b, len_b);
  result[len_a + len_b] = '\0';
  return result;
}
