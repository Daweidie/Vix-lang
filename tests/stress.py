import subprocess
import sys
import pytest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from helpers import compile_vix, run_binary, compile_and_run


@pytest.mark.stress
class TestDeepNesting:
    def test_deeply_nested_if(self, compiler, tmp_path):
        depth = 20
        lines = ['fn main(): i32 {']
        for i in range(depth):
            lines.append('    ' * (i + 1) + 'if (1) {')
        lines.append('    ' * (depth + 1) + 'print("deep")')
        for i in range(depth):
            lines.append('    ' * (depth - i) + '}')
        lines.append('    return 0\n}')
        src = '\n'.join(lines)
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "deep"

    def test_deeply_nested_expressions(self, compiler, tmp_path):
        expr = "1"
        for _ in range(30):
            expr = f"({expr} + 1)"
        src = f'fn main(): i32 {{ print({expr}) return 0 }}'
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "31"

    def test_deeply_nested_blocks(self, compiler, tmp_path):
        depth = 10
        lines = ['fn main(): i32 {']
        for i in range(depth):
            lines.append('    ' * (i + 1) + 'if (1) {')
        lines.append('    ' * (depth + 1) + 'print("block")')
        for i in range(depth):
            lines.append('    ' * (depth - i) + '}')
        lines.append('    return 0\n}')
        src = '\n'.join(lines)
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "block"


