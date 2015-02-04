//////////////////////////////////////////////////////////////////////////////
//
/// group.cpp - Character grouping Functions
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
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#include "server.h"
#include "structs.h"
#include "utils.h"
#include "utility.h"
#include "group.h"
#include "protos.h"

extern rpie::server engine;
extern const char* dirs[];
extern const char* verbal_speeds[];

bool
is_with_group (CHAR_DATA * ch)
{
	CHAR_DATA *tch;

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (tch == ch)
			continue;

		if (tch->following == ch ||
			ch->following == tch ||
			(ch->following && tch->following == ch->following))
		{
			return 1;
		}
	}

	return 0;
}

void
do_follow (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	CHAR_DATA *leader = NULL;
	CHAR_DATA *tch = NULL;
	CHAR_DATA *orig_leader = NULL;

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char ("Follow whom?\n");
		return;
	}

	if (!(leader = get_char_room_vis (ch, buf)))
	{
		ch->send_to_char ("There is nobody here by that name.\n");
		return;
	}

	if ((!ch->get_trust()) && leader != ch
		&& IS_SET (leader->plr_flags, GROUP_CLOSED))
	{
		ch->send_to_char
			("That individual's group is currently closed to new followers.\n");
		ch->act("$n just attempted to join your group.", true, 0, leader,
			TO_VICT | _ACT_FORMAT);
		return;
	}

		
	if (leader != ch)
	{
		if (leader->following == ch)
		{
			ch->send_to_char
				("You'll need to ask them to stop following you first.\n");
			return;
		}
		orig_leader = leader;
		while (leader->following)
			leader = leader->following;
		
		if ((!ch->get_trust()) && leader != ch
			&& IS_SET (leader->plr_flags, GROUP_CLOSED))
		{
			ch->send_to_char
				("That individual's group is currently closed to new followers.\n");
			ch->act("$n just attempted to join your group.", true, 0, leader,
				TO_VICT | _ACT_FORMAT);
			return;
		}
		ch->following = leader;
		
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (tch->following == ch)
			{
				tch->following = leader;
				tch->act("You fall into stride with the group's new leader, $N.",
					false, 0, leader, TO_CHAR | _ACT_FORMAT);
			}
		}
		if (orig_leader != leader)
		{
			if (!IS_SET (ch->flags, FLAG_WIZINVIS))
				sprintf (buf,
				"You begin following #5%s's#0 group's current leader, $N.",
				orig_leader->char_short());
			else if (IS_SET (ch->flags, FLAG_WIZINVIS))
				sprintf (buf,
				"You will secretly follow #5%s#0's group's current leader, $N.",
				orig_leader->char_short());
		}
		else
		{
			if (!IS_SET (ch->flags, FLAG_WIZINVIS))
				sprintf (buf, "You begin following $N.");
			else if (IS_SET (ch->flags, FLAG_WIZINVIS))
				sprintf (buf, "You will secretly follow $N.");
		}
			//bag the pmote, so they can set a new one
		ch->clear_pmote();
		ch->act(buf, false,  0, ch->following, TO_CHAR | _ACT_FORMAT);
		sprintf (buf, "$n falls into stride with you.");
		if (!IS_SET (ch->flags, FLAG_WIZINVIS))
			ch->act(buf, false,  0, ch->following, TO_VICT | _ACT_FORMAT);
		sprintf (buf, "$n falls into stride with $N.");
		if (!IS_SET (ch->flags, FLAG_WIZINVIS))
			ch->act(buf, false,  0, ch->following, TO_NOTVICT | _ACT_FORMAT);
		return;
	}

	if (leader == ch && ch->following && ch->following != ch)
	{
		sprintf (buf, "You will no longer follow $N.");
		ch->act(buf, false,  0, ch->following, TO_CHAR | _ACT_FORMAT);
		sprintf (buf, "$n is no longer following you.");
		if (!IS_SET (ch->flags, FLAG_WIZINVIS)
			&& ch->room == ch->following->room)
			ch->act(buf, false,  0, ch->following, TO_VICT | _ACT_FORMAT);
		sprintf (buf, "$n stops following $N.");
		if (!IS_SET (ch->flags, FLAG_WIZINVIS)
			&& ch->room == ch->following->room)
			ch->act(buf, false,  0, ch->following, TO_NOTVICT | _ACT_FORMAT);
		
		ch->following = 0;
				
		return;
	}

	if (leader == ch && (!ch->following || ch->following == ch))
	{
		ch->send_to_char ("You aren't following anyone!\n");
		return;
	}

}


