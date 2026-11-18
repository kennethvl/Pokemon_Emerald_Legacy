@ Mom, found in every Pokémon Center (after receiving the POKéDEX, and not in HARD / NUZLOCKE mode).
@ She offers the "modifier" services: Hyper Training, Nature, Ability and Hidden Power type changes.

@ Called from every Pokémon Center's OnTransition. FLAG_TEMP_10 hides Mom and is cleared on every map load.
PokemonCenter_EventScript_UpdateMom::
	goto_if_unset FLAG_SYS_POKEDEX_GET, PokemonCenter_EventScript_HideMom
	goto_if_set FLAG_HARD, PokemonCenter_EventScript_HideMom
	goto_if_set FLAG_NUZLOCKE, PokemonCenter_EventScript_HideMom
	clearflag FLAG_TEMP_10
	return

PokemonCenter_EventScript_HideMom::
	setflag FLAG_TEMP_10
	return

PokemonCenter_EventScript_Mom::
	lock
	faceplayer
	msgbox PokemonCenter_Text_MomWelcome, MSGBOX_DEFAULT
PokemonCenter_EventScript_MomMenu::
	message PokemonCenter_Text_MomWhatCanIDo
	waitmessage
	multichoice 0, 0, MULTI_PKMN_CENTER_MOM, FALSE
	switch VAR_RESULT
	case 0, PokemonCenter_EventScript_MomHyperTraining
	case 1, PokemonCenter_EventScript_MomNature
	case 2, PokemonCenter_EventScript_MomAbility
	case 3, PokemonCenter_EventScript_MomHiddenPower
	case 4, PokemonCenter_EventScript_MomGoodbye
	case MULTI_B_PRESSED, PokemonCenter_EventScript_MomGoodbye
	goto PokemonCenter_EventScript_MomGoodbye
	end

@ ---- Hyper Training: raise IVs to 31, MOM_SERVICE_PRICE per IV raised ----
PokemonCenter_EventScript_MomHyperTraining::
	msgbox PokemonCenter_Text_MomWhichMon, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, PokemonCenter_EventScript_MomMenu
	special MomCanModifyPartyMon
	goto_if_eq VAR_RESULT, FALSE, PokemonCenter_EventScript_MomCantModifyEgg
PokemonCenter_EventScript_MomHyperTrainingStat::
	message PokemonCenter_Text_MomWhichStat
	waitmessage
	multichoice 0, 0, MULTI_MOM_HYPER_TRAINING, FALSE
	goto_if_eq VAR_RESULT, MOM_HYPER_CANCEL, PokemonCenter_EventScript_MomMenu
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, PokemonCenter_EventScript_MomMenu
	copyvar VAR_0x8005, VAR_RESULT
	special MomBufferHyperTraining
	goto_if_eq VAR_RESULT, 0, PokemonCenter_EventScript_MomHyperAlreadyPerfect
	msgbox PokemonCenter_Text_MomHyperConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PokemonCenter_EventScript_MomMenu
	special MomPayForService
	goto_if_eq VAR_RESULT, FALSE, PokemonCenter_EventScript_MomNotEnoughMoney
	special MomApplyHyperTraining
	playfanfare MUS_LEVEL_UP
	msgbox PokemonCenter_Text_MomHyperDone, MSGBOX_DEFAULT
	waitfanfare
	goto PokemonCenter_EventScript_MomMenu
	end

PokemonCenter_EventScript_MomHyperAlreadyPerfect::
	msgbox PokemonCenter_Text_MomHyperAlreadyPerfect, MSGBOX_DEFAULT
	goto PokemonCenter_EventScript_MomMenu
	end

PokemonCenter_EventScript_MomCantModifyEgg::
	msgbox PokemonCenter_Text_MomEgg, MSGBOX_DEFAULT
	goto PokemonCenter_EventScript_MomMenu
	end

PokemonCenter_EventScript_MomNotEnoughMoney::
	msgbox PokemonCenter_Text_MomNotEnoughMoney, MSGBOX_DEFAULT
	goto PokemonCenter_EventScript_MomMenu
	end

@ ---- Nature change: changes how stats grow (like a Mint); the real nature is kept ----
PokemonCenter_EventScript_MomNature::
	msgbox PokemonCenter_Text_MomWhichMon, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, PokemonCenter_EventScript_MomMenu
	special MomCanModifyPartyMon
	goto_if_eq VAR_RESULT, FALSE, PokemonCenter_EventScript_MomCantModifyEgg
	message PokemonCenter_Text_MomWhichNature
	waitmessage
	copyvar VAR_0x8007, VAR_0x8004
	setvar VAR_0x8004, SCROLL_MULTI_MOM_NATURES
	special ShowScrollableMultichoice
	waitstate
	copyvar VAR_0x8004, VAR_0x8007
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, PokemonCenter_EventScript_MomMenu
	copyvar VAR_0x8005, VAR_RESULT
	special MomBufferNatureChange
	goto_if_eq VAR_RESULT, FALSE, PokemonCenter_EventScript_MomSameNature
	msgbox PokemonCenter_Text_MomNatureConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PokemonCenter_EventScript_MomMenu
	special MomPayForService
	goto_if_eq VAR_RESULT, FALSE, PokemonCenter_EventScript_MomNotEnoughMoney
	special MomApplyNatureChange
	playfanfare MUS_LEVEL_UP
	msgbox PokemonCenter_Text_MomNatureDone, MSGBOX_DEFAULT
	waitfanfare
	goto PokemonCenter_EventScript_MomMenu
	end

