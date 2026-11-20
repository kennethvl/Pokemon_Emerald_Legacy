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
// Hidden Power's power comes from the IVs and ranges from MODLOGIC_HP_MIN_POWER to MODLOGIC_HP_MAX_POWER.
#define MODLOGIC_HP_MIN_POWER 80
#define MODLOGIC_HP_MAX_POWER 120
unsigned char ModLogic_HiddenPowerPowerFromIVs(const unsigned char *ivs);

// --- Hyper Training ----------------------------------------------------------------------------
unsigned char ModLogic_CountIVsBelowMax(const unsigned char *ivs, unsigned char maxIV);
unsigned int ModLogic_HyperTrainCost(unsigned char statsToTrain, unsigned int pricePerStat);

// --- Ability -------------------------------------------------------------------------------------
// A species can switch abilities only if it has two different abilities (like the Ability Capsule).
unsigned char ModLogic_CanSwapAbility(unsigned char ability0, unsigned char ability1);
unsigned char ModLogic_OtherAbilitySlot(unsigned char slot);

// --- Mom's services in the PC -------------------------------------------------------------------
// Service ids, in the order of Mom's menu (data/scripts/pkmn_center_mom.inc passes the menu index).
#define MODLOGIC_MOM_SERVICE_HYPER_TRAINING 0
#define MODLOGIC_MOM_SERVICE_NATURE         1
#define MODLOGIC_MOM_SERVICE_ABILITY        2
#define MODLOGIC_MOM_SERVICE_HIDDEN_POWER   3
#define MODLOGIC_MOM_SERVICE_MAX_EVS        4
#define MODLOGIC_MOM_SERVICE_RELEARN_MOVES  5
#define MODLOGIC_MOM_NUM_SERVICES           6

// TRUE if the service can't do anything for this Pokémon, so the PC dims its icon and refuses it:
// Eggs for every service, all IVs at maxIV (Hyper Training), all EVs at maxEV (Max EVs), or only one
// ability (Ability). ivs/evs order: HP, Attack, Defense, Speed, Sp. Attack, Sp. Defense.
unsigned char ModLogic_MomServiceIsUseless(unsigned char service, unsigned char isEgg,
                                           const unsigned char *ivs, unsigned char maxIV,
                                           const unsigned char *evs, unsigned char maxEV,
                                           unsigned char ability0, unsigned char ability1);

// --- Mom's dialogue -----------------------------------------------------------------------------
// Story stage that picks Mom's greeting and goodbye. Later stages win.
#define MODLOGIC_MOM_STAGE_FRESH       0 // no badges yet
#define MODLOGIC_MOM_STAGE_BADGES      1 // 1+ badges
#define MODLOGIC_MOM_STAGE_BEAT_DAD    2 // beat Norman at the Petalburg Gym
#define MODLOGIC_MOM_STAGE_CRISIS      3 // Groudon/Kyogre crisis (VAR_SOOTOPOLIS_CITY_STATE 1-5)
#define MODLOGIC_MOM_STAGE_ALL_BADGES  4 // all 8 badges
#define MODLOGIC_MOM_STAGE_CHAMPION    5 // entered the Hall of Fame
#define MODLOGIC_MOM_NUM_STAGES        6
unsigned char ModLogic_MomDialogueStage(unsigned char numBadges, unsigned char beatDad,
                                        unsigned short sootopolisState, unsigned char champion);

#endif // GUARD_MON_MODIFIER_LOGIC_H
