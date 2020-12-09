/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
/*
==============================================================================

parasite

==============================================================================
*/

#include "g_local.h"
#include "m_parasite.h"


static int	sound_pain1;
static int	sound_pain2;
static int	sound_die;
static int	sound_launch;
static int	sound_impact;
static int	sound_suck;
static int	sound_reelin;
static int	sound_sight;
static int	sound_tap;
static int	sound_scratch;
static int	sound_search;
static int	sound_heal1;


void parasite_stand(edict_t *self);
void parasite_start_run(edict_t *self);
void parasite_run(edict_t *self);
void parasite_walk(edict_t *self);
void parasite_start_walk(edict_t *self);
void parasite_end_fidget(edict_t *self);
void parasite_do_fidget(edict_t *self);
void parasite_refidget(edict_t *self);
void parasite_reel_in(edict_t *self);
void parasite_grapple_check(edict_t *self);
void parasite_grapple(edict_t *self);


void parasite_launch(edict_t *self)
{
	gi.sound(self, CHAN_WEAPON, sound_launch, 1, ATTN_NORM, 0);
}

void parasite_sight(edict_t *self, edict_t *other)
{
	gi.sound(self, CHAN_WEAPON, sound_sight, 1, ATTN_NORM, 0);
}

void parasite_tap(edict_t *self)
{
	gi.sound(self, CHAN_WEAPON, sound_tap, 1, ATTN_IDLE, 0);
}

void parasite_scratch(edict_t *self)
{
	gi.sound(self, CHAN_WEAPON, sound_scratch, 1, ATTN_IDLE, 0);
}

void parasite_search(edict_t *self)
{
	gi.sound(self, CHAN_WEAPON, sound_search, 1, ATTN_IDLE, 0);
}


mframe_t parasite_frames_start_fidget[] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t parasite_move_start_fidget = { FRAME_stand18, FRAME_stand21, parasite_frames_start_fidget, parasite_do_fidget };

mframe_t parasite_frames_fidget[] =
{
	ai_stand, 0, parasite_scratch,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, parasite_scratch,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t parasite_move_fidget = { FRAME_stand22, FRAME_stand27, parasite_frames_fidget, parasite_refidget };

mframe_t parasite_frames_end_fidget[] =
{
	ai_stand, 0, parasite_scratch,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL
};
mmove_t parasite_move_end_fidget = { FRAME_stand28, FRAME_stand35, parasite_frames_end_fidget, parasite_stand };

void parasite_end_fidget(edict_t *self)
{
	self->monsterinfo.currentmove = &parasite_move_end_fidget;
}

void parasite_do_fidget(edict_t *self)
{
	self->monsterinfo.currentmove = &parasite_move_fidget;
}

void parasite_refidget(edict_t *self)
{
	if (random() <= 0.8)
		self->monsterinfo.currentmove = &parasite_move_fidget;
	else
		self->monsterinfo.currentmove = &parasite_move_end_fidget;
}

void parasite_idle(edict_t *self)
{
	self->monsterinfo.currentmove = &parasite_move_start_fidget;
}


mframe_t parasite_frames_stand[] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, parasite_tap,
	ai_stand, 0, NULL,
	ai_stand, 0, parasite_tap,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, parasite_tap,
	ai_stand, 0, NULL,
	ai_stand, 0, parasite_tap,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, parasite_tap,
	ai_stand, 0, NULL,
	ai_stand, 0, parasite_tap
};
mmove_t	parasite_move_stand = { FRAME_stand01, FRAME_stand17, parasite_frames_stand, parasite_stand };

void parasite_stand(edict_t *self)
{
	self->monsterinfo.currentmove = &parasite_move_stand;
}


mframe_t parasite_frames_run[] =
{
	ai_run, 30, parasite_grapple_check,
	ai_run, 30, NULL,
	ai_run, 22, NULL,
	ai_run, 19, NULL,
	ai_run, 24, NULL,
	ai_run, 28, NULL,
	ai_run, 25, NULL
};
mmove_t parasite_move_run = { FRAME_run03, FRAME_run09, parasite_frames_run, NULL };

