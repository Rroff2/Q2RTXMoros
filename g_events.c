#include "g_local.h"

// Rroff event system to do stuff outside of that defined in the map

// Currently upto 256 entries tracked in the level structure to save states where needed

// door inhibit entries with the same name as a spawn event will be disabled when that event activates
// otherwise they will be disabled when a player approaches the door with a security tag
// there is no checking of the security tag at this time having one will simply open the door and remove the inhibit

// name
// type of event i.e. button pressed
// point type i.e. spawn event, door inhibit
// message if used by the type of event
// effect if used by this event
// origin

// Remove trainling ,

event_point_t	event_point_list[] =
{
	{
		"base1$secret",
		"base1",
		EVENTFLAG_SECRET,
		EVENT_SPAWN,
		0,
		0,
		1089, 879, -161,
	},

	{
		"base3$quad",
		"base3",
		EVENTFLAG_PICKUP_POWER,
		EVENT_SPAWN,
		0,
		0,
		465, 551, -536,
	},

	// change to EVENT_GENERAL when we've implemented it
	{
		"bunk1$bridge",
		"bunk1",
		EVENTFLAG_BUTTON,
		EVENT_SPAWN,
		0,
		0,
		-514, -58, 256,
	},

	{
		"fact1$gbutton",
		"fact1",
		EVENTFLAG_BUTTON,
		EVENT_SPAWN,
		0,
		0,
		1588, -409, -81,
	},

	{
		"fact1$ebutton",
		"fact1",
		EVENTFLAG_BUTTON,
		EVENT_SPAWN,
		0,
		EVENTEFFECT_QUAKE_SHOCK,
		216, 630, 58,
	},

	// Event points that block things like doors until appropriate key is acquired
	// need to make sure these are only enabled per map name

	// door inhibits

	{
		"base1$exit",
		"base1",
		EVENTFLAG_NONE,
		EVENT_INHIBIT_DOOR_HEAD,
		"Biometric sensor denies you access",
		0,
		-1669, 1537, 142,
	},

	{
		"base3$train",
		"base3",
		EVENTFLAG_NONE,
		EVENT_INHIBIT_DOOR,
		"Denied! Security tag required",
		0,
		385, 859, -785,
	},

	{
		"bunk1$bridge",
		"bunk1",
		EVENTFLAG_NONE,
		EVENT_INHIBIT_DOOR,
		"This door is opened elsewhere.",
		0,
		-1849, 295, 46,
	},

	{
		"bunk1$exit",
		"bunk1",
		EVENTFLAG_NONE,
		EVENT_INHIBIT_DOOR,
		"Door requires a security tag!",
		0,
		-2903, 1473, 152,
	},

	{
		"fact1$exit1",
		"fact1",
		EVENTFLAG_NONE,
		EVENT_INHIBIT_DOOR,
		"Denied! Security tag required",
		0,
		1084, -1316, 46,
	},

	{
		"cool1$lifts",
		"cool1",
		EVENTFLAG_NONE,
		EVENT_INHIBIT_DOOR_HEAD,
		"Biometric sensor denies you access",
		0,
		-1396, -1524, -701,
	},

	{
		"hangar2$exit",
		"hangar2",
		EVENTFLAG_NONE,
		EVENT_INHIBIT_DOOR_HEAD,
		"Biometric sensor denies you access",
		EVENTEFFECT_LAUGH,
		884, -2397, -37,
	},

	// button inhibits

	{
		"jail1$exit",
		"jail1",
		EVENTFLAG_NONE,
		EVENT_INHIBIT_BUTTON,
		"Denied! Security tag required",
		EVENTEFFECT_LAUGH,
		-2147, 382, 120,
	},

	{
		"mine3$exit",
		"mine3",
		EVENTFLAG_NONE,
		EVENT_INHIBIT_BUTTON,
		"Denied! Security tag required",
		0,
		-1246, 1404, -509,
	},


	{ NULL }
};

// Effects for scripted events
void event_earthquake_think(edict_t *self)
{
	int     i;
	edict_t *e;

	if (self->last_move_time < level.time) {
		gi.positioned_sound(self->s.origin, self, CHAN_AUTO, self->noise_index, 1.0, ATTN_NONE, 0);
		self->last_move_time = level.time + 0.5;
	}

	for (i = 1, e = g_edicts + i; i < globals.num_edicts; i++, e++) {
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
		if (!e->groundentity)
			continue;

		e->groundentity = NULL;
		e->velocity[0] += crandom() * 150;
		e->velocity[1] += crandom() * 150;
		e->velocity[2] = self->speed * (100.0 / e->mass);
	}

	if (level.time < self->timestamp)
		self->nextthink = level.time + FRAMETIME;
	else
		G_FreeEdict(self);
}

