import pktpy
import pytest


def test_eval_int():
    assert pktpy.eval("1 + 2") == 3
    assert pktpy.eval("2 ** 10") == 1024
    assert pktpy.eval("-5") == -5


def test_eval_float():
    assert pktpy.eval("3.14") == 3.14
    assert pktpy.eval("2.0 * 3.0") == 6.0


def test_eval_str():
    assert pktpy.eval("'hello'") == "hello"
    assert pktpy.eval("'a' + 'b'") == "ab"


def test_eval_bool():
    assert pktpy.eval("True") is True
    assert pktpy.eval("False") is False
    assert pktpy.eval("1 == 1") is True
    assert pktpy.eval("1 == 2") is False


def test_eval_none():
    assert pktpy.eval("None") is None


def test_eval_list():
    result = pktpy.eval("[1, 2, 3]")
    assert result == "[1, 2, 3]"


def test_eval_dict():
    result = pktpy.eval("{'a': 1}")
    assert result == "{'a': 1}"


def test_exec_simple():
    pktpy.exec("x = 42")
    assert pktpy.eval("x") == 42


def test_exec_print(capsys):
    pktpy.exec("print('hello world')")
    captured = capsys.readouterr()
    assert "hello world" in captured.out


def test_run_file(tmp_path):
    script = tmp_path / "test.py"
    script.write_text("result = 1 + 2\nprint('ok')")
    pktpy.run_file(str(script))


def test_run_file_with_argv(tmp_path):
    script = tmp_path / "test_argv.py"
    script.write_text("import sys\nprint(sys.argv)")
    pktpy.run_file(str(script), argv=["arg1", "arg2"])


def test_run_file_not_found():
    with pytest.raises(FileNotFoundError):
        pktpy.run_file("/nonexistent/script.py")


def test_exec_syntax_error():
    with pytest.raises(RuntimeError):
        pktpy.exec("def foo(:\n    pass")


def test_builtins():
    assert pktpy.eval("len([1, 2, 3])") == 3
    assert pktpy.eval("list(range(3))") == "[0, 1, 2]"
    assert pktpy.eval("sum([1, 2, 3])") == 6


def test_math_module():
    assert pktpy.eval("__import__('math').sqrt(16)") == 4.0
    assert pktpy.eval("__import__('math').pi") == pytest.approx(3.141592653589793)


def test_json_module():
    result = pktpy.eval("__import__('json').dumps({'a': 1})")
    assert result == '{"a": 1}'
