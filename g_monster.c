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


//
// monster weapons
//

//FIXME mosnters should call these with a totally accurate direction
// and we can mess it up based on skill.  Spread should be for normal
// and we can tighten or loosen based on skill.  We could muck with
// the damages too, but I'm not sure that's such a good idea.
void monster_fire_bullet(edict_t *self, vec3_t start, vec3_t dir, int damage, int kick, int hspread, int vspread, int flashtype)
{
	vec3_t		forward, right, up, newdir;
	float		r, u;

	//vectoangles(dir, newdir);
	//AngleVectors(newdir, forward, right, up);

	//r = crandom() * hspread;
	//u = crandom() * vspread;
	//VectorMA(start, 8192, forward, end);
	//VectorMA(end, r, right, end);
	//VectorMA(end, u, up, end);

    fire_bullet(self, start, dir, damage, kick, hspread, vspread, MOD_UNKNOWN);

    gi.WriteByte(svc_muzzleflash2);
    gi.WriteShort(self - g_edicts);
    gi.WriteByte(flashtype);
    gi.multicast(start, MULTICAST_PVS);
}

void monster_fire_shotgun(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int flashtype)
{
    fire_shotgun(self, start, aimdir, damage, kick, hspread, vspread, count, MOD_UNKNOWN);

    gi.WriteByte(svc_muzzleflash2);
    gi.WriteShort(self - g_edicts);
    gi.WriteByte(flashtype);
    gi.multicast(start, MULTICAST_PVS);
}

void monster_fire_blaster(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, int effect)
{
    fire_blaster(self, start, dir, damage, speed, effect, qfalse);

    gi.WriteByte(svc_muzzleflash2);
    gi.WriteShort(self - g_edicts);
    gi.WriteByte(flashtype);
    gi.multicast(start, MULTICAST_PVS);
}

void monster_fire_grenade(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int flashtype)
{
    fire_grenade(self, start, aimdir, damage, speed, 2.5, damage + 40);

    gi.WriteByte(svc_muzzleflash2);
    gi.WriteShort(self - g_edicts);
    gi.WriteByte(flashtype);
    gi.multicast(start, MULTICAST_PVS);
}

void monster_fire_rocket(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype)
{
    fire_rocket(self, start, dir, damage, speed, damage + 20, damage);

    gi.WriteByte(svc_muzzleflash2);
    gi.WriteShort(self - g_edicts);
    gi.WriteByte(flashtype);
    gi.multicast(start, MULTICAST_PVS);
}

void monster_fire_railgun(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int flashtype)
{
    fire_rail(self, start, aimdir, damage, kick);

    gi.WriteByte(svc_muzzleflash2);
    gi.WriteShort(self - g_edicts);
    gi.WriteByte(flashtype);
    gi.multicast(start, MULTICAST_PVS);
}

void monster_fire_bfg(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int kick, float damage_radius, int flashtype)
{
    fire_bfg(self, start, aimdir, damage, speed, damage_radius);

    gi.WriteByte(svc_muzzleflash2);
    gi.WriteShort(self - g_edicts);
    gi.WriteByte(flashtype);
    gi.multicast(start, MULTICAST_PVS);
}



//
// Monster utility functions
//

void M_FliesOff(edict_t *self)
{
    self->s.effects &= ~EF_FLIES;
    self->s.sound = 0;
}

void M_FliesOn(edict_t *self)
{
    if (self->waterlevel)
        return;
    self->s.effects |= EF_FLIES;
    self->s.sound = gi.soundindex("infantry/inflies1.wav");
    self->think = M_FliesOff;
    self->nextthink = level.time + 60;
}

void M_FlyCheck(edict_t *self)
{
    if (self->waterlevel)
        return;

    if (random() > 0.5)
        return;

    self->think = M_FliesOn;
    self->nextthink = level.time + 5 + 10 * random();
}

void AttackFinished(edict_t *self, float time)
{
    self->monsterinfo.attack_finished = level.time + time;
}


