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
// m_move.c -- monster movement

#include "g_local.h"

#define STEPSIZE			18
#define COMBAT_STEPSIZE		22 // Rroff need to be careful with this as monsters get stuck in player

/*
=============
M_CheckBottom

Returns qfalse if any part of the bottom of the entity is off an edge that
is not a staircase.

=============
*/
int c_yes, c_no;

qboolean M_CheckBottom(edict_t *ent)
{
    vec3_t  mins, maxs, start, stop;
    trace_t trace;
    int     x, y;
    float   mid, bottom;

    VectorAdd(ent->s.origin, ent->mins, mins);
    VectorAdd(ent->s.origin, ent->maxs, maxs);

// if all of the points under the corners are solid world, don't bother
// with the tougher checks
// the corners must be within 16 of the midpoint
    start[2] = mins[2] - 1;
    for (x = 0 ; x <= 1 ; x++)
        for (y = 0 ; y <= 1 ; y++) {
            start[0] = x ? maxs[0] : mins[0];
            start[1] = y ? maxs[1] : mins[1];
            if (gi.pointcontents(start) != CONTENTS_SOLID)
                goto realcheck;
        }

    c_yes++;
    return qtrue;        // we got out easy

realcheck:
    c_no++;
//
// check it for real...
//
    start[2] = mins[2];

// the midpoint must be within 16 of the bottom
    start[0] = stop[0] = (mins[0] + maxs[0]) * 0.5;
    start[1] = stop[1] = (mins[1] + maxs[1]) * 0.5;
    stop[2] = start[2] - 2 * STEPSIZE;
    trace = gi.trace(start, vec3_origin, vec3_origin, stop, ent, MASK_MONSTERSOLID);

    if (trace.fraction == 1.0)
        return qfalse;
    mid = bottom = trace.endpos[2];

// the corners must be within 16 of the midpoint
    for (x = 0 ; x <= 1 ; x++)
        for (y = 0 ; y <= 1 ; y++) {
            start[0] = stop[0] = x ? maxs[0] : mins[0];
            start[1] = stop[1] = y ? maxs[1] : mins[1];

            trace = gi.trace(start, vec3_origin, vec3_origin, stop, ent, MASK_MONSTERSOLID);

            if (trace.fraction != 1.0 && trace.endpos[2] > bottom)
                bottom = trace.endpos[2];
            if (trace.fraction == 1.0 || mid - trace.endpos[2] > STEPSIZE)
                return qfalse;
        }

    c_yes++;
    return qtrue;
}


