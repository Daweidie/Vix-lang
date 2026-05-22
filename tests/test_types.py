import pytest
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from helpers import compile_and_run, compile_source

@pytest.mark.feature
class TestTypeAnnotations:
    @pytest.mark.parametrize("type_name,default_val", [
        ("i32", "0"),
        ("i64", "0"),
        ("f32", "0"),
        ("f64", "0"),
        ("string", '""'),
    ])
    def test_variable_type_annotation(self, compiler, tmp_path, type_name, default_val):
        src = f'fn main(): i32 {{ let x: {type_name} = {default_val} print(0) return 0 }}'
        compile_res, run_res = compile_and_run(compiler, src, tmp_path)
        assert compile_res.returncode == 0, f"Failed to compile with type {type_name}"

    def test_string_type_annotation(self, compiler, tmp_path):
        src = 'fn main(): i32 { let s: string = "hello" print(s) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "hello"

    def test_i32_type_annotation(self, compiler, tmp_path):
        src = 'fn main(): i32 { let x: i32 = 42 print(x) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "42"

    def test_i64_type_annotation(self, compiler, tmp_path):
        src = 'fn main(): i32 { let x: i64 = 100 print(x) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "100"

    def test_f64_type_annotation(self, compiler, tmp_path):
        src = 'fn main(): i32 { let x: f64 = 3.14 print(x) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert "3.14" in run.stdout.strip()

    def test_bool_type(self, compiler, tmp_path):
        src = 'fn main(): i32 { let t = true let f = false print(t) print(f) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert "1" in run.stdout
        assert "0" in run.stdout


# ============================================================
# 2. Type Inference
# ============================================================
@pytest.mark.feature
class TestTypeInference:
    def test_infer_i32_from_int_literal(self, compiler, tmp_path):
        src = 'fn main(): i32 { let x = 42 print(x) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "42"

    def test_infer_string_from_literal(self, compiler, tmp_path):
        src = 'fn main(): i32 { let s = "hello" print(s) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "hello"

    def test_infer_f64_from_float_literal(self, compiler, tmp_path):
        src = 'fn main(): i32 { let x = 3.14 print(x) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert "3.14" in run.stdout.strip()

    def test_infer_from_expression(self, compiler, tmp_path):
        src = 'fn main(): i32 { let x = 10 + 20 print(x) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "30"

    def test_infer_from_function_call(self, compiler, tmp_path):
        src = '''fn get_val() -> i32 { return 99 }
fn main(): i32 { let x = get_val() print(x) return 0 }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "99"


# ============================================================
# 3. Function Type Signatures
# ============================================================
@pytest.mark.feature
class TestFunctionTypes:
    def test_void_return(self, compiler, tmp_path):
        src = '''fn greet() { print("hi") }
fn main(): i32 { greet() return 0 }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "hi"

    def test_i32_return(self, compiler, tmp_path):
        src = '''fn add(a: i32, b: i32) -> i32 { return a + b }
fn main(): i32 { print(add(3, 4)) return 0 }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "7"

    def test_string_param_and_return(self, compiler, tmp_path):
        src = '''fn greet(name: string) { print("hello ", name) }
fn main(): i32 { greet("world") return 0 }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert "hello" in run.stdout

    def test_pointer_param(self, compiler, tmp_path):
        src = '''fn deref(p: ptr) -> i32 { return @p }
fn main(): i32 { let x = 42 print(deref(&x)) return 0 }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "42"

    def test_f64_params(self, compiler, tmp_path):
        src = '''fn add(a: f64, b: f64) -> f64 { return a + b }
fn main(): i32 { print(add(1.5, 2.5)) return 0 }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert "4" in run.stdout.strip()

    def test_function_no_return_type(self, compiler, tmp_path):
        src = '''fn do_nothing(a: i32) { print(a) }
fn main(): i32 { do_nothing(42) print("ok") return 0 }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert "ok" in run.stdout

    def test_multiple_params(self, compiler, tmp_path):
        src = '''fn sum5(a: i32, b: i32, c: i32, d: i32, e: i32) -> i32 { return a + b + c + d + e }
fn main(): i32 { print(sum5(1, 2, 3, 4, 5)) return 0 }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "15"


