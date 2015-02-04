//////////////////////////////////////////////////////////////////////////////
//
/// act.other.c : Miscellaneous Module 
//
//
// TODO: maybe this set of misc functions needs to go into class realted files
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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "server.h"
#include "structs.h"
#include "net_link.h"
#include "account.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "group.h"
#include "utility.h"

std::list<second_affect*> second_affect_list;
extern const char* dirs[];
extern const char* verbal_speeds[];
extern int second_affect_active;
extern rpie::server engine;
extern std::map<std::string, SUBCRAFT_HEAD_DATA*> craft_map;
extern std::list<zone_data*> zone_table;
extern std::map<std::string, SKILL_DATA*> skill_data_map;


typedef struct
{	int vnum;
	char* title;
} who_loc_info;


void
do_doitanyway (CHAR_DATA *ch, char * argument, int command)
{
	if (!ch->get_trust() && !IS_NPC(ch) && !command)
	{
		ch->send_to_char("You are not permitted to use this command.\n");
		return;
	}

	add_second_affect (SA_DOANYWAY, 1, ch, NULL, NULL, 0);
	command_interpreter(ch, argument);
	return;
}

void
do_commence (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch, *tch_next;
	DESCRIPTOR_DATA *td;
	int from_room = 0, to_room = 0;
	int commenced_in = 0; // 0 = gondor, 1 = mordor, 2 = haradrim, 3 = northman, 4 = orc, 5 = Balchoth
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (!IS_SET (ch->plr_flags, NEWBIE))
	{
		ch->send_to_char ("It appears that you've already begun play...\n");
		return;
	}

	if (IS_SET (ch->flags, FLAG_GUEST))
	{
		ch->send_to_char ("Sorry, but guests are not allowed into the gameworld.\n");
		return;
	}

	ch->act
		("$n decides to take the plunge, venturing off into the world for the very first time!",
		true, 0, 0, TO_ROOM | _ACT_FORMAT);

	sprintf (buf, "\n#6Welcome to %s!#0\n\n", MUD_NAME);
	ch->send_to_char (buf);

	if (ch->get_queue_position() != -1)
		update_assist_queue (ch, true);

	from_room = ch->in_room;


	int native_tongue = ch->get_native_tongue();
	
	if (native_tongue)
	{
		ch->skill_map[lookup_skill_name(native_tongue)] = calc_lookup (ch, REG_CAP, native_tongue);
	}

	
		//for players starting in base area
	ch->char_from_room();
	ch->char_to_room(START_LOC);
	ch->last_room = 0;

	// Grommit - Boost higher races in Gondor to 50 westron
	if (ch->skill_map["Westron"] < 50 && ch->race > 0 )
	{
		ch->skill_map["Westron"] = 50 ;
	}
		//end of player location changes

	ch->plr_flags &= ~NEWBIE;
	ch->time_str.played = 0;
	save_char (ch, true);
	
	ch->act("$n has entered Middle-earth for the very first time!", true, 0, 0,
		TO_ROOM | _ACT_FORMAT);

	switch (commenced_in)
	{
	case 0:
		sprintf (buf, "#3[%s has entered Middle-earth for the first time in Angrenost.]#0", ch->char_short());
		break;
		
	}

	for (td = descriptor_list; td; td = td->next)
	{
		if (!td->character || td->connected != CON_PLYNG)
			continue;
		if (!is_brother (ch, td->character))
			continue;
		if (IS_SET (td->character->plr_flags, MENTOR))
		{
			ch->act(buf, true,  0, td->character, TO_VICT | _ACT_FORMAT);
		}
	}

	// Delete auto-genned individual debriefing area, if applicable.

	if (from_room >= 100000 
		&& vtor (from_room) 
		&& !str_cmp (vtor (from_room)->name, PREGAME_ROOM_NAME))
	{

		for (tch = (vtor (from_room))->people; tch; tch = tch_next)
		{
			tch_next = tch->next_in_room;
			if (!IS_NPC (tch))
			{
				tch->char_from_room();
				if ((to_room = tch->last_room) && vtor (to_room))
					tch->char_to_room(to_room);
				else
					tch->char_to_room(OOC_LOUNGE);
			}
		}

		delete_contiguous_rblock (vtor (from_room), -1, -1);
	}

	do_look (ch, NULL, 0);
}

void
do_quit (CHAR_DATA * ch, char *argument, int cmd)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH]= { '\0' };
	CHAR_DATA *tch;
	DESCRIPTOR_DATA *d;
	std::list<CHAR_DATA*>::iterator tch_iterator;
	
	if (ch->desc && ch->desc->original)
	{
		ch->send_to_char ("Not while you are switched.  RETURN first.\n");
		return;
	}

	if (ch->is_switched())
		return;

	if (IS_NPC (ch))
	{
		ch->send_to_char ("NPC's cannot quit.\n");
		return;
	}

	argument = one_argument (argument, arg);

	if (!IS_SET (ch->flags, FLAG_GUEST)
		&& (!ch->get_trust())
		&& !IS_SET (ch->room->room_flags, SAFE_Q) 
		&& ch->desc 
		&& cmd != 3)
	{
		ch->send_to_char("You may not quit in this area. Most inns, taverns, and so forth are\n"
			"designated as safe-quit areas; if you are in the wilderness, you may\n"
			"also use the CAMP command to create a safe-quit room. #1If you simply\n"
			"drop link without quitting, your character will remain here, and may\n"
			"be harmed or injured while you are away. We make no guarantees!#0\n");
		return;
	}

	
	if (!IS_NPC (ch) 
		&& ch->pc->edit_player)
	{
		(ch->pc->edit_player)->unload_pc();
		ch->pc->edit_player = NULL;
	}

	clear_player_from_second_affects(ch);
	remove_affect_type (ch, MAGIC_SIT_TABLE);
	
	
	ch->act("Goodbye, friend.. Come back soon!", false, 0, 0, TO_CHAR);
	ch->act("$n leaves the area.", true, 0, 0, TO_ROOM);

	sprintf (buf, "%s has left the game.\n", ch->name);

	if (!ch->pc->admin_loaded)
		send_to_gods (buf);

	d = ch->desc;

	sprintf (buf, "%s is quitting.  Saving character.", ch->name);
	system_log (buf, false);

	ch->pc->last_logoff = time (0);


	ch->extract_char();

	if (!ch)
		return;

	if (d)
		d->character = NULL;
	else 
		return;

	
	if (d && (!d->acct || str_cmp(d->acct->name.c_str (), "Guest") == 0))
	{
		close_socket (d);
		return;
	}

	d->connected = CON_ACCOUNT_MENU;
	nanny (d, "");
}

void
do_save (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH];

	
	if (IS_NPC (ch))
		return;

	sprintf (buf, "Saving %s.\n", ch->name);
	ch->send_to_char (buf);
	save_char (ch, true);
}

void
do_sneak (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	int dir;

	if (IS_SET (ch->room->room_flags, OOC) && (!ch->get_trust()))
	{
		ch->send_to_char ("You cannot do this in an OOC area.\n");
		return;
	}

	if (ch->skill_map["Sneak"] < 1)
	{
		ch->send_to_char ("You just aren't stealthy enough to try.\n");
		return;
	}

	if (ch->is_subduer())
	{
		ch->send_to_char ("You can't sneak while you have someone in tow.\n");
		return;
	}

	if (ch->is_encumbered())
	{
		ch->send_to_char ("You are too encumbered to sneak.\n");
		return;
	}

	
	if (ch->speed == SPEED_JOG ||
		ch->speed == SPEED_RUN || ch->speed == SPEED_SPRINT)
	{
		sprintf (buf, "You can't sneak and %s at the same time.\n",
			verbal_speeds[ch->speed]);
		ch->send_to_char (buf);
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf)
	{

		
		if (IS_NPC (ch) && IS_SET (ch->affected_by, AFF_SNEAK))
		{
			sprintf (buf, "%s", dirs[cmd]);
		}
		else  //send a message to an NPC? if they are animated, maybe it works.
		{
			ch->send_to_char ("Sneak in what direction?\n");
			return;
		}

	}

	if ((dir = index_lookup (dirs, buf)) == -1)
	{
		ch->send_to_char ("Sneak where?\n");
		return;
	}

	
	// Heads up to the player sneaking into a no-hide room,
	// as long as they can see inside
	ROOM_DATA * dest;
	 
	if (ch->room->dir_option[dir])
		dest = vtor(ch->room->dir_option[dir]->to_room);
	
	if (dest 
		&& IS_SET (dest->room_flags, NOHIDE) 
		&& *argument != '!'
		&& (!is_dark (dest)
		|| get_affect (ch, MAGIC_AFFECT_INFRAVISION)
		|| IS_SET (ch->affected_by, AFF_INFRAVIS)))
	{
		char message [AVG_STRING_LENGTH] = "";
		sprintf (message, "   As you quietly approach the area ahead, "
			"you notice that there \nare no hiding places available.  "
			"If you wish to sneak out of this \narea into the one "
			"ahead, then: #6 sneak %s !#0\n", buf);
		ch->send_to_char (message);
		return;
	}

		//count number of non_friendly viewers in the room who could see the sneaker
	int population = 0;
	CHAR_DATA* tmp_ch;
	
	for (tmp_ch = ch->room->people; tmp_ch;
		 tmp_ch = tmp_ch->next_in_room)
	{
		if (tmp_ch != ch && can_see_mob(ch, tmp_ch)
			&& !are_grouped (tmp_ch, ch))
		{
			population += 1;
		}
	}

	
	if (((population > 2) && skill_use (ch, "Sneak", population*5))
		|| !ch->would_reveal())
	{
		magic_add_affect (ch, MAGIC_SNEAK, -1, 0, 0, 0, 0);
		ch->send_to_char("You think you can be stealthy while moving.\n");
		
	}
	else
	{
		remove_affect_type (ch, MAGIC_HIDDEN);
		ch->act("$n attempts to be stealthy.", true, 0, 0, TO_ROOM);
		ch->send_to_char("You think you can be stealthy while moving.\n");
	}

	do_move (ch, argument, dir);
}

