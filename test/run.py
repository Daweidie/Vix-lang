import os
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Optional


@dataclass
class Expectation:
    should_compile: bool
    run_output: Optional[List[str]] = None
    compile_error_contains: Optional[str] = None


ROOT = Path(__file__).resolve().parent
COMPILER = ROOT.parent / "src" / "vixc"
BIN_DIR = ROOT / "bin"


EXPECT: Dict[str, Expectation] = {
    "test2.vix": Expectation(False, compile_error_contains="capturing local variables"),
    "test3.vix": Expectation(False, compile_error_contains="capturing local variables"),
    "test4.vix": Expectation(False, compile_error_contains="type mismatch"),
    "test7.vix": Expectation(False, compile_error_contains="type mismatch"),
    "test8.vix": Expectation(False, compile_error_contains="type mismatch"),
    "test10.vix": Expectation(True, run_output=["3", "2"]),
    "test13.vix": Expectation(True, run_output=["2"]),
    "test14.vix": Expectation(True, run_output=["2"]),
    "test15.vix": Expectation(True, run_output=["3"]),
    "test16.vix": Expectation(False, compile_error_contains="self-recursive struct fields"),
    "test17.vix": Expectation(False, compile_error_contains="self-recursive struct fields"),
    "test18.vix": Expectation(True, run_output=["matched-err"]),
    "test19.vix": Expectation(True, run_output=["10", "20", "30", "3"]),
    "test20.vix": Expectation(True, run_output=["45"]),
    "test21.vix": Expectation(True, run_output=["55"]),
    "test22.vix": Expectation(True, run_output=["large", "medium", "small", "non-positive"]),
    "test23.vix": Expectation(True, run_output=["25"]),
    "test24.vix": Expectation(True, run_output=["1", "2", "42", "hello"]),
    "test25.vix": Expectation(True, run_output=["3", "5", "1", "5"]),
    "test26.vix": Expectation(True, run_output=["42", "hello", "43"]),
    "test27.vix": Expectation(False, compile_error_contains="undefined identifier"),
    "test28.vix": Expectation(True, run_output=["99", "test"]),
    "test29.vix": Expectation(True, run_output=["5"]),
    "test30.vix": Expectation(True, run_output=["42", "world"]),
}


def _sorted_tests() -> List[Path]:
    tests = list(ROOT.glob("test*.vix"))

    def key(p: Path) -> int:
        m = re.match(r"test(\d+)\.vix$", p.name)
        return int(m.group(1)) if m else 10**9

    return sorted(tests, key=key)


def run_cmd(cmd: List[str], cwd: Path) -> subprocess.CompletedProcess:
    return subprocess.run(cmd, cwd=str(cwd), text=True, capture_output=True)


def _bin_path_for(path: Path) -> Path:
    # Keep each test artifact under test/bin for unified execution.
    # Executables do not require a file extension on Linux.
    return BIN_DIR / path.stem


def compile_test(path: Path) -> (bool | Optional[Path]):
    exp = EXPECT.get(path.name)
    out_bin = _bin_path_for(path)
    compile_res = run_cmd([str(COMPILER), str(path), "-o", str(out_bin)], ROOT)

    if exp is not None and not exp.should_compile:
        if compile_res.returncode == 0:
            print(f"[FAIL] {path.name}: expected compile failure but compiled")
            return False, None
        if exp.compile_error_contains and exp.compile_error_contains not in compile_res.stderr:
            print(f"[FAIL] {path.name}: compile failed but missing expected error text")
            return False, None
        print(f"[PASS] {path.name}: expected compile error")
        return True, None

    if compile_res.returncode != 0:
        print(f"[FAIL] {path.name}: compile failed")
        print(compile_res.stderr.splitlines()[:6])
        return False, None

    print(f"[PASS] {path.name}: compiled -> {out_bin.name}")
    return True, out_bin


def run_test(path: Path, bin_path: Path) -> bool:
    exp = EXPECT.get(path.name)

    run_res = run_cmd([str(bin_path)], ROOT)
    if run_res.returncode != 0:
        print(f"[FAIL] {path.name}: runtime failed ({run_res.returncode})")
        return False

    out_lines = [line.strip() for line in run_res.stdout.splitlines() if line.strip()]
    if exp is not None and exp.run_output is not None:
        if out_lines != exp.run_output:
            print(f"[FAIL] {path.name}: output mismatch")
            print(f"  expected: {exp.run_output}")
            print(f"  actual:   {out_lines}")
            return False

    print(f"[PASS] {path.name}")
    return True


def main() -> int:
    tests = _sorted_tests()
    if not tests:
        print("No testN.vix files found")
        return 1

    BIN_DIR.mkdir(parents=True, exist_ok=True)

    for old_bin in BIN_DIR.glob("test*"):
        if old_bin.is_file():
            old_bin.unlink(missing_ok=True)

    compile_passed = 0
    runnable: List[tuple[Path, Path]] = []
    for t in tests:
        ok, out_bin = compile_test(t)
        if ok:
            compile_passed += 1
            if out_bin is not None:
                runnable.append((t, out_bin))

    run_passed = 0
    for src, exe in runnable:
        if run_test(src, exe):
            run_passed += 1

    passed = compile_passed - (len(runnable) - run_passed)

    total = len(tests)
    print(f"\nCompile summary: {compile_passed}/{total} passed")
    print(f"Run summary: {run_passed}/{len(runnable)} passed")
    print(f"Summary: {passed}/{total} passed")
    return 0 if passed == total else 2

if __name__ == "__main__":
    sys.exit(main())