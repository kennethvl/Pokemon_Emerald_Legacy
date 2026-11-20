#include "global.h"
#include "event_data.h"
#include "field_message_box.h"
#include "random.h"
#include "mom_services.h"
#include "mon_modifier_logic.h"
#include "pokemon.h"
#include "string_util.h"
#include "strings.h"
#include "battle_main.h"
#include "pokemon_summary_screen.h"
#include "constants/abilities.h"
#include "constants/flags.h"
#include "constants/vars.h"
#include "constants/pkmn_center_mom.h"
#include "constants/pokemon.h"

// Mom's services work on a struct BoxPokemon, so the same code handles party Pokémon (&gPlayerParty[i].box)
// and Pokémon in the PC boxes. The PC (pokemon_storage_system.c) runs the menus and messages.

// If boxMon is a party Pokémon, recalculate its stats (boxed Pokémon don't store stats).
static void RecalcStatsIfParty(struct BoxPokemon *boxMon)
{
    u8 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (&gPlayerParty[i].box == boxMon)
        {
            CalculateMonStats(&gPlayerParty[i]);
            return;
        }
    }
}

static const u8 sIVMonData[NUM_STATS] =
{
    MON_DATA_HP_IV, MON_DATA_ATK_IV, MON_DATA_DEF_IV, MON_DATA_SPEED_IV, MON_DATA_SPATK_IV, MON_DATA_SPDEF_IV,
};

static const u8 sEVMonData[NUM_STATS] =
{
    MON_DATA_HP_EV, MON_DATA_ATK_EV, MON_DATA_DEF_EV, MON_DATA_SPEED_EV, MON_DATA_SPATK_EV, MON_DATA_SPDEF_EV,
};

static void GetBoxMonIVs(struct BoxPokemon *boxMon, u8 *ivs)
{
    u8 i;

    for (i = 0; i < NUM_STATS; i++)
        ivs[i] = GetBoxMonData(boxMon, sIVMonData[i]);
}

static void GetBoxMonEVs(struct BoxPokemon *boxMon, u8 *evs)
{
    u8 i;

    for (i = 0; i < NUM_STATS; i++)
        evs[i] = GetBoxMonData(boxMon, sEVMonData[i]);
}

// Called for every icon the PC shows, so it only reads the data the service needs.
bool8 Mom_IsServiceUseless(u8 service, struct BoxPokemon *boxMon)
{
    u8 ivs[NUM_STATS] = {0}, evs[NUM_STATS] = {0};
    u16 species = GetBoxMonData(boxMon, MON_DATA_SPECIES);

    if (species == SPECIES_NONE)
        return TRUE;
    if (service == MODLOGIC_MOM_SERVICE_HYPER_TRAINING)
        GetBoxMonIVs(boxMon, ivs);
    else if (service == MODLOGIC_MOM_SERVICE_MAX_EVS)
        GetBoxMonEVs(boxMon, evs);
    return ModLogic_MomServiceIsUseless(service, GetBoxMonData(boxMon, MON_DATA_IS_EGG),
                                        ivs, MAX_PER_STAT_IVS, evs, MAX_PER_STAT_EVS,
                                        gSpeciesInfo[species].abilities[0], gSpeciesInfo[species].abilities[1]);
}

// ---- Hyper Training: raise IVs to 31 ----

static const u8 sText_AllIVs[] = _("all its IVs");
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

// MON_DATA_*_IV for each single-stat selection (index MOM_HYPER_ALL is unused).
static const u8 sHyperTrainingMonData[] =
{
    [MOM_HYPER_HP]    = MON_DATA_HP_IV,
    [MOM_HYPER_ATK]   = MON_DATA_ATK_IV,
    [MOM_HYPER_DEF]   = MON_DATA_DEF_IV,
    [MOM_HYPER_SPATK] = MON_DATA_SPATK_IV,
    [MOM_HYPER_SPDEF] = MON_DATA_SPDEF_IV,
    [MOM_HYPER_SPEED] = MON_DATA_SPEED_IV,
};

const u8 *Mom_GetHyperTrainingLabel(u8 selection)
{
    return sHyperTrainingLabels[selection];
}

