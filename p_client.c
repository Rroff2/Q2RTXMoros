/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2019, NVIDIA CORPORATION. All rights reserved.

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

void ClientUserinfoChanged(edict_t *ent, char *userinfo);

void SP_misc_teleporter_dest(edict_t *ent);

//
// Gross, ugly, disgustuing hack section
//

// this function is an ugly as hell hack to fix some map flaws
//
// the coop spawn spots on some maps are SNAFU.  There are coop spots
// with the wrong targetname as well as spots with no name at all
//
// we use carnal knowledge of the maps to fix the coop spot targetnames to match
// that of the nearest named single player spot

void SP_FixCoopSpots(edict_t *self)
{
    edict_t *spot;
    vec3_t  d;

    spot = NULL;

    while (1) {
        spot = G_Find(spot, FOFS(classname), "info_player_start");
        if (!spot)
            return;
        if (!spot->targetname)
            continue;
        VectorSubtract(self->s.origin, spot->s.origin, d);
        if (VectorLength(d) < 384) {
            if ((!self->targetname) || Q_stricmp(self->targetname, spot->targetname) != 0) {
//              gi.dprintf("FixCoopSpots changed %s at %s targetname from %s to %s\n", self->classname, vtos(self->s.origin), self->targetname, spot->targetname);
                self->targetname = spot->targetname;
            }
            return;
        }
    }
}

// now if that one wasn't ugly enough for you then try this one on for size
// some maps don't have any coop spots at all, so we need to create them
// where they should have been

void SP_CreateCoopSpots(edict_t *self)
{
    edict_t *spot;

    if (Q_stricmp(level.mapname, "security") == 0) {
        spot = G_Spawn();
        spot->classname = "info_player_coop";
        spot->s.origin[0] = 188 - 64;
        spot->s.origin[1] = -164;
        spot->s.origin[2] = 80;
        spot->targetname = "jail3";
        spot->s.angles[1] = 90;

        spot = G_Spawn();
        spot->classname = "info_player_coop";
        spot->s.origin[0] = 188 + 64;
        spot->s.origin[1] = -164;
        spot->s.origin[2] = 80;
        spot->targetname = "jail3";
        spot->s.angles[1] = 90;

        spot = G_Spawn();
        spot->classname = "info_player_coop";
        spot->s.origin[0] = 188 + 128;
        spot->s.origin[1] = -164;
        spot->s.origin[2] = 80;
        spot->targetname = "jail3";
        spot->s.angles[1] = 90;

        return;
    }
}


/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(edict_t *self)
{
    if (!coop->value)
        return;
    if (Q_stricmp(level.mapname, "security") == 0) {
        // invoke one of our gross, ugly, disgusting hacks
        self->think = SP_CreateCoopSpots;
        self->nextthink = level.time + FRAMETIME;
    }
}

// Rroff: Drop a flare for coop

void dropflare_playeruse(edict_t *self, edict_t *other)
{
	// if we use a flare and currently have one check it isn't our current flare
	// if we use a flare and it is the same as our old one swap them around

	if (other->client)
	{
		if (other->client->resp.flare)
		{
			if (self != other->client->resp.flare)
			{
				other->client->resp.oldflare = other->client->resp.flare;
				other->client->resp.flare = self;
			}
		} else {
			other->client->resp.flare = self;
		}
	}

	if (!(other->client->anim_priority > ANIM_WAVE))
	{
		if (other->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			other->client->anim_priority = ANIM_WAVE;
			other->s.frame = FRAME_crattak1 - 1;
			other->client->anim_end = FRAME_crattak3;
		} else {

			other->client->anim_priority = ANIM_WAVE;
			other->s.frame = FRAME_point01 - 1;
			other->client->anim_end = FRAME_point06;
		}

		other->client->weaponstate = WEAPON_ACTIVATING;
		other->client->ps.gunframe = 0;
	}


	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_SPARKS);
	gi.WritePosition(self->s.origin);
	gi.WriteDir(vec3_origin);
	gi.multicast(other->s.origin, MULTICAST_PVS);

	gi.sound(other, CHAN_AUTO, gi.soundindex("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
}
void drop_flare_think(edict_t *self)
{
	self->delay = 0;
}

void drop_flare(edict_t *other)
{
	edict_t		*ent;
	float		*v;

	ent = G_Spawn();

	ent->classname = "spawn_flare";
	//ent->solid = SOLID_NOT;

	v = tv(0, 0, -24);
	VectorAdd(other->s.origin, v, ent->s.origin);
	VectorCopy(other->s.angles, ent->s.angles);

	ent->s.modelindex = gi.modelindex("models/objects/laser/tris.md2");
	// ent->s.effects = EF_PLASMA | EF_TRACKER; (cool darkness effect)
	ent->s.effects = EF_IONRIPPER;
	ent->s.renderfx = RF_GLOW;
	ent->playeruse = dropflare_playeruse;

	// Be better if it was non-solid but need to fix being able to pick it

	VectorSet(ent->mins, -24, -24, 0);
	VectorSet(ent->maxs, 24, 24, 4);
	//VectorSet(ent->mins, -8, -8, 0);
	//VectorSet(ent->maxs, 8, 8, 8);
	ent->solid = SOLID_BBOX;
	//ent->svflags |= SVF_DEADMONSTER;
	ent->delay = 0;
	ent->think = drop_flare_think;

	gi.linkentity(ent);

}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/

void dropflare_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!other->client)
		return;

	if (other->health <= 0)
		return;

	if (other->client->resp.spectator)
		return;

	if ((self == other->client->resp.flare) || (self == other->client->resp.oldflare))
		return; // don't fire the same one again if it is an old or current one

	dropflare_playeruse(self, other);
}

void SP_info_player_deathmatch(edict_t *self)
{
	edict_t		*spot;
	vec3_t		d;
	qboolean	dosupply = qtrue;

	// Rroff - move these to a seperate list of points later
	// don't need to use up edicts on them
	if (!deathmatch->value) {
		VectorCopy(self->s.origin, self->pos1);
		self->think = spawn_roamer; // let other things spawn first
		self->nextthink = level.time + FRAMETIME;
		
		// don't need to link
		// gi.linkentity(self);

		if (coop->value)
		{
			//drop_flare(self);
			self->s.modelindex = gi.modelindex("models/objects/laser/tris.md2");
			self->s.effects = EF_IONRIPPER;
			self->s.renderfx = RF_GLOW;
			self->playeruse = dropflare_playeruse;
			self->touch = dropflare_touch;
			self->s.origin[2] -= 24; // might mess with roamer spawning

			VectorSet(self->mins, -24, -24, 0);
			VectorSet(self->maxs, 24, 24, 4);
			self->solid = SOLID_BBOX;
			self->movetype = MOVETYPE_STEP;
			self->mass = 99999;

			gi.linkentity(self);
		} else {
			self->svflags |= SVF_NOCLIENT;
		}

		/*if (random() < 0.15)
		{
			spot = NULL;

			// Rroff - looks kind of cool at the start of the game
			// when the pack gets thrown out
			if (!Q_stricmp(level.mapname, "base1"))
			{
				while (1) {
					spot = G_Find(spot, FOFS(classname), "info_player_start");
					if (!spot)
						break;
					VectorSubtract(self->s.origin, spot->s.origin, d);
					if (VectorLength(d) < 256) {
						dosupply = qfalse;
						break;
					}
				}
			}

			if (dosupply)
				Drop_Item(self, FindItemByClassname("item_resupply"));
		}*/
	}
	else {
		SP_misc_teleporter_dest(self);
	}
}

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games
*/

void SP_info_player_coop(edict_t *self)
{
    if (!coop->value) {
        G_FreeEdict(self);
        return;
    }

    if ((Q_stricmp(level.mapname, "jail2") == 0)   ||
        (Q_stricmp(level.mapname, "jail4") == 0)   ||
        (Q_stricmp(level.mapname, "mine1") == 0)   ||
        (Q_stricmp(level.mapname, "mine2") == 0)   ||
        (Q_stricmp(level.mapname, "mine3") == 0)   ||
        (Q_stricmp(level.mapname, "mine4") == 0)   ||
        (Q_stricmp(level.mapname, "lab") == 0)     ||
        (Q_stricmp(level.mapname, "boss1") == 0)   ||
        (Q_stricmp(level.mapname, "fact3") == 0)   ||
        (Q_stricmp(level.mapname, "biggun") == 0)  ||
        (Q_stricmp(level.mapname, "space") == 0)   ||
        (Q_stricmp(level.mapname, "command") == 0) ||
        (Q_stricmp(level.mapname, "power2") == 0) ||
        (Q_stricmp(level.mapname, "strike") == 0)) {
        // invoke one of our gross, ugly, disgusting hacks
        self->think = SP_FixCoopSpots;
        self->nextthink = level.time + FRAMETIME;
    }
}


/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
void SP_info_player_intermission(void)
{
}


//=======================================================================


void player_pain(edict_t *self, edict_t *other, float kick, int damage)
{
    // player pain is handled at the end of the frame in P_DamageFeedback
}


qboolean IsFemale(edict_t *ent)
{
    char        *info;

    if (!ent->client)
        return qfalse;

    info = Info_ValueForKey(ent->client->pers.userinfo, "gender");
    if (info[0] == 'f' || info[0] == 'F')
        return qtrue;
    return qfalse;
}

