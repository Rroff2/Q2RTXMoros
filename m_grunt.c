#include "g_local.h"
#include "m_player.h"

void grunt_turn(edict_t *self);

/* TODO:

Add additional death anims
static sound defs for precache etc?
fix death animation positioning
gender pain sounds
pain responses - gestures and anger if a client
reimplement walk in run function
make sure we aren't accidentally referencing actor anywhere
ability to disable grunt footsteps?
add jump - use hold frame until on the ground again?
client use - add routine to use nearby actor if nothing else to use
gender jump sounds
add animations when using a grunt including waving over and away, etc.
fix muzzle flash offset
make monsters react to grunt
*/

// **************** NEED TO ADD FUNCTION POINTERS TO G_PTRS.C *******************

// Rroff - friendly grunt that the player can interact with to do things like follow them
//			will attack monsters and be attacked by monsters

// need to add this to local and reference as a spawn function in relevant place
// also add spawnflags below for different skins, weapons, ai styles, gender model etc.

void grunt_step(edict_t *self)
{
	int		r;

	if (self->groundentity)
	{
		r = 1 + (rand() % 4);
		gi.sound(self, CHAN_VOICE, gi.soundindex(va("player/step%i.wav",r)), 0.75, ATTN_STATIC, 0);
	}
}

edict_t *gruntFindMonster(edict_t *self)
{
	edict_t	*ent = NULL;
	edict_t	*best = NULL;
	float	dist, near = 99999;
	vec3_t	v;

	while ((ent = findradius(ent, self->s.origin, 1024)) != NULL)
	{
		if (ent == self)
			continue;
		if (!(ent->svflags & SVF_MONSTER))
			continue;
		if (ent->monsterinfo.aiflags & AI_GOOD_GUY)
			continue;
		if (ent->owner) // ???
			continue;
		if (ent->health <= 0)
			continue;
		if (!(visible(self, ent)))
			continue;

		// any monster we find now that isn't attacking
		// will attack back - this will have a delayed response
		// from monsters that come into the fight area late
		if (!ent->enemy)
		{
			ent->enemy = self;
			FoundTarget(ent);
		}

		VectorSubtract(self->s.origin, ent->s.origin, v);
		dist = VectorLength(v);
		if (dist < near)
		{
			near = dist;
			best = ent;
		}
	}

	return best;
}

void grunt_search(edict_t *self)
{
	edict_t		*ent = NULL;

	// do we need this?
	if (self->movetarget && !(self->movetarget->inuse))
	{
		self->monsterinfo.grunt_follow = 0;
		self->movetarget = self->goalentity = NULL;
		self->monsterinfo.stand(self);
		self->monsterinfo.pausetime = level.time + 100000000;
	}

	if (self->enemy && (self->enemy->health <= 0))
	{
		self->enemy = NULL;
		self->monsterinfo.stand(self);
		self->monsterinfo.pausetime = level.time + 100000000;
	}

	if (self->enemy)
	{
		if ((self->monsterinfo.search_time) && (level.time > self->monsterinfo.search_time)) {
			self->enemy = NULL;
			self->monsterinfo.stand(self);
			self->monsterinfo.pausetime = level.time + 100000000;
			return;
		}
	}

	if ((self->monsterinfo.grunt_follow == 1) && (!self->enemy) && (self->movetarget))
	{
		if (SV_HSLCloseEnough(self, self->movetarget, 100) || self->movetarget->health <=0)
		{
			self->monsterinfo.stand(self);
			//self->monsterinfo.pausetime = level.time + 3;
		}
		else
		{
			self->monsterinfo.walk(self);
		}
	}

	// clear out movetarget and reset goalentity

	if (!(self->enemy))
	{
		ent = gruntFindMonster(self);
		if (ent)
		{
			self->monsterinfo.grunt_follow = 0;
			self->movetarget = NULL;
			self->enemy = ent;
			FoundTarget(self);

			return;
		}
	}

	if (!(self->enemy) && !(self->movetarget) && (random() < 0.5) && (level.time > self->delay))
	{
		self->delay = level.time + 2;

		while (self->ideal_yaw > 360)
			self->ideal_yaw -= 360;
		while (self->ideal_yaw < 0)
			self->ideal_yaw += 360;

		self->ideal_yaw += 180 + (-45 + (rand() % 90));
		if (self->ideal_yaw > 360)
			self->ideal_yaw -= 360;

		grunt_turn(self);
	}
}