mframe_t parasite_frames_start_run[] =
{
	ai_run, 0,	NULL,
	ai_run, 30, NULL,
};
mmove_t parasite_move_start_run = { FRAME_run01, FRAME_run02, parasite_frames_start_run, parasite_run };

mframe_t parasite_frames_stop_run[] =
{
	ai_run, 20, NULL,
	ai_run, 20,	NULL,
	ai_run, 12, NULL,
	ai_run, 10, NULL,
	ai_run, 0,  NULL,
	ai_run, 0,  NULL
};
mmove_t parasite_move_stop_run = { FRAME_run10, FRAME_run15, parasite_frames_stop_run, NULL };

void parasite_start_run(edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &parasite_move_stand;
	else
		self->monsterinfo.currentmove = &parasite_move_start_run;
}

void parasite_run(edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &parasite_move_stand;
	else
		self->monsterinfo.currentmove = &parasite_move_run;
}


mframe_t parasite_frames_walk[] =
{
	ai_walk, 30, NULL,
	ai_walk, 30, NULL,
	ai_walk, 22, NULL,
	ai_walk, 19, NULL,
	ai_walk, 24, NULL,
	ai_walk, 28, NULL,
	ai_walk, 25, NULL
};
mmove_t parasite_move_walk = { FRAME_run03, FRAME_run09, parasite_frames_walk, parasite_walk };

mframe_t parasite_frames_start_walk[] =
{
	ai_walk, 0,	NULL,
	ai_walk, 30, parasite_walk
};
mmove_t parasite_move_start_walk = { FRAME_run01, FRAME_run02, parasite_frames_start_walk, NULL };

mframe_t parasite_frames_stop_walk[] =
{
	ai_walk, 20, NULL,
	ai_walk, 20,	NULL,
	ai_walk, 12, NULL,
	ai_walk, 10, NULL,
	ai_walk, 0,  NULL,
	ai_walk, 0,  NULL
};
mmove_t parasite_move_stop_walk = { FRAME_run10, FRAME_run15, parasite_frames_stop_walk, NULL };

void parasite_start_walk(edict_t *self)
{
	self->monsterinfo.currentmove = &parasite_move_start_walk;
}

void parasite_walk(edict_t *self)
{
	self->monsterinfo.currentmove = &parasite_move_walk;
}


mframe_t parasite_frames_pain1[] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 0,	NULL,
	ai_move, 6,	NULL,
	ai_move, 16, NULL,
	ai_move, -6, NULL,
	ai_move, -7, NULL,
	ai_move, 0, NULL
};
mmove_t parasite_move_pain1 = { FRAME_pain101, FRAME_pain111, parasite_frames_pain1, parasite_start_run };

void parasite_pain(edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < (self->max_health / 2))
		self->s.skinnum = 1;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	if ((skill->value == 3) && (kick < 200))
		return;		// no pain anims in nightmare

	if (random() < 0.5)
		gi.sound(self, CHAN_VOICE, sound_pain1, 1, ATTN_NORM, 0);
	else
		gi.sound(self, CHAN_VOICE, sound_pain2, 1, ATTN_NORM, 0);

	self->monsterinfo.currentmove = &parasite_move_pain1;
}

mframe_t parasite_frames_grapple2[] =
{
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 15,	NULL,
	ai_charge, 0,	parasite_grapple,
	ai_charge, 0,	parasite_grapple,
	ai_charge, 0,	parasite_grapple,
	ai_charge, 0,	parasite_grapple,
	ai_charge, 0,	parasite_grapple,
	ai_charge, 0,	parasite_grapple,
	ai_charge, 0,	parasite_grapple,
	ai_charge, 0,	parasite_grapple,
	ai_charge, 0,	parasite_grapple,
	ai_charge, 0,	parasite_grapple,
	ai_charge, 0,	parasite_reel_in,				// let go
	ai_charge, -2,	NULL,
	ai_charge, -2,	NULL,
	ai_charge, -3,	NULL,
	ai_charge, 0,	NULL
};
mmove_t parasite_move_grapple2 = { FRAME_drain01, FRAME_drain18, parasite_frames_grapple2, parasite_start_run };