void
do_hood (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *obj;

	if (!(obj = get_equip (ch, WEAR_NECK_1))
		|| (!IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)))
	{

		if (!(obj = get_equip (ch, WEAR_NECK_2))
			|| (!IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)))
		{

			if (!(obj = get_equip (ch, WEAR_ABOUT))
				|| (!IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)))
			{
				ch->send_to_char ("You are not wearing a hooded item properly.\n");
				return;
			}
		}
	}

	if (!IS_SET (ch->affected_by, AFF_HOODED))
	{

		ch->act("You raise $p's hood, obscuring your face.", false, obj, 0,
			TO_CHAR | _ACT_FORMAT);

		ch->act("$n raises $p's hood, concealing $s face.",
			false, obj, 0, TO_ROOM | _ACT_FORMAT);

		ch->affected_by |= AFF_HOODED;

		return;
	}

	ch->affected_by &= ~AFF_HOODED;

	ch->act("You lower $p's hood, revealing your features.", false, obj, 0,
		TO_CHAR | _ACT_FORMAT);
	ch->act("$n lowers $p's hood, revealing $s features.", false, obj, 0,
		TO_ROOM | _ACT_FORMAT);

	return;
}


void
do_hide (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *obj;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	AFFECTED_TYPE *af;
	CHAR_DATA *tch;

	if (IS_SET (ch->room->room_flags, OOC) && (!ch->get_trust()))
	{
		ch->send_to_char ("You cannot do this in an OOC area.\n");
		return;
	}

	if (ch->skill_map["Hide"] < 1)
	{
		ch->send_to_char ("You lack the skill to hide.\n");
		return;
	}

	if (IS_SET (ch->room->room_flags, NOHIDE))
	{
		ch->send_to_char ("This room offers no hiding spots.\n");
		return;
	}

	if (ch->delay_type == DEL_HIDE)
	{
		ch->send_to_char ("You are already trying to hide.\n");
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		if (ch->is_encumbered())
		{
			ch->send_to_char ("You are too encumbered to hide.\n");
			return;
		}

		if (get_affect(ch, MAGIC_HIDDEN))
		{
			ch->send_to_char ("You are already hidden.\n");
			return;
		}

		ch->send_to_char ("You start trying to conceal yourself.\n");
		ch->act("$n starts looking for a place to hide.", false, 0, 0, TO_ROOM);
		ch->delay_type = DEL_HIDE;
		ch->delay = 5;
	}

	if (*buf)
	{
		if (!(obj = get_obj_in_dark (ch, buf, ch->right_hand)) && !(obj = get_obj_in_dark (ch, buf, ch->left_hand)))
		{
			ch->send_to_char ("You don't have that.\n");
			return;
		}

		if (!get_obj_in_list_vis (ch, buf, ch->right_hand) && !get_obj_in_list_vis (ch, buf, ch->left_hand))
		{
			ch->act("It is too dark to hide it.", false, 0, 0, TO_CHAR);
			return;
		}

		

		if (!get_affect(ch, MAGIC_HIDDEN))
			ch->act("$n begins looking for a place to hide $p.", false, obj, 0, TO_ROOM | _ACT_FORMAT);

		ch->delay_type = DEL_HIDE_OBJ;
		ch->delay_info1 = (long int) obj;
		ch->delay = 5;
		return;
	}
}




/**********************************************************************
* CASE 1:
* Usage: palm <item>
*        will get item from room.
*             Uses Sleight
*
* CASE 2:
* Usage: palm <item> into <container>
*        will put item into container in the room, including tables
*              Uses Sleight
*
* CASE 3:
* Usage: palm <item> into <targetPC> <contaienr>
*        will put item into container worn by PC (target or self)
*             Uses Steal if PC is target
*             Uses Sleight if PC is self
*
* CASE 4:
* Usage: palm <item> from <container>
*        takes item from container worn by PC
*             Uses Sleight
*
* CASE 5:
* Usage: palm <item> from <targetPC> <container>
*        takes item from container worn by PC
*             Uses Sleight if PC is self
*
* Note: Must use STEAL command if taking from a container worn
* by targetPC.
*
**********************************************************************/

