#include "mon_modifier_logic.h"

unsigned char ModLogic_EncodeOverride(unsigned char value)
{
    return value + 1;
}

unsigned char ModLogic_HasOverride(unsigned char storedField)
{
    return storedField != MODLOGIC_NO_OVERRIDE;
}

unsigned char ModLogic_DecodeOverride(unsigned char storedField)
{
    return storedField - 1;
}

unsigned char ModLogic_StatNature(unsigned int personality, unsigned char storedNatureField, unsigned char numNatures)
{
    if (ModLogic_HasOverride(storedNatureField))
        return ModLogic_DecodeOverride(storedNatureField);
    return personality % numNatures;
}

unsigned char ModLogic_HiddenPowerTypeFromIVs(const unsigned char *ivs, unsigned char numTypes, unsigned char mysteryType)
{
    unsigned char typeBits = ((ivs[0] & 1) << 0)
                           | ((ivs[1] & 1) << 1)
                           | ((ivs[2] & 1) << 2)
                           | ((ivs[3] & 1) << 3)
                           | ((ivs[4] & 1) << 4)
                           | ((ivs[5] & 1) << 5);
    // Subtract 3 because Normal and ??? are excluded; the + 1 skips past Normal, the check skips ???.
    unsigned char type = ((numTypes - 3) * typeBits) / 63 + 1;

    if (type >= mysteryType)
        type++;
    return type;
}

unsigned char ModLogic_HiddenPowerPowerFromIVs(const unsigned char *ivs)
{
    unsigned char powerBits = ((ivs[0] & 2) >> 1)
                            | ((ivs[1] & 2) << 0)
                            | ((ivs[2] & 2) << 1)
                            | ((ivs[3] & 2) << 2)
                            | ((ivs[4] & 2) << 3)
                            | ((ivs[5] & 2) << 4);

    return ((MODLOGIC_HP_MAX_POWER - MODLOGIC_HP_MIN_POWER) * powerBits) / 63 + MODLOGIC_HP_MIN_POWER;
}

unsigned char ModLogic_HiddenPowerType(const unsigned char *ivs, unsigned char storedTypeField,
                                       unsigned char numTypes, unsigned char mysteryType)
{
    if (ModLogic_HasOverride(storedTypeField))
        return ModLogic_DecodeOverride(storedTypeField);
    return ModLogic_HiddenPowerTypeFromIVs(ivs, numTypes, mysteryType);
}

unsigned char ModLogic_CountIVsBelowMax(const unsigned char *ivs, unsigned char maxIV)
{
    unsigned char i, count = 0;

    for (i = 0; i < 6; i++)
        if (ivs[i] < maxIV)
            count++;
    return count;
}

unsigned int ModLogic_HyperTrainCost(unsigned char statsToTrain, unsigned int pricePerStat)
{
    return statsToTrain * pricePerStat;
}

unsigned char ModLogic_CanSwapAbility(unsigned char ability0, unsigned char ability1)
{
    return ability1 != 0 && ability1 != ability0;
}

unsigned char ModLogic_OtherAbilitySlot(unsigned char slot)
{
    return slot ? 0 : 1;
}

unsigned char ModLogic_MomDialogueStage(unsigned char numBadges, unsigned char beatDad,
                                        unsigned short sootopolisState, unsigned char champion)
{
    if (champion)
        return MODLOGIC_MOM_STAGE_CHAMPION;
    if (numBadges >= 8)
        return MODLOGIC_MOM_STAGE_ALL_BADGES;
    if (sootopolisState >= 1 && sootopolisState <= 5)
        return MODLOGIC_MOM_STAGE_CRISIS;
    if (beatDad)
        return MODLOGIC_MOM_STAGE_BEAT_DAD;
    if (numBadges > 0)
        return MODLOGIC_MOM_STAGE_BADGES;
    return MODLOGIC_MOM_STAGE_FRESH;
}

static unsigned char AllAtLeast(const unsigned char *values, unsigned char max)
{
    unsigned char i;

    for (i = 0; i < 6; i++)
    {
        if (values[i] < max)
            return 0;
    }
    return 1;
}

unsigned char ModLogic_MomServiceIsUseless(unsigned char service, unsigned char isEgg,
                                           const unsigned char *ivs, unsigned char maxIV,
                                           const unsigned char *evs, unsigned char maxEV,
                                           unsigned char ability0, unsigned char ability1)
{
    if (isEgg)
        return 1;
    switch (service)
    {
    case MODLOGIC_MOM_SERVICE_HYPER_TRAINING:
        return AllAtLeast(ivs, maxIV);
    case MODLOGIC_MOM_SERVICE_MAX_EVS:
        return AllAtLeast(evs, maxEV);
    case MODLOGIC_MOM_SERVICE_ABILITY:
        return !ModLogic_CanSwapAbility(ability0, ability1);
    default:
        return 0;
    }
}
