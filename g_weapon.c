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

static void silly_impact(vec3_t start, vec3_t plane, qboolean scorch)
{
	vec3_t		normal_angles, forward, right, up;
	vec3_t		offset, pos;
	qboolean	small = qfalse;
	edict_t		*ent;

	trace_t		tr;

	if (g_nodecals->value)
		return;

	vectoangles(plane, normal_angles);
	AngleVectors(normal_angles, forward, right, up);

	VectorCopy(start, pos);
	VectorMA(pos, -8, right, pos);
	VectorMA(pos, 4, up, pos);
	VectorMA(pos, -0.2, forward, pos);

	tr = gi.trace(pos, NULL, NULL, start, NULL, MASK_SOLID);

	if (!tr.startsolid)
		small = qtrue;

	if (small == qfalse)
	{
		VectorCopy(start, pos);
		VectorMA(pos, 6, right, pos);
		VectorMA(pos, 7, up, pos);
		VectorMA(pos, -0.2, forward, pos);

		tr = gi.trace(pos, NULL, NULL, start, NULL, MASK_SOLID);

		if (!tr.startsolid)
			small = qtrue;
	}

	if (small == qfalse)
	{
		VectorCopy(start, pos);
		VectorMA(pos, -1, right, pos);
		VectorMA(pos, -8, up, pos);
		VectorMA(pos, -0.2, forward, pos);

		tr = gi.trace(pos, NULL, NULL, start, NULL, MASK_SOLID);

		if (!tr.startsolid)
			small = qtrue;
	}

	if (!small)
	{
		ent = G_SpawnNC();
		
		ent->s.modelindex = gi.modelindex("models/objects/impacts/tris.md2");
		ent->s.renderfx |= RF_TRANSLUCENT | 0x80000000;

		if (scorch)
		{
			// railgun, etc.

			//ent->s.modelindex = gi.modelindex("models/objects/splatter/tris2.md2");
			//ent->s.skinnum = 1;
			//ent->s.effects |= EF_SPHERETRANS;
			ent->s.skinnum = 7;

			//ent->s.renderfx |= RF_TRANSLUCENT | 0x80000000;
		}
		else
		{
			//ent->s.modelindex = gi.modelindex("models/objects/impacts/bullet.md2");
			// ent->s.skinnum = 1;
			// ent->s.renderfx |= 0x80000000; //???
			ent->s.skinnum = 2;
		}

		ent->movetype = MOVETYPE_NONE;
		vectoangles(forward, ent->s.angles);
		ent->s.angles[ROLL] = rand() % 360;
		VectorCopy(start, ent->s.origin);
		
		ent->think = G_FreeEdict;
		ent->nextthink = level.time + 999;

		gi.linkentity(ent);
	}
}

/*
=================
check_dodge

This is a support routine used when a client is firing
a non-instant attack weapon.  It checks to see if a
monster's dodge function should be called.
=================
*/
static void check_dodge(edict_t *self, vec3_t start, vec3_t dir, int speed)
{
    vec3_t  end;
    vec3_t  v;
    trace_t tr;
    float   eta;

    // easy mode only ducks one quarter the time
    if (skill->value == 0) {
        if (random() > 0.25)
            return;
    }
    VectorMA(start, 8192, dir, end);
    tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);
    if ((tr.ent) && (tr.ent->svflags & SVF_MONSTER) && (tr.ent->health > 0) && (tr.ent->monsterinfo.dodge) && infront(tr.ent, self)) {
        VectorSubtract(tr.endpos, start, v);
        eta = (VectorLength(v) - tr.ent->maxs[0]) / speed;
        tr.ent->monsterinfo.dodge(tr.ent, self, eta);
    }
}


/*
=================
fire_hit

Used for all impact (hit/punch/slash) attacks
=================
*/
qboolean fire_hit(edict_t *self, vec3_t aim, int damage, int kick)
{
    trace_t     tr;
    vec3_t      forward, right, up;
    vec3_t      v;
    vec3_t      point;
    float       range;
    vec3_t      dir;

	if (!(self->enemy))
		return qfalse;

    //see if enemy is in range
    VectorSubtract(self->enemy->s.origin, self->s.origin, dir);
    range = VectorLength(dir);
    if (range > aim[0])
        return qfalse;

    if (aim[1] > self->mins[0] && aim[1] < self->maxs[0]) {
        // the hit is straight on so back the range up to the edge of their bbox
        range -= self->enemy->maxs[0];
    } else {
        // this is a side hit so adjust the "right" value out to the edge of their bbox
        if (aim[1] < 0)
            aim[1] = self->enemy->mins[0];
        else
            aim[1] = self->enemy->maxs[0];
    }

    VectorMA(self->s.origin, range, dir, point);

    tr = gi.trace(self->s.origin, NULL, NULL, point, self, MASK_SHOT);
    if (tr.fraction < 1) {
        if (!tr.ent->takedamage)
            return qfalse;
        // if it will hit any client/monster then hit the one we wanted to hit
        if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
            tr.ent = self->enemy;
    }

    AngleVectors(self->s.angles, forward, right, up);
    VectorMA(self->s.origin, range, forward, point);
    VectorMA(point, aim[1], right, point);
    VectorMA(point, aim[2], up, point);
    VectorSubtract(point, self->enemy->s.origin, dir);

    // do the damage
    T_Damage(tr.ent, self, self, dir, point, vec3_origin, damage, kick / 2, DAMAGE_NO_KNOCKBACK, MOD_HIT);

    if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
        return qfalse;

    // do our special form of knockback here
    VectorMA(self->enemy->absmin, 0.5, self->enemy->size, v);
    VectorSubtract(v, point, v);
    VectorNormalize(v);
    VectorMA(self->enemy->velocity, kick, v, self->enemy->velocity);
    if (self->enemy->velocity[2] > 0)
        self->enemy->groundentity = NULL;
    return qtrue;
}


/*
=================
fire_lead

This is an internal support routine used for bullet/pellet based weapons.
=================
*/

