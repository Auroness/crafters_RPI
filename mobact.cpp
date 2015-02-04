//////////////////////////////////////////////////////////////////////////////
//
/// mobact.cpp - Mobiles AI Routines
//
//
/// Crafters RPI
/// Copyright (C) 2015 M. C. Huston
/// Authors: M.C Huston (auroness@gmail.com)
//
/// Shadows of Isildur RPI Engine++
/// Copyright (C) 2004-2006 C. W. McHenry
/// Authors: C. W. McHenry (traithe@middle-earth.us)
///          Jonathan W. Webb (sighentist@middle-earth.us)
/// URL: http://www.middle-earth.us
//
/// May includes portions derived from Harshlands
/// Authors: Charles Rand (Rassilon)
/// URL: http://www.harshlands.net
//
/// May include portions derived under license from DikuMUD Gamma (0.0)
/// which are Copyright (C) 1990, 1991 DIKU
/// Authors: Hans Henrik Staerfeldt (bombman@freja.diku.dk)
///          Tom Madson (noop@freja.diku.dk)
///          Katja Nyboe (katz@freja.diku.dk)
///          Michael Seifert (seifert@freja.diku.dk)
///          Sebastian Hammer (quinn@freja.diku.dk)
//
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include "structs.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "group.h"
#include "utility.h"

extern const char *dirs[];


CHAR_DATA *
mob_remembers_whom (CHAR_DATA * ch)
{
	CHAR_DATA *tch;
	ROOM_DATA *room;
	struct memory_data *mem;

	room = ch->room;

	if (IS_NPC (ch) && !IS_SET (ch->mob->action, ACT_MEMORY))
		return 0;

	for (tch = room->people; tch; tch = tch->next_in_room)
	{

		if (IS_NPC (tch) || !can_see_mob(ch, tch))
			continue;

		for (mem = ch->remembers; mem; mem = mem->next)
			if (!strcmp (mem->name, tch->name))
			{

				if (IS_SET (tch->flags, FLAG_SUBDUING))
					continue;

				return tch;
			}
	}

	return NULL;
}

int
mob_remembers (CHAR_DATA * ch)
{
	CHAR_DATA *tch;

	if ((tch = mob_remembers_whom (ch)))
	{

		return 1;
	}

	return 0;
}

#define MOB_IGNORES_DOORS -1
int
mob_wander (CHAR_DATA * ch)
{
	ROOM_DATA *room_exit;
	ROOM_DATA *room;
	int room_exit_virt;
	int exit_tab[6];
	int num_exits = 0;
	int to_exit;
	int i;
	int door = MOB_IGNORES_DOORS;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };


	if (IS_NPC(ch) && number (0, 5))
		return 0;
	else if (IS_NPC(ch) && number (0, 2))
		return 0;

	room = ch->room;

	if (IS_NPC(ch) && IS_SET (ch->mob->profession, PROF_SENTINEL))
		return 0;

	if (IS_NPC(ch) && IS_SET (ch->mob->profession, PROF_VEHICLE))
		return 0;

	if ((ch->following && ch->following->room == ch->room) || number (0, 9))
		return 0;

	if (ch->get_position() != STAND)
		return 0;

	if (ch->desc && ch->desc->original)
		return 0;


	if (ch->is_subduee())
		return 0;

	/* Decide here if we want to open a door */
	door = (ch->race > 29) ? MOB_IGNORES_DOORS : (rand () % 6);

	for (i = 0; i <= LAST_DIR; i++)
	{

		if ((!is_exit(ch, i)) ||	/* IF Dir not an exit */
			(is_exit(ch, i)->to_room == NOWHERE) ||	/* OR Exit to nowhere */
			(IS_SET (is_exit(ch, i)->port_flags, EX_CLOSED) &&	/* OR closed doors AND */
			((i != door) ||	/* -- IF ch is ignoring door */
			(is_exit(ch, i)->port_flags != (EX_ISDOOR | EX_CLOSED | EX_ISGATE))	/* -- OR they are complex */
			)))
		{
			continue;
		}

		room_exit = vtor (is_exit(ch, i)->to_room);

		if (!room_exit)
		{
			continue;
		}

		
		if (!IS_SET (room_exit->room_flags, NO_MOB)
			&& !(IS_MERCHANT (ch)
			&& IS_SET (room_exit->room_flags, NO_MERCHANT))
			&& !(ch->mob->noaccess_flags & room_exit->room_flags)
			&& (!ch->mob->access_flags || ch->mob->access_flags & room_exit->room_flags)
			&& (IS_NPC(ch) && !(IS_SET (ch->mob->action, ACT_FLYING))
				&& (room_exit->terrain_type == SECT_UNDERWATER))
			)
			exit_tab[num_exits++] = i;
	}

	if (num_exits == 0)
		return 0;

	to_exit = number (1, num_exits) - 1;

	if (vtor (is_exit(ch, exit_tab[to_exit])->to_room)->nVirtual == ch->last_room)
		to_exit = (to_exit + 1) % num_exits;

	room_exit_virt = vtor (is_exit(ch, exit_tab[to_exit])->to_room)->nVirtual;

	ch->last_room = room_exit_virt;

	i = exit_tab[to_exit];
	if ((ch->room->dir_option[i]) && (IS_SET (ch->room->dir_option[i]->port_flags, EX_CLOSED)))
	{
		one_argument (ch->room->dir_option[i]->keyword, buf2);
		sprintf (buf, "%s %s", buf2, dirs[i]);
		do_open (ch, buf, 0);
	}

	if (IS_SET (ch->affected_by, AFF_SNEAK) && ch->speed < 3)
	{
		ch->speed = 0;
		do_sneak (ch, NULL, exit_tab[to_exit]);
	}
	else
		do_move (ch, "", exit_tab[to_exit]);

	return 1;
}


