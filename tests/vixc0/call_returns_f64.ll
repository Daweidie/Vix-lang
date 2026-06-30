; ModuleID = 'vixc0'
source_filename = "vixc0"

@.fmt.f64 = internal constant [4 x i8] c"%f\0A\00"

declare i32 @printf(ptr, ...)

declare ptr @malloc(i64)

declare i32 @strcmp(ptr, ptr)

declare i32 @strlen(ptr)

define double @half() {
entry:
  ret double 2.500000e+00
}

define i32 @main() {
entry:
  %call0 = call double @half()
  %x = alloca double, align 8
  store double %call0, ptr %x, align 8
  %load1 = load double, ptr %x, align 8
  %0 = call i32 (ptr, ...) @printf(ptr @.fmt.f64, double %load1)
  ret i32 0
}
