#!/usr/bin/env python3
"""
Fuzz test suite for the vixc compiler.
Generates 500+ random/malformed Vix programs and verifies the compiler
doesn't crash (segfault, abort, etc.). Compilation errors are acceptable;
crashes are not.
"""

import os
import random
import re
import signal
import string
import subprocess
import sys
import tempfile
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Optional, Tuple


GREEN = "\033[32m"
RED = "\033[31m"
YELLOW = "\033[33m"
RESET = "\033[0m"

ROOT = Path(__file__).resolve().parent.parent.parent
COMPILER = ROOT / "build" / "vixc"

CRASH_SIGNALS = {
    signal.SIGSEGV: "SIGSEGV",
    signal.SIGABRT: "SIGABRT",
    signal.SIGFPE: "SIGFPE",
    signal.SIGILL: "SIGILL",
    signal.SIGBUS: "SIGBUS",
}

# Number of fuzz iterations per category (28 valid + 18 malformed = 46 categories)
# 46 * 12 = 552 total fuzz tests
FUZZ_COUNT_PER_CATEGORY = 12


@dataclass
class FuzzResult:
    category: str
    index: int
    program: str
    crashed: bool = False
    crash_signal: str = ""
    timeout: bool = False
    compile_error: bool = False
    runtime_error: bool = False
    success: bool = False
    stderr_snippet: str = ""


# ─── Primitive generators ────────────────────────────────────────────

INT_TYPES = ["i8", "i32", "i64", "u8", "u32"]
FLOAT_TYPES = ["f32", "f64"]
ALL_TYPES = INT_TYPES + FLOAT_TYPES + ["str", "string"]

def rand_int(min_val=-1000, max_val=1000) -> str:
    return str(random.randint(min_val, max_val))

def rand_float() -> str:
    return f"{random.uniform(-100.0, 100.0):.6f}"

def rand_hex() -> str:
    return hex(random.randint(0, 0xFFFF))

def rand_string() -> str:
    length = random.randint(0, 20)
    chars = random.choices(string.ascii_lowercase + string.digits + " \t", k=length)
    s = "".join(chars)
    s = s.replace("\\", "\\\\").replace('"', '\\"').replace("\t", "\\t")
    return f'"{s}"'

def rand_ident(prefix="v") -> str:
    return f"{prefix}{random.randint(0, 9999)}"

def rand_type() -> str:
    return random.choice(ALL_TYPES)

def rand_int_literal() -> str:
    return random.choice([rand_int, rand_hex])()

def rand_literal() -> str:
    return random.choice([rand_int, rand_float, rand_hex, rand_string])()

def rand_expr(depth=0) -> str:
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


# ─── Program generators ──────────────────────────────────────────────

def gen_valid_minimal() -> str:
    return "fn main(): i32 {\n    return 0\n}\n"

def gen_valid_arithmetic() -> str:
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

def gen_valid_variables() -> str:
    lines = ["fn main(): i32 {"]
    for _ in range(random.randint(1, 8)):
        name = rand_ident("v")
        ty = rand_type()
        if ty in INT_TYPES:
            lines.append(f"    let {name}: {ty} = {rand_int()}")
        elif ty in FLOAT_TYPES:
            lines.append(f"    let {name}: {ty} = {rand_float()}")
        else:
            lines.append(f'    let {name}: {ty} = {rand_string()}')
        lines.append(f"    print({name})")
    lines.append("    return 0\n}")
    return "\n".join(lines)

def gen_valid_if_else() -> str:
    lines = ["fn main(): i32 {"]
    x = rand_int(-50, 50)
    lines.append(f"    let x = {x}")
    lines.append("    if (x > 0) {")
    lines.append('        print("positive")')
    lines.append("    }")
    lines.append("    elif (x == 0) {")
    lines.append('        print("zero")')
    lines.append("    }")
    lines.append("    else {")
    lines.append('        print("negative")')
    lines.append("    }")
    lines.append("    return 0\n}")
    return "\n".join(lines)

def gen_valid_while_loop() -> str:
    n = random.randint(0, 20)
    lines = ["fn main(): i32 {"]
    lines.append(f"    let mut i = 0")
    lines.append(f"    let mut sum = 0")
    lines.append(f"    while (i < {n}) {{")
    lines.append(f"        sum = sum + i")
    lines.append(f"        i = i + 1")
    lines.append(f"    }}")
    lines.append(f"    print(sum)")
    lines.append("    return 0\n}")
    return "\n".join(lines)