// if we are running and have an enemy
// and they are above us
// see if we can grapple at a point above the player

void parasite_grapple_check(edict_t *self)
{
	trace_t		tr;
	vec3_t      forward, dir;
	vec3_t      end;
	float		d;

	if (!(self->enemy))
		return;

	// randomly return here so we don't constantly do this while running

	VectorSubtract(self->enemy->s.origin, self->s.origin, dir);

	d = VectorLength(dir);

	if (d > 768 || d < 64)
		return;

	if (self->enemy->s.origin[2] < (self->s.origin[2] + 64))
		return;

	// need to check we aren't directly under player, etc.
	// which probably won't end well

	VectorCopy(self->enemy->s.origin, end);

	end[2] += 300;

	tr = gi.trace(self->s.origin, NULL, NULL, end, self, MASK_SHOT);

	// bit crude
	if (tr.fraction < 0.7 || tr.fraction == 1.0)
		return;

	self->following_mode = qtrue;

	VectorCopy(tr.endpos, self->pos1);

	gi.positioned_sound(self->pos1, self, CHAN_AUTO, sound_search, 1, ATTN_NORM, 0.1);

	parasite_launch(self);

	self->monsterinfo.currentmove = &parasite_move_grapple2;
}

// Rroff
// If too far from enemy try and hit a spot above enemy
// and use to lauch at

void parasite_grapple_start(edict_t *self)
{
	trace_t     tr;
	vec3_t      forward, dir;
	vec3_t      end;

	float		delta;

	if (!(self->enemy))
		return;

	// re-use following_mode for when grapple has connected
	self->following_mode = qfalse;

	VectorCopy(self->s.angles, dir);
	dir[PITCH] = -60 + (rand() % 20);
	dir[YAW] -= (-15 + (rand() % 30));
	AngleVectors(dir, forward, NULL, NULL);

	VectorMA(self->s.origin, 1024, forward, end);

	tr = gi.trace(self->s.origin, NULL, NULL, end, self, MASK_SHOT);

	// we should make sure we hit static world geometry

/*	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_PARASITE_ATTACK);
	gi.WriteShort(self - g_edicts);
	gi.WritePosition(self->s.origin);
	gi.WritePosition(tr.endpos);
	gi.multicast(self->s.origin, MULTICAST_PVS);*/

	if (tr.fraction == 1.0)
		return;

	// Don't grapple off the sky!
	// (Currently untested)!
	if ((tr.surface) && (tr.surface->flags & SURF_SKY))
		return;

	self->following_mode = qtrue;

	VectorCopy(tr.endpos, self->pos1);

	//gi.positioned_sound(self->pos1, self, CHAN_WEAPON, sound_launch, 1, ATTN_NORM, 0.1);
	gi.positioned_sound(self->pos1, self, CHAN_AUTO, sound_search, 1, ATTN_NORM, 0.1);

	//parasite_search(self);
	parasite_launch(self);
}

void parasite_grapple(edict_t *self)
{
	trace_t     tr;
	vec3_t		forward, right, offset, start;
	vec3_t		dir;

	if (!(self->following_mode))
		return;

	tr = gi.trace(self->s.origin, NULL, NULL, self->pos1, self, MASK_SHOT);

	VectorSubtract(self->pos1, self->s.origin, dir);

	if ((tr.fraction != 1.0) || (VectorLength(dir) < 64))
	{
		self->following_mode = qfalse;
		parasite_reel_in(self);
		return;
	}

	AngleVectors(self->s.angles, forward, right, NULL);
	VectorSet(offset, 24, 0, 6);
	G_ProjectSource(self->s.origin, offset, forward, right, start);

	//VectorSubtract(self->pos1, self->s.origin, dir);
	VectorNormalize(dir);
	VectorScale(dir, 600, self->velocity);

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_PARASITE_ATTACK);
	gi.WriteShort(self - g_edicts);
	gi.WritePosition(start);
	gi.WritePosition(self->pos1);
	gi.multicast(self->s.origin, MULTICAST_PVS);
}

