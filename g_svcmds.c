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


void    Svcmd_Test_f(void)
{
    gi.cprintf(NULL, PRINT_HIGH, "Svcmd_Test_f()\n");
}

/*
==============================================================================

PACKET FILTERING


You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and any unspecified digits will match any value, so you can specify an entire class C network with "addip 192.246.40".

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

writeip
Dumps "addip <ip>" commands to listip.cfg so it can be execed at a later date.  The filter lists are not saved and restored by default, because I beleive it would cause too much confusion.

filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.


==============================================================================
*/

typedef struct {
    unsigned    mask;
    unsigned    compare;
} ipfilter_t;

#define MAX_IPFILTERS   1024

ipfilter_t  ipfilters[MAX_IPFILTERS];
int         numipfilters;

/*
=================
StringToFilter
=================
*/
static qboolean StringToFilter(char *s, ipfilter_t *f)
{
    char    num[128];
    int     i, j;
    byte    b[4];
    byte    m[4];

    for (i = 0 ; i < 4 ; i++) {
        b[i] = 0;
        m[i] = 0;
    }

    for (i = 0 ; i < 4 ; i++) {
        if (*s < '0' || *s > '9') {
            gi.cprintf(NULL, PRINT_HIGH, "Bad filter address: %s\n", s);
            return qfalse;
        }

        j = 0;
        while (*s >= '0' && *s <= '9') {
            num[j++] = *s++;
        }
        num[j] = 0;
        b[i] = atoi(num);
        if (b[i] != 0)
            m[i] = 255;

        if (!*s)
            break;
        s++;
    }

    f->mask = *(unsigned *)m;
    f->compare = *(unsigned *)b;

    return qtrue;
}

/*
=================
SV_FilterPacket
=================
*/
qboolean SV_FilterPacket(char *from)
{
    int     i;
    unsigned    in;
    byte m[4];
    char *p;

    i = 0;
    p = from;
    while (*p && i < 4) {
        m[i] = 0;
        while (*p >= '0' && *p <= '9') {
            m[i] = m[i] * 10 + (*p - '0');
            p++;
        }
        if (!*p || *p == ':')
            break;
        i++, p++;
    }

    in = *(unsigned *)m;

    for (i = 0 ; i < numipfilters ; i++)
        if ((in & ipfilters[i].mask) == ipfilters[i].compare)
            return (int)filterban->value;

    return (int)!filterban->value;
}


/*
=================
SV_AddIP_f
=================
*/
void SVCmd_AddIP_f(void)
{
    int     i;

    if (gi.argc() < 3) {
        gi.cprintf(NULL, PRINT_HIGH, "Usage:  addip <ip-mask>\n");
        return;
    }

    for (i = 0 ; i < numipfilters ; i++)
        if (ipfilters[i].compare == 0xffffffff)
            break;      // free spot
    if (i == numipfilters) {
        if (numipfilters == MAX_IPFILTERS) {
            gi.cprintf(NULL, PRINT_HIGH, "IP filter list is full\n");
            return;
        }
        numipfilters++;
    }

    if (!StringToFilter(gi.argv(2), &ipfilters[i]))
        ipfilters[i].compare = 0xffffffff;
}

/*
=================
SV_RemoveIP_f
=================
*/
void SVCmd_RemoveIP_f(void)
{
    ipfilter_t  f;
    int         i, j;

    if (gi.argc() < 3) {
        gi.cprintf(NULL, PRINT_HIGH, "Usage:  sv removeip <ip-mask>\n");
        return;
    }

    if (!StringToFilter(gi.argv(2), &f))
        return;

    for (i = 0 ; i < numipfilters ; i++)
        if (ipfilters[i].mask == f.mask
            && ipfilters[i].compare == f.compare) {
            for (j = i + 1 ; j < numipfilters ; j++)
                ipfilters[j - 1] = ipfilters[j];
            numipfilters--;
            gi.cprintf(NULL, PRINT_HIGH, "Removed.\n");
            return;
        }
    gi.cprintf(NULL, PRINT_HIGH, "Didn't find %s.\n", gi.argv(2));
}

/*
=================
SV_ListIP_f
=================
*/
void SVCmd_ListIP_f(void)
{
    int     i;
    byte    b[4];

    gi.cprintf(NULL, PRINT_HIGH, "Filter list:\n");
    for (i = 0 ; i < numipfilters ; i++) {
        *(unsigned *)b = ipfilters[i].compare;
        gi.cprintf(NULL, PRINT_HIGH, "%3i.%3i.%3i.%3i\n", b[0], b[1], b[2], b[3]);
    }
}

/*
=================
SV_WriteIP_f
=================
*/
void SVCmd_WriteIP_f(void)
{
    FILE    *f;
    char    name[MAX_OSPATH];
    size_t  len;
    byte    b[4];
    int     i;
    cvar_t  *game;

    game = gi.cvar("game", "", 0);

    if (!*game->string)
        len = Q_snprintf(name, sizeof(name), "%s/listip.cfg", GAMEVERSION);
    else
        len = Q_snprintf(name, sizeof(name), "%s/listip.cfg", game->string);

    if (len >= sizeof(name)) {
        gi.cprintf(NULL, PRINT_HIGH, "File name too long\n");
        return;
    }

    gi.cprintf(NULL, PRINT_HIGH, "Writing %s.\n", name);

    f = fopen(name, "wb");
    if (!f) {
        gi.cprintf(NULL, PRINT_HIGH, "Couldn't open %s\n", name);
        return;
    }

    fprintf(f, "set filterban %d\n", (int)filterban->value);

    for (i = 0 ; i < numipfilters ; i++) {
        *(unsigned *)b = ipfilters[i].compare;
        fprintf(f, "sv addip %i.%i.%i.%i\n", b[0], b[1], b[2], b[3]);
    }

    fclose(f);
}

