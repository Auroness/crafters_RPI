//////////////////////////////////////////////////////////////////////////////
//
/// act.offensive.cpp - Violence and Offensive Functions
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

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <string>
#include "structs.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "group.h"
#include "utility.h"

extern const char* dirs[];
extern const char* verbose_dirs[];
extern const char* opposite_dirs[];


/* ch here is the victim mob or PC */
void
notify_guardians (CHAR_DATA * ch, CHAR_DATA * tch, int cmd)
{
	unsigned short int flag = 0x00;
	int nMobVnum = 0;
	char buf[AVG_STRING_LENGTH];
	char *strViolation[] = {
		"Hits to injure", "Hits to kill", "Hits to injure", "Took aim at",
		"Slings at", "Throws at",
		"Attempts to steal from", "Attempts to pick the lock of"
	};

	return;
		//TODO: this needs serious bug fixing skipping it for now
	
	
	if (!ch 
		|| !tch 
		|| IS_NPC (ch)	// ignore attacks by npcs 
		|| IS_SET (ch->flags, FLAG_GUEST) 
		|| ch->get_trust()	// ignore attacks by imms
		|| tch->get_trust()	// ignore attacks on imms
		)
	{

		return;

	}

	if ((cmd < 6 
		 && tch->mob 
		 && ((nMobVnum = tch->mob->nVirtual) == 1121	// birds
			 || nMobVnum == 3005	// snakes
			 || nMobVnum == 4000 ))	// chipmunks
		|| ((IS_NPC(tch) && IS_SET (tch->mob->profession, PROF_PREY)) && cmd >= 3))
	{
		
		return;
		
	}
	
	flag |= (!IS_NPC (tch)) ? (GUARDIAN_PC) : 0;
	flag |= (tch->race >= 0
		&& tch->race <= 29) ? (GUARDIAN_NPC_HUMANOIDS) : (GUARDIAN_NPC_WILDLIFE);
	flag |= (tch->mob->shop) ? (GUARDIAN_NPC_SHOPKEEPS) : 0;
	flag |= (IS_SET (tch->mob->profession, PROF_SENTINEL)) ? (GUARDIAN_NPC_SENTINELS) : 0;
	flag |= (IS_SET (tch->mob->profession, PROF_ENFORCER)) ? (GUARDIAN_NPC_ENFORCERS) : 0;
	flag |= ((tch->right_hand && tch->right_hand->obj_flags.type_flag == ITEM_KEY)
		|| (tch->left_hand
		&& tch->left_hand->obj_flags.type_flag ==
		ITEM_KEY)) ? (GUARDIAN_NPC_KEYHOLDER) : 0;

	if (ch->in_room == tch->in_room)
	{

		sprintf (buf, "#3[Guardian: %s%s]#0 %s %s%s in %d.",
			ch->name,
			IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
			strViolation[cmd],
			(!IS_NPC (tch)) ? tch->name : (tch->short_descr),
			(!IS_NPC (tch)
			&& IS_SET (tch->plr_flags,
			NEW_PLAYER_TAG)) ? " (new)" : (IS_SET (flag,
			GUARDIAN_NPC_KEYHOLDER)
			? " (keyholder)"
			: (IS_SET
			(flag,
			GUARDIAN_NPC_SHOPKEEPS)
			? " (shopkeeper)"
			: (IS_SET
			(flag,
			GUARDIAN_NPC_ENFORCERS)
			?
			" (enforcer)"
			: (IS_SET
			(flag,
			GUARDIAN_NPC_SENTINELS)
			?
			" (sentinel)"
			: "")))),
			tch->in_room);

	}
	else
	{

		sprintf (buf, "#3[Guardian: %s%s]#0 %s %s%s in %d, from %d.#0",
			ch->name,
			IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
			strViolation[cmd],
			(!IS_NPC (tch)) ? tch->name : (tch->short_descr),
			(!IS_NPC (tch)
			&& IS_SET (tch->plr_flags,
			NEW_PLAYER_TAG)) ? " (new)" : (IS_SET (flag,
			GUARDIAN_NPC_KEYHOLDER)
			? " (keyholder)"
			: (IS_SET
			(flag,
			GUARDIAN_NPC_SHOPKEEPS)
			? " (shopkeeper)"
			: (IS_SET
			(flag,
			GUARDIAN_NPC_ENFORCERS)
			?
			" (enforcer)"
			: (IS_SET
			(flag,
			GUARDIAN_NPC_SENTINELS)
			?
			" (sentinel)"
			: "")))),
			tch->in_room, ch->in_room);
	}
	buf[11] = toupper (buf[11]);
	send_to_guardians (buf, flag);

}