static void fire_lead(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
{
    trace_t     tr;
    vec3_t      dir;
    vec3_t      forward, right, up;
    vec3_t      end;
    float       r;
    float       u;
    vec3_t      water_start;
    qboolean    water = qfalse, doinc = qtrue;
    int         content_mask = MASK_SHOT | MASK_WATER;
	qboolean	bouncer = qfalse; // Rroff qfalse if the shot missed

	if (mod == MOD_CHAINGUN && self->client)
	{
		if (self->client->pers.mods & MU_BOUNCER)
		{
			kick = 35;
		}
	}

    tr = gi.trace(self->s.origin, NULL, NULL, start, self, MASK_SHOT);
    if (!(tr.fraction < 1.0)) {
        vectoangles(aimdir, dir);
        AngleVectors(dir, forward, right, up);

        r = crandom() * hspread;
        u = crandom() * vspread;
        VectorMA(start, 8192, forward, end);
        VectorMA(end, r, right, end);
        VectorMA(end, u, up, end);

        if (gi.pointcontents(start) & MASK_WATER) {
            water = qtrue;
            VectorCopy(start, water_start);
            content_mask &= ~MASK_WATER;
        }

        tr = gi.trace(start, NULL, NULL, end, self, content_mask);

        // see if we hit water
        if (tr.contents & MASK_WATER) {
            int     color;

            water = qtrue;
            VectorCopy(tr.endpos, water_start);

            if (!VectorCompare(start, tr.endpos)) {
                if (tr.contents & CONTENTS_WATER) {
                    if (strcmp(tr.surface->name, "*brwater") == 0)
                        color = SPLASH_BROWN_WATER;
                    else
                        color = SPLASH_BLUE_WATER;
                } else if (tr.contents & CONTENTS_SLIME)
                    color = SPLASH_SLIME;
                else if (tr.contents & CONTENTS_LAVA)
                    color = SPLASH_LAVA;
                else
                    color = SPLASH_UNKNOWN;

                if (color != SPLASH_UNKNOWN) {
                    gi.WriteByte(svc_temp_entity);
                    gi.WriteByte(TE_SPLASH);
                    gi.WriteByte(8);
                    gi.WritePosition(tr.endpos);
                    gi.WriteDir(tr.plane.normal);
                    gi.WriteByte(color);
                    gi.multicast(tr.endpos, MULTICAST_PVS);
                }

                // change bullet's course when it enters water
                VectorSubtract(end, start, dir);
                vectoangles(dir, dir);
                AngleVectors(dir, forward, right, up);
                r = crandom() * hspread * 2;
                u = crandom() * vspread * 2;
                VectorMA(water_start, 8192, forward, end);
                VectorMA(end, r, right, end);
                VectorMA(end, u, up, end);
            }

            // re-trace ignoring water this time
            tr = gi.trace(water_start, NULL, NULL, end, self, MASK_SHOT);
        }
    }

    // send gun puff / flash
    if (!((tr.surface) && (tr.surface->flags & SURF_SKY))) {
        if (tr.fraction < 1.0) {
            if (tr.ent->takedamage) {
				bouncer = qtrue;
				if ((mod == MOD_SHOTGUN || mod == MOD_SABOT) && self->client && tr.ent->svflags & SVF_MONSTER)
				{
					if (self->client->pers.player_class & PC_CYBORG)
					{
						if (tr.ent->monsterinfo.dot_time[DOT_CYBORG] > level.time)
							damage *= 1.15;
						if (tr.ent->monsterinfo.dot_time[DOT_CYBORG] < level.time + 10)
							tr.ent->monsterinfo.dot_time[DOT_CYBORG] = level.time + 10;
						tr.ent->monsterinfo.dot_amt[DOT_CYBORG] = 2;
						tr.ent->monsterinfo.dot_ent = self;
					}
				}
				if (mod == MOD_INCENDIARY)
				{
					T_Damage(tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, (DAMAGE_BULLET | DAMAGE_INCENDIARY), mod);

					if (tr.ent->svflags & SVF_MONSTER)
					{
						if (!tr.ent->monsterinfo.armoured)
						{
							// Incendiary always overwrites index 0 - multiple players won't
							// contribute additional dot from this effect
							if (tr.ent->health > 0)
							{
								if (tr.ent->monsterinfo.dot_time[DOT_INCENDIARY] < level.time + 5)
									tr.ent->monsterinfo.dot_time[DOT_INCENDIARY] = level.time + 5;
								tr.ent->monsterinfo.dot_amt[DOT_INCENDIARY] = 2;
							}
						} else {
							doinc = qfalse;
						}
					}

					if (doinc)
					{
						gi.WriteByte(svc_temp_entity);
						gi.WriteByte(TE_HEATBEAM_SPARKS);
						gi.WritePosition(tr.endpos);
						gi.WriteDir(tr.plane.normal);
						gi.multicast(tr.endpos, MULTICAST_PVS);
					}
				} else if(mod == MOD_SABOT) {
					T_Damage(tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, (DAMAGE_BULLET | DAMAGE_AP), mod);
				} else {
					T_Damage(tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
				}
            } else {
                if (strncmp(tr.surface->name, "sky", 3) != 0) {
					if (mod == MOD_INCENDIARY)
					{
						gi.WriteByte(svc_temp_entity);
						gi.WriteByte(TE_SPARKS);
						gi.WritePosition(tr.endpos);
						gi.WriteDir(tr.plane.normal);
						gi.multicast(tr.endpos, MULTICAST_PVS);

						te_impact = TE_HEATBEAM_SPARKS;
					}

					if (mod == MOD_SABOT)
						te_impact = TE_SPARKS;

                    gi.WriteByte(svc_temp_entity);
                    gi.WriteByte(te_impact);
                    gi.WritePosition(tr.endpos);
                    gi.WriteDir(tr.plane.normal);
                    gi.multicast(tr.endpos, MULTICAST_PVS);

					if (g_sillygore->value)
					{
						if (!((tr.ent->solid == SOLID_BSP) && (tr.ent->s.modelindex != 1)))
						{
							if (mod == MOD_PTURRET || mod == MOD_CHAINGUN || mod == MOD_SSHOTGUN)
							{
								if (random() < 0.4)
									silly_impact(tr.endpos, tr.plane.normal, 0);
							}
							else {
								silly_impact(tr.endpos, tr.plane.normal, 0);
							}
						}
					}

                    if (self->client)
                        PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
                }
            }
        }
    }

	if (mod == MOD_SABOT)
	{
		vec3_t	end2, pos;

		if (water)
		{
			VectorCopy(water_start, end2);
		}
		else {
			VectorCopy(tr.endpos, end2);
		}

		VectorAdd(start, end2, pos);
		VectorScale(pos, 0.5, pos);

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BUBBLETRAIL);
		gi.WritePosition(start);
		gi.WritePosition(end2);
		gi.multicast(pos, MULTICAST_PVS);
	}

	// later make it random with low chance and only if sillygore is on

	if ((mod == MOD_CHAINGUN || mod == MOD_PTURRET || mod == MOD_GRUNT || mod == MOD_INCENDIARY) && (random() < 0.3) && g_sillygore->value)
	{
		vec3_t		end2, pos, v;
		edict_t		*tracer;
		float		position;

		if (water)
		{
			VectorCopy(water_start, end2);
		}
		else {
			VectorCopy(tr.endpos, end2);
		}

		VectorSubtract(end2, start, v);

		if (VectorLength(v) > 96)
		{
			VectorCopy(v, pos);
			VectorNormalize(v);
			vectoangles(v, v);
			AngleVectors(v, forward, NULL, NULL);

			position = 0.05 + (random() * 0.8);

			VectorScale(pos, position, pos);
			VectorAdd(start, pos, pos);

			tracer = G_SpawnNC();
			tracer->solid = SOLID_NOT;
			tracer->movetype = MOVETYPE_FLY;
			tracer->svflags |= SVF_DEADMONSTER;
			tracer->s.modelindex = gi.modelindex("models/proj/tracer/tris.md2");
			tracer->s.renderfx |= RF_FULLBRIGHT;
			tracer->s.effects |= EF_IONRIPPER;
			tracer->nextthink = level.time + FRAMETIME;
			tracer->think = G_FreeEdict;
			tracer->classname = "tracer";

			VectorCopy(pos, tracer->s.origin);
			vectoangles(forward, tracer->s.angles);
			VectorScale(forward, 600, tracer->velocity);

			gi.linkentity(tracer);
		}
	}

    // if went through water, determine where the end and make a bubble trail
    if (water) {
        vec3_t  pos;

        VectorSubtract(tr.endpos, water_start, dir);
        VectorNormalize(dir);
        VectorMA(tr.endpos, -2, dir, pos);
        if (gi.pointcontents(pos) & MASK_WATER)
            VectorCopy(pos, tr.endpos);
        else
            tr = gi.trace(pos, NULL, NULL, water_start, tr.ent, MASK_WATER);

        VectorAdd(water_start, tr.endpos, pos);
        VectorScale(pos, 0.5, pos);

        gi.WriteByte(svc_temp_entity);
        gi.WriteByte(TE_BUBBLETRAIL);
        gi.WritePosition(water_start);
        gi.WritePosition(tr.endpos);
        gi.multicast(pos, MULTICAST_PVS);
    }

	// if (ent->client->buttons & BUTTON_ATTACK)
	// Rroff - only count shots while the player is pressing fire?
	// and ignore spin down misses? idea is to award player for accuracy
	// with the chaingun LOL so timing counts as well

	if (mod == MOD_CHAINGUN && self->client)
	{
		if (self->client->pers.mods & MU_BOUNCER)
		{
			if (bouncer)
			{
				if (self->client->bouncer_framenum <= level.framenum + 150)
				{
					if (self->client->bouncer_framenum > level.framenum)
						self->client->bouncer_framenum += 30;
					else
						self->client->bouncer_framenum = level.framenum + 30;

					gi.sound(self, CHAN_ITEM, gi.soundindex("items/protect4.wav"), 1, ATTN_NORM, 0);
				}
				if (self->health < (self->max_health * 1.5))
					self->health += 1;
			} else {
				if (self->client->bouncer_framenum > level.framenum)
					self->client->bouncer_framenum -= 10;

				if (self->health >= 102)
				{
					self->health -= 2;
				}
				else if (self->health == 101)
					self->health -= 1;

				// pain effects get a bit annoying but better way to take health
				//T_Damage(self, world, world, vec3_origin, self->s.origin, vec3_origin, 1, 0, (DAMAGE_NO_ARMOR | DAMAGE_NO_KILL), MOD_UNKNOWN);
			}
		}
	}
}


/*
=================
fire_bullet

Fires a single round.  Used for machinegun and chaingun.  Would be fine for
pistols, rifles, etc....
=================
*/
void fire_bullet(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod)
{
    fire_lead(self, start, aimdir, damage, kick, TE_GUNSHOT, hspread, vspread, mod);
}


/*
=================
fire_shotgun

Shoots shotgun pellets.  Used by shotgun and super shotgun.
=================
*/
void fire_shotgun(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod)
{
    int     i;

	if (self->client)
	{
		if (mod == MOD_SABOT)
		{
			fire_lead(self, start, aimdir, damage, kick, TE_SHOTGUN, hspread, vspread, mod);
			return;
		}
	}

    for (i = 0; i < count; i++)
        fire_lead(self, start, aimdir, damage, kick, TE_SHOTGUN, hspread, vspread, mod);
}


/*
=================
fire_blaster

Fires a single blaster bolt.  Used by the blaster and hyper blaster.
=================
*/
void blaster_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
    int     mod;

	vec3_t		normal_angles, forward, right, up;
	vec3_t		offset, pos;
	qboolean	small;
	edict_t		*temp;

	trace_t		tr;

    if (other == self->owner)
        return;

    if (surf && (surf->flags & SURF_SKY)) {
        G_FreeEdict(self);
        return;
    }

	if (g_sillygore->value && !other->takedamage && (g_nodecals->value == 0))
	{
		if (plane && !((other->solid == SOLID_BSP) && (other->s.modelindex != 1)))
		{
			small = qfalse;

			vectoangles(plane->normal, normal_angles);
			AngleVectors(normal_angles, forward, right, up);

			VectorCopy(self->s.origin, pos);
			VectorMA(pos, -8, right, pos);
			VectorMA(pos, 4, up, pos);
			VectorMA(pos, -0.2, forward, pos);

			tr = gi.trace(pos, NULL, NULL, self->s.origin, self, MASK_SOLID);

			if (!tr.startsolid)
				small = qtrue;

			if (small == qfalse)
			{
				VectorCopy(self->s.origin, pos);
				VectorMA(pos, 6, right, pos);
				VectorMA(pos, 7, up, pos);
				VectorMA(pos, -0.2, forward, pos);

				tr = gi.trace(pos, NULL, NULL, self->s.origin, self, MASK_SOLID);

				if (!tr.startsolid)
					small = qtrue;
			}

			if (small == qfalse)
			{
				VectorCopy(self->s.origin, pos);
				VectorMA(pos, -1, right, pos);
				VectorMA(pos, -8, up, pos);
				VectorMA(pos, -0.2, forward, pos);

				tr = gi.trace(pos, NULL, NULL, self->s.origin, self, MASK_SOLID);

				if (!tr.startsolid)
					small = qtrue;
			}

			if (!small)
			{
				temp = G_SpawnNC();
				//temp->s.modelindex = gi.modelindex("models/objects/impacts/blaster.md2");
				temp->s.modelindex = gi.modelindex("models/objects/impacts/tris.md2");
				temp->s.skinnum = 3;
				temp->movetype = MOVETYPE_NONE;
				vectoangles(forward, temp->s.angles);
				temp->s.angles[ROLL] = rand() % 360;
				VectorCopy(self->s.origin, temp->s.origin);
				temp->s.renderfx |= RF_TRANSLUCENT | 0x80000000;
				temp->think = G_FreeEdict;
				temp->nextthink = level.time + 999;

				gi.linkentity(temp);
			}
		}
	}

    if (self->owner->client)
        PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

    if (other->takedamage) {
        if (self->spawnflags & 1)
            mod = MOD_HYPERBLASTER;
        else
            mod = MOD_BLASTER;
        T_Damage(other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
    } else {
        gi.WriteByte(svc_temp_entity);
        gi.WriteByte(TE_BLASTER);
        gi.WritePosition(self->s.origin);
        if (!plane)
            gi.WriteDir(vec3_origin);
        else
            gi.WriteDir(plane->normal);
        gi.multicast(self->s.origin, MULTICAST_PVS);
    }

    G_FreeEdict(self);
}

void fire_blaster(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper)
{
    edict_t *bolt;
    trace_t tr;

    VectorNormalize(dir);

    bolt = G_Spawn();
    bolt->svflags = SVF_DEADMONSTER;
    // yes, I know it looks weird that projectiles are deadmonsters
    // what this means is that when prediction is used against the object
    // (blaster/hyperblaster shots), the player won't be solid clipped against
    // the object.  Right now trying to run into a firing hyperblaster
    // is very jerky since you are predicted 'against' the shots.
    VectorCopy(start, bolt->s.origin);
    VectorCopy(start, bolt->s.old_origin);
    vectoangles(dir, bolt->s.angles);
    VectorScale(dir, speed, bolt->velocity);
    bolt->movetype = MOVETYPE_FLYMISSILE;
    bolt->clipmask = MASK_SHOT;
    bolt->solid = SOLID_BBOX;
    bolt->s.effects |= effect;
    VectorClear(bolt->mins);
    VectorClear(bolt->maxs);
    bolt->s.modelindex = gi.modelindex("models/objects/laser/tris.md2");
    bolt->s.sound = gi.soundindex("misc/lasfly.wav");
    bolt->owner = self;
    bolt->touch = blaster_touch;
    bolt->nextthink = level.time + 2;
    bolt->think = G_FreeEdict;
    bolt->dmg = damage;
    bolt->classname = "bolt";
    if (hyper)
        bolt->spawnflags = 1;
    gi.linkentity(bolt);

    if (self->client)
        check_dodge(self, bolt->s.origin, dir, speed);

    tr = gi.trace(self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
    if (tr.fraction < 1.0) {
        VectorMA(bolt->s.origin, -10, dir, bolt->s.origin);
        bolt->touch(bolt, tr.ent, NULL, NULL);
    }
}


/*
=================
fire_grenade
=================
*/

void Grenade_Explode(edict_t *ent)
{
    vec3_t      origin;
    int         mod;

    //if (ent->owner->client)
    //   PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

    //FIXME: if we are onground then raise our Z just a bit since we are a point?
    if (ent->enemy) {
        float   points;
        vec3_t  v;
        vec3_t  dir;

        VectorAdd(ent->enemy->mins, ent->enemy->maxs, v);
        VectorMA(ent->enemy->s.origin, 0.5, v, v);
        VectorSubtract(ent->s.origin, v, v);
        points = ent->dmg - 0.5 * VectorLength(v);
        VectorSubtract(ent->enemy->s.origin, ent->s.origin, dir);
        if (ent->spawnflags & 1)
            mod = MOD_HANDGRENADE;
        else
            mod = MOD_GRENADE;

		/*if (ent->owner->client)
		{
			if (ent->owner->client->pers.mods & MU_GL)
				mod = MOD_GL2;
		}*/

		if (ent->moded & MU_GL)
			mod = MOD_GL2;

        T_Damage(ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
    }

    if (ent->spawnflags & 2)
        mod = MOD_HELD_GRENADE;
    else if (ent->spawnflags & 1)
        mod = MOD_HG_SPLASH;
    else
        mod = MOD_G_SPLASH;

	if (ent->owner->client)
	{
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

		//if (ent->owner->client->pers.mods & MU_GL)
		if (ent->moded & MU_GL)
			mod = MOD_GL2;
	}

    T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, mod);

    VectorMA(ent->s.origin, -0.02, ent->velocity, origin);
    gi.WriteByte(svc_temp_entity);
    if (ent->waterlevel) {
        if (ent->groundentity)
            gi.WriteByte(TE_GRENADE_EXPLOSION_WATER);
        else
            gi.WriteByte(TE_ROCKET_EXPLOSION_WATER);
    } else {
        if (ent->groundentity)
            gi.WriteByte(TE_GRENADE_EXPLOSION);
        else
            gi.WriteByte(TE_ROCKET_EXPLOSION);
    }
    gi.WritePosition(origin);
    gi.multicast(ent->s.origin, MULTICAST_PHS);

	// Rroff an additional effect would be good
	//if (mod == MOD_GL2)
	//{
	//}

    G_FreeEdict(ent);
}

void Trip_Die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;

	if (self->owner->client)
	{
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);
	}

	T_RadiusDamage(self, attacker, self->dmg, NULL, self->dmg_radius, MOD_TRIPMINE);

	gi.WriteByte(svc_temp_entity);
	if (self->waterlevel) {
		if (self->groundentity)
			gi.WriteByte(TE_GRENADE_EXPLOSION_WATER);
		else
			gi.WriteByte(TE_ROCKET_EXPLOSION_WATER);
	}
	else {
		if (self->groundentity)
			gi.WriteByte(TE_GRENADE_EXPLOSION);
		else
			gi.WriteByte(TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition(self->s.origin);
	gi.multicast(self->s.origin, MULTICAST_PHS);

	G_FreeEdict(self);
}

edict_t *grenade_FindNearestMonster(edict_t *self)
{
	edict_t		*ent = NULL;
	edict_t		*best = NULL;
	edict_t		*pref = NULL;

	vec3_t		v;
	float		dist1, dist2=999999, nearest = 999999;

	while ((ent = findradius(ent, self->s.origin, 1024)) != NULL) {
		if (ent == self)
			continue;
		if (!(ent->svflags & SVF_MONSTER))
			continue;
		if (ent->flags & FL_FLY)
			continue;
		if (ent->monsterinfo.aiflags & AI_GOOD_GUY)
			continue;
		if (ent->owner)
			continue;
		if (ent->health <= 0)
			continue;
		if (!visible(self, ent))
			continue;
		VectorSubtract(ent->s.origin, self->s.origin, v);
		dist1 = VectorLength(v);

		if (self->owner)
		{
			if (self->owner->client)
			{
				if (infront(self->owner, ent))
				{
					if (dist1 < dist2)
					{
						pref = ent;
						dist2 = dist1;
					}
				}
			}
		}

		if (dist1 > nearest)
			continue;

		if (!best) {
			best = ent;
			nearest = dist1;
			continue;
		}
		best = ent;
		nearest = dist1;
	}

	if (pref)
		return pref;

	return best;
}

void Trip_Think(edict_t *self)
{
	vec3_t		start, end, forward, mins, maxs;
	trace_t		tr;

	if (level.time > (self->wait + 120))
	{
		Grenade_Explode(self);
		return;
	}

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->s.origin, 200, forward, end);

	VectorSet(mins, -4, -4, -4);
	VectorSet(maxs, 4, 4, 4);

	tr = gi.trace(self->s.origin, mins, maxs, end, self, MASK_MONSTERSOLID);

	if (tr.ent)
	{
		if ((tr.ent->svflags & SVF_MONSTER) && (tr.ent->health > 0))
		{
			Grenade_Explode(self);
			return;
		}
	}

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_BFG_LASER);
	gi.WritePosition(self->s.origin);
	gi.WritePosition(tr.endpos);
	gi.multicast(self->s.origin, MULTICAST_PHS);

	/*VectorSet(offset, 0, 0, 0);

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_GRAPPLE_CABLE);
	gi.WriteShort(self - g_edicts);
	gi.WritePosition(self->s.origin);
	gi.WritePosition(tr.endpos);
	gi.WritePosition(offset);
	gi.multicast(self->s.origin, MULTICAST_PHS);*/

	self->nextthink = level.time + FRAMETIME;
}

