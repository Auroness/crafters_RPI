//////////////////////////////////////////////////////////////////////////////
//
/// magic.cpp - Magic and Psionics
//
// TODO: Remove magic and psionics?
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
#include <time.h>

#ifndef MACOSX
	//#include <malloc.h>
#endif


#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "server.h"
#include "structs.h"
#include "net_link.h"
#include "account.h"
#include "utils.h"
#include "protos.h"
#include "utility.h"


extern rpie::server engine;
extern const char *dirs[];


int
is_direction (char *argument)
{
	int i = 0;

	for (i = 0; i <= LAST_DIR; i++)
	{
		if (!strn_cmp (dirs[i], argument, strlen (argument)))
			return i;
	}

	return -1;
}

void
update_target (CHAR_DATA * tch, int affect_type)
{
	if (affect_type == MAGIC_AFFECT_SLEEP)
	{
		do_sleep (tch, "", 0);
	}
}


void
do_accept (CHAR_DATA * ch, char *argument, int cmd)
{
	if ((ch->delay_type != DEL_APP_APPROVE &&
		ch->delay_type != DEL_INVITE &&
		ch->delay_type != DEL_PURCHASE_ITEM &&
		ch->delay_type != DEL_ORDER_ITEM ))
	{
		ch->send_to_char ("No transaction is pending.\n");
		return;
	}

	if (ch->delay_type == DEL_APP_APPROVE)
	{
		answer_application (ch, argument, 345);
		return;
	}
	else if (ch->delay_type == DEL_INVITE)
	{
		invite_accept (ch, argument);
		return;
	}
	else if (ch->delay_type == DEL_PURCHASE_ITEM)
	{
		do_buy (ch, "", 2);
		return;
	}
	else
		ch->send_to_char ("No transaction is pending.\n");
}

void
activate_refresh (CHAR_DATA * ch)
{
	ch->move = ch->max_move;
}


void
do_decline (struct char_data *ch, char *argument, int cmd)
{
	if (ch->delay_type != DEL_PURCHASE_ITEM && 
		ch->delay_type != DEL_APP_APPROVE && 
		ch->delay_type != DEL_INVITE)
	{
		ch->send_to_char ("No transaction is pending.\n");
		return;
	}

	if (ch->delay_type == DEL_INVITE)
	{
		ch->break_delay();
		return;
	}

	if (ch->delay_type == DEL_APP_APPROVE)
	{
		answer_application (ch, argument, 0);
		return;
	}

	if (ch->delay_type == DEL_PURCHASE_ITEM)
	{
		ch->break_delay();
		return;
	}

	if (ch->delay_who)
	{
		free_mem (ch->delay_who);
		ch->delay_who = NULL;
	}

	ch->delay = 0;
}

void
do_dreams (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	DREAM_DATA *dream;
	int count = 0;

	if (IS_NPC (ch))
	{
		ch->send_to_char ("This is a PC only command.\n");
		return;
	}

	if (!ch->pc->dreamed)
	{
		ch->send_to_char ("You don't recall anything memorable.\n");
		return;
	}

	argument = one_argument (argument, buf);

	for (dream = ch->pc->dreamed; dream; dream = dream->next)
		count++;

	if (!*buf)
	{
		sprintf (buf, "You recall having %d dream%s.\n",
			count, count > 1 ? "s" : "");
		ch->send_to_char (buf);
		return;
	}

	if (!isdigit (*buf))
	{
		ch->send_to_char ("Either type 'dreams' or 'dreams <number>'.\n");
		return;
	}

	if (strtol (buf, NULL, 10) > count || strtol (buf, NULL, 10) < 1)
	{
		ch->send_to_char ("That number doesn't correspond to a dream.\n");
		return;
	}

	count = strtol (buf, NULL, 10) - 1;

	for (dream = ch->pc->dreamed; count; count--)
		dream = dream->next;

	page_string (ch->desc, dream->dream);
}



void
post_dream (DESCRIPTOR_DATA * d)
{
	time_t current_time;
	char *date;
	DREAM_DATA *dream;
	CHAR_DATA *ch;

	ch = d->character->delay_ch;

	if (!d->pending_message->message)
	{
		d->character->send_to_char ("Dream aborted.\n");
		(d->character->delay_ch)->unload_pc();
		return;
	}

	dream = new DREAM_DATA;
	dream->dream = d->pending_message->message;
	dream->next = ch->pc->dreams;
	ch->pc->dreams = dream;
	
	current_time = time (0);

	date = (char *) asctime (localtime (&current_time));

	/* asctime adds a \n to the end of the date string - remove it */

	if (strlen (date) > 1)
		date[strlen (date) - 1] = '\0';

	add_message (1,		/* new message */
		ch->name,	/* PC board */
		-2,		/* Virtual # */
		(d->character)->name,	/* Imm name */
		date,
		"Entry via GIVEDREAM command.", "", ch->pc->dreams->dream, MF_DREAM);

}

