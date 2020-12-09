#include "g_local.h"

// For adding items to maps at load time

//Drop_Item(ent, FindItemByClassname("item_resupply"));
//SpawnItem(self, FindItem("Health"));

/*fitem = FindItem("Armor Shard");
index = ITEM_INDEX(fitem);
attacker->client->pers.inventory[index] += 2;

gi.sound(attacker, CHAN_ITEM, gi.soundindex("misc/ar2_pkup.wav"), 1, ATTN_NORM, 0);

attacker->client->bonus_alpha = 0.25;

attacker->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex("i_jacketarmor");
attacker->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS + ITEM_INDEX(FindItem("Armor Shard"));
attacker->client->pickup_msg_time = level.time + 3.0;*/


edict_t *Add_Item(gitem_t *item, vec3_t start, int quantity)
{
	edict_t *dropped;
	dropped = G_Spawn();
	dropped->classname = item->classname;

	//dropped->spawnflags = DROPPED_ITEM;
	gi.setmodel(dropped, item->world_model);

	VectorCopy(start, dropped->s.origin);

	dropped->count = quantity;

	gi.linkentity(dropped);

	SpawnItem(dropped, item);

	return dropped;
}

void addItems(void)
{
	vec3_t		start;
	edict_t		*ent;

	//if (Q_stricmp(level.mapname, "fact1") == 0) {
	//}

	if (deathmatch->value)
		return;

	if (Q_stricmp(level.mapname, "base1") == 0) {
		//VectorSet(start, 2, 43, 46);
		//Add_Item(FindItemByClassname("item_resupply"), start, 1);
		//VectorSet(start, 2, 43, 46);
		//Add_Item(FindItemByClassname("item_tombu"), start, 1);

		VectorSet(start, -1226, 1430, 152);
		Add_Item(FindItemByClassname("ammo_shells"), start, 1);

		VectorSet(start, 755.0, 1088.4, -212.0);
		Add_Item(FindItemByClassname("ammo_shells"), start, 1);

		VectorSet(start, 416.9, 1121.5, -240.0);
		Add_Item(FindItemByClassname("ammo_shells"), start, 1);

		VectorSet(start, -1180.4, 1408.2, -30.0);
		Add_Item(FindItemByClassname("ammo_shells"), start, 1);
	}

	if (Q_stricmp(level.mapname, "base2") == 0) {
		VectorSet(start, 704, -709, -76);
		Add_Item(FindItemByClassname("item_resupply"), start, 1);
	}

	if (Q_stricmp(level.mapname, "base3") == 0) {
		VectorSet(start, 1824, 225, -257);
		Add_Item(FindItemByClassname("ammo_bullets"), start, 1);

		VectorSet(start, 1820, 159, -257);
		Add_Item(FindItemByClassname("ammo_bullets"), start, 1);

		VectorSet(start, 1126, 288, -465);
		Add_Item(FindItemByClassname("ammo_shells"), start, 1);
	}

	if (Q_stricmp(level.mapname, "bunk1") == 0) {
		VectorSet(start, -1476, 1249, 24);
		Add_Item(FindItemByClassname("item_resupply"), start, 1);

		VectorSet(start, -500, 1665, 36);
		Add_Item(FindItemByClassname("item_armor_combat"), start, 1);
	}

	if (Q_stricmp(level.mapname, "ware1") == 0) {
		VectorSet(start, -1227, -1279, -73);
		Add_Item(FindItemByClassname("item_resupply"), start, 1);

		VectorSet(start, -759, -516, -68);
		Add_Item(FindItemByClassname("item_tombu"), start, 1);
	}

	if (Q_stricmp(level.mapname, "jail1") == 0) {
		VectorSet(start, -2051, -707, -65);
		Add_Item(FindItemByClassname("item_resupply"), start, 1);

		VectorSet(start, -34, -3410, -74);
		Add_Item(FindItemByClassname("item_resupply"), start, 1);

		VectorSet(start, -2924, -555, 48);
		Add_Item(FindItemByClassname("ammo_bullets"), start, 1);

		VectorSet(start, -2867, -555, 48);
		Add_Item(FindItemByClassname("ammo_bullets"), start, 1);

		VectorSet(start, -2530, 49, 48);
		Add_Item(FindItemByClassname("ammo_shells"), start, 1);
	}

	if (Q_stricmp(level.mapname, "jail2") == 0) {
		VectorSet(start, -1467, 810, 110);
		Add_Item(FindItemByClassname("item_stimu"), start, 1);
	}

	if (Q_stricmp(level.mapname, "jail5") == 0) {
		VectorSet(start, 961, 3107, -68);
		Add_Item(FindItemByClassname("item_resupply"), start, 1);

		VectorSet(start, 166, 484, -359);
		Add_Item(FindItemByClassname("item_resupply"), start, 1);
	}

	if (Q_stricmp(level.mapname, "security") == 0) {
		VectorSet(start, 480, 3326, -7);
		Add_Item(FindItemByClassname("item_healthu"), start, 1);
	}

	if (Q_stricmp(level.mapname, "fact1") == 0) {

		VectorSet(start, 1502.2, 863.4, -70);
		Add_Item(FindItemByClassname("ammo_bullets"), start, 1);

		VectorSet(start, 1697.5, 991.5, -70);
		Add_Item(FindItemByClassname("ammo_shells"), start, 1);

		VectorSet(start, 948.6, 416.6, -555.0);
		Add_Item(FindItemByClassname("item_resupply"), start, 1);

		VectorSet(start, -738, 630, 16);
		Add_Item(FindItemByClassname("item_resupply"), start, 1);
	}

	if (Q_stricmp(level.mapname, "strike") == 0) {
		VectorSet(start, -2044, -445, 90);
		Add_Item(FindItemByClassname("item_resupply"), start, 1);
	}

	if (Q_stricmp(level.mapname, "mine1") == 0) {
		//VectorSet(start, -376, -1955, 990);
		//Add_Item(FindItemByClassname("item_grenadeu"), start, 1);
	}

	if (Q_stricmp(level.mapname, "mine2") == 0) {
		VectorSet(start, -519, 581, -428);
		Add_Item(FindItemByClassname("item_tripu"), start, 1);

		VectorSet(start, 88, 945, -359);
		Add_Item(FindItemByClassname("item_resupply"), start, 1);
	}

	if (Q_stricmp(level.mapname, "mine3") == 0) {
		VectorSet(start, -1054, 847, -485);
		Add_Item(FindItemByClassname("item_resupply"), start, 1);
	}

	if (Q_stricmp(level.mapname, "mine4") == 0) {
		VectorSet(start, -747, -197, 383);
		Add_Item(FindItemByClassname("item_slingpack"), start, 1);
	}

	if (Q_stricmp(level.mapname, "train") == 0) {
		//VectorSet(start, 2, 43, 46);
		//Add_Item(FindItemByClassname("item_resupply"), start, 1);
		//VectorSet(start, -319, -1066, -232);
		//Add_Item(FindItemByClassname("item_cybernetics"), start, 1);

		VectorSet(start, -319, -1066, -232);
		Add_Item(FindItemByClassname("item_slingpack"), start, 1);

		VectorSet(start, -979, 270, 73);
		Add_Item(FindItemByClassname("item_resupply"), start, 1);
	}

	if (Q_stricmp(level.mapname, "power2") == 0) {
		VectorSet(start, -1451, 1266, -274);
		Add_Item(FindItemByClassname("item_pack"), start, 1);
	}

	if (Q_stricmp(level.mapname, "waste1") == 0) {
		VectorSet(start, -1809, -170, -338);
		Add_Item(FindItemByClassname("item_glu"), start, 1);

		VectorSet(start, -2183, 1612, -481);
		Add_Item(FindItemByClassname("item_tombu"), start, 1);
	}

	if (Q_stricmp(level.mapname, "waste3") == 0) {
		VectorSet(start, -584, -2014, -225);
		Add_Item(FindItemByClassname("item_grenadeu"), start, 1);
	}

	if (Q_stricmp(level.mapname, "biggun") == 0) {
		VectorSet(start, -584, -2014, -225);
		Add_Item(FindItemByClassname("item_sabotu"), start, 1);
	}

	if (Q_stricmp(level.mapname, "cool1") == 0) {
		VectorSet(start, -1756, -1278, -831);
		Add_Item(FindItemByClassname("item_solaru"), start, 1);

		VectorSet(start, -1372, -1008, -1193);
		Add_Item(FindItemByClassname("item_pack"), start, 1);

		VectorSet(start, -2154, 10, -1129);
		Add_Item(FindItemByClassname("item_slingpack"), start, 1);
	}

	if (Q_stricmp(level.mapname, "hangar1") == 0) {
		VectorSet(start, 1046, -478, 1459);
		Add_Item(FindItemByClassname("item_adu"), start, 1);
	}

	if (Q_stricmp(level.mapname, "hangar2") == 0) {
		VectorSet(start, 1368.1, 1115.3, -465.8);
		Add_Item(FindItemByClassname("item_resupply"), start, 1);

		VectorSet(start, 153.7, -1859.9, -1297.8);
		Add_Item(FindItemByClassname("item_pack"), start, 1);

		VectorSet(start, 951.6, -1647.3, -81.8);
		Add_Item(FindItemByClassname("item_pack"), start, 1);

		VectorSet(start, 1888, -942, -1377);
		Add_Item(FindItemByClassname("item_adu"), start, 1);
	}

	if (Q_stricmp(level.mapname, "command") == 0) {
		VectorSet(start, 200, 710, 446);
		Add_Item(FindItemByClassname("item_resupply"), start, 1);

		VectorSet(start, -124, -287, 46);
		Add_Item(FindItemByClassname("item_pack"), start, 1);
	}

	if (Q_stricmp(level.mapname, "lab") == 0) {
		VectorSet(start, -780, 59, 226);
		Add_Item(FindItemByClassname("item_healthu"), start, 1);

		VectorSet(start, -1700, 1384, -417);
		Add_Item(FindItemByClassname("item_stimu"), start, 1);

		VectorSet(start, -306, 502, 38);
		Add_Item(FindItemByClassname("item_solaru"), start, 1);

		VectorSet(start, -306, 368, 38);
		Add_Item(FindItemByClassname("item_blasteru"), start, 1);
	}

	if (Q_stricmp(level.mapname, "city1") == 0) {
		VectorSet(start, -782, -2595, 1105);
		Add_Item(FindItemByClassname("item_quad"), start, 1);

		VectorSet(start, -1263, 418, 60);
		Add_Item(FindItemByClassname("item_pack"), start, 1);

		VectorSet(start, -2128, -1926, 408);
		Add_Item(FindItemByClassname("item_pack"), start, 1);
	}

	if (Q_stricmp(level.mapname, "boss2") == 0) {
		VectorSet(start, -597.7, -961.2, 197.9);
		Add_Item(FindItemByClassname("item_resupply"), start, 1);

		VectorSet(start, -295.8, -979.2, 76.9);
		Add_Item(FindItemByClassname("item_pack"), start, 1);

		VectorSet(start, -608, -954, 76.9);
		Add_Item(FindItemByClassname("item_pack"), start, 1);
	}
}