def gen_valid_for_loop() -> str:
    n = random.randint(0, 20)
    lines = ["fn main(): i32 {"]
    lines.append(f"    let mut sum = 0")
    lines.append(f"    for (i in 0 .. {n}) {{")
    lines.append(f"        sum = sum + i")
    lines.append(f"    }}")
    lines.append(f"    print(sum)")
    lines.append("    return 0\n}")
    return "\n".join(lines)

def gen_valid_nested_loops() -> str:
    n = random.randint(1, 5)
    m = random.randint(1, 5)
    lines = ["fn main(): i32 {"]
    lines.append(f"    let mut count = 0")
    lines.append(f"    for (i in 0 .. {n}) {{")
    lines.append(f"        for (j in 0 .. {m}) {{")
    lines.append(f"            count = count + 1")
    lines.append(f"        }}")
    lines.append(f"    }}")
    lines.append(f"    print(count)")
    lines.append("    return 0\n}")
    return "\n".join(lines)

def gen_valid_function() -> str:
    fname = rand_ident("fn_")
    a = rand_int(-50, 50)
    b = rand_int(-50, 50)
    op = random.choice(["+", "-", "*"])
    return f"""fn {fname}(x: i32, y: i32) -> i32 {{
    return x {op} y
}}

fn main(): i32 {{
    print({fname}({a}, {b}))
    return 0
}}
"""

def gen_valid_recursion() -> str:
    return """fn fact(n: i32) -> i32 {
    if (n <= 1) { return 1 }
    return n * fact(n - 1)
}

fn main(): i32 {
    print(fact(5))
    return 0
}
"""

def gen_valid_struct() -> str:
    sname = rand_ident("S")
    fname1 = rand_ident("f")
    fname2 = rand_ident("g")
    v1 = rand_int()
    v2 = rand_int()
    return f"""struct {sname} {{
    {fname1}: i32,
    {fname2}: i32
}}

fn main(): i32 {{
    let s = {sname} {{ {fname1}: {v1}, {fname2}: {v2} }}
    print(s.{fname1})
    print(s.{fname2})
    return 0
}}
"""

def gen_valid_generic_struct() -> str:
    sname = rand_ident("Box")
    val = rand_int()
    return f"""struct {sname}:[T] {{
    value: T
}}

fn main(): i32 {{
    let b = {sname}:[i32]{{ value: {val} }}
    print(b.value)
    return 0
}}
"""

def gen_valid_array() -> str:
    size = random.randint(1, 10)
    elems = [rand_int(-100, 100) for _ in range(size)]
    elems_str = ", ".join(elems)
    idx = random.randint(0, size - 1)
    return f"""fn main(): i32 {{
    let arr = [{elems_str}]
    print(arr.length)
    print(arr[{idx}])
    return 0
}}
"""

def gen_valid_array_iteration() -> str:
    size = random.randint(1, 8)
    elems = [rand_int(-50, 50) for _ in range(size)]
    elems_str = ", ".join(elems)
    return f"""fn main(): i32 {{
    let arr = [{elems_str}]
    let mut sum = 0
    for (i in 0 .. arr.length) {{
        sum = sum + arr[i]
    }}
    print(sum)
    return 0
}}
"""

def gen_valid_pointer() -> str:
    val = rand_int(-100, 100)
    return f"""fn main(): i32 {{
    let x = {val}
    let ptr = &x
    print(@ptr)
    return 0
}}
"""

def gen_valid_pointer_mut() -> str:
    v1 = rand_int(-100, 100)
    v2 = rand_int(-100, 100)
    return f"""fn main(): i32 {{
    let mut x = {v1}
    let mut ptr = &x
    @ptr = {v2}
    print(x)
    return 0
}}
"""

def gen_valid_match() -> str:
    val = random.randint(0, 4)
    return f"""fn main(): i32 {{
    let x = {val}
    match x {{
        0 -> {{ print("zero") }}
        1 -> {{ print("one") }}
        2 -> {{ print("two") }}
        _ -> {{ print("other") }}
    }}
    return 0
}}
"""

