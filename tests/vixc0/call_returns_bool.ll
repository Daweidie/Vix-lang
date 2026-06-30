; ModuleID = 'vixc0'
source_filename = "vixc0"

@.fmt.i32 = internal constant [4 x i8] c"%d\0A\00"

declare i32 @printf(ptr, ...)

declare ptr @malloc(i64)

declare i32 @strcmp(ptr, ptr)

declare i32 @strlen(ptr)

define i32 @yes() {
entry:
  ret i32 1
}

define i32 @main() {
entry:
  %call0 = call i32 @yes()
  %cond1 = icmp ne i32 %call0, 0
  br i1 %cond1, label %if.then0, label %if.else1

if.then0:                                         ; preds = %entry
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 1)
  br label %if.end2

if.else1:                                         ; preds = %entry
  %1 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 0)
  br label %if.end2

if.end2:                                          ; preds = %if.else1, %if.then0
  ret i32 0
}
