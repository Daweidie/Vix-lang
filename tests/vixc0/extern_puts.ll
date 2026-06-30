; ModuleID = 'vixc0'
source_filename = "vixc0"

@.strlit0 = internal constant [10 x i8] c"extern ok\00"

declare i32 @printf(ptr, ...)

declare ptr @malloc(i64)

declare i32 @strcmp(ptr, ptr)

declare i32 @strlen(ptr)

declare i32 @puts(ptr)

define i32 @main() {
entry:
  %call1 = call i32 @puts(ptr @.strlit0)
  ret i32 0
}