/*
=============
SV_movestep

Called by monster program code.
The move will be adjusted for slopes and stairs, but if the move isn't
possible, no move is done, qfalse is returned, and
pr_global_struct->trace_normal is set to the normal of the blocking wall
=============
*/
//FIXME since we need to test end position contents here, can we avoid doing
//it again later in catagorize position?
qboolean SV_movestep(edict_t *ent, vec3_t move, qboolean relink)
{
    float       dz;
    vec3_t      oldorg, neworg, end;
    trace_t     trace;
    int         i;
    float       stepsize;
    vec3_t      test;
    int         contents;
	ai_avoid_t	*avoid;
	vec3_t		v;

// try the move
    VectorCopy(ent->s.origin, oldorg);
    VectorAdd(ent->s.origin, move, neworg);

	// Rroff - crude method to try and stop lockdown monsters
	// frying themselves on the lasers
	if ((ent->monsterinfo.aiflags2 & AI2_CONTROL))
	{
		for (avoid = ai_avoid_list; avoid->mapname; avoid++)
		{
			if (Q_stricmp(level.mapname, avoid->mapname) == 0)
			{
				VectorSubtract(avoid->origin, neworg, v);
				if (VectorLength(v) <= avoid->dist)
				{
					// don't let them jump immediately so they don't try
					// to hurdle the lasers!
					ent->monsterinfo.last_jump_time = level.time + 3;
					return qfalse;
				}
			}
		}
	}

// flying monsters don't step up
    if (ent->flags & (FL_SWIM | FL_FLY)) {
        // try one move with vertical motion, then one without
        for (i = 0 ; i < 2 ; i++) {
            VectorAdd(ent->s.origin, move, neworg);
            if (i == 0 && ent->enemy) {
                if (!ent->goalentity)
                    ent->goalentity = ent->enemy;
                dz = ent->s.origin[2] - ent->goalentity->s.origin[2];
                if (ent->goalentity->client) {
                    if (dz > 40)
                        neworg[2] -= 8;
                    if (!((ent->flags & FL_SWIM) && (ent->waterlevel < 2)))
                        if (dz < 30)
                            neworg[2] += 8;
                } else {
                    if (dz > 8)
                        neworg[2] -= 8;
                    else if (dz > 0)
                        neworg[2] -= dz;
                    else if (dz < -8)
                        neworg[2] += 8;
                    else
                        neworg[2] += dz;
                }
            }
            trace = gi.trace(ent->s.origin, ent->mins, ent->maxs, neworg, ent, MASK_MONSTERSOLID);

            // fly monsters don't enter water voluntarily
            if (ent->flags & FL_FLY) {
                if (!ent->waterlevel) {
                    test[0] = trace.endpos[0];
                    test[1] = trace.endpos[1];
                    test[2] = trace.endpos[2] + ent->mins[2] + 1;
                    contents = gi.pointcontents(test);
                    if (contents & MASK_WATER)
                        return qfalse;
                }
            }

            // swim monsters don't exit water voluntarily
            if (ent->flags & FL_SWIM) {
                if (ent->waterlevel < 2) {
                    test[0] = trace.endpos[0];
                    test[1] = trace.endpos[1];
                    test[2] = trace.endpos[2] + ent->mins[2] + 1;
                    contents = gi.pointcontents(test);
                    if (!(contents & MASK_WATER))
                        return qfalse;
                }
            }

            if (trace.fraction == 1) {
                VectorCopy(trace.endpos, ent->s.origin);
                if (relink) {
                    gi.linkentity(ent);
                    G_TouchTriggers(ent);
                }
                return qtrue;
            }

            if (!ent->enemy)
                break;
        }

        return qfalse;
    }

// push down from a step height above the wished position
	// Rroff increase step size if chasing an enemy
	// might have unpredictable results or look odd if its too big
    if (!(ent->monsterinfo.aiflags & AI_NOSTEP))
		if (ent->enemy)
		{
			stepsize = COMBAT_STEPSIZE;
		} else {
			stepsize = STEPSIZE;
		}
    else
        stepsize = 1;

    neworg[2] += stepsize;
    VectorCopy(neworg, end);
    end[2] -= stepsize * 2;

    trace = gi.trace(neworg, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);

    if (trace.allsolid)
        return qfalse;

    if (trace.startsolid) {
        neworg[2] -= stepsize;
        trace = gi.trace(neworg, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);
        if (trace.allsolid || trace.startsolid)
            return qfalse;
    }


    // don't go in to water
	// this causes a problem if the monster is on a low bridge
	// over water
	// can we just let them go into water until their water level
	// hits 1?

	// don't allow monsters to walk into slime, etc.

	if (ent->waterlevel == 0) {
		test[0] = trace.endpos[0];
		test[1] = trace.endpos[1];
		test[2] = trace.endpos[2] + ent->mins[2] + 1;
		contents = gi.pointcontents(test);

		if (contents & (CONTENTS_LAVA|CONTENTS_SLIME))
			return qfalse;
	}

    /*if (ent->waterlevel == 0) {
        test[0] = trace.endpos[0];
        test[1] = trace.endpos[1];
		// Rroff - just testing - don't go into deep water but allow shallow?
        //test[2] = trace.endpos[2] + ent->mins[2] + 1;
		test[2] = (trace.endpos[2] + ent->mins[2] + 1) - 22; // was 22
        contents = gi.pointcontents(test);

        if (contents & MASK_WATER)
            return qfalse;
    }*/

	// Rroff don't allow transition to deep water
	// not properly tested at this time - might result in running on the spot in water
	//if (ent->waterlevel == 1) {
	// doesn't work well on slopes - maybe check ahead
	// might not have any options in some cases
	if (ent->waterlevel == 1) {
		test[0] = trace.endpos[0];
		test[1] = trace.endpos[1];
		test[2] = trace.endpos[2]; // was 26
		// at this point we are wading so need to know water depth really
		contents = gi.pointcontents(test);

		if (contents & MASK_WATER)
			return qfalse;

		// as a hack don't go down slopes in water
		//if (trace.plane.normal && trace.plane.normal[2] < 0.7)
		//	return qfalse;
	}

    if (trace.fraction == 1) {
        // if monster had the ground pulled out, go ahead and fall
        if (ent->flags & FL_PARTIALGROUND) {
            VectorAdd(ent->s.origin, move, ent->s.origin);
            if (relink) {
                gi.linkentity(ent);
                G_TouchTriggers(ent);
            }
            ent->groundentity = NULL;
            return qtrue;
        }

        return qfalse;       // walked off an edge
    }

// check point traces down for dangling corners
    VectorCopy(trace.endpos, ent->s.origin);

    if (!M_CheckBottom(ent)) {
        if (ent->flags & FL_PARTIALGROUND) {
            // entity had floor mostly pulled out from underneath it
            // and is trying to correct
            if (relink) {
                gi.linkentity(ent);
                G_TouchTriggers(ent);
            }
            return qtrue;
        }
        VectorCopy(oldorg, ent->s.origin);
        return qfalse;
    }

    if (ent->flags & FL_PARTIALGROUND) {
        ent->flags &= ~FL_PARTIALGROUND;
    }
    ent->groundentity = trace.ent;
    ent->groundentity_linkcount = trace.ent->linkcount;

// the move is ok
    if (relink) {
        gi.linkentity(ent);
        G_TouchTriggers(ent);
    }
    return qtrue;
}


