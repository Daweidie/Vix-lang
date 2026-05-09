import os
import random
import signal
import string
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import pytest
from helpers import COMPILER


CRASH_SIGNALS = {
    signal.SIGSEGV: "SIGSEGV",
    signal.SIGABRT: "SIGABRT",
    signal.SIGFPE: "SIGFPE",
    signal.SIGILL: "SIGILL",
    signal.SIGBUS: "SIGBUS",
}


def rand_int(min_val=-100, max_val=100):
    return str(random.randint(min_val, max_val))


def rand_float():
    return f"{random.uniform(-100.0, 100.0):.6f}"


def rand_string():
    length = random.randint(0, 20)
    chars = random.choices(string.ascii_lowercase + string.digits, k=length)
    return f'"' + "".join(chars) + '"'


def rand_ident(prefix="v"):
    return f"{prefix}{random.randint(0, 9999)}"


def rand_type():
    return random.choice(["i8", "i32", "i64", "u8", "u32", "f32", "f64", "str", "string"])


def rand_int_literal():
    return random.choice([rand_int, lambda: hex(random.randint(0, 0xFFFF))])()


def rand_literal():
    return random.choice([rand_int, rand_float, rand_string, lambda: hex(random.randint(0, 0xFFFF))])()


def rand_expr(depth=0):
    if depth > 3:
        return rand_int_literal()
    kind = random.choice(["lit", "binop", "ident", "paren", "unary"])
    if kind == "lit":
        return rand_literal()
    elif kind == "binop":
        op = random.choice(["+", "-", "*", "/", "%", "==", "!=", "<", ">", "<=", ">="])
        return f"({rand_expr(depth+1)} {op} {rand_expr(depth+1)})"
    elif kind == "ident":
        return rand_ident("x")
    elif kind == "paren":
        return f"({rand_expr(depth+1)})"
    else:
        return f"(0 - {rand_expr(depth+1)})"


def gen_valid_arithmetic():
    lines = ["fn main(): i32 {"]
    for _ in range(random.randint(1, 10)):
        a = rand_int(-100, 100)
        b = rand_int(-100, 100)
        op = random.choice(["+", "-", "*", "/", "%"])
        if op in ("/", "%") and b == "0":
            b = "1"
        lines.append(f"    print({a} {op} {b})")
    lines.append("    return 0\n}")
    return "\n".join(lines)


def gen_valid_variables():
    lines = ["fn main(): i32 {"]
    for _ in range(random.randint(1, 8)):
        name = rand_ident("v")
        ty = rand_type()
        if ty in ("i8", "i32", "i64", "u8", "u32"):
            lines.append(f"    let {name}: {ty} = {rand_int()}")
        elif ty in ("f32", "f64"):
            lines.append(f"    let {name}: {ty} = {rand_float()}")
        else:
            lines.append(f'    let {name}: {ty} = {rand_string()}')
        lines.append(f"    print({name})")
    lines.append("    return 0\n}")
    return "\n".join(lines)


def gen_valid_if_else():
    x = rand_int(-50, 50)
    return f'''fn main(): i32 {{
    let x = {x}
    if (x > 0) {{ print("positive") }}
    elif (x == 0) {{ print("zero") }}
    else {{ print("negative") }}
    return 0
}}'''


def gen_valid_while_loop():
    n = random.randint(0, 20)
    return f'''fn main(): i32 {{
    let mut i = 0
    let mut sum = 0
    while (i < {n}) {{
        sum = sum + i
        i = i + 1
    }}
    print(sum)
    return 0
}}'''


def gen_valid_for_loop():
    n = random.randint(0, 20)
    return f'''fn main(): i32 {{
    let mut sum = 0
    for (i in 0 .. {n}) {{
        sum = sum + i
    }}
    print(sum)
    return 0
}}'''


def gen_valid_function():
    fname = rand_ident("fn_")
    a = rand_int(-50, 50)
    b = rand_int(-50, 50)
    op = random.choice(["+", "-", "*"])
    return f'''fn {fname}(x: i32, y: i32) -> i32 {{
    return x {op} y
}}
fn main(): i32 {{
    print({fname}({a}, {b}))
    return 0
}}'''


def gen_valid_struct():
    sname = rand_ident("S")
    fname1 = rand_ident("f")
    fname2 = rand_ident("g")
    v1 = rand_int()
    v2 = rand_int()
    return f'''struct {sname} {{ {fname1}: i32, {fname2}: i32 }}
fn main(): i32 {{
    let s = {sname} {{ {fname1}: {v1}, {fname2}: {v2} }}
    print(s.{fname1})
    print(s.{fname2})
    return 0
}}'''