void grenade_think(edict_t *self)
{
	vec3_t		dir, forward, right, start, end;
	float		dist, h, vangle;
	trace_t		tr;
	edict_t		*ent;
	int			n;

	if (self->owner)
	{
		if (self->owner->client)
		{
			if (!self->owner->client->pers.mods & MU_GRENADE)
			{
				Grenade_Explode(self);
				return;
			}

		}
	}

	// Rroff - make it so we can have a limited number following the player?
	if (level.time > (self->wait + 15))
	{
		// If following player don't explode and hurt them
		if (self->following_mode)
		{
			//self->s.modelindex = gi.modelindex("sprites/s_bfg3.sp2");
			//self->s.effects &= ~EF_ANIM_ALLFAST;

			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_BFG_BIGEXPLOSION);
			gi.WritePosition(self->s.origin);
			gi.multicast(self->s.origin, MULTICAST_PVS);

			gi.sound(self, CHAN_VOICE, gi.soundindex("floater/fltatck1.wav"), 1, ATTN_NORM, 0);

			n = rand() % 5;
			while (n--)
				ThrowDebris(self, "models/objects/debris2/tris.md2", 2, self->s.origin);

			G_FreeEdict(self);
			return;
		} else {
			self->enemy = NULL;
			Grenade_Explode(self);
			return;
		}
	}

	// should also check visibility and also not extend indefinitely
	//if (self->following_mode == qtrue) // extend operation while following player
	//	self->wait = level.time;

	// some of the time resets on finding a new target below could extend the life
	// of the grenade indefinitely if it keeps bouncing from finding and losing targets

	self->following_mode = qfalse;

	if (self->movetype != MOVETYPE_STEP)
	{
		self->nextthink = level.time + FRAMETIME;
		return;
	}

	if (!self->enemy)
	{
		self->enemy = grenade_FindNearestMonster(self);

		if (self->enemy)
		{
			self->wait = level.time;
			gi.sound(self, CHAN_VOICE, gi.soundindex("misc/comp_up.wav"), 1, ATTN_NORM, 0);
		}

		if (!self->enemy)
		{
			// do we need to make sure owner is valid?
			if (visible(self, self->owner))
			{
				self->following_mode = qtrue;
				VectorSubtract(self->owner->s.origin, self->s.origin, dir);
				vectoangles(dir, dir);
				AnglesNormalize(dir);

				self->ideal_yaw = dir[YAW];

				if (anglemod(self->s.angles[YAW]) != anglemod(self->ideal_yaw))
					M_ChangeYaw(self);

				self->movedir[YAW] = self->s.angles[YAW];
			} else {
				if (level.time > self->random + 3)
				{
					self->random = level.time;
					vangle = self->s.angles[YAW] - 180;
					vangle = vangle + (-120 + (random() * 240));

					if (vangle < 0)
						vangle = 360 + vangle;

					if (vangle > 360)
						vangle = vangle - 360;

					self->ideal_yaw = vangle;
				}
			}
		}
	}

	if (self->enemy)
	{
		if (self->enemy->health <= 0)
		{
			self->nextthink = level.time + FRAMETIME;
			self->enemy = grenade_FindNearestMonster(self);
			if (self->enemy)
			{
				self->wait = level.time;
				gi.sound(self, CHAN_VOICE, gi.soundindex("misc/comp_up.wav"), 1, ATTN_NORM, 0);
			}
			return;
		}

		VectorSubtract(self->enemy->s.origin, self->s.origin, dir);
		
		// Rroff when moving on the ground we no long have a working touch function
		// so do it by proximity

		//if (self->movetype == MOVETYPE_STEP)
		//{
			dist = VectorLength(dir);
			h = self->enemy->maxs[2];
			if (dist < (h + 12))
			{
				Grenade_Explode(self);
				return;
			}
		//}

		if (!visible(self, self->enemy))
		{
			if (level.time > self->last_time + 5)
			{
				ent = grenade_FindNearestMonster(self);
				if (ent)
				{
					// Rroff move rethinks to top of function as a single case?
					gi.sound(self, CHAN_VOICE, gi.soundindex("misc/comp_up.wav"), 1, ATTN_NORM, 0);
					self->wait = level.time;
					self->nextthink = level.time + FRAMETIME;
					self->enemy = ent;
					return;
				}
			}

			if (level.time > self->random + 3)
			{
				self->random = level.time;
				vangle = self->s.angles[YAW] - 180;
				vangle = vangle + (-120 + (random() * 240));

				if (vangle < 0)
					vangle = 360 + vangle;

				if (vangle > 360)
					vangle = vangle - 360;

				self->ideal_yaw = vangle;
			}
		} else {

			self->last_time = level.time;

			vectoangles(dir, dir);
			AnglesNormalize(dir);

			self->ideal_yaw = dir[YAW];

			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_BFG_LASER);
			gi.WritePosition(self->s.origin);
			gi.WritePosition(self->enemy->s.origin);
			gi.multicast(self->s.origin, MULTICAST_PHS);

			/*gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_MONSTER_HEATBEAM);
			gi.WriteShort(self - g_edicts);
			gi.WritePosition(self->s.origin);
			gi.WritePosition(self->enemy->s.origin);
			gi.multicast(self->s.origin, MULTICAST_PVS);*/


		}

		if (anglemod(self->s.angles[YAW]) != anglemod(self->ideal_yaw))
			M_ChangeYaw(self);

		self->movedir[YAW] = self->s.angles[YAW];

		/*gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BFG_LASER);
		gi.WritePosition(self->s.origin);
		gi.WritePosition(self->enemy->s.origin);
		gi.multicast(self->s.origin, MULTICAST_PHS);*/

	}

	AngleVectors(self->movedir, forward, right, NULL);

	
	if (self->groundentity)
	{
		//AngleVectors(self->s.angles, forward, right, up);

		//VectorScale(forward, 600, self->velocity);
		VectorScale(forward, 700, self->velocity);

		//self->velocity[2] = 200;

		VectorMA(self->s.origin, 5, forward, end);

		// tends to bounce on the spot if we don't manage to clear the obstruction

		tr = gi.trace(self->s.origin, vec3_origin, vec3_origin, end, self, MASK_SHOT);
		if (tr.fraction != 1.0)
		{
			self->delay = level.time + 0.5;
			//self->velocity[2] += 600;
			self->velocity[2] = 600;
		} else {
			VectorCopy(tr.endpos, end);
			end[2] -= 18;
			tr = gi.trace(tr.endpos, vec3_origin, vec3_origin, end, self, MASK_SHOT);
			// Rroff crude test to see if we would go over an edge
			// for now just jump and hope for the best ideally would try a new chase dir
			// like monsters but that is a lot of work and need to flag direct or indirect
			// routing instead of just updating ideal_yaw
			if (tr.fraction == 1.0)
			{
				self->velocity[2] = 300;
			}
		}

		// horrid way to do it
		// but avelocity having too many issues

		//self->s.angles[PITCH] -= 200;
		self->s.angles[PITCH] -= 200;


		//VectorScale(self->movedir, 200, self->velocity);
	} else {
		if (level.time < self->delay)
			VectorScale(forward, 400, self->velocity);
	}

	// crude way to not get caught in water
	// seems to be causing some issues
	if (gi.pointcontents(self->s.origin) & MASK_WATER)
		self->velocity[2] = 200;

	self->nextthink = level.time + FRAMETIME;
}

