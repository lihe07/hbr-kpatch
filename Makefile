export NDK="$(HOME)/Android/Sdk/ndk/27.0.12077973/"

API=30

CC=$(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android$(API)-clang

kpatch: main.c asm.c mem.c shellcode.s 
	$(CC) -o kpatch shellcode.s main.c asm.c mem.c

test: kpatch
	adb push kpatch /data/local/tmp
	adb shell su -c /data/local/tmp/kpatch

