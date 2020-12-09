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
// g_turret.c

#include "g_local.h"
#include "m_player.h"


void AnglesNormalize(vec3_t vec)
{
    while (vec[0] > 360)
        vec[0] -= 360;
    while (vec[0] < 0)
        vec[0] += 360;
    while (vec[1] > 360)
        vec[1] -= 360;
    while (vec[1] < 0)
        vec[1] += 360;
}

float SnapToEights(float x)
{
    x *= 8.0;
    if (x > 0.0)
        x += 0.5;
    else
        x -= 0.5;
    return 0.125 * (int)x;
}


void turret_blocked(edict_t *self, edict_t *other)
{
    edict_t *attacker;

    if (other->takedamage) {
        if (self->teammaster->owner)
            attacker = self->teammaster->owner;
        else
            attacker = self->teammaster;
        T_Damage(other, self, attacker, vec3_origin, other->s.origin, vec3_origin, self->teammaster->dmg, 10, 0, MOD_CRUSH);
    }
}

/*QUAKED turret_breach (0 0 0) ?
This portion of the turret can change both pitch and yaw.
The model  should be made with a flat pitch.
It (and the associated base) need to be oriented towards 0.
Use "angle" to set the starting angle.

"speed"     default 50
"dmg"       default 10
"angle"     point this forward
"target"    point this at an info_notnull at the muzzle tip
"minpitch"  min acceptable pitch angle : default -30
"maxpitch"  max acceptable pitch angle : default 30
"minyaw"    min acceptable yaw angle   : default 0
"maxyaw"    max acceptable yaw angle   : default 360
*/

void turret_breach_fire(edict_t *self)
{
    vec3_t  f, r, u;
    vec3_t  start;
    int     damage;
    int     speed;

    AngleVectors(self->s.angles, f, r, u);
    VectorMA(self->s.origin, self->move_origin[0], f, start);
    VectorMA(start, self->move_origin[1], r, start);
    VectorMA(start, self->move_origin[2], u, start);

    damage = 100 + random() * 50;
    speed = 550 + 50 * skill->value;
    fire_rocket(self->teammaster->owner, start, f, damage, speed, 150, damage);
    gi.positioned_sound(start, self, CHAN_WEAPON, gi.soundindex("weapons/rocklf1a.wav"), 1, ATTN_NORM, 0);
}

void turret_breach_think(edict_t *self)
{
    edict_t *ent;
    vec3_t  current_angles;
    vec3_t  delta;

    VectorCopy(self->s.angles, current_angles);
    AnglesNormalize(current_angles);

    AnglesNormalize(self->move_angles);
    if (self->move_angles[PITCH] > 180)
        self->move_angles[PITCH] -= 360;

    // clamp angles to mins & maxs
    if (self->move_angles[PITCH] > self->pos1[PITCH])
        self->move_angles[PITCH] = self->pos1[PITCH];
    else if (self->move_angles[PITCH] < self->pos2[PITCH])
        self->move_angles[PITCH] = self->pos2[PITCH];

    if ((self->move_angles[YAW] < self->pos1[YAW]) || (self->move_angles[YAW] > self->pos2[YAW])) {
        float   dmin, dmax;

        dmin = fabs(self->pos1[YAW] - self->move_angles[YAW]);
        if (dmin < -180)
            dmin += 360;
        else if (dmin > 180)
            dmin -= 360;
        dmax = fabs(self->pos2[YAW] - self->move_angles[YAW]);
        if (dmax < -180)
            dmax += 360;
        else if (dmax > 180)
            dmax -= 360;
        if (fabs(dmin) < fabs(dmax))
            self->move_angles[YAW] = self->pos1[YAW];
        else
            self->move_angles[YAW] = self->pos2[YAW];
    }

    VectorSubtract(self->move_angles, current_angles, delta);
    if (delta[0] < -180)
        delta[0] += 360;
    else if (delta[0] > 180)
        delta[0] -= 360;
    if (delta[1] < -180)
        delta[1] += 360;
    else if (delta[1] > 180)
        delta[1] -= 360;
    delta[2] = 0;

    if (delta[0] > self->speed * FRAMETIME)
        delta[0] = self->speed * FRAMETIME;
    if (delta[0] < -1 * self->speed * FRAMETIME)
        delta[0] = -1 * self->speed * FRAMETIME;
    if (delta[1] > self->speed * FRAMETIME)
        delta[1] = self->speed * FRAMETIME;
    if (delta[1] < -1 * self->speed * FRAMETIME)
        delta[1] = -1 * self->speed * FRAMETIME;

    VectorScale(delta, 1.0 / FRAMETIME, self->avelocity);

    self->nextthink = level.time + FRAMETIME;

    for (ent = self->teammaster; ent; ent = ent->teamchain)
        ent->avelocity[1] = self->avelocity[1];

    // if we have adriver, adjust his velocities
    if (self->owner) {
        float   angle;
        float   target_z;
        float   diff;
        vec3_t  target;
        vec3_t  dir;

        // angular is easy, just copy ours
        self->owner->avelocity[0] = self->avelocity[0];
        self->owner->avelocity[1] = self->avelocity[1];

        // x & y
        angle = self->s.angles[1] + self->owner->move_origin[1];
        angle *= (M_PI * 2 / 360);
        target[0] = SnapToEights(self->s.origin[0] + cos(angle) * self->owner->move_origin[0]);
        target[1] = SnapToEights(self->s.origin[1] + sin(angle) * self->owner->move_origin[0]);
        target[2] = self->owner->s.origin[2];

        VectorSubtract(target, self->owner->s.origin, dir);
        self->owner->velocity[0] = dir[0] * 1.0 / FRAMETIME;
        self->owner->velocity[1] = dir[1] * 1.0 / FRAMETIME;

        // z
        angle = self->s.angles[PITCH] * (M_PI * 2 / 360);
        target_z = SnapToEights(self->s.origin[2] + self->owner->move_origin[0] * tan(angle) + self->owner->move_origin[2]);

        diff = target_z - self->owner->s.origin[2];
        self->owner->velocity[2] = diff * 1.0 / FRAMETIME;

        if (self->spawnflags & 65536) {
            turret_breach_fire(self);
            self->spawnflags &= ~65536;
        }
    }
}