qboolean IsNeutral(edict_t *ent)
{
    char        *info;

    if (!ent->client)
        return qfalse;

    info = Info_ValueForKey(ent->client->pers.userinfo, "gender");
    if (info[0] != 'f' && info[0] != 'F' && info[0] != 'm' && info[0] != 'M')
        return qtrue;
    return qfalse;
}

void ClientObituary(edict_t *self, edict_t *inflictor, edict_t *attacker)
{
    int         mod;
    char        *message;
    char        *message2;
    qboolean    ff;

    if (coop->value && attacker->client)
        meansOfDeath |= MOD_FRIENDLY_FIRE;

    if (deathmatch->value || coop->value) {
        ff = meansOfDeath & MOD_FRIENDLY_FIRE;
        mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
        message = NULL;
        message2 = "";

        switch (mod) {
        case MOD_SUICIDE:
            message = "suicides";
            break;
        case MOD_FALLING:
            message = "cratered";
            break;
        case MOD_CRUSH:
            message = "was squished";
            break;
        case MOD_WATER:
            message = "sank like a rock";
            break;
        case MOD_SLIME:
            message = "melted";
            break;
        case MOD_LAVA:
            message = "does a back flip into the lava";
            break;
        case MOD_EXPLOSIVE:
        case MOD_BARREL:
            message = "blew up";
            break;
        case MOD_EXIT:
            message = "found a way out";
            break;
        case MOD_TARGET_LASER:
            message = "saw the light";
            break;
        case MOD_TARGET_BLASTER:
            message = "got blasted";
            break;
        case MOD_BOMB:
        case MOD_SPLASH:
        case MOD_TRIGGER_HURT:
            message = "was in the wrong place";
            break;
		case MOD_GAMBLE:
			message = "gambled and lost";
			break;

        }
        if (attacker == self) {
            switch (mod) {
            case MOD_HELD_GRENADE:
                message = "tried to put the pin back in";
                break;
            case MOD_HG_SPLASH:
            case MOD_G_SPLASH:
                if (IsNeutral(self))
                    message = "tripped on its own grenade";
                else if (IsFemale(self))
                    message = "tripped on her own grenade";
                else
                    message = "tripped on his own grenade";
                break;
            case MOD_R_SPLASH:
                if (IsNeutral(self))
                    message = "blew itself up";
                else if (IsFemale(self))
                    message = "blew herself up";
                else
                    message = "blew himself up";
                break;
            case MOD_BFG_BLAST:
                message = "should have used a smaller gun";
                break;
            default:
                if (IsNeutral(self))
                    message = "killed itself";
                else if (IsFemale(self))
                    message = "killed herself";
                else
                    message = "killed himself";
                break;
            }
        }
        if (message) {
            gi.bprintf(PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);
            if (deathmatch->value)
                self->client->resp.score--;
            self->enemy = NULL;
            return;
        }

        self->enemy = attacker;
        if (attacker && attacker->client) {
            switch (mod) {
            case MOD_BLASTER:
                message = "was blasted by";
                break;
            case MOD_SHOTGUN:
                message = "was gunned down by";
                break;
            case MOD_SSHOTGUN:
                message = "was blown away by";
                message2 = "'s super shotgun";
                break;
            case MOD_MACHINEGUN:
                message = "was machinegunned by";
                break;
            case MOD_CHAINGUN:
                message = "was cut in half by";
                message2 = "'s chaingun";
                break;
            case MOD_GRENADE:
                message = "was popped by";
                message2 = "'s grenade";
                break;
            case MOD_G_SPLASH:
                message = "was shredded by";
                message2 = "'s shrapnel";
                break;
            case MOD_ROCKET:
                message = "ate";
                message2 = "'s rocket";
                break;
            case MOD_R_SPLASH:
                message = "almost dodged";
                message2 = "'s rocket";
                break;
            case MOD_HYPERBLASTER:
                message = "was melted by";
                message2 = "'s hyperblaster";
                break;
            case MOD_RAILGUN:
                message = "was railed by";
                break;
            case MOD_BFG_LASER:
                message = "saw the pretty lights from";
                message2 = "'s BFG";
                break;
            case MOD_BFG_BLAST:
                message = "was disintegrated by";
                message2 = "'s BFG blast";
                break;
            case MOD_BFG_EFFECT:
                message = "couldn't hide from";
                message2 = "'s BFG";
                break;
            case MOD_HANDGRENADE:
                message = "caught";
                message2 = "'s handgrenade";
                break;
            case MOD_HG_SPLASH:
                message = "didn't see";
                message2 = "'s handgrenade";
                break;
            case MOD_HELD_GRENADE:
                message = "feels";
                message2 = "'s pain";
                break;
            case MOD_TELEFRAG:
                message = "tried to invade";
                message2 = "'s personal space";
                break;
            }
            if (message) {
                gi.bprintf(PRINT_MEDIUM, "%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
                if (deathmatch->value) {
                    if (ff)
                        attacker->client->resp.score--;
                    else
                        attacker->client->resp.score++;
                }
                return;
            }
        }
    }

    gi.bprintf(PRINT_MEDIUM, "%s died.\n", self->client->pers.netname);
    if (deathmatch->value)
        self->client->resp.score--;
}


void Touch_Item(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void TossClientWeapon(edict_t *self)
{
    gitem_t     *item;
    edict_t     *drop;
    qboolean    quad;
    float       spread;

    if (!deathmatch->value)
        return;

    item = self->client->pers.weapon;
    if (! self->client->pers.inventory[self->client->ammo_index])
        item = NULL;
    if (item && (strcmp(item->pickup_name, "Blaster") == 0))
        item = NULL;

    if (!((int)(dmflags->value) & DF_QUAD_DROP))
        quad = qfalse;
    else
        quad = (self->client->quad_framenum > (level.framenum + 10));

    if (item && quad)
        spread = 22.5;
    else
        spread = 0.0;

    if (item) {
        self->client->v_angle[YAW] -= spread;
        drop = Drop_Item(self, item);
        self->client->v_angle[YAW] += spread;
        drop->spawnflags = DROPPED_PLAYER_ITEM;
    }

    if (quad) {
        self->client->v_angle[YAW] += spread;
        drop = Drop_Item(self, FindItemByClassname("item_quad"));
        self->client->v_angle[YAW] -= spread;
        drop->spawnflags |= DROPPED_PLAYER_ITEM;

        drop->touch = Touch_Item;
        drop->nextthink = level.time + (self->client->quad_framenum - level.framenum) * FRAMETIME;
        drop->think = G_FreeEdict;
    }
}


/*
==================
LookAtKiller
==================
*/
void LookAtKiller(edict_t *self, edict_t *inflictor, edict_t *attacker)
{
    vec3_t      dir;

    if (attacker && attacker != world && attacker != self) {
        VectorSubtract(attacker->s.origin, self->s.origin, dir);
    } else if (inflictor && inflictor != world && inflictor != self) {
        VectorSubtract(inflictor->s.origin, self->s.origin, dir);
    } else {
        self->client->killer_yaw = self->s.angles[YAW];
        return;
    }

    if (dir[0])
        self->client->killer_yaw = 180 / M_PI * atan2(dir[1], dir[0]);
    else {
        self->client->killer_yaw = 0;
        if (dir[1] > 0)
            self->client->killer_yaw = 90;
        else if (dir[1] < 0)
            self->client->killer_yaw = -90;
    }
    if (self->client->killer_yaw < 0)
        self->client->killer_yaw += 360;


}

/*
==================
player_die
==================
*/
void player_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
    int     n;

    VectorClear(self->avelocity);

    self->takedamage = DAMAGE_YES;
    self->movetype = MOVETYPE_TOSS;

    self->s.modelindex2 = 0;    // remove linked weapon model
	self->s.modelindex4 = 0;    // remove health banner if still there

	self->client->health_frame = 0;

    self->s.angles[0] = 0;
    self->s.angles[2] = 0;

    self->s.sound = 0;
    self->client->weapon_sound = 0;

    self->maxs[2] = -8;

//  self->solid = SOLID_NOT;
    self->svflags |= SVF_DEADMONSTER;

    if (!self->deadflag) {
        self->client->respawn_time = level.time + 1.0;
        LookAtKiller(self, inflictor, attacker);
        self->client->ps.pmove.pm_type = PM_DEAD;
        ClientObituary(self, inflictor, attacker);
        TossClientWeapon(self);
        if (deathmatch->value)
            Cmd_Help_f(self);       // show scores

        // clear inventory
        // this is kind of ugly, but it's how we want to handle keys in coop
        for (n = 0; n < game.num_items; n++) {
            if (coop->value && itemlist[n].flags & IT_KEY)
                self->client->resp.coop_respawn.inventory[n] = self->client->pers.inventory[n];
            self->client->pers.inventory[n] = 0;
        }
    }

    // remove powerups
    self->client->quad_framenum = 0;
    self->client->invincible_framenum = 0;
    self->client->breather_framenum = 0;
    self->client->enviro_framenum = 0;
	self->client->berserk_framenum = 0;
	self->client->reaper_framenum = 0;
    self->flags &= ~FL_POWER_ARMOR;

    if (self->health < -40) {
        // gib
        gi.sound(self, CHAN_BODY, gi.soundindex("misc/udeath.wav"), 1, ATTN_NORM, 0);
        for (n = 0; n < 4; n++)
            ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
        ThrowClientHead(self, damage);

        self->takedamage = DAMAGE_NO;
    } else {
        // normal death
        if (!self->deadflag) {
            static int i;

            i = (i + 1) % 3;
            // start a death animation
            self->client->anim_priority = ANIM_DEATH;
            if (self->client->ps.pmove.pm_flags & PMF_DUCKED) {
                self->s.frame = FRAME_crdeath1 - 1;
                self->client->anim_end = FRAME_crdeath5;
            } else switch (i) {
                case 0:
                    self->s.frame = FRAME_death101 - 1;
                    self->client->anim_end = FRAME_death106;
                    break;
                case 1:
                    self->s.frame = FRAME_death201 - 1;
                    self->client->anim_end = FRAME_death206;
                    break;
                case 2:
                    self->s.frame = FRAME_death301 - 1;
                    self->client->anim_end = FRAME_death308;
                    break;
                }
            gi.sound(self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand() % 4) + 1)), 1, ATTN_NORM, 0);
        }
    }

    self->deadflag = DEAD_DEAD;

    gi.linkentity(self);
}

