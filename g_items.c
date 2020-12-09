/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include "g_local.h"
#include "m_player.h"


qboolean    Pickup_Weapon(edict_t *ent, edict_t *other);
void        Use_Weapon(edict_t *ent, gitem_t *inv);
void        Drop_Weapon(edict_t *ent, gitem_t *inv);

void Weapon_Blaster(edict_t *ent);
void Weapon_Shotgun(edict_t *ent);
void Weapon_SuperShotgun(edict_t *ent);
void Weapon_Machinegun(edict_t *ent);
void Weapon_Chaingun(edict_t *ent);
void Weapon_HyperBlaster(edict_t *ent);
void Weapon_RocketLauncher(edict_t *ent);
void Weapon_Grenade(edict_t *ent);
void Weapon_GrenadeLauncher(edict_t *ent);
void Weapon_Railgun(edict_t *ent);
void Weapon_BFG(edict_t *ent);
void Weapon_FlareGun(edict_t *ent);

gitem_armor_t jacketarmor_info  = { 25,  50, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info  = { 50, 100, .60, .30, ARMOR_COMBAT};
gitem_armor_t bodyarmor_info    = {100, 200, .80, .60, ARMOR_BODY};

static int  jacket_armor_index;
static int  combat_armor_index;
static int  body_armor_index;
static int  power_screen_index;
static int  power_shield_index;

#define HEALTH_IGNORE_MAX   1
#define HEALTH_TIMED        2

void Use_Quad(edict_t *ent, gitem_t *item);
static int  quad_drop_timeout_hack;

void Pickup_doFX(edict_t *ent, gitem_t *it)
{
	if (it->pickup_sound)
	{
		ent->client->bonus_alpha = 0.25;

		ent->client->ps.stats[STAT_PICKUP_BACK] = gi.imageindex("tag3");
		ent->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(it->icon);
		ent->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS + ITEM_INDEX(it);
		ent->client->pickup_msg_time = level.time + 3.0;
		gi.sound(ent, CHAN_ITEM, gi.soundindex(it->pickup_sound), 1, ATTN_NORM, 0);
	}
}

//======================================================================

/*
===============
GetItemByIndex
===============
*/
gitem_t *GetItemByIndex(int index)
{
    if (index == 0 || index >= game.num_items)
        return NULL;

    return &itemlist[index];
}


/*
===============
FindItemByClassname

===============
*/
gitem_t *FindItemByClassname(char *classname)
{
    int     i;
    gitem_t *it;

    it = itemlist;
    for (i = 0 ; i < game.num_items ; i++, it++) {
        if (!it->classname)
            continue;
        if (!Q_stricmp(it->classname, classname))
            return it;
    }

    return NULL;
}

/*
===============
FindItem

===============
*/
gitem_t *FindItem(char *pickup_name)
{
    int     i;
    gitem_t *it;

    it = itemlist;
    for (i = 0 ; i < game.num_items ; i++, it++) {
        if (!it->pickup_name)
            continue;
        if (!Q_stricmp(it->pickup_name, pickup_name))
            return it;
    }

    return NULL;
}

//======================================================================

void DoRespawn(edict_t *ent)
{
    if (ent->team) {
        edict_t *master;
        int count;
        int choice;

        master = ent->teammaster;

        for (count = 0, ent = master; ent; ent = ent->chain, count++)
            ;

        choice = rand() % count;

        for (count = 0, ent = master; count < choice; ent = ent->chain, count++)
            ;
    }

    ent->svflags &= ~SVF_NOCLIENT;
    ent->solid = SOLID_TRIGGER;
    gi.linkentity(ent);

    // send an effect
    ent->s.event = EV_ITEM_RESPAWN;
}

void LockdownRespawn(edict_t *ent)
{
	if (level.lockdown == 2)
	{
		if (level.lockdown_ent && ((level.lockdown_ent->moded == LK_FOREVER) || (level.lockdown_ent->moded == LK_CUSTOM)))
		{
			if ((ent->item->flags & IT_POWERUP))
				ent->nextthink = level.time + 180;
			else
				ent->nextthink = level.time + 30;
		}
		else
		{
			ent->nextthink = level.time + 300;
		}

		ent->think = DoRespawn;
	}
	else
	{
		ent->nextthink = level.time + FRAMETIME;
	}
}

void SetRespawn(edict_t *ent, float delay)
{
    ent->flags |= FL_RESPAWN;
    ent->svflags |= SVF_NOCLIENT;
    ent->solid = SOLID_NOT;

	if (level.lockdown == 2)
	{
		if (level.lockdown_ent && ((level.lockdown_ent->moded == LK_FOREVER) || level.lockdown_ent->moded == LK_CUSTOM))
		{
			if ((ent->item->flags & IT_POWERUP))
				delay = 180;
			else
				delay = 30;
		}
		else
		{
			delay = 300;
		}
	}

	if (level.lockdown == 1)
	{
		ent->nextthink = level.time + FRAMETIME;
		ent->think = LockdownRespawn;
	}
	else
	{
		ent->nextthink = level.time + delay;
		ent->think = DoRespawn;
	}
    gi.linkentity(ent);
}


//======================================================================

qboolean Pickup_Powerup(edict_t *ent, edict_t *other)
{
    int     quantity;

    quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];
    if ((skill->value == 1 && quantity >= 2) || (skill->value >= 2 && quantity >= 1))
        return qfalse;

    if ((coop->value) && (ent->item->flags & IT_STAY_COOP) && (quantity > 0))
        return qfalse;

	if (ent->item->flags & IT_MOD)
	{
		// add the MU_ type to each item defintion tag field
		// check player doesn't have either this item already or the MU_
		// for other->client->pers.mods

		//if (coop->value)
		//{
			if (quantity > 0)
				return qfalse;
			if (ent->item->tag && other->client->pers.mods & ent->item->tag)
				return qfalse;
		//}

		// Rroff ugly way to keep these over coop respawns
		other->client->resp.coop_respawn.inventory[ITEM_INDEX(ent->item)] = 1;
		other->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;
		gi.cprintf(other, PRINT_HIGH, "Activate %s from the inventory to use it\n", ent->item->pickup_name);

		if (coop->value)
		{
			Pickup_doFX(other, ent->item);
			return qfalse;
		}
	}
	else
	{
		other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value || level.lockdown == 1 || level.lockdown == 2))
		SetRespawn(ent, ent->item->quantity);

    if (deathmatch->value) {
        //if (!(ent->spawnflags & DROPPED_ITEM))
        if (((int)dmflags->value & DF_INSTANT_ITEMS) || ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM))) {
            if ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
                quad_drop_timeout_hack = (ent->nextthink - level.time) / FRAMETIME;
            ent->item->use(other, ent->item);
        }
    } else {
		// Rroff do event on powerup pick up
		doEvent(ent, EVENTFLAG_PICKUP_POWER);
	}


    return qtrue;
}

void Drop_General(edict_t *ent, gitem_t *item)
{
    Drop_Item(ent, item);
    ent->client->pers.inventory[ITEM_INDEX(item)]--;
    ValidateSelectedItem(ent);
}


//======================================================================

qboolean Pickup_Adrenaline(edict_t *ent, edict_t *other)
{
    if (!deathmatch->value)
        other->max_health += 1;

    if (other->health < other->max_health)
        other->health = other->max_health;

    if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
        SetRespawn(ent, ent->item->quantity);

    return qtrue;
}

qboolean Pickup_AncientHead(edict_t *ent, edict_t *other)
{
    other->max_health += 2;

    if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
        SetRespawn(ent, ent->item->quantity);

    return qtrue;
}

qboolean Pickup_Resupply(edict_t *ent, edict_t *other)
{
	gitem_t		*item;
	int			index;
	qboolean	choices = qfalse;

	item = FindItem("Machinegun");
	if (item) {
		index = ITEM_INDEX(item);
		if (other->client->pers.inventory[index] == 0)
		{
			if (random() <= 0.5)
			{
				other->client->pers.inventory[index] = 1;
				other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(item->icon);
				other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS + index;
				other->client->pickup_msg_time = level.time + 3.0;

				if (other->client->pers.weapon->weapmodel == WEAP_BLASTER)
				{
					other->client->newweapon = item;
					ChangeWeapon(other);
				}
			} else {
				choices = qtrue;
			}
		}
	}

	if (choices)
	{
		item = FindItem("Shotgun");
		if (item) {
			index = ITEM_INDEX(item);
			if (other->client->pers.inventory[index] == 0)
			{
				other->client->pers.inventory[index] = 1;
				other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(item->icon);
				other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS + index;
				other->client->pickup_msg_time = level.time + 3.0;

				if (other->client->pers.weapon->weapmodel == WEAP_BLASTER)
				{
					other->client->newweapon = item;
					ChangeWeapon(other);
				}
			}
		}
	}

	item = FindItem("Bullets");
	if (item) {
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += 75 + (rand() % 25);
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
			other->client->pers.inventory[index] = other->client->pers.max_bullets;
	}

	item = FindItem("Shells");
	if (item) {
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += 18 + (rand() % 4);
		if (other->client->pers.inventory[index] > other->client->pers.max_shells)
			other->client->pers.inventory[index] = other->client->pers.max_shells;
	}

	item = FindItem("Grenades");
	if (item) {
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += 4;
		if (other->client->pers.inventory[index] > other->client->pers.max_grenades)
			other->client->pers.inventory[index] = other->client->pers.max_grenades;
	}

	if (other->health < other->max_health)
	{
		other->health += 25;
		if (other->health > other->max_health)
			other->health = other->max_health;

		gi.sound(other, CHAN_ITEM, gi.soundindex("items/n_health.wav"), 1.0, ATTN_NORM, 0);
	}

	// Rroff kind of horrid way to do this
	if (ent->enemy)
	{
		ent->enemy->teamchain = NULL;
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && ((deathmatch->value) || (coop->value)))
		SetRespawn(ent, ent->item->quantity);

	return qtrue;
}

qboolean Pickup_Slingpack(edict_t *ent, edict_t *other)
{
	gitem_t		*item;
	int			index;
	qboolean	choices = qfalse;

	item = FindItem("Machinegun");
	if (item) {
		index = ITEM_INDEX(item);
		if (other->client->pers.inventory[index] == 0)
		{
			if (random() <= 0.5)
			{
				other->client->pers.inventory[index] = 1;
				other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(item->icon);
				other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS + index;
				other->client->pickup_msg_time = level.time + 3.0;

				if (other->client->pers.weapon->weapmodel == WEAP_BLASTER)
				{
					other->client->newweapon = item;
					ChangeWeapon(other);
				}
			}
			else {
				choices = qtrue;
			}
		}
	}

	if (choices)
	{
		item = FindItem("Shotgun");
		if (item) {
			index = ITEM_INDEX(item);
			if (other->client->pers.inventory[index] == 0)
			{
				other->client->pers.inventory[index] = 1;
				other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(item->icon);
				other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS + index;
				other->client->pickup_msg_time = level.time + 3.0;

				if (other->client->pers.weapon->weapmodel == WEAP_BLASTER)
				{
					other->client->newweapon = item;
					ChangeWeapon(other);
				}
			}
		}
	}

	item = FindItem("Bullets");
	if (item) {
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += 50 + (rand() % 10);
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
			other->client->pers.inventory[index] = other->client->pers.max_bullets;
	}

	item = FindItem("Shells");
	if (item) {
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += 11 + (rand() % 3);
		if (other->client->pers.inventory[index] > other->client->pers.max_shells)
			other->client->pers.inventory[index] = other->client->pers.max_shells;
	}

	item = FindItem("Grenades");
	if (item) {
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += 3;
		if (other->client->pers.inventory[index] > other->client->pers.max_grenades)
			other->client->pers.inventory[index] = other->client->pers.max_grenades;
	}

	// Rroff kind of horrid way to do this
	if (ent->enemy)
	{
		ent->enemy->teamchain = NULL;
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && ((deathmatch->value) || (coop->value)))
		SetRespawn(ent, ent->item->quantity);

	return qtrue;
}

qboolean Pickup_Bandolier(edict_t *ent, edict_t *other)
{
    gitem_t *item;
    int     index;

	if (!(other->client->pers.player_class & PC_LIMITED_AMMO))
	{
		if (other->client->pers.max_bullets < 250)
			other->client->pers.max_bullets = 250;
		if (other->client->pers.max_shells < 150)
			other->client->pers.max_shells = 150;
		if (other->client->pers.max_cells < 250)
			other->client->pers.max_cells = 250;
		if (other->client->pers.max_slugs < 75)
			other->client->pers.max_slugs = 75;
	}

    item = FindItem("Bullets");
    if (item) {
        index = ITEM_INDEX(item);
        other->client->pers.inventory[index] += item->quantity;
        if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
            other->client->pers.inventory[index] = other->client->pers.max_bullets;
    }

    item = FindItem("Shells");
    if (item) {
        index = ITEM_INDEX(item);
        other->client->pers.inventory[index] += item->quantity;
        if (other->client->pers.inventory[index] > other->client->pers.max_shells)
            other->client->pers.inventory[index] = other->client->pers.max_shells;
    }

    if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
        SetRespawn(ent, ent->item->quantity);

    return qtrue;
}

qboolean Pickup_Pack(edict_t *ent, edict_t *other)
{
    gitem_t		*item;
    int			index;
	qboolean	change = qfalse;

	if (!(other->client->pers.player_class & PC_LIMITED_AMMO))
	{
		if (other->client->pers.max_bullets < 300)
			other->client->pers.max_bullets = 300;
		if (other->client->pers.max_shells < 200)
			other->client->pers.max_shells = 200;
		if (other->client->pers.max_rockets < 100)
			other->client->pers.max_rockets = 100;
		if (other->client->pers.max_grenades < 100)
			other->client->pers.max_grenades = 100;
		if (other->client->pers.max_cells < 300)
			other->client->pers.max_cells = 300;
		if (other->client->pers.max_slugs < 100)
			other->client->pers.max_slugs = 100;
	}

	item = FindItem("Machinegun");
	if (item) {
		index = ITEM_INDEX(item);
		if (other->client->pers.inventory[index] == 0)
		{
			other->client->pers.inventory[index] = 1;
			other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(item->icon);
			other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS + index;
			other->client->pickup_msg_time = level.time + 3.0;

			if (other->client->pers.weapon->weapmodel == WEAP_BLASTER)
			{
				other->client->newweapon = item;
				change = qtrue;
			}
		}
	}

	item = FindItem("Shotgun");
	if (item) {
		index = ITEM_INDEX(item);
		if (other->client->pers.inventory[index] == 0)
		{
			other->client->pers.inventory[index] = 1;
			other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(item->icon);
			other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS + index;
			other->client->pickup_msg_time = level.time + 3.0;

			if (other->client->pers.weapon->weapmodel == WEAP_BLASTER)
			{
				other->client->newweapon = item;
				change = qtrue;
			}
		}
	}

	item = FindItem("Grenade Launcher");
	if (item) {
		index = ITEM_INDEX(item);
		if (other->client->pers.inventory[index] == 0)
		{
			other->client->pers.inventory[index] = 1;
			other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(item->icon);
			other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS + index;
			other->client->pickup_msg_time = level.time + 3.0;

			if (other->client->pers.weapon->weapmodel == WEAP_BLASTER)
			{
				other->client->newweapon = item;
				change = qtrue;
			}
		}
	}

	item = FindItem("Super Shotgun");
	if (item) {
		index = ITEM_INDEX(item);
		if (other->client->pers.inventory[index] == 0)
		{
			other->client->pers.inventory[index] = 1;
			other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(item->icon);
			other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS + index;
			other->client->pickup_msg_time = level.time + 3.0;

			if (other->client->pers.weapon->weapmodel == WEAP_BLASTER)
			{
				other->client->newweapon = item;
				change = qtrue;
			}
		}
	}

	if (change)
		ChangeWeapon(other);

    item = FindItem("Bullets");
    if (item) {
        index = ITEM_INDEX(item);
        other->client->pers.inventory[index] += item->quantity * 3;
        if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
            other->client->pers.inventory[index] = other->client->pers.max_bullets;
    }

    item = FindItem("Shells");
    if (item) {
        index = ITEM_INDEX(item);
        other->client->pers.inventory[index] += item->quantity * 3;
        if (other->client->pers.inventory[index] > other->client->pers.max_shells)
            other->client->pers.inventory[index] = other->client->pers.max_shells;
    }

    item = FindItem("Cells");
    if (item) {
        index = ITEM_INDEX(item);
        other->client->pers.inventory[index] += item->quantity * 3;
        if (other->client->pers.inventory[index] > other->client->pers.max_cells)
            other->client->pers.inventory[index] = other->client->pers.max_cells;
    }

    item = FindItem("Grenades");
    if (item) {
        index = ITEM_INDEX(item);
        other->client->pers.inventory[index] += item->quantity * 3;
        if (other->client->pers.inventory[index] > other->client->pers.max_grenades)
            other->client->pers.inventory[index] = other->client->pers.max_grenades;
    }

    item = FindItem("Rockets");
    if (item) {
        index = ITEM_INDEX(item);
        other->client->pers.inventory[index] += item->quantity * 3;
        if (other->client->pers.inventory[index] > other->client->pers.max_rockets)
            other->client->pers.inventory[index] = other->client->pers.max_rockets;
    }

    item = FindItem("Slugs");
    if (item) {
        index = ITEM_INDEX(item);
        other->client->pers.inventory[index] += item->quantity * 3;
        if (other->client->pers.inventory[index] > other->client->pers.max_slugs)
            other->client->pers.inventory[index] = other->client->pers.max_slugs;
    }

    if (!(ent->spawnflags & DROPPED_ITEM) && ((deathmatch->value) || (coop->value) || (level.lockdown == 1) || (level.lockdown == 2)))
        SetRespawn(ent, ent->item->quantity);

    return qtrue;
}

