; ModuleID = 'vixc0'
source_filename = "vixc0"

%Result = type { i32, i32, ptr, ptr, double }

@.strlit3 = internal constant [1 x i8] zeroinitializer
@.strlit10 = internal constant [1 x i8] zeroinitializer
@.strlit16 = internal constant [1 x i8] zeroinitializer
@.strlit22 = internal constant [6 x i8] c"error\00"
@.strlit28 = internal constant [1 x i8] zeroinitializer
@.strlit43 = internal constant [8 x i8] c"correct\00"
@.fmt.string = internal constant [4 x i8] c"%s\0A\00"
@.fmt.i32 = internal constant [4 x i8] c"%d\0A\00"
@.strlit68 = internal constant [6 x i8] c"error\00"
@.strlit73 = internal constant [14 x i8] c"error matched\00"

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
  %x = alloca %Result, align 8
  store %Result { i32 0, i32 42, ptr @.strlit16, ptr null, double 0.000000e+00 }, ptr %x, align 8
  %y = alloca %Result, align 8
  store %Result { i32 1, i32 0, ptr @.strlit22, ptr null, double 0.000000e+00 }, ptr %y, align 8
  %z = alloca %Result, align 8
  store %Result { i32 0, i32 0, ptr @.strlit28, ptr null, double 0.000000e+00 }, ptr %z, align 8
  %load32 = load %Result, ptr %x, align 8
  %extract33 = extractvalue %Result %load32, 0
  %cmp34 = icmp eq i32 %extract33, 0
  %bool35 = zext i1 %cmp34 to i32
  %cond36 = icmp ne i32 %bool35, 0
  br i1 %cond36, label %if.then0, label %if.else1

if.then0:                                         ; preds = %entry
  %load37 = load %Result, ptr %x, align 8
  %extract38 = extractvalue %Result %load37, 1
  %v = alloca i32, align 4
  store i32 %extract38, ptr %v, align 4
  %load39 = load i32, ptr %v, align 4
  %cmp40 = icmp eq i32 %load39, 42
  %bool41 = zext i1 %cmp40 to i32
  %cond42 = icmp ne i32 %bool41, 0
  br i1 %cond42, label %if.then3, label %if.else4

if.else1:                                         ; preds = %entry
  %load44 = load %Result, ptr %x, align 8
  %extract45 = extractvalue %Result %load44, 0
  %cmp46 = icmp eq i32 %extract45, 1
  %bool47 = zext i1 %cmp46 to i32
  %cond48 = icmp ne i32 %bool47, 0
  br i1 %cond48, label %if.then6, label %if.else7

if.end2:                                          ; preds = %if.end8, %if.end5
  %load52 = load %Result, ptr %y, align 8
  %extract53 = extractvalue %Result %load52, 0
  %cmp54 = icmp eq i32 %extract53, 0
  %bool55 = zext i1 %cmp54 to i32
  %cond56 = icmp ne i32 %bool55, 0
  br i1 %cond56, label %if.then9, label %if.else10

if.then3:                                         ; preds = %if.then0
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt.string, ptr @.strlit43)
  br label %if.end5

if.else4:                                         ; preds = %if.then0
  br label %if.end5

if.end5:                                          ; preds = %if.else4, %if.then3
  br label %if.end2

if.then6:                                         ; preds = %if.else1
  %load49 = load %Result, ptr %x, align 8
  %extract50 = extractvalue %Result %load49, 2
  %e = alloca ptr, align 8
  store ptr %extract50, ptr %e, align 8
  %load51 = load ptr, ptr %e, align 8
  %1 = call i32 (ptr, ...) @printf(ptr @.fmt.string, ptr %load51)
  br label %if.end8

if.else7:                                         ; preds = %if.else1
  br label %if.end8

if.end8:                                          ; preds = %if.else7, %if.then6
  br label %if.end2

if.then9:                                         ; preds = %if.end2
  %load57 = load %Result, ptr %y, align 8
  %extract58 = extractvalue %Result %load57, 1
  %v1 = alloca i32, align 4
  store i32 %extract58, ptr %v1, align 4
  %load59 = load i32, ptr %v1, align 4
  %2 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 %load59)
  br label %if.end11

if.else10:                                        ; preds = %if.end2
  %load60 = load %Result, ptr %y, align 8
  %extract61 = extractvalue %Result %load60, 0
  %cmp62 = icmp eq i32 %extract61, 1
  %bool63 = zext i1 %cmp62 to i32
  %cond64 = icmp ne i32 %bool63, 0
  br i1 %cond64, label %if.then12, label %if.else13

if.end11:                                         ; preds = %if.end14, %if.then9
  %load74 = load %Result, ptr %z, align 8
  %extract75 = extractvalue %Result %load74, 0
  %cmp76 = icmp eq i32 %extract75, 0
  %bool77 = zext i1 %cmp76 to i32
  %cond78 = icmp ne i32 %bool77, 0
  br i1 %cond78, label %if.then18, label %if.else19

if.then12:                                        ; preds = %if.else10
  %load65 = load %Result, ptr %y, align 8
  %extract66 = extractvalue %Result %load65, 2
  %e2 = alloca ptr, align 8
  store ptr %extract66, ptr %e2, align 8
  %load67 = load ptr, ptr %e2, align 8
  %strcmp69 = call i32 @strcmp(ptr %load67, ptr @.strlit68)
  %cmp70 = icmp eq i32 %strcmp69, 0
  %bool71 = zext i1 %cmp70 to i32
  %cond72 = icmp ne i32 %bool71, 0
  br i1 %cond72, label %if.then15, label %if.else16

if.else13:                                        ; preds = %if.else10
  br label %if.end14

if.end14:                                         ; preds = %if.else13, %if.end17
  br label %if.end11

if.then15:                                        ; preds = %if.then12
  %3 = call i32 (ptr, ...) @printf(ptr @.fmt.string, ptr @.strlit73)
  br label %if.end17

if.else16:                                        ; preds = %if.then12
  br label %if.end17

if.end17:                                         ; preds = %if.else16, %if.then15
  br label %if.end14

if.then18:                                        ; preds = %if.end11
  %load79 = load %Result, ptr %z, align 8
  %extract80 = extractvalue %Result %load79, 1
  %v3 = alloca i32, align 4
  store i32 %extract80, ptr %v3, align 4
  %load81 = load i32, ptr %v3, align 4
  %4 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 %load81)
  br label %if.end20

if.else19:                                        ; preds = %if.end11
  %load82 = load %Result, ptr %z, align 8
  %extract83 = extractvalue %Result %load82, 0
  %cmp84 = icmp eq i32 %extract83, 1
  %bool85 = zext i1 %cmp84 to i32
  %cond86 = icmp ne i32 %bool85, 0
  br i1 %cond86, label %if.then21, label %if.else22

if.end20:                                         ; preds = %if.end23, %if.then18
  ret i32 0

if.then21:                                        ; preds = %if.else19
  %load87 = load %Result, ptr %z, align 8
  %extract88 = extractvalue %Result %load87, 2
  %e4 = alloca ptr, align 8
  store ptr %extract88, ptr %e4, align 8
  %load89 = load ptr, ptr %e4, align 8
  %5 = call i32 (ptr, ...) @printf(ptr @.fmt.string, ptr %load89)
  br label %if.end23

if.else22:                                        ; preds = %if.else19
  br label %if.end23

if.end23:                                         ; preds = %if.else22, %if.then21
  br label %if.end20
}
