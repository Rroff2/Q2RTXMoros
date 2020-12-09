/*
==============================================
Healbot
==============================================
*/

#include "g_local.h"
#include "m_healbot.h"

static int	sound_idle;
static int	sound_death;

static int	sound_hook_launch;
static int	sound_hook_hit;
static int	sound_hook_heal;
static int	sound_hook_retract;

edict_t *healbot_FindNearestMonster(edict_t *self)
{
	edict_t		*ent = NULL;
	edict_t		*best = NULL;
	edict_t		*pref = NULL;

	vec3_t		v;
	float		dist1, dist2 = 999999, nearest = 999999;

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
		if (ent->health > (ent->max_health * 0.8)) // don't worry about slightly wounded
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

// try and follow our owner
void healbot_chase(edict_t *self)
{
	if (self->teammaster)
	{
		//self->goalentity = self->movetarget = self->teammaster;
		//self->monsterinfo.run(self);
		if (self->teammaster->inuse)
		{
			if (self->teammaster->health <= 0)
			{
				self->die(self, NULL, NULL, 0, NULL);
				return;
			}
			self->enemy = self->teammaster;
			FoundTarget(self);
		} else {
			self->die(self, NULL, NULL, 0, NULL);
		}
	}
	else {
		self->die(self, NULL, NULL, 0, NULL);
	}
}

void healbot_triage(edict_t *self)
{
	float		h;
	edict_t		*ent;

	//if (self->velocity[2] < 100)
	//{
	//	self->velocity[2] = 100;
	//}

	ent = healbot_FindNearestMonster(self);

	if (!ent)
		return;

	// hopefully won't make the healbot go crazy

	if (self->enemy)
	{
		if (ent == self->enemy)
			return;
	}

	if (ent->health >= ent->max_health)
	{
		if (self->teammaster)
		{
			if (self->teammaster->inuse)
			{
				self->enemy = self->teammaster;
				FoundTarget(self);
				return;
			}
		}
	}

	// don't swap away until we've brought our current heal target upto half health
	if (self->enemy)
	{
		h = self->enemy->health / (float)self->enemy->max_health;
		if (h < 0.5)
			return;

		// don't swap to healing another target if it's health is over 50% and we
		// are healing someone already

		h = ent->health / (float)ent->max_health;

		if ((self->enemy->health < self->enemy->max_health) && (h > 0.5))
			return;
	}

	self->enemy = ent;
	FoundTarget(self);

}

// check the current level.sight_client to see if visible and if they need healing more
// than our current "enemy"

/*void healbot_triage_playerowned(edict_t *self)
{
	float	h;
	// don't bother if the current sight client isn't visible or is full health
	// currently we just use the looping sight client mechanics
	// rather than look for a player

	if (!level.sight_client)
		return;

	// hopefully won't make the healbot go crazy - if the current player it is checking
	// is OK then return to following owner

	if (level.sight_client == self->enemy)
		return;

		// should be over or equal
	if (level.sight_client->health == level.sight_client->max_health)
	{
		self->enemy = self->teammaster;
		FoundTarget(self);
		return;
	}

	// don't swap away until we've brought our current heal target upto half health
	if (self->enemy)
	{
		h = self->enemy->health / (float)self->enemy->max_health;
		if (h < 0.5)
			return;

		// don't swap to healing another target if it's health is over 50% and we
		// are healing someone already

		h = level.sight_client->health / (float)level.sight_client->max_health;

		if ((self->enemy->health < self->enemy->max_health) && (h > 0.5))
			return;
	}

	//if (!visible(self, level.sight_client))
	//	return;

	self->enemy = level.sight_client;
	FoundTarget(self);

}*/

mframe_t healbot_frames_walk[] =
{
	ai_walk, 10, NULL,
	ai_walk, 10, healbot_chase,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, healbot_chase,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL,
	ai_walk, 10, NULL
};
mmove_t	healbot_move_walk = { FRAME_idle01, FRAME_idle45, healbot_frames_walk, NULL };

mframe_t healbot_frames_run[] =
{
	ai_run, 24, NULL,
	ai_run, 24, healbot_triage,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, healbot_triage,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, healbot_triage,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, healbot_triage,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL,
	ai_run, 24, NULL
};
mmove_t	healbot_move_run = { FRAME_idle01, FRAME_idle45, healbot_frames_run, NULL };

mframe_t healbot_frames_stand[] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, healbot_chase,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, healbot_chase,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t	healbot_move_stand = { FRAME_idle01, FRAME_idle45, healbot_frames_stand, NULL };

void healbot_idle(edict_t *self)
{
	gi.sound(self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

void healbot_walk(edict_t *self)
{
	self->monsterinfo.currentmove = &healbot_move_walk;
}

void healbot_run(edict_t *self)
{
	self->monsterinfo.currentmove = &healbot_move_run;
}

void healbot_stand(edict_t *self)
{
	self->monsterinfo.currentmove = &healbot_move_stand;
}

void healbot_launch(edict_t *self)
{
	gi.sound(self, CHAN_WEAPON, sound_hook_launch, 1, ATTN_NORM, 0);
}

void healbot_retract(edict_t *self)
{
	gi.sound(self, CHAN_WEAPON, sound_hook_retract, 1, ATTN_NORM, 0);
}

void healbot_heal(edict_t *self)
{
	vec3_t	offset, start, f, r, end, dir;
	vec3_t	angles;
	trace_t	tr;
	float	distance;

	if (!self->enemy)
		return;

	if (!self->enemy->inuse)
		return;

	// see if we can hit our current enemy - proceed with animation if we can

	// do we need to check both of these?

	if (self->velocity[2] < 100)
	{
		self->velocity[2] = 100;
	}

	if ((self->enemy->health <= 0) || (self->enemy->deadflag))
	{
		return;
	}

	if (self->enemy->health >= self->enemy->max_health)
		return;

	AngleVectors(self->s.angles, f, r, NULL);
	VectorSet(offset, 7, 0, 0);
	G_ProjectSource(self->s.origin, offset, f, r, start);

	VectorSubtract(start, self->enemy->s.origin, dir);
	distance = VectorLength(dir);
	if (distance > 768)
		return;

	// check for min/max pitch
	vectoangles(dir, angles);
	if (angles[0] < -180)
		angles[0] += 360;
	if (fabs(angles[0]) > 60)
		return;

	tr = gi.trace(start, NULL, NULL, self->enemy->s.origin, self, MASK_SHOT);
	if (tr.fraction != 1.0 && tr.ent != self->enemy)
		return;

	if (self->s.frame == FRAME_heal05)
	{
		gi.sound(self->enemy, CHAN_AUTO, sound_hook_hit, 1, ATTN_NORM, 0);
	}

	if (self->s.frame == FRAME_heal06)
		gi.sound(self, CHAN_WEAPON, sound_hook_heal, 1, ATTN_NORM, 0);

	// Rroff need to fix the start of the cable a bit
	
	VectorMA(start, 16, f, start);

	VectorCopy(self->enemy->s.origin, end);

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_MEDIC_CABLE_ATTACK);
	gi.WriteShort(self - g_edicts);
	gi.WritePosition(start);
	gi.WritePosition(end);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	//gi.WriteByte(svc_temp_entity);
	//gi.WriteByte(TE_FORCEWALL);
	//gi.WritePosition(end);
	//gi.WritePosition(start);
	//gi.WriteByte(0xD0 + (rand() & 3));
	//gi.multicast(self->s.origin, MULTICAST_PVS);

	//self->enemy->health += 4;

	self->enemy->health += 12; // 12 = approx 40 health per second
	// player machinegun is about 80 dps IIRC

	if (self->enemy->health > self->enemy->max_health)
		self->enemy->health = self->enemy->max_health;
}

mframe_t healbot_frames_attack[] =
{
	ai_charge, 2, NULL,
	ai_charge, 3, NULL,
	ai_charge, 5, NULL,
	ai_charge, 2, healbot_launch,
	ai_charge, 1, healbot_heal,
	ai_charge, 2, healbot_heal,
	ai_charge, 3, healbot_heal,
	ai_charge, 5, healbot_heal,
	ai_charge, 2, healbot_heal,
	ai_charge, 1, healbot_retract,
	ai_charge, 2, NULL,
	ai_move, 3, NULL,
	ai_move, 5, NULL,
	ai_move, 2, NULL,
	ai_move, 1, NULL
};
mmove_t healbot_move_attack = { FRAME_heal01, FRAME_heal15, healbot_frames_attack, healbot_run };

void healbot_pain(edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	gi.sound(self, CHAN_VOICE, gi.soundindex("world/airhiss2.wav"), 1, ATTN_NORM, 0);

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_SPLASH);
	gi.WriteByte(16);
	gi.WritePosition(self->s.origin);
	gi.WriteDir(vec3_origin);
	gi.WriteByte(1);	//sparks
	gi.multicast(self->s.origin, MULTICAST_PVS);

}

void healbot_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	gi.sound(self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	if (self->teammaster)
	{
		if (self->teammaster->inuse)
		{
			if (self->teammaster->svflags & SVF_MONSTER)
				self->teammaster->monsterinfo.controlling = NULL;
		}
	}
	BecomeExplosion1(self);
}

void healbot_search(edict_t *self)
{
	// do anything here?
}

void healbot_attack(edict_t *self)
{
	vec3_t	offset, start, f, r, end, dir;
	vec3_t	angles;
	trace_t	tr;
	float	distance;

	// see if we can hit our current enemy - proceed with animation if we can

	if (!self->enemy->inuse)
		return;

	if ((self->enemy->health <= 0) || (self->enemy->deadflag))
	{
		return;
	}

	if (self->enemy->health >= self->enemy->max_health)
		return;

	AngleVectors(self->s.angles, f, r, NULL);
	VectorSet(offset, 0, 0, -2);
	G_ProjectSource(self->s.origin, offset, f, r, start);

	VectorSubtract(start, self->enemy->s.origin, dir);
	distance = VectorLength(dir);
	if (distance > 768)
		return;

	// check for min/max pitch
	vectoangles(dir, angles);
	if (angles[0] < -180)
		angles[0] += 360;
	if (fabs(angles[0]) > 60)
		return;

	tr = gi.trace(start, NULL, NULL, self->enemy->s.origin, self, MASK_SHOT);
	if (tr.fraction != 1.0 && tr.ent != self->enemy)
		return;

	self->monsterinfo.currentmove = &healbot_move_attack;
}


// This doesn't exist as a map item so add the function to g_local.h
// it will need to fully set itself up on spawn - ent is the entity
// that spawned it - use that as its "owner"

edict_t *monster_healbot(edict_t *ent, vec3_t start)
{
	edict_t		*self;

	sound_idle = gi.soundindex("world/scan1.wav");
	sound_death = gi.soundindex("flyer/flydeth1.wav");

	sound_hook_launch = gi.soundindex("medic/medatck2.wav");
	sound_hook_hit = gi.soundindex("medic/medatck3.wav");
	sound_hook_heal = gi.soundindex("medic/medatck4.wav");
	sound_hook_retract = gi.soundindex("medic/medatck5.wav");

	self = G_Spawn();

	// spawn at the supplied start position - whatever function passed it should have
	// checked it is valid

	VectorCopy(start, self->s.origin);

	self->s.sound = gi.soundindex("floater/fltsrch1.wav");
	self->classname = "monster_healbot";
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/healbot/tris.md2");
	VectorSet(self->mins, -8, -8, -8);
	VectorSet(self->maxs, 8, 8, 16);

	self->health = 200;
	self->gib_health = -80; // not used - explodes on death
	self->mass = 120;

	self->pain = healbot_pain;
	self->die = healbot_die;

	self->teammaster = ent; // so we always know who the owner is

	self->monsterinfo.stand = healbot_stand;
	self->monsterinfo.walk = healbot_walk;
	self->monsterinfo.run = healbot_run;
	self->monsterinfo.search = healbot_search;
	self->monsterinfo.attack = healbot_attack;
	//self->monsterinfo.melee = floater_melee;
	self->monsterinfo.idle = healbot_idle;

	//self->monsterinfo.aiflags |= AI_GOOD_GUY; // Rroff will need tweaking
	self->monsterinfo.aiflags |= AI_MEDIC2;

	self->descname = "Heal bot";

	gi.linkentity(self);

	self->monsterinfo.scale = MODEL_SCALE;

	flymonster_start(self);

	return self;
}