//============================================================================

/*
===============
M_ChangeYaw

===============
*/
void M_ChangeYaw(edict_t *ent)
{
    float   ideal;
    float   current;
    float   move;
    float   speed;

    current = anglemod(ent->s.angles[YAW]);
    ideal = ent->ideal_yaw;

    if (current == ideal)
        return;

    move = ideal - current;
    speed = ent->yaw_speed;
    if (ideal > current) {
        if (move >= 180)
            move = move - 360;
    } else {
        if (move <= -180)
            move = move + 360;
    }
    if (move > 0) {
        if (move > speed)
            move = speed;
    } else {
        if (move < -speed)
            move = -speed;
    }

    ent->s.angles[YAW] = anglemod(current + move);
}


/*
======================
SV_StepDirection

Turns to the movement direction, and walks the current distance if
facing it.

======================
*/
qboolean SV_StepDirection(edict_t *ent, float yaw, float dist)
{
    vec3_t      move, oldorigin;
    float       delta;

    ent->ideal_yaw = yaw;
    M_ChangeYaw(ent);

    yaw = yaw * M_PI * 2 / 360;
    move[0] = cos(yaw) * dist;
    move[1] = sin(yaw) * dist;
    move[2] = 0;

    VectorCopy(ent->s.origin, oldorigin);
    if (SV_movestep(ent, move, qfalse)) {
        delta = ent->s.angles[YAW] - ent->ideal_yaw;
        if (delta > 45 && delta < 315) {
            // not turned far enough, so don't take the step
            VectorCopy(oldorigin, ent->s.origin);
        }
        gi.linkentity(ent);
        G_TouchTriggers(ent);
        return qtrue;
    }
    gi.linkentity(ent);
    G_TouchTriggers(ent);
    return qfalse;
}

/*
======================
SV_FixCheckBottom

======================
*/
void SV_FixCheckBottom(edict_t *ent)
{
    ent->flags |= FL_PARTIALGROUND;
}