void
do_palm (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char contbuf[MAX_STRING_LENGTH]= { '\0' };
	char msgbuf[MAX_STRING_LENGTH]= { '\0' };
	char objtarget[MAX_STRING_LENGTH]= { '\0' };
	CHAR_DATA *tch;
	OBJ_DATA *cobj = NULL;  //container
	OBJ_DATA *tobj = NULL;  //item being palmed
	AFFECTED_TYPE *af;
	int modifier = 0;
	bool into = false;
	bool from = false;
	bool targchk = false;  //is there a target PC?
	bool contchk = false;  //is there a container?



	if (IS_SET (ch->room->room_flags, OOC) && (!ch->get_trust()))
	{
		ch->send_to_char ("You cannot do this in an OOC area.\n");
		return;
	}

	if ((ch->skill_map["Steal"] < 1) && (ch->skill_map["Sleight"] < 1))
	{
		ch->send_to_char ("You lack the skill to palm objects.\n");
		return;
	}


	argument = one_argument (argument, objtarget);

	if (!*objtarget)
	{
		ch->send_to_char ("What did you wish to palm?\n");
		return;
	}

	if (ch->right_hand && ch->left_hand)
	{
		ch->send_to_char("One of your hands needs to be free before attempting to palm something.\n");
		return;
	}

	argument = one_argument (argument, buf); //into or from

	if (!*buf)
	{
		/****** CASE 1: palm <item> (from room) *******/
		if (!(tobj = get_obj_in_list_vis (ch, objtarget, ch->room->contents)))
		{
			ch->send_to_char ("You don't see that here.\n");
			return;
		}

		if (tobj->obj_flags.weight / 100 > 3)
		{
			ch->send_to_char ("That's too heavy for you to pick up very stealthily.\n");
			return;
		}

		if (!IS_SET (tobj->obj_flags.wear_flags, ITEM_TAKE))
		{
			ch->send_to_char ("That cannot be picked up.\n");
			return;
		}

		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			
			if (!skill_use (ch, "Sleight", tobj->obj_flags.weight / 100) || skill_use (tch, "Scan", 20))
			{
				ch->act("You try to palm $p, but are spotted by $N before you can get your hands on it!", true, tobj, tch, TO_CHAR | _ACT_FORMAT);
				ch->act("You catch $n trying to take $p beneath your notice, but stop the rogue in the act!", true, tobj, tch, TO_VICT | _ACT_FORMAT);
				ch->act("$N catches $n trying to take $p beneath $S notice, but prevents them from doing so.", true, tobj, tch, TO_NOTVICT | _ACT_FORMAT);
				return;
			}
		}

		obj_from_room (&tobj, 0);
		clear_omote (tobj);
		ch->act("You carefully attempt to palm $p.", false,  tobj, 0,
			TO_CHAR | _ACT_FORMAT);

		/* Alert the staff of the theft */
		sprintf (msgbuf, "#3[Guardian: %s%s]#0 Tries to palm %s in %d.",
			ch->name,
			IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
			tobj->short_description,
			ch->in_room);
		send_to_guardians (msgbuf, 0xFF);

		if (!skill_use (ch, "Sleight", tobj->obj_flags.weight / 100))
		{
			ch->act("$n attempts to surreptitiously take $p.", false,  tobj, 0, TO_ROOM | _ACT_FORMAT);
		}

		obj_to_char (tobj, ch);
		return;
	} //CASE 1:


	/****** CASE 2, 3, 4 & 5:
	palm <item> [from | into] [target?] <container>
	*****/
	if (!str_cmp (buf, "into"))
	{
		//container/target name
		argument = one_argument (argument, contbuf);
		into = true;
	}
	else if (!str_cmp (buf, "from"))
	{
		//container/target name
		argument = one_argument (argument, contbuf);
		from = true;
	}
	else
	{
		if (!from & !into)
		{
			ch->send_to_char ("Do you wish to palm INTO or FROM a container?\n");
			return;
		}
	}

	if (!(tch = get_char_room_vis (ch, contbuf))) //contbuf is not PC, so it may be a container CASE 2 or 4
	{
		if (!(cobj = get_obj_in_list (contbuf, ch->equip)) && !(cobj = get_obj_in_list_vis (ch, contbuf, ch->room->contents)))
		{
			ch->send_to_char ("You don't see that here.\n");
			return;
		}

		if (cobj == get_obj_in_list (contbuf, ch->equip))
		{
			tch =  get_char_room_vis (ch, "self");
			targchk = true; //self is the target PC CASE 3 & 5
			contchk = true;
		}

		if (cobj == get_obj_in_list_vis (ch, contbuf, ch->room->contents))
		{
			targchk = false;
			contchk = true;
		}

		if (cobj->obj_flags.type_flag != ITEM_CONTAINER)
		{
			ch->send_to_char ("You can only palm items into or from containers.\n");
			return;
		}

	}
	else
	{
		targchk = true; //there is a target PC CASE 3 & 5
	}

	/*** CASE 2: palm <item> into <container> ***/
	// item is tobj
	// cobj is container
	if (into && !targchk && contchk)
	{
		if (!(tobj = get_obj_in_list (objtarget, ch->right_hand))
			&& !(tobj = get_obj_in_list (objtarget, ch->left_hand)))
		{
			ch->send_to_char ("What did you wish to palm into it?\n");
			return;
		}

		if (tobj->obj_flags.weight / 100 > 3)
		{
			ch->send_to_char ("That's too heavy for you to palm very stealthily.\n");
			return;
		}

		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			
			if (!skill_use (ch, "Sleight", tobj->obj_flags.weight / 100) || skill_use (tch, "Scan", 20))
			{
				sprintf(buf, "You try to palm $p into $P, but are spotted by #5%s#0 before you can get to it!", tch->char_short());
				ch->act(buf, true,  tobj, cobj, TO_CHAR | _ACT_FORMAT);
				ch->act("You catch $n trying to get near $p beneath your notice, but stop the rogue in the act!", true, tobj, tch, TO_VICT | _ACT_FORMAT);
				ch->act("$N catches $n trying to get near $p beneath $S notice, but prevents them from doing so.", true, tobj, tch, TO_NOTVICT | _ACT_FORMAT);
				return;
			}
		}

		//Treat tables as a special container
		if (!IS_SET (cobj->obj_flags.extra_flags, ITEM_TABLE))
		{
			ch->act("You carefully slide $p onto $P.", false,  tobj,
				cobj, TO_CHAR | _ACT_FORMAT);

			/* Alert the staff of the theft */
			sprintf (msgbuf,
				"#3[Guardian: %s%s]#0 Tries to slip %s onto %s in %d.",
				ch->name,
				IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
				tobj->short_description,
				cobj->short_description,
				ch->in_room);
			send_to_guardians (msgbuf, 0xFF);

			if (!skill_use
				(ch, "Sleight", tobj->obj_flags.weight / 100))
				ch->act("$n attempts to surreptitiously place $p atop $P.",
				false,  tobj, cobj, TO_ROOM | _ACT_FORMAT);
		}
		else
		{
			ch->act("You carefully slide $p into $P.", false,  tobj,
				cobj, TO_CHAR | _ACT_FORMAT);

			/* Alert the staff of the theft */
			sprintf (msgbuf,
				"#3[Guardian: %s%s]#0 Tries to slip %s into %s in %d.",
				ch->name,
				IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
				tobj->short_description,
				cobj->short_description,
				ch->in_room);
			send_to_guardians (msgbuf, 0xFF);

			if (!skill_use
				(ch, "Sleight", tobj->obj_flags.weight / 100))
				ch->act("$n attempts to surreptitiously place $p into $P.",
				false,  tobj, cobj, TO_ROOM | _ACT_FORMAT);
		}

		//transfer any special effects
		for (af = tobj->xaffected; af; af = af->next)
			affect_modify (ch, af->type, af->a.spell.location,
			af->a.spell.modifier, tobj->obj_flags.bitvector,
			false, 0);

		if (ch->right_hand == tobj)
			ch->right_hand = NULL;
		else if (ch->left_hand == tobj)
			ch->left_hand = NULL;
		obj_to_obj (tobj, cobj);
		return;
	}//CASE 2

	/*** CASE 3: palm <item> into <targetPC> <container> ***/
	// item is tobj
	// tch is targetPC
	// cobj is container
	if (into && targchk)
	{
		argument = one_argument (argument, contbuf);

		if (!contchk)
		{
			if (!(cobj = get_obj_in_list (contbuf, tch->equip)))
			{
				ch->send_to_char ("You don't see that container.\n");
				return;
			}

			if (cobj->obj_flags.type_flag != ITEM_CONTAINER)
			{
				ch->send_to_char ("You can only palm items into containers.\n");
				return;
			}
		}

		if (!(tobj = get_obj_in_list (objtarget, ch->right_hand))
			&& !(tobj = get_obj_in_list (objtarget, ch->left_hand)))
		{
			ch->send_to_char ("What did you wish to palm into it?\n");
			return;
		}

		if (tobj->obj_flags.weight / 100 > 3)
		{
			ch->send_to_char ("That's too heavy for you to palm very stealthily.\n");
			return;
		}

		if (tch == ch)
		{
			ch->act("You carefully slide $p into $P.", false,  tobj, cobj, TO_CHAR | _ACT_FORMAT);

			/* Alert the staff of the theft */
			sprintf (msgbuf,
				"#3[Guardian: %s%s]#0 Tries to secretly place %s into %s in %d.",
				ch->name,
				IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
				tobj->short_description,
				cobj->short_description,
				ch->in_room);
			send_to_guardians (msgbuf, 0xFF);


			if (!skill_use (ch, "Sleight", tobj->obj_flags.weight / 100))
				ch->act("$n attempts to surreptitiously manipulate $p.",
				false,  tobj, 0, TO_ROOM | _ACT_FORMAT);


		}
		else
		{
			modifier = tch->skill_map["Scan"] / 5;
			modifier += tobj->obj_flags.weight / 100;
			modifier += 15;

			sprintf (msgbuf, "You carefully slide $p into #5%s#0's $P.",
				 tch->char_short());
			ch->act(msgbuf, false,  tobj, cobj, TO_CHAR | _ACT_FORMAT);

			/* Alert the staff of the theft */
			sprintf (msgbuf, "#3[Guardian: %s%s]#0 Plants %s on %s in %d.",
				ch->name,
				IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
				tobj->short_description,
				(!IS_NPC (tch)) ? tch->name : (tch->short_descr),
				ch->in_room);
			send_to_guardians (msgbuf, 0xFF);

			if (!skill_use (ch, "Steal", modifier))
			{
				sprintf (msgbuf,
					"$n approaches you and surreptitiously slips $p into #2%s#0!",
					obj_short_desc (cobj));
				ch->act(msgbuf, false,  tobj, tch, TO_VICT | _ACT_FORMAT);

				sprintf (msgbuf,
					"$n approaches $N and surreptitiously slips $p into #2%s#0.",
					obj_short_desc (cobj));
				ch->act(msgbuf, false,  tobj, tch, TO_NOTVICT | _ACT_FORMAT);
			}
		}

		for (af = tobj->xaffected; af; af = af->next)
			affect_modify (ch, af->type, af->a.spell.location,
			af->a.spell.modifier, tobj->obj_flags.bitvector,
			false, 0);

		if (ch->right_hand == tobj)
			ch->right_hand = NULL;
		else if (ch->left_hand == tobj)
			ch->left_hand = NULL;
		obj_to_obj (tobj, cobj);
		return;
	} //CASE 3

	/*** CASE 4: palm <item> from <container> ***/
	// item is tobj
	// cobj is container
	if (from && !targchk && contchk)
	{
		if (!(tobj = get_obj_in_list_vis (ch, objtarget, cobj->contains)))
		{
			ch->send_to_char ("You don't see such an item in that container.\n");
			return;
		}

		if (tobj->obj_flags.weight / 100 > 3)
		{
			ch->send_to_char ("That's too heavy for you to palm very stealthily.\n");
			return;
		}

		if (ch->right_hand && ch->left_hand)
		{
			ch->send_to_char("One of your hands needs to be free before attempting to palm something.\n");
			return;
		}

		ch->act("You carefully flick $p from $P into your hand.", false,
			tobj, cobj, TO_CHAR | _ACT_FORMAT);

		/* Alert the staff of the theft */
		sprintf (msgbuf,
			"#3[Guardian: %s%s]#0 Tries to secretly draw %s in %d.",
			ch->name,
			IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
			tobj->short_description,
			ch->in_room);
		send_to_guardians (msgbuf, 0xFF);

		if (!skill_use (ch, "Sleight", tobj->obj_flags.weight / 100))
			ch->act("$n attempts to handle $p surreptitiously.", false, 
			tobj, 0, TO_ROOM | _ACT_FORMAT);


		obj_from_obj (&tobj, 0, cobj); //rmoves tobj from wherever it is
		obj_to_char (tobj, ch);
		return;

	}// CASE 4


	/*** CASE 5: palm <item> from <targetPC> <container> ***/
	// item is tobj
	// tch is targetPC
	// cobj is container

	if (from && targchk)
	{
		argument = one_argument (argument, contbuf);

		if (!contchk)
		{
			if (!(cobj = get_obj_in_list (contbuf, tch->equip)))
			{
				ch->send_to_char("What did you wish to palm from?\n");
				return;
			}

			if (cobj->obj_flags.type_flag != ITEM_CONTAINER)
			{
				ch->send_to_char ("You can only palm items from containers.\n");
				return;
			}
		}

		if (!(tobj = get_obj_in_list_vis (ch, objtarget, cobj->contains)))
		{
			ch->send_to_char ("You don't see such an item in that container.\n");
			return;
		}

		if (tobj->obj_flags.weight / 100 > 3)
		{
			ch->send_to_char ("That's too heavy for you to palm very stealthily.\n");
			return;
		}

		if (ch->right_hand && ch->left_hand)
		{
			ch->send_to_char("One of your hands needs to be free before attempting to palm something.\n");
			return;
		}

		if (tch == ch)
		{
			ch->act("You carefully attempt to palm $p from $P.", false, 
				tobj, cobj, TO_CHAR | _ACT_FORMAT);

			/* Alert the staff of the theft */
			sprintf (msgbuf,
				"#3[Guardian: %s%s]#0 Tries to palm %s from %s in %d.",
				ch->name,
				IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? " (new)" : "",
				tobj->short_description,
				cobj->short_description,
				ch->in_room);
			send_to_guardians (msgbuf, 0xFF);

			if (!skill_use (ch, "Sleight", tobj->obj_flags.weight / 100))
				ch->act("$n gets $p from $P.", false,  tobj, cobj, 		     TO_ROOM | _ACT_FORMAT);

			obj_from_obj (&tobj, 0, cobj); //removes tobj from wherever it is
			obj_to_char (tobj, ch);
		} //tch = ch
		else
		{
			//must use the steal command. Besides, you can't see the item in the other guys container
			ch->send_to_char ("You can't see into that container.\n");
			return;
		}
		return;
	} // CASE 5

	return;
}//end function


