// Set damage and damageForLabel to 5 when the player is hit
.balign 4
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
.balign 4
.global multiple_damage_boost
multiple_damage_boost:
  // x0 = ptr_to_this
  // x1 = ptr_to_hitContent

  ldrb w2, [x1, #0x60] // isHeal
  // Jump to end if is heal
  cbnz w2, multiple_damage_boost_end
  
  ldr w2, [x0, #0x20] // w2 == 0 -> this is enemy | else this is player
  cbz w2, multiple_damage_boost_this_is_enemy

  multiple_damage_boost_this_is_player:

  // Get <damage>k__BackingField
  ldr w2, [x1, #0x10]
  lsr w2, w2, #7
  str w2, [x1, #0x10]

  // Get <damageForLabel>k__BackingField
  ldr w2, [x1, #0x14]
  lsr w2, w2, #7
  str w2, [x1, #0x14]

  // No critical hit, no poison
  strb wzr, [x1, #0x90] // <isPoison>k__BackingField
  strb wzr, [x1, #0x91] // <isCriticalHit>k__BackingField

  // No spDamage, no epDamage
  str wzr, [x1, #0x1c] // <spDamage>k__BackingField
  str wzr, [x1, #0x20] // <spDamageForLabel>k__BackingField
  str wzr, [x1, #0x28] // <epDamage>k__BackingField
  str wzr, [x1, #0x2c] // <epDamageForLabel>k__BackingField

  b multiple_damage_boost_end

  multiple_damage_boost_this_is_enemy:

  ldr w2, [x1, #0x10]
  add w2, w2, #100
  lsl w2, w2, #5
  str w2, [x1, #0x10]

  ldr w2, [x1, #0x14]
  add w2, w2, #100
  lsl w2, w2, #5
  str w2, [x1, #0x14]
  
  // Also boost the spDamage and epDamage
  ldr w2, [x1, #0x1c] //
  lsl w2, w2, #4
  str w2, [x1, #0x1c]
  ldr w2, [x1, #0x20]
  lsl w2, w2, #4
  str w2, [x1, #0x20]
  ldr w2, [x1, #0x28]
  lsl w2, w2, #4
  str w2, [x1, #0x28]
  ldr w2, [x1, #0x2c]
  lsl w2, w2, #4
  str w2, [x1, #0x2c]

  // I will never miss
  strb wzr, [x1, #0x92] // <isMiss>k__BackingField
  strb wzr, [x1, #0x93] // <isResist>k__BackingField
  strb wzr, [x1, #0x96] // <isDodge>k__BackingField

  // I will make critical hit
  mov w2, #1
  strb w2, [x1, #0x94] // <isCriticalHit>k__BackingField

  multiple_damage_boost_end:
.ascii "END_OF_SYMBOL"

.balign 4
.global demo_patch
demo_patch:
  nop
  nop
  nop
  nop

.ascii "END_OF_SYMBOL"
  