void turret_breach_finish_init(edict_t *self)
{
    // get and save info for muzzle location
    if (!self->target) {
        gi.dprintf("%s at %s needs a target\n", self->classname, vtos(self->s.origin));
    } else {
        self->target_ent = G_PickTarget(self->target);
        VectorSubtract(self->target_ent->s.origin, self->s.origin, self->move_origin);
        G_FreeEdict(self->target_ent);
    }

    self->teammaster->dmg = self->dmg;
    self->think = turret_breach_think;
    self->think(self);
}

void SP_turret_breach(edict_t *self)
{
    self->solid = SOLID_BSP;
    self->movetype = MOVETYPE_PUSH;
    gi.setmodel(self, self->model);

    if (!self->speed)
        self->speed = 50;
    if (!self->dmg)
        self->dmg = 10;

    if (!st.minpitch)
        st.minpitch = -30;
    if (!st.maxpitch)
        st.maxpitch = 30;
    if (!st.maxyaw)
        st.maxyaw = 360;

    self->pos1[PITCH] = -1 * st.minpitch;
    self->pos1[YAW]   = st.minyaw;
    self->pos2[PITCH] = -1 * st.maxpitch;
    self->pos2[YAW]   = st.maxyaw;

    self->ideal_yaw = self->s.angles[YAW];
    self->move_angles[YAW] = self->ideal_yaw;

    self->blocked = turret_blocked;

    self->think = turret_breach_finish_init;
    self->nextthink = level.time + FRAMETIME;
    gi.linkentity(self);
}


/*QUAKED turret_base (0 0 0) ?
This portion of the turret changes yaw only.
MUST be teamed with a turret_breach.
*/

void SP_turret_base(edict_t *self)
{
    self->solid = SOLID_BSP;
    self->movetype = MOVETYPE_PUSH;
    gi.setmodel(self, self->model);
    self->blocked = turret_blocked;
    gi.linkentity(self);
}


/*QUAKED turret_driver (1 .5 0) (-16 -16 -24) (16 16 32)
Must NOT be on the team with the rest of the turret parts.
Instead it must target the turret_breach.
*/

void infantry_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage);
void infantry_stand(edict_t *self);
void monster_use(edict_t *self, edict_t *other, edict_t *activator);

void turret_driver_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
    edict_t *ent;

    // level the gun
    self->target_ent->move_angles[0] = 0;

    // remove the driver from the end of them team chain
    for (ent = self->target_ent->teammaster; ent->teamchain != self; ent = ent->teamchain)
        ;
    ent->teamchain = NULL;
    self->teammaster = NULL;
    self->flags &= ~FL_TEAMSLAVE;

    self->target_ent->owner = NULL;
    self->target_ent->teammaster->owner = NULL;

    infantry_die(self, inflictor, attacker, damage);
}