int
has_been_sighted (CHAR_DATA * ch, CHAR_DATA * target)
{
	SIGHTED_DATA *sighted;

	if (!ch || !target)
		return 0;

	if (ch->get_trust())
		return 1;

	if (IS_NPC (ch) && !ch->desc)
		return 1;			/* We know non-animated NPCs only acquire targets via SCANning; */
	/* don't need anti-twink code for them. */

	for (sighted = ch->sighted; sighted; sighted = sighted->next)
	{
		if (sighted->target == target)
			return 1;
	}

	return 0;
}


void
do_command (CHAR_DATA * ch, char *argument, int cmd)
{
	int everyone = 0;
	CHAR_DATA *victim = NULL;
	CHAR_DATA *next_in_room;
	CHAR_DATA *tch;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char command[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char ("Command whom?\n");
		return;
	}

	/* We can command invis chars as well, why not :) */

	if (!str_cmp (buf, "all") || !str_cmp (buf, "follower") ||
		!str_cmp (buf, "followers"))
		everyone = 1;

	else if (!(victim = get_char_room_vis (ch, buf)) &&
		!(victim = get_char_room (buf, ch->in_room)))
	{
		ch->send_to_char ("They aren't here.\n");
		return;
	}

	while (isspace (*argument))
		argument++;

	if (!*argument)
	{
		ch->send_to_char ("What is your command?\n");
		return;
	}

	strcpy (command, argument);

	if (victim == ch)
	{
		ch->send_to_char ("You don't have to give yourself commands.\n");
		return;
	}

	if (victim)
	{

		if (!is_higher_rank (ch, victim) && victim->following != ch)
		{
			ch->act("You do not have the authority to command $N.", false, 0,
				victim, TO_CHAR);
			return;
		}

		if (IS_NPC(victim) && IS_SET(victim->mob->action, ACT_NOCOMMAND))
		{
			ch->act("$N is not accepting orders at the moment.", false, 0, victim, TO_CHAR);
			return;
		}

		if (!IS_NPC(victim)) //cannot command PC - keeps them guessing who is PC and who is animated NPC
		{
			ch->act("$N is not accepting orders at the moment.", false, 0, victim, TO_CHAR);
			return;
		}
		
		sprintf (buf, "#5%s#0 commands you to '%s'.\n", ch->char_short(),
			command);
		buf[2] = toupper (buf[2]);
		victim->send_to_char (buf);


		if (victim->mob)
		{
			ch->send_to_char ("You give the command.\n");
			command_interpreter (victim, command);
		}
		else
		{
			sprintf (buf, "You command #5%s#0 to '%s'.\n",
				victim->char_short(), command);
			ch->send_to_char (buf);
		}

		return;
	}

	for (tch = ch->room->people; tch; tch = next_in_room)
	{

		next_in_room = tch->next_in_room;

		if (!is_higher_rank (ch, tch) && (tch->following != ch))
			continue;

		sprintf (buf, "#5%s#0 commands you to '%s'.\n", ch->char_short(),
			command);
		buf[2] = toupper (buf[2]);
		tch->send_to_char (buf);

		if (tch->mob)
			command_interpreter (tch, command);
	}

	ch->send_to_char ("You give the command.\n");
}




void
do_flee (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir;
	char direction_arg[MAX_STRING_LENGTH]= { '\0' };
	int aff_type = 0;
	
	if (!ch->can_move())
		return;
	
	
	if (get_affect (ch, MAGIC_AFFECT_PARALYSIS))
	{
		ch->send_to_char ("You are paralyzed and unable to flee!\n");
		return;
	}
	
	if (IS_SET (ch->flags, FLAG_FLEE))
	{
		ch->send_to_char ("You are already trying to escape!\n");
		return;
	}
	
	
	dir = -1;
	argument = one_argument (argument, direction_arg);
	
		//choose a direction to flee and remember if we are panic fleeing
	if (!*direction_arg)
	{
		ch->flags |= FLAG_FLEE;
		
		ch->act("$n's eyes dart about desperately looking for an escape path!",
			 false, 0, 0, TO_ROOM);
		magic_add_affect (ch, AFFECT_FLEE_ANY, 1, 0, 0, 0, 0);
		flee_attempt (ch, 0);  //0 means we don't know where we are fleeing yet
		return;
	}
	else 
		dir = index_lookup (dirs, direction_arg);
	
		//step 2 - can they go that way, and if they can, add affect
	if (dir == -1)
		{
			ch->send_to_char ("There is no such direction to run!\n");
			return;
		}
	
	else if (!CAN_GO (ch, dir))
	{
		ch->send_to_char ("You can't find escape that way!\n");
		return;
	}
	
	else 
	{
		switch (dir)
		{
			case 0:
				aff_type = AFFECT_FLEE_NORTH;
				break;
			case 1:
				aff_type = AFFECT_FLEE_EAST;
				break;
			case 2:
				aff_type = AFFECT_FLEE_SOUTH;
				break;
			case 3:
				aff_type = AFFECT_FLEE_WEST;
				break;
			case 4:
				aff_type = AFFECT_FLEE_NORTHEAST;
				break;
			case 5:
				aff_type = AFFECT_FLEE_SOUTHEAST;
				break;
			case 6:
				aff_type = AFFECT_FLEE_SOUTHWEST;
				break;
			case 7:
				aff_type = AFFECT_FLEE_NORTHWEST;
				break;
			case 8:
				aff_type = AFFECT_FLEE_UP;
				break;
			case 9:
				aff_type = AFFECT_FLEE_DOWN;
				break;
		}
		magic_add_affect (ch, aff_type, -1, 0, 0, 0, 0);
	}

	ch->flags |= FLAG_FLEE;
	
	ch->act("$n's eyes dart about looking for an escape path!",
		 false, 0, 0, TO_ROOM);
	
	flee_attempt (ch, 1); //1 to show we have a direction to flee
	
	return;
}

