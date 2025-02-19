#include "main.h"
#include "shellcode.h"
#include <stdint.h>

int game_pid = 0;
uint64_t base_addr = 0;
uint64_t text_addr = 0;

bool is_root_user() { return getuid() == 0; }

typedef struct {
  uint64_t target_addr;
  uint32_t original_first_inst;
  uint64_t trampoline_addr;
  uint64_t trampoline_size;
} hook_func;

uint64_t trampoline_addr = 0;
uint64_t trampoline_size = 0x2c68f70 - 0x2c68178;

uint64_t symbol_size(uint8_t *func) {
  char pattern[] = "END_OF_SYMBOL";
  for (int i = 0;; i++) {
    if (memcmp(func + i, pattern, sizeof(pattern) - 1) == 0) {
      return i;
    }
  }
  return 0;
}

void inline_hook(uint64_t addr, uint8_t *func) {
  // 0. Determine the func size.
  uint64_t func_size = symbol_size(func);
  // Check if the trampoline can fit the func
  if (func_size > trampoline_size) {
    fprintf(stderr, "Error: func too large\n");
    return;
  }

  // Read the first instruction of the target function
  uint8_t *first_inst = read_mem(addr, 4);

  // Check if first_inst is a unconditional branch, if so, skip
  if ((first_inst[3] >> 2) == 0b101) {
    printf("First instruction is a branch, skipping\n");
    return;
  }

  printf("func_size: %lx\n", func_size);

  // Write the func to the trampoline
  patch_mem(trampoline_addr, func, func_size);

  // Write the first instruction to the trampoline
  patch_mem(trampoline_addr + func_size, first_inst, 4);
  func_size += 4;

  // Write a branch back to the next inst of target function
  uint8_t jmp[4];

  asm_b(jmp, (addr + 4 - trampoline_addr - func_size) >> 2);
  patch_mem(trampoline_addr + func_size, jmp, 4);
  func_size += 4;

  // Write some nops as padding
  uint8_t nop[4] = {0x1f, 0x20, 0x03, 0xd5};
  for (int i = 0; i < 4; i++) {
    patch_mem(trampoline_addr + func_size, nop, 4);
    func_size += 4;
  }

  // Replace the first instruction of the target function with a branch to the
  // trampoline
  int64_t offset = ((int64_t)trampoline_addr) - ((int64_t)addr);
  printf("offset: %ld\n", offset);
  asm_b(jmp, offset >> 2);

  patch_mem(addr, jmp, 4);
  printf("Res:");
  quick_print(addr);

  trampoline_addr += func_size;
  trampoline_size -= func_size;

  printf("trampoline_addr: %lx\n", trampoline_addr);
  printf("trampoline_size: %lx\n", trampoline_size);
}

void on_exit() { printf("Exiting\n"); }

int main() {
  if (!is_root_user()) {
    fprintf(stderr, "Error: root privileges required\n");
    return 1;
  }
  // atexit(on_exit);

  game_pid = find_process(".heavenburnsred");
  if (game_pid == -1) {
    fprintf(stderr, "Error: process not found\n");
    return 1;
  }

  printf("Found process with PID %d\n", game_pid);

  if (!find_base()) {
    fprintf(stderr, "Error: base address not found\n");
    return 1;
  }
  printf("Base address: %lx\n", base_addr);

  uint64_t ApplyExpectedDamage = base_addr + 0x03709f9c;

  printf("ApplyExpectedDamage: %lx\n", ApplyExpectedDamage);
  quick_print(ApplyExpectedDamage + 0x114);

  uint64_t demofunc = text_addr + 0xbd456c + 0x1c;

  printf("demofunc: %lx\n", demofunc);
  quick_print(demofunc);

  uint64_t RuntimeInit = text_addr + 0x2be734;
  printf("RuntimeInit: %lx\n", RuntimeInit);
  quick_print(RuntimeInit);
  trampoline_addr = RuntimeInit;

  printf("Hooking ApplyExpectedDamage\n");
  inline_hook(ApplyExpectedDamage, (uint8_t *)mitigate_damage_to_player);

  printf("Hooking DemoFunc\n");
  inline_hook(demofunc, (uint8_t *)demo_patch);
}
