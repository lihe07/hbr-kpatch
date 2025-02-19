#include "main.h"

// PC + (imm26 << 2)
void asm_b(uint8_t *inst, int32_t imm26) {
  // Ensure imm26 is within the valid range for a branch instruction
  // Range is -2^25 to 2^25 - 4 (-33554432 to 33554428)
  if (imm26 < -33554432 || imm26 > 33554428) {
    fprintf(stderr, "Error: imm26 out of range\n");
    abort();
  }

  // The B instruction format:
  // 31-26: 000101 (opcode for B)
  // 25-0:  imm26 (26-bit immediate)

  // First, mask imm26 to ensure we only use the lower 26 bits
  uint32_t imm = (uint32_t)(imm26 & 0x3FFFFFF);

  // Construct the instruction:
  // Set opcode (000101) in bits 31-26
  // Set immediate in bits 25-0
  uint32_t instruction = 0x14000000 | imm;

  // Write the instruction to the provided buffer
  memcpy(inst, &instruction, sizeof(instruction));

  printf("B: ");
  for (int i = 0; i < 4; i++) {
    printf("%02x", inst[i]);
  }
  printf("\n");
}

void asm_br_to_zero(uint8_t *inst) {
  // Construct the instruction encoding
  // e0031fd6
  // return 0xd61f03e0;
  inst[0] = 0xe0;
  inst[1] = 0x03;
  inst[2] = 0x1f;
  inst[3] = 0xd6;
}