//======================================================================

void Use_Quad(edict_t *ent, gitem_t *item)
{
    int     timeout;

    ent->client->pers.inventory[ITEM_INDEX(item)]--;
    ValidateSelectedItem(ent);

    if (quad_drop_timeout_hack) {
        timeout = quad_drop_timeout_hack;
        quad_drop_timeout_hack = 0;
    } else {
		if (ent->client->pers.player_class & PC_QUAD_BONUS)
			timeout = 450;
		else
			timeout = 300;
    }

    if (ent->client->quad_framenum > level.framenum)
        ent->client->quad_framenum += timeout;
    else
        ent->client->quad_framenum = level.framenum + timeout;

    gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Breather(edict_t *ent, gitem_t *item)
{
    ent->client->pers.inventory[ITEM_INDEX(item)]--;
    ValidateSelectedItem(ent);

    if (ent->client->breather_framenum > level.framenum)
        ent->client->breather_framenum += 300;
    else
        ent->client->breather_framenum = level.framenum + 300;

//  gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Envirosuit(edict_t *ent, gitem_t *item)
{
    ent->client->pers.inventory[ITEM_INDEX(item)]--;
    ValidateSelectedItem(ent);

    if (ent->client->enviro_framenum > level.framenum)
        ent->client->enviro_framenum += 300;
    else
        ent->client->enviro_framenum = level.framenum + 300;

//  gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Invulnerability(edict_t *ent, gitem_t *item)
{
    ent->client->pers.inventory[ITEM_INDEX(item)]--;
    ValidateSelectedItem(ent);

    if (ent->client->invincible_framenum > level.framenum)
        ent->client->invincible_framenum += 300;
    else
        ent->client->invincible_framenum = level.framenum + 300;

    gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Silencer(edict_t *ent, gitem_t *item)
{
    ent->client->pers.inventory[ITEM_INDEX(item)]--;
    ValidateSelectedItem(ent);
    ent->client->silencer_shots += 30;

	gi.cprintf(ent, PRINT_HIGH,"While using silencer monsters will find it harder to spot you\n");

	// Rroff additionally make player with silencer harder for enemy to spot

//  gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

// Medic station - tries to drop where player is looking when used
// need to limit ability to drop it/probably just regen on player after X amount of time
// if they don't have it - track owner and remove if they drop a new one
// this might result in multiple dropped over level changes

void Support_Think(edict_t *self)
{
	edict_t		*cl_ent, *ent;
	vec3_t		v;
	float		dist;
	int			i;
	qboolean	heal = qfalse;


	if (level.time > self->timestamp + 180 || self->teammaster->client->support_time != self->timestamp)
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BFG_BIGEXPLOSION);
		gi.WritePosition(self->s.origin);
		gi.multicast(self->s.origin, MULTICAST_PVS);

		gi.sound(self, CHAN_VOICE, gi.soundindex("items/respawn1.wav"), 1, ATTN_NORM, 0);

		if (self->teamchain)
			G_FreeEdict(self->teamchain);

		G_FreeEdict(self->chain);
		G_FreeEdict(self);
		return;
	}

	//if (level.time > self->delay + 0.4)
	//if (level.time > self->delay + 2)
	if (level.time > self->delay + 2)
	{
		self->delay = level.time;

		for (i = 0; i < game.maxclients; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if (!cl_ent->inuse || game.clients[i].resp.spectator)
			{
				continue;
			}

			if (cl_ent->health <= 0)
				continue;

			VectorSubtract(cl_ent->s.origin, self->s.origin, v);
			dist = VectorLength(v);

			if (dist <= 256)
			{
				// flag the time the player was in range of the station
				cl_ent->client->support_last_time = level.time;

				if (cl_ent->health < cl_ent->max_health)
				{
					heal = qtrue;
					cl_ent->health += 12;
					//cl_ent->health += 10;
					if (cl_ent->health > cl_ent->max_health)
						cl_ent->health = cl_ent->max_health;
				}
			}
		}
	}

	if (level.time > self->wait + 30)
	{
		self->wait = level.time;
		if (!self->teamchain || !self->teamchain->inuse)
		{
			ent = Drop_Item(self, FindItemByClassname("item_slingpack"));
			self->teamchain = ent;
			ent->enemy = self;

			gi.sound(self, CHAN_VOICE, gi.soundindex("items/respawn1.wav"), 1.0, ATTN_NORM, 0);
		}
	}

	// won't fire very often but should give a bit of sound now and again
	if (heal && !(level.framenum & 7))
		gi.sound(self, CHAN_VOICE, gi.soundindex("items/protect3.wav"), 0.75, ATTN_NORM, 0);


	self->nextthink = level.time + FRAMETIME;
}

// Player should not be able to use medic station and explosive box or other support
// powers at the same time

void Use_Support(edict_t *ent, gitem_t *item)
{
	vec3_t		offset, forward, right, start, end;
	vec3_t		mins, maxs;
	trace_t		tr;
	edict_t		*sup, *tmp;

	edict_t		*fent = NULL;

	if (ent->client->weaponstate == WEAPON_FIRING)
		return;

	AngleVectors(ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 0, ent->viewheight);
	G_ProjectSource(ent->s.origin, offset, forward, right, start);
	VectorMA(start, 384, forward, end);

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

	if (tr.fraction == 1.0)
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
		return; // need to hit something
	}

	if (tr.plane.normal[2])
	{
		if (tr.plane.normal[2] < 0.7)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
			return;
		}
	}

	if (tr.ent->s.modelindex != 1)
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
		return;
	}

	if (tr.surface && tr.surface->flags & SURF_SKY)
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
		return;
	}

	VectorSet(mins, -16, -16, 0);
	VectorSet(maxs, 16, 16, 12);
	
	VectorCopy(tr.endpos, start);
	VectorCopy(tr.endpos, end);

	end[2] += 56;

	tr = gi.trace(start, mins, maxs, end, NULL, MASK_SHOT);

	if (!tr.allsolid)
	{
		VectorCopy(start, end);
		VectorCopy(tr.endpos, start);

		tr = gi.trace(start, mins, maxs, end, NULL, MASK_SHOT);

		if (tr.allsolid)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
			return;
		}

		if (tr.plane.normal[2])
		{
			if (tr.plane.normal[2] < 0.7)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
				return;
			}
		}

		if (tr.ent->s.modelindex != 1)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
			return;
		}

		// Don't allow overlapping medic stations
		// could be sped up by looping through the other players and tagging their stations to them
		while ((fent = findradiusall(fent, tr.endpos, 600)) != NULL)
		{
			if ((fent->flags & FL_MEDICSTATION) && (fent->teammaster != ent))
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
				gi.cprintf(ent, PRINT_HIGH, "Medic stations can't overlap\n");
				return;
			}
		}

		ent->client->support_time = level.time;

		// use cvar to disable sphere due to lack of RTX feature support ?

		tmp = G_Spawn();
		tmp->classname = "Healing Sphere";
		VectorCopy(tr.endpos, tmp->s.origin);
		tmp->s.modelindex = gi.modelindex("models/objects/support/sphere.md2");
		tmp->s.effects = EF_SPHERETRANS;
		//tmp->s.renderfx = RF_FULLBRIGHT | RF_TRANSLUCENT | RF_DEPTHHACK;
		//tmp->s.renderfx = RF_TRANSLUCENT | RF_DEPTHHACK | 0x80000000;
		tmp->s.renderfx = RF_DEPTHHACK | 0x80000000;

		gi.linkentity(tmp);

		sup = G_Spawn();
		VectorCopy(tr.endpos, sup->s.origin);
		sup->s.angles[YAW] = ent->s.angles[YAW];
		sup->classname = "Medic Station";
		sup->teammaster = ent;
		sup->chain = tmp;
		sup->timestamp = ent->client->support_time;
		sup->think = Support_Think;
		sup->nextthink = level.time + FRAMETIME;
		sup->flags |= FL_MEDICSTATION;

		sup->s.modelindex = gi.modelindex("models/objects/support/healing.md2");
		sup->s.renderfx = RF_GLOW;

		sup->delay = level.time;
		sup->wait = level.time;

		gi.linkentity(sup);

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_TELEPORT_EFFECT);
		gi.WritePosition(sup->s.origin);
		gi.multicast(sup->s.origin, MULTICAST_PVS);

		if (!(ent->client->anim_priority > ANIM_WAVE))
		{
			if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->client->anim_priority = ANIM_WAVE;
				ent->s.frame = FRAME_crattak1 - 1;
				ent->client->anim_end = FRAME_crattak3;
			}
			else {

				ent->client->anim_priority = ANIM_WAVE;
				ent->s.frame = FRAME_point01 - 1;
				ent->client->anim_end = FRAME_point06;
			}
		}

	}

	ValidateSelectedItem(ent);
}

void Use_Taunt(edict_t *ent, gitem_t *item)
{
	edict_t			*other = NULL;
	qboolean		gotone = qfalse;
	int				armour_index;

	if (ent->client->taunt_framenum > level.framenum)
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
		return;
	}

	if (!(ent->client->anim_priority > ANIM_WAVE) && !(ent->client->ps.pmove.pm_flags & PMF_DUCKED))
	{
		ent->client->anim_priority = ANIM_WAVE;

		ent->s.frame = FRAME_taunt01 - 1;
		ent->client->anim_end = FRAME_taunt17;
	}

	gi.sound(ent, CHAN_VOICE, gi.soundindex("*fall2.wav"), 1, ATTN_NORM, 0);
	PlayerNoise(ent, ent->s.origin, PNOISE_SELF);

	// make it so recently taunted monsters don't switch away for a few seconds?

	while ((other = findradius(other, ent->s.origin, 1024)) != NULL)
	{
		if (other->client)
			continue;
		if (other == ent)
			continue;
		if (!(other->svflags & SVF_MONSTER))
			continue;
		if (other->monsterinfo.aiflags & AI_GOOD_GUY)
			continue;
		if (other->health <= 0)
			continue;
		if (!gi.inPHS(ent->s.origin, other->s.origin))
			continue;

		other->enemy = ent;
		FoundTarget(other);
		gotone = qtrue;
	}

	if (gotone)
	{
		armour_index = ArmorIndex(ent);

		if (!armour_index)
			ent->client->pers.inventory[jacket_armor_index] = 100;
		else
			ent->client->pers.inventory[armour_index] += 100;

		ent->client->taunt_framenum = level.framenum + 300;
	}

	ValidateSelectedItem(ent);
}

// hookd incase we want to do different stuff later
void Explobox_delay(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + 2 * FRAMETIME;
	self->think = barrel_explode;
	self->activator = attacker;
}

void Explobox_Think(edict_t *self)
{
	if (level.time > self->timestamp + 180 || self->chain->client->support_time != self->timestamp)
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BFG_BIGEXPLOSION);
		gi.WritePosition(self->s.origin);
		gi.multicast(self->s.origin, MULTICAST_PVS);

		gi.sound(self, CHAN_VOICE, gi.soundindex("items/respawn1.wav"), 1, ATTN_NORM, 0);

		G_FreeEdict(self);
		return;
	}

	self->nextthink = level.time + FRAMETIME;
}

void Use_ExploBox(edict_t *ent, gitem_t *item)
{
	vec3_t		offset, forward, right, start, end;
	vec3_t		mins, maxs;
	trace_t		tr;
	edict_t		*sup;

	if (ent->client->weaponstate == WEAPON_FIRING)
		return;

	if (ent->client->support_time && level.time < ent->client->support_time + 15)
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
		return;
	}

	AngleVectors(ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 0, ent->viewheight);
	G_ProjectSource(ent->s.origin, offset, forward, right, start);
	VectorMA(start, 384, forward, end);

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

	if (tr.fraction == 1.0)
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
		return; // need to hit something
	}

	if (tr.plane.normal[2])
	{
		if (tr.plane.normal[2] < 0.7)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
			return;
		}
	}

	if (tr.ent->s.modelindex != 1)
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
		return;
	}

	if (tr.surface && tr.surface->flags & SURF_SKY)
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
		return;
	}

	VectorSet(mins, -16, -16, 0);
	VectorSet(maxs, 16, 16, 40);

	VectorCopy(tr.endpos, start);
	VectorCopy(tr.endpos, end);

	end[2] += 56;

	tr = gi.trace(start, mins, maxs, end, NULL, MASK_SHOT);

	if (!tr.allsolid)
	{
		VectorCopy(start, end);
		VectorCopy(tr.endpos, start);

		tr = gi.trace(start, mins, maxs, end, NULL, MASK_SHOT);

		if (tr.allsolid)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
			return;
		}

		if (tr.plane.normal[2])
		{
			if (tr.plane.normal[2] < 0.7)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
				return;
			}
		}

		if (tr.ent->s.modelindex != 1)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
			return;
		}

		ent->client->support_time = level.time;

		sup = G_Spawn();
		VectorCopy(tr.endpos, sup->s.origin);
		sup->s.angles[YAW] = ent->s.angles[YAW];
		sup->classname = "Explo Box";
		sup->timestamp = ent->client->support_time;
		sup->think = Explobox_Think;
		sup->nextthink = level.time + FRAMETIME;
		sup->chain = ent;
		// can't use ->owner for solid stuff as it doesn't block/touch owner

		sup->solid = SOLID_BBOX;
		sup->movetype = MOVETYPE_STEP;

		sup->model = "models/objects/barrels/tris.md2";
		sup->s.modelindex = gi.modelindex(sup->model);

		VectorSet(sup->mins, -16, -16, 0);
		VectorSet(sup->maxs, 16, 16, 40);

		sup->mass = 300;
		sup->health = 10;
		sup->dmg = 150;

		sup->die = Explobox_delay;
		sup->takedamage = DAMAGE_YES;
		sup->monsterinfo.aiflags = AI_NOSTEP;

		sup->touch = barrel_touch;

		// see if we can get away without drop to floor

		gi.linkentity(sup);

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_TELEPORT_EFFECT);
		gi.WritePosition(sup->s.origin);
		gi.multicast(sup->s.origin, MULTICAST_PVS);

		if (!(ent->client->anim_priority > ANIM_WAVE))
		{
			if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->client->anim_priority = ANIM_WAVE;
				ent->s.frame = FRAME_crattak1 - 1;
				ent->client->anim_end = FRAME_crattak3;
			}
			else {

				ent->client->anim_priority = ANIM_WAVE;
				ent->s.frame = FRAME_point01 - 1;
				ent->client->anim_end = FRAME_point06;
			}
		}

		gi.sound(sup, CHAN_VOICE, gi.soundindex("world/land.wav"), 1, ATTN_NORM, 0);
	}

	ValidateSelectedItem(ent);
}

