#include "global.h"
#include "event_data.h"
#include "money.h"
#include "mom_services.h"
#include "mon_modifier_logic.h"
#include "pokemon.h"
#include "string_util.h"
#include "battle_main.h"
#include "constants/abilities.h"
#include "constants/pkmn_center_mom.h"

// Party Pokémon the service is applied to: VAR_0x8004 (set by the ChoosePartyMon special).
#define SERVICE_MON (&gPlayerParty[gSpecialVar_0x8004])

static const u8 sText_AllIVs[] = _("IVs");
static const u8 sText_HpIV[] = _("HP IV");
static const u8 sText_AttackIV[] = _("ATTACK IV");
static const u8 sText_DefenseIV[] = _("DEFENSE IV");
static const u8 sText_SpAtkIV[] = _("SP. ATK IV");
static const u8 sText_SpDefIV[] = _("SP. DEF IV");
static const u8 sText_SpeedIV[] = _("SPEED IV");

static const u8 *const sHyperTrainingLabels[] =
{
    [MOM_HYPER_ALL]   = sText_AllIVs,
    [MOM_HYPER_HP]    = sText_HpIV,
    [MOM_HYPER_ATK]   = sText_AttackIV,
    [MOM_HYPER_DEF]   = sText_DefenseIV,
    [MOM_HYPER_SPATK] = sText_SpAtkIV,
    [MOM_HYPER_SPDEF] = sText_SpDefIV,
    [MOM_HYPER_SPEED] = sText_SpeedIV,
};

// MON_DATA_*_IV for each Hyper Training selection (index 0, "all", is unused).
static const u8 sHyperTrainingMonData[] =
{
    [MOM_HYPER_HP]    = MON_DATA_HP_IV,
    [MOM_HYPER_ATK]   = MON_DATA_ATK_IV,
    [MOM_HYPER_DEF]   = MON_DATA_DEF_IV,
    [MOM_HYPER_SPATK] = MON_DATA_SPATK_IV,
    [MOM_HYPER_SPDEF] = MON_DATA_SPDEF_IV,
    [MOM_HYPER_SPEED] = MON_DATA_SPEED_IV,
};

// VAR_RESULT = TRUE if the chosen Pokémon can be modified (not an Egg). Buffers its nickname in STR_VAR_1.
void MomCanModifyPartyMon(void)
{
    struct Pokemon *mon = SERVICE_MON;

    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    gSpecialVar_Result = !GetMonData(mon, MON_DATA_IS_EGG) && GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE;
}

static void GetMonIVs(struct Pokemon *mon, u8 *ivs)
{
    ivs[STAT_HP] = GetMonData(mon, MON_DATA_HP_IV);
    ivs[STAT_ATK] = GetMonData(mon, MON_DATA_ATK_IV);
    ivs[STAT_DEF] = GetMonData(mon, MON_DATA_DEF_IV);
    ivs[STAT_SPEED] = GetMonData(mon, MON_DATA_SPEED_IV);
    ivs[STAT_SPATK] = GetMonData(mon, MON_DATA_SPATK_IV);
    ivs[STAT_SPDEF] = GetMonData(mon, MON_DATA_SPDEF_IV);
}

// How many IVs a Hyper Training selection (VAR_0x8005) would raise.
static u8 CountIVsToTrain(struct Pokemon *mon, u8 selection)
{
    if (selection == MOM_HYPER_ALL)
    {
        u8 ivs[NUM_STATS];

        GetMonIVs(mon, ivs);
        return ModLogic_CountIVsBelowMax(ivs, MAX_PER_STAT_IVS);
    }
    return GetMonData(mon, sHyperTrainingMonData[selection]) < MAX_PER_STAT_IVS;
}