int
mob_weather_reaction (CHAR_DATA * ch)
{
	AFFECTED_TYPE *af;

	if (!get_equip (ch, WEAR_ABOUT))
		return 0;

	if (!IS_SET (get_equip (ch, WEAR_ABOUT)->obj_flags.extra_flags, ITEM_MASK))
		return 0;

	af = get_affect (ch, MAGIC_RAISED_HOOD);

	if (IS_SET(ch->room->room_flags, INDOORS)
		 || (ch->room->terrain_type == SECT_CAVE)
		 || weather_info[ch->room->wzone].state <= CHANCE_RAIN)
	{
		if (af && ch->is_hooded())
		{
			affect_remove (ch, af);
			do_hood (ch, "", 0);
			return 1;
		}
		else
			return 0;
	}

	if (get_affect (ch, MAGIC_RAISED_HOOD))
		return 0;

	if (!ch->is_hooded() 
		&& ch->room->terrain_type != SECT_CAVE
		&& !IS_SET(ch->room->room_flags, INDOORS)
		&& weather_info[ch->room->wzone].state > CHANCE_RAIN)
	{
		if (!af)
		{
			af = new AFFECTED_TYPE;
			af->next = NULL;
			af->type = MAGIC_RAISED_HOOD;
			affect_to_char (ch, af);
		}
		do_hood (ch, "", 0);
		return 1;
	}

	return 0;
}




void
npc_evasion (CHAR_DATA * ch, int dir)
{
	int roll = 0;
	int offset = 0;
	int count = 0;
	int value = 0;
	
	do_stand (ch, "", 0);

		if (ch->following)
	{
		return;
		
	}

	roll = number (1, 5);
	offset = dir + roll;
	do_set (ch, "run", 0);
	
	while(count <= LAST_DIR)
	{
		value = count + offset;
		if (value > LAST_DIR)
			value = value - LAST_DIR;
		
		if (CAN_GO(ch, value))
		{
			do_move (ch, "", value);
		}
		count ++;
	}
	
	do_set (ch, "walk", 0);
}