void Use_PTurret(edict_t *ent, gitem_t *item)
{
	int				index;
	gitem_t			*fitem;

	fitem = FindItem("Bullets");
	index = ITEM_INDEX(fitem);

	if (ent->client->pers.inventory[index] < 100)
	{
		gi.cprintf(ent, PRINT_HIGH, "Turret needs 100 bullets to deploy or resupply\n");
		return;
	}

	if (pturret_deploy(ent))
		ent->client->pers.inventory[index] -= 100;

	ValidateSelectedItem(ent);
}

void Use_Grunt(edict_t *ent, gitem_t *item)
{
	grunt_deploy(ent);

	ValidateSelectedItem(ent);
}

void Use_ThrowStim(edict_t *ent, gitem_t *item)
{
	vec3_t		forward;
	int			amount = 25;
	int			index;
	gitem_t		*fitem;

	fitem = FindItem("cells");
	index = ITEM_INDEX(fitem);

	if (ent->client->weaponstate == WEAPON_FIRING)
		return;

	if (ent->client->pers.inventory[index] < 5)
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
		return;
	}

	ent->client->pers.inventory[index] -= 5;

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);

	throw_stimpack(ent, amount, ent->s.origin, forward);

	if (!(ent->client->anim_priority > ANIM_WAVE))
	{
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			ent->client->anim_priority = ANIM_WAVE;
			ent->s.frame = FRAME_crattak1 - 1;
			ent->client->anim_end = FRAME_crattak3;
		}
		else {

			ent->client->anim_priority = ANIM_WAVE;
			ent->s.frame = FRAME_point01;
			ent->client->anim_end = FRAME_point06;
		}
	}

	ValidateSelectedItem(ent);
}

void AmmoStation_Think(edict_t *self)
{
	edict_t		*ent;

	gitem_t		*it;
	int			i;
	edict_t		*cl_ent;
	float		dist;
	vec3_t		v;

	if (level.time > self->timestamp + 180 || self->owner->client->ammostation_time != self->timestamp)
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BFG_BIGEXPLOSION);
		gi.WritePosition(self->s.origin);
		gi.multicast(self->s.origin, MULTICAST_PVS);

		gi.sound(self, CHAN_VOICE, gi.soundindex("items/respawn1.wav"), 1, ATTN_NORM, 0);

		if (self->teamchain)
			G_FreeEdict(self->teamchain);

		G_FreeEdict(self);
		return;
	}

	if (level.time > self->delay + 10)
	{

		for (i = 0; i < game.maxclients; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if (!cl_ent->inuse || game.clients[i].resp.spectator)
			{
				continue;
			}

			if (cl_ent->health <= 0)
				continue;

			VectorSubtract(cl_ent->s.origin, self->s.origin, v);
			dist = VectorLength(v);

			if (dist <= 128)
			{
				it = FindItem("shells");

				if (it)
				{
					Add_Ammo(cl_ent, it, 5);
					Pickup_doFX(cl_ent, it);
				}

				it = FindItem("bullets");

				if (it)
				{
					Add_Ammo(cl_ent, it, 25);
					Pickup_doFX(cl_ent, it);
				}

				it = FindItem("grenades");

				if (it)
				{
					Add_Ammo(cl_ent, it, 1);
					Pickup_doFX(cl_ent, it);
				}

				it = FindItem("rockets");

				if (it)
				{
					Add_Ammo(cl_ent, it, 1);
					Pickup_doFX(cl_ent, it);
				}

				it = FindItem("slugs");

				if (it)
				{
					Add_Ammo(cl_ent, it, 3);
					Pickup_doFX(cl_ent, it);
				}

				it = FindItem("cells");

				if (it)
				{
					Add_Ammo(cl_ent, it, 20);
					Pickup_doFX(cl_ent, it);
				}
			}
		}

		self->delay = level.time;
	}

	if (level.time > self->wait + 30)
	{
		self->wait = level.time;
		if (!self->teamchain || !self->teamchain->inuse)
		{
			ent = Drop_Item(self, FindItemByClassname("item_resupply"));
			self->teamchain = ent;
			ent->enemy = self;

			gi.sound(self, CHAN_VOICE, gi.soundindex("items/respawn1.wav"), 1.0, ATTN_NORM, 0);
		}
	}

	self->nextthink = level.time + FRAMETIME;
}

void Use_AmmoStation(edict_t *ent, gitem_t *item)
{
	vec3_t		offset, forward, right, start, end;
	vec3_t		mins, maxs;
	trace_t		tr;
	edict_t		*sup;

	if (ent->client->weaponstate == WEAPON_FIRING)
		return;

	AngleVectors(ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 0, ent->viewheight);
	G_ProjectSource(ent->s.origin, offset, forward, right, start);
	VectorMA(start, 384, forward, end);

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

	if (tr.fraction == 1.0)
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
		return; // need to hit something
	}

	if (tr.plane.normal[2])
	{
		if (tr.plane.normal[2] < 0.7)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
			return;
		}
	}

	if (tr.ent->s.modelindex != 1)
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
		return;
	}

	if (tr.surface && tr.surface->flags & SURF_SKY)
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
		return;
	}

	VectorSet(mins, -16, -16, 0);
	VectorSet(maxs, 16, 16, 12);

	VectorCopy(tr.endpos, start);
	VectorCopy(tr.endpos, end);

	end[2] += 56;

	tr = gi.trace(start, mins, maxs, end, NULL, MASK_SHOT);

	if (!tr.allsolid)
	{
		VectorCopy(start, end);
		VectorCopy(tr.endpos, start);

		tr = gi.trace(start, mins, maxs, end, NULL, MASK_SHOT);

		if (tr.allsolid)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
			return;
		}

		if (tr.plane.normal[2])
		{
			if (tr.plane.normal[2] < 0.7)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
				return;
			}
		}

		if (tr.ent->s.modelindex != 1)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
			return;
		}

		ent->client->ammostation_time = level.time;


		sup = G_Spawn();
		VectorCopy(tr.endpos, sup->s.origin);
		sup->s.angles[YAW] = ent->s.angles[YAW];
		sup->classname = "Ammo Station";
		sup->owner = ent;
		sup->timestamp = ent->client->ammostation_time;
		sup->think = AmmoStation_Think;
		sup->nextthink = level.time + FRAMETIME;

		sup->s.modelindex = gi.modelindex("models/objects/support/engineer.md2");
		sup->s.renderfx = RF_GLOW;

		sup->delay = level.time;
		sup->wait = level.time;

		gi.linkentity(sup);

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_TELEPORT_EFFECT);
		gi.WritePosition(sup->s.origin);
		gi.multicast(sup->s.origin, MULTICAST_PVS);

		if (!(ent->client->anim_priority > ANIM_WAVE))
		{
			if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->client->anim_priority = ANIM_WAVE;
				ent->s.frame = FRAME_crattak1 - 1;
				ent->client->anim_end = FRAME_crattak3;
			}
			else {

				ent->client->anim_priority = ANIM_WAVE;
				ent->s.frame = FRAME_point01 - 1;
				ent->client->anim_end = FRAME_point06;
			}
		}

	}

	ValidateSelectedItem(ent);
}

// This is a bit messy - probably should automate a lot of this with entries
// on the items themselves - functions for turning on or off their modname
// messages and which other items(s) to remove and what to add

// we can check against the tracking to see if the player has been awarded
// stuff like bouncer than certain classes get

