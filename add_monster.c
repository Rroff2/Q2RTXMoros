#include "g_local.h"

// This file has all our messy per-map hardcoded monster spawns

// Change up the classname of spawning monsters in some cases to increase difficulty
// check if there is enough space for a bigger monster in the place of a smaller one
// if necessary - don't perform such a check unless changing to a bigger monster
// if we don't return after a change make sure we don't change to a monster
// that then gets changed further down the function again

// ** This (ModifySpawnMonster) isn't used currently as it seems to cause issues when a map reloads from a 
// save game or gamemap command such as entites changing origin unexpectedly and incorrect
// sounds being played for an action **

void ModifySpawnMonster(edict_t *ent)
{
	vec3_t		mins, maxs, start, end;
	qboolean	bigspawn = qfalse;

	trace_t		tr;

	if (!Q_stricmp(ent->classname, "monster_soldier_ss"))
	{
		if (random() < 0.3)
		{
			ent->classname = "monster_infantry";
			return;
		}
	}

	if (!Q_stricmp(ent->classname, "monster_soldier_light"))
	{
		if (random() < 0.7)
		{
			if (random() < 0.6)
			{
				ent->classname = "monster_berserk";
				return;
			}
		}
		else {
			ent->classname = "monster_infantry";
			return;
		}
	}

	if (!Q_stricmp(ent->classname, "monster_soldier"))
	{
		ent->classname = "monster_infantry";
		return;
	}

	/*VectorSet(mins, -32, -32, -24);
	VectorSet(maxs, 32, 32, 32);
	VectorCopy(ent->s.origin, start);
	VectorCopy(ent->s.origin, end);

	// ent isn't really valid here
	tr = gi.trace(start, mins, maxs, end, ent, MASK_MONSTERSOLID);*/

}

// This isn't really an add monster thing but could get messy so added here
// it would also call the above function to modify monster if we ever fix that
// returns qfalse unless the entity should be removed in which case it returns qtrue

// Func_wall spawnflags 7 will make them trigger on their targetname starting on

qboolean ModifySpawnEnt(edict_t *ent)
{
	if (deathmatch->value)
		return qfalse;

	if (Q_stricmp(level.mapname, "base2") == 0)
	{
		if (Q_stricmp(ent->classname, "func_wall") == 0 && ent->model)
		{
			// Add the ladder back into the base2 secret
			if (Q_stricmp(ent->model, "*6") == 0)
			{
				ent->spawnflags = 0;
				return qfalse;
			}
			// Remove the ceiling at the top of the ladder above
			if (Q_stricmp(ent->model, "*10") == 0)
			{
				return qtrue; // alt set spawnflags 1792
			}
			//pedestal in the water
			if (Q_stricmp(ent->model, "*11") == 0)
			{
				ent->spawnflags = 0;
				return qfalse;
			}
		}
	}

	if (Q_stricmp(level.mapname, "base3") == 0)
	{
		if (Q_stricmp(ent->classname, "func_wall") == 0 && ent->model)
		{
			// Add back the large box in courtyard
			if (Q_stricmp(ent->model, "*5") == 0)
			{
				ent->spawnflags = 0;
				return qfalse;
			}

			// Add the ladder back in base3
			if (Q_stricmp(ent->model, "*6") == 0)
			{
				ent->spawnflags = 0;
				ent->s.origin[0] += 1616;
				ent->s.origin[1] += 192;
				ent->s.origin[2] -= 108;

				return qfalse;
			}
		}
	}

	if (Q_stricmp(level.mapname, "bunk1") == 0)
	{
		if (Q_stricmp(ent->classname, "func_wall") == 0 && ent->model)
		{
			if (Q_stricmp(ent->model, "*8") == 0)
			{
				ent->spawnflags = 7;
				ent->targetname = "t157";
				return qfalse;
			}
		}
	}

	if (Q_stricmp(level.mapname, "strike") == 0)
	{
		if (Q_stricmp(ent->classname, "func_wall") == 0 && ent->model)
		{
			if (Q_stricmp(ent->model, "*10") == 0)
			{
				ent->spawnflags = 0;
				ent->s.origin[0] -= 16;
				ent->s.origin[1] += 100;
				ent->s.origin[2] += 200;
				return qfalse;
			}

			if (Q_stricmp(ent->model, "*11") == 0)
			{
				ent->spawnflags = 0;
				return qfalse;
			}

			if (Q_stricmp(ent->model, "*12") == 0)
			{
				ent->spawnflags = 0;
				return qfalse;
			}

			if (Q_stricmp(ent->model, "*14") == 0)
			{
				ent->s.origin[0] -= 590;
				ent->s.origin[1] += 48;
				ent->s.origin[2] += 300;
				ent->spawnflags = 0;
				return qfalse;
			}

			if (Q_stricmp(ent->model, "*15") == 0)
			{
				ent->spawnflags = 0;
				return qfalse;
			}
			if (Q_stricmp(ent->model, "*16") == 0)
			{
				ent->spawnflags = 0;
				ent->s.origin[0] -= 8;
				ent->s.origin[2] += 480;
				return qfalse;
			}
			if (Q_stricmp(ent->model, "*17") == 0)
			{
				ent->spawnflags = 0;
				return qfalse;
			}
		}

		if (Q_stricmp(ent->classname, "misc_teleporter") == 0 && ent->target)
		{
			if (Q_stricmp(ent->target, "t206") == 0)
			{
				ent->s.origin[0] = -523;
				ent->s.origin[1] = 590;
				ent->s.origin[2] = 174;

				ent->spawnflags = 0;
				return qfalse;
			}

			if (Q_stricmp(ent->target, "t205") == 0)
			{
				ent->spawnflags = 0;
				return qfalse;
			}
		}

		if (Q_stricmp(ent->classname, "misc_teleporter_dest") == 0 && ent->targetname)
		{
			if (Q_stricmp(ent->targetname, "t206") == 0)
			{
				ent->spawnflags = 0;
				ent->s.origin[0] = -2162;
				ent->s.origin[1] = -208;
				ent->s.origin[2] = 238;
				return qfalse;
			}

			if (Q_stricmp(ent->targetname, "t205") == 0)
			{
				ent->spawnflags = 0;
				ent->s.origin[0] = -523;
				ent->s.origin[1] = 494;
				ent->s.origin[2] = 174;
				return qfalse;
			}
		}
	}

	if (Q_stricmp(level.mapname, "train") == 0)
	{
		if (Q_stricmp(ent->classname, "func_wall") == 0 && ent->model)
		{
			//platform ledge in the big cave room
			if (Q_stricmp(ent->model, "*28") == 0)
			{
				ent->spawnflags = 0;
				return qfalse;
			}
		}
	}

	if (Q_stricmp(level.mapname, "boss1") == 0)
	{
		if (Q_stricmp(ent->classname, "func_wall") == 0 && ent->model)
		{
			if (Q_stricmp(ent->model, "*10") == 0)
			{
				return qtrue;
			}
		}

		if (Q_stricmp(ent->classname, "func_wall") == 0 && ent->model)
		{
			if (Q_stricmp(ent->model, "*11") == 0)
			{
				return qtrue;
			}
		}
	}

	if (Q_stricmp(level.mapname, "jail3") == 0)
	{
		if (Q_stricmp(ent->classname, "misc_deadsoldier") == 0)
		{
			if ((ent->s.origin[0] == -1764) && (ent->s.origin[1] == 228) && (ent->s.origin[2] == 778))
				return qtrue;
		}
	}

	if (Q_stricmp(level.mapname, "city1") == 0)
	{
		if (Q_stricmp(ent->classname, "misc_teleporter") == 0 && ent->target)
		{
			if (Q_stricmp(ent->target, "t283") == 0)
			{
				VectorSet(ent->s.origin, 270, -2633, 656);
				ent->spawnflags = 0;
				return qfalse;
			}

			if (Q_stricmp(ent->target, "t304") == 0)
			{
				VectorSet(ent->s.origin, -1928, -1923, 376);
				ent->spawnflags = 0;
				return qfalse;
			}
		}

		if (Q_stricmp(ent->classname, "misc_teleporter_dest") == 0 && ent->targetname)
		{
			if (Q_stricmp(ent->targetname, "t282") == 0)
			{
				VectorSet(ent->s.origin, -2189, -1927, 376);
				ent->spawnflags = 0;
				return qfalse;
			}

			if (Q_stricmp(ent->targetname, "t304") == 0)
			{
				ent->spawnflags = 0;
				return qfalse;
			}
		}

		/*if (Q_stricmp(ent->classname, "func_wall") == 0 && ent->model)
		{
			if (Q_stricmp(ent->model, "*33") == 0)
			{
				return qtrue;
			}
		}*/


		if (Q_stricmp(ent->classname, "trigger_push") == 0 && ent->model)
		{
			if (Q_stricmp(ent->model, "*56") == 0)
			{
				return qtrue;
			}
		}

		if (Q_stricmp(ent->classname, "func_door") == 0 && ent->model)
		{
			if (Q_stricmp(ent->model, "*30") == 0)
			{
				ent->targetname = "t10";
				return qfalse;
			}

			if (Q_stricmp(ent->model, "*31") == 0)
			{
				ent->targetname = "t10";
				return qfalse;
			}
		}
	}

	/*if (Q_stricmp(level.mapname, "city3") == 0)
	{
		if (ent->model)
		{
			if (Q_stricmp(ent->model, "*148") == 0)
			{
				return qtrue;
			}
		}
	}*/

	return qfalse;
}

