; ModuleID = 'VixModule'
source_filename = "VixModule"
target triple = "x86_64-pc-linux-gnu"

@fmt_i32_nl = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

declare i32 @printf(ptr, ...)

declare i64 @strlen(ptr)

define i32 @main() {
entry:
  %x = alloca i32, align 4
  store i32 5, ptr %x, align 4
  %calltmp = call i32 @inner()
  %0 = call i32 (ptr, ...) @printf(ptr @fmt_i32_nl, i32 %calltmp)
  ret i32 0

func_end:                                         ; No predecessors!
  ret i32 0
}

define i32 @inner() {
entry:
  ret i32 0

func_end:                                         ; No predecessors!
  ret i32 0
}