// Inputs: VAR_0x8004 = party slot, VAR_0x8005 = selection.
// Outputs: VAR_RESULT = VAR_0x8006 = total price (0 if there is nothing to train),
//          STR_VAR_1 = nickname, STR_VAR_2 = price, STR_VAR_3 = what is being trained.
void MomBufferHyperTraining(void)
{
    struct Pokemon *mon = SERVICE_MON;
    u32 cost = ModLogic_HyperTrainCost(CountIVsToTrain(mon, gSpecialVar_0x8005), MOM_SERVICE_PRICE);

    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, cost, STR_CONV_MODE_LEFT_ALIGN, 5);
    StringCopy(gStringVar3, sHyperTrainingLabels[gSpecialVar_0x8005]);
    gSpecialVar_0x8006 = cost;
    gSpecialVar_Result = cost;
}

// Raises the selected IV(s) to the maximum (31) and recalculates stats.
void MomApplyHyperTraining(void)
{
    struct Pokemon *mon = SERVICE_MON;
    u32 maxIV = MAX_PER_STAT_IVS;
    u8 i;

    if (gSpecialVar_0x8005 == MOM_HYPER_ALL)
    {
        for (i = MOM_HYPER_HP; i <= MOM_HYPER_SPEED; i++)
            SetMonData(mon, sHyperTrainingMonData[i], &maxIV);
    }
    else
    {
        SetMonData(mon, sHyperTrainingMonData[gSpecialVar_0x8005], &maxIV);
    }
    CalculateMonStats(mon);
}

// The ability slot (0 or 1) the Pokémon currently uses; falls back to slot 0 if its species has no ability in slot 1.
static u8 GetCurrentAbilitySlot(struct Pokemon *mon, u16 species)
{
    u8 slot = GetMonData(mon, MON_DATA_ABILITY_NUM) ? 1 : 0;

    if (gSpeciesInfo[species].abilities[slot] == ABILITY_NONE)
        slot = 0;
    return slot;
}

// Input: VAR_0x8004 = party slot. Output: VAR_RESULT = TRUE if the species has two different abilities to swap
// between (like the Ability Capsule). Buffers the price (STR_VAR_1), current (STR_VAR_2) and new (STR_VAR_3)
// ability names, and sets VAR_0x8006 to the price.
void MomBufferAbilityChange(void)
{
    struct Pokemon *mon = SERVICE_MON;
    u16 species = GetMonData(mon, MON_DATA_SPECIES);
    u8 slot = GetCurrentAbilitySlot(mon, species);
    u8 otherSlot = ModLogic_OtherAbilitySlot(slot);

    ConvertIntToDecimalStringN(gStringVar1, MOM_SERVICE_PRICE, STR_CONV_MODE_LEFT_ALIGN, 5);
    StringCopy(gStringVar2, gAbilityNames[gSpeciesInfo[species].abilities[slot]]);
    StringCopy(gStringVar3, gAbilityNames[gSpeciesInfo[species].abilities[otherSlot]]);
    gSpecialVar_0x8006 = MOM_SERVICE_PRICE;
    gSpecialVar_Result = ModLogic_CanSwapAbility(gSpeciesInfo[species].abilities[0], gSpeciesInfo[species].abilities[1]);
}

// Switches the Pokémon to its other ability.
void MomApplyAbilityChange(void)
{
    struct Pokemon *mon = SERVICE_MON;
    u8 newSlot = ModLogic_OtherAbilitySlot(GetCurrentAbilitySlot(mon, GetMonData(mon, MON_DATA_SPECIES)));

    SetMonData(mon, MON_DATA_ABILITY_NUM, &newSlot);
}

// Charges VAR_0x8006 money. VAR_RESULT = TRUE if the player could pay (and was charged).
void MomPayForService(void)
{
    u32 cost = gSpecialVar_0x8006;

    if (IsEnoughMoney(&gSaveBlock1Ptr->money, cost))
    {
        RemoveMoney(&gSaveBlock1Ptr->money, cost);
        gSpecialVar_Result = TRUE;
    }
    else
    {
        gSpecialVar_Result = FALSE;
    }
}
