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
  %__for_end_i4 = alloca i32, align 4
  store i32 8, ptr %__for_end_i4, align 4
  br label %for.cond0

for.cond0:                                        ; preds = %for.step2, %entry
  %load0 = load i32, ptr %i, align 4
  %load1 = load i32, ptr %__for_end_i4, align 4
  %cmp2 = icmp slt i32 %load0, %load1
  %bool3 = zext i1 %cmp2 to i32
  %cond4 = icmp ne i32 %bool3, 0
  br i1 %cond4, label %for.body1, label %for.end3

for.body1:                                        ; preds = %for.cond0
  %load5 = load i32, ptr %i, align 4
  %cmp6 = icmp eq i32 %load5, 2
  %bool7 = zext i1 %cmp6 to i32
  %cond8 = icmp ne i32 %bool7, 0
  br i1 %cond8, label %if.then5, label %if.else6

for.step2:                                        ; preds = %if.end10, %if.then5
  %load14 = load i32, ptr %i, align 4
  %add15 = add i32 %load14, 1
  store i32 %add15, ptr %i, align 4
  br label %for.cond0

for.end3:                                         ; preds = %if.then8, %for.cond0
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 99)
  ret i32 0

if.then5:                                         ; preds = %for.body1
  br label %for.step2

if.else6:                                         ; preds = %for.body1
  br label %if.end7

if.end7:                                          ; preds = %if.else6
  %load9 = load i32, ptr %i, align 4
  %cmp10 = icmp eq i32 %load9, 5
  %bool11 = zext i1 %cmp10 to i32
  %cond12 = icmp ne i32 %bool11, 0
  br i1 %cond12, label %if.then8, label %if.else9

if.then8:                                         ; preds = %if.end7
  br label %for.end3

if.else9:                                         ; preds = %if.end7
  br label %if.end10

if.end10:                                         ; preds = %if.else9
  %load13 = load i32, ptr %i, align 4
  %1 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 %load13)
  br label %for.step2
}