void
mobile_routines (int pulse)
{
	int zone, dir = 0, jailer = 0;
	CHAR_DATA *ch, *tch;
	ROOM_DATA *room;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	std::list<char_data*>::iterator tch_iterator;
	
	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		ch = *tch_iterator;

		if (ch->deleted)
			continue;

		if (IS_SET (ch->flags, FLAG_BINDING))
			continue;

		if (ch->desc)
			continue;

		if (!ch->room)
			continue;

		if (GET_FLAG (ch, FLAG_LEAVING))
			continue;

		/* close the door of the room we are coming from */
		if (GET_FLAG (ch, FLAG_ENTERING))
		{

			if (is_exit(ch, ch->from_dir) &&
				(is_exit(ch, ch->from_dir)->port_flags == EX_ISDOOR ||
				is_exit(ch, ch->from_dir)->port_flags == EX_ISGATE) &&
				!ch->following && ch->race <= 29)
			{

				one_argument (is_exit(ch, ch->from_dir)->keyword, buf2);
				sprintf (buf, "%s %s", buf2, dirs[ch->from_dir]);
				do_close (ch, buf, 0);

			}

			continue;
		}


		if (GET_FLAG (ch, FLAG_FLEE))
			continue;


		if (ch->delay)
			continue;

		

		if (ch->is_subduee())
			continue;

		room = ch->room;
		zone = room->zone;

		if (!IS_NPC (ch) || !ch->is_awake())
			continue;


		if ((rand () % 40 == 0) && mob_weather_reaction (ch))
			continue;

		if (!((pulse + 1) % PULSE_SMART_MOBS) && mob_wander (ch))
			continue;

	}				/* for */

}

int
is_threat (CHAR_DATA * ch, CHAR_DATA * tch)
{
	THREAT_DATA *tmp;
	ATTACKER_DATA *tmp_att;

	for (tmp = ch->threats; tmp; tmp = tmp->next)
		if (tmp->source == tch)
			return 1;

	for (tmp_att = ch->attackers; tmp_att; tmp_att = tmp_att->next)
		if (tmp_att->attacker == tch)
			return 1;

	return 0;
}

int
would_attack (CHAR_DATA * ch, CHAR_DATA * tch)
{
	if (ch == tch)
		return 0;

	if (IS_NPC (tch) && IS_SET (tch->mob->profession, PROF_VEHICLE))
		return 0;
	
	if (IS_NPC (ch) && IS_SET (ch->mob->profession, PROF_VEHICLE))
		return 0;

	if (IS_NPC (ch) && IS_NPC (tch))
	{

		//soldiers should not attack wildlife
		if(IS_NPC(ch) 
			&& IS_SET (ch->mob->profession, PROF_SOLDIER)
			&& IS_SET (tch->mob->profession, PROF_WILDLIFE))
			   
		{
			return 0;
		}
	
		//wildlife will not attach each other
		if (IS_NPC(ch) 
			&& IS_SET (ch->mob->profession, PROF_WILDLIFE) 
			&& IS_SET (tch->mob->profession, PROF_WILDLIFE))
			{
			return 0;
			}
	}
	
	
		//normally NPC won't risk chasing down a retreating group
	if (IS_NPC(ch) && get_affect(tch, AFFECT_GROUP_RETREAT))
		return 0;
	
	/* Wrestling */


	if (tch->is_subduee())
		return 0;

	if (can_see_mob(ch, tch) 
		&& (IS_NPC(ch) && IS_SET (ch->mob->action, ACT_AGGRESSIVE)) 
		&& (IS_NPC(ch) 
			&& !IS_SET (ch->mob->action, ACT_WIMPY) 
			|| !tch->is_awake()) 
		&& !is_brother (ch, tch))
		return 1;

	if (is_threat (ch, tch))
		return 1;

	return 0;
}

void
do_would (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (argument, buf);

	if (!*buf || *buf == '?')
	{
		ch->send_to_char ("Would determines if a mob would attack you.\n");
		return;
	}

	if ((tch = get_char_room (buf, ch->in_room)))
	{
		if (would_attack (tch, ch))
			tch->act("$n would attack $N.", false, 0, ch, TO_VICT);
		if (would_attack (ch, tch))
			ch->act("You would attack $N.", false, 0, tch, TO_CHAR);
	}
	else
		ch->send_to_char ("Couldn't find that mob.\n");
}