// TRUE if the selection (MOM_HYPER_*) would raise at least one IV.
bool8 Mom_CanHyperTrain(struct BoxPokemon *boxMon, u8 selection)
{
    if (selection == MOM_HYPER_ALL)
    {
        u8 ivs[NUM_STATS];

        GetBoxMonIVs(boxMon, ivs);
        return ModLogic_CountIVsBelowMax(ivs, MAX_PER_STAT_IVS) != 0;
    }
    return GetBoxMonData(boxMon, sHyperTrainingMonData[selection]) < MAX_PER_STAT_IVS;
}

void Mom_ApplyHyperTraining(struct BoxPokemon *boxMon, u8 selection)
{
    u32 maxIV = MAX_PER_STAT_IVS;
    u8 i;

    if (selection == MOM_HYPER_ALL)
    {
        for (i = 0; i < NUM_STATS; i++)
            SetBoxMonData(boxMon, sIVMonData[i], &maxIV);
    }
    else
    {
        SetBoxMonData(boxMon, sHyperTrainingMonData[selection], &maxIV);
    }
    RecalcStatsIfParty(boxMon);
}

// ---- Max EVs: every stat's EVs to 255 (the total EV cap is 6 x 255 too) ----

void Mom_ApplyMaxEVs(struct BoxPokemon *boxMon)
{
    u8 maxEV = MAX_PER_STAT_EVS;
    u8 i;

    for (i = 0; i < NUM_STATS; i++)
        SetBoxMonData(boxMon, sEVMonData[i], &maxEV);
    RecalcStatsIfParty(boxMon);
}

// ---- Ability change: switch between the species' two abilities (like an Ability Capsule) ----

// The ability slot (0 or 1) the Pokémon uses; falls back to slot 0 if its species has no ability in slot 1.
static u8 GetCurrentAbilitySlot(struct BoxPokemon *boxMon)
{
    u16 species = GetBoxMonData(boxMon, MON_DATA_SPECIES);
    u8 slot = GetBoxMonData(boxMon, MON_DATA_ABILITY_NUM) ? 1 : 0;

    if (gSpeciesInfo[species].abilities[slot] == ABILITY_NONE)
        slot = 0;
    return slot;
}

// The ability the Pokémon would switch to.
u8 Mom_GetOtherAbility(struct BoxPokemon *boxMon)
{
    u16 species = GetBoxMonData(boxMon, MON_DATA_SPECIES);

    return gSpeciesInfo[species].abilities[ModLogic_OtherAbilitySlot(GetCurrentAbilitySlot(boxMon))];
}

void Mom_ApplyAbilitySwap(struct BoxPokemon *boxMon)
{
    u8 newSlot = ModLogic_OtherAbilitySlot(GetCurrentAbilitySlot(boxMon));

    SetBoxMonData(boxMon, MON_DATA_ABILITY_NUM, &newSlot);
}

// ---- Nature change (stats only, like a Mint); the real nature is kept ----

static const u8 sText_NatureUp[] = _(" +");
static const u8 sText_NatureDown[] = _(" -");
static const u8 sText_NatureNeutral[] = _(" (neutral)");
static const u8 sText_StatAtk[] = _("ATK");
static const u8 sText_StatDef[] = _("DEF");
static const u8 sText_StatSpeed[] = _("SPEED");
static const u8 sText_StatSpAtk[] = _("SP.ATK");
static const u8 sText_StatSpDef[] = _("SP.DEF");
static const u8 *const sNatureStatNames[NUM_NATURE_STATS] =
{
    sText_StatAtk,
    sText_StatDef,
    sText_StatSpeed,
    sText_StatSpAtk,
    sText_StatSpDef,
};

static EWRAM_DATA u8 sNatureOptionText[NUM_NATURES][32] = {0};