//=======================================================================

/*
==============
InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
void InitClientPersistant(gclient_t *client)
{
	gitem_t     *item;

	memset(&client->pers, 0, sizeof(client->pers));

	item = FindItem("Blaster");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;

	client->pers.weapon = item;

	if (sv_flaregun->integer > 0)
	{
		// Q2RTX: Spawn with a flare gun and some grenades to use with it.
		// Flare gun is new and not found anywhere in the game as a pickup item.
		gitem_t* item_flareg = FindItem("Flare Gun");
		if (item_flareg)
		{
			client->pers.inventory[ITEM_INDEX(item_flareg)] = 1;

			if (sv_flaregun->integer == 2)
			{
				gitem_t* item_grenades = FindItem("Grenades");
				client->pers.inventory[ITEM_INDEX(item_grenades)] = 5;
			}
		}
	}

	if (coop->value)
	{
		item = FindItem("[Field Medic]");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;

		item = FindItem("[Technician]");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;

		item = FindItem("[Cyborg]");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;

		item = FindItem("[Grunt]");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;

		item = FindItem("[Assault]");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;
	}

    client->pers.health         = 100;
    client->pers.max_health     = 100;

	client->pers.pool_health = 0;
	client->pers.pool_health_max = 0;

    client->pers.max_bullets    = 200;
    client->pers.max_shells     = 100;
    client->pers.max_rockets    = 50;
    client->pers.max_grenades   = 50;
    client->pers.max_cells      = 200;
    client->pers.max_slugs      = 50;

    client->pers.connected = qtrue;
}


void InitClientResp(gclient_t *client)
{
    memset(&client->resp, 0, sizeof(client->resp));
    client->resp.enterframe = level.framenum;
    client->resp.coop_respawn = client->pers;
}

/*
==================
SaveClientData

Some information that should be persistant, like health,
is still stored in the edict structure, so it needs to
be mirrored out to the client structure before all the
edicts are wiped.
==================
*/
void SaveClientData(void)
{
    int     i;
    edict_t *ent;

    for (i = 0 ; i < game.maxclients ; i++) {
        ent = &g_edicts[1 + i];
        if (!ent->inuse)
            continue;
        game.clients[i].pers.health = ent->health;
        game.clients[i].pers.max_health = ent->max_health;
        game.clients[i].pers.savedFlags = (ent->flags & (FL_GODMODE | FL_NOTARGET | FL_POWER_ARMOR));
        if (coop->value)
            game.clients[i].pers.score = ent->client->resp.score;
    }
}

