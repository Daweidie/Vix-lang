; ModuleID = 'vixc0'
source_filename = "vixc0"

%Expr = type { i32, i32, ptr, ptr, double }

@.strlit3 = internal constant [1 x i8] zeroinitializer
@.strlit9 = internal constant [1 x i8] zeroinitializer
@.strlit28 = internal constant [1 x i8] zeroinitializer
@.strlit34 = internal constant [1 x i8] zeroinitializer
@.fmt.i32 = internal constant [4 x i8] c"%d\0A\00"

declare i32 @printf(ptr, ...)

declare ptr @malloc(i64)

declare i32 @strcmp(ptr, ptr)

declare i32 @strlen(ptr)

define %Expr @Num(i32 %0) {
entry:
  %value = alloca i32, align 4
  store i32 %0, ptr %value, align 4
  %load1 = load i32, ptr %value, align 4
  %insert2 = insertvalue %Expr { i32 0, i32 undef, ptr undef, ptr undef, double undef }, i32 %load1, 1
  %insert4 = insertvalue %Expr %insert2, ptr @.strlit3, 2
  %insert5 = insertvalue %Expr %insert4, ptr null, 3
  %insert6 = insertvalue %Expr %insert5, double 0.000000e+00, 4
  ret %Expr %insert6
}

define %Expr @Nil() {
entry:
  ret %Expr { i32 1, i32 0, ptr @.strlit9, ptr null, double 0.000000e+00 }
}

define i32 @eval(%Expr %0) {
entry:
  %e = alloca %Expr, align 8
  store %Expr %0, ptr %e, align 8
  %load13 = load %Expr, ptr %e, align 8
  %extract14 = extractvalue %Expr %load13, 0
  %cmp15 = icmp eq i32 %extract14, 0
  %bool16 = zext i1 %cmp15 to i32
  %cond17 = icmp ne i32 %bool16, 0
  br i1 %cond17, label %if.then0, label %if.else1

if.then0:                                         ; preds = %entry
  %load18 = load %Expr, ptr %e, align 8
  %extract19 = extractvalue %Expr %load18, 1
  %n = alloca i32, align 4
  store i32 %extract19, ptr %n, align 4
  %load20 = load i32, ptr %n, align 4
  ret i32 %load20

if.else1:                                         ; preds = %entry
  %load21 = load %Expr, ptr %e, align 8
  %extract22 = extractvalue %Expr %load21, 0
  %cmp23 = icmp eq i32 %extract22, 1
  %bool24 = zext i1 %cmp23 to i32
  %cond25 = icmp ne i32 %bool24, 0
  br i1 %cond25, label %if.then3, label %if.else4

if.end2:                                          ; preds = %if.end5
  ret i32 0

if.then3:                                         ; preds = %if.else1
  ret i32 0

if.else4:                                         ; preds = %if.else1
  br label %if.end5

if.end5:                                          ; preds = %if.else4
  br label %if.end2
}

define i32 @main() {
entry:
  %x = alloca %Expr, align 8
  store %Expr { i32 0, i32 100, ptr @.strlit28, ptr null, double 0.000000e+00 }, ptr %x, align 8
  %z = alloca %Expr, align 8
  store %Expr { i32 1, i32 0, ptr @.strlit34, ptr null, double 0.000000e+00 }, ptr %z, align 8
  %load38 = load %Expr, ptr %x, align 8
  %call39 = call i32 @eval(%Expr %load38)
  %vx = alloca i32, align 4
  store i32 %call39, ptr %vx, align 4
  %load40 = load %Expr, ptr %z, align 8
  %call41 = call i32 @eval(%Expr %load40)
  %vz = alloca i32, align 4
  store i32 %call41, ptr %vz, align 4
  %load42 = load i32, ptr %vx, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 %load42)
  %load43 = load i32, ptr %vz, align 4
  %1 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 %load43)
  %load44 = load %Expr, ptr %x, align 8
  %extract45 = extractvalue %Expr %load44, 0
  %cmp46 = icmp eq i32 %extract45, 0
  %bool47 = zext i1 %cmp46 to i32
  %cond48 = icmp ne i32 %bool47, 0
  br i1 %cond48, label %if.then6, label %if.else7

if.then6:                                         ; preds = %entry
  %load49 = load %Expr, ptr %x, align 8
  %extract50 = extractvalue %Expr %load49, 1
  %n = alloca i32, align 4
  store i32 %extract50, ptr %n, align 4
  %load51 = load i32, ptr %n, align 4
  %2 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 %load51)
  br label %if.end8

if.else7:                                         ; preds = %entry
  %load52 = load %Expr, ptr %x, align 8
  %extract53 = extractvalue %Expr %load52, 0
  %cmp54 = icmp eq i32 %extract53, 1
  %bool55 = zext i1 %cmp54 to i32
  %cond56 = icmp ne i32 %bool55, 0
  br i1 %cond56, label %if.then9, label %if.else10

if.end8:                                          ; preds = %if.end11, %if.then6
  ret i32 0

if.then9:                                         ; preds = %if.else7
  %3 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 -2)
  br label %if.end11

if.else10:                                        ; preds = %if.else7
  br label %if.end11

if.end11:                                         ; preds = %if.else10, %if.then9
  br label %if.end8
}
