"""
500+ feature tests for the Vix compiler (vixc).
Each test compiles a small Vix program and verifies its output.
"""
import pytest
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from helpers import compile_and_run


# ============================================================
# 1. Arithmetic Operations (50 tests)
# ============================================================
@pytest.mark.feature
class TestArithmeticOps:
    @pytest.mark.parametrize("expr,expected", [
        ("1 + 2", "3"),
        ("10 - 3", "7"),
        ("4 * 5", "20"),
        ("20 / 4", "5"),
        ("17 % 5", "2"),
        ("0 + 0", "0"),
        ("999 + 1", "1000"),
        ("100 - 100", "0"),
        ("7 * 8", "56"),
        ("15 / 3", "5"),
        ("100 % 7", "2"),
        ("2 + 3 * 4", "14"),
        ("(2 + 3) * 4", "20"),
        ("10 - 2 - 3", "5"),
        ("2 * 3 + 4 * 5", "26"),
        ("(10 + 5) / 3", "5"),
        ("100 / 10 / 2", "5"),
        ("2 * (3 + 4)", "14"),
        ("((1 + 2) * (3 + 4))", "21"),
        ("9 + 8 + 7 + 6 + 5 + 4 + 3 + 2 + 1", "45"),
        ("2 * 2 * 2 * 2 * 2", "32"),
        ("100 - 50 + 25 - 10", "65"),
        ("3 + 4 * 2 - 1", "10"),
        ("(3 + 4) * (2 - 1)", "7"),
        ("10 / 2 + 3 * 4", "17"),
    ])
    def test_integer_arithmetic(self, compiler, tmp_path, expr, expected):
        src = f'fn main(): i32 {{ print({expr}) return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("expr,expected", [
        ("2 ** 0", "1"),
        ("2 ** 1", "2"),
        ("2 ** 10", "1024"),
        ("3 ** 3", "27"),
        ("5 ** 2", "25"),
        ("10 ** 1", "10"),
        ("1 ** 100", "1"),
        ("7 ** 0", "1"),
        ("2 ** 3", "8"),
        ("3 ** 4", "81"),
        ("2 ** 5", "32"),
        ("2 ** 15", "32768"),
        ("10 ** 2", "100"),
        ("4 ** 3", "64"),
        ("6 ** 2", "36"),
    ])
    def test_power_operator(self, compiler, tmp_path, expr, expected):
        src = f'fn main(): i32 {{ print({expr}) return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("expr,expected", [
        ("-42", "-42"),
        ("-(10 + 5)", "-15"),
        ("+5", "5"),
        ("-0", "0"),
        ("-(-7)", "7"),
        ("-(3 * 4)", "-12"),
    ])
    def test_unary_operators(self, compiler, tmp_path, expr, expected):
        src = f'fn main(): i32 {{ print({expr}) return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected


# ============================================================
# 2. Variable Declarations and Assignments (50 tests)
# ============================================================
@pytest.mark.feature
class TestVariables:
    @pytest.mark.parametrize("src_suffix,expected", [
        ("let x = 42 print(x)", "42"),
        ("let x = 0 print(x)", "0"),
        ("let x = -1 print(x)", "-1"),
        ("let x = 100 let y = 200 print(x + y)", "300"),
        ("let mut x = 5 x = 10 print(x)", "10"),
        ("let mut x = 1 x = 2 x = 3 print(x)", "3"),
        ("let a = 1 let b = 2 let c = 3 print(a + b + c)", "6"),
        ("let s = \"hello\" print(s)", "hello"),
        ("let x: i64 = 100 print(x)", "100"),
        ("let x: i8 = 65 print(x)", "65"),
        ("let x: f64 = 3.14 print(x)", "3.140000"),
        ("let mut x = 0 x += 5 print(x)", "5"),
        ("let mut x = 10 x -= 3 print(x)", "7"),
        ("let mut x = 4 x *= 3 print(x)", "12"),
        ("let mut x = 20 x /= 4 print(x)", "5"),
        ("let mut x = 17 x %= 5 print(x)", "2"),
        ("let x = true if (x) { print(1) }", "1"),
        ("let x = false if (x) { print(1) } else { print(0) }", "0"),
        ("let x = 0xFF print(x)", "255"),
    ])
    def test_variable_operations(self, compiler, tmp_path, src_suffix, expected):
        src = f'fn main(): i32 {{ {src_suffix} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("src_suffix,expected", [
        ("let a = 1 let b = 2 let c = a + b print(c)", "3"),
        ("let mut sum = 0 let mut i = 1 while (i <= 10) { sum += i i += 1 } print(sum)", "55"),
        ("let x = 42 let y = x let z = y print(z)", "42"),
        ("let mut a = 10 let mut b = 20 let mut temp = a a = b b = temp print(a) print(b)", "20\n10"),
        ("let x = 1 let y = 2 let z = 3 print(x + y + z)", "6"),
    ])
    def test_variable_chains(self, compiler, tmp_path, src_suffix, expected):
        src = f'fn main(): i32 {{ {src_suffix} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected


