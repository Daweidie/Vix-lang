; ModuleID = 'vixc0'
source_filename = "vixc0"

@.fmt.i32 = internal constant [4 x i8] c"%d\0A\00"

declare i32 @printf(ptr, ...)

declare ptr @malloc(i64)

declare i32 @strcmp(ptr, ptr)

declare i32 @strlen(ptr)

define i32 @add(i32 %0, i32 %1) {
entry:
  %a = alloca i32, align 4
  store i32 %0, ptr %a, align 4
  %b = alloca i32, align 4
  store i32 %1, ptr %b, align 4
  %load0 = load i32, ptr %a, align 4
  %load1 = load i32, ptr %b, align 4
  %add2 = add i32 %load0, %load1
  ret i32 %add2
}

define i32 @main() {
entry:
  %call3 = call i32 @add(i32 2, i32 3)
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 %call3)
  ret i32 0
}