void Grenade_Touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		normal_angles, forward;

    if (other == ent->owner)
        return;

    if (surf && (surf->flags & SURF_SKY)) {
        G_FreeEdict(ent);
        return;
    }

	if (ent->moded == MU_GRENADE)
	{
		// reduce the amount of noise
		if (!other->takedamage && ent->count != 1) {
			return;
		}

		if (ent->count == 1)
		{
			ent->count = 0;
			ent->enemy = grenade_FindNearestMonster(ent);
			if (ent->enemy)
				gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/comp_up.wav"), 1, ATTN_NORM, 0);
			ent->movetype = MOVETYPE_STEP;
		}
	}

	// shouldn't have more than one mod type active
	if (ent->moded == MU_TRIP)
	{
		if (plane && !other->takedamage && !((other->solid == SOLID_BSP) && (other->s.modelindex != 1)))
		{
			vectoangles(plane->normal, normal_angles);
			AngleVectors(normal_angles, forward, NULL, NULL);
			vectoangles(forward, ent->s.angles);

			ent->movetype = MOVETYPE_NONE;
			ent->touch = NULL;
			ent->wait = level.time + 120;
			ent->dmg_radius *= 2;

			ent->nextthink = level.time + FRAMETIME;
			ent->think = Trip_Think;

			return;
		}
	}

    if (!other->takedamage) {
        if (ent->spawnflags & 1) {
            if (random() > 0.5)
                gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
            else
                gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
        } else {
            gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
        }
        return;
    }

    ent->enemy = other;
    Grenade_Explode(ent);
}

