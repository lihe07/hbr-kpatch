// Set damage and damageForLabel to 5 when the player is hit
.global mitigate_damage_to_player
mitigate_damage_to_player:
  // x0 = ptr_to_this
  // x1 = ptr_to_hitContent

  // We should only use scratch regs x2-x7
  ldr w2, [x0, #0x20] // w2 == 0 -> this is enemy | else this is player
  cbz w2, mitigate_damage_to_player_end

  // Set <damage>k__BackingField
  mov w3, #5
  str w3, [x1, #0x10]

  // Set <damageForLabel>k__BackingField
  str w3, [x1, #0x14]

  mitigate_damage_to_player_end:

.ascii "END_OF_SYMBOL"

// Multiply the damage to the enemy
// Divide the damage to the player
multiple_damage_boost:
  // x0 = ptr_to_this
  // x1 = ptr_to_hitContent
  ldr w2, [x0, #0x20] // w2 == 0 -> this is enemy | else this is player
  cbz w2, multiple_damage_boost_this_is_enemy

  multiple_damage_boost_this_is_player:

  // Get <damage>k__BackingField
  ldr w2, [x1, #0x10]
  lsr w2, w2, #2
  str w2, [x1, #0x10]

  // Get <damageForLabel>k__BackingField
  ldr w2, [x1, #0x14]
  lsr w2, w2, #2
  str w2, [x1, #0x14]

  b multiple_damage_boost_end

  .balign 4
  multiple_damage_boost_this_is_enemy:

  ldr w2, [x1, #0x10]
  lsl w2, w2, #2
  str w2, [x1, #0x10]

  ldr w2, [x1, #0x14]
  lsl w2, w2, #2
  str w2, [x1, #0x14]

  multiple_damage_boost_end:
.ascii "END_OF_SYMBOL"

.global demo_patch
demo_patch:
  nop
  nop
  nop
  nop

.ascii "END_OF_SYMBOL"
  
