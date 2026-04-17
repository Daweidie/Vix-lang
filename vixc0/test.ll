; ModuleID = 'VixModule'
source_filename = "VixModule"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@fmt_s_nl = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1
@fmt_i32_nl = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define i32 @main() {
entry:
  ret i32 0
}

declare i32 @printf(ptr, ...)