// "ADAMANT +ATK -SP.ATK" style label so the player can see what each nature does. The returned string stays
// valid (it is kept in EWRAM), so it can be used as a list item.
const u8 *Mom_GetNatureOptionText(u8 nature)
{
    u8 *str = StringCopy(sNatureOptionText[nature], gNatureNamePointers[nature]);
    u8 stat, up = NUM_NATURE_STATS, down = NUM_NATURE_STATS;

    for (stat = 0; stat < NUM_NATURE_STATS; stat++)
    {
        if (gNatureStatTable[nature][stat] > 0)
            up = stat;
        else if (gNatureStatTable[nature][stat] < 0)
            down = stat;
    }

    if (up == NUM_NATURE_STATS)
    {
        StringCopy(str, sText_NatureNeutral);
    }
    else
    {
        str = StringCopy(str, sText_NatureUp);
        str = StringCopy(str, sNatureStatNames[up]);
        str = StringCopy(str, sText_NatureDown);
        StringCopy(str, sNatureStatNames[down]);
    }
    return sNatureOptionText[nature];
}

// The nature the Pokémon's stats follow (the override if there is one).
u8 Mom_GetStatNature(struct BoxPokemon *boxMon)
{
    return ModLogic_StatNature(GetBoxMonData(boxMon, MON_DATA_PERSONALITY), GetBoxMonData(boxMon, MON_DATA_NATURE_OVERRIDE), NUM_NATURES);
}

// Sets the stat nature. Choosing the Pokémon's real nature simply removes the override.
void Mom_ApplyNature(struct BoxPokemon *boxMon, u8 nature)
{
    u8 stored = ModLogic_EncodeOverride(nature);

    if (nature == GetNatureFromPersonality(GetBoxMonData(boxMon, MON_DATA_PERSONALITY)))
        stored = MODLOGIC_NO_OVERRIDE;
    SetBoxMonData(boxMon, MON_DATA_NATURE_OVERRIDE, &stored);
    RecalcStatsIfParty(boxMon);
}

// ---- Hidden Power type change (bypasses the IV-derived type) ----

static const u8 sHiddenPowerTypes[MOM_NUM_HIDDEN_POWER_TYPES] =
{
    TYPE_FIGHTING, TYPE_FLYING, TYPE_POISON, TYPE_GROUND, TYPE_ROCK, TYPE_BUG, TYPE_GHOST, TYPE_STEEL,
    TYPE_FIRE, TYPE_WATER, TYPE_GRASS, TYPE_ELECTRIC, TYPE_PSYCHIC, TYPE_ICE, TYPE_DRAGON, TYPE_DARK,
};

u8 Mom_GetHiddenPowerListType(u8 index)
{
    return sHiddenPowerTypes[index];
}

u8 Mom_GetHiddenPowerType(struct BoxPokemon *boxMon)
{
    u8 ivs[NUM_STATS];

    GetBoxMonIVs(boxMon, ivs);
    return ModLogic_HiddenPowerType(ivs, GetBoxMonData(boxMon, MON_DATA_HP_TYPE_OVERRIDE), NUMBER_OF_MON_TYPES, TYPE_MYSTERY);
}

// Stores the type as the Hidden Power type, independent of the IVs (its power still comes from the IVs).
void Mom_ApplyHiddenPower(struct BoxPokemon *boxMon, u8 type)
{
    u8 stored = ModLogic_EncodeOverride(type);

    SetBoxMonData(boxMon, MON_DATA_HP_TYPE_OVERRIDE, &stored);
}

// ---- Move Relearner ----

// Number of level-up and Egg moves the Pokémon could relearn.
u8 Mom_CountMovesToRelearn(struct BoxPokemon *boxMon)
{
    struct Pokemon mon;
    bool8 includeEggMoves = gRelearnerIncludesEggMoves;
    u8 count;

    BoxMonToMon(boxMon, &mon);
    gRelearnerIncludesEggMoves = TRUE;
    count = GetNumberOfRelearnableMoves(&mon);
    gRelearnerIncludesEggMoves = includeEggMoves;
    return count;
}

// Mom's Move Relearner offers Egg moves as well as level-up moves; her menu turns this off again.
void MomEnableEggMovesRelearner(void)
{
    gRelearnerIncludesEggMoves = TRUE;
}

void MomDisableEggMovesRelearner(void)
{
    gRelearnerIncludesEggMoves = FALSE;
}

// ---- Greetings and goodbyes, by story stage (see ModLogic_MomDialogueStage) ----

#define MOM_GREETINGS_PER_STAGE 3
#define MOM_GOODBYES_PER_STAGE  2