void M_CheckGround(edict_t *ent)
{
    vec3_t      point;
    trace_t     trace;

    if (ent->flags & (FL_SWIM | FL_FLY))
        return;

    if (ent->velocity[2] > 100) {
        ent->groundentity = NULL;
        return;
    }

// if the hull point one-quarter unit down is solid the entity is on ground
    point[0] = ent->s.origin[0];
    point[1] = ent->s.origin[1];
    point[2] = ent->s.origin[2] - 0.25;

    trace = gi.trace(ent->s.origin, ent->mins, ent->maxs, point, ent, MASK_MONSTERSOLID);

    // check steepness
    if (trace.plane.normal[2] < 0.7 && !trace.startsolid) {
        ent->groundentity = NULL;
        return;
    }

//  ent->groundentity = trace.ent;
//  ent->groundentity_linkcount = trace.ent->linkcount;
//  if (!trace.startsolid && !trace.allsolid)
//      VectorCopy (trace.endpos, ent->s.origin);
    if (!trace.startsolid && !trace.allsolid) {
        VectorCopy(trace.endpos, ent->s.origin);
        ent->groundentity = trace.ent;
        ent->groundentity_linkcount = trace.ent->linkcount;
        ent->velocity[2] = 0;
    }
}


void M_CatagorizePosition(edict_t *ent)
{
    vec3_t      point;
    int         cont;
	float		push;
	vec3_t		v, forward;

	// Rroff - might cause the monster to jump to its death
	// probably should check down for lava, etc.
	if (ent->groundentity && (ent->waterlevel < 2))
	{
		if ((ent->groundentity->movetype == MOVETYPE_PUSH) && (ent->groundentity->s.modelindex != 1))
		{
			if (VectorLength(ent->groundentity->velocity) || VectorLength(ent->groundentity->avelocity))
			{
				if ((ent->monsterinfo.jumpmove) && (level.time > ent->monsterinfo.last_jump_time + 1))
				{
					ent->monsterinfo.jumpmove(ent);
					ent->monsterinfo.last_jump_time = level.time + 1.5;
				}
			}
		}
	}

//
// get waterlevel
//
    point[0] = ent->s.origin[0];
    point[1] = ent->s.origin[1];
    point[2] = ent->s.origin[2] + ent->mins[2] + 1;
    cont = gi.pointcontents(point);

    if (!(cont & MASK_WATER)) {
        ent->waterlevel = 0;
        ent->watertype = 0;
        return;
    }

	// Rroff :D
	// bit goofy sometimes but mostly works

	if (cont & MASK_CURRENT)
	{
		VectorClear(v);

		if (cont & CONTENTS_CURRENT_0)
			v[0] += 1;
		if (cont & CONTENTS_CURRENT_90)
			v[1] += 1;
		if (cont & CONTENTS_CURRENT_180)
			v[0] -= 1;
		if (cont & CONTENTS_CURRENT_270)
			v[1] -= 1;
		if (cont & CONTENTS_CURRENT_UP)
			v[2] += 1;
		if (cont & CONTENTS_CURRENT_DOWN)
			v[2] -= 1;

		if (ent->groundentity)
			VectorMA(ent->velocity, 75, v, ent->velocity);
		else
			VectorMA(ent->velocity, 100, v, ent->velocity);
	}

    ent->watertype = cont;
    ent->waterlevel = 1;
    point[2] += 26;
    cont = gi.pointcontents(point);
    if (!(cont & MASK_WATER))
        return;

	if (!ent->groundentity && !(cont & MASK_CURRENT))
	{
		if (!(ent->flags & FL_SWIM) && !(ent->flags & FL_FLY)) {
			// give them a little nudge in the direction they are trying to go
			AngleVectors(ent->s.angles, forward, NULL, NULL);
			if (VectorLength(ent->velocity) < 75)
				VectorMA(ent->velocity, 75, forward, ent->velocity);
		}
	}

    ent->waterlevel = 2;
    point[2] += 22;
    cont = gi.pointcontents(point);
	if (cont & MASK_WATER)
	{
		ent->waterlevel = 3;

		// Rroff - trying to give them a fighting chance!
		if (!(ent->flags & FL_SWIM) && !(ent->flags & FL_FLY)) {
			if (ent->velocity[2] < 150)
			{
				ent->velocity[2] += 150;
				if (ent->velocity[2] > 150)
					ent->velocity[2] = 150;
			}
		}
	}
}