mframe_t parasite_frames_grapple[] =
{
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,
	ai_charge, 15,	parasite_grapple_start,
	ai_charge, 0,	parasite_grapple,
	ai_charge, 0,	parasite_grapple,
	ai_charge, 0,	parasite_grapple,
	ai_charge, 0,	parasite_grapple,
	ai_charge, 0,	parasite_grapple,
	ai_charge, 0,	parasite_grapple,
	ai_charge, 0,	parasite_grapple,
	ai_charge, 0,	parasite_grapple,
	ai_charge, 0,	parasite_grapple,
	ai_charge, 0,	parasite_grapple,
	ai_charge, 0,	parasite_reel_in,				// let go
	ai_charge, -2,	NULL,
	ai_charge, -2,	NULL,
	ai_charge, -3,	NULL,
	ai_charge, 0,	NULL
};
mmove_t parasite_move_grapple = { FRAME_drain01, FRAME_drain18, parasite_frames_grapple, parasite_start_run };


static qboolean parasite_drain_attack_ok(vec3_t start, vec3_t end)
{
	vec3_t	dir, angles;

	// check for max distance
	VectorSubtract(start, end, dir);
	//if (VectorLength(dir) > 256)
	if (VectorLength(dir) > 384) // don't really want it too long but need a reasonable distance
		return qfalse;

	// check for min/max pitch
	vectoangles(dir, angles);
	if (angles[0] < -180)
		angles[0] += 360;
	if (fabs(angles[0]) > 30)
		return qfalse;

	return qtrue;
}

void parasite_drain_attack(edict_t *self)
{
	vec3_t	offset, start, f, r, end, dir;
	trace_t	tr;
	int damage;

	AngleVectors(self->s.angles, f, r, NULL);
	VectorSet(offset, 24, 0, 6);
	G_ProjectSource(self->s.origin, offset, f, r, start);

	VectorCopy(self->enemy->s.origin, end);
	if (!parasite_drain_attack_ok(start, end))
	{
		end[2] = self->enemy->s.origin[2] + self->enemy->maxs[2] - 8;
		if (!parasite_drain_attack_ok(start, end))
		{
			end[2] = self->enemy->s.origin[2] + self->enemy->mins[2] + 8;
			if (!parasite_drain_attack_ok(start, end))
			{
				return;
			}
		}
	}
	VectorCopy(self->enemy->s.origin, end);

	tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);
	if (tr.ent != self->enemy)
		return;

	if (self->s.frame == FRAME_drain03)
	{
		damage = 5;
		gi.sound(self->enemy, CHAN_AUTO, sound_impact, 1, ATTN_NORM, 0);
	}
	else
	{
		if (self->s.frame == FRAME_drain04) {
			gi.sound(self, CHAN_WEAPON, sound_suck, 1, ATTN_NORM, 0);
			//gi.sound(self, CHAN_VOICE, sound_heal1, 1, ATTN_NORM, 0);
		}
		damage = 2;

		// Rroff - grab some health from the target we are draining
		if (self->health <= self->max_health) // hard set to 250 in original mod was based on max_health
		{
			self->health += 5;
			if (self->health > self->max_health)
			{
				self->health = self->max_health;
			}

			/*if (self->s.frame > FRAME_drain03 && !(self->monsterinfo.aiflags & AI_RESURRECTING)) {
				self->monsterinfo.aiflags |= AI_RESURRECTING; // Rroff - maybe not the best idea to repurpose this but nothing conflicts at the moment
			} else {
				self->monsterinfo.aiflags &= ~AI_RESURRECTING;
			}*/

			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_FORCEWALL);
			gi.WritePosition(start);
			gi.WritePosition(end);
			gi.WriteByte(0xE7 + (rand() & 3));
			gi.multicast(self->s.origin, MULTICAST_PVS);

		}
	}

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_PARASITE_ATTACK);
	gi.WriteShort(self - g_edicts);
	gi.WritePosition(start);
	gi.WritePosition(end);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	VectorSubtract(start, end, dir);
	//T_Damage (self->enemy, self, self, dir, self->enemy->s.origin, vec3_origin, damage, 0, DAMAGE_NO_KNOCKBACK, MOD_UNKNOWN);

	// Rroff add some pull from the drain attack
	T_Damage(self->enemy, self, self, dir, self->enemy->s.origin, vec3_origin, damage, 40, 0, MOD_TONGUE);
}

