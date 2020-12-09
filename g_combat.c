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
// g_combat.c

#include "g_local.h"

#define MAX_HORDE_REWARD 12

char *horde_rewards[MAX_HORDE_REWARD] = {
	"[Regen Mod]",
	"[Blaster Mod]",
	"[Machinegun Mod]",
	"[Tombstone]",
	"[Grenade Mod]",
	"[GLauncher Mod]",
	"[Solar]",
	"[Sabot Mod]",
	"[Tripmines]",
	"[Scavenger]",
	"[Active Defns]",
	"[Stim Pack]",
};

/*
============
CanDamage

Returns qtrue if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
qboolean CanDamage(edict_t *targ, edict_t *inflictor)
{
    vec3_t  dest;
    trace_t trace;

// bmodels need special checking because their origin is 0,0,0
    if (targ->movetype == MOVETYPE_PUSH) {
        VectorAdd(targ->absmin, targ->absmax, dest);
        VectorScale(dest, 0.5, dest);
        trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
        if (trace.fraction == 1.0)
            return qtrue;
        if (trace.ent == targ)
            return qtrue;
        return qfalse;
    }

    trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, targ->s.origin, inflictor, MASK_SOLID);
    if (trace.fraction == 1.0)
        return qtrue;

    VectorCopy(targ->s.origin, dest);
    dest[0] += 15.0;
    dest[1] += 15.0;
    trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
    if (trace.fraction == 1.0)
        return qtrue;

    VectorCopy(targ->s.origin, dest);
    dest[0] += 15.0;
    dest[1] -= 15.0;
    trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
    if (trace.fraction == 1.0)
        return qtrue;

    VectorCopy(targ->s.origin, dest);
    dest[0] -= 15.0;
    dest[1] += 15.0;
    trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
    if (trace.fraction == 1.0)
        return qtrue;

    VectorCopy(targ->s.origin, dest);
    dest[0] -= 15.0;
    dest[1] -= 15.0;
    trace = gi.trace(inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
    if (trace.fraction == 1.0)
        return qtrue;


    return qfalse;
}

void givehordereward(void)
{
	gitem_t		*it;
	int			amount, index;
	edict_t		*cl_ent;
	int			i;
	edict_t     *it_ent;

	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || game.clients[i].resp.spectator)
		{
			continue;
		}

		if (cl_ent->health <= 0)
			continue;

		//it = FindItem("[Regen Mod]");
		it = FindItem(horde_rewards[(rand() % MAX_HORDE_REWARD)]);

		if (it)
		{
			it_ent = G_Spawn();
			it_ent->classname = it->classname;
			SpawnItem(it_ent, it);
			Touch_Item(it_ent, cl_ent, NULL, NULL);
			if (it_ent->inuse)
				G_FreeEdict(it_ent);
		}
	}
}


/*
============
Killed
============
*/
void Killed(edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		give;

    if (targ->health < -999)
        targ->health = -999;

    targ->enemy = attacker;

    if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD)) {
		if (targ->monsterinfo.aiflags2 & AI2_CONTROL)
		{
			cpoint_trigger_spawn();
		}

		if ((targ->monsterinfo.aiflags2 & AI2_CONTROL_REWARD) && !(targ->monsterinfo.aiflags2 & AI2_CONTROL_GOALTIMER))
		{
			givehordereward();
		}

		if (targ->monsterinfo.aiflags2 & AI2_CONTROL_GOALTIMER)
		{
			if (level.lockdown_ent)
			{
				givehordereward();

				level.lockdown_ent->turret_ammo--;
				level.lockdown_ent->gib_health--;
				level.found_goals++;
			}
		}
//      targ->svflags |= SVF_DEADMONSTER;   // now treat as a different content type
        if (!(targ->monsterinfo.aiflags & AI_GOOD_GUY)) {
            level.killed_monsters++;
            if (coop->value && attacker->client)
                attacker->client->resp.score++;
            // medics won't heal monsters that they kill themselves
            if (strcmp(attacker->classname, "monster_medic") == 0)
                targ->owner = attacker;
        }
    }

    if (targ->movetype == MOVETYPE_PUSH || targ->movetype == MOVETYPE_STOP || targ->movetype == MOVETYPE_NONE) {
        // doors, triggers, etc
        targ->die(targ, inflictor, attacker, damage, point);
        return;
    }

    if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD)) {
        targ->touch = NULL;
        monster_death_use(targ);
		targ->playeruse = misc_deadsoldier_playeruse;

		if (attacker->client)
		{
			if (attacker->client->pers.mods & MU_GAMBLER)
			{
				//if (random() < 0.2)
				if (random() < 0.3)
				{
					if (attacker->health > 1)
					{
						T_Damage(attacker, targ, targ, vec3_origin, attacker->s.origin, vec3_origin, 25, 0, DAMAGE_NO_KILL, MOD_GAMBLE);
						gi.sound(attacker, CHAN_ITEM, gi.soundindex("medic/medatck5.wav"), 1, ATTN_NORM, 0);
					} else {
						if (attacker->health == 1)
						{
							give = targ->max_health;

							if (targ->max_health < 40)
								give = 40;

							// kind of is the case!
							if (attacker->health < attacker->max_health)
							{
								attacker->health += (give * 0.15);
								if (attacker->health > attacker->max_health)
									attacker->health = attacker->max_health;
								gi.sound(attacker, CHAN_ITEM, gi.soundindex("items/s_health.wav"), 0.75, ATTN_NORM, 0);
							}
						}
					}
				} else {
					//if (random() < 0.4)
					if (random() < 0.3)
					{
						if (attacker->health > 0)
						{
							//if (!(attacker->client->quad_framenum > level.framenum))
							// player class with quad bonus get additional time here?
							if (attacker->client->quad_framenum <= level.framenum + 50)
							{
								if (attacker->client->quad_framenum > level.framenum)
									attacker->client->quad_framenum += 50;
								else
									attacker->client->quad_framenum = level.framenum + 50;

								gi.sound(attacker, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
							}
						}
					}
				}
			}
		}
    }

    targ->die(targ, inflictor, attacker, damage, point);
}