/*
================
SV_NewChaseDir

================
*/
#define DI_NODIR    -1
void SV_NewChaseDir(edict_t *actor, edict_t *enemy, float dist)
{
    float   deltax, deltay;
    float   d[3];
    float   tdir, olddir, turnaround;

    //FIXME: how did we get here with no enemy
    if (!enemy)
        return;

	// Rroff prevent roaming enemy changing direction too frequently
	if (level.time < actor->monsterinfo.roamer_time + 1)
	{
		return; // testing don't try to reset yaw too often
	}

    olddir = anglemod((int)(actor->ideal_yaw / 45) * 45);
    turnaround = anglemod(olddir - 180);

    deltax = enemy->s.origin[0] - actor->s.origin[0];
    deltay = enemy->s.origin[1] - actor->s.origin[1];
    if (deltax > 10)
        d[1] = 0;
    else if (deltax < -10)
        d[1] = 180;
    else
        d[1] = DI_NODIR;
    if (deltay < -10)
        d[2] = 270;
    else if (deltay > 10)
        d[2] = 90;
    else
        d[2] = DI_NODIR;

// try direct route
    if (d[1] != DI_NODIR && d[2] != DI_NODIR) {
        if (d[1] == 0)
            tdir = d[2] == 90 ? 45 : 315;
        else
            tdir = d[2] == 90 ? 135 : 215;

        if (tdir != turnaround && SV_StepDirection(actor, tdir, dist))
            return;
    }

// try other directions
    if (((rand() & 3) & 1) ||  fabsf(deltay) > fabsf(deltax)) {
        tdir = d[1];
        d[1] = d[2];
        d[2] = tdir;
    }

    if (d[1] != DI_NODIR && d[1] != turnaround
        && SV_StepDirection(actor, d[1], dist))
        return;

    if (d[2] != DI_NODIR && d[2] != turnaround
        && SV_StepDirection(actor, d[2], dist))
        return;

    /* there is no direct path to the player, so pick another direction */

    if (olddir != DI_NODIR && SV_StepDirection(actor, olddir, dist))
        return;

    if (rand() & 1) { /*randomly determine direction of search*/
        for (tdir = 0 ; tdir <= 315 ; tdir += 45)
            if (tdir != turnaround && SV_StepDirection(actor, tdir, dist))
                return;
    } else {
        for (tdir = 315 ; tdir >= 0 ; tdir -= 45)
            if (tdir != turnaround && SV_StepDirection(actor, tdir, dist))
                return;
    }

    if (turnaround != DI_NODIR && SV_StepDirection(actor, turnaround, dist))
        return;

    actor->ideal_yaw = olddir;      // can't move

// if a bridge was pulled out from underneath a monster, it may not have
// a valid standing position at all

    if (!M_CheckBottom(actor))
        SV_FixCheckBottom(actor);
}

/*
======================
SV_CloseEnough

======================
*/
qboolean SV_CloseEnough(edict_t *ent, edict_t *goal, float dist)
{
    int     i;

    for (i = 0 ; i < 3 ; i++) {
        if (goal->absmin[i] > ent->absmax[i] + dist)
            return qfalse;
        if (goal->absmax[i] < ent->absmin[i] - dist)
            return qfalse;
    }
    return qtrue;
}

/*
======================
SV_HCloseEnough
Horizontally close enough
======================
*/
qboolean SV_HCloseEnough(edict_t *ent, edict_t *goal, int dist)
{
	int		s1, s2, distance;

	s1 = abs(ent->s.origin[0] - goal->s.origin[0]);
	s2 = abs(ent->s.origin[1] - goal->s.origin[1]);
	distance = sqrt(s1 * s1 + s2 * s2);

	if (distance > dist)
		return qfalse;

	return qtrue;
}

/*
======================
SV_HSLCloseEnough
Horizontally close enough at roughly the same level
======================
*/
qboolean SV_HSLCloseEnough(edict_t *ent, edict_t *goal, int dist)
{
	int		s1, s2, distance, height;

	s1 = abs(ent->s.origin[0] - goal->s.origin[0]);
	s2 = abs(ent->s.origin[1] - goal->s.origin[1]);
	height = fabsf(ent->s.origin[2] - goal->s.origin[2]);
	distance = sqrt(s1 * s1 + s2 * s2);

	if (height > 64)
		return qfalse;

	if (distance > dist)
		return qfalse;

	return qtrue;
}

// find a point that is at least 300 units from us, further from our enemy than we are
// and closer to us than it is to the enemy
// don't call this without a valid enemy or old_enemy or failing that goalentity