mframe_t grunt_frames_stand[] = {
	{ ai_stand, 0, grunt_search },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },

	{ ai_stand, 0, grunt_search },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },

	{ ai_stand, 0, grunt_search },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },

	{ ai_stand, 0, grunt_search },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL },
	{ ai_stand, 0, NULL }
};
mmove_t grunt_move_stand = { FRAME_stand01, FRAME_stand40, grunt_frames_stand, NULL };

void grunt_stand(edict_t *self)
{
	self->monsterinfo.currentmove = &grunt_move_stand;

	// randomize on startup
	if (level.time < 1.0)
		self->s.frame = self->monsterinfo.currentmove->firstframe + (rand() % (self->monsterinfo.currentmove->lastframe - self->monsterinfo.currentmove->firstframe + 1));
}

mframe_t grunt_frames_turn[] =
{
	ai_turn, 0,  NULL,
	ai_turn, 0,  grunt_step,
	ai_turn, 0,  grunt_search,
	ai_turn, 0,  NULL,
	ai_turn, 0,  grunt_step,
	ai_turn, 0,  grunt_stand
};
mmove_t grunt_move_turn = { FRAME_run1, FRAME_run6, grunt_frames_turn, NULL };


void grunt_turn(edict_t *self)
{
	self->monsterinfo.currentmove = &grunt_move_turn;
}

mframe_t grunt_frames_walk[] =
{
	ai_walk, 18,  NULL,
	ai_walk, 18,  grunt_step,
	ai_walk, 18,  grunt_search,
	ai_walk, 18,  NULL,
	ai_walk, 18,  grunt_step,
	ai_walk, 18,  grunt_search
};
mmove_t grunt_move_walk = { FRAME_run1, FRAME_run6, grunt_frames_walk, NULL };

void grunt_walk(edict_t *self)
{
	self->monsterinfo.currentmove = &grunt_move_walk;
}

mframe_t grunt_frames_run[] =
{
	ai_run, 28,  NULL,
	ai_run, 30, grunt_step,
	ai_run, 33, grunt_search,
	ai_run, 30,  NULL,
	ai_run, 30, grunt_step,
	ai_run, 33, grunt_search

};
mmove_t grunt_move_run = { FRAME_run1, FRAME_run6, grunt_frames_run, NULL };

void grunt_run(edict_t *self)
{

	self->s.sound = 0;
	//if ((level.time < self->pain_debounce_time) && (!self->enemy)) {
	if (!(self->enemy)) {
		if (self->movetarget && self->movetarget->inuse)
		{
			grunt_walk(self); // currently not implemented - probably reuse target_actor, etc.
		}
		else
		{
			grunt_stand(self);
		}

		return;
	}

	if (self->monsterinfo.aiflags & AI_STAND_GROUND) {
		grunt_stand(self);
		return;
	}

	self->monsterinfo.currentmove = &grunt_move_run;
}

void grunt_firemg(edict_t *self)
{
	vec3_t  start, target, offset;
	vec3_t  forward, right;

	// might need to move forwards (18.4) for rocket launcher, etc.
	VectorSet(offset, 7, 9, 7);
	//VectorSet(offset, 0, 7.4, 9.6);

	AngleVectors(self->s.angles, forward, right, NULL);
	G_ProjectSource(self->s.origin, offset, forward, right, start);

	if (self->enemy) {
		if (self->enemy->health > 0) {
			VectorMA(self->enemy->s.origin, -0.2, self->enemy->velocity, target);
			target[2] += self->enemy->viewheight;
		} // yeah make sure they are good and dead!
		else {
			VectorCopy(self->enemy->absmin, target);
			target[2] += (self->enemy->size[2] / 2);
		}
		VectorSubtract(target, start, forward);
		VectorNormalize(forward);
	}
	else {
		// do we need to do this again?
		AngleVectors(self->s.angles, forward, NULL, NULL);
	}

	//monster_fire_bullet(self, start, forward, 3, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MZ2_ACTOR_MACHINEGUN_1);

	fire_bullet(self, start, forward, 3, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_GRUNT);

	// get the machinegun firing sound and dlight
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(self - g_edicts);
	gi.WriteByte(MZ_MACHINEGUN);
	gi.multicast(start, MULTICAST_PVS);

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_SHOTGUN);
	gi.WritePosition(start);
	gi.WriteDir(vec3_origin);
	gi.multicast(start, MULTICAST_PVS);

	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
}