void Use_Mod(edict_t *ent, gitem_t *item)
{
	int				index, n;
	gitem_t			*fitem;

	//if (!Q_stricmp(it->classname, classname))
	//item = FindItem("Security Tag");
	//index = ITEM_INDEX(item);
	//other->client->pers.inventory[index]--;

	// add classes to mods MU_??? ?
	// could iterate through inventory to remove all IT_CLASS

	// adjust current number of things like grenades in inventory to match?

	// Medic - normal health, limited ammo, limited weapons
	// gets medic station, ability to use cells to create health packs

	// remove weapons a class can't use

	if (item->flags & IT_CLASS)
	{
		if (!Q_stricmp(item->classname, "class_medic"))
		{
			// find other class packages and remove
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Medic Station"))] = 1;
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Throw Stim"))] = 1;

			ent->client->resp.coop_respawn.inventory[ITEM_INDEX(FindItem("Medic Station"))] = 1;
			ent->client->resp.coop_respawn.inventory[ITEM_INDEX(FindItem("Throw Stim"))] = 1;

			ent->health = 100;
			ent->max_health = 100;

			ent->client->pers.max_bullets = 160;
			ent->client->pers.max_shells = 80;
			ent->client->pers.max_rockets = 40;
			ent->client->pers.max_grenades = 10;
			ent->client->pers.max_cells = 160;
			ent->client->pers.max_slugs = 40;

			// some of these don't matter as medic can't use heavy weapons

			ent->client->pers.player_class |= (PC_MEDIC | PC_LIMITED_AMMO | PC_LIGHT_WEAPS | PC_NO_GL);
		}

		//Technician - low health, limited ammo, limited weapons
		//Explosive box, ammo station, turret and target painting drone
		//Gets tripmines?

		if (!Q_stricmp(item->classname, "class_tech"))
		{
			// find other class packages and remove
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Explosive Box"))] = 1;
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Ammo Station"))] = 1;
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Turret"))] = 1;

			ent->client->resp.coop_respawn.inventory[ITEM_INDEX(FindItem("Explosive Box"))] = 1;
			ent->client->resp.coop_respawn.inventory[ITEM_INDEX(FindItem("Ammo Station"))] = 1;
			ent->client->resp.coop_respawn.inventory[ITEM_INDEX(FindItem("Turret"))] = 1;

			if (ent->client->pers.inventory[ITEM_INDEX(FindItem("[Tripmines:On]"))] == 0)
			{
				ent->client->pers.inventory[ITEM_INDEX(FindItem("[Tripmines]"))] = 1;
				ent->client->resp.coop_respawn.inventory[ITEM_INDEX(FindItem("[Tripmines]"))] = 1;
			}

			ent->health = 80;
			ent->max_health = 80;

			ent->client->pers.max_bullets = 160;
			ent->client->pers.max_shells = 80;
			ent->client->pers.max_rockets = 40;
			ent->client->pers.max_grenades = 10;
			ent->client->pers.max_cells = 160;
			ent->client->pers.max_slugs = 40;

			ent->client->pers.player_class |= (PC_TECH | PC_LIMITED_AMMO | PC_LIGHT_WEAPS);
		}

		//Cyborg - gets higher max health, no ammo limits, no weapon limits
		// regenerates health upto 30% slowly, has power shield
		// Bonus with combat shotgun causes bleed, bleeding targets take more damage
		// from your shotgun
	

		if (!Q_stricmp(item->classname, "class_cyborg"))
		{
			if (ent->client->pers.inventory[ITEM_INDEX(FindItem("[Sabot Mod:On]"))] == 0)
			{
				ent->client->pers.inventory[ITEM_INDEX(FindItem("[Sabot Mod]"))] = 1;
				ent->client->resp.coop_respawn.inventory[ITEM_INDEX(FindItem("[Sabot Mod]"))] = 1;
			}

			//ent->health = 125;
			//ent->max_health = 125;
			ent->health = 55;
			ent->max_health = 70;

			ent->client->pers.max_bullets = 200;
			ent->client->pers.max_shells = 100;
			ent->client->pers.max_rockets = 50;
			ent->client->pers.max_grenades = 50;
			ent->client->pers.max_cells = 200;
			ent->client->pers.max_slugs = 50;

			ent->flags |= FL_POWER_ARMOR;

			ent->client->pers.player_class |= PC_CYBORG;

			gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power1.wav"), 1, ATTN_NORM, 0);
		}

		// Grunt - limited ammo, medium weapons, gets taunt ability

		if (!Q_stricmp(item->classname, "class_grunt"))
		{
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Taunt"))] = 1;
			ent->client->resp.coop_respawn.inventory[ITEM_INDEX(FindItem("Taunt"))] = 1;

			if (ent->client->pers.inventory[ITEM_INDEX(FindItem("[The Bouncer:On]"))] == 0)
			{
				ent->client->pers.inventory[ITEM_INDEX(FindItem("[The Bouncer]"))] = 1;
				ent->client->resp.coop_respawn.inventory[ITEM_INDEX(FindItem("[The Bouncer]"))] = 1;
			}

			ent->health = 100;
			ent->max_health = 100;

			ent->client->pers.max_bullets = 200;
			ent->client->pers.max_shells = 100;
			ent->client->pers.max_rockets = 50;
			ent->client->pers.max_grenades = 50;
			ent->client->pers.max_cells = 200;
			ent->client->pers.max_slugs = 50;

			ent->client->pers.player_class |= (PC_GRUNT | PC_LIMITED_AMMO | PC_MEDIUM_WEAPS);
		}

		// Assault - normal health, no ammo limits starts with max ammo set high
		// does higher damage for 15 seconds after any player kills a monster

		if (!Q_stricmp(item->classname, "class_assault"))
		{
			ent->health = 100;
			ent->max_health = 100;

			ent->client->pers.max_bullets = 300;
			ent->client->pers.max_shells = 200;
			ent->client->pers.max_rockets = 100;
			ent->client->pers.max_grenades = 100;
			ent->client->pers.max_cells = 300;
			ent->client->pers.max_slugs = 100;

			if (ent->client->pers.inventory[ITEM_INDEX(FindItem("[Blaster Mod:On]"))] == 0)
			{
				ent->client->pers.inventory[ITEM_INDEX(FindItem("[Blaster Mod]"))] = 1;
				ent->client->resp.coop_respawn.inventory[ITEM_INDEX(FindItem("[Blaster Mod]"))] = 1;
			}

			ent->client->pers.player_class |= (PC_ASSAULT | PC_QUAD_BONUS);
		}

		ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
		ent->client->resp.coop_respawn.inventory[ITEM_INDEX(item)] = 0;

		ent->client->resp.coop_respawn.player_class = ent->client->pers.player_class;

		/*for (n = 0; n < game.num_items; n++)
		{
			if (itemlist[n].flags & IT_CLASS)
			{
				ent->client->pers.inventory[n] = 0;
				ent->client->resp.coop_respawn.inventory[n] = 0;
			}
		}*/

		ent->client->resp.coop_respawn.health = ent->health;
		ent->client->resp.coop_respawn.max_health = ent->max_health;


		ent->client->resp.coop_respawn.max_bullets = ent->client->pers.max_bullets;
		ent->client->resp.coop_respawn.max_shells = ent->client->pers.max_shells;
		ent->client->resp.coop_respawn.max_rockets = ent->client->pers.max_rockets;
		ent->client->resp.coop_respawn.max_grenades = ent->client->pers.max_grenades;
		ent->client->resp.coop_respawn.max_cells = ent->client->pers.max_cells;
		ent->client->resp.coop_respawn.max_slugs = ent->client->pers.max_slugs;

		fitem = FindItem("Blaster");
		ent->client->newweapon = fitem;
		ChangeWeapon(ent);

		for (n = 0; n < game.num_items; n++)
		{
			if (itemlist[n].flags & IT_CLASS)
			{
				ent->client->pers.inventory[n] = 0;
				ent->client->resp.coop_respawn.inventory[n] = 0;
			}

			if (itemlist[n].flags & IT_WEAPON)
			{
				if (ent->client->pers.player_class & PC_NO_GL)
				{
					if (!Q_stricmp(itemlist[n].classname, "weapon_grenadelauncher"))
					{
						ent->client->pers.inventory[n] = 0;
						ent->client->resp.coop_respawn.inventory[n] = 0;
					}
				}

				if (ent->client->pers.player_class & PC_LIGHT_WEAPS)
				{
					if (!(!Q_stricmp(itemlist[n].classname, "weapon_machinegun") ||
						!Q_stricmp(itemlist[n].classname, "weapon_shotgun") ||
						!Q_stricmp(itemlist[n].classname, "weapon_grenadelauncher") ||
						!Q_stricmp(itemlist[n].classname, "ammo_grenades") ||
						!Q_stricmp(itemlist[n].classname, "weapon_flaregun") ||
						!Q_stricmp(itemlist[n].classname, "weapon_blaster")
						))
					{

						ent->client->pers.inventory[n] = 0;
						ent->client->resp.coop_respawn.inventory[n] = 0;
					}
				}

				if (ent->client->pers.player_class & PC_MEDIUM_WEAPS)
				{
					if (!(!Q_stricmp(itemlist[n].classname, "weapon_machinegun") ||
						!Q_stricmp(itemlist[n].classname, "weapon_shotgun") ||
						!Q_stricmp(itemlist[n].classname, "weapon_grenadelauncher") ||
						!Q_stricmp(itemlist[n].classname, "ammo_grenades") ||
						!Q_stricmp(itemlist[n].classname, "weapon_supershotgun") ||
						!Q_stricmp(itemlist[n].classname, "weapon_chaingun") ||
						!Q_stricmp(itemlist[n].classname, "weapon_flaregun") ||
						!Q_stricmp(itemlist[n].classname, "weapon_blaster")
						))
					{

						ent->client->pers.inventory[n] = 0;
						ent->client->resp.coop_respawn.inventory[n] = 0;
					}
				}
			}
		}

		ValidateSelectedItem(ent);

		return;
	}

	if (!Q_stricmp(item->classname, "item_blasteru"))
	{
		fitem = FindItem("[Blaster Mod:On]");
		ent->client->pers.mods |= MU_BLASTER;

		gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/sshotr1b.wav"), 1, ATTN_NORM, 0);

		gi.cprintf(ent, PRINT_HIGH, "When available Blaster now uses cells for higher damage\n");
	}

	if (!Q_stricmp(item->classname, "item_blasterue"))
	{
		fitem = FindItem("[Blaster Mod]");
		ent->client->pers.mods &= ~MU_BLASTER;

		gi.cprintf(ent, PRINT_HIGH, "Blaster modification disabled\n");

		gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/sshotr1b.wav"), 1, ATTN_NORM, 0);
	}

	if (!Q_stricmp(item->classname, "item_mgu"))
	{
		fitem = FindItem("[Machinegun Mod:On]");
		ent->client->pers.mods |= MU_MG;

		gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/sshotr1b.wav"), 1, ATTN_NORM, 0);

		gi.cprintf(ent, PRINT_HIGH, "Incendiary rounds available for machinegun\n");
		gi.cprintf(ent, PRINT_HIGH, "Incendiary rounds do 25%% more damage to light monsters\n");
		gi.cprintf(ent, PRINT_HIGH, "And an additional 2 damage per second for 5 seconds\n");
	}

	if (!Q_stricmp(item->classname, "item_mgue"))
	{
		fitem = FindItem("[Machinegun Mod]");
		ent->client->pers.mods &= ~MU_MG;

		gi.cprintf(ent, PRINT_HIGH, "You go back to regular ammo\n");

		gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/sshotr1b.wav"), 1, ATTN_NORM, 0);
	}

	if (!Q_stricmp(item->classname, "item_healthu"))
	{
		fitem = FindItem("[Regen Mod:On]");
		ent->client->pers.mods |= MU_REGEN;
		ent->client->pers.pool_health_max = 200;
		ent->client->healthtick_time = level.time;
		if (ent->client->pers.pool_health > 200)
			ent->client->pers.pool_health = 200;

		gi.cprintf(ent, PRINT_HIGH, "Health now regenerates over time from a pool based on health pickups\n");
		gi.cprintf(ent, PRINT_HIGH, "Health pickups into the pool get a 25 percent bonus amount pool capped at 200\n");

		gi.sound(ent, CHAN_ITEM, gi.soundindex("medic/medatck5.wav"), 1, ATTN_NORM, 0);
	}

	if (!Q_stricmp(item->classname, "item_healthue"))
	{
		fitem = FindItem("[Regen Mod]");
		ent->client->pers.mods &= ~MU_REGEN;

		gi.cprintf(ent, PRINT_HIGH, "Health regeneration disabled\n");

		gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
	}

	if (!Q_stricmp(item->classname, "item_tombu"))
	{
		if (deathmatch->value)
		{
			gi.cprintf(ent, PRINT_HIGH, "This modification can't be used in deathmatch\n");
			return;
		}

		fitem = FindItem("[Tombstone:On]");
		ent->client->pers.mods |= MU_TOMBSTONE;

		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect3.wav"), 1, ATTN_NORM, 0);

		gi.cprintf(ent, PRINT_HIGH, "Gibbed monsters drop armor\n");
	}

	if (!Q_stricmp(item->classname, "item_tombue"))
	{
		fitem = FindItem("[Tombstone]");
		ent->client->pers.mods &= ~MU_TOMBSTONE;

		gi.cprintf(ent, PRINT_HIGH, "You no longer get bonus armor from gibbing monsters\n");

		gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
	}

	if (!Q_stricmp(item->classname, "item_grenadeu"))
	{
		if (deathmatch->value)
		{
			gi.cprintf(ent, PRINT_HIGH, "This modification can't be used in deathmatch\n");
			return;
		}

		if (ent->client->pers.mods & MU_TRIP)
		{
			gi.cprintf(ent, PRINT_HIGH, "This modification can't be used with tripmines\n");
			return;
		}

		fitem = FindItem("[Grenade Mod:On]");
		ent->client->pers.mods |= MU_GRENADE;

		gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/sshotr1b.wav"), 1, ATTN_NORM, 0);

		gi.cprintf(ent, PRINT_HIGH, "Grenades now actively seek targets\n");
	}

	if (!Q_stricmp(item->classname, "item_grenadeue"))
	{
		fitem = FindItem("[Grenade Mod]");
		ent->client->pers.mods &= ~MU_GRENADE;

		gi.cprintf(ent, PRINT_HIGH, "Grenades no long actively seek targets\n");

		gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/sshotr1b.wav"), 1, ATTN_NORM, 0);
	}

	if (!Q_stricmp(item->classname, "item_gambleru"))
	{
		if (deathmatch->value)
		{
			gi.cprintf(ent, PRINT_HIGH, "This modification can't be used in deathmatch\n");
			return;
		}

		fitem = FindItem("[The Gambler:On]");
		ent->client->pers.mods |= MU_GAMBLER;

		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect3.wav"), 1, ATTN_NORM, 0);

		gi.cprintf(ent, PRINT_HIGH, "Every kill has a chance to decrease your health by 25 points\n");
		gi.cprintf(ent, PRINT_HIGH, "Leaving you with at least 1 health\n");
		gi.cprintf(ent, PRINT_HIGH, "And a chance to enable a 5 second quad damage\n");
		gi.cprintf(ent, PRINT_HIGH, "Kills at 1 health have a chance to gain 15%% of victim's max health\n");
	}

	if (!Q_stricmp(item->classname, "item_gamblerue"))
	{
		fitem = FindItem("[The Gambler]");
		ent->client->pers.mods &= ~MU_GAMBLER;

		gi.cprintf(ent, PRINT_HIGH, "You no longer gamble\n");

		gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
	}

	if (!Q_stricmp(item->classname, "item_edgeu"))
	{
		if (deathmatch->value)
		{
			gi.cprintf(ent, PRINT_HIGH, "This modification can't be used in deathmatch\n");
			return;
		}

		fitem = FindItem("[The Edge:On]");
		ent->client->pers.mods |= MU_EDGE;

		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect3.wav"), 1, ATTN_NORM, 0);

		gi.cprintf(ent, PRINT_HIGH, "You take double damage but deal 75 percent more\n");
	}

	if (!Q_stricmp(item->classname, "item_edgeue"))
	{
		fitem = FindItem("[The Edge]");
		ent->client->pers.mods &= ~MU_EDGE;

		gi.cprintf(ent, PRINT_HIGH, "You step back from the edge\n");

		gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
	}

	if (!Q_stricmp(item->classname, "item_solaru"))
	{
		fitem = FindItem("[Solar:On]");
		ent->client->pers.mods |= MU_SOLAR;

		gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/sshotr1b.wav"), 1, ATTN_NORM, 0);

		gi.cprintf(ent, PRINT_HIGH, "When in bright areas some of your ammo cells recharge\n");
	}

	if (!Q_stricmp(item->classname, "item_solarue"))
	{
		fitem = FindItem("[Solar]");
		ent->client->pers.mods &= ~MU_SOLAR;

		gi.cprintf(ent, PRINT_HIGH, "Solar recharging disabled\n");

		gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/sshotr1b.wav"), 1, ATTN_NORM, 0);
	}

	if (!Q_stricmp(item->classname, "item_sabotu"))
	{
		fitem = FindItem("[Sabot Mod:On]");
		ent->client->pers.mods |= MU_SABOT;

		gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/sshotr1b.wav"), 1, ATTN_NORM, 0);

		gi.cprintf(ent, PRINT_HIGH, "Combat shotgun now shoots sabot slugs\n");
		gi.cprintf(ent, PRINT_HIGH, "Damage is increased 15%% against armoured targets\n");
	}

	if (!Q_stricmp(item->classname, "item_sabotue"))
	{
		fitem = FindItem("[Sabot Mod]");
		ent->client->pers.mods &= ~MU_SABOT;

		gi.cprintf(ent, PRINT_HIGH, "Combat shotgun no longer shoots sabot slugs\n");

		gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/sshotr1b.wav"), 1, ATTN_NORM, 0);
	}

	if (!Q_stricmp(item->classname, "item_berserku"))
	{
		if (deathmatch->value)
		{
			gi.cprintf(ent, PRINT_HIGH, "This modification can't be used in deathmatch\n");
			return;
		}

		fitem = FindItem("[Berserk:On]");
		ent->client->pers.mods |= MU_BERSERK;

		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect3.wav"), 1, ATTN_NORM, 0);

		gi.cprintf(ent, PRINT_HIGH, "You go on a rampage\n");
		gi.cprintf(ent, PRINT_HIGH, "Killing a monster with a shotgun activates berserk mode\n");
		gi.cprintf(ent, PRINT_HIGH, "While using a shotgun your damage and knockback is increased\n");
		gi.cprintf(ent, PRINT_HIGH, "Short range damage is reduced a lot all other damage is reduced to half\n");
	}

	if (!Q_stricmp(item->classname, "item_berserkue"))
	{
		fitem = FindItem("[Berserk]");
		ent->client->pers.mods &= ~MU_BERSERK;

		gi.cprintf(ent, PRINT_HIGH, "You regain your senses\n");

		gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
	}

	if (!Q_stricmp(item->classname, "item_reapu"))
	{
		if (deathmatch->value)
		{
			gi.cprintf(ent, PRINT_HIGH, "This modification can't be used in deathmatch\n");
			return;
		}

		fitem = FindItem("[The Reaper:On]");
		ent->client->pers.mods |= MU_REAPER;

		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect3.wav"), 1, ATTN_NORM, 0);

		//gi.cprintf(ent, PRINT_HIGH, "Every kill you gain 10%% of victim's max health, surprising a monster gets 15%\n");
		gi.cprintf(ent, PRINT_HIGH, "Every kill you gain 2%% of victim's max health, surprising a monster gets 4%\n");
		gi.cprintf(ent, PRINT_HIGH, "Railgun shots cost you 20%% of max health but do 1.5x damage\n");
		gi.cprintf(ent, PRINT_HIGH, "Railgun shots activate the reaper - kills within the timer gain you extra health\n");
	}

	if (!Q_stricmp(item->classname, "item_reapue"))
	{
		fitem = FindItem("[The Reaper]");
		ent->client->pers.mods &= ~MU_REAPER;

		gi.cprintf(ent, PRINT_HIGH, "Back to reality\n");

		gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
	}

	if (!Q_stricmp(item->classname, "item_glu"))
	{
		if (deathmatch->value)
		{
			gi.cprintf(ent, PRINT_HIGH, "This modification can't be used in deathmatch\n");
			return;
		}

		fitem = FindItem("[GLauncher Mod:On]");
		ent->client->pers.mods |= MU_GL;

		gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/sshotr1b.wav"), 1, ATTN_NORM, 0);

		gi.cprintf(ent, PRINT_HIGH, "Grenade launcher fires further, does increased radius damage\nand causes monsters to bleed 4 health for 5 seconds\n");
	}

	if (!Q_stricmp(item->classname, "item_glue"))
	{
		fitem = FindItem("[GLauncher Mod]");
		ent->client->pers.mods &= ~MU_GL;

		gi.cprintf(ent, PRINT_HIGH, "Grenade launcher bonuses removed\n");

		gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/sshotr1b.wav"), 1, ATTN_NORM, 0);
	}

	if (!Q_stricmp(item->classname, "item_tripu"))
	{
		if (deathmatch->value)
		{
			gi.cprintf(ent, PRINT_HIGH, "This modification can't be used in deathmatch\n");
			return;
		}

		if (ent->client->pers.mods & MU_GRENADE)
		{
			gi.cprintf(ent, PRINT_HIGH, "This modification can't be used with seeking grenades\n");
			return;
		}

		fitem = FindItem("[Tripmines:On]");
		ent->client->pers.mods |= MU_TRIP;

		gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/sshotr1b.wav"), 1, ATTN_NORM, 0);

		gi.cprintf(ent, PRINT_HIGH, "Grenades now stick to surfaces detonated by a laser trip\n");
	}

	if (!Q_stricmp(item->classname, "item_tripue"))
	{
		fitem = FindItem("[Tripmines]");
		ent->client->pers.mods &= ~MU_TRIP;

		gi.cprintf(ent, PRINT_HIGH, "Grenades no longer act as trip mines\n");

		gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/sshotr1b.wav"), 1, ATTN_NORM, 0);
	}

	if (!Q_stricmp(item->classname, "item_scavu"))
	{
		fitem = FindItem("[Scavenger:On]");
		ent->client->pers.mods |= MU_SCAVENGER;

		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect3.wav"), 1, ATTN_NORM, 0);

		gi.cprintf(ent, PRINT_HIGH, "You can find items on dead soldiers\n");
	}

	if (!Q_stricmp(item->classname, "item_scavue"))
	{
		fitem = FindItem("[Scavenger]");
		ent->client->pers.mods &= ~MU_SCAVENGER;

		gi.cprintf(ent, PRINT_HIGH, "You are now less capable at finding resources\n");

		gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
	}

	if (!Q_stricmp(item->classname, "item_adu"))
	{
		fitem = FindItem("[Active Defns:On]");
		ent->client->pers.mods |= MU_ACTIVEDEF;

		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect3.wav"), 1, ATTN_NORM, 0);

		gi.cprintf(ent, PRINT_HIGH, "Active defence online - hostile rockets and grenades are destroyed\n");
		gi.cprintf(ent, PRINT_HIGH, "At the cost of 3 energy cells, 2 second cooldown between activations\n");
	}

	if (!Q_stricmp(item->classname, "item_adue"))
	{
		fitem = FindItem("[Active Defns]");
		ent->client->pers.mods &= ~MU_ACTIVEDEF;

		gi.cprintf(ent, PRINT_HIGH, "Active defence offline\n");

		gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
	}

	if (!Q_stricmp(item->classname, "item_stimu"))
	{
		fitem = FindItem("[Stim Pack:On]");
		ent->client->pers.mods |= MU_STIM;

		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect3.wav"), 1, ATTN_NORM, 0);

		gi.cprintf(ent, PRINT_HIGH, "Stim pack - after picking up health you take half damage for 15 seconds and jump higher\n");
	}

	if (!Q_stricmp(item->classname, "item_stimue"))
	{
		fitem = FindItem("[Stime Pack]");
		ent->client->pers.mods &= ~MU_STIM;

		gi.cprintf(ent, PRINT_HIGH, "Stim no longer in effect\n");

		gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
	}

	if (!Q_stricmp(item->classname, "item_bounceru"))
	{
		fitem = FindItem("[The Bouncer:On]");
		ent->client->pers.mods |= MU_BOUNCER;

		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect3.wav"), 1, ATTN_NORM, 0);

		gi.cprintf(ent, PRINT_HIGH, "Every hit with the chaingun adds a damage resistance bonus\n");
		gi.cprintf(ent, PRINT_HIGH, "And 1 health upto 1.5 times max health\n");
		gi.cprintf(ent, PRINT_HIGH, "Every miss drops time from the bonus and 1 health while above 100 health\n");
	}

	if (!Q_stricmp(item->classname, "item_bouncerue"))
	{
		fitem = FindItem("[The Bouncer]");
		ent->client->pers.mods &= ~MU_BOUNCER;

		gi.cprintf(ent, PRINT_HIGH, "Bouncer no longer in effect\n");

		gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
	}

	if (fitem)
		index = ITEM_INDEX(fitem);

	if (index)
	{
		index = ITEM_INDEX(fitem);
		ent->client->pers.inventory[index] = 1;
		ent->client->resp.coop_respawn.inventory[index] = 1;
	}

	ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
	ent->client->resp.coop_respawn.inventory[ITEM_INDEX(item)] = 0;

	ent->client->resp.coop_respawn.mods = ent->client->pers.mods;

	ValidateSelectedItem(ent);
}