# ============================================================
# 4. Numeric Type Promotion
# ============================================================
@pytest.mark.feature
class TestTypePromotion:
    def test_i32_to_i64_promotion(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let a: i32 = 10
    let b: i64 = 20
    print(a + b)
    return 0
}'''
        compile_res, _ = compile_and_run(compiler, src, tmp_path)
        assert compile_res.returncode == 0

    def test_i32_literal_in_i64_context(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let x: i64 = 42
    print(x)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "42"


# ============================================================
# 5. Pointer Types
# ============================================================
@pytest.mark.feature
class TestPointerTypes:
    def test_address_of_and_deref(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let x = 42
    let p = &x
    print(@p)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "42"

    def test_pointer_mutation(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let mut x = 10
    let mut p = &x
    @p = 20
    print(x)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "20"

    def test_swap_via_pointers(self, compiler, tmp_path):
        src = '''fn swap(mut a: &i32, mut b: &i32) {
    let temp = @a
    @a = @b
    @b = temp
}
fn main(): i32 {
    let a = 10
    let b = 20
    swap(&a, &b)
    print(a)
    print(b)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert "20" in run.stdout
        assert "10" in run.stdout

    def test_ptr_type_annotation(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let x = 99
    let p: ptr = &x
    print(@p)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "99"


# ============================================================
# 6. Array Types
# ============================================================
@pytest.mark.feature
class TestArrayTypes:
    def test_fixed_array_type(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let arr: [i32 * 3] = [10, 20, 30]
    print(arr[0])
    print(arr[1])
    print(arr[2])
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert "10" in run.stdout
        assert "20" in run.stdout
        assert "30" in run.stdout

    def test_dynamic_array_type(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let arr = [1, 2, 3, 4, 5]
    print(arr.length)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "5"

    def test_array_mutation(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let mut arr = [1, 2, 3]
    arr[1] = 99
    print(arr[1])
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "99"

    def test_string_array(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let arr = ["hello", "world"]
    print(arr[0])
    print(arr[1])
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert "hello" in run.stdout
        assert "world" in run.stdout


# ============================================================
# 7. Struct Types
# ============================================================
@pytest.mark.feature
class TestStructTypes:
    def test_struct_definition_and_access(self, compiler, tmp_path):
        src = '''type Point = struct { x: i32, y: i32 }
fn main(): i32 {
    let p = Point { x: 10, y: 20 }
    print(p.x)
    print(p.y)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert "10" in run.stdout
        assert "20" in run.stdout

    def test_struct_with_string_fields(self, compiler, tmp_path):
        src = '''type Person = struct { name: string, age: i32 }
fn main(): i32 {
    let p = Person { name: "Alice", age: 30 }
    print(p.name)
    print(p.age)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert "Alice" in run.stdout
        assert "30" in run.stdout

    def test_struct_mutation(self, compiler, tmp_path):
        src = '''type Counter = struct { value: i32 }
fn main(): i32 {
    let mut c = Counter { value: 0 }
    c = Counter { value: 42 }
    print(c.value)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "42"

    def test_nested_struct(self, compiler, tmp_path):
        src = '''type Address = struct { city: string }
type Person = struct { name: string, addr: Address }
fn main(): i32 {
    let a = Address { city: "Beijing" }
    let p = Person { name: "Bob", addr: a }
    print(p.name)
    print(p.addr.city)
    return 0
}'''
        compile_res, _ = compile_and_run(compiler, src, tmp_path)
        assert compile_res.returncode == 0


# ============================================================
# 8. ADT Types (Algebraic Data Types)
# ============================================================
@pytest.mark.feature
class TestADTTypes:
    def test_option_type(self, compiler, tmp_path):
        src = '''fn find(x: i32) -> ?i32 {
    if (x > 0) { return Some(x) }
    return None
}
fn main(): i32 {
    let r = find(5)
    print(0)
    return 0
}'''
        compile_res, _ = compile_and_run(compiler, src, tmp_path)
        assert compile_res.returncode == 0

    def test_result_type(self, compiler, tmp_path):
        src = '''fn safe_div(a: i32, b: i32) -> Result[i32, string] {
    if (b == 0) { return Err("division by zero") }
    return Ok(a / b)
}
fn main(): i32 {
    let r = safe_div(10, 2)
    print(0)
    return 0
}'''
        compile_res, _ = compile_and_run(compiler, src, tmp_path)
        assert compile_res.returncode == 0

    def test_custom_adt(self, compiler, tmp_path):
        src = '''type Color = Red | Green | Blue
fn main(): i32 {
    let c = Red
    print(0)
    return 0
}'''
        compile_res, _ = compile_and_run(compiler, src, tmp_path)
        assert compile_res.returncode == 0


# ============================================================
# 9. String Type
# ============================================================
@pytest.mark.feature
class TestStringType:
    def test_string_literal(self, compiler, tmp_path):
        src = 'fn main(): i32 { print("hello world") return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "hello world"

    def test_string_variable(self, compiler, tmp_path):
        src = 'fn main(): i32 { let s = "test" print(s) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "test"

    def test_string_annotation(self, compiler, tmp_path):
        src = 'fn main(): i32 { let s: string = "annotated" print(s) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "annotated"

    def test_string_length(self, compiler, tmp_path):
        src = 'fn main(): i32 { let s = "hello" print(s.length) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "5"

    def test_string_index(self, compiler, tmp_path):
        src = 'fn main(): i32 { let s = "abc" print(s[0]) return 0 }'
        compile_res, _ = compile_and_run(compiler, src, tmp_path)
        assert compile_res.returncode == 0

    def test_string_in_print(self, compiler, tmp_path):
        src = 'fn main(): i32 { let name = "Vix" print("Hello, ", name, "!") return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert "Hello" in run.stdout
        assert "Vix" in run.stdout


# ============================================================
# 10. Extern Function Types
# ============================================================
@pytest.mark.feature
class TestExternTypes:
    def test_extern_function_call(self, compiler, tmp_path):
        src = '''extern "C" { fn printf(format: ptr, ...) -> i32 }
fn main() -> i32 {
    printf("extern works\\n")
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert "extern works" in run.stdout

    def test_extern_with_varargs(self, compiler, tmp_path):
        src = '''extern "C" { fn printf(format: ptr, ...) -> i32 }
fn main() -> i32 {
    printf("value: %d\\n", 42)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert "42" in run.stdout


# ============================================================
# 11. Type Error Detection (should fail compilation)
# ============================================================
@pytest.mark.feature
class TestTypeErrors:
    def test_type_mismatch_let(self, compiler, tmp_path):
        src = 'fn main(): i32 { let x: i32 = "hello" return 0 }'
        compile_res, _ = compile_and_run(compiler, src, tmp_path)
        assert compile_res.returncode != 0

    def test_immutable_assignment(self, compiler, tmp_path):
        src = 'fn main(): i32 { let x = 10 x = 20 return 0 }'
        compile_res, _ = compile_and_run(compiler, src, tmp_path)
        assert compile_res.returncode != 0

    def test_undefined_variable(self, compiler, tmp_path):
        src = 'fn main(): i32 { print(x) return 0 }'
        compile_res, _ = compile_and_run(compiler, src, tmp_path)
        assert compile_res.returncode != 0

    def test_undefined_function(self, compiler, tmp_path):
        src = 'fn main(): i32 { foo() return 0 }'
        compile_res, _ = compile_and_run(compiler, src, tmp_path)
        assert compile_res.returncode != 0

    def test_no_main_function(self, compiler, tmp_path):
        src = 'fn foo() { print("hi") }'
        compile_res, _ = compile_and_run(compiler, src, tmp_path)
        assert compile_res.returncode != 0


# ============================================================
# 12. Compound Assignment Types
# ============================================================
@pytest.mark.feature
class TestCompoundAssignment:
    def test_plus_assign(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let mut x = 10
    x += 5
    print(x)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "15"

    def test_minus_assign(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let mut x = 10
    x -= 3
    print(x)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "7"

    def test_multiply_assign(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let mut x = 5
    x *= 3
    print(x)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "15"

    def test_divide_assign(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let mut x = 20
    x /= 4
    print(x)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "5"

    def test_modulo_assign(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let mut x = 17
    x %= 5
    print(x)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "2"


# ============================================================
# 13. Void Type
# ============================================================
@pytest.mark.feature
class TestVoidType:
    def test_void_function_as_statement(self, compiler, tmp_path):
        src = '''fn log(msg: string) { print(msg) }
fn main(): i32 {
    log("test")
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "test"

    def test_void_function_no_return(self, compiler, tmp_path):
        src = '''fn do_stuff() {
    let x = 1
    let y = 2
    print(x + y)
}
fn main(): i32 { do_stuff() return 0 }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "3"


# ============================================================
# 14. Implicit Return Type (void when no return type specified)
# ============================================================
@pytest.mark.feature
class TestImplicitReturnType:
    def test_function_without_return_type(self, compiler, tmp_path):
        src = '''fn greet(name: string) {
    print("Hello, ", name)
}
fn main(): i32 {
    greet("World")
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert "Hello" in run.stdout

    def test_function_body_ends_with_void(self, compiler, tmp_path):
        src = '''fn do_loop() {
    for (i in 0 .. 3) {
        print(i)
    }
}
fn main(): i32 {
    do_loop()
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert "0" in run.stdout


# ============================================================
# 15. Type Compatibility in Expressions
# ============================================================
@pytest.mark.feature
class TestTypeCompatibility:
    def test_i32_arithmetic(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let a: i32 = 10
    let b: i32 = 3
    print(a + b)
    print(a - b)
    print(a * b)
    print(a / b)
    print(a % b)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert "13" in run.stdout
        assert "7" in run.stdout
        assert "30" in run.stdout
        assert "3" in run.stdout
        assert "1" in run.stdout

    def test_comparison_returns_bool(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let a = 5
    let b = 10
    if (a < b) { print("less") }
    if (b > a) { print("greater") }
    if (a == a) { print("equal") }
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert "less" in run.stdout
        assert "greater" in run.stdout
        assert "equal" in run.stdout

    def test_string_comparison(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let a = "hello"
    let b = "hello"
    if (a == b) { print("match") }
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "match"

    def test_logical_operators(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let a = true
    let b = false
    if (a and a) { print("and_ok") }
    if (a or b) { print("or_ok") }
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert "and_ok" in run.stdout
        assert "or_ok" in run.stdout
