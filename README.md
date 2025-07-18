# Text Editor

This is a text editor created in C using a tutorial referenced in:

- [Kilo Editor](https://viewsourcecode.org/snaptoken/kilo/)

## Compilation

```bash
cmake -S . -B build
cmake --build build
```

## Attach debugger

To attach a debugger to a running process:

```bash
sudo sysctl -w kernel.yama.ptrace_scope=0
```

## References

- For escape sequences refer to the [VT 100 Programmer Information manual](https://vt100.net/docs/vt100-ug/chapter3.html)
