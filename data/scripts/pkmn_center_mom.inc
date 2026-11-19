@ Mom, found in every Pokémon Center (after receiving the POKéDEX, and not in HARD / NUZLOCKE mode).
@ She offers the free "modifier" services: Hyper Training, Nature, Ability and Hidden Power type changes,
@ max EVs and the Move Relearner (level-up and Egg moves).

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
	special MomShowGreeting
	waitmessage
	waitbuttonpress
PokemonCenter_EventScript_MomMenu::
	special MomDisableEggMovesRelearner
	message PokemonCenter_Text_MomWhatCanIDo
	waitmessage
	setvar VAR_0x8004, SCROLL_MULTI_MOM_MAIN
	special ShowScrollableMultichoice
	waitstate
	switch VAR_RESULT
	case 0, PokemonCenter_EventScript_MomHyperTraining
	case 1, PokemonCenter_EventScript_MomNature
	case 2, PokemonCenter_EventScript_MomAbility
	case 3, PokemonCenter_EventScript_MomHiddenPower
	case 4, PokemonCenter_EventScript_MomMaxEVs
	case 5, PokemonCenter_EventScript_MomMoveRelearner
	case 6, PokemonCenter_EventScript_MomGoodbye
	case MULTI_B_PRESSED, PokemonCenter_EventScript_MomGoodbye
	goto PokemonCenter_EventScript_MomGoodbye
	end

@ ---- Hyper Training: raise IVs to 31 (free) ----
PokemonCenter_EventScript_MomHyperTraining::
	msgbox PokemonCenter_Text_MomWhichMon, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, PokemonCenter_EventScript_MomMenu
	special MomCanModifyPartyMon
	goto_if_eq VAR_RESULT, FALSE, PokemonCenter_EventScript_MomHyperTrainingEgg
PokemonCenter_EventScript_MomHyperTrainingStat::
	message PokemonCenter_Text_MomWhichStat
	waitmessage
	multichoice 0, 0, MULTI_MOM_HYPER_TRAINING, FALSE
	goto_if_eq VAR_RESULT, MOM_HYPER_CANCEL, PokemonCenter_EventScript_MomHyperTraining
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, PokemonCenter_EventScript_MomHyperTraining
	copyvar VAR_0x8005, VAR_RESULT
	special MomBufferHyperTraining
	goto_if_eq VAR_RESULT, 0, PokemonCenter_EventScript_MomHyperAlreadyPerfect
	msgbox PokemonCenter_Text_MomHyperConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PokemonCenter_EventScript_MomHyperTrainingStat
	special MomApplyHyperTraining
	playfanfare MUS_LEVEL_UP
	msgbox PokemonCenter_Text_MomHyperDone, MSGBOX_DEFAULT
	waitfanfare
	goto PokemonCenter_EventScript_MomHyperTraining
	end

PokemonCenter_EventScript_MomHyperAlreadyPerfect::
	msgbox PokemonCenter_Text_MomHyperAlreadyPerfect, MSGBOX_DEFAULT
	goto PokemonCenter_EventScript_MomHyperTrainingStat
	end

@ ---- Nature change: changes how stats grow (like a Mint); the real nature is kept ----
PokemonCenter_EventScript_MomNature::
	msgbox PokemonCenter_Text_MomWhichMon, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, PokemonCenter_EventScript_MomMenu
	special MomCanModifyPartyMon
	goto_if_eq VAR_RESULT, FALSE, PokemonCenter_EventScript_MomNatureEgg
	special MomBufferCurrentNature
	message PokemonCenter_Text_MomWhichNature
	waitmessage
	copyvar VAR_0x8007, VAR_0x8004
	setvar VAR_0x8004, SCROLL_MULTI_MOM_NATURES
	special ShowScrollableMultichoice
	waitstate
	copyvar VAR_0x8004, VAR_0x8007
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, PokemonCenter_EventScript_MomNature
	copyvar VAR_0x8005, VAR_RESULT
	special MomBufferNatureChange
	goto_if_eq VAR_RESULT, FALSE, PokemonCenter_EventScript_MomSameNature
	msgbox PokemonCenter_Text_MomNatureConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PokemonCenter_EventScript_MomNature
	special MomApplyNatureChange
	playfanfare MUS_LEVEL_UP
	msgbox PokemonCenter_Text_MomNatureDone, MSGBOX_DEFAULT
	waitfanfare
	goto PokemonCenter_EventScript_MomNature
	end

PokemonCenter_EventScript_MomSameNature::
	msgbox PokemonCenter_Text_MomSameNature, MSGBOX_DEFAULT
	goto PokemonCenter_EventScript_MomNature
	end