qboolean FindTarget(edict_t *self);

void turret_driver_think(edict_t *self)
{
    vec3_t  target;
    vec3_t  dir;
    float   reaction_time;

    self->nextthink = level.time + FRAMETIME;

    if (self->enemy && (!self->enemy->inuse || self->enemy->health <= 0))
        self->enemy = NULL;

    if (!self->enemy) {
        if (!FindTarget(self))
            return;
        self->monsterinfo.trail_time = level.time;
        self->monsterinfo.aiflags &= ~AI_LOST_SIGHT;
    } else {
        if (visible(self, self->enemy)) {
            if (self->monsterinfo.aiflags & AI_LOST_SIGHT) {
                self->monsterinfo.trail_time = level.time;
                self->monsterinfo.aiflags &= ~AI_LOST_SIGHT;
            }
        } else {
            self->monsterinfo.aiflags |= AI_LOST_SIGHT;
            return;
        }
    }

    // let the turret know where we want it to aim
    VectorCopy(self->enemy->s.origin, target);
    target[2] += self->enemy->viewheight;
    VectorSubtract(target, self->target_ent->s.origin, dir);
    vectoangles(dir, self->target_ent->move_angles);

    // decide if we should shoot
    if (level.time < self->monsterinfo.attack_finished)
        return;

    reaction_time = (3 - skill->value) * 1.0;
    if ((level.time - self->monsterinfo.trail_time) < reaction_time)
        return;

    self->monsterinfo.attack_finished = level.time + reaction_time + 1.0;
    //FIXME how do we really want to pass this along?
    self->target_ent->spawnflags |= 65536;
}

void turret_driver_link(edict_t *self)
{
    vec3_t  vec;
    edict_t *ent;

    self->think = turret_driver_think;
    self->nextthink = level.time + FRAMETIME;

    self->target_ent = G_PickTarget(self->target);
    self->target_ent->owner = self;
    self->target_ent->teammaster->owner = self;
    VectorCopy(self->target_ent->s.angles, self->s.angles);

    vec[0] = self->target_ent->s.origin[0] - self->s.origin[0];
    vec[1] = self->target_ent->s.origin[1] - self->s.origin[1];
    vec[2] = 0;
    self->move_origin[0] = VectorLength(vec);

    VectorSubtract(self->s.origin, self->target_ent->s.origin, vec);
    vectoangles(vec, vec);
    AnglesNormalize(vec);
    self->move_origin[1] = vec[1];

    self->move_origin[2] = self->s.origin[2] - self->target_ent->s.origin[2];

    // add the driver to the end of them team chain
    for (ent = self->target_ent->teammaster; ent->teamchain; ent = ent->teamchain)
        ;
    ent->teamchain = self;
    self->teammaster = self->target_ent->teammaster;
    self->flags |= FL_TEAMSLAVE;
}

void SP_turret_driver(edict_t *self)
{
    if (deathmatch->value) {
        G_FreeEdict(self);
        return;
    }

    self->movetype = MOVETYPE_PUSH;
    self->solid = SOLID_BBOX;
    self->s.modelindex = gi.modelindex("models/monsters/infantry/tris.md2");
    VectorSet(self->mins, -16, -16, -24);
    VectorSet(self->maxs, 16, 16, 32);

    self->health = 100;
    self->gib_health = 0;
    self->mass = 200;
    self->viewheight = 24;

    self->die = turret_driver_die;
    self->monsterinfo.stand = infantry_stand;

    self->flags |= FL_NO_KNOCKBACK;

    level.total_monsters++;

    self->svflags |= SVF_MONSTER;
    self->s.renderfx |= RF_FRAMELERP;
    self->takedamage = DAMAGE_AIM;
    self->use = monster_use;
    self->clipmask = MASK_MONSTERSOLID;
    VectorCopy(self->s.origin, self->s.old_origin);
    self->monsterinfo.aiflags |= AI_STAND_GROUND | AI_DUCKED;

    if (st.item) {
        self->item = FindItemByClassname(st.item);
        if (!self->item)
            gi.dprintf("%s at %s has bad item: %s\n", self->classname, vtos(self->s.origin), st.item);
    }

    self->think = turret_driver_link;
    self->nextthink = level.time + FRAMETIME;

    gi.linkentity(self);
}