// Spawns a monster into the world outside of map definitions
// this function assumes whatever called it did checking of position and contents

edict_t *Add_MonsterTele(char *classname, char *itemname, vec3_t start, vec3_t spawn_angles, int event, char *eventname, float distance, qboolean roamer)
{
	edict_t		*ent;

	ent = Add_Monster(classname, itemname, start, spawn_angles, event, eventname, distance, roamer);

	if (ent->spawnflags & SPAWNFLAG_EVENT)
		ent->spawnflags |= SPAWNFLAG_EVENT_TELE;

	return ent;
}

edict_t *Add_Monster(char *classname, char *itemname, vec3_t start, vec3_t spawn_angles, int event, char *eventname, float distance, qboolean roamer)
{
	edict_t		*ent;

	ent = G_Spawn();
	ent->classname = classname;

	VectorCopy(start, ent->s.origin);
	VectorCopy(spawn_angles, ent->s.angles);

	//gi.dprintf("Add_Monster %s at %s\n", classname, vtos(ent->s.origin));

	// Rroff add a flag passed to function for this?
	ent->spawnflags |= 1;

	if (roamer)
		ent->monsterinfo.aiflags |= AI_ROAMING;

	if (event)
	{
		ent->monsterinfo.eventtype = event;
		ent->monsterinfo.eventname = eventname;
		ent->spawnflags |= (2 | SPAWNFLAG_EVENT);
		ent->wait = distance; // how far from event - maybe move to monsterinfo?
	}

	ED_CallSpawn(ent);

	gi.unlinkentity(ent);
	KillBox(ent);
	gi.linkentity(ent);

	if (itemname)
		ent->item = FindItemByClassname(itemname);

	return ent;
}

edict_t *Add_Grunt(char *classname, char *itemname, vec3_t start, vec3_t spawn_angles, int event, char *eventname, float distance, char *gruntname, int weapon)
{
	edict_t		*ent;

	ent = G_Spawn();
	ent->classname = classname;

	VectorCopy(start, ent->s.origin);
	VectorCopy(spawn_angles, ent->s.angles);

	//gi.dprintf("Add_Monster %s at %s\n", classname, vtos(ent->s.origin));

	// Rroff add a flag passed to function for this?
	ent->spawnflags |= 1;

	if (event)
	{
		ent->monsterinfo.eventtype = event;
		ent->monsterinfo.eventname = eventname;
		ent->spawnflags |= (2 | SPAWNFLAG_EVENT);
		ent->wait = distance; // how far from event - maybe move to monsterinfo?
	}

	ent->monsterinfo.grunt_name = gruntname;
	ent->monsterinfo.grunt_weapon = weapon;

	if (Q_stricmp(gruntname, "willits") == 0)
	{
		//ent->monsterinfo.aiflags |= AI_STAND_GROUND;
		ent->monsterinfo.power_armor_power = 200;
		ent->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
	}

	ED_CallSpawn(ent);

	gi.unlinkentity(ent);
	KillBox(ent);
	gi.linkentity(ent);

	if (itemname)
		ent->item = FindItemByClassname(itemname);

	return ent;
}

// Spawn a roaming monster