/*
================
SpawnDamage
================
*/
void SpawnDamage(int type, vec3_t origin, vec3_t normal, int damage)
{
    if (damage > 255)
        damage = 255;
    gi.WriteByte(svc_temp_entity);
    gi.WriteByte(type);
//  gi.WriteByte (damage);
    gi.WritePosition(origin);
    gi.WriteDir(normal);
    gi.multicast(origin, MULTICAST_PVS);
}


/*
============
T_Damage

targ        entity that is being damaged
inflictor   entity that is causing the damage
attacker    entity that caused the inflictor to damage targ
    example: targ=monster, inflictor=rocket, attacker=player

dir         direction of the attack
point       point at which the damage is being inflicted
normal      normal vector from that point
damage      amount of damage being inflicted
knockback   force to be applied against targ as a result of the damage

dflags      these flags are used to control how T_Damage works
    DAMAGE_RADIUS           damage was indirect (from a nearby explosion)
    DAMAGE_NO_ARMOR         armor does not protect from this damage
    DAMAGE_ENERGY           damage is from an energy based weapon
    DAMAGE_NO_KNOCKBACK     do not affect velocity, just view angles
    DAMAGE_BULLET           damage is from a bullet (used for ricochets)
    DAMAGE_NO_PROTECTION    kills godmode, armor, everything
============
*/
static int CheckPowerArmor(edict_t *ent, vec3_t point, vec3_t normal, int damage, int dflags)
{
    gclient_t   *client;
    int         save;
    int         power_armor_type;
    int         index;
    int         damagePerCell;
    int         pa_te_type;
    int         power;
    int         power_used;

    if (!damage)
        return 0;

    client = ent->client;

    if (dflags & DAMAGE_NO_ARMOR)
        return 0;

    index = 0;  // shut up gcc

    if (client) {
        power_armor_type = PowerArmorType(ent);
        if (power_armor_type != POWER_ARMOR_NONE) {
            index = ITEM_INDEX(FindItem("Cells"));
            power = client->pers.inventory[index];
        }
    } else if (ent->svflags & SVF_MONSTER) {
        power_armor_type = ent->monsterinfo.power_armor_type;
        power = ent->monsterinfo.power_armor_power;
    } else
        return 0;

    if (power_armor_type == POWER_ARMOR_NONE)
        return 0;
    if (!power)
        return 0;

    if (power_armor_type == POWER_ARMOR_SCREEN) {
        vec3_t      vec;
        float       dot;
        vec3_t      forward;

        // only works if damage point is in front
        AngleVectors(ent->s.angles, forward, NULL, NULL);
        VectorSubtract(point, ent->s.origin, vec);
        VectorNormalize(vec);
        dot = DotProduct(vec, forward);
        if (dot <= 0.3)
            return 0;

        damagePerCell = 1;
        pa_te_type = TE_SCREEN_SPARKS;
        damage = damage / 3;
    } else {
        damagePerCell = 2;
        pa_te_type = TE_SHIELD_SPARKS;
        damage = (2 * damage) / 3;
    }

    save = power * damagePerCell;
    if (!save)
        return 0;
    if (save > damage)
        save = damage;

    SpawnDamage(pa_te_type, point, normal, save);
    ent->powerarmor_time = level.time + 0.2;

    power_used = save / damagePerCell;

    if (client)
        client->pers.inventory[index] -= power_used;
    else
        ent->monsterinfo.power_armor_power -= power_used;
    return save;
}