void fire_grenade(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
    edict_t		*grenade;
    vec3_t		dir;
    vec3_t		forward, right, up;

    vectoangles(aimdir, dir);
    AngleVectors(dir, forward, right, up);

    grenade = G_Spawn();
    VectorCopy(start, grenade->s.origin);
  
	grenade->movetype = MOVETYPE_BOUNCE;
	
	if (self->client)
	{
		if (self->client->pers.mods & MU_GL)
		{
			grenade->gravity = 0.65;
			speed *= 1.3;
			damage_radius *= 1.5;
			grenade->moded = MU_GL;
			//grenade->movetype = MOVETYPE_TOSS;
		}
	}

	VectorScale(aimdir, speed, grenade->velocity);
    VectorMA(grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
    VectorMA(grenade->velocity, crandom() * 10.0, right, grenade->velocity);
    VectorSet(grenade->avelocity, 300, 300, 300);
    
    grenade->clipmask = MASK_SHOT;
    grenade->solid = SOLID_BBOX;
    grenade->s.effects |= EF_GRENADE;
    VectorClear(grenade->mins);
    VectorClear(grenade->maxs);
    grenade->s.modelindex = gi.modelindex("models/objects/grenade/tris.md2");
    grenade->owner = self;
    grenade->touch = Grenade_Touch;
    grenade->nextthink = level.time + timer;
    grenade->think = Grenade_Explode;
    grenade->dmg = damage;
    grenade->dmg_radius = damage_radius;
    grenade->classname = "grenade";
	grenade->projectile = qtrue;
	grenade->timestamp = level.time;

    gi.linkentity(grenade);
}

void fire_grenade2(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
    edict_t		*grenade;
    vec3_t		dir;
    vec3_t		forward, right, up;
	qboolean	modded = qfalse;

	// don't allow more than one active mod type

	if (self->client)
	{
		if (self->client->pers.mods & MU_GRENADE)
		{
			modded = qtrue;
		}

		if (self->client->pers.mods & MU_TRIP)
		{
			modded = qtrue;
		}
	}

    vectoangles(aimdir, dir);
    AngleVectors(dir, forward, right, up);

    grenade = G_Spawn();
    VectorCopy(start, grenade->s.origin);
    VectorScale(aimdir, speed, grenade->velocity);

    VectorMA(grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
    VectorMA(grenade->velocity, crandom() * 10.0, right, grenade->velocity);

	if (modded)
	{
		if (self->client->pers.mods & MU_GRENADE)
		{
			VectorCopy(dir, grenade->s.angles);
			grenade->yaw_speed = 20;
			grenade->s.angles[ROLL] = 90;
			VectorCopy(dir, grenade->movedir);
		}
	} else {
		VectorSet(grenade->avelocity, 300, 300, 300);
	}

    grenade->clipmask = MASK_SHOT;
    grenade->solid = SOLID_BBOX;
    grenade->s.effects |= EF_GRENADE;
    VectorClear(grenade->mins);
    VectorClear(grenade->maxs);
    grenade->s.modelindex = gi.modelindex("models/objects/grenade2/tris.md2");
    grenade->owner = self;
    grenade->touch = Grenade_Touch;

	if (modded)
	{
		if (self->client->pers.mods & MU_GRENADE)
		{
			VectorSet(grenade->mins, -4, -4, -4);
			VectorSet(grenade->maxs, 4, 4, 4);
			grenade->nextthink = level.time + FRAMETIME;
			grenade->think = grenade_think;
			grenade->wait = level.time;
			grenade->count = 1;
			//grenade->s.sound = gi.soundindex("boss3/w_loop.wav"); // need a better sound
			grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");
			grenade->movetype = MOVETYPE_BOUNCE;
			grenade->moded = MU_GRENADE;
		}

		if (self->client->pers.mods & MU_TRIP)
		{
			VectorSet(grenade->mins, -2, -2, -2);
			VectorSet(grenade->maxs, 2, 2, 2);

			grenade->nextthink = level.time + timer;
			grenade->think = Grenade_Explode;
			grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");
			grenade->movetype = MOVETYPE_TOSS;
			grenade->moded = MU_TRIP;
			grenade->takedamage = qtrue;
			grenade->health = 15;
			grenade->die = Trip_Die;
		}
	} else {
		grenade->nextthink = level.time + timer;
		grenade->think = Grenade_Explode;
		grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");
		grenade->movetype = MOVETYPE_BOUNCE;
	}

    grenade->dmg = damage;
    grenade->dmg_radius = damage_radius;
    grenade->classname = "hgrenade";
	grenade->projectile = qtrue;
	grenade->timestamp = level.time;

    if (held)
        grenade->spawnflags = 3;
    else
        grenade->spawnflags = 1;


    if (timer <= 0.0)
        Grenade_Explode(grenade);
    else {
        gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
        gi.linkentity(grenade);
    }
}


/*
=================
fire_rocket
=================
*/
void rocket_touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
    vec3_t      origin;
    int         n;

	vec3_t		normal_angles, forward, right, up;
	vec3_t		offset, pos;
	qboolean	small;
	edict_t		*temp;

	trace_t		tr;

    if (other == ent->owner)
        return;

    if (surf && (surf->flags & SURF_SKY)) {
        G_FreeEdict(ent);
        return;
    }

	if (g_sillygore->value && !other->takedamage && (g_nodecals->value == 0))
	{
		if (plane && !((other->solid == SOLID_BSP) && (other->s.modelindex != 1)))
		{
			small = qfalse;

			vectoangles(plane->normal, normal_angles);
			AngleVectors(normal_angles, forward, right, up);

			VectorCopy(ent->s.origin, pos);
			VectorMA(pos, -8, right, pos);
			VectorMA(pos, 4, up, pos);
			VectorMA(pos, -0.2, forward, pos);

			tr = gi.trace(pos, NULL, NULL, ent->s.origin, ent, MASK_SOLID);

			if (!tr.startsolid)
				small = qtrue;

			if (small == qfalse)
			{
				VectorCopy(ent->s.origin, pos);
				VectorMA(pos, 6, right, pos);
				VectorMA(pos, 7, up, pos);
				VectorMA(pos, -0.2, forward, pos);

				tr = gi.trace(pos, NULL, NULL, ent->s.origin, ent, MASK_SOLID);

				if (!tr.startsolid)
					small = qtrue;
			}

			if (small == qfalse)
			{
				VectorCopy(ent->s.origin, pos);
				VectorMA(pos, -1, right, pos);
				VectorMA(pos, -8, up, pos);
				VectorMA(pos, -0.2, forward, pos);

				tr = gi.trace(pos, NULL, NULL, ent->s.origin, ent, MASK_SOLID);

				if (!tr.startsolid)
					small = qtrue;
			}

			if (!small)
			{
				temp = G_SpawnNC();
				//temp->s.modelindex = gi.modelindex("models/objects/splatter/tris2.md2");
				temp->s.modelindex = gi.modelindex("models/objects/impacts/tris.md2");
				temp->s.skinnum = 4;
				temp->movetype = MOVETYPE_NONE;
				vectoangles(forward, temp->s.angles);
				temp->s.angles[ROLL] = rand() % 360;
				VectorCopy(ent->s.origin, temp->s.origin);
				temp->s.renderfx |= RF_TRANSLUCENT | 0x80000000;
				temp->think = G_FreeEdict;
				temp->nextthink = level.time + 999;

				gi.linkentity(temp);
			}
		}
	}

    if (ent->owner->client)
        PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

    // calculate position for the explosion entity
    VectorMA(ent->s.origin, -0.02, ent->velocity, origin);

    if (other->takedamage) {
        T_Damage(other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
    } else {
        // don't throw any debris in net games
        if (!deathmatch->value && !coop->value) {
            if ((surf) && !(surf->flags & (SURF_WARP | SURF_TRANS33 | SURF_TRANS66 | SURF_FLOWING))) {
                n = rand() % 5;
                while (n--)
                    ThrowDebris(ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
            }
        }
    }

    T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);

    gi.WriteByte(svc_temp_entity);
    if (ent->waterlevel)
        gi.WriteByte(TE_ROCKET_EXPLOSION_WATER);
    else
        gi.WriteByte(TE_ROCKET_EXPLOSION);
    gi.WritePosition(origin);
    gi.multicast(ent->s.origin, MULTICAST_PHS);

    G_FreeEdict(ent);
}

void fire_rocket(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
    edict_t *rocket;

    rocket = G_Spawn();
    VectorCopy(start, rocket->s.origin);
    VectorCopy(dir, rocket->movedir);
    vectoangles(dir, rocket->s.angles);
    VectorScale(dir, speed, rocket->velocity);
    rocket->movetype = MOVETYPE_FLYMISSILE;
    rocket->clipmask = MASK_SHOT;
    rocket->solid = SOLID_BBOX;
    rocket->s.effects |= EF_ROCKET;
    VectorClear(rocket->mins);
    VectorClear(rocket->maxs);
    rocket->s.modelindex = gi.modelindex("models/objects/rocket/tris.md2");
    rocket->owner = self;
    rocket->touch = rocket_touch;
    rocket->nextthink = level.time + 8000 / speed;
    rocket->think = G_FreeEdict;
    rocket->dmg = damage;
    rocket->radius_dmg = radius_damage;
    rocket->dmg_radius = damage_radius;
    rocket->s.sound = gi.soundindex("weapons/rockfly.wav");
    rocket->classname = "rocket";
	rocket->projectile = qtrue;
	rocket->timestamp = level.time;

    if (self->client)
        check_dodge(self, rocket->s.origin, dir, speed);

    gi.linkentity(rocket);
}


/*
=================
fire_rail
=================
*/
void fire_rail(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick)
{
    vec3_t      from;
    vec3_t      end;
    trace_t     tr;
    edict_t     *ignore;
    int         mask;
    qboolean    water;

    VectorMA(start, 8192, aimdir, end);
    VectorCopy(start, from);
    ignore = self;
    water = qfalse;
    mask = MASK_SHOT | CONTENTS_SLIME | CONTENTS_LAVA;
    while (ignore) {
        tr = gi.trace(from, NULL, NULL, end, ignore, mask);

        if (tr.contents & (CONTENTS_SLIME | CONTENTS_LAVA)) {
            mask &= ~(CONTENTS_SLIME | CONTENTS_LAVA);
            water = qtrue;
        } else {
            //ZOID--added so rail goes through SOLID_BBOX entities (gibs, etc)
            if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client) ||
                (tr.ent->solid == SOLID_BBOX))
                ignore = tr.ent;
            else
                ignore = NULL;

            if ((tr.ent != self) && (tr.ent->takedamage))
                T_Damage(tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_RAILGUN);
        }

        VectorCopy(tr.endpos, from);
    }

    // send gun puff / flash
    gi.WriteByte(svc_temp_entity);
    gi.WriteByte(TE_RAILTRAIL);
    gi.WritePosition(start);
    gi.WritePosition(tr.endpos);
    gi.multicast(self->s.origin, MULTICAST_PHS);
//  gi.multicast (start, MULTICAST_PHS);

	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (g_sillygore->value && tr.ent)
		{
			if (!((tr.ent->solid == SOLID_BSP) && (tr.ent->s.modelindex != 1)))
				silly_impact(tr.endpos, tr.plane.normal, 1);
		}
	}

    if (water) {
        gi.WriteByte(svc_temp_entity);
        gi.WriteByte(TE_RAILTRAIL);
        gi.WritePosition(start);
        gi.WritePosition(tr.endpos);
        gi.multicast(tr.endpos, MULTICAST_PHS);
    }

    if (self->client)
        PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
}