void grunt_firecg(edict_t *self)
{
	vec3_t  start, target, offset;
	vec3_t  forward, right;
	int		i;

	VectorSet(offset, 18.4, 9, 8);

	AngleVectors(self->s.angles, forward, right, NULL);
	G_ProjectSource(self->s.origin, offset, forward, right, start);

	if (self->enemy) {
		if (self->enemy->health > 0) {
			VectorMA(self->enemy->s.origin, -0.2, self->enemy->velocity, target);
			target[2] += self->enemy->viewheight;
		} // yeah make sure they are good and dead!
		else {
			VectorCopy(self->enemy->absmin, target);
			target[2] += (self->enemy->size[2] / 2);
		}
		VectorSubtract(target, start, forward);
		VectorNormalize(forward);
	}
	else {
		AngleVectors(self->s.angles, forward, NULL, NULL);
	}

	//monster_fire_bullet(self, start, forward, 3, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MZ2_ACTOR_MACHINEGUN_1);

	for (i = 0; i < 3; i++)
	{
		fire_bullet(self, start, forward, 3, 4, 500, 700, MOD_GRUNT);
	}

	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(self - g_edicts);
	gi.WriteByte(MZ_CHAINGUN3);
	gi.multicast(start, MULTICAST_PVS);

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_SHOTGUN);
	gi.WritePosition(start);
	gi.WriteDir(vec3_origin);
	gi.multicast(start, MULTICAST_PVS);

	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;
}

void grunt_fire(edict_t *self)
{
	if (!(self->monsterinfo.aiflags & AI_HOLD_FRAME))
		self->monsterinfo.grunt_firing_time = level.time;

	if (self->monsterinfo.grunt_weapon == WEAP_MACHINEGUN)
	{
		grunt_firemg(self);
		return;
	}

	if (self->monsterinfo.grunt_weapon == WEAP_CHAINGUN)
	{
		if (!(self->monsterinfo.aiflags & AI_HOLD_FRAME))
			gi.sound(self, CHAN_AUTO, gi.soundindex("weapons/chngnu1a.wav"), 1, ATTN_NORM, 0);
		else if ((self->s.sound == 0) && (level.time > self->monsterinfo.grunt_firing_time + 1))
			self->s.sound = gi.soundindex("weapons/chngnl1a.wav");

		grunt_firecg(self);
		return;
	}

}

// precache sound?
void grunt_firedown(edict_t *self)
{
	self->s.sound = 0;

	if (self->monsterinfo.grunt_weapon == WEAP_CHAINGUN)
		gi.sound(self, CHAN_AUTO, gi.soundindex("weapons/chngnd1a.wav"), 1, ATTN_NORM, 0);
}

// shorten animation for weapons that don't "pump"?

mframe_t grunt_frames_attack[] =
{
	ai_charge, 0,  grunt_fire, // fire frame
	ai_charge, 0,  grunt_firedown,
	ai_charge, 0,   NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,   NULL,
	ai_charge, 0,  NULL,
	ai_charge, 0,   NULL,
	ai_charge, 0,   NULL
};
mmove_t grunt_move_attack = { FRAME_attack1, FRAME_attack8, grunt_frames_attack, grunt_run };

void grunt_attack(edict_t *self)
{
	int		n;

	self->monsterinfo.currentmove = &grunt_move_attack;
	n = (rand() & 15) + 3 + 7;
	self->monsterinfo.pausetime = level.time + n * FRAMETIME;
}

void grunt_jump_up(edict_t *self)
{
	vec3_t	forward;

	if (!(self->groundentity))
		return;

	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorScale(forward, 200, self->velocity);
	self->velocity[2] = 400;

	gi.sound(self, CHAN_VOICE, gi.soundindex("player/male/jump1.wav"), 1, ATTN_NORM, 0);
}

mframe_t grunt_frames_jump[] =
{
	ai_move, 5, grunt_jump_up,
	ai_move, 4,  NULL,
	ai_move, 0,  NULL
};
mmove_t grunt_move_jump = { FRAME_jump1, FRAME_jump3, grunt_frames_jump, grunt_run };

void grunt_jump(edict_t *self)
{
	// check a jump if valid otherwise revert to run
	self->monsterinfo.currentmove = &grunt_move_jump;
}

mframe_t grunt_frames_flipoff[] =
{
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL

};
mmove_t grunt_move_flipoff = { FRAME_flip01, FRAME_flip12, grunt_frames_flipoff, grunt_run };