static const u8 sText_MomHelloFresh1[] = _("Oh, {PLAYER}! There you are!\nAre you eating properly out there?\pYour father says I worry too much.\nHe's probably right… but I'm MOM!$");
static const u8 sText_MomHelloFresh2[] = _("{PLAYER}! Let me look at you…\nYou've grown already, I swear!\pDid you pack your spare socks?\nDon't roll your eyes at me, dear!$");
static const u8 sText_MomHelloFresh3[] = _("Oh, {PLAYER}! I couldn't help it,\nI had to see how you were doing!\pYour dad's waiting in PETALBURG.\nGo say hi to him sometime, okay?$");

static const u8 sText_MomHelloBadges1[] = _("{PLAYER}! Is that a GYM BADGE?\nOh, show me, show me!\pI'm telling your father the very\nminute I get home!$");
static const u8 sText_MomHelloBadges2[] = _("There's my little GYM challenger!\pYour dad keeps asking about you.\nHe pretends he doesn't, of course.$");
static const u8 sText_MomHelloBadges3[] = _("Oh, {PLAYER}, your hair's a mess!\nHold still, let me fix it…\pThere. Now you look like a proper\nPOKéMON TRAINER, dear.$");

static const u8 sText_MomHelloBeatDad1[] = _("{PLAYER}… your father called me\nright after your match.\pHe said you were wonderful.\nI think he was crying a little!$");
static const u8 sText_MomHelloBeatDad2[] = _("You beat your father, {PLAYER}!\nHe's been smiling all week.\pDon't tell him I told you, but he\nput your photo up at the GYM.$");
static const u8 sText_MomHelloBeatDad3[] = _("{PLAYER}! I'm so very proud of you.\pYour dad taught you to be strong,\nand I taught you to be kind!$");

static const u8 sText_MomHelloCrisis1[] = _("{PLAYER}! Oh, thank goodness!\nI saw the storms on the news…\pPromise me you'll be careful.\nYour father's out there helping, too.$");
static const u8 sText_MomHelloCrisis2[] = _("Come here, let me hug you.\nI've been so worried, dear.\pYour dad says you'll be just fine.\nHe's never wrong about you.$");
static const u8 sText_MomHelloCrisis3[] = _("This rain and this heat… it's all\nso frightening, {PLAYER}.\pIf anyone can make this right,\nit's my brave little one.$");

static const u8 sText_MomHelloAllBadges1[] = _("Eight BADGES, {PLAYER}! Eight!\nThe POKéMON LEAGUE is next…\pI'll be cheering the loudest.\nYour father's already practicing!$");
static const u8 sText_MomHelloAllBadges2[] = _("VICTORY ROAD, huh? I'll be brave\nabout it. I will. I promise.\p…Did you pack enough POTIONS?\nAnd a sweater? It gets chilly!$");
static const u8 sText_MomHelloAllBadges3[] = _("Your father says you're ready.\pI know you are, too… but you'll\nalways be my baby, {PLAYER}.$");

static const u8 sText_MomHelloChampion1[] = _("My baby, the CHAMPION! I tell\neveryone at the market, you know.\pYour dad's told his whole GYM.\nTwice. He's so proud, it's silly!$");
static const u8 sText_MomHelloChampion2[] = _("Oh, CHAMPION {PLAYER}! Should I bow?\nHa ha! Come here, give me a hug.\pYou'll always be my sweet child,\nno matter how many titles you win.$");
static const u8 sText_MomHelloChampion3[] = _("Your father and I watched your\nmatch on TV. We both cried!\pDon't forget to come home for\ndinner sometime, CHAMPION.$");

static const u8 *const sMomGreetings[MODLOGIC_MOM_NUM_STAGES][MOM_GREETINGS_PER_STAGE] =
{
    [MODLOGIC_MOM_STAGE_FRESH]      = {sText_MomHelloFresh1, sText_MomHelloFresh2, sText_MomHelloFresh3},
    [MODLOGIC_MOM_STAGE_BADGES]     = {sText_MomHelloBadges1, sText_MomHelloBadges2, sText_MomHelloBadges3},
    [MODLOGIC_MOM_STAGE_BEAT_DAD]   = {sText_MomHelloBeatDad1, sText_MomHelloBeatDad2, sText_MomHelloBeatDad3},
    [MODLOGIC_MOM_STAGE_CRISIS]     = {sText_MomHelloCrisis1, sText_MomHelloCrisis2, sText_MomHelloCrisis3},
    [MODLOGIC_MOM_STAGE_ALL_BADGES] = {sText_MomHelloAllBadges1, sText_MomHelloAllBadges2, sText_MomHelloAllBadges3},
    [MODLOGIC_MOM_STAGE_CHAMPION]   = {sText_MomHelloChampion1, sText_MomHelloChampion2, sText_MomHelloChampion3},
};