void M_WorldEffects(edict_t *ent)
{
    int     dmg;

    if (ent->health > 0) {
        if (!(ent->flags & FL_SWIM)) {
            if (ent->waterlevel < 3) {
                ent->air_finished = level.time + 12;
            } else if (ent->air_finished < level.time) {
                // drown!
                if (ent->pain_debounce_time < level.time) {
                    dmg = 2 + 2 * floor(level.time - ent->air_finished);
                    if (dmg > 15)
                        dmg = 15;
                    T_Damage(ent, world, world, vec3_origin, ent->s.origin, vec3_origin, dmg, 0, DAMAGE_NO_ARMOR, MOD_WATER);
                    ent->pain_debounce_time = level.time + 1;
                }
            }
        } else {
            if (ent->waterlevel > 0) {
                ent->air_finished = level.time + 9;
            } else if (ent->air_finished < level.time) {
                // suffocate!
                if (ent->pain_debounce_time < level.time) {
                    dmg = 2 + 2 * floor(level.time - ent->air_finished);
                    if (dmg > 15)
                        dmg = 15;
                    T_Damage(ent, world, world, vec3_origin, ent->s.origin, vec3_origin, dmg, 0, DAMAGE_NO_ARMOR, MOD_WATER);
                    ent->pain_debounce_time = level.time + 1;
                }
            }
        }
    }

    if (ent->waterlevel == 0) {
        if (ent->flags & FL_INWATER) {
            gi.sound(ent, CHAN_BODY, gi.soundindex("player/watr_out.wav"), 1, ATTN_NORM, 0);
            ent->flags &= ~FL_INWATER;
        }
        return;
    }

    if ((ent->watertype & CONTENTS_LAVA) && !(ent->flags & FL_IMMUNE_LAVA)) {
        if (ent->damage_debounce_time < level.time) {
            ent->damage_debounce_time = level.time + 0.2;
            T_Damage(ent, world, world, vec3_origin, ent->s.origin, vec3_origin, 10 * ent->waterlevel, 0, 0, MOD_LAVA);
        }
    }
    if ((ent->watertype & CONTENTS_SLIME) && !(ent->flags & FL_IMMUNE_SLIME)) {
        if (ent->damage_debounce_time < level.time) {
            ent->damage_debounce_time = level.time + 1;
            T_Damage(ent, world, world, vec3_origin, ent->s.origin, vec3_origin, 4 * ent->waterlevel, 0, 0, MOD_SLIME);
        }
    }

    if (!(ent->flags & FL_INWATER)) {
        if (!(ent->svflags & SVF_DEADMONSTER)) {
            if (ent->watertype & CONTENTS_LAVA)
                if (random() <= 0.5)
                    gi.sound(ent, CHAN_BODY, gi.soundindex("player/lava1.wav"), 1, ATTN_NORM, 0);
                else
                    gi.sound(ent, CHAN_BODY, gi.soundindex("player/lava2.wav"), 1, ATTN_NORM, 0);
            else if (ent->watertype & CONTENTS_SLIME)
                gi.sound(ent, CHAN_BODY, gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);
            else if (ent->watertype & CONTENTS_WATER)
                gi.sound(ent, CHAN_BODY, gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);
        }

        ent->flags |= FL_INWATER;
        ent->damage_debounce_time = 0;
    }
}


// Rroff this wasn't really necessary
// but might help a few cases
void M_droptofloor__(edict_t *ent)
{
	vec3_t      end;
	vec3_t	start;
    trace_t     trace;

	VectorCopy(ent->s.origin, start);
	VectorCopy(ent->s.origin, end);
	end[2] += 128;
	trace = gi.trace(start, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);

	if (trace.allsolid)
		return;

    VectorCopy(ent->s.origin, end);
    end[2] -= 256;

	trace = gi.trace(trace.endpos, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);

    if (trace.fraction == 1 || trace.allsolid)
        return;

    VectorCopy(trace.endpos, ent->s.origin);

    gi.linkentity(ent);
    M_CheckGround(ent);
    M_CatagorizePosition(ent);
}

void M_droptofloor(edict_t *ent)
{
	vec3_t      end;
	trace_t     trace;

	ent->s.origin[2] += 1;
	VectorCopy(ent->s.origin, end);
	end[2] -= 256;

	trace = gi.trace(ent->s.origin, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);

	if (trace.fraction == 1 || trace.allsolid)
		return;

	VectorCopy(trace.endpos, ent->s.origin);

	gi.linkentity(ent);
	M_CheckGround(ent);
	M_CatagorizePosition(ent);
}