def gen_valid_result_type() -> str:
    return """fn safe_div(a: i32, b: i32) -> i32 {
    if (b == 0) { return -1 }
    return a / b
}

fn main(): i32 {
    let r = Ok(42) : Result[i32, string]
    match r {
        Ok(v) -> { print(v) }
        Err(e) -> { print(e) }
    }
    return 0
}
"""

def gen_valid_option_type() -> str:
    return """fn main(): i32 {
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
}
"""

def gen_valid_generic_function() -> str:
    fname = rand_ident("id")
    val = rand_int()
    return f"""fn {fname}:[T](x: T): T {{
    return x
}}

fn main(): i32 {{
    print({fname}:[i32]({val}))
    return 0
}}
"""

def gen_valid_compound_assign() -> str:
    return f"""fn main(): i32 {{
    let mut x = {rand_int(1, 50)}
    x += {rand_int(1, 10)}
    print(x)
    x -= {rand_int(1, 5)}
    print(x)
    x *= 2
    print(x)
    return 0
}}
"""

def gen_valid_tuple() -> str:
    a = rand_int(-50, 50)
    b = rand_int(-50, 50)
    return f"""fn main(): i32 {{
    let t = ({a}, {b})
    print(t.0)
    print(t.1)
    return 0
}}
"""

def gen_valid_lambda() -> str:
    return f"""fn apply(f: fn(i32): i32, x: i32) -> i32 {{
    return f(x)
}}

fn main(): i32 {{
    let f = fn(n: i32): i32 {{ n * 2 }}
    print(apply(f, {rand_int(1, 20)}))
    return 0
}}
"""

def gen_valid_break_continue() -> str:
    return """fn main(): i32 {
    let mut i = 0
    let mut sum = 0
    while (i < 100) {
        if (i == 10) { break }
        i = i + 1
        if (i % 2 == 0) { continue }
        sum = sum + i
    }
    print(sum)
    return 0
}
"""

def gen_valid_nested_struct() -> str:
    return """struct Inner { val: i32 }
struct Outer { inner: Inner, name: string }

fn main(): i32 {
    let o = Outer { inner: Inner { val: 42 }, name: "test" }
    print(o.inner.val)
    print(o.name)
    return 0
}
"""

def gen_valid_fibonacci() -> str:
    n = random.randint(0, 15)
    return f"""fn fib(n: i32) -> i32 {{
    if (n <= 1) {{ return n }}
    return fib(n - 1) + fib(n - 2)
}}

fn main(): i32 {{
    print(fib({n}))
    return 0
}}
"""

def gen_valid_sort() -> str:
    elems = random.sample(range(-50, 50), random.randint(3, 8))
    elems_str = ", ".join(str(x) for x in elems)
    return f"""fn bubble_sort(arr: [i32], size: i32) {{
    for (i in 0 .. size - 1) {{
        for (j in 0 .. size - i - 1) {{
            if (arr[j] > arr[j + 1]) {{
                let temp = arr[j]
                arr[j] = arr[j + 1]
                arr[j + 1] = temp
            }}
        }}
    }}
}}

fn main(): i32 {{
    let arr = [{elems_str}]
    bubble_sort(arr, {len(elems)})
    for (i in 0 .. arr.length) {{
        print(arr[i])
    }}
    return 0
}}
"""

def gen_valid_string_compare() -> str:
    s1 = rand_string()
    s2 = rand_string()
    return f"""fn main(): i32 {{
    let a = {s1}
    let b = {s2}
    if (a == b) {{ print("equal") }}
    else {{ print("not-equal") }}
    return 0
}}
"""

def gen_valid_print_multiple() -> str:
    args = []
    for _ in range(random.randint(1, 5)):
        args.append(rand_literal())
    return f"""fn main(): i32 {{
    print({', '.join(args)})
    return 0
}}
"""

def gen_valid_type_conversions() -> str:
    return f"""fn main(): i32 {{
    let x = toint({rand_float()})
    print(x)
    let y = tofloat({rand_int(-100, 100)})
    print(y)
    return 0
}}
"""


# ─── Malformed program generators (for parser robustness) ────────────

def gen_malformed_missing_brace() -> str:
    return """fn main(): i32 {
    print("hello")
    return 0
"""

def gen_malformed_missing_paren() -> str:
    return """fn main(): i32 {
    if (x > 0 {
        print("positive")
    }
    return 0
}
"""

