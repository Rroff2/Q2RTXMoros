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



/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission(edict_t *ent)
{
    if (deathmatch->value || coop->value)
        ent->client->showscores = qtrue;
    VectorCopy(level.intermission_origin, ent->s.origin);
    ent->client->ps.pmove.origin[0] = level.intermission_origin[0] * 8;
    ent->client->ps.pmove.origin[1] = level.intermission_origin[1] * 8;
    ent->client->ps.pmove.origin[2] = level.intermission_origin[2] * 8;
    VectorCopy(level.intermission_angle, ent->client->ps.viewangles);
    ent->client->ps.pmove.pm_type = PM_FREEZE;
    ent->client->ps.gunindex = 0;
    ent->client->ps.blend[3] = 0;
    ent->client->ps.rdflags &= ~RDF_UNDERWATER;

    // clean up powerup info
    ent->client->quad_framenum = 0;
    ent->client->invincible_framenum = 0;
    ent->client->breather_framenum = 0;
    ent->client->enviro_framenum = 0;
	ent->client->berserk_framenum = 0;
	ent->client->reaper_framenum = 0;
	ent->client->bouncer_framenum = 0;
	ent->client->taunt_framenum = 0;
    ent->client->grenade_blew_up = qfalse;
    ent->client->grenade_time = 0;

    ent->viewheight = 0;
    ent->s.modelindex = 0;
    ent->s.modelindex2 = 0;
    ent->s.modelindex3 = 0;
    ent->s.modelindex = 0;
    ent->s.effects = 0;
    ent->s.sound = 0;
    ent->solid = SOLID_NOT;

    // add the layout

    if (deathmatch->value || coop->value) {
        DeathmatchScoreboardMessage(ent, NULL);
        gi.unicast(ent, qtrue);
    }

}

void BeginIntermission(edict_t *targ)
{
    int     i, n;
    edict_t *ent, *client;

    if (level.intermissiontime)
        return;     // already activated

    game.autosaved = qfalse;

    // respawn any dead clients
    for (i = 0 ; i < maxclients->value ; i++) {
        client = g_edicts + 1 + i;
        if (!client->inuse)
            continue;
        if (client->health <= 0)
            respawn(client);
    }

    level.intermissiontime = level.time;
    level.changemap = targ->map;

    if (strstr(level.changemap, "*")) {
        if (coop->value) {
            for (i = 0 ; i < maxclients->value ; i++) {
                client = g_edicts + 1 + i;
                if (!client->inuse)
                    continue;
                // strip players of all keys between units
                for (n = 0; n < MAX_ITEMS; n++) {
                    if (itemlist[n].flags & IT_KEY)
                        client->client->pers.inventory[n] = 0;
                }
            }
        }
    } else {
        if (!deathmatch->value) {
			if (!(level.lockdown_ent && ((level.lockdown_ent->moded == LK_FOREVER) || (level.lockdown_ent->moded == LK_CUSTOM))))
			{
				level.exitintermission = 1;     // go immediately to the next level
				return;
			}
        }
    }

    level.exitintermission = 0;

    // find an intermission spot
    ent = G_Find(NULL, FOFS(classname), "info_player_intermission");
    if (!ent) {
        // the map creator forgot to put in an intermission point...
        ent = G_Find(NULL, FOFS(classname), "info_player_start");
        if (!ent)
            ent = G_Find(NULL, FOFS(classname), "info_player_deathmatch");
    } else {
        // chose one of four spots
        i = rand() & 3;
        while (i--) {
            ent = G_Find(ent, FOFS(classname), "info_player_intermission");
            if (!ent)   // wrap around the list
                ent = G_Find(ent, FOFS(classname), "info_player_intermission");
        }
    }

    VectorCopy(ent->s.origin, level.intermission_origin);
    VectorCopy(ent->s.angles, level.intermission_angle);

    // move all clients to the intermission point
    for (i = 0 ; i < maxclients->value ; i++) {
        client = g_edicts + 1 + i;
        if (!client->inuse)
            continue;
        MoveClientToIntermission(client);
    }
}