// Rroff - player turret below

void ChangePitch(edict_t *ent)
{
	float	ideal;
	float	current;
	float	move;
	float	speed;

	current = anglemod(ent->s.angles[PITCH]);
	ideal = ent->ideal_pitch;

	if (current == ideal)
		return;

	move = ideal - current;
	speed = ent->yaw_speed;
	if (ideal > current)
	{
		if (move >= 180)
			move = move - 360;
	}
	else
	{
		if (move <= -180)
			move = move + 360;
	}
	if (move > 0)
	{
		if (move > speed)
			move = speed;
	}
	else
	{
		if (move < -speed)
			move = -speed;
	}

	ent->s.angles[PITCH] = anglemod(current + move);
}

// Find a target for turrets

edict_t *pturret_FindMonster(edict_t *self, qboolean inview)
{
	edict_t		*ent = NULL;
	edict_t		*best = NULL;
	qboolean	old_valid = qfalse;

	// check dead flag as well as health
	// check range, try and defend owner?
	// make sure we check owner is valid and kill it if player left, etc.

	// for some reason this rather basic, dumb section of code
	// appears to have somewhat intelligent target selection in action

	while ((ent = findradius(ent, self->s.origin, 1536)) != NULL)
	{
		if (ent->client)
			continue;
		if (ent == self)
			continue;
		if (!(ent->svflags & SVF_MONSTER))
			continue;
		if (ent->monsterinfo.aiflags & AI_GOOD_GUY)
			continue;
		//		if (ent->owner) // left over from another use but left here incase of reuse
		//			continue;
		if (ent->health <= 0)
			continue;
		//		if (ent->nextthink)
		//			continue;
//		if ((inview) && (!infront(self, ent)))
//			continue;
		if (!visible(self, ent))
			continue;

		// Rroff - testing
		// any monster we find now that doesn't have an enemy
		// set them on the turret owner - don't want too much
		// of a free ride
		if (!ent->enemy)
		{
			if (self->teammaster->inuse && self->teammaster->client && self->teammaster->health > 0)
				ent->enemy = self->teammaster;
			else
				ent->enemy = self;

			FoundTarget(ent);
		}
		//

		if ((inview) && (!infront(self, ent)))
			continue;

		if (self->oldenemy)
		{
			if (ent == self->oldenemy)
			{
				old_valid = qtrue;
				continue;
			}
		}
		if (!best)
		{
			best = ent;
			continue;
		}
		best = ent;
	}

	if (!(best) && (old_valid))
		return self->oldenemy;

	return best;
}

void pturret_fire(edict_t *self)
{
	vec3_t	f, r, u, ri;
	vec3_t	start;
	int		damage;
	int		speed;
	int		i, rm;
	float	offset = 0;

	AngleVectors(self->s.angles, f, r, u);

	VectorMA(self->s.origin, 24, f, start);
	VectorMA(start, 1.4, u, start);

	VectorCopy(r, ri);
	VectorInverse(ri);

	// Rroff - probably reduce this a bit - shouldn't outperform a player with chaingun
	//damage = 8;
	damage = 6;

	for (i = 0; i < 3; i++)
	{
		/*if ((g_sillygore->value) && (g_sillygore->integer == 2))
		{
			if (random() < 0.5)
			{
				if (random() > 0.3)
					eject_brass(self, start, ri, 3);
				else
					eject_brass(self, start, r, 4);
			}
		}*/

		//rm = 1 + (rand() % 5);

		//gi.positioned_sound(start, self, CHAN_WEAPON, gi.soundindex(va("weapons/machgf%ib.wav", rm)), 1, ATTN_NORM, offset);

		//offset += 0.066;

		fire_bullet(self->teamchain->teamchain, start, f, damage, 2, 500, 700, MOD_PTURRET);
	}

	// Bit underwhelming
	if ((g_sillygore->value) && (g_sillygore->integer == 2))
	{
		if (random() < 0.5)
		{
			if (random() > 0.3)
				eject_brass(self, start, ri, 3);
			else
				eject_brass(self, start, ri, 4);
		}
	}

	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(self - g_edicts);
	gi.WriteByte(MZ_CHAINGUN3);
	gi.multicast(self->s.origin, MULTICAST_PVS);


	VectorMA(start, 1, f, start);

	// horrid but sounds more constant than other implementations/using muzzleflashes
	// could shove into for loop above and increment start offset

	/*rm = 1 + (rand() % 5);

	gi.positioned_sound(start, self, CHAN_WEAPON, gi.soundindex(va("weapons/machgf%ib.wav", rm)), 1, ATTN_NORM, 0);

	rm = 1 + (rand() % 5);

	gi.positioned_sound(start, self, CHAN_WEAPON, gi.soundindex(va("weapons/machgf%ib.wav", rm)), 1, ATTN_NORM, 0);

	rm = 1 + (rand() % 5);

	gi.positioned_sound(start, self, CHAN_WEAPON, gi.soundindex(va("weapons/machgf%ib.wav", rm)), 1, ATTN_NORM, 0.1);

	rm = 1 + (rand() % 5);

	gi.positioned_sound(start, self, CHAN_WEAPON, gi.soundindex(va("weapons/machgf%ib.wav", rm)), 1, ATTN_NORM, 0.1);

	rm = 1 + (rand() % 5);

	gi.positioned_sound(start, self, CHAN_WEAPON, gi.soundindex(va("weapons/machgf%ib.wav", rm)), 1, ATTN_NORM, 0.2);
	*/
	

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_SHOTGUN); // alt TE_SPARKS
	gi.WritePosition(start);
	gi.WriteDir(f);
	gi.multicast(start, MULTICAST_PVS);

	if (random() < 0.2)
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_SPARKS);
		gi.WritePosition(start);
		gi.WriteDir(f);
		gi.multicast(start, MULTICAST_PVS);
	}

	// horrid hack

	/*gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(self - g_edicts);
	gi.WriteByte(MZ_NUKE2);
	gi.multicast(self->s.origin, MULTICAST_PVS);*/

}