const char *
get_room_desc_tag (CHAR_DATA * ch, ROOM_DATA * room)
{
	if (!room->extra
		|| desc_weather[room->wzone] == WR_NORMAL
		|| room->extra->weather_desc[desc_weather[room->wzone]].empty())
	{

		if (room->extra 
			&& !(room->extra->weather_desc[WR_NIGHT].empty()) 
			&& (global_sun_light <= SUN_TWILIGHT))
		{

			return weather_room[WR_NIGHT];
		}
		else
		{

			return NULL;

		}

	}
	else
	{
		return weather_room[desc_weather[room->wzone]];
	}
}

void
post_typo (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *ch;
	char msg[MAX_STRING_LENGTH]= { '\0' };
	char *date;
	char *date2;
	const char *wr;
	time_t account_time;

	account_time = d->acct->created_on;
	date = (char *) asctime (localtime (&account_time));

	account_time = time (0);
	date2 = (char *) asctime (localtime (&account_time));

	date2[strlen (date2) - 1] = '\0';

	ch = d->character;
	if (!d->pending_message->message)
	{
		ch->send_to_char ("No typo report posted.\n");
		return;
	}

	wr = get_room_desc_tag (ch, ch->room);
	sprintf (msg, "From: %s [%d%s%s]\n", ch->name, ch->in_room,
		wr ? " - rset " : "", wr ? wr : "");
	sprintf (msg + strlen (msg), "\n");
	sprintf (msg + strlen (msg), "%s", ch->desc->pending_message->message);

	add_message (1, d->pending_message->info,
		-5, ch->desc->acct->name.c_str (), date2, d->pending_message->subject, "", msg, 0);

	ch->send_to_char("Thank you! Your typo report has been entered into our tracking system.\n");

	free_mem(d->pending_message);
	d->pending_message=NULL;
}



void
do_typo (CHAR_DATA * ch, char *argument, int cmd)
{
	char arg[MAX_STRING_LENGTH] = { '\0'};

	if (IS_NPC (ch))
	{
		ch->send_to_char ("Mobs can't submit bug reports.\n");
		return;
	}

	/* retrieve the category */
	if (!(argument = one_argument(argument,arg)))
	{
		ch->send_to_char ("Usage: typo <category> <title>\n"
			"Valid categories: Code, Room, Craft, Mob, Object, Misc\n");
		return;
	}


	/* Match a valid option then invert...thus, if does not match any option here */
	if (!( !strcasecmp(arg,"Code") || !strcasecmp(arg,"Room") || !strcasecmp(arg,"Craft") ||
		!strcasecmp(arg,"Mob") || !strcasecmp(arg,"Object") || !strcasecmp(arg,"Misc") ))
	{
		ch->send_to_char ("Usage: typo <category> <title>\n"
			"Valid categories: Code, Room, Craft, Mob, Object, Misc\n");
		return;

	}

	/* check for a second argument */
	if (!*argument)
	{
		ch->send_to_char ("Please select a brief title for your typo\n");
		return;
	}

	
	ch->send_to_char("Enter a typo report to be submitted to the admins. Terminate\n"
		"the editor with an '@' symbol. Please note that your post\n"
		"will be stamped with all pertinent contact information; no\n"
		"need to include that in the body of your message. Thanks for\n"
		"doing your part to help improve our world!\n");

	ch->make_quiet();

	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;

	std::stringstream ss;
	ss << "typos-" << arg;

	ch->desc->pending_message->info = duplicateString (ss.str().c_str()); /* category */
	ch->desc->pending_message->subject = duplicateString (argument); /* title */

	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->desc->proc = post_typo;
}

void
do_compact(CHAR_DATA * ch, char *argument, int cmd)
{
	const char *message[2] =
	{
		"You are now in the uncompacted mode.\n"
		"You are now in compact mode.\n",
	};

	// toggle FLAG_COMPACT and test if it is ON or OFF.
	// deliver the appropriate message to the player
	ch->send_to_char (message[((ch->flags ^= FLAG_COMPACT) & FLAG_COMPACT)]);
}

void
sa_stand (SECOND_AFFECT * sa)
{
	if (!is_he_somewhere (sa->ch))
		return;

	if ((sa->ch)->get_position() == STAND)
		return;

	do_stand (sa->ch, "", 0);
}

void
sa_get (SECOND_AFFECT * sa)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (!is_he_somewhere (sa->ch))
		return;

	sa->obj->tmp_flags &= ~SA_DROPPED;

	if (sa->obj == sa->ch->right_hand || sa->obj == sa->ch->left_hand)
		return;

	sprintf (buf, "get .c %d", sa->obj->coldload_id);

	command_interpreter (sa->ch, buf);
}

void
sa_wear (SECOND_AFFECT * sa)
{
	int num;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (!is_he_somewhere (sa->ch))
		return;

	if (!(num = is_obj_in_list (sa->obj, sa->ch->right_hand)) &&
		!(num = is_obj_in_list (sa->obj, sa->ch->left_hand)))
	{
		if (!IS_SET (sa->ch->flags, FLAG_COMPETE) ||
			!is_obj_in_list (sa->obj, sa->ch->room->contents))
			return;

		extract_obj (sa->obj);

		return;
	}

	sprintf (buf, "wear .c %d", sa->obj->coldload_id);

	command_interpreter (sa->ch, buf);
}

void
sa_close_door (SECOND_AFFECT * sa)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };

	sprintf (buf, "close %s", sa->info);
	command_interpreter (sa->ch, buf);

	sprintf (buf, "lock %s", sa->info);
	command_interpreter (sa->ch, buf);
}

void
sa_knock_out (SECOND_AFFECT * sa)
{
	if (!is_he_somewhere (sa->ch))
		return;

	if ((sa->ch)->get_position() != SLEEP)
		return;

	if (get_affect (sa->ch, MAGIC_AFFECT_SLEEP))
		return;

	(sa->ch)->set_position(REST);

	(sa->ch)->send_to_char("You regain consciousness, flat on the ground.");
	(sa->ch)->act("$n regains consciousness.", false, 0, 0, TO_ROOM);
}

void
sa_move (SECOND_AFFECT * sa)
{
	if (!is_he_somewhere (sa->ch))
		return;


	do_move (sa->ch, "", sa->info2);
}

void
sa_command (SECOND_AFFECT * sa)
{
	char buf [MAX_STRING_LENGTH]= { '\0' };
	sprintf (buf, "%s", sa->info);
	command_interpreter(sa->ch, buf);
}


void
add_second_affect (int type, int seconds, CHAR_DATA * ch, OBJ_DATA * obj,
				   const char *info, int info2)
{
	second_affect *sa;

	sa = new second_affect();
	sa->type = type;
	sa->seconds = seconds;
	sa->ch = ch;
	sa->obj = obj;
	sa->info2 = info2;

	if (info)
		sa->info = duplicateString (info);
	else
		sa->info = NULL;

	second_affect_list.push_back(sa);
}

SECOND_AFFECT *
get_second_affect (CHAR_DATA * ch, int type, OBJ_DATA * obj)
{
	std::list<second_affect*>::iterator sa_it;
	second_affect* tsa;
	
	if (second_affect_active)
		return NULL;
	
	if (second_affect_list.size() > 0)
	{
		for (sa_it = second_affect_list.begin(); sa_it != second_affect_list.end(); sa_it++)
		{
			tsa = *sa_it;
			
			if ((!ch || tsa->ch == ch) 
				&& (tsa->type == type))
			{
				if (!obj || obj == tsa->obj)
				{
					return tsa;
				}
			}
		}
	}
	
	return NULL;
}

void
remove_second_affect (SECOND_AFFECT * sa)
{
	std::list<second_affect*>::iterator sa_it;
	second_affect* tsa;
	
	if (second_affect_list.size() > 0)
	{
		for (sa_it = second_affect_list.begin(); sa_it != second_affect_list.end(); sa_it++)
		{
			tsa = *sa_it;
			
			if (*tsa == *sa)
			{
				second_affect_list.erase(sa_it);
				break;
			}
		}
	}
}

void
clear_player_from_second_affects (CHAR_DATA *ch)
{
	std::list<second_affect*>::iterator sa_it;
	second_affect* tsa;
	
	if (second_affect_list.size() > 0)
	{
		for (sa_it = second_affect_list.begin(); sa_it != second_affect_list.end(); sa_it++)
		{
			tsa = *sa_it;
			if (tsa->ch == ch 
				|| (tsa->obj 
					&& (CHAR_DATA *) tsa->obj == ch))
			{
				second_affect_list.erase(sa_it);
			}
		}
	}
}


	//we may delete some, but not all of the affects, so we need to count instead of relying on the end() fucntion.
void
second_affect_update (void)
{
	SECOND_AFFECT *tsa;
	extern int second_affect_active;
	std::list<second_affect*>::iterator sa_it;
	int loop_count;
	 
	loop_count = second_affect_list.size();
	
	if (loop_count > 0)
	{
		for (sa_it = second_affect_list.begin();
			 loop_count > 0;
			 sa_it++)
		{
			tsa = *sa_it;  
			
			if (!tsa)
			{
				loop_count --;
				continue;
			}
			
			--tsa->seconds;
			if (tsa->seconds > 0)
			{
				loop_count --;
				continue;
			}
			
			second_affect_active = 1;
			
			switch (tsa->type)
			{
				case SA_STAND:
					sa_stand (tsa);
					break;
				case SA_GET_OBJ:
					sa_get (tsa);
					break;
				case SA_WEAR_OBJ:
					sa_wear (tsa);
					break;
				case SA_CLOSE_DOOR:
					sa_close_door (tsa);
					break;
				case SA_KNOCK_OUT:
					sa_knock_out (tsa);
					break;
				case SA_MOVE:
					sa_move (tsa);
					break;
				case SA_COMMAND:
					sa_command (tsa);
					break;
				default:
					break;
			}
			
			second_affect_active = 0;
			second_affect_list.erase(sa_it);
			
			if (second_affect_list.size() == 0)
				break;
			
			loop_count --;
		}
	}
	return;
}