edict_t	*runpoint(edict_t *self)
{
	edict_t		*ent = NULL, *avoid = NULL, *best = NULL;
	vec3_t		v;
	float		dist1, dist2, dist3;
	// dist1 is our distance from the objects
	// dist2 is our distance from our enemy
	// dist3 is the distance of the ent from our enemy

	if (!self->enemy)
	{
		if (self->oldenemy)
		{
			avoid = self->oldenemy;
		} else {
			avoid = self->goalentity; // this might not have desired effect
		}
		if (!avoid)
			return NULL;
	} else {
		avoid = self->enemy;
	}

	while ((ent = findradius(ent, self->s.origin, 1024)) != NULL) {
		if (ent == avoid)
			continue;
		if (self->goalentity)
		{
			if (ent == self->goalentity)
				continue;
		}
		if (ent->client)
			continue;
		if (!visible(self, ent))
			continue;
		if (abs(self->s.origin[2] - ent->s.origin[2]) > 96)
			continue;
		VectorSubtract(ent->s.origin, self->s.origin, v);
		dist1 = VectorLength(v);
		if (dist1 < 300)
			continue;
		VectorSubtract(avoid->s.origin, self->s.origin, v);
		dist2 = VectorLength(v);
		VectorSubtract(ent->s.origin, avoid->s.origin, v);
		dist3 = VectorLength(v);
		if (dist3 < dist2)
			continue;
		if (dist3 < dist1)
			continue;
		if (ent)
		{
			best = ent;
			if (random() < 0.3)
				break;
		}
	}

	if (!(ent) && (best))
		return best;

	return ent;
}