//======================================================================

qboolean Pickup_Key(edict_t *ent, edict_t *other)
{
    if (coop->value) {
        if (strcmp(ent->classname, "key_power_cube") == 0) {
            if (other->client->pers.power_cubes & ((ent->spawnflags & 0x0000ff00) >> 8))
                return qfalse;
            other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
            other->client->pers.power_cubes |= ((ent->spawnflags & 0x0000ff00) >> 8);
        } else {
            if (other->client->pers.inventory[ITEM_INDEX(ent->item)])
                return qfalse;
            other->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;
        }
        return qtrue;
    }
    other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	// Rroff trigger event on key pickup
	doEvent(ent, EVENTFLAG_PICKUP_KEY);

    return qtrue;
}

//======================================================================

qboolean Pickup_Mat(edict_t *ent, edict_t *other)
{
	other->client->pers.inventory[ITEM_INDEX(ent->item)]+= ent->item->quantity;

	if (!Q_stricmp(ent->classname, "item_cybernetics"))
	{
		//gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/sshotr1b.wav"), 1, ATTN_NORM, 0);

		gi.cprintf(other, PRINT_HIGH, "Prototype cybernetic components\n");
	}

	if (!Q_stricmp(ent->classname, "item_electronics"))
	{
		//gi.sound(ent, CHAN_ITEM, gi.soundindex("weapons/sshotr1b.wav"), 1, ATTN_NORM, 0);

		gi.cprintf(other, PRINT_HIGH, "A random selection of electrical parts\n");
	}

	if (!Q_stricmp(ent->classname, "item_plating"))
	{

		gi.cprintf(other, PRINT_HIGH, "Armour plating\n");
	}

	if (!Q_stricmp(ent->classname, "item_mechs"))
	{

		gi.cprintf(other, PRINT_HIGH, "Collection of Strogg mechanical parts\n");
	}

	if (!Q_stricmp(ent->classname, "item_cpu"))
	{

		gi.cprintf(other, PRINT_HIGH, "Intact Strogg processing unit\n");
	}

	return qtrue;
}

//======================================================================

qboolean Add_Ammo(edict_t *ent, gitem_t *item, int count)
{
    int         index;
    int         max;

    if (!ent->client)
        return qfalse;

    if (item->tag == AMMO_BULLETS)
        max = ent->client->pers.max_bullets;
    else if (item->tag == AMMO_SHELLS)
        max = ent->client->pers.max_shells;
    else if (item->tag == AMMO_ROCKETS)
        max = ent->client->pers.max_rockets;
    else if (item->tag == AMMO_GRENADES)
        max = ent->client->pers.max_grenades;
    else if (item->tag == AMMO_CELLS)
        max = ent->client->pers.max_cells;
    else if (item->tag == AMMO_SLUGS)
        max = ent->client->pers.max_slugs;
    else
        return qfalse;

    index = ITEM_INDEX(item);

    if (ent->client->pers.inventory[index] == max)
        return qfalse;

    ent->client->pers.inventory[index] += count;

    if (ent->client->pers.inventory[index] > max)
        ent->client->pers.inventory[index] = max;

    return qtrue;
}

qboolean Pickup_Ammo(edict_t *ent, edict_t *other)
{
    int         oldcount;
    int         count;
    qboolean    weapon;

    weapon = (ent->item->flags & IT_WEAPON);
    if ((weapon) && ((int)dmflags->value & DF_INFINITE_AMMO))
        count = 1000;
    else if (ent->count)
        count = ent->count;
    else
        count = ent->item->quantity;

    oldcount = other->client->pers.inventory[ITEM_INDEX(ent->item)];

    if (!Add_Ammo(other, ent->item, count))
        return qfalse;

    if (weapon && !oldcount) {
        if (other->client->pers.weapon != ent->item && (!deathmatch->value || other->client->pers.weapon == FindItem("blaster")))
            other->client->newweapon = ent->item;
    }

    if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) && (deathmatch->value || level.lockdown == 1 || level.lockdown == 2))
        SetRespawn(ent, 30);
    return qtrue;
}

void Drop_Ammo(edict_t *ent, gitem_t *item)
{
    edict_t *dropped;
    int     index;

    index = ITEM_INDEX(item);
    dropped = Drop_Item(ent, item);
    if (ent->client->pers.inventory[index] >= item->quantity)
        dropped->count = item->quantity;
    else
        dropped->count = ent->client->pers.inventory[index];

    if (ent->client->pers.weapon &&
        ent->client->pers.weapon->tag == AMMO_GRENADES &&
        item->tag == AMMO_GRENADES &&
        ent->client->pers.inventory[index] - dropped->count <= 0) {
        gi.cprintf(ent, PRINT_HIGH, "Can't drop current weapon\n");
        G_FreeEdict(dropped);
        return;
    }

    ent->client->pers.inventory[index] -= dropped->count;
    ValidateSelectedItem(ent);
}


//======================================================================

void MegaHealth_think(edict_t *self)
{
    if (self->owner->health > self->owner->max_health) {
        self->nextthink = level.time + 1;
        self->owner->health -= 1;
        return;
    }

    if (!(self->spawnflags & DROPPED_ITEM) && (deathmatch->value))
        SetRespawn(self, 20);
    else
        G_FreeEdict(self);
}

qboolean Pickup_Health(edict_t *ent, edict_t *other)
{

	if (other->client->pers.mods & MU_REGEN)
	{ 
		other->client->pers.pool_health += ent->count * 1.25;
		if (other->client->pers.pool_health > other->client->pers.pool_health_max)
			other->client->pers.pool_health = other->client->pers.pool_health_max;
	} else {
		if (!(ent->style & HEALTH_IGNORE_MAX))
			if (other->health >= other->max_health)
				return qfalse;

		other->health += ent->count;

		if (!(ent->style & HEALTH_IGNORE_MAX)) {
			if (other->health > other->max_health)
				other->health = other->max_health;
		}
	}

	if (other->client->pers.mods & MU_STIM)
	{
		other->client->stim_time = level.time;
	}

    if (ent->style & HEALTH_TIMED) {
        ent->think = MegaHealth_think;
        ent->nextthink = level.time + 5;
        ent->owner = other;
        ent->flags |= FL_RESPAWN;
        ent->svflags |= SVF_NOCLIENT;
        ent->solid = SOLID_NOT;
    } else {
        if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value || level.lockdown == 1 || level.lockdown == 2))
            SetRespawn(ent, 30);
    }

    return qtrue;
}

//======================================================================

int ArmorIndex(edict_t *ent)
{
    if (!ent->client)
        return 0;

    if (ent->client->pers.inventory[jacket_armor_index] > 0)
        return jacket_armor_index;

    if (ent->client->pers.inventory[combat_armor_index] > 0)
        return combat_armor_index;

    if (ent->client->pers.inventory[body_armor_index] > 0)
        return body_armor_index;

    return 0;
}

qboolean Pickup_Armor(edict_t *ent, edict_t *other)
{
    int             old_armor_index;
    gitem_armor_t   *oldinfo;
    gitem_armor_t   *newinfo;
    int             newcount;
    float           salvage;
    int             salvagecount;

    // get info on new armor
    newinfo = (gitem_armor_t *)ent->item->info;

    old_armor_index = ArmorIndex(other);

    // handle armor shards specially
    if (ent->item->tag == ARMOR_SHARD) {
        if (!old_armor_index)
            other->client->pers.inventory[jacket_armor_index] = 2;
        else
            other->client->pers.inventory[old_armor_index] += 2;
    }

    // if player has no armor, just use it
    else if (!old_armor_index) {
        other->client->pers.inventory[ITEM_INDEX(ent->item)] = newinfo->base_count;
    }

    // use the better armor
    else {
        // get info on old armor
        if (old_armor_index == jacket_armor_index)
            oldinfo = &jacketarmor_info;
        else if (old_armor_index == combat_armor_index)
            oldinfo = &combatarmor_info;
        else // (old_armor_index == body_armor_index)
            oldinfo = &bodyarmor_info;

        if (newinfo->normal_protection > oldinfo->normal_protection) {
            // calc new armor values
            salvage = oldinfo->normal_protection / newinfo->normal_protection;
            salvagecount = salvage * other->client->pers.inventory[old_armor_index];
            newcount = newinfo->base_count + salvagecount;
            if (newcount > newinfo->max_count)
                newcount = newinfo->max_count;

            // zero count of old armor so it goes away
            other->client->pers.inventory[old_armor_index] = 0;

            // change armor to new item with computed value
            other->client->pers.inventory[ITEM_INDEX(ent->item)] = newcount;
        } else {
            // calc new armor values
            salvage = newinfo->normal_protection / oldinfo->normal_protection;
            salvagecount = salvage * newinfo->base_count;
            newcount = other->client->pers.inventory[old_armor_index] + salvagecount;
            if (newcount > oldinfo->max_count)
                newcount = oldinfo->max_count;

            // if we're already maxed out then we don't need the new armor
            if (other->client->pers.inventory[old_armor_index] >= newcount)
                return qfalse;

            // update current armor value
            other->client->pers.inventory[old_armor_index] = newcount;
        }
    }

    //if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value || level.lockdown == 1 || level.lockdown == 2))
        SetRespawn(ent, 20);

    return qtrue;
}

//======================================================================

int PowerArmorType(edict_t *ent)
{
    if (!ent->client)
        return POWER_ARMOR_NONE;

    if (!(ent->flags & FL_POWER_ARMOR))
        return POWER_ARMOR_NONE;

	if (ent->client->pers.player_class & PC_CYBORG)
		return POWER_ARMOR_SHIELD;

    if (ent->client->pers.inventory[power_shield_index] > 0)
        return POWER_ARMOR_SHIELD;

    if (ent->client->pers.inventory[power_screen_index] > 0)
        return POWER_ARMOR_SCREEN;

    return POWER_ARMOR_NONE;
}

void Use_PowerArmor(edict_t *ent, gitem_t *item)
{
    int     index;

    if (ent->flags & FL_POWER_ARMOR) {
        ent->flags &= ~FL_POWER_ARMOR;
        gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
    } else {
        index = ITEM_INDEX(FindItem("cells"));
        if (!ent->client->pers.inventory[index]) {
            gi.cprintf(ent, PRINT_HIGH, "No cells for power armor.\n");
            return;
        }
        ent->flags |= FL_POWER_ARMOR;
        gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power1.wav"), 1, ATTN_NORM, 0);
    }
}

qboolean Pickup_PowerArmor(edict_t *ent, edict_t *other)
{
    int     quantity;

	// Rroff - cyborg has power armor by default
	if (other->client->pers.player_class & PC_CYBORG)
	{
		return qfalse;
	}

    quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

    other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

    if (deathmatch->value) {
        if (!(ent->spawnflags & DROPPED_ITEM))
            SetRespawn(ent, ent->item->quantity);
        // auto-use for DM only if we didn't already have one
        if (!quantity)
            ent->item->use(other, ent->item);
    }

    return qtrue;
}

void Drop_PowerArmor(edict_t *ent, gitem_t *item)
{
    if ((ent->flags & FL_POWER_ARMOR) && (ent->client->pers.inventory[ITEM_INDEX(item)] == 1))
        Use_PowerArmor(ent, item);
    Drop_General(ent, item);
}

//======================================================================

/*
===============
Touch_Item
===============
*/
void Touch_Item(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
    qboolean    taken;

    if (!other->client)
        return;
    if (other->health < 1)
        return;     // dead people can't pickup
    if (!ent->item->pickup)
        return;     // not a grabbable item?

    taken = ent->item->pickup(ent, other);

    if (taken) {
        // flash the screen
        other->client->bonus_alpha = 0.25;

        // show icon and name on status bar
		other->client->ps.stats[STAT_PICKUP_BACK] = gi.imageindex("tag3");
        other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(ent->item->icon);
        other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS + ITEM_INDEX(ent->item);
        other->client->pickup_msg_time = level.time + 3.0;

        // change selected item
        if (ent->item->use)
            other->client->pers.selected_item = other->client->ps.stats[STAT_SELECTED_ITEM] = ITEM_INDEX(ent->item);

        if (ent->item->pickup == Pickup_Health) {
            if (ent->count == 2)
                gi.sound(other, CHAN_ITEM, gi.soundindex("items/s_health.wav"), 1, ATTN_NORM, 0);
            else if (ent->count == 10)
                gi.sound(other, CHAN_ITEM, gi.soundindex("items/n_health.wav"), 1, ATTN_NORM, 0);
            else if (ent->count == 25)
                gi.sound(other, CHAN_ITEM, gi.soundindex("items/l_health.wav"), 1, ATTN_NORM, 0);
            else // (ent->count == 100)
                gi.sound(other, CHAN_ITEM, gi.soundindex("items/m_health.wav"), 1, ATTN_NORM, 0);
        } else if (ent->item->pickup_sound) {
            gi.sound(other, CHAN_ITEM, gi.soundindex(ent->item->pickup_sound), 1, ATTN_NORM, 0);
        }
    }

    if (!(ent->spawnflags & ITEM_TARGETS_USED)) {
        G_UseTargets(ent, other);
        ent->spawnflags |= ITEM_TARGETS_USED;
    }

    if (!taken)
        return;

    if (!((coop->value) && (ent->item->flags & IT_STAY_COOP)) || (ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM))) {
        if (ent->flags & FL_RESPAWN)
            ent->flags &= ~FL_RESPAWN;
        else
            G_FreeEdict(ent);
    }
}

//======================================================================

void drop_temp_touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
    if (other == ent->owner)
        return;

    Touch_Item(ent, other, plane, surf);
}

void drop_make_touchable(edict_t *ent)
{
    ent->touch = Touch_Item;
    if (deathmatch->value) {
        ent->nextthink = level.time + 29;
        ent->think = G_FreeEdict;
    }
}