/*
=================
fire_bfg
=================
*/
void bfg_explode(edict_t *self)
{
    edict_t *ent;
    float   points;
    vec3_t  v;
    float   dist;

    if (self->s.frame == 0) {
        // the BFG effect
        ent = NULL;
        while ((ent = findradius(ent, self->s.origin, self->dmg_radius)) != NULL) {
            if (!ent->takedamage)
                continue;
            if (ent == self->owner)
                continue;
            if (!CanDamage(ent, self))
                continue;
            if (!CanDamage(ent, self->owner))
                continue;

            VectorAdd(ent->mins, ent->maxs, v);
            VectorMA(ent->s.origin, 0.5, v, v);
            VectorSubtract(self->s.origin, v, v);
            dist = VectorLength(v);
            points = self->radius_dmg * (1.0 - sqrt(dist / self->dmg_radius));
            if (ent == self->owner)
                points = points * 0.5;

            gi.WriteByte(svc_temp_entity);
            gi.WriteByte(TE_BFG_EXPLOSION);
            gi.WritePosition(ent->s.origin);
            gi.multicast(ent->s.origin, MULTICAST_PHS);
            T_Damage(ent, self, self->owner, self->velocity, ent->s.origin, vec3_origin, (int)points, 0, DAMAGE_ENERGY, MOD_BFG_EFFECT);
        }
    }

    self->nextthink = level.time + FRAMETIME;
    self->s.frame++;
    if (self->s.frame == 5)
        self->think = G_FreeEdict;
}

void bfg_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
    if (other == self->owner)
        return;

    if (surf && (surf->flags & SURF_SKY)) {
        G_FreeEdict(self);
        return;
    }

    if (self->owner->client)
        PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

    // core explosion - prevents firing it into the wall/floor
    if (other->takedamage)
        T_Damage(other, self, self->owner, self->velocity, self->s.origin, plane->normal, 200, 0, 0, MOD_BFG_BLAST);
    T_RadiusDamage(self, self->owner, 200, other, 100, MOD_BFG_BLAST);

    gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/bfg__x1b.wav"), 1, ATTN_NORM, 0);
    self->solid = SOLID_NOT;
    self->touch = NULL;
    VectorMA(self->s.origin, -1 * FRAMETIME, self->velocity, self->s.origin);
    VectorClear(self->velocity);
    self->s.modelindex = gi.modelindex("sprites/s_bfg3.sp2");
    self->s.frame = 0;
    self->s.sound = 0;
    self->s.effects &= ~EF_ANIM_ALLFAST;
    self->think = bfg_explode;
    self->nextthink = level.time + FRAMETIME;
    self->enemy = other;

    gi.WriteByte(svc_temp_entity);
    gi.WriteByte(TE_BFG_BIGEXPLOSION);
    gi.WritePosition(self->s.origin);
    gi.multicast(self->s.origin, MULTICAST_PVS);
}


void bfg_think(edict_t *self)
{
    edict_t *ent;
    edict_t *ignore;
    vec3_t  point;
    vec3_t  dir;
    vec3_t  start;
    vec3_t  end;
    int     dmg;
    trace_t tr;

    if (deathmatch->value)
        dmg = 5;
    else
        dmg = 10;

    ent = NULL;
    while ((ent = findradius(ent, self->s.origin, 256)) != NULL) {
        if (ent == self)
            continue;

        if (ent == self->owner)
            continue;

        if (!ent->takedamage)
            continue;

        if (!(ent->svflags & SVF_MONSTER) && (!ent->client) && (strcmp(ent->classname, "misc_explobox") != 0))
            continue;

        VectorMA(ent->absmin, 0.5, ent->size, point);

        VectorSubtract(point, self->s.origin, dir);
        VectorNormalize(dir);

        ignore = self;
        VectorCopy(self->s.origin, start);
        VectorMA(start, 2048, dir, end);
        while (1) {
            tr = gi.trace(start, NULL, NULL, end, ignore, CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_DEADMONSTER);

            if (!tr.ent)
                break;

            // hurt it if we can
            if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER) && (tr.ent != self->owner))
                T_Damage(tr.ent, self, self->owner, dir, tr.endpos, vec3_origin, dmg, 1, DAMAGE_ENERGY, MOD_BFG_LASER);

            // if we hit something that's not a monster or player we're done
            if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client)) {
                gi.WriteByte(svc_temp_entity);
                gi.WriteByte(TE_LASER_SPARKS);
                gi.WriteByte(4);
                gi.WritePosition(tr.endpos);
                gi.WriteDir(tr.plane.normal);
                gi.WriteByte(self->s.skinnum);
                gi.multicast(tr.endpos, MULTICAST_PVS);
                break;
            }

            ignore = tr.ent;
            VectorCopy(tr.endpos, start);
        }

        gi.WriteByte(svc_temp_entity);
        gi.WriteByte(TE_BFG_LASER);
        gi.WritePosition(self->s.origin);
        gi.WritePosition(tr.endpos);
        gi.multicast(self->s.origin, MULTICAST_PHS);

    }

    self->nextthink = level.time + FRAMETIME;
}


