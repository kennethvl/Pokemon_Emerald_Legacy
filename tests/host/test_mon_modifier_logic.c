#include <stdio.h>
#include "mon_modifier_logic.h"

static int sFailures;
static int sChecks;

#define CHECK_EQ(a, b) \
    do { \
        long long va_ = (long long)(a), vb_ = (long long)(b); \
        sChecks++; \
        if (va_ != vb_) { \
            sFailures++; \
            printf("FAIL %s:%d: %s == %s (%lld vs %lld)\n", __FILE__, __LINE__, #a, #b, va_, vb_); \
        } \
    } while (0)

#define NUM_NATURES 25
#define NUM_TYPES 18
#define TYPE_MYSTERY 9
enum { T_NORMAL, T_FIGHTING, T_FLYING, T_POISON, T_GROUND, T_ROCK, T_BUG, T_GHOST, T_STEEL, T_MYSTERY,
       T_FIRE, T_WATER, T_GRASS, T_ELECTRIC, T_PSYCHIC, T_ICE, T_DRAGON, T_DARK };

static void TestOverrideEncoding(void)
{
    unsigned char v;

    CHECK_EQ(MODLOGIC_NO_OVERRIDE, 0);
    CHECK_EQ(ModLogic_HasOverride(0), 0);
    // Value 0 (Hardy / Normal) must still be storable and distinguishable from "no override".
    CHECK_EQ(ModLogic_EncodeOverride(0), 1);
    CHECK_EQ(ModLogic_HasOverride(ModLogic_EncodeOverride(0)), 1);
    for (v = 0; v < 24; v++)
    {
        CHECK_EQ(ModLogic_DecodeOverride(ModLogic_EncodeOverride(v)), v);
        CHECK_EQ(ModLogic_EncodeOverride(v) < 32, 1); // fits the 5-bit stored field
    }
    CHECK_EQ(ModLogic_EncodeOverride(NUM_NATURES - 1) < 32, 1);
    CHECK_EQ(ModLogic_EncodeOverride(NUM_TYPES - 1) < 32, 1);
}

static void TestStatNature(void)
{
    // No override: derived from the personality value.
    CHECK_EQ(ModLogic_StatNature(0, 0, NUM_NATURES), 0);
    CHECK_EQ(ModLogic_StatNature(24, 0, NUM_NATURES), 24);
    CHECK_EQ(ModLogic_StatNature(25 * 1000 + 7, 0, NUM_NATURES), 7);
    // Override wins, whatever the personality is.
    CHECK_EQ(ModLogic_StatNature(7, ModLogic_EncodeOverride(12), NUM_NATURES), 12);
    CHECK_EQ(ModLogic_StatNature(7, ModLogic_EncodeOverride(0), NUM_NATURES), 0);
    CHECK_EQ(ModLogic_StatNature(7, ModLogic_EncodeOverride(24), NUM_NATURES), 24);
}

static void TestHiddenPower(void)
{
    const unsigned char all31[6]   = {31, 31, 31, 31, 31, 31};
    const unsigned char all30[6]   = {30, 30, 30, 30, 30, 30};
    const unsigned char fighting[6]= {31, 31, 30, 30, 30, 30};
    const unsigned char bug[6]     = {31, 31, 31, 30, 31, 30};
    const unsigned char fire[6]    = {31, 30, 31, 30, 30, 31};
    const unsigned char dragon[6]  = {30, 31, 31, 31, 31, 31};
    unsigned char t;

    // Natural types match the well-known IV spreads.
    CHECK_EQ(ModLogic_HiddenPowerTypeFromIVs(all31, NUM_TYPES, TYPE_MYSTERY), T_DARK);
    CHECK_EQ(ModLogic_HiddenPowerTypeFromIVs(all30, NUM_TYPES, TYPE_MYSTERY), T_FIGHTING);
    CHECK_EQ(ModLogic_HiddenPowerTypeFromIVs(fighting, NUM_TYPES, TYPE_MYSTERY), T_FIGHTING);
    CHECK_EQ(ModLogic_HiddenPowerTypeFromIVs(bug, NUM_TYPES, TYPE_MYSTERY), T_BUG);
    CHECK_EQ(ModLogic_HiddenPowerTypeFromIVs(fire, NUM_TYPES, TYPE_MYSTERY), T_FIRE);
    CHECK_EQ(ModLogic_HiddenPowerTypeFromIVs(dragon, NUM_TYPES, TYPE_MYSTERY), T_DRAGON);

    // Never Normal and never ???.
    {
        unsigned int bits;
        for (bits = 0; bits < 64; bits++)
        {
            unsigned char ivs[6];
            int i;
            for (i = 0; i < 6; i++)
                ivs[i] = (bits >> i) & 1;
            t = ModLogic_HiddenPowerTypeFromIVs(ivs, NUM_TYPES, TYPE_MYSTERY);
            CHECK_EQ(t != T_NORMAL && t != T_MYSTERY && t < NUM_TYPES, 1);
        }
    }

    // Override bypasses the IVs entirely, including Normal (type 0).
    CHECK_EQ(ModLogic_HiddenPowerType(all31, ModLogic_EncodeOverride(T_WATER), NUM_TYPES, TYPE_MYSTERY), T_WATER);
    CHECK_EQ(ModLogic_HiddenPowerType(all31, ModLogic_EncodeOverride(T_NORMAL), NUM_TYPES, TYPE_MYSTERY), T_NORMAL);
    CHECK_EQ(ModLogic_HiddenPowerType(all31, 0, NUM_TYPES, TYPE_MYSTERY), T_DARK);

    // Power comes from IVs only and is unaffected by any type override.
    CHECK_EQ(ModLogic_HiddenPowerPowerFromIVs(all31), 120);
    CHECK_EQ(ModLogic_HiddenPowerPowerFromIVs(all30), 120);
    {
        const unsigned char zero[6] = {0, 0, 0, 0, 0, 0};
        unsigned int bits;
        unsigned char lowest = 255, highest = 0;

        CHECK_EQ(ModLogic_HiddenPowerPowerFromIVs(zero), 80);
        // Across every IV combination the power stays within 80-120 and both ends are reachable.
        for (bits = 0; bits < 64; bits++)
        {
            unsigned char ivs[6], power;
            int i;
            for (i = 0; i < 6; i++)
                ivs[i] = ((bits >> i) & 1) << 1; // only the second-lowest bit matters for power
            power = ModLogic_HiddenPowerPowerFromIVs(ivs);
            CHECK_EQ(power >= 80 && power <= 120, 1);
            if (power < lowest) lowest = power;
            if (power > highest) highest = power;
        }
        CHECK_EQ(lowest, 80);
        CHECK_EQ(highest, 120);
    }
}

static void TestHyperTraining(void)
{
    const unsigned char none[6]  = {31, 31, 31, 31, 31, 31};
    const unsigned char some[6]  = {31, 10, 31, 0, 30, 31};
    const unsigned char all[6]   = {0, 0, 0, 0, 0, 0};

    CHECK_EQ(ModLogic_CountIVsBelowMax(none, 31), 0);
    CHECK_EQ(ModLogic_CountIVsBelowMax(some, 31), 3);
    CHECK_EQ(ModLogic_CountIVsBelowMax(all, 31), 6);

    // Prices: 10 per stat, so training all six costs 60 and nothing to train costs nothing.
    CHECK_EQ(ModLogic_HyperTrainCost(0, 10), 0);
    CHECK_EQ(ModLogic_HyperTrainCost(1, 10), 10);
    CHECK_EQ(ModLogic_HyperTrainCost(6, 10), 60);
    CHECK_EQ(ModLogic_HyperTrainCost(ModLogic_CountIVsBelowMax(some, 31), 10), 30);
}

static void TestAbility(void)
{
    CHECK_EQ(ModLogic_CanSwapAbility(65, 0), 0);   // single ability
    CHECK_EQ(ModLogic_CanSwapAbility(65, 65), 0);  // both slots identical
    CHECK_EQ(ModLogic_CanSwapAbility(65, 34), 1);  // two different abilities
    CHECK_EQ(ModLogic_CanSwapAbility(0, 0), 0);
    CHECK_EQ(ModLogic_OtherAbilitySlot(0), 1);
    CHECK_EQ(ModLogic_OtherAbilitySlot(1), 0);
}

static void TestMomServiceIsUseless(void)
{
    const unsigned char perfect[6] = {31, 31, 31, 31, 31, 31};
    const unsigned char oneLow[6]  = {31, 31, 31, 30, 31, 31};
    const unsigned char maxEVs[6]  = {255, 255, 255, 255, 255, 255};
    const unsigned char someEVs[6] = {255, 255, 0, 255, 255, 255};

    // Hyper Training: useless only when every IV is maxed
    CHECK_EQ(ModLogic_MomServiceIsUseless(MODLOGIC_MOM_SERVICE_HYPER_TRAINING, 0, perfect, 31, someEVs, 255, 1, 2), 1);
    CHECK_EQ(ModLogic_MomServiceIsUseless(MODLOGIC_MOM_SERVICE_HYPER_TRAINING, 0, oneLow, 31, maxEVs, 255, 1, 2), 0);
    // Max EVs: useless only when every EV is maxed
    CHECK_EQ(ModLogic_MomServiceIsUseless(MODLOGIC_MOM_SERVICE_MAX_EVS, 0, oneLow, 31, maxEVs, 255, 1, 2), 1);
    CHECK_EQ(ModLogic_MomServiceIsUseless(MODLOGIC_MOM_SERVICE_MAX_EVS, 0, perfect, 31, someEVs, 255, 1, 2), 0);
    // Ability: useless with a single ability (or two identical ones)
    CHECK_EQ(ModLogic_MomServiceIsUseless(MODLOGIC_MOM_SERVICE_ABILITY, 0, oneLow, 31, someEVs, 255, 65, 0), 1);
    CHECK_EQ(ModLogic_MomServiceIsUseless(MODLOGIC_MOM_SERVICE_ABILITY, 0, oneLow, 31, someEVs, 255, 65, 65), 1);
    CHECK_EQ(ModLogic_MomServiceIsUseless(MODLOGIC_MOM_SERVICE_ABILITY, 0, perfect, 31, maxEVs, 255, 65, 34), 0);
    // Nature, Hidden Power and Relearn Moves are never dimmed by this rule
    CHECK_EQ(ModLogic_MomServiceIsUseless(MODLOGIC_MOM_SERVICE_NATURE, 0, perfect, 31, maxEVs, 255, 65, 0), 0);
    CHECK_EQ(ModLogic_MomServiceIsUseless(MODLOGIC_MOM_SERVICE_HIDDEN_POWER, 0, perfect, 31, maxEVs, 255, 65, 0), 0);
    CHECK_EQ(ModLogic_MomServiceIsUseless(MODLOGIC_MOM_SERVICE_RELEARN_MOVES, 0, perfect, 31, maxEVs, 255, 65, 0), 0);
    // Eggs: always useless
    CHECK_EQ(ModLogic_MomServiceIsUseless(MODLOGIC_MOM_SERVICE_NATURE, 1, oneLow, 31, someEVs, 255, 65, 34), 1);
    CHECK_EQ(ModLogic_MomServiceIsUseless(MODLOGIC_MOM_SERVICE_RELEARN_MOVES, 1, oneLow, 31, someEVs, 255, 65, 34), 1);
}

static void TestMomDialogueStage(void)
{
    CHECK_EQ(ModLogic_MomDialogueStage(0, 0, 0, 0), MODLOGIC_MOM_STAGE_FRESH);
    CHECK_EQ(ModLogic_MomDialogueStage(1, 0, 0, 0), MODLOGIC_MOM_STAGE_BADGES);
    CHECK_EQ(ModLogic_MomDialogueStage(4, 0, 0, 0), MODLOGIC_MOM_STAGE_BADGES);
    CHECK_EQ(ModLogic_MomDialogueStage(5, 1, 0, 0), MODLOGIC_MOM_STAGE_BEAT_DAD);
    CHECK_EQ(ModLogic_MomDialogueStage(6, 1, 1, 0), MODLOGIC_MOM_STAGE_CRISIS);
    CHECK_EQ(ModLogic_MomDialogueStage(6, 1, 5, 0), MODLOGIC_MOM_STAGE_CRISIS);
    CHECK_EQ(ModLogic_MomDialogueStage(7, 1, 7, 0), MODLOGIC_MOM_STAGE_BEAT_DAD); // crisis over
    CHECK_EQ(ModLogic_MomDialogueStage(8, 1, 7, 0), MODLOGIC_MOM_STAGE_ALL_BADGES);
    CHECK_EQ(ModLogic_MomDialogueStage(8, 1, 7, 1), MODLOGIC_MOM_STAGE_CHAMPION);
    CHECK_EQ(ModLogic_MomDialogueStage(0, 0, 0, 1), MODLOGIC_MOM_STAGE_CHAMPION);
}

int main(void)
{
    TestMomDialogueStage();
    TestMomServiceIsUseless();
    TestOverrideEncoding();
    TestStatNature();
    TestHiddenPower();
    TestHyperTraining();
    TestAbility();

    if (sFailures)
    {
        printf("%d of %d checks FAILED\n", sFailures, sChecks);
        return 1;
    }
    printf("all %d checks passed\n", sChecks);
    return 0;
}
