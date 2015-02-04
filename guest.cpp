//////////////////////////////////////////////////////////////////////////////
//
/// guest.cpp - Guest login generation and routines
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

#include "structs.h"
#include "net_link.h"
#include "account.h"
#include "protos.h"
#include "utils.h"

void
create_guest_avatar (DESCRIPTOR_DATA * d, char *argument)
{
	CHAR_DATA *ch = NULL;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char account_name[MAX_STRING_LENGTH]= { '\0' };
	char tname_buf[MAX_STRING_LENGTH]= { '\0' };
	int roll;
	extern int finished_booting;
	extern int guest_conns;

	if (!d)
		return;

	*buf = '\0';
	*buf2 = '\0';
	*account_name = '\0';
	*tname_buf = '\0';

	if (d->character)
		(d->character)->extract_char() ;

	sprintf (tname_buf, "%s-Guest", d->acct->name.c_str ());

	d->character = new_char (1);

	d->character->deleted = 0;

	d->original = NULL;

	ch = d->character;

	ch->color = d->acct->color;

	d->character->pc->create_state = STATE_APPROVED;
	d->prompt_mode = 1;
	d->character->desc = d;
	d->character->pc->owner = d;
	d->character->pc->load_count = 1;

		//TODO re-work this when we can go live for normal guests
		roll = number (1, 3);
	
	
	if (roll == 1)
		ch->race = lookup_race_id ("Stoor Hobbit");
	else if (roll == 2)
		ch->race = lookup_race_id ("Fallohide Hobbit");
	else if (roll == 3)
		ch->race = lookup_race_id ("Harfoot Hobbit");
	else
		ch->race = RACE_DEFAULT;
	
	if (ch->race == -1)
		ch->race = RACE_DEFAULT;
		
	/***
	else if (roll == 4)
		ch->race = 23;
	else if (roll == 5)
		ch->race = 120;
	else if (roll == 6)
		ch->race = 119;
	else if (roll == 7)
		ch->race = 89;
	else if (roll == 8)
		ch->race = 3;
	else if (roll == 9)
		ch->race = 28;
	else if (roll == 10)
		ch->race = 69;
	else if (roll == 11)
		ch->race = 64;
	 ***/
	

	d->character->race = ch->race;

	d->character->flags |= FLAG_GUEST;
	
	/* Bestow the random traits to the new guest avatar */
		//TODO re-enable this when we go live
		randomize_mobile (ch);

	ch->pc->account_name = duplicateString (d->acct->name.c_str ());

	if (is_guide (d->acct->name.c_str ()))
		ch->pc->is_guide = true;

	
	/* Address naming issues with our user's new account handle */

	ch->name = duplicateString (tname_buf);

	if (ch->pc->is_guide)
		sprintf (buf2, "Guide %s", ch->name);
	else
		sprintf (buf2, "%s", ch->name);

	
	if (ch->keywords)
	{
		sprintf (buf2 + strlen(buf2), " %s", ch->keywords);
		free_mem (ch->keywords);
	}
	
	ch->keywords = duplicateString(buf2);
	
	ch->hit = 100;
	ch->max_hit = 100;
	ch->move = 100;
	ch->max_move = 100;

		
	if (d->character->race >= 0 && d->character->race <= 120)
	{
		int	native_tongue = ch->get_native_tongue();
		if (native_tongue)
		{
			ch->skill_map[lookup_skill_name(native_tongue)] = calc_lookup (ch, REG_CAP, native_tongue);
		}
		ch->speaks = lookup_skill_name(native_tongue);
	}

	guest_conns++;

	if (ch->description)
		free_mem (ch->description);

	if (ch->pc->is_guide)
		ch->description =
		duplicateString
		("One of our friendly player #BGuides#0 is here, awaiting questions.\n");
	else
		ch->description =
		duplicateString
		("Another Guest is here, passing through. Be sure to welcome them!\n");

	ch->plr_flags |= NEWBIE_HINTS;

	if (ch->get_trust() == 1)
	{
		ch->flags |= FLAG_ISADMIN;
	}
	else if (ch->get_trust() > 1)
	{
		ch->flags |= FLAG_ISADMIN;
		ch->flags |= FLAG_WIZNET;
	}
	else
		ch->flags &= ~FLAG_ISADMIN;

		// TODO enable this once we have actual clothing for our people
	if (ch->race != 89 && ch->race != 69 && ch->race != 64)
		ch->equip_newbie();
	

	
	ch->hunger = -1;
	ch->thirst = -1;

	// If we're recreating, we're either recovering from a reboot or returning a dead
	// guest to the lounge, in which case we can skip a lot of this.

	ch->pc_to_game();

	ch->char_to_room(OOC_LOUNGE);

	if (str_cmp (argument, "recreate"))
	{
		(d->character)->act("$n is incarnated in a soft glimmer of light.", true, 			0, 0, TO_ROOM | _ACT_FORMAT);
		sprintf (buf, "%s [%s] has entered the lounge.", ch->name,
			ch->desc->strClientHostname);
		send_to_gods (buf);
		d->connected = CON_PLYNG;
		guest_conns++;
		sprintf (buf, "%s has logged in from %s as %s.",
			d->character->char_short(),
			d->strClientHostname,
			d->character->name);
		*buf = toupper (*buf);
		system_log (buf, false);
		mysql_safe_query
			("UPDATE newsletter_stats SET guest_logins=guest_logins+1");
		do_look (ch, "", 0);
	}
	else
	{
		if (finished_booting)	// Dead Guest being returned to the lounge.
			ch->act
			("$n appears in a sudden glimmer of light, looking slightly dazed.",
			true, 0, 0, TO_ROOM | _ACT_FORMAT);
		ch->act
			("You feel your form briefly waver before it solidifies into yet another new guise, returned safely to the pleasant confines of Club Endore.",
			false, 0, 0, TO_CHAR | _ACT_FORMAT);
	}
}
