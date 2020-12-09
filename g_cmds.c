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
#include "m_player.h"


char *ClientTeam(edict_t *ent)
{
    char        *p;
    static char value[512];

    value[0] = 0;

    if (!ent->client)
        return value;

    strcpy(value, Info_ValueForKey(ent->client->pers.userinfo, "skin"));
    p = strchr(value, '/');
    if (!p)
        return value;

    if ((int)(dmflags->value) & DF_MODELTEAMS) {
        *p = 0;
        return value;
    }

    // if ((int)(dmflags->value) & DF_SKINTEAMS)
    return ++p;
}

qboolean OnSameTeam(edict_t *ent1, edict_t *ent2)
{
    char    ent1Team [512];
    char    ent2Team [512];

    if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
        return qfalse;

    strcpy(ent1Team, ClientTeam(ent1));
    strcpy(ent2Team, ClientTeam(ent2));

    if (strcmp(ent1Team, ent2Team) == 0)
        return qtrue;
    return qfalse;
}


void SelectNextItem(edict_t *ent, int itflags)
{
    gclient_t   *cl;
    int         i, index;
    gitem_t     *it;

    cl = ent->client;

    if (cl->chase_target) {
        ChaseNext(ent);
        return;
    }

    // scan  for the next valid one
    for (i = 1 ; i <= MAX_ITEMS ; i++) {
        index = (cl->pers.selected_item + i) % MAX_ITEMS;
        if (!cl->pers.inventory[index])
            continue;
        it = &itemlist[index];
        if (!it->use)
            continue;
        if (!(it->flags & itflags))
            continue;

        cl->pers.selected_item = index;
        return;
    }

    cl->pers.selected_item = -1;
}

void SelectPrevItem(edict_t *ent, int itflags)
{
    gclient_t   *cl;
    int         i, index;
    gitem_t     *it;

    cl = ent->client;

    if (cl->chase_target) {
        ChasePrev(ent);
        return;
    }

    // scan  for the next valid one
    for (i = 1 ; i <= MAX_ITEMS ; i++) {
        index = (cl->pers.selected_item + MAX_ITEMS - i) % MAX_ITEMS;
        if (!cl->pers.inventory[index])
            continue;
        it = &itemlist[index];
        if (!it->use)
            continue;
        if (!(it->flags & itflags))
            continue;

        cl->pers.selected_item = index;
        return;
    }

    cl->pers.selected_item = -1;
}

void ValidateSelectedItem(edict_t *ent)
{
    gclient_t   *cl;

    cl = ent->client;

    if (cl->pers.inventory[cl->pers.selected_item])
        return;     // valid

    SelectNextItem(ent, -1);
}