@ ---- Ability change: switch between the species' two abilities (like an Ability Capsule) ----
PokemonCenter_EventScript_MomAbility::
	msgbox PokemonCenter_Text_MomWhichMon, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, PokemonCenter_EventScript_MomMenu
	special MomCanModifyPartyMon
	goto_if_eq VAR_RESULT, FALSE, PokemonCenter_EventScript_MomAbilityEgg
	special MomBufferAbilityChange
	goto_if_eq VAR_RESULT, FALSE, PokemonCenter_EventScript_MomOnlyOneAbility
	msgbox PokemonCenter_Text_MomAbilityConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PokemonCenter_EventScript_MomAbility
	special MomApplyAbilityChange
	playfanfare MUS_LEVEL_UP
	msgbox PokemonCenter_Text_MomAbilityDone, MSGBOX_DEFAULT
	waitfanfare
	goto PokemonCenter_EventScript_MomAbility
	end

PokemonCenter_EventScript_MomOnlyOneAbility::
	msgbox PokemonCenter_Text_MomOnlyOneAbility, MSGBOX_DEFAULT
	goto PokemonCenter_EventScript_MomAbility
	end

@ ---- Hidden Power type change: the type no longer depends on the IVs ----
PokemonCenter_EventScript_MomHiddenPower::
	msgbox PokemonCenter_Text_MomWhichMon, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, PokemonCenter_EventScript_MomMenu
	special MomCanModifyPartyMon
	goto_if_eq VAR_RESULT, FALSE, PokemonCenter_EventScript_MomHiddenPowerEgg
	message PokemonCenter_Text_MomWhichType
	waitmessage
	copyvar VAR_0x8007, VAR_0x8004
	setvar VAR_0x8004, SCROLL_MULTI_MOM_HP_TYPES
	special ShowScrollableMultichoice
	waitstate
	copyvar VAR_0x8004, VAR_0x8007
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, PokemonCenter_EventScript_MomHiddenPower
	copyvar VAR_0x8005, VAR_RESULT
	special MomBufferHiddenPowerChange
	goto_if_eq VAR_RESULT, FALSE, PokemonCenter_EventScript_MomSameType
	msgbox PokemonCenter_Text_MomTypeConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PokemonCenter_EventScript_MomHiddenPower
	special MomApplyHiddenPowerChange
	playfanfare MUS_LEVEL_UP
	msgbox PokemonCenter_Text_MomTypeDone, MSGBOX_DEFAULT
	waitfanfare
	goto PokemonCenter_EventScript_MomHiddenPower
	end

PokemonCenter_EventScript_MomSameType::
	msgbox PokemonCenter_Text_MomSameType, MSGBOX_DEFAULT
	goto PokemonCenter_EventScript_MomHiddenPower
	end

@ ---- Max EVs: set every stat's EVs to the maximum (free) ----
PokemonCenter_EventScript_MomMaxEVs::
	msgbox PokemonCenter_Text_MomWhichMon, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, PokemonCenter_EventScript_MomMenu
	special MomCanModifyPartyMon
	goto_if_eq VAR_RESULT, FALSE, PokemonCenter_EventScript_MomMaxEVsEgg
	special MomBufferMaxEVs
	goto_if_eq VAR_RESULT, FALSE, PokemonCenter_EventScript_MomEVsAlreadyMax
	msgbox PokemonCenter_Text_MomEVConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PokemonCenter_EventScript_MomMaxEVs
	special MomApplyMaxEVs
	playfanfare MUS_LEVEL_UP
	msgbox PokemonCenter_Text_MomEVDone, MSGBOX_DEFAULT
	waitfanfare
	goto PokemonCenter_EventScript_MomMaxEVs
	end

PokemonCenter_EventScript_MomEVsAlreadyMax::
	msgbox PokemonCenter_Text_MomEVsAlreadyMax, MSGBOX_DEFAULT
	goto PokemonCenter_EventScript_MomMaxEVs
	end

@ ---- Move Relearner: reteach level-up moves and teach Egg moves (free, no HEART SCALE) ----
PokemonCenter_EventScript_MomMoveRelearner::
	clearflag FLAG_EGG_MOVES_TUTOR
	special MomEnableEggMovesRelearner
	msgbox PokemonCenter_Text_MomWhichMon, MSGBOX_DEFAULT
	special ChooseMonForMoveRelearner
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, PokemonCenter_EventScript_MomMenu
	special IsSelectedMonEgg
	goto_if_eq VAR_RESULT, TRUE, PokemonCenter_EventScript_MomMoveRelearnerEgg
	goto_if_eq VAR_0x8005, 0, PokemonCenter_EventScript_MomNoMovesToRelearn
	msgbox PokemonCenter_Text_MomWhichMove, MSGBOX_DEFAULT
	special TeachMoveRelearnerMove
	waitstate
	goto_if_eq VAR_0x8004, 0, PokemonCenter_EventScript_MomMoveRelearner
	msgbox PokemonCenter_Text_MomMoveDone, MSGBOX_DEFAULT
	goto PokemonCenter_EventScript_MomMoveRelearner
	end

