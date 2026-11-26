# C++ code style

Both HydraLegacy and RaftPirates use the same `.clang-format` configuration (LLVM-based, four-space indentation, 100-column target, attached braces, and no compact nonempty function bodies). Keep public API names and wire keys intact, even when they differ from modern naming conventions.

Install clang-format 17 (a standalone formatter, not an IDE). On Windows, `python -m pip install clang-format==17.0.6` is one lightweight option. From the repository root:

```sh
python tools/format.py
python tools/format.py --check
```

The first command formats C/C++ headers, implementations, tests, examples, and generated callback includes; the second checks without modifying files. You can also point the script at an installed formatter with `--clang-format /path/to/clang-format`. Format generated JNI callback includes after regenerating them. Formatting is not a build dependency.

Use descriptive local names when introducing code, keep functions focused, and avoid combining unrelated statements on a line. Add tests for changes to behavior. Prefer comments that describe recovered behavior or implementation assumptions rather than restating code. Avoid wholesale renames of recovered symbols.
