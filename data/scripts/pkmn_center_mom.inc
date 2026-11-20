@ Mom, found in every Pokémon Center (after receiving the POKéDEX, and not in HARD / NUZLOCKE mode).
@ She offers the free "modifier" services: Hyper Training, Nature, Ability and Hidden Power type changes,
@ max EVs and the Move Relearner (level-up and Egg moves). Each service opens the PC in Mom's mode
@ (MomOpenPC in src/pokemon_storage_system.c), where the player picks any party or boxed Pokémon and the
@ service runs without leaving the PC.

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
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, PokemonCenter_EventScript_MomGoodbye
	goto_if_ge VAR_RESULT, MOM_NUM_SERVICES, PokemonCenter_EventScript_MomGoodbye
	@ The menu index is the service (MODLOGIC_MOM_SERVICE_*): open the PC for it, then come back here.
	copyvar VAR_0x8004, VAR_RESULT
	special MomOpenPC
	waitstate
	goto PokemonCenter_EventScript_MomMenu
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