void M_SetEffects(edict_t *ent)
{
    ent->s.effects &= ~(EF_COLOR_SHELL | EF_POWERSCREEN | EF_PENT);
    ent->s.renderfx &= ~(RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE);

    if (ent->monsterinfo.aiflags & AI_RESURRECTING) {
        ent->s.effects |= EF_COLOR_SHELL;
        ent->s.renderfx |= RF_SHELL_RED;
    }

	if (ent->monsterinfo.bonus_time && (level.time < ent->monsterinfo.bonus_time + 30))
	{
		if (!(level.framenum & 15))
		{
			ent->s.effects |= EF_PENT;
		}
		//ent->s.effects |= EF_HALF_DAMAGE;
		//ent->s.renderfx |= RF_GLOW;
	}

    if (ent->health <= 0)
        return;

    if (ent->powerarmor_time > level.time) {
        if (ent->monsterinfo.power_armor_type == POWER_ARMOR_SCREEN) {
            ent->s.effects |= EF_POWERSCREEN;
        } else if (ent->monsterinfo.power_armor_type == POWER_ARMOR_SHIELD) {
            ent->s.effects |= EF_COLOR_SHELL;
            ent->s.renderfx |= RF_SHELL_GREEN;
        }
    }
}


void M_MoveFrame(edict_t *self)
{
    mmove_t *move;
    int     index;

    move = self->monsterinfo.currentmove;
    self->nextthink = level.time + FRAMETIME;

    if ((self->monsterinfo.nextframe) && (self->monsterinfo.nextframe >= move->firstframe) && (self->monsterinfo.nextframe <= move->lastframe)) {
        self->s.frame = self->monsterinfo.nextframe;
        self->monsterinfo.nextframe = 0;
    } else {
        if (self->s.frame == move->lastframe) {
            if (move->endfunc) {
                move->endfunc(self);

                // regrab move, endfunc is very likely to change it
                move = self->monsterinfo.currentmove;

                // check for death
                if (self->svflags & SVF_DEADMONSTER)
                    return;
            }
        }

        if (self->s.frame < move->firstframe || self->s.frame > move->lastframe) {
            self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
            self->s.frame = move->firstframe;
        } else {
            if (!(self->monsterinfo.aiflags & AI_HOLD_FRAME)) {
                self->s.frame++;
                if (self->s.frame > move->lastframe)
                    self->s.frame = move->firstframe;
            }
        }
    }

    index = self->s.frame - move->firstframe;
    if (move->frame[index].aifunc) {
        if (!(self->monsterinfo.aiflags & AI_HOLD_FRAME))
            move->frame[index].aifunc(self, move->frame[index].dist * self->monsterinfo.scale);
        else
            move->frame[index].aifunc(self, 0);
    }

    if (move->frame[index].thinkfunc)
        move->frame[index].thinkfunc(self);
}

