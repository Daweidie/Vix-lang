; ModuleID = 'vixc0'
source_filename = "vixc0"

@.fmt.i32 = internal constant [4 x i8] c"%d\0A\00"

declare i32 @printf(ptr, ...)

declare ptr @malloc(i64)

declare i32 @strcmp(ptr, ptr)

declare i32 @strlen(ptr)

define i32 @main() {
entry:
  %i = alloca i32, align 4
  store i32 0, ptr %i, align 4
  br label %while.cond0

while.cond0:                                      ; preds = %if.end5, %if.then3, %entry
  %load0 = load i32, ptr %i, align 4
  %cmp1 = icmp slt i32 %load0, 5
  %bool2 = zext i1 %cmp1 to i32
  %cond3 = icmp ne i32 %bool2, 0
  br i1 %cond3, label %while.body1, label %while.end2

while.body1:                                      ; preds = %while.cond0
  %load4 = load i32, ptr %i, align 4
  %add5 = add i32 %load4, 1
  store i32 %add5, ptr %i, align 4
  %load6 = load i32, ptr %i, align 4
  %cmp7 = icmp eq i32 %load6, 3
  %bool8 = zext i1 %cmp7 to i32
  %cond9 = icmp ne i32 %bool8, 0
  br i1 %cond9, label %if.then3, label %if.else4

while.end2:                                       ; preds = %while.cond0
  ret i32 0

if.then3:                                         ; preds = %while.body1
  br label %while.cond0

if.else4:                                         ; preds = %while.body1
  br label %if.end5

if.end5:                                          ; preds = %if.else4
  %load10 = load i32, ptr %i, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 %load10)
  br label %while.cond0
}