void parasite_drain_reel(edict_t *self)
{
	vec3_t	offset, start, f, r, end, dir;
	trace_t	tr;
	int damage;
	vec3_t	v;
	float	len;

	AngleVectors(self->s.angles, f, r, NULL);
	VectorSet(offset, 24, 0, 6);
	G_ProjectSource(self->s.origin, offset, f, r, start);

	VectorCopy(self->enemy->s.origin, end);

	tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);

	if (tr.ent != self->enemy)
	{
		// will cause the tongue thing to jump forward again if in subsequent
		// frames the parasite can hit its enemy.

		if (self->s.frame > 16)
		{
			VectorSubtract(start, end, v);
			len = VectorLength(v) * (1 - (((float)self->s.frame + 1) / 20.0));

			// doesn't quite match the original angle but probably ok as it would move a bit
			// as it lashed back
			VectorMA(start, len, f, end);
		}

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_PARASITE_ATTACK);
		gi.WriteShort(self - g_edicts);
		gi.WritePosition(start);
		gi.WritePosition(end);
		gi.multicast(self->s.origin, MULTICAST_PVS);

		return;
	}

	// Rroff move end back as a percentage of its length based on
	// self->s.frame upto 20 at which point it vanishes

	if (self->s.frame > 16)
	{
		VectorSubtract(start, end, v);
		len = VectorLength(v) * (1 - (((float)self->s.frame - 16) / 4.0));

		// doesn't quite match the original angle but probably ok as it would move a bit
		// as it lashed back
		VectorMA(start, len, f, end);
	}

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_PARASITE_ATTACK);
	gi.WriteShort(self - g_edicts);
	gi.WritePosition(start);
	gi.WritePosition(end);
	gi.multicast(self->s.origin, MULTICAST_PVS);

}

void parasite_drain_reel2(edict_t *self)
{
	vec3_t	offset, start, f, r, end, dir;
	trace_t	tr;
	int damage;
	vec3_t	v;
	float	len;

	if (self->delay <= 0)
		return;

	AngleVectors(self->s.angles, f, r, NULL);
	VectorSet(offset, 24, 0, 6);
	G_ProjectSource(self->s.origin, offset, f, r, start);

	VectorCopy(self->enemy->s.origin, end);

	len = self->delay * (1 - (((float)self->s.frame - 51) / 5.0));

	if (len <= 0)
		return;

	// doesn't quite match the original angle but probably ok as it would move a bit
	// as it lashed back
	VectorMA(start, len, f, end);

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_PARASITE_ATTACK);
	gi.WriteShort(self - g_edicts);
	gi.WritePosition(start);
	gi.WritePosition(end);
	gi.multicast(self->s.origin, MULTICAST_PVS);

}

mframe_t parasite_frames_drain[] =
{
	ai_charge, 0,	parasite_launch,
	ai_charge, 0,	NULL,
	ai_charge, 15,	parasite_drain_attack,			// Target hits
	ai_charge, 0,	parasite_drain_attack,			// drain
	ai_charge, 0,	parasite_drain_attack,			// drain
	ai_charge, 0,	parasite_drain_attack,			// drain
	ai_charge, 0,	parasite_drain_attack,			// drain
	ai_charge, -2,  parasite_drain_attack,			// drain
	ai_charge, -2,	parasite_drain_attack,			// drain
	ai_charge, -3,	parasite_drain_attack,			// drain
	ai_charge, -2,	parasite_drain_attack,			// drain
	ai_charge, 0,	parasite_drain_attack,			// drain
	ai_charge, -1,  parasite_drain_attack,			// drain
	ai_charge, 0,	parasite_reel_in,				// let go
	ai_charge, -2,	NULL,
	ai_charge, -2,	NULL,
	ai_charge, -3,	NULL,
	ai_charge, 0,	NULL
};
mmove_t parasite_move_drain = { FRAME_drain01, FRAME_drain18, parasite_frames_drain, parasite_start_run };