static int CheckArmor(edict_t *ent, vec3_t point, vec3_t normal, int damage, int te_sparks, int dflags)
{
    gclient_t   *client;
    int         save;
    int         index;
    gitem_t     *armor;

    if (!damage)
        return 0;

    client = ent->client;

    if (!client)
        return 0;

    if (dflags & DAMAGE_NO_ARMOR)
        return 0;

    index = ArmorIndex(ent);
    if (!index)
        return 0;

    armor = GetItemByIndex(index);

    if (dflags & DAMAGE_ENERGY)
        save = ceil(((gitem_armor_t *)armor->info)->energy_protection * damage);
    else
        save = ceil(((gitem_armor_t *)armor->info)->normal_protection * damage);
    if (save >= client->pers.inventory[index])
        save = client->pers.inventory[index];

    if (!save)
        return 0;

    client->pers.inventory[index] -= save;
    SpawnDamage(te_sparks, point, normal, save);

    return save;
}

void M_ReactToDamage(edict_t *targ, edict_t *attacker)
{
	edict_t			*spot;

	if (!(attacker->client) && !(attacker->svflags & SVF_MONSTER))
	{
		if (!(attacker->monsterinfo.aiflags & AI_PLAYERPROXY))
			return;
	}

	if (attacker == targ || attacker == targ->enemy)
	{
		if (!(targ->monsterinfo.aiflags & AI_RUNAWAY))
			return;
	}

	// BUG FIX #86
	// dead monsters, like misc_deadsoldier, don't have AI functions, but 
	// M_ReactToDamage might still be called on them
	if (targ->svflags & SVF_DEADMONSTER)
		return;

	if (targ->monsterinfo.aiflags & AI_MEDIC2)
		return; // should really run away from an attack

	// Rroff do we need this? not sure it is working right
	if (targ->monsterinfo.aiflags & AI_RUNAWAY)
	{
		if (level.time < targ->monsterinfo.runaway_time + 1)
			return;

		targ->monsterinfo.aiflags &= ~(AI_RUNAWAY | AI_ASSUMED);
		targ->goalentity = NULL;
	}

    // if we are a good guy monster and our attacker is a player
    // or another good guy, do not get mad at them
    if (targ->monsterinfo.aiflags & AI_GOOD_GUY) {
        if (attacker->client || (attacker->monsterinfo.aiflags & AI_GOOD_GUY))
            return;
    }

    // we now know that we are not both good guys

	if ((attacker->monsterinfo.aiflags & AI_PLAYERPROXY) || (attacker->monsterinfo.aiflags & AI_GRUNT))
	{
		targ->enemy = attacker;
		if (!(targ->monsterinfo.aiflags & AI_DUCKED))
		{
			FoundTarget(targ);
			return;
		}
	}

    // if attacker is a client, get mad at them because he's good and we're not
    if (attacker->client) {
        targ->monsterinfo.aiflags &= ~AI_SOUND_TARGET;

        // this can only happen in coop (both new and old enemies are clients)
        // only switch if can't see the current enemy
        if (targ->enemy && targ->enemy->client) {
            if (visible(targ, targ->enemy)) {
                targ->oldenemy = attacker;
                return;
            }
            targ->oldenemy = targ->enemy;
        }
        targ->enemy = attacker;
        if (!(targ->monsterinfo.aiflags & AI_DUCKED))
            FoundTarget(targ);
        return;
    }

	// it's the same base (walk/swim/fly) type and a different classname and it's not a tank
	// (they spray too much), get mad at them
	// Rroff - nightmare skill don't attack another monster so readily
	if (((targ->flags & (FL_FLY | FL_SWIM)) == (attacker->flags & (FL_FLY | FL_SWIM))) &&
		(strcmp(targ->classname, attacker->classname) != 0) &&
		(strcmp(attacker->classname, "monster_tank") != 0) &&
		(strcmp(attacker->classname, "monster_supertank") != 0) &&
		(strcmp(attacker->classname, "monster_makron") != 0) &&
		(strcmp(attacker->classname, "monster_jorg") != 0)) {
		if ((skill->value != 3) || (targ->health < (targ->max_health*0.5)))
		{
			if (targ->enemy && targ->enemy->client)
				targ->oldenemy = targ->enemy;
			targ->enemy = attacker;
			if (!(targ->monsterinfo.aiflags & AI_DUCKED))
				FoundTarget(targ);
		} else {
			if (level.time > targ->monsterinfo.friendlyfire_time)
			{
				targ->monsterinfo.friendlyfire_time = level.time + 5;
				if (random() < 0.2)
				{
					if (targ->enemy && targ->enemy->client)
						targ->oldenemy = targ->enemy;
					targ->enemy = attacker;
					if (!(targ->monsterinfo.aiflags & AI_DUCKED))
						FoundTarget(targ);
				} else {
					// Rroff just testing getting the attacker to try and find
					// a better spot
					if (!(attacker->monsterinfo.aiflags & AI_GOOD_GUY) && (level.time > attacker->monsterinfo.runaway_time + 10))
					{
						spot = runpoint(attacker);
						if (spot)
						{
							attacker->oldenemy = attacker->enemy;
							//ent->enemy = NULL;
							attacker->goalentity = spot;
							attacker->monsterinfo.run(attacker);
							attacker->monsterinfo.aiflags |= AI_RUNAWAY;
							attacker->monsterinfo.runaway_time = level.time - 3; // was 5
						}
					}
				}
			}
		}
	}
    // if they *meant* to shoot us, then shoot back
    else if (attacker->enemy == targ) {
        if (targ->enemy && targ->enemy->client)
            targ->oldenemy = targ->enemy;
        targ->enemy = attacker;
        if (!(targ->monsterinfo.aiflags & AI_DUCKED))
            FoundTarget(targ);
    }
    // otherwise get mad at whoever they are mad at (help our buddy) unless it is us!
    else if (attacker->enemy && attacker->enemy != targ) {
        if (targ->enemy && targ->enemy->client)
            targ->oldenemy = targ->enemy;
        targ->enemy = attacker->enemy;
        if (!(targ->monsterinfo.aiflags & AI_DUCKED))
            FoundTarget(targ);
    }
}

