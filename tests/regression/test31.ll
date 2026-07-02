; ModuleID = 'vixc0'
source_filename = "vixc0"

%Result = type { i32, i32, ptr, ptr, double }

@.strlit3 = internal constant [1 x i8] zeroinitializer
@.strlit10 = internal constant [1 x i8] zeroinitializer
@.strlit16 = internal constant [1 x i8] zeroinitializer
@.strlit22 = internal constant [17 x i8] c"division by zero\00"
@.fmt.i32 = internal constant [4 x i8] c"%d\0A\00"
@.fmt.string = internal constant [4 x i8] c"%s\0A\00"

declare i32 @printf(ptr, ...)

declare ptr @malloc(i64)

declare i32 @strcmp(ptr, ptr)

declare i32 @strlen(ptr)

define %Result @Ok(i32 %0) {
entry:
  %value = alloca i32, align 4
  store i32 %0, ptr %value, align 4
  %load1 = load i32, ptr %value, align 4
  %insert2 = insertvalue %Result { i32 0, i32 undef, ptr undef, ptr undef, double undef }, i32 %load1, 1
  %insert4 = insertvalue %Result %insert2, ptr @.strlit3, 2
  %insert5 = insertvalue %Result %insert4, ptr null, 3
  %insert6 = insertvalue %Result %insert5, double 0.000000e+00, 4
  ret %Result %insert6
}

define %Result @Err(i32 %0) {
entry:
  %value = alloca i32, align 4
  store i32 %0, ptr %value, align 4
  %load8 = load i32, ptr %value, align 4
  %insert9 = insertvalue %Result { i32 1, i32 undef, ptr undef, ptr undef, double undef }, i32 %load8, 1
  %insert11 = insertvalue %Result %insert9, ptr @.strlit10, 2
  %insert12 = insertvalue %Result %insert11, ptr null, 3
  %insert13 = insertvalue %Result %insert12, double 0.000000e+00, 4
  ret %Result %insert13
}

define i32 @main() {
entry:
  %ok = alloca %Result, align 8
  store %Result { i32 0, i32 42, ptr @.strlit16, ptr null, double 0.000000e+00 }, ptr %ok, align 8
  %bad = alloca %Result, align 8
  store %Result { i32 1, i32 0, ptr @.strlit22, ptr null, double 0.000000e+00 }, ptr %bad, align 8
  %load26 = load %Result, ptr %ok, align 8
  %extract27 = extractvalue %Result %load26, 0
  %cmp28 = icmp eq i32 %extract27, 0
  %bool29 = zext i1 %cmp28 to i32
  %cond30 = icmp ne i32 %bool29, 0
  br i1 %cond30, label %if.then0, label %if.else1

if.then0:                                         ; preds = %entry
  %load31 = load %Result, ptr %ok, align 8
  %extract32 = extractvalue %Result %load31, 1
  %v = alloca i32, align 4
  store i32 %extract32, ptr %v, align 4
  %load33 = load i32, ptr %v, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 %load33)
  br label %if.end2

if.else1:                                         ; preds = %entry
  %load34 = load %Result, ptr %ok, align 8
  %extract35 = extractvalue %Result %load34, 0
  %cmp36 = icmp eq i32 %extract35, 1
  %bool37 = zext i1 %cmp36 to i32
  %cond38 = icmp ne i32 %bool37, 0
  br i1 %cond38, label %if.then3, label %if.else4

if.end2:                                          ; preds = %if.end5, %if.then0
  %load42 = load %Result, ptr %bad, align 8
  %extract43 = extractvalue %Result %load42, 0
  %cmp44 = icmp eq i32 %extract43, 0
  %bool45 = zext i1 %cmp44 to i32
  %cond46 = icmp ne i32 %bool45, 0
  br i1 %cond46, label %if.then6, label %if.else7

if.then3:                                         ; preds = %if.else1
  %load39 = load %Result, ptr %ok, align 8
  %extract40 = extractvalue %Result %load39, 2
  %e = alloca ptr, align 8
  store ptr %extract40, ptr %e, align 8
  %load41 = load ptr, ptr %e, align 8
  %1 = call i32 (ptr, ...) @printf(ptr @.fmt.string, ptr %load41)
  br label %if.end5

if.else4:                                         ; preds = %if.else1
  br label %if.end5

if.end5:                                          ; preds = %if.else4, %if.then3
  br label %if.end2

if.then6:                                         ; preds = %if.end2
  %load47 = load %Result, ptr %bad, align 8
  %extract48 = extractvalue %Result %load47, 1
  %v1 = alloca i32, align 4
  store i32 %extract48, ptr %v1, align 4
  %load49 = load i32, ptr %v1, align 4
  %2 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 %load49)
  br label %if.end8

if.else7:                                         ; preds = %if.end2
  %load50 = load %Result, ptr %bad, align 8
  %extract51 = extractvalue %Result %load50, 0
  %cmp52 = icmp eq i32 %extract51, 1
  %bool53 = zext i1 %cmp52 to i32
  %cond54 = icmp ne i32 %bool53, 0
  br i1 %cond54, label %if.then9, label %if.else10

if.end8:                                          ; preds = %if.end11, %if.then6
  ret i32 0

if.then9:                                         ; preds = %if.else7
  %load55 = load %Result, ptr %bad, align 8
  %extract56 = extractvalue %Result %load55, 2
  %e2 = alloca ptr, align 8
  store ptr %extract56, ptr %e2, align 8
  %load57 = load ptr, ptr %e2, align 8
  %3 = call i32 (ptr, ...) @printf(ptr @.fmt.string, ptr %load57)
  br label %if.end11

if.else10:                                        ; preds = %if.else7
  br label %if.end11

if.end11:                                         ; preds = %if.else10, %if.then9
  br label %if.end8
}
