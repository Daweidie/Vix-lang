; ModuleID = 'vixc0'
source_filename = "vixc0"

@.fmt.i32 = internal constant [4 x i8] c"%d\0A\00"

declare i32 @printf(ptr, ...)

declare ptr @malloc(i64)

define i32 @fib(i32 %0) {
entry:
  %n = alloca i32, align 4
  store i32 %0, ptr %n, align 4
  %load0 = load i32, ptr %n, align 4
  %cmp1 = icmp sle i32 %load0, 1
  %bool2 = zext i1 %cmp1 to i32
  %cond3 = icmp ne i32 %bool2, 0
  br i1 %cond3, label %if.then0, label %if.else1

if.then0:                                         ; preds = %entry
  %load4 = load i32, ptr %n, align 4
  ret i32 %load4

if.else1:                                         ; preds = %entry
  br label %if.end2

if.end2:                                          ; preds = %if.else1
  %load5 = load i32, ptr %n, align 4
  %sub6 = sub i32 %load5, 1
  %call7 = call i32 @fib(i32 %sub6)
  %load8 = load i32, ptr %n, align 4
  %sub9 = sub i32 %load8, 2
  %call10 = call i32 @fib(i32 %sub9)
  %add11 = add i32 %call7, %call10
  ret i32 %add11
}

define i32 @main() {
entry:
  %call12 = call i32 @fib(i32 40)
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 %call12)
  ret i32 0
}