void
do_scommand (CHAR_DATA * ch, char * argument, int cmd)
{
	if (!IS_NPC(ch) && !ch->get_trust() && !cmd)
	{
		ch->send_to_char("You are not permitted to use this command.\n");
		return;
	}
	std::string ArgumentList = argument, ThisArgument;
	ArgumentList = one_argument(ArgumentList, ThisArgument);
	if (ArgumentList.empty() || ThisArgument.empty())
	{
		ch->send_to_char("Correct syntax: #6scommand delay command#0.\n");
		return;
	}
	
	if (!(is_real_number(ThisArgument.c_str())))
	{
		char * p;
		int rolls = strtol (ThisArgument.c_str(), &p, 0);
		int die = strtol ((*p)?(p+1):(p),0,0);
		if ((rolls > 0 && rolls <=100) && (die > 0 && die <= 1000) && (*p == ' ' || *p == 'd'))
		{
			std::ostringstream conversion;
			conversion << dice(rolls, die);
			ThisArgument.assign(conversion.str());
		}
		else
		{
			ch->send_to_char("Delay must be a number or #2x#0d#2y#0.\n");
			return;
		}
	}
	add_second_affect(SA_COMMAND, atoi(ThisArgument.c_str()), ch, NULL, ArgumentList.c_str(), 0);
	return;
}



void
rl_minute_affect_update (void)
{
	CHAR_DATA *ch;
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *next_af;
	std::list<CHAR_DATA*>::iterator tch_iterator;
	
	next_minute_update += 60;	/* This is a RL minute */


	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		ch = *tch_iterator;

		if (ch->deleted)
			continue;

		if (!ch->room)
			continue;

		if (IS_SET (ch->room->room_flags, OOC))
			continue;

		for (af = ch->hour_affects; af; af = next_af)
		{

			next_af = af->next;

			if (af->type == MAGIC_SIT_TABLE)
				continue;

			/*** NOTE:  Make sure these are excluded in hour_affect_update ***/

			if (af->type >= MAGIC_AFFECT_FIRST && af->type <= MAGIC_AFFECT_LAST)
			{
				if (--af->a.spell.duration <= 0)
				{
					
					if (af->type == MAGIC_AFFECT_SLEEP)
					{
						affect_remove (ch, af);
						do_wake (ch, "", 0);
						if (IS_NPC (ch))
							do_stand (ch, "", 0);
					}
					else
						affect_remove (ch, af);
				}
			}

			if (af->type == MAGIC_PETITION_MESSAGE)
			{
				if (--af->a.spell.duration <= 0)
					affect_remove (ch, af);
			}

			if (af->type == MAGIC_SKILL_GAIN_STOP)
			{
				if (--af->a.spell.duration <= 0)
					affect_remove (ch, af);
			}
		}
	}
}


void
ten_second_update (void)
{
	CHAR_DATA *ch;
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *next_af;
	std::list<CHAR_DATA*>::iterator tch_iterator;
	
	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		ch = *tch_iterator;

		if (ch->deleted)
			continue;

		if (!ch->room)
			continue;

		if (IS_SET (ch->room->room_flags, OOC))
			continue;

		for (af = ch->hour_affects; af; af = next_af)
		{

			next_af = af->next;

			if (af->type == MAGIC_SIT_TABLE)
				continue;

			/***** Exclusion must be made in hour_affect_update!!! ******/
			if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST)
			{

				if (IS_NPC (ch))
					continue;

				if (!af->a.craft->timer)
					continue;
				
				af->a.craft->timer -= 10;

				if (af->a.craft->timer < 0)
					af->a.craft->timer = 0;

				if (af->a.craft->timer == 0)
					activate_phase (ch, af);
			}


			else if (af->type >= MAGIC_CRIM_HOODED &&
				af->type < MAGIC_CRIM_HOODED + 100)
			{

				af->a.spell.duration -= 10;

				if (af->a.spell.duration <= 0)
					affect_remove (ch, af);
			}

			else if (af->type == MAGIC_STARED || af->type == MAGIC_WARNED)
			{
				af->a.spell.duration -= 10;

				if (af->a.spell.duration <= 0)
					affect_remove (ch, af);
			}
		}

		if (ch->deleted)
			continue;

	}
}

void
payday (CHAR_DATA * ch, CHAR_DATA * employer, AFFECTED_TYPE * af)
{
	int i;
	float cash_value_per_day;
	int num_days;
	int tot_value;
	OBJ_DATA *tobj;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	OBJ_DATA *obj;
	bool paid = false;

	if (time_info.holiday >= 1)
	{
		ch->send_to_char ("Check again after the feastday has ended.\n");
		return;
	}


	if (af->a.job.cash)
	{
		cash_value_per_day = (af->a.job.cash/af->a.job.days);
		num_days = time_info.accum_days - af->a.job.pay_date;
		tot_value = (int)(num_days * cash_value_per_day);
		
		if (employer)
		{
			if (keeper_has_money(employer, tot_value))
			{
				keeper_money_to_char(employer, ch, tot_value);
				subtract_keeper_money (employer, tot_value);
				sprintf (buf,
					"INSERT INTO payroll "
					"(time, shopkeep, who, customer, amount, "
					"room, gametime) "
					"VALUES (NOW(), %d, '%s','%s', %d, %d,'%d-%d-%d %d:00')",
						 employer->mob->nVirtual,
						 ch->name,
						  ch->char_short(),
						 tot_value,
						 ch->in_room,
						 time_info.year,
						 time_info.month + 1,
						 time_info.day + 1,
						 time_info.hour);
				mysql_safe_query (buf);

				sprintf (buf, "$N pays you for all your hard work.");
				ch->act(buf, true,  0, employer, TO_CHAR | _ACT_FORMAT);
				ch->act("$N pays $n some coins.", false,  0, employer, 	       TO_NOTVICT | _ACT_FORMAT);
				paid = true;
			} //employer/keeper has the coin
			else
			{
				sprintf (buf, "$N whispers to you that they do not have the funds available. Try again later.");
				ch->act(buf, true,  0, employer, TO_CHAR | _ACT_FORMAT);
				ch->act("$N whispers something to $n.", false,  0, employer,  TO_NOTVICT | _ACT_FORMAT);
			} //employer Does NOT have the coin
		}//there is an employer
		else
		{
			obj = load_object (GONDOR_1);
			if (obj)
			{
				obj->count = tot_value;
				obj_to_char (obj, ch);
				sprintf (buf, "You are paid %d coppers.\n", tot_value);
				ch->send_to_char (buf);
				paid = true;
			}
			else 
			{
				sprintf (buf, "$N whispers to you that they do not have the funds available. Try again later.");
				ch->act(buf, true,  0, employer, TO_CHAR | _ACT_FORMAT);
				ch->act("$N whispers something to $n.", false,  0, employer,  TO_NOTVICT | _ACT_FORMAT);
			}
			
		} // there is not an employer
	} //paid in cash

	if (af->a.job.count && (tobj = vtoo (af->a.job.object_vnum)))
	{

		cash_value_per_day = (af->a.job.count/af->a.job.days);
		num_days = time_info.accum_days - af->a.job.pay_date;
		tot_value = (int)(num_days * cash_value_per_day);
		
		if (tot_value >= 1)
		{
			if (load_object (af->a.job.object_vnum))
			{
				for (i = tot_value; i; i--)
					obj_to_char (load_object (af->a.job.object_vnum), ch);
				
				
				if (employer)
				{
					sprintf (buf, "$N pays you %d x $p.", tot_value);
					ch->act(buf, false,  tobj, employer, TO_CHAR | _ACT_FORMAT);
					ch->act("$N pays $n with $o.", true, tobj, employer,
						 TO_NOTVICT | _ACT_FORMAT);
					paid = true;
				}
				else
				{
					sprintf (buf, "You are paid %d x $p.", tot_value);
					ch->act(buf, false,  tobj, 0, TO_CHAR | _ACT_FORMAT);
					ch->act("$n is paid with $o.", true, tobj, 0,
						 TO_ROOM | _ACT_FORMAT);
					paid = true;
				}
			}
			else
			{
				ch->send_to_char("There was an error loading your pay objects. Please inform staff.\n");
				paid = false;
			}
			
		}
	}

	if (paid)
	{
		af->a.job.pay_date = time_info.accum_days;
		sprintf (buf, "\nYour next full payday period is in %d days.\n", af->a.job.days);
		ch->send_to_char (buf);
	}

	return;
}

void
do_payday (CHAR_DATA * ch, char *argument, int cmd)
{
	int i;
	int t;
	bool isEmployed = false;
	CHAR_DATA *employer = NULL;
	OBJ_DATA *tobj = NULL;
	AFFECTED_TYPE *af;
	CHAR_DATA *tch;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buffer[MAX_STRING_LENGTH]= { '\0' };

	if (time_info.holiday >= 1)
	{
		ch->send_to_char ("Check again after the feastday has ended.\n");
		return;
	}

	t = time_info.year * 12 * 30 + time_info.month * 30 + time_info.day;

	for (i = JOB_1; i <= JOB_3; i++)
	{
		employer = 0;

		if (!(af = get_affect (ch, i)))
			continue;

		isEmployed = true;

		if (af->a.job.employer)
			employer = vtom (af->a.job.employer);

		/* PC action to get paid if it is time */
		if (t >= af->a.job.pay_date)
		{
			if (!af->a.job.employer)
			{
				payday (ch, NULL, af);
				continue;
			}
			for (tch = ch->room->people; tch; tch = tch->next_in_room)
			{
				if (IS_NPC (tch) && tch->mob->nVirtual == af->a.job.employer)
				{
					payday (ch, tch, af);
					continue;
				}
			}
		}

		/* Either its not time to be paid, or employer is not around */

		if ((af->a.job.employer) && (employer))
		{
			sprintf (buf, "%s#0 will pay you ", employer->short_descr);
			*buf = toupper (*buf);
			sprintf (buffer, "#5%s", buf);
			sprintf (buf, "%s", buffer);
		}
		else
			strcpy (buf, "You get paid ");

		if (af->a.job.cash)
		{
			sprintf (buf + strlen (buf), "%d coppers", af->a.job.cash);
			if (af->a.job.count && vtoo (af->a.job.object_vnum))
				strcat (buf, " and ");
		}

		if (af->a.job.count && (tobj = vtoo (af->a.job.object_vnum)))
		{
			if (af->a.job.count == 1)
				strcat (buf, "$p");
			else
				sprintf (buf + strlen (buf), "%d x $p", af->a.job.count);
		}

		sprintf (buf + strlen (buf), ", every %d day%s.",
				 af->a.job.days,
				 af->a.job.days == 1 ? "" : "s");

		ch->act(buf, true,  tobj, employer, TO_CHAR | _ACT_FORMAT);
	}

	if (!isEmployed)
	{
		ch->send_to_char ("You do not appear to have been setup with a payday.\n");
	}
}