void FetchClientEntData(edict_t *ent)
{
    ent->health = ent->client->pers.health;
    ent->max_health = ent->client->pers.max_health;
    ent->flags |= ent->client->pers.savedFlags;
    if (coop->value)
        ent->client->resp.score = ent->client->pers.score;
}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float   PlayersRangeFromSpot(edict_t *spot)
{
    edict_t *player;
    float   bestplayerdistance;
    vec3_t  v;
    int     n;
    float   playerdistance;


    bestplayerdistance = 9999999;

    for (n = 1; n <= maxclients->value; n++) {
        player = &g_edicts[n];

        if (!player->inuse)
            continue;

        if (player->health <= 0)
            continue;

        VectorSubtract(spot->s.origin, player->s.origin, v);
        playerdistance = VectorLength(v);

        if (playerdistance < bestplayerdistance)
            bestplayerdistance = playerdistance;
    }

    return bestplayerdistance;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
edict_t *SelectRandomDeathmatchSpawnPoint(void)
{
    edict_t *spot, *spot1, *spot2;
    int     count = 0;
    int     selection;
    float   range, range1, range2;

    spot = NULL;
    range1 = range2 = 99999;
    spot1 = spot2 = NULL;

    while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
        count++;
        range = PlayersRangeFromSpot(spot);
        if (range < range1) {
            range1 = range;
            spot1 = spot;
        } else if (range < range2) {
            range2 = range;
            spot2 = spot;
        }
    }

    if (!count)
        return NULL;

    if (count <= 2) {
        spot1 = spot2 = NULL;
    } else
        count -= 2;

    selection = rand() % count;

    spot = NULL;
    do {
        spot = G_Find(spot, FOFS(classname), "info_player_deathmatch");
        if (spot == spot1 || spot == spot2)
            selection++;
    } while (selection--);

    return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/
edict_t *SelectFarthestDeathmatchSpawnPoint(void)
{
    edict_t *bestspot;
    float   bestdistance, bestplayerdistance;
    edict_t *spot;


    spot = NULL;
    bestspot = NULL;
    bestdistance = 0;
    while ((spot = G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
        bestplayerdistance = PlayersRangeFromSpot(spot);

        if (bestplayerdistance > bestdistance) {
            bestspot = spot;
            bestdistance = bestplayerdistance;
        }
    }

    if (bestspot) {
        return bestspot;
    }

    // if there is a player just spawned on each and every start spot
    // we have no choice to turn one into a telefrag meltdown
    spot = G_Find(NULL, FOFS(classname), "info_player_deathmatch");

    return spot;
}

edict_t *SelectDeathmatchSpawnPoint(void)
{
    if ((int)(dmflags->value) & DF_SPAWN_FARTHEST)
        return SelectFarthestDeathmatchSpawnPoint();
    else
        return SelectRandomDeathmatchSpawnPoint();
}


edict_t *SelectCoopSpawnPoint(edict_t *ent)
{
    int     index;
    edict_t *spot = NULL;
    char    *target;

    index = ent->client - game.clients;

    // player 0 starts in normal player spawn point
    if (!index)
        return NULL;

    spot = NULL;

    // assume there are four coop spots at each spawnpoint
    while (1) {
        spot = G_Find(spot, FOFS(classname), "info_player_coop");
        if (!spot)
            return NULL;    // we didn't have enough...

        target = spot->targetname;
        if (!target)
            target = "";
        if (Q_stricmp(game.spawnpoint, target) == 0) {
            // this is a coop spawn point for one of the clients here
            index--;
            if (!index)
                return spot;        // this is it
        }
    }


    return spot;
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
void    SelectSpawnPoint(edict_t *ent, vec3_t origin, vec3_t angles)
{
    edict_t		*spot = NULL;
	trace_t		tr;
	vec3_t		start, end, mins, maxs;
	qboolean	adjust = qtrue;

	if (deathmatch->value)
	{
		spot = SelectDeathmatchSpawnPoint();
	}
	else if (coop->value)
	{
		// Rroff this would be better handled with a small array of previous flares
		VectorSet(mins, -16, -16, -24);
		VectorSet(maxs, 16, 16, 32);

		if (ent->client->resp.flare)
		{
			VectorCopy(ent->client->resp.flare->s.origin, start);
			VectorCopy(ent->client->resp.flare->s.origin, end);
			end[2] += 33;

			tr = gi.trace(start, mins, maxs, end, ent, MASK_PLAYERSOLID);

			if (!tr.allsolid)
			{
				//ent->client->resp.flare->delay = 1;
				//ent->client->resp.flare->nextthink = level.time + 4;
				spot = ent->client->resp.flare;
				VectorCopy(tr.endpos, origin);
				adjust = qfalse;
			}
		}

		if (!spot && ent->client->resp.oldflare)
		{
			VectorCopy(ent->client->resp.oldflare->s.origin, start);
			VectorCopy(ent->client->resp.oldflare->s.origin, end);
			end[2] += 33;

			tr = gi.trace(start, mins, maxs, end, ent, MASK_PLAYERSOLID);

			if (!tr.allsolid)
			{
				//ent->client->resp.flare->delay = 1;
				//ent->client->resp.flare->nextthink = level.time + 4;
				spot = ent->client->resp.oldflare;
				VectorCopy(tr.endpos, origin);
				adjust = qfalse;
			}
		}

		if (!spot)
			spot = SelectCoopSpawnPoint(ent);
	}

    // find a single player start spot
    if (!spot) {
        while ((spot = G_Find(spot, FOFS(classname), "info_player_start")) != NULL) {
            if (!game.spawnpoint[0] && !spot->targetname)
                break;

            if (!game.spawnpoint[0] || !spot->targetname)
                continue;

            if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
                break;
        }

        if (!spot) {
            if (!game.spawnpoint[0]) {
                // there wasn't a spawnpoint without a target, so use any
                spot = G_Find(spot, FOFS(classname), "info_player_start");
            }
            if (!spot)
                gi.error("Couldn't find spawn point %s", game.spawnpoint);
        }
    }

	if (adjust)
	{
		VectorCopy(spot->s.origin, origin);
		origin[2] += 9;
	}
    VectorCopy(spot->s.angles, angles);
}

//======================================================================


void InitBodyQue(void)
{
    int     i;
    edict_t *ent;

    level.body_que = 0;
    for (i = 0; i < BODY_QUEUE_SIZE ; i++) {
        ent = G_Spawn();
        ent->classname = "bodyque";
    }
}

void body_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
    int n;

    if (self->health < -40) {
        gi.sound(self, CHAN_BODY, gi.soundindex("misc/udeath.wav"), 1, ATTN_NORM, 0);
        for (n = 0; n < 4; n++)
            ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
        self->s.origin[2] -= 48;
        ThrowClientHead(self, damage);
        self->takedamage = DAMAGE_NO;
    }
}

void CopyToBodyQue(edict_t *ent)
{
    edict_t     *body;

    gi.unlinkentity(ent);

    // grab a body que and cycle to the next one
    body = &g_edicts[game.maxclients + level.body_que + 1];
    level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

    // send an effect on the removed body
    if (body->s.modelindex) {
        gi.WriteByte(svc_temp_entity);
        gi.WriteByte(TE_BLOOD);
        gi.WritePosition(body->s.origin);
        gi.WriteDir(vec3_origin);
        gi.multicast(body->s.origin, MULTICAST_PVS);
    }

    gi.unlinkentity(body);
    body->s = ent->s;
    body->s.number = body - g_edicts;
    body->s.event = EV_OTHER_TELEPORT;

    body->svflags = ent->svflags;
    VectorCopy(ent->mins, body->mins);
    VectorCopy(ent->maxs, body->maxs);
    VectorCopy(ent->absmin, body->absmin);
    VectorCopy(ent->absmax, body->absmax);
    VectorCopy(ent->size, body->size);
    VectorCopy(ent->velocity, body->velocity);
    VectorCopy(ent->avelocity, body->avelocity);
    body->solid = ent->solid;
    body->clipmask = ent->clipmask;
    body->owner = ent->owner;
    body->movetype = ent->movetype;
    body->groundentity = ent->groundentity;

    body->die = body_die;
    body->takedamage = DAMAGE_YES;

    gi.linkentity(body);
}

void respawn(edict_t *self)
{
    if (deathmatch->value || coop->value) {
        // spectator's don't leave bodies
        if (self->movetype != MOVETYPE_NOCLIP)
            CopyToBodyQue(self);
        self->svflags &= ~SVF_NOCLIENT;
        PutClientInServer(self);

        // add a teleportation effect
        self->s.event = EV_PLAYER_TELEPORT;

        // hold in place briefly
        self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
        self->client->ps.pmove.pm_time = 14;

        self->client->respawn_time = level.time;

		if (self->client->pers.player_class & PC_CYBORG)
		{
			self->flags |= FL_POWER_ARMOR;
		}

        return;
    }

    // restart the entire server
    gi.AddCommandString("pushmenu loadgame\n");
}

/*
 * only called when pers.spectator changes
 * note that resp.spectator should be the opposite of pers.spectator here
 */
void spectator_respawn(edict_t *ent)
{
    int i, numspec;

    // if the user wants to become a spectator, make sure he doesn't
    // exceed max_spectators

    if (ent->client->pers.spectator) {
        char *value = Info_ValueForKey(ent->client->pers.userinfo, "spectator");
        if (*spectator_password->string &&
            strcmp(spectator_password->string, "none") &&
            strcmp(spectator_password->string, value)) {
            gi.cprintf(ent, PRINT_HIGH, "Spectator password incorrect.\n");
            ent->client->pers.spectator = qfalse;
            gi.WriteByte(svc_stufftext);
            gi.WriteString("spectator 0\n");
            gi.unicast(ent, qtrue);
            return;
        }

        // count spectators
        for (i = 1, numspec = 0; i <= maxclients->value; i++)
            if (g_edicts[i].inuse && g_edicts[i].client->pers.spectator)
                numspec++;

        if (numspec >= maxspectators->value) {
            gi.cprintf(ent, PRINT_HIGH, "Server spectator limit is full.");
            ent->client->pers.spectator = qfalse;
            // reset his spectator var
            gi.WriteByte(svc_stufftext);
            gi.WriteString("spectator 0\n");
            gi.unicast(ent, qtrue);
            return;
        }
    } else {
        // he was a spectator and wants to join the game
        // he must have the right password
        char *value = Info_ValueForKey(ent->client->pers.userinfo, "password");
        if (*password->string && strcmp(password->string, "none") &&
            strcmp(password->string, value)) {
            gi.cprintf(ent, PRINT_HIGH, "Password incorrect.\n");
            ent->client->pers.spectator = qtrue;
            gi.WriteByte(svc_stufftext);
            gi.WriteString("spectator 1\n");
            gi.unicast(ent, qtrue);
            return;
        }
    }

    // clear client on respawn
    ent->client->resp.score = ent->client->pers.score = 0;

    ent->svflags &= ~SVF_NOCLIENT;
    PutClientInServer(ent);

    // add a teleportation effect
    if (!ent->client->pers.spectator)  {
        // send effect
        gi.WriteByte(svc_muzzleflash);
        gi.WriteShort(ent - g_edicts);
        gi.WriteByte(MZ_LOGIN);
        gi.multicast(ent->s.origin, MULTICAST_PVS);

        // hold in place briefly
        ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
        ent->client->ps.pmove.pm_time = 14;
    }

    ent->client->respawn_time = level.time;

    if (ent->client->pers.spectator)
        gi.bprintf(PRINT_HIGH, "%s has moved to the sidelines\n", ent->client->pers.netname);
    else
        gi.bprintf(PRINT_HIGH, "%s joined the game\n", ent->client->pers.netname);
}

//==============================================================


/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
void PutClientInServer(edict_t *ent)
{
    vec3_t  mins = { -16, -16, -24};
    vec3_t  maxs = {16, 16, 32};
    int     index;
    vec3_t  spawn_origin, spawn_angles;
    gclient_t   *client;
    int     i;
    client_persistant_t saved;
    client_respawn_t    resp;

    // find a spawn point
    // do it before setting health back up, so farthest
    // ranging doesn't count this client
    SelectSpawnPoint(ent, spawn_origin, spawn_angles);

    index = ent - g_edicts - 1;
    client = ent->client;

    // deathmatch wipes most client data every spawn
    if (deathmatch->value) {
        char        userinfo[MAX_INFO_STRING];

        resp = client->resp;
        memcpy(userinfo, client->pers.userinfo, sizeof(userinfo));
        InitClientPersistant(client);
        ClientUserinfoChanged(ent, userinfo);
    } else {
//      int         n;
        char        userinfo[MAX_INFO_STRING];

        resp = client->resp;
        memcpy(userinfo, client->pers.userinfo, sizeof(userinfo));
        // this is kind of ugly, but it's how we want to handle keys in coop
//      for (n = 0; n < game.num_items; n++)
//      {
//          if (itemlist[n].flags & IT_KEY)
//              resp.coop_respawn.inventory[n] = client->pers.inventory[n];
//      }
        resp.coop_respawn.game_helpchanged = client->pers.game_helpchanged;
        resp.coop_respawn.helpchanged = client->pers.helpchanged;
        client->pers = resp.coop_respawn;
        ClientUserinfoChanged(ent, userinfo);
        if (resp.score > client->pers.score)
            client->pers.score = resp.score;
    } 

    // clear everything but the persistant data
    saved = client->pers;
    memset(client, 0, sizeof(*client));
    client->pers = saved;
	if (client->pers.health <= 0)
	{
		InitClientPersistant(client);
	}
    client->resp = resp;

    // copy some data from the client to the entity
    FetchClientEntData(ent);

    // clear entity values
    ent->groundentity = NULL;
    ent->client = &game.clients[index];
    ent->takedamage = DAMAGE_AIM;
    ent->movetype = MOVETYPE_WALK;
    ent->viewheight = 22;
    ent->inuse = qtrue;
    ent->classname = "player";
    ent->mass = 200;
    ent->solid = SOLID_BBOX;
    ent->deadflag = DEAD_NO;
    ent->air_finished = level.time + 12;
    ent->clipmask = MASK_PLAYERSOLID;
    ent->model = "players/male/tris.md2";
    ent->pain = player_pain;
    ent->die = player_die;
    ent->waterlevel = 0;
    ent->watertype = 0;
    ent->flags &= ~FL_NO_KNOCKBACK;
    ent->svflags &= ~SVF_DEADMONSTER;

    VectorCopy(mins, ent->mins);
    VectorCopy(maxs, ent->maxs);
    VectorClear(ent->velocity);

    // clear playerstate values
    memset(&ent->client->ps, 0, sizeof(client->ps));

    client->ps.pmove.origin[0] = spawn_origin[0] * 8;
    client->ps.pmove.origin[1] = spawn_origin[1] * 8;
    client->ps.pmove.origin[2] = spawn_origin[2] * 8;

    if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV)) {
        client->ps.fov = 90;
    } else {
        client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
        if (client->ps.fov < 1)
            client->ps.fov = 90;
        else if (client->ps.fov > 160)
            client->ps.fov = 160;
    }

    client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

    // clear entity state values
    ent->s.effects = 0;
    ent->s.modelindex = 255;        // will use the skin specified model
    ent->s.modelindex2 = 255;       // custom gun model
    // sknum is player num and weapon number
    // weapon number will be added in changeweapon
    ent->s.skinnum = ent - g_edicts - 1;

	if (!g_healthbars->value)
	{
		client->health_frame = 0;
		ent->s.modelindex4 = 0;
	} else {
		client->health_frame = 10;
		ent->s.modelindex4 = gi.modelindex("sprites/health10.sp2");
	}

	// Rroff make sure we have something defined for this
	// we can probably remove other entries unless we change it

	ent->client->ps.stats[STAT_PICKUP_BACK] = gi.imageindex("tag3");

    ent->s.frame = 0;
    VectorCopy(spawn_origin, ent->s.origin);
    ent->s.origin[2] += 1;  // make sure off ground
    VectorCopy(ent->s.origin, ent->s.old_origin);

    // set the delta angle
    for (i = 0 ; i < 3 ; i++) {
        client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);
    }

    ent->s.angles[PITCH] = 0;
    ent->s.angles[YAW] = spawn_angles[YAW];
    ent->s.angles[ROLL] = 0;
    VectorCopy(ent->s.angles, client->ps.viewangles);
    VectorCopy(ent->s.angles, client->v_angle);

    // spawn a spectator
    if (client->pers.spectator) {
        client->chase_target = NULL;

        client->resp.spectator = qtrue;

        ent->movetype = MOVETYPE_NOCLIP;
        ent->solid = SOLID_NOT;
        ent->svflags |= SVF_NOCLIENT;
        ent->client->ps.gunindex = 0;
        gi.linkentity(ent);
        return;
    } else
        client->resp.spectator = qfalse;

    if (!KillBox(ent)) {
        // could't spawn in?
    }

    gi.linkentity(ent);

    // force the current weapon up
    client->newweapon = client->pers.weapon;
    ChangeWeapon(ent);
}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in
deathmatch mode, so clear everything out before starting them.
=====================
*/
void ClientBeginDeathmatch(edict_t *ent)
{
    G_InitEdict(ent);

    InitClientResp(ent->client);

    // locate ent at a spawn point
    PutClientInServer(ent);

    if (level.intermissiontime) {
        MoveClientToIntermission(ent);
    } else {
        // send effect
        gi.WriteByte(svc_muzzleflash);
        gi.WriteShort(ent - g_edicts);
        gi.WriteByte(MZ_LOGIN);
        gi.multicast(ent->s.origin, MULTICAST_PVS);
    }

    gi.bprintf(PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);

    // make sure all view stuff is valid
    ClientEndServerFrame(ent);
}