PokemonCenter_EventScript_MomSameNature::
	msgbox PokemonCenter_Text_MomSameNature, MSGBOX_DEFAULT
	goto PokemonCenter_EventScript_MomMenu
	end

@ ---- Ability change: switch between the species' two abilities (like an Ability Capsule) ----
PokemonCenter_EventScript_MomAbility::
	msgbox PokemonCenter_Text_MomWhichMon, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, PokemonCenter_EventScript_MomMenu
	special MomCanModifyPartyMon
	goto_if_eq VAR_RESULT, FALSE, PokemonCenter_EventScript_MomCantModifyEgg
	special MomBufferAbilityChange
	goto_if_eq VAR_RESULT, FALSE, PokemonCenter_EventScript_MomOnlyOneAbility
	msgbox PokemonCenter_Text_MomAbilityConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PokemonCenter_EventScript_MomMenu
	special MomPayForService
	goto_if_eq VAR_RESULT, FALSE, PokemonCenter_EventScript_MomNotEnoughMoney
	special MomApplyAbilityChange
	playfanfare MUS_LEVEL_UP
	msgbox PokemonCenter_Text_MomAbilityDone, MSGBOX_DEFAULT
	waitfanfare
	goto PokemonCenter_EventScript_MomMenu
	end

PokemonCenter_EventScript_MomOnlyOneAbility::
	msgbox PokemonCenter_Text_MomOnlyOneAbility, MSGBOX_DEFAULT
	goto PokemonCenter_EventScript_MomMenu
	end

PokemonCenter_EventScript_MomHiddenPower::
	goto PokemonCenter_EventScript_MomNotReady
	end

PokemonCenter_EventScript_MomNotReady::
	msgbox PokemonCenter_Text_MomNotReady, MSGBOX_DEFAULT
	goto PokemonCenter_EventScript_MomMenu
	end

PokemonCenter_EventScript_MomGoodbye::
	msgbox PokemonCenter_Text_MomGoodbye, MSGBOX_DEFAULT
	release
	end

PokemonCenter_Text_MomWelcome:
	.string "Oh, {PLAYER}! What a lovely surprise!\n"
	.string "Your mother is always looking out\l"
	.string "for you, you know.$"

PokemonCenter_Text_MomWhatCanIDo:
	.string "Now, what can your mother do for\n"
	.string "you today, dear?$"

PokemonCenter_Text_MomNotReady:
	.string "Oh, I'm still learning how to do\n"
	.string "that one, dear.\p"
	.string "Ask me again a little later.$"

PokemonCenter_Text_MomGoodbye:
	.string "Take care out there, {PLAYER}!\n"
	.string "And don't forget to eat well!$"

PokemonCenter_Text_MomWhichMon:
	.string "Which POKéMON would you like me\n"
	.string "to help, dear?$"

PokemonCenter_Text_MomEgg:
	.string "Oh, that's an EGG, dear.\n"
	.string "It's much too young for that!$"

PokemonCenter_Text_MomNotEnoughMoney:
	.string "Oh dear, you don't have enough\n"
	.string "money for that.$"

PokemonCenter_Text_MomWhichStat:
	.string "Which one should I train?$"

PokemonCenter_Text_MomHyperConfirm:
	.string "I'll max out {STR_VAR_1}'s {STR_VAR_3}\n"
	.string "for ¥{STR_VAR_2}. Is that all right?$"

PokemonCenter_Text_MomHyperDone:
	.string "All done! {STR_VAR_1} has been\n"
	.string "hyper trained, dear!$"

PokemonCenter_Text_MomHyperAlreadyPerfect:
	.string "That's already as strong as it\n"
	.string "can be, dear!$"

PokemonCenter_Text_MomOnlyOneAbility:
	.string "Oh, that POKéMON has just the one\n"
	.string "ability. There's nothing to switch.$"

PokemonCenter_Text_MomAbilityConfirm:
	.string "I can switch {STR_VAR_2} for\n"
	.string "{STR_VAR_3}. That's ¥{STR_VAR_1}.\p"
	.string "Is that all right?$"

PokemonCenter_Text_MomAbilityDone:
	.string "All done! Its ability is now\n"
	.string "{STR_VAR_3}, dear!$"

PokemonCenter_Text_MomWhichNature:
	.string "Which nature would you like?$"

PokemonCenter_Text_MomNatureConfirm:
	.string "Its stats will grow like a\n"
	.string "{STR_VAR_3} POKéMON, not {STR_VAR_2}.\p"
	.string "That's ¥{STR_VAR_1}. Is that all right?$"

PokemonCenter_Text_MomNatureDone:
	.string "All done! Its stats now follow the\n"
	.string "{STR_VAR_3} nature, dear!$"

PokemonCenter_Text_MomSameNature:
	.string "Oh, its stats already follow that\n"
	.string "nature, dear!$"