// Rroff - these commands do little in the way of sanity checking
// and assume whoever calls them knows what they are doing
// only 1 computer should be created per game map
// and spawn points should not exceed LK_MAX_CUSTOM

// first 16 spots reserved
// lastused for first 16 spots will be boss spawn type

void SVCmd_HordeCreateSpawn_f(void)
{
	//char			*mapname;
	float			x, y, z, a;
	spawn_point_t	*n;
	int				i;

	//mapname = gi.argv(2);
	x = atof(gi.argv(2));
	y = atof(gi.argv(3));
	z = atof(gi.argv(4));
	a = atof(gi.argv(5));

	// <mapname> <X> <Y> <Z> <YAW>
	//gi.dprintf("Mapname %s %f %f %f %f\n", mapname, x, y, z, a);

	n = &custom_spawn_list[17];

	for (i = 17; i < LK_MAX_CUSTOM; i++, n++)
	{
		if (!(n->child))
		{
			n->child = qtrue;
			//strcpy(n->mapname, level.mapname);
			n->mapname = level.mapname;
			n->origin[0] = x;
			n->origin[1] = y;
			n->origin[2] = z;
			n->angles[PITCH] = 0;
			n->angles[YAW] = a;
			n->angles[ROLL] = 0;
			n->lastused = 0;
			return;
		}
	}

	gi.dprintf("Warning: Ran out of horde spawn points\n");
}

// Angles aren't used here
// TYPE:
// 1, 2 - Hornet boss positions
// 9 - Super tank

void SVCmd_HordeCreateBossSpawn_f(void)
{
	//char			*mapname;
	float			x, y, z, a;
	spawn_point_t	*n;
	int				i;

	//mapname = gi.argv(2);
	x = atof(gi.argv(2));
	y = atof(gi.argv(3));
	z = atof(gi.argv(4));
	a = atof(gi.argv(5));

	// <mapname> <X> <Y> <Z> <TYPE>
	//gi.dprintf("Mapname %s %f %f %f %f\n", mapname, x, y, z, a);

	n = &custom_spawn_list[1];

	for (i = 1; i < 16; i++, n++)
	{
		if (!(n->child))
		{
			n->child = qtrue;
			//strcpy(n->mapname, level.mapname);
			n->mapname = level.mapname;
			n->origin[0] = x;
			n->origin[1] = y;
			n->origin[2] = z;
			n->angles[PITCH] = 0;
			n->angles[YAW] = 0;
			n->angles[ROLL] = 0;
			n->lastused = a;
			return;
		}
	}

	gi.dprintf("Warning: Ran out of horde boss spawn points\n");
}

// X Y Z A

void SVCmd_HordeCreateComputer_f(void)
{
	float			x, y, z, a;
	vec3_t			start, spawn_angles;

	x = atof(gi.argv(2));
	y = atof(gi.argv(3));
	z = atof(gi.argv(4));
	a = atof(gi.argv(5));

	VectorSet(start, x, y, z);
	VectorSet(spawn_angles, 0, a, 0);
	cpoint_spawn(start, spawn_angles, "lockdown$custom", LK_CUSTOM);

}

void SVCmd_CreateCoopSpot_f(void)
{
	float			x, y, z, a;
	edict_t			*spot;

	x = atof(gi.argv(2));
	y = atof(gi.argv(3));
	z = atof(gi.argv(4));
	a = atof(gi.argv(5));

	spot = G_Spawn();
	spot->classname = "info_player_coop";
	spot->s.origin[0] = x;
	spot->s.origin[1] = y;
	spot->s.origin[2] = z;
	//spot->targetname = "sewer64";
	spot->s.angles[1] = a;

	// need to test this
	if (gi.argc() >= 7)
	{
		spot->targetname = gi.argv(6);
	}
}

/*
=================
ServerCommand

ServerCommand will be called when an "sv" command is issued.
The game can issue gi.argc() / gi.argv() commands to get the rest
of the parameters
=================
*/
void    ServerCommand(void)
{
    char    *cmd;

    cmd = gi.argv(1);
    if (Q_stricmp(cmd, "test") == 0)
        Svcmd_Test_f();
    else if (Q_stricmp(cmd, "addip") == 0)
        SVCmd_AddIP_f();
    else if (Q_stricmp(cmd, "removeip") == 0)
        SVCmd_RemoveIP_f();
    else if (Q_stricmp(cmd, "listip") == 0)
        SVCmd_ListIP_f();
    else if (Q_stricmp(cmd, "writeip") == 0)
        SVCmd_WriteIP_f();
	else if (Q_stricmp(cmd, "hordecreatespawn") == 0)
		SVCmd_HordeCreateSpawn_f();
	else if (Q_stricmp(cmd, "hordecreatebossspawn") == 0)
		SVCmd_HordeCreateBossSpawn_f();
	else if (Q_stricmp(cmd, "hordecreatecomputer") == 0)
		SVCmd_HordeCreateComputer_f();
	else if (Q_stricmp(cmd, "createcoopspot") == 0)
		SVCmd_CreateCoopSpot_f();
    else
        gi.cprintf(NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd);
}