// self is the base, chain is the support. chain->chain the driver (which does the thinking and shooting)
//chain->chain->chain is the barrel if not included in the driver
void pturret_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	G_FreeEdict(self->chain->chain->chain); // LOL

	// shouldn't be different between base and driver
	if (self->teammaster->inuse)
		self->teammaster->turret_ammo = 0;

	G_FreeEdict(self->chain->chain);
	G_FreeEdict(self->chain);

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_EXPLOSION2);
	gi.WritePosition(self->s.origin);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	G_FreeEdict(self);
}

void pturret_think(edict_t *self)
{
	edict_t	*target = NULL;
	edict_t	*ent;
	qboolean tvalid, chainsound;

	vec3_t	aimpoint, dir;

	int		h;


	if (level.time > self->timestamp + 180 || self->teammaster->client->turret_time != self->timestamp)
	{
		/*gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BFG_BIGEXPLOSION);
		gi.WritePosition(self->s.origin);
		gi.multicast(self->s.origin, MULTICAST_PVS);

		gi.sound(self, CHAN_VOICE, gi.soundindex("items/respawn1.wav"), 1, ATTN_NORM, 0);*/

		// Rroff - uurrrh yeah
		// I'm sure this can be done better - will rewrite later with turret specific
		// variables as well

		self->teamchain->teamchain->die(self->teamchain->teamchain, NULL, NULL, 0, vec3_origin);
		
		return;
	}

	// update health on the turret
	ent = self->teamchain->teamchain;

	if (!g_healthbars->value) {
		if (ent->chain->s.modelindex4 != 0)
			ent->chain->s.modelindex4 = 0;
	}
	else {
		h = ((100 / (float)ent->max_health) * ent->health) / 10;
		if (h < 0)
			h = 0;

		if (h == 0 && ent->health > 0)
			h = 1;

		if (h > 10)
			h = 10;

		if (h != ent->moded)
		{
			if (h == 0)
			{
				ent->moded = h;
				ent->chain->s.modelindex4 = 0;
			}
			else {
				ent->moded = h;
				ent->chain->s.modelindex4 = gi.modelindex(va("sprites/health%i.sp2", h));
			}
		}
	}

	// air_finished = records last time we had line of sight on our target
	// wait = built in delay before acquiring a target
	// delay = delay for the idle looking around sequence
	// random = last time turret fired for rate of fire control

	chainsound = qfalse;

	if (self->enemy && (!self->enemy->inuse || self->enemy->health <= 0))
	{
		self->enemy = NULL;
		self->wait = level.time;
	}

	if (self->enemy && (level.time > self->air_finished + 5))
	{
		self->oldenemy = self->enemy;
		self->enemy = NULL;
		self->wait = level.time;
		gi.sound(self, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
		//gi.dprintf("Lost target\n");
	}

	if (!self->enemy)
	{
		if ((level.time > self->random + 0.1) && (level.time < self->random + 1))
		{
			self->chain->s.angles[ROLL] += 100;
			AnglesNormalize(self->chain->s.angles);
		}

		if ((self->s.sound == gi.soundindex("weapons/chngnl1a.wav")))
			gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/chngnd1a.wav"), 1, ATTN_NORM, 0);

		self->s.sound = 0;
		// acquire target with a bit of delay/don't search every frame
		if (level.time > self->wait + 1)
		{
			target = pturret_FindMonster(self, qtrue);
			self->wait = level.time;
		}

		if (target)
		{
			self->enemy = target;
			gi.sound(self, CHAN_VOICE, gi.soundindex("misc/comp_up.wav"), 1, ATTN_NORM, 0);
			self->random = level.time;

			// burst fire mode
			// self->powerarmor_time = level.time + 2;
		}

		if (anglemod(self->s.angles[YAW]) != anglemod(self->ideal_yaw))
		{
			M_ChangeYaw(self);
			self->teamchain->s.angles[YAW] = self->s.angles[YAW];
			self->chain->s.angles[YAW] = self->s.angles[YAW];

			self->s.sound = gi.soundindex("boss3/w_loop.wav");
		}

		ChangePitch(self);

		// make sure the barrel pitch is the same as the driver
		self->chain->s.angles[PITCH] = self->s.angles[PITCH];

		if (level.time > self->delay)
		{

			while (self->ideal_yaw > 360)
				self->ideal_yaw -= 360;
			while (self->ideal_yaw < 0)
				self->ideal_yaw += 360;

			self->ideal_yaw += 180 + (-45 + (rand() % 90));
			if (self->ideal_yaw > 360)
				self->ideal_yaw -= 360;

			//self->ideal_yaw = random() * 360;
			//self->ideal_yaw += 91 + (random() * 179);
			//self->ideal_yaw += 180 + (-90 + (crandom() * 180));
			/*self->ideal_yaw += 180 + (-45 + (crandom() * 90));
			if (self->ideal_yaw > 360)
				self->ideal_yaw -= 360;*/

			self->ideal_pitch = 0;
			self->delay = level.time + 1 + random() * 3;
		}

	}

	if (self->enemy)
	{
		// check the pitch to target so we aren't endlessly shooting at an enemy
		// we can't turn to aim at i.e. flying above

		// if we lose sight of an enemy reset if more than a few seconds have passed
		if (visible(self, self->enemy)) // might change this a bit so less costly in visibility checks
		{
			VectorCopy(self->enemy->s.origin, aimpoint);

			// add some slight variation to where we are aiming
			// need to refine this a bit so we aren't nodding around
			// when not firing, etc. putting it in the firing bit below
			// simulates some recoil

			/*if (level.time > self->random + 0.1)
			{
				if (level.time < self->powerarmor_time)
				{
					self->speed = crandom()*(self->enemy->viewheight - 2);

					pturret_fire(self);

					self->s.sound = gi.soundindex("weapons/chngnl1a.wav");
					chainsound = true;

					self->chain->s.angles[ROLL] += 100;
					AnglesNormalize(self->chain->s.angles);

					self->random = level.time;
				} else {
					self->random = level.time + 2;
					self->powerarmor_time = level.time + 4;
				}
			}*/

			if (level.time > self->random + 0.1)
			{

				if (self->turret_ammo >= 3)
				{
					self->turret_ammo -= 3;
				}
				else {
					self->teamchain->teamchain->die(self->teamchain->teamchain, NULL, NULL, 0, vec3_origin);

					return;
				}

				self->speed = crandom()*(self->enemy->viewheight - 2);

				pturret_fire(self);

				self->s.sound = gi.soundindex("weapons/chngnl1a.wav");
				chainsound = qtrue;

				self->chain->s.angles[ROLL] += 100;
				AnglesNormalize(self->chain->s.angles);

				self->random = level.time;
			}


			//if (!chainsound && (level.time > self->random + 0.3))
			//{
			//	if ((self->count == 2) && (self->s.sound == gi.soundindex("weapons/chngnl1a.wav")))
			//		gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/chngnd1a.wav"), 1, ATTN_NORM, 0);
			//	self->s.sound = 0;
			//}

			/*gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_BFG_LASER);
			gi.WritePosition(self->s.origin);
			gi.WritePosition(self->enemy->s.origin);
			gi.multicast(self->s.origin, MULTICAST_PHS);
			*/


			// aimpoint[2] += self->enemy->viewheight;

			aimpoint[2] += self->speed;
			VectorSubtract(aimpoint, self->s.origin, dir);
			vectoangles(dir, dir);

			AnglesNormalize(dir);

			if (dir[PITCH] > 180)
				dir[PITCH] -= 360;

			// don't count a target outside of our pitch as valid indefinitely
			// need to adjust targeting so we don't immediately retarget this one though
			// doesn't seem to work quite right but an interesting effect so leaving it for now
			// doesn't seem to break things too much

			tvalid = qtrue;
			// clamp angles to mins & maxs
			if (dir[PITCH] > self->pos1[PITCH])
			{
				dir[PITCH] = self->pos1[PITCH];
				tvalid = qfalse;
			}
			else if (dir[PITCH] < self->pos2[PITCH])
			{
				dir[PITCH] = self->pos2[PITCH];
				tvalid = qfalse;
			}

			if (tvalid)
				self->air_finished = level.time; // last recorded sighting

			// not really necessary with the current setup

			if ((dir[YAW] < self->pos1[YAW]) || (dir[YAW] > self->pos2[YAW]))
			{
				float	dmin, dmax;

				dmin = fabs(self->pos1[YAW] - dir[YAW]);
				if (dmin < -180)
					dmin += 360;
				else if (dmin > 180)
					dmin -= 360;
				dmax = fabs(self->pos2[YAW] - dir[YAW]);
				if (dmax < -180)
					dmax += 360;
				else if (dmax > 180)
					dmax -= 360;
				if (fabs(dmin) < fabs(dmax))
					dir[YAW] = self->pos1[YAW];
				else
					dir[YAW] = self->pos2[YAW];
			}

			self->ideal_yaw = dir[YAW];
			self->ideal_pitch = dir[PITCH];

			if (anglemod(self->s.angles[YAW]) != anglemod(self->ideal_yaw))
			{
				M_ChangeYaw(self);
				self->teamchain->s.angles[YAW] = self->s.angles[YAW];
				self->chain->s.angles[YAW] = self->s.angles[YAW];

				if (!chainsound)
				{
					self->s.sound = gi.soundindex("boss3/w_loop.wav");
				}
			}

			ChangePitch(self);

			self->chain->s.angles[PITCH] = self->s.angles[PITCH];

		}
		else { // enemy not visible but we have a target
			if ((level.time > self->random + 0.1) && (level.time < self->random + 1))
			{
				self->chain->s.angles[ROLL] += 100;
				AnglesNormalize(self->chain->s.angles);
			}
			if (level.time > self->random + 0.3)
			{
				if ((self->s.sound == gi.soundindex("weapons/chngnl1a.wav")))
					gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/chngnd1a.wav"), 1, ATTN_NORM, 0);
				self->s.sound = 0;
			}
		}
	}

	if (self->teammaster->inuse)
		self->teammaster->turret_ammo = self->turret_ammo;

	self->nextthink = level.time + FRAMETIME;
}

void pturret_playeruse(edict_t *self, edict_t *other)
{
	if (other == self->teammaster)
		self->die(self, NULL, NULL, 0, vec3_origin);
}

qboolean pturret_deploy(edict_t *ent)
{
	vec3_t		offset, forward, right, start, end;
	vec3_t		mins, maxs;
	trace_t		tr;
	edict_t		*turret, *tsup, *tdrv, *tbl;

	if (ent->client->weaponstate == WEAPON_FIRING)
		return qfalse;

	AngleVectors(ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 0, ent->viewheight);
	G_ProjectSource(ent->s.origin, offset, forward, right, start);
	VectorMA(start, 384, forward, end);

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

	if (tr.fraction == 1.0)
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
		return qfalse; // need to hit something
	}

	// if tr.ent is classname "pturret_base" and teammaster is ent
	// return qtrue and add ammo to the turret ->chain->chain
	// plus reset its time

	if ((Q_stricmp(tr.ent->classname, "pturret_base") == 0) && (tr.ent->teammaster == ent))
	{
		// resupplying turret also slightly extends its life span
		// and repairs health
		ent->client->turret_time = level.time + 30;
		tr.ent->chain->chain->timestamp = ent->client->turret_time;
		tr.ent->chain->chain->turret_ammo += 252;
		tr.ent->health = 200;
		gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/ar2_pkup.wav"), 1, ATTN_NORM, 0);
		return qtrue;
	}

	if (tr.plane.normal[2])
	{
		if (tr.plane.normal[2] < 0.7)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
			return qfalse;
		}
	}

	if (tr.ent->s.modelindex != 1)
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
		return qfalse;
	}

	if (tr.surface && tr.surface->flags & SURF_SKY)
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
		return qfalse;
	}

	VectorSet(mins, -24, -24, -24);
	VectorSet(maxs, 24, 24, 24);

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
			return qfalse;
		}

		if (tr.plane.normal[2])
		{
			if (tr.plane.normal[2] < 0.7)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
				return qfalse;
			}
		}

		if (tr.ent->s.modelindex != 1)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);
			return qfalse;
		}

		ent->client->turret_time = level.time;

		turret = G_Spawn();

		turret->classname = "pturret_base";

		VectorCopy(tr.endpos, end);
		end[2] -= 24;

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_TELEPORT_EFFECT);
		gi.WritePosition(tr.endpos); // move to end?
		gi.multicast(tr.endpos, MULTICAST_PVS);

		VectorCopy(end, turret->s.origin);
		VectorSet(turret->mins, -24, -24, 0);
		VectorSet(turret->maxs, 24, 24, 32);

		turret->solid = SOLID_BBOX;
		turret->movetype = MOVETYPE_NONE;

		turret->health = 200;
		turret->max_health = 200;
		turret->moded = 0; // this reflects the updated health frame
		turret->monsterinfo.aiflags |= (AI_GOOD_GUY | AI_PLAYERPROXY);

		turret->die = pturret_die;
		turret->playeruse = pturret_playeruse;
		turret->takedamage = DAMAGE_YES;
		turret->viewheight = 24;
		turret->teammaster = ent;

		turret->s.modelindex = gi.modelindex("models/weapons/turret/tris_base.md2");

		gi.linkentity(turret);

		// ================

		tsup = G_Spawn();
		tsup->classname = "pturret_support";
		VectorCopy(end, tsup->s.origin);
		tsup->s.origin[2] += 5;
		tsup->s.modelindex = gi.modelindex("models/weapons/turret2/tris_support.md2");

		tsup->solid = SOLID_NOT;
		tsup->movetype = MOVETYPE_NOCLIP;

		gi.linkentity(tsup);

		tsup->teamchain = turret;
		turret->chain = tsup;

		// ================

		tdrv = G_Spawn();
		tdrv->classname = "pturret_driver";
		VectorCopy(end, tdrv->s.origin);

		tdrv->s.modelindex = gi.modelindex("models/weapons/turret2/tris_driver.md2");
		tdrv->s.origin[2] += 27.6;

		tdrv->solid = SOLID_NOT;
		tdrv->movetype = MOVETYPE_NOCLIP;
		tdrv->timestamp = ent->client->turret_time;
		tdrv->think = pturret_think;
		tdrv->nextthink = level.time + FRAMETIME;
		tdrv->pos1[PITCH] = -1 * -60;
		tdrv->pos1[YAW] = 0;
		tdrv->pos2[PITCH] = -1 * 60;
		tdrv->pos2[YAW] = 360;
		tdrv->yaw_speed = 10;
		tdrv->ideal_yaw = random() * 360;
		tdrv->ideal_pitch = 0;
		tdrv->delay = level.time + 1 + random() * 3;
		tdrv->wait = level.time;
		tdrv->teammaster = ent;
		//tdrv->turret_ammo = 501;
		tdrv->turret_ammo = 252;

		gi.linkentity(tdrv);

		tdrv->teamchain = tsup;
		tsup->chain = tdrv;

		// ================

		tbl = G_Spawn();
		tsup->classname = "pturret_barrel";
		VectorCopy(end, tbl->s.origin);
		tbl->s.origin[2] += 27.6;
		tbl->s.modelindex = gi.modelindex("models/weapons/turret2/tris_barrel.md2");
		tbl->solid = SOLID_NOT;
		tbl->movetype = MOVETYPE_NOCLIP;

		gi.linkentity(tbl);

		tbl->teamchain = tdrv;
		tdrv->chain = tbl;

		// ================

		gi.sound(turret, CHAN_VOICE, gi.soundindex("misc/tele_up.wav"), 1, ATTN_NORM, 0);

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

		return qtrue;
	}

	gi.sound(ent, CHAN_VOICE, gi.soundindex("world/lasoff1.wav"), 1, ATTN_NORM, 0);

	return qfalse;
}