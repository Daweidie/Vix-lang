import os
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Optional


GREEN = "\033[32m"
RED = "\033[31m"
RESET = "\033[0m"


@dataclass
class Expectation:
    should_compile: bool
    run_output: Optional[List[str]] = None
    compile_error_contains: Optional[str] = None


ROOT = Path(__file__).resolve().parent.parent
COMPILER = ROOT / "build" / "vixc"
REGRESSION_DIR = Path(__file__).resolve().parent / "regression"
BIN_DIR = Path(__file__).resolve().parent / "bin"


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
    "test31.vix": Expectation(True, run_output=["42", "division by zero"]),
    "test32.vix": Expectation(True, run_output=["1", "2", "3"]),
    "test33.vix": Expectation(True, run_output=["5", "cannot divide by zero"]),
    "test34.vix": Expectation(True, run_output=["100", "-1"]),
    "test35.vix": Expectation(True, run_output=["10", "20", "30"]),
    "test36.vix": Expectation(True, run_output=["correct", "error matched", "0"]),
    "test37.vix": Expectation(True, run_output=["i32", "empty is empty"]),
    "test38.vix": Expectation(True, run_output=["5", "2", "3", "1", "5", "hello", "world", "1.500000"]),
    "test39.vix": Expectation(True, run_output=["0", "0", "0"]),
    "test40.vix": Expectation(True, run_output=["42", "100", "3.140000", "hello", "65", "50", "101", "4.000000"]),
    "test41.vix": Expectation(True, run_output=["10", "20", "test", "3", "1", "10"]),
    "test42.vix": Expectation(True, run_output=["7", "world", "100", "200"]),
    "test43.vix": Expectation(True, run_output=["30", "60", "55", "5.140000", "hello", "world", "1"]),
    "test44.vix": Expectation(True, run_output=["42", "-1", "alice"]),
    "test45.vix": Expectation(True, run_output=["10", "2"]),
    "test46.vix": Expectation(True, run_output=["150", "130", "390", "39", "4", "3"]),
    "test47.vix": Expectation(True, run_output=["ab", "bc", "ascending", "mixed", "2", "2", "0"]),
    "test48.vix": Expectation(True, run_output=["100", "3", "103", "21"]),
    "test49.vix": Expectation(True, run_output=["5", "10", "50", "99", "77", "55", "291"]),
    "test50.vix": Expectation(True, run_output=["255", "10", "16", "42", "100", "3.140000", "2.500000"]),
    "test51.vix": Expectation(True, run_output=["1", "2", "3", "4", "5", "6", "7", "8", "9"]),
    "test52.vix": Expectation(True, run_output=["123", "3.14", "A", "1", "0", "0"]),
    "test53.vix": Expectation(True, run_output=["has-value", "error"]),
    "test54.vix": Expectation(True, run_output=["55", "5050", "120", "3628800", "1", "0"]),
    "test55.vix": Expectation(True, run_output=["7", "17", "0"]),
    "test56.vix": Expectation(True, run_output=["5", "-1", "1", "1", "4", "5"]),
    "test57.vix": Expectation(True, run_output=["0", "1", "2", "3", "4", "5", "6", "7", "8"]),
    "test58.vix": Expectation(True, run_output=["42", "hello", "2.718000"]),
    "test59.vix": Expectation(True, run_output=["30", "10", "world"]),
    "test60.vix": Expectation(True, run_output=["10", "20", "20", "10", "20", "40"]),
    "test61.vix": Expectation(False, compile_error_contains="non-exhaustive match"),
    "test62.vix": Expectation(True, run_output=["30", "20", "42", "25", "2"]),
    "test63.vix": Expectation(True, run_output=["14", "20", "4", "5", "26"]),
    "test64.vix": Expectation(True, run_output=["30", "10", "200", "2", "0"]),
    "test65.vix": Expectation(True, run_output=["greater", "equal", "not-equal", "less", "gte", "lte"]),
    "test66.vix": Expectation(True, run_output=["both", "at-least-one"]),
    "test67.vix": Expectation(True, run_output=["5", "15", "5"]),
    "test68.vix": Expectation(True, run_output=["52", "32", "420"]),
    "test69.vix": Expectation(True, run_output=["200", "55", "255"]),
    "test70.vix": Expectation(True, run_output=["1000000", "999998", "1999998"]),
    "test71.vix": Expectation(True, run_output=["103", "97", "300", "33", "1"]),
    "test72.vix": Expectation(True, run_output=["5.140000", "6.280000"]),
    "test73.vix": Expectation(True, run_output=["4.000000", "3.750000", "1.666667"]),
    "test74.vix": Expectation(True, run_output=["same", "different"]),
    "test75.vix": Expectation(True, run_output=["Hello, World!", "not-empty"]),
    "test76.vix": Expectation(True, run_output=["line1", "line2", "tab\there", "backslash: \\", "quote: \""]),
    "test77.vix": Expectation(True, run_output=["medium"]),
    "test78.vix": Expectation(True, run_output=["5", "4", "3", "2", "1", "done"]),
    "test79.vix": Expectation(True, run_output=["0", "1", "2", "3", "4", "5", "6", "7", "8"]),
    "test80.vix": Expectation(True, run_output=["0", "1", "2", "3", "4"]),
    "test81.vix": Expectation(True, run_output=["55"]),
    "test82.vix": Expectation(True, run_output=["9"]),
    "test83.vix": Expectation(True, run_output=["5"]),
    "test84.vix": Expectation(True, run_output=["5"]),
    "test85.vix": Expectation(True, run_output=["0", "1", "5", "55"]),
    "test86.vix": Expectation(True, run_output=["1", "1", "120", "3628800"]),
    "test87.vix": Expectation(True, run_output=["4", "25", "1", "5"]),
    "test88.vix": Expectation(True, run_output=["6", "60", "0", "0"]),
    "test89.vix": Expectation(True, run_output=["0", "1", "25", "100", "9"]),
    "test90.vix": Expectation(True, run_output=["10", "20"]),
    "test91.vix": Expectation(True, run_output=["0"]),
    "test92.vix": Expectation(True, run_output=["42", "test"]),
    "test93.vix": Expectation(True, run_output=["3", "10", "30"]),
    "test94.vix": Expectation(True, run_output=["100", "num", "hello", "str"]),
    "test95.vix": Expectation(True, run_output=["4", "6", "3", "8"]),
    "test96.vix": Expectation(True, run_output=["10", "20", "30"]),
    "test97.vix": Expectation(True, run_output=["5", "ok"]),
    "test98.vix": Expectation(True, run_output=["15", "60"]),
    "test99.vix": Expectation(True, run_output=["3", "1", "2", "3"]),
    "test100.vix": Expectation(True, run_output=["10", "20", "30", "3"]),
    "test101.vix": Expectation(True, run_output=["4", "99"]),
    "test102.vix": Expectation(True, run_output=["42"]),
    "test103.vix": Expectation(True, run_output=["20", "20"]),
    "test104.vix": Expectation(True, run_output=["10", "20", "20", "10"]),
    "test105.vix": Expectation(True, run_output=["two"]),
    "test106.vix": Expectation(True, run_output=["zero", "one", "many"]),
    "test107.vix": Expectation(True, run_output=["3", "-1"]),
    "test108.vix": Expectation(True, run_output=["0", "0", "0.000000", "ok", "0"]),
    "test109.vix": Expectation(True, run_output=["15", "12", "48", "24", "3"]),
    "test110.vix": Expectation(True, run_output=["255", "0", "10", "16", "256"]),
    "test111.vix": Expectation(True, run_output=["10", "20"]),
    "test112.vix": Expectation(True, run_output=["1", "2", "3"]),
    "test113.vix": Expectation(True, run_output=["10", "15", "11"]),
    "test114.vix": Expectation(True, run_output=["10", "42"]),
    "test115.vix": Expectation(True, run_output=["3"]),
    "test116.vix": Expectation(True, run_output=["100"]),
    "test117.vix": Expectation(True, run_output=["10", "10", "10", "10", "0"]),
    "test118.vix": Expectation(True, run_output=["0", "-1", "-100", "1", "0"]),
    "test119.vix": Expectation(True, run_output=["1", "0", "1", "0", "1"]),
    "test120.vix": Expectation(True, run_output=["5", "5", "0", "100"]),
    "test121.vix": Expectation(True, run_output=["3", "2", "5", "7", "10", "5"]),
    "test122.vix": Expectation(True, run_output=["0", "0", "1", "1", "0", "1", "1", "0"]),
    "test123.vix": Expectation(True, run_output=["1", "2", "1024", "27", "1000"]),
    "test124.vix": Expectation(True, run_output=["0", "1", "55", "6765"]),
    "test125.vix": Expectation(True, run_output=["1", "1", "120", "3628800"]),
    "test126.vix": Expectation(True, run_output=["12", "21", "36", "100"]),
    "test127.vix": Expectation(True, run_output=["5050"]),
    "test128.vix": Expectation(True, run_output=["Alice", "30"]),
    "test129.vix": Expectation(True, run_output=["10", "20", "30", "5"]),
    "test130.vix": Expectation(True, run_output=["55"]),
    "test131.vix": Expectation(True, run_output=["11", "12", "22", "25", "34", "64", "90"]),
    "test132.vix": Expectation(True, run_output=["5", "4", "3", "2", "1"]),
    "test133.vix": Expectation(True, run_output=["3", "1", "0"]),
    "test134.vix": Expectation(True, run_output=["9", "100"]),
    "test135.vix": Expectation(True, run_output=["2", "0", "-1"]),
    "test136.vix": Expectation(True, run_output=["hello", "world"]),
    "test137.vix": Expectation(True, run_output=["equal", "not-equal"]),
    "test138.vix": Expectation(True, run_output=["20", "40", "60"]),
    "test139.vix": Expectation(True, run_output=["1", "60"]),
    "test140.vix": Expectation(True, run_output=["3", "3", "3", "3", "3"]),
    "test141.vix": Expectation(True, run_output=["0", "1", "8", "111"]),
    "test142.vix": Expectation(True, run_output=["0", "1", "6", "36"]),
    "test143.vix": Expectation(True, run_output=["5", "0", "10", "0", "10"]),
    "test144.vix": Expectation(True, run_output=["15", "120", "-5"]),
    "test145.vix": Expectation(True, run_output=["3", "1", "-3"]),
    "test146.vix": Expectation(True, run_output=["0", "0", "10", "20"]),
    "test147.vix": Expectation(True, run_output=["5"]),
    "test148.vix": Expectation(True, run_output=["35"]),
    "test149.vix": Expectation(True, run_output=["7"]),
    "test150.vix": Expectation(True, run_output=["1024"]),
    "test151.vix": Expectation(True, run_output=["0", "1", "2", "3"]),
    "test152.vix": Expectation(True, run_output=["red", "blue"]),
    "test153.vix": Expectation(True, run_output=["42", "oops"]),
    "test154.vix": Expectation(True, run_output=["10", "none"]),
    "test155.vix": Expectation(True, run_output=["55"]),
    "test156.vix": Expectation(True, run_output=["1", "2", "fizz", "4", "buzz", "fizz", "7", "8", "fizz", "buzz", "11", "fizz", "13", "14", "fizzbuzz"]),
    "test157.vix": Expectation(True, run_output=["5", "5"]),
    "test158.vix": Expectation(True, run_output=["five"]),
    "test159.vix": Expectation(True, run_output=["0", "0", "0.000000", "ok"]),
    "test160.vix": Expectation(True, run_output=["3", "42.000000", "0"]),
    "test161.vix": Expectation(True, run_output=["60", "230", "610", "900"]),
    "test162.vix": Expectation(True, run_output=["499500"]),
    "test163.vix": Expectation(True, run_output=["0", "0", "1", "4", "81"]),
    "test164.vix": Expectation(True, run_output=["1", "0", "1", "1"]),
    "test165.vix": Expectation(True, run_output=["2", "6", "4", "1", "1"]),
    "test166.vix": Expectation(True, run_output=["45", "55", "35"]),
    "test167.vix": Expectation(True, run_output=["10", "20", "30", "5"]),
    "test168.vix": Expectation(True, run_output=["zero", "zero"]),
    "test169.vix": Expectation(True, run_output=["150", "5"]),
    "test170.vix": Expectation(True, run_output=["30", "200", "6"]),
    "test171.vix": Expectation(True, run_output=["1", "6", "2", "3", "5", "29"]),
    "test172.vix": Expectation(True, run_output=["42", "none", "hello"]),
    "test173.vix": Expectation(True, run_output=["has-value", "none", "has-string"]),
    "test174.vix": Expectation(True, run_output=["true", "not-false", "and-false", "or-true"]),
    "test175.vix": Expectation(True, run_output=["1024", "27", "1", "5", "1"]),
    "test176.vix": Expectation(True, run_output=["20", "10"]),
    "test177.vix": Expectation(True, run_output=["255", "10", "265", "16"]),
    "test178.vix": Expectation(True, run_output=["10", "20", "30", "100", "200"]),
    "test179.vix": Expectation(True, run_output=["10", "20", "30", "60"]),
    "test180.vix": Expectation(True, run_output=["1", "2", "100", "200"]),
    "test181.vix": Expectation(True, run_output=["100"]),
    "test182.vix": Expectation(True, run_output=["0", "100", "0.000000", "large"]),
    "test183.vix": Expectation(True, run_output=["7", "12", "12", "11"]),
    "test184.vix": Expectation(True, run_output=["255", "10", "265", "16"]),
    "test185.vix": Expectation(True, run_output=["50", "15", "6"]),
    "test186.vix": Expectation(True, run_output=["0", "1", "3", "4", "15"]),
    "test187.vix": Expectation(True, run_output=["1", "13", "3"]),
    "test188.vix": Expectation(True, run_output=["3", "10", "20", "30"]),
    "test189.vix": Expectation(True, run_output=["0", "1", "less", "not-zero"]),
    "test190.vix": Expectation(True, run_output=["10", "20", "30", "100", "300"]),
    "test191.vix": Expectation(True, run_output=["2432902008176640000", "255", "1.500000"]),
    "test192.vix": Expectation(True, run_output=["10", "11", "12"]),
    "test193.vix": Expectation(True, run_output=["12.000000", "30.000000"]),
    "test194.vix": Expectation(True, run_output=["active"]),
    "test195.vix": Expectation(True, run_output=["42", "-1"]),
    "test196.vix": Expectation(True, run_output=["world", "hello world", "1", "0", "1"]),
    "test197.vix": Expectation(True, run_output=["100", "100"]),
    "test198.vix": Expectation(True, run_output=["100", "100", "30", "20"]),
    "test199.vix": Expectation(True, run_output=["5", "1", "5", "15"]),
    "test200.vix": Expectation(True, run_output=["10", "20"]),
    "test201.vix": Expectation(True, run_output=["22", "2", "120", "1", "2"]),
    "test202.vix": Expectation(True, run_output=["1", "2"]),
    "test203.vix": Expectation(True, run_output=["4", "25", "12", "21"]),
    "test204.vix": Expectation(True, run_output=["2", "3", "5", "7", "11", "13", "17", "19"]),
    "test205.vix": Expectation(True, run_output=["42", "42"]),
    "test206.vix": Expectation(False, compile_error_contains="undefined identifier"),
    "test207.vix": Expectation(False, compile_error_contains="redefinition"),
    "test208.vix": Expectation(True, run_output=[]),
    "test209.vix": Expectation(True, run_output=["255", "10", "265"]),
    "test210.vix": Expectation(True, run_output=["42"]),
    "test211.vix": Expectation(True, run_output=["30"]),
    "test212.vix": Expectation(True, run_output=["15", "12", "48", "24", "3"]),
}


