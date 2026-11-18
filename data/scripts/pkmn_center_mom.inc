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

PokemonCenter_EventScript_MomHyperTraining::
	goto PokemonCenter_EventScript_MomNotReady
	end

PokemonCenter_EventScript_MomNature::
	goto PokemonCenter_EventScript_MomNotReady
	end

PokemonCenter_EventScript_MomAbility::
	goto PokemonCenter_EventScript_MomNotReady
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