void spawn_roamer(edict_t *self)
{
	edict_t		*ent, *spot;
	vec3_t		mins, maxs, d, start, end, s;
	trace_t		tr;
	int			contents;
	float		chance;
	float		chancevh = 0.01;
	float		chancefh = 0.75;
	float		chancenh = 0.5;
	qboolean	bullets = qfalse;
	qboolean	noammo = qfalse;

	// Rroff - we use pos1 instead of origin as some stuff moves later
	// for various reasons, pos1 should be updated before this is called

	// Don't spawn within 200 units of a player start position


	// Don't want to go too nuts (edit this later)

	if (nomonsters->value)
		return;

	if (!g_moremonsters->value)
	{
		if (level.total_monsters > 63)
			return;
	}

	// Some maps already have a lot of monsters and/or are pushing engine constraints
	// such as max edicts like space so don't spawn roaming monsters on those
	if (Q_stricmp(level.mapname, "space") == 0)
		return;

	if ((Q_stricmp(level.mapname, "sewer64") == 0) && (g_hordemode->value))
		return;

	if (g_hordecustom->value)
		return;

	// Try and not get too close to max edicts - some maps don't leave much headroom
	// this might make the check above a bit redundant

	//if (globals.num_edicts > 840)
	if (level.num_edicts > 840)
		return;

	// check just above to make sure it isn't in a solid even if we can find a spawn above
	// then below to eliminate dead soldiers on rocks in the middle of lava, etc.
	VectorCopy(self->pos1, s);

	s[2] += 17;

	contents = gi.pointcontents(s);

	if (contents & MASK_MONSTERSOLID)
	{
		return;
	}

	// rather crude tests to eliminate known problem spots

	s[1] += 48;
	s[2] -= 64;

	contents = gi.pointcontents(s);

	if (contents & MASK_WATER)
	{
		return;
	}

	s[0] += 45;
	s[2] += 24;

	contents = gi.pointcontents(s);

	if (contents & MASK_WATER)
	{
		return;
	}

	spot = NULL;

	while (1) {
		spot = G_Find(spot, FOFS(classname), "info_player_start");
		if (!spot)
			break;
		VectorSubtract(self->pos1, spot->s.origin, d);
		if (VectorLength(d) < 200) {
			return;
		}
	}

	if (coop->value || g_hardmonsters->value)
	{
		chancevh = 0.02;
		chancefh = 0.6;
		chancenh = 0.38;
	}

	VectorSet(mins, -16, -16, -24);
	VectorSet(maxs, 16, 16, 32);

	VectorCopy(self->pos1, start);
	VectorCopy(self->pos1, end);

	// should push down from above the potential spawn origin
	// try and find a spot on the ground

	//end[2] += 56;
	end[2] += 80;

	tr = gi.trace(start, mins, maxs, end, NULL, MASK_MONSTERSOLID);

	// why not use vectorcopy?
	s[0] = tr.endpos[0];
	s[1] = tr.endpos[1];
	s[2] = tr.endpos[2];
	contents = gi.pointcontents(s);

	if (contents & MASK_WATER)
	{
		//gi.dprintf("Tried to spawn in water\n");
		return;
	}

	if (tr.allsolid)
		return;

	VectorCopy(tr.endpos, start);
	VectorCopy(tr.endpos, end);
	end[2] -= 384;

	tr = gi.trace(tr.endpos, mins, maxs, end, NULL, MASK_MONSTERSOLID);

	s[0] = tr.endpos[0];
	s[1] = tr.endpos[1];
	s[2] = tr.endpos[2];
	contents = gi.pointcontents(s);

	// shouldn't be allsolid
	if ((contents & MASK_WATER) || (tr.fraction == 1.0) || tr.startsolid || tr.allsolid)
	{
		return;
	}

	ent = G_Spawn();

	if (random() < chancevh)
	{
		if (random() < 0.6)
		{
			ent->classname = "monster_brain";
			if (random() < 0.2) {
				ent->item = FindItemByClassname("item_quad");
			}
			else {
				ent->item = FindItemByClassname("item_armor_jacket");
			}
		}
		else {
			ent->classname = "monster_floater";
			if (random() < 0.2) {
				ent->item = FindItemByClassname("item_ancient_head");
			}
			else {
				ent->item = FindItemByClassname("item_adrenaline");
			}
		}
	} else {
		chance = random();
		if (chance < chancenh)
		{
			ent->classname = "monster_soldier";
		} else if (chance < 0.8) {
			ent->classname = "monster_soldier_ss";
			bullets = qtrue;
		} else {
			if (random() < chancefh)
			{
				ent->classname = "monster_infantry";
				bullets = qtrue;
			} else {
				if (random() < 0.5)
				{
					ent->classname = "monster_berserk";
					noammo = qtrue;
				}
				else
				{
					ent->classname = "monster_gunner";
					bullets = qtrue;
				}
			}
		}

		if (random() < 0.4)
		{
			ent->item = FindItemByClassname("item_armor_shard");
		} else {
			if (random() < 0.2)
			{
				ent->item = FindItemByClassname("ammo_grenades");
			} else {
				if (random() < 0.5)
				{
					if (!noammo)
					{
						if (bullets)
							ent->item = FindItemByClassname("ammo_bullets");
						else
							ent->item = FindItemByClassname("ammo_shells");
					} else {
						if (random()<0.3)
							ent->item = FindItemByClassname("item_armor_shard");
					}
				}
			}
		}
	}

	//VectorCopy(start, ent->s.origin);
	VectorCopy(tr.endpos, ent->s.origin);
	VectorCopy(self->s.angles, ent->s.angles);

	ent->spawnflags |= 1;
	ent->monsterinfo.aiflags |= AI_ROAMING;

	ED_CallSpawn(ent);

	//gi.unlinkentity(ent); // Probably not needed with the box check above
	//KillBox(ent);
	//gi.linkentity(ent);
}

// hardcoded additional/event monsters per map
// probably move these out to external files and provide and interface later