mframe_t parasite_frames_break[] =
{
	ai_charge, 0, parasite_drain_reel,
	ai_charge, -3, parasite_drain_reel,
	ai_charge, 1, parasite_drain_reel,
	ai_charge, 2, parasite_drain_reel,
	ai_charge, -3, parasite_drain_reel,
	ai_charge, 1, parasite_drain_reel,
	ai_charge, 1, parasite_drain_reel,
	ai_charge, 3, parasite_drain_reel,
	ai_charge, 0, parasite_drain_reel,
	ai_charge, -18, parasite_drain_reel,
	ai_charge, 3, parasite_drain_reel,
	ai_charge, 9, parasite_drain_reel,
	ai_charge, 6, parasite_drain_reel,
	ai_charge, 0, parasite_drain_reel,
	ai_charge, -18, parasite_drain_reel,
	ai_charge, 0, parasite_drain_reel,
	ai_charge, 8, parasite_drain_reel,
	ai_charge, 9, parasite_drain_reel,
	ai_charge, 0, parasite_drain_reel,
	ai_charge, -18, parasite_drain_reel,
	ai_charge, 0,	NULL,
	ai_charge, 0,	NULL,		// airborne
	ai_charge, 0,	NULL,		// airborne
	ai_charge, 0,	NULL,		// slides
	ai_charge, 0,	NULL,		// slides
	ai_charge, 0,	NULL,		// slides
	ai_charge, 0,	NULL,		// slides
	ai_charge, 4,	NULL,
	ai_charge, 11,	NULL,
	ai_charge, -2,	NULL,
	ai_charge, -5,	NULL,
	ai_charge, 1,	NULL
};
mmove_t parasite_move_break = { FRAME_break01, FRAME_break32, parasite_frames_break, parasite_start_run };

void parasite_reel_in(edict_t *self)
{
	vec3_t	offset, start, f, r, end, dir;
	vec3_t	v;


	gi.sound(self, CHAN_WEAPON, sound_reelin, 1, ATTN_NORM, 0);

	//self->monsterinfo.aiflags &= ~AI_RESURRECTING;

	/*
	self->delay = 0;

	if (self->enemy)
	{
		AngleVectors(self->s.angles, f, r, NULL);
		VectorSet(offset, 24, 0, 6);
		G_ProjectSource(self->s.origin, offset, f, r, start);

		VectorCopy(self->enemy->s.origin, end);
		if (!parasite_drain_attack_ok(start, end))
		{
			end[2] = self->enemy->s.origin[2] + self->enemy->maxs[2] - 8;
			if (!parasite_drain_attack_ok(start, end))
			{
				end[2] = self->enemy->s.origin[2] + self->enemy->mins[2] + 8;
				if (!parasite_drain_attack_ok(start, end))
					return;
			}
		}

		VectorSubtract(self->s.origin, self->enemy->s.origin, v);
		self->delay = VectorLength(v);

		parasite_drain_reel2(self);
	}*/

	//self->monsterinfo.currentmove = &parasite_move_break;
}

/*
===
Break Stuff Ends
===
*/