/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage(edict_t *ent, edict_t *killer)
{
    char    entry[1024];
    char    string[1400];
    int     stringlength;
    int     i, j, k;
    int     sorted[MAX_CLIENTS];
    int     sortedscores[MAX_CLIENTS];
    int     score, total;
    int     x, y;
    gclient_t   *cl;
    edict_t     *cl_ent;
    char    *tag;

    // sort the clients by score
    total = 0;
    for (i = 0 ; i < game.maxclients ; i++) {
        cl_ent = g_edicts + 1 + i;
        if (!cl_ent->inuse || game.clients[i].resp.spectator)
            continue;
        score = game.clients[i].resp.score;
        for (j = 0 ; j < total ; j++) {
            if (score > sortedscores[j])
                break;
        }
        for (k = total ; k > j ; k--) {
            sorted[k] = sorted[k - 1];
            sortedscores[k] = sortedscores[k - 1];
        }
        sorted[j] = i;
        sortedscores[j] = score;
        total++;
    }

    // print level name and exit rules
    string[0] = 0;

    stringlength = strlen(string);

    // add the clients in sorted order
    if (total > 12)
        total = 12;

    for (i = 0 ; i < total ; i++) {
        cl = &game.clients[sorted[i]];
        cl_ent = g_edicts + 1 + sorted[i];

        x = (i >= 6) ? 160 : 0;
        y = 32 + 32 * (i % 6);

        // add a dogtag
        if (cl_ent == ent)
            tag = "tag1";
        else if (cl_ent == killer)
            tag = "tag2";
        else
            tag = NULL;
        if (tag) {
            Q_snprintf(entry, sizeof(entry),
                       "xv %i yv %i picn %s ", x + 32, y, tag);
            j = strlen(entry);
            if (stringlength + j > 1024)
                break;
            strcpy(string + stringlength, entry);
            stringlength += j;
        }

        // send the layout
        Q_snprintf(entry, sizeof(entry),
                   "client %i %i %i %i %i %i ",
                   x, y, sorted[i], cl->resp.score, cl->ping, (level.framenum - cl->resp.enterframe) / 600);
        j = strlen(entry);
        if (stringlength + j > 1024)
            break;
        strcpy(string + stringlength, entry);
        stringlength += j;
    }

    gi.WriteByte(svc_layout);
    gi.WriteString(string);
}


/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
==================
*/
void DeathmatchScoreboard(edict_t *ent)
{
    DeathmatchScoreboardMessage(ent, ent->enemy);
    gi.unicast(ent, qtrue);
}


/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/
void Cmd_Score_f(edict_t *ent)
{
    ent->client->showinventory = qfalse;
    ent->client->showhelp = qfalse;

    if (!deathmatch->value && !coop->value)
        return;

    if (ent->client->showscores) {
        ent->client->showscores = qfalse;
        return;
    }

    ent->client->showscores = qtrue;
    DeathmatchScoreboard(ent);
}


/*
==================
HelpComputer

Draw help computer.
==================
*/
void HelpComputer(edict_t *ent)
{
    char    string[1024];
    char    *sk;

    if (skill->value == 0)
        sk = "easy";
    else if (skill->value == 1)
        sk = "medium";
    else if (skill->value == 2)
        sk = "hard";
    else
        sk = "hard+";

	if (level.lockdown_ent && ((level.lockdown_ent->moded == LK_FOREVER) || (level.lockdown_ent->moded == LK_CUSTOM)))
	{
		Q_snprintf(string, sizeof(string),
			"xv 32 yv 8 picn help "         // background
			"xv 202 yv 12 string2 \"%s\" "      // skill
			"xv 0 yv 24 cstring2 \"%s\" "       // level name
			"xv 0 yv 54 cstring2 \"%s\" "       // help 1
			"xv 0 yv 110 cstring2 \"%s\" "      // help 2
			"xv 50 yv 164 string2 \" kills     goals      wave\" "
			"xv 50 yv 172 string2 \"%3i/%3i     %i/%i        %i\" ",
			sk,
			level.level_name,
			game.helpmessage1,
			game.helpmessage2,
			level.killed_monsters, level.total_monsters,
			level.found_goals, level.total_goals,
			(10-level.lockdown_ent->count));
	}
	else
	{
		// send the layout
		Q_snprintf(string, sizeof(string),
			"xv 32 yv 8 picn help "         // background
			"xv 202 yv 12 string2 \"%s\" "      // skill
			"xv 0 yv 24 cstring2 \"%s\" "       // level name
			"xv 0 yv 54 cstring2 \"%s\" "       // help 1
			"xv 0 yv 110 cstring2 \"%s\" "      // help 2
			"xv 50 yv 164 string2 \" kills     goals    secrets\" "
			"xv 50 yv 172 string2 \"%3i/%3i     %i/%i       %i/%i\" ",
			sk,
			level.level_name,
			game.helpmessage1,
			game.helpmessage2,
			level.killed_monsters, level.total_monsters,
			level.found_goals, level.total_goals,
			level.found_secrets, level.total_secrets);
	}

    gi.WriteByte(svc_layout);
    gi.WriteString(string);
    gi.unicast(ent, qtrue);
}


