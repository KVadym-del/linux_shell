# linux_shell (TO BE RENAMED)

> [!WARNING]
> This project is still in development and not yet ready for production use.

*linux_shell (TO BE RENAMED)* is a tiny linux distribution.

## Features
- Currently there is no features at all. :(

## Getting Started
To correctly build the project, you need to have the following folder structure:
```
base_linux/
├── linux_shell
└── linux
```
```bash
cd linux
```
Then, you need o configure linux before building it:
* 64-bit kernel
* Enable ELF support
* Initial RAM filesystem and RAM disk (initramfs/initrd) support

After that, you can build the project with:
```bash
make -j8
```

After building linux kernel, you can build linux_shell:
```bash
cd ../linux_shell
LUA= VIM= make
```
*If you have any static build of lua or vim you can move them to the `linux_shell/bin` folder or put the paths into LUA= VIM=*

At the end you would be able to run the following command:
```bash
qemu-system-x86_64 -cdrom arch/x86/boot/image.iso
```
which will boot the linux.

## Buildin programs
* `ls` - list directory contents
* `mkdir` - make directories
* `rm` - remove files or directories
* `touch` - create empty files

## License
This project is licensed under the GNU GENERAL PUBLIC LICENSE - see the [LICENSE](License) file for details.