void
do_givedream (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *who;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (argument, buf);

	if (!(who = load_pc (buf)))
	{
		ch->send_to_char ("No such PC.\n");
		return;
	}

	if (who->pc->create_state != 2)
	{
		ch->send_to_char ("PC is not in state 2.\n");
		who->unload_pc();
		return;
	}

	ch->make_quiet();

	free_mem(ch->desc->descStr);
	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;
	
	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;
	
	ch->delay_who = NULL;
	ch->delay_ch = who;
	
	ch->desc->proc = post_dream;
}



int
friendly (CHAR_DATA * ch, CHAR_DATA * friendPtr)
{
	
	if (friendPtr->following != ch)
		return 0;
	else {
		return 1;
	}

	
}


int 
apply_affect (CHAR_DATA * ch, int type, int duration, int power)
{
	AFFECTED_TYPE *af;

	if ((af = get_affect (ch, type)))
	{

		if (af->a.spell.duration == -1)
			return 1;

		if (af->a.spell.duration < duration)
			af->a.spell.duration = duration;


		return 1;
	}

	af = new AFFECTED_TYPE;
	af->next = NULL;
	af->type = type;
	af->a.spell.duration = duration;
	af->a.spell.modifier = power;
	af->a.spell.location = 0;
	af->a.spell.bitvector = 0;
	af->a.spell.t = 0;
	af->a.spell.sn = 0;

	affect_to_char (ch, af);

	return 0;
}

int
magic_add_affect (CHAR_DATA * ch, int type, int duration, int modifier,
				  int location, int bitvector, int sn)
{
	AFFECTED_TYPE *af;
	
	af = get_affect (ch, type);
	
	if (af)
	{
		
		if (af->a.spell.duration == -1)	/* Perm already */
			return 0;
		
		if (duration == -1)
			af->a.spell.duration = duration;
		else
			af->a.spell.duration += duration;
		
		af->a.spell.modifier = MAX (af->a.spell.modifier, modifier);
		return 0;
	}
	else
	{
		af = new AFFECTED_TYPE;
		af->next = NULL;
		af->type = type;
		af->a.spell.duration = duration;
		af->a.spell.modifier = modifier;
		af->a.spell.location = location;
		af->a.spell.bitvector = bitvector;
		af->a.spell.t = 0;
		af->a.spell.sn = sn;
		
		affect_to_char (ch, af);
		
		af = NULL;
		return 1;
	}
}
void
job_add_affect (CHAR_DATA * ch, int type, int days, int pay_date, int cash,
				int count, int object_vnum, int employer)
{
	AFFECTED_TYPE *af;

	if ((af = get_affect (ch, type)))
		affect_remove (ch, af);

	af = new AFFECTED_TYPE;

	af->type = type;
	af->a.job.days = days;			//days in normal period
	af->a.job.pay_date = pay_date;  //date he was paid last
	af->a.job.cash = cash;			//amount he gets paid
	af->a.job.count = count;
	af->a.job.object_vnum = object_vnum;
	af->a.job.employer = employer;

	affect_to_char (ch, af);
}

void
table_add_affect (CHAR_DATA * ch, OBJ_DATA * obj, int type)
{
	AFFECTED_TYPE *af;
	
	if ((af = get_affect (ch, type)))
		affect_remove (ch, af);
	
	af = new AFFECTED_TYPE;
	
	af->type = type;
	af->a.table.obj = obj;	
	affect_to_char (ch, af);
}

int
magic_add_obj_affect (OBJ_DATA * obj, int type, int duration, int modifier,
					  int location, int bitvector, int sn)
{
	AFFECTED_TYPE *af;

	if ((af = get_obj_affect (obj, type)))
	{
		af->a.spell.duration += duration;
		af->a.spell.modifier = MAX (af->a.spell.modifier, modifier);
		return 0;
	}

	af = new AFFECTED_TYPE;
	af->next = NULL;
	af->type = type;
	af->a.spell.duration = duration;
	af->a.spell.modifier = modifier;
	af->a.spell.location = location;
	af->a.spell.bitvector = bitvector;
	af->a.spell.sn = sn;

	affect_to_obj (obj, af);

	return 1;
}

void
magic_affect (CHAR_DATA * ch, int magic)
{
	switch (magic)
	{
	default:
		printf ("Unknown magic: %d\n", magic);
		break;
	}
}