void
do_group (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch = NULL, *top_leader = NULL;
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	std::ostringstream buf;
	char arg[MAX_STRING_LENGTH]= { '\0' };
	char targ[MAX_STRING_LENGTH]= { '\0' };
	bool found = false;

	argument = one_argument (argument, arg);

	if (!(top_leader = ch->following))
	{
		top_leader = ch;
	}
	
	if (*arg)
	{
		if (is_abbrev (arg, "open"))
		{
			if (!IS_SET (ch->plr_flags, GROUP_CLOSED))
			{
				ch->send_to_char ("Your group is already open!\n");
				return;
			}
			ch->plr_flags &= ~GROUP_CLOSED;
			ch->send_to_char ("You will now allow people to follow you.\n");
			return;
		}

		else if (is_abbrev (arg, "speed"))
		{
			sprintf (buf2, "Your group travels at #2%s#0 when you travel.\n", verbal_speeds[speed_group(ch)]);
			ch->send_to_char (buf2);
			return;
		}

		else if (is_abbrev (arg, "close"))
		{
			if (IS_SET (ch->plr_flags, GROUP_CLOSED))
			{
				ch->send_to_char ("Your group is already closed!\n");
				return;
			}
			ch->plr_flags |= GROUP_CLOSED;
			ch->send_to_char ("You will no longer allow people to follow you.\n");
			return;
		}

		else if (is_abbrev (arg, "kick"))
		{
			if (!(top_leader == ch))
			{
				ch->send_to_char ("You are not the leader!\n");
				return;
			}	
			
			argument = one_argument (argument, targ);
			if (!targ)
			{
				ch->send_to_char ("Who do you wish to kick out of your group.\n");
				return;
			}
			tch = get_char_room_vis (ch, targ);
			
			if (!(tch) || !(is_with_group(tch)))
			{
				ch->send_to_char ("You don't see them here.\n");
				return;
			}
			
			if (tch == ch)
			{
				ch->send_to_char ("Sorry, you are stuck with being the leader.\n");
				return;
			}
			
			
			
			
			tch->following = 0;
			sprintf (buf2, "You have removed $N from the group.");
			ch->act(buf2, false, 0, tch, TO_CHAR);
			
			sprintf (buf2, "$N has removed you from the group.");
			tch->act(buf2, false, 0, ch, TO_CHAR);
			
			return;
			
		}
		
	}

	

	if (!top_leader)
	{
		ch->send_to_char ("You aren't in a group.\n");
		return;
	}

	buf << "#5" << top_leader->char_short() << "#0 leading: " << std::endl << std::endl;

	for (tch = top_leader->room->people; tch; tch = tch->next_in_room)
	{
		if (tch->following != top_leader)
		{
			continue;
		}

		if (found != false)
		{
			buf << "," << std::endl;
		}

		buf << "   #5" << tch->char_short() << "#0 ";
		found = true;
	}

	buf << "." << std::endl;

	if (!found)
	{
		ch->send_to_char ("You aren't in a group.\n");
		return;
	}

	ch->send_to_char (buf.str().c_str());
}