qboolean CheckTeamDamage(edict_t *targ, edict_t *attacker)
{
    //FIXME make the next line real and uncomment this block
    // if ((ability to damage a teammate == OFF) && (targ's team == attacker's team))
    return qfalse;
}

// Rroff - any assault class
void assaultBonus()
{
	edict_t		*cl_ent;
	int			i;

	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || game.clients[i].resp.spectator)
		{
			continue;
		}

		if (cl_ent->client->pers.player_class & PC_ASSAULT)
		{
			cl_ent->client->last_kill_time = level.time;
		}
	}
}

void T_Damage(edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod)
{
    gclient_t   *client;
    int         take;
    int         save;
    int         asave;
    int         psave;
    int         te_sparks;
	int			give;

	int				index;
	gitem_t			*fitem;
	qboolean		deadtag = qfalse;
	float			dist;
	vec3_t			v;

    if (!targ->takedamage)
        return;

    // friendly fire avoidance
    // if enabled you can't hurt teammates (but you can hurt yourself)
    // knockback still occurs
    if ((targ != attacker) && ((deathmatch->value && ((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS))) || coop->value)) {
        if (OnSameTeam(targ, attacker)) {
            if ((int)(dmflags->value) & DF_NO_FRIENDLY_FIRE)
                damage = 0;
            else
                mod |= MOD_FRIENDLY_FIRE;
        }
    }
    meansOfDeath = mod;

	// don't let turrets hurt other players
	if ((mod == MOD_PTURRET) && (targ->client))
		damage = 0;

	// don't let friendly grunts hurt players!
	if ((mod == MOD_GRUNT) && (targ->client))
		damage = 0;

	if ((attacker->monsterinfo.aiflags & AI_PLAYERPROXY) && (targ->client))
		damage = 0;

    // easy mode takes half damage
    if (skill->value == 0 && deathmatch->value == 0 && targ->client) {
        damage *= 0.5;
        if (!damage)
            damage = 1;
    }

	if (targ->client)
	{
		if (targ->client->pers.mods & MU_EDGE)
		{
			damage *= 2;
		}
	}

	if (attacker->client)
	{
		if (attacker->client->pers.mods & MU_EDGE)
		{
			damage *= 1.75;
		}

		if (attacker->client->pers.player_class & PC_ASSAULT)
		{
			if (level.time < attacker->client->last_kill_time + 15)
				damage *= 1.15; // Rroff damage bonus for assault for 15 seconds after a kill
		}
	}

    client = targ->client;

	if (targ->svflags & SVF_MONSTER)
	{
		if ((targ->monsterinfo.armoured) && (dflags & DAMAGE_AP))
		{
			damage *= 1.15; // Rroff armour penetrating damage does 15% extra
		}

		if (!(targ->monsterinfo.armoured) && (dflags & DAMAGE_INCENDIARY))
		{
			damage *= 1.25; // Rroff incendiary damage does 25% extra
		}
	}

    if (dflags & DAMAGE_BULLET)
        te_sparks = TE_BULLET_SPARKS;
    else
        te_sparks = TE_SPARKS;

    VectorNormalize(dir);

// bonus damage for suprising a monster
	if (!(dflags & DAMAGE_RADIUS) && (targ->svflags & SVF_MONSTER) && (attacker->client) && (!targ->enemy) && (targ->health > 0))
	{
		damage *= 2;
		targ->monsterinfo.surprised_frame = level.framenum;
	}

    if (targ->flags & FL_NO_KNOCKBACK)
        knockback = 0;

// figure momentum add
    if (!(dflags & DAMAGE_NO_KNOCKBACK)) {
        if ((knockback) && (targ->movetype != MOVETYPE_NONE) && (targ->movetype != MOVETYPE_BOUNCE) && (targ->movetype != MOVETYPE_PUSH) && (targ->movetype != MOVETYPE_STOP)) {
            vec3_t  kvel;
            float   mass;

            if (targ->mass < 50)
                mass = 50;
            else
                mass = targ->mass;

            if (targ->client  && attacker == targ)
                VectorScale(dir, 1600.0 * (float)knockback / mass, kvel);   // the rocket jump hack...
            else
                VectorScale(dir, 500.0 * (float)knockback / mass, kvel);

            VectorAdd(targ->velocity, kvel, targ->velocity);
        }
    }

    take = damage;
    save = 0;

    // check for godmode
    if ((targ->flags & FL_GODMODE) && !(dflags & DAMAGE_NO_PROTECTION)) {
        take = 0;
        save = damage;
        SpawnDamage(te_sparks, point, normal, save);
    }

	// Rroff needs to be above invincibility unless we check for that - also need to take into account god mode
	if ((client && client->berserk_framenum > level.framenum) && !(dflags & DAMAGE_NO_PROTECTION) && !(targ->flags & FL_GODMODE)) {
		if (targ->pain_debounce_time < level.time) {
			gi.sound(targ, CHAN_ITEM, gi.soundindex("items/protect4.wav"), 1, ATTN_NORM, 0);
			targ->pain_debounce_time = level.time + 2;
		}

		if (client->pers.weapon->weapmodel == WEAP_SHOTGUN || client->pers.weapon->weapmodel == WEAP_SUPERSHOTGUN)
		{
			// is it a given these effects will be close range? if so might just do it based on distance
			// though might save some distance calcs and allows some variation in multiplier
			if (mod == MOD_HIT || mod == MOD_G_SPLASH || mod == MOD_R_SPLASH || mod == MOD_BFG_LASER || mod == MOD_HG_SPLASH || mod == MOD_CRUSH)
			{
				take = damage * 0.1;
				save = damage * 0.9;
			}
			else {
				if (attacker) // should always be valid
				{
					VectorSubtract(attacker->s.origin, targ->s.origin, v);
					dist = VectorLength(v);
					if (dist <= 128)
					{
						take = damage * 0.25;
						save = damage * 0.75;
					}
					else {
						take = save = damage * 0.5;
					}
				}
				else {
					take = save = damage * 0.5;
				}
			}
		}
	}

    // check for invincibility
    if ((client && client->invincible_framenum > level.framenum) && !(dflags & DAMAGE_NO_PROTECTION)) {
        if (targ->pain_debounce_time < level.time) {
            gi.sound(targ, CHAN_ITEM, gi.soundindex("items/protect4.wav"), 1, ATTN_NORM, 0);
            targ->pain_debounce_time = level.time + 2;
        }
        take = 0;
        save = damage;
    }

    psave = CheckPowerArmor(targ, point, normal, take, dflags);
    take -= psave;

    asave = CheckArmor(targ, point, normal, take, te_sparks, dflags);
    take -= asave;

    //treat cheat/powerup savings the same as armor
    asave += save;

    // team damage avoidance
    if (!(dflags & DAMAGE_NO_PROTECTION) && CheckTeamDamage(targ, attacker))
        return;

	if (targ->svflags & SVF_MONSTER)
	{
		// Tank commander bonus is a decent buff

		if (targ->monsterinfo.bonus_time && level.time < targ->monsterinfo.bonus_time + 30)
		{
			if (coop->value || g_hardmonsters->value)
				take *= 0.25;
			else
				take *= 0.5;
		}
	}

	if (attacker->svflags & SVF_MONSTER)
	{
		// Tank commander bonus is a decent buff

		if (attacker->monsterinfo.bonus_time && level.time < attacker->monsterinfo.bonus_time + 30)
		{
			if (coop->value || g_hardmonsters->value)
				take *= 1.33;
			else
				take *= 1.25;
		}
	}

	if (targ->client)
	{
		if (level.framenum < (targ->client->taunt_framenum - 200))
		{
			take *= 0.5;
		} else if (targ->client->taunt_framenum > level.framenum) {
			take *= 0.75;
		}

		// handled by power shield instead
		//if (targ->client->pers.player_class & PC_CYBORG)
		//	take *= 0.75;

		if (targ->client->pers.mods & MU_STIM)
		{
			if (level.time <= targ->client->stim_time + 15)
				take *= 0.5;
		}

		if (targ->client->pers.mods & MU_BOUNCER)
		{
			if (client->bouncer_framenum > level.framenum)
			{
				take *= 0.33;
			}
		}
	}

// do the damage
    if (take) {
        /*if ((targ->svflags & SVF_MONSTER) || (client))
        {
            // SpawnDamage(TE_BLOOD, point, normal, take);
            SpawnDamage(TE_BLOOD, point, dir, take);
        }
        else
            SpawnDamage(te_sparks, point, normal, take);*/

		if ((targ->svflags & SVF_MONSTER) || (client))
		{
			if (targ->svflags & SVF_MONSTER)
			{
				if ((targ->monsterinfo.armoured) && (dflags & DAMAGE_AP) && (random() > 0.3) && (targ->health > 30))
				{
					gi.WriteByte(svc_temp_entity);
					gi.WriteByte(TE_SPLASH);
					gi.WriteByte(16);
					gi.WritePosition(point);
					gi.WriteDir(normal);
					gi.WriteByte(1);	//sparks
					gi.multicast(point, MULTICAST_PVS);
				}
				else {
					SpawnDamage(TE_BLOOD, point, normal, take);
				}
			}
			else {
				SpawnDamage(TE_BLOOD, point, normal, take);
			}
		}
		else
		{
			SpawnDamage(te_sparks, point, normal, take);
		}


        targ->health = targ->health - take;

		// Rroff
		if (attacker->client && (targ->svflags & SVF_MONSTER) && (targ->health > 0))
		{
			// Modified grenade launcher
			if (mod == MOD_GL2)
			{
				if (targ->monsterinfo.dot_time[DOT_GRENADE] < level.time + 5)
					targ->monsterinfo.dot_time[DOT_GRENADE] = level.time + 5;
				targ->monsterinfo.dot_amt[DOT_GRENADE] = 4;

				gi.WriteByte(svc_temp_entity);
				gi.WriteByte(TE_SPLASH);
				gi.WriteByte(16);
				gi.WritePosition(point);
				gi.WriteDir(normal);
				gi.WriteByte(1);	//sparks
				gi.multicast(point, MULTICAST_PVS);
			}
		}

		if ((targ->health <= 0) && (dflags & DAMAGE_NO_KILL))
			targ->health = 1;

        if (targ->health <= 0) {
            if ((targ->svflags & SVF_MONSTER) || (client))
                targ->flags |= FL_NO_KNOCKBACK;

			// Rroff stuff in response to killing a monster
			if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD))
			{
				if (attacker->client)
				{
					assaultBonus();

					if (mod == MOD_CHAINGUN && attacker->client->pers.tracked_bouncer < 5)
					{
						if (attacker->client->firedown)
						{
							attacker->client->firekillcount += 1;

							// every 4 kills gets a point - reduce to 3?
							if (attacker->client->firekillcount > 3)
							{
								attacker->client->firekillcount = 0;


								attacker->client->pers.tracked_bouncer += 1;
								attacker->client->resp.coop_respawn.tracked_bouncer += 1;

								if (attacker->client->pers.tracked_bouncer == 5)
								{
									fitem = FindItem("[The Bouncer]");
									index = ITEM_INDEX(fitem);

									if (attacker->client->pers.inventory[index] < 1)
									{
										attacker->client->pers.inventory[index] = 1;

										attacker->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(fitem->icon);
										attacker->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS + index;
										attacker->client->pickup_msg_time = level.time + 3.0;

										gi.cprintf(attacker, PRINT_HIGH, "Activate [The Bouncer] from the inventory to use it\n");

										gi.sound(attacker, CHAN_ITEM, gi.soundindex("misc/secret.wav"), 1, ATTN_NORM, 0);
									}
								}
								else {
									gi.sound(attacker, CHAN_ITEM, gi.soundindex("misc/comp_up.wav"), 0.5, ATTN_NORM, 0);
								}
							}
						}
					}
					//points for killing a monster while having less than 10 health
					//can cheese this with high armour so might tweak that
					if (attacker->health < 10)
					{
						if (attacker->client->pers.tracked_edge < 10)
						{
							attacker->client->pers.tracked_edge += 1;
							attacker->client->resp.coop_respawn.tracked_edge += 1;

							if (attacker->client->pers.tracked_edge == 10)
							{
								fitem = FindItem("[The Edge]");
								index = ITEM_INDEX(fitem);

								if (attacker->client->pers.inventory[index] < 1)
								{
									attacker->client->pers.inventory[index] = 1;

									attacker->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(fitem->icon);
									attacker->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS + index;
									attacker->client->pickup_msg_time = level.time + 3.0;

									gi.cprintf(attacker, PRINT_HIGH, "Activate [The Edge] from the inventory to use it\n");

									gi.sound(attacker, CHAN_ITEM, gi.soundindex("misc/secret.wav"), 1, ATTN_NORM, 0);
								}
							} else {
								gi.sound(attacker, CHAN_ITEM, gi.soundindex("misc/comp_up.wav"), 0.5, ATTN_NORM, 0);
							}
						}
					}
					// points for monsters killed while using quad
					if (attacker->client->quad_framenum > level.framenum)
					{
						if (attacker->client->pers.tracked_gambler < 30)
						{
							attacker->client->pers.tracked_gambler += 1;
							attacker->client->resp.coop_respawn.tracked_gambler += 1;
							gi.sound(attacker, CHAN_ITEM, gi.soundindex("misc/comp_up.wav"), 0.5, ATTN_NORM, 0);

							if (attacker->client->pers.tracked_gambler == 30)
							{
								fitem = FindItem("[The Gambler]");
								index = ITEM_INDEX(fitem);

								if (attacker->client->pers.inventory[index] < 1)
								{
									attacker->client->pers.inventory[index] = 1;

									attacker->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(fitem->icon);
									attacker->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS + index;
									attacker->client->pickup_msg_time = level.time + 3.0;

									gi.cprintf(attacker, PRINT_HIGH, "Activate [The Gambler] from the inventory to use it\n");

									gi.sound(attacker, CHAN_ITEM, gi.soundindex("misc/secret.wav"), 1, ATTN_NORM, 0);
								}
							} else {
								gi.sound(attacker, CHAN_ITEM, gi.soundindex("misc/comp_up.wav"), 0.5, ATTN_NORM, 0);
							}
						}
					}

					// add a point for killing a monster before they noticed
					if (level.framenum < targ->monsterinfo.surprised_frame + 3)
					{
						if (attacker->client->pers.tracked_reaper < 10)
						{
							attacker->client->pers.tracked_reaper += 1;
							attacker->client->resp.coop_respawn.tracked_reaper += 1;
							gi.sound(attacker, CHAN_ITEM, gi.soundindex("misc/comp_up.wav"), 0.5, ATTN_NORM, 0);

							if (attacker->client->pers.tracked_reaper == 10)
							{
								fitem = FindItem("[The Reaper]");
								index = ITEM_INDEX(fitem);

								if (attacker->client->pers.inventory[index] < 1)
								{
									attacker->client->pers.inventory[index] = 1;

									attacker->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(fitem->icon);
									attacker->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS + index;
									attacker->client->pickup_msg_time = level.time + 3.0;

									gi.cprintf(attacker, PRINT_HIGH, "Activate [The Reaper] from the inventory to use it\n");

									gi.sound(attacker, CHAN_ITEM, gi.soundindex("misc/secret.wav"), 1, ATTN_NORM, 0);
								}
							} else {
								gi.sound(attacker, CHAN_ITEM, gi.soundindex("misc/comp_up.wav"), 0.5, ATTN_NORM, 0);
							}

						}
					}

					if ((attacker->client->pers.mods & MU_BERSERK) && (attacker->health > 0))
					{
						if (mod == MOD_SHOTGUN || mod == MOD_SSHOTGUN || mod == MOD_SABOT)
						{
							if (attacker->client->berserk_framenum <= level.framenum + 50)
							{
								if (attacker->client->berserk_framenum > level.framenum)
									attacker->client->berserk_framenum += 50;
								else
									attacker->client->berserk_framenum = level.framenum + 50;

								gi.sound(attacker, CHAN_ITEM, gi.soundindex("items/protect.wav"), 1, ATTN_NORM, 0);
							}
						}
					}

					if ((attacker->client->pers.mods & MU_REAPER) && (attacker->health > 0))
					{
						// ignores max health

						//if (!targ->max_health)
						//	give = 20;

						give = targ->max_health;

						if (targ->max_health < 50)
							give = 50;

						//if (attacker->client->reaper_framenum > level.framenum)
						//	give *= 1.75; // was 1.5, 1.75 or 2?
											// railgun only for this bonus?

						// should we cap it?
						// MH will cause count down?
						// should additional health above max health count down?

						if (attacker->health < attacker->max_health)
						{
							if (attacker->client->reaper_framenum > level.framenum)
							{
								attacker->health += (give * 0.175);
							}
							else {
								if (level.framenum < targ->monsterinfo.surprised_frame + 3)
									attacker->health += (give * 0.04); // 0.15?
								else
									attacker->health += (give * 0.02); // 0.1
							}

							if (attacker->health > attacker->max_health)
								attacker->health = attacker->max_health;
						}

						gi.sound(attacker, CHAN_ITEM, gi.soundindex("items/s_health.wav"), 0.75, ATTN_NORM, 0);
					}
				}
			}

			Killed(targ, inflictor, attacker, take, point);

            return;
        }
    }

    if (targ->svflags & SVF_MONSTER) {
        M_ReactToDamage(targ, attacker);
        if (!(targ->monsterinfo.aiflags & AI_DUCKED) && (take)) {
            targ->pain(targ, attacker, knockback, take);
            // nightmare mode monsters don't go into pain frames often
            if (skill->value == 3)
                targ->pain_debounce_time = level.time + 5;
        }
    } else if (client) {
        if (!(targ->flags & FL_GODMODE) && (take))
            targ->pain(targ, attacker, knockback, take);
    } else if (take) {
        if (targ->pain)
            targ->pain(targ, attacker, knockback, take);
    }

    // add to the damage inflicted on a player this frame
    // the total will be turned into screen blends and view angle kicks
    // at the end of the frame
    if (client) {
        client->damage_parmor += psave;
        client->damage_armor += asave;
        client->damage_blood += take;
        client->damage_knockback += knockback;
        VectorCopy(point, client->damage_from);
    }
}


/*
============
T_RadiusDamage
============
*/
void T_RadiusDamage(edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod)
{
    float   points;
    edict_t *ent = NULL;
    vec3_t  v;
    vec3_t  dir;

    while ((ent = findradius(ent, inflictor->s.origin, radius)) != NULL) {
        if (ent == ignore)
            continue;
        if (!ent->takedamage)
            continue;

        VectorAdd(ent->mins, ent->maxs, v);
        VectorMA(ent->s.origin, 0.5, v, v);
        VectorSubtract(inflictor->s.origin, v, v);
        points = damage - 0.5 * VectorLength(v);
        if (ent == attacker)
            points = points * 0.5;
        if (points > 0) {
            if (CanDamage(ent, inflictor)) {
                VectorSubtract(ent->s.origin, inflictor->s.origin, dir);
                T_Damage(ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
            }
        }
    }
}