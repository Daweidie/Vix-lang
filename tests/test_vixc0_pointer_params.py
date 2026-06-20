import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parent.parent
VIXC0 = ROOT / "bootstrap" / "vixc0"


@pytest.fixture(scope="session")
def vixc0_binary():
    if shutil.which("vixc") is None:
        pytest.skip("vixc is required to build vixc0")
    result = subprocess.run(
        ["make", "-C", str(ROOT / "bootstrap"), "all"],
        capture_output=True,
        text=True,
        timeout=60,
    )
    assert result.returncode == 0, result.stdout + result.stderr
    assert VIXC0.exists()
    return VIXC0


def test_vixc0_accepts_pointer_function_params(vixc0_binary, tmp_path):
    if shutil.which("opt") is None:
        pytest.skip("opt is required to verify vixc0 LLVM IR")
    if shutil.which("lli") is None:
        pytest.skip("lli is required to execute vixc0 LLVM IR")

    ir = tmp_path / "swap.ll"
    source = ROOT / "examples" / "swap.vix"

    compile_result = subprocess.run(
        [str(vixc0_binary), str(source)],
        capture_output=True,
        text=True,
        timeout=20,
    )
    assert compile_result.returncode == 0, compile_result.stdout + compile_result.stderr
    ir.write_text(compile_result.stdout)

    verify_result = subprocess.run(
        ["opt", "-passes=verify", "-disable-output", str(ir)],
        capture_output=True,
        text=True,
        timeout=20,
    )
    assert verify_result.returncode == 0, verify_result.stdout + verify_result.stderr

    run_result = subprocess.run(
        ["lli", str(ir)],
        capture_output=True,
        text=True,
        timeout=20,
    )
    assert run_result.returncode == 0, run_result.stdout + run_result.stderr
    assert run_result.stdout == "20\n10\n"