/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void ClientBegin(edict_t *ent)
{
    int     i;

    ent->client = game.clients + (ent - g_edicts - 1);

    if (deathmatch->value) {
        ClientBeginDeathmatch(ent);
        return;
    }

    // if there is already a body waiting for us (a loadgame), just
    // take it, otherwise spawn one from scratch
    if (ent->inuse == qtrue) {
        // the client has cleared the client side viewangles upon
        // connecting to the server, which is different than the
        // state when the game is saved, so we need to compensate
        // with deltaangles
        for (i = 0 ; i < 3 ; i++)
            ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->ps.viewangles[i]);
    } else {
        // a spawn point will completely reinitialize the entity
        // except for the persistant data that was initialized at
        // ClientConnect() time
        G_InitEdict(ent);
        ent->classname = "player";
        InitClientResp(ent->client);

		// Rroff on map change clear out a coop player's save flare spawn points
		// should be done above

        PutClientInServer(ent);
    }

    if (level.intermissiontime) {
        MoveClientToIntermission(ent);
    } else {
        // send effect if in a multiplayer game
        if (game.maxclients > 1) {
            gi.WriteByte(svc_muzzleflash);
            gi.WriteShort(ent - g_edicts);
            gi.WriteByte(MZ_LOGIN);
            gi.multicast(ent->s.origin, MULTICAST_PVS);

            gi.bprintf(PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
        }
    }

    // make sure all view stuff is valid
    ClientEndServerFrame(ent);
}

