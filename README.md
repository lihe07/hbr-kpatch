## KPatch

Kernel level hooking and patching for [a certain game](https://tw.heaven-burns-red.com/).

Requires root privileges.

### Mechanism

0. Launch as an individual process.

1. Find libil2cpp.so by checking SONAME of regions in `/proc/self/maps`.

2. Locate the base address of libil2cpp.so.

3. Prepare a trampoline region in `Runtime::Init` (which is only called once).

4. Inline hook related functions.

5. Clean up and terminate.

All memory operations are done through SysFS. No ptrace used.

Technically, this is impossible to be detected by normal means. Hide your root status and you're good to go.