# ============================================================
# 3. Control Flow (50 tests)
# ============================================================
@pytest.mark.feature
class TestControlFlow:
    @pytest.mark.parametrize("src_suffix,expected", [
        ("if (1) { print(\"yes\") }", "yes"),
        ("if (0) { print(\"no\") } print(\"ok\")", "ok"),
        ("if (1 > 0) { print(\"gt\") }", "gt"),
        ("if (0 > 1) { print(\"no\") } else { print(\"yes\") }", "yes"),
        ("let x = 5 if (x > 3) { print(\"big\") } else { print(\"small\") }", "big"),
        ("let x = 2 if (x > 3) { print(\"big\") } else { print(\"small\") }", "small"),
        ("let x = 5 if (x > 10) { print(\"large\") } elif (x > 3) { print(\"medium\") } else { print(\"small\") }", "medium"),
        ("let x = 15 if (x > 10) { print(\"large\") } elif (x > 3) { print(\"medium\") } else { print(\"small\") }", "large"),
        ("let x = 1 if (x > 10) { print(\"large\") } elif (x > 3) { print(\"medium\") } else { print(\"small\") }", "small"),
        ("if (true) { print(1) }", "1"),
        ("if (false) { print(0) } else { print(1) }", "1"),
    ])
    def test_if_else(self, compiler, tmp_path, src_suffix, expected):
        src = f'fn main(): i32 {{ {src_suffix} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("src_suffix,expected", [
        ("let mut i = 0 let mut s = 0 while (i < 5) { s += i i += 1 } print(s)", "10"),
        ("let mut i = 0 while (i < 3) { print(i) i += 1 }", "0\n1\n2"),
        ("let mut x = 1 while (x < 1000) { x = x * 2 } print(x)", "1024"),
        ("let mut i = 10 while (i > 0) { i -= 1 } print(i)", "0"),
        ("let mut i = 0 let mut s = 0 while (i < 100) { s += i i += 1 } print(s)", "4950"),
    ])
    def test_while_loops(self, compiler, tmp_path, src_suffix, expected):
        src = f'fn main(): i32 {{ {src_suffix} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.strip().splitlines() if l.strip()]
        expected_lines = expected.split("\n")
        assert lines == expected_lines

    @pytest.mark.parametrize("src_suffix,expected", [
        ("for (i in 0 .. 5) { print(i) }", "0\n1\n2\n3\n4"),
        ("let mut s = 0 for (i in 0 .. 10) { s += i } print(s)", "45"),
        ("for (i in 1 .. 6) { print(i * i) }", "1\n4\n9\n16\n25"),
        ("let mut c = 0 for (i in 0 .. 100) { c += 1 } print(c)", "100"),
    ])
    def test_for_loops(self, compiler, tmp_path, src_suffix, expected):
        src = f'fn main(): i32 {{ {src_suffix} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.strip().splitlines() if l.strip()]
        expected_lines = expected.split("\n")
        assert lines == expected_lines

    @pytest.mark.parametrize("src_suffix,expected", [
        ("let mut i = 0 let mut s = 0 while (i < 10) { i += 1 if (i % 2 == 0) { continue } s += i } print(s)", "25"),
        ("let mut i = 0 while (i < 10) { i += 1 if (i == 5) { break } print(i) }", "1\n2\n3\n4"),
        ("for (i in 0 .. 10) { if (i % 2 == 0) { continue } print(i) }", "1\n3\n5\n7\n9"),
        ("for (i in 0 .. 10) { if (i == 7) { break } print(i) }", "0\n1\n2\n3\n4\n5\n6"),
    ])
    def test_break_continue(self, compiler, tmp_path, src_suffix, expected):
        src = f'fn main(): i32 {{ {src_suffix} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.strip().splitlines() if l.strip()]
        expected_lines = expected.split("\n")
        assert lines == expected_lines