void event_effect(edict_t *self, int effect, vec3_t origin)
{
	edict_t		*ent;

	if (effect == EVENTEFFECT_QUAKE) {
		ent = G_Spawn();
		VectorCopy(origin, ent->s.origin);
		ent->speed = 200;
		ent->count = 5;
		ent->classname = "temp_quake";

		ent->timestamp = level.time + ent->count;
		ent->nextthink = level.time + FRAMETIME;
		ent->last_move_time = 0;

		ent->svflags |= SVF_NOCLIENT;
		ent->think = event_earthquake_think;
		ent->noise_index = gi.soundindex("world/quake.wav");

		gi.linkentity(ent);
	}

	if (effect == EVENTEFFECT_QUAKE_SHOCK) {
		ent = G_Spawn();
		VectorCopy(origin, ent->s.origin);
		ent->speed = 300;
		ent->count = 3;
		ent->classname = "temp_quake";

		ent->timestamp = level.time + ent->count;
		ent->nextthink = level.time + 1.0;
		ent->last_move_time = 0;

		ent->svflags |= SVF_NOCLIENT;
		ent->think = event_earthquake_think;
		ent->noise_index = gi.soundindex("world/quake.wav");

		gi.linkentity(ent);

		gi.sound(ent, CHAN_VOICE, gi.soundindex("world/flyby1.wav"), 1, ATTN_NONE, 0);
	}

	if (effect == EVENTEFFECT_LAUGH) {
		gi.positioned_sound(origin, g_edicts, CHAN_AUTO, gi.soundindex("makron/laf1.wav"), 1, ATTN_NONE, 0);
	}
}

// Find monsters tagged with matching eventname
// otherwise find monsters tagged as event spawned without a specific event type
// within a short distance

edict_t *findeventmonster(edict_t *from, vec3_t org, char *eventname)
{
	vec3_t  eorg;
	int     j;

	if (!from)
		from = g_edicts;
	else
		from++;
	for (; from < &g_edicts[globals.num_edicts]; from++) {
		if (!from->inuse)
			continue;
		if (!(from->svflags & SVF_MONSTER))
			continue;
		if (!(from->spawnflags & SPAWNFLAG_EVENT))
			continue;
		// if we have matching event names don't bother with distance calculations
		if (eventname && from->monsterinfo.eventname) {
			if (Q_stricmp(from->monsterinfo.eventname, eventname) == 0)
				return from;
			else
				continue;
		}
		for (j = 0; j < 3; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j]) * 0.5);
		if (VectorLength(eorg) > from->wait)
			continue;
		return from;
	}

	return NULL;
}

// Something has fired an event
void doEvent(edict_t *self, int event)
{
	edict_t			*ent = NULL;
	event_point_t	*ep;
	vec3_t			d;
	char			*eventname = NULL;
	int				i = 0;

	for (ep = event_point_list; ep->name; ep++)
	{
		if (!(Q_stricmp(level.mapname, ep->mapname) == 0))
			continue;

		//gi.dprintf("Event at %s\n", vtos(ep->origin));

		// This works for events based on a static point like a small button or pickup
		// but not for moving entities like bosses or big trigger boxes, etc.
		if ((ep->event & event) && (ep->etype == EVENT_SPAWN)) {
			VectorSubtract(self->s.origin, ep->origin, d);
			if (VectorLength(d) < 128) {
				eventname = ep->name;
				if (ep->effect)
				{
					event_effect(self, ep->effect, ep->origin);
				}
				break; // if one exists use the first one
			}
		}
	}

	// if there is a door inhibit with the same name enable the door
	// we really should rewrite all this as a general event system rather than spawning event system

	if (eventname)
	{
		for (ep = event_point_list; ep->name; ep++, i++)
		{
			if (!(Q_stricmp(level.mapname, ep->mapname) == 0))
				continue;
			if (ep->etype == EVENT_INHIBIT_DOOR || ep->etype == EVENT_INHIBIT_DOOR_HEAD) {
				if ((Q_stricmp(ep->name, eventname) == 0))
					level.event_point_enabled[i] = 0;
			}
		}
	} else {
		//if (self->eventname)
			eventname = self->eventname;

			// won't fire if the player is passed as the event entity (unless player is tagged with an event)
	}

	// if we don't have an eventname from the events list try and inherit it from
	// the object calling the event

	while ((ent = findeventmonster(ent, self->s.origin, eventname)) != NULL) {
		if (ent) {
			if (ent->monsterinfo.eventtype & event)
			{
				ent->use(ent, NULL, NULL); // hopefully NULL doesn't break anything in this case some stuff isn't sanity checked
			}
		}
	}
}

// Rroff - event trigger box

void event_trigger_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (!other->client)
		return;

	doEvent(self, EVENTFLAG_TRIGGER);

	G_FreeEdict(self);
}

edict_t *event_trigger(vec3_t origin, vec3_t mins, vec3_t maxs, char *eventname)
{
	edict_t		*ent;

	ent = G_Spawn();

	ent->classname = "event_trigger";
	ent->eventname = eventname;

	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->touch = event_trigger_touch;

	VectorCopy(origin, ent->s.origin);
	VectorCopy(mins, ent->mins);
	VectorCopy(maxs, ent->maxs);

	gi.linkentity(ent);

	return ent;
}