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

void UpdateChaseCam(edict_t *ent)
{
    vec3_t o, ownerv, goal;
    edict_t *targ;
    vec3_t forward, right;
    trace_t trace;
    int i;
    vec3_t angles;

	if (coop->value)
	{
		if (!ent->client->chase_target->inuse
			|| ent->client->chase_target->health <= 0) {
			edict_t *old = ent->client->chase_target;
			ChaseNext(ent);
			if (ent->client->chase_target == old) {
				ent->client->chase_target = NULL;
				ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
				return;
			}
		}
	} else {
		// is our chase target gone?
		if (!ent->client->chase_target->inuse
			|| ent->client->chase_target->client->resp.spectator) {
			edict_t *old = ent->client->chase_target;
			ChaseNext(ent);
			if (ent->client->chase_target == old) {
				ent->client->chase_target = NULL;
				ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
				return;
			}
		}
	}

    targ = ent->client->chase_target;

    VectorCopy(targ->s.origin, ownerv);

	if (coop->value)
	{
		ownerv[2] += targ->viewheight * 2;
		VectorCopy(targ->s.angles, angles);
	}
	else
	{
		ownerv[2] += targ->viewheight;
		VectorCopy(targ->client->v_angle, angles);
	}

    if (angles[PITCH] > 56)
        angles[PITCH] = 56;
    AngleVectors(angles, forward, right, NULL);
    VectorNormalize(forward);

	if (coop->value)
		VectorMA(ownerv, -128, forward, o);
	else
		VectorMA(ownerv, -30, forward, o);

    if (o[2] < targ->s.origin[2] + 20)
        o[2] = targ->s.origin[2] + 20;

    // jump animation lifts
    if (!targ->groundentity)
        o[2] += 16;

    trace = gi.trace(ownerv, vec3_origin, vec3_origin, o, targ, MASK_SOLID);

    VectorCopy(trace.endpos, goal);

    VectorMA(goal, 2, forward, goal);

    // pad for floors and ceilings
    VectorCopy(goal, o);
    o[2] += 6;
    trace = gi.trace(goal, vec3_origin, vec3_origin, o, targ, MASK_SOLID);
    if (trace.fraction < 1) {
        VectorCopy(trace.endpos, goal);
        goal[2] -= 6;
    }

    VectorCopy(goal, o);
    o[2] -= 6;
    trace = gi.trace(goal, vec3_origin, vec3_origin, o, targ, MASK_SOLID);
    if (trace.fraction < 1) {
        VectorCopy(trace.endpos, goal);
        goal[2] += 6;
    }

	if (targ->deadflag)
		ent->client->ps.pmove.pm_type = PM_DEAD;
	else
		ent->client->ps.pmove.pm_type = PM_FREEZE;

    VectorCopy(goal, ent->s.origin);

	if (coop->value)
	{
		for (i = 0; i < 3; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(targ->s.angles[i] - ent->client->resp.cmd_angles[i]);
	} else {
		for (i = 0; i < 3; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(targ->client->v_angle[i] - ent->client->resp.cmd_angles[i]);
	}

	if (targ->deadflag) {
		ent->client->ps.viewangles[ROLL] = 40;
		ent->client->ps.viewangles[PITCH] = -15;
		if (coop->value)
			ent->client->ps.viewangles[YAW] = targ->s.angles[YAW];
		else
			ent->client->ps.viewangles[YAW] = targ->client->killer_yaw;
    } else {
		if (coop->value)
		{
			VectorCopy(targ->s.angles, ent->client->ps.viewangles);
			VectorCopy(targ->s.angles, ent->client->v_angle);
		} else {
			VectorCopy(targ->client->v_angle, ent->client->ps.viewangles);
			VectorCopy(targ->client->v_angle, ent->client->v_angle);
		}
    }

    ent->viewheight = 0;
    ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
    gi.linkentity(ent);
}

void ChaseNext(edict_t *ent)
{
    int i;
    edict_t *e;

    if (!ent->client->chase_target)
        return;

	ent->client->chase_mode = qfalse;

	// if coop keep moving through ents until we find a monster

	if (coop->value)
	{
		i = ent->client->chase_target - g_edicts;
		do {
			i++;
			if (i > globals.num_edicts)
				i = game.maxclients + 1;
			e = g_edicts + i;
			if (!e->inuse)
				continue;
			if (e->svflags & SVF_MONSTER && e->health > 0 && !(e->svflags & SVF_NOCLIENT))
				break;
		} while (e != ent->client->chase_target);

		ent->client->chase_target = e;
		ent->client->update_chase = qtrue;

		return;
	}

    i = ent->client->chase_target - g_edicts;
    do {
        i++;
        if (i > maxclients->value)
            i = 1;
        e = g_edicts + i;
        if (!e->inuse)
            continue;
        if (!e->client->resp.spectator)
            break;
    } while (e != ent->client->chase_target);

    ent->client->chase_target = e;
    ent->client->update_chase = qtrue;
}

void ChasePrev(edict_t *ent)
{
    int i;
    edict_t *e;

    if (!ent->client->chase_target)
        return;

	ent->client->chase_mode = qfalse;

	if (coop->value)
	{
		i = ent->client->chase_target - g_edicts;
		do {
			i--;
			if (i < game.maxclients + 1)
				i = globals.num_edicts;
			e = g_edicts + i;
			if (!e->inuse)
				continue;
			if (e->svflags & SVF_MONSTER && e->health > 0 && !(e->svflags & SVF_NOCLIENT))
				break;
		} while (e != ent->client->chase_target);

		ent->client->chase_target = e;
		ent->client->update_chase = qtrue;

		return;
	}

    i = ent->client->chase_target - g_edicts;
    do {
        i--;
        if (i < 1)
            i = maxclients->value;
        e = g_edicts + i;
        if (!e->inuse)
            continue;
        if (!e->client->resp.spectator)
            break;
    } while (e != ent->client->chase_target);

    ent->client->chase_target = e;
    ent->client->update_chase = qtrue;
}

void GetChaseTarget(edict_t *ent)
{
    int i;
    edict_t *other;

	ent->client->chase_mode = qfalse;

	if (coop->value)
	{
		for (i = game.maxclients + 1; i < globals.num_edicts; i++) {
			other = g_edicts + i;
			if (other->inuse && other->svflags & SVF_MONSTER && other->health > 0 && !(other->svflags & SVF_NOCLIENT)) {
				ent->client->chase_target = other;
				ent->client->update_chase = qtrue;
				UpdateChaseCam(ent);
				return;
			}
		}

		gi.centerprintf(ent, "No monsters found");

		return;
	}

    for (i = 1; i <= maxclients->value; i++) {
        other = g_edicts + i;
        if (other->inuse && !other->client->resp.spectator) {
            ent->client->chase_target = other;
            ent->client->update_chase = qtrue;
            UpdateChaseCam(ent);
            return;
        }
    }
    gi.centerprintf(ent, "No other players to chase.");
}