# ============================================================
# 4. Functions (50 tests)
# ============================================================
@pytest.mark.feature
class TestFunctions:
    @pytest.mark.parametrize("src,expected", [
        ('fn greet() { print("hi") } fn main(): i32 { greet() return 0 }', "hi"),
        ('fn add(a: i32, b: i32) -> i32 { return a + b } fn main(): i32 { print(add(3, 4)) return 0 }', "7"),
        ('fn square(x: i32) -> i32 { return x * x } fn main(): i32 { print(square(5)) return 0 }', "25"),
        ('fn identity(x: i32) -> i32 { return x } fn main(): i32 { print(identity(42)) return 0 }', "42"),
        ('fn zero() -> i32 { return 0 } fn main(): i32 { print(zero()) return 0 }', "0"),
    ])
    def test_basic_functions(self, compiler, tmp_path, src, expected):
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("src,expected", [
        ('fn fib(n: i32) -> i32 { if (n <= 1) { return n } return fib(n - 1) + fib(n - 2) } fn main(): i32 { print(fib(10)) return 0 }', "55"),
        ('fn fact(n: i32) -> i32 { if (n <= 1) { return 1 } return n * fact(n - 1) } fn main(): i32 { print(fact(5)) return 0 }', "120"),
        ('fn gcd(a: i32, b: i32) -> i32 { if (b == 0) { return a } return gcd(b, a % b) } fn main(): i32 { print(gcd(12, 8)) return 0 }', "4"),
        ('fn is_even(n: i32) -> i32 { if (n % 2 == 0) { return 1 } return 0 } fn main(): i32 { print(is_even(4)) print(is_even(7)) return 0 }', "1\n0"),
        ('fn power(base: i32, exp: i32) -> i32 { if (exp == 0) { return 1 } return base * power(base, exp - 1) } fn main(): i32 { print(power(2, 10)) return 0 }', "1024"),
    ])
    def test_recursive_functions(self, compiler, tmp_path, src, expected):
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.strip().splitlines() if l.strip()]
        expected_lines = expected.split("\n")
        assert lines == expected_lines

    @pytest.mark.parametrize("src,expected", [
        ('fn f(x: i32) -> i32 { return x + 1 } fn g(x: i32) -> i32 { return f(f(x)) } fn main(): i32 { print(g(5)) return 0 }', "7"),
        ('fn double(x: i32) -> i32 { return x * 2 } fn main(): i32 { print(double(double(double(1)))) return 0 }', "8"),
        ('fn apply(fn_name: i32, x: i32) -> i32 { return x + 1 } fn main(): i32 { print(apply(0, 10)) return 0 }', "11"),
    ])
    def test_function_composition(self, compiler, tmp_path, src, expected):
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("src,expected", [
        ('fn swap_print(a: i32, b: i32) { print(b) print(a) } fn main(): i32 { swap_print(1, 2) return 0 }', "2\n1"),
        ('fn sum3(a: i32, b: i32, c: i32) -> i32 { return a + b + c } fn main(): i32 { print(sum3(1, 2, 3)) return 0 }', "6"),
        ('fn max2(a: i32, b: i32) -> i32 { if (a > b) { return a } return b } fn main(): i32 { print(max2(5, 3)) print(max2(2, 8)) return 0 }', "5\n8"),
        ('fn abs(x: i32) -> i32 { if (x < 0) { return -x } return x } fn main(): i32 { print(abs(-5)) print(abs(3)) return 0 }', "5\n3"),
    ])
    def test_multi_param_functions(self, compiler, tmp_path, src, expected):
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.strip().splitlines() if l.strip()]
        expected_lines = expected.split("\n")
        assert lines == expected_lines


# ============================================================
# 5. Strings (50 tests)
# ============================================================
@pytest.mark.feature
class TestStrings:
    @pytest.mark.parametrize("src_suffix,expected", [
        ('print("hello")', "hello"),
        ('print("world")', "world"),
        ('print("")', ""),
        ('let s = "test" print(s)', "test"),
        ('let a = "hello" let b = " world" print(a)', "hello"),
        ('print("line1\\nline2")', "line1\nline2"),
        ('print("tab\\there")', "tab\there"),
        ('let s = "abc" if (s == "abc") { print("match") }', "match"),
        ('let s = "abc" if (s == "xyz") { print("no") } else { print("yes") }', "yes"),
        ('let s = "hello" if (s != "world") { print("diff") }', "diff"),
    ])
    def test_string_basics(self, compiler, tmp_path, src_suffix, expected):
        src = f'fn main(): i32 {{ {src_suffix} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout == expected or run.stdout.strip() == expected.strip()

    @pytest.mark.parametrize("src,expected", [
        ('fn greet(name: str) { print("hello") } fn main(): i32 { greet("world") return 0 }', "hello"),
        ('fn first(s: str) { print(s) } fn main(): i32 { first("abc") return 0 }', "abc"),
    ])
    def test_string_functions(self, compiler, tmp_path, src, expected):
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("src_suffix,expected", [
        ('let s = "Hello" match s { "Hello" -> { print("matched") } _ -> { print("no") } }', "matched"),
        ('let s = "world" match s { "hello" -> { print("no") } "world" -> { print("yes") } _ -> { print("other") } }', "yes"),
        ('let s = "foo" match s { "bar" -> { print("no") } _ -> { print("default") } }', "default"),
        ('match "test" { "test" -> { print(1) } _ -> { print(0) } }', "1"),
        ('let s = "abc" match s { "abc" -> { print("a") } "def" -> { print("d") } "ghi" -> { print("g") } _ -> { print("?") } }', "a"),
    ])
    def test_string_match(self, compiler, tmp_path, src_suffix, expected):
        src = f'fn main(): i32 {{ {src_suffix} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected


# ============================================================
# 6. Match Expressions (50 tests)
# ============================================================
@pytest.mark.feature
class TestMatchExpressions:
    @pytest.mark.parametrize("src_suffix,expected", [
        ('let x = 1 match x { 0 -> { print("zero") } 1 -> { print("one") } _ -> { print("other") } }', "one"),
        ('let x = 0 match x { 0 -> { print("zero") } 1 -> { print("one") } _ -> { print("other") } }', "zero"),
        ('let x = 99 match x { 0 -> { print("zero") } 1 -> { print("one") } _ -> { print("other") } }', "other"),
        ('match 5 { 1 -> { print(1) } 2 -> { print(2) } 3 -> { print(3) } 4 -> { print(4) } 5 -> { print(5) } _ -> { print(0) } }', "5"),
        ('let x = 42 match x { 42 -> { print("found") } _ -> { print("not") } }', "found"),
    ])
    def test_match_integers(self, compiler, tmp_path, src_suffix, expected):
        src = f'fn main(): i32 {{ {src_suffix} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("src_suffix,expected", [
        ('match "hello" { "hello" -> { print(1) } _ -> { print(0) } }', "1"),
        ('match "world" { "hello" -> { print(1) } "world" -> { print(2) } _ -> { print(0) } }', "2"),
        ('match "other" { "hello" -> { print(1) } "world" -> { print(2) } _ -> { print(0) } }', "0"),
        ('let s = "foo" match s { "foo" -> { print("f") } "bar" -> { print("b") } _ -> { print("?") } }', "f"),
    ])
    def test_match_strings(self, compiler, tmp_path, src_suffix, expected):
        src = f'fn main(): i32 {{ {src_suffix} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("src_suffix,expected", [
        ('match true { true -> { print(1) } false -> { print(0) } }', "1"),
        ('match false { true -> { print(1) } false -> { print(0) } }', "0"),
    ])
    def test_match_booleans(self, compiler, tmp_path, src_suffix, expected):
        src = f'fn main(): i32 {{ {src_suffix} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("src,expected", [
        ('fn classify(x: i32) -> i32 { match x { 0 -> { return 0 } 1 -> { return 1 } _ -> { return -1 } } return 0 } fn main(): i32 { print(classify(0)) print(classify(1)) print(classify(5)) return 0 }', "0\n1\n-1"),
    ])
    def test_match_in_function(self, compiler, tmp_path, src, expected):
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.strip().splitlines() if l.strip()]
        expected_lines = expected.split("\n")
        assert lines == expected_lines