void
hour_affect_update (void)
{
	int t;
	CHAR_DATA *ch;
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *next_af;
	std::list<CHAR_DATA*>::iterator tch_iterator;
	
	t = time_info.year * 12 * 30 + time_info.month * 30 + time_info.day;

	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		ch = *tch_iterator;

		if (ch->deleted || !ch->room)
			continue;

		if (ch->room && IS_SET (ch->room->room_flags, OOC))
			continue;

		for (af = ch->hour_affects; af; af = next_af)
		{

			next_af = af->next;

			if (af->type == MAGIC_SIT_TABLE)
				continue;

			if (af->type >= JOB_1 && af->type <= JOB_3)
			{

				if (t < af->a.job.pay_date)
					continue;

				if (af->a.job.employer) /// \todo Check as dead code
					continue;

				/* payday (ch, NULL, af); */
				continue;
			}

			if (af->type >= MAGIC_AFFECT_FIRST && af->type <= MAGIC_AFFECT_LAST)
				continue;

			if (af->type == MAGIC_SKILL_GAIN_STOP)
				continue;

			if (af->type >= MAGIC_CRIM_HOODED &&
				af->type < MAGIC_CRIM_HOODED + 100)
				continue;

			if (af->type == MAGIC_STARED)
				continue;

			if (af->type == MAGIC_RAISED_HOOD)
				continue;

			if (af->type == MAGIC_CRAFT_DELAY
				|| af->type == MAGIC_CRAFT_BRANCH_STOP)
				continue;

			if (af->type == MAGIC_PETITION_MESSAGE)
				continue;


			if (af->type == AFFECT_SHADOW ||
				(af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST))
				continue;

			if (af->a.spell.duration > 0)
				af->a.spell.duration--;

			if (af->a.spell.duration)
				continue;

			else if (af->type == AFFECT_LOST_CON)
			{
				ch->send_to_char ("You finally feel at your best once more.\n");
				ch->con += af->a.spell.sn;
				ch->tmp_con += af->a.spell.sn;
			}

			/******************************************/
			/*   SPELL WEAR-OFF MESSAGES               */
			/******************************************/

			affect_remove (ch, af);
		}
	}
}

/**************************************
* mode:
* 0 - initial learning of skill
* 1 - know skill, learn additional points to the skill
*
******************************************/
void teach_skill (CHAR_DATA * student, int skill, CHAR_DATA * teacher)
{
	int mode;
	int modifier = 0;
	float multi = 0.0;
	float percentage;
	int multiplier = 1;
	int learn_chance = 0;
	int skill_diff = 100;
	int min;
	int max;
	int char_skills = 0;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char *skill_name;
	int roll;


	skill_name = strdup(lookup_skill_name(skill));
						
	if (student->skill_map[skill_name] < 0)
	{
		student->skill_map[skill_name] = 0;
	}

	if (student->skill_map[skill_name] > 1) // adding to existing skill
	{
		
		skill_diff = teacher->skill_map[skill_name] - student->skill_map[skill_name];
		if (skill_diff < 15)
		{
			teacher->send_to_char("You are not advanced enough beyond your student to educate them further.\n");
			return;
		}
		mode = 1;
	}
	else //learning new skill
	{
		mode = 0;
	}

	//timer for skill check
	if (get_affect (student, MAGIC_SKILL_GAIN_STOP))
	{
		teacher->send_to_char("Your student seems distracted and unable to pay attention to your teaching.\n");
		sprintf (buf, "$N tries to teach you something about '%s', but you are still focused on your last lesson.", skill_name);
		student->act(buf, false, 0, teacher, TO_CHAR | _ACT_FORMAT);

		return;
	}

	//teachers may learn a little bit from the teaching too.
	skill_use (teacher, skill_name, 0);

	// Chance to learn it
	//modified by the number of skills already known
	//and by the difficulty of the skill
	char_skills = student->skill_map.size();

	
	percentage = ((double)char_skills / (double)skill_data_map.size());

	if (percentage >= 0.0 && percentage <= .10) // 12 skills
		multiplier = 4;
	else if (percentage > .10 && percentage <= .15) // 17 skills
		multiplier = 3;
	else if (percentage > .15 && percentage <= .20) // 23 skills
		multiplier = 2;
	else
		multiplier = 1;

	learn_chance = MIN ((int)(calc_lookup (student, REG_LV, skill) * multiplier), 65);

	learn_chance += (teacher->tmp_intel + student->tmp_intel)/2;
	learn_chance += (teacher->tmp_wil + student->tmp_wil)/2;

	roll = number (1, 100);
	if (roll > learn_chance)
	{
			
		teacher->send_to_char("The intricacies of this skill seem to be beyond your pupil at this time.\n");
		sprintf (buf, "$N tries to teach you something about '%s', but you just don't understand.", skill_name);
		student->act(buf, false, 0, teacher, TO_CHAR | _ACT_FORMAT);

			//timer to prevent repeated use of teach skill when it fails
		//30 to 180 minutes (1/2 hour to 3 hours) until they can learn a skill again
		if (!get_affect (student, MAGIC_SKILL_GAIN_STOP))
		{
			min = 30;

			max = 120 + number (1, 60);
			max = MIN (180, max);

			magic_add_affect (student, MAGIC_SKILL_GAIN_STOP,
				number (min, max), 0, 0, 0, 0);

		}

		return;
	}

	// How much they learn
	//INT/WIL bonus/penalty for teacher
	modifier = teacher->tmp_intel - 14;
	modifier = teacher->tmp_wil - 14;

	//INT/WIL bonus/penalty for student
	modifier += student->tmp_intel - 14;
	modifier += student->tmp_wil - 14;

	modifier *= 1;	//modifer adjustment for worth of INT/WIL

	//skill level bonus/penalty for teacher
	modifier += (teacher->skill_map[skill_name] - 50);

	if(modifier > 80)
		modifier = 80;

	if(modifier < 0)
		modifier = 0;

	//convert to a multiplier (approx range of .25 to 5)
	multi = (double)(100/(100 - modifier));


	if (mode == 1) //add to existing skill
	{
		student->skill_map[skill_name] += (int) (multi * calc_lookup (student, REG_LV, skill));
	}

	else //new skill
	{
		student->skill_map[skill_name] = (int) (multi * calc_lookup (student, REG_OV, skill));
	}

	//reduce to CAP value if needed
	if (student->skill_map[skill_name] > calc_lookup (student, REG_CAP, skill))
	{
		student->skill_map[skill_name] = calc_lookup (student, REG_CAP, skill);
	}

		//they learned it, so standard skill timer applies
	//1380 to 1500 minutes (23 to 25 hours) until they can learn a skill again
	if (!get_affect (student, MAGIC_SKILL_GAIN_STOP))
	{
		min = 1300;

		max = 1440 + number (1, 60);
		max = MIN (1500, max);

		magic_add_affect (student, MAGIC_SKILL_GAIN_STOP,
			number (min, max), 0, 0, 0, 0);

		teacher->send_to_char("Your student seems to have learned something.\n");
		sprintf (buf, "$N teachs you something new about '%s'.", skill_name);
		student->act(buf, false, 0, teacher, TO_CHAR | _ACT_FORMAT);
	}

	return;

}

void
open_skill (CHAR_DATA * ch, int skill)
{
	std::map<std::string, SKILL_DATA*>::iterator skill_it;
	std::string sk_name;
	int skill_val;
	
	if (skill <= 0)
		return;
	
	sk_name = lookup_skill_name(skill);
	
	if (sk_name.empty())
		return;
	
	skill_val = calc_lookup (ch, REG_OV, skill);
	ch->skill_map[sk_name] = skill_val;
		
}

void
expose_skill (CHAR_DATA * ch, int skill)
{
	std::map<std::string, SKILL_DATA*>::iterator skill_it;
	std::string sk_name;
	int skill_val;

	if (skill <= 0)
		return;
	
	sk_name = lookup_skill_name(skill);
	
	if (sk_name.empty())
		return;

	skill_val = calc_lookup (ch, REG_OV, skill);
	ch->skill_map[sk_name] = (int)(skill_val/2);

	
}

#define MIN_PREREQ	15

int
prereq_skill (CHAR_DATA * ch, CHAR_DATA * victim, int skill,
			  int prereq1, int prereq2)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (prereq1 && (victim->skill_map[lookup_skill_name(prereq1)] < MIN_PREREQ))
	{

		sprintf (buf, "$N cannot learn '%s' until $S learns '%s' sufficiently.",
			lookup_skill_name(skill), lookup_skill_name(prereq1));
		ch->act(buf, true,  0, victim, TO_CHAR);

		sprintf (buf, "$N tries to teach you '%s', but cannot until you learn "
			"'%s' sufficiently.", lookup_skill_name(skill), lookup_skill_name(prereq1));
		victim->act(buf, true,  0, ch, TO_CHAR);

		return 0;
	}

	else if (prereq2 && (victim->skill_map[lookup_skill_name(prereq2)] < MIN_PREREQ))
	{

		sprintf (buf, "$N cannot learn '%s' until $E learns '%s' sufficiently.",
			lookup_skill_name(skill), lookup_skill_name(prereq2));
		ch->act(buf, true,  0, victim, TO_CHAR);

		sprintf (buf, "$N tries to teach you '%s', but cannot until you learn "
			"'%s' sufficiently.", lookup_skill_name(skill), lookup_skill_name(prereq2));
		victim->act(buf, true, 0, ch, TO_CHAR);

		return 0;
	}

	return 1;
}