edict_t *Drop_Item(edict_t *ent, gitem_t *item)
{
    edict_t *dropped;
    vec3_t  forward, right;
    vec3_t  offset;

    dropped = G_Spawn();

    dropped->classname = item->classname;
    dropped->item = item;
    dropped->spawnflags = DROPPED_ITEM;
    dropped->s.effects = item->world_model_flags;
    dropped->s.renderfx = RF_GLOW;
    VectorSet(dropped->mins, -15, -15, -15);
    VectorSet(dropped->maxs, 15, 15, 15);
    gi.setmodel(dropped, dropped->item->world_model);
    dropped->solid = SOLID_TRIGGER;
    dropped->movetype = MOVETYPE_TOSS;
    dropped->touch = drop_temp_touch;
    dropped->owner = ent;

	if (Q_stricmp(dropped->classname, "item_health_dropl") == 0)
	{
		dropped->count = 25;
	}

	if (Q_stricmp(dropped->classname, "item_health_dropm") == 0)
	{
		dropped->count = 10;
	}

	if (Q_stricmp(dropped->classname, "item_health_drops") == 0)
	{
		dropped->count = 2;
		dropped->style = HEALTH_IGNORE_MAX;
	}

    if (ent->client) {
        trace_t trace;

        AngleVectors(ent->client->v_angle, forward, right, NULL);
        VectorSet(offset, 24, 0, -16);
        G_ProjectSource(ent->s.origin, offset, forward, right, dropped->s.origin);
        trace = gi.trace(ent->s.origin, dropped->mins, dropped->maxs,
                         dropped->s.origin, ent, CONTENTS_SOLID);
        VectorCopy(trace.endpos, dropped->s.origin);
    } else {
        AngleVectors(ent->s.angles, forward, right, NULL);
        VectorCopy(ent->s.origin, dropped->s.origin);
    }

    VectorScale(forward, 100, dropped->velocity);
    dropped->velocity[2] = 300;

    dropped->think = drop_make_touchable;
    dropped->nextthink = level.time + 1;

    gi.linkentity(dropped);

    return dropped;
}

void Use_Item(edict_t *ent, edict_t *other, edict_t *activator)
{
    ent->svflags &= ~SVF_NOCLIENT;
    ent->use = NULL;

    if (ent->spawnflags & ITEM_NO_TOUCH) {
        ent->solid = SOLID_BBOX;
        ent->touch = NULL;
    } else {
        ent->solid = SOLID_TRIGGER;
        ent->touch = Touch_Item;
    }

    gi.linkentity(ent);
}

//======================================================================

