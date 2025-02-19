#include <dirent.h>
#include <elf.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

// Global variables
extern int game_pid;
extern uint64_t base_addr;
extern uint64_t text_addr;

// asm.c
void asm_b(uint8_t *inst, int32_t imm26);
void asm_br_to_zero(uint8_t *inst);

// mem.c
void patch_mem(uint64_t addr, uint8_t *data, size_t size);
void *read_mem(uint64_t addr, size_t size);
uint64_t find_base();
void quick_print(uint64_t addr);
int find_process(char *process_name);
