; ModuleID = 'VixModule'
source_filename = "VixModule"
target triple = "x86_64-pc-linux-gnu"

@Ok = constant i32 0
@Err = constant i32 1
@str_lit = private unnamed_addr constant [5 x i8] c"boom\00", align 1
@str_lit.1 = private unnamed_addr constant [11 x i8] c"matched-ok\00", align 1
@fmt_s_nl = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1
@str_lit.2 = private unnamed_addr constant [12 x i8] c"matched-err\00", align 1
@fmt_s_nl.3 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1

declare i32 @printf(ptr, ...)

declare i64 @strlen(ptr)

define i32 @main() {
entry:
  %e = alloca i32, align 4
  %v = alloca i32, align 4
  %bad = alloca i32, align 4
  store i32 1, ptr %bad, align 4
  %bad1 = load i32, ptr %bad, align 4
  %eqtmp = icmp eq i32 %bad1, 0
  br i1 %eqtmp, label %then, label %else

then:                                             ; preds = %entry
  %bad2 = load i32, ptr %bad, align 4
  store i32 %bad2, ptr %v, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @fmt_s_nl, ptr @str_lit.1)
  br label %ifcont

else:                                             ; preds = %entry
  %bad3 = load i32, ptr %bad, align 4
  %eqtmp4 = icmp eq i32 %bad3, 1
  br i1 %eqtmp4, label %then5, label %else7

ifcont:                                           ; preds = %ifcont8, %then
  ret i32 0

then5:                                            ; preds = %else
  %bad6 = load i32, ptr %bad, align 4
  store i32 %bad6, ptr %e, align 4
  %1 = call i32 (ptr, ...) @printf(ptr @fmt_s_nl.3, ptr @str_lit.2)
  br label %ifcont8

else7:                                            ; preds = %else
  br label %ifcont8

ifcont8:                                          ; preds = %else7, %then5
  br label %ifcont

func_end:                                         ; No predecessors!
  ret i32 0
}
