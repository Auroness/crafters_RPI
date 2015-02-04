//////////////////////////////////////////////////////////////////////////////
//
/// guides.cpp - Routines for quasi-staff Guides
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
#include <ctype.h>
#include <string.h>

#include "structs.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"

CHAR_DATA *assist_queue = NULL;

void
do_assist (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch = NULL;
	int pos = 0, i = 0;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (!IS_GUIDE (ch) && (!ch->get_trust())
		&& !IS_SET (ch->plr_flags, NEW_PLAYER_TAG))
	{
		ch->send_to_char("Only Guides, staff members, and new characters may use this command.\n");
		return;
	}

	if (!str_cmp (argument, "request"))
	{
		if (str_cmp (ch->room->name, PREGAME_ROOM_NAME))
		{
			ch->send_to_char
				("This command may only be invoked in a pre-game debriefing room.\n");
			return;
		}
		if (!IS_SET (ch->plr_flags, NEW_PLAYER_TAG))
		{
			ch->send_to_char
				("The assist queue is only available to new characters.\n");
			return;
		}
		if ((pos = ch->get_queue_position()) > 0)
		{
			sprintf (buf,
				"You are already number #6%d#0 in the assist queue.\n",
				pos);
			ch->send_to_char(buf);
			return;
		}
		update_assist_queue (ch, false);
		ch->assist_pos = ch->get_queue_position();
		sprintf (buf, "You are now number #6%d#0 in the assist queue.\n",
			ch->get_queue_position ());
		ch->send_to_char(buf);
		return;
	}
	else if (!str_cmp (argument, "cancel"))
	{
		if ((pos = ch->get_queue_position()) == -1)
		{
			ch->send_to_char("You are not currently in the assist queue.\n");
			return;
		}
		update_assist_queue (ch, true);
		ch->send_to_char("You have been removed from the assist queue.\n");
		return;
	}
	else if (!str_cmp (argument, "list") || !str_cmp (argument, "queue"))
	{
		if (!IS_GUIDE (ch) && (!ch->get_trust()))
		{
			ch->send_to_char
				("Only Guides and staff members may see the assist queue.\n");
			return;
		}
		if (!assist_queue)
		{
			ch->send_to_char("The assist queue is currently empty.\n");
			return;
		}
		sprintf (buf, "#6Currently waiting in the assist queue:#0\n\n");
		for (tch = assist_queue; tch; tch = tch->next_assist)
		{
			i++;
			sprintf (buf + strlen (buf), "  %2d. %s\n", i, tch->pc->account_name);
		}
		ch->send_to_char(buf);
		return;
	}
	else if (!str_cmp (argument, "answer"))
	{
		if (!IS_GUIDE (ch) && (!ch->get_trust()))
		{
			ch->send_to_char
				("Only Guides and staff members may answer assist requests.\n");
			return;
		}
		if (IS_GUIDE (ch) && !IS_SET (ch->flags, FLAG_GUEST))
		{
			ch->send_to_char
				("You may only answer assist requests via your Guest login.\n");
			return;
		}
		if (!(tch = assist_queue))
		{
			ch->send_to_char
				("There is currently no-one waiting in the assist queue.\n");
			return;
		}
		tch->act("Your request for assistance has been answered!", true, 0, 0,
			TO_CHAR);

		if (tch->in_room != ch->in_room)
		{
			tch->send_to_char("\n");
			ch->act("$n vanishes in a subtle glimmer of light.", true, 0, 0,
				TO_ROOM | _ACT_FORMAT);
			ch->char_from_room();
			ch->char_to_room(tch->in_room);
			ch->act("$n appears in a subtle glimmer of light.", true, 0, 0,
				TO_ROOM | _ACT_FORMAT);
			ch->send_to_char("\n");
			ch->act
				("The world around you fades away in a glimmer of light and you feel yourself whisked instantly to $N's location.",
				false, 0, tch, TO_CHAR | _ACT_FORMAT);
			ch->send_to_char("\n");
		}
		else
			ch->send_to_char("\n");

		update_assist_queue (tch, true);

		ch->send_to_char("\n");

		do_look (ch, "", 0);

		return;
	}
	else if (!str_cmp (argument, "return"))
	{
		if (!IS_GUIDE (ch) || !IS_SET (ch->flags, FLAG_GUEST))
		{
			ch->send_to_char("This command is only for Guide guest logins.\n");
			return;
		}
		if (ch->in_room == OOC_LOUNGE)
		{
			ch->send_to_char("You're already in Club Endore!\n");
			return;
		}
		ch->act("$n vanishes in a subtle glimmer of light.", true, 0, 0,
			TO_ROOM | _ACT_FORMAT);
		ch->char_from_room();
		ch->char_to_room(OOC_LOUNGE);
		ch->act("$n appears in a subtle glimmer of light.", true, 0, 0,
			TO_ROOM | _ACT_FORMAT);
		ch->send_to_char("\n");
		ch->act
			("The world around you fades away in a glimmer of light and you feel yourself whisked instantly back to Club Endore.",
			false, 0, 0, TO_CHAR | _ACT_FORMAT);
		ch->send_to_char("\n");
		do_look (ch, "", 0);
		return;
	}
	else if (*argument)
	{
		ch->send_to_char("See #6HELP ASSIST#0 for command usage.\n");
		return;
	}

	if ((pos = ch->get_queue_position()) > 0)
	{
		sprintf (buf, "You are currently number #6%d#0 in the assist queue.\n",
			pos);
		ch->send_to_char(buf);
		return;
	}
	else
		ch->send_to_char("You are not currently in the assist queue.\n");

	return;
}


void
update_assist_queue (CHAR_DATA * ch, bool remove)
{
	CHAR_DATA *tch;
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int pos = 0;

	if (remove)
	{
		if (assist_queue == ch)
			assist_queue = ch->next_assist;
		else
			for (tch = assist_queue; tch && tch->next_assist;
				tch = tch->next_assist)
			{
				if (tch->next_assist == ch)
					tch->next_assist = tch->next_assist->next_assist;
			}

			ch->next_assist = NULL;

			sprintf (buf, "%s has been removed from the assist queue.",
				ch->pc->account_name);
			buf[2] = toupper (buf[0]);

			send_to_guides (buf);

			for (d = descriptor_list; d; d = d->next)
			{
				if (!d->character || d->connected != CON_PLYNG)
					continue;
				if ((pos = d->character->get_queue_position ()) > 0
					&& d->character != ch && pos < d->character->assist_pos)
				{
					sprintf (buf,
						"#6OOC:#0 You have advanced to position #6%d#0 in the assist queue.\n",
						pos);
					d->character->send_to_char(buf);
				}
				d->character->assist_pos = pos;
			}
	}
	else
	{
		if (!assist_queue)
			assist_queue = ch;
		else
			for (tch = assist_queue; tch; tch = tch->next_assist)
			{
				if (!tch->next_assist)
				{
					tch->next_assist = ch;
					ch->next_assist = NULL;
					break;
				}
			}
			sprintf (buf, "%s has been added to the assist queue.",
				ch->pc->account_name);
			buf[0] = toupper (buf[0]);

			send_to_guides (buf);
	}
}

	//TODO: need a new way to specify who a guide is - maybe a flag instead of forum membership?
/*                                                                          *
* function: is_guide                                                       *
*                                                                          */
int
is_guide (const char *username)
{
	
		
	if (!username || !*username)
		return 0;

        // Note: need to re-establish new Guide parameters with new forum system.
        return 0;
}