void parasite_attack(edict_t *self)
{
	vec3_t		dir;
	//	if (random() <= 0.2)
	//		self->monsterinfo.currentmove = &parasite_move_break;
	//	else

	if (self->enemy)
	{
		//VectorSubtract(self->enemy->s.origin, self->s.origin, dir);
		//if (VectorLength(dir) > 384)
		if (!(parasite_drain_attack_ok(self->s.origin, self->enemy->s.origin)))
		{
			self->monsterinfo.currentmove = &parasite_move_grapple;
			return;
		}

		if (!(visible(self, self->enemy)))
		{
			self->monsterinfo.currentmove = &parasite_move_grapple;
			return;
		}
	}
	
	self->monsterinfo.currentmove = &parasite_move_drain;
	//self->monsterinfo.currentmove = &parasite_move_grapple;
}



/*
===
Death Stuff Starts
===
*/

void parasite_dead(edict_t *self)
{
	VectorSet(self->mins, -16, -16, -24);
	VectorSet(self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity(self);
}

mframe_t parasite_frames_death[] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t parasite_move_death = { FRAME_death101, FRAME_death107, parasite_frames_death, parasite_dead };

void parasite_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	// check for gib
	if (self->health <= self->gib_health)
	{
		gi.sound(self, CHAN_VOICE, gi.soundindex("misc/udeath.wav"), 1, ATTN_NORM, 0);

		if ((g_sillygore->value) && (g_sillygore->integer == 2))
		{
			for (n = 0; n < game.gibcount; n++)
			{
				if (n < 3)
				{
					ThrowGib(self, "models/objects/gibs/arm/tris.md2", damage, GIB_ORGANIC);
				}
				else {
					if (random() < 0.3)
					{
						ThrowGib(self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
					}
					else if (random() < 0.3)
					{
						ThrowGib(self, "models/objects/gibs/bone2/tris.md2", damage, GIB_ORGANIC);
					}
					else {
						ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
					}
				}
			}
		}
		else {
			for (n = 0; n < 2; n++)
				ThrowGib(self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
			for (n = 0; n < 4; n++)
				ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		}
		ThrowHead(self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;

		// Rroff bit horrid doing it here but only reliable way
		// do we need to check attacker is valid?
		if (attacker->client)
		{
			if ((attacker->client->pers.mods & MU_TOMBSTONE))
			{
				Drop_Item(self, FindItem("Armor Shard"));
			}
		}

		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	// regular death
	gi.sound(self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.currentmove = &parasite_move_death;
	//self->monsterinfo.aiflags &= ~AI_RESURRECTING;
}

/*
===
End Death Stuff
===
*/

/*QUAKED monster_parasite (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_monster_parasite(edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict(self);
		return;
	}

	sound_pain1 = gi.soundindex("parasite/parpain1.wav");
	sound_pain2 = gi.soundindex("parasite/parpain2.wav");
	sound_die = gi.soundindex("parasite/pardeth1.wav");
	sound_launch = gi.soundindex("parasite/paratck1.wav");
	sound_impact = gi.soundindex("parasite/paratck2.wav");
	sound_suck = gi.soundindex("parasite/paratck3.wav");
	sound_reelin = gi.soundindex("parasite/paratck4.wav");
	sound_sight = gi.soundindex("parasite/parsght1.wav");
	sound_tap = gi.soundindex("parasite/paridle1.wav");
	sound_scratch = gi.soundindex("parasite/paridle2.wav");
	sound_search = gi.soundindex("parasite/parsrch1.wav");
	sound_heal1 = gi.soundindex("items/s_health.wav");

	self->s.modelindex = gi.modelindex("models/monsters/parasite/tris.md2");
	VectorSet(self->mins, -16, -16, -24);
	VectorSet(self->maxs, 16, 16, 24);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	self->health = 175;
	self->max_health = 250;
	self->gib_health = -50;
	self->mass = 250;

	self->pain = parasite_pain;
	self->die = parasite_die;

	self->monsterinfo.stand = parasite_stand;
	self->monsterinfo.walk = parasite_start_walk;
	self->monsterinfo.run = parasite_start_run;
	self->monsterinfo.attack = parasite_attack;
	self->monsterinfo.sight = parasite_sight;
	self->monsterinfo.idle = parasite_idle;

	self->descname = "Parasite";

	gi.linkentity(self);

	self->monsterinfo.currentmove = &parasite_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start(self);
}