void monster_think(edict_t *self)
{
	vec3_t		distance;
	float		range;
	edict_t		*ent;
	int			dotamount = 0, c;
	int			mtime1 = 10, mtime2 = 20;

    M_MoveFrame(self);
    if (self->linkcount != self->monsterinfo.linkcount) {
        self->monsterinfo.linkcount = self->linkcount;
        M_CheckGround(self);
    }
    M_CatagorizePosition(self);
    M_WorldEffects(self);
    M_SetEffects(self);

	// if horde monster

	if ((self->monsterinfo.aiflags2 & AI2_CONTROL_GOALTIMER))
	{
		if (level.time > self->last_time)
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
			}
		}
	}

	// if lockdown monster and don't have a current enemy
	// and not in search mode flag search mode and find nearest node

	if ((self->monsterinfo.aiflags2 & AI2_CONTROL) && (self->enemy))
	{
		self->monsterinfo.aiflags2 &= ~AI2_CONTROL_SEARCH;
		self->monsterinfo.ai_roaming_time = 0;
		self->monsterinfo.aiflags &= ~AI_ROAMING;
	}

	// Rroff - this needs to be as efficient as possible
	if ((self->monsterinfo.aiflags2 & AI2_CONTROL) && !(self->monsterinfo.aiflags2 & AI2_CONTROL_GOALTIMER))
	{
		if ((level.total_monsters - level.killed_monsters) > 0 && (level.total_monsters - level.killed_monsters) < 4)
		{
			mtime1 = 45;
			mtime2 = 45;
		}
		/*if (level.time > self->monsterinfo.ai_last_sight + 30)
		{
			//if (nearestplayer(self) > 1536)
			// if too close players might see them blink out of existence too often
			if (nearestplayer(self) > 1024)
			{
				cpoint_move_monster(self);
				self->monsterinfo.ai_last_sight = level.time;
			}
		}*/

		//if (level.time > self->monsterinfo.ai_last_sight + 75)
		if (level.time > self->monsterinfo.ai_last_sight + mtime2)
		{
			cpoint_move_monster(self);
			self->monsterinfo.ai_last_sight = level.time;
		}

		//if ((level.time > self->monsterinfo.ai_last_sight + 20) && (level.sight_client))
		if ((level.time > self->monsterinfo.ai_last_sight + mtime1) && (level.sight_client))
		{
			if (!gi.inPHS(self->s.origin, level.sight_client->s.origin))
			{
				cpoint_move_monster(self);
				self->monsterinfo.ai_last_sight = level.time;
			}
		}
	}

	if (self->monsterinfo.timebomb)
	{
		if (level.time > self->monsterinfo.timebomb)
		{
			self->monsterinfo.timebomb = 0;
			T_RadiusDamage(self, self, 150, NULL, 120, MOD_EXPLOSIVE);
			T_Damage(self, world, world, vec3_origin, self->s.origin, vec3_origin, 150, DAMAGE_NO_KNOCKBACK, 0, MOD_UNKNOWN);
			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_EXPLOSION2);
			gi.WritePosition(self->s.origin);
			gi.multicast(self->s.origin, MULTICAST_PVS);
		}
	}

	if ((level.time > self->monsterinfo.dottick_time) && (self->health > 0))
	{
		self->monsterinfo.dottick_time = level.time + 1.0;

		for (c = 0; c <= MAX_DOTI; c++)
		{
			if (level.time < self->monsterinfo.dot_time[c])
			{
				dotamount += self->monsterinfo.dot_amt[c];
				if (c == DOT_CYBORG)
				{
					if (self->monsterinfo.dot_ent && self->monsterinfo.dot_ent->inuse && self->monsterinfo.dot_ent->health > 0)
					{
						if (self->monsterinfo.dot_ent->health < (self->monsterinfo.dot_ent->max_health)) // was max_health * 1.5
						self->monsterinfo.dot_ent->health += 1;
					}
				}
			}
		}

		if (dotamount > 0)
			T_Damage(self, world, world, vec3_origin, self->s.origin, vec3_origin, dotamount, DAMAGE_NO_KNOCKBACK, 0, MOD_UNKNOWN);
	}

	// Not sure it is really worth using this
	if ((self->monsterinfo.aiflags & AI_RUNAWAY) && (level.time > self->monsterinfo.runaway_time + 5)) // 9, was 15 or 3
	{
		if (self->monsterinfo.aiflags & AI_ASSUMED)
		{ 
			self->goalentity = self->enemy = self->oldenemy = NULL;
			self->monsterinfo.aiflags &= ~(AI_RUNAWAY | AI_ASSUMED);
			self->monsterinfo.pausetime = level.time + 100000000;
			self->monsterinfo.stand(self);
		}
		else {
			self->goalentity = self->enemy = NULL;
			self->monsterinfo.aiflags &= ~(AI_RUNAWAY | AI_ASSUMED);
			//self->monsterinfo.pausetime = level.time + 100000000;
			//self->monsterinfo.stand(self);
			self->monsterinfo.aiflags |= AI_ROAMING;

			// Rroff - return might not be good here if we add stuff below later
			if (FindTarget(self))
				return;

			self->monsterinfo.walk(self);

			if (self->oldenemy)
			{
				if (self->oldenemy->inuse)
				{
					if (self->oldenemy->health > 0 && self->oldenemy->client)
					{
						self->enemy = self->goalentity = self->oldenemy;
						self->oldenemy = NULL;
						self->monsterinfo.aiflags |= AI_ROAMING;
						self->monsterinfo.run(self);
					}
				}
			}
		}
	}

	if ((self->monsterinfo.aiflags & AI_RUNAWAY))
	{
		if (self->goalentity && self->goalentity->inuse)
		{

			/*gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_BFG_LASER);
			gi.WritePosition(self->s.origin);
			gi.WritePosition(self->goalentity->s.origin);
			gi.multicast(self->s.origin, MULTICAST_PHS);*/

			VectorSubtract(self->goalentity->s.origin, self->s.origin, distance);
			range = VectorLength(distance);

			// have to be careful this doesn't result in running on the spot
			// trying to get close to the point - might be worth moving the
			// distance check to the edge of the object

			if ((range < 150) && !(self->monsterinfo.aiflags & AI_ASSUMED)) {
				ent = runpoint(self);
				if (ent)
				{
					if (ent != self->goalentity)
					{
						self->goalentity = ent;
						self->monsterinfo.run(self);
						self->monsterinfo.aiflags |= AI_RUNAWAY;
						//self->monsterinfo.runaway_time = level.time;
					}
				} else {
					self->goalentity = self->enemy = NULL;
					self->monsterinfo.aiflags &= ~(AI_RUNAWAY | AI_ASSUMED);
					//self->monsterinfo.pausetime = level.time + 100000000;
					//self->monsterinfo.stand(self);
					
					// Rroff - return might not be good here if we add stuff below later
					// somewhat defeats the point of running away maybe
					if (FindTarget(self))
						return;

					self->monsterinfo.aiflags |= AI_ROAMING;
					self->monsterinfo.walk(self);

					// if old_enemy gets changed since runaway was initiated
					// they can forget about the player and wander off here :(

					if (self->oldenemy)
					{
						if (self->oldenemy->inuse)
						{
							if (self->oldenemy->health > 0 && self->oldenemy->client)
							{
								self->enemy = self->goalentity = self->oldenemy;
								self->oldenemy = NULL;
								self->monsterinfo.aiflags |= AI_ROAMING;
								self->monsterinfo.run(self);
							}
						}
					}
				}
			}

			if ((range < 80) && (self->monsterinfo.aiflags & AI_ASSUMED))
			{
				self->goalentity = self->enemy = self->oldenemy = NULL;
				self->monsterinfo.aiflags &= ~(AI_RUNAWAY | AI_ASSUMED);
				self->monsterinfo.pausetime = level.time + 100000000;
				self->monsterinfo.stand(self);
			}

		} else {
			self->goalentity = self->enemy = NULL;
			self->monsterinfo.aiflags &= ~(AI_RUNAWAY | AI_ASSUMED);
			//self->monsterinfo.pausetime = level.time + 100000000;
			//self->monsterinfo.stand(self);

			// Rroff - return might not be good here if we add stuff below later
			if (FindTarget(self))
				return;

			self->monsterinfo.aiflags |= AI_ROAMING;
			self->monsterinfo.walk(self);

			// if old_enemy gets changed since runaway was initiated
			// they can forget about the player and wander off here :(

			if (self->oldenemy)
			{
				if (self->oldenemy->inuse)
				{
					if (self->oldenemy->health > 0 && self->oldenemy->client)
					{
						self->enemy = self->goalentity = self->oldenemy;
						self->oldenemy = NULL;
						self->monsterinfo.aiflags |= AI_ROAMING;
						self->monsterinfo.run(self);
					}
				}
			}
		}
	}
}


