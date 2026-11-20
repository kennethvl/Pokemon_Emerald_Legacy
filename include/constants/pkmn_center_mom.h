#ifndef GUARD_CONSTANTS_PKMN_CENTER_MOM_H
#define GUARD_CONSTANTS_PKMN_CENTER_MOM_H

// Number of services in Mom's menu (the menu index is the service, see MODLOGIC_MOM_SERVICE_* in
// mon_modifier_logic.h). Items after them (CANCEL) close the menu.
#define MOM_NUM_SERVICES 6

// Hyper Training selections (order matches MENU_MOM_HYPER_* in pokemon_storage_system.c)
#define MOM_HYPER_ALL     0
#define MOM_HYPER_HP      1
#define MOM_HYPER_ATK     2
#define MOM_HYPER_DEF     3
#define MOM_HYPER_SPATK   4
#define MOM_HYPER_SPDEF   5
#define MOM_HYPER_SPEED   6

#endif // GUARD_CONSTANTS_PKMN_CENTER_MOM_H
