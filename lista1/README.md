# Task list 1


## Prerequisites

- meson build
- ninja build (might be pulled as dependency when installing meson)
- C++ compiler

##  Compilation

To keep repository clean, wrap dependencies need to be pulled manually

```bash
meson wrap install asio
meson setup buildir
cd builddir
meson compile
```

## IDE/Editor support

meson generates compile_commands.json in builddir. after running meson compile commands your environment shows include errors, then consider creating symlink in the top-level directory

```bash
ln -s builddir/compile_commands.json ./compile_commands.json
```