/*
================
monster_use

Using a monster makes it angry at the current activator
================
*/
void monster_use(edict_t *self, edict_t *other, edict_t *activator)
{
    if (self->enemy)
        return;
    if (self->health <= 0)
        return;
	// Rroff - not sure what is happening here
	// sometimes door coming down and hurting a monster has null activator
	// causing crash
	if (!activator)
		return;
    if (activator->flags & FL_NOTARGET)
        return;
    if (!(activator->client) && !(activator->monsterinfo.aiflags & AI_GOOD_GUY))
        return;
	if (self->monsterinfo.aiflags & AI_MEDIC2)
		return;

// delay reaction so if the monster is teleported, its sound is still heard
    self->enemy = activator;
    FoundTarget(self);
}


void monster_start_go(edict_t *self);


void monster_triggered_spawn(edict_t *self)
{
    self->s.origin[2] += 1;
    KillBox(self);

    self->solid = SOLID_BBOX;
    self->movetype = MOVETYPE_STEP;
    self->svflags &= ~SVF_NOCLIENT;
	self->spawnflags &= ~SPAWNFLAG_EVENT; // Rroff - clear the spawnflag so we know they are spawned
    self->air_finished = level.time + 12;
    gi.linkentity(self);

	if (self->spawnflags & SPAWNFLAG_EVENT_TELE)
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_TELEPORT_EFFECT);
		gi.WritePosition(self->s.origin);
		gi.multicast(self->s.origin, MULTICAST_PVS);

		gi.sound(self, CHAN_VOICE, gi.soundindex("misc/tele1.wav"), 1, ATTN_NORM, 0);
	}

    monster_start_go(self);

    if (self->enemy && !(self->spawnflags & 1) && !(self->enemy->flags & FL_NOTARGET)) {
        FoundTarget(self);
    } else {
        self->enemy = NULL;
    }
}