static const u8 sText_MomByeFresh1[] = _("Take care out there, {PLAYER}!\nAnd don't forget to eat well!$");
static const u8 sText_MomByeFresh2[] = _("Brush your teeth, call home, and\nlisten to your POKéMON, dear!$");
static const u8 sText_MomByeBadges1[] = _("Go get that next BADGE, dear!\nDad and I are rooting for you!$");
static const u8 sText_MomByeBadges2[] = _("Wear clean socks to your next\nGYM battle! It's good luck!$");
static const u8 sText_MomByeBeatDad1[] = _("Your father's dying to hear all\nabout your next BADGE!$");
static const u8 sText_MomByeBeatDad2[] = _("Be kind to your POKéMON, dear.\nYour father always says so.$");
static const u8 sText_MomByeCrisis1[] = _("Please be careful, {PLAYER}.\nCome home safe to us.$");
static const u8 sText_MomByeCrisis2[] = _("Dad and I love you very much.\nNow go be brave, sweetheart.$");
static const u8 sText_MomByeAllBadges1[] = _("Go show the LEAGUE who you are!\nAnd eat a good breakfast first!$");
static const u8 sText_MomByeAllBadges2[] = _("Your dad says trust your POKéMON.\nI say: wear a scarf, dear!$");
static const u8 sText_MomByeChampion1[] = _("Come home for dinner soon, dear.\nYour father's making his curry!$");
static const u8 sText_MomByeChampion2[] = _("Off you go, CHAMPION! But you're\nstill doing the dishes at home.$");

static const u8 *const sMomGoodbyes[MODLOGIC_MOM_NUM_STAGES][MOM_GOODBYES_PER_STAGE] =
{
    [MODLOGIC_MOM_STAGE_FRESH]      = {sText_MomByeFresh1, sText_MomByeFresh2},
    [MODLOGIC_MOM_STAGE_BADGES]     = {sText_MomByeBadges1, sText_MomByeBadges2},
    [MODLOGIC_MOM_STAGE_BEAT_DAD]   = {sText_MomByeBeatDad1, sText_MomByeBeatDad2},
    [MODLOGIC_MOM_STAGE_CRISIS]     = {sText_MomByeCrisis1, sText_MomByeCrisis2},
    [MODLOGIC_MOM_STAGE_ALL_BADGES] = {sText_MomByeAllBadges1, sText_MomByeAllBadges2},
    [MODLOGIC_MOM_STAGE_CHAMPION]   = {sText_MomByeChampion1, sText_MomByeChampion2},
};

static u8 GetMomDialogueStage(void)
{
    u8 numBadges = 0;
    u16 flag;

    for (flag = FLAG_BADGE01_GET; flag < FLAG_BADGE01_GET + NUM_BADGES; flag++)
    {
        if (FlagGet(flag))
            numBadges++;
    }
    return ModLogic_MomDialogueStage(numBadges, FlagGet(FLAG_DEFEATED_PETALBURG_GYM),
                                     VarGet(VAR_SOOTOPOLIS_CITY_STATE), FlagGet(FLAG_SYS_GAME_CLEAR));
}

// Prints a random greeting for the current story stage. The script follows with waitmessage + waitbuttonpress.
void MomShowGreeting(void)
{
    ShowFieldMessage(sMomGreetings[GetMomDialogueStage()][Random() % MOM_GREETINGS_PER_STAGE]);
}

// Prints a random goodbye for the current story stage. The script follows with waitmessage + waitbuttonpress.
void MomShowGoodbye(void)
{
    ShowFieldMessage(sMomGoodbyes[GetMomDialogueStage()][Random() % MOM_GOODBYES_PER_STAGE]);
}