/*
================
droptofloor
================
*/
void droptofloor(edict_t *ent)
{
    trace_t     tr;
    vec3_t      dest;
    float       *v;

    v = tv(-15, -15, -15);
    VectorCopy(v, ent->mins);
    v = tv(15, 15, 15);
    VectorCopy(v, ent->maxs);

    if (ent->model)
        gi.setmodel(ent, ent->model);
    else
        gi.setmodel(ent, ent->item->world_model);
    ent->solid = SOLID_TRIGGER;
    ent->movetype = MOVETYPE_TOSS;
    ent->touch = Touch_Item;

    v = tv(0, 0, -128);
    VectorAdd(ent->s.origin, v, dest);

    tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
    if (tr.startsolid) {
        gi.dprintf("droptofloor: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
        G_FreeEdict(ent);
        return;
    }

    VectorCopy(tr.endpos, ent->s.origin);

    if (ent->team) {
        ent->flags &= ~FL_TEAMSLAVE;
        ent->chain = ent->teamchain;
        ent->teamchain = NULL;

        ent->svflags |= SVF_NOCLIENT;
        ent->solid = SOLID_NOT;
        if (ent == ent->teammaster) {
            ent->nextthink = level.time + FRAMETIME;
            ent->think = DoRespawn;
        }
    }

    if (ent->spawnflags & ITEM_NO_TOUCH) {
        ent->solid = SOLID_BBOX;
        ent->touch = NULL;
        ent->s.effects &= ~EF_ROTATE;
        ent->s.renderfx &= ~RF_GLOW;
    }

    if (ent->spawnflags & ITEM_TRIGGER_SPAWN) {
        ent->svflags |= SVF_NOCLIENT;
        ent->solid = SOLID_NOT;
        ent->use = Use_Item;
    }

    gi.linkentity(ent);
}


/*
===============
PrecacheItem

Precaches all data needed for a given item.
This will be called for each item spawned in a level,
and for each item in each client's inventory.
===============
*/
void PrecacheItem(gitem_t *it)
{
    char    *s, *start;
    char    data[MAX_QPATH];
    int     len;
    gitem_t *ammo;

    if (!it)
        return;

    if (it->pickup_sound)
        gi.soundindex(it->pickup_sound);
    if (it->world_model)
        gi.modelindex(it->world_model);
    if (it->view_model)
        gi.modelindex(it->view_model);
    if (it->icon)
        gi.imageindex(it->icon);

    // parse everything for its ammo
    if (it->ammo && it->ammo[0]) {
        ammo = FindItem(it->ammo);
        if (ammo != it)
            PrecacheItem(ammo);
    }

    // parse the space seperated precache string for other items
    s = it->precaches;
    if (!s || !s[0])
        return;

    while (*s) {
        start = s;
        while (*s && *s != ' ')
            s++;

        len = s - start;
        if (len >= MAX_QPATH || len < 5)
            gi.error("PrecacheItem: %s has bad precache string", it->classname);
        memcpy(data, start, len);
        data[len] = 0;
        if (*s)
            s++;

        // determine type based on extension
        if (!strcmp(data + len - 3, "md2"))
            gi.modelindex(data);
        else if (!strcmp(data + len - 3, "sp2"))
            gi.modelindex(data);
        else if (!strcmp(data + len - 3, "wav"))
            gi.soundindex(data);
        if (!strcmp(data + len - 3, "pcx"))
            gi.imageindex(data);
    }
}

/*
============
SpawnItem

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might
be on an entity that hasn't spawned yet.
============
*/
void SpawnItem(edict_t *ent, gitem_t *item)
{
    PrecacheItem(item);

    if (ent->spawnflags) {
        if (strcmp(ent->classname, "key_power_cube") != 0) {
            ent->spawnflags = 0;
            gi.dprintf("%s at %s has invalid spawnflags set\n", ent->classname, vtos(ent->s.origin));
        }
    }

    // some items will be prevented in deathmatch
    if (deathmatch->value) {
        if ((int)dmflags->value & DF_NO_ARMOR) {
            if (item->pickup == Pickup_Armor || item->pickup == Pickup_PowerArmor) {
                G_FreeEdict(ent);
                return;
            }
        }
        if ((int)dmflags->value & DF_NO_ITEMS) {
            if (item->pickup == Pickup_Powerup) {
                G_FreeEdict(ent);
                return;
            }
        }
        if ((int)dmflags->value & DF_NO_HEALTH) {
            if (item->pickup == Pickup_Health || item->pickup == Pickup_Adrenaline || item->pickup == Pickup_AncientHead) {
                G_FreeEdict(ent);
                return;
            }
        }
        if ((int)dmflags->value & DF_INFINITE_AMMO) {
            if ((item->flags == IT_AMMO) || (strcmp(ent->classname, "weapon_bfg") == 0)) {
                G_FreeEdict(ent);
                return;
            }
        }
    }

    if (coop->value && (strcmp(ent->classname, "key_power_cube") == 0)) {
        ent->spawnflags |= (1 << (8 + level.power_cubes));
        level.power_cubes++;
    }

    // don't let them drop items that stay in a coop game
    if ((coop->value) && (item->flags & IT_STAY_COOP)) {
        item->drop = NULL;
    }

    ent->item = item;
    ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
    ent->think = droptofloor;
    ent->s.effects = item->world_model_flags;
    ent->s.renderfx = RF_GLOW;
    if (ent->model)
        gi.modelindex(ent->model);
}

//======================================================================

gitem_t itemlist[] = {
    {
        NULL
    },  // leave index 0 alone

	// Rroff do upgrades here so they are prominent in the list

	// need to create model/pics for these
	// should we change icons for weapons, etc. when they are active?
	{
		"item_healthu",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE,
		NULL,
		/* icon */      "p_megahealth",
		/* pickup */    "[Regen Mod]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_REGEN,
		/* precache */ ""
	},

	{
		"item_healthue",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE,
		NULL,
		/* icon */      "p_megahealth",
		/* pickup */    "[Regen Mod:On]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_REGEN,
		/* precache */ ""
	},

	{
		"item_blasteru",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE,
		NULL,
		/* icon */      "a_blaster",
		/* pickup */    "[Blaster Mod]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_BLASTER,
		/* precache */ ""
	},

	{
		"item_blasterue",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE,
		NULL,
		/* icon */      "a_blaster",
		/* pickup */    "[Blaster Mod:On]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_BLASTER,
		/* precache */ ""
	},

	{
		"item_mgu",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE,
		NULL,
		/* icon */      "w_machinegun",
		/* pickup */    "[Machinegun Mod]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_MG,
		/* precache */ ""
	},

	{
		"item_mgue",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE,
		NULL,
		/* icon */      "w_machinegun",
		/* pickup */    "[Machinegun Mod:On]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_MG,
		/* precache */ ""
	},

	{
		"item_tombu",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE, // need a proper model
		NULL,
		/* icon */      "i_jacketarmor",
		/* pickup */    "[Tombstone]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_TOMBSTONE,
		/* precache */ ""
	},

	{
		"item_tombue",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "i_jacketarmor",
		/* pickup */    "[Tombstone:On]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_TOMBSTONE,
		/* precache */ ""
	},

	{
		"item_grenadeu",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE, // need a proper model
		NULL,
		/* icon */      "a_grenades",
		/* pickup */    "[Grenade Mod]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_GRENADE,
		/* precache */ ""
	},

	{
		"item_grenadeue",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "a_grenades",
		/* pickup */    "[Grenade Mod:On]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_GRENADE,
		/* precache */ ""
	},

	{
		"item_glu",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE, // need a proper model
		NULL,
		/* icon */      "w_glauncher",
		/* pickup */    "[GLauncher Mod]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_GL,
		/* precache */ ""
	},

	{
		"item_glue",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "w_glauncher",
		/* pickup */    "[GLauncher Mod:On]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_GL,
		/* precache */ ""
	},


	{
		"item_gambleru",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE, // need a proper model
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "[The Gambler]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_GAMBLER,
		/* precache */ ""
				},

	{
		"item_gamblerue",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "[The Gambler:On]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_GAMBLER,
		/* precache */ ""
	},

	{
		"item_edgeu",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE, // need a proper model
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "[The Edge]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_EDGE,
		/* precache */ ""
	},

	{
		"item_edgeue",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "[The Edge:On]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_EDGE,
		/* precache */ ""
	},

	{
		"item_solaru",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE, // need a proper model
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "[Solar]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_SOLAR,
		/* precache */ ""
	},

	{
		"item_solarue",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "[Solar:On]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_SOLAR,
		/* precache */ ""
	},

	{
		"item_sabotu",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE, // need a proper model
		NULL,
		/* icon */      "w_shotgun",
		/* pickup */    "[Sabot Mod]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_SABOT,
		/* precache */ ""
	},

	{
		"item_sabotue",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "w_shotgun",
		/* pickup */    "[Sabot Mod:On]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_SABOT,
		/* precache */ ""
	},

	{
		"item_berserku",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE, // need a proper model
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "[Berserk]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_BERSERK,
		/* precache */ ""
	},

	{
		"item_berserkue",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "[Berserk:On]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_BERSERK,
		/* precache */ ""
	},

	{
		"item_reapu",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE, // need a proper model
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "[The Reaper]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_REAPER,
		/* precache */ ""
	},

	{
		"item_reapue",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "[The Reaper:On]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_REAPER,
		/* precache */ ""
	},

	{
		"item_tripu",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE, // need a proper model
		NULL,
		/* icon */      "a_grenades",
		/* pickup */    "[Tripmines]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_TRIP,
		/* precache */ ""
	},

	{
		"item_tripue",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "a_grenades",
		/* pickup */    "[Tripmines:On]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_TRIP,
		/* precache */ ""
	},

	{
		"item_scavu",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE, // need a proper model
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "[Scavenger]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_SCAVENGER,
		/* precache */ ""
	},

	{
		"item_scavue",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "[Scavenger:On]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_SCAVENGER,
		/* precache */ ""
	},

	{
		"item_adu",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE, // need a proper model
		NULL,
		/* icon */      "a_blaster",
		/* pickup */    "[Active Defns]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_ACTIVEDEF,
		/* precache */ ""
	},

	{
		"item_adue",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "a_blaster",
		/* pickup */    "[Active Defns:On]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_ACTIVEDEF,
		/* precache */ ""
	},

	{
		"item_stimu",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE, // need a proper model
		NULL,
		/* icon */      "i_health",
		/* pickup */    "[Stim Pack]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_STIM,
		/* precache */ ""
	},

	{
		"item_stimue",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "i_health",
		/* pickup */    "[Stim Pack:On]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_STIM,
		/* precache */ ""
	},

	{
		"item_bounceru",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE, // need a proper model
		NULL,
		/* icon */      "w_chaingun",
		/* pickup */    "[The Bouncer]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_BOUNCER,
		/* precache */ ""
	},

	{
		"item_bouncerue",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "w_chaingun",
		/* pickup */    "[The Bouncer:On]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_MOD,
		0,
		NULL,
		MU_BOUNCER,
		/* precache */ ""
	},

	// Rroff - ability items here

	{
		"item_support",
		Pickup_Powerup,
		Use_Support,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "i_health",
		/* pickup */    "Medic Station", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
		/* precache */ ""
	},

	{
		"item_taunt",
		Pickup_Powerup,
		Use_Taunt,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "i_bodyarmor",
		/* pickup */    "Taunt", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
		/* precache */ ""
	},

	{
		"item_explobox",
		Pickup_Powerup,
		Use_ExploBox,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "a_blaster",
		/* pickup */    "Explosive Box", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
		/* precache */ ""
	},

	{
		"item_medicstim",
		Pickup_Powerup,
		Use_ThrowStim,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "i_health",
		/* pickup */    "Throw Stim", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
		/* precache */ ""
	},

	{
		"item_ammostation",
		Pickup_Powerup,
		Use_AmmoStation,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "a_bullets",
		/* pickup */    "Ammo Station", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
		/* precache */ ""
	},

	{
		"item_pturret",
		Pickup_Powerup,
		Use_PTurret,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "w_chaingun",
		/* pickup */    "Turret",
		/* width */     2,
		1,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
		/* precache */ ""
	},

	{
		"item_grunt",
		Pickup_Powerup,
		Use_Grunt,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "Grunt",
		/* width */     2,
		1,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
		/* precache */ ""
	},

	{
		"class_medic",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "i_health",
		/* pickup */    "[Field Medic]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_CLASS,
		0,
		NULL,
		0,
		/* precache */ ""
	},

	{
		"class_tech",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "[Technician]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_CLASS,
		0,
		NULL,
		0,
		/* precache */ ""
	},

	{
		"class_cyborg",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "[Cyborg]",
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_CLASS,
		0,
		NULL,
		0,
		/* precache */ ""
	},

	{
		"class_grunt",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "[Grunt]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_CLASS,
		0,
		NULL,
		0,
		/* precache */ ""
	},

	{
		"class_assault",
		Pickup_Powerup,
		Use_Mod,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2",EF_ROTATE,
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "[Assault]", // Need a better name
		/* width */     2,
		1,
		NULL,
		IT_POWERUP | IT_CLASS,
		0,
		NULL,
		0,
		/* precache */ ""
	},

    //
    // ARMOR
    //

    /*QUAKED item_armor_body (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "item_armor_body",
        Pickup_Armor,
        NULL,
        NULL,
        NULL,
        "misc/ar1_pkup.wav",
        "models/items/armor/body/tris.md2", EF_ROTATE,
        NULL,
        /* icon */      "i_bodyarmor",
        /* pickup */    "Body Armor",
        /* width */     3,
        0,
        NULL,
        IT_ARMOR,
        0,
        &bodyarmor_info,
        ARMOR_BODY,
        /* precache */ ""
    },

    /*QUAKED item_armor_combat (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "item_armor_combat",
        Pickup_Armor,
        NULL,
        NULL,
        NULL,
        "misc/ar1_pkup.wav",
        "models/items/armor/combat/tris.md2", EF_ROTATE,
        NULL,
        /* icon */      "i_combatarmor",
        /* pickup */    "Combat Armor",
        /* width */     3,
        0,
        NULL,
        IT_ARMOR,
        0,
        &combatarmor_info,
        ARMOR_COMBAT,
        /* precache */ ""
    },

    /*QUAKED item_armor_jacket (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "item_armor_jacket",
        Pickup_Armor,
        NULL,
        NULL,
        NULL,
        "misc/ar1_pkup.wav",
        "models/items/armor/jacket/tris.md2", EF_ROTATE,
        NULL,
        /* icon */      "i_jacketarmor",
        /* pickup */    "Jacket Armor",
        /* width */     3,
        0,
        NULL,
        IT_ARMOR,
        0,
        &jacketarmor_info,
        ARMOR_JACKET,
        /* precache */ ""
    },

    /*QUAKED item_armor_shard (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "item_armor_shard",
        Pickup_Armor,
        NULL,
        NULL,
        NULL,
        "misc/ar2_pkup.wav",
        "models/items/armor/shard/tris.md2", EF_ROTATE,
        NULL,
        /* icon */      "i_jacketarmor",
        /* pickup */    "Armor Shard",
        /* width */     3,
        0,
        NULL,
        IT_ARMOR,
        0,
        NULL,
        ARMOR_SHARD,
        /* precache */ ""
    },


    /*QUAKED item_power_screen (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "item_power_screen",
        Pickup_PowerArmor,
        Use_PowerArmor,
        Drop_PowerArmor,
        NULL,
        "misc/ar3_pkup.wav",
        "models/items/armor/screen/tris.md2", EF_ROTATE,
        NULL,
        /* icon */      "i_powerscreen",
        /* pickup */    "Power Screen",
        /* width */     0,
        60,
        NULL,
        IT_ARMOR,
        0,
        NULL,
        0,
        /* precache */ ""
    },

    /*QUAKED item_power_shield (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "item_power_shield",
        Pickup_PowerArmor,
        Use_PowerArmor,
        Drop_PowerArmor,
        NULL,
        "misc/ar3_pkup.wav",
        "models/items/armor/shield/tris.md2", EF_ROTATE,
        NULL,
        /* icon */      "i_powershield",
        /* pickup */    "Power Shield",
        /* width */     0,
        60,
        NULL,
        IT_ARMOR,
        0,
        NULL,
        0,
        /* precache */ "misc/power2.wav misc/power1.wav"
    },


    //
    // WEAPONS
    //

    /* weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)
    always owned, never in the world
    */
    {
        "weapon_blaster",
        NULL,
        Use_Weapon,
        NULL,
        Weapon_Blaster,
        "misc/w_pkup.wav",
        NULL, 0,
        "models/weapons/v_blast/tris.md2",
        /* icon */      "w_blaster",
        /* pickup */    "Blaster",
        0,
        0,
        NULL,
        IT_WEAPON | IT_STAY_COOP,
        WEAP_BLASTER,
        NULL,
        0,
        /* precache */ "weapons/blastf1a.wav misc/lasfly.wav"
    },

    /*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "weapon_shotgun",
        Pickup_Weapon,
        Use_Weapon,
        Drop_Weapon,
        Weapon_Shotgun,
        "misc/w_pkup.wav",
        "models/weapons/g_shotg/tris.md2", EF_ROTATE,
        "models/weapons/v_shotg/tris.md2",
        /* icon */      "w_shotgun",
        /* pickup */    "Shotgun",
        0,
        1,
        "Shells",
        IT_WEAPON | IT_STAY_COOP,
        WEAP_SHOTGUN,
        NULL,
        0,
        /* precache */ "weapons/shotgf1b.wav weapons/shotgr1b.wav"
    },

    /*QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "weapon_supershotgun",
        Pickup_Weapon,
        Use_Weapon,
        Drop_Weapon,
        Weapon_SuperShotgun,
        "misc/w_pkup.wav",
        "models/weapons/g_shotg2/tris.md2", EF_ROTATE,
        "models/weapons/v_shotg2/tris.md2",
        /* icon */      "w_sshotgun",
        /* pickup */    "Super Shotgun",
        0,
        2,
        "Shells",
        IT_WEAPON | IT_STAY_COOP,
        WEAP_SUPERSHOTGUN,
        NULL,
        0,
        /* precache */ "weapons/sshotf1b.wav"
    },

    /*QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "weapon_machinegun",
        Pickup_Weapon,
        Use_Weapon,
        Drop_Weapon,
        Weapon_Machinegun,
        "misc/w_pkup.wav",
        "models/weapons/g_machn/tris.md2", EF_ROTATE,
        "models/weapons/v_machn/tris.md2",
        /* icon */      "w_machinegun",
        /* pickup */    "Machinegun",
        0,
        1,
        "Bullets",
        IT_WEAPON | IT_STAY_COOP,
        WEAP_MACHINEGUN,
        NULL,
        0,
        /* precache */ "weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav"
    },

    /*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "weapon_chaingun",
        Pickup_Weapon,
        Use_Weapon,
        Drop_Weapon,
        Weapon_Chaingun,
        "misc/w_pkup.wav",
        "models/weapons/g_chain/tris.md2", EF_ROTATE,
        "models/weapons/v_chain/tris.md2",
        /* icon */      "w_chaingun",
        /* pickup */    "Chaingun",
        0,
        1,
        "Bullets",
        IT_WEAPON | IT_STAY_COOP,
        WEAP_CHAINGUN,
        NULL,
        0,
        /* precache */ "weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav` weapons/chngnd1a.wav"
    },

    /*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "ammo_grenades",
        Pickup_Ammo,
        Use_Weapon,
        Drop_Ammo,
        Weapon_Grenade,
        "misc/am_pkup.wav",
        "models/items/ammo/grenades/medium/tris.md2", 0,
        "models/weapons/v_handgr/tris.md2",
        /* icon */      "a_grenades",
        /* pickup */    "Grenades",
        /* width */     3,
        5,
        "grenades",
        IT_AMMO | IT_WEAPON,
        WEAP_GRENADES,
        NULL,
        AMMO_GRENADES,
        /* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
    },

    /*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "weapon_grenadelauncher",
        Pickup_Weapon,
        Use_Weapon,
        Drop_Weapon,
        Weapon_GrenadeLauncher,
        "misc/w_pkup.wav",
        "models/weapons/g_launch/tris.md2", EF_ROTATE,
        "models/weapons/v_launch/tris.md2",
        /* icon */      "w_glauncher",
        /* pickup */    "Grenade Launcher",
        0,
        1,
        "Grenades",
        IT_WEAPON | IT_STAY_COOP,
        WEAP_GRENADELAUNCHER,
        NULL,
        0,
        /* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
    },

    /*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "weapon_rocketlauncher",
        Pickup_Weapon,
        Use_Weapon,
        Drop_Weapon,
        Weapon_RocketLauncher,
        "misc/w_pkup.wav",
        "models/weapons/g_rocket/tris.md2", EF_ROTATE,
        "models/weapons/v_rocket/tris.md2",
        /* icon */      "w_rlauncher",
        /* pickup */    "Rocket Launcher",
        0,
        1,
        "Rockets",
        IT_WEAPON | IT_STAY_COOP,
        WEAP_ROCKETLAUNCHER,
        NULL,
        0,
        /* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
    },

    /*QUAKED weapon_hyperblaster (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "weapon_hyperblaster",
        Pickup_Weapon,
        Use_Weapon,
        Drop_Weapon,
        Weapon_HyperBlaster,
        "misc/w_pkup.wav",
        "models/weapons/g_hyperb/tris.md2", EF_ROTATE,
        "models/weapons/v_hyperb/tris.md2",
        /* icon */      "w_hyperblaster",
        /* pickup */    "HyperBlaster",
        0,
        1,
        "Cells",
        IT_WEAPON | IT_STAY_COOP,
        WEAP_HYPERBLASTER,
        NULL,
        0,
        /* precache */ "weapons/hyprbu1a.wav weapons/hyprbl1a.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav misc/lasfly.wav"
    },

    /*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "weapon_railgun",
        Pickup_Weapon,
        Use_Weapon,
        Drop_Weapon,
        Weapon_Railgun,
        "misc/w_pkup.wav",
        "models/weapons/g_rail/tris.md2", EF_ROTATE,
        "models/weapons/v_rail/tris.md2",
        /* icon */      "w_railgun",
        /* pickup */    "Railgun",
        0,
        1,
        "Slugs",
        IT_WEAPON | IT_STAY_COOP,
        WEAP_RAILGUN,
        NULL,
        0,
        /* precache */ "weapons/rg_hum.wav"
    },

    /*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "weapon_bfg",
        Pickup_Weapon,
        Use_Weapon,
        Drop_Weapon,
        Weapon_BFG,
        "misc/w_pkup.wav",
        "models/weapons/g_bfg/tris.md2", EF_ROTATE,
        "models/weapons/v_bfg/tris.md2",
        /* icon */      "w_bfg",
        /* pickup */    "BFG10K",
        0,
        50,
        "Cells",
        IT_WEAPON | IT_STAY_COOP,
        WEAP_BFG,
        NULL,
        0,
        /* precache */ "sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 weapons/bfg__f1y.wav weapons/bfg__l1a.wav weapons/bfg__x1b.wav weapons/bfg_hum.wav"
    },

	/*QUAKED weapon_flaregun (.3 .3 1) (-16 -16 -16) (16 16 16)*/
	{ 
		"weapon_flaregun", // class name 
		Pickup_Weapon, // Function to use to pickup weapon 
		Use_Weapon,  // Function to use to use weapon 
		Drop_Weapon, // Function to use to drop weapon 
		Weapon_FlareGun, // Function called every frame this weapon is active 
		"misc/w_pkup.wav",// Sound to play when picked up 
		"models/weapons/g_flareg/tris.md2", // Item model for placement on maps 
		EF_ROTATE,//Flags 
		"models/weapons/v_flareg/tris.md3",//Model player sees 
		"w_flareg", //name of item icon in item list (minus .pcx) 
		"Flare Gun", //Item name (ie use flare gun) 
		0, // Count width (for timed things like quad) 
		1, // Ammo per shot 
		"Grenades", // Type of ammo to use 
		IT_WEAPON, // IT_WEAPON, IT_ARMOR, or IT_AMMO 
		WEAP_FLAREGUN,
		NULL, // userinfo? (void*) 
		0, // tag 
		"" //things to precache 
	},
    //
    // AMMO ITEMS
    //

    /*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "ammo_shells",
        Pickup_Ammo,
        NULL,
        Drop_Ammo,
        NULL,
        "misc/am_pkup.wav",
        "models/items/ammo/shells/medium/tris.md2", 0,
        NULL,
        /* icon */      "a_shells",
        /* pickup */    "Shells",
        /* width */     3,
        10,
        NULL,
        IT_AMMO,
        0,
        NULL,
        AMMO_SHELLS,
        /* precache */ ""
    },

    /*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "ammo_bullets",
        Pickup_Ammo,
        NULL,
        Drop_Ammo,
        NULL,
        "misc/am_pkup.wav",
        "models/items/ammo/bullets/medium/tris.md2", 0,
        NULL,
        /* icon */      "a_bullets",
        /* pickup */    "Bullets",
        /* width */     3,
        50,
        NULL,
        IT_AMMO,
        0,
        NULL,
        AMMO_BULLETS,
        /* precache */ ""
    },

    /*QUAKED ammo_cells (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "ammo_cells",
        Pickup_Ammo,
        NULL,
        Drop_Ammo,
        NULL,
        "misc/am_pkup.wav",
        "models/items/ammo/cells/medium/tris.md2", 0,
        NULL,
        /* icon */      "a_cells",
        /* pickup */    "Cells",
        /* width */     3,
        50,
        NULL,
        IT_AMMO,
        0,
        NULL,
        AMMO_CELLS,
        /* precache */ ""
    },

    /*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "ammo_rockets",
        Pickup_Ammo,
        NULL,
        Drop_Ammo,
        NULL,
        "misc/am_pkup.wav",
        "models/items/ammo/rockets/medium/tris.md2", 0,
        NULL,
        /* icon */      "a_rockets",
        /* pickup */    "Rockets",
        /* width */     3,
        5,
        NULL,
        IT_AMMO,
        0,
        NULL,
        AMMO_ROCKETS,
        /* precache */ ""
    },

    /*QUAKED ammo_slugs (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "ammo_slugs",
        Pickup_Ammo,
        NULL,
        Drop_Ammo,
        NULL,
        "misc/am_pkup.wav",
        "models/items/ammo/slugs/medium/tris.md2", 0,
        NULL,
        /* icon */      "a_slugs",
        /* pickup */    "Slugs",
        /* width */     3,
        10,
        NULL,
        IT_AMMO,
        0,
        NULL,
        AMMO_SLUGS,
        /* precache */ ""
    },


    //
    // POWERUP ITEMS
    //
    /*QUAKED item_quad (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "item_quad",
        Pickup_Powerup,
        Use_Quad,
        Drop_General,
        NULL,
        "items/pkup.wav",
        "models/items/quaddama/tris.md2", EF_ROTATE,
        NULL,
        /* icon */      "p_quad",
        /* pickup */    "Quad Damage",
        /* width */     2,
        60,
        NULL,
        IT_POWERUP,
        0,
        NULL,
        0,
        /* precache */ "items/damage.wav items/damage2.wav items/damage3.wav"
    },

    /*QUAKED item_invulnerability (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "item_invulnerability",
        Pickup_Powerup,
        Use_Invulnerability,
        Drop_General,
        NULL,
        "items/pkup.wav",
        "models/items/invulner/tris.md2", EF_ROTATE,
        NULL,
        /* icon */      "p_invulnerability",
        /* pickup */    "Invulnerability",
        /* width */     2,
        300,
        NULL,
        IT_POWERUP,
        0,
        NULL,
        0,
        /* precache */ "items/protect.wav items/protect2.wav items/protect4.wav"
    },

    /*QUAKED item_silencer (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "item_silencer",
        Pickup_Powerup,
        Use_Silencer,
        Drop_General,
        NULL,
        "items/pkup.wav",
        "models/items/silencer/tris.md2", EF_ROTATE,
        NULL,
        /* icon */      "p_silencer",
        /* pickup */    "Silencer",
        /* width */     2,
        60,
        NULL,
        IT_POWERUP,
        0,
        NULL,
        0,
        /* precache */ ""
    },

    /*QUAKED item_breather (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "item_breather",
        Pickup_Powerup,
        Use_Breather,
        Drop_General,
        NULL,
        "items/pkup.wav",
        "models/items/breather/tris.md2", EF_ROTATE,
        NULL,
        /* icon */      "p_rebreather",
        /* pickup */    "Rebreather",
        /* width */     2,
        60,
        NULL,
        IT_STAY_COOP | IT_POWERUP,
        0,
        NULL,
        0,
        /* precache */ "items/airout.wav"
    },

    /*QUAKED item_enviro (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "item_enviro",
        Pickup_Powerup,
        Use_Envirosuit,
        Drop_General,
        NULL,
        "items/pkup.wav",
        "models/items/enviro/tris.md2", EF_ROTATE,
        NULL,
        /* icon */      "p_envirosuit",
        /* pickup */    "Environment Suit",
        /* width */     2,
        60,
        NULL,
        IT_STAY_COOP | IT_POWERUP,
        0,
        NULL,
        0,
        /* precache */ "items/airout.wav"
    },

    /*QUAKED item_ancient_head (.3 .3 1) (-16 -16 -16) (16 16 16)
    Special item that gives +2 to maximum health
    */
    {
        "item_ancient_head",
        Pickup_AncientHead,
        NULL,
        NULL,
        NULL,
        "items/pkup.wav",
        "models/items/c_head/tris.md2", EF_ROTATE,
        NULL,
        /* icon */      "i_fixme",
        /* pickup */    "Ancient Head",
        /* width */     2,
        60,
        NULL,
        0,
        0,
        NULL,
        0,
        /* precache */ ""
    },

    /*QUAKED item_adrenaline (.3 .3 1) (-16 -16 -16) (16 16 16)
    gives +1 to maximum health
    */
    {
        "item_adrenaline",
        Pickup_Adrenaline,
        NULL,
        NULL,
        NULL,
        "items/pkup.wav",
        "models/items/adrenal/tris.md2", EF_ROTATE,
        NULL,
        /* icon */      "p_adrenaline",
        /* pickup */    "Adrenaline",
        /* width */     2,
        60,
        NULL,
        0,
        0,
        NULL,
        0,
        /* precache */ ""
    },

	// Rroff - create custom assets for this
	/*QUAKED item_resupply (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"item_resupply",
		Pickup_Resupply,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/resupply/tris.md2", EF_ROTATE,
		NULL,
		/* icon */      "i_pack",
		/* pickup */    "Resupply Pack",
		/* width */     2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
		/* precache */ ""
	},

	// Rroff - create custom assets for this
	/*QUAKED item_resupply (.3 .3 1) (-16 -16 -16) (16 16 16)
	*/
	{
		"item_slingpack",
		Pickup_Slingpack,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/resupply/light.md2", EF_ROTATE,
		NULL,
		/* icon */      "i_pack",
		/* pickup */    "Sling Pack",
		/* width */     2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
		/* precache */ ""
	},

    /*QUAKED item_bandolier (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "item_bandolier",
        Pickup_Bandolier,
        NULL,
        NULL,
        NULL,
        "items/pkup.wav",
        "models/items/band/tris.md2", EF_ROTATE,
        NULL,
        /* icon */      "p_bandolier",
        /* pickup */    "Bandolier",
        /* width */     2,
        60,
        NULL,
        0,
        0,
        NULL,
        0,
        /* precache */ ""
    },

    /*QUAKED item_pack (.3 .3 1) (-16 -16 -16) (16 16 16)
    */
    {
        "item_pack",
        Pickup_Pack,
        NULL,
        NULL,
        NULL,
        "items/pkup.wav",
        "models/items/pack/tris.md2", EF_ROTATE,
        NULL,
        /* icon */      "i_pack",
        /* pickup */    "Ammo Pack",
        /* width */     2,
        180,
        NULL,
        0,
        0,
        NULL,
        0,
        /* precache */ ""
    },

    //
    // KEYS
    //
    /*QUAKED key_data_cd (0 .5 .8) (-16 -16 -16) (16 16 16)
    key for computer centers
    */
    {
        "key_data_cd",
        Pickup_Key,
        NULL,
        Drop_General,
        NULL,
        "items/pkup.wav",
        "models/items/keys/data_cd/tris.md2", EF_ROTATE,
        NULL,
        "k_datacd",
        "Data CD",
        2,
        0,
        NULL,
        IT_STAY_COOP | IT_KEY,
        0,
        NULL,
        0,
        /* precache */ ""
    },

    /*QUAKED key_power_cube (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN NO_TOUCH
    warehouse circuits
    */
    {
        "key_power_cube",
        Pickup_Key,
        NULL,
        Drop_General,
        NULL,
        "items/pkup.wav",
        "models/items/keys/power/tris.md2", EF_ROTATE,
        NULL,
        "k_powercube",
        "Power Cube",
        2,
        0,
        NULL,
        IT_STAY_COOP | IT_KEY,
        0,
        NULL,
        0,
        /* precache */ ""
    },

    /*QUAKED key_pyramid (0 .5 .8) (-16 -16 -16) (16 16 16)
    key for the entrance of jail3
    */
    {
        "key_pyramid",
        Pickup_Key,
        NULL,
        Drop_General,
        NULL,
        "items/pkup.wav",
        "models/items/keys/pyramid/tris.md2", EF_ROTATE,
        NULL,
        "k_pyramid",
        "Pyramid Key",
        2,
        0,
        NULL,
        IT_STAY_COOP | IT_KEY,
        0,
        NULL,
        0,
        /* precache */ ""
    },

    /*QUAKED key_data_spinner (0 .5 .8) (-16 -16 -16) (16 16 16)
    key for the city computer
    */
    {
        "key_data_spinner",
        Pickup_Key,
        NULL,
        Drop_General,
        NULL,
        "items/pkup.wav",
        "models/items/keys/spinner/tris.md2", EF_ROTATE,
        NULL,
        "k_dataspin",
        "Data Spinner",
        2,
        0,
        NULL,
        IT_STAY_COOP | IT_KEY,
        0,
        NULL,
        0,
        /* precache */ ""
    },

    /*QUAKED key_pass (0 .5 .8) (-16 -16 -16) (16 16 16)
    security pass for the security level
    */
    {
        "key_pass",
        Pickup_Key,
        NULL,
        Drop_General,
        NULL,
        "items/pkup.wav",
        "models/items/keys/pass/tris.md2", EF_ROTATE,
        NULL,
        "k_security",
        "Security Pass",
        2,
        0,
        NULL,
        IT_STAY_COOP | IT_KEY,
        0,
        NULL,
        0,
        /* precache */ ""
    },

	/*QUAKED key_tag (0 .5 .8) (-16 -16 -16) (16 16 16)
	security pass for inhibited doors
	*/
	{
		"key_tag",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/keys/pass/tris.md2", EF_ROTATE,
		NULL,
		"k_security",
		"Security Tag",
		2,
		0,
		NULL,
		IT_KEY,
		0,
		NULL,
		0,
		/* precache */ ""
	},

	/*QUAKED key_tag (0 .5 .8) (-16 -16 -16) (16 16 16)
	security pass for inhibited doors
	*/
	{
		"key_head",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/objects/gibs/head2/tris.md2", EF_ROTATE,
		NULL,
		"k_comhead",
		"Strogg Head",
		2,
		0,
		NULL,
		IT_KEY,
		0,
		NULL,
		0,
		/* precache */ ""
	},

    /*QUAKED key_blue_key (0 .5 .8) (-16 -16 -16) (16 16 16)
    normal door key - blue
    */
    {
        "key_blue_key",
        Pickup_Key,
        NULL,
        Drop_General,
        NULL,
        "items/pkup.wav",
        "models/items/keys/key/tris.md2", EF_ROTATE,
        NULL,
        "k_bluekey",
        "Blue Key",
        2,
        0,
        NULL,
        IT_STAY_COOP | IT_KEY,
        0,
        NULL,
        0,
        /* precache */ ""
    },

    /*QUAKED key_red_key (0 .5 .8) (-16 -16 -16) (16 16 16)
    normal door key - red
    */
    {
        "key_red_key",
        Pickup_Key,
        NULL,
        Drop_General,
        NULL,
        "items/pkup.wav",
        "models/items/keys/red_key/tris.md2", EF_ROTATE,
        NULL,
        "k_redkey",
        "Red Key",
        2,
        0,
        NULL,
        IT_STAY_COOP | IT_KEY,
        0,
        NULL,
        0,
        /* precache */ ""
    },

    /*QUAKED key_commander_head (0 .5 .8) (-16 -16 -16) (16 16 16)
    tank commander's head
    */
    {
        "key_commander_head",
        Pickup_Key,
        NULL,
        Drop_General,
        NULL,
        "items/pkup.wav",
        "models/monsters/commandr/head/tris.md2", EF_GIB,
        NULL,
        /* icon */      "k_comhead",
        /* pickup */    "Commander's Head",
        /* width */     2,
        0,
        NULL,
        IT_STAY_COOP | IT_KEY,
        0,
        NULL,
        0,
        /* precache */ ""
    },

    /*QUAKED key_airstrike_target (0 .5 .8) (-16 -16 -16) (16 16 16)
    tank commander's head
    */
    {
        "key_airstrike_target",
        Pickup_Key,
        NULL,
        Drop_General,
        NULL,
        "items/pkup.wav",
        "models/items/keys/target/tris.md2", EF_ROTATE,
        NULL,
        /* icon */      "i_airstrike",
        /* pickup */    "Airstrike Marker",
        /* width */     2,
        0,
        NULL,
        IT_STAY_COOP | IT_KEY,
        0,
        NULL,
        0,
        /* precache */ ""
    },

    {
        NULL,
        Pickup_Health,
        NULL,
        NULL,
        NULL,
        "items/pkup.wav",
        NULL, 0,
        NULL,
        /* icon */      "i_health",
        /* pickup */    "Health",
        /* width */     3,
        0,
        NULL,
        0,
        0,
        NULL,
        0,
        /* precache */ "items/s_health.wav items/n_health.wav items/l_health.wav items/m_health.wav"
    },

	{
		"item_health_dropl",
		Pickup_Health,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/healing/large/tris.md2", 0,
		NULL,
		/* icon */      "i_health",
		/* pickup */    "Health",
		/* width */     3,
		0,
		NULL,
		0,
		0,
		NULL,
		0,
		/* precache */ "items/s_health.wav items/n_health.wav items/l_health.wav items/m_health.wav"
	},

	{
		"item_health_dropm",
		Pickup_Health,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/healing/medium/tris.md2", 0,
		NULL,
		/* icon */      "i_health",
		/* pickup */    "Health",
		/* width */     3,
		0,
		NULL,
		0,
		0,
		NULL,
		0,
		/* precache */ "items/s_health.wav items/n_health.wav items/l_health.wav items/m_health.wav"
	},

	{
		"item_health_drops",
		Pickup_Health,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/healing/stimpack/tris.md2", 0,
		NULL,
		/* icon */      "i_health",
		/* pickup */    "Health",
		/* width */     3,
		0,
		NULL,
		0,
		0,
		NULL,
		0,
		/* precache */ "items/s_health.wav items/n_health.wav items/l_health.wav items/m_health.wav"
	},

	/*QUAKED item_cybernetics (.3 .3 1) (-16 -16 -16) (16 16 16)
	Materials
	*/
	{
		"item_cybernetics",
		Pickup_Mat,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/ammo/nuke/tris.md2", EF_ROTATE, // placeholder
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "#Cybernetic Components",
		/* width */     2,
		1,
		NULL,
		IT_MAT,
		0,
		NULL,
		0,
		/* precache */ ""
	},

	/*QUAKED item_electronics (.3 .3 1) (-16 -16 -16) (16 16 16)
	Materials
	*/
	{
		"item_electronics",
		Pickup_Mat,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/ammo/nuke/tris.md2", EF_ROTATE, // placeholder
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "#Electronic Parts",
		/* width */     2,
		1,
		NULL,
		IT_MAT,
		0,
		NULL,
		0,
		/* precache */ ""
	},

	/*QUAKED item_plating (.3 .3 1) (-16 -16 -16) (16 16 16)
	Materials
	*/
	{
		"item_plating",
		Pickup_Mat,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/ammo/nuke/tris.md2", EF_ROTATE, // placeholder
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "#Armour Plate",
		/* width */     2,
		1,
		NULL,
		IT_MAT,
		0,
		NULL,
		0,
		/* precache */ ""
	},

	/*QUAKED item_mechs (.3 .3 1) (-16 -16 -16) (16 16 16)
	Materials
	*/
	{
		"item_mechs",
		Pickup_Mat,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/ammo/nuke/tris.md2", EF_ROTATE, // placeholder
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "#Mechanical Parts",
		/* width */     2,
		1,
		NULL,
		IT_MAT,
		0,
		NULL,
		0,
		/* precache */ ""
	},

	/*QUAKED item_cpu (.3 .3 1) (-16 -16 -16) (16 16 16)
	Materials
	*/
	{
		"item_cpu",
		Pickup_Mat,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/ammo/nuke/tris.md2", EF_ROTATE, // placeholder
		NULL,
		/* icon */      "i_fixme",
		/* pickup */    "#Strogg CPU",
		/* width */     2,
		1,
		NULL,
		IT_MAT,
		0,
		NULL,
		0,
		/* precache */ ""
	},

    // end of list marker
    {NULL}
};


