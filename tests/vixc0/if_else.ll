; ModuleID = 'vixc0'
source_filename = "vixc0"

@.fmt.i32 = internal constant [4 x i8] c"%d\0A\00"

declare i32 @printf(ptr, ...)

declare ptr @malloc(i64)

declare i32 @strcmp(ptr, ptr)

declare i32 @strlen(ptr)

define i32 @main() {
entry:
  %x = alloca i32, align 4
  store i32 0, ptr %x, align 4
  br i1 false, label %if.then0, label %if.else1

if.then0:                                         ; preds = %entry
  store i32 1, ptr %x, align 4
  br label %if.end2

if.else1:                                         ; preds = %entry
  br i1 true, label %if.then3, label %if.else4

if.end2:                                          ; preds = %if.end5, %if.then0
  %load2 = load i32, ptr %x, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 %load2)
  ret i32 0

if.then3:                                         ; preds = %if.else1
  store i32 2, ptr %x, align 4
  br label %if.end5

if.else4:                                         ; preds = %if.else1
  store i32 3, ptr %x, align 4
  br label %if.end5

if.end5:                                          ; preds = %if.else4, %if.then3
  br label %if.end2
}