/*
===========
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
void ClientUserinfoChanged(edict_t *ent, char *userinfo)
{
    char    *s;
    int     playernum;

    // check for malformed or illegal info strings
    if (!Info_Validate(userinfo)) {
        strcpy(userinfo, "\\name\\badinfo\\skin\\male/grunt");
    }

    // set name
    s = Info_ValueForKey(userinfo, "name");
    strncpy(ent->client->pers.netname, s, sizeof(ent->client->pers.netname) - 1);

    // set spectator
    s = Info_ValueForKey(userinfo, "spectator");
    // spectators are only supported in deathmatch
	// Rroff coop test
    //if (deathmatch->value && *s && strcmp(s, "0"))
	if ((deathmatch->value || (coop->value && g_coopflags->value)) && *s && strcmp(s, "0"))
        ent->client->pers.spectator = qtrue;
    else
        ent->client->pers.spectator = qfalse;

    // set skin
    s = Info_ValueForKey(userinfo, "skin");

    playernum = ent - g_edicts - 1;

    // combine name and skin into a configstring
    gi.configstring(CS_PLAYERSKINS + playernum, va("%s\\%s", ent->client->pers.netname, s));

    // fov
    if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV)) {
        ent->client->ps.fov = 90;
    } else {
        ent->client->ps.fov = atoi(Info_ValueForKey(userinfo, "fov"));
        if (ent->client->ps.fov < 1)
            ent->client->ps.fov = 90;
        else if (ent->client->ps.fov > 160)
            ent->client->ps.fov = 160;
    }

    // handedness
    s = Info_ValueForKey(userinfo, "hand");
    if (strlen(s)) {
        ent->client->pers.hand = atoi(s);
    }

    // save off the userinfo in case we want to check something later
    strncpy(ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo) - 1);
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning qfalse.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again, but
loadgames will.
============
*/
qboolean ClientConnect(edict_t *ent, char *userinfo)
{
    char    *value;

    // check to see if they are on the banned IP list
    value = Info_ValueForKey(userinfo, "ip");
    if (SV_FilterPacket(value)) {
        Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
        return qfalse;
    }

    // check for a spectator
    value = Info_ValueForKey(userinfo, "spectator");
	// Rroff coop test
    //if (deathmatch->value && *value && strcmp(value, "0")) {
	if ((deathmatch->value || (coop->value && g_coopflags->value)) && *value && strcmp(value, "0")) {
        int i, numspec;

        if (*spectator_password->string &&
            strcmp(spectator_password->string, "none") &&
            strcmp(spectator_password->string, value)) {
            Info_SetValueForKey(userinfo, "rejmsg", "Spectator password required or incorrect.");
            return qfalse;
        }

        // count spectators
        for (i = numspec = 0; i < maxclients->value; i++)
            if (g_edicts[i + 1].inuse && g_edicts[i + 1].client->pers.spectator)
                numspec++;

        if (numspec >= maxspectators->value) {
            Info_SetValueForKey(userinfo, "rejmsg", "Server spectator limit is full.");
            return qfalse;
        }
    } else {
        // check for a password
        value = Info_ValueForKey(userinfo, "password");
        if (*password->string && strcmp(password->string, "none") &&
            strcmp(password->string, value)) {
            Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
            return qfalse;
        }
    }

	// Rroff - a bit crude way to stop people reconnecting to work around the team wipe check
	// need to implement something better later

	if (coop->value && level.lockdown_ent && ((level.lockdown_ent->moded == LK_FOREVER) || (level.lockdown_ent->moded == LK_CUSTOM)))
	{
		if ((level.lockdown_ent->delay == 0) && !(level.intermissiontime))
		{
			Info_SetValueForKey(userinfo, "rejmsg", "Can't join with horde wave already in progress");
			return qfalse;
		}
	}


    // they can connect
    ent->client = game.clients + (ent - g_edicts - 1);

    // if there is already a body waiting for us (a loadgame), just
    // take it, otherwise spawn one from scratch
    if (ent->inuse == qfalse) {
        // clear the respawning variables
        InitClientResp(ent->client);
        if (!game.autosaved || !ent->client->pers.weapon)
            InitClientPersistant(ent->client);
    }

    ClientUserinfoChanged(ent, userinfo);

    if (game.maxclients > 1)
        gi.dprintf("%s connected\n", ent->client->pers.netname);

    ent->svflags = 0; // make sure we start with known default
    ent->client->pers.connected = qtrue;
    return qtrue;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void ClientDisconnect(edict_t *ent)
{
    //int     playernum;
	int				index;
	gitem_t			*item;
	edict_t			*dropped;

    if (!ent->client)
        return;

    gi.bprintf(PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);

    // send effect
    if (ent->inuse) {
        gi.WriteByte(svc_muzzleflash);
        gi.WriteShort(ent - g_edicts);
        gi.WriteByte(MZ_LOGOUT);
        gi.multicast(ent->s.origin, MULTICAST_PVS);
    }

	// Rroff - this is a bit of a horrid way to do it - probably should tag these with an IT_ entry
	// and loop through the client inventory
	// will also need to pass the quantity (FIXME: Make security tags multiple pickup capable)

	item = FindItem("Security Tag");
	if (item) {
		index = ITEM_INDEX(item);
		if (ent->client->pers.inventory[index]) {
			dropped = Drop_Item(ent, item);
			dropped->count = ent->client->pers.inventory[index];
		}
	}

	item = FindItem("Strogg Head");
	if (item) {
		index = ITEM_INDEX(item);
		if (ent->client->pers.inventory[index]) {
			dropped = Drop_Item(ent, item);
			dropped->count = ent->client->pers.inventory[index];
		}
	}

    gi.unlinkentity(ent);
    ent->s.modelindex = 0;
    ent->s.sound = 0;
    ent->s.event = 0;
    ent->s.effects = 0;
    ent->solid = SOLID_NOT;
    ent->inuse = qfalse;
    ent->classname = "disconnected";
    ent->client->pers.connected = qfalse;

    // FIXME: don't break skins on corpses, etc
    //playernum = ent-g_edicts-1;
    //gi.configstring (CS_PLAYERSKINS+playernum, "");
}


//==============================================================


edict_t *pm_passent;

// pmove doesn't need to know about passent and contentmask
trace_t q_gameabi PM_trace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
    if (pm_passent->health > 0)
        return gi.trace(start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
    else
        return gi.trace(start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

unsigned CheckBlock(void *b, int c)
{
    int v, i;
    v = 0;
    for (i = 0 ; i < c ; i++)
        v += ((byte *)b)[i];
    return v;
}
void PrintPmove(pmove_t *pm)
{
    unsigned    c1, c2;

    c1 = CheckBlock(&pm->s, sizeof(pm->s));
    c2 = CheckBlock(&pm->cmd, sizeof(pm->cmd));
    Com_Printf("sv %3i:%i %i\n", pm->cmd.impulse, c1, c2);
}

void ClientPick(edict_t *ent, vec3_t forward)
{
	vec3_t		start;
	vec3_t		end;
	trace_t		tr;
	float		*v;

	// raise the line pick upto players eye height for better accuracy (player weapons do this -8 for gun pos)

	// valid picks only lasts 1 second if its not updated
	if (level.time > ent->client->picked_time + 1)
	{
		ent->client->picked_ent = NULL;
	}

	v = tv(0, 0, ent->viewheight);
	VectorAdd(ent->s.origin, v, start);

	VectorMA(start, 768, forward, end); // may want to increase distance

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

	if ((tr.ent) && (tr.fraction < 1.0))
	{
		if (((tr.ent->client) || (tr.ent->svflags & SVF_MONSTER)) && (tr.ent->health > 0))
		{
			ent->client->picked_ent = tr.ent;
			ent->client->picked_time = level.time;
		}
	}
}

void ClientShove(edict_t *ent, vec3_t forward)
{
	vec3_t		start;
	vec3_t		end;
	trace_t		tr;
	float		*v;
	vec3_t		shove;
	qboolean	doShove = qfalse;

	v = tv(0, 0, ent->viewheight);
	VectorAdd(ent->s.origin, v, start);

	VectorMA(start, 65, forward, end); // was 64

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

	if ((tr.ent) && (tr.fraction < 1.0)) // working around some issues/bugs?
	{
		if ((tr.ent->svflags & SVF_MONSTER) && (tr.ent->health > 0))
		{
			if (!(ent->client->weaponstate == WEAPON_FIRING))
			{
				if ((tr.ent->mass <= 400) && (!(tr.ent->monsterinfo.aiflags & AI_GOOD_GUY))) // we aren't superman :P
				{
					VectorCopy(forward, shove);
					VectorNormalize(shove);
					shove[2] += 0.75;

					VectorMA(tr.ent->velocity, 250, shove, tr.ent->velocity);

					if (tr.ent->velocity[2] > 0)
					{
						tr.ent->groundentity = NULL;
					}

					if (tr.ent->pain)
						tr.ent->pain(tr.ent, ent, 250, 50);
				}
			}
		}
	}
}

// Rroff: Utility function for a client using something
// trace forwards 64 and try to use the playeruse function on an entity

void ClientUse(edict_t *ent, vec3_t forward)
{
	vec3_t		start;
	vec3_t		end;
	trace_t		tr;
	float		*v;
	vec3_t		shove;
	qboolean	doShove = qfalse;

	// raise the line pick upto players eye height for better accuracy (player weapons do this -8 for gun pos)

	v = tv(0, 0, ent->viewheight);
	VectorAdd(ent->s.origin, v, start);

	//gi.sound (ent, CHAN_VOICE, gi.soundindex ("misc/talk.wav"), 1, ATTN_STATIC, 0);

	VectorMA(start, 65, forward, end); // was 64

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

	// Rroff this is a bit backwards - we should really check other use cases and if nothing found
	// go into a generic shove animation then check if we hit anything in a couple of frames

	if ((tr.ent) && (tr.fraction < 1.0)) // working around some issues/bugs?
	{
		if ((tr.ent->svflags & SVF_MONSTER) && (tr.ent->health > 0))
		{
			if (!(ent->client->weaponstate == WEAPON_FIRING))
			{
				if ((tr.ent->mass <= 400) && (!(tr.ent->monsterinfo.aiflags & AI_GOOD_GUY))) // we aren't superman :P
				{
					//if ((!(ent->client->ps.pmove.pm_flags & PMF_DUCKED)) && (!(ent->client->anim_priority > ANIM_WAVE)))
					//{
					//	ent->client->anim_priority = ANIM_WAVE;
					//	//ent->s.frame = FRAME_point01-1;
					//	ent->s.frame = FRAME_flip01;
					//	ent->client->anim_end = FRAME_flip04;
					//}

					if (!(ent->client->anim_priority > ANIM_WAVE))
					{
						if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
						{
							ent->client->anim_priority = ANIM_WAVE;
							ent->s.frame = FRAME_crattak1;
							ent->client->anim_end = FRAME_crattak3;
						} else {
							ent->client->anim_priority = ANIM_WAVE;
							ent->s.frame = FRAME_flip01-1;
							ent->client->anim_end = FRAME_flip04;
						}
					}

					// Rroff instead of shoving immediately do the animation
					// and delay a couple of frames - might not always hit the same target if things are happening quickly

					/*VectorCopy(forward, shove);
					VectorNormalize(shove);
					shove[2] += 0.75;

					VectorMA(tr.ent->velocity, 250, shove, tr.ent->velocity);

					if (tr.ent->velocity[2] > 0)
					{
						tr.ent->groundentity = NULL;
					}

					if (tr.ent->pain)
						tr.ent->pain(tr.ent, ent, 250, 50);
						*/

					doShove = qtrue;
					gi.sound(ent, CHAN_VOICE, gi.soundindex("*fall2.wav"), 1, ATTN_NORM, 0);
					PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
					ent->client->doShove = qtrue;

					ent->client->weaponstate = WEAPON_ACTIVATING;
					ent->client->ps.gunframe = 0;

					// check if the player action here should result in the player
					// losing cloak/invisiblity, etc. if such was implemented
					//if (ent->cloak)
					//	Use_Cloak_Deactivate(ent);

				}
			}
		}

		if (tr.ent->playeruse)
		{
			if (!(doShove))
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/ar2_pkup.wav"), 1, ATTN_STATIC, 0);
			}

			tr.ent->playeruse(tr.ent, ent); // self is the object, ent is the activating object
		} else {
			if (!(doShove))
				gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/talk.wav"), 1, ATTN_STATIC, 0);
		}
	}
}

// This is quite slow for something activated frequently
// could speed it up by tracking relevant projectiles
// in an index

qboolean doplayerad(edict_t *self)
{
	edict_t		*ent = NULL;
	vec3_t		offset, start, forward, right, end, v;
	float		dist;
	int			c = 0;

	while ((ent = findradius(ent, self->s.origin, 700)) != NULL) {
		if (c > 2)
			break;
		if (!ent->projectile)
			continue;
		if (level.time < ent->timestamp + 0.1)
			continue;
		if (!visible(self, ent))
			continue;
		//if (!infront(self, ent))
		//	continue;
		if (!ent->owner)
			continue;
		if (ent->owner->client)
			continue;
		if (!(ent->owner->svflags & SVF_MONSTER))
			continue;
		if (ent)
		{
			c++;
			VectorCopy(self->s.origin, start);

			VectorSubtract(ent->s.origin, self->s.origin, v);

			dist = VectorLength(v);

			vectoangles(v, v);
			AngleVectors(v, forward, NULL, NULL);

			VectorMA(start, 16, forward, start);

			if (dist > 30)
				VectorMA(self->s.origin, dist - 16, forward, end);
			else
				VectorCopy(ent->s.origin, end);

			// as an entity can only have one beam we need to use the projectile
			// not the player as the beam source and make sure start and end are
			// the right (wrong) way around

			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_LIGHTNING);
			gi.WriteShort(ent - g_edicts);
			gi.WriteShort(self - g_edicts);
			gi.WritePosition(end);
			gi.WritePosition(start);
			gi.multicast(start, MULTICAST_PVS);

			gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/bfg__x1b.wav"), 1, ATTN_NORM, 0);

			BecomeExplosion1(ent);
		}
	}

	if (c > 0)
		return qtrue;

	return qfalse;
}

void domonster(edict_t *ent)
{
	vec3_t		offset, forward, right, start, end;
	vec3_t		mins, maxs;
	trace_t		tr;
	edict_t		*temp;

	// We probably should filter what monsters a player can control
	// to try and stop them interfering with game critical scripted events

	if (ent->client->chase_target->health <= 0)
		return;

	AngleVectors(ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 0, ent->viewheight);
	G_ProjectSource(ent->s.origin, offset, forward, right, start);
	VectorMA(start, 1024, forward, end);

	tr = gi.trace(start, NULL, NULL, end, ent->client->chase_target, MASK_SHOT);

	if (tr.fraction == 1.0)
		return;

	if (ent->client->chase_monster_target && ent->client->chase_monster_target->inuse)
		G_FreeEdict(ent->client->chase_monster_target);

	if ((tr.ent->client || tr.ent->svflags & SVF_MONSTER || tr.ent->takedamage) && tr.ent->health > 0)
	{
		ent->client->chase_target->enemy = tr.ent;
		ent->client->chase_target->monsterinfo.aiflags &= ~(AI_RUNAWAY | AI_ASSUMED);
		FoundTarget(ent->client->chase_target);
		return;
	}

	temp = G_Spawn();
	temp->svflags |= SVF_NOCLIENT; // not really needed
	temp->think = G_FreeEdict;
	temp->nextthink = level.time + 30;
	VectorCopy(tr.endpos, temp->s.origin);

	ent->client->chase_monster_target = temp;

	ent->client->chase_target->oldenemy = ent->client->chase_target->enemy;
	ent->client->chase_target->goalentity = ent->client->chase_target->enemy = temp;
	ent->client->chase_target->monsterinfo.run(ent->client->chase_target);
	ent->client->chase_target->monsterinfo.aiflags |= (AI_RUNAWAY | AI_ASSUMED);
	ent->client->chase_target->monsterinfo.runaway_time = level.time;

}

void pickChaseTarget(edict_t *ent)
{
	vec3_t		offset, forward, right, start, end;
	vec3_t		mins, maxs;
	trace_t		tr;

	AngleVectors(ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 0, ent->viewheight);
	G_ProjectSource(ent->s.origin, offset, forward, right, start);
	VectorMA(start, 1024, forward, end);

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

	if (tr.fraction == 1.0)
		return;

	if (tr.ent->svflags & SVF_MONSTER && tr.ent->health > 0) {
		ent->client->chase_target = tr.ent;
		ent->client->update_chase = qtrue;
	}

}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
void ClientThink(edict_t *ent, usercmd_t *ucmd)
{
    gclient_t		*client;
    edict_t			*other;
    int				i, j, h, take, r;
    pmove_t			pm;
	vec3_t			start, forward, right, offset, end;

	qboolean		inAir = qfalse;
	float			dropRate;
	int				doImpact = 0;
	vec3_t			impactV;

	int				index;
	gitem_t			*fitem;

    level.current_entity = ent;
    client = ent->client;

    if (level.intermissiontime) {
        client->ps.pmove.pm_type = PM_FREEZE;
        // can exit intermission after five seconds
        if (level.time > level.intermissiontime + 5.0
            && (ucmd->buttons & BUTTON_ANY))
            level.exitintermission = qtrue;
        return;
    }

	if (!(ent->groundentity))
	{
		inAir = qtrue;
		dropRate = ent->velocity[2];
		VectorCopy(ent->velocity, impactV);
		VectorNormalize(impactV);
	}

    pm_passent = ent;

    if (ent->client->chase_target && !ent->client->chase_mode) {

        client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
        client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
        client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

    } else {

        // set up for pmove
        memset(&pm, 0, sizeof(pm));

        if (ent->movetype == MOVETYPE_NOCLIP)
            client->ps.pmove.pm_type = PM_SPECTATOR;
        else if (ent->s.modelindex != 255)
            client->ps.pmove.pm_type = PM_GIB;
        else if (ent->deadflag)
            client->ps.pmove.pm_type = PM_DEAD;
        else
            client->ps.pmove.pm_type = PM_NORMAL;

		client->ps.pmove.gravity = sv_gravity->value;

		if (client->pers.mods & MU_STIM)
		{
			if (level.time <= client->stim_time + 15)
				client->ps.pmove.gravity = (sv_gravity->value * 0.66);
		}

        pm.s = client->ps.pmove;

        for (i = 0 ; i < 3 ; i++) {
            pm.s.origin[i] = ent->s.origin[i] * 8;
            pm.s.velocity[i] = ent->velocity[i] * 8;
        }

        if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s))) {
            pm.snapinitial = qtrue;
            //      gi.dprintf ("pmove changed!\n");
        }

        pm.cmd = *ucmd;

        pm.trace = PM_trace;    // adds default parms
        pm.pointcontents = gi.pointcontents;

        // perform a pmove
        gi.Pmove(&pm);

        // save results of pmove
        client->ps.pmove = pm.s;
        client->old_pmove = pm.s;

        for (i = 0 ; i < 3 ; i++) {
            ent->s.origin[i] = pm.s.origin[i] * 0.125;
            ent->velocity[i] = pm.s.velocity[i] * 0.125;
        }

        VectorCopy(pm.mins, ent->mins);
        VectorCopy(pm.maxs, ent->maxs);

        client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
        client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
        client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

        if (ent->groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0)) {
			// Rroff with silencer jump doesn't alert enemy
			if (ent->client->silencer_shots)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("player/gasp2.wav"), 1, ATTN_NORM, 0);
			} else {
				gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
				PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
			}
        }

        ent->viewheight = pm.viewheight;
        ent->waterlevel = pm.waterlevel;
        ent->watertype = pm.watertype;
        ent->groundentity = pm.groundentity;
        if (pm.groundentity)
            ent->groundentity_linkcount = pm.groundentity->linkcount;

        if (ent->deadflag) {
            client->ps.viewangles[ROLL] = 40;
            client->ps.viewangles[PITCH] = -15;
            client->ps.viewangles[YAW] = client->killer_yaw;
        } else {
            VectorCopy(pm.viewangles, client->v_angle);
            VectorCopy(pm.viewangles, client->ps.viewangles);
        }

		if ((ent->groundentity) && (inAir))
		{
			// just landed
			if (dropRate < -299)
			//if (dropRate < -200)
			{
				doImpact = abs(dropRate) / 20;
				if (doImpact > 100)
					doImpact = 100;
				if (doImpact < 1)
					doImpact = 1;
			}
		}

        gi.linkentity(ent);

        if (ent->movetype != MOVETYPE_NOCLIP)
            G_TouchTriggers(ent);

        // touch other objects
        for (i = 0 ; i < pm.numtouch ; i++) {
            other = pm.touchents[i];
            for (j = 0 ; j < i ; j++)
                if (pm.touchents[j] == other)
                    break;
            if (j != i)
                continue;   // duplicated

			if ((other->svflags & SVF_MONSTER) && (other->health > 0) && (doImpact))
			{
				if (other->pain)
					other->pain(other, ent, 250, 50);

				// bit of a hack need to work out / normalise direction probably
				// and make sure some of the other numbers aren't invalid like kick
				// origin would actually be the point on the edge of the bounding box
				T_Damage(other, ent, ent, impactV, ent->s.origin, vec3_origin, doImpact, (abs(dropRate) - 299), DAMAGE_NO_KNOCKBACK, MOD_HIT);
			}

			// if we have landed on a monster or monsters do impact stuff here

            if (!other->touch)
                continue;
            other->touch(other, ent, NULL, NULL);
        }

    }

    client->oldbuttons = client->buttons;
    client->buttons = ucmd->buttons;
    client->latched_buttons |= client->buttons & ~client->oldbuttons;

    // save light level the player is standing on for
    // monster sighting AI
    ent->light_level = ucmd->lightlevel;

	// Rroff - pick forward every frame :s

	// not actually using this right now

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);

	if (ent->client->doShove)
	{
		if (ent->client->ps.gunframe >= 3)
		{
			ClientShove(ent, forward);
			ent->client->doShove = qfalse;
		}
	}

	// probably need to check times on this or something so it doesn't get delayed and the effect happen much later

	if (client->latched_buttons & BUTTON_ATTACK)
	{
		client->firedown = qtrue;
	}

	if (!client->buttons & BUTTON_ATTACK)
	{
		client->firedown = qfalse;
		client->firekillcount = 0;
	}
	

	if ((client->latched_buttons & BUTTON_USE) && (client->button_use_down == 0) && (level.time > client->button_use_time))
	{
		client->button_use_time = level.time + 0.25;
		client->button_use_down = 1;

		if (client->resp.spectator)
		{
			client->latched_buttons = 0;

			if (client->chase_mode)
			{
				client->chase_mode = qfalse;
			}
			else {
				if (client->chase_target) {
					client->chase_target = NULL;
					client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
				}
				else
					GetChaseTarget(ent);
			}
			// spectators can use something?
		} else {
			ClientUse(ent, forward);
		}
	}

	if ((!(client->buttons & BUTTON_USE)) && (client->button_use_down == 1))
	{
		client->button_use_down = 0;
		// do unuse actions here? :S
	}

	//ClientPick(ent, forward);

    // fire weapon from final position if needed
    if (client->latched_buttons & BUTTON_ATTACK) {
        if (client->resp.spectator) {
			client->latched_buttons = 0;

			// Rroff - change this in coop
			// as use will unlatch from current chase target
			// instead do actions based on whether we are following something or not
			// and use the point as an action for our monster

            /*client->latched_buttons = 0;

            if (client->chase_target) {
                client->chase_target = NULL;
                client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
            } else
                GetChaseTarget(ent);*/

			// first click unlatches view but keeps chase target
			if (client->chase_target && client->chase_target->health > 0) {
				if (!client->chase_mode)
				{
					client->chase_mode = qtrue;
					client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
				}
				else {
					// do action based on where looking
					if (coop->value)
						domonster(ent);
				}
			} else {
				if (coop->value)
					pickChaseTarget(ent);
			}

        } else if (!client->weapon_thunk) {
            client->weapon_thunk = qtrue;
            Think_Weapon(ent);
        }
    }

    if (client->resp.spectator) {
        if (ucmd->upmove >= 10) {
            if (!(client->ps.pmove.pm_flags & PMF_JUMP_HELD)) {
                client->ps.pmove.pm_flags |= PMF_JUMP_HELD;
                if (client->chase_target)
                    ChaseNext(ent);
                else
                    GetChaseTarget(ent);
            }
        } else
            client->ps.pmove.pm_flags &= ~PMF_JUMP_HELD;

		if (coop->value) {
			if (client->chase_target && !client->chase_mode)
				UpdateChaseCam(ent);
		}
    }

    // update chase cam if being followed
    for (i = 1; i <= maxclients->value; i++) {
        other = g_edicts + i;
        if (other->inuse && other->client->chase_target == ent)
            UpdateChaseCam(other);
    }

	// Rroff horrid hack for muzzle effects
	// would need to do this for the various different player models
	// also need one for crouched attack

	/*ent->s.modelindex3 = 0;
	if (ent->s.frame == FRAME_attack2)
	{
		r = rand() % 2;
		ent->s.modelindex3 = gi.modelindex(va("models/weapons/mzfx/male/flash%i.md2",r));
	}*/

	/*ent->s.modelindex3 = 0;

	if (ent->s.frame == FRAME_attack1 || ent->s.frame == FRAME_attack2)
	{
		ent->s.modelindex3 = gi.modelindex("models/weapons/mzfx/male/tris.md2");
	}*/

	/*if (ent->s.frame == FRAME_attack1 || ent->s.frame == FRAME_attack2)
	{
		if (ent->s.frame == FRAME_attack1)
			VectorSet(offset, 7, 9, 7);
			//VectorSet(offset, 24, 9, 7);
		else
			//VectorSet(offset, 24, 9, 13);
			VectorSet(offset, 6, 9, 13);
			// corrected 7, 7.4, 9.6 ???
		// 7, 7, 9
		// 6, 13, 9
		AngleVectors(ent->s.angles, forward, right, NULL);
		G_ProjectSource(ent->s.origin, offset, forward, right, start);

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_SHOTGUN);
		gi.WritePosition(start);
		gi.WriteDir(forward);
		gi.multicast(start, MULTICAST_PVS);

		/*gi.WriteByte(svc_muzzleflash);
		gi.WriteShort(self - g_edicts);
		gi.WriteByte(MZ_NUKE2);
		gi.multicast(self->s.origin, MULTICAST_PVS);*/
		/*
	}*/

	if ((g_ejectbrass->value == 2) && (ent->s.frame == FRAME_attack2))
	{
		VectorSet(offset, 6, 9, 13);
		AngleVectors(ent->s.angles, forward, right, NULL);
		G_ProjectSource(ent->s.origin, offset, forward, right, start);

	}

	// Rroff - health bars above players

	if (!g_healthbars->value) {
		if (ent->s.modelindex4 != 0)
			ent->s.modelindex4 = 0;
	} else {
		h = ((100 / (float)ent->max_health) * ent->health) / 10;
		if (h < 0)
			h = 0;

		if (h == 0 && ent->health > 0)
			h = 1;

		if (h > 10)
			h = 10;

		if (h != ent->client->health_frame)
		{
			if (h == 0)
			{
				ent->client->health_frame = h;
				ent->s.modelindex4 = 0;
			}
			else {
				ent->client->health_frame = h;
				ent->s.modelindex4 = gi.modelindex(va("sprites/health%i.sp2", h));
			}
		}
	}

	if (ent->health > 0)
	{
		if (ent->client->pers.mods & MU_ACTIVEDEF)
		{
			if (level.time > ent->client->ad_time)
			{
				fitem = FindItem("Cells");
				index = ITEM_INDEX(fitem);

				if (ent->client->pers.inventory[index] >= 3)
				{
					if (doplayerad(ent))
					{
						ent->client->pers.inventory[index]-=3;
						ent->client->ad_time = level.time + 3; // was 2
						ent->client->ad_last = level.time;
					} else {
						ent->client->ad_time = level.time + 0.5;
					}
					
				}
			}
		}

		if (((ent->client->pers.mods & MU_SOLAR) && (ent->light_level > 10) || (ent->client->pers.player_class & PC_CYBORG)) && (level.time > ent->client->solar_time + 2))
		{
			ent->client->solar_time = level.time;

			fitem = FindItem("Cells");
			index = ITEM_INDEX(fitem);

			// do a pickup if none exist?

			if (
				((ent->client->pers.mods & MU_SOLAR) && (ent->client->pers.inventory[index] < (ent->client->pers.max_cells * 0.25))) ||
				((ent->client->pers.player_class & PC_CYBORG) && (ent->client->pers.inventory[index] < 50))
					)
			{
				ent->client->pers.inventory[index]++;

				if (ent->client->pers.inventory[index] < 2) {
					ent->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(fitem->icon);
					ent->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS + index;
					ent->client->pickup_msg_time = level.time + 3.0;
				}
			}
		}

		if (ent->client->pers.mods & MU_REGEN || ent->client->pers.player_class & PC_CYBORG)
		{
			if (level.time > ent->client->healthtick_time + 1)
			{
				ent->client->healthtick_time = level.time;

				if ((ent->client->pers.player_class & PC_CYBORG) && (ent->health < (ent->max_health * 0.3)))
				{
					//ent->health += 4;
					ent->health += 2;
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/s_health.wav"), 0.75, ATTN_NORM, 0);
				} else {
					if (ent->health < ent->max_health && ent->client->pers.pool_health > 0)
					{
						take = ent->max_health - ent->health;
						if (take > 5)
							take = 5;
						if (take > ent->client->pers.pool_health)
							take = ent->client->pers.pool_health;

						ent->health += take;
						ent->client->pers.pool_health -= take;

						gi.sound(ent, CHAN_ITEM, gi.soundindex("items/s_health.wav"), 0.75, ATTN_NORM, 0);
					}
				}
			}
		}
	}
}