void fire_bfg(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius)
{
    edict_t *bfg;

    bfg = G_Spawn();
    VectorCopy(start, bfg->s.origin);
    VectorCopy(dir, bfg->movedir);
    vectoangles(dir, bfg->s.angles);
    VectorScale(dir, speed, bfg->velocity);
    bfg->movetype = MOVETYPE_FLYMISSILE;
    bfg->clipmask = MASK_SHOT;
    bfg->solid = SOLID_BBOX;
    bfg->s.effects |= EF_BFG | EF_ANIM_ALLFAST;
    VectorClear(bfg->mins);
    VectorClear(bfg->maxs);
    bfg->s.modelindex = gi.modelindex("sprites/s_bfg1.sp2");
    bfg->owner = self;
    bfg->touch = bfg_touch;
    bfg->nextthink = level.time + 8000 / speed;
    bfg->think = G_FreeEdict;
    bfg->radius_dmg = damage;
    bfg->dmg_radius = damage_radius;
    bfg->classname = "bfg blast";
    bfg->s.sound = gi.soundindex("weapons/bfg__l1a.wav");

    bfg->think = bfg_think;
    bfg->nextthink = level.time + FRAMETIME;
    bfg->teammaster = bfg;
    bfg->teamchain = NULL;

    if (self->client)
        check_dodge(self, bfg->s.origin, dir, speed);

    gi.linkentity(bfg);
}

/*
 * Drops a spark from the flare flying thru the air.  Checks to make
 * sure we aren't in the water.
 */
void flare_sparks(edict_t *self)
{
	vec3_t dir;
	vec3_t forward, right, up;
	// Spawn some sparks.  This isn't net-friendly at all, but will 
	// be fine for single player. 
	// 
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_FLARE);

    gi.WriteShort(self - g_edicts);
    // if this is the first tick of flare, set count to 1 to start the sound
    gi.WriteByte( self->timestamp - level.time < 14.75 ? 0 : 1);

    gi.WritePosition(self->s.origin);

	// If we are still moving, calculate the normal to the direction 
	 // we are travelling. 
	 // 
	if (VectorLength(self->velocity) > 0.0)
	{
		vectoangles(self->velocity, dir);
		AngleVectors(dir, forward, right, up);

		gi.WriteDir(up);
	}
	// If we're stopped, just write out the origin as our normal 
	// 
	else
	{
		gi.WriteDir(vec3_origin);
	}
	gi.multicast(self->s.origin, MULTICAST_PVS);
}

/*
   void flare_think( edict_t *self )

   Purpose: The think function of a flare round.  It generates sparks
			on the flare using a temp entity, and kills itself after
			self->timestamp runs out.
   Parameters:
	 self: A pointer to the edict_t structure representing the
		   flare round.  self->timestamp is the value used to
		   measure the lifespan of the round, and is set in
		   fire_flaregun blow.

   Notes:
	 - I'm not sure how much bandwidth is eaten by spawning a temp
	   entity every FRAMETIME seconds.  It might very well turn out
	   that the sparks need to go bye-bye in favor of less bandwidth
	   usage.  Then again, why the hell would you use this gun on
	   a DM server????

	 - I haven't seen self->timestamp used anywhere else in the code,
	   but I never really looked that hard.  It doesn't seem to cause
	   any problems, and is aptly named, so I used it.
 */
void flare_think(edict_t *self)
{
	// self->timestamp is 15 seconds after the flare was spawned. 
	// 
	if (level.time > self->timestamp)
	{
		G_FreeEdict(self);
		return;
	}

	// We're still active, so lets shoot some sparks. 
	// 
	flare_sparks(self);
	
	// We'll think again in .2 seconds 
	// 
	self->nextthink = level.time + 0.2;
}

void flare_touch(edict_t *ent, edict_t *other,
	cplane_t *plane, csurface_t *surf)
{
	// Flares don't weigh that much, so let's have them stop 
	// the instant they whack into anything. 
	// 
	VectorClear(ent->velocity);
}

void fire_flaregun(edict_t *self, vec3_t start, vec3_t aimdir,
	int damage, int speed, float timer,
	float damage_radius)
{
	edict_t *flare;
	vec3_t dir;
	vec3_t forward, right, up;

	vectoangles(aimdir, dir);
	AngleVectors(dir, forward, right, up);

	flare = G_Spawn();
	VectorCopy(start, flare->s.origin);
	VectorScale(aimdir, speed, flare->velocity);
	VectorSet(flare->avelocity, 300, 300, 300);
	flare->movetype = MOVETYPE_BOUNCE;
	flare->clipmask = MASK_SHOT;
	flare->solid = SOLID_BBOX;

	const float size = 4;
	VectorSet(flare->mins, -size, -size, -size);
	VectorSet(flare->maxs, size, size, size);

	flare->s.modelindex = gi.modelindex("models/objects/flare/tris.md2");
	flare->owner = self;
	flare->touch = flare_touch;
	flare->nextthink = FRAMETIME;
	flare->think = flare_think;
	flare->radius_dmg = damage;
	flare->dmg_radius = damage_radius;
	flare->classname = "flare";
	flare->timestamp = level.time + 15.0; //live for 15 seconds 
	gi.linkentity(flare);
}

void Head_Touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		n;

	vec3_t	dir;
	vec3_t	forward;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(ent);
		return;
	}

	if (!other->takedamage)
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/fhit3.wav"), 1, ATTN_NORM, 0);
		//if (ent->groundentity)
		//	VectorClear(ent->avelocity);
		//else
		return;
	} else {
		//if (!ent->groundentity)
		//{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("chick/chkatck4.wav"), 1, ATTN_NORM, 0);
			T_Damage(other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, ent->dmg * 2, 0, MOD_ROCK);

			n = rand() % 3;
			while (n--)
				ThrowGib(ent, "models/objects/gibs/sm_meat/tris.md2", ent->dmg, GIB_ORGANIC);
		//}
	}

	G_FreeEdict(ent);
}

void Head_Think(edict_t *self)
{
	vec3_t		dir, forward, right, start, end;
	trace_t		tr;
	int			n;

	if (level.time > self->delay)
	{
		n = rand() % 3;
		while (n--)
			ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", self->dmg, GIB_ORGANIC);

		G_FreeEdict(self);
		return;
	}

	self->nextthink = level.time + FRAMETIME;
}

void fire_head(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles(aimdir, dir);
	AngleVectors(dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy(start, grenade->s.origin);
	VectorScale(aimdir, speed, grenade->velocity);
	VectorCopy(dir, grenade->movedir);
	VectorCopy(dir, grenade->s.angles);
	VectorMA(grenade->velocity, 175, up, grenade->velocity);
	//VectorSet(grenade->avelocity, 50, 50, 50);
	VectorSet(grenade->avelocity, -600, 0, 0);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GIB;
	// 8 better for prediction :s
	VectorSet(grenade->mins, -2, -2, -2);
	VectorSet(grenade->maxs, 2, 2, 2);
	//VectorClear(grenade->mins);
	//VectorClear(grenade->maxs);
	grenade->s.modelindex = gi.modelindex("models/objects/gibs/head2/tris.md2");
	grenade->owner = self;
	grenade->touch = Head_Touch;
	grenade->nextthink = level.time + FRAMETIME;
	grenade->think = Head_Think;
	grenade->delay = level.time + 3;
	//grenade->nextthink = level.time + 3;
	//grenade->think = G_FreeEdict;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "head";
	//grenade->flags |= FL_TOUCH;
	grenade->gravity = 0.65;

	gi.linkentity(grenade);
}

void Rock_Touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		n;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(ent);
		return;
	}

	if (!other->takedamage)
	{
		//ent->s.effects &= ~EF_GRENADE;
		gi.sound(ent, CHAN_VOICE, gi.soundindex("brain/melee3.wav"), 1, ATTN_NORM, 0);
		if (ent->groundentity)
			VectorClear(ent->avelocity);
		else
			VectorSet(ent->avelocity, 200, 0, 0);
		return;
	}
	else {
		if (!ent->groundentity)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("chick/chkatck4.wav"), 1, ATTN_NORM, 0);
			T_Damage(other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, ent->dmg * 2, 0, MOD_ROCK);

			n = rand() % 5;
			while (n--)
				ThrowDebris(ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
		}
	}

	G_FreeEdict(ent);
}

