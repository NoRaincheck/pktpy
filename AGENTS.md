## Commands

- Build: `uv sync --group dev`
- Test: `uv run pytest`
- Lint: `uv run ruff check`
- Format: `uv run ruff format`
- Build wheel: `uv build`

## Conventions

- C extension in `src/_pocketpy_module.c`, Python package in `pktpy/`
- Tests go in `tests/` using pytest
- pocketpy C sources live in `third_party/pocketpy/` (git submodule)
