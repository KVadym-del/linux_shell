# linux_shell (TO BE RENAMED)

> [!WARNING]
> This project is still in development and not yet ready for production use.

*linux_shell (TO BE RENAMED)* is a tiny linux distribution.

## Current Status / Features
- Minimal interactive shell (built into `init`) with command history and basic line editing.
- Statically linked toy implementations of several classic Unix utilities.
- Simple text editor (`edit`) with:
  - Raw mode terminal handling
  - Basic cursor movement (arrows)
  - Insert / backspace / newline
  - Save (Ctrl-S) & Quit (Ctrl-Q)
  - Dirty indicator `*`
- Colorized `ls` (directories in blue).
- Reusable utility helpers (argument parsing, error printing, RAII FDs / DIR, full-buffer write).

## Planned / Ideas
- Command completion
- Enhanced editor (scrolling, paging, search)
- Optional Lua / Vim integration when provided statically
- Configuration & tests

## Directory Layout Expectation
```
base_linux/
├── linux            (Linux kernel source tree)
└── linux_shell      (this repository)
```
Work from inside `linux` first to configure & build the kernel, then build `linux_shell`.

## Kernel Configuration (required options)
Ensure at least:
- 64-bit kernel
- ELF binary format support
- Initial RAM filesystem / initrd support (CONFIG_BLK_DEV_INITRD=y, CONFIG_INITRAMFS_SOURCE or similar)

Then build the kernel:
```bash
cd linux
make -j$(nproc)
```

## Building linux_shell
From the sibling directory:
```bash
cd ../linux_shell
LUA= VIM= make
```
*If you have any static build of lua or vim you can move them to the `linux_shell/bin` folder or put the paths into LUA= VIM=*

At the end you would be able to run the following command:
```bash
qemu-system-x86_64 -cdrom arch/x86/boot/image.iso
```
This should boot into the minimal environment and drop you into the shell (via `init`).

## Built-in Programs
| Program | Description                                                     |
| ------- | --------------------------------------------------------------- |
| `init`  | Entry point; sets up and launches the minimal interactive shell |
| `ls`    | List directory contents (directories in blue)                   |
| `mkdir` | Create a directory (mode 0755)                                  |
| `rm`    | Remove files (no verbose success output)                        |
| `touch` | Create or truncate files                                        |
| `cat`   | Concatenate files to standard output (supports `-` for stdin)   |
| `edit`  | Simple in-terminal text editor (Ctrl-S save, Ctrl-Q quit)       |

Location: all binaries live in `bin/` after `make`.

## Contributing / Development Notes
- Code aims to avoid iostream and use raw syscalls + small helpers.
- Utilities share common logic in `src/include/util.hpp` (argument helpers, error formatting, RAII wrappers).
- Static linking flags are tuned for size; adjust `Makefile` if debugging (e.g. remove `-O3`, add `-g`).
- Patches improving safety, clarity, or reducing duplication are welcome.

## License
GNU GENERAL PUBLIC LICENSE — see [LICENSE](License) for details.