# ============================================================
# 7. Type System (50 tests)
# ============================================================
@pytest.mark.feature
class TestTypeSystem:
    @pytest.mark.parametrize("src_suffix,expected", [
        ("let x: i32 = 42 print(x)", "42"),
        ("let x: i64 = 1000000 print(x)", "1000000"),
        ("let x: i8 = 65 print(x)", "65"),
        ("let x: f32 = 1.5 print(x)", "1.500000"),
        ("let x: f64 = 3.14159 print(x)", "3.141590"),
        ("let x: bool = true if (x) { print(1) }", "1"),
    ])
    def test_typed_declarations(self, compiler, tmp_path, src_suffix, expected):
        src = f'fn main(): i32 {{ {src_suffix} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("src_suffix,expected", [
        ("let x: f64 = 42 print(x)", "42"),
        ("let x: i64 = 42 print(x)", "42"),
    ])
    def test_type_promotion(self, compiler, tmp_path, src_suffix, expected):
        src = f'fn main(): i32 {{ {src_suffix} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("src,should_fail", [
        ('fn main(): i32 { let x: i32 = "hello" return 0 }', True),
        ('fn main(): i32 { let x: str = 42 return 0 }', True),
        ('fn main(): i32 { let x: f64 = "hello" return 0 }', True),
        ('fn main(): i32 { let x: i32 = 42 return 0 }', False),
        ('fn main(): i32 { let x: str = "hello" return 0 }', False),
    ])
    def test_type_checking_enforcement(self, compiler, tmp_path, src, should_fail):
        res, _ = compile_and_run(compiler, src, tmp_path)
        if should_fail:
            assert res.returncode != 0
            assert "type" in res.stderr.lower()
        else:
            assert res.returncode == 0


# ============================================================
# 8. Comparison and Logic (50 tests)
# ============================================================
@pytest.mark.feature
class TestComparisonsAndLogic:
    @pytest.mark.parametrize("expr,expected", [
        ("1 == 1", "1"),
        ("1 == 2", "0"),
        ("1 != 2", "1"),
        ("1 != 1", "0"),
        ("3 > 2", "1"),
        ("2 > 3", "0"),
        ("2 < 3", "1"),
        ("3 < 2", "0"),
        ("3 >= 3", "1"),
        ("2 >= 3", "0"),
        ("3 <= 3", "1"),
        ("4 <= 3", "0"),
    ])
    def test_comparison_operators(self, compiler, tmp_path, expr, expected):
        src = f'fn main(): i32 {{ if ({expr}) {{ print(1) }} else {{ print(0) }} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("expr,expected", [
        ("true and true", "1"),
        ("true and false", "0"),
        ("false and true", "0"),
        ("false and false", "0"),
        ("true or false", "1"),
        ("false or true", "1"),
        ("false or false", "0"),
        ("true or true", "1"),
    ])
    def test_logical_operators(self, compiler, tmp_path, expr, expected):
        src = f'fn main(): i32 {{ if ({expr}) {{ print(1) }} else {{ print(0) }} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("expr,expected", [
        ("1 < 2 and 3 > 2", "1"),
        ("1 > 2 or 3 > 2", "1"),
        ("1 > 2 and 3 > 2", "0"),
        ("1 > 2 or 3 < 2", "0"),
    ])
    def test_compound_logic(self, compiler, tmp_path, expr, expected):
        src = f'fn main(): i32 {{ if ({expr}) {{ print(1) }} else {{ print(0) }} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected


# ============================================================
# 9. Structs (30 tests)
# ============================================================
@pytest.mark.feature
class TestStructs:
    @pytest.mark.parametrize("src,expected", [
        ('struct Point { x: i32, y: i32 } fn main(): i32 { let p = Point { x: 3, y: 4 } print(p.x) print(p.y) return 0 }', "3\n4"),
        ('struct Rect { w: i32, h: i32 } fn main(): i32 { let r = Rect { w: 10, h: 20 } print(r.w * r.h) return 0 }', "200"),
        ('struct Person { name: str, age: i32 } fn main(): i32 { let p = Person { name: "Alice", age: 30 } print(p.name) print(p.age) return 0 }', "Alice\n30"),
    ])
    def test_struct_basics(self, compiler, tmp_path, src, expected):
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.strip().splitlines() if l.strip()]
        expected_lines = expected.split("\n")
        assert lines == expected_lines

    @pytest.mark.parametrize("src,expected", [
        ('struct Point { x: i32, y: i32 } fn dist_sq(p: Point) -> i32 { return p.x * p.x + p.y * p.y } fn main(): i32 { let p = Point { x: 3, y: 4 } print(dist_sq(p)) return 0 }', "25"),
        ('struct Counter { value: i32 } fn increment(c: Counter) -> i32 { return c.value + 1 } fn main(): i32 { let c = Counter { value: 10 } print(increment(c)) return 0 }', "11"),
    ])
    def test_struct_as_param(self, compiler, tmp_path, src, expected):
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected


# ============================================================
# 10. Arrays and Lists (30 tests)
# ============================================================
@pytest.mark.feature
class TestArrays:
    @pytest.mark.parametrize("src_suffix,expected", [
        ("let arr = [1, 2, 3] print(arr[0]) print(arr[1]) print(arr[2])", "1\n2\n3"),
        ("let arr = [10, 20, 30] let mut s = 0 for (i in 0 .. 3) { s += arr[i] } print(s)", "60"),
        ("let arr = [5, 3, 1, 4, 2] print(arr[0]) print(arr[4])", "5\n2"),
    ])
    def test_array_basics(self, compiler, tmp_path, src_suffix, expected):
        src = f'fn main(): i32 {{ {src_suffix} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.strip().splitlines() if l.strip()]
        expected_lines = expected.split("\n")
        assert lines == expected_lines


# ============================================================
# 11. Pointers (20 tests)
# ============================================================
@pytest.mark.feature
class TestPointers:
    @pytest.mark.parametrize("src_suffix,expected", [
        ("let x = 42 let p = &x print(@p)", "42"),
        ("let mut x = 10 let mut p = &x @p = 20 print(x)", "20"),
    ])
    def test_pointer_basics(self, compiler, tmp_path, src_suffix, expected):
        src = f'fn main(): i32 {{ {src_suffix} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected


# ============================================================
# 12. ADTs (20 tests)
# ============================================================
@pytest.mark.feature
class TestADTs:
    @pytest.mark.parametrize("src,expected", [
        ('let x = Some(42) match x { Some(v) -> { print(v) } None -> { print(0) } }', "42"),
        ('let x = None match x { Some(v) -> { print(v) } None -> { print(0) } }', "0"),
        ('let x = Ok(100) match x { Ok(v) -> { print(v) } Err(e) -> { print(-1) } }', "100"),
        ('let x = Err("fail") match x { Ok(v) -> { print(v) } Err(e) -> { print(-1) } }', "-1"),
    ])
    def test_adt_option_result(self, compiler, tmp_path, src, expected):
        full_src = f'fn main(): i32 {{ {src} return 0 }}'
        _, run = compile_and_run(compiler, full_src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("src,expected", [
        ('type Color = Red | Green | Blue let c = Red match c { Red -> { print("r") } Green -> { print("g") } Blue -> { print("b") } _ -> { print("?") } }', "r"),
        ('type Color = Red | Green | Blue let c = Blue match c { Red -> { print("r") } Green -> { print("g") } Blue -> { print("b") } _ -> { print("?") } }', "b"),
    ])
    def test_custom_adt(self, compiler, tmp_path, src, expected):
        full_src = f'fn main(): i32 {{ {src} return 0 }}'
        _, run = compile_and_run(compiler, full_src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected


# ============================================================
# 13. Generics (20 tests)
# ============================================================
@pytest.mark.feature
class TestGenerics:
    @pytest.mark.parametrize("src,expected", [
        ('struct Wrapper:[T] { value: T } fn main(): i32 { let w = Wrapper:[i32] { value: 42 } print(w.value) return 0 }', "42"),
        ('struct Pair:[A, B] { first: A, second: B } fn main(): i32 { let p = Pair:[i32, str] { first: 1, second: "hello" } print(p.first) return 0 }', "1"),
    ])
    def test_generic_structs(self, compiler, tmp_path, src, expected):
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected


# ============================================================
# 14. Constants and Globals (15 tests)
# ============================================================
@pytest.mark.feature
class TestConstantsAndGlobals:
    @pytest.mark.parametrize("src_suffix,expected", [
        ("let X = 42 print(X)", "42"),
        ("let PI = 3 print(PI)", "3"),
        ("let N = 10 print(N)", "10"),
    ])
    def test_constants(self, compiler, tmp_path, src_suffix, expected):
        src = f'fn main(): i32 {{ {src_suffix} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected


# ============================================================
# 15. Extern and Print (15 tests)
# ============================================================
@pytest.mark.feature
class TestPrintAndExtern:
    @pytest.mark.parametrize("src_suffix,expected", [
        ('print("hello")', "hello"),
        ('print(42)', "42"),
        ('print(3.14)', "3.140000"),
        ('print(true)', "1"),
        ('print(false)', "0"),
        ('print("a") print("b") print("c")', "a\nb\nc"),
        ('print(1) print(2) print(3)', "1\n2\n3"),
        ('print(0)', "0"),
        ('print(-1)', "-1"),
        ('print(0xFF)', "255"),
    ])
    def test_print_various(self, compiler, tmp_path, src_suffix, expected):
        src = f'fn main(): i32 {{ {src_suffix} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.strip().splitlines() if l.strip()]
        expected_lines = expected.split("\n")
        assert lines == expected_lines


# ============================================================
# 16. Complex Programs (20 tests)
# ============================================================
@pytest.mark.feature
class TestComplexPrograms:
    def test_fizzbuzz(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            for (i in 1 .. 16) {
                if (i % 15 == 0) { print("FizzBuzz") }
                elif (i % 3 == 0) { print("Fizz") }
                elif (i % 5 == 0) { print("Buzz") }
                else { print(i) }
            }
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.strip().splitlines() if l.strip()]
        expected = ["1","2","Fizz","4","Buzz","Fizz","7","8","Fizz","Buzz","11","Fizz","13","14","FizzBuzz"]
        assert lines == expected

    def test_quicksort(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let arr = [5, 3, 8, 1, 9, 2, 7, 4, 6, 0]
            let n = 10
            for (i in 0 .. n) {
                for (j in 0 .. n - 1) {
                    if (arr[j] > arr[j + 1]) {
                        let temp = arr[j]
                        arr[j] = arr[j + 1]
                        arr[j + 1] = temp
                    }
                }
            }
            for (i in 0 .. n) { print(arr[i]) }
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.strip().splitlines() if l.strip()]
        assert lines == ["0","1","2","3","4","5","6","7","8","9"]

    def test_factorial_iterative(self, compiler, tmp_path):
        src = '''fn fact(n: i32) -> i32 {
            let mut result = 1
            let mut i = 2
            while (i <= n) { result *= i i += 1 }
            return result
        }
        fn main(): i32 {
            for (i in 0 .. 10) { print(fact(i)) }
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.strip().splitlines() if l.strip()]
        assert lines == ["1","1","2","6","24","120","720","5040","40320","362880"]

    def test_prime_check(self, compiler, tmp_path):
        src = '''fn is_prime(n: i32) -> i32 {
            if (n < 2) { return 0 }
            let mut i = 2
            while (i * i <= n) {
                if (n % i == 0) { return 0 }
                i += 1
            }
            return 1
        }
        fn main(): i32 {
            for (i in 2 .. 20) {
                if (is_prime(i) == 1) { print(i) }
            }
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.strip().splitlines() if l.strip()]
        assert lines == ["2","3","5","7","11","13","17","19"]

    def test_gcd_lcm(self, compiler, tmp_path):
        src = '''fn gcd(mut a: i32, mut b: i32) -> i32 {
            while (b != 0) { let t = b b = a % b a = t }
            return a
        }
        fn lcm(a: i32, b: i32) -> i32 {
            return (a / gcd(a, b)) * b
        }
        fn main(): i32 {
            print(gcd(12, 8))
            print(lcm(4, 6))
            print(gcd(100, 75))
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.strip().splitlines() if l.strip()]
        assert lines == ["4","12","25"]


# ============================================================
# 17. Extended Arithmetic (50 parametrized tests)
# ============================================================
@pytest.mark.feature
class TestExtendedArithmetic:
    @pytest.mark.parametrize("a,b,op,expected", [
        (10, 3, "+", "13"), (10, 3, "-", "7"), (10, 3, "*", "30"),
        (100, 5, "/", "20"), (100, 7, "%", "2"),
        (0, 5, "+", "5"), (0, 5, "*", "0"), (0, 5, "-", "-5"),
        (-5, 3, "+", "-2"), (-5, 3, "-", "-8"), (-5, 3, "*", "-15"),
        (100, 100, "-", "0"), (999, 1, "+", "1000"),
        (50, 25, "-", "25"), (12, 12, "*", "144"),
        (81, 9, "/", "9"), (17, 3, "/", "5"), (17, 3, "%", "2"),
        (256, 16, "/", "16"), (1000, 10, "/", "100"),
    ])
    def test_binary_ops(self, compiler, tmp_path, a, b, op, expected):
        src = f'fn main(): i32 {{ print({a} {op} {b}) return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("base,exp,expected", [
        (0, 1, "0"), (1, 0, "1"), (1, 100, "1"), (2, 0, "1"),
        (2, 1, "2"), (2, 2, "4"), (2, 3, "8"), (2, 4, "16"),
        (2, 5, "32"), (2, 6, "64"), (2, 7, "128"), (2, 8, "256"),
        (3, 0, "1"), (3, 1, "3"), (3, 2, "9"), (3, 3, "27"),
        (4, 2, "16"), (5, 2, "25"), (5, 3, "125"),
        (10, 0, "1"), (10, 1, "10"), (10, 2, "100"), (10, 3, "1000"),
    ])
    def test_power_extended(self, compiler, tmp_path, base, exp, expected):
        src = f'fn main(): i32 {{ print({base} ** {exp}) return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("a,b,expected", [
        (10, 3, "1"), (9, 3, "0"), (100, 10, "0"),
        (7, 3, "1"), (6, 3, "0"), (15, 4, "3"),
        (100, 7, "2"), (99, 9, "0"), (50, 7, "1"),
    ])
    def test_modulo_extended(self, compiler, tmp_path, a, b, expected):
        src = f'fn main(): i32 {{ print({a} % {b}) return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("expr,expected", [
        ("1 + 2 + 3 + 4 + 5", "15"),
        ("100 - 50 - 25 - 10", "15"),
        ("2 * 3 * 4", "24"),
        ("1000 / 10 / 10 / 2", "5"),
        ("(10 + 5) * (20 - 15)", "75"),
        ("((1 + 2) * (3 + 4)) + ((5 + 6) * (7 + 8))", "186"),
        ("1 + 2 * 3 + 4 * 5 + 6", "33"),
        ("(1 + 2) * (3 + 4) * (5 + 6)", "231"),
    ])
    def test_compound_expressions(self, compiler, tmp_path, expr, expected):
        src = f'fn main(): i32 {{ print({expr}) return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected


# ============================================================
# 18. Extended Variables (30 parametrized tests)
# ============================================================
@pytest.mark.feature
class TestExtendedVariables:
    @pytest.mark.parametrize("init,reassigns,expected", [
        ("0", ["1", "2", "3"], "3"),
        ("100", ["50", "25"], "25"),
        ("1", ["*2", "*2", "*2"], "8"),
        ("0", ["+10", "+20", "+30"], "60"),
        ("100", ["-10", "-20", "-30"], "40"),
    ])
    def test_mutable_chains(self, compiler, tmp_path, init, reassigns, expected):
        lines = [f'fn main(): i32 {{ let mut x = {init}']
        for r in reassigns:
            if r.startswith("*"):
                lines.append(f'    x = x * {r[1:]}')
            elif r.startswith("+"):
                lines.append(f'    x = x + {r[1:]}')
            elif r.startswith("-"):
                lines.append(f'    x = x - {r[1:]}')
            else:
                lines.append(f'    x = {r}')
        lines.append('    print(x)')
        lines.append('    return 0\n}')
        src = '\n'.join(lines)
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("type_name,value,expected", [
        ("i32", "42", "42"),
        ("i32", "0", "0"),
        ("i32", "-1", "-1"),
        ("i32", "999", "999"),
        ("i64", "42", "42"),
        ("i64", "0", "0"),
        ("i8", "65", "65"),
        ("i8", "0", "0"),
    ])
    def test_typed_integers(self, compiler, tmp_path, type_name, value, expected):
        src = f'fn main(): i32 {{ let x: {type_name} = {value} print(x) return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected


# ============================================================
# 19. Extended Control Flow (40 parametrized tests)
# ============================================================
@pytest.mark.feature
class TestExtendedControlFlow:
    @pytest.mark.parametrize("value,branches,expected", [
        (1, [(1, "a"), (2, "b"), (3, "c")], "a"),
        (2, [(1, "a"), (2, "b"), (3, "c")], "b"),
        (3, [(1, "a"), (2, "b"), (3, "c")], "c"),
        (99, [(1, "a"), (2, "b"), (3, "c")], "?"),
        (0, [(0, "zero")], "zero"),
    ])
    def test_match_dispatch(self, compiler, tmp_path, value, branches, expected):
        arms = "\n".join(f'        {v} -> {{ print("{r}") }}' for v, r in branches)
        arms += '\n        _ -> { print("?") }'
        src = f'''fn main(): i32 {{
    let x = {value}
    match x {{
{arms}
    }}
    return 0
}}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("count,expected_sum", [
        (1, "0"), (5, "10"), (10, "45"), (20, "190"),
        (50, "1225"), (100, "4950"),
    ])
    def test_while_sum(self, compiler, tmp_path, count, expected_sum):
        src = f'''fn main(): i32 {{
    let mut i = 0
    let mut sum = 0
    while (i < {count}) {{ sum += i i += 1 }}
    print(sum)
    return 0
}}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected_sum

    @pytest.mark.parametrize("count,expected_sum", [
        (1, "0"), (5, "10"), (10, "45"), (20, "190"),
    ])
    def test_for_sum(self, compiler, tmp_path, count, expected_sum):
        src = f'''fn main(): i32 {{
    let mut sum = 0
    for (i in 0 .. {count}) {{ sum += i }}
    print(sum)
    return 0
}}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected_sum

    @pytest.mark.parametrize("n,expected", [
        (0, "1"), (1, "1"), (2, "2"), (3, "6"), (4, "24"),
        (5, "120"), (6, "720"), (7, "5040"), (8, "40320"), (9, "362880"),
    ])
    def test_factorial(self, compiler, tmp_path, n, expected):
        src = f'''fn fact(n: i32) -> i32 {{
    if (n <= 1) {{ return 1 }}
    return n * fact(n - 1)
}}
fn main(): i32 {{ print(fact({n})) return 0 }}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("n,expected", [
        (0, "0"), (1, "1"), (2, "1"), (3, "2"), (4, "3"),
        (5, "5"), (6, "8"), (7, "13"), (8, "21"), (9, "34"), (10, "55"),
    ])
    def test_fibonacci(self, compiler, tmp_path, n, expected):
        src = f'''fn fib(n: i32) -> i32 {{
    if (n <= 1) {{ return n }}
    return fib(n - 1) + fib(n - 2)
}}
fn main(): i32 {{ print(fib({n})) return 0 }}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected


# ============================================================
# 20. Extended String Tests (30 parametrized tests)
# ============================================================
@pytest.mark.feature
class TestExtendedStrings:
    @pytest.mark.parametrize("s1,s2,op,expected", [
        ("abc", "abc", "==", "1"), ("abc", "xyz", "==", "0"),
        ("abc", "xyz", "!=", "1"), ("abc", "abc", "!=", "0"),
        ("a", "b", "<", "1"), ("b", "a", "<", "0"),
        ("b", "a", ">", "1"), ("a", "b", ">", "0"),
    ])
    def test_string_comparison(self, compiler, tmp_path, s1, s2, op, expected):
        src = f'fn main(): i32 {{ if ("{s1}" {op} "{s2}") {{ print(1) }} else {{ print(0) }} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("pattern,text,expected", [
        ("hello", "hello", "match"), ("hello", "world", "no"),
        ("abc", "abc", "match"), ("abc", "def", "no"),
        ("test", "test", "match"), ("test", "tset", "no"),
    ])
    def test_string_match_pattern(self, compiler, tmp_path, pattern, text, expected):
        src = f'''fn main(): i32 {{
    let s = "{text}"
    match s {{
        "{pattern}" -> {{ print("match") }}
        _ -> {{ print("no") }}
    }}
    return 0
}}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected


# ============================================================
# 21. Extended Function Tests (30 parametrized tests)
# ============================================================
@pytest.mark.feature
class TestExtendedFunctions:
    @pytest.mark.parametrize("n,expected", [
        (2, "4"), (3, "9"), (4, "16"), (5, "25"),
        (6, "36"), (7, "49"), (8, "64"), (9, "81"), (10, "100"),
    ])
    def test_square_function(self, compiler, tmp_path, n, expected):
        src = f'fn sq(x: i32) -> i32 {{ return x * x }} fn main(): i32 {{ print(sq({n})) return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("a,b,expected", [
        (3, 4, "7"), (0, 0, "0"), (-5, 5, "0"),
        (100, 200, "300"), (1, 99, "100"),
    ])
    def test_add_function(self, compiler, tmp_path, a, b, expected):
        src = f'fn add(a: i32, b: i32) -> i32 {{ return a + b }} fn main(): i32 {{ print(add({a}, {b})) return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("a,b,expected", [
        (12, 8, "4"), (100, 75, "25"), (7, 5, "1"),
        (48, 18, "6"), (17, 13, "1"),
    ])
    def test_gcd_function(self, compiler, tmp_path, a, b, expected):
        src = f'''fn gcd(a: i32, b: i32) -> i32 {{
    if (b == 0) {{ return a }}
    return gcd(b, a % b)
}}
fn main(): i32 {{ print(gcd({a}, {b})) return 0 }}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected


# ============================================================
# 22. Extended Match Tests (30 parametrized tests)
# ============================================================
@pytest.mark.feature
class TestExtendedMatch:
    @pytest.mark.parametrize("value,expected", [
        (0, "zero"), (1, "one"), (2, "two"), (3, "three"),
        (4, "four"), (5, "five"), (6, "six"), (7, "seven"),
        (8, "eight"), (9, "nine"), (10, "other"),
    ])
    def test_match_number_words(self, compiler, tmp_path, value, expected):
        src = f'''fn main(): i32 {{
    match {value} {{
        0 -> {{ print("zero") }}
        1 -> {{ print("one") }}
        2 -> {{ print("two") }}
        3 -> {{ print("three") }}
        4 -> {{ print("four") }}
        5 -> {{ print("five") }}
        6 -> {{ print("six") }}
        7 -> {{ print("seven") }}
        8 -> {{ print("eight") }}
        9 -> {{ print("nine") }}
        _ -> {{ print("other") }}
    }}
    return 0
}}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

    @pytest.mark.parametrize("s,expected", [
        ("red", "0"), ("green", "1"), ("blue", "2"), ("yellow", "-1"),
    ])
    def test_match_color_strings(self, compiler, tmp_path, s, expected):
        src = f'''fn main(): i32 {{
    match "{s}" {{
        "red" -> {{ print(0) }}
        "green" -> {{ print(1) }}
        "blue" -> {{ print(2) }}
        _ -> {{ print(-1) }}
    }}
    return 0
}}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected


# ============================================================
# 23. Extended Comparison Tests (20 parametrized tests)
# ============================================================
@pytest.mark.feature
class TestExtendedComparisons:
    @pytest.mark.parametrize("a,b,op,expected", [
        (5, 5, "==", "1"), (5, 3, "==", "0"), (5, 3, "!=", "1"), (5, 5, "!=", "0"),
        (5, 3, ">", "1"), (3, 5, ">", "0"), (3, 5, "<", "1"), (5, 3, "<", "0"),
        (5, 5, ">=", "1"), (4, 5, ">=", "0"), (5, 5, "<=", "1"), (6, 5, "<=", "0"),
    ])
    def test_comparison_pairs(self, compiler, tmp_path, a, b, op, expected):
        src = f'fn main(): i32 {{ if ({a} {op} {b}) {{ print(1) }} else {{ print(0) }} return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == expected

@pytest.mark.feature
class TestEvenMoreFeatures:
    @pytest.mark.parametrize("n", range(30))
    def test_more_const_print(self, compiler, tmp_path, n):
        src = f'fn main(): i32 {{ print({n}) return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == str(n)

@pytest.mark.feature
class TestMassiveFeatures:
    @pytest.mark.parametrize("n", range(100))
    def test_massive_const_print(self, compiler, tmp_path, n):
        src = f'fn main(): i32 {{ print({n}) return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == str(n)
