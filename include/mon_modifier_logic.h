#ifndef GUARD_MON_MODIFIER_LOGIC_H
#define GUARD_MON_MODIFIER_LOGIC_H

// Pure rules for the Pokémon Center Mom "modifier" services (Hyper Training, Nature, Ability and
// Hidden Power type changes). Like level_up_logic.h this depends on nothing but the C language so it can
// be tested on the host (see tests/host). Game code calls these helpers instead of open-coding the rules.

// --- Stored overrides --------------------------------------------------------------------------
// The nature and Hidden Power type overrides are stored in small bit fields of the Pokémon's data.
// A stored value of 0 means "no override" (so Pokémon from existing saves have none); otherwise the
// stored value is (nature or type) + 1.
#define MODLOGIC_NO_OVERRIDE 0

unsigned char ModLogic_EncodeOverride(unsigned char value);
unsigned char ModLogic_HasOverride(unsigned char storedField);
unsigned char ModLogic_DecodeOverride(unsigned char storedField); // only meaningful if HasOverride

// Nature used for stat calculation: the override if there is one, otherwise the nature derived from
// the personality value. The Pokémon's real (personality) nature is never modified.
unsigned char ModLogic_StatNature(unsigned int personality, unsigned char storedNatureField, unsigned char numNatures);

// --- Hidden Power ------------------------------------------------------------------------------
// ivs order: HP, Attack, Defense, Speed, Sp. Attack, Sp. Defense.
// Returns the Hidden Power type: the override if there is one, otherwise the type derived from the IVs.
// numTypes is the number of types including ???; mysteryType is the ??? type id (skipped).
unsigned char ModLogic_HiddenPowerType(const unsigned char *ivs, unsigned char storedTypeField,
                                       unsigned char numTypes, unsigned char mysteryType);
unsigned char ModLogic_HiddenPowerTypeFromIVs(const unsigned char *ivs, unsigned char numTypes, unsigned char mysteryType);
unsigned char ModLogic_HiddenPowerPowerFromIVs(const unsigned char *ivs);

// --- Hyper Training ----------------------------------------------------------------------------
unsigned char ModLogic_CountIVsBelowMax(const unsigned char *ivs, unsigned char maxIV);
unsigned int ModLogic_HyperTrainCost(unsigned char statsToTrain, unsigned int pricePerStat);

// --- Ability -------------------------------------------------------------------------------------
// A species can switch abilities only if it has two different abilities (like the Ability Capsule).
unsigned char ModLogic_CanSwapAbility(unsigned char ability0, unsigned char ability1);
unsigned char ModLogic_OtherAbilitySlot(unsigned char slot);

#endif // GUARD_MON_MODIFIER_LOGIC_H
