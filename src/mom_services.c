#include "global.h"
#include "event_data.h"
#include "field_message_box.h"
#include "random.h"
#include "money.h"
#include "mom_services.h"
#include "mon_modifier_logic.h"
#include "pokemon.h"
#include "string_util.h"
#include "strings.h"
#include "battle_main.h"
#include "pokemon_summary_screen.h"
#include "constants/abilities.h"
#include "constants/field_specials.h"
#include "constants/flags.h"
#include "constants/vars.h"
#include "constants/pkmn_center_mom.h"
#include "constants/pokemon.h"

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

// ---- Nature change (stats only, like a Mint) ----

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

// "ADAMANT +ATK -SP.ATK" style label so the player can see what each nature does.
static void BuildNatureOptionText(u8 nature)
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
        return;
    }
    str = StringCopy(str, sText_NatureUp);
    str = StringCopy(str, sNatureStatNames[up]);
    str = StringCopy(str, sText_NatureDown);
    StringCopy(str, sNatureStatNames[down]);
}

// ---- Hidden Power type change (bypasses the IV-derived type) ----

static const u8 sHiddenPowerTypes[MOM_NUM_HIDDEN_POWER_TYPES] =
{
    TYPE_FIGHTING, TYPE_FLYING, TYPE_POISON, TYPE_GROUND, TYPE_ROCK, TYPE_BUG, TYPE_GHOST, TYPE_STEEL,
    TYPE_FIRE, TYPE_WATER, TYPE_GRASS, TYPE_ELECTRIC, TYPE_PSYCHIC, TYPE_ICE, TYPE_DRAGON, TYPE_DARK,
};

const u8 *MomGetScrollOptionText(u8 scrollId, u8 index)
{
    if (scrollId == SCROLL_MULTI_MOM_NATURES)
    {
        BuildNatureOptionText(index);
        return sNatureOptionText[index];
    }
    if (scrollId == SCROLL_MULTI_MOM_HP_TYPES)
        return gTypeNames[sHiddenPowerTypes[index]];
    return gText_EmptyString7;
}

// Inputs: VAR_0x8004 = party slot, VAR_0x8005 = chosen type (list index).
// Outputs: VAR_RESULT = TRUE if the type would change; STR_VAR_1 = price, STR_VAR_2 = current Hidden Power type,
//          STR_VAR_3 = chosen type; VAR_0x8006 = price.
void MomBufferHiddenPowerChange(void)
{
    struct Pokemon *mon = SERVICE_MON;
    u8 current = GetMonHiddenPowerType(mon);
    u8 chosen = sHiddenPowerTypes[gSpecialVar_0x8005];

    ConvertIntToDecimalStringN(gStringVar1, MOM_SERVICE_PRICE, STR_CONV_MODE_LEFT_ALIGN, 5);
    StringCopy(gStringVar2, gTypeNames[current]);
    StringCopy(gStringVar3, gTypeNames[chosen]);
    gSpecialVar_0x8006 = MOM_SERVICE_PRICE;
    gSpecialVar_Result = chosen != current;
}

// Stores the chosen type (VAR_0x8005) as the Hidden Power type, independent of the IVs. Hyper Training or any
// other IV change no longer affects it. Hidden Power's power still comes from the IVs.
void MomApplyHiddenPowerChange(void)
{
    struct Pokemon *mon = SERVICE_MON;
    u8 stored = ModLogic_EncodeOverride(sHiddenPowerTypes[gSpecialVar_0x8005]);

    SetMonData(mon, MON_DATA_HP_TYPE_OVERRIDE, &stored);
}

// Inputs: VAR_0x8004 = party slot, VAR_0x8005 = chosen nature (list index).
// Outputs: VAR_RESULT = TRUE if the nature would change; STR_VAR_1 = price, STR_VAR_2 = current stat nature,
//          STR_VAR_3 = chosen nature; VAR_0x8006 = price.
void MomBufferNatureChange(void)
{
    struct Pokemon *mon = SERVICE_MON;
    u8 current = GetMonStatNature(mon);

    ConvertIntToDecimalStringN(gStringVar1, MOM_SERVICE_PRICE, STR_CONV_MODE_LEFT_ALIGN, 5);
    StringCopy(gStringVar2, gNatureNamePointers[current]);
    StringCopy(gStringVar3, gNatureNamePointers[gSpecialVar_0x8005]);
    gSpecialVar_0x8006 = MOM_SERVICE_PRICE;
    gSpecialVar_Result = gSpecialVar_0x8005 != current;
}

// Sets the stat nature to the chosen one (VAR_0x8005). The Pokémon's real nature is untouched; choosing its
// real nature simply removes the override.
void MomApplyNatureChange(void)
{
    struct Pokemon *mon = SERVICE_MON;
    u8 stored = ModLogic_EncodeOverride(gSpecialVar_0x8005);

    if (gSpecialVar_0x8005 == GetNature(mon))
        stored = MODLOGIC_NO_OVERRIDE;
    SetMonData(mon, MON_DATA_NATURE_OVERRIDE, &stored);
    CalculateMonStats(mon);
}

// ---- Max EVs: every stat's EVs to 255 (ignores the usual 510 total cap) ----

static const u8 sEVMonData[NUM_STATS] =
{
    MON_DATA_HP_EV, MON_DATA_ATK_EV, MON_DATA_DEF_EV, MON_DATA_SPEED_EV, MON_DATA_SPATK_EV, MON_DATA_SPDEF_EV,
};

// Input: VAR_0x8004 = party slot. Output: VAR_RESULT = TRUE if any of the Pokémon's EVs is below the maximum.
void MomBufferMaxEVs(void)
{
    struct Pokemon *mon = SERVICE_MON;
    u8 i;

    gSpecialVar_Result = FALSE;
    for (i = 0; i < NUM_STATS; i++)
    {
        if (GetMonData(mon, sEVMonData[i]) < MAX_PER_STAT_EVS)
            gSpecialVar_Result = TRUE;
    }
}

// Sets all six EVs to MAX_PER_STAT_EVS and recalculates stats.
void MomApplyMaxEVs(void)
{
    struct Pokemon *mon = SERVICE_MON;
    u8 maxEV = MAX_PER_STAT_EVS;
    u8 i;

    for (i = 0; i < NUM_STATS; i++)
        SetMonData(mon, sEVMonData[i], &maxEV);
    CalculateMonStats(mon);
}

// Mom's Move Relearner offers Egg moves as well as level-up moves; the menu turns this off again.
void MomEnableEggMovesRelearner(void)
{
    gRelearnerIncludesEggMoves = TRUE;
}

void MomDisableEggMovesRelearner(void)
{
    gRelearnerIncludesEggMoves = FALSE;
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