void monster_triggered_spawn_use(edict_t *self, edict_t *other, edict_t *activator)
{
    // we have a one frame delay here so we don't telefrag the guy who activated us
    self->think = monster_triggered_spawn;
    self->nextthink = level.time + FRAMETIME;

	// Rroff trigger spawned monster spawned by an event won't have an activator currently
	// FIXME
	if (!(self->spawnflags & SPAWNFLAG_EVENT))
	{
		if (activator && activator->client)
			self->enemy = activator;
	}

    self->use = monster_use;
}

void monster_triggered_start(edict_t *self)
{
    self->solid = SOLID_NOT;
    self->movetype = MOVETYPE_NONE;
    self->svflags |= SVF_NOCLIENT;
    self->nextthink = 0;
    self->use = monster_triggered_spawn_use;
}


/*
================
monster_death_use

When a monster dies, it fires all of its targets with the current
enemy as activator.
================
*/
void monster_death_use(edict_t *self)
{
    self->flags &= ~(FL_FLY | FL_SWIM);
    self->monsterinfo.aiflags &= AI_GOOD_GUY;

    if (self->item) {
        Drop_Item(self, self->item);
        self->item = NULL;
    }

    if (self->deathtarget)
        self->target = self->deathtarget;

    if (!self->target)
        return;

    G_UseTargets(self, self->enemy);
}


//============================================================================

qboolean monster_start(edict_t *self)
{
    if (deathmatch->value) {
        G_FreeEdict(self);
        return qfalse;
    }

    if ((self->spawnflags & 4) && !(self->monsterinfo.aiflags & AI_GOOD_GUY)) {
        self->spawnflags &= ~4;
        self->spawnflags |= 1;
//      gi.dprintf("fixed spawnflags on %s at %s\n", self->classname, vtos(self->s.origin));
    }

    if (!(self->monsterinfo.aiflags & AI_GOOD_GUY) && !(self->monsterinfo.aiflags & AI_MEDIC2))
        level.total_monsters++;

    self->nextthink = level.time + FRAMETIME;
    self->svflags |= SVF_MONSTER;
    self->s.renderfx |= RF_FRAMELERP;
    self->takedamage = DAMAGE_AIM;
    self->air_finished = level.time + 12;
    self->use = monster_use;

	//memset(self->monsterinfo.dot_time, 0, MAX_DOTI * sizeof(float));
	//memset(self->monsterinfo.dot_amt, 0, MAX_DOTI * sizeof(float));

	// Rroff - allow some monsters to have a higher max health than their base health
	// with the changes to parasites they can drain health from their target and become tougher
	// need to make sure this doesn't conflict with anything

	if (!(self->max_health))
	{
		self->max_health = self->health;
	}

    self->clipmask = MASK_MONSTERSOLID;

    self->s.skinnum = 0;
    self->deadflag = DEAD_NO;
    self->svflags &= ~SVF_DEADMONSTER;

    if (!self->monsterinfo.checkattack)
        self->monsterinfo.checkattack = M_CheckAttack;
    VectorCopy(self->s.origin, self->s.old_origin);

    if (st.item) {
        self->item = FindItemByClassname(st.item);
        if (!self->item)
            gi.dprintf("%s at %s has bad item: %s\n", self->classname, vtos(self->s.origin), st.item);
    }

    // randomize what frame they start on
    if (self->monsterinfo.currentmove)
        self->s.frame = self->monsterinfo.currentmove->firstframe + (rand() % (self->monsterinfo.currentmove->lastframe - self->monsterinfo.currentmove->firstframe + 1));

    return qtrue;
}