PokemonCenter_EventScript_MomNoMovesToRelearn::
	msgbox PokemonCenter_Text_MomNoMoves, MSGBOX_DEFAULT
	goto PokemonCenter_EventScript_MomMoveRelearner
	end

@ ---- An Egg was picked: say so and let the player pick again for the same service ----
PokemonCenter_EventScript_MomHyperTrainingEgg::
	msgbox PokemonCenter_Text_MomEgg, MSGBOX_DEFAULT
	goto PokemonCenter_EventScript_MomHyperTraining
	end

PokemonCenter_EventScript_MomNatureEgg::
	msgbox PokemonCenter_Text_MomEgg, MSGBOX_DEFAULT
	goto PokemonCenter_EventScript_MomNature
	end

PokemonCenter_EventScript_MomAbilityEgg::
	msgbox PokemonCenter_Text_MomEgg, MSGBOX_DEFAULT
	goto PokemonCenter_EventScript_MomAbility
	end

PokemonCenter_EventScript_MomHiddenPowerEgg::
	msgbox PokemonCenter_Text_MomEgg, MSGBOX_DEFAULT
	goto PokemonCenter_EventScript_MomHiddenPower
	end

PokemonCenter_EventScript_MomMaxEVsEgg::
	msgbox PokemonCenter_Text_MomEgg, MSGBOX_DEFAULT
	goto PokemonCenter_EventScript_MomMaxEVs
	end

PokemonCenter_EventScript_MomMoveRelearnerEgg::
	msgbox PokemonCenter_Text_MomEgg, MSGBOX_DEFAULT
	goto PokemonCenter_EventScript_MomMoveRelearner
	end

PokemonCenter_EventScript_MomGoodbye::
	special MomDisableEggMovesRelearner
	special MomShowGoodbye
	waitmessage
	waitbuttonpress
	release
	end

PokemonCenter_Text_MomWhatCanIDo:
	.string "Now, what can your mother do for\n"
	.string "you today, dear?$"

PokemonCenter_Text_MomWhichMon:
	.string "Which POKéMON would you like me\n"
	.string "to help, dear?$"

PokemonCenter_Text_MomEgg:
	.string "Oh, that's an EGG, dear.\n"
	.string "It's much too young for that!$"

PokemonCenter_Text_MomWhichStat:
	.string "Which one should I train?$"

PokemonCenter_Text_MomHyperConfirm:
	.string "I'll max out {STR_VAR_1}'s {STR_VAR_3}\n"
	.string "for free. Is that all right?$"

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
	.string "{STR_VAR_3}. It's on the house!\p"
	.string "Is that all right?$"

PokemonCenter_Text_MomAbilityDone:
	.string "All done! Its ability is now\n"
	.string "{STR_VAR_3}, dear!$"

PokemonCenter_Text_MomWhichNature:
	.string "{STR_VAR_1}: {STR_VAR_2}\n"
	.string "Which nature would you like?$"

PokemonCenter_Text_MomNatureConfirm:
	.string "Its stats will grow like a\n"
	.string "{STR_VAR_3} POKéMON, not {STR_VAR_2}.\p"
	.string "It's free. Is that all right?$"

PokemonCenter_Text_MomNatureDone:
	.string "All done! Its stats now follow the\n"
	.string "{STR_VAR_3} nature, dear!$"

PokemonCenter_Text_MomSameNature:
	.string "Oh, its stats already follow that\n"
	.string "nature, dear!$"

PokemonCenter_Text_MomWhichType:
	.string "Which type should HIDDEN POWER be?$"

PokemonCenter_Text_MomTypeConfirm:
	.string "HIDDEN POWER will be {STR_VAR_3}-type\n"
	.string "instead of {STR_VAR_2}-type.\p"
	.string "It's free. Is that all right?$"

PokemonCenter_Text_MomTypeDone:
	.string "All done! HIDDEN POWER is now\n"
	.string "{STR_VAR_3}-type, dear!$"

PokemonCenter_Text_MomSameType:
	.string "Oh, HIDDEN POWER is already that\n"
	.string "type, dear!$"

PokemonCenter_Text_MomEVConfirm:
	.string "I'll give {STR_VAR_1} the training\n"
	.string "of a lifetime. Every stat, maxed!\p"
	.string "It's free. Is that all right?$"

PokemonCenter_Text_MomEVDone:
	.string "All done! {STR_VAR_1} is as trained\n"
	.string "as a POKéMON can be, dear!$"

PokemonCenter_Text_MomEVsAlreadyMax:
	.string "Oh, that POKéMON is already as\n"
	.string "trained as it can be, dear!$"

PokemonCenter_Text_MomWhichMove:
	.string "Which move should I remind it of?$"

PokemonCenter_Text_MomMoveDone:
	.string "There you go! No charge for\n"
	.string "family, dear!$"

PokemonCenter_Text_MomNoMoves:
	.string "Hmm, there's no move I can remind\n"
	.string "that POKéMON of, dear.$"