/*
==================
Cmd_Help_f

Display the current help message
==================
*/
void Cmd_Help_f(edict_t *ent)
{
    // this is for backwards compatability
    if (deathmatch->value) {
        Cmd_Score_f(ent);
        return;
    }

    ent->client->showinventory = qfalse;
    ent->client->showscores = qfalse;

    if (ent->client->showhelp && (ent->client->pers.game_helpchanged == game.helpchanged)) {
        ent->client->showhelp = qfalse;
        return;
    }

    ent->client->showhelp = qtrue;
    ent->client->pers.helpchanged = 0;
    HelpComputer(ent);
}

void hud(edict_t *ent)
{
	char	entry[1024];
	char	string[1400];
	int		entryLen;
	int		stringLen;
	int		i;
	int		py, hx;
	int		hlen;

	char	picfile[MAX_OSPATH];
	char	health[12];
	char	*ping;

	int		x, y;
	float	dist, angle, angle2;
	float	x1, y1;

	int		radar_range = 1024;
	int		radar_size = 124; // 124 for inside border, 128 for edge
	int		radar_clip = 992; // range/size*size-4
	int		radar_rangex2 = 2048;
	// border clipping radar sigs = distance/128*124
	// 1984, 4096, 128 original values

	gclient_t	*cl;
	edict_t		*cl_ent;

	string[0] = 0;

	// ==============================================

	// if we have more than 4 clients in coop (some modified versions support 8)
	// need to add an extra frame for the next 4

	Q_snprintf(entry, sizeof(entry),
		"xl 0 yv 32 picn p1 "
	);

	entryLen = strlen(entry);
	stringLen = strlen(string);
	if (!(stringLen + entryLen > 1400))
		strcpy(string + stringLen, entry);

	// not tested as 4 current limit
	// alignment might not match up on second frame
	if (game.maxclients > 4)
	{
		Q_snprintf(entry, sizeof(entry),
			"xl 0 yv 180 picn p1 "
		);

		entryLen = strlen(entry);
		stringLen = strlen(string);
		if (!(stringLen + entryLen > 1400))
			strcpy(string + stringLen, entry);
	}

	py = 36; // 36, 16, 36

	if (ent->client->resp.spectator)
	{
		if (ent->client->chase_target && ent->client->chase_target->inuse && ent->client->chase_target->health > 0)
		{
			if ((ent->client->chase_target->max_health == 0) || (ent->client->chase_target->health == 0))
			{
				hx = -64;
			}
			else {
				if (ent->client->chase_target->health > ent->client->chase_target->max_health)
				{
					hx = 36;
				}
				else {
					hx = 100 - ((100 / (float)ent->client->chase_target->max_health) * ent->client->chase_target->health);
					hx = 36 - hx;
				}
			}

			Q_snprintf(entry, sizeof(entry),
				"xl 36 yv %i string \"%s\" "
				"xl %i yv %i picn s2 "
				"xl 2 yv %i picn sp ",
				py, ent->client->chase_target->descname, hx, py + 16, py
			);

			entryLen = strlen(entry);
			stringLen = strlen(string);
			if (!(stringLen + entryLen > 1400))
				strcpy(string + stringLen, entry);

			py += 36;
		}
	}
	else {

		for (i = 0; i < game.maxclients; i++)
		{
			cl = &game.clients[i];
			cl_ent = g_edicts + 1 + i;

			if (!cl_ent->inuse)
				continue;

			*picfile = 0;

			// should check if player picture exists first?
			// if this is running on the server though that doesn't help clients

			if (atoi(Info_ValueForKey(cl->pers.userinfo, "cl_hudfix")) != 0)
			{
				Q_snprintf(picfile, sizeof(picfile), "/players/male/grunt_i.pcx");
			}
			else {
				Q_snprintf(picfile, sizeof(picfile), "/players/%s_i.pcx", Info_ValueForKey(cl->pers.userinfo, "skin"));
			}

			if ((cl_ent->max_health == 0) || (cl_ent->health == 0))
			{
				hx = -64;
			}
			else {
				if (cl_ent->health > cl_ent->max_health)
				{
					hx = 36;
				}
				else {
					hx = 100 - ((100 / (float)cl_ent->max_health) * cl_ent->health);
					hx = 36 - hx;
				}
			}

			// might need to stop the icon flickering constantly if a client
			// is going back and forth over a limit

			ping = "n4";

			if (cl->ping < 999)
				ping = "n3";

			if (cl->ping < 150)
				ping = "n2";

			if (cl->ping < 65)
				ping = "n1";

			Q_snprintf(entry, sizeof(entry),
				"xl 36 yv %i string \"%s\" "
				"xl %i yv %i picn s2 "
				"xl 2 yv %i picn \"%s\" "
				"xl 18 yv %i picn \"%s\" ",
				py, cl->pers.netname, hx, py + 16, py, picfile, py + 16, ping
			);

			entryLen = strlen(entry);
			stringLen = strlen(string);
			if (!(stringLen + entryLen > 1400))
				strcpy(string + stringLen, entry);

			py += 36;
		}
	}

	Q_snprintf(entry, sizeof(entry),
		"xl 0 yv 32 picn pp "
	);

	entryLen = strlen(entry);
	stringLen = strlen(string);
	if (!(stringLen + entryLen > 1400))
		strcpy(string + stringLen, entry);

	if (game.maxclients > 4)
	{

		Q_snprintf(entry, sizeof(entry),
			"xl 0 yv 180 picn pp "
		);

		entryLen = strlen(entry);
		stringLen = strlen(string);
		if (!(stringLen + entryLen > 1400))
			strcpy(string + stringLen, entry);
	}

	// =======================================================================


	/*if (ent->client->pickedEnt)
	{
		if (ent->client->pickedEnt->max_health > 0)
		{
			hlen = (int)(((float)10 / ent->client->pickedEnt->max_health)*ent->client->pickedEnt->health);
			if (hlen < 0)
				hlen = 0;
			if ((ent->client->pickedEnt->health > 0) && (hlen == 0))
				hlen = 1;
			if (hlen > 10)
				hlen = 10;
			*health = 0;
			memset(health, '\x8B', hlen);
			health[hlen] = 0;

			if (ent->client->pickedEnt->svflags & (SVF_MONSTER | SVF_DEADMONSTER))
			{
				Com_sprintf(entry, sizeof(entry),
					"xv 96 yt 4 picn tag2 "
					"xv 100 yt 8 picn k_comhead "
					"xv 126 yt 8 string \"%s\" "
					"xv 126 yt 16 string \"%s\" ", ent->client->pickedEnt->descname, health); // 22
				entryLen = strlen(entry);
				stringLen = strlen(string);
				if (!(stringLen + entryLen > 1400))
					strcpy(string + stringLen, entry);
			}

			if (ent->client->pickedEnt->client)
			{
				Com_sprintf(entry, sizeof(entry),
					"xv 96 yt 4 picn tag2 "
					"xv 96 yt 4 picn /players/male/grunt_i.pcx "
					"xv 126 yt 8 string \"%s\" "
					"xv 126 yt 16 string \"%s\" ", ent->client->pickedEnt->client->pers.netname, health); // 22
				entryLen = strlen(entry);
				stringLen = strlen(string);
				if (!(stringLen + entryLen > 1400))
					strcpy(string + stringLen, entry);
			}
		}
	}*/

	// RADAR - experimental
	// change this to only show when not many monsters left
	// when monster count is low should they stop moving to new positions?

	if (
		(level.lockdown_ent && ((level.lockdown_ent->moded == LK_FOREVER) || (level.lockdown_ent->moded == LK_CUSTOM))) &&
		(
		((level.total_monsters - level.killed_monsters) > 0 && (level.total_monsters - level.killed_monsters) < 4)
			||
			(g_coophud->integer > 2)
			)
		)
	{

		Q_snprintf(entry, sizeof(entry), "xl 4 yv -132 picn r "); // -200
		entryLen = strlen(entry);
		stringLen = strlen(string);
		if (!(stringLen + entryLen > 1024))
			strcpy(string + stringLen, entry);

		angle = ent->s.angles[YAW];

		angle = 360 - angle; // reflect coord system
		angle += 90; // adjust for clockwise offset
		angle = angle / 360;
		angle -= (int)angle;
		angle *= 360;

		cl_ent = &g_edicts[(int)maxclients->value + 1];
		for (i = maxclients->value + 1; i < globals.num_edicts; i++, cl_ent++)
		{
			if (!cl_ent->inuse)
				continue;

			if (!(cl_ent->svflags & SVF_MONSTER))
				continue;

			if (cl_ent->monsterinfo.aiflags & AI_GOOD_GUY)
				continue;

			if (cl_ent->svflags & SVF_NOCLIENT)
				continue;

			if (cl_ent->health <= 0)
				continue;

			x1 = (cl_ent->s.origin[0] - ent->s.origin[0]);
			y1 = (cl_ent->s.origin[1] - ent->s.origin[1]);

			dist = sqrt((x1*x1) + (y1*y1));

			if (dist > radar_clip)
			{
				dist = radar_clip;
			}

			angle2 = atan2(y1, x1) * 180 / M_PI;

			if (angle2 < 0)
				angle2 += 360;
			angle2 = 360 - angle2; // reflect coord system

			angle2 -= angle;
			if (angle2 < 0)
				angle2 += 360;

			angle2 += 90; // adjust for clockwise offset
			angle2 = angle2 / 360;
			angle2 -= (int)angle2;
			angle2 *= 360;

			angle2 *= (M_PI / 180);

			dist = dist / radar_rangex2 * radar_size;
			x1 = x1 / radar_rangex2 * radar_size;
			y1 = y1 / radar_rangex2 * radar_size;

			x1 = sin(angle2)*dist;
			y1 = cos(angle2)*dist;

			x = 69 + x1;
			//y = -137 - y1;
			y = -69 - y1;

			Q_snprintf(entry, sizeof(entry), "xl %i yv %i picn p ", x - 4, y - 4);

			entryLen = strlen(entry);
			stringLen = strlen(string);
			if (!(stringLen + entryLen > 1024))
				strcpy(string + stringLen, entry);
		}
	}

	//

	gi.WriteByte(svc_layout);
	gi.WriteString(string);
	gi.unicast(ent, qtrue);
}