//=================================================================================

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f(edict_t *ent)
{
    char        *name;
    gitem_t     *it;
    int         index;
    int         i;
    qboolean    give_all;
    edict_t     *it_ent;

    if (deathmatch->value && !sv_cheats->value) {
        gi.cprintf(ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
        return;
    }

    name = gi.args();

    if (Q_stricmp(name, "all") == 0)
        give_all = qtrue;
    else
        give_all = qfalse;

    if (give_all || Q_stricmp(gi.argv(1), "health") == 0) {
        if (gi.argc() == 3)
            ent->health = atoi(gi.argv(2));
        else
            ent->health = ent->max_health;
        if (!give_all)
            return;
    }

    if (give_all || Q_stricmp(name, "weapons") == 0) {
        for (i = 0 ; i < game.num_items ; i++) {
            it = itemlist + i;
            if (!it->pickup)
                continue;
            if (!(it->flags & IT_WEAPON))
                continue;
            ent->client->pers.inventory[i] += 1;
        }
        if (!give_all)
            return;
    }

    if (give_all || Q_stricmp(name, "ammo") == 0) {
        for (i = 0 ; i < game.num_items ; i++) {
            it = itemlist + i;
            if (!it->pickup)
                continue;
            if (!(it->flags & IT_AMMO))
                continue;
            Add_Ammo(ent, it, 1000);
        }
        if (!give_all)
            return;
    }

    if (give_all || Q_stricmp(name, "armor") == 0) {
        gitem_armor_t   *info;

        it = FindItem("Jacket Armor");
        ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

        it = FindItem("Combat Armor");
        ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

        it = FindItem("Body Armor");
        info = (gitem_armor_t *)it->info;
        ent->client->pers.inventory[ITEM_INDEX(it)] = info->max_count;

        if (!give_all)
            return;
    }

    if (give_all || Q_stricmp(name, "Power Shield") == 0) {
        it = FindItem("Power Shield");
        it_ent = G_Spawn();
        it_ent->classname = it->classname;
        SpawnItem(it_ent, it);
        Touch_Item(it_ent, ent, NULL, NULL);
        if (it_ent->inuse)
            G_FreeEdict(it_ent);

        if (!give_all)
            return;
    }

    if (give_all) {
        for (i = 0 ; i < game.num_items ; i++) {
            it = itemlist + i;
            if (!it->pickup)
                continue;
            if (it->flags & (IT_ARMOR | IT_WEAPON | IT_AMMO))
                continue;
            ent->client->pers.inventory[i] = 1;
        }
        return;
    }

    it = FindItem(name);
    if (!it) {
        name = gi.argv(1);
        it = FindItem(name);
        if (!it) {
            gi.cprintf(ent, PRINT_HIGH, "unknown item\n");
            return;
        }
    }

    if (!it->pickup) {
        gi.cprintf(ent, PRINT_HIGH, "non-pickup item\n");
        return;
    }

    index = ITEM_INDEX(it);

    if (it->flags & IT_AMMO) {
        if (gi.argc() == 3)
            ent->client->pers.inventory[index] = atoi(gi.argv(2));
        else
            ent->client->pers.inventory[index] += it->quantity;
    } else {
        it_ent = G_Spawn();
        it_ent->classname = it->classname;
        SpawnItem(it_ent, it);
        Touch_Item(it_ent, ent, NULL, NULL);
        if (it_ent->inuse)
            G_FreeEdict(it_ent);
    }
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f(edict_t *ent)
{
    if (deathmatch->value && !sv_cheats->value) {
        gi.cprintf(ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
        return;
    }

    ent->flags ^= FL_GODMODE;
    if (!(ent->flags & FL_GODMODE))
        gi.cprintf(ent, PRINT_HIGH, "godmode OFF\n");
    else
        gi.cprintf(ent, PRINT_HIGH, "godmode ON\n");
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f(edict_t *ent)
{
    if (deathmatch->value && !sv_cheats->value) {
        gi.cprintf(ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
        return;
    }

    ent->flags ^= FL_NOTARGET;
    if (!(ent->flags & FL_NOTARGET))
        gi.cprintf(ent, PRINT_HIGH, "notarget OFF\n");
    else
        gi.cprintf(ent, PRINT_HIGH, "notarget ON\n");
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f(edict_t *ent)
{
    if (deathmatch->value && !sv_cheats->value) {
        gi.cprintf(ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
        return;
    }

    if (ent->movetype == MOVETYPE_NOCLIP) {
        ent->movetype = MOVETYPE_WALK;
        gi.cprintf(ent, PRINT_HIGH, "noclip OFF\n");
    } else {
        ent->movetype = MOVETYPE_NOCLIP;
        gi.cprintf(ent, PRINT_HIGH, "noclip ON\n");
    }
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f(edict_t *ent)
{
    int         index;
    gitem_t     *it;
    char        *s;

    s = gi.args();
    it = FindItem(s);
    if (!it) {
        gi.cprintf(ent, PRINT_HIGH, "unknown item: %s\n", s);
        return;
    }
    if (!it->use) {
        gi.cprintf(ent, PRINT_HIGH, "Item is not usable.\n");
        return;
    }
    index = ITEM_INDEX(it);
    if (!ent->client->pers.inventory[index]) {
        gi.cprintf(ent, PRINT_HIGH, "Out of item: %s\n", s);
        return;
    }

    it->use(ent, it);
}


/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
void Cmd_Drop_f(edict_t *ent)
{
    int         index;
    gitem_t     *it;
    char        *s;

    s = gi.args();
    it = FindItem(s);
    if (!it) {
        gi.cprintf(ent, PRINT_HIGH, "unknown item: %s\n", s);
        return;
    }
    if (!it->drop) {
        gi.cprintf(ent, PRINT_HIGH, "Item is not dropable.\n");
        return;
    }
    index = ITEM_INDEX(it);
    if (!ent->client->pers.inventory[index]) {
        gi.cprintf(ent, PRINT_HIGH, "Out of item: %s\n", s);
        return;
    }

    it->drop(ent, it);
}


/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f(edict_t *ent)
{
    int         i;
    gclient_t   *cl;

    cl = ent->client;

    cl->showscores = qfalse;
    cl->showhelp = qfalse;

    if (cl->showinventory) {
        cl->showinventory = qfalse;
        return;
    }

    cl->showinventory = qtrue;

    gi.WriteByte(svc_inventory);
    for (i = 0 ; i < MAX_ITEMS ; i++) {
        gi.WriteShort(cl->pers.inventory[i]);
    }
    gi.unicast(ent, qtrue);
}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f(edict_t *ent)
{
    gitem_t     *it;

    ValidateSelectedItem(ent);

    if (ent->client->pers.selected_item == -1) {
        gi.cprintf(ent, PRINT_HIGH, "No item to use.\n");
        return;
    }

    it = &itemlist[ent->client->pers.selected_item];
    if (!it->use) {
        gi.cprintf(ent, PRINT_HIGH, "Item is not usable.\n");
        return;
    }
    it->use(ent, it);
}

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f(edict_t *ent)
{
    gclient_t   *cl;
    int         i, index;
    gitem_t     *it;
    int         selected_weapon;

    cl = ent->client;

    if (!cl->pers.weapon)
        return;

    selected_weapon = ITEM_INDEX(cl->pers.weapon);

    // scan  for the next valid one
    for (i = 1 ; i <= MAX_ITEMS ; i++) {
        index = (selected_weapon + i) % MAX_ITEMS;
        if (!cl->pers.inventory[index])
            continue;
        it = &itemlist[index];
        if (!it->use)
            continue;
        if (!(it->flags & IT_WEAPON))
            continue;
        it->use(ent, it);
        if (cl->pers.weapon == it)
            return; // successful
    }
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f(edict_t *ent)
{
    gclient_t   *cl;
    int         i, index;
    gitem_t     *it;
    int         selected_weapon;

    cl = ent->client;

    if (!cl->pers.weapon)
        return;

    selected_weapon = ITEM_INDEX(cl->pers.weapon);

    // scan  for the next valid one
    for (i = 1 ; i <= MAX_ITEMS ; i++) {
        index = (selected_weapon + MAX_ITEMS - i) % MAX_ITEMS;
        if (!cl->pers.inventory[index])
            continue;
        it = &itemlist[index];
        if (!it->use)
            continue;
        if (!(it->flags & IT_WEAPON))
            continue;
        it->use(ent, it);
        if (cl->pers.weapon == it)
            return; // successful
    }
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f(edict_t *ent)
{
    gclient_t   *cl;
    int         index;
    gitem_t     *it;

    cl = ent->client;

    if (!cl->pers.weapon || !cl->pers.lastweapon)
        return;

    index = ITEM_INDEX(cl->pers.lastweapon);
    if (!cl->pers.inventory[index])
        return;
    it = &itemlist[index];
    if (!it->use)
        return;
    if (!(it->flags & IT_WEAPON))
        return;
    it->use(ent, it);
}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f(edict_t *ent)
{
    gitem_t     *it;

    ValidateSelectedItem(ent);

    if (ent->client->pers.selected_item == -1) {
        gi.cprintf(ent, PRINT_HIGH, "No item to drop.\n");
        return;
    }

    it = &itemlist[ent->client->pers.selected_item];
    if (!it->drop) {
        gi.cprintf(ent, PRINT_HIGH, "Item is not dropable.\n");
        return;
    }
    it->drop(ent, it);
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f(edict_t *ent)
{
    if ((level.time - ent->client->respawn_time) < 5)
        return;
    ent->flags &= ~FL_GODMODE;
    ent->health = 0;
    meansOfDeath = MOD_SUICIDE;
    player_die(ent, ent, ent, 100000, vec3_origin);
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f(edict_t *ent)
{
	// risk of a loop
	if (doHUD(ent))
	{
		gi.WriteByte(svc_stufftext);
		//gi.WriteString("togglemenu\n");
		gi.WriteString("pushmenu main\n");
		gi.unicast(ent, qtrue);
		//gi.cprintf(ent, PRINT_HIGH, "Press [ESC] again for menu\n");

		//ent->client->showhudtime = level.time + 2; // was 3
	}

    ent->client->showscores = qfalse;
    ent->client->showhelp = qfalse;
    ent->client->showinventory = qfalse;
}


int PlayerSort(void const *a, void const *b)
{
    int     anum, bnum;

    anum = *(int *)a;
    bnum = *(int *)b;

    anum = game.clients[anum].ps.stats[STAT_FRAGS];
    bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

    if (anum < bnum)
        return -1;
    if (anum > bnum)
        return 1;
    return 0;
}

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f(edict_t *ent)
{
    int     i;
    int     count;
    char    small[64];
    char    large[1280];
    int     index[256];

    count = 0;
    for (i = 0 ; i < maxclients->value ; i++)
        if (game.clients[i].pers.connected) {
            index[count] = i;
            count++;
        }

    // sort by frags
    qsort(index, count, sizeof(index[0]), PlayerSort);

    // print information
    large[0] = 0;

    for (i = 0 ; i < count ; i++) {
        Q_snprintf(small, sizeof(small), "%3i %s\n",
                   game.clients[index[i]].ps.stats[STAT_FRAGS],
                   game.clients[index[i]].pers.netname);
        if (strlen(small) + strlen(large) > sizeof(large) - 100) {
            // can't print all of them in one packet
            strcat(large, "...\n");
            break;
        }
        strcat(large, small);
    }

    gi.cprintf(ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}

/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f(edict_t *ent)
{
    int     i;

    i = atoi(gi.argv(1));

    // can't wave when ducked
    if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
        return;

    if (ent->client->anim_priority > ANIM_WAVE)
        return;

    ent->client->anim_priority = ANIM_WAVE;

    switch (i) {
    case 0:
        gi.cprintf(ent, PRINT_HIGH, "flipoff\n");
        ent->s.frame = FRAME_flip01 - 1;
        ent->client->anim_end = FRAME_flip12;
        break;
    case 1:
        gi.cprintf(ent, PRINT_HIGH, "salute\n");
        ent->s.frame = FRAME_salute01 - 1;
        ent->client->anim_end = FRAME_salute11;
        break;
    case 2:
        gi.cprintf(ent, PRINT_HIGH, "taunt\n");
        ent->s.frame = FRAME_taunt01 - 1;
        ent->client->anim_end = FRAME_taunt17;
        break;
    case 3:
        gi.cprintf(ent, PRINT_HIGH, "wave\n");
        ent->s.frame = FRAME_wave01 - 1;
        ent->client->anim_end = FRAME_wave11;
        break;
    case 4:
    default:
        gi.cprintf(ent, PRINT_HIGH, "point\n");
        ent->s.frame = FRAME_point01 - 1;
        ent->client->anim_end = FRAME_point12;
        break;
    }
}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f(edict_t *ent, qboolean team, qboolean arg0)
{
    int     i, j;
    edict_t *other;
    char    *p;
    char    text[2048];
    gclient_t *cl;

    if (gi.argc() < 2 && !arg0)
        return;

    if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
        team = qfalse;

    if (team)
        Q_snprintf(text, sizeof(text), "(%s): ", ent->client->pers.netname);
    else
        Q_snprintf(text, sizeof(text), "%s: ", ent->client->pers.netname);

    if (arg0) {
        strcat(text, gi.argv(0));
        strcat(text, " ");
        strcat(text, gi.args());
    } else {
        p = gi.args();

        if (*p == '"') {
            p++;
            p[strlen(p) - 1] = 0;
        }
        strcat(text, p);
    }

    // don't let text be too long for malicious reasons
    if (strlen(text) > 150)
        text[150] = 0;

    strcat(text, "\n");

    if (flood_msgs->value) {
        cl = ent->client;

        if (level.time < cl->flood_locktill) {
            gi.cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
                       (int)(cl->flood_locktill - level.time));
            return;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when) / sizeof(cl->flood_when[0])) + i;
        if (cl->flood_when[i] &&
            level.time - cl->flood_when[i] < flood_persecond->value) {
            cl->flood_locktill = level.time + flood_waitdelay->value;
            gi.cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
                       (int)flood_waitdelay->value);
            return;
        }
        cl->flood_whenhead = (cl->flood_whenhead + 1) %
                             (sizeof(cl->flood_when) / sizeof(cl->flood_when[0]));
        cl->flood_when[cl->flood_whenhead] = level.time;
    }

    if (dedicated->value)
        gi.cprintf(NULL, PRINT_CHAT, "%s", text);

    for (j = 1; j <= game.maxclients; j++) {
        other = &g_edicts[j];
        if (!other->inuse)
            continue;
        if (!other->client)
            continue;
        if (team) {
            if (!OnSameTeam(ent, other))
                continue;
        }
        gi.cprintf(other, PRINT_CHAT, "%s", text);
    }
}

void Cmd_PlayerList_f(edict_t *ent)
{
    int i;
    char st[80];
    char text[1400];
    edict_t *e2;

    // connect time, ping, score, name
    *text = 0;
    for (i = 0, e2 = g_edicts + 1; i < maxclients->value; i++, e2++) {
        if (!e2->inuse)
            continue;

        Q_snprintf(st, sizeof(st), "%02d:%02d %4d %3d %s%s\n",
                   (level.framenum - e2->client->resp.enterframe) / 600,
                   ((level.framenum - e2->client->resp.enterframe) % 600) / 10,
                   e2->client->ping,
                   e2->client->resp.score,
                   e2->client->pers.netname,
                   e2->client->resp.spectator ? " (spectator)" : "");
        if (strlen(text) + strlen(st) > sizeof(text) - 50) {
            sprintf(text + strlen(text), "And more...\n");
            gi.cprintf(ent, PRINT_HIGH, "%s", text);
            return;
        }
        strcat(text, st);
    }
    gi.cprintf(ent, PRINT_HIGH, "%s", text);
}

// Rroff - trace forward from player eye height
// if we hit an entity spit out some information on it
void Cmd_EntityInfo_f(edict_t *ent)
{
	vec3_t		forward, start, end;
	float		*v;
	trace_t		tr;
	int			flags;

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);

	v = tv(0, 0, ent->viewheight);
	VectorAdd(ent->s.origin, v, start);

	VectorMA(start, 768, forward, end); // may want to increase distance

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

	// Might need to check the tr.fraction to work around some bugs

	if (tr.ent)
	{
		flags = tr.ent->spawnflags;
		gi.dprintf("Entity info for: %s\n===== \n Model: %s\n===== \n Spawnflags: %i\n Target: %s Targetname: %s\n",tr.ent->classname, tr.ent->model, flags, tr.ent->target, tr.ent->targetname);
		gi.dprintf("Entity position %s looking %s\n", vtos(tr.ent->s.origin), vtos(tr.ent->s.angles));
	}

	gi.dprintf("\nPlayer origin %s looking %s\n", vtos(ent->s.origin), vtos(ent->s.angles));
	gi.dprintf("\nEntity load %i of %i [%i]\n", globals.num_edicts, globals.max_edicts, game.maxentities);

	gi.dprintf("Entity has:");
	if (tr.ent->enemy)
	{
		gi.dprintf(" enemy %s ", tr.ent->enemy->classname);
	}

	if (tr.ent->oldenemy)
	{
		gi.dprintf(" old enemy");
	}

	if (tr.ent->goalentity)
	{
		gi.dprintf(" goalentity");
	}
	gi.dprintf("\n");

	if (tr.ent->svflags & SVF_MONSTER)
	{
		if (tr.ent->monsterinfo.aiflags & AI_ROAMING)
			gi.dprintf("Roaming monster\n");

	}

	gi.dprintf("Trace endpos: %3.1f,%3.1f,%3.1f\n", tr.endpos[0], tr.endpos[1], tr.endpos[2]);
}

void Cmd_PosInfo_f(edict_t *ent)
{
	float		y;

	edict_t		*tmp;

	y = ent->s.angles[1];
	if (y < 0)
		y = 360 + y;


	//gi.dprintf("\nEye position %f, %f, %f looking %f, %f, %f\n", ent->s.origin[0], ent->s.origin[1], (ent->s.origin[2] + ent->viewheight), ent->s.angles[0], y, ent->s.angles[2]);

	// add monster spawns
	//gi.dprintf("VectorSet(start,%3.1f,%3.1f,%3.1f);\n", ent->s.origin[0], ent->s.origin[1], (ent->s.origin[2] + ent->viewheight));
	//gi.dprintf("VectorSet(spawn_angles,%3.1f,%3.1f,%3.1f);\n", 0, y, 0);
	//gi.dprintf("Add_Monster(\"%s\", NULL, start, spawn_angles, 0, NULL, 0, 0);\n", gi.args());

	// lockdown monster spawns
	// gi.dprintf("{\nqtrue,\n\"%s\",\n", level.mapname);
	// gi.dprintf("%3.1f,%3.1f,%3.1f,\n", ent->s.origin[0], ent->s.origin[1], (ent->s.origin[2] + ent->viewheight));
	// gi.dprintf("%3.1f,%3.1f,%3.1f,\n0\n},\n", 0, y, 0);

	// lockdown ai routing
	gi.dprintf("{qtrue,\"%s\",\"%s\",%3.1f,%3.1f,%3.1f},\n", level.mapname, gi.args(), ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]);

	tmp = G_Spawn();

	tmp->classname = "pos_flare";

	VectorCopy(ent->s.origin, tmp->s.origin);
	tmp->s.modelindex = gi.modelindex("models/objects/laser/tris.md2");
	tmp->s.renderfx = RF_FULLBRIGHT;
	tmp->solid = SOLID_NOT;
	tmp->flags |= FL_TOUCH;
	tmp->movetype = MOVETYPE_TOSS;

	VectorSet(tmp->mins, -16, -16, 0);
	VectorSet(tmp->maxs, 16, 16, 56);

	gi.linkentity(tmp);
}

void Cmd_GenInfo_f(edict_t *ent)
{
	edict_t     *e;
	int			i;
	int			dmg = 0, health = 0;

	e = &g_edicts[game.maxclients + 1];
	for (i = game.maxclients + 1; i < globals.num_edicts; i++, e++) {
		if (e->inuse){
			if (!Q_stricmp(e->classname, "ammo_grenades"))
				dmg += 625;
			if (!Q_stricmp(e->classname, "ammo_bullets"))
				dmg += 400;
			if (!Q_stricmp(e->classname, "ammo_shells"))
				dmg += 480;
			if (!Q_stricmp(e->classname, "ammo_cells"))
				dmg += 1000;
			if (!Q_stricmp(e->classname, "ammo_rockets"))
				dmg += 500;
			if (!Q_stricmp(e->classname, "ammo_slugs"))
				dmg += 1500;
			if (e->svflags & SVF_MONSTER)
				health += ent->health;
		}
	}

	gi.dprintf("Monster combined health: %i Item damage total: %i\n", health, dmg);

	gi.dprintf("Class info: %i\n", ent->client->pers.player_class);

}

void Cmd_GameInfo_f(edict_t *self)
{
	int			i, reuse;
	edict_t		*ent;

	for (i = 0; i < globals.num_edicts; i++) {
		ent = &g_edicts[i];
		if (ent->svflags & SVF_REUSE)
			reuse = 1;
		else
			reuse = 0;
		gi.dprintf("Edict %i[%i] inuse: %i reuse: %i : %s at %s\n", i, ent->s.number, ent->inuse, reuse, ent->classname, vtos(ent->s.origin));
	}

	gi.dprintf("Currently allocated %i of %i [%i] edicts with %i allocated at level load time\n", globals.num_edicts, game.maxentities, globals.max_edicts, level.num_edicts);
}


void Cmd_Ability1_f(edict_t *ent)
{
	int			index;
	gitem_t		*fitem = NULL;

	if (!coop->value)
		return;

	if (ent->client->pers.player_class & PC_MEDIC)
	{
		fitem = FindItem("Medic Station");
	}

	if (ent->client->pers.player_class & PC_TECH)
	{
		fitem = FindItem("Turret");
	}

	if (ent->client->pers.player_class & PC_GRUNT)
	{
		fitem = FindItem("Taunt");
	}

	if (!fitem)
		return;

	index = ITEM_INDEX(fitem);

	if (ent->client->pers.inventory[index] > 0)
	{
		fitem->use(ent, fitem);
	}
}

void Cmd_Ability2_f(edict_t *ent)
{
	int			index;
	gitem_t		*fitem = NULL;

	if (!coop->value)
		return;

	if (ent->client->pers.player_class & PC_MEDIC)
	{
		fitem = FindItem("Throw Stim");
	}

	if (ent->client->pers.player_class & PC_TECH)
	{
		fitem = FindItem("Ammo Station");
	}

	if (!fitem)
		return;

	index = ITEM_INDEX(fitem);

	if (ent->client->pers.inventory[index] > 0)
	{
		fitem->use(ent, fitem);
	}
}

void Cmd_Ability3_f(edict_t *ent)
{
	int			index;
	gitem_t		*fitem = NULL;

	if (!coop->value)
		return;

	if (ent->client->pers.player_class & PC_TECH)
	{
		fitem = FindItem("Explosive Box");
	}

	if (!fitem)
		return;

	index = ITEM_INDEX(fitem);

	if (ent->client->pers.inventory[index] > 0)
	{
		fitem->use(ent, fitem);
	}
}

// Rroff prevent spawning at a way point flare

void Cmd_Noflare_f(edict_t *ent)
{
	if ((ent->client->resp.flare) || (ent->client->resp.oldflare))
	{
		ent->client->resp.flare = NULL;
		ent->client->resp.oldflare = NULL;
		gi.cprintf(ent, PRINT_HIGH, "You will respawn at the start of the map.\n");
	}
	else {
		gi.cprintf(ent, PRINT_HIGH, "You didn't have a valid flare spawn point anyway.\n");
	}
}

void Cmd_HordeSpawnPoint_f(edict_t *ent)
{
	gi.dprintf("sv hordecreatespawn %3.1f %3.1f %3.1f %3.1f\n", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2], ent->s.angles[1]);
}

void Cmd_HordeSpawnBoss_f(edict_t *ent)
{
	gi.dprintf("sv hordecreatebossspawn %3.1f %3.1f %3.1f %i\n", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2], atoi(gi.argv(1)));
}

// need to drop to floor (height - 24)

void Cmd_HordeSpawnComputer_f(edict_t *ent)
{
	gi.dprintf("sv hordecreatecomputer %3.1f %3.1f %3.1f %3.1f\n", ent->s.origin[0], ent->s.origin[1], (ent->s.origin[2]-24), ent->s.angles[1]);
}

void Cmd_CoopSpawnPoint_f(edict_t *ent)
{

	if (gi.argc() >= 1)
		gi.dprintf("sv createcoopspot %3.1f %3.1f %3.1f %3.1f %s\n", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2], ent->s.angles[1], gi.argv(1));
	else
		gi.dprintf("sv createcoopspot %3.1f %3.1f %3.1f %3.1f\n", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2], ent->s.angles[1]);
}


/*
=================
ClientCommand
=================
*/
void ClientCommand(edict_t *ent)
{
    char    *cmd;

    if (!ent->client)
        return;     // not fully in game yet

    cmd = gi.argv(0);

    if (Q_stricmp(cmd, "players") == 0) {
        Cmd_Players_f(ent);
        return;
    }
    if (Q_stricmp(cmd, "say") == 0) {
        Cmd_Say_f(ent, qfalse, qfalse);
        return;
    }
    if (Q_stricmp(cmd, "say_team") == 0) {
        Cmd_Say_f(ent, qtrue, qfalse);
        return;
    }
    if (Q_stricmp(cmd, "score") == 0) {
        Cmd_Score_f(ent);
        return;
    }
    if (Q_stricmp(cmd, "help") == 0) {
        Cmd_Help_f(ent);
        return;
    }

    if (level.intermissiontime)
        return;

    if (Q_stricmp(cmd, "use") == 0)
        Cmd_Use_f(ent);
    else if (Q_stricmp(cmd, "drop") == 0)
        Cmd_Drop_f(ent);
    else if (Q_stricmp(cmd, "give") == 0)
        Cmd_Give_f(ent);
    else if (Q_stricmp(cmd, "god") == 0)
        Cmd_God_f(ent);
    else if (Q_stricmp(cmd, "notarget") == 0)
        Cmd_Notarget_f(ent);
    else if (Q_stricmp(cmd, "noclip") == 0)
        Cmd_Noclip_f(ent);
    else if (Q_stricmp(cmd, "inven") == 0)
        Cmd_Inven_f(ent);
    else if (Q_stricmp(cmd, "invnext") == 0)
        SelectNextItem(ent, -1);
    else if (Q_stricmp(cmd, "invprev") == 0)
        SelectPrevItem(ent, -1);
    else if (Q_stricmp(cmd, "invnextw") == 0)
        SelectNextItem(ent, IT_WEAPON);
    else if (Q_stricmp(cmd, "invprevw") == 0)
        SelectPrevItem(ent, IT_WEAPON);
    else if (Q_stricmp(cmd, "invnextp") == 0)
        SelectNextItem(ent, IT_POWERUP);
    else if (Q_stricmp(cmd, "invprevp") == 0)
        SelectPrevItem(ent, IT_POWERUP);
    else if (Q_stricmp(cmd, "invuse") == 0)
        Cmd_InvUse_f(ent);
    else if (Q_stricmp(cmd, "invdrop") == 0)
        Cmd_InvDrop_f(ent);
    else if (Q_stricmp(cmd, "weapprev") == 0)
        Cmd_WeapPrev_f(ent);
    else if (Q_stricmp(cmd, "weapnext") == 0)
        Cmd_WeapNext_f(ent);
    else if (Q_stricmp(cmd, "weaplast") == 0)
        Cmd_WeapLast_f(ent);
    else if (Q_stricmp(cmd, "kill") == 0)
        Cmd_Kill_f(ent);
    else if (Q_stricmp(cmd, "putaway") == 0)
        Cmd_PutAway_f(ent);
    else if (Q_stricmp(cmd, "wave") == 0)
        Cmd_Wave_f(ent);
    else if (Q_stricmp(cmd, "playerlist") == 0)
        Cmd_PlayerList_f(ent);
	// remove these for release
	/*
	else if (Q_stricmp(cmd, "entityinfo") == 0)
		Cmd_EntityInfo_f(ent);
	else if (Q_stricmp(cmd, "position") == 0)
		Cmd_PosInfo_f(ent);
	else if (Q_stricmp(cmd, "infostuff") == 0)
		Cmd_GenInfo_f(ent);
	else if (Q_stricmp(cmd, "gameinfo") == 0)
		Cmd_GameInfo_f(ent);
	*/
	// end remove
	else if (Q_stricmp(cmd, "noflare") == 0)
		Cmd_Noflare_f(ent);
	else if (Q_stricmp(cmd, "ability1") == 0)
		Cmd_Ability1_f(ent);
	else if (Q_stricmp(cmd, "ability2") == 0)
		Cmd_Ability2_f(ent);
	else if (Q_stricmp(cmd, "ability3") == 0)
		Cmd_Ability3_f(ent);
	else if (Q_stricmp(cmd, "hordespawnpoint") == 0)
		Cmd_HordeSpawnPoint_f(ent);
	else if (Q_stricmp(cmd, "hordespawnboss") == 0)
		Cmd_HordeSpawnBoss_f(ent);
	else if (Q_stricmp(cmd, "hordecomputer") == 0)
		Cmd_HordeSpawnComputer_f(ent);
	else if (Q_stricmp(cmd, "coopspot") == 0)
		Cmd_CoopSpawnPoint_f(ent);
    else    // anything that doesn't match a command will be a chat
        Cmd_Say_f(ent, qfalse, qtrue);
}


// spawn computer
// spawn point
// boss point
// coop point