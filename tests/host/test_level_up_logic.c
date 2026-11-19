#include <stdio.h>
#include <string.h>
#include "level_up_logic.h"

static int sFailures;
static int sChecks;

#define CHECK(cond) \
    do { \
        sChecks++; \
        if (!(cond)) { \
            sFailures++; \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        } \
    } while (0)

#define CHECK_EQ(a, b) \
    do { \
        long long va_ = (long long)(a), vb_ = (long long)(b); \
        sChecks++; \
        if (va_ != vb_) { \
            sFailures++; \
            printf("FAIL %s:%d: %s == %s (%lld vs %lld)\n", __FILE__, __LINE__, #a, #b, va_, vb_); \
        } \
    } while (0)

#define MOVE(lv, id) ((unsigned short)(((lv) << LEVELLOGIC_LV_SHIFT) | (id)))
#define END LEVELLOGIC_END

// ---- Test data: a small three-stage line (ids are arbitrary) -------------------------------
enum { SP_NONE, SP_BABY, SP_MID, SP_FINAL, SP_LONER };
enum { M_SCRATCH = 10, M_GROWL, M_EMBER, M_BITE, M_SLASH, M_FLAME, M_FLY, M_WING, M_INFERNO, M_LATE };

static const unsigned short sBabyMoves[] = {
    MOVE(1, M_SCRATCH), MOVE(1, M_GROWL), MOVE(7, M_EMBER), MOVE(13, M_BITE), MOVE(19, M_SLASH), END
};
static const unsigned short sMidMoves[] = {
    MOVE(1, M_SCRATCH), MOVE(1, M_GROWL), MOVE(7, M_EMBER), MOVE(13, M_BITE),
    MOVE(20, M_SLASH), MOVE(27, M_FLAME), MOVE(34, M_FLY), MOVE(41, M_INFERNO), END
};
static const unsigned short sFinalMoves[] = {
    MOVE(1, M_SCRATCH), MOVE(36, M_WING), MOVE(38, M_FLY), MOVE(39, M_INFERNO), MOVE(46, M_LATE), END
};
// Deliberately unsorted: level order must still be honoured.
static const unsigned short sUnsortedMoves[] = {
    MOVE(30, M_FLY), MOVE(12, M_BITE), MOVE(30, M_WING), MOVE(5, M_EMBER), MOVE(12, M_GROWL), END
};
static const unsigned short sEmptyMoves[] = { END };

struct EvoStep { unsigned short from, to; unsigned char level; };
static const struct EvoStep sEvoChain[] = {
    { SP_BABY, SP_MID, 16 },
    { SP_MID, SP_FINAL, 36 },
};

static const unsigned short *LearnsetOf(unsigned short species)
{
    switch (species)
    {
    case SP_BABY: return sBabyMoves;
    case SP_MID: return sMidMoves;
    case SP_FINAL: return sFinalMoves;
    default: return sEmptyMoves;
    }
}

// Mirrors GetEvolutionTargetSpecies for level-based evolutions: one stage per call.
static unsigned short EvolveOneStage(unsigned short species, unsigned char level)
{
    unsigned i;

    for (i = 0; i < sizeof(sEvoChain) / sizeof(sEvoChain[0]); i++)
        if (sEvoChain[i].from == species && EvoLogic_LevelReached(sEvoChain[i].level, level))
            return sEvoChain[i].to;
    return SP_NONE;
}

static int CollectMoves(const unsigned short *learnset, unsigned char from, unsigned char to, unsigned short *out, int max)
{
    struct LevelMoveCursor cursor;
    int n = 0;
    unsigned short move;

    LevelMoveCursor_Init(&cursor, from, to);
    while ((move = LevelMoveCursor_Next(&cursor, learnset)) != 0 && n < max)
        out[n++] = move;
    return n;
}

// ---- Move ranges -----------------------------------------------------------------------------
static void TestMoveRanges(void)
{
    unsigned short m[16];
    int n;

    // Single level behaves like the old per-level lookup (from = level - 1, to = level).
    n = CollectMoves(sBabyMoves, 6, 7, m, 16);
    CHECK_EQ(n, 1); CHECK_EQ(m[0], M_EMBER);
    n = CollectMoves(sBabyMoves, 0, 1, m, 16);
    CHECK_EQ(n, 2); CHECK_EQ(m[0], M_SCRATCH); CHECK_EQ(m[1], M_GROWL); // same-level ties keep table order
    n = CollectMoves(sBabyMoves, 7, 8, m, 16);
    CHECK_EQ(n, 0);

    // Multi-level gain: every crossed level's move is offered, in level order.
    n = CollectMoves(sBabyMoves, 5, 20, m, 16);
    CHECK_EQ(n, 3); CHECK_EQ(m[0], M_EMBER); CHECK_EQ(m[1], M_BITE); CHECK_EQ(m[2], M_SLASH);

    // The lower bound is exclusive and the upper bound inclusive.
    n = CollectMoves(sBabyMoves, 7, 13, m, 16);
    CHECK_EQ(n, 1); CHECK_EQ(m[0], M_BITE);
    n = CollectMoves(sBabyMoves, 13, 19, m, 16);
    CHECK_EQ(n, 1); CHECK_EQ(m[0], M_SLASH);

    // Unsorted tables still come out in level order.
    n = CollectMoves(sUnsortedMoves, 0, 40, m, 16);
    CHECK_EQ(n, 5);
    CHECK_EQ(m[0], M_EMBER); CHECK_EQ(m[1], M_BITE); CHECK_EQ(m[2], M_GROWL);
    CHECK_EQ(m[3], M_FLY); CHECK_EQ(m[4], M_WING);

    // Empty / inverted / degenerate ranges.
    CHECK_EQ(CollectMoves(sBabyMoves, 20, 20, m, 16), 0);
    CHECK_EQ(CollectMoves(sBabyMoves, 30, 10, m, 16), 0);
    CHECK_EQ(CollectMoves(sEmptyMoves, 0, 100, m, 16), 0);
    CHECK_EQ(CollectMoves(sBabyMoves, 255, 100, m, 16), 0);
    // Whole-game range from level 0 to 100 returns every entry exactly once.
    CHECK_EQ(CollectMoves(sFinalMoves, 0, 100, m, 16), 5);

    // The cursor keeps its place between calls (declined moves are not offered again).
    {
        struct LevelMoveCursor cursor;
        LevelMoveCursor_Init(&cursor, 5, 20);
        CHECK_EQ(LevelMoveCursor_Next(&cursor, sBabyMoves), M_EMBER);
        CHECK_EQ(LevelMoveCursor_Next(&cursor, sBabyMoves), M_BITE);
        CHECK_EQ(LevelMoveCursor_Next(&cursor, sBabyMoves), M_SLASH);
        CHECK_EQ(LevelMoveCursor_Next(&cursor, sBabyMoves), 0);
        CHECK_EQ(LevelMoveCursor_Next(&cursor, sBabyMoves), 0); // stays exhausted
    }
}

// ---- EXP flow ----------------------------------------------------------------------------------
static void TestExpAwards(void)
{
    // Base EXP does not depend on the receiving Pokemon's level (no level-difference scaling).
    CHECK_EQ(ExpLogic_BaseExp(64, 50), 64 * 50 / 7);
    CHECK_EQ(ExpLogic_BaseExp(1, 1), 0);
    CHECK_EQ(ExpLogic_Award(ExpLogic_BaseExp(1, 1), 0, 0, 0), 1); // never zero

    // Every eligible party member gets the full amount.
    CHECK_EQ(ExpLogic_Award(100, 0, 0, 0), 100);
    CHECK_EQ(ExpLogic_Award(101, 0, 0, 0), 101);
    CHECK_EQ(ExpLogic_Award(0, 0, 0, 0), 1);

    // Modifiers are 1.5x each and apply in sequence.
    CHECK_EQ(ExpLogic_Award(100, 1, 0, 0), 150);
    CHECK_EQ(ExpLogic_Award(100, 0, 1, 0), 150);
    CHECK_EQ(ExpLogic_Award(100, 0, 0, 1), 150);
    CHECK_EQ(ExpLogic_Award(100, 1, 1, 1), 100 * 150 / 100 * 150 / 100 * 150 / 100);

    // Who may gain EXP: alive, below max level, not level-capped.
    CHECK(ExpLogic_CanGainExp(1, 50, 100, 0));
    CHECK(!ExpLogic_CanGainExp(0, 50, 100, 0));  // fainted
    CHECK(!ExpLogic_CanGainExp(1, 100, 100, 0)); // max level
    CHECK(!ExpLogic_CanGainExp(1, 40, 100, 1));  // level cap (Nuzlocke/hard mode)
    CHECK(ExpLogic_CanGainExp(1, 99, 100, 0));
}

// Medium-fast growth: exp = level^3.
static unsigned int ExpForLevel(unsigned int level) { return level * level * level; }

// Mirrors the battle loop: the EXP is applied one level at a time (as the controller does) and the
// summary must appear exactly once, after the final level.
static void SimulateAward(unsigned char startLevel, unsigned int startExp, unsigned int gain,
                          unsigned char maxLevel, unsigned char *finalLevel, int *summaries, int *levelsGained)
{
    unsigned char level = startLevel;
    unsigned int exp = startExp;
    int pending = 0;

    *summaries = 0;
    *levelsGained = 0;
    for (;;)
    {
        unsigned int leftover = 0;
        int gained = 0;

        if (level < maxLevel)
        {
            if (exp + gain >= ExpForLevel(level + 1))
            {
                leftover = gain - (ExpForLevel(level + 1) - exp);
                exp = ExpForLevel(level + 1);
                level++;
                gained = 1;
                pending = 1;
                (*levelsGained)++;
            }
            else
            {
                exp += gain;
            }
        }
        (void)gained;
        if (ExpLogic_LevelUpSummaryDue(pending, leftover))
        {
            (*summaries)++;
            pending = 0;
        }
        if (leftover == 0)
            break;
        gain = leftover;
    }
    *finalLevel = level;
}

static void TestLevelUpSummaryBatching(void)
{
    unsigned char finalLevel;
    int summaries, levels;

    CHECK(!ExpLogic_LevelUpSummaryDue(0, 0));
    CHECK(!ExpLogic_LevelUpSummaryDue(0, 500));
    CHECK(!ExpLogic_LevelUpSummaryDue(1, 500)); // more EXP still to apply: not the final level yet
    CHECK(ExpLogic_LevelUpSummaryDue(1, 0));

    // No level gained: no summary.
    SimulateAward(10, ExpForLevel(10), 5, 100, &finalLevel, &summaries, &levels);
    CHECK_EQ(finalLevel, 10); CHECK_EQ(summaries, 0); CHECK_EQ(levels, 0);

    // One level: one summary.
    SimulateAward(10, ExpForLevel(10), ExpForLevel(11) - ExpForLevel(10), 100, &finalLevel, &summaries, &levels);
    CHECK_EQ(finalLevel, 11); CHECK_EQ(summaries, 1); CHECK_EQ(levels, 1);

    // Many levels in one award: still exactly one summary, at the final level.
    SimulateAward(10, ExpForLevel(10), ExpForLevel(25) - ExpForLevel(10), 100, &finalLevel, &summaries, &levels);
    CHECK_EQ(finalLevel, 25); CHECK_EQ(summaries, 1); CHECK_EQ(levels, 15);

    // A partial last level: the summary still waits for the leftover EXP to be applied.
    SimulateAward(10, ExpForLevel(10), ExpForLevel(25) - ExpForLevel(10) + 7, 100, &finalLevel, &summaries, &levels);
    CHECK_EQ(finalLevel, 25); CHECK_EQ(summaries, 1); CHECK_EQ(levels, 15);

    // Level cap / max level mid-award: gains stop at the cap and the summary is not lost.
    SimulateAward(10, ExpForLevel(10), ExpForLevel(40) - ExpForLevel(10), 20, &finalLevel, &summaries, &levels);
    CHECK_EQ(finalLevel, 20); CHECK_EQ(summaries, 1); CHECK_EQ(levels, 10);

    // Already at the cap: nothing happens.
    SimulateAward(20, ExpForLevel(20), 100000, 20, &finalLevel, &summaries, &levels);
    CHECK_EQ(finalLevel, 20); CHECK_EQ(summaries, 0); CHECK_EQ(levels, 0);
}

// ---- Evolution stages ---------------------------------------------------------------------------
static void TestEvolutionRules(void)
{
    CHECK(EvoLogic_LevelReached(16, 16));
    CHECK(EvoLogic_LevelReached(16, 17));
    CHECK(!EvoLogic_LevelReached(16, 15));
    CHECK(EvoLogic_LevelReached(1, 1));

    CHECK_EQ(EvoLogic_MoveRangeStart(EVOLOGIC_NO_RANGE, 40), 39); // default: current level only
    CHECK_EQ(EvoLogic_MoveRangeStart(EVOLOGIC_NO_RANGE, 1), 0);
    CHECK_EQ(EvoLogic_MoveRangeStart(14, 40), 14);                // recorded level before the gain
    CHECK_EQ(EvoLogic_MoveRangeStart(40, 40), 40);                // empty range (Rare Candy at level 100)
    CHECK_EQ(EvoLogic_MoveRangeStart(50, 40), 39);                // nonsense start falls back to default

    // Evolution stages: a single event evolves exactly one stage even if later stages are also reachable.
    CHECK_EQ(EvolveOneStage(SP_BABY, 40), SP_MID);
    CHECK_EQ(EvolveOneStage(SP_MID, 40), SP_FINAL);
    CHECK_EQ(EvolveOneStage(SP_BABY, 15), SP_NONE);
    CHECK_EQ(EvolveOneStage(SP_FINAL, 100), SP_NONE);
    CHECK_EQ(EvolveOneStage(SP_LONER, 100), SP_NONE);
}

// Full level-up event: pre-evolution moves for the gained range, then ONE evolution, then the
// evolved form's moves from the same range (as after a battle or a catch).
static void TestEvolutionEventMoves(void)
{
    unsigned short m[16];
    unsigned short species = SP_BABY;
    unsigned char startLevel = 14, level = 40;
    unsigned short evolved;
    int n;

    // Moves offered at level-up time come from the pre-evolution learnset: only SLASH@19 is in (14, 40].
    n = CollectMoves(LearnsetOf(species), startLevel, level, m, 16);
    CHECK_EQ(n, 1); CHECK_EQ(m[0], M_SLASH);

    // One evolution per event: the baby form becomes the middle form, not the final form.
    evolved = EvolveOneStage(species, level);
    CHECK_EQ(evolved, SP_MID);

    // The evolved form is offered every move it learns in the gained range, which the old
    // "current level only" lookup would have missed (SLASH@20, FLAME@27, FLY@34).
    n = CollectMoves(LearnsetOf(evolved), EvoLogic_MoveRangeStart(startLevel, level), level, m, 16);
    CHECK_EQ(n, 3); CHECK_EQ(m[0], M_SLASH); CHECK_EQ(m[1], M_FLAME); CHECK_EQ(m[2], M_FLY);
    // Without a recorded range only the current level is checked: nothing at level 40.
    n = CollectMoves(LearnsetOf(evolved), EvoLogic_MoveRangeStart(EVOLOGIC_NO_RANGE, level), level, m, 16);
    CHECK_EQ(n, 0);

    // The next event (after a capture or battle) can evolve the next stage.
    startLevel = level; level = 41; species = evolved;
    evolved = EvolveOneStage(species, level);
    CHECK_EQ(evolved, SP_FINAL);
    n = CollectMoves(LearnsetOf(species), startLevel, level, m, 16);
    CHECK_EQ(n, 1); CHECK_EQ(m[0], M_INFERNO);                     // mid-form move at 41
    n = CollectMoves(LearnsetOf(evolved), startLevel, level, m, 16);
    CHECK_EQ(n, 0);                                                 // nothing new in (40, 41] for the final form

    // A big jump straight past the final form's own levels offers them in level order.
    n = CollectMoves(LearnsetOf(SP_FINAL), 35, 40, m, 16);
    CHECK_EQ(n, 3); CHECK_EQ(m[0], M_WING); CHECK_EQ(m[1], M_FLY); CHECK_EQ(m[2], M_INFERNO);

    // Rare Candy at level 100 evolves without a level gain, so no extra moves are offered.
    n = CollectMoves(LearnsetOf(SP_FINAL), EvoLogic_MoveRangeStart(100, 100), 100, m, 16);
    CHECK_EQ(n, 0);
    // Evolution that is not level-up driven (e.g. trade) keeps the default: moves at the current level.
    n = CollectMoves(LearnsetOf(SP_FINAL), EvoLogic_MoveRangeStart(EVOLOGIC_NO_RANGE, 46), 46, m, 16);
    CHECK_EQ(n, 1); CHECK_EQ(m[0], M_LATE);
}

static void TestRareCandy(void)
{
    // Normal level-up.
    CHECK_EQ(RareCandyLogic_Outcome(50, 100, 0, 0, 0), RARE_CANDY_LEVEL_UP);
    CHECK_EQ(RareCandyLogic_Outcome(50, 100, 0, 0, 1), RARE_CANDY_LEVEL_UP); // evolution is handled after the level
    CHECK_EQ(RareCandyLogic_Outcome(99, 100, 0, 0, 0), RARE_CANDY_LEVEL_UP);

    // Level 100: only works if a level-up evolution is available, and never raises the level.
    CHECK_EQ(RareCandyLogic_Outcome(100, 100, 0, 0, 1), RARE_CANDY_EVOLVE_ONLY);
    CHECK_EQ(RareCandyLogic_Outcome(100, 100, 0, 0, 0), RARE_CANDY_NO_EFFECT);
    // Hard-mode level cap is >= the cap at level 100, but no level is gained, so evolving is fine.
    CHECK_EQ(RareCandyLogic_Outcome(100, 100, 1, 0, 1), RARE_CANDY_EVOLVE_ONLY);

    // Level caps still block raising the level.
    CHECK_EQ(RareCandyLogic_Outcome(40, 100, 1, 0, 0), RARE_CANDY_NO_EFFECT);
    CHECK_EQ(RareCandyLogic_Outcome(40, 100, 1, 0, 1), RARE_CANDY_NO_EFFECT);

    // Dead Nuzlocke Pokemon cannot use it at all.
    CHECK_EQ(RareCandyLogic_Outcome(50, 100, 0, 1, 0), RARE_CANDY_NO_EFFECT);
    CHECK_EQ(RareCandyLogic_Outcome(100, 100, 0, 1, 1), RARE_CANDY_NO_EFFECT);
}

int main(void)
{
    TestMoveRanges();
    TestExpAwards();
    TestLevelUpSummaryBatching();
    TestEvolutionRules();
    TestEvolutionEventMoves();
    TestRareCandy();

    if (sFailures)
    {
        printf("%d of %d checks FAILED\n", sFailures, sChecks);
        return 1;
    }
    printf("all %d checks passed\n", sChecks);
    return 0;
}
