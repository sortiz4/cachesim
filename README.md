## Requirements
- GNU G++ 6.3.0
- GNU Make 4.1

## Assumptions
- Configuration and access files must be ASCII text files.

## Compilation
- `make` or `make debug` to compile a debug binary.
- `make release` to compile an optimized binary.
  - `libstdc++` will be statically linked.
- `make clean` to remove compiled binaries.

## Usage and Testing
The cache simulator requires a configuration and access file.

```
Usage: cachesim conf access
```

Three test cases - `t1`, `t2`, and `t3` - are provided along with their
expected output under `test` (output format is slightly different).

Additionally, two Python scripts, `groups` and `random` (under `test`), were
created to generate access files. `random` will generate a list of completely
random instructions and 32-bit addresses, while `groups` will simulate spatial
locality in groups of size 32. Both scripts use a fixed seed.

```
Usage: groups|random output
```

Where `output` is the desired location of the generated access file.
