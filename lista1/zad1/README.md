# Task list 1


## Prerequisites

- meson build
- ninja build (might be pulled as dependency when installing meson)
- C++ compiler

##  Compilation

To keep repository clean, wrap dependencies need to be pulled manually

```bash
mkdir subprojects
meson wrap install asio
meson setup buildir
cd builddir
meson compile
# Run tests
meson test
```

## IDE/Editor support

meson generates compile_commands.json in builddir. If after running meson compile commands your environment shows include errors, then consider creating symlink in the top-level directory

```bash
ln -s builddir/compile_commands.json ./compile_commands.json
```

## Formatting
There's .clang-format included. If you have formatting on save enabled, and suddenly
all your files are rearanged, consider checking your environment setting. You should definitely be able to configure it to prioritize .clang-format files.

## For all the strugglers
There are several shortcuts that you can take the make your life easier and cut down on boiler plate.
### Marshaling
Lots of boiler plate there - In general you can make it simpler by converting entire structs to byte arrays (C-style unions might be useful here). This
is generally discouraged even for PODs (client & server might be on different platforms). If you find good & simple marshaling library, then go for it.
Purists might also consider using std::Tuple instead of struct to allow iteration over fields at the expense of readibility.
### Client/server
Merge connection code with filesystem and client code. Use OS sockets if you feel like it.
### Filesystem
Using vanilla OS syscalls might be a good idea, as they have less abstractions (at the expense of cross-platform compilation - doubt you care though). Be careful as type sizes may not be the same across OSes even within POSIX - but you will be fine as you can run client and server on the same system.
