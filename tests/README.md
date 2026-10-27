# Tests

## bmd_check_test

Checks `coop_bmd_file_ok` (src/bmd_check.cpp) against real model files: every `.bmd` under the
folder must pass, damaged copies of one must be rejected, and trailing padding must be accepted.

```
clang++ -std=c++20 -I src -I dusklight/sdk/include tests/bmd_check_test.cpp src/bmd_check.cpp -o bmd_test
./bmd_test models /tmp
```