def gen_valid_array():
    size = random.randint(1, 10)
    elems = [rand_int(-100, 100) for _ in range(size)]
    return f'''fn main(): i32 {{
    let arr = [{", ".join(elems)}]
    print(arr.length)
    print(arr[{random.randint(0, size - 1)}])
    return 0
}}'''


def gen_valid_pointer():
    val = rand_int(-100, 100)
    return f'''fn main(): i32 {{
    let x = {val}
    let ptr = &x
    print(@ptr)
    return 0
}}'''


def gen_valid_match():
    val = random.randint(0, 4)
    return f'''fn main(): i32 {{
    let x = {val}
    match x {{
        0 -> {{ print("zero") }}
        1 -> {{ print("one") }}
        2 -> {{ print("two") }}
        _ -> {{ print("other") }}
    }}
    return 0
}}'''


def gen_valid_tuple():
    a = rand_int(-50, 50)
    b = rand_int(-50, 50)
    return f'''fn main(): i32 {{
    let t = ({a}, {b})
    print(t.0)
    print(t.1)
    return 0
}}'''


def gen_valid_compound_assign():
    return f'''fn main(): i32 {{
    let mut x = {rand_int(1, 50)}
    x += {rand_int(1, 10)}
    print(x)
    x -= {rand_int(1, 5)}
    print(x)
    x *= 2
    print(x)
    return 0
}}'''


def gen_valid_recursion():
    return '''fn fact(n: i32) -> i32 {
    if (n <= 1) { return 1 }
    return n * fact(n - 1)
}
fn main(): i32 {
    print(fact(5))
    return 0
}'''


def gen_valid_generic_function():
    fname = rand_ident("id")
    val = rand_int()
    return f'''fn {fname}:[T](x: T): T {{ return x }}
fn main(): i32 {{
    print({fname}:[i32]({val}))
    return 0
}}'''


def gen_valid_generic_struct():
    sname = rand_ident("Box")
    val = rand_int()
    return f'''struct {sname}:[T] {{ value: T }}
fn main(): i32 {{
    let b = {sname}:[i32]{{ value: {val} }}
    print(b.value)
    return 0
}}'''


def gen_malformed_missing_brace():
    return '''fn main(): i32 {
    print("hello")
    return 0'''


def gen_malformed_invalid_token():
    return '''fn main(): i32 {
    let x = @@@
    return 0
}'''


def gen_malformed_empty():
    return ""


def gen_malformed_type_errors():
    return '''fn main(): i32 {
    let x: i32 = "hello"
    return 0
}'''


def gen_malformed_deeply_nested_expr():
    expr = "1"
    for _ in range(random.randint(20, 50)):
        expr = f"({expr} + 1)"
    return f'''fn main(): i32 {{
    print({expr})
    return 0
}}'''


def gen_malformed_unterminated_string():
    return '''fn main(): i32 {
    let s = "this string never ends
    return 0
}'''


def gen_malformed_random_tokens():
    tokens = ["fn", "let", "mut", "if", "else", "while", "for", "in", "return",
              "struct", "match", "{", "}", "(", ")", "[", "]", "=", "==", "+",
              "-", "*", "/", "print", "123", '"hello"', "true", "false"]
    lines = ["fn main(): i32 {"]
    for _ in range(random.randint(1, 15)):
        line = " ".join(random.choices(tokens, k=random.randint(1, 6)))
        lines.append(f"    {line}")
    lines.append("    return 0\n}")
    return "\n".join(lines)


VALID_GENERATORS = [
    gen_valid_arithmetic,
    gen_valid_variables,
    gen_valid_if_else,
    gen_valid_while_loop,
    gen_valid_for_loop,
    gen_valid_function,
    gen_valid_struct,
    gen_valid_array,
    gen_valid_pointer,
    gen_valid_match,
    gen_valid_tuple,
    gen_valid_compound_assign,
    gen_valid_recursion,
    gen_valid_generic_function,
    gen_valid_generic_struct,
]

MALFORMED_GENERATORS = [
    gen_malformed_missing_brace,
    gen_malformed_invalid_token,
    gen_malformed_empty,
    gen_malformed_type_errors,
    gen_malformed_deeply_nested_expr,
    gen_malformed_unterminated_string,
    gen_malformed_random_tokens,
]