void addMonsters(void)
{
	vec3_t		start, spawn_angles, mins, maxs;
	edict_t		*ent;
	edict_t		*spot;

	// For angles the middle value is the yaw - the monster left is 90, right 270 when looking 0
	// For origin the last value is the height

	if (nomonsters->value)
		return;

	// doesn't really belong in add_monster at all except it is PVE related

	if (Q_stricmp(level.mapname, "sewer64") == 0) {
		if (g_hordemode->value)
		{
			VectorSet(start, -602, 745, 514);
			VectorSet(spawn_angles, 0, 270, 0);
			cpoint_spawn(start, spawn_angles, "lockdown$sewer64", LK_FOREVER);

			spot = G_Spawn();
			spot->classname = "info_player_coop";
			spot->s.origin[0] = -435;
			spot->s.origin[1] = 505;
			spot->s.origin[2] = 558;
			//spot->targetname = "sewer64";
			spot->s.angles[1] = 120;

			spot = G_Spawn();
			spot->classname = "info_player_coop";
			spot->s.origin[0] = -572;
			spot->s.origin[1] = 390;
			spot->s.origin[2] = 558;
			//spot->targetname = "sewer64";
			spot->s.angles[1] = 88;

			spot = G_Spawn();
			spot->classname = "info_player_coop";
			spot->s.origin[0] = -690;
			spot->s.origin[1] = 585;
			spot->s.origin[2] = 558;
			//spot->targetname = "sewer64";
			spot->s.angles[1] = 27;

			spot = G_Spawn();
			spot->classname = "info_player_coop";
			spot->s.origin[0] = -713;
			spot->s.origin[1] = 700;
			spot->s.origin[2] = 558;
			//spot->targetname = "sewer64";
			spot->s.angles[1] = -9;

			spot = G_Spawn();
			spot->classname = "info_player_coop";
			spot->s.origin[0] = -434;
			spot->s.origin[1] = 761;
			spot->s.origin[2] = 558;
			//spot->targetname = "sewer64";
			spot->s.angles[1] = -138;

			spot = G_Spawn();
			spot->classname = "info_player_coop";
			spot->s.origin[0] = -574;
			spot->s.origin[1] = 872;
			spot->s.origin[2] = 640;
			//spot->targetname = "sewer64";
			spot->s.angles[1] = -92;

		}
	}

	// ===================================================================================================================
	// BASE1
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "base1") == 0) {

		VectorSet(start, 1202, 185, -16);
		VectorSet(spawn_angles, 0, 180, 0);
		Add_Monster("monster_infantry", "key_head", start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 637, 7, -138);
		VectorSet(spawn_angles, 0, 0, 0);
		Add_Monster("monster_infantry", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 1056, 136, -123);
		VectorSet(spawn_angles, 0, 270, 0);
		Add_Monster("monster_infantry", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -680, 856, 74);
		VectorSet(spawn_angles, 0, 220, 0);
		Add_Monster("monster_berserk", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 419, -410, -28);
		VectorSet(spawn_angles, 0, 270, 0);
		Add_Monster("monster_berserk", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -1524, 1296, 14);
		VectorSet(spawn_angles, 0, 45, 0);
		Add_Monster("monster_medic", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 482, 990, -200);
		VectorSet(spawn_angles, 0, 270, 0);
		Add_Monster("monster_parasite", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 905, 603, -278);
		VectorSet(spawn_angles, 0, 0, 0);
		Add_Monster("monster_flipper", NULL, start, spawn_angles, EVENTFLAG_SECRET, "base1$secret", 768, 1);

		VectorSet(start, 954, 513, -258);
		VectorSet(spawn_angles, 0, 0, 0);
		Add_Monster("monster_flipper", NULL, start, spawn_angles, EVENTFLAG_SECRET, "base1$secret", 768, 1);
	}

	// ===================================================================================================================
	// BASE2
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "base2") == 0) {

		VectorSet(start, 101, -865, -159);
		VectorSet(spawn_angles, 0, 45, 0);
		Add_Monster("monster_infantry", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 450, 122, 66);
		VectorSet(spawn_angles, 0, 90, 0);
		Add_Monster("monster_gunner", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 8, 81, 246);
		VectorSet(spawn_angles, 0, 90, 0);
		Add_Monster("monster_gunner", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -131, -394, 17);
		VectorSet(spawn_angles, 0, 180, 0);
		Add_Monster("monster_brain", "item_adrenaline", start, spawn_angles, 0, NULL, 0, 0);

		// Rroff not monster spawning but...
		if (g_hordemode->value)
		{
			ent = G_Spawn();
			ent->classname = "target_laser";
			ent->dmg = 9999;
			ent->spawnflags |= 67;
			ent->targetname = "lockdown$base2";
			VectorSet(ent->s.origin, 217.0, 1080, 24);
			VectorSet(ent->s.angles, 0.0, 180, 0.0);

			ED_CallSpawn(ent);

			ent = G_Spawn();
			ent->classname = "target_laser";
			ent->dmg = 9999;
			ent->spawnflags |= 67;
			ent->targetname = "lockdown$base2";
			VectorSet(ent->s.origin, 217.0, 1080, 56);
			VectorSet(ent->s.angles, 0.0, 180, 0.0);

			ED_CallSpawn(ent);

			ent = G_Spawn();
			ent->classname = "target_laser";
			ent->dmg = 9999;
			ent->spawnflags |= 67;
			ent->targetname = "lockdown$base2";
			VectorSet(ent->s.origin, 217.0, 1080, 88);
			VectorSet(ent->s.angles, 0.0, 180, 0.0);

			ED_CallSpawn(ent);

			//VectorSet(start, 474, 1791, -192);
			VectorSet(start, 484, 1890, -128);
			VectorSet(spawn_angles, 0, 180, 0);
			cpoint_spawn(start, spawn_angles, "lockdown$base2", LK_EASY);
		}
	}

	// ===================================================================================================================
	// BASE3
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "base3") == 0) {
		VectorSet(start, 85, 270, -374);
		VectorSet(spawn_angles, 0, 0, 0);
		Add_Monster("monster_floater", NULL, start, spawn_angles, EVENTFLAG_PICKUP_POWER, "base3$quad", 0, 1);
	}

	// ===================================================================================================================
	// TRAIN
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "train") == 0) {

		VectorSet(start, -1520, 1091, 144);
		VectorSet(spawn_angles, 0, 270, 0);
		Add_Monster("monster_gunner", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -1347, 135, 205);
		VectorSet(spawn_angles, 0, 270, 0);
		Add_Monster("monster_gunner", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -1000, -890, - 25);
		VectorSet(spawn_angles, 0, 90, 0);
		Add_Monster("monster_berserk", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -1833, 753, 238);
		VectorSet(spawn_angles, 0, 0, 0);
		Add_Monster("monster_infantry", "key_tag", start, spawn_angles, 0, NULL, 0, 0);
	}

	// ===================================================================================================================
	// BUNK1
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "bunk1") == 0) {

		VectorSet(start, -985, -59, 250);
		VectorSet(spawn_angles, 0, 0, 0);
		Add_Monster("monster_gunner", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 155, 792, 106);
		VectorSet(spawn_angles, 0, 237, 0);
		Add_Monster("monster_parasite", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -1506, 460, 158);
		VectorSet(spawn_angles, 0, 45, 0);
		Add_Monster("monster_infantry", "ammo_bullets", start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -1882, 1660, 24);
		VectorSet(spawn_angles, 0, 270, 0);
		Add_Monster("monster_soldier_ss", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -2387, 2332, 112);
		VectorSet(spawn_angles, 0, 270, 0);
		Add_Monster("monster_berserk", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -1728, 2302, 103);
		VectorSet(spawn_angles, 0, 180, 0);
		Add_Monster("monster_gunner", "ammo_grenades", start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 147, 477, 318);
		VectorSet(spawn_angles, 0, 129, 0);
		Add_Monster("monster_gunner", "ammo_grenades", start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -693, 1915, 120);
		VectorSet(spawn_angles, 0, 244, 0);
		Add_Monster("monster_chick", "key_tag", start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -599, 219, 530);
		VectorSet(spawn_angles, 0, 0, 0);
		Add_Monster("monster_floater", NULL, start, spawn_angles, EVENTFLAG_PLAT, NULL, 1024, 1);
	}

	// ===================================================================================================================
	// FACT1
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "fact1") == 0) {
		VectorSet(start, 582, 960, 48);
		VectorSet(spawn_angles, 0, 0, 0);
		Add_Monster("monster_tank", "", start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 1215, 503, -498);
		VectorSet(spawn_angles, 0, 90, 0);
		//Add_Monster("monster_gunner", NULL, start, spawn_angles, 0, NULL, 0, 0);
		Add_Monster("monster_chick", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 1424, 413, 73);
		VectorSet(spawn_angles, 0, 180, 0);
		Add_Monster("monster_infantry", "ammo_bullets", start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 456, 758, 300);
		VectorSet(spawn_angles, 0, 0, 0);
		Add_Monster("monster_flyer", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 462, 1127, 300);
		VectorSet(spawn_angles, 0, 0, 0);
		Add_Monster("monster_flyer", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 486, -881, 261);
		VectorSet(spawn_angles, 0, 0, 0);
		Add_Monster("monster_flyer", NULL, start, spawn_angles, 0, NULL, 0, 1);

		VectorSet(start, 1294, 668, 367);
		VectorSet(spawn_angles, 0, 0, 0);
		Add_Monster("monster_flyer", NULL, start, spawn_angles, EVENTFLAG_BUTTON, "fact1$gbutton", 0, 1);

		VectorSet(start, 1299, 220, 238);
		VectorSet(spawn_angles, 0, 0, 0);
		Add_Monster("monster_flyer", NULL, start, spawn_angles, EVENTFLAG_BUTTON, "fact1$gbutton", 0, 1);

		VectorSet(start, 1594, 718, -44);
		VectorSet(spawn_angles, 0, 270, 0);
		Add_Monster("monster_gladiator", "item_armor_combat", start, spawn_angles, 0, NULL, 0, 1);

		VectorSet(start, 822, 411, -503);
		VectorSet(spawn_angles, 0, 0, 0);
		Add_Monster("monster_flyer", NULL, start, spawn_angles, EVENTFLAG_BUTTON, NULL, 512, 1);

		VectorSet(start, 1477, 733, -614);
		VectorSet(spawn_angles, 0, 180, 0);
		Add_Monster("monster_infantry", "ammo_bullets", start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 1611, 6, -55);
		VectorSet(spawn_angles, 0, 90, 0);
		Add_Monster("monster_berserk", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 1599, -385, -209);
		VectorSet(spawn_angles, 0, 270, 0);
		Add_Monster("monster_brain", "item_sabotu", start, spawn_angles, 0, NULL, 0, 0);

		if (g_hordemode->value)
		{
			//VectorSet(start, 474, 1791, -192);
			VectorSet(start, -927.1, 512.7, 0.0);
			VectorSet(spawn_angles, 0.0, 0.0, 0.0);
			cpoint_spawn(start, spawn_angles, "lockdown$fact1", LK_FACT1);

			VectorSet(start, -593, 765, 93);
			VectorSet(spawn_angles, 0, 0, 0);
			Add_MonsterTele("monster_supertank", "key_tag", start, spawn_angles, EVENTFLAG_NONE, "fact1$lockdown", 0, 0);
		}
		else
		{
			VectorSet(start, -593, 765, 93);
			VectorSet(spawn_angles, 0, 0, 0);
			Add_Monster("monster_supertank", "key_tag", start, spawn_angles, EVENTFLAG_BUTTON, "fact1$ebutton", 0, 0);

			VectorSet(start, -692, 997, 206);
			VectorSet(spawn_angles, 0, 0, 0);
			Add_Monster("monster_flyer", "", start, spawn_angles, EVENTFLAG_BUTTON, "fact1$ebutton", 0, 1);
		}

	}

	// ===================================================================================================================
	// FACT2
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "fact2") == 0) {
		VectorSet(start, 1559, -862, -51);
		VectorSet(spawn_angles, 0, 180, 0);
		Add_Monster("monster_parasite", "item_scavu", start, spawn_angles, 0, NULL, 0, 0);
	}

	// ===================================================================================================================
	// WARE1
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "ware1") == 0) {
		VectorSet(start, -2298, -49, -190);
		VectorSet(spawn_angles, 0, 0, 0);
		Add_Monster("monster_berserk", NULL, start, spawn_angles, EVENTFLAG_DOOR, NULL, 350, 1);

		VectorSet(start, -2116, -156, 280);
		VectorSet(spawn_angles, 0, 270, 0);
		Add_Monster("monster_soldier", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -969, -672, 80);
		VectorSet(spawn_angles, 0, 223, 0);
		Add_Monster("monster_infantry", "ammo_bullets", start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -1221, -1719, 130);
		VectorSet(spawn_angles, 0, 180, 0);
		Add_Monster("monster_berserk", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -1008, -1332, -167);
		VectorSet(spawn_angles, 0, 272, 0);
		Add_Monster("monster_berserk", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -1105, -1287, -143);
		VectorSet(spawn_angles, 0, 270, 0);
		Add_Monster("monster_tank", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -617, -2043, -137);
		VectorSet(spawn_angles, 0, 90, 0);
		Add_Monster("monster_gunner", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 412, -1112, 152);
		VectorSet(spawn_angles, 0, 219, 0);
		Add_Monster("monster_chick", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 666, -1593, 165);
		VectorSet(spawn_angles, 0, 169, 0);
		Add_Monster("monster_chick", NULL, start, spawn_angles, 0, NULL, 0, 0);
	}

	// ===================================================================================================================
	// WARE2
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "ware2") == 0) {
		VectorSet(start, 124.9, -424.8, 275.9);
		VectorSet(spawn_angles, 0.0, 270, 0.0);
		Add_Monster("monster_gladiator", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -221, 939, -99);
		VectorSet(spawn_angles, 0, 270, 0);
		Add_Monster("monster_infantry", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -129, 244, -94);
		VectorSet(spawn_angles, 0, 90, 0);
		Add_Monster("monster_berserk", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 1890, -532, 298);
		VectorSet(spawn_angles, 0, 180, 0);
		Add_Monster("monster_floater", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 1545, -132, -201);
		VectorSet(spawn_angles, 0, 280, 0);
		Add_Monster("monster_berserk", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 2105.4, 618.0, -202.4);
		VectorSet(spawn_angles, 0.0, 271.9, 0.0);
		Add_Monster("monster_mutant", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 2015.5, 888.0, -193.8);
		VectorSet(spawn_angles, 0.0, 270, 0.0);
		Add_Monster("monster_medic", NULL, start, spawn_angles, 0, NULL, 0, 0);
	}

	// ===================================================================================================================
	// JAIL3
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "jail3") == 0) {

		//VectorSet(start, -1860, 251, 860);
		//VectorSet(spawn_angles, 0.0, 300, 0.0);

		VectorSet(start, -1833, 201, 860);
		VectorSet(spawn_angles, 0.0, 300, 0.0);

		ent = Add_Grunt("misc_grunt", "weapon_chaingun", start, spawn_angles, EVENTFLAG_TRIGGER, "willits", 0, "Willits", WEAP_CHAINGUN);
		ent->monsterinfo.aiflags |= AI_STAND_GROUND;
		ent->s.skinnum = 13;

		// Rroff bit of a strange place to add this but it works

		VectorSet(start, -861, -36, 992);
		VectorSet(mins, -64, -64, 0);
		VectorSet(maxs, 64, 64, 128);
		event_trigger(start, mins, maxs, "willits");
	}

	// ===================================================================================================================
	// JAIL1
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "jail1") == 0) {
		VectorSet(start, -2136.8, 270.4, 103.8);
		VectorSet(spawn_angles, 0.0, 270.8, 0.0);
		Add_Monster("monster_tank_commander", "key_tag", start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -2554.6, -692.3, 476.5);
		VectorSet(spawn_angles, 0.0, 36.4, 0.0);
		Add_Monster("monster_gladiator", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -1546.9, 6.0, 434.9);
		VectorSet(spawn_angles, 0.0, 179.3, 0.0);
		Add_Monster("monster_floater", NULL, start, spawn_angles, EVENTFLAG_TANK_25, NULL, 1024, 0);

		VectorSet(start, -1543.5, 131.9, 434.9);
		VectorSet(spawn_angles, 0.0, 179.3, 0.0);
		Add_Monster("monster_floater", NULL, start, spawn_angles, EVENTFLAG_TANK_25, NULL, 1024, 0);

		VectorSet(start, -2139.1, 350.4, 170.1);
		VectorSet(spawn_angles, 0.0, 270.9, 0.0);
		Add_MonsterTele("monster_medic", NULL, start, spawn_angles, EVENTFLAG_TANK_25, NULL, 1024, 0);

		//VectorSet(start, -2554.6, -692.3, 476.5);
		//VectorSet(spawn_angles, 0.0, 36.4, 0.0);
		//Add_Monster("monster_gladiator", NULL, start, spawn_angles, EVENTFLAG_TANK_25, NULL, 2048, 0);
	}

	// ===================================================================================================================
	// JAIL5
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "jail5") == 0) {

		VectorSet(start, 981.9, 2233.1, 87.9);
		VectorSet(spawn_angles, 0.0, 91.8, 0.0);
		Add_Monster("monster_mutant", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 1209.8, 2225.9, 76.0);
		VectorSet(spawn_angles, 0.0, 88.8, 0.0);
		Add_Monster("monster_mutant", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 1042.3, 2941.8, -172.4);
		VectorSet(spawn_angles, 0.0, 274.1, 0.0);
		Add_Monster("monster_berserk", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 583.9, 2782.1, -166.5);
		VectorSet(spawn_angles, 0.0, 315.7, 0.0);
		Add_Monster("monster_infantry", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 1394.0, 2329.3, -172.6);
		VectorSet(spawn_angles, 0.0, 174.8, 0.0);
		Add_Monster("monster_infantry", NULL, start, spawn_angles, 0, NULL, 0, 0);
	}


	// ===================================================================================================================
	// SECURITY
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "security") == 0) {
		//VectorSet(start, -196, 1905, 270);
		//VectorSet(spawn_angles, 0, 0, 0);
		//Add_Monster("monster_tank", NULL, start, spawn_angles, EVENTFLAG_BOSS_SIGHT, 1024, 0);
	}

	// ===================================================================================================================
	// POWER1
	// ===================================================================================================================

	// this monster item should be the blaster upgrade chip
	if (Q_stricmp(level.mapname, "power1") == 0) {
		VectorSet(start, -246, 1418, 220);
		VectorSet(spawn_angles, 0, 0, 0);
		Add_Monster("monster_brain", "item_blasteru", start, spawn_angles, 0, NULL, 0, 0);
	}

	// ===================================================================================================================
	// POWER2
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "power2") == 0) {
		VectorSet(start, 808, 1024, 42);
		VectorSet(spawn_angles, 0, 0, 0);
		Add_Monster("monster_brain", "item_mgu", start, spawn_angles, 0, NULL, 0, 0);
	}

	// ===================================================================================================================
	// MINE1
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "mine1") == 0) {
		VectorSet(start, -376, -1955, 990);
		VectorSet(spawn_angles, 0.0, 180.6, 0.0);
		Add_Monster("monster_brain", "item_grenadeu", start, spawn_angles, 0, NULL, 0, 0);
	}

	// ===================================================================================================================
	// MINE2
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "mine2") == 0) {

		if (g_hordemode->value)
		{
			ent = G_Spawn();
			ent->classname = "target_laser";
			ent->dmg = 9999;
			ent->spawnflags |= 195;
			ent->targetname = "lockdown$mine2";
			VectorSet(ent->s.origin, 1424.0, 239.4, 115);
			VectorSet(ent->s.angles, 0.0, 180, 0.0);

			ED_CallSpawn(ent);

			ent = G_Spawn();
			ent->classname = "target_laser";
			ent->dmg = 9999;
			ent->spawnflags |= 195;
			ent->targetname = "lockdown$mine2";
			VectorSet(ent->s.origin, 1424.0, 239.4, 147);
			VectorSet(ent->s.angles, 0.0, 180, 0.0);

			ED_CallSpawn(ent);

			ent = G_Spawn();
			ent->classname = "target_laser";
			ent->dmg = 9999;
			ent->spawnflags |= 195;
			ent->targetname = "lockdown$mine2";
			VectorSet(ent->s.origin, 1424.0, 239.4, 179);
			VectorSet(ent->s.angles, 0.0, 180, 0.0);

			ED_CallSpawn(ent);

			ent = G_Spawn();
			ent->classname = "target_laser";
			ent->dmg = 9999;
			ent->spawnflags |= 195;
			ent->targetname = "lockdown$mine2";
			VectorSet(ent->s.origin, 1424.0, 239.4, 211);
			VectorSet(ent->s.angles, 0.0, 180, 0.0);

			ED_CallSpawn(ent);

			ent = G_Spawn();
			ent->classname = "target_laser";
			ent->dmg = 9999;
			ent->spawnflags |= 195;
			ent->targetname = "lockdown$mine2";
			VectorSet(ent->s.origin, 1800.0, 588.2, -156);
			VectorSet(ent->s.angles, 0.0, 0.0, 0.0);

			ED_CallSpawn(ent);

			ent = G_Spawn();
			ent->classname = "target_laser";
			ent->dmg = 9999;
			ent->spawnflags |= 195;
			ent->targetname = "lockdown$mine2";
			VectorSet(ent->s.origin, 1800.0, 588.2, -124);
			VectorSet(ent->s.angles, 0.0, 0.0, 0.0);

			ED_CallSpawn(ent);

			ent = G_Spawn();
			ent->classname = "target_laser";
			ent->dmg = 9999;
			ent->spawnflags |= 195;
			ent->targetname = "lockdown$mine2";
			VectorSet(ent->s.origin, 1800.0, 588.2, -92);
			VectorSet(ent->s.angles, 0.0, 0.0, 0.0);

			ED_CallSpawn(ent);

			ent = G_Spawn();
			ent->classname = "target_laser";
			ent->dmg = 9999;
			ent->spawnflags |= 195;
			ent->targetname = "lockdown$mine2";
			VectorSet(ent->s.origin, 1800.0, 588.2, -60);
			VectorSet(ent->s.angles, 0.0, 0.0, 0.0);

			ED_CallSpawn(ent);

			VectorSet(start, 1630.8, 411.8, -184.0);
			VectorSet(spawn_angles, 0, 270, 0);
			cpoint_spawn(start, spawn_angles, "lockdown$mine2", LK_EASY);
		}
	}

	// ===================================================================================================================
	// MINE3
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "mine3") == 0) {
		VectorSet(start, -1193, 1546, -490);
		VectorSet(spawn_angles, 0, 270, 0);
		Add_Monster("monster_medic", "key_tag", start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -1183.1, 1437.6, -324.1);
		VectorSet(spawn_angles, 0.0, 266.1, 0.0);
		Add_Monster("monster_hover", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -1498.0, 1197.1, -282.5);
		VectorSet(spawn_angles, 0.0, 37.5, 0.0);
		Add_Monster("monster_hover", NULL, start, spawn_angles, 0, NULL, 0, 0);
	}

	// ===================================================================================================================
	// MINE4
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "mine4") == 0) {
		VectorSet(start, 2905.9, 646.4, 151.4);
		VectorSet(spawn_angles, 0.0, 145.3, 0.0);
		Add_Monster("monster_floater", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 2796.9, 1027.0, 144.8);
		VectorSet(spawn_angles, 0.0, 210.0, 0.0);
		Add_Monster("monster_floater", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 2613.6, 878.9, 84.9);
		VectorSet(spawn_angles, 0.0, 180.0, 0.0);
		Add_Monster("monster_tank", NULL, start, spawn_angles, 0, NULL, 0, 0);
	}

	// ===================================================================================================================
	// STRIKE
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "strike") == 0)
	{
		//VectorSet(start, -486, 532, 232);
		//VectorSet(spawn_angles, 0, 67, 0);
		//Add_Monster("monster_gladiator", "item_armor_combat", start, spawn_angles, 0, NULL, 0, 1);

		VectorSet(start, -417.6, 1122.9, -457.6);
		VectorSet(spawn_angles, 0.0, 43.6, 0.0);
		Add_Monster("monster_flipper", "item_scavu", start, spawn_angles, 0, NULL, 0, 1);

		VectorSet(start, -2196.1, -330.0, -324.9);
		VectorSet(spawn_angles, 0.0, 135.7, 0.0);
		Add_Monster("monster_flipper", "item_mgu", start, spawn_angles, 0, NULL, 0, 1);
	}

	// ===================================================================================================================
	// COOL1
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "cool1") == 0) {
		VectorSet(start, -989.8, -1254.9, -672.6);
		VectorSet(spawn_angles, 0.0, 170.5, 0.0);
		Add_Monster("monster_tank", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -1738.3, -680.3, -681.0);
		VectorSet(spawn_angles, 0.0, 294.9, 0.0);
		Add_Monster("monster_gladiator", "key_head", start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -1718.5, -1016.1, -675.1);
		VectorSet(spawn_angles, 0.0, 329.2, 0.0);
		Add_Monster("monster_tank", NULL, start, spawn_angles, 0, NULL, 0, 0);
	}

	// ===================================================================================================================
	// WASTE1
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "waste1") == 0)
	{

		if (g_hordemode->value)
		{
			/*ent = G_Spawn();
			ent->classname = "target_laser";
			ent->dmg = 9999;
			ent->spawnflags |= 195;
			ent->targetname = "lockdown$waste1";
			VectorSet(ent->s.origin, -2324, -535, -183.4);
			VectorSet(ent->s.angles, 0.0, 180, 0.0);

			ED_CallSpawn(ent);

			ent = G_Spawn();
			ent->classname = "target_laser";
			ent->dmg = 9999;
			ent->spawnflags |= 195;
			ent->targetname = "lockdown$waste1";
			VectorSet(ent->s.origin, -2337, -535, -135);
			VectorSet(ent->s.angles, 0.0, 180, 0.0);

			ED_CallSpawn(ent);

			ent = G_Spawn();
			ent->classname = "target_laser";
			ent->dmg = 9999;
			ent->spawnflags |= 195;
			ent->targetname = "lockdown$waste1";
			VectorSet(ent->s.origin, -2349.0, -535, -87);
			VectorSet(ent->s.angles, 0.0, 180, 0.0);

			ED_CallSpawn(ent);*/

			ent = G_Spawn();
			ent->classname = "target_laser";
			ent->dmg = 9999;
			ent->spawnflags |= 195;
			ent->targetname = "lockdown$waste1";
			VectorSet(ent->s.origin, -1690, 354, -180);
			VectorSet(ent->s.angles, 0.0, 0.0, 0.0);

			ED_CallSpawn(ent);

			ent = G_Spawn();
			ent->classname = "target_laser";
			ent->dmg = 9999;
			ent->spawnflags |= 195;
			ent->targetname = "lockdown$waste1";
			VectorSet(ent->s.origin, -1690, 354, -150);
			VectorSet(ent->s.angles, 0.0, 0.0, 0.0);

			ED_CallSpawn(ent);

			ent = G_Spawn();
			ent->classname = "target_laser";
			ent->dmg = 9999;
			ent->spawnflags |= 195;
			ent->targetname = "lockdown$waste1";
			VectorSet(ent->s.origin, -64, 1253, -180);
			VectorSet(ent->s.angles, 0.0, 180, 0.0);

			ED_CallSpawn(ent);

			ent = G_Spawn();
			ent->classname = "target_laser";
			ent->dmg = 9999;
			ent->spawnflags |= 195;
			ent->targetname = "lockdown$waste1";
			VectorSet(ent->s.origin, -64, 1253, -140);
			VectorSet(ent->s.angles, 0.0, 180, 0.0);

			ED_CallSpawn(ent);

			ent = G_Spawn();
			ent->classname = "target_laser";
			ent->dmg = 9999;
			ent->spawnflags |= 195;
			ent->targetname = "lockdown$waste1";
			VectorSet(ent->s.origin, -64, 1253, -100);
			VectorSet(ent->s.angles, 0.0, 180, 0.0);

			ED_CallSpawn(ent);

			//VectorSet(start, -2587, -637, -200);
			//VectorSet(spawn_angles, 0, 320, 0);

			VectorSet(start, -1564, 407, -200);
			VectorSet(spawn_angles, 0, 90, 0);
			cpoint_spawn(start, spawn_angles, "lockdown$waste1", LK_WASTE1);
		}
	}

	// ===================================================================================================================
	// WASTE3
	// ===================================================================================================================


	if (Q_stricmp(level.mapname, "waste3") == 0) {
		VectorSet(start, -1566.3, -1522.5, -74.9);
		VectorSet(spawn_angles, 0.0, 179.4, 0.0);
		Add_Monster("monster_brain", "item_glu", start, spawn_angles, 0, NULL, 0, 0);
	}

	// ===================================================================================================================
	// HANGAR1
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "hangar1") == 0) {
		VectorSet(start, 884.9, -944.8, 1009.5);
		VectorSet(spawn_angles, 0.0, 196.2, 0.0);
		Add_Monster("monster_flipper", "item_tripu", start, spawn_angles, 0, NULL, 0, 1);
	}

	// ===================================================================================================================
	// HANGAR2
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "hangar2") == 0) {

		VectorSet(start, 1520.3, -2007.9, -36.3);
		VectorSet(spawn_angles, 0.0, 189.7, 0.0);
		Add_MonsterTele("monster_hover", NULL, start, spawn_angles, EVENTFLAG_BOSS_SIGHT, NULL, 2048, 0);

		VectorSet(start, 1406.0, -2408.0, 63.6);
		VectorSet(spawn_angles, 0.0, 150.8, 0.0);
		Add_MonsterTele("monster_hover", NULL, start, spawn_angles, EVENTFLAG_BOSS_SIGHT, NULL, 2048, 0);

		VectorSet(start, 1263.9, -1666.1, -50.4);
		VectorSet(spawn_angles, 0.0, 205.3, 0.0);
		Add_MonsterTele("monster_gladiator", "ammo_slugs", start, spawn_angles, EVENTFLAG_BOSS_SIGHT, NULL, 2048, 0);

		VectorSet(start, 984.8, -1691.0, -33.9);
		VectorSet(spawn_angles, 0.0, 272.1, 0.0);
		Add_MonsterTele("monster_tank", "item_pack", start, spawn_angles, EVENTFLAG_BOSS_50, NULL, 2048, 0);

		VectorSet(start, 804.4, -2325.0, -42.9);
		VectorSet(spawn_angles, 0.0, 50.2, 0.0);
		Add_MonsterTele("monster_tank", "item_adrenaline", start, spawn_angles, EVENTFLAG_BOSS_50, NULL, 2048, 0);

		VectorSet(start, 1341.9, -2028.3, 20.8);
		VectorSet(spawn_angles, 0.0, 198.1, 0.0);
		Add_MonsterTele("monster_boss2", NULL, start, spawn_angles, EVENTFLAG_BOSS_DIE, NULL, 2048, 0);

		VectorSet(start, 1384.9, -2278.8, 163.3);
		VectorSet(spawn_angles, 0.0, 168.0, 0.0);
		Add_MonsterTele("monster_boss2", "key_head", start, spawn_angles, EVENTFLAG_BOSS_DIE, NULL, 2048, 0);
	}

	// ===================================================================================================================
	// HANGAR2
	// ===================================================================================================================

	if (Q_stricmp(level.mapname, "boss2") == 0) {

		VectorSet(start, 40.9, -1187.4, 464.9);
		VectorSet(spawn_angles, 0.0, 47.0, 0.0);
		Add_Monster("monster_hover", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, 21.3, -654.3, 422.9);
		VectorSet(spawn_angles, 0.0, 184.7, 0.0);
		Add_Monster("monster_hover", NULL, start, spawn_angles, 0, NULL, 0, 0);

		VectorSet(start, -449.1, -330.8, 196.3);
		VectorSet(spawn_angles, 0.0, 267.6, 0.0);
		Add_MonsterTele("monster_tank_commander", "ammo_bullets", start, spawn_angles, EVENTFLAG_BOSS_50, NULL, 2048, 0);

		VectorSet(start, -450.1, -1596.1, 205.6);
		VectorSet(spawn_angles, 0.0, 90.6, 0.0);
		Add_MonsterTele("monster_tank_commander", "ammo_rockets", start, spawn_angles, EVENTFLAG_BOSS_50, NULL, 2048, 0);

		VectorSet(start, -125.0, -952.0, 420.8);
		VectorSet(spawn_angles, 0.0, 180.2, 0.0);
		Add_MonsterTele("monster_medic", "item_adrenaline", start, spawn_angles, EVENTFLAG_BOSS_50, NULL, 2048, 0);

		VectorSet(start, 306.9, -480.6, 242.0);
		VectorSet(spawn_angles, 0.0, 222.0, 0.0);
		Add_MonsterTele("monster_chick", "ammo_cells", start, spawn_angles, EVENTFLAG_BOSS_SIGHT, NULL, 2048, 0);

		VectorSet(start, 295.4, -1400.5, 204.8);
		VectorSet(spawn_angles, 0.0, 142.6, 0.0);
		Add_MonsterTele("monster_chick", "ammo_cells", start, spawn_angles, EVENTFLAG_BOSS_SIGHT, NULL, 2048, 0);

		VectorSet(start, -531.4, -1451.3, 389.3);
		VectorSet(spawn_angles, 0.0, 40.8, 0.0);
		Add_MonsterTele("monster_hover", "ammo_cells", start, spawn_angles, EVENTFLAG_BOSS_50, NULL, 2048, 0);

		VectorSet(start, -576.8, -448.0, 425.8);
		VectorSet(spawn_angles, 0.0, 313.7, 0.0);
		Add_MonsterTele("monster_hover", "ammo_cells", start, spawn_angles, EVENTFLAG_BOSS_50, NULL, 2048, 0);

		/*VectorSet(start, -95.6, -839.0, 174.3);
		VectorSet(spawn_angles, 0.0, 0.2, 0.0);
		Add_MonsterTele("misc_insane", NULL, start, spawn_angles, EVENTFLAG_BOSS_DIE, NULL, 2048, 0);

		VectorSet(start, -91.8, -928.3, 174.3);
		VectorSet(spawn_angles, 0.0, 0.2, 0.0);
		Add_MonsterTele("misc_insane", NULL, start, spawn_angles, EVENTFLAG_BOSS_DIE, NULL, 2048, 0);

		VectorSet(start, -88.3, -1055.9, 174.3);
		VectorSet(spawn_angles, 0.0, 0.2, 0.0);
		Add_MonsterTele("misc_insane", NULL, start, spawn_angles, EVENTFLAG_BOSS_DIE, NULL, 2048, 0);

		VectorSet(start, -197.0, -1084.4, 174.1);
		VectorSet(spawn_angles, 0.0, 0.2, 0.0);
		Add_MonsterTele("misc_insane", NULL, start, spawn_angles, EVENTFLAG_BOSS_DIE, NULL, 2048, 0);

		VectorSet(start, -197.0, -957.9, 174.1);
		VectorSet(spawn_angles, 0.0, 0.2, 0.0);
		Add_MonsterTele("misc_insane", NULL, start, spawn_angles, EVENTFLAG_BOSS_DIE, NULL, 2048, 0);

		VectorSet(start, -197.0, -828.8, 174.1);
		VectorSet(spawn_angles, 0.0, 0.2, 0.0);
		Add_MonsterTele("misc_insane", NULL, start, spawn_angles, EVENTFLAG_BOSS_DIE, NULL, 2048, 0);*/
	}
}

