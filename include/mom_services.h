#ifndef GUARD_MOM_SERVICES_H
#define GUARD_MOM_SERVICES_H

// Specials used by the Pokémon Center Mom script (data/scripts/pkmn_center_mom.inc).
// They work on the party Pokémon in VAR_0x8004 and set VAR_RESULT.
void MomCanModifyPartyMon(void);
void MomBufferHyperTraining(void);
void MomApplyHyperTraining(void);
void MomBufferAbilityChange(void);
void MomApplyAbilityChange(void);
void MomBufferNatureChange(void);
void MomApplyNatureChange(void);
void MomPayForService(void);

// Text of option `index` of the runtime-built scrollable multichoice lists (SCROLL_MULTI_MOM_*).
const u8 *MomGetScrollOptionText(u8 scrollId, u8 index);

#endif // GUARD_MOM_SERVICES_H
