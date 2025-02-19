// mitigate_damage_to_player
// This is applied at the beginning of ApplyExpectedHitContent
.global mitigate_damage_to_player
mitigate_damage_to_player:
  // x0 = ptr_to_this
  // x1 = ptr_to_hitContent

  // We should only use scratch regs x2-x7

  ldr w2, [x0, #0x20] // w2 == 0 -> this is enemy | else this is player
  cbz w2, mitigate_damage_to_player_end

  // Get <damage>k__BackingField
  ldr w2, [x1, #0x10]

  // w2 = min(w2, w3)
  mov w3, #5
  str w3, [x1, #0x10]

  // Get <damageForLabel>k__BackingField
  ldr w2, [x1, #0x14]
  str w3, [x1, #0x14]

  mitigate_damage_to_player_end:

.ascii "END_OF_SYMBOL"

.global demo_patch
demo_patch:
  nop
  nop
  nop
  nop

.ascii "END_OF_SYMBOL"
  
