; ModuleID = 'vixc0'
source_filename = "vixc0"

%Result = type { i32, i32, ptr, ptr, double }

@.strlit3 = internal constant [1 x i8] zeroinitializer
@.strlit19 = internal constant [22 x i8] c"cannot divide by zero\00"
@.fmt.i32 = internal constant [4 x i8] c"%d\0A\00"
@.strlit44 = internal constant [1 x i8] zeroinitializer
@.strlit83 = internal constant [4 x i8] c"---\00"
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

define %Result @Err(ptr %0) {
entry:
  %value = alloca ptr, align 8
  store ptr %0, ptr %value, align 8
  %load9 = load ptr, ptr %value, align 8
  %insert10 = insertvalue %Result { i32 1, i32 0, ptr undef, ptr undef, double undef }, ptr %load9, 2
  %insert11 = insertvalue %Result %insert10, ptr null, 3
  %insert12 = insertvalue %Result %insert11, double 0.000000e+00, 4
  ret %Result %insert12
}

define void @safe_divide(i32 %0, i32 %1) {
entry:
  %a = alloca i32, align 4
  store i32 %0, ptr %a, align 4
  %b = alloca i32, align 4
  store i32 %1, ptr %b, align 4
  %load13 = load i32, ptr %b, align 4
  %cmp14 = icmp eq i32 %load13, 0
  %bool15 = zext i1 %cmp14 to i32
  %cond16 = icmp ne i32 %bool15, 0
  br i1 %cond16, label %if.then0, label %if.else1

if.then0:                                         ; preds = %entry
  %result = alloca %Result, align 8
  store %Result { i32 1, i32 0, ptr @.strlit19, ptr null, double 0.000000e+00 }, ptr %result, align 8
  %load23 = load %Result, ptr %result, align 8
  %extract24 = extractvalue %Result %load23, 0
  %cmp25 = icmp eq i32 %extract24, 0
  %bool26 = zext i1 %cmp25 to i32
  %cond27 = icmp ne i32 %bool26, 0
  br i1 %cond27, label %if.then3, label %if.else4

if.else1:                                         ; preds = %entry
  %load40 = load i32, ptr %a, align 4
  %load41 = load i32, ptr %b, align 4
  %div42 = sdiv i32 %load40, %load41
  %insert43 = insertvalue %Result { i32 0, i32 undef, ptr undef, ptr undef, double undef }, i32 %div42, 1
  %insert45 = insertvalue %Result %insert43, ptr @.strlit44, 2
  %insert46 = insertvalue %Result %insert45, ptr null, 3
  %insert47 = insertvalue %Result %insert46, double 0.000000e+00, 4
  %result1 = alloca %Result, align 8
  store %Result %insert47, ptr %result1, align 8
  %load48 = load %Result, ptr %result1, align 8
  %extract49 = extractvalue %Result %load48, 0
  %cmp50 = icmp eq i32 %extract49, 0
  %bool51 = zext i1 %cmp50 to i32
  %cond52 = icmp ne i32 %bool51, 0
  br i1 %cond52, label %if.then9, label %if.else10

if.end2:                                          ; preds = %if.end11, %if.end5
  ret void

if.then3:                                         ; preds = %if.then0
  %load28 = load %Result, ptr %result, align 8
  %extract29 = extractvalue %Result %load28, 1
  %v = alloca i32, align 4
  store i32 %extract29, ptr %v, align 4
  %load30 = load i32, ptr %v, align 4
  %2 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 %load30)
  br label %if.end5

if.else4:                                         ; preds = %if.then0
  %load31 = load %Result, ptr %result, align 8
  %extract32 = extractvalue %Result %load31, 0
  %cmp33 = icmp eq i32 %extract32, 1
  %bool34 = zext i1 %cmp33 to i32
  %cond35 = icmp ne i32 %bool34, 0
  br i1 %cond35, label %if.then6, label %if.else7

if.end5:                                          ; preds = %if.end8, %if.then3
  br label %if.end2

if.then6:                                         ; preds = %if.else4
  %load36 = load %Result, ptr %result, align 8
  %extract37 = extractvalue %Result %load36, 2
  %e = alloca ptr, align 8
  store ptr %extract37, ptr %e, align 8
  %3 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 -1)
  br label %if.end8

if.else7:                                         ; preds = %if.else4
  br label %if.end8