def _sorted_tests() -> List[Path]:
    tests = list(REGRESSION_DIR.glob("test*.vix"))

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
            print(f"{RED}[FAIL]{RESET} {path.name}: expected compile failure but compiled")
            return False, None
        if exp.compile_error_contains and exp.compile_error_contains not in compile_res.stderr:
            print(f"{RED}[FAIL]{RESET} {path.name}: compile failed but missing expected error text")
            return False, None
        print(f"{GREEN}[PASS]{RESET} {path.name}: expected compile error")
        return True, None

    if compile_res.returncode != 0:
        print(f"{RED}[FAIL]{RESET} {path.name}: compile failed")
        print(compile_res.stderr.splitlines()[:6])
        return False, None

    print(f"{GREEN}[PASS]{RESET} {path.name}: compiled -> {out_bin.name}")
    return True, out_bin


def run_test(path: Path, bin_path: Path) -> bool:
    exp = EXPECT.get(path.name)

    run_res = run_cmd([str(bin_path)], ROOT)
    if run_res.returncode != 0:
        print(f"{RED}[FAIL]{RESET} {path.name}: runtime failed ({run_res.returncode})")
        return False

    out_lines = [line.strip() for line in run_res.stdout.splitlines() if line.strip()]
    if exp is not None and exp.run_output is not None:
        if out_lines != exp.run_output:
            print(f"{RED}[FAIL]{RESET} {path.name}: output mismatch")
            print(f"  expected: {exp.run_output}")
            print(f"  actual:   {out_lines}")
            return False

    print(f"{GREEN}[PASS]{RESET} {path.name}")
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