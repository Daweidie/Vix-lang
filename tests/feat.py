import sys
import pytest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from helpers import compile_vix, run_binary, compile_and_run


@pytest.mark.feature
class TestArithmetic:
    def test_basic_addition(self, compiler, tmp_path):
        src = 'fn main(): i32 { print(2 + 3) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "5"

    def test_basic_subtraction(self, compiler, tmp_path):
        src = 'fn main(): i32 { print(10 - 3) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "7"

    def test_basic_multiplication(self, compiler, tmp_path):
        src = 'fn main(): i32 { print(4 * 5) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "20"

    def test_basic_division(self, compiler, tmp_path):
        src = 'fn main(): i32 { print(20 / 4) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "5"

    def test_modulo(self, compiler, tmp_path):
        src = 'fn main(): i32 { print(17 % 5) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "2"

    def test_power(self, compiler, tmp_path):
        src = '''fn power(base: i32, exp: i32) -> i32 {
            let result = 1
            let i = 0
            while (i < exp) { result = result * base i = i + 1 }
            return result
        }
        fn main(): i32 { print(power(2, 10)) return 0 }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "1024"

    def test_negative_numbers(self, compiler, tmp_path):
        src = 'fn main(): i32 { print(-42) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "-42"

    def test_operator_precedence(self, compiler, tmp_path):
        src = 'fn main(): i32 { print(2 + 3 * 4) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "14"

    def test_parenthesized_expression(self, compiler, tmp_path):
        src = 'fn main(): i32 { print((2 + 3) * 4) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "20"

    def test_hex_literal(self, compiler, tmp_path):
        src = 'fn main(): i32 { print(0xFF) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "255"

    def test_compound_add_assign(self, compiler, tmp_path):
        src = 'fn main(): i32 { let mut x = 10 x += 5 print(x) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "15"

    def test_compound_sub_assign(self, compiler, tmp_path):
        src = 'fn main(): i32 { let mut x = 10 x -= 3 print(x) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "7"

    def test_compound_mul_assign(self, compiler, tmp_path):
        src = 'fn main(): i32 { let mut x = 5 x *= 3 print(x) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "15"

    def test_chained_arithmetic(self, compiler, tmp_path):
        src = 'fn main(): i32 { let mut x = 1 x = x + 1 x = x * 3 x = x - 2 print(x) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "4"


@pytest.mark.feature
class TestVariables:
    def test_let_declaration(self, compiler, tmp_path):
        src = 'fn main(): i32 { let x = 42 print(x) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "42"

    def test_typed_declaration(self, compiler, tmp_path):
        src = 'fn main(): i32 { let x: i64 = 100 print(x) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "100"

    def test_mutable_variable(self, compiler, tmp_path):
        src = 'fn main(): i32 { let mut x = 10 x = 20 print(x) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "20"

    def test_multiple_variables(self, compiler, tmp_path):
        src = 'fn main(): i32 { let a = 1 let b = 2 let c = 3 print(a + b + c) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "6"

    def test_string_variable(self, compiler, tmp_path):
        src = 'fn main(): i32 { let s = "hello" print(s) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "hello"

    def test_float_variable(self, compiler, tmp_path):
        src = 'fn main(): i32 { let x: f64 = 3.14 print(x) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "3.140000"

    def test_bool_variable(self, compiler, tmp_path):
        src = 'fn main(): i32 { let b = true if (b) { print("yes") } return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "yes"

    def test_variable_shadowing(self, compiler, tmp_path):
        src = '''fn inner() { let x = 2 print(x) }
        fn main(): i32 { let x = 1 inner() print(x) return 0 }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert lines == ["2", "1"]

    def test_multiple_mutable_reassignments(self, compiler, tmp_path):
        src = 'fn main(): i32 { let mut x = 0 x = 1 x = 2 x = 3 print(x) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "3"