def has_crash_signal(returncode):
    if returncode < 0:
        sig = -returncode
        return sig in CRASH_SIGNALS
    return False


def run_fuzz_case(program, compiler, tmp_path):
    src = tmp_path / "fuzz.vix"
    src.write_text(program)
    bin_path = tmp_path / "fuzz_bin"
    try:
        compile_res = subprocess.run(
            [str(compiler), str(src), "-o", str(bin_path)],
            capture_output=True, text=True, timeout=10,
        )
    except subprocess.TimeoutExpired:
        return "timeout", "compile timeout"

    if compile_res.returncode != 0:
        return "compile_error", compile_res.stderr[:200]

    if not bin_path.exists():
        return "compile_error", "binary not created"

    try:
        run_res = subprocess.run(
            [str(bin_path)], capture_output=True, text=True, timeout=5,
        )
    except subprocess.TimeoutExpired:
        return "timeout", "runtime timeout"

    if has_crash_signal(run_res.returncode):
        sig = -run_res.returncode
        return "crash", CRASH_SIGNALS[sig]

    return "ok", ""


@pytest.mark.fuzz
class TestFuzzValid:
    @pytest.mark.parametrize("gen_idx", range(len(VALID_GENERATORS)))
    def test_valid_fuzz(self, compiler, tmp_path, gen_idx):
        random.seed(42 + gen_idx)
        gen = VALID_GENERATORS[gen_idx]
        for i in range(8):
            program = gen()
            status, detail = run_fuzz_case(program, compiler, tmp_path)
            assert status != "crash", (
                f"Generator {gen.__name__} iteration {i} caused crash: {detail}\n"
                f"Program:\n{program[:300]}"
            )
            assert status != "timeout", (
                f"Generator {gen.__name__} iteration {i} timed out"
            )


@pytest.mark.fuzz
class TestFuzzMalformed:
    @pytest.mark.parametrize("gen_idx", range(len(MALFORMED_GENERATORS)))
    def test_malformed_fuzz(self, compiler, tmp_path, gen_idx):
        random.seed(100 + gen_idx)
        gen = MALFORMED_GENERATORS[gen_idx]
        for i in range(8):
            program = gen()
            status, detail = run_fuzz_case(program, compiler, tmp_path)
            assert status != "crash", (
                f"Generator {gen.__name__} iteration {i} caused crash: {detail}\n"
                f"Program:\n{program[:300]}"
            )


@pytest.mark.fuzz
class TestFuzzRandom:
    def test_random_programs(self, compiler, tmp_path):
        random.seed(200)
        for i in range(50):
            n_lines = random.randint(1, 30)
            lines = []
            for _ in range(n_lines):
                kind = random.choice([
                    "let", "assign", "print", "if", "while", "for",
                    "expr", "fn_call", "return", "break", "continue", "comment"
                ])
                if kind == "let":
                    mut = random.choice(["let ", "let mut ", "mut ", ""])
                    name = rand_ident()
                    ty = random.choice([""] + [": " + rand_type()])
                    val = rand_literal()
                    lines.append(f"    {mut}{name}{ty} = {val}")
                elif kind == "assign":
                    lines.append(f"    {rand_ident()} = {rand_literal()}")
                elif kind == "print":
                    lines.append(f"    print({rand_expr()})")
                elif kind == "if":
                    lines.append(f"    if ({rand_expr()}) {{")
                elif kind == "while":
                    lines.append(f"    while ({rand_expr()}) {{")
                elif kind == "for":
                    lines.append(f"    for ({rand_ident()} in {rand_int(0, 10)} .. {rand_int(10, 20)}) {{")
                elif kind == "expr":
                    lines.append(f"    {rand_expr()}")
                elif kind == "fn_call":
                    lines.append(f"    {rand_ident('fn')}({rand_int()}, {rand_int()})")
                elif kind == "return":
                    lines.append(f"    return {rand_expr()}")
                elif kind == "break":
                    lines.append("    break")
                elif kind == "continue":
                    lines.append("    continue")
                else:
                    c = "".join(random.choices(string.ascii_letters + " ", k=random.randint(1, 30)))
                    lines.append(f"    // {c}")

            for _ in range(random.randint(0, 3)):
                lines.append("    }")

            body = "\n".join(lines)
            program = f"fn main(): i32 {{\n{body}\n    return 0\n}}\n"

            status, detail = run_fuzz_case(program, compiler, tmp_path)
            assert status != "crash", (
                f"Random program {i} caused crash: {detail}\n"
                f"Program:\n{program[:300]}"
            )