//=======================================================================

/*
===============
G_SetStats
===============
*/
void G_SetStats(edict_t *ent)
{
    gitem_t     *item;
    int         index, cells;
    int         power_armor_type;
	int			horde_timer, toxic_timer, horde_timer2;
	qboolean	active_pu;

    //
    // health
    //
    ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
    ent->client->ps.stats[STAT_HEALTH] = ent->health;

    //
    // ammo
    //
	if (!ent->client->ammo_index /* || !ent->client->pers.inventory[ent->client->ammo_index] */) {
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;

		// pers.weapon not valid if player is dead
		if ((ent->health > 0) && (ent->client->pers.mods & MU_BLASTER) && (ent->client->pers.weapon->weapmodel == WEAP_BLASTER))
		{
			index = ITEM_INDEX(FindItem("cells"));
			item = &itemlist[index];
			if (item) {
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex(item->icon);
				ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[index];
			}
		}

		//if ((level.framenum & 8) && (ent->client->pers.mods & MU_ACTIVEDEF))
		if ((ent->health > 0) && (ent->client->pers.mods & MU_ACTIVEDEF) && (level.time < ent->client->ad_last + 0.5))
		{
			index = ITEM_INDEX(FindItem("cells"));
			item = &itemlist[index];
			if (item) {
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex(item->icon);
				ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[index];
			}
		}
	}
	else {
		item = &itemlist[ent->client->ammo_index];
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex(item->icon);
		ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
	}

    //
    // armor
    //

	// Rroff - cyborg power armor never deactivates
	power_armor_type = PowerArmorType(ent);

	if (power_armor_type) {
		cells = ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))];
		if (!(ent->client->pers.player_class & PC_CYBORG))
		{
			if (cells == 0) {
				// ran out of cells for power armor
				ent->flags &= ~FL_POWER_ARMOR;
				gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
				power_armor_type = 0;;
			}
		}
	}

    index = ArmorIndex(ent);
    if (power_armor_type && (!index || (level.framenum & 8))) {
        // flash between power armor and other armor icon
        ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex("i_powershield");
        ent->client->ps.stats[STAT_ARMOR] = cells;
    } else if (index) {
        item = GetItemByIndex(index);
        ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex(item->icon);
        ent->client->ps.stats[STAT_ARMOR] = ent->client->pers.inventory[index];
    } else {
        ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
        ent->client->ps.stats[STAT_ARMOR] = 0;
    }

    //
    // pickup message
    //
    if (level.time > ent->client->pickup_msg_time) {
        ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
        ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
    }

    //
    // timers
    //
	active_pu = qtrue;
    if (ent->client->quad_framenum > level.framenum) {
        ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_quad");
        ent->client->ps.stats[STAT_TIMER] = (ent->client->quad_framenum - level.framenum) / 10;
    } else if (ent->client->invincible_framenum > level.framenum) {
        ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_invulnerability");
        ent->client->ps.stats[STAT_TIMER] = (ent->client->invincible_framenum - level.framenum) / 10;
    } else if (ent->client->enviro_framenum > level.framenum) {
        ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_envirosuit");
        ent->client->ps.stats[STAT_TIMER] = (ent->client->enviro_framenum - level.framenum) / 10;
    } else if (ent->client->breather_framenum > level.framenum) {
        ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_rebreather");
        ent->client->ps.stats[STAT_TIMER] = (ent->client->breather_framenum - level.framenum) / 10;
	} else if (ent->client->berserk_framenum > level.framenum) {
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_invulnerability");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->berserk_framenum - level.framenum) / 10;
	} else if (ent->client->reaper_framenum > level.framenum) {
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_invulnerability"); // need icons for these
		ent->client->ps.stats[STAT_TIMER] = (ent->client->reaper_framenum - level.framenum) / 10;
	} else if (ent->client->bouncer_framenum > level.framenum) {
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("w_chaingun"); // need icons for these
		ent->client->ps.stats[STAT_TIMER] = (ent->client->bouncer_framenum - level.framenum) / 10;
	} else if (ent->client->taunt_framenum > level.framenum) {
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("i_bodyarmor"); // need icons for these
		ent->client->ps.stats[STAT_TIMER] = (ent->client->taunt_framenum - level.framenum) / 10;
	} else {
        ent->client->ps.stats[STAT_TIMER_ICON] = 0;
        ent->client->ps.stats[STAT_TIMER] = 0;
		active_pu = qfalse;
    }

	// Rroff - if both regen and silencer are in effect then silencer will be overwritten
	// same for solar cell recharge

	if ((!active_pu || (level.framenum & 8)) && (ent->turret_ammo > 0))
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("a_bullets");
		ent->client->ps.stats[STAT_TIMER] = ent->turret_ammo;
	}

	if ((!active_pu || (level.framenum & 8)) && (ent->client->pers.player_class & PC_MEDIC))
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("a_cells");
		ent->client->ps.stats[STAT_TIMER] = ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))];
	}

	if ((!active_pu || (level.framenum & 8)) && ((ent->client->pers.mods & MU_SOLAR) || (ent->client->pers.player_class & PC_CYBORG)))
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("a_cells");
		ent->client->ps.stats[STAT_TIMER] = ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))];
	}

	if ((!active_pu || (level.framenum & 8)) && (ent->client->silencer_shots))
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_silencer");
		ent->client->ps.stats[STAT_TIMER] = ent->client->silencer_shots;
	}

	if ((!active_pu || (level.framenum & 8)) && (ent->client->pers.mods & MU_REGEN))
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex("p_megahealth");
		ent->client->ps.stats[STAT_TIMER] = ent->client->pers.pool_health;
	}

	// Rroff horde mode timer

	if ((level.lockdown_ent) && ((level.lockdown_ent->turret_ammo > 0) || (level.lockdown_ent->flags & FL_TOXIC)))
	{
		ent->client->ps.stats[STAT_HORDE_ICON] = gi.imageindex("i_airstrike");

		horde_timer = (int)(level.lockdown_ent->last_time - level.time);
		horde_timer2 = (int)(level.lockdown_ent->random - level.time);

		if ((horde_timer > 0) && (level.lockdown_ent->gib_health > 0))
		{
			if ((horde_timer2 > 0) && (level.framenum & 8))
			{
				ent->client->ps.stats[STAT_HORDE_TIMER] = horde_timer2;
			}
			else
			{
				ent->client->ps.stats[STAT_HORDE_TIMER] = horde_timer;
			}
		}
		else
		{
			if (horde_timer2 > 0)
				ent->client->ps.stats[STAT_HORDE_TIMER] = horde_timer2;
			else
				ent->client->ps.stats[STAT_HORDE_TIMER] = 0;
		}

		if (level.lockdown_ent->flags & FL_TOXIC)
		{
			toxic_timer = level.lockdown_ent->wait - level.time;
			if (toxic_timer < 0)
				toxic_timer = 0;

			ent->client->ps.stats[STAT_TOXIC_ICON] = gi.imageindex("a_blaster");
			ent->client->ps.stats[STAT_TOXIC_TIMER] = toxic_timer;
		}
	} else {
		ent->client->ps.stats[STAT_HORDE_TIMER] = 0;
		ent->client->ps.stats[STAT_TOXIC_TIMER] = 0;
	}

    //
    // selected item
    //
    if (ent->client->pers.selected_item == -1)
        ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
    else
        ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex(itemlist[ent->client->pers.selected_item].icon);

    ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;

    //
    // layouts
    //
    ent->client->ps.stats[STAT_LAYOUTS] = 0;

    if (deathmatch->value) {
        if (ent->client->pers.health <= 0 || level.intermissiontime
            || ent->client->showscores)
            ent->client->ps.stats[STAT_LAYOUTS] |= 1;
        if (ent->client->showinventory && ent->client->pers.health > 0)
            ent->client->ps.stats[STAT_LAYOUTS] |= 2;
    } else {
        if (ent->client->showscores || ent->client->showhelp || doHUD(ent))
            ent->client->ps.stats[STAT_LAYOUTS] |= 1;
        if (ent->client->showinventory && ent->client->pers.health > 0)
            ent->client->ps.stats[STAT_LAYOUTS] |= 2;
    }

    //
    // frags
    //
    ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

    //
    // help icon / current weapon if not shown
    //
    if (ent->client->pers.helpchanged && (level.framenum & 8))
        ent->client->ps.stats[STAT_HELPICON] = gi.imageindex("i_help");
    else if ((ent->client->pers.hand == CENTER_HANDED || ent->client->ps.fov > 91)
             && ent->client->pers.weapon)
        ent->client->ps.stats[STAT_HELPICON] = gi.imageindex(ent->client->pers.weapon->icon);
    else
        ent->client->ps.stats[STAT_HELPICON] = 0;

    ent->client->ps.stats[STAT_SPECTATOR] = 0;
}

