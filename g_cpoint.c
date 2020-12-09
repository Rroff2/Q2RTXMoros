#include "g_local.h"

// Lockdown mode with security laser grid


// search through monsters
// abort if we find one with cpoint flag
// if none drop the count and search for spawn points
// spawn monster wave tagged with cpoint flag
// is it worth checking player proximity/increasing count if not?

// If a lockdown monster has lost sight of their target, find nearest route node
// and use it as the last sight position if they are close to the node then head
// for the next node (check route name)

// search the ai_routing for the mapname and if found set a level constant

// need to "wake" up ai if no player sighted/enemy to follow route

// add items
typedef struct
{
	char	*monster;
	char	*item;
	char	*coopitem;
} control_wave_monsters_t;

typedef struct
{
	control_wave_monsters_t	*spawn;
} control_wave_t;

ai_avoid_t ai_avoid_list[] = {
	{"base2", 95, 1093, 38, 48},
	{"mine2", 1852, 581, -131, 48},
	{"waste1", -120.3,1224.3,-153.9, 48},
	{"waste1", -1668.8,368.8,-153.9, 48},
	{NULL}
};

horde_point_t horde_point_list[] = {
	{"sewer64", -1666, 347, 485, HP_HORNET1}, // air boss spawn outside
	{"sewer64", 776, 462, 684, HP_HORNET2}, // air boss spawn inside
	{"sewer64", 2896, -696, 471, HP_COMMANDER1}, // tank commander spawn
	{"sewer64", 1631, -743, 598, HP_COMMANDER2}, // tank commander spawn
	{"sewer64", 3115, -569, 180, HP_SUPERTANK1}, // supertank spawn
	{NULL}
};

// monster classname can be "skip" or "random"

control_wave_monsters_t wave1[] = {
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", NULL, NULL},
	{NULL}
};

control_wave_monsters_t wave2[] = {
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_infantry", "ammo_bullets", NULL},
	{NULL}
};

control_wave_monsters_t wave3[] = {
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_infantry", "item_heal_drops", NULL},
	{"monster_infantry", NULL, NULL},
	{NULL}
};