/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health(edict_t *self)
{
    if (deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH)) {
        G_FreeEdict(self);
        return;
    }

    self->model = "models/items/healing/medium/tris.md2";
    self->count = 10;
    SpawnItem(self, FindItem("Health"));
    gi.soundindex("items/n_health.wav");
}

/*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_small(edict_t *self)
{
    if (deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH)) {
        G_FreeEdict(self);
        return;
    }

    self->model = "models/items/healing/stimpack/tris.md2";
    self->count = 2;
    SpawnItem(self, FindItem("Health"));
    self->style = HEALTH_IGNORE_MAX;
    gi.soundindex("items/s_health.wav");
}

/*QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_large(edict_t *self)
{
    if (deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH)) {
        G_FreeEdict(self);
        return;
    }

    self->model = "models/items/healing/large/tris.md2";
    self->count = 25;
    SpawnItem(self, FindItem("Health"));
    gi.soundindex("items/l_health.wav");
}

/*QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_mega(edict_t *self)
{
    if (deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH)) {
        G_FreeEdict(self);
        return;
    }

    self->model = "models/items/mega_h/tris.md2";
    self->count = 100;
    SpawnItem(self, FindItem("Health"));
    gi.soundindex("items/m_health.wav");
    self->style = HEALTH_IGNORE_MAX | HEALTH_TIMED;
}


void InitItems(void)
{
    game.num_items = sizeof(itemlist) / sizeof(itemlist[0]) - 1;
}



/*
===============
SetItemNames

Called by worldspawn
===============
*/
void SetItemNames(void)
{
    int     i;
    gitem_t *it;

    for (i = 0 ; i < game.num_items ; i++) {
        it = &itemlist[i];
        gi.configstring(CS_ITEMS + i, it->pickup_name);
    }

    jacket_armor_index = ITEM_INDEX(FindItem("Jacket Armor"));
    combat_armor_index = ITEM_INDEX(FindItem("Combat Armor"));
    body_armor_index   = ITEM_INDEX(FindItem("Body Armor"));
    power_screen_index = ITEM_INDEX(FindItem("Power Screen"));
    power_shield_index = ITEM_INDEX(FindItem("Power Shield"));
}

void stim_touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		i;

	i = ent->count;

	if ((other == ent->owner) && (level.time < (ent->nextthink - 9.5)))
		return;

	if (other->client)
	{
		if (other->health > 0)
		{
			if (other->health >= other->max_health)
				return;

			gi.sound(other, CHAN_ITEM, gi.soundindex("items/s_health.wav"), 1, ATTN_NORM, 0);

			other->client->bonus_alpha = 0.25;

			other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex("i_health");
			other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS + ITEM_INDEX(FindItem("Health"));
			other->client->pickup_msg_time = level.time + 3.0;

			other->health += i;

			if (other->health > other->max_health)
			{
				other->health = other->max_health;
			}

			G_FreeEdict(ent);
		}
	}
}

void throw_stimpack(edict_t *ent, int amount, vec3_t start, vec3_t aimdir)
{
	edict_t		*stim;
	vec3_t		dir;
	vec3_t		forward, right, up;

	vectoangles(aimdir, dir);
	AngleVectors(dir, forward, right, up);


	stim = G_Spawn();

	VectorCopy(start, stim->s.origin);
	// VectorCopy(dir, stim->s.angles);

	VectorScale(aimdir, 800, stim->velocity);
	VectorMA(stim->velocity, 200 + crandom() * 10.0, up, stim->velocity);
	VectorMA(stim->velocity, crandom() * 10.0, right, stim->velocity);

	VectorSet(stim->mins, -15, -15, -15);
	VectorSet(stim->maxs, 15, 15, 15);


	stim->movetype = MOVETYPE_TOSS;
	stim->clipmask = MASK_SHOT;
	stim->solid = SOLID_TRIGGER;

	stim->s.modelindex = gi.modelindex("models/items/healing/stimpack/tris.md2");
	stim->s.renderfx = RF_GLOW;
	stim->gravity = 0.5;
	stim->count = amount;

	stim->owner = ent;
	stim->touch = stim_touch;
	stim->nextthink = level.time + 10;
	stim->think = G_FreeEdict;

	stim->classname = "stimpack";

	gi.linkentity(stim);
}