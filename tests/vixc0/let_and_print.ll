; ModuleID = 'vixc0'
source_filename = "vixc0"

@.fmt.i32 = internal constant [4 x i8] c"%d\0A\00"
@.fmt.i64 = internal constant [5 x i8] c"%ld\0A\00"

declare i32 @printf(ptr, ...)

declare ptr @malloc(i64)

declare i32 @strcmp(ptr, ptr)

declare i32 @strlen(ptr)

define i32 @main() {
entry:
  %a = alloca i32, align 4
  store i32 10, ptr %a, align 4
  %b = alloca i64, align 8
  store i64 20, ptr %b, align 4
  %load0 = load i32, ptr %a, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 %load0)
  %load1 = load i64, ptr %b, align 4
  %1 = call i32 (ptr, ...) @printf(ptr @.fmt.i64, i64 %load1)
  ret i32 0
}