@pytest.mark.feature
class TestControlFlow:
    def test_if_true(self, compiler, tmp_path):
        src = 'fn main(): i32 { if (true) { print("yes") } return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "yes"

    def test_if_false_no_else(self, compiler, tmp_path):
        src = 'fn main(): i32 { if (false) { print("no") } print("ok") return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "ok"

    def test_if_else(self, compiler, tmp_path):
        src = 'fn main(): i32 { let x = 5 if (x > 3) { print("big") } else { print("small") } return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "big"

    def test_if_elif_else(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let x = 5
            if (x > 10) { print("large") }
            elif (x > 3) { print("medium") }
            else { print("small") }
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "medium"

    def test_while_loop(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let mut i = 0
            let mut sum = 0
            while (i < 5) {
                sum = sum + i
                i = i + 1
            }
            print(sum)
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "10"

    def test_for_loop(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let mut sum = 0
            for (i in 0 .. 5) {
                sum = sum + i
            }
            print(sum)
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "10"

    def test_break(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let mut i = 0
            while (true) {
                if (i == 5) { break }
                i = i + 1
            }
            print(i)
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "5"

    def test_continue(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let mut sum = 0
            for (i in 0 .. 10) {
                if (i % 2 == 0) { continue }
                sum = sum + i
            }
            print(sum)
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "25"

    def test_nested_loops(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let mut count = 0
            for (i in 0 .. 3) {
                for (j in 0 .. 3) {
                    count = count + 1
                }
            }
            print(count)
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "9"


@pytest.mark.feature
class TestFunctions:
    def test_simple_function(self, compiler, tmp_path):
        src = '''fn add(a: i32, b: i32) -> i32 { return a + b }
        fn main(): i32 { print(add(3, 4)) return 0 }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "7"

    def test_recursive_function(self, compiler, tmp_path):
        src = '''fn fact(n: i32) -> i32 {
            if (n <= 1) { return 1 }
            return n * fact(n - 1)
        }
        fn main(): i32 { print(fact(10)) return 0 }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "3628800"

    def test_function_calling_function(self, compiler, tmp_path):
        src = '''fn double(x: i32) -> i32 { return x * 2 }
        fn quadruple(x: i32) -> i32 { return double(double(x)) }
        fn main(): i32 { print(quadruple(5)) return 0 }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "20"

    def test_void_function(self, compiler, tmp_path):
        src = '''fn greet() { print("hi") }
        fn main(): i32 { greet() return 0 }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "hi"

    def test_fibonacci(self, compiler, tmp_path):
        src = '''fn fib(n: i32) -> i32 {
            if (n <= 1) { return n }
            return fib(n - 1) + fib(n - 2)
        }
        fn main(): i32 { print(fib(10)) return 0 }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "55"

    def test_function_with_no_args(self, compiler, tmp_path):
        src = '''fn get_val() -> i32 { return 42 }
        fn main(): i32 { print(get_val()) return 0 }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "42"

    def test_lambda(self, compiler, tmp_path):
        src = '''fn apply(f: fn(i32): i32, x: i32) -> i32 { return f(x) }
        fn main(): i32 {
            let f = fn(n: i32): i32 { n * 3 }
            print(apply(f, 7))
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "21"


@pytest.mark.feature
class TestStructs:
    def test_basic_struct(self, compiler, tmp_path):
        src = '''struct Point { x: i32, y: i32 }
        fn main(): i32 {
            let p = Point { x: 10, y: 20 }
            print(p.x)
            print(p.y)
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert lines == ["10", "20"]

    def test_nested_struct(self, compiler, tmp_path):
        src = '''struct Inner { val: i32 }
        struct Outer { inner: Inner, name: string }
        fn main(): i32 {
            let o = Outer { inner: Inner { val: 42 }, name: "test" }
            print(o.inner.val)
            print(o.name)
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert lines == ["42", "test"]

    def test_struct_with_array(self, compiler, tmp_path):
        src = '''struct Bag { items: [i32] }
        fn main(): i32 {
            let b = Bag { items: [1, 2, 3] }
            print(b.items.length)
            print(b.items[1])
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert lines == ["3", "2"]

    def test_struct_field_read(self, compiler, tmp_path):
        src = '''struct Counter { value: i32, label: string }
        fn main(): i32 {
            let c = Counter { value: 42, label: "test" }
            print(c.value)
            print(c.label)
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert lines == ["42", "test"]


@pytest.mark.feature
class TestArrays:
    def test_array_literal(self, compiler, tmp_path):
        src = 'fn main(): i32 { let arr = [10, 20, 30] print(arr[0]) print(arr[1]) print(arr[2]) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert lines == ["10", "20", "30"]

    def test_array_length(self, compiler, tmp_path):
        src = 'fn main(): i32 { let arr = [1, 2, 3, 4, 5] print(arr.length) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "5"

    def test_array_index_mutation(self, compiler, tmp_path):
        src = 'fn main(): i32 { let arr = [1, 2, 3] arr[1] = 99 print(arr[1]) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "99"

    def test_array_iteration(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let arr = [10, 20, 30]
            let mut sum = 0
            for (i in 0 .. arr.length) {
                sum = sum + arr[i]
            }
            print(sum)
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "60"


@pytest.mark.feature
class TestPointers:
    def test_address_of_and_deref(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let x = 42
            let ptr = &x
            print(@ptr)
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "42"

    def test_pointer_mutation(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let mut x = 10
            let mut ptr = &x
            @ptr = 99
            print(x)
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "99"

    def test_pointer_read_write(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let mut x = 10
            let ptr = &x
            print(@ptr)
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "10"


@pytest.mark.feature
class TestMatch:
    def test_match_int_literal(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let x = 2
            match x {
                1 -> { print("one") }
                2 -> { print("two") }
                _ -> { print("other") }
            }
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "two"

    def test_match_with_wildcard(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let x = 99
            match x {
                0 -> { print("zero") }
                _ -> { print("nonzero") }
            }
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "nonzero"

    def test_match_multiple_values(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            for (i in 0 .. 5) {
                match i {
                    0 -> { print("zero") }
                    1 -> { print("one") }
                    2 -> { print("two") }
                    _ -> { print("many") }
                }
            }
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert lines == ["zero", "one", "two", "many", "many"]


@pytest.mark.feature
class TestGenerics:
    def test_generic_function(self, compiler, tmp_path):
        src = '''fn identity:[T](x: T): T { return x }
        fn main(): i32 {
            print(identity:[i32](42))
            print(identity:[str]("hello"))
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert lines == ["42", "hello"]

    def test_generic_struct(self, compiler, tmp_path):
        src = '''struct Box:[T] { value: T }
        fn main(): i32 {
            let b = Box:[i32]{ value: 42 }
            print(b.value)
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "42"

    def test_generic_inference(self, compiler, tmp_path):
        src = '''fn id:[T](x: T): T { return x }
        fn main(): i32 {
            print(id:[i32](123))
            print(id:[str]("abc"))
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert lines == ["123", "abc"]


@pytest.mark.feature
class TestADT:
    def test_result_type(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let r = Ok(42) : Result[i32, string]
            match r {
                Ok(v) -> { print(v) }
                Err(e) -> { print(e) }
            }
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "42"

    def test_option_type(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let a = Some(10)
            match a {
                Some(v) -> { print(v) }
                None -> { print("none") }
            }
            let b = None
            match b {
                Some(v) -> { print(v) }
                None -> { print("none") }
            }
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert lines == ["10", "none"]

    def test_custom_adt(self, compiler, tmp_path):
        src = '''type Color = Red | Green | Blue
        fn main(): i32 {
            let c = Red
            match c {
                Red -> { print("red") }
                Green -> { print("green") }
                Blue -> { print("blue") }
            }
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "red"


@pytest.mark.feature
class TestTuples:
    def test_tuple_access(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let t = (10, 20)
            print(t.0)
            print(t.1)
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert lines == ["10", "20"]


@pytest.mark.feature
class TestTypeConversions:
    def test_toint(self, compiler, tmp_path):
        src = 'fn main(): i32 { let x = toint(3.9) print(x) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "3"

    def test_tofloat(self, compiler, tmp_path):
        src = 'fn main(): i32 { let x = tofloat(42) print(x) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "42.000000"


@pytest.mark.feature
class TestStringOps:
    def test_string_equality(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let a = "hello"
            let b = "hello"
            let c = "world"
            if (a == b) { print("equal") }
            if (a != c) { print("different") }
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert lines == ["equal", "different"]

    def test_string_escape_sequences(self, compiler, tmp_path):
        src = r'fn main(): i32 { print("line1\nline2") return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert lines == ["line1", "line2"]


@pytest.mark.feature
class TestComments:
    def test_line_comment(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let x = 10 // this is a comment
            print(x)
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "10"

    def test_block_comment(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            /* this is a
               block comment */
            let x = 42
            print(x)
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "42"


@pytest.mark.feature
class TestBooleanLogic:
    def test_and_or(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let a = true
            let b = false
            if (a and b) { print("both") }
            if (a or b) { print("at-least-one") }
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "at-least-one"

    def test_comparison_operators(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            if (5 > 3) { print("gt") }
            if (3 < 5) { print("lt") }
            if (5 >= 5) { print("gte") }
            if (3 <= 5) { print("lte") }
            if (5 == 5) { print("eq") }
            if (5 != 3) { print("ne") }
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert lines == ["gt", "lt", "gte", "lte", "eq", "ne"]


@pytest.mark.feature
class TestBuiltinFunctions:
    def test_print_single_arg(self, compiler, tmp_path):
        src = 'fn main(): i32 { print(42) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "42"

    def test_print_string(self, compiler, tmp_path):
        src = 'fn main(): i32 { print("hello world") return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "hello world"

    def test_print_expressions(self, compiler, tmp_path):
        src = 'fn main(): i32 { print(2 + 3) return 0 }'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "5"


@pytest.mark.feature
class TestTypeStructDef:
    def test_type_struct_basic(self, compiler, tmp_path):
        src = '''type Point = struct { x: i32, y: i32 }
fn main(): i32 {
    let p = Point { x: 10, y: 20 }
    print(p.x + p.y)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "30"

    def test_type_struct_multiple(self, compiler, tmp_path):
        src = '''type Person = struct { name: string, age: i32 }
type Book = struct { title: string, year: i32 }
fn main(): i32 {
    let p = Person { name: "Alice", age: 25 }
    let b = Book { title: "Vix", year: 2026 }
    print(p.age)
    print(b.year)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = run.stdout.strip().split("\n")
        assert lines[0] == "25"
        assert lines[1] == "2026"

    def test_type_struct_field_types(self, compiler, tmp_path):
        src = '''type Rect = struct { width: f64, height: f64 }
fn main(): i32 {
    let r = Rect { width: 3.0, height: 4.0 }
    print(r.width + r.height)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert "7" in run.stdout.strip()

    def test_type_struct_in_function(self, compiler, tmp_path):
        src = '''type Pair = struct { a: i32, b: i32 }
fn sum(p: Pair): i32 = p.a + p.b
fn main(): i32 {
    let p = Pair { a: 3, b: 4 }
    print(sum(p))
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "7"


@pytest.mark.feature
class TestFnExprBody:
    def test_fn_expr_body_basic(self, compiler, tmp_path):
        src = '''fn add(a: i32, b: i32): i32 = a + b
fn main(): i32 {
    print(add(3, 4))
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "7"

    def test_fn_expr_body_single_param(self, compiler, tmp_path):
        src = '''fn double(x: i32): i32 = x * 2
fn main(): i32 {
    print(double(5))
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "10"

    def test_fn_expr_body_no_params(self, compiler, tmp_path):
        src = '''fn get_val(): i32 = 42
fn main(): i32 {
    print(get_val())
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "42"

    def test_fn_expr_body_with_struct(self, compiler, tmp_path):
        src = '''type Point = struct { x: i32, y: i32 }
fn add(p: Point): i32 = p.x + p.y
fn main(): i32 {
    let p = Point { x: 10, y: 20 }
    print(add(p))
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "30"

    def test_fn_expr_body_complex(self, compiler, tmp_path):
        src = '''fn abs(x: i32): i32 = if (x < 0) { 0 - x } else { x }
fn main(): i32 {
    print(abs(-5))
    print(abs(3))
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = run.stdout.strip().split("\n")
        assert lines[0] == "5"
        assert lines[1] == "3"


@pytest.mark.feature
class TestIfExpression:
    def test_if_expr_basic(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let a = 5
    let b = if (a > 3) { 15 } else { 25 }
    print(b)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "15"

    def test_if_expr_else_branch(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let a = 1
    let b = if (a > 3) { 15 } else { 25 }
    print(b)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "25"

    def test_if_expr_in_let(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let x = 10
    let msg = if (x > 5) { 1 } else { 0 }
    print(msg)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "1"

    def test_if_expr_as_arg(self, compiler, tmp_path):
        src = '''fn double(x: i32): i32 = x * 2
fn main(): i32 {
    let a = 5
    print(double(if (a > 3) { 10 } else { 20 }))
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "20"

    def test_if_expr_nested(self, compiler, tmp_path):
        src = '''fn main(): i32 {
    let a = 5
    let b = 3
    let c = if (a > 10) { 1 } elif (a > 3) { 2 } else { 3 }
    print(c)
    return 0
}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "2"
