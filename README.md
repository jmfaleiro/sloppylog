### Setup

```
git submodule init
git submodule update
mkdir build
cd build; cmake ..
make
```

If this error is encountered: `xcrun: error: invalid active developer path,` run this command:
```
xcode-select --install
```

#### Running Tests

```
cd build
test/test_lockmanager/<test_to_run>
```

#### Debugging

```
cd build
lldb test/test_lockmanager/<test_to_run>
```




