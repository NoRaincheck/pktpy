import sys

import pktpy


def main():
    argv = sys.argv[1:]

    if not argv or argv[0] in ("-h", "--help"):
        print("usage: pktpy [--profile] [--debug] [--compile] <filename> [args...]")
        print()
        print("A portable Python 3.x interpreter (pocketpy).")
        sys.exit(0)

    positional = []

    for a in argv:
        if a.startswith("--"):
            continue
        positional.append(a)

    if not positional:
        print(
            "usage: pktpy [--profile] [--debug] [--compile] <filename> [args...]",
            file=sys.stderr,
        )
        sys.exit(1)

    filename = positional[0]

    pktpy.initialize()

    try:
        pktpy.run_file(filename, argv=positional)
    except FileNotFoundError:
        print(f"Error: cannot open file '{filename}'", file=sys.stderr)
        pktpy.finalize()
        sys.exit(1)
    except RuntimeError:
        pktpy.finalize()
        sys.exit(1)

    pktpy.finalize()


if __name__ == "__main__":
    main()
