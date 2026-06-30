; ModuleID = 'vixc0'
source_filename = "vixc0"

@.fmt.i32 = internal constant [4 x i8] c"%d\0A\00"

declare i32 @printf(ptr, ...)

declare ptr @malloc(i64)

declare i32 @strcmp(ptr, ptr)

declare i32 @strlen(ptr)

define i32 @main() {
entry:
  %count = alloca i32, align 4
  store i32 0, ptr %count, align 4
  %i = alloca i32, align 4
  store i32 5, ptr %i, align 4
  %__for_end_i4 = alloca i32, align 4
  store i32 5, ptr %__for_end_i4, align 4
  br label %for.cond0

for.cond0:                                        ; preds = %for.step2, %entry
  %load0 = load i32, ptr %i, align 4
  %load1 = load i32, ptr %__for_end_i4, align 4
  %cmp2 = icmp slt i32 %load0, %load1
  %bool3 = zext i1 %cmp2 to i32
  %cond4 = icmp ne i32 %bool3, 0
  br i1 %cond4, label %for.body1, label %for.end3

for.body1:                                        ; preds = %for.cond0
  %load5 = load i32, ptr %count, align 4
  %add6 = add i32 %load5, 1
  store i32 %add6, ptr %count, align 4
  br label %for.step2

for.step2:                                        ; preds = %for.body1
  %load7 = load i32, ptr %i, align 4
  %add8 = add i32 %load7, 1
  store i32 %add8, ptr %i, align 4
  br label %for.cond0

for.end3:                                         ; preds = %for.cond0
  %load9 = load i32, ptr %count, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 %load9)
  ret i32 0
}
