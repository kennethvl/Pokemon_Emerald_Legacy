FortreeCity_Mart_MapScripts::
	.byte 0

FortreeCity_Mart_EventScript_Clerk::
	call_if_unset FLAG_SYS_GAME_CLEAR, FortreeCity_Mart_EventScript_Clerk_MainGame
	call_if_set FLAG_SYS_GAME_CLEAR, FortreeCity_Mart_EventScript_Clerk_PostGame
	end

FortreeCity_Mart_EventScript_Clerk_MainGame::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart FortreeCity_Mart_Pokemart
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

FortreeCity_Mart_EventScript_Clerk_PostGame::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart FortreeCity_Mart_Pokemart_PostGame
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
FortreeCity_Mart_Pokemart:
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_SUPER_POTION
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_REVIVE
	.2byte ITEM_SUPER_REPEL
	.2byte ITEM_WOOD_MAIL
	.2byte ITEM_POKE_BALL
	.2byte ITEM_MASTER_BALL
	.2byte ITEM_PREMIER_BALL
	.2byte ITEM_LUXURY_BALL
	.2byte ITEM_NET_BALL
	.2byte ITEM_DIVE_BALL
	.2byte ITEM_NEST_BALL
	.2byte ITEM_REPEAT_BALL
	.2byte ITEM_TIMER_BALL
	.2byte ITEM_MOOMOO_MILK
	.2byte ITEM_ETHER
	.2byte ITEM_PP_UP
	.2byte ITEM_ELIXIR
	.2byte ITEM_MAX_ETHER
	.2byte ITEM_RARE_CANDY
	.2byte ITEM_LUCKY_EGG
	.2byte ITEM_AMULET_COIN
	.2byte ITEM_NONE
	release
	end

	.align 2
FortreeCity_Mart_Pokemart_PostGame:
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_SUPER_POTION
	.2byte ITEM_HYPER_POTION
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_REVIVE
	.2byte ITEM_SUPER_REPEL
	.2byte ITEM_WOOD_MAIL
	.2byte ITEM_SHARP_BEAK
	.2byte ITEM_POKE_BALL
	.2byte ITEM_MASTER_BALL
	.2byte ITEM_PREMIER_BALL
	.2byte ITEM_LUXURY_BALL
	.2byte ITEM_NET_BALL
	.2byte ITEM_DIVE_BALL
	.2byte ITEM_NEST_BALL
	.2byte ITEM_REPEAT_BALL
	.2byte ITEM_TIMER_BALL
	.2byte ITEM_MOOMOO_MILK
	.2byte ITEM_ETHER
	.2byte ITEM_PP_UP
	.2byte ITEM_ELIXIR
	.2byte ITEM_MAX_ETHER
	.2byte ITEM_RARE_CANDY
	.2byte ITEM_LUCKY_EGG
	.2byte ITEM_AMULET_COIN
	.2byte ITEM_NONE
	release
	end

FortreeCity_Mart_EventScript_Woman::
	msgbox FortreeCity_Mart_Text_SuperRepelBetter, MSGBOX_NPC
	end

FortreeCity_Mart_EventScript_Girl::
	msgbox FortreeCity_Mart_Text_StockUpOnItems, MSGBOX_NPC
	end

FortreeCity_Mart_EventScript_Boy::
	msgbox FortreeCity_Mart_Text_RareCandyMakesMonGrow, MSGBOX_NPC
	end

FortreeCity_Mart_Text_SuperRepelBetter:
	.string "SUPER REPEL lasts a long time,\n"
	.string "and it gets the job done.\p"
	.string "It's much better than an ordinary\n"
	.string "REPEL.$"

FortreeCity_Mart_Text_StockUpOnItems:
	.string "I always stock up on more items than\n"
	.string "I'm sure I'll need.\p"
	.string "You never know what might happen.\n"
	.string "Better to be safe than sorry!$"

FortreeCity_Mart_Text_RareCandyMakesMonGrow:
	.string "A RARE CANDY makes a POKéMON grow\n"
	.string "immediately by one level.$"

