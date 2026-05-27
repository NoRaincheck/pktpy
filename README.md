# pktpy — pocketpy packaged for PyPI

[pocketpy](https://github.com/pocketpy/pocketpy) is a portable Python 3.x
interpreter written in C11, designed as a drop-in replacement for Lua in
game scripting. This package wraps it for distribution on PyPI.

## Usage

```shell
# Run a script
uv run pktpy path/to/script.py

# Or via pip
pip install pktpy
pktpy path/to/script.py
```

## Example

```shell
$ uv run pktpy example/example.py
[2, 3, 5, 7, 11, 13, 17, 19]
```
```