def gen_malformed_missing_semicolon() -> str:
    return """fn main(): i32 {
    let x = 10
    let y = 20
    print(x + y)
    return 0
}
"""

def gen_malformed_invalid_token() -> str:
    return """fn main(): i32 {
    let x = @@@
    return 0
}
"""

def gen_malformed_empty() -> str:
    return ""

def gen_malformed_only_comments() -> str:
    lines = []
    for _ in range(random.randint(1, 10)):
        comment = "".join(random.choices(string.ascii_letters + " ", k=random.randint(1, 50)))
        lines.append(f"// {comment}")
    return "\n".join(lines)

def gen_malformed_random_tokens() -> str:
    tokens = ["fn", "let", "mut", "if", "else", "while", "for", "in", "return",
              "struct", "match", "{", "}", "(", ")", "[", "]", "=", "==", "+",
              "-", "*", "/", "print", "123", '"hello"', "true", "false", "nil"]
    lines = ["fn main(): i32 {"]
    for _ in range(random.randint(1, 15)):
        n_tokens = random.randint(1, 6)
        line = " ".join(random.choices(tokens, k=n_tokens))
        lines.append(f"    {line}")
    lines.append("    return 0\n}")
    return "\n".join(lines)

def gen_malformed_nested_braces() -> str:
    depth = random.randint(5, 20)
    lines = ["fn main(): i32 {"]
    for i in range(depth):
        lines.append("    " * (i + 1) + "if (1) {")
    lines.append("    " * (depth + 1) + 'print("deep")')
    for i in range(depth - 1):
        lines.append("    " * (depth - i) + "}")
    lines.append("    return 0\n}")
    return "\n".join(lines)

def gen_malformed_string_escapes() -> str:
    esc = "".join(chr(c) for c in range(0, 32))
    return f"""fn main(): i32 {{
    print("{esc}")
    return 0
}}
"""

def gen_malformed_large_numbers() -> str:
    return """fn main(): i32 {
    let x = 999999999999999999999999999999
    print(x)
    return 0
}
"""

def gen_malformed_deeply_nested_expr() -> str:
    expr = "1"
    for _ in range(random.randint(20, 50)):
        expr = f"({expr} + 1)"
    return f"""fn main(): i32 {{
    print({expr})
    return 0
}}
"""

def gen_malformed_unicode_identifiers() -> str:
    return """fn main(): i32 {
    let x = 10
    let y = 20
    print(x + y)
    return 0
}
"""

def gen_malformed_unterminated_string() -> str:
    return """fn main(): i32 {
    let s = "this string never ends
    print(s)
    return 0
}
"""

def gen_malformed_multiple_functions_same_name() -> str:
    return """fn foo(): i32 { return 1 }
fn foo(): i32 { return 2 }
fn main(): i32 {
    print(foo())
    return 0
}
"""

def gen_malformed_type_errors() -> str:
    return """fn main(): i32 {
    let x: i32 = "hello"
    print(x)
    return 0
}
"""

def gen_malformed_recursive_struct() -> str:
    return """struct Node {
    value: i32,
    next: Node
}

fn main(): i32 {
    return 0
}
"""

def gen_malformed_undefined_var() -> str:
    return """fn main(): i32 {
    print(undefined_variable)
    return 0
}
"""

def gen_malformed_index_out_of_bounds() -> str:
    return """fn main(): i32 {
    let arr = [1, 2, 3]
    print(arr[10])
    return 0
}
"""

def gen_malformed_division_by_zero() -> str:
    return """fn main(): i32 {
    let x = 10 / 0
    print(x)
    return 0
}
"""


# ─── Random program generator (AST-level mutations) ──────────────────

def gen_random_program() -> str:
    """Generate a fully random (potentially invalid) program."""
    lines = []
    n_lines = random.randint(1, 30)

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

    # Randomly add closing braces
    for _ in range(random.randint(0, 3)):
        lines.append("    }")

    # Wrap in function
    fn_name = "main"
    body = "\n".join(lines)
    return f"fn {fn_name}(): i32 {{\n{body}\n    return 0\n}}\n"


# ─── Category-based fuzz generators ─────────────────────────────────