/* dir_flag == 0 --- we don't know where we are fleeing, we need to find a direction
 * dir_flag == 1 --- we know exactly where we are trying to flee.
 * if AFFECT_FLEE_ANY is used, then dirction will change in later 
 * calls to flee_attempt
 * affects are added to remember the direction if one is choosen
 * and this direction is used in later calls to flee_attempt
 */
int
flee_attempt (CHAR_DATA * ch, int dir_flag)
{
	int dir;
	int tdir;
	int index;
	int enem_test;
	int enemies = 0;
	int mobless_count = 0;
	int mobbed_count = 0;
	int mobless_dirs[6];
	int mobbed_dirs[6];
	CHAR_DATA *tch;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	ROOM_DATA *troom;
	
		
	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
				
		if (tch->get_position()!= STAND)
			continue;
		
		if (!can_see_mob(tch, ch))
			continue;
		
		enemies++;
	}
	
	
	for (tdir = 0; tdir <= LAST_DIR; tdir++)
	{
		
		if (!CAN_GO (ch, tdir))
			continue;
		
			//if it is not a normal portal, we can't flee that way
		if (!ch->room->dir_option[tdir])
			continue;
		
		if (vtor (is_exit(ch, tdir)->to_room)->people)
			mobbed_dirs[mobbed_count++] = tdir;
		else
			mobless_dirs[mobless_count++] = tdir;
	}
	
	
	
	if (!mobbed_count && !mobless_count)
	{
		ch->send_to_char ("There is nowhere to go!  You continue fighting!\n");
		ch->flags &= ~FLAG_FLEE;
		return 0;
	}
	
		
		//find a direction to flee in
	if (dir_flag == 0 && get_affect (ch, AFFECT_FLEE_ANY))
	{
		if (mobless_count)
			dir = mobless_dirs[number (0, mobless_count - 1)];
		else
			dir = mobbed_dirs[number (0, mobbed_count - 1)];
		
	}
	else
	{
		for(index = AFFECT_FLEE_NORTH;
			index <= AFFECT_FLEE_DOWN;
			index ++)
		{
			if (get_affect (ch, index))
				dir = index - 2150;
		}
	}
	
	enem_test = number (0, enemies);
	
	if (enemies && enem_test)
	{
		switch (number (1, 3))
		{
			case 1:
				ch->send_to_char ("You attempt escape, but fail this time . . .\n");
				break;
			case 2:
				ch->send_to_char ("You nearly escape, but are blocked this time . . .\n");
				break;
			case 3:
				ch->send_to_char ("You continue seeking escape . . .\n");
				break;
		}
		
		ch->act("$n nearly flees!", true, 0, 0, TO_ROOM);
		
		return 0;
	}

	
	troom = ch->room;
	
	/* stop_fighting_sounds (ch, troom); */
	ch->stop_followers();
	ch->following = ch;
	if (!IS_SET (ch->plr_flags, GROUP_CLOSED))
		ch->plr_flags |= GROUP_CLOSED;
		
	do_move (ch, "", dir);
	
	sprintf (buf, "$n #3flees %s!#0", dirs[dir]);
	ch->act(buf, false,  0, 0, TO_ROOM);
	
	sprintf (buf, "#3YOU FLEE %s!#0", dirs[dir]);
	ch->act(buf, false,  0, 0, TO_CHAR);
	
	if (!enemies)
		sprintf (buf, "\nYou easily escaped to the %s.\n", dirs[dir]);
	else
		sprintf (buf, "\nYou barely escaped to the %s.\n", dirs[dir]);
	ch->send_to_char (buf);
	
		//quit fleeing
	ch->flags &= ~FLAG_FLEE;
		
	
		//forget all directions we are trying to flee and forget panic flee
	for(index = AFFECT_FLEE_NORTH;
		index <= AFFECT_FLEE_ANY;
		index ++)
	{
		remove_affect_type (ch, index);
			
	}
	
	return 1;
}