int
meets_craft_teaching_requirements (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
	PHASE_DATA *phase;
	char *skill_name;
	int i;
	
	for (i = 1; i < MAX_PHASES_PER_SUBCRAFT; i++)
	{
		phase = craft->phases[i];
		skill_name = strdup(lookup_skill_name(phase->skill));
		
		if (phase->skill)
		{
			if ((ch->skill_map[skill_name] < 1)
				|| (ch->skill_map[skill_name]) < 
				(int) ((phase->dice * phase->sides) * .75))
				return 0;
		}
	}

	return 1;
}

int
meets_craft_learning_requirements (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
	PHASE_DATA *phase;
	char *skill_name;
	int i;
	
	for (i = 1; i < MAX_PHASES_PER_SUBCRAFT; i++)
	{
		phase = craft->phases[i];
		if (phase->skill)
			if ((ch->skill_map[skill_name] < 1)
				|| (ch->skill_map[skill_name]) <
				(int) ((phase->dice * phase->sides) * .33))
				return 0;
	}

	return 1;
}

void
do_teach (CHAR_DATA * ch, char *argument, int cmd)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int skill_num;
	char *skill_name;
	CHAR_DATA *victim;
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	
	if (IS_SET (ch->room->room_flags, OOC))
	{
		ch->send_to_char ("This is not allowed in OOC areas.\n");
		return;
	}

	half_chop (argument, arg1, arg2);

	if (!*arg1)
	{
		ch->send_to_char ("Teach what?\n");
		return;
	}

	if (!*arg2)
	{
		ch->send_to_char ("Teach what to who?\n");
		return;
	}

	if (!(victim = get_char_room_vis (ch, arg2)))
	{
		ch->send_to_char ("They aren't here.\n");
		return;
	}

	if (IS_NPC (victim))
	{
		ch->send_to_char ("They are too busy being important to learn anything.\n");
		return;
	}


	if (lookup_skill_id(arg1) < 0)
	{
		ch->send_to_char ("No such skill.\n");
		return;
	}

	skill_name = strdup(arg1);
	
	//Regular Skills
	if (ch->skill_map[skill_name] < 1)
	{
		ch->send_to_char ("You don't know that skill!\n");
		return;
	}

	if (ch->skill_map[skill_name] < 15)
	{
		ch->send_to_char ("You don't yet know that skill well enough.\n");
		return;
	}


	
	//Dependant skills
	if (!str_cmp(skill_name, "Sneak"))
	{
		if (!prereq_skill (ch, victim, lookup_skill_id("Sneak"), lookup_skill_id("Hide"), 0))
		{
			ch->send_to_char ("They don't know enought about hiding.\n");
			return;
		}
	}
	
	else if (!str_cmp(skill_name, "Steal"))
	{
		if (!prereq_skill (ch, victim, lookup_skill_id("Steal"), lookup_skill_id("Sneak"), lookup_skill_id("Hide")))
		{
			ch->send_to_char ("They don't know enought about several things.\n");
			return;
		} 
	}
	
	
	skill_num = lookup_skill_id(skill_name);
	
	teach_skill (victim, skill_num, ch);

	ch->send_to_char("Your training session is over.\n");
}

void
add_memory (CHAR_DATA * add, CHAR_DATA * mob)
{
	struct memory_data *memory;
	char name[MAX_STRING_LENGTH]= { '\0' };

	if (IS_NPC (add))
		one_argument (add->keywords, name);
	else
		strcpy (name, add->name);

	for (memory = mob->remembers; memory; memory = memory->next)
		if (!strcmp (memory->name, name))
			return;

	memory = new memory_data;
	memory->name = duplicateString (name);
	memory->next = mob->remembers;
	mob->remembers = memory;
}

void
forget (CHAR_DATA * ch, CHAR_DATA * foe)
{
	struct memory_data *mem;
	struct memory_data *tmem;

	if (!ch->remembers)
		return;

	if (!strcmp (foe->name, ch->remembers->name))
	{
		mem = ch->remembers;
		ch->remembers = ch->remembers->next;
		free_mem (mem);
		return;
	}

	for (mem = ch->remembers; mem->next; mem = mem->next)
	{
		if (!strcmp (foe->name, mem->next->name))
		{
			tmem = mem->next;
			mem->next = tmem->next;
			free_mem (tmem);
			return;
		}
	}
}


int
has_a_key (CHAR_DATA * mob)
{
	if (mob->right_hand && mob->right_hand->obj_flags.type_flag == ITEM_KEY)
		return 1;
	if (mob->left_hand && mob->left_hand->obj_flags.type_flag == ITEM_KEY)
		return 1;

	return 0;
}

void
do_knock (CHAR_DATA * ch, char *argument, int cmd)
{
	int door;
	int target_room;
	int trigger_info;
	int key;
	char dir[MAX_STRING_LENGTH]= { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char key_name[MAX_STRING_LENGTH]= { '\0' };
	CHAR_DATA *tch;
	ROOM_DATA *room;
	std::ostringstream knock_error;

	argument = one_argument (argument, buf);
	argument = one_argument (argument, dir);

	if ((door = find_door (ch, buf, dir)) == -1)
		return;

	if (!IS_SET (is_exit(ch, door)->port_flags, EX_ISDOOR)
		&& !IS_SET (is_exit(ch, door)->port_flags, EX_ISGATE))
	{
		ch->send_to_char ("That's not a door.\n");
		return;
	}

	else if (!IS_SET (is_exit(ch, door)->port_flags, EX_CLOSED))
	{
		ch->send_to_char ("It's already open!\n");
		return;
	}

	target_room = is_exit(ch, door)->to_room;

	if (!vtor (target_room))
	{
		ch->send_to_char ("Actually, that door doesn't lead anywhere.\n");
		return;
	}

	sprintf (buf, "You hear tapping from the %s.\n", dirs[rev_dir[door]]);
	send_to_room (buf, target_room);

	ch->act("You rap on the door.", false,  0, 0, TO_CHAR);
	ch->act("$n raps on the door.", false,  0, 0, TO_ROOM);

	room_exit_data* revdir = room->dir_option[rev_dir[door]];
	if (revdir==NULL)
	{
		std::ostringstream oss;
		oss << "#1Building error: The door between " << ch->in_room << " and " <<
			room->nVirtual << " is on a one-way exit.#0\n";
		send_to_gods(oss.str().c_str());
	}
	key = revdir->key;

	for (tch = room->people; tch; tch = tch->next_in_room)
	{

		if (!IS_NPC (tch) || !tch->is_awake() || !can_see_mob(tch, tch))	/* Too dark if he can't see self. */
			continue;

		if (!has_a_key (tch))
			continue;

		if (tch->desc)
			continue;


		if (!IS_SET (room->dir_option[rev_dir[door]]->port_flags, EX_LOCKED)
			|| has_key (tch, NULL, key))
		{
			one_argument (room->dir_option[rev_dir[door]]->keyword, key_name);
			sprintf (buf, "unlock %s %s", key_name, dirs[rev_dir[door]]);
			command_interpreter (tch, buf);
			sprintf (buf, "open %s %s", key_name, dirs[rev_dir[door]]);
			command_interpreter (tch, buf);
			sprintf (buf, "%s %s", key_name, dirs[rev_dir[door]]);
			add_second_affect (SA_CLOSE_DOOR, IS_NPC (ch) ? 5 : 10,
				tch, NULL, buf, 0);
			return;
		}
	}
}


void
do_nod (CHAR_DATA * ch, char *argument, int cmd)
{
	int opened_a_door = 0;
	int dir;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char key_name[MAX_STRING_LENGTH]= { '\0' };
	CHAR_DATA *victim = NULL;

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->act("You nod.", false,  0, 0, TO_CHAR);
		ch->act("$n nods.", false,  0, 0, TO_ROOM);
		return;
	}

	if (!strcmp(buf, "doorman") || !strcmp(buf, "doorkeep") || !strcmp(buf, "gatekeep"))
	{
		argument = one_argument(argument, buf);
		int which_doorman = 1;
		if (buf && *buf && atoi(buf) > 1)
			which_doorman = atoi(buf);

		for (victim = ch->room->people; victim; victim = victim->next_in_room)
		{
			if (IS_NPC(victim) 
				&& victim->is_awake() 
				&& can_see_mob(victim, ch) 
				&& is_brother(ch, victim) 
				&& has_a_key(victim) 
				&& !victim->desc)
			{
				which_doorman--;
				if (!which_doorman)
					break;
			}
		}
		if (!victim)
		{
			ch->send_to_char("There doesn't appear to be any doorman here.");
			return;
		}
	}

	if (!victim && !(victim = get_char_room_vis (ch, buf)))
	{
		ch->send_to_char ("You don't see that person.\n");
		return;
	}

	if (IS_NPC (victim)
		&& victim->is_awake()
		&& can_see_mob(victim, ch)
		&& is_brother (ch, victim) 
		&& has_a_key (victim) 
		&& !victim->desc)
	{
		
		argument = one_argument (argument, buf);

		if (!*buf)
		{
			for (dir = 0; dir <= LAST_DIR; dir++)
			{

				if (!is_exit(ch, dir))
					continue;

				if (IS_SET (is_exit(ch, dir)->port_flags, EX_LOCKED)
					&& !has_key (victim, NULL, is_exit(ch, dir)->key))
					continue;

				one_argument (is_exit(victim, dir)->keyword, key_name);
				sprintf (buf, "unlock %s %s", key_name, dirs[dir]);
				command_interpreter (victim, buf);
				sprintf (buf, "open %s %s", key_name, dirs[dir]);
				command_interpreter (victim, buf);
				sprintf (buf, "%s %s", key_name, dirs[dir]);
				add_second_affect (SA_CLOSE_DOOR, 10, victim, NULL, buf, 0);

				opened_a_door = 1;
			}
		}
		else
		{
			dir = is_direction (buf);
			if (dir == -1 || !is_exit(ch, dir))
			{
				ch->send_to_char ("There is no exit in that direction.\n");
				return;
			}
			if (IS_SET (is_exit(ch, dir)->port_flags, EX_LOCKED)
				&& !has_key (victim, NULL, is_exit(ch, dir)->key))
				;
			else
			{
				one_argument (is_exit(victim, dir)->keyword, key_name);
				sprintf (buf, "unlock %s %s", key_name, dirs[dir]);
				command_interpreter (victim, buf);
				sprintf (buf, "open %s %s", key_name, dirs[dir]);
				command_interpreter (victim, buf);
				sprintf (buf, "%s %s", key_name, dirs[dir]);
				add_second_affect (SA_CLOSE_DOOR, 10, victim, NULL, buf, 0);

				opened_a_door = 1;
			}
		}

		if (opened_a_door)
			return;
	}

	sprintf (buf, "You nod to #5%s#0.", victim->char_short());
	ch->act(buf, false,  0, victim, TO_CHAR | _ACT_FORMAT);

	ch->act("$n nods to you.", true, 0, victim, TO_VICT | _ACT_FORMAT);
	ch->act("$n nods to $N.", true, 0, victim, TO_NOTVICT | _ACT_FORMAT);
}

