#include "level_up_logic.h"

void LevelMoveCursor_Init(struct LevelMoveCursor *cursor, unsigned char fromLevel, unsigned char toLevel)
{
    cursor->index = 0;
    cursor->level = fromLevel + 1;
    cursor->toLevel = toLevel;
    // fromLevel == 255 would wrap; treat that as an empty range.
    if (fromLevel == 0xFF)
        cursor->level = toLevel + 1;
}

unsigned short LevelMoveCursor_Next(struct LevelMoveCursor *cursor, const unsigned short *learnset)
{
    // Scan level by level so moves come out in level order even if the table is not sorted.
    while (cursor->level <= cursor->toLevel && cursor->level != 0)
    {
        while (learnset[cursor->index] != LEVELLOGIC_END)
        {
            unsigned short entry = learnset[cursor->index];

            cursor->index++;
            if (((entry & LEVELLOGIC_MOVE_LV) >> LEVELLOGIC_LV_SHIFT) == cursor->level)
                return entry & LEVELLOGIC_MOVE_ID;
        }
        cursor->level++;
        cursor->index = 0;
    }
    return 0;
}

unsigned short ExpLogic_BaseExp(unsigned short expYield, unsigned char defeatedLevel)
{
    return (unsigned short)(expYield * defeatedLevel / 7);
}

unsigned int ExpLogic_Award(unsigned short baseExp, unsigned char hasLuckyEgg,
                            unsigned char isTrainerBattle, unsigned char isTradedBoost)
{
    unsigned int exp = baseExp;

    if (exp == 0)
        exp = 1;
    if (hasLuckyEgg)
        exp = exp * 150 / 100;
    if (isTrainerBattle)
        exp = exp * 150 / 100;
    if (isTradedBoost)
        exp = exp * 150 / 100;
    return exp;
}

unsigned char ExpLogic_CanGainExp(unsigned char isAlive, unsigned char level, unsigned char maxLevel, unsigned char isLevelCapped)
{
    return isAlive && level < maxLevel && !isLevelCapped;
}

unsigned char ExpLogic_LevelUpSummaryDue(unsigned char gainedLevelThisAward, unsigned int leftoverExp)
{
    return gainedLevelThisAward && leftoverExp == 0;
}

unsigned char EvoLogic_LevelReached(unsigned int requiredLevel, unsigned int level)
{
    return requiredLevel <= level;
}

unsigned char EvoLogic_MoveRangeStart(unsigned char recordedStartLevel, unsigned char currentLevel)
{
    if (recordedStartLevel == EVOLOGIC_NO_RANGE || recordedStartLevel > currentLevel)
        return currentLevel > 0 ? currentLevel - 1 : 0;
    return recordedStartLevel;
}

unsigned char RareCandyLogic_Outcome(unsigned char level, unsigned char maxLevel, unsigned char isLevelCapped,
                                     unsigned char isDeadNuzlocke, unsigned char hasEligibleEvolution)
{
    if (isDeadNuzlocke)
        return RARE_CANDY_NO_EFFECT;
    if (level >= maxLevel)
        return hasEligibleEvolution ? RARE_CANDY_EVOLVE_ONLY : RARE_CANDY_NO_EFFECT;
    if (isLevelCapped)
        return RARE_CANDY_NO_EFFECT;
    return RARE_CANDY_LEVEL_UP;
}