void
followers_follow (CHAR_DATA * ch, int dir, int leave_time, int arrive_time, int port_ident)
{
	CHAR_DATA *tch;

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (tch == ch || GET_FLAG (tch, FLAG_LEAVING))
			continue;

				
		if (tch->get_position()== SIT)
		{
			tch->act("You can't follow $N while sitting.",
				false, 0, ch, TO_CHAR);
			tch->clear_moves();
			return;
		}

		else if (tch->get_position()== REST)
		{
			tch->act("You can't follow $N while resting.",
				false, 0, ch, TO_CHAR);
			tch->clear_moves();
			return;
		}

		if(tch->delay_type == DEL_HIDE)
		{
			tch->act("You can't follow $N while looking for a place to hide.",
				false, 0, ch, TO_CHAR);
			tch->clear_moves();
			return;
		}	

		else if (tch->get_position()< STAND)
			return;

		if (get_affect (tch, MAGIC_HIDDEN) && (tch->skill_map["Sneak"]>1))
		{
			if (odds_sqrt (skill_level (tch, "Sneak", 0)) >= number (1, 100)
				|| !tch->would_reveal())
			{
				magic_add_affect (tch, MAGIC_SNEAK, -1, 0, 0, 0, 0);
			}
			else
			{
				remove_affect_type (tch, MAGIC_HIDDEN);
				tch->act("$n attempts to be stealthy.", true, 0, 0, TO_ROOM);
			}
		}

		move (tch, "", dir, leave_time + arrive_time, port_ident);
	}
}

void
follower_catchup (CHAR_DATA * ch)
{
	CHAR_DATA *tch;
	QE_DATA *qe;

	if (!ch->room)
		return;

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
		if (ch->following == tch)
			break;

	if (!tch || !GET_FLAG (tch, FLAG_LEAVING) || !can_see_mob(tch, ch))
		return;

	for (qe = quarter_event_list; qe->ch != tch; qe = qe->next)
		;

	if (!qe)
		return;

	
	if (get_affect (ch, MAGIC_HIDDEN) && (ch->skill_map["Sneak"] > 0))
	{
		if (odds_sqrt (skill_level (ch, "Sneak", 0)) >= number (1, 100)
			|| !ch->would_reveal())
		{
			magic_add_affect (ch, MAGIC_SNEAK, -1, 0, 0, 0, 0);
		}
		else
		{
			remove_affect_type (ch, MAGIC_HIDDEN);
			ch->act("$n attempts to be stealthy.", true, 0, 0, TO_ROOM);
		}
	}

	move (ch, "", qe->dir, qe->event_time + qe->arrive_time, 0);
}



int 
num_followers (CHAR_DATA * ch)
{
	CHAR_DATA		*top_leader = NULL;
	CHAR_DATA		*tch = NULL;
	int group_count = 0;  

	if (!(top_leader = ch->following))
		top_leader = ch;

	for (tch = top_leader->room->people; tch; tch = tch->next_in_room)
	{
		if (tch->following != top_leader)
			continue;
		if (!can_see_mob(ch, tch))
			continue;
		group_count = group_count + 1;

	}

	return (group_count);

}

float
group_light_level(CHAR_DATA* ch)
{
	CHAR_DATA *tch;
	float light_level = 0;
	
	light_level = char_light_carry(ch);
	
	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (tch->following == ch ||
			ch->following == tch ||
			(ch->following && tch->following == ch->following))
		{
			light_level += char_light_carry(tch);
		}
	}
	
	return (light_level);
}
int 
speed_group (CHAR_DATA * ch)
{
	CHAR_DATA		*top_leader = NULL;
	CHAR_DATA		*tch = NULL;
	int lowest_speed = 1;  //trudge
	
	if (!(top_leader = ch->following))
		top_leader = ch;
	
	lowest_speed = top_leader->speed;
	
	for (tch = top_leader->room->people; tch; tch = tch->next_in_room)
	{
		if (tch->following != top_leader)
			continue;
		
		if (!can_see_mob(ch, tch))
			continue;
	
			//walk = 0, crawl = 1, ... sprint = 5
			// 1...2...0...3...4...5
	
		if ((tch->speed < lowest_speed) && (tch->speed != 0) && (lowest_speed != 0))
			lowest_speed = tch->speed;
		else if ((lowest_speed == 0) && (tch->speed > 2))
			lowest_speed = 0;
		else if ((tch->speed == 0) && (lowest_speed > 2))
			lowest_speed = 0;
		else if ((tch->speed > lowest_speed) && (lowest_speed == 0))
			lowest_speed = tch->speed;
	}
	return (lowest_speed);
}


