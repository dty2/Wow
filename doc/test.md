# Test

```txt
.
├── cases  # <- put ".test" file
│   ├── help.test
│   ├── list.test
│   ├── manual.test
│   ├── next.test
│   ├── previous.test
│   ├── refresh.test
│   ├── test.list
│   ├── unique-instance.test
│   └── version.test
├── CMakeLists.txt
├── fixtures
│   ├── config  # <- simulation dir $XDG_RUNTIME_DIR/wow
│   └── workdir  # <- simulation dir $XDG_CONFIG_HOME/wow
├── modify.sh
├── path.py.in
└── runtest.py  # <- regression script , run ".test" file
```

> [!NOTE]
> after adding a new ".test" file, run `cmake --build build` please

```bash
cd build

# run all test
ctest

# run single test
python3 ./test/runtest.py ../test/cases/[测试名].test
```