// Rroff - additional horde functions here
// mostly related to spawning monsters

// style is used to define what the entity does
// entity is cleared after being used as a reference
// so shouldn't add towards the entity limit but will cause a lot of entities
// to be allocated

// style 1, 2, 9 define hornet boss 1 and 2 spawn points and 9 super tank boss
// 17 marks computer spawn, 0 is assumed to be a monster spawn point
// other values are reserved for possibly future use

/*QUAKED info_horde (1 .5 0) (-16 -16 -24) (16 16 32)
Horde mode positions
"Style": 1-17 are for specific functionality
1 and 2 mark possible Hornet boss spawn points these should be in the air
9 marks the Super Tank boss spawn point

17 marks the point for the computer to spawn

Other values are reserved for possible future use

No "style" or 0 the spot marks a possible monster spawn point
*/

void SP_info_horde(edict_t *self)
{
	spawn_point_t	*n;
	int				i;
	vec3_t			start, spawn_angles;

	if (!(g_hordecustom->value))
	{
		G_FreeEdict(self);
		return;
	}

	if (self->style == 17)
	{
		VectorCopy(self->s.origin, start);
		start[2] -= 24;
		VectorSet(spawn_angles, 0, self->s.angles[YAW], 0);
		cpoint_spawn(start, spawn_angles, "lockdown$custom", LK_CUSTOM);
		G_FreeEdict(self);
		return;
	}

	if (self->style > 0 && self->style < 17)
	{
		n = &custom_spawn_list[1];

		for (i = 1; i < 16; i++, n++)
		{
			if (!(n->child))
			{
				n->child = qtrue;
				n->mapname = level.mapname;
				VectorCopy(self->s.origin, n->origin);
				n->lastused = self->style;
				G_FreeEdict(self);
				return;
			}
		}

		gi.dprintf("Warning: Ran out of horde boss spawn points\n");

		G_FreeEdict(self);
		return;
	}

	n = &custom_spawn_list[17];

	for (i = 17; i < LK_MAX_CUSTOM; i++, n++)
	{
		if (!(n->child))
		{
			n->child = qtrue;
			n->mapname = level.mapname;
			n->angles[PITCH] = 0;
			VectorCopy(self->s.origin, n->origin);
			n->angles[YAW] = self->s.angles[YAW];
			n->angles[ROLL] = 0;
			n->lastused = 0;
			G_FreeEdict(self);
			return;
		}
	}

	gi.dprintf("Warning: Ran out of horde spawn points\n");
	G_FreeEdict(self);
}
