import subprocess
import sys
import tempfile
from pathlib import Path


def run_pktpy(*args):
    return subprocess.run(
        [sys.executable, "-m", "pktpy", *args],
        capture_output=True,
        text=True,
    )


def test_help():
    result = run_pktpy("--help")
    assert result.returncode == 0
    assert "usage:" in result.stdout


def test_help_short():
    result = run_pktpy("-h")
    assert result.returncode == 0
    assert "usage:" in result.stdout


def test_no_args():
    result = run_pktpy()
    assert result.returncode == 0
    assert "usage:" in result.stdout


def test_run_script():
    with tempfile.NamedTemporaryFile(mode="w", suffix=".py", delete=False) as f:
        f.write("print('hello from test')\n")
        f.flush()
        path = f.name

    try:
        result = run_pktpy(path)
        assert result.returncode == 0
        assert "hello from test" in result.stdout
    finally:
        Path(path).unlink(missing_ok=True)


def test_run_script_with_args():
    with tempfile.NamedTemporaryFile(mode="w", suffix=".py", delete=False) as f:
        f.write("import sys\nfor a in sys.argv:\n    print(a)\n")
        f.flush()
        path = f.name

    try:
        result = run_pktpy(path, "arg1", "arg2")
        assert result.returncode == 0
        assert "arg1" in result.stdout
        assert "arg2" in result.stdout
    finally:
        Path(path).unlink(missing_ok=True)


def test_file_not_found():
    result = run_pktpy("/nonexistent/script.py")
    assert result.returncode == 1
    assert "Error: cannot open file" in result.stderr


def test_syntax_error():
    with tempfile.NamedTemporaryFile(mode="w", suffix=".py", delete=False) as f:
        f.write("def foo(:\n    pass\n")
        f.flush()
        path = f.name

    try:
        result = run_pktpy(path)
        assert result.returncode == 1
    finally:
        Path(path).unlink(missing_ok=True)