/*
===============
G_CheckChaseStats
===============
*/
void G_CheckChaseStats(edict_t *ent)
{
    int i;
    gclient_t *cl;

    for (i = 1; i <= maxclients->value; i++) {
        cl = g_edicts[i].client;
        if (!g_edicts[i].inuse || cl->chase_target != ent)
            continue;
        memcpy(cl->ps.stats, ent->client->ps.stats, sizeof(cl->ps.stats));
        G_SetSpectatorStats(g_edicts + i);
    }
}

/*
===============
G_SetSpectatorStats
===============
*/
void G_SetSpectatorStats(edict_t *ent)
{
    gclient_t *cl = ent->client;

    if (!cl->chase_target)
        G_SetStats(ent);

    cl->ps.stats[STAT_SPECTATOR] = 1;

    // layouts are independant in spectator
    cl->ps.stats[STAT_LAYOUTS] = 0;
    if (cl->pers.health <= 0 || level.intermissiontime || cl->showscores)
        cl->ps.stats[STAT_LAYOUTS] |= 1;

	if (coop->value)
	{
		if (cl->chase_target)
		{
			if (cl->showscores || doHUD(ent))
				cl->ps.stats[STAT_LAYOUTS] |= 1;
		}
	}
	else
	{
		if (cl->showinventory && cl->pers.health > 0)
			cl->ps.stats[STAT_LAYOUTS] |= 2;

		if (cl->chase_target && cl->chase_target->inuse)
			cl->ps.stats[STAT_CHASE] = CS_PLAYERSKINS +
			(cl->chase_target - g_edicts) - 1;
		else
			cl->ps.stats[STAT_CHASE] = 0;
	}
}