/*
======================
M_MoveToGoal
======================
*/
void M_MoveToGoal(edict_t *ent, float dist)
{
    edict_t     *goal, *spot;
	qboolean	valid;

	vec3_t		forward, start, end, aimdir;
	vec3_t		mins, maxs, v, point;
	vec3_t		ladder_start;
	trace_t		tr;
	float		vangle;
	float		len, d, h, av;
	qboolean	doJump = qfalse;
	qboolean	preJump = qfalse;
	float		edist, vdist;

    goal = ent->goalentity;

	// Rroff check for ladder

	if (!(ent->groundentity) && (ent->monsterinfo.aiflags2 & AI2_LADDER))
	{
		VectorCopy(ent->s.origin, ladder_start);
		ladder_start[2] -= ent->mins[2];
		AngleVectors(ent->s.angles, forward, NULL, NULL);
		VectorCopy(forward, aimdir);
		VectorMA(ladder_start, 8, aimdir, end);
		//VectorSet(mins, -16, -16, -8);
		VectorSet(mins, -16, -16, 0);
		VectorSet(maxs, 16, 16, 8);

		tr = gi.trace(ladder_start, mins, maxs, end, ent, MASK_ALL);

		if (tr.fraction < 1.0)
		{

			if (tr.fraction < 0.1 && !(ent->flags & (FL_FLY | FL_SWIM)))
			{
				if (tr.contents & CONTENTS_LADDER)
				{
					// forward doesn't really do anything here :(
					// get cancelled out before it is clear to move that direction
					VectorScale(forward, 200, ent->velocity);
					if (ent->velocity[2] < 400)
						ent->velocity[2] = 400;

					ent->monsterinfo.last_jump_time = level.time;
				}
			}
		}
	}

	// Rroff help to clear lips? seems to work
	// could also base it on vertical velocity
	//if (!ent->groundentity && ((level.time > ent->monsterinfo.last_jump_time + 0.1) && (level.time < ent->monsterinfo.last_jump_time + 0.6)))
	if (!(ent->groundentity) && ((level.time > ent->monsterinfo.last_jump_time) && (level.time < ent->monsterinfo.last_jump_time + 0.4)))
	{
			//AngleVectors(ent->s.angles, forward, NULL, NULL);
			//VectorMA(ent->velocity, 120, forward, ent->velocity);
			av = ent->velocity[2] * 0.5;
			if (av > 0)
			{
				AngleVectors(ent->s.angles, forward, NULL, NULL);
				VectorMA(ent->velocity, av, forward, ent->velocity);
			}
	}

	// horrid hack to try and fix something
	// sometimes monsters that have jumped into water get stuck between two
	// status otherwise but this sometimes makes them shoot through the water in a glide motion
	// other changes might have fixed it so commented out for now

	/*if (!(!(ent->flags & FL_SWIM) && (ent->waterlevel > 0 && ent->waterlevel < 3)))
	{
		if (!ent->groundentity && !(ent->flags & (FL_FLY | FL_SWIM)))
			return;
	}*/

	// our original attempt to fix above - probably have to reuse this

	if (!(!(ent->flags & FL_SWIM) && (ent->waterlevel == 1)))
	{
		if (!ent->groundentity && !(ent->flags & (FL_FLY | FL_SWIM)))
			return;
	}


	//if (!ent->groundentity && !(ent->flags & (FL_FLY | FL_SWIM)))
	//	return;

// if the next step hits the enemy, return immediately
//    if (ent->enemy && SV_CloseEnough(ent, ent->enemy, dist))
//        return;

	valid = SV_StepDirection(ent, ent->ideal_yaw, dist);

	if ((ent->monsterinfo.aiflags2 & AI2_CONTROL) && (ent->monsterinfo.aiflags2 & AI2_CONTROL_SEARCH) && !(ent->monsterinfo.aiflags & AI_ROAMING) && !(valid))
	{
		ent->monsterinfo.ai_roaming_time = level.time + 2;
		ent->monsterinfo.aiflags |= AI_ROAMING;
		ent->monsterinfo.ai_current = NULL;
	}

	// Rroff try and pre-emptively jump upwards
	// this duplicates a lot of the processing of the roamer bit below so combine these later
	// had some problems with the medic stuff - probably sorted now but left in here incase

	if (ent->enemy && !(ent->monsterinfo.aiflags & AI_MEDIC) && !(ent->monsterinfo.aiflags & AI_MEDIC2))
	{
		// randomly try a different path?
		//if (valid && (random() < 0.1) && !(ent->monsterinfo.aiflags & AI_RUNAWAY))
		if (valid && !(ent->monsterinfo.aiflags & AI_RUNAWAY) && !(ent->monsterinfo.aiflags & AI_STAND_GROUND))
		{
			// This will delay the first time this happens until up to 45 seconds into a map
			//if (level.time > (ent->monsterinfo.runaway_time + (15 + rand() % 30)))
			if ((level.time > (ent->monsterinfo.runaway_time + 5)) && !(ent->monsterinfo.aiflags & AI_GRUNT))
			{
				//SV_NewChaseDir(ent, goal, dist);
				/*vangle = ent->s.angles[YAW] - 180;
				vangle = vangle + (-120 + (random() * 240));

				if (vangle < 0)
					vangle = 360 + vangle;

				if (vangle > 360)
					vangle = vangle - 360;

				ent->ideal_yaw = vangle;*/
				VectorSubtract(ent->enemy->s.origin, ent->s.origin, v);
				edist = VectorLength(v);
				vdist = fabsf(ent->s.origin[2] - ent->enemy->s.origin[2]);

				if (edist > 1024 || ((SV_HCloseEnough(ent, ent->enemy, 68)) && vdist > 16 && !(ent->flags & (FL_FLY | FL_SWIM))))
				{
					// should use a movement chain really
					// using objects the monster can see
					spot = runpoint(ent);
					if (spot)
					{
						ent->oldenemy = ent->enemy;
						//ent->enemy = NULL;
						ent->goalentity = spot;
						ent->monsterinfo.run(ent);
						ent->monsterinfo.aiflags |= AI_RUNAWAY;
						ent->monsterinfo.runaway_time = level.time;
					}
					// return;
				}
			}
		}

		AngleVectors(ent->s.angles, forward, NULL, NULL);
		VectorCopy(forward, aimdir);
		aimdir[2] = 0;

		len = 96;
		VectorMA(ent->s.origin, len, aimdir, end);
		VectorSet(mins, -16, -16, -8);
		VectorSet(maxs, 16, 16, 8);

		tr = gi.trace(ent->s.origin, mins, maxs, end, ent, MASK_ALL);

		if (tr.fraction < 1.0)
		{

			// ladders
			// need to make sure they are square on the ladder really otherwise
			// tend to hit things overhanging a ladder and bounce
			if ((tr.fraction < 0.1) && !(ent->flags & (FL_FLY | FL_SWIM)) && (ent->monsterinfo.aiflags2 & AI2_LADDER))
			{
				if (tr.contents & CONTENTS_LADDER)
				{
					// check if we are hitting something above and nudge?
					VectorScale(forward, 150, ent->velocity);
					ent->velocity[2] = 350;
					return;
				}
			}

			preJump = qtrue;

			if (tr.ent)
			{
				if (tr.ent == ent->enemy)
					preJump = qfalse;
				if (tr.ent->client)
				{
					if ((ent->monsterinfo.aiflags & AI_RUNAWAY) && !(ent->monsterinfo.aiflags & AI_ASSUMED))
					{
						vangle = ent->s.angles[YAW] - 180;
						vangle = vangle + (-120 + (random() * 240));

						if (vangle < 0)
							vangle = 360 + vangle;

						if (vangle > 360)
							vangle = vangle - 360;

						ent->ideal_yaw = vangle;
					}
				}
			}
		}
	}

	// Rroff main jump check - if a fly/swim monster has a jump function this might do odd things
	// use a rand here to calm them down a bit so they aren't constantly jumping

	if (preJump || (!(valid) && (ent->enemy)))
	{
		VectorCopy(ent->maxs, maxs);
		maxs[2] = ent->mins[2];

		AngleVectors(ent->s.angles, forward, NULL, NULL);
		VectorNormalize(forward);
		VectorCopy(ent->s.origin, start);
		VectorAdd(ent->mins, maxs, v);
		//d = VectorLength(v) * 0.5;
		d = VectorLength(v);
		//VectorMA(start, d + 12, forward, start);
		//if (preJump)
		//	d += 96; // was 96
		//VectorMA(start, d + 48, forward, start);
		VectorMA(start, d, forward, start);
		start[2] += ent->viewheight;
		VectorCopy(start, end);
		end[2] += (2 * ent->maxs[2]);

		tr = gi.trace(start, ent->mins, ent->maxs, end, ent, MASK_SHOT); // don't want to try and jump monster solid clips

		// this might work a bit odd if the ladder is too far away
		// tuning the fraction result below 1.0 might help
		// might need to change the last jump time as well

		if (tr.fraction < 1.0 && !(ent->flags & (FL_FLY | FL_SWIM)))
		{
			if (tr.contents & CONTENTS_LADDER)
			{
				// check if we are hitting something above and nudge?
				VectorScale(forward, 300, ent->velocity);
				if (ent->velocity[2] < 350)
					ent->velocity[2] = 350;
				return;
			}
		}

		if (!tr.allsolid)
		{
			doJump = qtrue;
		}

		// Might want to trace forward here so as to try and avoid jumping over other monsters too much

		// non-exhaustive, limited check to try and avoid jumping into voids or lava, etc.

		// Get a rough idea of our landing position
		// this doesn't work well if the jump is partially blocked

		VectorCopy(tr.endpos, start);

		VectorMA(tr.endpos, d + 160, forward, end);

		tr = gi.trace(start, ent->mins, ent->maxs, end, ent, MASK_SHOT);

		/*gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_DEBUGTRAIL);
		gi.WritePosition(start);
		gi.WritePosition(tr.endpos);
		gi.multicast(start, MULTICAST_PVS);*/

		VectorCopy(tr.endpos, start);
		//VectorMA(start, d * 2, forward, start);
		VectorCopy(start, end);
		end[2] -= 384;

		//tr = gi.trace(start, ent->mins, ent->maxs, end, ent, (MASK_SHOT | MASK_WATER));
		tr = gi.trace(start, vec3_origin, vec3_origin, end, ent, (MASK_MONSTERSOLID | MASK_WATER));

		/*gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_DEBUGTRAIL);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (start, MULTICAST_PVS);*/

		// we reached the bottom - it's too far to jump down
		// or we would land in something nasty so don't jump
		VectorCopy(tr.endpos, point);
		point[2] += ent->mins[2];

		if ((tr.fraction == 1.0) || (gi.pointcontents(point) & MASK_WATER) || (tr.contents & MASK_WATER) || tr.allsolid)
		{
			doJump = qfalse;
		}


		// this hasn't been tested
		if (tr.fraction < 1.0)
		{
			if (tr.plane.normal[2]) {
				if (tr.plane.normal[2] < 0.7) // only do jumps onto very level ground
				{
					doJump = qfalse;
				}
			}
		}

		// need to trace up from the final landing position to test this
		// probably should check the monster has headroom before even trying
		//a jump as well but that is more traces for minimal return
		/*h = (ent->maxs[2] - ent->mins[2])* 1.25;

		// Don't jump if the ending position is cramped
		if (start[2] - tr.endpos[2] < h)
			doJump = qfalse;
			*/

		// some monsters that haven't got a jump move still need to be processed through some stuff here
		// potentially depending on features we are using

		// checking ent->monsterinfo.last_jump_time isn't default stops the initial jump
		// though I might leave that in as it sometimes is pretty cool

		//if (!ent->monsterinfo.last_jump_time) - do we need to initialise it?
		//((rand() % 3) < 2) &&

		if ((doJump) && (ent->monsterinfo.jumpmove) && (level.time > ent->monsterinfo.last_jump_time + 1))
		{
			// prevent monster constantly trying an incorrect move to nowhere
			if ((random() < 0.3) && !(ent->monsterinfo.aiflags & AI_GRUNT))
			{
				ent->monsterinfo.last_jump_time = level.time + 2;
				SV_NewChaseDir(ent, goal, dist);
				return;
			}
			ent->monsterinfo.jumpmove(ent);
			ent->monsterinfo.last_jump_time = level.time;
			return;
		} else {
			if ((random() < 0.07) && !(ent->monsterinfo.aiflags & AI_GRUNT)) {
				spot = runpoint(ent);
				if (spot)
				{
					ent->oldenemy = ent->enemy;
					//ent->enemy = NULL;
					ent->goalentity = spot;
					ent->monsterinfo.run(ent);
					ent->monsterinfo.aiflags |= AI_RUNAWAY;
					ent->monsterinfo.runaway_time = level.time;
				}
			}
		}
		ent->monsterinfo.last_jump_time = level.time;
	}

	// if the next step hits the enemy, return immediately
	if (ent->enemy && SV_CloseEnough(ent, ent->enemy, dist))
	{
		// Rroff try and remove the running on the spot if the player is a bit above or below

		h = fabsf(ent->enemy->s.origin[2] - ent->s.origin[2]);
		if (h<22)
			return;
	}

	// Rroff make roaming enemy pre-emptively change direction if approaching a wall
	// need to avoid too long checks so they don't turn around before steps/slopes

	if (((ent->monsterinfo.aiflags & AI_ROAMING) || (ent->monsterinfo.aiflags & AI_RUNAWAY)) && !(ent->monsterinfo.aiflags & AI_ASSUMED)) // probably shouldn't do it with an enemy as it might mess with melee
	{
		if (!(ent->enemy) && (level.time > ent->monsterinfo.roamer_time + 1))
		{
			ent->monsterinfo.roamer_time = level.time;
			AngleVectors(ent->s.angles, forward, NULL, NULL);
			VectorCopy(forward, aimdir);

			//len = 34 + (rand() % 48);
			// could skip the 3rd axis here
			// unlikely to have a situation where mins and max are horizontally different


			len = sqrt((ent->maxs[0] * ent->maxs[0]) + (ent->maxs[1] * ent->maxs[1])) + 48;
			//len = 72;

			// should use monster mins/maxes + short dist
			VectorMA(ent->s.origin, len, aimdir, end); // was 64, then 72

			VectorSet(mins, -8, -8, -8);
			VectorSet(maxs, 8, 8, 8);

			tr = gi.trace(ent->s.origin, mins, maxs, end, ent, MASK_MONSTERSOLID);

			//if (tr.fraction != 1.0)
			if ((tr.fraction != 1.0) || (!valid))
			{
				vangle = ent->s.angles[YAW] - 180;

				while (vangle > 360)
					vangle -= 360;

				vangle = vangle + (-120 + (random() * 240));

				if (vangle < 0)
					vangle = 360 + vangle;

				if (vangle > 360)
					vangle = vangle - 360;

				ent->ideal_yaw = vangle;
			}
		}
	}

	if ((ent->monsterinfo.aiflags & AI_ASSUMED) && !valid) {
		if (ent->inuse)
			SV_NewChaseDir(ent, goal, dist);
		return;
	}

// bump around...
    if ((rand() & 3) == 1 || !valid) {
        if (ent->inuse)
            SV_NewChaseDir(ent, goal, dist);
    }
}


/*
===============
M_walkmove
===============
*/
qboolean M_walkmove(edict_t *ent, float yaw, float dist)
{
    vec3_t  move;

	// Rroff - another attempt to get monsters on top of ledges, etc. when jumping

	if (!ent->groundentity && !(ent->flags & (FL_FLY | FL_SWIM)))
	{
		if (!(ent->monsterinfo.jumpmove && ((level.time > ent->monsterinfo.last_jump_time) && (level.time < ent->monsterinfo.last_jump_time + 0.4))))
			return qfalse;
	}

    yaw = yaw * M_PI * 2 / 360;

    move[0] = cos(yaw) * dist;
    move[1] = sin(yaw) * dist;
    move[2] = 0;

    return SV_movestep(ent, move, qtrue);
}