if.end8:                                          ; preds = %if.else7, %if.then6
  br label %if.end5

if.then9:                                         ; preds = %if.else1
  %load53 = load %Result, ptr %result1, align 8
  %extract54 = extractvalue %Result %load53, 1
  %v2 = alloca i32, align 4
  store i32 %extract54, ptr %v2, align 4
  %load55 = load i32, ptr %v2, align 4
  %4 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 %load55)
  br label %if.end11

if.else10:                                        ; preds = %if.else1
  %load56 = load %Result, ptr %result1, align 8
  %extract57 = extractvalue %Result %load56, 0
  %cmp58 = icmp eq i32 %extract57, 1
  %bool59 = zext i1 %cmp58 to i32
  %cond60 = icmp ne i32 %bool59, 0
  br i1 %cond60, label %if.then12, label %if.else13

if.end11:                                         ; preds = %if.end14, %if.then9
  br label %if.end2

if.then12:                                        ; preds = %if.else10
  %load61 = load %Result, ptr %result1, align 8
  %extract62 = extractvalue %Result %load61, 2
  %e3 = alloca ptr, align 8
  store ptr %extract62, ptr %e3, align 8
  %5 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 -1)
  br label %if.end14

if.else13:                                        ; preds = %if.else10
  br label %if.end14

if.end14:                                         ; preds = %if.else13, %if.then12
  br label %if.end11
}

define i32 @main() {
entry:
  %x = alloca ptr, align 8
  store ptr inttoptr (i32 42 to ptr), ptr %x, align 8
  %load65 = load ptr, ptr %x, align 8
  br label %match.test16

match.end15:                                      ; preds = %match.test20, %match.body19, %match.body17
  %y = alloca ptr, align 8
  store ptr null, ptr %y, align 8
  %load74 = load ptr, ptr %y, align 8
  br label %match.test22

match.test16:                                     ; preds = %entry
  %cmp66 = icmp ne ptr %load65, null
  %bool67 = zext i1 %cmp66 to i32
  %cond68 = icmp ne i32 %bool67, 0
  br i1 %cond68, label %match.body17, label %match.test18

match.body17:                                     ; preds = %match.test16
  %ptrtoint69 = ptrtoint ptr %load65 to i64
  %v = alloca i32, align 4
  store i64 %ptrtoint69, ptr %v, align 4
  %load70 = load i32, ptr %v, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 %load70)
  br label %match.end15

match.test18:                                     ; preds = %match.test16
  %cmp71 = icmp eq ptr %load65, null
  %bool72 = zext i1 %cmp71 to i32
  %cond73 = icmp ne i32 %bool72, 0
  br i1 %cond73, label %match.body19, label %match.test20

match.body19:                                     ; preds = %match.test18
  %1 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 0)
  br label %match.end15

match.test20:                                     ; preds = %match.test18
  br label %match.end15

match.end21:                                      ; preds = %match.test26, %match.body25, %match.body23
  %2 = call i32 (ptr, ...) @printf(ptr @.fmt.string, ptr @.strlit83)
  call void @safe_divide(i32 10, i32 2)
  call void @safe_divide(i32 10, i32 0)
  call void @safe_divide(i32 100, i32 7)
  ret i32 0

match.test22:                                     ; preds = %match.end15
  %cmp75 = icmp ne ptr %load74, null
  %bool76 = zext i1 %cmp75 to i32
  %cond77 = icmp ne i32 %bool76, 0
  br i1 %cond77, label %match.body23, label %match.test24

match.body23:                                     ; preds = %match.test22
  %ptrtoint78 = ptrtoint ptr %load74 to i64
  %v1 = alloca i32, align 4
  store i64 %ptrtoint78, ptr %v1, align 4
  %load79 = load i32, ptr %v1, align 4
  %3 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 %load79)
  br label %match.end21

match.test24:                                     ; preds = %match.test22
  %cmp80 = icmp eq ptr %load74, null
  %bool81 = zext i1 %cmp80 to i32
  %cond82 = icmp ne i32 %bool81, 0
  br i1 %cond82, label %match.body25, label %match.test26

match.body25:                                     ; preds = %match.test24
  %4 = call i32 (ptr, ...) @printf(ptr @.fmt.i32, i32 0)
  br label %match.end21

match.test26:                                     ; preds = %match.test24
  br label %match.end21
}
