#ifndef GUARD_LEVEL_UP_LOGIC_H
#define GUARD_LEVEL_UP_LOGIC_H

// Pure helpers for EXP awards, level-up move ranges, evolution timing and Rare Candy eligibility.
// This header and src/level_up_logic.c intentionally depend on nothing but the C language so they
// can be compiled and tested on the host (see tests/host). Game code calls these helpers instead of
// open-coding the rules, so the host tests exercise the same logic the game runs.

// Layout of a level-up learnset entry (must match LEVEL_UP_MOVE_* in constants/pokemon.h;
// pokemon.c asserts this at compile time).
#define LEVELLOGIC_MOVE_ID   0x01FF
#define LEVELLOGIC_MOVE_LV   0xFE00
#define LEVELLOGIC_LV_SHIFT  9
#define LEVELLOGIC_END       0xFFFF

// --- Level-up move ranges -------------------------------------------------------------------

// Walks a learnset and yields every move learned in the level range (fromLevel, toLevel],
// ordered by level (ties keep learnset order), regardless of how the table itself is sorted.
struct LevelMoveCursor
{
    unsigned short index;
    unsigned char level;   // level currently being scanned
    unsigned char toLevel; // inclusive upper bound
};

void LevelMoveCursor_Init(struct LevelMoveCursor *cursor, unsigned char fromLevel, unsigned char toLevel);
// Returns the next move id, or 0 when the range is exhausted.
unsigned short LevelMoveCursor_Next(struct LevelMoveCursor *cursor, const unsigned short *learnset);

// --- EXP awards ------------------------------------------------------------------------------

// Gen VI style base EXP: no level-difference scaling.
unsigned short ExpLogic_BaseExp(unsigned short expYield, unsigned char defeatedLevel);
// EXP for one party member. Participants receive the full amount, everyone else half (min 1).
// Modifiers (Lucky Egg, trainer battle, traded Pokemon) are each x1.5 and applied in sequence.
unsigned int ExpLogic_Award(unsigned short baseExp, unsigned char isParticipant, unsigned char hasLuckyEgg,
                            unsigned char isTrainerBattle, unsigned char isTradedBoost);
// A Pokemon can gain EXP only if it is alive, below max level and not held back by a level cap.
unsigned char ExpLogic_CanGainExp(unsigned char isAlive, unsigned char level, unsigned char maxLevel, unsigned char isLevelCapped);
// The EXP is applied one level at a time. The level-up summary is shown once, after the final
// level: i.e. when at least one level was gained and there is no EXP left over to apply.
unsigned char ExpLogic_LevelUpSummaryDue(unsigned char gainedLevelThisAward, unsigned int leftoverExp);

// --- Evolution ---------------------------------------------------------------------------------

// Level requirement of a level-based evolution.
unsigned char EvoLogic_LevelReached(unsigned int requiredLevel, unsigned int level);
// Lowest level (exclusive) of the move range offered after an evolution. A recorded start level
// of EVOLOGIC_NO_RANGE means "no range recorded": offer the moves of the current level only.
#define EVOLOGIC_NO_RANGE 0xFF
unsigned char EvoLogic_MoveRangeStart(unsigned char recordedStartLevel, unsigned char currentLevel);

// --- Rare Candy --------------------------------------------------------------------------------

enum
{
    RARE_CANDY_NO_EFFECT,
    RARE_CANDY_LEVEL_UP,
    RARE_CANDY_EVOLVE_ONLY, // at max level: triggers an eligible evolution without raising the level
};

unsigned char RareCandyLogic_Outcome(unsigned char level, unsigned char maxLevel, unsigned char isLevelCapped,
                                     unsigned char isDeadNuzlocke, unsigned char hasEligibleEvolution);

#endif // GUARD_LEVEL_UP_LOGIC_H