mframe_t grunt_frames_taunt[] =
{
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL
};
mmove_t grunt_move_taunt = { FRAME_taunt01, FRAME_taunt17, grunt_frames_taunt, grunt_run };


mframe_t grunt_frames_pain1[] =
{
	ai_move, -5, NULL,
	ai_move, 4,  NULL,
	ai_move, 1,  NULL,
	ai_move, 0,  NULL
};
mmove_t grunt_move_pain1 = { FRAME_pain101, FRAME_pain104, grunt_frames_pain1, grunt_run };

mframe_t grunt_frames_pain2[] =
{
	ai_move, -4, NULL,
	ai_move, 4,  NULL,
	ai_move, 0,  NULL,
	ai_move, -4, NULL
};
mmove_t grunt_move_pain2 = { FRAME_pain201, FRAME_pain204, grunt_frames_pain2, grunt_run };

mframe_t grunt_frames_pain3[] =
{
	ai_move, -4, NULL,
	ai_move, 4,  NULL,
	ai_move, 0,  NULL,
	ai_move, -4, NULL

};
mmove_t grunt_move_pain3 = { FRAME_pain301, FRAME_pain304, grunt_frames_pain3, grunt_run };


void grunt_pain(edict_t *self, edict_t *other, float kick, int damage)
{
	int     n, r, l;

	// currently no pain skins

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;

	r = 1 + (rand() & 1);

	if (self->health < 25)
		l = 25;
	else if (self->health < 50)
		l = 50;
	else if (self->health < 75)
		l = 75;
	else
		l = 100;
	gi.sound(self, CHAN_VOICE, gi.soundindex(va("player/male/pain%i_%i.wav", l, r)), 1, ATTN_NORM, 0);

	//gi.sound(self, CHAN_VOICE, gi.soundindex("player/male/pain25_1.wav"), 1, ATTN_NORM, 0);

	if ((other->client) && (random() < 0.4))
	{
		vec3_t	v;
		char	*name;

		VectorSubtract(other->s.origin, self->s.origin, v);
		self->ideal_yaw = vectoyaw(v);

		if (random() < 0.5)
			self->monsterinfo.currentmove = &grunt_move_flipoff;
		else
			self->monsterinfo.currentmove = &grunt_move_taunt;

		return;
	}

	n = rand() % 3;
	if (n == 0)
		self->monsterinfo.currentmove = &grunt_move_pain1;
	else if (n == 1)
		self->monsterinfo.currentmove = &grunt_move_pain2;
	else
		self->monsterinfo.currentmove = &grunt_move_pain3;
}