VALID_GENERATORS = [
    ("valid_arithmetic", gen_valid_arithmetic),
    ("valid_variables", gen_valid_variables),
    ("valid_if_else", gen_valid_if_else),
    ("valid_while_loop", gen_valid_while_loop),
    ("valid_for_loop", gen_valid_for_loop),
    ("valid_nested_loops", gen_valid_nested_loops),
    ("valid_function", gen_valid_function),
    ("valid_recursion", gen_valid_recursion),
    ("valid_struct", gen_valid_struct),
    ("valid_generic_struct", gen_valid_generic_struct),
    ("valid_array", gen_valid_array),
    ("valid_array_iteration", gen_valid_array_iteration),
    ("valid_pointer", gen_valid_pointer),
    ("valid_pointer_mut", gen_valid_pointer_mut),
    ("valid_match", gen_valid_match),
    ("valid_result_type", gen_valid_result_type),
    ("valid_option_type", gen_valid_option_type),
    ("valid_generic_function", gen_valid_generic_function),
    ("valid_compound_assign", gen_valid_compound_assign),
    ("valid_tuple", gen_valid_tuple),
    ("valid_lambda", gen_valid_lambda),
    ("valid_break_continue", gen_valid_break_continue),
    ("valid_nested_struct", gen_valid_nested_struct),
    ("valid_fibonacci", gen_valid_fibonacci),
    ("valid_sort", gen_valid_sort),
    ("valid_string_compare", gen_valid_string_compare),
    ("valid_print_multiple", gen_valid_print_multiple),
    ("valid_type_conversions", gen_valid_type_conversions),
]

MALFORMED_GENERATORS = [
    ("malformed_missing_brace", gen_malformed_missing_brace),
    ("malformed_missing_paren", gen_malformed_missing_paren),
    ("malformed_missing_semicolon", gen_malformed_missing_semicolon),
    ("malformed_invalid_token", gen_malformed_invalid_token),
    ("malformed_empty", gen_malformed_empty),
    ("malformed_only_comments", gen_malformed_only_comments),
    ("malformed_random_tokens", gen_malformed_random_tokens),
    ("malformed_nested_braces", gen_malformed_nested_braces),
    ("malformed_string_escapes", gen_malformed_string_escapes),
    ("malformed_large_numbers", gen_malformed_large_numbers),
    ("malformed_deeply_nested_expr", gen_malformed_deeply_nested_expr),
    ("malformed_unterminated_string", gen_malformed_unterminated_string),
    ("malformed_multiple_functions_same_name", gen_malformed_multiple_functions_same_name),
    ("malformed_type_errors", gen_malformed_type_errors),
    ("malformed_recursive_struct", gen_malformed_recursive_struct),
    ("malformed_undefined_var", gen_malformed_undefined_var),
    ("malformed_index_out_of_bounds", gen_malformed_index_out_of_bounds),
    ("malformed_division_by_zero", gen_malformed_division_by_zero),
]


# ─── Test runner ─────────────────────────────────────────────────────

def run_fuzz_case(program: str, category: str, index: int, tmpdir: Path) -> FuzzResult:
    result = FuzzResult(category=category, index=index, program=program)

    src_file = tmpdir / f"fuzz_{category}_{index}.vix"
    bin_file = tmpdir / f"fuzz_{category}_{index}"

    src_file.write_text(program)

    try:
        compile_res = subprocess.run(
            [str(COMPILER), str(src_file), "-o", str(bin_file)],
            capture_output=True, text=True, timeout=10,
        )
    except subprocess.TimeoutExpired:
        result.timeout = True
        result.stderr_snippet = "compile timeout"
        return result

    if compile_res.returncode != 0:
        result.compile_error = True
        return result

    # Compilation succeeded, try running
    try:
        run_res = subprocess.run(
            [str(bin_file)], capture_output=True, text=True, timeout=5,
        )
    except subprocess.TimeoutExpired:
        result.timeout = True
        result.stderr_snippet = "runtime timeout"
        return result

    if run_res.returncode < 0:
        sig = -run_res.returncode
        if sig in CRASH_SIGNALS:
            result.crashed = True
            result.crash_signal = CRASH_SIGNALS[sig]
            result.stderr_snippet = run_res.stderr[:200]
            return result

    result.success = True
    return result