/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
void ClientBeginServerFrame(edict_t *ent)
{
    gclient_t   *client;
    int         buttonMask;
	qboolean	lockdown_respawn = qtrue;

    if (level.intermissiontime)
        return;

    client = ent->client;

	// Rroff coop test
	//if (deathmatch->value &&
    if ((deathmatch->value || (coop->value && g_coopflags->value)) &&
        client->pers.spectator != client->resp.spectator &&
        (level.time - client->respawn_time) >= 5) {
        spectator_respawn(ent);
        return;
    }

    // run weapon animations if it hasn't been done by a ucmd_t
    if (!client->weapon_thunk && !client->resp.spectator)
        Think_Weapon(ent);
    else
        client->weapon_thunk = qfalse;

	// Rroff - horde mode forever clients can only respawn in the intermission
	if (coop->value && level.lockdown_ent && ((level.lockdown_ent->moded == LK_FOREVER) || (level.lockdown_ent->moded == LK_CUSTOM)))
	{
		if (level.lockdown_ent->delay == 0)
			lockdown_respawn = qfalse;
	}

    if (ent->deadflag) {
        // wait for any button just going down
        if (level.time > client->respawn_time && lockdown_respawn) {
            // in deathmatch, only wait for attack button
            if (deathmatch->value)
                buttonMask = BUTTON_ATTACK;
            else
                buttonMask = -1;

            if ((client->latched_buttons & buttonMask) ||
                (deathmatch->value && ((int)dmflags->value & DF_FORCE_RESPAWN))) {
                respawn(ent);
                client->latched_buttons = 0;
            }
        }
        return;
    }

    // add player trail so monsters can follow
    if (!deathmatch->value)
        if (!visible(ent, PlayerTrail_LastSpot()))
            PlayerTrail_Add(ent->s.old_origin);

    client->latched_buttons = 0;
}