void
do_camp (CHAR_DATA * ch, char *argument, int cmd)
{
	int terrain_type;

	if (ch->is_switched())
		return;
	
	if (ch->desc && ch->desc->original)
	{
		ch->send_to_char ("Not while switched. Use RETURN first.\n");
		return;
	}

	if ((!ch->get_trust()) && IS_SET (ch->room->room_flags, SAFE_Q))
	{
		ch->delayed_camp4();
		return;
	}

	terrain_type = ch->room->terrain_type;

	if (terrain_type != SECT_WOODS && terrain_type != SECT_FOREST &&
		terrain_type != SECT_FIELD && terrain_type != SECT_HILLS)
	{
		ch->send_to_char ("You can only camp in the woods, forest, a "
			"field or the hills.\n");
		return;
	}

	
	ch->send_to_char ("You search for a suitable location to build a lean-to.\n");
	ch->act("$n begins looking around in the brush.", true, 0, 0, TO_ROOM);

	ch->delay_type = DEL_CAMP1;
	ch->delay = 30;
}


void
knock_out (CHAR_DATA * ch, int seconds)
{
	SECOND_AFFECT *sa;

	if (ch->get_position() > SLEEP)
	{
		ch->send_to_char ("You stagger to the ground, losing consciousness!\n");
		ch->act("$n staggers to the ground, losing consciousness.", false,  0,
			0, TO_ROOM);
		ch->set_position(SLEEP);
	}

	if ((sa = get_second_affect (ch, SA_KNOCK_OUT, NULL)))
	{
		if (sa->seconds < seconds)
			sa->seconds = seconds;
		return;
	}

	add_second_affect (SA_KNOCK_OUT, seconds, ch, NULL, NULL, 0);
}

void
do_tables (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *obj;
	CHAR_DATA *tmp;
	AFFECTED_TYPE *af_table = NULL;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	int table_count = 0;

	ch->send_to_char ("\n");

	for (obj = ch->room->contents; obj; obj = obj->next_content)
	{

		if (obj->obj_flags.type_flag != ITEM_CONTAINER ||
			!IS_SET (obj->obj_flags.extra_flags, ITEM_TABLE))
			continue;

		table_count++;

		sprintf (buf, "#6%s#0\n", obj_desc (obj, 2));

		for (tmp = ch->room->people; tmp; tmp = tmp->next_in_room)
		{
			af_table = get_affect (tmp, MAGIC_SIT_TABLE);
			if (af_table && is_at_table (tmp, obj) && tmp != ch)
			{
				char* charShort = duplicateString(tmp->char_short());

				sprintf (buf2, "    #5%s#0 is seated here.\n",
					CAP (charShort));
				free_mem (charShort);
				strcat (buf, buf2);
			}
		}

		ch->send_to_char (buf);
	}

	if (!table_count)
		ch->send_to_char ("   None.\n");

	return;
}


// Command Ownership, for transfering ownership of mobs
// Syntax: OWNERSHIP TRANSFER <mob> <character> or OWNERSHIP SET <mob> <character>

void do_ownership (CHAR_DATA *ch, char *argument, int command)
{
	CHAR_DATA *owned_mob, *target;
	std::string ArgumentList = argument;
	std::string ThisArgument;
	std::string Output;
	bool transfer = true;
	int loaded_char = 0;

	ArgumentList = one_argument(ArgumentList, ThisArgument);

	if ((ThisArgument.find("?", 0) != std::string::npos) || ThisArgument.empty())
	{
		ch->send_to_char("Syntax is:\n");
		ch->send_to_char("ownership transfer <mob> <character>\n");
		ch->send_to_char("(Staff level)\nownership set <mob> <character> \n");
		return;
	}

	if (ThisArgument.find("set", 0) != std::string::npos)
	{
		transfer = false;

		if (ch->get_trust() < 2)
		{
			ch->send_to_char ("You can only transfer ownership.\n");
			return;
		}
	}

	ArgumentList = one_argument(ArgumentList, ThisArgument);

	if (ThisArgument.empty())
	{
		ch->send_to_char ("Which individual do you wish to transfer ownership of?\n");
		return;
	}

	owned_mob = get_char_room_vis (ch, ThisArgument.c_str());

	if (!owned_mob)
	{
		if (ch->get_trust() > 1)
		{
			owned_mob = get_char ((char*)ThisArgument.c_str());
		}

		if (!owned_mob)
		{
			ch->send_to_char ("Cannot find mobile with keyword \"#2");
			ch->send_to_char (ThisArgument.c_str());
			ch->send_to_char ("#0\".\n");
			return;
		}
	}

	if (!IS_NPC(owned_mob))
	{
		ch->send_to_char ("You have no authority to deliniate the ownership of a PC.\n");
		return;
	}

	if (IS_NPC(owned_mob) 
		&& !owned_mob->mob->owner  
		&& (ch->get_trust() < 2))
	{
		ch->send_to_char ("You have no authority to deliniate the ownership of this individual.\n");
		return;
	}

	if (owned_mob->mob->owner 
		&& strcmp(owned_mob->mob->owner, ch->name) 
		&& (ch->get_trust() < 2))
	{
		ch->send_to_char ("You have no authority to deliniate the ownership of this individual.\n");
		return;
	}

	ArgumentList = one_argument(ArgumentList, ThisArgument);


	if (ThisArgument.empty())
	{
		ch->send_to_char ("Transfer the ownership to whom?\n");
		return;
	}
	//new owner must be in the room and visible since get_char checks online PC only
	////so we use load_pc to get around this
	//
	target = get_char_room_vis(ch, ThisArgument.c_str());
	if (!target && (ch->get_trust() > 1))
	{
		target = load_pc((char*)ThisArgument.c_str());
		loaded_char = 1;
	}

	if (!target)
	{
		ch->send_to_char ("You do not see a person with the keyword \"#2");
		ch->send_to_char (ThisArgument.c_str());
		ch->send_to_char ("#0\" to transfer #5");
		ch->send_to_char (owned_mob->char_short());
		ch->send_to_char ("#0 to.\n");
		return;
	}

	if (!transfer && (ch->get_trust() > 1))
	{

		ThisArgument[0] = toupper(ThisArgument[0]);
		owned_mob->mob->owner = duplicateString (ThisArgument.c_str());
		ch->send_to_char ("Setting ownership of #5");
		ch->send_to_char (owned_mob->char_short());
		ch->send_to_char ("#0 to \"#2");
		ch->send_to_char (ThisArgument.c_str());
		ch->send_to_char ("#0\".");
		if (loaded_char)
			target->unload_pc();
		return;

	}

	else
	{

		ArgumentList = one_argument(ArgumentList, ThisArgument);

		if (IS_NPC(target) && (ThisArgument.find('!') == std::string::npos))
		{
			ch->send_to_char ("You are proposing to transfer ownership of #5");
			ch->send_to_char (owned_mob->char_short());
			ch->send_to_char ("#0 to #5");
			ch->send_to_char (target->char_short());
			ch->send_to_char ("#0, who is an NPC. Please confirm by typing #6OWNERSHIP TRANSFER <owned_mob> <target> !#0\n");
			return;
		}


		Output.assign(target->name);
		Output[0] = toupper(Output[0]);
		owned_mob->mob->owner = duplicateString (Output.c_str());
		ch->send_to_char ("You transfer ownership of #5");
		ch->send_to_char (owned_mob->char_short());
		ch->send_to_char ("#0 to #5");
		ch->send_to_char (target->char_short());
		ch->send_to_char ("#0.\n");
		Output.assign("#5");
		Output.append(ch->char_short());
		Output.append("#0 transfers ownership of #5");
		Output.append(owned_mob->char_short());
		Output.append("#0 to you.\n");
		Output[2] = toupper(Output[2]);
		target->send_to_char(Output.c_str());
		if (loaded_char)
			target->unload_pc();
		return;
	}
	if (loaded_char)
		target->unload_pc();

	return;
}