control_wave_monsters_t wave4[] = {
	{"monster_soldier_ss", "item_health_drops", NULL},
	{"monster_soldier", "item_health_drops", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_gunner", "item_armor_jacket", NULL},
	{NULL}
};

control_wave_monsters_t wave5[] = {
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_gunner", "ammo_grenades", NULL},
	{"monster_gunner", "item_adrenaline", NULL},
	{NULL}
};

control_wave_monsters_t wave6[] = {
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{NULL}
};

control_wave_monsters_t wave7[] = {
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", "ammo_bullets", NULL},
	{"monster_gunner", "item_health_drops", NULL},
	{"monster_gunner", "item_health_drops", NULL},
	{"monster_gunner", "item_health_drops", NULL},
	{"monster_gladiator", "item_armor_combat", NULL},
	{NULL}
};

control_wave_monsters_t wave8[] = {
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{NULL}
};

control_wave_monsters_t wave9[] = {
	{"monster_infantry", "ammo_bullets", NULL},
	{"monster_infantry", "item_health_drops", NULL},
	{"monster_infantry", "ammo_bullets", NULL},
	{"monster_infantry", "item_health_drops", NULL},
	{"monster_infantry", "item_health_drops", NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_gunner", "item_health_drops", NULL},
	{"monster_gunner", "ammo_bullets", NULL},
	{"monster_gunner", "item_armor_shard", NULL},
	{NULL}
};

control_wave_monsters_t wave10[] = {
	{"monster_gladiator", "item_armor_shard", NULL},
	{"monster_gladiator", "item_armor_shard", NULL},
	{"monster_infantry", "ammo_bullets", NULL},
	{"monster_infantry", "ammo_bullets", NULL},
	{"monster_infantry", "ammo_bullets", NULL},
	{"monster_gladiator", "item_armor_combat", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_tank", "item_health_drops", NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_light", "item_adrenaline", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{NULL}
};

// hard

control_wave_monsters_t wave1h[] = {
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "item_armor_shard", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "item_armor_shard", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{NULL}
};

control_wave_monsters_t wave2h[] = {
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_infantry", "ammo_bullets", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "item_health_drops", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "item_armor_shard", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{NULL}
};

control_wave_monsters_t wave3h[] = {
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_infantry", "item_heal_drops", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{NULL}
};

control_wave_monsters_t wave4h[] = {
	{"monster_soldier_ss", "item_health_drops", NULL},
	{"monster_soldier", "item_health_drops", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_gunner", "item_armor_jacket", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "item_armor_shard", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "item_armor_shard", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{NULL}
};

control_wave_monsters_t wave5h[] = {
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_gunner", "ammo_grenades", NULL},
	{"monster_gunner", "item_adrenaline", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "item_armor_shard", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{NULL}
};

control_wave_monsters_t wave6h[] = {
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{NULL}
};

control_wave_monsters_t wave7h[] = {
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", "ammo_bullets", NULL},
	{"monster_gunner", "item_health_drops", NULL},
	{"monster_gunner", "item_health_drops", NULL},
	{"monster_gunner", "item_health_drops", NULL},
	{"monster_gladiator", "item_armor_combat", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "item_armor_shard", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "item_armor_shard", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "item_armor_shard", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{NULL}
};

control_wave_monsters_t wave8h[] = {
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "item_armor_shard", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{NULL}
};

control_wave_monsters_t wave9h[] = {
	{"monster_infantry", "ammo_bullets", NULL},
	{"monster_infantry", "item_health_drops", NULL},
	{"monster_infantry", "ammo_bullets", NULL},
	{"monster_infantry", "item_health_drops", NULL},
	{"monster_infantry", "item_health_drops", NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_gunner", "item_health_drops", NULL},
	{"monster_gunner", "ammo_bullets", NULL},
	{"monster_gunner", "item_armor_shard", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "item_armor_shard", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{NULL}
};

control_wave_monsters_t wave10h[] = {
	{"monster_gladiator", "item_armor_shard", NULL},
	{"monster_gladiator", "item_armor_shard", NULL},
	{"monster_infantry", "ammo_bullets", NULL},
	{"monster_infantry", "ammo_bullets", NULL},
	{"monster_infantry", "ammo_bullets", NULL},
	{"monster_gladiator", "item_armor_combat", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_tank", "item_health_drops", NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_light", "item_adrenaline", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "item_armor_shard", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{NULL}
};

// end hard

control_wave_monsters_t wave1waste1[] = {
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier", "item_health_dropm", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier", "item_health_dropm", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_chick", "ammo_rockets", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{NULL}
};

control_wave_monsters_t wave2waste1[] = {
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier", "item_health_dropm", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_chick", "item_adrenaline", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier", "item_health_dropm", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_chick", "ammo_rockets", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{NULL}
};

control_wave_monsters_t wave3waste1[] = {
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_gladiator", "item_armor_combat", NULL},
	{"monster_soldier", "item_health_dropm", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_gladiator", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier", "item_health_dropl", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", "ammo_rockets", NULL},
	{"monster_gladiator", "ammo_slugs", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{NULL}
};

control_wave_monsters_t wave4waste1[] = {

	{"monster_mutant", NULL, NULL},
	{"monster_mutant", NULL, NULL},
	{"monster_mutant", NULL, NULL},
	{"monster_parasite", NULL, NULL},
	{"monster_parasite", NULL, NULL},
	{"monster_parasite", NULL, NULL},
	{"monster_parasite", NULL, NULL},
	{"monster_medic", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_armor_shard", NULL},
	{"monster_soldier_light", "item_armor_shard", NULL},
	{"monster_soldier_light", "item_armor_shard", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_mutant", NULL, NULL},
	{"monster_mutant", NULL, NULL},
	{"monster_parasite", NULL, NULL},
	{"monster_soldier_light", "item_armor_shard", NULL},
	{"monster_soldier_light", "item_armor_shard", NULL},
	{NULL}
};

control_wave_monsters_t wave5waste1[] = {
	{"monster_gunner", "ammo_bullets", NULL},
	{"monster_gunner", "ammo_bullets", NULL},
	{"monster_gunner", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_gunner", NULL, NULL},
	{"monster_gunner", NULL, NULL},
	{"monster_gunner", NULL, NULL},
	{"monster_gunner", NULL, NULL},
	{"monster_medic", NULL, NULL},
	{"monster_gunner", NULL, NULL},
	{"monster_medic", NULL, NULL},
	{"monster_gunner", NULL, NULL},
	{"monster_chick", "ammo_rockets", NULL},
	{"monster_gunner", NULL, NULL},
	{"monster_chick", "ammo_rockets", NULL},
	{"monster_gunner", "ammo_grenades", NULL},
	{"monster_gunner", NULL, NULL},
	{"monster_gladiator", "ammo_slugs", NULL},
	{"monster_gunner", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{NULL}
};

control_wave_monsters_t wave6waste1[] = {
	{"monster_gunner", "ammo_bullets", NULL},
	{"monster_gunner", NULL, NULL},
	{"monster_gunner", "ammo_grenades", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_gunner", "ammo_bullets", NULL},
	{"monster_gunner", NULL, NULL},
	{"monster_gunner", NULL, NULL},
	{"monster_gunner", "ammo_bullets", NULL},
	{"monster_medic", NULL, NULL},
	{"monster_gunner", NULL, NULL},
	{"monster_medic", NULL, NULL},
	{"monster_berserk", NULL, NULL},
	{"monster_gunner", NULL, NULL},
	{"monster_chick", NULL, NULL},
	{"monster_gunner", "ammo_bullets", NULL},
	{"monster_chick", NULL, NULL},
	{"monster_gunner", "ammo_bullets", NULL},
	{"monster_gunner", "ammo_bullets", NULL},
	{"monster_berserk", NULL, NULL},
	{"monster_gladiator", "item_armor_body", NULL},
	{"monster_gunner", "ammo_grenades", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_berserk", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{NULL}
};

control_wave_monsters_t wave7waste1[] = {
	{"monster_gunner", "item_armor_shard", NULL},
	{"monster_gunner", "item_armor_shard", NULL},
	{"monster_gunner", "item_armor_shard", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_gunner", "item_armor_shard", NULL},
	{"monster_gunner", "item_armor_shard", NULL},
	{"monster_gunner", "item_armor_shard", NULL},
	{"monster_gunner", "item_armor_shard", NULL},
	{"monster_medic", NULL, NULL},
	{"monster_gunner", "item_armor_shard", NULL},
	{"monster_medic", NULL, NULL},
	{"monster_tank", NULL, NULL},
	{"monster_gunner", "item_armor_shard", NULL},
	{"monster_chick", NULL, NULL},
	{"monster_gunner", "item_armor_shard", NULL},
	{"monster_chick", NULL, NULL},
	{"monster_gunner", "item_armor_shard", NULL},
	{"monster_gunner", NULL, NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_gladiator", "ammo_slugs", NULL},
	{"monster_gunner", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_tank", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{NULL}
};

control_wave_monsters_t wave8waste1[] = {
	{"monster_chick", "ammo_rockets", NULL},
	{"monster_chick", NULL, NULL},
	{"monster_chick", "ammo_rockets", NULL},
	{"monster_chick", NULL, NULL},
	{"monster_chick", NULL, NULL},
	{"monster_chick", "ammo_rockets", NULL},
	{"monster_chick", NULL, NULL},
	{"monster_chick", "ammo_rockets", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_medic", NULL, NULL},
	{"monster_medic", NULL, NULL},
	{"monster_medic", NULL, NULL},
	{"monster_parasite", "item_adrenaline", NULL},
	{"monster_parasite", NULL, NULL},
	{NULL}
};

control_wave_monsters_t wave9waste1[] = {
	{"monster_chick", NULL, NULL},
	{"monster_gladiator", NULL, NULL},
	{"monster_floater", "ammo_cells", NULL},
	{"monster_hover", "ammo_cells", NULL},
	{"monster_hover", "ammo_cells", NULL},
	{"monster_chick", "item_quad", NULL},
	{"monster_floater", "ammo_cells", NULL},
	{"monster_gladiator", "item_armor_combat", NULL},
	{"monster_gladiator", "ammo_slugs", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_hover", "ammo_cells", NULL},
	{NULL}
};

control_wave_monsters_t wave10waste1[] = {
	{"monster_brain", "ammo_cells", NULL},
	{"monster_gladiator", "ammo_slugs", NULL},
	{"monster_brain", NULL, NULL},
	{"monster_tank", NULL, NULL},
	{"monster_gunner", "ammo_grenades", NULL},
	{"monster_gunner", NULL, NULL},
	{"monster_gunner", "ammo_grenades", NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_tank", NULL, NULL},
	{"monster_gunner", NULL, NULL},
	{"monster_gunner", NULL, NULL},
	{"monster_berserk", "item_armor_shard", NULL},
	{"monster_tank_commander", NULL, NULL},
	{"monster_brain", "item_adrenaline", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_infantry", "ammo_bullets", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", "ammo_bullets", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", "ammo_bullets", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", "ammo_bullets", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", "ammo_bullets", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", "ammo_bullets", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{NULL}
};

control_wave_monsters_t wave1fact1[] = {
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{NULL}
};

control_wave_monsters_t wave2fact1[] = {
	{"monster_soldier_ss", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_ss", "item_health_drops", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier_ss", NULL, NULL},
	{NULL}
};

control_wave_monsters_t wave3fact1[] = {
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", "item_armor_shard", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", "item_resupply", NULL},
	{NULL}
};

control_wave_monsters_t wave4fact1[] = {
	{"monster_gunner", "ammo_grenades", NULL},
	{"monster_gunner", "ammo_grenades", NULL},
	{"monster_gunner", "ammo_bullets", NULL},
	{"monster_berserk", NULL, NULL},
	{"monster_gunner", "ammo_grenades", NULL},
	{"monster_gunner", "ammo_grenades", NULL},
	{"monster_gunner", "ammo_bullets", NULL},
	{"monster_berserk", NULL, NULL},
	{"monster_berserk", "item_quad", NULL},
	{NULL}
};

control_wave_monsters_t wave5fact1[] = {
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_parasite", "item_adrenaline", NULL},
	{"monster_parasite", NULL, NULL},
	{"monster_parasite", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_parasite", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{NULL}
};

control_wave_monsters_t wave6fact1[] = {
	{"monster_soldier_ss", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_ss", "item_health_drops", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier_light", NULL, NULL},
	{NULL}
};

control_wave_monsters_t wave7fact1[] = {
	{"monster_soldier_ss", NULL, NULL},
	{"monster_flyer", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_flyer", NULL, NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_flyer", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_flyer", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_flyer", "ammo_cells", NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_flyer", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_flyer", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_flyer", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_flyer", "ammo_cells", NULL},
	{NULL}
};

control_wave_monsters_t wave8fact1[] = {
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", "ammo_bullets", NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_infantry", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{NULL}
};

control_wave_monsters_t wave9fact1[] = {
	{"monster_berserk", NULL, NULL},
	{"monster_berserk", NULL, NULL},
	{"monster_berserk", NULL, NULL},
	{"monster_berserk", NULL, NULL},
	{"monster_gunner", "ammo_grenades", NULL},
	{"monster_chick", "ammo_cells", NULL},
	{"monster_gunner", "ammo_bullets", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", NULL, NULL},
	{NULL}
};

control_wave_monsters_t wave10fact1[] = {
	//{"monster_supertank", "key_tag", NULL},
	{"monster_soldier_ss", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", "ammo_bullets", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_ss", "item_health_drops", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier", "ammo_shells", NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_ss", "ammo_grenades", NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier_light", "item_health_drops", NULL},
	{"monster_soldier", NULL, NULL},
	{"monster_soldier_ss", NULL, NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_berkserk", NULL, NULL},
	{"monster_soldier_light", NULL, NULL},
	{"monster_berkserk", NULL, NULL},
	{NULL}
};

control_wave_monsters_t wave1forever1a[] = {
	{"random", NULL, NULL},
	{NULL}
};

control_wave_monsters_t wave1forever1[] = {
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{NULL}
};

control_wave_monsters_t wave1forever2[] = {
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{NULL}
};

control_wave_monsters_t wave1forever3[] = {
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{NULL}
};

control_wave_monsters_t wave1forever4[] = {
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{NULL}
};

control_wave_monsters_t wave1forever5[] = {
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{NULL}
};

control_wave_monsters_t wave1forever6[] = {
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{"random", NULL, NULL},
	{NULL}
};

//list in reverse order as count starts at 10

//static const control_wave_t control_wave_list[] = {

control_wave_t control_wave_list_easy[] = {
	{&wave10},
	{&wave9},
	{&wave8},
	{&wave7},
	{&wave6},
	{&wave5},
	{&wave4},
	{&wave3},
	{&wave2},
	{&wave1}
};

control_wave_t control_wave_list_medium[] = {
	{&wave10},
	{&wave9},
	{&wave8},
	{&wave7},
	{&wave6},
	{&wave5},
	{&wave4},
	{&wave3},
	{&wave2},
	{&wave1}
};

control_wave_t control_wave_list_hard[] = {
	{&wave10h},
	{&wave9h},
	{&wave8h},
	{&wave7h},
	{&wave6h},
	{&wave5h},
	{&wave4h},
	{&wave3h},
	{&wave2h},
	{&wave1h}
};

control_wave_t control_wave_list_waste1[] = {
	{&wave10waste1},
	{&wave9waste1},
	{&wave8waste1},
	{&wave7waste1},
	{&wave6waste1},
	{&wave5waste1},
	{&wave4waste1},
	{&wave3waste1},
	{&wave2waste1},
	{&wave1waste1}
};

control_wave_t control_wave_list_fact1[] = {
	{&wave10fact1},
	{&wave9fact1},
	{&wave8fact1},
	{&wave7fact1},
	{&wave6fact1},
	{&wave5fact1},
	{&wave4fact1},
	{&wave3fact1},
	{&wave2fact1},
	{&wave1fact1}
};

control_wave_t control_wave_list_forever[] = {
	{&wave1forever5},
	{&wave1forever4},
	{&wave1forever3},
	{&wave1forever3},
	{&wave1forever3},
	{&wave1forever2},
	{&wave1forever2},
	{&wave1forever2},
	{&wave1forever1},
	{&wave1forever1}
};

spawn_point_t *custom_spawn_list;

spawn_point_t spawn_point_list[] =
{
	/*	{
			qtrue,
			"testing1",
			0, 0, 0,
			0, 0, 0,
			0
		},

		{
			qtrue,
			"testing2",
			0, 0, 0,
			0, 0, 0,
			0
		},

		{ qfalse, "divider", 0, 0, 0, 0, 0, 0, 0 },*/

		// don't use first index

		{ qfalse, "divider", 0, 0, 0, 0, 0, 0, 0 },

		// additional spawns

		{
			qtrue,
			"base2",
			650.9,2101.5,-209.9,
			0.0,130.2,0.0,
			0
		},

		{
			qtrue,
			"base2",
			580.0,2295.3,-209.9,
			0.0,173.0,0.0,
			0
		},

		{
			qtrue,
			"base2",
			492.3,2507.3,-209.9,
			0.0,236.6,0.0,
			0
		},

		{
			qtrue,
			"base2",
			289.3,2111.3,-209.9,
			0.0,73.7,0.0,
			0
		},

		{
			qtrue,
			"base2",
			42.4,2142.9,-145.9,
			0.0,89.1,0.0,
			0
		},

		{
			qtrue,
			"base2",
			41.0,2478.6,-145.9,
			0.0,285.6,0.0,
			0
		},

		{
			qtrue,
			"base2",
			-54.5,2308.8,-145.9,
			0.0,219.3,0.0,
			0
		},

		{
			qtrue,
			"base2",
			459.8,1779.9,-145.9,
			0.0,176.5,0.0,
			0
		},

		{
			qtrue,
			"base2",
			680.0,1630.4,46.1,
			0.0,179.3,0.0,
			0
		},

		{
			qtrue,
			"base2",
			88.1,1812.5,46.1,
			0.0,4.7,0.0,
			0
		},

		{
			qtrue,
			"base2",
			290.8,2466.5,54.1,
			0.0,212.3,0.0,
			0
		},

		{
			qtrue,
			"base2",
			290.0,2192.4,54.1,
			0.0,259.2,0.0,
			0
		},

	// original list starts
	{
		qtrue,
		"base2",
		582.6,1536.1,-145.9,
		0.0,232.6,0.0,
		0
	},

	{
		qtrue,
		"base2",
		573.8,1418.6,-145.9,
		0.0,177.5,0.0,
		0
	},

	{
		qtrue,
		"base2",
		565.9,1303.0,-145.9,
		0.0,177.5,0.0,
		0
	},

	{
		qtrue,
		"base2",
		295.3,1286.3,-145.9,
		0.0,87.4,0.0,
		0
	},

	{
		qtrue,
		"base2",
		108.0,1269.1,-145.9,
		0.0,56.3,0.0,
		0
	},

	{
		qtrue,
		"base2",
		63.8,1478.6,-145.9,
		0.0,353.7,0.0,
		0
	},

	{
		qtrue,
		"base2",
		67.8,1402.5,46.1,
		0.0,25.1,0.0,
		0
	},

	{
		qtrue,
		"base2",
		195.8,1240.5,46.1,
		0.0,86.3,0.0,
		0
	},

	{
		qtrue,
		"base2",
		377.8,1229.5,46.1,
		0.0,88.7,0.0,
		0
	},

	{
		qtrue,
		"base2",
		584.5,1216.1,46.1,
		0.0,135.0,0.0,
		0
	},

	{
		qtrue,
		"base2",
		574.8,1461.8,46.1,
		0.0,180.2,0.0,
		0
	},

	{
		qtrue,
		"base2",
		572.1,1641.5,46.1,
		0.0,179.7,0.0,
		0
	},

	{
		qtrue,
		"base2",
		577.0,1765.8,46.1,
		0.0,265.8,0.0,
		0
	},

	{
		qtrue,
		"base2",
		62.5,1983.3,46.3,
		0.0,348.6,0.0,
		0
	},

	{
		qtrue,
		"base2",
		-48.6,2146.6,46.3,
		0.0,0.2,0.0,
		0
	},

	{
		qtrue,
		"base2",
		265.0,2032.5,56.1,
		0.0,226.3,0.0,
		0
	},

	{
		qtrue,
		"base2",
		-139.9,2336.4,-145.9,
		0.0,266.1,0.0,
		0
	},

	{
		qtrue,
		"base2",
		-192.4,2236.9,-145.9,
		0.0,266.1,0.0,
		0
	},

	{
		qtrue,
		"base2",
		-196.1,2146.8,-145.9,
		0.0,266.1,0.0,
		0
	},

	{
		qtrue,
		"base2",
		-200.6,2042.0,-145.9,
		0.0,266.1,0.0,
		0
	},

	{
		qtrue,
		"base2",
		-203.3,1972.9,-145.9,
		0.0,307.5,0.0,
		0
	},

	{
		qtrue,
		"base2",
		-147.3,1728.4,-145.9,
		0.0,52.2,0.0,
		0
	},

	{
		qtrue,
		"base2",
		-162.5,1833.5,-145.9,
		0.0,3.5,0.0,
		0
	},

	{
		qtrue,
		"base2",
		0.4,1855.8,-145.9,
		0.0,172.7,0.0,
		0
	},

	{
		qtrue,
		"base2",
		93.4,1785.8,-145.9,
		0.0,1.2,0.0,
		0
	},

	{
		qtrue,
		"base2",
		267.9,1789.9,-145.9,
		0.0,282.8,0.0,
		0
	},

	{ qfalse, "divider", 0, 0, 0, 0, 0, 0, 0 },

	{
		qtrue,
		"mine2",
		1868.9,524,-137.9, // might be a bit close to laser avoid spot
		0.0,264.4,0.0,
		0
	},

	{
		qtrue,
		"mine2",
		1828.6,484.0,-137.9,
		0.0,264.4,0.0,
		0
	},

	{
		qtrue,
		"mine2",
		1869.8,435.0,-137.9,
		0.0,264.4,0.0,
		0
	},

	{
		qtrue,
		"mine2",
		1861.6,370.0,-137.9,
		0.0,264.4,0.0,
		0
	},

	{
		qtrue,
		"mine2",
		1845.6,251.3,-137.9,
		0.0,229.9,0.0,
		0
	},

	{
		qtrue,
		"mine2",
		1798.4,159.8,-137.9,
		0.0,177.2,0.0,
		0
	},

	{
		qtrue,
		"mine2",
		1688.8,162.1,-137.9,
		0.0,177.2,0.0,
		0
	},

	{
		qtrue,
		"mine2",
		1635.0,257.4,-137.9,
		0.0,177.6,0.0,
		0
	},

	{
		qtrue,
		"mine2",
		1451.0,166.0,-137.9,
		0.0,0.6,0.0,
		0
	},

	{
		qtrue,
		"mine2",
		1287.8,166.0,-137.9,
		0.0,356.2,0.0,
		0
	},

	{
		qtrue,
		"mine2",
		1204.1,164.6,-137.9,
		0.0,353.8,0.0,
		0
	},

	{
		qtrue,
		"mine2",
		1249.3,-211.9,-57.9,
		0.0,61.1,0.0,
		0
	},

	{
		qtrue,
		"mine2",
		1439.6,-252.6,-57.9,
		0.0,111.1,0.0,
		0
	},

	{
		qtrue,
		"mine2",
		1344.0,-385.6,-57.9,
		0.0,89.7,0.0,
		0
	},

	{
		qtrue,
		"mine2",
		1101.0,-265.3,-57.9,
		0.0,181.7,0.0,
		0
	},

	{
		qtrue,
		"mine2",
		980.1,-295.4,-57.9,
		0.0,322.9,0.0,
		0
	},

	{
		qtrue,
		"mine2",
		950.4,-416.1,-57.9,
		0.0,261.0,0.0,
		0
	},

	{
		qtrue,
		"mine2",
		943.5,-525.6,-57.9,
		0.0,270.1,0.0,
		0
	},

	{
		qtrue,
		"mine2",
		798.9,-650.9,-57.9,
		0.0,359.9,0.0,
		0
	},

	{ qfalse, "divider", 0, 0, 0, 0, 0, 0, 0 },

	{
		qtrue,
		"waste1",
		-2516.6,-920.9,-417.9,
		0.0,309.2,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2326.5,-922.1,-417.9,
		0.0,269.2,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2326.5,-1051.4,-417.9,
		0.0,269.2,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2481.8,-1144.6,-417.9,
		0.0,315.3,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2396.0,-1430.1,-417.9,
		0.0,0.7,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2307.5,-1537.4,-417.9,
		0.0,43.3,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2182.5,-1552.3,-417.9,
		0.0,85.2,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2003.8,-1422.4,-417.9,
		0.0,0.9,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1874.4,-1385.0,-417.9,
		0.0,359.3,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1725.3,-1408.4,-417.9,
		0.0,352.8,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1462.3,-1393.1,-417.9,
		0.0,142.5,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1243.3,-999.9,-593.9,
		0.0,221.5,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1478.6,-971.4,-593.9,
		0.0,269.3,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1489.0,-1133.0,-593.9,
		0.0,264.8,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1354.4,-1356.9,-593.9,
		0.0,119.0,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1870.5,-1308.1,-593.9,
		0.0,179.7,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1755.8,-1435.4,-593.9,
		0.0,358.5,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2119.3,-1426.1,-593.9,
		0.0,128.7,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2013.9,-1123.8,-593.9,
		0.0,147.9,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2111.6,-954.6,-593.9,
		0.0,268.1,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2435.0,-909.0,-593.9,
		0.0,304.1,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2437.6,-1123.9,-593.9,
		0.0,338.0,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2350.4,-1355.1,-593.9,
		0.0,17.9,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2216.6,-1413.4,-593.9,
		0.0,43.5,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1201.5,-766.6,-577.9,
		0.0,178.6,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1436.3,-756.9,-577.9,
		0.0,120.2,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1623.4,-758.0,-577.9,
		0.0,183.6,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1835.8,-766.5,-577.9,
		0.0,89.9,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2048.8,-702.5,-577.9,
		0.0,41.2,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1823.4,-544.9,-713.9,
		0.0,88.7,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1361.5,-593.0,-713.9,
		0.0,111.9,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1194.4,-483.1,-713.9,
		0.0,135.0,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1479.9,-638.6,-713.9,
		0.0,171.7,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1788.9,-199.3,-741.9,
		0.0,213.4,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1809.1,-279.3,-741.9,
		0.0,178.1,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1971.3,-261.4,-737.9,
		0.0,177.5,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2060.5,-256.5,-737.9,
		0.0,178.6,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2364.0,-362.0,-737.9,
		0.0,215.2,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2377.5,-143.4,-737.9,
		0.0,215.1,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2531.5,-251.6,-737.9,
		0.0,299.8,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2544.8,-393.9,-737.9,
		0.0,291.0,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2505.5,-560.6,-737.9,
		0.0,358.9,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2218.5,-1000.5,-741.9,
		0.0,95.8,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2373.8,-964.6,-741.9,
		0.0,85.0,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2202.5,-1150.5,-741.9,
		0.0,93.5,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2200.0,-1330.5,-741.9,
		0.0,267.7,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2202.8,-1474.1,-741.9,
		0.0,268.5,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2138.6,-1961.0,-733.9,
		0.0,94.8,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2266.5,-1876.5,-733.9,
		0.0,87.5,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1595.4,775.6,-153.9,
		0.0,322.2,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1501.1,811.5,-153.9,
		0.0,68.4,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1662.9,625.0,-153.9,
		0.0,3.6,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1642.3,498.0,-153.9,
		0.0,18.9,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1515.1,538.0,-153.9,
		0.0,18.9,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1250.3,626.5,-153.9,
		0.0,178.9,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1177.0,565.6,-153.9,
		0.0,174.0,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1062.4,553.3,-153.9,
		0.0,355.0,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-983.4,628.3,-153.9,
		0.0,356.0,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-778.6,766.4,-153.9,
		0.0,85.2,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-747.1,885.5,-153.9,
		0.0,42.7,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-741.3,1009.3,-153.9,
		0.0,320.3,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-587.5,975.1,-153.9,
		0.0,358.3,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-490.3,944.6,-153.9,
		0.0,351.6,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-300.5,970.1,-153.9,
		0.0,354.9,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-698.4,491.0,-153.9,
		0.0,266.7,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-764.1,372.5,-153.9,
		0.0,257.9,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-736.6,205.8,-153.9,
		0.0,296.4,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-726.3,92.3,-153.9,
		0.0,273.4,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-612.8,80.6,-153.9,
		0.0,356.9,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-471.5,143.5,-153.9,
		0.0,183.9,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-323.3,60.3,-153.9,
		0.0,354.0,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-188.6,110.1,-153.9,
		0.0,358.5,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-32.3,71.8,-153.9,
		0.0,348.5,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		348.0,191.5,-153.9,
		0.0,173.3,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		292.9,8.5,-153.9,
		0.0,174.3,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-719.6,-118.0,-153.9,
		0.0,255.2,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-803.8,-251.4,-153.9,
		0.0,181.8,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-979.6,-269.0,-153.9,
		0.0,181.6,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1182.0,-305.4,-153.9,
		0.0,204.1,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1470.9,-234.4,-153.9,
		0.0,173.6,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1650.5,-270.4,-153.9,
		0.0,180.4,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1853.6,-261.1,-153.9,
		0.0,143.2,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1908.8,-73.8,-153.9,
		0.0,270.5,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2131.9,-255.0,-153.9,
		0.0,178.7,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2229.9,-274.4,-153.9,
		0.0,178.3,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2396.1,-413.5,-153.9,
		0.0,269.9,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2415.4,-543.1,-153.9,
		0.0,263.5,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2310.0,-689.4,-153.9,
		0.0,241.0,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-2566.3,-742.4,-153.9,
		0.0,293.1,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1198.9,-439.8,-257.9,
		0.0,214.5,0.0,
		0
	},

	{
		qtrue,
		"waste1",
		-1182.8,-29.5,-257.9,
		0.0,184.3,0.0,
		0
	},

	{ qfalse, "divider", 0, 0, 0, 0, 0, 0, 0 },

	{
		qtrue,
		"fact1",
		-565.6,632.9,46.1,
		0.0,312.3,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		-674.0,632.9,46.1,
		0.0,233.3,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		-605.4,748.9,46.1,
		0.0,74.8,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		-529.9,910.3,46.1,
		0.0,356.4,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		-394.8,879.3,46.1,
		0.0,355.0,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		-237.5,913.6,46.1,
		0.0,1.3,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		-43.1,918.3,46.1,
		0.0,253.9,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		-7.0,823.0,46.1,
		0.0,222.2,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		1.1,620.6,46.1,
		0.0,229.3,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		141.9,449.5,46.1,
		0.0,173.4,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		122.5,374.4,46.1,
		0.0,178.1,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		4.1,333.5,46.1,
		0.0,263.6,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		-221.8,187.9,46.1,
		0.0,346.3,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		-353.3,56.4,46.1,
		0.0,9.2,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		-549.0,41.6,46.1,
		0.0,1.4,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		-359.0,246.0,46.1,
		0.0,343.0,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		107.8,94.3,46.1,
		0.0,178.6,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		-89.3,-109.0,46.1,
		0.0,358.3,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		10.3,-136.3,46.1,
		0.0,106.9,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		-111.9,-390.0,46.1,
		0.0,0.2,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		3.6,-302.0,46.1,
		0.0,354.8,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		143.3,-309.6,46.1,
		0.0,356.6,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		331.1,-329.5,46.1,
		0.0,187.2,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		220.9,-581.0,46.1,
		0.0,85.7,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		439.3,-559.8,46.1,
		0.0,175.9,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		77.5,-1147.0,46.1,
		0.0,0.4,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		492.9,-1142.9,46.1,
		0.0,358.4,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		995.1,-1142.3,46.1,
		0.0,180.0,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		601.5,418.1,-533.9,
		0.0,0.5,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		764.9,418.9,-533.9,
		0.0,1.7,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		849.0,418.9,-533.9,
		0.0,0.8,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		958.3,546.6,-533.9,
		0.0,269.2,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		956.5,602.8,-533.9,
		0.0,86.5,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		752.0,1052.5,-649.9,
		0.0,7.9,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		792.9,1356.5,-649.9,
		0.0,321.3,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		1029.8,1355.0,-649.9,
		0.0,297.9,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		1469.5,1178.5,-649.9,
		0.0,177.5,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		1475.0,875.0,-649.9,
		0.0,269.5,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		1446.9,975.6,-649.9,
		0.0,80.0,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		1353.0,668.6,-81.9,
		0.0,256.0,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		1338.5,527.4,-81.9,
		0.0,271.9,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		1087.5,509.0,-81.9,
		0.0,306.3,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		1259.1,213.9,-81.9,
		0.0,355.0,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		1472.1,195.5,-81.9,
		0.0,357.7,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		1573.1,38.1,-81.9,
		0.0,90.5,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		1625.3,-118.1,-81.9,
		0.0,87.5,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		1601.3,-291.8,-81.9,
		0.0,92.1,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		1445.9,406.9,46.1,
		0.0,174.3,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		1591.1,734.1,-65.9,
		0.0,93.1,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		1582.6,957.8,-49.9,
		0.0,180.2,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		1641.9,418.1,-65.9,
		0.0,96.7,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		1087.1,950.3,14.1,
		0.0,1.1,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		942.9,942.4,14.1,
		0.0,180.8,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		730.0,964.5,14.1,
		0.0,177.9,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		260.1,940.5,-1.9,
		0.0,265.6,0.0,
		0
	},

	{
		qtrue,
		"fact1",
		193.3,630.0,46.1,
		0.0,157.3,0.0,
		0
	},

	{ qfalse, "divider", 0, 0, 0, 0, 0, 0, 0 },

	// Sewer64 begins...

	{
		qtrue,
		"sewer64",
		-698.8,548.5,558.1,
		0.0,51.9,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-588.5,652.4,558.1,
		0.0,58.5,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-427.9,489.5,558.1,
		0.0,114.7,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-438.9,776.6,558.1,
		0.0,291.9,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-441.8,953.9,558.1,
		0.0,91.2,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-448.0,1156.0,558.1,
		0.0,93.0,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-448.0,1361.3,558.1,
		0.0,90.8,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-610.3,1541.5,558.1,
		0.0,245.7,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-511.8,1648.9,558.1,
		0.0,305.5,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-705.6,1169.3,558.1,
		0.0,267.7,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-693.4,905.0,558.1,
		0.0,270.6,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-718.0,1634.8,558.1,
		0.0,333.4,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-194.4,1889.1,558.1,
		0.0,190.3,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-184.0,1664.3,558.1,
		0.0,207.1,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		192.1,1667.6,558.1,
		0.0,141.9,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		164.3,1858.1,558.1,
		0.0,180.1,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		193.1,2048.1,558.1,
		0.0,82.7,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		58.5,2172.1,558.1,
		0.0,176.5,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-61.1,2087.5,558.1,
		0.0,273.7,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-67.1,2297.4,558.1,
		0.0,91.2,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		52.3,2422.6,558.1,
		0.0,12.5,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		178.4,2521.8,558.1,
		0.0,85.8,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		158.1,2778.1,558.1,
		0.0,6.7,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		43.9,2689.5,558.1,
		0.0,272.1,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-82.8,2858.9,558.1,
		0.0,290.1,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-74.5,2562.8,558.1,
		0.0,271.8,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		323.5,2817.5,574.1,
		0.0,187.8,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		636.1,2823.0,574.1,
		0.0,360.0,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		520.9,2815.3,574.1,
		0.0,183.2,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-577.5,351.5,574.1,
		0.0,87.6,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-573.4,239.9,574.1,
		0.0,88.7,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-570.1,-6.6,574.1,
		0.0,268.4,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-570.6,79.4,574.1,
		0.0,92.2,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-236.5,633.9,558.1,
		0.0,177.0,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		-102.1,633.4,558.1,
		0.0,359.9,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		38.3,633.4,558.1,
		0.0,0.8,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		291.4,642.1,558.1,
		0.0,308.1,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		420.6,519.4,558.1,
		0.0,248.6,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		320.4,364.1,558.1,
		0.0,267.3,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		325.0,194.4,558.1,
		0.0,269.7,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		322.4,38.3,558.1,
		0.0,270.8,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		695.1,-70.5,558.1,
		0.0,79.7,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		546.3,-150.3,558.1,
		0.0,145.2,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		841.3,-132.4,558.1,
		0.0,359.9,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		1091.5,-266.9,558.1,
		0.0,88.6,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		1180.3,-112.8,558.1,
		0.0,357.5,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		1345.9,-265.5,558.1,
		0.0,94.2,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		1449.0,-112.1,558.1,
		0.0,352.9,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		1602.9,-120.3,558.1,
		0.0,357.9,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		1808.1,12.4,558.1,
		0.0,137.9,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		1815.0,408.4,558.1,
		0.0,176.2,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		1858.9,658.6,558.1,
		0.0,100.5,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		1894.0,887.1,558.1,
		0.0,93.5,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		1819.0,1175.6,558.1,
		0.0,187.4,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		1872.1,1315.3,558.1,
		0.0,65.4,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		1884.4,1464.9,558.1,
		0.0,8.7,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		2016.5,1474.8,558.1,
		0.0,0.9,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		2375.3,1474.8,430.1,
		0.0,0.5,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		2475.4,1474.8,430.1,
		0.0,0.7,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		2817.6,1470.8,430.1,
		0.0,93.4,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		2676.8,1643.0,430.1,
		0.0,272.8,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		2683.1,1819.1,430.1,
		0.0,91.7,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		2726.9,1968.3,430.1,
		0.0,4.1,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		2690.4,2225.4,430.1,
		0.0,3.0,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		2695.3,2098.1,430.1,
		0.0,353.9,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		3243.0,1985.8,478.1,
		0.0,183.8,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		3046.9,1983.9,478.1,
		0.0,175.4,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		2546.1,2425.8,430.1,
		0.0,137.4,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		2563.5,2523.6,430.1,
		0.0,145.1,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		2242.9,2381.6,430.1,
		0.0,88.5,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		2097.4,2371.0,430.1,
		0.0,89.1,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		1618.0,2357.6,430.1,
		0.0,21.0,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		1830.4,2371.8,430.1,
		0.0,87.7,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		1743.3,2554.5,430.1,
		0.0,12.5,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		1733.1,2852.8,430.1,
		0.0,12.2,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		1727.8,3204.4,430.1,
		0.0,332.5,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		1461.6,3256.8,430.1,
		0.0,177.9,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		1865.9,3266.6,430.1,
		0.0,190.2,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		2376.0,3260.6,430.1,
		0.0,269.2,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		2231.1,3235.9,430.1,
		0.0,268.4,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		2562.8,3237.3,430.1,
		0.0,237.9,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		2554.6,3122.3,430.1,
		0.0,225.4,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		1560.5,3269.9,430.1,
		0.0,358.6,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		912.0,3456.0,486.1,
		0.0,180.3,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		1266.4,3254.5,174.1,
		0.0,143.2,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		906.0,3631.6,174.1,
		0.0,182.0,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		840.6,3255.8,174.1,
		0.0,138.4,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		415.0,3458.6,176.5,
		0.0,359.1,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		239.3,3448.3,176.8,
		0.0,179.7,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		64.3,3232.9,177.6,
		0.0,269.7,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		65.8,3064.8,176.4,
		0.0,271.4,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		67.9,2941.5,178.4,
		0.0,272.0,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		185.6,2488.5,174.1,
		0.0,110.2,0.0,
		0
	},
	{
		qtrue,
		"sewer64",
		179.8,2334.8,174.1,
		0.0,269.7,0.0,
		0
	},
	{
	qtrue,
	"sewer64",
	199.6,1906.5,174.1,
	0.0,180.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	170.5,1653.9,174.1,
	0.0,176.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	12.8,1659.8,174.1,
	0.0,176.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-105.1,1662.1,174.1,
	0.0,177.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-228.8,1662.1,174.1,
	0.0,179.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-425.4,1668.8,174.1,
	0.0,142.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-457.4,1438.5,174.1,
	0.0,77.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-460.4,1284.6,174.1,
	0.0,87.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-447.5,1095.0,174.1,
	0.0,169.4,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-448.0,962.9,174.1,
	0.0,268.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-450.5,821.8,174.1,
	0.0,267.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-453.0,689.8,174.1,
	0.0,268.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-592.0,358.1,142.1,
	0.0,83.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-564.6,200.0,178.4,
	0.0,267.4,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-569.9,25.5,179.1,
	0.0,269.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-426.9,-124.4,177.9,
	0.0,5.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-254.6,-120.4,178.1,
	0.0,2.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-115.0,-112.1,173.8,
	0.0,1.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	121.8,25.0,176.5,
	0.0,86.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	122.4,151.6,166.6,
	0.0,90.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-65.3,357.5,174.1,
	0.0,74.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-57.8,721.8,174.1,
	0.0,38.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-20.3,1353.6,174.1,
	0.0,356.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	156.8,1341.4,174.1,
	0.0,360.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	285.4,1341.4,174.1,
	0.0,1.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	402.3,1346.1,174.1,
	0.0,3.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	578.5,1520.4,174.1,
	0.0,267.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	558.6,1641.5,174.1,
	0.0,88.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	572.3,1791.6,174.1,
	0.0,89.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	572.3,1925.0,174.1,
	0.0,89.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	964.0,1901.9,174.1,
	0.0,85.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	961.0,1739.9,174.1,
	0.0,269.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	964.3,1598.4,174.1,
	0.0,271.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	969.6,1498.3,174.1,
	0.0,274.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1121.9,1346.0,174.1,
	0.0,170.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1263.5,1332.5,174.1,
	0.0,175.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1381.9,1320.3,174.1,
	0.0,341.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1872.3,1116.4,176.5,
	0.0,180.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2018.4,1109.9,179.0,
	0.0,174.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1468.6,137.6,174.1,
	0.0,273.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1461.5,357.9,174.1,
	0.0,86.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1461.5,517.5,174.1,
	0.0,89.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1461.5,654.8,174.1,
	0.0,90.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1456.0,773.6,174.1,
	0.0,91.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1249.8,950.0,174.1,
	0.0,359.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1059.9,976.9,174.1,
	0.0,122.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	877.1,965.9,174.1,
	0.0,182.4,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	695.8,955.3,174.1,
	0.0,182.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	557.0,945.9,174.1,
	0.0,180.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	455.5,945.9,174.1,
	0.0,180.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	312.1,442.0,174.1,
	0.0,242.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	304.0,587.0,174.1,
	0.0,90.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	304.6,716.5,174.1,
	0.0,88.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	305.0,810.5,174.1,
	0.0,88.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2231.4,1333.5,177.9,
	0.0,93.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2223.0,1476.4,176.4,
	0.0,91.4,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2250.6,1671.4,177.1,
	0.0,3.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3405.1,2247.9,174.1,
	0.0,189.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3251.0,2363.6,174.1,
	0.0,249.4,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2787.8,2242.3,174.1,
	0.0,354.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3021.8,800.3,110.1,
	0.0,86.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3004.8,619.3,110.1,
	0.0,271.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3005.4,436.4,110.1,
	0.0,270.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3013.5,117.4,110.1,
	0.0,299.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3014.4,-150.8,110.1,
	0.0,0.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3042.5,-445.6,110.1,
	0.0,78.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3251.1,-554.8,110.1,
	0.0,108.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3441.8,-587.1,110.1,
	0.0,176.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3467.8,-455.5,110.1,
	0.0,251.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3501.4,-726.5,110.1,
	0.0,224.4,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3157.6,-687.0,110.1,
	0.0,270.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2914.6,-453.6,110.1,
	0.0,38.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2912.0,-584.6,110.1,
	0.0,211.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2617.5,-571.4,110.1,
	0.0,280.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2769.5,-843.9,110.1,
	0.0,17.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2750.4,-1136.4,110.1,
	0.0,351.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2881.0,-1185.5,110.1,
	0.0,97.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2522.0,-771.4,110.1,
	0.0,178.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1983.9,-731.6,110.1,
	0.0,88.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2049.5,-631.5,110.1,
	0.0,356.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2153.3,-636.4,110.1,
	0.0,356.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2249.9,-812.5,110.1,
	0.0,17.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3048.5,1515.3,430.1,
	0.0,85.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3229.0,1495.1,430.1,
	0.0,94.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3337.8,1480.6,430.1,
	0.0,354.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3517.0,1333.5,430.1,
	0.0,115.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3516.5,1202.9,430.1,
	0.0,273.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3519.1,1103.1,430.1,
	0.0,273.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3514.8,945.3,430.1,
	0.0,177.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3522.4,808.0,430.1,
	0.0,275.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3388.3,722.0,430.1,
	0.0,277.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3261.9,996.8,430.1,
	0.0,360.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3266.5,836.4,430.1,
	0.0,272.4,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3286.3,634.3,430.1,
	0.0,301.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3402.8,558.6,430.1,
	0.0,0.4,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3253.0,341.0,430.1,
	0.0,285.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3396.5,267.4,430.1,
	0.0,338.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3256.4,88.5,430.1,
	0.0,267.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3381.9,-77.1,430.1,
	0.0,358.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3506.6,-299.1,430.1,
	0.0,271.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3506.1,54.0,430.1,
	0.0,86.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3236.6,-145.4,430.1,
	0.0,182.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3262.8,-313.1,430.1,
	0.0,269.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3387.4,-446.0,430.1,
	0.0,358.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3519.5,-552.9,430.1,
	0.0,277.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3530.6,-737.5,430.1,
	0.0,185.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3264.6,-586.4,430.1,
	0.0,89.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3153.5,-716.4,430.1,
	0.0,176.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2979.8,-492.3,430.1,
	0.0,84.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3105.4,-457.6,430.1,
	0.0,359.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2888.1,-1274.8,430.1,
	0.0,91.4,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2868.8,-892.9,430.1,
	0.0,105.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2877.6,-545.9,430.1,
	0.0,237.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2743.0,-691.8,430.1,
	0.0,183.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2354.4,-545.4,430.1,
	0.0,278.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2125.1,-774.0,494.1,
	0.0,271.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2134.6,-1011.4,494.3,
	0.0,330.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2381.4,-1289.1,430.1,
	0.0,267.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2489.3,-1159.1,430.1,
	0.0,3.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2654.0,-1146.8,430.1,
	0.0,3.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2374.0,-1426.1,430.1,
	0.0,271.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2374.0,-1545.4,430.1,
	0.0,271.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2374.0,-1666.5,430.1,
	0.0,271.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2487.8,-1859.5,430.1,
	0.0,272.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2612.1,-1860.1,430.1,
	0.0,358.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2790.8,-1846.5,430.1,
	0.0,251.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2733.3,-2351.0,302.1,
	0.0,128.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2621.3,-2085.6,302.1,
	0.0,223.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2363.1,-2370.9,302.1,
	0.0,78.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2360.1,-2097.0,302.1,
	0.0,358.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2494.0,-1997.9,302.1,
	0.0,88.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2609.3,-1844.8,302.1,
	0.0,358.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2762.1,-1843.4,302.1,
	0.0,183.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2364.1,-1855.0,302.1,
	0.0,352.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2244.6,-1958.9,302.1,
	0.0,238.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2077.3,-1857.5,302.1,
	0.0,297.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2002.4,-2019.4,302.1,
	0.0,298.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2057.6,-2366.5,302.1,
	0.0,77.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1850.1,-1290.0,558.1,
	0.0,140.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1730.8,-1349.6,558.1,
	0.0,91.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1854.0,-908.5,558.1,
	0.0,87.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1874.4,-533.1,558.1,
	0.0,263.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1773.9,-644.6,558.1,
	0.0,175.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1578.0,-538.0,558.1,
	0.0,276.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1591.0,-938.3,558.1,
	0.0,271.4,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1540.0,-1115.3,558.1,
	0.0,213.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1475.9,-1380.9,558.1,
	0.0,270.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1366.9,-1303.5,558.1,
	0.0,170.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1089.6,-1301.0,558.1,
	0.0,24.4,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	942.3,-1127.3,558.1,
	0.0,37.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	938.6,-1402.9,558.1,
	0.0,266.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	833.8,-1527.5,558.1,
	0.0,359.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	975.4,-1518.1,558.1,
	0.0,265.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1107.3,-1569.8,558.1,
	0.0,267.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1215.1,-1424.3,558.1,
	0.0,97.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1340.5,-1521.1,558.1,
	0.0,354.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1610.4,-1549.9,558.1,
	0.0,174.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1605.4,-1707.5,558.1,
	0.0,275.4,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1602.6,-1844.0,558.1,
	0.0,265.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1567.6,-1993.4,558.1,
	0.0,183.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1588.6,-2151.9,558.1,
	0.0,167.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1578.4,-2492.1,558.1,
	0.0,180.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1223.4,-2362.4,462.1,
	0.0,93.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1226.1,-2103.1,462.1,
	0.0,87.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1214.8,-1907.3,462.1,
	0.0,223.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1047.8,-1950.8,462.1,
	0.0,92.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	971.4,-2115.1,462.1,
	0.0,282.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	983.8,-2407.4,462.1,
	0.0,278.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1061.9,-2556.5,462.1,
	0.0,152.4,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	508.9,-2375.0,485.1,
	0.0,178.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	402.6,-2369.9,482.1,
	0.0,179.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	268.1,-2369.9,483.0,
	0.0,180.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	9.1,-2316.0,484.1,
	0.0,96.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-6.9,-2142.0,483.9,
	0.0,94.4,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-15.0,-1975.8,473.3,
	0.0,94.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-219.4,-1718.9,462.1,
	0.0,355.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-347.8,-1718.9,462.1,
	0.0,180.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-579.1,-1722.8,462.1,
	0.0,182.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	0.5,-1513.0,485.1,
	0.0,262.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-12.4,-1363.3,480.4,
	0.0,274.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-4.1,-1046.9,508.3,
	0.0,269.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1.1,-773.1,578.6,
	0.0,90.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-4.9,-433.0,574.1,
	0.0,90.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1.6,-264.0,574.1,
	0.0,91.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-106.6,-128.4,574.1,
	0.0,177.4,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-235.9,-132.3,574.1,
	0.0,182.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-365.3,-127.8,574.1,
	0.0,179.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-429.8,-127.0,574.1,
	0.0,180.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-727.5,-122.6,574.1,
	0.0,358.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-957.9,-125.9,574.1,
	0.0,179.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	962.9,-838.3,558.1,
	0.0,356.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1194.5,-842.9,558.1,
	0.0,141.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	985.6,-628.5,558.1,
	0.0,19.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1213.9,-615.9,558.1,
	0.0,262.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1343.4,-396.9,558.1,
	0.0,90.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1346.5,-499.5,558.1,
	0.0,272.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1647.4,-712.0,558.1,
	0.0,179.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1083.4,-482.0,558.1,
	0.0,274.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1080.5,-378.0,558.1,
	0.0,83.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	772.5,739.8,558.1,
	0.0,262.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	767.8,1036.8,558.1,
	0.0,88.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	760.8,937.3,558.1,
	0.0,88.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1140.1,-135.5,400.5,
	0.0,186.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1140.1,-135.5,661.0,
	0.0,186.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1714.8,-151.1,174.1,
	0.0,219.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1791.0,-214.6,174.1,
	0.0,222.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-2084.0,-40.4,174.1,
	0.0,324.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-2072.8,217.4,174.1,
	0.0,306.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1894.8,481.1,174.1,
	0.0,280.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1864.8,-751.3,174.1,
	0.0,275.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1886.0,-631.6,174.1,
	0.0,98.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1897.3,-497.3,174.1,
	0.0,96.4,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1924.6,-936.0,174.1,
	0.0,306.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1803.5,-990.4,174.1,
	0.0,313.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1505.0,-1533.6,174.1,
	0.0,124.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1143.6,-1573.6,174.1,
	0.0,142.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1081.1,-1179.3,174.1,
	0.0,284.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-986.8,-1270.0,174.1,
	0.0,248.4,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-879.1,-1355.8,174.1,
	0.0,213.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-904.3,-1470.6,174.1,
	0.0,183.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-880.9,-1718.5,206.1,
	0.0,172.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1032.6,-1798.0,191.3,
	0.0,149.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1497.8,-1731.8,238.1,
	0.0,85.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1680.8,-1567.6,254.1,
	0.0,42.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1860.8,-1382.5,302.1,
	0.0,71.4,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1929.0,-1039.6,334.0,
	0.0,332.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1897.3,-769.0,382.1,
	0.0,298.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1920.4,-569.5,398.1,
	0.0,295.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-2131.9,-274.1,430.1,
	0.0,118.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-2242.0,-62.5,446.1,
	0.0,115.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-2328.8,139.1,462.1,
	0.0,78.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-2271.9,178.9,462.1,
	0.0,13.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-2205.9,328.9,478.1,
	0.0,344.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-2101.1,481.0,494.1,
	0.0,308.4,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1987.0,478.1,494.1,
	0.0,338.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1540.0,399.3,174.1,
	0.0,244.4,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1287.5,-1150.9,179.0,
	0.0,269.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1280.3,-984.3,178.0,
	0.0,94.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1285.8,-761.0,174.9,
	0.0,91.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-1072.9,-470.8,142.3,
	0.0,181.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-911.5,-467.5,142.3,
	0.0,358.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-773.4,-470.3,142.3,
	0.0,358.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-579.8,-434.5,178.5,
	0.0,96.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	-588.3,-272.5,178.1,
	0.0,93.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2387.1,2605.1,46.1,
	0.0,168.0,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2212.9,2616.8,46.1,
	0.0,173.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1882.9,2611.6,46.1,
	0.0,178.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1645.9,2633.8,46.1,
	0.0,178.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1430.5,2619.9,46.1,
	0.0,181.9,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1257.8,2619.4,46.1,
	0.0,178.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1112.4,2619.4,46.1,
	0.0,178.8,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	944.9,2452.8,114.3,
	0.0,86.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	966.3,2287.0,144.8,
	0.0,92.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2401.0,2995.6,46.1,
	0.0,184.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2226.0,3001.4,46.1,
	0.0,177.3,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1923.3,3001.8,46.1,
	0.0,275.2,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1671.6,3001.6,46.1,
	0.0,178.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	1306.3,2993.6,46.1,
	0.0,255.6,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	755.6,2982.6,46.1,
	0.0,281.1,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2524.6,2793.5,44.6,
	0.0,178.5,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	2706.4,2816.6,50.1,
	0.0,358.7,0.0,
	0
	},
	{
	qtrue,
	"sewer64",
	3009.1,2573.0,50.5,
	0.0,270.5,0.0,
	0
	},

	{ qfalse, NULL, 0, 0, 0, 0, 0, 0, 0 }
};

// check visibility?
// delay between waves?

// if g_lockdown is 1 use this if set to 2 use custom map support
// and search for control point spawn objects (yet to be implemented)

void centermsg(char *msg)
{
	int			i;
	edict_t		*cl_ent;

	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || game.clients[i].resp.spectator)
		{
			continue;
		}

		gi.centerprintf(cl_ent, msg);
	}
}

qboolean checkvisible(edict_t *ent)
{
	int			i;
	edict_t		*cl_ent;

	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || game.clients[i].resp.spectator)
		{
			continue;
		}

		if (visible(ent, cl_ent))
			return qtrue;
	}

	return qfalse;
}

qboolean spotvisible(vec3_t spot1, edict_t *other)
{
	vec3_t  spot2;
	trace_t trace;

	VectorCopy(other->s.origin, spot2);
	spot2[2] += other->viewheight;

	trace = gi.trace(spot1, vec3_origin, vec3_origin, spot2, other, MASK_OPAQUE);

	if (trace.fraction == 1.0)
		return qtrue;
	return qfalse;
}

qboolean spotvisible2(vec3_t spot1, edict_t *other)
{
	vec3_t  spot2;
	trace_t trace;

	VectorCopy(other->s.origin, spot2);
	spot2[2] += other->viewheight;
	trace = gi.trace(spot1, vec3_origin, vec3_origin, spot2, other, MASK_SHOT);

	if (trace.fraction == 1.0)
		return qtrue;
	return qfalse;
}

qboolean checkspotvisible(vec3_t spot1)
{
	int			i;
	edict_t		*cl_ent;

	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || game.clients[i].resp.spectator)
		{
			continue;
		}

		if (spotvisible(spot1, cl_ent))
			return qtrue;
	}

	return qfalse;
}

float nearestplayer(edict_t *ent)
{
	int			i;
	edict_t		*cl_ent;
	float		dist, nearest = 99999;
	vec3_t		v;

	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || game.clients[i].resp.spectator)
		{
			continue;
		}

		if (cl_ent->health <= 0)
			continue;

		VectorSubtract(cl_ent->s.origin, ent->s.origin, v);
		dist = VectorLength(v);
		if (dist < nearest)
			nearest = dist;
	}

	return nearest;
}

float nearestplayer2(vec3_t spot1)
{
	int			i;
	edict_t		*cl_ent;
	float		dist, nearest = 99999;
	vec3_t		v;

	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || game.clients[i].resp.spectator)
		{
			continue;
		}

		if (cl_ent->health <= 0)
			continue;

		VectorSubtract(cl_ent->s.origin, spot1, v);
		dist = VectorLength(v);
		if (dist < nearest)
			nearest = dist;
	}

	return nearest;
}

// needs redoing but works for now
// intended funcitonality:
// spawn progressively harder monsters as the wave count increases
// also progressively increasing the number of monster slots that spawn
// harder monsters
// needs to progress through easy, medium and hard
// have to be very careful here not to blow through the sound indices
// budget of 256

// monster_num starts at 0
char *randommonster(edict_t *ent, int monster_num)
{
	int			wave_num, exit_rand;
	float		chance_easy, chance_hard, pos_modifier;
	float		mod_easy = 50; // 50
	float		mod_hard = 100; // 100
	float		mod_pos = 100; // 100
	float		mod_rate_easy = 1.0625;
	float		mod_rate_hard = 1.08; // 1.125

	wave_num = 10 - ent->count;

	chance_easy = 1 - ((wave_num - 1) / mod_easy);

	chance_easy /= mod_rate_easy;

	if (chance_easy < 0.1)
		chance_easy = 0.1;

	chance_hard = wave_num / mod_hard;

	chance_hard *= mod_rate_hard;

	if (chance_hard > 0.3)
		chance_hard = 0.3;

	pos_modifier = (wave_num / (monster_num + 1)) / mod_pos;

	pos_modifier *= mod_rate_hard;

	if (pos_modifier > 0.3)
		pos_modifier = 0.3;

	chance_hard += pos_modifier;

	if (random() < chance_easy)
	{
		exit_rand = rand() % 4;

		if (exit_rand == 3)
		{
			return "monster_soldier_ss";
		}

		if (exit_rand == 2)
		{
			return "monster_soldier";
		}

		// Rroff - had to remove to save on save on sound index :(
		//if (exit_rand == 1 && (random() < 0.65))
		//{
		//	return "monster_flyer";
		//}

		return "monster_soldier_light";
	}

	if (random() < chance_hard)
	{

		if (random() < 0.5)
		{
			return "monster_gladiator";
		}

		if (random() < 0.5)
		{
			return "monster_mutant";
		}

		if (random() < 0.6)
		{
			return "monster_tank";
		}

		if (random() < 0.3)
		{
			return "monster_floater";
		}

		if (random() < 0.3)
		{
			return "monster_tank_commander";
		}


		// Rroff - had to remove to save on save on sound index :(
		//if (random() < 0.5)
		//{
		//	return "monster_brain";
		//}
	}

	if (random() < 0.2)
	{
		return "monster_chick";
	}

	if (random() < 0.3)
	{
		return "monster_infantry";
	}

	if (random() < 0.3)
	{
		return "monster_gunner";
	}

	if (random() < 0.3)
	{
		return "monster_berserk";
	}

	if (random() < 0.3)
	{
		return "monster_parasite";
	}

	if (random() < 0.3)
	{
		return "monster_floater";
	}

	return "monster_soldier_light";
}

qboolean checkalldead(void)
{
	int			i;
	edict_t		*cl_ent;

	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || game.clients[i].resp.spectator)
		{
			continue;
		}

		if (!cl_ent->deadflag)
			return qfalse;

		//if (cl_ent->health > 0)
		//	return qfalse;
	}

	return qtrue;
}

// spawn a goal that has to be captured within a timer
// players in close proximity will cause the counter to go down
// players not close counter goes back up

void cpoint_goal_think(edict_t *self)
{
	qboolean	sonar = qtrue;

	if (self->count <= 0)
	{
		self->s.modelindex4 = 0;
	}
	else {
		self->s.modelindex4 = gi.modelindex(va("sprites/health%i.sp2", self->count));
	}

	if (self->count == 10)
	{
		if (level.lockdown_ent)
			level.lockdown_ent->turret_ammo--;

		level.found_goals++;

		gi.bprintf(PRINT_HIGH, "Data download complete!\n");
		centermsg("Data download complete!");

		self->think = G_FreeEdict;
		self->nextthink = level.time + 1;

		return;
	}

	if (level.time > self->random)
	{
		if (!level.intermissiontime)
		{
			if (level.lockdown_ent)
			{
				gi.bprintf(PRINT_HIGH, "Timed objective failed - map ending on wave %i\n", (10 - level.lockdown_ent->count));
				centermsg(va("Failed! on wave %i", (10 - level.lockdown_ent->count)));
			}
			else
			{
				gi.bprintf(PRINT_HIGH, "Timed objective failed\n");
				centermsg("Failed!");
			}
			EndDMLevel();
			return;
		}
	}

	if (self->count < 10)
	{
		// should we check line of sight to nearest player to prevent
		// capture through walls? not really worth it IMO
		if (nearestplayer(self) <= 128)
		{
			sonar = qfalse;
			self->count++;
			self->s.effects |= EF_SPINNINGLIGHTS;
		}
		else
		{
			self->s.effects &= ~EF_SPINNINGLIGHTS;

			if (self->count > 0)
				self->count--;
		}
	}

	if (sonar && level.time > self->delay + 5)
	{
		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, gi.soundindex("world/radio3.wav"), 1.0, ATTN_NORM, 0);
		self->delay = level.time;
	}
	//gi.sound(self, CHAN_VOICE, gi.soundindex("makron/rail_up.wav"), 1.0, ATTN_NONE, 0);

	self->nextthink = level.time + 1;
}

qboolean cpoint_spawn_goal(int goal_type)
{
	edict_t		*ent, *point;

	point = SelectRandomDeathmatchSpawnPoint();

	if (!point)
		return qfalse;

	ent = G_Spawn();

	ent->classname = "cpoint_goal";

	if (goal_type == 0)
		ent->s.modelindex = gi.modelindex("models/monsters/commandr/head/tris.md2");

	if (goal_type == 1)
		ent->s.modelindex = gi.modelindex("models/items/keys/pyramid/tris.md2");

	ent->s.renderfx = RF_GLOW | RF_DEPTHHACK;
	ent->random = level.time + 300;
	ent->count = 0;

	//ent->s.sound = gi.soundindex("world/comp_hum2.wav");

	VectorCopy(point->s.origin, ent->s.origin);
	VectorCopy(point->s.angles, ent->s.angles);

	ent->think = cpoint_goal_think;
	ent->nextthink = level.time + 1;

	gi.sound(ent, CHAN_NO_PHS_ADD + CHAN_VOICE, gi.soundindex("world/radio3.wav"), 1.0, ATTN_NORM, 0);
	ent->delay = level.time;

	gi.linkentity(ent);

	return qtrue;
}

void cpoint_casualty_think(edict_t *self)
{
	qboolean	sonar = qtrue;
	int			counter;

	counter = self->count / 30;

	if (counter <= 0)
	{
		self->s.modelindex4 = 0;
	}
	else {
		self->s.modelindex4 = gi.modelindex(va("sprites/health%i.sp2", counter));
	}

	if (level.time > self->random)
	{
		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, gi.soundindex("player/male/death1.wav"), 1.0, ATTN_NORM, 0);

		if (!level.intermissiontime)
		{
			if (level.lockdown_ent)
			{
				gi.bprintf(PRINT_HIGH, "Timed objective failed - map ending on wave %i\n", (10 - level.lockdown_ent->count));
				centermsg(va("Failed! on wave %i", (10 - level.lockdown_ent->count)));
			}
			else
			{
				gi.bprintf(PRINT_HIGH, "Timed objective failed\n");
				centermsg("Failed!");
			}
			EndDMLevel();
			return;
		}
	}

	self->s.frame++;

	if (self->s.frame > 266)
		self->s.frame = 252;

	self->nextthink = level.time + FRAMETIME;

	if (counter == 10)
	{
		if (level.lockdown_ent)
			level.lockdown_ent->turret_ammo--;

		level.found_goals++;

		gi.bprintf(PRINT_HIGH, "Casualty evac'd!\n");
		centermsg("Casualty evac complete!");

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_TELEPORT_EFFECT);
		gi.WritePosition(self->s.origin);
		gi.multicast(self->s.origin, MULTICAST_PVS);

		gi.sound(self, CHAN_VOICE, gi.soundindex("misc/tele1.wav"), 1, ATTN_NORM, 0);

		self->think = G_FreeEdict;
		self->nextthink = level.time + FRAMETIME;

		return;
	}

	if (counter < 10)
	{
		// should we check line of sight to nearest player to prevent
		// capture through walls? not really worth it IMO
		if (nearestplayer(self) <= 128)
		{
			sonar = qfalse;
			self->count++;
		}
		else
		{
			if (self->count > 29)
			{
				self->count -= 2;

				if (self->count < 29)
					self->count = 29;
			}
		}
	}

	if (sonar && level.time > self->delay + 5)
	{
		self->delay = level.time;

		if (random() < 0.3)
		{
			gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, gi.soundindex("player/male/pain75_2.wav"), 1.0, ATTN_NORM, 0);
			return;
		}

		if (random() < 0.3)
		{
			gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, gi.soundindex("player/male/pain100_2.wav"), 1.0, ATTN_NORM, 0);
			return;
		}

		if (random() < 0.3)
		{
			gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, gi.soundindex("player/male/pain25_2.wav"), 1.0, ATTN_NORM, 0);
			return;
		}

		if (random() < 0.3)
		{
			gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, gi.soundindex("insane/insane1.wav"), 1.0, ATTN_NORM, 0);
			return;
		}

		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, gi.soundindex("insane/insane9.wav"), 1.0, ATTN_NORM, 0);

	}
}

qboolean cpoint_spawn_casualty(void)
{
	edict_t		*ent, *point;

	point = SelectRandomDeathmatchSpawnPoint();

	if (!point)
		return qfalse;

	ent = G_Spawn();

	ent->classname = "cpoint_goal";

	ent->s.modelindex = gi.modelindex("models/monsters/insane/tris.md2");
	ent->s.skinnum = 2;
	ent->s.frame = 252;

	ent->s.renderfx = RF_GLOW | RF_DEPTHHACK;
	ent->random = level.time + 300;
	ent->count = 29;

	//ent->s.sound = gi.soundindex("world/comp_hum2.wav");

	VectorCopy(point->s.origin, ent->s.origin);
	VectorCopy(point->s.angles, ent->s.angles);

	ent->s.origin[2] -= 20;
	ent->s.angles[PITCH] = -90;

	ent->think = cpoint_casualty_think;
	ent->nextthink = level.time + FRAMETIME;

	// gi.sound(ent, CHAN_NO_PHS_ADD + CHAN_VOICE, gi.soundindex("world/radio3.wav"), 1.0, ATTN_NORM, 0);
	ent->delay = level.time;

	gi.linkentity(ent);

	return qtrue;
}

// an AI2_CONTROL monster on death should call this
// check through all monsters and if we find a trigger spawn monster
// with AI2_CONTROL set then find a point and spawn it there
void cpoint_trigger_spawn(void)
{
	edict_t		*from = g_edicts;

	for (; from < &g_edicts[globals.num_edicts]; from++) {
		if (!from->inuse)
			continue;
		if (!(from->svflags & SVF_MONSTER))
			continue;
		if (!(from->monsterinfo.aiflags2 & AI2_CONTROL_SPAWN))
			continue;
		//if (!(from->spawnflags & 2))
		//	continue;

		cpoint_move_monster(from);
		from->use(from, NULL, NULL);
		from->monsterinfo.aiflags2 &= ~AI2_CONTROL_SPAWN;
		break;
	}
}

// if a monster has been too long without seeing a player find a spot to move them to
// incase they've somehow gone out of bounds or got stuck

// need to try and reduce instances of runaway loops in sv_trace here :S

void cpoint_move_monster(edict_t *self)
{
	spawn_point_t				*cp, *cpc, *any, *notvis;
	qboolean					found = qfalse;
	qboolean					dotele = qfalse;
	int							index;
	float						dist, nearest;
	trace_t						tr;
	vec3_t						start, end, v;
	float						max_dist = 512;
	//float						min_dist = 256;
	float						min_dist = 128;
	int							i;

	// don't move a monster if RADAR is up
	// this might cause a problem if monster is out of bounds

	//if ((level.total_monsters - level.killed_monsters) > 0 && (level.total_monsters - level.killed_monsters) < 4)
	//	return;

	// don't move a monster if a player is in view

	//if (!(self->spawnflags & 2))
	//{
		if (checkvisible(self))
			return;
	//}

	//if ((self->moded == LK_WASTE1) || (self->moded == LK_FOREVER))
	if (self->moded == LK_WASTE1)
		max_dist = 1024;
	//else
	//	max_dist = 512;

	// try and get stragglers near to players
	if (self->moded == LK_FOREVER)
	{
		min_dist = 96;
		max_dist = 1024;
	}

	if (self->moded != LK_CUSTOM)
	{
		if (!self->radius_dmg)
		{
			index = 0;

			for (cp = spawn_point_list; cp->mapname; cp++)
			{
				if ((Q_stricmp(level.mapname, cp->mapname) == 0) && cp->child)
				{
					found = qtrue;
					break;
				}
				index++;
			}

			if (!found)
				return;

			self->radius_dmg = index;
		}
		else
		{
			index = self->radius_dmg;
		}
	}

	cpc = NULL;
	any = NULL;
	notvis = NULL;
	dotele = qfalse;
	nearest = 9999;
	dist = 99999;

	if (self->moded == LK_CUSTOM)
	{
		cp = &custom_spawn_list[17];

		for (i = 17; i < LK_MAX_CUSTOM; i++, cp++)
		{
			if (!(cp->child))
				break;

			if ((cp->lastused > 0) && (level.time < (cp->lastused + 4)))
				continue;

			VectorCopy(cp->origin, start);
			VectorCopy(cp->origin, end);
			end[2] += 4;

			tr = gi.trace(start, self->mins, self->maxs, end, self, MASK_MONSTERSOLID);

			if (tr.startsolid)
				continue;

			any = cp;

			if (checkspotvisible(cp->origin))
				continue;

			//dist = nearestplayer2(cp->origin);
			if (level.sight_client)
			{
				if (!(gi.inPHS(cp->origin, level.sight_client->s.origin)))
					continue;

				if (fabsf((cp->origin[2] - level.sight_client->s.origin[2])) > 64)
					continue;

				VectorSubtract(cp->origin, level.sight_client->s.origin, v);
				dist = VectorLength(v);
			}
			else
			{
				continue;
			}

			if (dist < nearest)
			{
				notvis = cp;
				nearest = dist;
			}

			if ((dist < min_dist) || (dist > max_dist))
				continue;

			cpc = cp;
			break;
		}
	}
	else
	{
		for (cp = spawn_point_list + index; cp->child; cp++)
		{

			if ((cp->lastused > 0) && (level.time < (cp->lastused + 4)))
				continue;

			VectorCopy(cp->origin, start);
			VectorCopy(cp->origin, end);
			end[2] += 4;

			tr = gi.trace(start, self->mins, self->maxs, end, self, MASK_MONSTERSOLID);

			if (tr.startsolid)
				continue;

			any = cp;

			if (checkspotvisible(cp->origin))
				continue;

			//dist = nearestplayer2(cp->origin);
			if (level.sight_client)
			{
				if (fabsf((cp->origin[2] - level.sight_client->s.origin[2])) > 64)
					continue;

				VectorSubtract(cp->origin, level.sight_client->s.origin, v);
				dist = VectorLength(v);
			}
			else {
				continue;
			}

			if (dist < nearest)
			{
				notvis = cp;
				nearest = dist;
			}

			if ((dist < min_dist) || (dist > max_dist))
				continue;
			cpc = cp;
			break;
		}
	}

	if (!cpc)
		cpc = notvis;
	if (!cpc)
	{
		cpc = any;
		dotele = qtrue;
	}

	if (!cpc)
		return;

	cpc->lastused = level.time;

	VectorCopy(cpc->origin, self->s.old_origin);
	VectorCopy(cpc->origin, self->s.origin);
	VectorCopy(cpc->angles, self->s.angles);
	self->s.event = EV_OTHER_TELEPORT;

	// Can we just do this without breaking anything
	// to suppress the sound of the monster exiting water
	// if teleported from in water to a dry position?
	self->flags &= ~FL_INWATER;

	if (dotele)
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_TELEPORT_EFFECT);
		gi.WritePosition(self->s.origin);
		gi.multicast(self->s.origin, MULTICAST_PVS);

		gi.sound(self, CHAN_VOICE, gi.soundindex("misc/tele1.wav"), 1, ATTN_NORM, 0);
	}

	gi.unlinkentity(self);
	KillBox(self);
	gi.linkentity(self);
}

void cpoint_wave(edict_t *self)
{
	edict_t						*ent;
	spawn_point_t				*cp, *cpc, *any, *notvis;
	int							index;
	qboolean					found = qfalse;
	qboolean					dotele = qfalse;
	control_wave_t				*wave;
	control_wave_monsters_t		*spawn;
	float						dist, nearest;
	float						max_dist;
	trace_t						tr;
	vec3_t						start, end;
	int							monster_num = 0;
	horde_point_t				*point;
	int							hp, wave_num;
	int							i, c = 0;
	spawn_point_t				*custompoint;

	strncpy(game.helpmessage2, "Kill them all", sizeof(game.helpmessage1) - 1);

	level.total_goals = 0;
	level.found_goals = 0;

	wave_num = 10 - self->count;

	//if ((self->moded == LK_WASTE1) || (self->moded == LK_FOREVER))
	if (self->moded == LK_WASTE1)
		max_dist = 1024;
	else
		max_dist = 512;

	// find spots in the list and spawn each wave according to count
	// tag them with ai2_control

	// find the start of our section of the list
	if (self->moded != LK_CUSTOM)
	{
		if (!(self->radius_dmg))
		{
			index = 0;

			for (cp = spawn_point_list; cp->mapname; cp++)
			{
				if ((Q_stricmp(level.mapname, cp->mapname) == 0) && cp->child)
				{
					found = qtrue;
					break;
				}
				index++;
			}

			if (!found)
				return;

			self->radius_dmg = index;
		}
		else
		{
			index = self->radius_dmg;
		}
	}

	if (self->moded == LK_EASY)
	{
		if (g_hordemode->integer == 3)
			wave = control_wave_list_hard + self->count;
		else
			wave = control_wave_list_easy + self->count;
	}

	// Rroff - implement these later
	if (self->moded == LK_MEDIUM)
		wave = control_wave_list_easy + self->count;

	if (self->moded == LK_HARD)
		wave = control_wave_list_hard + self->count;
	//

	if (self->moded == LK_WASTE1)
		wave = control_wave_list_waste1 + self->count;

	if (self->moded == LK_FACT1)
		wave = control_wave_list_fact1 + self->count;

	if ((self->moded == LK_FOREVER) || (self->moded == LK_CUSTOM))
	{
		if (self->count < 0)
			wave = control_wave_list_forever;
		else
			wave = control_wave_list_forever + self->count;
	}

	// Ugly fix for the last wave on resume from a saved game
	// where any spawn point list changes won't have been preserved

	//for (cp = spawn_point_list + index; cp->child; cp++)
	//	cp->used = 99999;

	for (spawn = wave->spawn; spawn->monster; spawn++)
	{
		// if iterations of this loop are over LK_MAX_ACTIVE
		// set spawn flag to trigger spawned

		if (Q_stricmp(spawn->monster, "skip") == 0)
			continue;
		// try and find a spot that isn't visible to any players
		// and in the range of 256-512 distance to nearest player
		// failing that grab a spot that isn't visible
		// failing that use any spot that isn't already used

		//gi.dprintf(": %s\n", spawn->monster);
		cpc = NULL;
		any = NULL;
		notvis = NULL;
		dotele = qfalse;
		nearest = 9999;

		if (self->moded == LK_CUSTOM)
		{
			cp = &custom_spawn_list[17];

			for (i = 17; i < LK_MAX_CUSTOM; i++, cp++)
			{
				if (!(cp->child))
					break;

				if ((cp->lastused > 0) && (level.time < (cp->lastused + 4)))
					continue;

				VectorCopy(cp->origin, start);
				VectorCopy(cp->origin, end);
				end[2] += 1;

				tr = gi.trace(start, self->mins, self->maxs, end, self, MASK_MONSTERSOLID);

				if (tr.startsolid)
					continue;

				any = cp;

				if (checkspotvisible(cp->origin))
					continue;

				dist = nearestplayer2(cp->origin);

				if (dist < nearest)
				{
					notvis = cp;
					nearest = dist;
				}

				if ((dist < 256) || (dist > max_dist))
					continue;
				cpc = cp;
				break;
			}
		}
		else
		{
			for (cp = spawn_point_list + index; cp->child; cp++)
			{
				//if (cp->used == self->count)
				//	continue;
				if ((cp->lastused > 0) && (level.time < (cp->lastused + 4)))
					continue;

				VectorCopy(cp->origin, start);
				VectorCopy(cp->origin, end);
				end[2] += 1;

				tr = gi.trace(start, self->mins, self->maxs, end, self, MASK_MONSTERSOLID);

				if (tr.startsolid)
					continue;

				any = cp;

				if (checkspotvisible(cp->origin))
					continue;

				dist = nearestplayer2(cp->origin);

				if (dist < nearest)
				{
					notvis = cp;
					nearest = dist;
				}

				if ((dist < 256) || (dist > max_dist))
					continue;
				cpc = cp;
				break;
			}
		}

		if (!cpc)
			cpc = notvis;
		if (!cpc)
		{
			cpc = any;
			dotele = qtrue;
		}

		if (!cpc) // ran out of places to spawn
			return;

		// mark this one and use it
		// monsters over LK_MAX_ACTIVE shouldn't reserve spots
		// but not really a problem as there shouldn't be any actual spawning monsters
		// after that
		//cpc->used = self->count;
		cpc->lastused = level.time;

		// check if classname is random and select a monster

		c++;

		ent = G_Spawn();

		if (c > LK_MAX_ACTIVE)
		{
			ent->spawnflags |= 2;
			ent->monsterinfo.aiflags2 |= AI2_CONTROL_SPAWN;
		}

		if (Q_stricmp(spawn->monster, "random") == 0)
		{
			//ent->classname = randommonster(self, monster_num);
			//strcpy(ent->classname, randommonster(self, monster_num));
			ent->classname = randommonster(self, monster_num);
		}
		else {
			ent->classname = spawn->monster;
		}

		if (Q_stricmp(ent->classname, "monster_tank_commander") == 0)
		{
			ent->monsterinfo.aiflags2 |= AI2_CONTROL_REWARD;
		}
		
		//ent->monsterinfo.aiflags |= AI_ROAMING;
		ent->monsterinfo.aiflags2 |= AI2_CONTROL;
		ent->monsterinfo.ai_last_sight = level.time;
		ent->monsterinfo.lockdown_wave = self->count;

		VectorCopy(cpc->origin, ent->s.origin);
		VectorCopy(cpc->angles, ent->s.angles);

		ED_CallSpawn(ent);

		if (!(ent->spawnflags & 2))
		{
			if (dotele)
			{
				gi.WriteByte(svc_temp_entity);
				gi.WriteByte(TE_TELEPORT_EFFECT);
				gi.WritePosition(ent->s.origin); // move to end?
				gi.multicast(ent->s.origin, MULTICAST_PVS);

				gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/tele1.wav"), 1, ATTN_NORM, 0);
			}

			gi.unlinkentity(ent);
			KillBox(ent);
			gi.linkentity(ent);
		}

		if (coop->value)
		{
			if (spawn->coopitem)
				ent->item = FindItemByClassname(spawn->coopitem);
		}
		else
		{
			if (spawn->item)
				ent->item = FindItemByClassname(spawn->item);
		}

		monster_num++;

	}

	// horrid hardcoded stuff :(
	// arrange so that longest timer wins out?

	//if (((10 - self->count) == 1) && (self->moded == LK_FOREVER))

	if (
		((wave_num % 12) == 0) // 12
		&& ((self->moded == LK_FOREVER) || (self->moded == LK_CUSTOM))
		)
	{
		self->wait = level.time + 180;
		self->last_time = level.time + 180;
		self->flags |= FL_TOXIC;

		strncpy(game.helpmessage2, "Complete the wave\nBefore toxic levels\nbecome lethal", sizeof(game.helpmessage1) - 1);
		gi.bprintf(PRINT_HIGH, "Warning: Toxic levels in %s rising!\n", level.level_name);
		game.helpchanged++;
	}

	if (
		((wave_num % 20) == 0) // 27 // 20
		&& ((self->moded == LK_FOREVER) || (self->moded == LK_CUSTOM))
		)
	{
		// can loop the first bit to add multiple goals
		if (random() < 0.5)
		{
			for (i = 0; i < 5; i++)
			{
				found = cpoint_spawn_goal((rand() % 2));

				if (found)
				{
					level.total_goals++;
					self->turret_ammo++;
				}
			}

			if (found)
			{
				self->random = level.time + 300;

				strncpy(game.helpmessage2, "Download data from\nStrogg data devices\nwithin the timer", sizeof(game.helpmessage1) - 1);
				gi.bprintf(PRINT_HIGH, "Objective: Find Strogg data devices and download data within the timer\n");
				game.helpchanged++;
			}
		}
		else
		{
			found = cpoint_spawn_casualty();

			if (found)
			{

				level.total_goals++;
				self->turret_ammo++;

				self->random = level.time + 300;

				strncpy(game.helpmessage2, "Find and prep the casualty\nfor extraction", sizeof(game.helpmessage1) - 1);
				gi.bprintf(PRINT_HIGH, "Objective: Find the wounded marine before it is too late\n");
				game.helpchanged++;
			}
		}
	}

	// every 20 waves spawn boss
	// tweaked to 15
	if (
		((wave_num % 15) == 0) // 20
		&& ((self->moded == LK_FOREVER) || (self->moded == LK_CUSTOM))
		)
	{
		found = qfalse;
		if (random() < 0.5)
		{
			hp = HP_SUPERTANK1;
		}
		else
		{
			if (random() < 0.5)
				hp = HP_HORNET1;
			else
				hp = HP_HORNET2;
		}

		if (self->moded == LK_FOREVER)
		{
			for (point = horde_point_list; point->mapname; point++)
			{
				if ((Q_stricmp(level.mapname, point->mapname) == 0))
				{
					if (point->type == hp)
					{
						found = qtrue;
						break;
					}
				}
			}
		}

		if (self->moded == LK_CUSTOM)
		{
			custompoint = &custom_spawn_list[1];

			for (i = 1; i < 16; i++, custompoint++)
			{
				if ((int)custompoint->lastused == hp)
				{
					found = qtrue;
					break;
				}
			}
		}

		if (found)
		{
			level.total_goals++;
			self->turret_ammo++;
			self->gib_health++;
			self->last_time = level.time + 180;
			game.helpchanged++;

			ent = G_Spawn();

			if (hp == HP_HORNET1 || hp == HP_HORNET2)
			{
				strncpy(game.helpmessage2, "Destroy Hornet boss\nwithin the timer", sizeof(game.helpmessage1) - 1);
				gi.bprintf(PRINT_HIGH, "Objective: Destroy Hornet boss within the timer\n");

				ent->classname = "monster_boss2";
			}

			if (hp == HP_SUPERTANK1)
			{
				strncpy(game.helpmessage2, "Destroy Supertank boss\nwithin the timer", sizeof(game.helpmessage1) - 1);
				gi.bprintf(PRINT_HIGH, "Objective: Destroy Supertank boss within the timer\n");

				ent->classname = "monster_supertank";
			}

			ent->monsterinfo.aiflags2 |= (AI2_CONTROL | AI2_CONTROL_GOALTIMER);
			ent->monsterinfo.ai_last_sight = level.time;
			ent->last_time = level.time + 180;

			if (self->moded == LK_FOREVER)
				VectorCopy(point->origin, ent->s.origin);

			if (self->moded == LK_CUSTOM)
				VectorCopy(custompoint->origin, ent->s.origin);

			ED_CallSpawn(ent);

			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_TELEPORT_EFFECT);
			gi.WritePosition(ent->s.origin); // move to end?
			gi.multicast(ent->s.origin, MULTICAST_PVS);

			gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/tele1.wav"), 1, ATTN_NORM, 0);

			gi.unlinkentity(ent);
			KillBox(ent);
			gi.linkentity(ent);
		}
	}
}

void cpoint_think(edict_t *self)
{
	int			i;
	edict_t		*ent = NULL;
	edict_t     *e;
	//edict_t		*cl_ent;
	//vec2_t		v;
	//qboolean	clientnear = qfalse;

	if (self->count <= 0)
	{
		self->s.modelindex4 = 0;
	}
	else {
		self->s.modelindex4 = gi.modelindex(va("sprites/health%i.sp2", self->count));
	}

	/*for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || game.clients[i].resp.spectator)
		{
			continue;
		}

		if (cl_ent->health <= 0)
			continue;

		VectorSubtract(cl_ent->s.origin, self->s.origin, v);

		if (VectorLength(v) < 200)
			clientnear = qtrue;
	}

	// should we only check this initially to get things started?
	// and/or move this to touch or use triggered to start things?

	if (!clientnear)
	{
		self->nextthink = level.time + 1;
		return;
	}*/

	if (((self->moded == LK_FOREVER) || (self->moded == LK_CUSTOM)) && (checkalldead()))
	{
		if (level.intermissiontime)
			return;

		gi.bprintf(PRINT_HIGH, "All players died - map ending on wave %i\n", (10 - self->count));
		centermsg(va("Failed! on wave %i", (10 - self->count)));
		EndDMLevel();
		return;
	}

	if (self->delay > 0)
	{
		self->delay = 0;
		self->dmg = 0;

		if ((self->count > 0) || ((self->moded == LK_FOREVER) || (self->moded == LK_CUSTOM)))
		{
			self->s.sound = 0;
			centermsg(va("Wave %i begins", (11 - self->count)));
			self->count--;
			cpoint_wave(self);
		}

		self->nextthink = level.time + 2;
	}

	if (level.sight_client)
	{
		//PlayerNoise(level.sight_client, self->s.origin, PNOISE_SELF);
		PlayerNoise(level.sight_client, level.sight_client->s.origin, PNOISE_SELF);
	}

	if ((self->count > 0) || ((self->moded == LK_FOREVER) || (self->moded == LK_CUSTOM)))
	{
		if (self->turret_ammo > 0)
		{
			self->nextthink = level.time + 2;
			return;
		}

		e = &g_edicts[game.maxclients + 1];
		for (i = game.maxclients + 1; i < globals.num_edicts; i++, e++) {
			// we still have a monster alive
			if ((e->inuse) && (e->health > 0) && (e->svflags & SVF_MONSTER) && (e->monsterinfo.aiflags2 & AI2_CONTROL))
			{
				self->nextthink = level.time + 2;
				return;
			}
		}

		if (self->dmg == 0 && self->count < 10)
		{
			self->dmg = 1;

			centermsg(va("Wave complete!", (11 - self->count)));

			self->flags &= ~FL_TOXIC;

			self->nextthink = level.time + 2;
			return;
		}

		centermsg(va("Wave %i begins in 10 seconds", (11 - self->count)));

		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, gi.soundindex("world/10_0.wav"), 1, ATTN_NONE, 0);

		// reduce count and spawn a wave
		self->delay = 1;
		self->nextthink = level.time + 10;
		return;
	}

	// keep checking last wave until all dead

	if (self->turret_ammo > 0)
	{
		self->nextthink = level.time + 2;
		return;
	}

	e = &g_edicts[game.maxclients + 1];
	for (i = game.maxclients + 1; i < globals.num_edicts; i++, e++) {
		// we still have a monster alive
		if ((e->inuse) && (e->health > 0) && (e->svflags & SVF_MONSTER) && (e->monsterinfo.aiflags2 & AI2_CONTROL))
		{
			self->nextthink = level.time + 2;
			return;
		}
	}

	G_UseTargets(self, self);


	//centermsg("All waves complete - lockdown overridden");
	if (self->moded == LK_FACT1)
	{
		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, gi.soundindex("misc/comp_up.wav"), 1.0, ATTN_NONE, 0);
		centermsg("Strogg security response defeated");
		self->eventname = "fact1$lockdown";
		doEvent(self, EVENTFLAG_NONE);
	}
	else
	{
		gi.sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1.0, ATTN_NONE, 0);
		centermsg("Bypass successful! security lasers disabled");
	}

	self->s.sound = 0;
	self->s.renderfx &= ~RF_GLOW;

	level.lockdown = 3;
}

void cpoint_playeruse(edict_t *self, edict_t *other)
{
	self->nextthink = level.time + 3;
	self->think = cpoint_think;
	self->playeruse = NULL;
	//cpoint_wave(self);
	if (self->moded == LK_FACT1)
	{
		self->s.sound = gi.soundindex("world/klaxon1.wav");
	}
	else
	{
		self->s.sound = 0;
	}

	if ((self->moded == LK_FOREVER) || (self->moded == LK_CUSTOM))
	{
		strncpy(game.helpmessage1, "Defeat infinite waves of\nincreasingly harder enemy", sizeof(game.helpmessage2) - 1);
		game.helpchanged++;
	}
	else
	{
		// no sound in infinite waves mode to try and reduce unique sound instance
		// due to limitations on number of sound indices
		gi.sound(self, CHAN_VOICE, gi.soundindex("world/uplink2.wav"), 1, ATTN_IDLE, 0);
	}

	//centermsg("Commencing hack of lockdown system");
	if ((self->moded == LK_FACT1) || ((self->moded == LK_FOREVER) || (self->moded == LK_CUSTOM)))
	{
		centermsg("Triggering security alert");
	}
	else
	{
		centermsg("Attempting bypass of security laser control");
	}

	level.lockdown = 2;
	level.lockdown_ent = self;
}

edict_t *cpoint_spawn(vec3_t origin, vec3_t angles, char *target, int difficulty)
{
	edict_t		*ent;

	ent = G_Spawn();

	VectorSet(ent->mins, -24, -24, 0);
	VectorSet(ent->maxs, 24, 24, 24);

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->classname = "control_point";
	ent->count = 10; // has to be in range 0-10 for visual bar to work without changing code above
	ent->s.modelindex = gi.modelindex("models/items/keys/target/tris.md2");
	ent->s.renderfx |= RF_GLOW;
	ent->target = target;
	ent->moded = difficulty;

	VectorCopy(origin, ent->s.origin);
	VectorCopy(angles, ent->s.angles);

	ent->playeruse = cpoint_playeruse;

	// try and reduce number of unique sounds in infinite waves mode
	if ((difficulty == LK_FOREVER) || (difficulty == LK_CUSTOM))
		ent->s.sound = gi.soundindex("world/radio3.wav");
	else
		ent->s.sound = gi.soundindex("world/comp_hum3.wav");

	gi.linkentity(ent);

	level.lockdown = 1;

	return ent;
}

ai_routing_t ai_route_list[] = {
	{qfalse,"divider",NULL,0,0,0}, // never use first entry
	// original paths
	{qtrue,"base2","path1",389.9,1867.4,-167.9},
	{qtrue,"base2","path1",369.1,1498.0,-167.9},
	{qtrue,"base2","path1",522.0,1407.4,-167.9},
	{qtrue,"base2","path1",514.3,1280.5,-167.9},
	{qtrue,"base2","path1",153.9,1269.6,-167.9},
	{qtrue,"base2","path1",97.9,1406.9,-167.9},
	{qtrue,"base2","path1",186.5,1547.6,-167.9},
	{qtrue,"base2","path1",184.5,1751.4,-167.9},
	{qtrue,"base2","path1",20.6,1829.3,-167.9},
	{qtrue,"base2","path1",-123.3,1834.0,-167.9},
	{qtrue,"base2","path1",-151.1,1770.0,-167.9},
	{qtrue,"base2","path1",-191.5,1850.3,-167.9},
	{qtrue,"base2","path1",-176.5,2279.8,-167.9},
	{qtrue,"base2","path1",8.8,2308.4,-167.9},
	{qtrue,"base2","path1",48.4,2249.4,-167.9},
	{qtrue,"base2","path1",278.4,2345.5,-231.9},
	{qtrue,"base2","path1",345.3,2224.4,-231.9},
	{qtrue,"base2","path1",569.1,2251.0,-231.9},
	/*{qfalse,"base2",NULL,0,0,0},
	{qtrue,"base2","path2",81.6,1422.1,24.1},
	{qtrue,"base2","path2",52.0,1262.1,24.1},
	{qtrue,"base2","path2",147.6,1233.0,24.1},
	{qtrue,"base2","path2",562.0,1211.0,24.1},
	{qtrue,"base2","path2",572.6,1385.3,24.1},
	{qtrue,"base2","path2",577.3,1592.4,24.1},
	{qtrue,"base2","path2",563.3,1695.9,24.1},
	{qtrue,"base2","path2",629.0,1794.9,24.1},
	{qtrue,"base2","path2",509.8,1818.3,24.1},
	{qtrue,"base2","path2",344.1,1775.0,24.1},
	{qtrue,"base2","path2",183.0,1781.5,24.1},
	{qtrue,"base2","path2",182.6,1982.3,24.1},
	{qtrue,"base2","path2",-16.0,1967.1,24.1},
	{qtrue,"base2","path2",-34.4,2066.1,24.1},
	{qtrue,"base2","path2",-34.8,2531.9,24.1},
	{qtrue,"base2","path2",-21.8,2604.6,24.1},
	{qtrue,"base2","path2",281.4,2595.9,24.1},
	{qtrue,"base2","path2",673.0,2592.4,24.1},
	{qtrue,"base2","path2",830.0,2641.0,24.1},*/
	{qfalse,"base2",NULL,0,0,0},
	{qtrue,"base2","path3",636.9,2298.8,-231.9},
	{qtrue,"base2","path3",572.3,2204.9,-231.9},
	{qtrue,"base2","path3",344.1,2232.3,-231.9},
	{qtrue,"base2","path3",288.0,2294.0,-231.9},
	{qtrue,"base2","path3",176.1,2298.6,-200.0},
	{qtrue,"base2","path3",44.8,2304.6,-167.9},
	{qtrue,"base2","path3",-169.8,2294.1,-167.9},
	{qtrue,"base2","path3",-186.1,2032.1,-167.9},
	{qtrue,"base2","path3",-186.1,1752.1,-167.9},
	{qtrue,"base2","path3",-90.4,1844.8,-167.9},
	{qtrue,"base2","path3",55.5,1856.6,-167.9},
	{qtrue,"base2","path3",311.5,1843.9,-167.9},
	{qtrue,"base2","path3",378.5,1812.0,-167.9},
	{qtrue,"base2","path3",384.5,1491.4,-167.9},
	{qtrue,"base2","path3",498.5,1381.3,-167.9},
	{qtrue,"base2","path3",450.9,1292.4,-167.9},
	{qtrue,"base2","path3",302.0,1255.6,-167.9},
	{qtrue,"base2","path3",159.4,1262.5,-167.9},
	{qtrue,"base2","path3",127.8,1371.0,-167.9},
	{qtrue,"base2","path3",187.8,1489.8,-167.9},
	{qtrue,"base2","path3",180.8,1641.5,-167.9},
	{qtrue,"base2","path3",191.8,1839.0,-167.9},
	{qtrue,"base2","path3",-156.9,1870.8,-167.9},
	{qtrue,"base2","path3",-193.5,1938.8,-167.9},
	{qtrue,"base2","path3",-182.6,2265.8,-167.9},
	{qtrue,"base2","path3",-116.1,2293.6,-167.9},
	{qtrue,"base2","path3",41.8,2311.6,-167.9},
	{qtrue,"base2","path3",185.5,2308.8,-203.1},
	{qtrue,"base2","path3",300.9,2302.6,-231.9},
	{qtrue,"base2","path3",319.4,2098.8,-231.9},
	{qfalse,"base2",NULL,0,0,0},
	{qtrue,"base2","path4",63.5,1468.1,24.1},
	{qtrue,"base2","path4",54.8,1216.0,24.1},
	{qtrue,"base2","path4",286.4,1219.9,24.1},
	{qtrue,"base2","path4",576.3,1213.8,24.1},
	{qtrue,"base2","path4",576.3,1419.4,24.1},
	{qtrue,"base2","path4",576.3,1655.9,24.1},
	{qtrue,"base2","path4",585.1,1777.5,24.1},
	{qtrue,"base2","path4",335.9,1783.3,24.1},
	{qtrue,"base2","path4",216.0,1785.5,24.1},
	{qtrue,"base2","path4",185.4,1910.8,24.1},
	{qtrue,"base2","path4",185.6,1982.0,24.1},
	{qtrue,"base2","path4",-8.8,1967.1,24.1},
	{qtrue,"base2","path4",-38.3,1991.8,24.1},
	{qtrue,"base2","path4",-37.0,2605.5,24.1},
	{qtrue,"base2","path4",197.6,2593.0,24.1},
	{qtrue,"base2","path4",413.8,2593.9,24.1},
	{qtrue,"base2","path4",584.1,2594.8,24.1},
	{qtrue,"base2","path4",840.6,2599.0,24.1},
	//additional
	{qfalse,"divider",NULL,0,0,0},
	{qtrue,"mine2","path1",1866.3,487.5,-159.9},
	{qtrue,"mine2","path1",1854.9,310.5,-159.9},
	{qtrue,"mine2","path1",1862.3,222.9,-159.9},
	{qtrue,"mine2","path1",1783.6,173.5,-159.9},
	{qtrue,"mine2","path1",1615.9,256.6,-159.9},
	{qtrue,"mine2","path1",1563.0,199.3,-159.9},
	{qtrue,"mine2","path1",1449.0,167.4,-159.9},
	{qtrue,"mine2","path1",1334.4,170.5,-159.9},
	{qtrue,"mine2","path1",1340.9,76.9,-159.9},
	{qtrue,"mine2","path1",1341.0,-112.1,-79.9},
	{qtrue,"mine2","path1",1228.5,-219.8,-79.9},
	{qtrue,"mine2","path1",1213.1,-301.0,-79.9},
	{qtrue,"mine2","path1",1092.1,-275.1,-79.9},
	{qtrue,"mine2","path1",980.1,-278.6,-79.9},
	{qtrue,"mine2","path1",946.0,-425.0,-79.9},
	{qtrue,"mine2","path1",955.4,-608.3,-79.9},
	{qtrue,"mine2","path1",767.9,-673.6,-79.9},
	{qfalse,"divider",NULL,0,0,0},
	{qtrue,"waste1","path1",-2500.0,-972.4,-439.9},
	{qtrue,"waste1","path1",-2412.9,-1321.3,-439.9},
	{qtrue,"waste1","path1",-2332.0,-1463.3,-439.9},
	{qtrue,"waste1","path1",-2196.5,-1523.3,-439.9},
	{qtrue,"waste1","path1",-2060.4,-1402.3,-439.9},
	{qtrue,"waste1","path1",-1879.4,-1394.3,-439.9},
	{qtrue,"waste1","path1",-1647.6,-1373.8,-439.9},
	{qtrue,"waste1","path1",-1483.3,-1362.9,-439.9},
	{qtrue,"waste1","path1",-1299.4,-1336.9,-471.9},
	{qtrue,"waste1","path1",-1174.6,-1327.4,-503.9},
	{qtrue,"waste1","path1",-1155.5,-1160.5,-583.9},
	{qtrue,"waste1","path1",-1279.8,-1054.5,-615.9},
	{qtrue,"waste1","path1",-1314.4,-969.8,-615.9},
	{qtrue,"waste1","path1",-1395.4,-1073.8,-615.9},
	{qtrue,"waste1","path1",-1391.3,-1161.8,-615.9},
	{qtrue,"waste1","path1",-1511.9,-1300.4,-615.9},
	{qtrue,"waste1","path1",-1693.5,-1325.9,-615.9},
	{qtrue,"waste1","path1",-1946.6,-1317.6,-615.9},
	{qtrue,"waste1","path1",-2102.0,-1323.6,-615.9},
	{qtrue,"waste1","path1",-2143.5,-1188.5,-615.9},
	{qtrue,"waste1","path1",-2066.6,-1082.6,-615.9},
	{qtrue,"waste1","path1",-2201.9,-1029.9,-615.9},
	{qtrue,"waste1","path1",-2326.9,-1013.4,-615.9},
	{qtrue,"waste1","path1",-2351.5,-1101.9,-615.9},
	{qfalse,"waste1",NULL,0,0,0},
	{qtrue,"waste1","path2",-1947.0,-986.5,-615.9},
	{qtrue,"waste1","path2",-2221.1,-1177.4,-615.9},
	{qtrue,"waste1","path2",-2238.5,-1328.4,-615.9},
	{qtrue,"waste1","path2",-2104.4,-1403.6,-615.9},
	{qtrue,"waste1","path2",-1967.9,-1434.1,-615.9},
	{qtrue,"waste1","path2",-1640.9,-1434.1,-615.9},
	{qtrue,"waste1","path2",-1374.6,-1427.6,-615.9},
	{qtrue,"waste1","path2",-1326.9,-1385.9,-615.9},
	{qtrue,"waste1","path2",-1376.4,-1300.8,-615.9},
	{qtrue,"waste1","path2",-1386.8,-1090.1,-615.9},
	{qtrue,"waste1","path2",-1245.1,-1014.0,-615.9},
	{qtrue,"waste1","path2",-1195.4,-1182.4,-567.9},
	{qtrue,"waste1","path2",-1155.5,-1313.1,-503.9},
	{qtrue,"waste1","path2",-1230.3,-1342.1,-503.9},
	{qtrue,"waste1","path2",-1307.6,-1337.6,-471.9},
	{qtrue,"waste1","path2",-1427.1,-1354.1,-439.9},
	{qtrue,"waste1","path2",-1924.6,-1413.1,-439.9},
	{qtrue,"waste1","path2",-2099.9,-1396.4,-439.9},
	{qtrue,"waste1","path2",-2306.0,-1341.5,-439.8},
	{qtrue,"waste1","path2",-2349.1,-1066.9,-439.8},
	{ qfalse,"waste1",NULL,0,0,0 },
	{qtrue,"waste1","path3",-1118.0,-767.9,-599.9},
	{qtrue,"waste1","path3",-1316.0,-785.9,-599.9},
	{qtrue,"waste1","path3",-1533.4,-774.8,-599.9},
	{qtrue,"waste1","path3",-1749.1,-771.4,-599.9},
	{qtrue,"waste1","path3",-1997.4,-771.4,-599.9},
	{ qfalse,"waste1",NULL,0,0,0 },
	{qtrue,"waste1","path4",-1832.3,-554.4,-735.9},
	{qtrue,"waste1","path4",-1472.8,-559.9,-735.9},
	{qtrue,"waste1","path4",-1309.5,-564.3,-735.9},
	{qtrue,"waste1","path4",-1206.0,-448.8,-735.9},
	{qfalse,"waste1",NULL,0,0,0 },
	{qtrue,"waste1","path5",-1790.1,-233.3,-763.9},
	{qtrue,"waste1","path5",-1893.9,-257.1,-759.9},
	{qtrue,"waste1","path5",-2069.9,-264.5,-759.9},
	{qtrue,"waste1","path5",-2217.3,-251.9,-759.9},
	{qtrue,"waste1","path5",-2503.1,-276.6,-759.9},
	{qtrue,"waste1","path5",-2530.8,-391.0,-759.9},
	{qtrue,"waste1","path5",-2515.6,-516.1,-759.9},
	{qtrue,"waste1","path5",-2470.1,-545.4,-759.9},
	{qtrue,"waste1","path5",-2343.8,-541.3,-759.9},
	{qtrue,"waste1","path5",-2325.5,-686.0,-759.9},
	{qtrue,"waste1","path5",-2330.4,-843.9,-755.9},
	{qtrue,"waste1","path5",-2370.8,-1008.0,-763.9},
	{qtrue,"waste1","path5",-2266.8,-1082.3,-763.9},
	{qtrue,"waste1","path5",-2214.6,-1103.8,-763.9},
	{qtrue,"waste1","path5",-2199.3,-1240.1,-763.9},
	{qtrue,"waste1","path5",-2197.8,-1414.6,-763.9},
	{qtrue,"waste1","path5",-2201.5,-1557.8,-763.9},
	{qtrue,"waste1","path5",-2205.0,-1681.8,-759.9},
	{qtrue,"waste1","path5",-2208.3,-1806.9,-755.9},
	{qtrue,"waste1","path5",-2255.8,-1878.9,-755.9},
	{qtrue,"waste1","path5",-2175.8,-1953.1,-755.9},
	{qfalse,"waste1",NULL,0,0,0},
	{qtrue,"waste1","path6",-1946.0,-321.9,-175.9},
	{qtrue,"waste1","path6",-1873.4,-275.3,-175.9},
	{qtrue,"waste1","path6",-1629.6,-243.6,-175.9},
	{qtrue,"waste1","path6",-1170.5,-258.9,-175.9},
	{qtrue,"waste1","path6",-832.9,-268.1,-175.9},
	{qtrue,"waste1","path6",-736.6,-169.0,-175.9},
	{qtrue,"waste1","path6",-729.8,66.5,-175.9},
	{qtrue,"waste1","path6",-577.8,58.3,-175.9},
	{qtrue,"waste1","path6",-215.0,105.0,-175.9},
	{qtrue,"waste1","path6",-14.3,98.1,-175.9},
	{qtrue,"waste1","path6",188.0,-13.5,-175.9},
	{qtrue,"waste1","path6",348.8,52.0,-175.9},
	{qtrue,"waste1","path6",307.0,188.0,-175.9},
	{qtrue,"waste1","path6",-197.0,106.6,-175.9},
	{qtrue,"waste1","path6",-686.3,108.6,-175.9},
	{qtrue,"waste1","path6",-740.4,207.8,-175.9},
	{qtrue,"waste1","path6",-715.0,453.1,-175.9},
	{qtrue,"waste1","path6",-719.5,609.9,-175.9},
	{qtrue,"waste1","path6",-723.9,785.8,-175.9},
	{qtrue,"waste1","path6",-747.0,895.8,-175.9},
	{qtrue,"waste1","path6",-762.4,960.3,-175.9},
	{qtrue,"waste1","path6",-611.0,955.6,-175.9},
	{qtrue,"waste1","path6",-229.8,956.9,-175.9},
	{qtrue,"waste1","path6",-703.3,962.6,-175.9},
	{qtrue,"waste1","path6",-726.1,763.8,-175.9},
	{qtrue,"waste1","path6",-781.1,601.8,-175.9},
	{qtrue,"waste1","path6",-1141.5,593.5,-175.9},
	{qtrue,"waste1","path6",-1409.9,563.8,-175.9},
	{qtrue,"waste1","path6",-1572.8,496.8,-175.9},
	{qtrue,"waste1","path6",-1577.3,625.1,-175.9},
	{qtrue,"waste1","path6",-1495.9,754.3,-175.9},
	{qtrue,"waste1","path6",-1532.8,911.3,-175.9},
	{qfalse,"waste1",NULL,0,0,0},
	{qtrue,"waste1","path7",-1478.8,902.1,-175.9},
	{qtrue,"waste1","path7",-1500.4,696.4,-175.9},
	{qtrue,"waste1","path7",-1635.3,525.4,-175.9},
	{qtrue,"waste1","path7",-1472.0,534.5,-175.9},
	{qtrue,"waste1","path7",-1171.8,622.1,-175.9},
	{qtrue,"waste1","path7",-757.1,627.6,-175.9},
	{qtrue,"waste1","path7",-749.3,911.4,-175.9},
	{qtrue,"waste1","path7",-773.3,1099.0,-175.9},
	{qtrue,"waste1","path7",-711.0,959.6,-175.9},
	{qtrue,"waste1","path7",-563.9,958.6,-175.9},
	{qtrue,"waste1","path7",-248.5,958.6,-175.9},
	{qtrue,"waste1","path7",-137.6,997.5,-175.9},
	{qtrue,"waste1","path7",-293.5,954.4,-175.9},
	{qtrue,"waste1","path7",-689.9,954.4,-175.9},
	{qtrue,"waste1","path7",-746.1,664.9,-175.9},
	{qtrue,"waste1","path7",-730.0,449.5,-175.9},
	{qtrue,"waste1","path7",-778.0,217.9,-175.9},
	{qtrue,"waste1","path7",-651.0,95.0,-175.9},
	{qtrue,"waste1","path7",-468.6,146.4,-175.9},
	{qtrue,"waste1","path7",-212.8,131.9,-175.9},
	{qtrue,"waste1","path7",56.0,162.9,-175.9},
	{qtrue,"waste1","path7",197.5,11.6,-175.9},
	{qtrue,"waste1","path7",-192.0,93.1,-175.9},
	{qtrue,"waste1","path7",-690.6,73.8,-175.9},
	{qtrue,"waste1","path7",-713.8,-199.3,-175.9},
	{qtrue,"waste1","path7",-864.9,-233.1,-175.9},
	{qtrue,"waste1","path7",-1124.8,-248.4,-175.9},
	{qtrue,"waste1","path7",-1486.0,-225.6,-175.9},
	{qtrue,"waste1","path7",-1728.4,-213.0,-175.9},
	{qtrue,"waste1","path7",-1903.6,-228.0,-175.9},
	{qtrue,"waste1","path7",-1972.1,-135.9,-175.9},
	{qfalse,"waste1",NULL,0,0,0},
	{qtrue,"waste1","path8",-2131.8,-256.1,-175.9},
	{qtrue,"waste1","path8",-2307.6,-241.4,-175.9},
	{qtrue,"waste1","path8",-2397.0,-292.8,-175.8},
	{qtrue,"waste1","path8",-2400.3,-444.3,-175.8},
	{qtrue,"waste1","path8",-2402.9,-580.8,-175.8},
	{qtrue,"waste1","path8",-2298.0,-664.6,-175.8},
	{qtrue,"waste1","path8",-2286.3,-744.8,-175.9},
	{ qfalse,"divider",NULL,0,0,0 },
	{ qtrue,"fact1","path1",292.5,420.3,-583.9 },
	{ qtrue,"fact1","path1",388.4,418.3,-551.9 },
	{ qtrue,"fact1","path1",434.8,418.3,-543.9 },
	{ qtrue,"fact1","path1",962.8,422.5,-555.9 },
	{ qtrue,"fact1","path1",968.5,590.5,-553.4 },
	{ qfalse,"fact1",NULL,0,0,0 },
	{ qtrue,"fact1","path2",-289.9,145.5,24.1 },
	{ qtrue,"fact1","path2",-12.4,135.8,24.1 },
	{ qtrue,"fact1","path2",-7.9,-93.9,24.1 },
	{ qtrue,"fact1","path2",-1.8,458.6,24.1 },
	{ qtrue,"fact1","path2",-12.5,880.6,24.1 },
	{ qtrue,"fact1","path2",-337.9,899.1,24.1 },
	{ qtrue,"fact1","path2",-571.5,895.9,24.1 },
	{ qtrue,"fact1","path2",-607.1,705.5,24.1 },
	{ qtrue,"fact1","path2",-598.6,533.1,24.1 },
	{ qtrue,"fact1","path2",-452.5,501.6,24.1 },
	{ qtrue,"fact1","path2",-253.9,505.6,24.1 },
	{ qtrue,"fact1","path2",0.5,505.6,24.1 },
	{qfalse,NULL,NULL,0,0,0}
};