void grunt_dead(edict_t *self)
{
	VectorSet(self->mins, -16, -16, -24);
	VectorSet(self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;

	self->svflags |= SVF_DEADMONSTER;

	self->nextthink = 0;
	gi.linkentity(self);
}

mframe_t grunt_frames_death1[] = {
	{ ai_move, 0,   NULL },
	{ ai_move, 0,   NULL },
	{ ai_move, -13, NULL },
	{ ai_move, 14,  NULL },
	{ ai_move, 3,   NULL },
	{ ai_move, -2,  NULL }
};
mmove_t grunt_move_death1 = { FRAME_death101, FRAME_death106, grunt_frames_death1, grunt_dead };

// add additional death anims later

void grunt_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int     n;

	// check for gib
	if (self->health <= -80) {
		//      gi.sound (self, CHAN_VOICE, actor.sound_gib, 1, ATTN_NORM, 0);
		for (n = 0; n < 2; n++)
			ThrowGib(self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n = 0; n < 4; n++)
			ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowHead(self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

	// regular death
	// add gender sounds later
	gi.sound(self, CHAN_VOICE, gi.soundindex("player/male/death1.wav"), 1, ATTN_NORM, 0);
	// remove weapon model
	self->s.modelindex2 = 0;
	self->s.sound = 0;

	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	self->monsterinfo.currentmove = &grunt_move_death1;
}

mframe_t grunt_frames_salute[] =
{
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL,
	ai_turn, 0,  NULL
};
mmove_t grunt_move_salute = { FRAME_salute01, FRAME_salute11, grunt_frames_salute, grunt_run };


void grunt_salute(edict_t *self, edict_t *other)
{
	vec3_t		v;

	VectorSubtract(other->s.origin, self->s.origin, v);
	self->ideal_yaw = vectoyaw(v);

	self->monsterinfo.currentmove = &grunt_move_salute;
}

void grunt_playeruse(edict_t *self, edict_t *other)
{
	if (self->enemy)
	{
		gi.cprintf(other, PRINT_CHAT, "%s: I'm a little busy right now!\n", self->monsterinfo.grunt_name);
		return;
	}

	if (self->movetarget && (self->movetarget != other))
	{
		gi.cprintf(other, PRINT_CHAT, "%s: I'm a little busy right now!\n", self->monsterinfo.grunt_name);
		return;
	}

	if (self->monsterinfo.grunt_follow)
	{
		self->monsterinfo.grunt_follow = 0;
		self->movetarget = self->goalentity = NULL;
		self->monsterinfo.pausetime = level.time + 100000000;
		self->monsterinfo.stand(self);

		gi.cprintf(other, PRINT_CHAT, "%s: I'll just wait here\n", self->monsterinfo.grunt_name);

		if (random() < 0.3)
			grunt_salute(self, other);

		return;
	}

	gi.cprintf(other, PRINT_CHAT, "%s: I'll be right behind you\n", self->monsterinfo.grunt_name);

	if (random() < 0.33)
		grunt_salute(self, other);

	self->monsterinfo.grunt_follow = 1;

	self->movetarget = self->goalentity = other;
	self->monsterinfo.run(self);

}

/*QUAKED misc_grunt (1 .5 0) (-16 -16 -24) (16 16 32)

*/

void SP_misc_grunt(edict_t *self)
{

	if (deathmatch->value)
	{
		G_FreeEdict(self);
		return;
	}

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	self->s.modelindex = gi.modelindex("grunt/male/tris.md2");

	if (self->monsterinfo.grunt_weapon == WEAP_MACHINEGUN)
		self->s.modelindex2 = gi.modelindex("players/male/w_machinegun.md2");

	if (self->monsterinfo.grunt_weapon == WEAP_CHAINGUN)
		self->s.modelindex2 = gi.modelindex("players/male/w_chaingun.md2");

	VectorSet(self->mins, -16, -16, -24);
	VectorSet(self->maxs, 16, 16, 32);

	if (!self->health)
		self->health = 125;

	self->mass = 200;

	self->die = grunt_die;
	self->pain = grunt_pain;
	self->playeruse = grunt_playeruse;

	self->monsterinfo.stand = grunt_stand;
	self->monsterinfo.walk = grunt_walk;
	self->monsterinfo.run = grunt_run;
	self->monsterinfo.attack = grunt_attack;
	self->monsterinfo.search = grunt_search;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = NULL;
	self->monsterinfo.jumpmove = grunt_jump;

	self->monsterinfo.attack_range = 900;

	// worth custom checkattack?
	// melee attack?

	self->monsterinfo.aiflags |= (AI_GOOD_GUY | AI_GRUNT);

	if (!self->monsterinfo.grunt_name)
		self->monsterinfo.grunt_name = "Grunt";

	self->descname = "Grunt";

	gi.linkentity(self);

	self->monsterinfo.currentmove = &grunt_move_stand;
	self->monsterinfo.aiflags2 |= AI2_LADDER;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start(self);

	self->s.skinnum = 3;

	// urgh - needed to prevent modelindex2 flaking out
	// due to changes in this client version?
	self->s.renderfx &= ~RF_FRAMELERP;

}

void grunt_deploy(edict_t *ent)
{
	vec3_t		offset, forward, right, start, end;
	vec3_t		mins, maxs;
	trace_t		tr;
	edict_t		*grunt;

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

	VectorSet(mins, -16, -16, -24);
	VectorSet(maxs, 16, 16, 32);

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

		ent->client->grunt_time = level.time;

		grunt = G_Spawn();
		grunt->classname = "misc_grunt";

		VectorCopy(tr.endpos, grunt->s.origin);
		VectorCopy(grunt->s.origin, grunt->s.old_origin);
		VectorSet(grunt->s.angles, 0, ent->s.angles[YAW], 0);

		grunt->monsterinfo.grunt_weapon = WEAP_CHAINGUN;

		SP_misc_grunt(grunt);


		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_TELEPORT_EFFECT);
		gi.WritePosition(tr.endpos); // move to end?
		gi.multicast(tr.endpos, MULTICAST_PVS);

		gi.sound(grunt, CHAN_VOICE, gi.soundindex("misc/tele_up.wav"), 1, ATTN_NORM, 0);

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
}