@pytest.mark.stress
class TestLargePrograms:
    def test_many_functions(self, compiler, tmp_path):
        lines = []
        for i in range(50):
            lines.append(f'fn func_{i}() {{ print({i}) }}')
        lines.append('fn main(): i32 {')
        for i in range(50):
            lines.append(f'    func_{i}()')
        lines.append('    return 0\n}')
        src = '\n'.join(lines)
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        out_lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert len(out_lines) == 50
        assert out_lines[0] == "0"
        assert out_lines[49] == "49"

    def test_many_variables(self, compiler, tmp_path):
        lines = ['fn main(): i32 {']
        for i in range(100):
            lines.append(f'    let v{i} = {i}')
        lines.append('    let mut sum = 0')
        for i in range(100):
            lines.append(f'    sum = sum + v{i}')
        lines.append('    print(sum)')
        lines.append('    return 0\n}')
        src = '\n'.join(lines)
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "4950"

    def test_many_structs(self, compiler, tmp_path):
        lines = []
        for i in range(20):
            lines.append(f'struct S{i} {{ val: i32 }}')
        lines.append('fn main(): i32 {')
        for i in range(20):
            lines.append(f'    let s{i} = S{i}{{ val: {i} }}')
        lines.append(f'    print(s0.val)')
        lines.append(f'    print(s19.val)')
        lines.append('    return 0\n}')
        src = '\n'.join(lines)
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines_out = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert lines_out == ["0", "19"]

    def test_large_array(self, compiler, tmp_path):
        elems = ', '.join(str(i) for i in range(100))
        src = f'''fn main(): i32 {{
            let arr = [{elems}]
            print(arr.length)
            print(arr[0])
            print(arr[99])
            return 0
        }}'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert lines == ["100", "0", "99"]


@pytest.mark.stress
class TestLoopsStress:
    def test_large_loop_count(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let mut sum = 0
            for (i in 0 .. 10000) {
                sum = sum + i
            }
            print(sum)
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "49995000"

    def test_nested_loop_stress(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let mut count = 0
            for (i in 0 .. 100) {
                for (j in 0 .. 100) {
                    count = count + 1
                }
            }
            print(count)
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "10000"

    def test_while_loop_stress(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            let mut i = 0
            let mut sum = 0
            while (i < 10000) {
                sum = sum + i
                i = i + 1
            }
            print(sum)
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "49995000"


@pytest.mark.stress
class TestRecursionStress:
    def test_deep_recursion(self, compiler, tmp_path):
        src = '''fn countdown(n: i32) -> i32 {
            if (n <= 0) { return 0 }
            return countdown(n - 1)
        }
        fn main(): i32 {
            print(countdown(500))
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "0"

    def test_fibonacci_stress(self, compiler, tmp_path):
        src = '''fn fib(n: i32) -> i32 {
            if (n <= 1) { return n }
            return fib(n - 1) + fib(n - 2)
        }
        fn main(): i32 {
            print(fib(20))
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        assert run.stdout.strip() == "6765"

    def test_mutual_recursion(self, compiler, tmp_path):
        src = '''fn is_even(n: i32) -> i32 {
            if (n == 0) { return 1 }
            return is_odd(n - 1)
        }
        fn is_odd(n: i32) -> i32 {
            if (n == 0) { return 0 }
            return is_even(n - 1)
        }
        fn main(): i32 {
            print(is_even(10))
            print(is_odd(10))
            print(is_even(11))
            print(is_odd(11))
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert lines == ["1", "0", "0", "1"]


@pytest.mark.stress
class TestGenericStress:
    def test_multiple_generic_instantiations(self, compiler, tmp_path):
        src = '''fn identity:[T](x: T): T { return x }
        fn main(): i32 {
            print(identity:[i32](1))
            print(identity:[i64](2))
            print(identity:[str]("hello"))
            print(identity:[f64](3.14))
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert lines == ["1", "2", "hello", "3.140000"]


@pytest.mark.stress
class TestStringStress:
    def test_many_string_operations(self, compiler, tmp_path):
        lines = ['fn main(): i32 {']
        for i in range(20):
            lines.append(f'    print("string_{i}")')
        lines.append('    return 0\n}')
        src = '\n'.join(lines)
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        out_lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert len(out_lines) == 20
        assert out_lines[0] == "string_0"
        assert out_lines[19] == "string_19"


@pytest.mark.stress
class TestComplexPrograms:
    def test_bubble_sort(self, compiler, tmp_path):
        src = '''fn bubble_sort(arr: [i32], size: i32) {
            for (i in 0 .. size - 1) {
                for (j in 0 .. size - i - 1) {
                    if (arr[j] > arr[j + 1]) {
                        let temp = arr[j]
                        arr[j] = arr[j + 1]
                        arr[j + 1] = temp
                    }
                }
            }
        }
        fn main(): i32 {
            let arr = [5, 3, 8, 1, 9, 2, 7, 4, 6, 0]
            bubble_sort(arr, 10)
            for (i in 0 .. arr.length) {
                print(arr[i])
            }
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert lines == ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"]

    def test_fizzbuzz(self, compiler, tmp_path):
        src = '''fn main(): i32 {
            for (i in 1 .. 16) {
                if (i % 15 == 0) { print("fizzbuzz") }
                elif (i % 3 == 0) { print("fizz") }
                elif (i % 5 == 0) { print("buzz") }
                else { print(i) }
            }
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        expected = ["1", "2", "fizz", "4", "buzz", "fizz", "7", "8", "fizz", "buzz",
                    "11", "fizz", "13", "14", "fizzbuzz"]
        assert lines == expected

    def test_quicksort(self, compiler, tmp_path):
        src = '''fn quicksort(arr: [i32], low: i32, high: i32) {
            if (low < high) {
                let pivot = arr[high]
                let mut i = low - 1
                for (j in low .. high) {
                    if (arr[j] <= pivot) {
                        i = i + 1
                        let temp = arr[i]
                        arr[i] = arr[j]
                        arr[j] = temp
                    }
                }
                i = i + 1
                let temp = arr[i]
                arr[i] = arr[high]
                arr[high] = temp
                quicksort(arr, low, i - 1)
                quicksort(arr, i + 1, high)
            }
        }
        fn main(): i32 {
            let arr = [3, 6, 8, 10, 1, 2, 1]
            quicksort(arr, 0, 6)
            for (i in 0 .. arr.length) {
                print(arr[i])
            }
            return 0
        }'''
        _, run = compile_and_run(compiler, src, tmp_path)
        assert run is not None
        lines = [l.strip() for l in run.stdout.splitlines() if l.strip()]
        assert lines == ["1", "1", "2", "3", "6", "8", "10"]