void fire_rock(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles(aimdir, dir);
	AngleVectors(dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy(start, grenade->s.origin);
	VectorScale(aimdir, speed, grenade->velocity);
	VectorMA(grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA(grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet(grenade->avelocity, 50, 50, 50);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	//grenade->s.effects |= EF_GRENADE;
	// 8 better for prediction :s
	VectorSet(grenade->mins, -4, -4, -4);
	VectorSet(grenade->maxs, 4, 4, 4);
	//VectorClear(grenade->mins);
	//VectorClear(grenade->maxs);
	grenade->s.modelindex = gi.modelindex("models/objects/rock/tris.md2");
	grenade->owner = self;
	grenade->touch = Rock_Touch;
	grenade->nextthink = level.time + 3;
	grenade->think = G_FreeEdict;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "rock";

	gi.linkentity(grenade);
}

// acid stuff

void acid_think(edict_t *self)
{
	vec3_t	forward;
	vec3_t	dir;

	if (level.time > self->wait)
	{
		G_FreeEdict(self);
		return;
	}

	/*if (self->count == 1)
	{
		if (self->s.frame < 10)
		{
			self->s.frame++;
		}
	}*/

	//if (self->groundentity)
	//	self->s.modelindex = gi.modelindex("models/objects/acid/tris2.md2");

	//AngleVectors(self->s.angles, forward, NULL, NULL);

	//gi.WriteByte(svc_temp_entity);
	//gi.WriteByte(TE_GREENBLOOD);
	//gi.WritePosition(self->s.origin);
	//gi.WriteDir(forward);
	//gi.multicast(self->s.origin, MULTICAST_PVS);

	// cut down on messages by only updating every few frames and a longer distance

	/*gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_FORCEWALL);
	gi.WritePosition(self->s.origin);
	gi.WritePosition(self->pos2);
	//gi.WritePosition(self->s.old_origin);
	//gi.WriteByte(0xd0d1d2d3);
	gi.WriteByte(0xD3 + (rand() & 3));
	gi.multicast(self->s.origin, MULTICAST_PVS);*/

	// rate throttle angle changes to reduce bandwidth use?

	if (self->count != 1 && g_doacidfx->value)
	{
		//VectorCopy(self->s.origin, self->pos2);
		VectorCopy(self->velocity, dir);
		VectorNormalize(dir);
		vectoangles(dir, self->s.angles);
	}

	self->nextthink = level.time + FRAMETIME;
}

void acid_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		normal_angles, forward, right, up;
	vec3_t		offset, pos;
	qboolean	small;

	trace_t		tr;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if ((other->solid == SOLID_BSP) && (other->s.modelindex != 1))
		return;

	if (other->takedamage)
	{
		// flag for damage over time
		if (self->dmg == 1)
		{
			if (random() < 0.2) // randomise to reduce damage over time
				T_Damage(other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, self->dmg, 0, MOD_UNKNOWN);
		}
		else {
			gi.sound(self, CHAN_VOICE, gi.soundindex("misc/fhit3.wav"), 1, ATTN_NORM, 0);
			T_Damage(other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, self->dmg, 0, MOD_UNKNOWN);
			//self->s.modelindex = gi.modelindex("models/objects/acid/tris2.md2");
			self->s.modelindex = gi.modelindex("models/objects/impacts/tris.md2");
			self->s.skinnum = 0;

			self->dmg = 1;
		}
	}
	else
	{
		//if (!self->groundentity)
		//	return;

		//self->touch = NULL; // if we hit ground use another touch function so sounds don't repeat

		if (plane && (self->count != 1))
		{
			self->count = 1;
			small = qfalse;

			gi.sound(self, CHAN_VOICE, gi.soundindex("misc/fhit3.wav"), 1, ATTN_NORM, 0);
			//self->s.modelindex = gi.modelindex("models/objects/acid/tris2.md2");
			self->s.modelindex = gi.modelindex("models/objects/impacts/tris.md2");
			self->s.skinnum = 0;
			self->movetype = MOVETYPE_NONE;

			vectoangles(plane->normal, normal_angles);
			AngleVectors(normal_angles, forward, right, up);
			vectoangles(forward, self->s.angles);

			// kind of performance expensive check to minimise acid splashes
			// that stick out into empty space

			VectorCopy(self->s.origin, pos);
			VectorMA(pos, -8, right, pos);
			VectorMA(pos, 4, up, pos);
			VectorMA(pos, -0.2, forward, pos);

			tr = gi.trace(pos, NULL, NULL, self->s.origin, self, MASK_SOLID);

			// if small becomes true then we could just skip subsequent checks

			if (!tr.startsolid)
			{
				//self->s.modelindex = gi.modelindex("models/objects/acid/tris3.md2");
				small = qtrue;
			}

			/*gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_DEBUGTRAIL);
			gi.WritePosition(self->s.origin);
			gi.WritePosition(pos);
			gi.multicast(self->s.origin, MULTICAST_PHS);*/

			if (small == qfalse)
			{
				VectorCopy(self->s.origin, pos);
				VectorMA(pos, 6, right, pos);
				VectorMA(pos, 7, up, pos);
				VectorMA(pos, -0.2, forward, pos);

				tr = gi.trace(pos, NULL, NULL, self->s.origin, self, MASK_SOLID);

				if (!tr.startsolid)
				{
					//self->s.modelindex = gi.modelindex("models/objects/acid/tris3.md2");
					small = qtrue;
				}
			}

			if (small == qfalse)
			{
				VectorCopy(self->s.origin, pos);
				VectorMA(pos, -1, right, pos);
				VectorMA(pos, -8, up, pos);
				VectorMA(pos, -0.2, forward, pos);

				tr = gi.trace(pos, NULL, NULL, self->s.origin, self, MASK_SOLID);

				if (!tr.startsolid)
				{
					//self->s.modelindex = gi.modelindex("models/objects/acid/tris3.md2");
					small = qtrue;
				}
			}

			if (small == qtrue)
			{
				//self->s.modelindex = gi.modelindex("models/objects/acid/tris3.md2");
				self->s.modelindex = gi.modelindex("models/objects/impacts/tris.md2");
				self->s.skinnum = 1;
				gi.WriteByte(svc_temp_entity);
				gi.WriteByte(TE_GREENBLOOD);
				gi.WritePosition(self->s.origin);
				gi.WriteDir(forward);
				gi.multicast(self->s.origin, MULTICAST_PVS);
			}

			gi.unlinkentity(self);
			self->solid = SOLID_TRIGGER;

			VectorSet(self->mins, -8, -8, -8);
			VectorSet(self->maxs, 8, 8, 8);
			self->dmg = 1;

			gi.linkentity(self);
		}
	}
}

void fire_acid(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t		*bolt;
	vec3_t		forward, up, right;
	trace_t		tr;

	VectorNormalize(dir);

	bolt = G_Spawn();
	bolt->svflags = SVF_DEADMONSTER;
	// yes, I know it looks weird that projectiles are deadmonsters
	// what this means is that when prediction is used against the object
	// (blaster/hyperblaster shots), the player won't be solid clipped against
	// the object.  Right now trying to run into a firing hyperblaster
	// is very jerky since you are predicted 'against' the shots.
	VectorCopy(start, bolt->s.origin);
	VectorCopy(start, bolt->s.old_origin);
	//VectorCopy(start, bolt->pos2);
	vectoangles(dir, bolt->s.angles);
	VectorScale(dir, speed, bolt->velocity);

	AngleVectors(bolt->s.angles, forward, right, up);

	//VectorMA(bolt->velocity, 200 + crandom() * 10.0, up, bolt->velocity);
	VectorMA(bolt->velocity, 100 + crandom() * 10.0, up, bolt->velocity);
	VectorMA(bolt->velocity, crandom() * 10.0, right, bolt->velocity);

	bolt->movetype = MOVETYPE_TOSS;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	//bolt->s.effects |= EF_GREENGIB | EF_COLOR_SHELL;

	//bolt->s.effects |= EF_HYPERBLASTER | EF_TRACKER | EF_PLASMA;
	//bolt->s.effects |= EF_COLOR_SHELL;
	//bolt->s.renderfx |= RF_TRANSLUCENT | RF_SHELL_GREEN;
	//bolt->s.effects |= EF_SPHERETRANS;
	bolt->s.effects |= EF_GREENGIB;
	bolt->s.renderfx |= RF_TRANSLUCENT | 0x80000000;
	VectorClear(bolt->mins);
	VectorClear(bolt->maxs);
	//bolt->s.modelindex = gi.modelindex("models/objects/gibs/sm_meat/tris.md2");
	bolt->s.modelindex = gi.modelindex("models/objects/acid/tris.md2");
	//bolt->s.sound = gi.soundindex("misc/lasfly.wav");
	bolt->owner = self;
	bolt->touch = acid_touch;
	bolt->wait = level.time + 5;
	bolt->nextthink = level.time + FRAMETIME;
	bolt->think = acid_think;
	bolt->dmg = damage;
	bolt->classname = "acid";
	bolt->gravity = 0.33;

	gi.linkentity(bolt);

	if (self->client)
		check_dodge(self, bolt->s.origin, dir, speed);

	tr = gi.trace(self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA(bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch(bolt, tr.ent, NULL, NULL);
	}
}