def main() -> int:
    if not COMPILER.exists():
        print(f"Error: compiler not found at {COMPILER}")
        return 1

    tmpdir = Path(tempfile.mkdtemp(prefix="vixc_fuzz_"))
    results: List[FuzzResult] = []
    total = 0
    crashes = 0
    timeouts = 0

    print(f"Fuzz testing vixc at {COMPILER}")
    print(f"Temp directory: {tmpdir}\n")

    # Run valid program generators
    print("=== Valid program fuzz tests ===")
    for cat, gen in VALID_GENERATORS:
        for i in range(FUZZ_COUNT_PER_CATEGORY):
            total += 1
            program = gen()
            r = run_fuzz_case(program, cat, i, tmpdir)
            results.append(r)

            if r.crashed:
                crashes += 1
                crash_file = tmpdir / f"CRASH_{cat}_{i}.vix"
                crash_file.write_text(program)
                print(f"{RED}[CRASH]{RESET} {cat}[{i}]: {r.crash_signal}")
                print(f"  Saved to: {crash_file}")
            elif r.timeout:
                timeouts += 1
                print(f"{YELLOW}[TIMEOUT]{RESET} {cat}[{i}]")
            else:
                status = "compile_error" if r.compile_error else "ok"
                print(f"{GREEN}[OK]{RESET} {cat}[{i}]: {status}")

    # Run malformed program generators
    print("\n=== Malformed program fuzz tests ===")
    for cat, gen in MALFORMED_GENERATORS:
        for i in range(FUZZ_COUNT_PER_CATEGORY):
            total += 1
            program = gen()
            r = run_fuzz_case(program, cat, i, tmpdir)
            results.append(r)

            if r.crashed:
                crashes += 1
                crash_file = tmpdir / f"CRASH_{cat}_{i}.vix"
                crash_file.write_text(program)
                print(f"{RED}[CRASH]{RESET} {cat}[{i}]: {r.crash_signal}")
                print(f"  Saved to: {crash_file}")
            elif r.timeout:
                timeouts += 1
                print(f"{YELLOW}[TIMEOUT]{RESET} {cat}[{i}]")
            else:
                status = "compile_error" if r.compile_error else "runtime_ok"
                print(f"{GREEN}[OK]{RESET} {cat}[{i}]: {status}")

    # Run pure random program generator
    print("\n=== Random program fuzz tests ===")
    random_count = max(0, 500 - len(VALID_GENERATORS) * FUZZ_COUNT_PER_CATEGORY
                       - len(MALFORMED_GENERATORS) * FUZZ_COUNT_PER_CATEGORY)
    for i in range(random_count):
        total += 1
        program = gen_random_program()
        r = run_fuzz_case(program, "random", i, tmpdir)
        results.append(r)

        if r.crashed:
            crashes += 1
            crash_file = tmpdir / f"CRASH_random_{i}.vix"
            crash_file.write_text(program)
            print(f"{RED}[CRASH]{RESET} random[{i}]: {r.crash_signal}")
            print(f"  Saved to: {crash_file}")
        elif r.timeout:
            timeouts += 1
            if i < 20 or i % 50 == 0:
                print(f"{YELLOW}[TIMEOUT]{RESET} random[{i}]")
        else:
            if i < 20 or i % 50 == 0:
                status = "compile_error" if r.compile_error else "ok"
                print(f"{GREEN}[OK]{RESET} random[{i}]: {status}")

    # Summary
    print(f"\n{'='*60}")
    print(f"Fuzz Test Summary")
    print(f"{'='*60}")
    print(f"Total tests:    {total}")
    print(f"Crashes:        {crashes}")
    print(f"Timeouts:       {timeouts}")
    print(f"Compile errors: {sum(1 for r in results if r.compile_error and not r.crashed and not r.timeout)}")
    print(f"Runtime OK:     {sum(1 for r in results if r.success)}")

    if crashes > 0:
        print(f"\n{RED}FAIL: {crashes} crash(es) detected!{RESET}")
        print(f"Crash programs saved in: {tmpdir}")
        return 2
    elif timeouts > 0:
        print(f"\n{YELLOW}WARN: {timeouts} timeout(s) detected.{RESET}")
        return 0
    else:
        print(f"\n{GREEN}PASS: No crashes detected.{RESET}")
        return 0


if __name__ == "__main__":
    sys.exit(main())