void monster_start_go(edict_t *self)
{
    vec3_t  v;

    if (self->health <= 0)
        return;

    // check for target to combat_point and change to combattarget
    if (self->target) {
        qboolean    notcombat;
        qboolean    fixup;
        edict_t     *target;

        target = NULL;
        notcombat = qfalse;
        fixup = qfalse;
        while ((target = G_Find(target, FOFS(targetname), self->target)) != NULL) {
            if (strcmp(target->classname, "point_combat") == 0) {
                self->combattarget = self->target;
                fixup = qtrue;
            } else {
                notcombat = qtrue;
            }
        }
        if (notcombat && self->combattarget)
            gi.dprintf("%s at %s has target with mixed types\n", self->classname, vtos(self->s.origin));
        if (fixup)
            self->target = NULL;
    }

    // validate combattarget
    if (self->combattarget) {
        edict_t     *target;

        target = NULL;
        while ((target = G_Find(target, FOFS(targetname), self->combattarget)) != NULL) {
            if (strcmp(target->classname, "point_combat") != 0) {
                gi.dprintf("%s at (%i %i %i) has a bad combattarget %s : %s at (%i %i %i)\n",
                           self->classname, (int)self->s.origin[0], (int)self->s.origin[1], (int)self->s.origin[2],
                           self->combattarget, target->classname, (int)target->s.origin[0], (int)target->s.origin[1],
                           (int)target->s.origin[2]);
            }
        }
    }

    if (self->target) {
        self->goalentity = self->movetarget = G_PickTarget(self->target);
        if (!self->movetarget) {
            gi.dprintf("%s can't find target %s at %s\n", self->classname, self->target, vtos(self->s.origin));
            self->target = NULL;
            self->monsterinfo.pausetime = 100000000;
            self->monsterinfo.stand(self);
        } else if (strcmp(self->movetarget->classname, "path_corner") == 0) {
            VectorSubtract(self->goalentity->s.origin, self->s.origin, v);
            self->ideal_yaw = self->s.angles[YAW] = vectoyaw(v);
            self->monsterinfo.walk(self);
            self->target = NULL;
        } else {
            self->goalentity = self->movetarget = NULL;
            self->monsterinfo.pausetime = 100000000;
            self->monsterinfo.stand(self);
        }
    } else {
        self->monsterinfo.pausetime = 100000000;
        self->monsterinfo.stand(self);
    }

    self->think = monster_think;
    self->nextthink = level.time + FRAMETIME;
}


void walkmonster_start_go(edict_t *self)
{
    if (!(self->spawnflags & 2) && level.time < 1) {
        M_droptofloor(self);

        if (self->groundentity)
            if (!M_walkmove(self, 0, 0))
                gi.dprintf("%s in solid at %s\n", self->classname, vtos(self->s.origin));
    }

    if (!self->yaw_speed)
        self->yaw_speed = 20;
    self->viewheight = 25;

    monster_start_go(self);

    if (self->spawnflags & 2)
        monster_triggered_start(self);
}

void walkmonster_start(edict_t *self)
{
    self->think = walkmonster_start_go;
    monster_start(self);
}


void flymonster_start_go(edict_t *self)
{
    if (!M_walkmove(self, 0, 0))
        gi.dprintf("%s in solid at %s\n", self->classname, vtos(self->s.origin));

    if (!self->yaw_speed)
        self->yaw_speed = 10;
    self->viewheight = 25;

    monster_start_go(self);

    if (self->spawnflags & 2)
        monster_triggered_start(self);
}


void flymonster_start(edict_t *self)
{
    self->flags |= FL_FLY;
    self->think = flymonster_start_go;
    monster_start(self);
}


void swimmonster_start_go(edict_t *self)
{
    if (!self->yaw_speed)
        self->yaw_speed = 10;
    self->viewheight = 10;

    monster_start_go(self);

    if (self->spawnflags & 2)
        monster_triggered_start(self);
}

void swimmonster_start(edict_t *self)
{
    self->flags |= FL_SWIM;
    self->think = swimmonster_start_go;
    monster_start(self);
}
