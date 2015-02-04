//////////////////////////////////////////////////////////////////////////////
//
/// act.informative.c : Informational Module
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

#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "server.h"
#include "structs.h"
#include "net_link.h"
#include "account.h"
#include "utils.h"
#include "protos.h"
#include "decl.h"
#include "group.h"
#include "utility.h"


extern rpie::server engine;
extern const int sunrise[];
extern const int sunset[];
extern const char* dirs[];
extern const char* relative_dirs[];
extern const char* extended_dirs[];
extern const char* opposite_dirs[];
extern const int rev_dir[];
extern const char *month_name[12];
extern const char *exit_dirs[];
extern const char *room_bits[];

extern std::map<std::string, SKILL_DATA*> skill_data_map;
extern std::map<std::string, SUBCRAFT_HEAD_DATA*> craft_map;
extern std::map<int, OBJECT_MATERIAL*> object_material_map;
extern struct moon_data global_moon_values;

const char* verbal_speeds[9] = {
	"walk",			/* 1.00 */
	"trudge",			/* 2.50 */
	"pace",			/* 1.60 */
	"jog",			/* 0.66 */
	"run",			/* 0.50 */
	"sprint",			/* 0.33 */
	"immwalk",			/* 0    */
	"\n"
};

const char *holiday_names[8] = {
	"(null)",
	"mettare",
	"yestare",
	"tuilere",
	"loende",
	"enderi",
	"yaviere",
	"\n"
};

static const char *strTimeWord[25] = {
	"twelve", "one", "two", "three", "four", "five", "six", "seven", "eight",
	"nine", "ten", "eleven",
	"twelve", "one", "two", "three", "four", "five", "six", "seven", "eight",
	"nine", "ten", "eleven",
	"twelve"
};


const char *fog_states[4] = {
	"no fog",
	"thin fog",
	"thick fog",
	"\n"
};

const char *weather_states[9] = {
	"no rain",
	"chance of rain",
	"light rain",
	"steady rain",
	"heavy rain",
	"light snow",
	"steady snow",
	"heavy snow",
	"\n"
};

const char *weather_clouds[5] = {
	"clear sky",
	"light clouds",
	"heavy clouds",
	"overcast",
	"\n"
};

const char *wind_speeds[6] = {
	"calm",
	"breeze",
	"windy",
	"gale",
	"stormy",
	"\n"
};

const char *wind_directions[9] = {
	"westerly",
	"north westerly",
	"northerly",
	"north easterly",
	"easterly",
	"south easterly",
	"southerly",
	"south westerly",
	"\n"
};

const char *special_effects[5] = {
	"no effect",
	"volcanic smoke",
	"foul stench",
	"low mist",
	"\n"
};

const char *fullness[4] = {
	"less than half ",
	"about half ",
	"more than half ",
	""
};

const char *verbal_intox[6] = {
	"sober",
	"tipsy",
	"slightly drunk",
	"drunk",
	"intoxicated",
	"plastered"
};

const char *verbal_hunger[6] = {
	"starving",
	"hungry",
	"feeling slightly hungry",
	"feeling peckish",
	"quite full",
	"absolutely stuffed"
};

const char *verbal_thirst[6] = {
	"dying of thirst",
	"quite parched",
	"feeling thirsty",
	"feeling slightly thirsty",
	"nicely quenched",
	"completely sated"
};

const char *where[34] = {
	"<used as light>                ",
	"<worn as shield>               ",
	"<held as primary>              ",
	"<held as secondary>            ",
	"<held in both hands>           ",
	"<worn on head>                 ",
	"<worn in hair>                 ",	
	"<worn on the ears>             ",
	"<over the eyes>                ",
	"<worn on face>                 ",
	"<worn at throat>               ",
	"<worn at neck>                 ",
	"<worn at neck>                 ",
	"<worn about body>              ",
	"<worn on body>                 ",
	"<across the back>              ",
	"<worn over right shoulder>     ",
	"<worn over left shoulder>      ",
	"<about upper right arm>        ",	
	"<about upper left arm>         ",
	"<worn on arms>                 ",	
	"<worn on right wrist>          ",
	"<worn on left wrist>           ",
	"<worn on hands>                ",
	"<worn on right finger>         ",	
	"<worn on left finger>          ",
	"<worn about waist>             ",
	"<worn on belt>                 ",	
	"<worn on belt>                 ",
	"<worn on legs>                 ",
	"<worn on right ankle>          ",
	"<worn on left ankle>           ",
	"<worn on feet>                 ",
	"\n"
};


BOARD_DATA *full_board_list = NULL;


/*
 A random variance is used to get a rough estimate. The scan
 skill is used for the deviation, so those with a high scan
 skill will be more accurate than those with a low scan skill.
 */
int estimate_distance(CHAR_DATA* ch, int real_dist)
{
	int variance;
	int guess_dist;
	
	/*** Determine how far estimate may vary ***/
	if(ch->skill_map["Scan"]>70)
	{
		variance = number (95, 105);
	}
	else if(ch->skill_map["Scan"]>60)
	{
		variance = number (92, 108);
	}
	else if(ch->skill_map["Scan"]>50)
	{
		variance = number (90, 110);
	}
	else if(ch->skill_map["Scan"]>40)
	{
		variance = number (87, 113);
	}
	else
	{
		variance = number (85, 115);
	}
	
	
	guess_dist = (int)((real_dist*100)/variance);
	
	return (guess_dist);		
	
}

void do_credits (CHAR_DATA * ch, char *argument, int cmd)
{
	char* buffer;
	sprintf(buffer, "diku_license");
	do_help (ch, buffer, 0);
	return;
}

void target_sighted (CHAR_DATA * ch, CHAR_DATA * target)
{
	SIGHTED_DATA *sighted = NULL;

	if (!ch || !target)
		return;

	if (!ch->sighted)
	{
		ch->sighted = new SIGHTED_DATA;
		ch->sighted->next = NULL;
		ch->sighted->target = target;
		return;
	}
	for (sighted = ch->sighted; sighted; sighted = sighted->next)
	{
		if (sighted->target == target)
			return;
		if (!sighted->next)
		{
			sighted->next = new SIGHTED_DATA;
			sighted->next->next = NULL;
			sighted->next->target = target;
			return;
		}
	}
}

/***
 * TODO Why can't we point at objects? Just need to check for objects being visible,
 * and use mostly the same code.
 **/
void do_point (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char arg1[MAX_STRING_LENGTH] = { '\0' };
	char arg2[MAX_STRING_LENGTH] = { '\0' };
	char distance[MAX_STRING_LENGTH] = { '\0' };
	std::stringstream buffer;
	CHAR_DATA *target_mob = NULL, *tch = NULL;
	ROOM_EXIT_DATA *exit = NULL;
	ROOM_DATA *room = NULL;
	int dir = 0, range = 1;
	bool flag = false;
	
	buffer.clear();
	if (!*argument)
	{
		ch->send_to_char("Usage: point <direction> <target>\n");
		return;
	}

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);

	dir = index_lookup (dirs, arg1);
	if (dir == -1)
	{
		ch->send_to_char("What direction is that?\n");
		return;
	}
	
		//if there is no normal exit, check for diagonals
	if (!is_exit(ch, dir))
	{
		sprintf (buf, "%s %s", arg1, arg2);
		flag = point_diagonal(ch, buf);
		if (!flag)
		{
			ch->send_to_char("There isn't an exit in that direction.\n");
			return;
		}
		else //there are diagonals, and we checked for them already
		{
			return;
		}

	}

	room = vtor (is_exit(ch, dir)->to_room);
	exit = is_exit(ch, dir);

	if (exit
		&& (IS_SET (exit->port_flags, EX_ISDOOR)
		&& IS_SET (exit->port_flags, EX_CLOSED)
		&& !IS_SET (exit->port_flags, EX_ISGATE))
		|| (exit->type == PORTAL_BARRIER))
	{
		ch->send_to_char("Your view is blocked.\n");
		return;
	}

	target_mob = get_char_room_vis2 (ch, room->nVirtual, arg2);
	if (!target_mob || !has_been_sighted (ch, target_mob))
	{
		exit = room->dir_option[dir];
		if (!exit)
		{
			ch->send_to_char("You don't see them within range.\n");
			return;
		}
		if (exit
			&& (IS_SET (exit->port_flags, EX_ISDOOR)
				&& IS_SET (exit->port_flags, EX_CLOSED)
				&& !IS_SET (exit->port_flags, EX_ISGATE))
			|| (exit->type == PORTAL_BARRIER))
		{
			ch->send_to_char("Your view is blocked.\n");
			return;
		}
		if (room->dir_option[dir])
			room = vtor (room->dir_option[dir]->to_room);
		else
			room = NULL;
		
		if (is_sunlight_restricted (ch))
			return;
		
		if (!(target_mob = get_char_room_vis2 (ch, room->nVirtual, arg2))
			|| !has_been_sighted (ch, target_mob))
		{
			exit = room->dir_option[dir];
			if (!exit)
			{
				ch->send_to_char("You don't see them within range.\n");
				return;
			}
			if (exit
				&& (IS_SET (exit->port_flags, EX_ISDOOR)
					&& IS_SET (exit->port_flags, EX_CLOSED)
					&& !IS_SET (exit->port_flags, EX_ISGATE))
				|| (exit->type == PORTAL_BARRIER))
			{
				ch->send_to_char("Your view is blocked.\n");
				return;
			}
			if (room->dir_option[dir])
				room = vtor (room->dir_option[dir]->to_room);
			else
				room = NULL;
			
			if (!(target_mob = get_char_room_vis2 (ch, room->nVirtual, arg2))
				|| !has_been_sighted (ch, target_mob))
			{
				exit = room->dir_option[dir];
				if (!exit)
				{
					ch->send_to_char("You don't see them within range.\n");
					return;
				}
				if (exit
					&& (IS_SET (exit->port_flags, EX_ISDOOR)
						&& IS_SET (exit->port_flags, EX_CLOSED)
						&& !IS_SET (exit->port_flags, EX_ISGATE))
					|| (exit->type == PORTAL_BARRIER))
				{
					ch->send_to_char("Your view is blocked.\n");
					return;
				}
				ch->send_to_char("You don't see them within range.\n");
				return;
			}
			else
				range = 3;
		}
		else
			range = 2;
	}
	else
		range = 1;

	if (!target_mob || !can_see_mob(ch, target_mob) || !has_been_sighted (ch, target_mob))
	{
		ch->send_to_char("You don't see them within range.\n");
		return;
	}

	if (range == 2)
		sprintf (distance, "far ");
	else if (range == 3)
		sprintf (distance, "very far ");
	else
		*distance = '\0';

	sprintf (distance + strlen (distance), "to the %s", dirs[dir]);

	sprintf (buf, "You point at #5%s#0, %s.", target_mob->char_short(), distance);
	ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);

	sprintf (buf, "%s#0 points at #5%s#0, %s.",  ch->char_short(),
		target_mob->char_short(), distance);
	*buf = toupper (*buf);

		//prependeding the color code
	buffer << "#5" << buf;
	sprintf (buf, "%s", buffer.str().c_str());
	ch->act(buf, false,  0, 0, TO_ROOM | _ACT_FORMAT);

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (!can_see_mob(tch, ch))
			continue;
		target_sighted (tch, target_mob);
	}
}

void do_title (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char echo[MAX_STRING_LENGTH] = { '\0' };
	OBJ_DATA *obj = NULL;
	double skill = 0;

	if (!*argument)
	{
		ch->send_to_char("What did you wish to title?\n");
		return;
	}

	if (!strstr (argument, "\"") && !strstr (argument, "\'"))
	{
		ch->send_to_char
			("You must enclose the book's desired title in quotation marks.\n");
		return;
	}

	if (!ch->writes)
	{
		ch->send_to_char
			("In which script would you like to write? (See the SCRIBE command.)\n");
		return;
	}

	argument = one_argument (argument, buf);

	if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
	{
		ch->send_to_char("You need to be holding the book you wish to title.\n");
		return;
	}

	if (obj->book_title && (!ch->get_trust()))
	{
		ch->send_to_char("This work has already been titled.\n");
		return;
	}

	if (obj->obj_flags.type_flag != ITEM_BOOK)
	{
		ch->send_to_char("This command only works with books.\n");
		return;
	}

	skill =
		(ch->skill_map[ch->writes] * 0.70) + (ch->skill_map[ch->speaks] * 0.30);

	skill = (int) skill;
	skill = MIN (95, (int) skill);

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char("What did you wish to title this work?\n");
		return;
	}

	if (strlen (buf) > 55)
	{
		ch->send_to_char("There is a 55-character limit on book titles.\n");
		return;
	}

	obj->book_title = duplicateString (buf);
	obj->title_skill = (int) skill;
	obj->title_script = lookup_skill_id(ch->writes);
	obj->title_language = lookup_skill_id(ch->speaks);

	sprintf (echo, "You have entitled #2%s#0 '%s'.", obj->short_description,
		buf);
	ch->act(echo, false, 0, 0, TO_CHAR | _ACT_FORMAT);

}


char *tilde_eliminator (char *string)
{
	char *p = '\0';

	while ((p = strchr (string, '~')))
		*p = '-';

	return duplicateString (string);
}




char *find_ex_description (char *word, int room_num)
{
	int iter;
	ROOM_DATA *troom;
	
	troom = vtor(room_num);
	if (!troom)
		return NULL;
	
	for (iter = 0; iter <= MAX_EX_DESCR; iter++)
	{
		if (!troom->ex_description[iter])
			continue;
		
		if (isname(word, troom->ex_description[iter]->keyword))
		{
			return(troom->ex_description[iter]->description);
		}
	}
	

	return NULL;
}

//slope is measured in gradients, so 100 is vertical and 50 is a 45 degeree angle
void find_portal_description (ROOM_EXIT_DATA *exit, CHAR_DATA* tch)
{
	std::string outbuf;
	std::string slope_buf;
	
	outbuf.clear();
	slope_buf.clear();
	
	
		//get full description of the portal
	if (exit->fdesc && *exit->fdesc)
		outbuf.assign(exit->fdesc);
	
		//positive slopes
	if ((exit->slope >= 0) && (exit->slope <= 2))
		slope_buf.assign("\nThe path is nearly level.\n");
	
	else if ((exit->slope > 2) && (exit->slope <= 10))
		slope_buf.assign("\nThe path slopes gently upwards from here.\n");
		
	else if ((exit->slope > 10) && (exit->slope <= 25))
		slope_buf.assign("\nThe path slopes strongly upwards from here.\n");
	
	else if ((exit->slope > 25) && (exit->slope <= 50))
		slope_buf.assign("\nThe path slopes steeply upwards from here.\n");
	
	else if ((exit->slope > 50) && (exit->slope <= 75))
		slope_buf.assign("\nThe path slopes sharply upwards from here.\n");
	
	else if ((exit->slope > 75) && (exit->slope <= 100))
		slope_buf.assign("\nThe path is a very nearly sheer climb from here.\n");
	
		//negative slopes
	if ((exit->slope < 0) && (exit->slope >= -2))
		slope_buf.assign("\nThe path is nearly level.\n");
	
	else if ((exit->slope < -2) && (exit->slope >= -10))
		slope_buf.assign("\nThe path slopes gently downwards from here.\n");
	
	else if ((exit->slope < -10) && (exit->slope >= -25))
		slope_buf.assign("\nThe path slopes strongly downwards from here.\n");
	
	else if ((exit->slope < -25) && (exit->slope >= -50))
		slope_buf.assign("\nThe path slopes steeply downwards from here.\n");
	
	else if ((exit->slope < -50) && (exit->slope >= -75))
		slope_buf.assign("\nThe path slopes sharply downwards from here.\n");
	
	else if ((exit->slope < -75) && (exit->slope >= -100))
		slope_buf.assign("\nThe path is nearly a sheer drop from here.\n");
	
	
						
	if (!outbuf.empty())
	{
		outbuf.assign(slope_buf);
		tch->send_to_char(outbuf.c_str());
		
	}
	else if (!slope_buf.empty())
	{
		tch->send_to_char(slope_buf.c_str());
	}
	
	return;

	
}

/* New version of the compare command to compare objects

This function takes two object references as arguments
and compares attributes such as weight and cost to give
an in character comparison string. - Valarauka

*/
void do_compare(CHAR_DATA * ch, char *argument, int cmd)
{
	char		arg1 [MAX_STRING_LENGTH] = { '\0' };
	char		arg2 [MAX_STRING_LENGTH] = { '\0' };
	OBJ_DATA	*obj1 = NULL;
	OBJ_DATA	*obj2 = NULL;
	std::stringstream buffer;

	
	/*** CHECK FOR POSTIONS AND CONDITONS FIRST ***/

	if ( ch->get_position() < SLEEP ) {
		ch->send_to_char("You are unconscious!\n");
		return;
	}

	if ( ch->get_position() == SLEEP ) {
		ch->send_to_char("You are asleep.\n");
		return;
	}

	if ( ch->is_blind() ) {
		ch->send_to_char("You are blind!\n");
		return;
	}

	/*** Make sure enough arguments are specified***/

	argument = one_argument (argument, arg1);

	if ( !*arg1 ) {
		ch->send_to_char("Compare what?\n");
		return;
	}

	argument = one_argument (argument, arg2);

	if ( !*arg2 ) {
		ch->send_to_char("Compare it with what?\n");
		return;
	}


	/*** Find the objects being compared ***/

	if ( !(obj1 = get_obj_in_dark (ch, arg1, ch->right_hand)) &&
		!(obj1 = get_obj_in_dark (ch, arg1, ch->left_hand)) &&
		!(obj1 = get_obj_in_dark (ch, arg1, ch->equip)) &&
		!(obj1 = get_obj_in_dark (ch, arg1, ch->room->contents))) {

			ch->send_to_char("You don't see that.\n");
			return;
	}

	if ( !(obj2 = get_obj_in_dark (ch, arg2, ch->right_hand)) &&
		!(obj2 = get_obj_in_dark (ch, arg2, ch->left_hand)) &&
		!(obj2 = get_obj_in_dark (ch, arg2, ch->equip)) &&
		!(obj2 = get_obj_in_dark (ch, arg2, ch->room->contents))) {

			ch->send_to_char("You don't see that.\n");
			return;
	}

	/*** Compared objects must be of the same type ***/

	if(obj1->obj_flags.type_flag!=obj2->obj_flags.type_flag)
	{
		ch->send_to_char("You can only compare similar objects.");
		return;
	}

	/*** Cannot compare something to itself ***/
	if(obj1==obj2)
	{
		ch->send_to_char("Compare it with itself?");
		return;
	}

	/*** Start Comparison Proper ***/

	buffer.sync();
	buffer << "\nYou compare #2";
	buffer << obj1->short_description ;
	buffer << "#0 with #2" ;
	buffer << obj2->short_description ;
	buffer << "#0\n" ;
	

	ch->act(buffer.str().c_str(), false, 0, 0, TO_CHAR | _ACT_FORMAT);
	ch->send_to_char("\n");

	/*** Compare weights ***/
	if((obj1->obj_flags.weight + obj1->contained_wt) >
		(obj2->obj_flags.weight + obj2->contained_wt))
	{
		buffer.sync();
		buffer << "\nAfter a quick appraisal, you guess that #2";
		buffer << obj1->short_description ;
		buffer << "#0 is heavier than #2" ;
		buffer << obj2->short_description ;
		buffer << "#0\n" ;
		
	}
	else if((obj2->obj_flags.weight + obj2->contained_wt) >
		(obj1->obj_flags.weight + obj1->contained_wt))
	{
		buffer.sync();
		buffer << "\nAfter a quick appraisal, you guess that #2";
		buffer << obj1->short_description ;
		buffer << "#0 is lighter than #2" ;
		buffer << obj2->short_description ;
		buffer << "#0\n" ;

	}
	else
	{
		buffer.sync();
		buffer << "\nAfter a quick appraisal, you guess that #2" ;
		buffer << obj1->short_description ;
		buffer << "#0 weights about the same as #2" ;
		buffer << obj2->short_description ;
		buffer << "#0\n" ;
		
	}

	ch->act(buffer.str().c_str(), false,  0, 0, TO_CHAR | _ACT_FORMAT);	
	ch->send_to_char("\n");

	/*** Compare values ***/

	if(obj1->coppers > obj2->coppers)
	{
		buffer.sync();
		buffer << "\nYou estimate that #2";
		buffer << obj1->short_description ;
		buffer << "#0 looks to be worth more than #2" ;
		buffer << obj2->short_description ;
		buffer << "#0\n" ;

	}
	else if(obj2->coppers > obj1->coppers)
	{
		buffer.sync();
		buffer << "\nYou estimate that #2" ;
		buffer << obj1->short_description ;
		buffer << "#0 looks to be worth less than #2" ;
		buffer << obj2->short_description ;
		buffer << "#0\n" ;
	}
	else
	{
		buffer.sync();
		buffer << "\nYou estimate that #2" ;
		buffer << obj1->short_description ;
		buffer << "#0 looks to be worth about the same as #2" ;
		buffer << obj2->short_description ;
		buffer << "#0\n" ;
	}

	ch->act(buffer.str().c_str(), false,  0, 0, TO_CHAR | _ACT_FORMAT);
	ch->send_to_char("\n");

	/*** If food item compare decay timers***/
	if(obj1->obj_flags.type_flag == ITEM_FOOD)
	{
		if ((obj1->morphTime)&&(obj2->morphTime))
		{
			int time1, time2;
			
			time1 = obj1->morphTime - time (0);
			time2 = obj2->morphTime - time (0);
			
			if(time1 > time2)
			{
				buffer.sync();
				buffer << "\nAs you look over #2";
				buffer << obj1->short_description;
				buffer << "#0 you notice that it appears fresher than  #2";
				buffer << obj2->short_description;
				buffer << "#0\n";
				
			}
			else if(time2 > time1)
			{
				buffer.sync();
				buffer << "\nAs you look over #2";
				buffer << obj1->short_description;
				buffer << "#0 you notice that it appears less fresh than  #2";
				buffer << obj2->short_description;
				buffer << "#0\n";
			}
			else
			{
				buffer.sync();
				buffer << "\nAs you look over #2";
				buffer << obj1->short_description;
				buffer << "#0 you notice that it appears about as fresh as #2";
				buffer << obj2->short_description;
				buffer << "#0\n";
			}
			
			ch->act(buffer.str().c_str(), false,  0, 0, TO_CHAR | _ACT_FORMAT);
			ch->send_to_char("\n");
		}
	}

	/***** Compare time left in light objects ***/
	if ( obj1->obj_flags.type_flag == ITEM_LIGHT )
	{
		if ( obj1->o.light.hours > obj2->o.light.hours )
		{
			buffer.sync();
			buffer << "\nFrom the look of it #2";
			buffer << obj1->short_description;
			buffer << "#0 will provide light for longer than  #2";
			buffer << obj2->short_description;
			buffer << "#0\n";
		}
		else if(obj2->o.light.hours > obj1->o.light.hours)
		{
			buffer.sync();
			buffer << "\nFrom the look of it #2";
			buffer << obj1->short_description;
			buffer << "#0 will provide light for less time than  #2";
			buffer << obj2->short_description;
			buffer << "#0\n";
		}
		else
		{
			buffer.sync();
			buffer << "\nFrom the look of it #2";
			buffer << obj1->short_description;
			buffer << "#0 will provide light for about as long as #2";
			buffer << obj2->short_description;
			buffer << "#0\n";
		}

		ch->act(buffer.str().c_str(), false,  0, 0, TO_CHAR | _ACT_FORMAT);
		ch->send_to_char("\n");

	}

	/**** Compare capacity of containers ****/

	
	if ((obj1->obj_flags.type_flag == ITEM_CONTAINER)||
		( obj1->obj_flags.type_flag == ITEM_DRINKCON )||
		( obj1->obj_flags.type_flag == ITEM_DRYCON ))
	{
		if(obj1->o.od.value[0] > obj2->o.od.value[0])
		{
			buffer.sync();
			buffer << "\nFrom the look of it #2";
			buffer << obj1->short_description;
			buffer << "#0 will hold more than #2";
			buffer << obj2->short_description;
			buffer << "#0\n";
			
		}
		else if(obj2->o.od.value[0] > obj1->o.od.value[0])
		{
			buffer.sync();
			buffer << "\nFrom the look of it #2";
			buffer << obj1->short_description;
			buffer << "#0 will hold less than #2";
			buffer << obj2->short_description;
			buffer << "#0\n";
		}
		else
		{
			buffer.sync();
			buffer << "\nFrom the look of it #2";
			buffer << obj1->short_description;
			buffer << "#0 will hold about the same as #2";
			buffer << obj2->short_description;
			buffer << "#0\n";
		}
		
		ch->act(buffer.str().c_str(), false,  0, 0, TO_CHAR | _ACT_FORMAT);
		ch->send_to_char("\n");
	}


	return;
}

const char * writing_adj (int skill)
{
	if (skill <= 10)
		return "crudely";
	if (skill <= 20)
		return "poorly";
	if (skill <= 30)
		return "functionally";
	if (skill <= 40)
		return "with skill";
	if (skill <= 50)
		return "with great skill";
	if (skill <= 60)
		return "artfully";
	if (skill <= 70)
		return "beautifully";
	return "flawlessly";
}

int decipher_script (CHAR_DATA * ch, char * script, char * language, int skill)
{
	double check = 0;

	if ((ch->skill_map[script] < 1) || (ch->skill_map[language] < 1))
		return 0;

	if (skill > 0 && skill <= 15)
		check = 70;
	else if (skill > 15 && skill < 30)
		check = 50;
	else if (skill >= 30 && skill < 50)
		check = 30;
	else if (skill >= 50 && skill < 70)
		check = 20;
	else if (skill >= 70)
		check = 10;

	skill_use (ch, script, 0);
	skill_use (ch, language, 0);

	if (((ch->skill_map[script] * .70) + (ch->skill_map[language] * .30) ) >= check)
		return 1;
	else
		return 0;
}

void reading_check (CHAR_DATA * ch, OBJ_DATA * obj, WRITING_DATA * writing,
			   int page)
{
	std::stringstream output;
	char * lang_skill_name;
	char * script_skill_name;
	
	if (!writing || !writing->message)
	{
		ch->send_to_char
			("There seems to be a problem with this object. Please report it to a staff member.\n");
		return;
	}
	
	lang_skill_name = strdup(lookup_skill_name(writing->language));
	script_skill_name = strdup(lookup_skill_name(writing->script));
	
	if ((ch->skill_map[script_skill_name] < 1)
	&& (strcasecmp (writing->author, ch->name) != STR_MATCH))
	{
		output.sync();
		output << "This document is written in a script entirely unfamiliar to you.";
		ch->act (output.str().c_str(), false,  0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if ((ch->skill_map[lang_skill_name] < 1)
		&& (ch->skill_map[script_skill_name] > 0)
		&& strcasecmp (writing->author, ch->name) != STR_MATCH)
	{
		output.sync();
		output << "Although you recognise the script as " << script_skill_name << ", the mode and language in which this document is written is unknown to you.";
		ch->act (output.str().c_str(), false,  0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (!decipher_script
		(ch, script_skill_name, lang_skill_name, writing->skill)
		&& (strcasecmp (writing->author, ch->name) != STR_MATCH
		|| (ch->skill_map[script_skill_name] < 1)))
	{
		output.sync();
		output << "You find that you can make neither heads nor tails of this document.";
		ch->act (output.str().c_str(), false,  0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (!page)
	{
		output.sync();
		output << "#2On "; 
		output << obj->short_description;
		output << "#0, ";
		output << script_skill_name;
		output << " sigils of the ";
		output << lang_skill_name;
		output << " mode, scribed ";
		output << writing_adj (writing->skill);
		output << " in ";
		output << writing->ink;
		output << ", bear a message in the ";
		output << lang_skill_name;
		output << " tongue:";
	}
	else
	{
		output.sync();
		output << "On #2page ";
		output << page - 1;
		output << "#0, ";
		output << script_skill_name;
		output << " sigils of the ";
		output << lang_skill_name;
		output << " mode, scribed ";
		output << writing_adj (writing->skill);
		output << " in ";
		output << writing->ink;
		output << ", bear a message in the ";
		output << lang_skill_name;
		output << " tongue:";
	}

	ch->act (output.str().c_str(), false,  0, 0, TO_CHAR | _ACT_FORMAT);
	ch->send_to_char("\n");

	if (strcasecmp (writing->author, ch->name) != STR_MATCH)
	{
		skill_use (ch, script_skill_name, 0);
		if (!number (0, 1))
			skill_use (ch, lang_skill_name, 0);
	}

	output.sync();
	output << writing->message;
	page_string (ch->desc, output.str().c_str());
}

const char * article (const char *string)
{
	if (strcasecmp (string, "something") == STR_MATCH)
		return "";

	if (*string == 'a' || *string == 'e' || *string == 'i' ||
		*string == 'o' || *string == 'u')
		return "an ";
	else
		return "a ";
}
	

/***
 * TODO
 * Really need to break this down into separate functions instead of one large mess
 ***/
/************************************************************
 * mode == 0 - general look, no tables
 * mode == 1 - a container object that has contents/items worn by mobile
 * mode == 2
 * mode == 3 - invisible objects?
 * mode == 4 
 * mode == 5 - general look, all objects
 * mode == 6 - invisible items
 * mode == 7 - a container itself, tables will show illumination
 * mode == 15 - examine, shows extra information
 *
 ***********************************************************/
void show_obj_to_char (OBJ_DATA * obj, CHAR_DATA * ch, int mode, int level_see)
{
	int found = 0;
	int first_seen;
	unsigned int i = 0;
	WRITING_DATA *writing = NULL;
	CHAR_DATA *tch = NULL;
	OBJ_DATA *obj2 = NULL, *tobj = NULL;
	AFFECTED_TYPE *af = NULL;
	CLAN_DATA *tclan = NULL;
	char *p = '\0';
	std::stringstream buffer;
	std::stringstream tempstr;
	char buffer2[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	int count;
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	std::set<std::string>::iterator obj_it;
	std::map<int, OBJECT_MATERIAL*>::iterator it_material;
	extern const char *damage_severity[];
	
	
	
	
		//General look, if we have a description
		//this may be affected by light levels
	if ((mode == 0) && obj->description)
	{
		
		if (IS_TABLE (obj))
			return;
		
		buffer << "#2";
		
		if (obj->omote_str)
		{
			if (level_see == 2)
			{
			sprintf (buf, "%s %s", obj->short_description, obj->omote_str);
			}
			else 
			{
				sprintf (buf, "something %s", obj->omote_str);
			}

			if ((obj->count > 1) && (!IS_SET (obj->obj_flags.extra_flags, ITEM_STACK)))
			{
				sprintf (buf + strlen (buf), " (x%d)", obj->count);
			}
			if (buf[0] == '#')
			{
				buf[2] = toupper (buf[2]);
			}
			else
			{
				buf[0] = toupper (buf[0]);
			}
			buffer << buf;
		}
		else
		{
			buffer << obj_desc (obj, level_see);
			
		}
		
		if (get_obj_affect (obj, MAGIC_HIDDEN))
			buffer << " (hidden from view)";
		
		if (IS_SET (obj->obj_flags.extra_flags, ITEM_VNPC))
			buffer << " #6(vNPC)#0";
		
		
		buffer << ("#0");
	}
	
		//general look at a container
	if (mode == 7 && obj->description)
	{
		buffer.sync();
		buffer << "#2" << obj_desc (obj, level_see) << "#0";
		
		if (IS_TABLE (obj))
		{
			for (obj2 = obj->contains; obj2; obj2 = obj2->next_content)
				if (obj2->obj_flags.type_flag == ITEM_LIGHT && obj2->o.light.hours && obj2->o.light.on)
				{
					buffer << " #1(Illuminated)#0";
					continue;
				}
		}
	}
	
		//general look at item without a description
	else if (obj->short_description &&
			 ((mode == 1) 
			  || (mode == 2)
			  || (mode == 3)
			  || (mode == 4)))
	{
		
		buffer << "#2";
		buffer << obj_short_desc (obj);
		buffer << "#0";
		
		if (IS_SET (obj->obj_flags.extra_flags, ITEM_DESTROYED))
			buffer << " #1(destroyed)#0";
		
		if ((obj->count > 1) && (!IS_SET (obj->obj_flags.extra_flags, ITEM_STACK)))
			buffer << " (x%d)", obj->count;
	}
	
		//Displays the base description or sdesc for all items
		//this may be affected by light levels
	else if (mode == 5 || mode == 15)
	{
		
		if (obj->obj_flags.type_flag == ITEM_PARCHMENT)
		{
			if (!obj->writing_loaded)
				load_writing (obj);
			if (obj->writing && obj->writing->message)
			{
				if (ch->get_trust())
				{
					sprintf (buf, "#2[Penned by %s on %s.]#0\n\n",
							 obj->writing->author, obj->writing->date);
					ch->send_to_char(buf);
				}
				
				if (obj->carried_by 
					&& obj->carried_by != ch 
					&& (!ch->get_trust()))
				{
					ch->send_to_char
					("You aren't quite close enough to make out the words.\n");
					return;
				}
				
				if (level_see == 1)
				{
					ch->send_to_char
					("It is too dark to make out the words.\n");
					return;
				}
				
				reading_check (ch, obj, obj->writing, 0);
				return;
			}
			else
			{
				sprintf (buf, "%s#0 seems to be blank.",
						 obj->short_description);
				buffer.sync();
				buffer << "#2" << CAP(buf) << "#0\n";
				return;
			}
		}
		
		else if (obj->obj_flags.type_flag == ITEM_BOOK)
		{
			if (!obj->writing_loaded)
				load_writing (obj);
			if (!obj->open)
			{
				if (obj->full_description && *obj->full_description)
				{
					buffer.sync();
					buffer << "\n" << obj->full_description;
				}
				else
				{
					buffer.sync();
					buffer << "\n  It is #2" << OBJS (obj, ch) << "#0";
				}
				
				buffer << " #6This book seems to have " << obj->o.od.value[0] << " pages remaining.#0";
				
				if (obj->book_title)
				{
					if (level_see == 1)
						sprintf (buf, " #6You cannot quite see the book's title.#0");
					
					else if (decipher_script
							 (ch, lookup_skill_name(obj->title_script), lookup_skill_name(obj->title_language),
							  obj->title_skill))
						sprintf (buf, " #6The book has been entitled '%s.'#0",
								 obj->book_title);
					
					else
						sprintf (buf,
								 " #6You cannot quite decipher what seems to be this book's title.#0");
					
					reformat_string (buf, &p);
					buffer << "\n" << p;
					free_mem (p); //char*
				}
				page_string (ch->desc, buffer.str().c_str());
				return;
			}
			if (obj->carried_by && obj->carried_by != ch && (!ch->get_trust()))
			{
				ch->send_to_char
				(" You aren't quite close enough to make out the words.\n");
				return;
			}
			
			if (!obj->writing || !obj->o.od.value[0])
			{
				ch->send_to_char(" All its pages have been torn out.\n");
				return;
			}
			
			for (i = 2, writing = obj->writing; i <= obj->open; i++)
			{
				if (!writing->next_page)
					break;
				writing = writing->next_page;
			}
			
			if (writing && writing->message)
			{
				if (strcasecmp (writing->message, "blank") != STR_MATCH)
				{
					if (ch->get_trust())
					{
						sprintf (buf, "#2[Penned by %s on %s.]#0\n\n",
								 writing->author, writing->date);
						ch->send_to_char(buf);
					}
					else if (level_see == 1)
					{
						ch->send_to_char("It is too dark to make out the words.\n");
					}
					
					else 
						reading_check (ch, NULL, writing, i);
					
					return;
				}
				else
				{
					sprintf (buf, "This page seems to be blank.");
					buffer.sync();
					buffer << CAP (buf) << "\n";
					ch->send_to_char(buffer.str().c_str());
					return;
				}
			}
			else
			{
				sprintf (buf, "This page seems to be blank.");
				buffer.sync();
				buffer << CAP (buf) << "\n";
				ch->send_to_char(buffer.str().c_str());
				return;
			}
		}
		
		else
		{
			if (obj->full_description && *obj->full_description)
			{
				if ((obj->count > 1) && (!IS_SET (obj->obj_flags.extra_flags, ITEM_STACK)))
				{
					buffer.sync();
					buffer << "    It is #2" << OBJS (obj, ch) << "#0. (x" << obj->count << "). ";
				}
				else
				{
					buffer.sync();
					buffer << "    It is #2" << OBJS (obj, ch) << "#0.";
				}
				
				buffer << "\n" << obj->full_description;
			}
			else
			{
				if ((obj->count > 1) && (!IS_SET (obj->obj_flags.extra_flags, ITEM_STACK)))
				{
					buffer.sync();
					buffer << "    It is #2" << OBJS (obj, ch) << "#0. (x" << obj->count << "). ";
				}
				else
				{
					buffer.sync();
					buffer << "    It is #2" << OBJS (obj, ch) << "#0.";
				}
			}
			
			
			if (mode == 15)
			{
				buffer << " " << object__examine_damage (obj, ch);
			}
			
			if (IS_WEARABLE (obj) && obj->size && mode == 15)
			{
				buffer << " This garment would fit individuals wearing size " << sizes_named[obj->size] << ".";
			}
			
				// Show information about how many spaces at a table are taken or available 
			if (IS_TABLE (obj))
			{
					//get the number of people currently sat at the table
				count = 0;
				for (tch = ch->room->people; tch; tch = tch->next_in_room)
				{
					if ((af = get_affect (tch, MAGIC_SIT_TABLE)) &&
						af->a.table.obj == obj)
						count++;
				}
				
					//append approrpiate message
				if(obj->o.container.table_max_sitting == 0)
				{
					buffer << " There is plenty of space here.";
				}
				else if (obj->o.container.table_max_sitting == 1)
				{
					if(count==1)
					{
						buffer << " The single space here is taken.";
					}
					else
					{
						buffer << " The one space here is available.";
					}
				}
				else
				{
					if (!count)
					{
						
						buffer << " None of the " << obj->o.container.table_max_sitting << " places here are taken.";
					}
					else
					{
						buffer << " " << count << " of the " << obj->o.container.table_max_sitting << " places here ";
						if (count == 1)
							buffer << "is taken";
						else 
							buffer << "are taken";
						
					}
				}
			}
			
			if (obj->obj_flags.type_flag == ITEM_TOSSABLE && mode == 15
				&& obj->desc_keys && strlen (obj->desc_keys))
			{
				
				buffer << " #6Its sides are marked as follows:#0  #6";
				
				for (i = 0; i < strlen (obj->desc_keys); i++)
				{
					if (obj->desc_keys[i] == ' ')
					{
						buffer << ", ";
					}
					else
					{
						buffer << obj->desc_keys[i];
					}
				}
				
				buffer << "#0\n";
				
			}
			
			
			if (obj->obj_flags.type_flag == ITEM_REPAIR_KIT && mode == 15)
			{
				char* skill_name = lookup_skill_name(obj->o.od.value[3]);
				
				if (obj->o.od.value[3] < 0 && obj->o.od.value[5] < 0)
				{
					buffer << " This kit cannot repair anything!";
				}
				else if (obj->o.od.value[3]
						 && (ch->skill_map[skill_name] < obj->o.od.value[2]))
				{
					buffer << "  Your training is not yet sufficient to use these materials.";
				}
				else
				{
					buffer << " It can be used to repair";
					if (obj->o.od.value[5] == 0)
					{
						buffer << " any item made with";
					}
					else
					{
						buffer << " %s items made with" << item_types[(obj->o.od.value[5])];
					}
					if (obj->o.od.value[3] == 0)
					{
						buffer << " any skill.";
					}
					else
					{
						buffer << " the ";
						buffer << lookup_skill_name(obj->o.od.value[3]);
						buffer << " skill.";
					}
					
					if (obj->o.od.value[4] > 0)
					{
						buffer << " The maximum damage that can be repaired is ";
						buffer << damage_severity[obj->o.od.value[4]];
						buffer << ".";
					}
					
					if (obj->o.od.value[0] < 0)
					{
						buffer << " It has unlimited uses remaining.";
					}
					else 
					{
						buffer << " It has ";
						buffer << obj->o.od.value[0];
						buffer << " uses remaining.";
					}
				}
			}
			
			if (obj->obj_flags.type_flag == ITEM_INK && mode == 15)
			{
				buffer << " It appears to contain ";
				buffer << obj->ink_color;
				buffer << ".";
			}
			
			if (obj->obj_flags.type_flag == ITEM_DRYCON && mode == 15)
			{
				buffer << " It appears to hold about ";
				buffer << (obj->o.drycon.max_weight/100);
				buffer << " pounds.";
				
			}
			
			if (obj->clan_data)
			{
				if (is_clan_member(ch, obj->clan_data->name))
				{
					tclan = get_clandef(obj->clan_data->name);
					if (tclan)
					{
						buffer << " You see the mark of ";
						buffer << tclan->literal;
						buffer << ", bearing the rank of ";
						buffer << obj->clan_data->rank;
						buffer << ".";
						
						
					}
				}
			}
			
			/**** TODO change when crafts are updated
			 //go through the list of all possible crafts, since IMMORTALs know everything
			 if (ch->get_trust())
			 {
			 for (tcraft_iterator = craft_map.begin(); tcraft_iterator != craft_map.end(); tcraft_iterator++)
			 {
			 tcraft = tcraft_iterator->second;
			 if (!craft_uses (tcraft, obj->nVirtual))
			 continue;
			 if (crafts_found)
			 sprintf (buf + strlen (buf), ", ");
			 sprintf (buf + strlen (buf), "'%s %s'",
			 tcraft->command,
			 tcraft->subcraft_name);
			 crafts_found += 1;
			 }
			 }
			 else 
			 {
			 //loop through all of the crafts the PC knows
			 for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
			 {
			 if (!(af = get_affect (ch, i)))
			 continue;
			 if (!af->a.craft || !af->a.craft->subcraft)
			 continue;
			 if (!craft_uses (af->a.craft->subcraft, obj->nVirtual))
			 continue;
			 if (crafts_found)
			 sprintf (buf + strlen (buf), ", ");
			 sprintf (buf + strlen (buf), "'%s %s'",
			 af->a.craft->subcraft->command,
			 af->a.craft->subcraft->subcraft_name);
			 crafts_found += 1;
			 }
			 
			 }
			 
			 
			 if (crafts_found && mode == 15)
			 {
			 sprintf (output,
			 "   You realize that you could make use of this item in the following craft%s: %s.",
			 crafts_found != 1 ? "s" : "", buf);
			 reformat_string (output, &p);
			 buffer << "\n%s\n", p);
			 free_mem (p); // char*;
			 p = 0;
			 }
			 *************/
			
			
			first_seen = 1;
			
			if (IS_SET (obj->obj_flags.extra_flags, ITEM_TABLE))
			{
				for (obj2 = obj->contains; obj2; obj2 = obj2->next_content)
				{
					if (can_see_obj(ch, obj2))
					{
						if (first_seen)
							buffer << " On #2" <<OBJS (obj, ch) << "#0 you see";
						
						first_seen = 0;
						buffer << " " << OBJS (obj2, ch);
						
						if (obj2->obj_flags.type_flag == ITEM_LIGHT 
							&& obj2->o.light.hours 
							&& obj2->o.light.on)
							buffer << " #1(lit)#0";
						
						if (obj2->next_content)
							buffer << ", ";
						else
							buffer << ". ";
					}
				}
				
				/******
				 
				 //places available
				 count = 0;
				 for (tch = ch->room->people; tch; tch = tch->next_in_room)
				 {
				 if ((af = get_affect (tch, MAGIC_SIT_TABLE)) &&
				 af->a.table.obj == obj)
				 count++;
				 }
				 
				 if(obj->o.container.table_max_sitting == 0)
				 {
				 buffer << "\n   #6There is plenty of space here.#0");
				 }
				 else if (obj->o.container.table_max_sitting == 1)
				 {
				 if(count==1)
				 {
				 buffer << "\n   #6The single space here is taken.#0");
				 }
				 else
				 {
				 buffer << "\n   #6The one space here is available.#0");
				 }
				 }
				 else
				 {
				 buffer << 
				 "\n   #6%d of the %d places here %s taken.#0",
				 count,obj->o.container.table_max_sitting,count==1 ? "is" : "are");
				 }
				 ****/
			}
			
			
				//Materials and overall condition of item
			int counter = 0; //number of items remaining in obj->material
			for (i = 0; i < MAX_OBJ_MATERIALS; i++)
			{
				sprintf(buf2, "%s", obj->materials[i]);
				if (str_cmp(buf2, ""))
				{
					counter ++;
				}
			}
			for (i = 0; i < MAX_OBJ_MATERIALS; i++)
			{
				for (it_material = object_material_map.begin(); it_material != object_material_map.end(); it_material++)
				{
					if (!str_cmp(it_material->second->material_name, obj->materials[i]))
					{
						
						if (mode == 15)
						{
							buffer << " ";
							if 	(obj->item_wear > 100 )
							{
								buffer << it_material->second->qual_strings[7];
							}	
							else if (obj->item_wear == 100 )
							{
								buffer << it_material->second->qual_strings[6];
							}
							else if (obj->item_wear < 100 && obj->item_wear >= 90)
							{
								buffer << it_material->second->qual_strings[5];
							}
							else if (obj->item_wear < 90 && obj->item_wear >= 70)
							{
								buffer << it_material->second->qual_strings[4];
							}
							else if (obj->item_wear < 70 && obj->item_wear >= 50)
							{
								buffer << it_material->second->qual_strings[3];	
							}
							else if (obj->item_wear < 50 && obj->item_wear >= 20)
							{
								buffer << it_material->second->qual_strings[2];
							}
							else if (obj->item_wear < 20 && obj->item_wear >= 0)
							{
								buffer << it_material->second->qual_strings[1];
							}
							else if (obj->item_wear < 0)
							{
								buffer << it_material->second->qual_strings[0];
							}
						}
						else if (mode == 5)
						{
							if (counter == 1)
								sprintf (buffer2 + strlen(buffer2), " %s.", obj->materials[i]);
							else if (counter == 2)
								sprintf (buffer2 + strlen(buffer2), " %s, and ", obj->materials[i]);
							else if (counter == 3)
								sprintf (buffer2 + strlen(buffer2), " %s, ", obj->materials[i]);
							
							counter--;
						}
					}
				}
				
			}
			
			if (mode == 15)
			{
				if (*buffer2 && (str_cmp(buffer2," none.")))
				{
					buffer << " This ";
					buffer << fname(obj->name);
					buffer << " is made from ";
					buffer << buffer2;
					buffer << ".";
				}
				else if (*buffer2)
				{
					buffer << " This ";
					buffer << fname(obj->name);
					buffer << "is made from an unknown material.";
				}
				
			}
			
		}
	}
		//tag for invisible items
	else if (mode == 6)
	{
		if (can_see_obj(ch, obj) && IS_OBJ_STAT (obj, ITEM_INVISIBLE))
			buffer << "(invis) ";
		
		buffer << "#2";
		buffer << obj_short_desc (obj);
		buffer << "#0";
		
		
		if (ch->get_trust() && (obj->obj_flags.type_flag == ITEM_MONEY))
			buffer << " (" << obj->count << ")";
	}
	
	if (mode != 3)
	{
		found = false;
		if (IS_OBJ_STAT (obj, ITEM_INVISIBLE))
		{
			buffer << "(invisible)";
			found = true;
		}
	}
	
	if (mode == 1)
	{
			//extra info for special cases
		
			//container that is a light
		if (obj->obj_flags.type_flag == ITEM_LIGHT)
		{
			if (obj->o.light.hours && obj->o.light.on)
				buffer << " #1(lit)#0";
			else if (obj->o.light.hours && !obj->o.light.on)
				buffer << " #1(unlit)#0";
			else
				buffer << " #1(spent)#0";
		}
		
			//books
		if (obj->obj_flags.type_flag == ITEM_BOOK)
		{
			if(obj->book_title && decipher_script (ch, lookup_skill_name(obj->title_script), lookup_skill_name(obj->title_language), obj->title_skill))
			{
				buffer << " #2\"";
				buffer << obj->book_title;
				buffer << "\"#0";	
			}
			
			if (obj->open)
				buffer << " #1(open)#0";
			else if (!obj->open)
				buffer << " #1(closed)#0";
		}
		
			//Ink
		if (obj->obj_flags.type_flag == ITEM_INK)
		{
			if (obj->o.od.value[0] == obj->o.od.value[1])
				buffer << " #1(full)#0";
			else if (obj->o.od.value[0] > obj->o.od.value[1] / 2)
				buffer << " #1(mostly full)#0";
			else if (obj->o.od.value[0] == obj->o.od.value[1] / 2)
				buffer << " #1(half full)#0";
			else if (obj->o.od.value[0] < obj->o.od.value[1] / 2
					 && obj->o.od.value[0] > 0)
				buffer << " #1(mostly empty)#0";
			else if (obj->o.od.value[0] <= 0)
				buffer << " #1(empty)#0";
		}
		
	}
	buffer << "\n";
	
	if ((mode == 1) 
		 || (mode == 2)
		 || (mode == 3)
		 || (mode == 4))
	{
		reformat_string ((char*)buffer.str().c_str(), &p);
		page_string (ch->desc, p);
		free_mem (p); //char*
		p = NULL;				
	}
	else 
	{
		tempstr << reformat_desc((char*)buffer.str().c_str());
		page_string (ch->desc, tempstr.str().c_str());
		
	}
	
	
	


}

void list_portals_to_char(ROOM_DATA * troom, CHAR_DATA * ch)
{
	ROOM_PORTAL_DATA * tport = NULL;
	std::stringstream outbuf;
	int iter;
	
	outbuf.sync();
	
	for (iter = 0; iter <= MAX_PORTALS; iter++)
	{
		tport = vtop(troom->portals[iter]);
		if (tport)
		{
			if (tport->type > PORTAL_GATE) //ie, barriers, crossings, windows and transports)
			{
				if (tport->room_2 == troom->nVirtual)
				{
					outbuf << "There is " << tport->sdesc_or_default(2) << " " << tport->exit_dir(2) << ".\n";
				}
				else
				{
					outbuf << "There is " << tport->sdesc_or_default(1) << " " << tport->exit_dir(1) << ".\n";
				}
				
				outbuf << reformat_desc((char*)outbuf.str().c_str());
				ch->send_to_char(outbuf.str().c_str());
				
			}
			tport = NULL;
		}
	}
}

	//light_value is based on the candlepower in the room
void list_light_to_char(ROOM_DATA * troom, CHAR_DATA * ch)
{
	char outbuf[MAX_STRING_LENGTH] = { '\0' };
	float light_value;
	std::stringstream tempstr;
	
		light_value = (troom->light);
	
		//noon or brighter
	if (light_value > (SUN_NOON/2))
		sprintf(outbuf,"   Bright light, like an unclouded noon, allows unhampered vision.\n");
	
		//not quite noon but close
	else if ((light_value <= (SUN_NOON/2)) && (light_value > (SUN_NOON/4)))
		sprintf(outbuf,"   There is enough light to dispel all but the deepest shadows.\n");
	
		//less than noon but more than early morning
	else if ((light_value <= (SUN_NOON/4)) && (light_value > SUN_RISE_SET))
		sprintf(outbuf,"   Details are easy to make out and everything is well illuminated.\n");
	
		//less than early morning but more than dawn
	else if ((light_value <= SUN_RISE_SET) && (light_value > SUN_TWILIGHT))
		sprintf(outbuf,"   The ambient light is at a comfortable level, allowing easy vision of the immediate surroundings.\n");
	
		//less than dawn but more than dim room
	else if ((light_value <= SUN_TWILIGHT) && (light_value > DEFAULT_LIGHT))
		sprintf(outbuf,"   Shadows persist in the corners, though the light is good enough to see adequately by.\n");
	
		//less dim room but more than a faint light
	else if ((light_value <= DEFAULT_LIGHT) && (light_value > FAINT_LIGHT))
		sprintf(outbuf,"   Dim light allows you to see a few feet in all directions.\n");
	
		//less than a faint light but more than a just stars
	else if ((light_value <= FAINT_LIGHT) && (light_value > NO_MOON))
		sprintf(outbuf,"   Rough outlines of objects are visible, but darkness obscures any details.\n");
	
		//less than starlight (is_dark will be true)
	else 
		sprintf(outbuf,"   Nearly impenetrable darkness hampers your vision.\n");
	
	tempstr << reformat_desc(outbuf);	
	ch->send_to_char(tempstr.str().c_str());
	
	
	return;
}

void list_obj_to_char (OBJ_DATA * list, CHAR_DATA * ch, int mode, int show)
{
	OBJ_DATA *target = NULL;
	int found = 0, j = 0;
	int looked_for_tables = 0;
	OBJ_DATA *obj = NULL;
	bool clump = false;

	found = false;

	if (!list)
	{
		if (show)
			ch->send_to_char("Nothing.\n");
		return;
	}

	for (target = list; target; target = target->next_content)
	{
		j++;
	}

	if (j >= 25 && show != 4 && !list->in_obj)
		clump = true;

	for (target = list; target; target = target->next_content)
	{
			//enough light to see everything
		if (can_see_obj(ch, target) == 2)
		{

			if (!mode && !looked_for_tables && IS_TABLE (target))
			{

				for (obj = target; obj; obj = obj->next_content)
					if (IS_TABLE (obj))
						looked_for_tables++;

				//show first four tables otherwise group them
				if (looked_for_tables < 5)
				{
					for (obj = target; obj; obj = obj->next_content)
						if (IS_TABLE (obj))
							show_obj_to_char (obj, ch, 7, 2);
				}
				else
				{
					ch->send_to_char("#6There are several furnishings here.#0\n");
				}
			}

			
			if (clump)
				continue;

			show_obj_to_char (target, ch, mode, 2);

			found = true;
		}
			//can only see some things and not much detail
		else if (can_see_obj(ch, target) == 1)
		{
			
			if (!mode && !looked_for_tables && IS_TABLE (target))
			{
				
				for (obj = target; obj; obj = obj->next_content)
					if (IS_TABLE (obj))
						looked_for_tables++;
				
					//show hint of tables
				if (looked_for_tables)
				{
					ch->send_to_char("#6There may be furnishings here.#0\n");
				}
			}
			
			
			
			if (clump)
				continue;
			
			show_obj_to_char (target, ch, mode, 1);
			
			found = true;
		}
	}

	if (clump)
	{
		if (j >= 25 && j < 35)
			ch->send_to_char("#2The area is strewn with a number of objects.#0\n");
		else if (j >= 35 && j < 45)
			ch->send_to_char
			("#2The area is strewn with a sizeable number of objects.#0\n");
		else if (j >= 45 && j < 55)
			ch->send_to_char
			("#2The area is strewn with a large number of objects.#0\n");
		else if (j >= 55 && j < 65)
			ch->send_to_char
			("#2The area is strewn with a great number of objects.#0\n");
		else
			ch->send_to_char
			("#2The area is strewn with a staggering number of objects.#0\n");
	}
	else if (!found && show && show != 4)
		ch->send_to_char("Nothing.\n");
}

void list_char_to_char (CHAR_DATA * list, CHAR_DATA * ch)
{
	CHAR_DATA *target = NULL;
	int j = 0;
	int llevel;
	

	for (target = list; target; target = target->next_in_room)
	{
		if (target == ch)
			continue;
		j++;
	}

	if (j < 25)
	{
		for (target = list; target; target = target->next_in_room)
		{
			llevel = can_see_mob(ch, target);
			if (ch != target && ch->vehicle != target)
			{
				show_char_to_char (target, ch, 0, llevel);
			}
		}
	}
	else
	{
		if (j >= 25 && j < 35)
			ch->send_to_char("#5The area is filled by a crowd of individuals.\n#0");
		if (j >= 35 && j < 45)
			ch->send_to_char
			("#5The area is filled by a decently-sized crowd of individuals.\n#0");
		else if (j >= 45 && j < 55)
			ch->send_to_char
			("#5The area is filled by a sizeable crowd of individuals.\n#0");
		else if (j >= 55 && j < 65)
			ch->send_to_char
			("#5The area is filled by a large crowd of individuals.\n#0");
		else if (j > 65)
			ch->send_to_char
			("#5The area is filled by an immense crowd of individuals.\n#0");
	}

}

void show_contents (CHAR_DATA * ch, char *argument, int cmd)
{
	if (cmd == 1)
	{
		if (!ch->room->contents)
		{
			ch->send_to_char("\n   None.\n");
			return;
		}

		if (ch->room->contents)
		{
			ch->send_to_char("\n");
			list_obj_to_char (ch->room->contents, ch, 0, 4);
		}
		return;
	}
	else if (cmd == 2)
	{
		if ((!ch->room->people || !ch->room->people->next_in_room))
		{
			ch->send_to_char("\n   None.\n");
			return;
		}

		if (ch->room->people
			&& (ch->room->people->next_in_room || ch->room->people != ch))
		{
			ch->send_to_char("\n");
			list_char_to_char (ch->room->people, ch);
		}
		return;
	}
}

void do_contents (CHAR_DATA * ch, char *argument, int cmd)
{
	if (!ch->room->contents
		&& (!ch->room->people || !ch->room->people->next_in_room))
	{
		ch->send_to_char("   None.\n");
		return;
	}

	if (ch->room->contents)
	{
		ch->send_to_char("\n");
		list_obj_to_char (ch->room->contents, ch, 0, 4);
	}

	if (ch->room->people
		&& (ch->room->people->next_in_room || ch->room->people != ch))
	{
		ch->send_to_char("\n");
		list_char_to_char (ch->room->people, ch);
	}
}



char * left_right_grip (CHAR_DATA * ch)
{
	bool right = false, left = false;

	if (get_equip (ch, WEAR_PRIM))
		right = true;
	if (get_equip (ch, WEAR_SEC))
		left = true;
	
	if (get_equip (ch, WEAR_BOTH))
	{
		right = true;
		left = true;
	}
	if (get_equip (ch, WEAR_CARRY_L))
		left = true;
	if (get_equip (ch, WEAR_CARRY_R))
		right = true;

	if ((!right && left) || (!right && !left))
		return "right";
	if (right && !left)
		return "left";
	else
		system_log ("Error in left_right_grip, act.informative.c", true);

	return NULL;
}

char * worn_third_loc (CHAR_DATA * ch, OBJ_DATA * obj)
{
	std::stringstream buffer;
	
	buffer.sync();
	
	if (obj->location == WEAR_NECK_1 || obj->location == WEAR_NECK_2)
		buffer << "around " << HSHR (ch) << " neck";
	else if (obj->location == WEAR_BODY)
		buffer << "on " << HSHR (ch) << " body";
	else if (obj->location == WEAR_HEAD)
		buffer << "on " << HSHR (ch) << " head";
	else if (obj->location == WEAR_ARMS)
		buffer << "on " << HSHR (ch) << " arms";
	else if (obj->location == WEAR_ABOUT)
		buffer << "about " << HSHR (ch) << " body";
	else if (obj->location == WEAR_WAIST)
		buffer << "around " << HSHR (ch) << " waist";
	else if (obj->location == WEAR_WRIST_L)
		buffer << "on " << HSHR (ch) << " left wrist";
	else if (obj->location == WEAR_WRIST_R)
		buffer << "on " << HSHR (ch) << " right wrist";
	else if (obj->location == WEAR_HAIR)
		buffer << "in " << HSHR (ch) << " hair";
	else if (obj->location == WEAR_FACE)
		buffer << "on " << HSHR (ch) << " face";
	else if (obj->location == WEAR_ANKLE_L)
		buffer << "on " << HSHR (ch) << " left ankle";
	else if (obj->location == WEAR_ANKLE_R)
		buffer << "on " << HSHR (ch) << " right ankle";
	else if (obj->location == WEAR_BELT_1 || obj->location == WEAR_BELT_2)
		buffer << "on " << HSHR (ch) << " belt";
	else if (obj->location == WEAR_BACK)
		buffer << "on " << HSHR (ch) << " back";
	else if (obj->location == WEAR_THROAT)
		buffer << "around " << HSHR (ch) << " throat";
	else if (obj->location == WEAR_BLINDFOLD)
		buffer << "as a blindfold";
	else if (obj->location == WEAR_EAR)
		buffer << "on " << HSHR (ch) << " ear";
	else if (obj->location == WEAR_SHOULDER_R)
		buffer << "over " << HSHR (ch) << " right shoulder";
	else if (obj->location == WEAR_SHOULDER_L)
		buffer << "over " << HSHR (ch) << " left shoulder";
	else if (obj->location == WEAR_FEET)
		buffer << "on " << HSHR (ch) << " feet";
	else if (obj->location == WEAR_FINGER_R)
		buffer << "on " << HSHR (ch) << " right ring finger";
	else if (obj->location == WEAR_FINGER_L)
		buffer << "on " << HSHR (ch) << " left ring finger";
	else if (obj->location == WEAR_ARMBAND_R)
		buffer << "around " << HSHR (ch) << " upper right arm.";
	else if (obj->location == WEAR_ARMBAND_L)
		buffer << "around " << HSHR (ch) << " upper left arm.";
	else if (obj->location == WEAR_LEGS)
		buffer << "on " << HSHR (ch) << " legs";
	else if (obj->location == WEAR_HANDS)
		buffer << "on " << HSHR (ch) << " hands";
	else
		buffer << "#1in an uknown location#0";
	
	return ((char*)buffer.str().c_str());
}

	//mode = 0 -- long description, with annotations
	//mode = 1 -- from look command
	//mode = 15 -- from examine command
	//target -- character who is being looked 'at'
	//ch -- character who is doing the looking/examining
void show_char_to_char (CHAR_DATA * target, CHAR_DATA * ch, int mode, int llevel)
{
	OBJ_DATA *blindfold = NULL;
	OBJ_DATA *tobj = NULL;
	OBJ_DATA *eq = NULL;
	AFFECTED_TYPE *af = NULL;
	char *p = '\0';
	char buf[MAX_STRING_LENGTH] = { '\0' };
	int percent;
	int jdex;
	int crafts_found;
	int current_damage;
	int location;
	SUBCRAFT_HEAD_DATA *tcraft;
	ROOM_DATA *troom;
	std::stringstream buffer;
	std::stringstream buffer2;
	std::stringstream health;
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	
	if (!target || !ch || !mode)
		return;
	
	if (!mode)
	{
		
		if (target->is_subduee())
		{
			if (target->subdue == ch)
				ch->act("You have #5$N#0 in tow.", false, 0, target, TO_CHAR);
			return;
		}
		
		if (!can_see_mob(ch, target))
		{
			if (!GET_FLAG (target, FLAG_WIZINVIS) &&
				get_affect (ch, MAGIC_AFFECT_SENSE_LIFE))
				ch->send_to_char("\nYou sense a hidden life form in the room.\n\n");
			return;
		}
		
		if ((blindfold = get_equip (target, WEAR_BLINDFOLD)) 
			|| target->get_position() != target->default_pos)
		{
			if ( target->char_short())
			{
					//some light so we can see them
				if (llevel > 1)
				{
					buffer.sync();
					buffer << "#5";
					buffer <<  target->char_short();
					buffer <<  "#0";
					sprintf(buf, "%s", buffer.str().c_str());
					buf[2] = toupper (buf[2]);
					buffer.sync();
					buffer << buf;
				}
				else 
				{
					buffer.sync();
					buffer << "#5";
					buffer <<  "Someone";
					buffer <<  "#0";
					sprintf(buf, "%s", buffer.str().c_str());
					buf[2] = toupper (buf[2]);
					buffer.sync();
					buffer << buf;
				}
				
			}
			else
			{
				buffer.sync();
				buffer << "#5A nameless one#0";
				
			}
			
			switch (target->get_position())
			{
				
				case UNCON:
					if (blindfold)
						buffer << " is lying here, unconscious and blindfolded.";
					else
						" is lying here, unconscious.";
					break;
				case MORT:
					if (blindfold)
						buffer << " is lying here, mortally wounded and blindfolded.";
					else
						buffer << " is lying here, mortally wounded.";
					break;
				case DEAD:
					buffer << " is lying here, dead.";
					break;
				case STAND:
					if (blindfold)
						buffer << " is standing here, blindfolded.";
					else
						buffer << " is standing here.";
					break;
				case SIT:
					if (blindfold)
						buffer << " is sitting here, blindfolded";
					if (target->pmote_str)
					{
						buffer.sync();
						buffer << "#5";
						buffer <<  target->char_long();
						buffer <<  "#0";
						break;
					}
					else
						buffer << " is sitting here";
					
					if ((af = get_affect (target, MAGIC_SIT_TABLE)) &&
						is_obj_in_list (af->a.table.obj, target->room->contents))
					{
						tobj = af->a.table.obj;
						buffer << " at #2" << OBJS (tobj, target) << "#0.";
					}
					else
						buffer <<  ".";
					break;
				case REST:
					if (blindfold)
						buffer << " is resting here, blindfolded";
					else
					{
						if (target->pmote_str)
						{
							buffer.sync();
							buffer << "#5";
							buffer <<  target->char_long();
							buffer <<  "#0";
							break;
						}
						else
							buffer <<  " is resting here";
					}
					if ((af = get_affect (target, MAGIC_SIT_TABLE)) &&
						is_obj_in_list (af->a.table.obj, target->room->contents))
					{
						tobj = af->a.table.obj;
						buffer << " at #2" << OBJS (tobj, target) << "#0.";
					}
					else
						buffer << ".";
					break;
				case SLEEP:
					if (blindfold)
						buffer << " is sleeping here, blindfolded";
					else
						buffer <<  " is sleeping here";
					
					if ((af = get_affect (target, MAGIC_SIT_TABLE)) &&
						is_obj_in_list ((OBJ_DATA *) af->a.spell.t,
										target->room->contents))
					{
						tobj = (OBJ_DATA *) af->a.spell.t;
						buffer << " at #2" << OBJS (tobj, target) << "#0.";
					}
					else
						buffer <<  ".";
					
					break;
				default:
					buffer <<  " is floating here.";
					break;
			}
			
			if (ch->get_trust() > 1)
			{
				buffer <<  " #3(%s)#0", target->name;
				buffer << health.str().c_str();
			}
			
			if (!IS_NPC (target) && IS_SET (target->plr_flags, NEW_PLAYER_TAG))
				buffer <<  " #2(new player)#0";
			
			if (!IS_NPC (target) && IS_GUIDE(target) && IS_SET (target->flags, FLAG_GUEST))
				buffer <<  " #B(new player guide)#0";
			
			if (target->desc && target->desc->idle)
				buffer <<  " #1(idle)#0";
			
			if (((target->get_trust() > 1) || get_affect (ch, MAGIC_AFFECT_SENSE_LIFE))
				&& get_affect (target, MAGIC_HIDDEN))
				buffer <<  " #1(hidden)#0";
			
			if (are_grouped (target, ch))
				buffer << " #6(grouped)#0";
			
			if (GET_FLAG (target, FLAG_WIZINVIS))
				buffer <<  " #C(wizinvis)#0";
			
			if (get_affect (target, MAGIC_AFFECT_INVISIBILITY))
				buffer <<  " #1(invisible)#0";
			
			if (get_affect (target, MAGIC_AFFECT_CONCEALMENT)
				&& (target == ch 
					|| (target->get_trust() > 1) 
					|| get_affect (ch, MAGIC_AFFECT_SENSE_LIFE)))
				buffer <<  " #1(blend)#0";
			
			if (IS_SET (target->action, PLR_QUIET) 
				&& !IS_NPC (target))
				buffer <<  " #1(editing)#0";
			
			if (!IS_NPC (target) 
				&& !target->desc 
				&& !target->pc->admin_loaded)
				buffer <<  " #1(link dead)#0";
			
			if (ch->get_trust()
				&& target->pc 
				&& !target->desc 
				&& target->pc->admin_loaded)
				buffer <<  " #3(loaded)#0";
			
			if (target->desc 
				&& target->desc->original 
				&& ch->get_trust())
				buffer <<  " #2(animated)#0";
			
			
			if (!IS_NPC (target) 
				&& !target->desc 
				&& !target->pc->admin_loaded)
				buffer <<  " (%s)", target->name;
			
			buffer <<  "#0\n";
			reformat_string ((char*)buffer.str().c_str(), &p);
			ch->send_to_char(p);
			free_mem (p); // char*
		}
		
		else
		{	
			if (llevel == 2)
			{
				buffer.sync();
				buffer << "#5" << target->char_long() << "#0";
				sprintf(buf, "%s", buffer.str().c_str());
				buf[2] = toupper (buf[2]);
				buffer.sync();
				buffer << buf;
			}
			else if (llevel == 1)
			{
				buffer.sync();
				buffer << "#5" << target->char_short() << "#0";
				sprintf(buf, "%s", buffer.str().c_str());
				buf[2] = toupper (buf[2]);
				buffer.sync();
				buffer << buf;
			}
			else 
			{
				buffer.sync();
				buffer << "#5Someone#0";
			}
			
			
			if (target->is_subduer())
			{
				if (llevel == 2)
				{
						buffer <<  " has #5";
						buffer << target->subdue == ch ? "you" : target->subdue->char_short ();
						buffer << "#0 in tow.\n";
					
				}
				else
				{
					
						buffer << " has #5";
						buffer << target->subdue == ch ? "you" : "someone";
						buffer << "#0 in tow.\n";
						
					
				}
				
				sprintf(buf, "%s", buffer.str().c_str());
				buf[2] = toupper (buf[2]);
				buffer.sync();
				buffer << buf;
				ch->send_to_char(buffer.str().c_str());
				
			}
			
						
			else
			{
				if ((GET_FLAG (target, FLAG_ENTERING) ||
					 GET_FLAG (target, FLAG_LEAVING)))
				{
					if (ch->get_trust() > 1)
					{
						buffer <<  " #3(" << target->name << ")#0";
					}
					
					
					if (!IS_NPC (target) && IS_SET (target->plr_flags, NEW_PLAYER_TAG))
						buffer << " #2(new player)#0";
					
					if (!IS_NPC (target) && IS_GUIDE(target) && IS_SET (target->flags, FLAG_GUEST))
						buffer << " #B(new player guide)#0";
					
					if (target->desc && !IS_NPC (target) && target->desc->idle)
						buffer << "#1(idle)#0 ";
					
					if (GET_FLAG (target, FLAG_WIZINVIS))
						buffer << " #C(wizinvis)#0";
					
					if (get_affect (target, MAGIC_HIDDEN))
						buffer << " #1(hidden)#0";
					
					if (are_grouped (target, ch))
						buffer << " #6(grouped)#0";
					
					if (get_affect (target, MAGIC_AFFECT_INVISIBILITY))
						buffer << " #1(invisible)#0";
					
					if (get_affect (target, MAGIC_AFFECT_CONCEALMENT))
						buffer << " #1(blend)#0";
					
					if (IS_SET (target->action, PLR_QUIET) 
						&& !IS_NPC (target))
						buffer << " #1(editing)#0";
					
					if (target->desc && target->desc->original 
						&& ch->get_trust())
						buffer << " #2(animated)#0";
					
					if (!IS_NPC (target) 
						&& !target->desc 
						&& !target->pc->admin_loaded)
						buffer << " #1(link dead)#0";
					
					if (ch->get_trust() 
						&& !IS_NPC (target) 
						&& !target->desc
						&& target->pc->admin_loaded)
						buffer << " #3(loaded)#0";
					
					buffer << "#0\n";
					reformat_string ((char*)buffer.str().c_str(), &p);
					ch->send_to_char(p);
					free_mem (p); // char*
				}
				
				else if ((af = get_affect (target, AFFECT_SHADOW)) &&
						 af->a.shadow.edge != -1)
				{
					sprintf (buf, "%s",  target->char_short());
					*buf = toupper (*buf);
					buffer.sync();
					buffer << "#5" << buf << " is here, #0standing " << extended_dirs[af->a.shadow.edge] << ".";
					
					if (ch->get_trust() > 1)
					{
						buffer <<  " #3(" << target->name << ")#0";
					}
					
					ch->act(buffer.str().c_str(), false, 0, target, TO_CHAR | _ACT_FORMAT);
				}
				
				
				else
				{
					if (llevel < 2)
						buffer << " is here.";
					
					if (ch->get_trust() > 1)
					{
						buffer <<  " #3(" << target->name << ")#0";
						
					}
					
					
					
					if (!IS_NPC (target) && IS_SET (target->plr_flags, NEW_PLAYER_TAG))
						buffer <<  " #2(new player)#0";
					
					if (!IS_NPC (target) && IS_GUIDE(target) && IS_SET (target->flags, FLAG_GUEST))
						buffer <<  " #B(new player guide)#0";
					
					if (get_affect (target, MAGIC_HIDDEN))
						buffer <<  " #1(hidden)#0";
					
					if (are_grouped (target, ch))
						buffer <<  " #6(grouped)#0";
					
					if (target->desc && target->desc->idle)
						buffer <<  " #1(idle)#0";
					
					if (GET_FLAG (target, FLAG_WIZINVIS))
						buffer <<  " #C(wizinvis)#0";
					
					if (get_affect (target, MAGIC_AFFECT_INVISIBILITY))
						buffer <<  " #1(invisible)#0";
					
					if (get_affect (target, MAGIC_AFFECT_CONCEALMENT))
						buffer <<  " #1(blend)#0";
					
					if (IS_SET (target->action, PLR_QUIET) && !IS_NPC (target))
						buffer <<  " #1(editing)#0";
					
					if (target->desc 
						&& target->desc->original 
						&& ch->get_trust())
						buffer <<  " #2(animated)#0";
					
					if (!IS_NPC (target) 
						&& !target->desc 
						&& !target->pc->admin_loaded)
						buffer <<  " #1(link dead)#0";
					
					if (ch->get_trust() 
						&& !IS_NPC (target) 
						&& !target->desc
						&& target->pc->admin_loaded)
						buffer <<  " #3(loaded)#0";
					
					buffer <<  "#0\n";
					reformat_string ((char*)buffer.str().c_str(), &p);
					ch->send_to_char(p);
					free_mem (p); //char*
				}
			}
		}
		
	}
	else if (mode == 1 || mode == 15)
	{
		
		if (target->description)
		{
			
			if (((eq = get_equip (target, WEAR_HEAD))
				 && IS_SET (eq->obj_flags.extra_flags, ITEM_MASK))
				|| ((eq = get_equip (target, WEAR_FACE))
					&& IS_SET (eq->obj_flags.extra_flags, ITEM_MASK))
				|| ((eq = get_equip (target, WEAR_NECK_1))
					&& IS_SET (eq->obj_flags.extra_flags, ITEM_MASK)
					&& IS_SET (target->affected_by, AFF_HOODED))
				|| ((eq = get_equip (target, WEAR_NECK_2))
					&& IS_SET (eq->obj_flags.extra_flags, ITEM_MASK)
					&& IS_SET (target->affected_by, AFF_HOODED))
				|| ((eq = get_equip (target, WEAR_ABOUT))
					&& IS_SET (eq->obj_flags.extra_flags, ITEM_MASK)
					&& IS_SET (target->affected_by, AFF_HOODED)))
			{
				ch->send_to_char("This person's features are not visible.\n");
			}
			else
			{
				ch->send_to_char(target->description);
			}
		}
		else
		{
			target->act("You see nothing special about $m.", false, 0, ch, TO_VICT);
		}
		
		/* Show name (first keyword) if mobile is owned by the character examining the mobile*/
		if (mode == 15)
		{
			if(IS_NPC(target))
			{
				if(target->mob->owner)
				{
					if(!strcmp(target->mob->owner, ch->name))
					{
						buffer.sync();
						buffer << "\nYou recognise $n to be called "
						"%s.", target->name;
						target->act(buffer.str().c_str(), false,  0, ch, TO_VICT);
					}
				}
			}
		}
		
		/* show dmote */
		/* Description is formated to 65 char, but that will change so we use _ACT_FORMAT for consistancy */
		if (target->dmote_str)
		{
			target->act(buffer.str().c_str(), false,  0, ch, TO_VICT);
		}
		
			//show craft useage
		
		
		/* Show a character to another */
		
		if (target->max_hit > 0)
			percent = (100 * target->hit) / (target->max_hit);
		else
			percent = -1;	
		
		buffer.sync();
		buffer <<   target->char_short();
		sprintf(buf, "%s", buffer.str().c_str());
		buf[2] = toupper (buf[2]);
		buffer.sync();
		buffer << buf;
		
		if (IS_NPC(target) &&  IS_SET (target->mob->profession, PROF_VEHICLE))
		{
			if (percent >= 100)
				buffer << " is in pristine condition.\n";
			else if (percent >= 90)
				buffer << " is slightly damaged.\n";
			else if (percent >= 70)
				buffer << " is damaged, but still functional.\n";
			else if (percent >= 50)
				buffer << " is badly damaged.\n";
			else if (percent >= 25)
				buffer << " is barely functional.\n";
			else if (percent >= 10)
				buffer << " has sustained a great deal of damage!\n";
			else if (percent >= 0)
				buffer << " is about ready to collapse inward!\n";
			ch->send_to_char("\n");
			ch->send_to_char(buffer.str().c_str());
		}
		
		
		if (mode == 15 && target != ch && !target->is_hooded())
		{
			display_clan_ranks (target, ch);
		}
		
		
		if ((mode == 15) && (IS_NPC(target)))
		{
			buffer2.sync();
				//all crafts
			for (jdex = CRAFT_FIRST; jdex <= CRAFT_LAST; jdex++)
			{
				if (!(af = get_affect (ch, jdex)))
					continue;
				if (!af->a.craft || !af->a.craft->subcraft)
					continue;
				if (!craft_mob_uses (af->a.craft->subcraft, target->mob->nVirtual))
					continue;
				if (crafts_found)
					buffer2 << ", ";
				
				buffer2 << "'" << af->a.craft->subcraft->command << " " << af->a.craft->subcraft->subcraft_name << "'";
				
				crafts_found += 1;
			}
			
			if (ch->get_trust())
			{
					//just crafts a mortal actually has
				for (tcraft_iterator = craft_map.begin(); tcraft_iterator != craft_map.end(); tcraft_iterator++)
				{
					tcraft = tcraft_iterator->second;
					if (!craft_mob_uses (tcraft, target->mob->nVirtual))
						continue;
					if (crafts_found)
						buffer2 << ", ";
					
					buffer2 << "'" << af->a.craft->subcraft->command << " " << af->a.craft->subcraft->subcraft_name << "'";
					crafts_found += 1;
				}
			}
			
			
			if (crafts_found)
			{
				buffer << "   You realize that you could make use of this animal in the following craft";
				buffer << (crafts_found >= 1 ? "s" : "") << ": " << buffer2 << ".";
				reformat_string ((char*)buffer.str().c_str(), &p);
				ch->send_to_char(p);
				free_mem (p); //char*
				p = NULL;
			}
			
			
		}
		
				
				
		if (target == ch)
		{
			ch->send_to_char("\n");
			do_equipment (target, "", 0);
		}
		else
		{
			if (target->equip || target->right_hand || target->left_hand)
				ch->send_to_char("\n");
			
			if (target->right_hand)
			{
				if (target->right_hand->location == WEAR_PRIM
					|| target->right_hand->location == WEAR_SEC)
					sprintf (buf, "<held in right hand>  ");
				else if (target->right_hand->location == WEAR_BOTH)
					sprintf (buf, "<held in both hands>  ");
				else
					sprintf (buf, "<carried in right hand>  ");
				
				ch->send_to_char(buf);
				show_obj_to_char (target->right_hand, ch, 1, 2);
			}
			if (target->left_hand)
			{
				if (target->left_hand->location == WEAR_PRIM
					|| target->left_hand->location == WEAR_SEC)
					sprintf (buf, "<held in left hand>   ");
				else if (target->left_hand->location == WEAR_BOTH)
					sprintf (buf, "<held in both hands>  ");
				else
					sprintf (buf, "<carried in left hand>   ");
				
				ch->send_to_char(buf);
				show_obj_to_char (target->left_hand, ch, 1, 2);
			}
			
			if (target->equip && (target->left_hand || target->right_hand))
				ch->send_to_char("\n");
			
				//location == 0 is for "take/light", not a real wear_loc
			for (location = 1; location < MAX_WEAR; location++)
			{
				
				if (!(eq = get_equip (target, location)))
					continue;
				
				if (eq == target->right_hand || eq == target->left_hand)
					continue;
				
				ch->send_to_char(where[location]);
				
				if (location == WEAR_BLINDFOLD || IS_OBJ_VIS (ch, eq))
					show_obj_to_char (eq, ch, 1, 2);
				else
					ch->send_to_char("#2something#0\n");
				
			}
		}
	}
	
	
}

void do_search (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir = 0;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	if (ch->skill_map["Search"] < 1)
	{
		ch->send_to_char("You'll need to learn some search skills first.\n");
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf)
		dir = -1;
	else
	{
		dir = index_lookup (dirs, buf);
	}

	ch->act("$n carefully searches the area.", true, 0, 0, TO_ROOM | _ACT_SEARCH);
	ch->act("You search carefully...", false, 0, 0, TO_CHAR);

	ch->delay_type = DEL_SEARCH;
	ch->delay = 3 + number (0, 2);
	ch->delay_info1 = dir;
}



//////////////////////////////////////////////////////////////////////////////
// room_get_description ()
//////////////////////////////////////////////////////////////////////////////
//
/// \brief  Obtain the current room description
//
/// \param[in]  room  The room to examine (i.e. "this->get_description").
/// \return     The current room description
//
/// By default, return the room's description. If there are extra descriptions
/// defined, then pick the closest case and return that instead. If you have
/// an extra description for the current weather & time scenario, return that.
/// If you don't have a match for the current weather, but you do have a
/// description for night, and it is night time, return that description.
/// Otherwise fall back on the default room description.
//
//////////////////////////////////////////////////////////////////////////////
char * room_get_description (ROOM_DATA * room)
{
	const char * description;
	ROOM_EXTRA_DATA * room_extra;
	int weather_in_room;
	
	
	room_extra = room->extra;
	weather_in_room = desc_weather[room->wzone];
	description = room->description;
	
	//normal night descriptions

		if ((global_sun_light <= SUN_TWILIGHT) 
			&& (room_extra)
			&& (room_extra->weather_desc[WR_NIGHT].compare("")))
		{
			description = room_extra->weather_desc[WR_NIGHT].c_str();
		}		
	
	
		//do we have any other differences from normal to make?
	if (room_extra
			 && !(room_extra->weather_desc[weather_in_room].empty())
			 && (room_extra->weather_desc[weather_in_room].compare("")))
	{
		if ((global_sun_light <= SUN_TWILIGHT))
		{
					//normal weather, check seasons
			if ((time_info.season == SPRING)
				&& (room_extra->weather_desc[WR_NIGHT_SPRING].compare(""))) 
			{
				description = room_extra->weather_desc[WR_NIGHT_SPRING].c_str();
			}
			else if ((time_info.season == SUMMER)
					 && (room_extra->weather_desc[WR_NIGHT_SUMMER].compare(""))) 
			{
				description = room_extra->weather_desc[WR_NIGHT_SUMMER].c_str();
			}
			else if ((time_info.season == AUTUMN)
					 && (room_extra->weather_desc[WR_NIGHT_AUTUMN].compare(""))) 
			{
				description = room_extra->weather_desc[WR_NIGHT_AUTUMN].c_str();
			}
			else if ((time_info.season == WINTER)
					 && (room_extra->weather_desc[WR_NIGHT_WINTER].compare(""))) 
			{
				description = room_extra->weather_desc[WR_NIGHT_WINTER].c_str();
			}
			
				//nothing special, it is normal night
			if (room_extra->weather_desc[WR_NIGHT].compare("")) 
			{
				description = room_extra->weather_desc[WR_NIGHT].c_str();
			}
			
				//weather related
			if ((weather_info[room->wzone].fog > NO_FOG)
				&& (room_extra->weather_desc[WR_NIGHT_FOGGY].compare(""))) 
			{
				description = room_extra->weather_desc[WR_NIGHT_FOGGY].c_str();
			}
			
			if ((weather_info[room->wzone].clouds > CLEAR_SKY)
				&& (room_extra->weather_desc[WR_NIGHT_CLOUDY].compare(""))) 
			{
				description = room_extra->weather_desc[WR_NIGHT_CLOUDY].c_str();
			}
			
				//raining
			if ((weather_info[room->wzone].state > CHANCE_RAIN)
				&& (weather_info[room->wzone].state < LIGHT_SNOW)
				&& (room_extra->weather_desc[WR_NIGHT_RAINY].compare(""))) 
			{
				description = room_extra->weather_desc[WR_NIGHT_RAINY].c_str();
			}
			
			if ((weather_info[room->wzone].state >= STEADY_RAIN)
				&& (weather_info[room->wzone].state < LIGHT_SNOW)
				&& (weather_info[room->wzone].wind_speed > GALE)
				&& (room_extra->weather_desc[WR_NIGHT_STORMY].compare(""))) 
			{
				description = room_extra->weather_desc[WR_NIGHT_STORMY].c_str();
			}
			
				//snowing
			if ((weather_info[room->wzone].state >= LIGHT_SNOW)
				&& (room_extra->weather_desc[WR_NIGHT_SNOWY].compare(""))) 
			{
				description = room_extra->weather_desc[WR_NIGHT_SNOWY].c_str();
			}
			
			if ((weather_info[room->wzone].state >= STEADY_SNOW)
				&& (weather_info[room->wzone].wind_speed > GALE)
				&& (room_extra->weather_desc[WR_NIGHT_BLIZARD].compare(""))) 
			{
				description = room_extra->weather_desc[WR_NIGHT_BLIZARD].c_str();
			}
			
					
		}
		else
		{
				//normal daytime weather, check seasons
			if ((time_info.season == SPRING)
				&& (room_extra->weather_desc[WR_SPRING].compare(""))) 
			{
				description = room_extra->weather_desc[WR_SPRING].c_str();
			}
			else if ((time_info.season == SUMMER)
					 && (room_extra->weather_desc[WR_SUMMER].compare(""))) 
			{
				description = room_extra->weather_desc[WR_SUMMER].c_str();
			}
			else if ((time_info.season == AUTUMN)
					 && (room_extra->weather_desc[WR_AUTUMN].compare(""))) 
			{
				description = room_extra->weather_desc[WR_AUTUMN].c_str();
			}
			else if ((time_info.season == WINTER)
					 && (room_extra->weather_desc[WR_WINTER].compare(""))) 
			{
				description = room_extra->weather_desc[WR_WINTER].c_str();
			}
			
				//WEATHER RELATED
			if ((weather_info[room->wzone].fog > NO_FOG)
				&& (room_extra->weather_desc[WR_FOGGY].compare(""))) 
			{
				description = room_extra->weather_desc[WR_FOGGY].c_str();
			}
			
			if ((weather_info[room->wzone].clouds > CLEAR_SKY)
				&& (room_extra->weather_desc[WR_CLOUDY].compare(""))) 
			{
				description = room_extra->weather_desc[WR_CLOUDY].c_str();
			}
			
				//raining
			if ((weather_info[room->wzone].state > CHANCE_RAIN)
				&& (weather_info[room->wzone].state < LIGHT_SNOW)
				&& (room_extra->weather_desc[WR_RAINY].compare(""))) 
			{
				description = room_extra->weather_desc[WR_RAINY].c_str();
			}
			else if ((weather_info[room->wzone].state >= STEADY_RAIN)
				&& (weather_info[room->wzone].state < LIGHT_SNOW)
				&& (weather_info[room->wzone].wind_speed > GALE)
				&& (room_extra->weather_desc[WR_STORMY].compare(""))) 
			{
				description = room_extra->weather_desc[WR_STORMY].c_str();
			}	
			
				//snowing
			if ((weather_info[room->wzone].state >= LIGHT_SNOW)
				&& (room_extra->weather_desc[WR_SNOWY].compare(""))) 
			{
				description = room_extra->weather_desc[WR_SNOWY].c_str();
			}
			else if ((weather_info[room->wzone].state >= STEADY_SNOW)
				&& (weather_info[room->wzone].wind_speed > GALE)
				&& (room_extra->weather_desc[WR_BLIZARD].compare(""))) 
			{
				description = room_extra->weather_desc[WR_BLIZARD].c_str();
			}		
			
		}

	}
	
	return (char*)description;
}

	//cmd = 0 - normal use
	//cmd = 1 - first look at room - weather and time info
	//cmd = 2 - examine an object
void do_look (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir = 0;
	char *ptr = '\0';
	OBJ_DATA *obj = NULL;
	CHAR_DATA *tch = NULL;
	char arg1[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };
		
	
	if (ch->get_position() < SLEEP)
	{
		ch->send_to_char("You are unconscious!\n");
		return;
	}
	
	if (ch->get_position() == SLEEP)
	{
		ch->send_to_char("You are asleep.\n");
		return;
	}
	
	if (ch->is_blind())
	{
		ch->send_to_char("You are blind!\n");
		return;
	}
	
	if (!ch->room)
		ch->room = vtor (ch->in_room);
	
	if ((ch->room->light < NO_MOON)
		&& (!ch->get_trust())
		&& !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
		&& !IS_SET (ch->affected_by, AFF_INFRAVIS))
	{
		ch->send_to_char("It is too dark to see any details.\n");
		return;
	}
	
	
	argument = one_argument (argument, arg1);
		
	
		// Special GL Window 
	if ((strcasecmp (arg1, "window") == STR_MATCH && ch->in_room == OOC_LOUNGE)
		|| (!strn_cmp (arg1, "window", 6)
			&& strcasecmp (ch->room->name, PREGAME_ROOM_NAME) == STR_MATCH))
	{
		look_gl_window(ch);
		return;
	}
	
		// LOOK IN A CERTAIN DIRECTION 
	
	if (*arg1 && strcasecmp (arg1, "in") != STR_MATCH
		&& (dir = index_lookup (dirs, arg1)) != -1)
	{
		look_direction(ch, arg1);
		return;
	}	
			
		
		// LOOK INSIDE ANOTHER OBJECT 
	
	if (strcasecmp (arg1, "in") == STR_MATCH)
	{
		look_in_container (ch, argument);
		return;
	}	
			
		// LOOK AT SOMETHING 
	else if (*arg1)
	{
		
		if (strcasecmp (arg1, "at") == STR_MATCH)
			argument = one_argument (argument, arg1);
		
		if (!*arg1)
		{
			ch->send_to_char("Look at what?\n");
			return;
		}
		
			//portals
		look_portal_key(ch, arg1);
		
			// at all stuff worn by mob
		if ((tch = get_char_room_vis (ch, arg1)))
		{
			
			argument = one_argument (argument, arg1);
			
			if (*arg1
				&& ((obj = get_obj_in_list_vis (tch, arg1, tch->right_hand))
					|| (obj = get_obj_in_list_vis (tch, arg1, tch->left_hand))))
			{
				ch->act("$N is carrying $p.", false, obj, tch, TO_CHAR);
				ch->send_to_char("\n");
				if (cmd == 2)
					show_obj_to_char (obj, ch, 15, 2);
				else
					show_obj_to_char (obj, ch, 5, 2);
				
				return;
			}
			
			if (*arg1 && (obj = get_obj_in_list_vis (tch, arg1, tch->equip)))
			{
				ch->act("$N is wearing $p.", false, obj, tch, TO_CHAR);
				ch->send_to_char("\n");
				if (cmd == 2)
					show_obj_to_char (obj, ch, 15, 2);
				else
					show_obj_to_char (obj, ch, 5, 2);
				
				return;
			}
			
			if (*arg1)
			{
				ch->act("$N doesn't have that.", false, 0, tch, TO_CHAR);
				return;
			}
			
			if (cmd == 2)
				show_char_to_char (tch, ch, 15, 2);
			else
				show_char_to_char (tch, ch, 1, can_see_mob(ch, tch));
			
			return;
		}
		
			//at a specific item carried or worn by mob
		if (!(obj = get_obj_in_dark (ch, arg1, ch->equip)) &&
			!(obj = get_obj_in_dark (ch, arg1, ch->right_hand)) &&
			!(obj = get_obj_in_dark (ch, arg1, ch->left_hand)))
			obj = get_obj_in_list_vis (ch, arg1, ch->room->contents);
		
			// BOARDS 
		if (obj && obj->obj_flags.type_flag == ITEM_BOARD)
		{
			
			
			one_argument (obj->name, buf);
			retrieve_mysql_board_listing (ch, buf);
			return;
			
		}
		
			// Extra room description 
		if ((ptr = find_ex_description (arg1, ch->room->nVirtual))
			&& *ptr)
		{
			page_string (ch->desc, ptr);
			return;
		}
		
			
			// look at a single object in the room or in your hand */
		if (obj)
		{
			if (cmd == 2)
				show_obj_to_char (obj, ch, 15, 2);
			else
				show_obj_to_char (obj, ch, 5,2);
			
			return;
		}
		
			//look at collections of furnishings, objects or crowds
		if (*arg1)
		{
			if (!strn_cmp (arg1, "furnishings", strlen (arg1)) ||
				!strn_cmp (arg1, "furniture", strlen (arg1)) ||
				!strn_cmp (arg1, "tables", strlen (arg1)))
			{
				do_tables (ch, "", 0);
				return;
			}
			
					
			if (!strn_cmp (arg1, "objects", strlen (arg1)))
			{
				show_contents (ch, "", 1);
				return;
			}
			
			if (!strn_cmp (arg1, "crowd", strlen (arg1)) ||
				!strn_cmp (arg1, "individuals", strlen (arg1)))
			{
				show_contents (ch, "", 2);
				return;
			}
		}
		
		return;
	}
	
		//GENERAL look around the room
	else
	{	
		look_room(ch, cmd);
	
	}
}

int read_pc_message (CHAR_DATA * ch, char *name, char *argument)
{
	CHAR_DATA *who = NULL;
	MESSAGE_DATA *message = NULL;
	char mess_num_buf[MAX_STRING_LENGTH] = { '\0' };
	char disp_buf[MAX_STRING_LENGTH] = { '\0' };

	*name = toupper (*name);

	if (!(who = load_pc (name)))
		return 0;

	argument = one_argument (argument, mess_num_buf);

	if (!atoi (mess_num_buf))
	{
		ch->send_to_char("Which message?\n");
		who->unload_pc();
		return 1;
	}

	if (!(message = load_message (name, 7, atoi (mess_num_buf))))
	{
		ch->send_to_char("No such message.\n");
		who->unload_pc();
		return 1;
	}

	sprintf (disp_buf, "#6Date:#0    %s\n"
		"#6Author:#0  %s\n"
		"#6Subject:#0 %s\n\n%s", message->date, message->poster,
		message->subject, message->message);

	ch->send_to_char("\n");
	page_string (ch->desc, disp_buf);

	if (ch == who && !message->flags)
		mark_as_read (ch, atoi (mess_num_buf));

	free_mem(message);
	who->unload_pc();

	return 1;
}

int read_virtual_message (CHAR_DATA * ch, char *name, char *argument)
{
	MESSAGE_DATA *message = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char temp_buf[MAX_STRING_LENGTH] = { '\0' };


	*name = toupper (*name);

	argument = one_argument (argument, buf);

	if (!atoi (buf))
	{
		ch->send_to_char("Which message?\n");
		return 1;
	}

	if (!(message = load_message (name, 5, atoi (buf))))
	{
		ch->send_to_char("No such virtual board message.\n");
		return 1;
	}

	sprintf (temp_buf, "#6Date:#0     %s\n"
		"#6Author:#0   %s\n"
		"#6Subject:#0  %s\n"
		"#6Context:#0  http://www.middle-earth.us/index.php?display=staffportal&context=%ld&db=all##%ld\n\n%s",
		message->date, message->poster, message->subject,
		message->nTimestamp, message->nTimestamp, message->message);

	ch->send_to_char("\n");
	page_string (ch->desc, temp_buf);

	free_mem(message);

	return 1;
}

void do_read (CHAR_DATA * ch, char *argument, int cmd)
{
	int msg_num = 0;
	OBJ_DATA *board_obj = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	argument = one_argument (argument, buf);

	if (!isdigit (*buf))
	{
		if (!*buf)
		{
			ch->send_to_char("Which message?\n");
			return;
		}

		if (!
			((board_obj = get_obj_in_dark (ch, buf, ch->right_hand))
			&& (board_obj->obj_flags.type_flag == ITEM_BOOK
			|| board_obj->obj_flags.type_flag == ITEM_PARCHMENT
			|| board_obj->obj_flags.type_flag == ITEM_BOARD))
			&& !((board_obj = get_obj_in_dark (ch, buf, ch->left_hand))
			&& (board_obj->obj_flags.type_flag == ITEM_BOOK
			|| board_obj->obj_flags.type_flag == ITEM_PARCHMENT
			|| board_obj->obj_flags.type_flag == ITEM_BOARD))
			&& !((board_obj = get_obj_in_dark (ch, buf, ch->equip))
			&& (board_obj->obj_flags.type_flag == ITEM_BOOK
			|| board_obj->obj_flags.type_flag == ITEM_PARCHMENT
			|| board_obj->obj_flags.type_flag == ITEM_BOARD))
			&& !((board_obj = get_obj_in_list_vis (ch, buf, ch->room->contents))
			&& (board_obj->obj_flags.type_flag == ITEM_BOOK
			|| board_obj->obj_flags.type_flag == ITEM_PARCHMENT
			|| board_obj->obj_flags.type_flag == ITEM_BOARD)))
		{
			if ((!ch->get_trust()) || !read_pc_message (ch, buf, argument))
			{
				if ((!ch->get_trust()) || !read_virtual_message (ch, buf, argument))
				{
					ch->send_to_char("You can't see that board.\n");
				}
			}
			return;
		}

		if (board_obj->obj_flags.type_flag == ITEM_BOOK
			|| board_obj->obj_flags.type_flag == ITEM_PARCHMENT)
		{
			do_look (ch, board_obj->name, 0);
			return;
		}

		argument = one_argument (argument, buf);

		if (!isdigit (*buf))
		{
			ch->send_to_char("Which message on that board?\n");
			return;
		}
	}

	else
	{
		for (board_obj = ch->room->contents;
			board_obj; board_obj = board_obj->next_content)
		{

			if (!can_see_obj(ch, board_obj))
				continue;

			if (board_obj->obj_flags.type_flag == ITEM_BOARD)
				break;
		}

		if (!board_obj)
		{
			ch->send_to_char("You do not see a board here.\n");
			return;
		}
	}

	msg_num = atoi (buf);

	one_argument (board_obj->name, buf);

	if (board_obj->clan_data &&
		(!is_clan_member(ch, board_obj->clan_data->name)))
		
	{
		ch->send_to_char("You are not authorized to read these reports");
		return;
	}

	
	display_mysql_board_message (ch, buf, msg_num, 0);
}

void do_examine (CHAR_DATA * ch, char *argument, int cmd)
{
	do_look (ch, argument, 2);
}

	//Only display the diagonal exits when the exit command is used
void do_exits (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir = 0;
	ROOM_EXIT_DATA *exit = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char strExit[AVG_STRING_LENGTH] = { '\0' };
	char rsize[2] = { '\0' };
	ROOM_DATA *temp_room;
	
	if ((ch->room->light < NO_MOON)
		&& (!ch->get_trust())
		&& !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
		&& !IS_SET (ch->affected_by, AFF_INFRAVIS))
	{
		ch->send_to_char("You can't see a thing!\n");
		return;
	}


	for (dir = 0; dir <= LAST_DIR; dir++)
	{

		exit = is_exit(ch, dir);

		if (!exit || exit->to_room == NOWHERE)
			continue;

		if (!vtor (exit->to_room))
		{
			sprintf (buf, "Room %d leads to nonexistant room %d (do_exits)\n",
				ch->in_room, exit->to_room);
			continue;
		}

		if (vtor (exit->to_room)->room_size == ROOM_SIZE_DETAIL)
			strcpy(rsize, "S");
		else if (vtor (exit->to_room)->room_size == ROOM_SIZE_EXPLORE)
			strcpy(rsize, "M");
		else if (vtor (exit->to_room)->room_size == ROOM_SIZE_VALLEY)
			strcpy(rsize, "L");
		else if (vtor (exit->to_room)->room_size == ROOM_SIZE_STORAGE)
			strcpy(rsize, "T");
		else 
			strcpy(rsize, "-");

		
		strExit[0] = '\0';

		if (IS_SET (exit->port_flags, EX_ISDOOR)
			|| IS_SET (exit->port_flags, EX_ISGATE))
		{

			if (IS_SET (exit->port_flags, EX_CLOSED))
			{
				sprintf (strExit + strlen (strExit), "%s -  %s (Closed)",
						 exit_dirs[dir], exit->keyword);
			}
			else if (is_dark (vtor (exit->to_room))
					 && (!ch->get_trust())
					 && !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
					 && !IS_SET (ch->affected_by, AFF_INFRAVIS))
				sprintf (strExit + strlen (strExit),
				"%s - %s (Open) - Too dark to tell", exit_dirs[dir],
				exit->keyword);

			else
				sprintf (strExit + strlen (strExit),
				"%s - %s (Open) - %s[%s]",
				exit_dirs[dir], exit->keyword, vtor (exit->to_room)->name, rsize);
		}

		else if (exit->type > PORTAL_GATE) //ie, barriers, crossings, windows and transports
		{
			sprintf(strExit + strlen (strExit), "%s - %s",
					exit_dirs[dir],
					exit->ldesc);
		}
		else 
		{

			temp_room = vtor (exit->to_room);
			
			if (temp_room
				&& (temp_room->light < NO_MOON)
				&& (!ch->get_trust())
				&& !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
				&& !IS_SET (ch->affected_by, AFF_INFRAVIS))			
				
				sprintf (strExit + strlen (strExit), "%s - Too dark to tell",
				exit_dirs[dir]);

			else
				sprintf (strExit + strlen (strExit),
				"%s - %s[%s]", exit_dirs[dir], vtor (exit->to_room)->name, rsize);
		}

		if (ch->get_trust())
		{
			sprintf (strExit + strlen (strExit), " [%d]",
				vtor (exit->to_room)->nVirtual);
		}

		if (strExit && strExit[0] != '\0')
		{
			sprintf ( buf + strlen (buf), "%s\n", strExit);
		}
	}

	

	/** add a line if it is snowing hard **/
	if (weather_info[ch->room->wzone].state == HEAVY_SNOW
		&& (!get_affect (ch, MAGIC_AFFECT_INFRAVISION)
			&& !IS_SET (ch->affected_by, AFF_INFRAVIS))
		&& !IS_SET (ch->room->room_flags, INDOORS) && (!ch->get_trust()))
	{
		ch->send_to_char("The blowing snow makes it difficult to see anything!\n");
		
	}
	ch->send_to_char("Obvious exits:\n");

	if (*buf)
		ch->send_to_char(buf);
	else
		ch->send_to_char("None.\n");
}

int get_stat_range (int score)
{
	if (score <= 9)
		return 0;
	if (score >= 10 && score <= 12)
		return 1;
	if (score >= 13 && score <= 14)
		return 2;
	if (score >= 15 && score <= 16)
		return 3;
	if (score >= 17 && score < 18)
		return 4;
	if (score >= 18 && score < 19)
		return 5;
	if (score >= 19)
		return 6;
	
	return 0;
}

int get_comestible_range_food (int num)
{
	int break_value;
	
	break_value = MAX_CALORIE/4;
	
	if (num <= 0)
		return 0;
	if (num >= 1 && num <= break_value)
		return 1;
	if (num > (break_value) && (num <= (2*break_value)))
		return 2;
	if (num > (2*break_value) && (num <=(3*break_value)))
		return 3;
	if (num > (3*break_value)  && (num <=(4*break_value)))
		return 4;
	if (num > (4*break_value))
		return 5;
	return 0;
}

	//for fluids
int get_comestible_range (int num)
{
	int break_value;
	
	break_value = MAX_WATER/4;
	
	if (num <= 0)
		return 0;
	if (num >= 1 && num <= break_value)
		return 1;
	if (num > (break_value) && (num <= (2*break_value)))
		return 2;
	if (num > (2*break_value) && (num <=(3*break_value)))
		return 3;
	if (num > (3*break_value)  && (num <=(4*break_value)))
		return 4;
	if (num > (4*break_value))
		return 5;
	return 0;
}


char * suffix (int number)
{
	if (number == 1)
		return "st";
	else if (number == 2)
		return "nd";
	else if (number == 3)
		return "rd";
	else if (number < 20)
		return "th";
	else if ((number % 10) == 1)
		return "st";
	else if ((number % 10) == 2)
		return "nd";
	else if ((number % 10) == 3)
		return "rd";
	else
		return "th";
}

void do_score (CHAR_DATA * ch, char *argument, int cmd)
{
	int i = 0;
	int weight = 0;
	int time_diff = 0, days_remaining = 0, hours_remaining = 0;
	struct time_info_data playing_time;
	AFFECTED_TYPE *af = NULL;
	AFFECTED_TYPE *af_table = NULL;
	CHAR_DATA *rch = NULL;
	OBJ_DATA *vobj = NULL;
	char *p = '\0';
	char buf[MAX_STRING_LENGTH] = { '\0' };
	bool first = true;
	struct time_info_data birth_date;
	static char *verbal_stats[7] =
	{ "poor", "avg", "good", "great", "peak", "exceptional", "legendary"};
	

	birth_date = time_info;

	birth_date.minute -= age (ch).minute;
	if (birth_date.minute < 0)
	{
		birth_date.hour -= 1;
		birth_date.minute += 60;
	}
	birth_date.hour -= age (ch).hour;
	if (birth_date.hour < 0)
	{
		birth_date.day -= 1;
		birth_date.hour += 24;
	}
	birth_date.day -= age (ch).day;
	if (birth_date.day <= 0)
	{
		birth_date.month -= 1;
		birth_date.day += 30;
	}
	birth_date.month -= age (ch).month;
	if (birth_date.month < 0)
	{
		birth_date.year -= 1;
		birth_date.month += 12;
	}
	birth_date.year -= age(ch).year;

	ch->send_to_char("\n");
	if (!IS_SET (ch->flags, FLAG_GUEST))
		sprintf (buf, "#2%s, a %d year old %s %s:#0\n",
		ch->name, age(ch).year, lookup_race_variable (ch->race,
		RACE_NAME),
		sex_types[ch->sex]);
	else
		sprintf (buf, "#2Guest Login:#0 %s\n\n", ch->name);

	ch->send_to_char(buf);

	if (!IS_SET (ch->flags, FLAG_GUEST))
	{
		if ((!ch->get_trust()))
			sprintf (buf,
			"Str[#2%s#0] Dex[#2%s#0] Con[#2%s#0] Int[#2%s#0] Wil[#2%s#0] Agi[#2%s#0] Luk[#2%s#0]\n",
			verbal_stats[get_stat_range (ch->tmp_str)],
			verbal_stats[get_stat_range (ch->tmp_dex)],
			verbal_stats[get_stat_range (ch->tmp_con)],
			verbal_stats[get_stat_range (ch->tmp_intel)],
			verbal_stats[get_stat_range (ch->tmp_wil)],
			verbal_stats[get_stat_range (ch->tmp_agi)],
			verbal_stats[get_stat_range (ch->tmp_luk)]);
		else
			sprintf (buf,
			"Str[#2%d#0] Dex[#2%d#0] Con[#2%d#0] Int[#2%d#0] Wil[#2%d#0] Aura/Power[#2%d#0] Agi[#2%d#0] Luk[#2%d#0]\n",
			ch->tmp_str, ch->tmp_dex, ch->tmp_con, ch->tmp_intel,
			ch->tmp_wil, ch->tmp_aur, ch->tmp_agi, ch->tmp_luk);

		ch->send_to_char("\n");
		ch->send_to_char(buf);
	}

	// Segment to display power
	if (!IS_SET (ch->flags, FLAG_GUEST)) {
		int pcAur = ch->tmp_aur;

		if (pcAur < 4) {
			ch->send_to_char("Your soul #1flickers#0 like a candle, raw and untapped.");
		}
		
		else if (pcAur < 7) {
			ch->send_to_char("Your soul #1sparks#0 like embers flying free of a fire.");
		}
		else if (pcAur < 11) {
			ch->send_to_char("Your soul burns with #1flames#0.");
		}
		else if (pcAur < 16) {
			ch->send_to_char("Your soul burns with potent #9fire#0.");
		}
		else if (pcAur < 19) {
			ch->send_to_char("Power #9flares#0 within your soul.");
		}
		else if (pcAur < 32) {
			ch->send_to_char("#9Potency roars through your soul.#0");
		}
		else {
			ch->send_to_char("#9Your soul burns as consumingly as Anor.#0");
		}
		ch->send_to_char("\n");
	}// End of Power messages - Case

	if (IS_SET (ch->flags, FLAG_GUEST))
	{
		sprintf (buf, "You have been incarnated as #5%s#0.\n\n",
			 ch->char_short());
		ch->send_to_char(buf);
		sprintf (buf, "You are a #2%s#0 #2%s#0.\n", sex_types[ch->sex],
			lookup_race_variable (ch->race, RACE_NAME));
		ch->send_to_char(buf);
	}

	int acct_rpp = 0;
	if (!IS_NPC (ch) && !IS_SET (ch->flags, FLAG_GUEST) && ch->desc
		&& ch->desc->acct && (acct_rpp = ch->desc->acct->get_rpp ()) > 0
		&& IS_SET (ch->desc->acct->flags, ACCOUNT_RPPDISPLAY))
	{
		sprintf (buf, "Your account has been awarded #2%d#0 roleplay points.\n",
			acct_rpp);
		ch->send_to_char(buf);
	}

	/* Add support for listing, hunger, thirst, and intox. */
	sprintf (buf, "You are #2%s#0, and #2%s#0.\n",
		ch->hunger >=
		0 ? verbal_hunger[get_comestible_range_food(ch->hunger)] : "full",
		ch->thirst >=
		0 ? verbal_thirst[get_comestible_range (ch->thirst)] : "quenched");
	ch->send_to_char(buf);

	if (!IS_SET (ch->flags, FLAG_GUEST))
	{
		sprintf (buf, "You are #2%d#0 inches tall, and weigh #2%d#0 pounds.\n",
			ch->height, ch->get_weight() / 100);
		ch->send_to_char(buf);

		sprintf (buf,
			"You are of #2%s#0 build for one of your people, and wear size #2%s#0 (%s).\n",
			frames[ch->frame], sizes_named[ch->get_size()],
			sizes[ch->get_size()]);
		ch->send_to_char(buf);
	}


	ch->send_to_char(buf);

	if (GET_FLAG (ch, FLAG_AUTOFLEE))
		ch->send_to_char("If in combat, you will #2FLEE#0.\n");

	sprintf (buf, "You are currently speaking #2%s#0.\n", ch->speaks);
	ch->send_to_char(buf);

	if (ch->writes)
	{
		sprintf (buf, "You are currently set to write in #2%s#0.\n",
			ch->writes);
		ch->send_to_char(buf);
	}

	for (i = 0; i <= 3; i++)
		if (((ch->tmp_str) * enc_tab[i].str_mult_wt) >= ch->carrying())
			break;

	sprintf (buf, "You are currently #2%s#0.\n", enc_tab[i].encumbered_status);
	ch->send_to_char(buf);

	weight = ch->carrying();

	if (weight < 1000)
		weight = (weight + 50) / 100 * 100;
	else if (weight < 10000)
		weight = (weight + 500) / 1000 * 1000;
	else
		weight = (weight + 1250) / 2500 * 2500;

	if (ch->carrying() == 0)
		ch->send_to_char("You are carrying nothing.\n");
	else
	{
		sprintf (buf, "You are carrying about #2%d#0 pounds.\n", weight / 100);
		ch->send_to_char(buf);
	}

	if (!IS_SET (ch->flags, FLAG_GUEST))
	{
		if (birth_date.year > 0)
			sprintf (buf,
			"You were born on the #2%d%s#0 day of #2%s#0, #2%d#0.\n",
			birth_date.day, suffix (birth_date.day),
			month_name[birth_date.month], birth_date.year);
		else
			sprintf (buf,
			"You were born on the #2%d%s#0 day of #2%s#0, many millenia past.\n",
			birth_date.day, suffix (birth_date.day),
			month_name[birth_date.month]);
		ch->send_to_char(buf);
		playing_time =
			real_time_passed (time (0) - ch->time_str.logon + ch->time_str.played, 0);
		sprintf (buf,
			"You have been playing for #2%d#0 days and #2%d#0 hours.\n",
			playing_time.day, playing_time.hour);
		ch->send_to_char(buf);
	}

	
	switch (ch->get_position())
	{
	case DEAD:
		ch->send_to_char("You are DEAD!\n");
		break;
	case MORT:
		ch->send_to_char("You are mortally wounded, and should seek help!\n");
		break;
	case UNCON:
		ch->send_to_char("You are unconscious.\n");
		break;
	
	case SLEEP:
		af_table = get_affect (ch, MAGIC_SIT_TABLE);
		if (!af_table
			|| !is_obj_in_list ((OBJ_DATA *) af_table->a.spell.t,
			ch->room->contents))
			ch->send_to_char("You are sleeping.\n");
		else
		{
			sprintf (buf, "You are sleeping at #2%s#0.\n",
				obj_short_desc (af_table->a.table.obj));
			ch->send_to_char(buf);
		}
		break;
	case REST:
		af_table = get_affect (ch, MAGIC_SIT_TABLE);
		if (!af_table
			|| !is_obj_in_list ((OBJ_DATA *) af_table->a.spell.t,
			ch->room->contents))
			ch->send_to_char("You are resting.\n");
		else if (af_table && af_table->a.table.obj)
		{
			sprintf (buf, "You are resting at #2%s#0.\n",
				obj_short_desc (af_table->a.table.obj));
			ch->send_to_char(buf);
		}
		break;
	case SIT:
		af_table = get_affect (ch, MAGIC_SIT_TABLE);
		if (!af_table
			|| !is_obj_in_list ((OBJ_DATA *) af_table->a.spell.t,
			ch->room->contents))
			ch->send_to_char("You are sitting.\n");
		else
		{
			sprintf (buf, "You are sitting at #2%s#0.\n",
				obj_short_desc (af_table->a.table.obj));
			ch->send_to_char(buf);
		}
		break;
	case STAND:
		ch->send_to_char("You are standing.\n");
		break;
	default:
		ch->send_to_char("You are floating.\n");
		break;
	}

	if(ch->following)
	{
		if (first)
			ch->send_to_char("\n");
		ch->act("You are following $N.", false, 0, ch->following, TO_CHAR);
		first = false;
	}
	
	if (first == false)
		ch->send_to_char("\n");

	if (!IS_NPC (ch) && ch->pc->create_state == STATE_DIED)
		ch->send_to_char("You are DEAD.\n");
	
	if (!IS_NPC (ch) && ch->pc->create_state == STATE_RETIRED)
		ch->send_to_char("You are RETIRED, go play golf.\n");


	sprintf (buf, "You #2%s#0 when you travel.\n", verbal_speeds[ch->speed]);
	ch->send_to_char(buf);

	if (get_affect(ch,MAGIC_HIDDEN))
	{
		ch->send_to_char("You are currently #6hidden#0.\n");
	}

	if (IS_SET (ch->plr_flags, GROUP_CLOSED))
		ch->send_to_char("Your group is currently #2closed#0 to new members.\n");
	else
		ch->send_to_char("Your group is currently #2open#0 to new members.\n");

	if ((af = get_affect (ch, MAGIC_CRAFT_DELAY))
		&& time (0) < af->a.spell.modifier)
	{
		time_diff = af->a.spell.modifier - time (0);
		days_remaining = time_diff / (60 * 60 * 24);
		time_diff %= 60 * 60 * 24;
		hours_remaining = time_diff / (60 * 60);
		if (!days_remaining && !hours_remaining)
		{
			sprintf (buf,
				"Your craft timer has #2less than 1#0 RL hour remaining.\n");
		}
		else
		{
			sprintf (buf, "Your craft timer has approximately");
			if (days_remaining)
				sprintf (buf + strlen (buf), " #2%d#0 RL day%s ", days_remaining,
				days_remaining > 1 ? "s" : "");
			if (hours_remaining && days_remaining)
				sprintf (buf + strlen (buf), "and");
			if (hours_remaining)
				sprintf (buf + strlen (buf), " #2%d#0 RL hour%s ",
				hours_remaining, hours_remaining > 1 ? "s" : "");
			sprintf (buf + strlen (buf), "remaining.\n");
		}
		ch->send_to_char(buf);
	}

	if (ch->get_trust())
	{
		if (!ch->hour_affects)
			ch->send_to_char("No spells affect you.\n");
		else
		{
			for (af = ch->hour_affects; af; af = af->next)
			{


				if (af->type == AFFECT_SHADOW)
				{

					if (!af->a.shadow.shadow && af->a.shadow.edge != -1)
						sprintf (buf, "   Standing");

					else if (!is_he_somewhere (af->a.shadow.shadow))
						continue;

					else if (IS_NPC (af->a.shadow.shadow))
						sprintf (buf, "   Shadowing %s (%d)",
						af->a.shadow.shadow->short_descr,
						af->a.shadow.shadow->mob->nVirtual);
					else
						sprintf (buf, "   Shadowing PC %s",
						(af->a.shadow.shadow)->name);

					if (af->a.shadow.edge != -1)
						sprintf (buf + strlen (buf), " on %s edge.",
						dirs[af->a.shadow.edge]);

					strcat (buf, "\n");

					ch->send_to_char(buf);

					continue;
				}

				else if (af->type == MAGIC_SIT_TABLE)
				{
					sprintf (buf, "   Sitting at table affect.\n");
					ch->send_to_char(buf);
					continue;
				}

				if (af->type >= JOB_1 && af->type <= JOB_3)
				{

					i = (time_info.year * 12 * 30)
					+ (time_info.month * 30)
					+ time_info.day;
					
					i = i - af->a.job.pay_date;

					vobj = vtoo (af->a.job.object_vnum);

					sprintf (buf,
						"   Job %d:  %d days since last payday\n",
						af->type - JOB_1 + 1,
						i);

					sprintf (buf + strlen(buf),
						"             %d x %s",
						af->a.job.count,
						!vobj ? "UNDEFINED" : vobj->short_description
						);

					if (af->a.job.employer)
					{
						sprintf (buf + strlen(buf),
							"\n             Employer: %s \n",
							vtom(af->a.job.employer)->short_descr);
					}
					else
					{
						sprintf (buf+ strlen(buf),"\n");
					}

					ch->send_to_char(buf);
					continue;
				} //if (af->type >= JOB_1


				if (af->type >= MAGIC_CRIM_BASE &&
					af->type <= MAGIC_CRIM_BASE + 100)
				{
					sprintf (buf, "   Wanted in zone %d for %d hours.\n",
						af->type - MAGIC_CRIM_BASE, af->a.spell.duration);
					ch->send_to_char(buf);
					continue;
				}

				if (af->type >= MAGIC_CRIM_HOODED &&
					af->type < MAGIC_CRIM_HOODED + 100)
				{
					sprintf (buf, "   Hooded criminal charge in zone %d for "
						"%d RL seconds.\n",
						af->type - MAGIC_CRIM_HOODED,
						af->a.spell.duration);
					ch->send_to_char(buf);
					continue;
				}

				if (af->type == MAGIC_STARED)
				{
					sprintf (buf, "%d   Studied by an enforcer.  Won't be "
						"studied for %d RL seconds.\n",
						af->type, af->a.spell.duration);
					ch->send_to_char(buf);
					continue;
				}

				if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST)
					continue;

				if (af->type == MAGIC_CRAFT_DELAY || MAGIC_CRAFT_BRANCH_STOP)
					continue;

				if (af->type == MAGIC_HIDDEN) /// \todo Check this as dead code
				{
					sprintf (buf, "%d   Hidden.\n", af->type);
					ch->send_to_char(buf);
					continue;
				}

				if (af->type == MAGIC_SNEAK)
				{
					sprintf (buf, "%d   Currently trying to sneak.\n",
						af->type);
					ch->send_to_char(buf);
					continue;
				}
				if (af->type == SA_WARNED)
				{
					ch->send_to_char("You have been warned to surrender to an enforcer.\n");
					continue;
				}
				if (af->type == SA_ALREADY_WARNED)
				{
					ch->send_to_char("You will receive no warning from enforcers.\n");
					continue;
				}

				sprintf (buf, "   %s affects you by %d for %d seconds.\n",
					p == NULL ? "Unknown" : p,
					af->a.spell.modifier, af->a.spell.duration);
				ch->send_to_char(buf);
			}
		}
	}

	if (get_equip (ch, WEAR_BLINDFOLD))
		ch->send_to_char("YOU ARE BLINDFOLDED!\n");
	else if (ch->is_blind())
		ch->send_to_char("YOU ARE BLIND!\n");

	if (IS_SET (ch->affected_by, AFF_HOODED))
		ch->send_to_char("You are currently cloaked and hooded.\n");
	
	if (IS_SET (ch->affected_by, AFF_LEADER_COMMAND))
		ch->send_to_char("You are allowed to use special leadership commands.\n");


	if ((ch->voice_str)
		|| (ch->travel_str)
		|| (ch->pmote_str)
		|| (ch->dmote_str))
		ch->send_to_char("\n");
	
	if (ch->voice_str)
	{
		ch->send_to_char("\nYour current voice string: (#2");
		ch->send_to_char(ch->voice_str);
		ch->send_to_char("#0)\n");
	}

	if (ch->travel_str)
	{
		ch->send_to_char("Your current travel string: (#2");
		ch->send_to_char(ch->travel_str);
		ch->send_to_char("#0)\n");
	}

	if (ch->pmote_str)
	{
		ch->send_to_char("Your current pmote: (#2");
		ch->send_to_char(ch->pmote_str);
		ch->send_to_char("#0)\n");
	}

	if (ch->dmote_str)
	{
		ch->send_to_char("Your current dmote: ");
		ch->send_to_char(ch->dmote_str);
		ch->send_to_char("\n");
	}

	if (IS_SET (ch->plr_flags, NEWBCHAT)
		|| GET_FLAG (ch, FLAG_WIZNET))
		ch->send_to_char("\n");
		
	if (IS_SET (ch->plr_flags, NEWBCHAT))
	{
		ch->send_to_char("You are tuned into the newbie chat channel.");
		ch->send_to_char("\n");
		
	}
	
	if (GET_FLAG (ch, FLAG_WIZNET))
	{
		ch->send_to_char("You are tuned into the wiznet channel.");
		ch->send_to_char("\n");
		
	}
	/* after dmote, before clan dump, list subscribed petition zones for imms */
	if (ch->get_trust() > 1)
	{
		ch->send_to_char("You are subscribed to petitions from:#5");
		for (i=0; i<SPHERE_COUNT; i++)
		{
			if (IS_SET(ch->petition_flags,(1<<i)))
			{
				ch->send_to_char(" ");
				ch->send_to_char(spheres[i].name);
			}
		}
		ch->send_to_char("#0");
	}



	clan__do_score (ch);
}


void do_skills (CHAR_DATA * ch, char *argument, int cmd)
{
	int j = 2;
	int loaded_char = 0;
	int tskill;
	int skill_num;
	int counter;
	CHAR_DATA *who = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	std::string tskill_name;
	std::map<std::string, int> ::iterator skill_iterator;
	bool comp_flag = false;
	
	argument = one_argument (argument, buf);

		//if a mortal adds an argument, then he wants to see everything
	if ((!ch->get_trust()))
	{
		who = ch;
		if (!*buf)
			comp_flag = true;
	}
		//immortals will see all skills for the person named in the argument
	else 
	{
		if (!*buf)
			who = ch;
		else
			who = get_char_room_vis (ch, buf);
		
		if (!who)
		{
			who = load_pc (buf);
			loaded_char = 1;
		}
				
		if (!who)
		{
			ch->send_to_char("No body here by that name.\n");
			loaded_char = 0;
			return;
		}
		

		
	}

	*buf = '\0';

	if ((!ch->get_trust()))
	{
		j = 0;
		
		counter = who->skill_map.size();
		if (counter > 0)
		{
			for (skill_iterator = who->skill_map.begin(); skill_iterator != who->skill_map.end(); skill_iterator++)
			{
				if (skill_data_map.find(skill_iterator->first) != skill_data_map.end())
				{
					tskill = skill_iterator->second;
					counter --;
					
					if (tskill)
					{	
						tskill_name = skill_iterator->first.c_str();
						char* sk_lvl_name = strdup(SKIL_LEV(tskill));
						
							//if the level is exposed, and they want compact mode, it is not listed
							//it has spaces to keep the format lined up
						if ((str_cmp(sk_lvl_name, "Exposed  ") || !comp_flag))
							{
								sprintf (buf2, "#2%10.10s:#0 %s", tskill_name.c_str(), sk_lvl_name);
								
								while (strlen (buf2) < 14)
									strcat (buf2, " ");
								
								strcat (buf, buf2);
								j++;
								
								if (!(j % 3))
									strcat (buf, "\n");
							}
						
					}
					if (counter == 0)
						break;
				}
			}
		}
	}

	else
	{
		j = 0;
		counter = who->skill_map.size();
		if (counter > 0)
		{
			for (skill_iterator = who->skill_map.begin(); skill_iterator != who->skill_map.end(); skill_iterator++)
			{
				if (skill_data_map.find(skill_iterator->first) != skill_data_map.end())
				{
					tskill = skill_iterator->second;
					if (tskill)
					{	
						counter --;
						
						tskill_name = skill_iterator->first.c_str();
						skill_num = lookup_skill_id((char*)tskill_name.c_str());
						j++;
						sprintf (buf + strlen(buf), "#2%10.10s#0  [%03d/%03d]", tskill_name.c_str(), tskill,
								 calc_lookup (who, REG_CAP, skill_num));
						if (!(j % 3))
							strcat (buf, "\n");
					}
					if (counter == 0)
						break;
				}
			}
		}
		
	}

	ch->send_to_char(buf);

	if (buf[strlen (buf) - 1] != '\n')
		ch->send_to_char("\n");

	if (loaded_char)
		who->unload_pc();
}


char *time_string (CHAR_DATA * ch)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char suf[4] = { '\0' };
	int day = 0;
	int high_sun = 0;
	int nCharAstronomySkill = 0;
	char day_buf[AVG_STRING_LENGTH];
	char phrase[MAX_STRING_LENGTH];
	static char time_str[MAX_STRING_LENGTH] = { '\0' };
	
	const char *season_string[12] = {
		"deep winter",
		"late winter",
		"early spring",
		"mid-spring",
		"late spring",
		"early summer",
		"high summer",
		"late summer",
		"early autumn",
		"mid-autumn",
		"late autumn",
		"early winter"
	};

	
	high_sun = ((sunrise[time_info.month] + sunset[time_info.month]) / 2);

	sprintf (phrase, "[report error: %d]", time_info.hour);

	sprintf(phrase, "%s", time_phrase(high_sun));
	
	// Astronomy skill gives knowledge of more precise time
	if (ch 
		&& ch->skill_map["Astronomy"] > 0)
	{

		nCharAstronomySkill = ch->skill_map["Astronomy"];
		if (!IS_OUTSIDE (ch) || ((global_sun_light <= SUN_TWILIGHT) && !moon_light[ch->room->zone]))
		{
			nCharAstronomySkill -= 40;
		}
		else
		{
			nCharAstronomySkill -= (10 * weather_info[ch->room->wzone].clouds);
			nCharAstronomySkill -= (2 * weather_info[ch->room->wzone].state);
		}

		if (nCharAstronomySkill >= 70)
		{
			if (time_info.minute < 7)
			{
				sprintf (phrase + strlen (phrase), ", about %s o'clock,",
					strTimeWord[time_info.hour]);
			}
			else if (time_info.minute < 23)
			{
				sprintf (phrase + strlen (phrase), ", quarter-past %s o'clock,",
					strTimeWord[time_info.hour]);
			}
			else if (time_info.minute < 37)
			{
				sprintf (phrase + strlen (phrase), ", half-past %s o'clock,",
					strTimeWord[time_info.hour]);
			}
			else if (time_info.minute < 52)
			{
				sprintf (phrase + strlen (phrase), ", quarter-to %s o'clock,",
					strTimeWord[(time_info.hour + 1) % 24]);
			}
			else
			{
				sprintf (phrase + strlen (phrase), ", about %s o'clock,",
					strTimeWord[(time_info.hour + 1) % 24]);
			}
		}

		else if (nCharAstronomySkill >= 50)
		{
			if (time_info.minute < 15)
			{
				sprintf (phrase + strlen (phrase), ", about %s o'clock,",
					strTimeWord[time_info.hour]);
			}
			else if (time_info.minute < 45)
			{
				sprintf (phrase + strlen (phrase), ", half-past %s o'clock,",
					strTimeWord[time_info.hour]);
			}
			else
			{
				sprintf (phrase + strlen (phrase), ", about %s o'clock,",
					strTimeWord[(time_info.hour + 1) % 24]);
			}
		}

		else if (nCharAstronomySkill >= 30)
		{
			if (time_info.minute < 30)
			{
				sprintf (phrase + strlen (phrase), ", about %s o'clock,",
					strTimeWord[time_info.hour]);
			}
			else
			{
				sprintf (phrase + strlen (phrase), ", about %s o'clock,",
					strTimeWord[(time_info.hour + 1) % 24]);
			}
		}

	}

	sprintf (buf, "It is %s ", phrase);

	if (ch && ch->get_trust())
	{
		sprintf (buf + strlen (buf), "[%d:%s%d %s] ",
				 ((time_info.hour % 12 == 0) ? 12 : ((time_info.hour) % 12)),
				 time_info.minute >= 10 ? "" : "0",
				 time_info.minute,
				 ((time_info.hour >= 12) ? "pm" : "am"));
	}

	day = time_info.day + 1;	/* day in [1..35] */

	if (day == 1)
		strcpy (suf, "st");
	else if (day == 2)
		strcpy (suf, "nd");
	else if (day == 3)
		strcpy (suf, "rd");
	else if (day < 20)
		strcpy (suf, "th");
	else if ((day % 10) == 1)
		strcpy (suf, "st");
	else if ((day % 10) == 2)
		strcpy (suf, "nd");
	else if ((day % 10) == 3)
		strcpy (suf, "rd");
	else
		strcpy (suf, "th");

	sprintf (day_buf, "%d%s", day, suf);

	/* Special output for holidays */

	if (time_info.holiday == 0 &&
		!(time_info.month == 1 && day == 12) &&
		!(time_info.month == 4 && day == 10) &&
		!(time_info.month == 7 && day == 11) &&
		!(time_info.month == 10 && day == 12))
		sprintf (buf + strlen (buf), "on the %s day of the month of %s,", day_buf,
		month_name[(int) time_info.month]);
	else
	{
		if (time_info.holiday > 0)
		{
			sprintf (buf + strlen (buf), "on the feastday of %s,",
				holiday_names[time_info.holiday]);
		}
		else if (time_info.month == 1 && day == 12)
			sprintf (buf + strlen (buf), "on Erukyerme, The Prayer to Eru,");
		else if (time_info.month == 4 && day == 10)
			sprintf (buf + strlen (buf), "on Lairemerende, The Greenfest,");
		else if (time_info.month == 7 && day == 11)
			sprintf (buf + strlen (buf), "on Eruhantale, Thanksgiving to Eru,");
		else if (time_info.month == 10 && day == 12)
			sprintf (buf + strlen (buf), "on Airilaitale, The Hallowmas,");
	}

	sprintf (buf + strlen (buf),
		" %s in the year %d of the Steward's Reckoning.\n",
		season_string[(int) time_info.month], time_info.year);

	sprintf (time_str, "%s", buf);

	return time_str;
}

void do_time (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char *p;
	int day = time_info.day + 1, moon_r = 0, moon_s = 0;
	static char *strRelativeTime[8] = {
		"morning", "morning", "morning", "morning", "afternoon", "afternoon",
		"evening", "night"
	};

	sprintf (buf, "\n#6%s#0", time_string (ch));

	if (ch->skill_map["Astronomy"] > 0)
	{

		// Sunrise and set

		if (time_info.hour < sunrise[time_info.month])
		{

			sprintf (buf + strlen (buf),
				" #6Anor shall rise around %s o'clock this morning and will rest again around %s o'clock today.#0 ",
				strTimeWord[sunrise[time_info.month]],
				strTimeWord[sunset[time_info.month]]);

		}
		else if (time_info.hour > sunset[time_info.month])
		{

			sprintf (buf + strlen (buf),
				" #6Anor shall rise around %s o'clock this morning and will rest again around %s o'clock tomorrow.#0 ",
				strTimeWord[sunrise[time_info.month]],
				strTimeWord[sunset[time_info.month]]);

		}
		else
		{
			sprintf (buf + strlen (buf),
				" #6Anor shall rest around %s o'clock today and will rise again around %s o'clock tomorrow morning.#0 ",
				strTimeWord[sunset[time_info.month]],
				strTimeWord[sunrise[time_info.month]]);
		}


		moon_r = global_moon_values.moonrise; 
		moon_s = global_moon_values.moonset;


		if (moon_r < moon_s)
		{

			if (time_info.hour < moon_r)
			{
				sprintf (buf + strlen (buf),
					" #6 Ithil shall rise around %s o'clock this %s and will rest again around %s o'clock this %s.#0 ",
					strTimeWord[moon_r], strRelativeTime[moon_r / 3],
					strTimeWord[moon_s], strRelativeTime[moon_s / 3]);
			}
			else if (time_info.hour > moon_s)
			{
				sprintf (buf + strlen (buf),
					" #6 Ithil shall rise around %s o'clock tomorrow %s and will rest again around %s o'clock tomorrow %s.#0 ",
					strTimeWord[moon_r], strRelativeTime[moon_r / 3],
					strTimeWord[moon_s], strRelativeTime[moon_s / 3]);

			}
			else
			{
				sprintf (buf + strlen (buf),
					" #6 Ithil shall rest around %s o'clock this %s and will rise again around %s o'clock tomorrow %s.#0 ",
					strTimeWord[moon_s], strRelativeTime[moon_s / 3],
					strTimeWord[moon_r], strRelativeTime[moon_r / 3]);

			}

		}

		else
		{
			if (time_info.hour < moon_s)
			{
				sprintf (buf + strlen (buf),
					" #6 Ithil shall rest around %s o'clock this %s and will rise again around %s o'clock this %s.#0 ",
					strTimeWord[moon_s], strRelativeTime[moon_s / 3],
					strTimeWord[moon_r], strRelativeTime[moon_r / 3]);
			}
			else if (time_info.hour > moon_r)
			{
				sprintf (buf + strlen (buf),
					" #6 Ithil shall rest around %s o'clock tomorrow %s and will rise again around %s o'clock tomorrow %s.#0 ",
					strTimeWord[moon_s], strRelativeTime[moon_s / 3],
					strTimeWord[moon_r], strRelativeTime[moon_r / 3]);

			}
			else
			{
				sprintf (buf + strlen (buf),
					" #6 Ithil shall rise around %s o'clock this %s and will rest again around %s o'clock tomorrow %s.#0 ",
					strTimeWord[moon_r], strRelativeTime[moon_r / 3],
					strTimeWord[moon_s], strRelativeTime[moon_s / 3]);

			}
		}


		// Feastday info

		if (time_info.holiday == 0 &&
			!(time_info.month == 1 && day == 12) &&
			!(time_info.month == 4 && day == 10) &&
			!(time_info.month == 7 && day == 11) &&
			!(time_info.month == 10 && day == 12))
		{
			strcat (buf, "\n#6The next holiday will be ");
			if (time_info.month < 1
				|| (time_info.month == 1 && time_info.day <= 12))
			{
				sprintf (buf + strlen (buf), "Erukyerme in %d days.#0",
					((1 - time_info.month) * 30) + (12 - time_info.day));
			}
			else if (time_info.month < 3)
			{
				sprintf (buf + strlen (buf),
					"The Feastday of Tuilere in %d days.#0",
					((2 - time_info.month) * 30) + (30 - time_info.day));
			}
			else if (time_info.month < 4
				|| (time_info.month == 4 && time_info.day <= 10))
			{
				sprintf (buf + strlen (buf), "The Greenfest in %d days.#0",
					((4 - time_info.month) * 30) + (10 - time_info.day));
			}
			else if (time_info.month < 6)
			{
				// todo: add enderi in leapyears
				sprintf (buf + strlen (buf),
					"The Feastday of Loende in %d days.#0",
					((5 - time_info.month) * 30) + (30 - time_info.day));
			}
			else if (time_info.month < 7
				|| (time_info.month == 7 && time_info.day <= 11))
			{
				sprintf (buf + strlen (buf), "Eruhantale in %d days.#0",
					((7 - time_info.month) * 30) + (11 - time_info.day));
			}
			else if (time_info.month < 9)
			{
				sprintf (buf + strlen (buf),
					"The Feastday of Yaviere in %d days.#0",
					((8 - time_info.month) * 30) + (30 - time_info.day));
			}
			else if (time_info.month < 10
				|| (time_info.month == 10 && time_info.day <= 12))
			{
				sprintf (buf + strlen (buf), "The Hallowmas in %d days.#0",
					((10 - time_info.month) * 30) + (12 - time_info.day));
			}
			else
			{
				sprintf (buf + strlen (buf),
					"The Feastdays of Mettare & Yestare in %d days.#0",
					((12 - time_info.month) * 30) + (30 - time_info.day));
			}
		}
	}
	reformat_string (buf, &p);

	ch->send_to_char("\n");
	ch->send_to_char(p);

	free_mem (p); //char*
	p = NULL;

	if (ch->get_trust())
	{
		sprintf(buf, "\nThe following variables apply:\n");

		if (!is_dark (ch->room))
			sprintf (buf + strlen (buf), "   is_dark is false.\n");
		else
			sprintf (buf + strlen (buf),"   is_dark is true.\n");

		sprintf (buf + strlen (buf),"   Global Sunlight is %f.\n",
					  global_sun_light);
		
		sprintf (buf + strlen (buf),"   Global Moon light is %f.\n",
				 global_moon_values.light);
		
		sprintf (buf + strlen (buf),"   Global Moon Phase is %d.\n",
				 global_moon_values.phase);
		
		if (IS_OUTSIDE (ch))
			sprintf (buf + strlen (buf),"   OUTSIDE is true.\n");
		else
			sprintf (buf + strlen (buf),"   OUTSIDE is false.\n");

		if ((global_sun_light <= SUN_TWILIGHT))
			sprintf (buf + strlen (buf),"   (global_sun_light <= SUN_TWILIGHT) is true.\n");
		else
			sprintf (buf + strlen (buf),"   (global_sun_light <= SUN_TWILIGHT) is false.\n");


		sprintf (buf + strlen (buf), "   Light count in room:  %f\n", ch->room->light);
		
		sprintf (buf + strlen (buf), "   Season is: %s\n", seasons[time_info.season]);
		
		sprintf (buf + strlen (buf), "   Weather is: %d\n", desc_weather[ch->room->wzone]);
		
		int eq_days;
		if (time_info.month < 3)
		{
			eq_days = ((2 - time_info.month) * 30) + (30 - time_info.day);
		}
		else if (time_info.month >= 3)
		{
			eq_days = ((12 + 2 - time_info.month) * 30) + (30 - time_info.day);
		}
		
		sprintf (buf + strlen (buf), "   %d days since last Spring Equinox\n", (365 - eq_days));
		ch->send_to_char(buf);

		
	}
}

void do_weather (CHAR_DATA * ch, char *argument, int cmd)
{
	int ind = 0;
	char w_phrase[AVG_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	std::stringstream buffer;
	std::stringstream tempstr;
	char imm_buf[MAX_STRING_LENGTH];
	int wind_case = 0;
	int temp_case = 0;
	char wind[AVG_STRING_LENGTH] = { '\0' };
	int high_sun = 0;
	int i = 0;


	argument = one_argument (argument, buf);

	/*** Start set weather ***/
	if(!strcmp("set", buf))
	{
		argument = one_argument (argument, buf);

		if(!strcmp("?", buf))
		{
			ch->send_to_char("The following weather states are available:\n\n");

			/* Cycle through each struct and show options */
			sprintf (buf,"     Fog States\n");
			for (i = 0; *fog_states[i] != '\n'; i++)
			{
				sprintf (buf + strlen (buf),"     #6%s#0\n",
					fog_states[i]);
			}
			sprintf (buf + strlen (buf),"\n");

			sprintf (buf + strlen (buf),"     Precipitation States\n");
			for (i = 0; *weather_states[i] != '\n'; i++)
			{
				sprintf (buf + strlen (buf),"     #6%s#0\n",
					weather_states[i]);
			}
			sprintf (buf + strlen (buf),"\n");

			sprintf (buf + strlen (buf),"     Cloud Covers\n");
			for (i = 0; *weather_clouds[i] != '\n'; i++)
			{
				sprintf (buf + strlen (buf),"     #6%s#0\n",
					weather_clouds[i]);
			}
			sprintf (buf + strlen (buf),"\n");

			sprintf (buf + strlen (buf),"     Wind Speeds\n");
			for (i = 0; *wind_speeds[i] != '\n'; i++)
			{
				sprintf (buf + strlen (buf),"     #6%s#0\n",
					wind_speeds[i]);
			}
			sprintf (buf + strlen (buf),"\n");

			sprintf (buf + strlen (buf),"     Wind Directions\n");
			for (i = 0; *wind_directions[i] != '\n'; i++)
			{
				sprintf (buf + strlen (buf),"     #6%s#0\n",
					wind_directions[i]);
			}
			sprintf (buf + strlen (buf),"\n");

			sprintf (buf + strlen (buf),"     Special Effects\n");
			for (i = 0; *special_effects[i] != '\n'; i++)
			{
				sprintf (buf + strlen (buf),"     #6%s#0\n",
					special_effects[i]);
			}
			sprintf (buf + strlen (buf),"\n");

			page_string(ch->desc, buf);

			return;
		}

		if ((ch->get_trust())
			&& (ch->get_trust() > 3)
			&& *buf)
		{
			if ((ind = index_lookup (weather_states, buf)) != -1)
			{
				sprintf (buf, "You have changed the precipitation state to #6%s#0.\n",
					weather_states[ind]);
				ch->send_to_char(buf);
				weather_info[ch->room->wzone].state = ind;
				return;
			}
			else if ((ind = index_lookup (weather_clouds, buf)) != -1)
			{
				sprintf (buf, "You have changed cloud state to #6%s#0.\n",
					weather_clouds[ind]);
				ch->send_to_char(buf);
				weather_info[ch->room->wzone].clouds = ind;
				return;
			}
			else if ((ind = index_lookup (wind_speeds, buf)) != -1)
			{
				sprintf (buf, "You have changed wind speed to #6%s#0.\n",
					wind_speeds[ind]);
				ch->send_to_char(buf);
				weather_info[ch->room->wzone].wind_speed = ind;
				return;
			}
			else if ((ind = index_lookup (fog_states, buf)) != -1)
			{
				sprintf (buf, "You have changed the fog level to #6%s#0.\n",
					fog_states[ind]);
				ch->send_to_char(buf);
				weather_info[ch->room->wzone].fog = ind;
				return;
			}
			else if((ind = index_lookup (wind_directions, buf)) != -1)
			{
				sprintf (buf, "You have changed the wind direction to #6%s#0.\n",
					wind_directions[ind]);
				ch->send_to_char(buf);
				weather_info[ch->room->wzone].wind_dir = ind;
				return;
			}
			else if((ind = index_lookup (special_effects, buf)) != -1)
			{
				sprintf (buf, "You have changed the special effect to #6%s#0.\n",
					special_effects[ind]);
				ch->send_to_char(buf);
				weather_info[ch->room->wzone].special_effect = ind;
				return;
			}
			ch->send_to_char("That is not a recognized weather state.\n");
			return;
		}
	}
	/*** End set weather***/

	int weather_echo_state = 0; // 0 = outdoors, 1 = indoors, 2 = deep indoors
	if (!IS_OUTSIDE(ch))
	{
		for (int exit = 0; exit <= LAST_DIR; exit++)
		{
			if (is_exit(ch, exit) && (is_exit(ch, exit)->to_room != -1))
			{
				ROOM_DATA * room = vtor(is_exit(ch, exit)->to_room);
				if (!room)
					continue;

				if (!IS_SET(room->room_flags, INDOORS)
					&& room->terrain_type != SECT_URBAN
					&& room->terrain_type != SECT_CAVE
					&& room->terrain_type != SECT_UNDERWATER)
				{
					weather_echo_state = 1;
					break;
				}
			}
		}
		if (!weather_echo_state)
			weather_echo_state = 2;
	}

	if (ch->get_trust())
		weather_echo_state = 0;

	if (weather_info[ch->room->wzone].temperature < 120)
	{
		wind_case = 0;
		temp_case = 1;
	}
	if (weather_info[ch->room->wzone].temperature < 110)
	{
		wind_case = 1;
		temp_case = 2;
	}
	if (weather_info[ch->room->wzone].temperature < 100)
	{
		wind_case = 2;
		temp_case = 3;
	}
	if (weather_info[ch->room->wzone].temperature < 94)
	{
		wind_case = 3;
		temp_case = 3;
	}
	if (weather_info[ch->room->wzone].temperature < 90)
	{
		wind_case = 3;
		temp_case = 4;
	}
	if (weather_info[ch->room->wzone].temperature < 80)
	{
		wind_case = 4;
		temp_case = 5;
	}
	if (weather_info[ch->room->wzone].temperature < 75)
	{
		wind_case = 4;
		temp_case = 5;
	}
	if (weather_info[ch->room->wzone].temperature < 65)
	{
		wind_case = 5;
		temp_case = 6;
	}
	if (weather_info[ch->room->wzone].temperature < 55)
	{
		wind_case = 6;
		temp_case = 7;
	}
	if (weather_info[ch->room->wzone].temperature < 47)
	{
		wind_case = 7;
		temp_case = 8;
	}
	if (weather_info[ch->room->wzone].temperature < 38)
	{
		wind_case = 7;
		temp_case = 9;
	}
	if (weather_info[ch->room->wzone].temperature < 33)
	{
		wind_case = 8;
		temp_case = 10;
	}
	if (weather_info[ch->room->wzone].temperature < 21)
	{
		wind_case = 9;
		temp_case = 11;
	}
	if (weather_info[ch->room->wzone].temperature < 11)
	{
		wind_case = 9;
		temp_case = 12;
	}
	if (weather_info[ch->room->wzone].temperature < 1)
	{
		wind_case = 10;
		temp_case = 13;
	}
	if (weather_info[ch->room->wzone].temperature < -10)
	{
		wind_case = 10;
		temp_case = 14;
	}
	*buf = '\0';
	*buf2 = '\0';

	high_sun = ((sunrise[time_info.month] + sunset[time_info.month]) / 2);

	sprintf (w_phrase, "%s", time_phrase(high_sun));
					   
	if (time_info.season == SPRING)
		sprintf (buf2, "spring ");
	if (time_info.season == SUMMER)
		sprintf (buf2, "summer ");
	if (time_info.season == AUTUMN)
		sprintf (buf2, "autumn ");
	if (time_info.season == WINTER)
		sprintf (buf2, "winter ");
	
	*imm_buf = '\0';

	if (ch->get_trust())
		sprintf (imm_buf, " [%d F]",
		weather_info[ch->room->wzone].temperature);

	switch (temp_case)
	{
	case 0:
		sprintf (buf, "  It is a dangerously searing %s%s%s. ", buf2,
			w_phrase, imm_buf);
		break;
	case 1:
		sprintf (buf, "  It is a painfully blazing %s%s%s. ", buf2,
			w_phrase, imm_buf);
		break;
	case 2:
		sprintf (buf, "  It is a blistering %s%s%s. ", buf2, w_phrase,
			imm_buf);
		break;
	case 3:
		sprintf (buf, "  It is a sweltering %s%s%s. ", buf2, w_phrase,
			imm_buf);
		break;
	case 4:
		sprintf (buf, "  It is a hot %s%s%s. ", buf2, w_phrase, imm_buf);
		break;
	case 5:
		sprintf (buf, "  It is a temperate %s%s%s. ", buf2, w_phrase,
			imm_buf);
		break;
	case 6:
		sprintf (buf, "  It is a cool %s%s%s. ", buf2, w_phrase, imm_buf);
		break;
	case 7:
		sprintf (buf, "  It is a chill %s%s%s. ", buf2, w_phrase, imm_buf);
		break;
	case 8:
		sprintf (buf, "  It is a cold %s%s%s. ", buf2, w_phrase, imm_buf);
		break;
	case 9:
		sprintf (buf, "  It is a very cold %s%s%s. ", buf2, w_phrase,
			imm_buf);
		break;
	case 10:
		sprintf (buf, "  It is a frigid %s%s%s. ", buf2, w_phrase, imm_buf);
		break;
	case 11:
		sprintf (buf, "  It is a freezing %s%s%s. ", buf2, w_phrase,
			imm_buf);
		break;
	case 12:
		sprintf (buf, "  It is a numbingly frigid %s%s%s. ", buf2, w_phrase,
			imm_buf);
		break;
	case 13:
		sprintf (buf, "  It is a painfully freezing %s%s%s. ", buf2,
			w_phrase, imm_buf);
		break;
	case 14:
		sprintf (buf, "  It is a dangerously freezing %s%s%s. ", buf2,
			w_phrase, imm_buf);
		break;

	default:
		sprintf (buf, "  It is a cool %s%s%s. ", buf2, w_phrase, imm_buf);
		break;
	}

	*buf2 = '\0';
	*w_phrase = '\0';
	
	if (weather_info[ch->room->wzone].state >= LIGHT_RAIN)
	{
		if (weather_info[ch->room->wzone].wind_speed == STORMY)
			sprintf (w_phrase, "rolling");
		else if (weather_info[ch->room->wzone].wind_speed > BREEZE)
			sprintf (w_phrase, "flowing");
		else
			sprintf (w_phrase, "brooding");

		switch (weather_info[ch->room->wzone].clouds)
		{
		case LIGHT_CLOUDS:
			sprintf (buf2, "a scattering of grey clouds. ");
			break;
		case HEAVY_CLOUDS:
			sprintf (buf2, "dark, %s clouds. ", w_phrase);
			break;
		case OVERCAST:
			sprintf (buf2, "a thick veil of %s storm clouds. ", w_phrase);
			break;
		}

		if (weather_info[ch->room->wzone].fog == THICK_FOG)
			sprintf (buf2, " the fog-shrouded sky. ");

		switch (weather_info[ch->room->wzone].state)
		{
		case LIGHT_RAIN:
			sprintf (buf + strlen(buf), "A light drizzle is falling from %s", buf2);
			break;
		case STEADY_RAIN:
			sprintf (buf + strlen(buf), "A steady rain is falling from %s", buf2);
			break;
		case HEAVY_RAIN:
			sprintf (buf + strlen(buf), "A shower of rain is pouring from %s", buf2);
			break;
		case LIGHT_SNOW:
			sprintf (buf + strlen(buf), "A light snow is falling from %s", buf2);
			break;
		case STEADY_SNOW:
			sprintf (buf + strlen(buf), "Snow is falling from %s", buf2);
			break;
		case HEAVY_SNOW:
			sprintf (buf + strlen(buf),
				"Blinding snow swarms down from an obscured white sky. ");
			break;
		}
		
	}

	*buf2 = '\0';

	if (weather_info[ch->room->wzone].wind_speed)
	{
		if (weather_info[ch->room->wzone].wind_dir == NORTH_WIND)
			wind_case++;

		switch (wind_case)
		{

		case 0:
			sprintf (buf2, "searing");
			break;
		case 1:
			sprintf (buf2, "scorching");
			break;
		case 2:
			sprintf (buf2, "sweltering");
			break;
		case 3:
			sprintf (buf2, "hot");
			break;
		case 4:
			sprintf (buf2, "warm");
			break;
		case 5:
			sprintf (buf2, "cool");
			break;
		case 6:
			sprintf (buf2, "chill");
			break;
		case 7:
			sprintf (buf2, "cold");
			break;
		case 8:
			sprintf (buf2, "frigid");
			break;
		case 9:
			sprintf (buf2, "freezing");
			break;
		case 10:
			sprintf (buf2, "arctic");
			break;

		default:
			sprintf (buf2, "cool");
			break;
		}

		if (weather_echo_state == 1)
			sprintf (wind, "%s", buf2);
		else if (weather_info[ch->room->wzone].wind_dir == NORTH_WIND)
			sprintf (wind, "%s northerly", buf2);
		else if(weather_info[ch->room->wzone].wind_dir == NORTHEAST_WIND)
			sprintf (wind, "%s north easterly", buf2);
		else if(weather_info[ch->room->wzone].wind_dir == EAST_WIND)
			sprintf (wind, "%s easterly", buf2);
		else if(weather_info[ch->room->wzone].wind_dir == SOUTHEAST_WIND)
			sprintf (wind, "%s south easterly", buf2);
		else if(weather_info[ch->room->wzone].wind_dir == SOUTH_WIND)
			sprintf (wind, "%s southerly", buf2);
		else if(weather_info[ch->room->wzone].wind_dir == SOUTHWEST_WIND)
			sprintf (wind, "%s south westerly", buf2);
		else if(weather_info[ch->room->wzone].wind_dir == NORTHWEST_WIND)
			sprintf (wind, "%s north westerly", buf2);
		else
			sprintf (wind, "%s westerly", buf2);
	}


	if (weather_echo_state == 1 
		|| weather_info[ch->room->wzone].state >= LIGHT_RAIN
		|| !weather_info[ch->room->wzone].clouds
		|| weather_info[ch->room->wzone].fog == THICK_FOG)
	{
		switch (weather_info[ch->room->wzone].wind_speed)
		{
		case CALM:
			sprintf (buf + strlen(buf), "The air is calm and quiet");
			break;
		case BREEZE:
			sprintf (buf + strlen(buf), "There is a %s breeze", wind);
			break;
		case WINDY:
			sprintf (buf + strlen(buf), "There is a %s wind", wind);
			break;
		case GALE:
			sprintf (buf + strlen(buf), "A %s gale is blowing", wind);
			break;
		case STORMY:
			sprintf (buf + strlen(buf), "A %s wind whips and churns in a stormy fury",
				wind);
			break;
		}
		
		if (weather_echo_state == 1)
		{
			sprintf (buf + strlen(buf), " outside");
			sprintf(buf + strlen(buf), ". ");

			std::stringstream tempstr;
			tempstr << reformat_desc(buf);	
			ch->send_to_char(tempstr.str().c_str());
			
						
			
			return;
		}
		else 
		{
			sprintf(buf + strlen(buf), ". ");
		}
	}

	*buf2 = '\0';

	if (weather_info[ch->room->wzone].state < LIGHT_RAIN
		&& weather_info[ch->room->wzone].clouds
		&& weather_info[ch->room->wzone].fog < THICK_FOG)
	{
		if (weather_info[ch->room->wzone].state == NO_RAIN)
			sprintf (w_phrase, "rain");
		else
			sprintf (w_phrase, "white");

		switch (weather_info[ch->room->wzone].clouds)
		{
		case LIGHT_CLOUDS:
			sprintf (buf2, "Wispy %s clouds", w_phrase);
			break;
		case HEAVY_CLOUDS:
			sprintf (buf2, "Heavy %s clouds", w_phrase);
			break;
		case OVERCAST:
			sprintf (buf2, "A veil of thick %s clouds", w_phrase);
			break;
		}

		switch (weather_info[ch->room->wzone].wind_speed)
		{
		case CALM:
			sprintf (buf + strlen(buf), "%s hang motionless in the sky. ", buf2);
			break;
		case BREEZE:
			sprintf (buf + strlen(buf), "%s waft overhead upon a %s breeze. ", buf2,
				wind);
			break;
		case WINDY:
			sprintf (buf + strlen(buf), "%s waft overhead upon the %s winds. ", buf2,
				wind);
			break;
		case GALE:
			sprintf (buf + strlen(buf), "%s rush overhead upon a %s gale. ", buf2, wind);
			break;
		case STORMY:
			sprintf (buf + strlen(buf),
				"%s churn violently in the sky upon the %s winds. ",
				buf2, wind);
			break;
		}
		
	}

	if (!weather_echo_state && weather_info[ch->room->wzone].fog
		&& !(weather_info[ch->room->wzone].state >= LIGHT_RAIN
		&& weather_info[ch->room->wzone].fog == THICK_FOG))
	{
		if (weather_info[ch->room->wzone].fog == THIN_FOG)
			sprintf (buf + strlen(buf),"A patchy fog floats in the air. ");
		else
			sprintf (buf + strlen(buf),"A thick fog lies heavy upon the land. ");
	}

	
	/*** Special effects set by IMMs***/
	if (weather_info[ch->room->wzone].special_effect != NO_EFFECT)
	{
		//show effect specific message
		if (weather_info[ch->room->wzone].special_effect == VOLCANIC_SMOKE)
		{
			sprintf (buf + strlen(buf),"A cloud of thick, dust filled volcanic smoke drifts through the air. ");
		}
		else if (weather_info[ch->room->wzone].special_effect == FOUL_STENCH)
		{
			sprintf (buf + strlen(buf), "A foul stench permeates the area. ");
		}
		else if (!weather_echo_state && weather_info[ch->room->wzone].special_effect == LOW_MIST)
		{
			sprintf (buf + strlen(buf), "A low, eerie mist sits heavily upon the land. ");
		}
	}

	if (!weather_echo_state && moon_light[ch->room->zone] >= 1)
	{
		if (weather_info[ch->room->wzone].clouds <= LIGHT_CLOUDS)
		{
			if (global_moon_values.light == MOON_FULL)
				sprintf (buf + strlen(buf), "A full and gleaming Ithil limns the area in ghostly argent radiance. ");
			else if (global_moon_values.light == MOON_GIBOUS)
				sprintf (buf + strlen(buf), "A gibbous Ithil brightens the land. ");
			else if (global_moon_values.light == MOON_CRESCENT)
				sprintf (buf + strlen(buf), "A crescent of Ithil graces the sky. ");
			else 		
				sprintf (buf + strlen(buf), "Ithil's ethereal silhouette is barely visible in the sky.  ");
		}
		else
		{
			if (global_moon_values.light == MOON_FULL)
				sprintf (buf + strlen(buf), "A full and gleaming Ithil is barely visible through gaps in the clouds. ");
			else if (global_moon_values.light == MOON_GIBOUS)
				sprintf (buf + strlen(buf), "A gibbous Ithil is barely visible through gaps in the clouds.  ");
			else if (global_moon_values.light == MOON_CRESCENT)
				sprintf (buf + strlen(buf), "A crescent of Ithil is barely visible through gaps in the clouds.  ");
			else 		
				sprintf (buf + strlen(buf), "Ithil's ethereal silhouette is barely visible through gaps in the clouds.  ");
		}	
		
		
	}

	if (weather_echo_state == 2)
		sprintf (buf + strlen(buf), "You can glean no further details as you are too far from the outside world.\n");
	
	tempstr.sync();
	tempstr << reformat_desc(buf);	
	ch->send_to_char(tempstr.str().c_str());

}

void
post_help (DESCRIPTOR_DATA * d)
{
	char date[32] = "";
	time_t current_time = 0;

	mysql_safe_query
		("DELETE FROM helpfiles WHERE category = \'%s\' AND name = \'%s\'",
		d->character->delay_who, d->character->delay_who2);

	current_time = time (0);
	ctime_r (&current_time, date);
	if (strlen (date) > 1)
		date[strlen (date) - 1] = '\0';

	if (!d->pending_message->message)
	{
		d->character->send_to_char("No help entry written.\n");
		d->pending_message = NULL;
		d->character->delay_who = NULL;
		d->character->delay_who2 = NULL;
		d->character->delay_info1 = 0;
		return;
	}

	*d->character->delay_who = toupper (*d->character->delay_who);
	*d->character->delay_who2 = toupper (*d->character->delay_who2);

	mysql_safe_query
		("INSERT INTO helpfiles VALUES (\'%s\', \'%s\', \'\n%s\n\', \'(null)\', \'%d\', \'%s\', \'%s\')",
		d->character->delay_who2, d->character->delay_who,
		d->pending_message->message, d->character->delay_info1, date,
		d->character->name);

	d->pending_message = NULL;
	d->character->delay_who = NULL;
	d->character->delay_who2 = NULL;
	d->character->delay_info1 = 0;
}

void
do_hedit (CHAR_DATA * ch, char *argument, int cmd)
{
	char topic[MAX_STRING_LENGTH] = { '\0' };
	char subject[MAX_STRING_LENGTH] = { '\0' };
	char level[MAX_STRING_LENGTH] = { '\0' };
	int lvl = 0;

	argument = one_argument (argument, topic);
	argument = one_argument (argument, subject);
	argument = one_argument (argument, level);

	if (IS_NPC (ch))
	{
		ch->send_to_char("This is a PC-only command.\n");
		return;
	}

	if (!*topic || !*subject || !*level)
	{
		ch->send_to_char
			("You must specify the topic, subject, and required level for the help entry.\n");
		return;
	}

	if (!isdigit (*level))
	{
		ch->send_to_char
			("You must specify a number for the entry's required access level.\n");
		return;
	}

	lvl = atoi (level);

	ch->delay_who = duplicateString (topic);
	ch->delay_who2 = duplicateString (subject);
	ch->delay_info1 = lvl;

	ch->send_to_char("Enter the text of this database entry:\n");

	ch->make_quiet();

	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;
	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;
	ch->desc->proc = post_help;
}

void
do_vboards (CHAR_DATA * ch, char *argument, int cmd)
{
	MYSQL_RES *result = NULL;
	MYSQL_ROW row = NULL;
	char buf[MAX_STRING_LENGTH];
	char temp_buf[MAX_STRING_LENGTH];
	int colnum = 1;

	
	mysql_safe_query
		("SELECT board_name FROM virtual_boards GROUP BY board_name ORDER BY board_name ASC");
	result = mysql_store_result (database);

	while ((row = mysql_fetch_row (result)))
	{
		if (colnum == 1)
			sprintf (temp_buf + strlen (temp_buf), "%-29s", row[0]);
		else if (colnum == 2)
			sprintf (temp_buf + strlen (temp_buf), "%-29s", row[0]);
		else if (colnum == 3)
		{
			sprintf (temp_buf + strlen (temp_buf), "%s\n", row[0]);
			colnum = 1;
			continue;
		}
		colnum++;
	}

	sprintf (buf, "#6                      %s Virtual Boards#0\n", MUD_NAME);
	ch->send_to_char(buf);

	if (!*temp_buf)
		ch->send_to_char("None.\n");
	else
		page_string (ch->desc, temp_buf);

	if (colnum != 1)
		ch->send_to_char("\n");

}

void
log_missing_helpfile (CHAR_DATA * ch, char *string)
{
	char msg[MAX_STRING_LENGTH] = { '\0' };
	char subj[MAX_STRING_LENGTH] = { '\0' };
	MYSQL_RES *result = NULL;

	mysql_safe_query
		("SELECT * FROM unneeded_helpfiles WHERE name = '%s' AND (datestamp > UNIX_TIMESTAMP()-60*60*24*30*6)",
		string);
	result = mysql_store_result (database);

	if (result && mysql_num_rows (result) >= 1)
	{
		mysql_free_result (result);
		return;
	}

	mysql_safe_query ("DELETE FROM unneeded_helpfiles WHERE name = \'%s\'",
		string);

	sprintf (subj, "#1Not Found:#0 %s", string);
	sprintf (msg, "%s: help %s\n", ch->name, string);

	add_message (1, "Helpfiles", -5, ch->name, 0, subj, "", msg, 0);

	mysql_safe_query ("INSERT INTO unneeded_helpfiles VALUES (\'%s\', %d)",
		string, (int) time (0));
}


int
number_of_helpfiles_available (int player_level)
{
	// Build a query to get the number of helpfiles for that level
	int number_of_helpfiles = 0;
	std::ostringstream query_stream;
	query_stream << "SELECT COUNT(*) AS num_helpfiles "
		<< "FROM helpfiles "
		<< "WHERE required_level <= "
		<< player_level;

	std::string query = query_stream.str ();
	if ((mysql_safe_query ((char *)query.c_str())) == 0)
	{
		MYSQL_RES *result = 0;
		if ((result = mysql_store_result (database)) != 0)
		{
			MYSQL_ROW row = 0;
			if ((row = mysql_fetch_row (result)) != 0)
			{
				number_of_helpfiles = strtol (row[0],0,10);
			}
			mysql_free_result (result);
		}
		else
		{
			std::string error_message =
				"number_of_helpfiles_available: "
				"Couldn't get a helpfile count because: ";
			error_message += mysql_error (database);

			std::cerr << error_message << std::endl;
			system_log (error_message.c_str (), true);
		}
	}
	else
	{
		std::string error_message =
			"number_of_helpfiles_available: "
			"'mysql_safe_query' failed for the following reason: ";
		error_message += mysql_error (database);

		std::cerr << error_message << std::endl;
		system_log (error_message.c_str (), true);
	}

	return number_of_helpfiles;
}

std::string
output_categories_available (int player_level)
{
	std::string category_list;

	// Query a list of categories from the helpfiles table
	std::ostringstream query_stream;
	query_stream << "SELECT category "
		<< "FROM helpfiles "
		<< "WHERE required_level <= "
		<< player_level << ' '
		<< "GROUP BY category ASC";

	std::string query = query_stream.str ();
	if ((mysql_safe_query ((char *)query.c_str ())) == 0)
	{
		// Store a formatted table of categories
		MYSQL_RES *result = 0;
		if ((result = mysql_store_result (database)) != 0)
		{
			char category[512];
			int category_counter = 0;
			MYSQL_ROW row = 0;
			while ((row = mysql_fetch_row (result)))
			{
				sprintf (category, "#6%-18.18s#0 ", row[0]);
				category_list += category;
				if (!(category_counter % 4))
				{
					category_list += '\n';
				}
				category_counter++;
			}
			if ((category_counter % 4) != 1)
			{
				category_list += '\n';
			}
			mysql_free_result (result);
		}
		else
		{
			std::string error_message =
				"output_categories_available: "
				"Couldn't create a category list because: ";
			error_message += mysql_error (database);
			std::cerr << error_message << std::endl;
			system_log (error_message.c_str (), true);
		}
	}
	else
	{//changed for Auroness version
		std::string error_message =
			"output_categories_available: "
			"'mysql_safe_query' failed to query helpfiles database table because: ";
		error_message += mysql_error (database);

		std::cerr << error_message << std::endl;
		system_log (error_message.c_str (), true);
	}

	return category_list;
}

void do_help (CHAR_DATA * ch, char *argument, int cmd)
{
	MYSQL_RES *result = NULL;
	MYSQL_ROW row = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char temp_buf[MAX_STRING_LENGTH] = { '\0' };
	char arg1[MAX_STRING_LENGTH] = { '\0' };
	char arg2[MAX_STRING_LENGTH] = { '\0' };
	char example[MAX_STRING_LENGTH] = { '\0' };
	char original[MAX_STRING_LENGTH] = { '\0' };
	bool header_needed = false, category_list = false, soundex = false;
	int j = 1;
	int player_level = 0;
	std::string cat_list;

		
	if (strstr (argument, ":"))
	{
		ch->send_to_char("Please see #6HELP HELP#0 for proper syntax and usage.\n");
		return;
	}

	sprintf (original, "%s", argument);

	argument = one_argument (argument, arg1);

	if (*argument)
		argument = one_argument (argument, arg2);

	// Mortals and NPCs get Level 0 Help
	char level_condition[16];
	if ((!ch->get_trust()) || IS_NPC (ch))
	{
		player_level = 0;
		sprintf (level_condition, "= 0");
	}
	else
	{
		player_level = ch->pc->level;
		sprintf (level_condition, "<= %d", ch->pc->level);
	}

	if (!*arg1)
	{
		int help_available = number_of_helpfiles_available (player_level);
		cat_list = output_categories_available (player_level);

		// Query one helpfile name at random
		sprintf (buf,
			"SELECT name "
			"FROM helpfiles "
			"WHERE required_level %s "
			"ORDER BY RAND() DESC "
			"LIMIT 1", level_condition);

		if ((mysql_safe_query (buf)) !=0)
		{//changed for Auroness version
			std::cerr << "The library call 'mysql_safe_query' failed to "
				<< "query helpfiles database table for the following reason: "
				<< mysql_error (database) << std::endl;
			return;
		}

		// Store the helpfile name
		*buf = 0;
		if ((result = mysql_store_result (database)) != 0)
		{
			if ((row = mysql_fetch_row (result)))
			{
				sprintf (buf, "Helpfile of the Moment: #6%s#0", row[0]);
			}
			mysql_free_result (result);
		}
		else
		{
			sprintf (buf, "do_help: "
				"Couldn't create a category list because: %s\n",
				mysql_error (database));
			std::cerr << buf;
			system_log (buf, true);
		}

		// Spit out a generic syntax message
		sprintf (temp_buf,
			"\n                      #6%s Help Database#0\n\n"
			"There are currently #6%d#0 helpfiles accessible to you"
			" in our database.\n\n"
			"Helpfile Categories:\n\n%s\n%s\n\n"
			"To see a full list of helpfiles within these topics,"
			" use \'#6help on <topic name>#0\'.\n\n"
			"For a detailed description of HELP searching syntax,"
			" please see \'#6help help#0\'.\n\n"
			"Our helpfiles on the Web:"
			" #6http://www.middle-earth.us/index.php?display=help#0.\n",
			MUD_NAME,
			help_available,
			cat_list.empty () ? "#6None#0\n" : cat_list.c_str (),
			buf);

		page_string (ch->desc, temp_buf);
		return;
	}

	if (strcasecmp (arg1, "named") == STR_MATCH && *arg2)
		mysql_safe_query
		("SELECT * FROM helpfiles WHERE name LIKE '%%%s%%' AND required_level <= %d ORDER BY category,name ASC",
		arg2, ch->pc ? ch->pc->level : 0);
	else if (strcasecmp (arg1, "containing") == STR_MATCH && *arg2)
		mysql_safe_query
		("SELECT * FROM helpfiles WHERE (entry LIKE '%%%s%%' OR related_entries LIKE '%%%s%%') AND required_level <= %d ORDER BY category,name ASC",
		arg2, arg2, ch->pc ? ch->pc->level : 0);
	else if (strcasecmp (arg1, "on") == STR_MATCH && *arg2)
	{
		category_list = true;
		mysql_safe_query
			("SELECT * FROM helpfiles WHERE category LIKE '%s%%' AND required_level <= %d ORDER BY name ASC",
			arg2, ch->pc ? ch->pc->level : 0);
	}
	else if (*arg1 && !*arg2)
		mysql_safe_query
		("SELECT * FROM helpfiles WHERE name LIKE '%s%%' AND required_level <= %d ORDER BY category,name ASC",
		arg1, ch->pc ? ch->pc->level : 0);
	else if (*arg1 && *arg2)
		mysql_safe_query
		("SELECT * FROM helpfiles WHERE name = '%s' AND (category = '%s' AND required_level <= %d) ORDER BY category,name ASC",
		arg2, arg1, ch->pc ? ch->pc->level : 0);

	result = mysql_store_result (database);

	if (!result)
	{
		ch->send_to_char
			("No helpfile matching that query was found in the database.\n\n#6Please see HELP HELP to ensure you are using the proper query syntax.#0\n");
		log_missing_helpfile (ch, original);
		return;
	}

	if (mysql_num_rows (result) == 0)
	{
		result = NULL;
		mysql_safe_query
			("SELECT * FROM helpfiles WHERE category LIKE '%s%%' AND required_level <= %d ORDER BY name ASC",
			arg1, ch->pc ? ch->pc->level : 0);
		result = mysql_store_result (database);
		if (!result || mysql_num_rows (result) == 0)
		{
			mysql_safe_query
				("SELECT * FROM helpfiles WHERE SOUNDEX(category) LIKE SOUNDEX('%s') AND required_level <= %d ORDER BY category,name ASC",
				arg1, ch->pc ? ch->pc->level : 0);
			result = mysql_store_result (database);
			if (!result || mysql_num_rows (result) == 0)
			{
				mysql_safe_query
					("SELECT * FROM helpfiles WHERE SOUNDEX(name) LIKE SOUNDEX('%s') AND required_level <= %d ORDER BY category,name ASC",
					arg1, ch->pc ? ch->pc->level : 0);
				result = mysql_store_result (database);
				if (!result || mysql_num_rows (result) == 0)
				{
					mysql_safe_query
						("SELECT * FROM helpfiles WHERE related_entries LIKE '%%%s%%' AND required_level <= %d ORDER BY category,name ASC",
						arg1, ch->pc ? ch->pc->level : 0);
					result = mysql_store_result (database);
					if (!result || mysql_num_rows (result) == 0)
					{
						mysql_safe_query
							("SELECT * FROM helpfiles WHERE entry LIKE '%%%s%%' AND required_level <= %d ORDER BY category,name ASC",
							arg1, ch->pc ? ch->pc->level : 0);
						result = mysql_store_result (database);
						if (result || mysql_num_rows (result) > 0)
							category_list = true;
					}
				}
				else
					soundex = true;
			}
			else
				category_list = true;
		}
		else
			category_list = true;
	}
	if (mysql_num_rows (result) == 1)
	{
		row = mysql_fetch_row (result);
		if (!row || ch->get_trust() < atoi (row[4]))
		{
			if (!mysql_num_rows (result))
			{
				if (result != NULL)
					mysql_free_result (result);
				ch->send_to_char
					("No helpfile matching that query was found in the database.\n\n#6Please see HELP HELP to ensure you are using the proper query syntax.#0\n");
				mysql_free_result (result);
				log_missing_helpfile (ch, original);
				return;
			}
		}
		if (mysql_num_rows (result) == 1)
		{
			sprintf (temp_buf, "\n#6%s: %s#0\n", row[1], row[0]);
			if (*row[2] != '\n')
				strcat (temp_buf, "\n");
			sprintf (temp_buf + strlen (temp_buf), "%s", row[2]);
			if (*row[3] && strcasecmp (row[3], "(null)") != STR_MATCH)
				sprintf (temp_buf + strlen (temp_buf), "#6See Also:#0 %s\n\n", row[3]);
			sprintf (temp_buf + strlen (temp_buf), "#6Last Updated:#0 %s, by %s\n",
				row[5], CAP (row[6]));
		}
	}

	if (mysql_num_rows (result) > 1)
	{
		header_needed = true;
		while ((row = mysql_fetch_row (result)))
		{
			if (ch->get_trust() < atoi (row[4]))
				continue;
			if (header_needed)
			{
				if (!soundex)
					sprintf (temp_buf,
					"\nYour query matched the following helpfiles:\n\n   ");
				else
					sprintf (temp_buf,
					"\nThe following entries most closely matched your spelling:\n\n   ");
				header_needed = false;
			}
			if (!category_list)
				sprintf (arg2, "%13s: %s", row[1], row[0]);
			else
				sprintf (arg2, "%s", row[0]);
			if (!category_list)
			{
				if (j == 1)
				{
					sprintf (arg1, "%3d. #6%s#0", j, arg2);
					sprintf (example, "%s", row[0]);
					if (!strchr (example, ' '))
						sprintf (example,
						"\nTo pull up the first entry listed, type \'#6help %s %s#0\'.\n",
						LOW (row[1]), LOW (row[0]));
					else
						sprintf (example,
						"\nTo pull up the first entry listed, type \'#6help %s \"%s\"#0\'.\n",
						LOW (row[1]), row[0]);
				}
				else
					sprintf (arg1, "\n   %3d. #6%s#0", j, arg2);
			}
			else
				sprintf (arg1, "#6%-22.22s#0   ", arg2);
			if (!(j % 3) && category_list)
				strcat (arg1, "\n   ");
			j++;
			sprintf (temp_buf + strlen (temp_buf), "%s", arg1);
		}
		if ((((j - 1) % 3) && !header_needed) || !category_list)
			strcat (temp_buf, "\n");
		if (*example)
			sprintf (temp_buf + strlen (temp_buf), "%s", example);
	}

	mysql_free_result (result);
	result = NULL;

	if (*temp_buf)
		page_string (ch->desc, temp_buf);
	else
	{
		ch->send_to_char
			("No helpfile matching that query was found in the database.\n\n#6Please see HELP HELP to ensure you are using the proper query syntax.#0\n");
		log_missing_helpfile (ch, original);
		return;
	}
}

void do_users (CHAR_DATA * ch, char *argument, int cmd)
{
	DESCRIPTOR_DATA *d = NULL;
	CHAR_DATA *tch = NULL;
	char colon_char = '\0';
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char output[MAX_STRING_LENGTH] = { '\0' };
	char line[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	char buf3[MAX_STRING_LENGTH] = { '\0' };
	char buf4[MAX_STRING_LENGTH] = { '\0' };

	strcpy (buf, "\n#6Users Currently Connected:\n"
		"#6--------------------------\n\n#0");

	if (maintenance_lock)
		sprintf (buf + strlen (buf),
		"#2We are currently closed for maintenance.\n\n#0");

	if (pending_reboot)
		sprintf (buf + strlen (buf),
		"#2There is currently a reboot pending.\n\n#0");

	ch->send_to_char(buf);

	for (d = descriptor_list; d; d = d->next)
	{

		colon_char = ':';

		*buf2 = '\0';
		*buf3 = '\0';
		*buf4 = '\0';

		if (d->character && !d->character->desc)
			continue;

		if (d->character && d->original)
			tch = d->original;
		else
			tch = d->character;

		if (tch && tch->pc->level)
			colon_char = '*';

		if (tch && d->acct)
		{
			sprintf (line, "%s  %c ", tch->pc->account_name, colon_char);
			sprintf (line + strlen (line), " #2[%s]#0 ", tch->name);
			sprintf (line + strlen (line), "#3[%s]#0 ", d->strClientHostname);
			sprintf (line + strlen (line), "#5[%d]#0", tch->in_room);
			if (tch->pc->create_state == STATE_DIED)
				sprintf (line + strlen (line), " #1(Dead)#0");
			if (tch->pc->create_state == STATE_RETIRED)
				sprintf (line + strlen (line), " #1(Retired)#0");
			if (d->idle)
				sprintf (line + strlen (line), " #4(Idle)#0");
			if (IS_SET (tch->action, PLR_QUIET))
				sprintf (line + strlen (line), " #5(Edit)#0");
			if (IS_SET (tch->plr_flags, NEW_PLAYER_TAG))
				sprintf (line + strlen (line), " #2(New)#0");
			sprintf (line + strlen (line), "\n");
		}

		else
			continue;

		if (strlen (output) + strlen (line) > MAX_STRING_LENGTH)
			break;

		sprintf (output + strlen (output), "%s", line);
	}

	page_string (ch->desc, output);
}

char *worn_first_loc (OBJ_DATA * obj)
{
	if (obj->location == WEAR_NECK_1 || obj->location == WEAR_NECK_2)
		return "around your neck";
	else if (obj->location == WEAR_BODY)
		return "on your body";
	else if (obj->location == WEAR_HEAD)
		return "on your head";
	else if (obj->location == WEAR_ARMS)
		return "on your arms";
	else if (obj->location == WEAR_ABOUT)
		return "about your body";
	else if (obj->location == WEAR_WAIST)
		return "around your waist";
	else if (obj->location == WEAR_WRIST_L)
		return "on your left wrist";
	else if (obj->location == WEAR_WRIST_R)
		return "on your right wrist";
	else if (obj->location == WEAR_ANKLE_L)
		return "on your left ankle";
	else if (obj->location == WEAR_ANKLE_R)
		return "on your right ankle";
	else if (obj->location == WEAR_HAIR)
		return "in your hair";
	else if (obj->location == WEAR_FACE)
		return "on your face";
	else if (obj->location == WEAR_BELT_1 || obj->location == WEAR_BELT_2)
		return "on your belt";
	else if (obj->location == WEAR_BACK)
		return "on your back";
	else if (obj->location == WEAR_THROAT)
		return "around your throat";
	else if (obj->location == WEAR_BLINDFOLD)
		return "as a blindfold";
	else if (obj->location == WEAR_EAR)
		return "on your ear";
	else if (obj->location == WEAR_SHOULDER_R)
		return "over your right shoulder";
	else if (obj->location == WEAR_SHOULDER_L)
		return "over your left shoulder";
	else if (obj->location == WEAR_FEET)
		return "on your feet";
	else if (obj->location == WEAR_FINGER_R)
		return "on your right ring finger";
	else if (obj->location == WEAR_FINGER_L)
		return "on your left ring finger";
	else if (obj->location == WEAR_ARMBAND_R)
		return "around your upper right arm";
	else if (obj->location == WEAR_ARMBAND_L)
		return "around your upper left arm";
	else if (obj->location == WEAR_LEGS)
		return "on your legs";
	else if (obj->location == WEAR_HANDS)
		return "on your hands";

	else
		return "#1in an unknown location#0";
}

void do_equipment (CHAR_DATA * ch, char *argument, int cmd)
{
	int location = 0;
	int found = false;
	OBJ_DATA *eq = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	*buf = '\0';
	if (ch->right_hand)
	{
		if (ch->right_hand->location == WEAR_PRIM
			|| ch->right_hand->location == WEAR_SEC)
			sprintf (buf, "<held in right hand>  ");
		else if (ch->right_hand->location == WEAR_BOTH)
			sprintf (buf, "<held in both hands>  ");
		else
			sprintf (buf, "<carried in right hand>  ");

		ch->send_to_char(buf);
		show_obj_to_char (ch->right_hand, ch, 1, 2);
	}
	if (ch->left_hand)
	{
		if (ch->left_hand->location == WEAR_PRIM
			|| ch->left_hand->location == WEAR_SEC)
			sprintf (buf, "<held in left hand>   ");
		else if (ch->left_hand->location == WEAR_BOTH)
			sprintf (buf, "<held in both hands>  ");
		else
			sprintf (buf, "<carried in left hand>   ");

		ch->send_to_char(buf);
		show_obj_to_char (ch->left_hand, ch, 1, 2);
	}

	if (ch->left_hand || ch->right_hand)
		ch->send_to_char("\n");

		//location = 1 is for "take" not a real wear location
	for (location = 1; location < MAX_WEAR; location++)
	{

		if (!(eq = get_equip (ch, location)))
			continue;

		if (eq == ch->right_hand || eq == ch->left_hand)
			continue;

		ch->send_to_char(where[location]);

		if (location == WEAR_BLINDFOLD || IS_OBJ_VIS (ch, eq))
			show_obj_to_char (eq, ch, 1, 2);
		else
			ch->send_to_char("#2something#0\n");

		found = true;
	}

	if (!found && !IS_SET (ch->flags, FLAG_GUEST))
		ch->send_to_char("You are naked.\n");

}

void do_news (CHAR_DATA * ch, char *argument, int cmd)
{

	std::string msg_line;
	std::string output;

	std::ifstream fin( "MOTD" );

	output.clear();
	
	if( !fin )
	{
		system_log ("The MOTD could not be found", true);
		ch->send_to_char("The MOTD could not be found");
		return;
	}

	while( getline(fin, msg_line) )
	{
		output.append(msg_line);
	}

	fin.close();

	if (!output.empty())
	{
		ch->send_to_char(output.c_str());
		ch->send_to_char("\n");
	}

}

void do_wizlist (CHAR_DATA * ch, char *argument, int cmd)
{
	page_string (ch->desc, get_text_buffer (ch, text_list, "wizlist"));
}

std::string show_where_char (const CHAR_DATA * ch, int indent, std::string long_arg)
{
	char buf[MAX_STRING_LENGTH];
	
	if (!ch || !ch->room || ch->in_room == NOWHERE || !ch->short_descr
		|| !ch->name)
		return 0;

	memset (buf, ' ', indent * 3);
	buf[indent * 3] = '\0';

	sprintf (buf + strlen (buf), "%s",
		IS_NPC (ch) ? ch->short_descr : ch->name);

	if (IS_NPC (ch))
		sprintf (buf + strlen (buf), " (%d)", ch->mob->nVirtual);

	if (ch->in_room == -1)
		strcat (buf, " NOWHERE\n");
	else
		sprintf (buf + strlen (buf), " in room %d\n", ch->in_room);

	long_arg.append(buf);

	return (long_arg);
}

std::string show_where_obj (OBJ_DATA * obj, int indent, std::string long_arg)
{
	char buf[MAX_STRING_LENGTH];
	std::string tbuf;
	
	if (!obj)
		return (long_arg);

	memset (buf, ' ', indent * 3);
	buf[indent * 3] = '\0';

	sprintf (buf + strlen (buf), "%s (%d)",
		obj->short_description, obj->nVirtual);

	if (obj->in_room != NOWHERE)
		sprintf (buf + strlen (buf), " in room %d\n", obj->in_room);
	else if (obj->in_obj)
		sprintf (buf + strlen (buf), " in obj:\n");
	else if (obj->carried_by && obj->location == -1
		&& obj->carried_by->room != NULL)
		sprintf (buf + strlen (buf), " carried by:\n");
	else if (obj->equiped_by && obj->equiped_by->room != NULL)
		sprintf (buf + strlen (buf), " equipped by:\n");
	else
		sprintf (buf + strlen (buf), " nowhere\n");

	long_arg. append(buf);

	
	if (obj->in_obj)
	{
		tbuf = show_where_obj (obj->in_obj, (indent + 1), "");
	}
	else if (obj->equiped_by)
	{
		tbuf = show_where_obj (obj->in_obj, (indent + 1), "");
	}
	else if (obj->carried_by)
	{
		tbuf = show_where_char (obj->carried_by, (indent + 1), "");
	}
	
	long_arg. append(tbuf.c_str());

	return (long_arg);
}

void do_find (CHAR_DATA * ch, char *argument, int cmd)
{
	int nots = 0;
	int musts = 0;
	int zone = -1;
	int nVirtual = -1;
	int type = -1;
	int i;
	OBJ_DATA *obj = NULL;
	char *not_list[AVG_STRING_LENGTH] = { '\0' };
	char *must_list[AVG_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };
	std::string output;
	std::list<OBJ_DATA*>::iterator tobj_iterator;

	
	output.assign("");
	
	argument = one_argument (argument, buf);

	if (!*buf || *buf == '?')
	{
		ch->send_to_char("find +keyword     'keyword' must exist on obj\n");
		ch->send_to_char("     -keyword     'keyword' must not exist on obj.\n");
		ch->send_to_char("     zone         Only consider objects in this zone.\n");
		ch->send_to_char("     vnum         Just find the specific object vnum. (vnum must be greater than 100)\n");
		ch->send_to_char("\nExamples:\n\n");
		ch->send_to_char("   > find +bag +leather 10\n");
		ch->send_to_char("   > find 10001\n");
		ch->send_to_char("   > find +sword -rusty 12\n");
		return;
	}

	while (*buf)
	{

		if (*buf == '+')
			must_list[musts++] = duplicateString (buf + 1);

		else if (*buf == '-')
			not_list[nots++] = duplicateString (buf + 1);

		else if (isdigit (*buf))
		{
			if (atoi (buf) < 100)
				zone = atoi (buf);
			else
				nVirtual = atoi (buf);
		}

		else if ((type = index_lookup (item_types, buf)) != -1)
			;

		else
		{
			ch->send_to_char("Unknown keyword to where obj.\n");
			return;
		}

		argument = one_argument (argument, buf);
	}

	for (tobj_iterator = object_list.begin(); tobj_iterator != object_list.end(); tobj_iterator++)
	{
		obj = *tobj_iterator;

		if (obj->deleted)
			continue;

		for (i = 0; i < musts; i++)
			if (!isname (must_list[i], obj->name))
				break;

		if (i != musts)		/* Got out of loop w/o all musts being there */
			continue;

		for (i = 0; i < nots; i++)
			if (isname (must_list[i], obj->name))
				break;

		if (i != nots)		/* Got out of loop w/o all nots not being thr */
			continue;

		if (nVirtual != -1 && obj->nVirtual != nVirtual)
			continue;

		if (type != -1 && obj->obj_flags.type_flag != type)
			continue;

		/* Zone is a little tricky to determine */
		output.append(show_where_obj (obj, 0, ""));
		
		if (output.size()== 0)
			break;
	}

	page_string (ch->desc, output.c_str());
}

void do_locate (CHAR_DATA * ch, char *argument, int cmd)
{
	int num_clans = 0;
	int i;
	int musts = 0;
	int nots = 0;
	bitflag act_bits = 0;
	int position = -1;
	int ind = 0;
	int nVirtual = 0;
	int zone = -1;
	int pc_only = 0;
	CHAR_DATA *mob = NULL;
	char *arg = '\0';
	char clan_names[10][AVG_STRING_LENGTH] = { {'\0'}, {'\0'} };
	char not_list[50][AVG_STRING_LENGTH] = { {'\0'}, {'\0'} };
	char must_list[50][AVG_STRING_LENGTH] = { {'\0'}, {'\0'} };
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char acts[MAX_STRING_LENGTH] = { '\0' };
	std::string temp_buf;
	std::list<CHAR_DATA*>::iterator tch_iterator;

	argument = one_argument (argument, buf);

	if (!*buf || *buf == '?')
	{
		ch->send_to_char("locate [+/-mob name]\n"
			"       [zone]\n"
			"       [virtual]\n"
			"       act \"<action-bits>\"\n"
			"       [clan name]\n"
			"       clan <clan name>\n" "       [position]\n");
		return;
	}

	while (*buf)
	{

		if (*buf == '+')
			strcpy (must_list[musts++], buf + 1);

		else if (*buf == '-')
			strcpy (not_list[nots++], buf + 1);

		else if ((position = index_lookup (position_types, buf)) != -1)
			;

		else if (isdigit (*buf))
		{
			ind = atoi (buf);
			if (ind > 100)
				nVirtual = ind;
			else
				zone = ind;
		}

		else if (strcasecmp (buf, "act") == STR_MATCH)
		{

			argument = one_argument (argument, acts);

			arg = one_argument (acts, buf);

			while (*buf)
			{

				if ((ind = index_lookup (action_bits, buf)) == -1)
				{
					ch->send_to_char("No such action-bit:  ");
					ch->send_to_char(buf);
					ch->send_to_char("\n");
					return;
				}

				act_bits |= (1 << ind);

				arg = one_argument (arg, buf);
			}
		}

		else if (strcasecmp (buf, "clan") == STR_MATCH)
		{

			argument = one_argument (argument, buf);

			if ( !str_cmp (buf, "pc") ){
				argument = one_argument (argument, buf);
				pc_only++;
			}

			if (num_clans >= 9)
			{
				ch->send_to_char("Hey, 10 clans is enough!\n");
				return;
			}

			strcpy (clan_names[num_clans++], buf);
		}

		else if (isalpha (*buf))
		{

			if (num_clans >= 9)
			{
				ch->send_to_char("Hey, 10 clans is enough!\n");
				return;
			}

			strcpy (clan_names[num_clans++], buf);
		}

		else
		{
			ch->send_to_char("Unknown keyword:  ");
			ch->send_to_char(buf);
			ch->send_to_char("\n");
			return;
		}

		argument = one_argument (argument, buf);
	}


	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		mob = *tch_iterator;

		if (mob->deleted)
			continue;

		
		if (IS_NPC(mob) && pc_only)
			continue;

		if (zone != -1 && mob->room->zone != zone)
			continue;

		if (nVirtual && (!mob->mob || mob->mob->nVirtual != nVirtual))
			continue;

		if (position != -1 && mob->get_position() != position)
			continue;

		if (act_bits && (act_bits & mob->mob->action) != act_bits)
			continue;

		for (i = 0; i < num_clans; i++)
			if (is_clan_member (mob, clan_names[i]))
				break;

		if (num_clans && i >= num_clans)	/* Couldn't find a clan member */
			continue;

		for (i = 0; i < musts; i++)
			if (!isname (must_list[i], mob->keywords))
				break;

		if (i != musts)		/* Got out of loop w/o all musts being there */
			continue;

		for (i = 0; i < nots; i++)
			if (isname (not_list[i], mob->keywords))
				break;

		if (i != nots)		/* Got out of loop w/o all nots not being thr */
			continue;

		temp_buf.append(show_where_char (mob, 0, temp_buf));
		
		
	}

	page_string (ch->desc, temp_buf.c_str());
}

void do_where (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	int rpp;
	std::multimap<int, std::string> whereMap;
	std::multimap<int, std::string>::iterator it;
	std::stringstream outputWhere;
	CHAR_DATA *target;
	std::list<char_data*>::iterator tch_iterator;
	
	
	if ((!ch->get_trust())) {
		ch->send_to_char("What are you looking for?\n");
		return;
	}
	
	
	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		target = *tch_iterator;
		
		if (target->deleted)
			continue;
		
		if (IS_NPC (target) && (!target->desc))
			continue;
		
		if (target->in_room!=NOWHERE)
		{
			if (!IS_NPC (target) || (!target->desc))
				rpp = 0;
			else
				rpp = target->desc->acct->get_rpp ();
			
			std::pair<int, std::string> reportPair = target->reportWhere(rpp);
			
			whereMap.insert(reportPair);
		}
	}	
	
	sprintf(buf, "\n\n");
	
		// Inorder traversal, with the key being the room number, puts it into room order
	for (it = whereMap.begin(); it != whereMap.end(); it++)
	{
		if (it->second.size() > 80)
		{
			it->second = it->second.substr (0, it->second.find_last_of (" ", 80));
		}
		sprintf(buf + strlen(buf), "%s\n", it->second.c_str());
	}
	
		// Append the remainder of the where information
	sprintf(buf + strlen(buf),"\n\n#0Color key: #5Admin,#0 #6Admin PC,#0 #9P-Builder,#0 #3Guide,#0 #2New PC,#0 #1In Melee,#0 #4In Water#0\n");
	
	sprintf(buf + strlen(buf),"\n\n#0Flag key:\n");
	sprintf(buf + strlen(buf),"e - Editing, i - Idle, h - Hidden, s - Sleeping");
	sprintf(buf + strlen(buf),"\nL - Link Dead, U - Unconscious, X - Near Death\n");
	page_string (ch->desc, buf);
}

void roomCount(ROOM_DATA *rd)
{
	if (rd == NULL) {
		return;
	}
	rd->occupants = 0;
	for (CHAR_DATA *tch = rd->people; tch; tch = tch->next_in_room)
	{
		if (!IS_NPC(tch))
		{
			if (tch->desc 
				&& (!tch->get_trust()) 
				&& !get_affect(tch, MAGIC_HIDDEN))
				rd->occupants++;
		}
		else if (tch->desc 
				 && tch->desc->original 
				 && !get_affect(tch, MAGIC_HIDDEN))
			rd->occupants++;
	}
}

std::string gatheringPlace(int room_num, std::string name)
{
	return gatheringPlaceCore(room_num, name, true);
}

std::string gatheringPlaceCore(int room_num, std::string name, bool colorise)
{
	std::stringstream placestream;

	ROOM_DATA* rd = vtor(room_num);
	roomCount(rd);

	int num_occu = rd->occupants;

	if (num_occu > 0)
	{
		placestream << "In " << name << ", there " << (num_occu == 1 ? "is " : "are ");

		/* color codes are ignored entirely when colorise is false, for web export */
		if (colorise)
			placestream << "#2";

		placestream << num_occu;

		/* color codes are ignored entirely when colorise is false, for web export */
		if (colorise)
			placestream << "#0";

		placestream <<  (num_occu == 1 ? " player." : " players.") << std::endl;
	}

	return placestream.str();

}

void do_who (CHAR_DATA * ch, char *argument, int cmd)
{
	DESCRIPTOR_DATA *d = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	int mortals = 0;
	int immortals = 0;
	int guests = 0;
	std::stringstream buffer2;
	std::string output_buffer;
	std::string admin_buffer;
	bool avail_admins = false;
	
	admin_buffer.clear();
	admin_buffer.append("#2Available Staff#0: ");
	
	
	for (d = descriptor_list; d; d = d->next)
	{
		if (d->character && d->character->pc && d->connected == CON_PLYNG)
		{

				//TODO: summation out by clans when we need them. For now, just worry about mortal/guest
			 if (IS_SET (d->character->flags, FLAG_GUEST))
				 guests++;
			else
				mortals++;

			
				//immortals are not counted unless they are available
			if(d->character->pc->level > 0)
			{
				if(IS_SET( d->character->flags, FLAG_AVAILABLE ))
				{
					immortals++;
					admin_buffer.append(d->character->name);
					admin_buffer.append(" ");
					avail_admins = true;
				}
			}
		}
	}

	output_buffer.clear();
	
	if (mortals < 1)
		output_buffer.append("There are no beings within Middle-earth.\n");
	
	else if (mortals == 1)
		output_buffer.append("There is a single presence within Middle-earth.\n");
	else 
	{
		buffer2.sync();
		buffer2 << "There are " << mortals << " presences within Middle-earth.\n";
		output_buffer.append(buffer2.str().c_str());
	}

		//TODO: display populations counts by clans here.
	
		//Display population count of guest lounge
	if (guests > 1)
	{
		sprintf(buf, "Additionally, there are %d guests visiting our out-of-character Guest Lounge.\n", guests); 
	}
	else if (guests == 1)
	{
		sprintf(buf, "Additionally, there is 1 guest visiting our out-of-character Guest Lounge. \n");
	}
	if (guests)
		output_buffer.append(buf);
	
	sprintf(buf, "Our record is %d players, last seen on %s.\n", count_max_online, max_online_date);
	output_buffer.append(buf);
	
		//display available admins
	if (!avail_admins)
	{
		admin_buffer.append(" None\n");
	}

	output_buffer.append(admin_buffer.c_str());
	
	ch->send_to_char(output_buffer.c_str());
}

	//Scan should work by distance, not by room count
void do_scan (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir = 0;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	ROOM_EXIT_DATA *exit = NULL;
	ROOM_DATA *next_room = NULL;
	ROOM_DATA *temp_room = NULL;

	argument = one_argument (argument, buf);

	if (ch->skill_map["Scan"] < 1)
	{
		ch->send_to_char("You cannot focus well enough.\n");
		return;
	}

	if (is_sunlight_restricted (ch))
		return;

	if (IS_SET (ch->room->room_flags, STIFLING_FOG)
		&& (!get_affect (ch, MAGIC_AFFECT_INFRAVISION)
		&& !IS_SET (ch->affected_by, AFF_INFRAVIS)))
	{
		ch->send_to_char("The thick fog in the area prevents any such attempt.\n");
		return;
	}

	
	if ((!ch->get_trust())
		&& weather_info[ch->room->wzone].state == HEAVY_SNOW
		&& !IS_SET (ch->room->room_flags, INDOORS)
		&& (!get_affect (ch, MAGIC_AFFECT_INFRAVISION)
		&& !IS_SET (ch->affected_by, AFF_INFRAVIS)))
	{
		ch->act("The onslaught of blowing snow severely inhibits your scanning.", false, 0, 0, TO_CHAR);
		return;
	}


		// fast small area scan
	if (!*buf)
	{	
			//cmd ==1 for qscan
			//cmd == 0 for scan all
		if (cmd == 1)
		{
			ch->delay_type = DEL_QUICK_SCAN;
			ch->delay = 1;
		}
		else
		{
			ch->delay_type = DEL_SCAN_ALL;
			ch->delay = SCAN_DELAY;
		}
	} 

	else 
	{
		dir = index_lookup (dirs, buf);
		if (dir == -1)
		{
				//if it is not a direction, it could be a key word
			scan_portal_key(ch, buf);
			return;
		}
		
		exit = is_exit(ch, dir);
		
		if (exit)
		{
			if ((IS_SET (exit->port_flags, EX_ISDOOR) && IS_SET (exit->port_flags, EX_CLOSED))
				|| (IS_SET (exit->port_flags, EX_ISGATE) && IS_SET (exit->port_flags, EX_CLOSED))
				|| (exit->type == PORTAL_BARRIER)
				|| ((exit->type == PORTAL_WINDOW) && IS_SET (exit->port_flags, EX_CLOSED)))
			{
				ch->send_to_char("There's nothing to scan that way.\n");
				return;
			}
		}
		else 
		{
			temp_room = get_diagonal_move_room (ch, dir);
			
			if (!temp_room)
			{
				ch->send_to_char("There's nothing to scan that way.\n");
				return;
				
			}
			else
			{
				exit = new room_exit_data;
				exit->to_room = temp_room->nVirtual;
			}
		}
		
		if (!(next_room = vtor (exit->to_room)))
		{
			ch->send_to_char("There's nothing to scan that way.\n");
			return;
		}
		
		ch->delay_type = DEL_SCAN;
		ch->delay = SCAN_DELAY;
		ch->delay_info1 = dir;
	}
}

bool char__room_scan (CHAR_DATA * ch, ROOM_DATA * next_room, const char *ptrStrDir,
				 int nSkillLevel, bool bSaw, float curr_dist)
{
	int tlight;
	int dir;
	CHAR_DATA *tch = NULL;
	OBJ_DATA *tobj = NULL;
	char outbuf[AVG_STRING_LENGTH] = "";
	char tempbuf[AVG_STRING_LENGTH] = "";
	char leadbuf[AVG_STRING_LENGTH] = "";
	bool bSeen = false;
	bool room_seen = false;
	
	dir = ch->delay_info1;
	
	
	if (curr_dist == QUICK_SCAN_DIST)
		sprintf(leadbuf, "To the %s,", dirs[dir]);
	else 
		sprintf(leadbuf, "About %d yards to the %s,", estimate_distance(ch,curr_dist), dirs[dir]);
	

	if (ch->room->light <= next_room->light)
		sprintf(tempbuf, "%s you see:\n", leadbuf);
	else
		sprintf(tempbuf, "%s where it is darker, you see:\n", leadbuf);
	
	for (tch = next_room->people; tch; tch = tch->next_in_room)
	{

		if (!could_see (ch, tch))
			continue;

		if (tch == ch)
			continue;

		if (nSkillLevel < number (1, SKILL_CEILING)
			&& !has_been_sighted (ch, tch))
			continue;

		bSaw = true;
		bSeen = true;
		room_seen = true;
		sprintf (tempbuf + strlen(tempbuf), "#5%s#0\n", tch->char_long());
		target_sighted (ch, tch);		
	}
	
	if (bSeen)
	{
		ch->send_to_char(tempbuf);
		sprintf(tempbuf, "\n");
	}
	
	bSeen = false;
	for (tobj = next_room->contents; tobj; tobj = tobj->next_content)
	{
		tlight = could_see_obj (ch, tobj);
		
			//too dark to see the object
		if (tlight == 0)
			continue;

		if (nSkillLevel < number (1, SKILL_CEILING))
			continue;
		
		bSaw = true;
		bSeen = true;
		room_seen = true;
		
		ch->send_to_char(tempbuf);
		show_obj_to_char (tobj, ch, 7, could_see_obj (ch, tobj));
		ch->delay_info2 = 1;
		
	}

	
	if (room_seen)
	{
		sprintf(outbuf, "That area is ");
		
		if (next_room->room_size == ROOM_SIZE_DETAIL)
			sprintf(outbuf + strlen(outbuf), "small.\n\n");
		else if (next_room->room_size == ROOM_SIZE_EXPLORE)
			sprintf(outbuf + strlen(outbuf), "moderately sized.\n\n");
		else if (next_room->room_size == ROOM_SIZE_VALLEY)
			sprintf(outbuf + strlen(outbuf), "vast.\n\n");
		else if (next_room->room_size == ROOM_SIZE_STORAGE)
			sprintf(outbuf + strlen(outbuf), "cramped.\n\n");
		ch->send_to_char(outbuf);
	}
	
	return bSaw;
}

ROOM_DATA *get_diagonal_scan_room (CHAR_DATA * ch, int nDirOne, int nDirTwo)
{
	ROOM_EXIT_DATA *ptrTmpExit = NULL;
	ROOM_DATA *ptrTmpRoom = NULL;

	if ((ptrTmpExit = is_exit(ch, nDirOne))
		&& (!IS_SET (ptrTmpExit->port_flags, EX_ISDOOR))
		&& (ptrTmpRoom = vtor (ptrTmpExit->to_room))
		&& (!IS_SET (ptrTmpRoom->room_flags, INDOORS))
		&& (ptrTmpRoom->terrain_type != SECT_URBAN)
		&& (ptrTmpRoom->terrain_type != SECT_CAVE)
		&& (ptrTmpRoom->terrain_type != SECT_UNDERWATER)
		&& !(IS_SET (ptrTmpRoom->room_flags, STIFLING_FOG)
			 && (!get_affect (ch, MAGIC_AFFECT_INFRAVISION)
				 && !IS_SET (ch->affected_by, AFF_INFRAVIS)))
		&& (ptrTmpExit = ptrTmpRoom->dir_option[nDirTwo])
		&& (!IS_SET (ptrTmpExit->port_flags, EX_ISDOOR))
		&& (ptrTmpRoom = vtor (ptrTmpExit->to_room))
		&& (!IS_SET (ptrTmpRoom->room_flags, INDOORS))
		&& (ptrTmpRoom->terrain_type != SECT_URBAN)
		&& (ptrTmpRoom->terrain_type != SECT_CAVE)
		&& (ptrTmpRoom->terrain_type != SECT_UNDERWATER)
		&& !(IS_SET (ptrTmpRoom->room_flags, STIFLING_FOG)
			 && (!get_affect (ch, MAGIC_AFFECT_INFRAVISION)
				 && !IS_SET (ch->affected_by, AFF_INFRAVIS))))
	{

		return ptrTmpRoom;

	}

	return NULL;
}

/*
 * 
 *	A---B	We are at room 'N' and we want to go to room 'B'
 *	|    Y  Is 'N'-'A' or 'A'-'B' blocked? If not, we can move into 'B'
 *	|    |	Is 'N'-'X' or 'X'-'B' is blocked? If not, we move into 'Y'
 *	N----X	It doesn't matter if 'B' is a different room from 'Y'
 *			We are moving in that direction and may have wandered a little ways
 *
 */
ROOM_DATA *get_diagonal_move_room (CHAR_DATA * ch, int targ_dir)
{
	ROOM_DATA *ptrTmpRoomA = NULL;
	ROOM_DATA *ptrTmpRoomB = NULL;
	ROOM_DATA *ptrTmpRoomX = NULL;
	ROOM_DATA *ptrTmpRoomY = NULL;
	int dir_one = -1;
	int dir_two = -1;
	
	switch (targ_dir)
	{
		case NORTHEAST:
			dir_one = NORTH;
			dir_two = EAST;
			break;
		case SOUTHEAST:
			dir_one = SOUTH;
			dir_two = EAST;
			break;
		case SOUTHWEST:
			dir_one = SOUTH;
			dir_two = WEST;
			break;
		case NORTHWEST:
			dir_one = NORTH;
			dir_two = WEST;
			break;
		default:
			break;
	}
		

	
	if ((dir_one >= 0) && (can_go_exit(ch, dir_one)))
	{
		ptrTmpRoomA = vtor(ch->room->dir_option[dir_one]->to_room);

		if (ptrTmpRoomA
			&& ptrTmpRoomA->dir_option[dir_two]
			&& ptrTmpRoomA->dir_option[dir_two]->to_room != NOWHERE
			&& !IS_SET(ptrTmpRoomA->dir_option[dir_two]->port_flags, EX_CLOSED))
		{
			ptrTmpRoomB = vtor(ptrTmpRoomA->dir_option[dir_two]->to_room);
			if (ptrTmpRoomB)
				return (ptrTmpRoomB);
			else 
				return (NULL);

		}
	}	
	else if ((dir_two >= 0) && (can_go_exit(ch, dir_two)))
	{
		ptrTmpRoomX = vtor(ch->room->dir_option[dir_two]->to_room);
		if (ptrTmpRoomX
			&& ptrTmpRoomX->dir_option[dir_one]
			&& ptrTmpRoomX->dir_option[dir_one]->to_room != NOWHERE
			&& !IS_SET(ptrTmpRoomX->dir_option[dir_one]->port_flags, EX_CLOSED))
		{
			ptrTmpRoomY = vtor(ptrTmpRoomX->dir_option[dir_one]->to_room);
			if (ptrTmpRoomY)
				return (ptrTmpRoomY);
			else 
				return (NULL);
		}
	}

	else 
		return (NULL);
	
	return (NULL);
}



void do_qscan (CHAR_DATA * ch, char *argument, int cmd)
{
	do_scan (ch, "", 1);
	return;
}

void do_count (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	OBJ_DATA *obj = NULL;

	argument = one_argument (argument, buf);

	if (is_dark (ch->room) && !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
		&& (!ch->get_trust()) && !IS_SET (ch->affected_by, AFF_INFRAVIS))
	{
		ch->send_to_char("It's too dark to count coins.\n");
		return;
	}

	if (!*buf)
	{
		sprintf (buf, "You begin searching through your belongings, taking a tally of your coin.\n");
		ch->send_to_char(buf);
	}
	else
	{
		if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
			!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
		{
			ch->send_to_char("I don't see that group of coins.\n");
			return;
		}

		if (obj->obj_flags.type_flag != ITEM_MONEY)
		{
			ch->send_to_char("That isn't a group of coins.\n");
			return;
		}

		else if ((obj->obj_flags.type_flag == ITEM_MONEY)
				 && ((obj->nVirtual != GONDOR_1)
					 ||(obj->nVirtual != GONDOR_5)
					 ||(obj->nVirtual != GONDOR_50)
					 ||(obj->nVirtual != GONDOR_200)
					 ||(obj->nVirtual != GONDOR_K)
					 ||(obj->nVirtual != GONDOR_10K)))
		{
			ch->send_to_char("This may be money, but you don't know the value.\n");
			return;
			
		}
		sprintf (buf, "After a moment of sorting, you determine that there are %d"
			" coins in the pile.", obj->count);
		ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}
	ch->delay = 10;
	ch->delay_type = DEL_COUNT_COIN;
}

char* coin_sdesc( const OBJ_DATA * coin )
{
	std::string coindesc;
	
	switch (coin->nVirtual)
	{
		case GONDOR_1:
			coindesc.assign("semicircular copper coin");
			break;
			
		case GONDOR_5:
			coindesc.assign("large, rounded bronze coin");
			break;
			
		case GONDOR_50:
			coindesc.assign("thin, ridged silver coin");
			break;
			
		case GONDOR_200:
			coindesc.assign("heavy, oblong silver coin");
			break;
			
		case GONDOR_K:
			coindesc.assign("thick, hexagonal gold coin");
			break;
			
		case GONDOR_10K:
			coindesc.assign("thin, slightly fluted gold coin");
			break;
			
		default:
			coindesc.assign("semicircular copper coin");
			break;
	}
	
		//pluralize it
	if (coin->count > 1)
		coindesc.append("s");

	
	return (char*)coindesc.c_str();
}





void tracking_system_response (CHAR_DATA * ch, MESSAGE_DATA * message)
{
	CHAR_DATA *tch = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char from[MAX_STRING_LENGTH] = { '\0' };
	char subject[AVG_STRING_LENGTH] = { '\0' };
	char type[AVG_STRING_LENGTH] = { '\0' };

	if (!message)
		return;

	if (!ch->desc->pending_message->message || !*ch->desc->pending_message->message)
		return;

	account acct (message->poster);
	if (!acct.is_registered ())
	{
		if (!(tch = load_pc (message->poster)))
			return;
		else
			acct = account (tch->pc->account_name);
	}

	if (!acct.is_registered ())
	{
		if (tch != NULL)
			tch->unload_pc();
		return;
	}

	if (strcasecmp (ch->delay_who, "Bugs") == STR_MATCH)
		sprintf (type, "Bug");
	else if (strcasecmp (ch->delay_who, "Typos") == STR_MATCH)
		sprintf (type, "Typo");
	else if (strcasecmp (ch->delay_who, "Ideas") == STR_MATCH)
		sprintf (type, "Idea");
	else if (strcasecmp (ch->delay_who, "Submissions") == STR_MATCH)
		sprintf (type, "Writing Submission");
	else if (strcasecmp (ch->delay_who, "Petitions") == STR_MATCH)
		sprintf (type, "Petition");
	else
		sprintf (type, "Issue");

	sprintf (buf, "%s"
		"\n\n-- Original Report, Filed %s --\n\n"
		"%s", ch->desc->pending_message->message, message->date,
		message->message);

	sprintf (from, "%s <%s>", ch->name, ch->desc->acct->email.c_str ());

	if (strcasecmp (type, "Petition") == STR_MATCH)
	{
		sprintf (subject, "Re: Your Logged Petition");
		send_email (&acct, PET_EMAIL, from, subject, buf);
	}
	else
	{
		sprintf (subject, "Re: Your %s Report", type);
		send_email (&acct, REPORT_EMAIL, from, subject, buf);
	}

	tch->unload_pc();
}

void post_track_response (DESCRIPTOR_DATA * d)
{
	MESSAGE_DATA *message = NULL;
	MYSQL_RES *result = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	*buf = '\0';
	
	if (!strncasecmp(d->character->delay_who, "Typos",5))
		mysql_safe_query
		("UPDATE newsletter_stats SET resolved_typos=resolved_typos+1");
	else if (strcasecmp (d->character->delay_who, "Bugs") == STR_MATCH)
		mysql_safe_query
		("UPDATE newsletter_stats SET resolved_bugs=resolved_bugs+1");

	message =
		load_message (d->character->delay_who, 5, d->character->delay_info1);
	if (!d->pending_message->message)
	{
		d->character->send_to_char("No email sent.\n");
	}
	else
		d->character->send_to_char("The report poster has been sent an email notification.\n");
	tracking_system_response (d->character, message);

	mysql_safe_query
		("DELETE FROM virtual_boards WHERE post_number = \'%d\' AND board_name = \'%s\'",
		d->character->delay_info1, d->character->delay_who);

	if (mysql_safe_query
		("SELECT * FROM virtual_boards WHERE board_name = \'%s\'",
		d->character->delay_who))
		return;
	result = mysql_store_result (database);
	if (mysql_num_rows (result) == 0)
	{
		sprintf (buf, "vboards/%s", d->character->delay_who);
		unlink (buf);
	}

	d->character->delay_who = NULL;
	d->character->delay_info1 = 0;

	mysql_free_result (result);
	result = NULL;

	free_mem(message);
}

int erase_pc_board (CHAR_DATA * ch, char *name, char *argument)
{
	FILE *fp = NULL;
	unsigned int i = 0;
	CHAR_DATA *who = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	if (strlen (name) > 15)
		return 0;

	for (i = 0; i <= strlen (name); i++)
		if (!isalpha (*name))
			return 0;

	*name = toupper (*name);

	if (!(who = load_pc (name)))
		return 0;

	if (who->pc->level > ch->pc->level)
	{
		who->unload_pc();
		return 0;
	}

	who->unload_pc();

	if (!isdigit (*argument))
		return 0;

	if (atoi (argument) > 255)
		return 0;

	sprintf (buf, "player_boards/%s.%06d", name, atoi (argument));
	if (!(fp = fopen (buf, "r")))
	{
		return 0;
	}
	else
		fclose (fp);

	sprintf (buf, "rm player_boards/%s.%06d", name, atoi (argument));
	system (buf);

	return 1;
}

int erase_journal (CHAR_DATA * ch, char *argument)
{
	FILE *fp = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	if (!isdigit (*argument))
	{
		ch->send_to_char("The specified journal entry must be a number.\n");
		return 1;
	}

	if (atoi (argument) > 255)
	{
		ch->send_to_char("Enter a number between 1 and 255 to erase.\n");
		return 1;
	}

	sprintf (buf, "player_journals/%s.%06d", ch->name, atoi (argument));
	if (!(fp = fopen (buf, "r")))
	{
		ch->send_to_char("That journal entry could not be found.\n");
		return 1;
	}
	else
		fclose (fp);

	sprintf (buf, "rm player_journals/%s.%06d", ch->name, atoi (argument));
	system (buf);

	ch->send_to_char("The specified journal entry has been erased.\n");

	return 1;
}

void do_jerase (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };

	argument = one_argument (argument, buf);

	if (IS_SET (ch->flags, FLAG_GUEST))
	{
		ch->send_to_char("Journals are unavailable to guests.\n");
		return;
	}

	if (!*buf)
	{
		ch->send_to_char("Which message do you wish to erase?\n");
		return;
	}

	if (!erase_mysql_board_post (ch, ch->name, 3, buf))
	{
		ch->send_to_char("I couldn't find that journal entry.\n");
		return;
	}

	ch->send_to_char("Journal entry erased successfully.\n");
}

void do_erase (CHAR_DATA * ch, char *argument, int cmd)
{
	int msg_no = 0;
	MESSAGE_DATA *message = NULL;
	OBJ_DATA *obj = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char("Which message do you wish to erase?\n");
		return;
	}

	if (IS_NPC (ch))
	{
		ch->send_to_char("You can't do this while switched.\n");
		return;
	}

	if (!isdigit (*buf))
	{
		if (!(obj = get_obj_in_list_vis (ch, buf, ch->room->contents)) &&
			!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
			!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
		{
			if ((!ch->get_trust())
				|| (ch->get_trust() == 1)
				|| !erase_mysql_board_post (ch, buf, 2, argument))
			{
				if ((!ch->get_trust())
					|| (ch->get_trust() == 1)
					|| !erase_mysql_board_post (ch, buf, 1, argument))
				{
					if ((!ch->get_trust()) || (ch->get_trust() == 1))
						ch->send_to_char("You do not see that board here.\n");
					else
						ch->send_to_char
						("Either the specified board is not here, or the specified message does not exist.\n");
					return;
				}
				else
				{
					ch->send_to_char("The specified message has been erased.\n");
					return;
				}
			}
			else
			{
				ch->send_to_char("The specified message has been erased.\n");
				return;
			}
		}

		if (obj->obj_flags.type_flag != ITEM_BOARD)
		{
			ch->act("$p isn't a board.", false, obj, 0, TO_CHAR);
			return;
		}

		one_argument (obj->name, buf2);
		argument = one_argument (argument, buf);
	}

	else
	{
		if (!(obj = get_obj_in_list_vis (ch, "board", ch->room->contents)) &&
			!(obj = get_obj_in_list_vis (ch, "board", ch->left_hand)) &&
			!(obj = get_obj_in_list_vis (ch, "board", ch->right_hand)))
		{
			ch->send_to_char("You do not see that board here.\n");
			return;
		}

		if (obj->obj_flags.type_flag != ITEM_BOARD)
		{
			ch->act("$p, the first board here, isn't a real board.",
				false, obj, 0, TO_CHAR);
			return;
		}

		one_argument (obj->name, buf2);
	}

	if (!isdigit (*buf))
	{
		ch->send_to_char("Expected a message number to erase.\n");
		return;
	}

	msg_no = atoi (buf);

	if (!(message = load_message (buf2, 6, msg_no)))
	{
		ch->send_to_char("That message doesn't exist.\n");
		return;
	}

	if ((ch->get_trust() < 1)
		&& strcasecmp (ch->name, message->poster) != STR_MATCH)
	{
		ch->send_to_char("You can only erase your own messages.\n");
		free_mem(message);
		return;
	}

	if (erase_mysql_board_post (ch, buf2, 0, buf))
		ch->send_to_char("The specified message has been erased.\n");
	else
		ch->send_to_char("There was a problem erasing that message.\n");

	free_mem(message);
}

int write_virtual_board (CHAR_DATA * ch, char *name, char *argument)
{
	if (strlen (name) > 15)
		return 0;

	if (!isalpha (*name))
		return 0;

	*name = toupper (*name);

	if (!*argument)
	{
		ch->send_to_char("Please include a subject for your post.\n");
		return 0;
	}

	while (*argument == ' ')
		argument++;

	free_mem (ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;

	/* We need to borrow the poster slot to save the board name */

	ch->desc->pending_message->poster = duplicateString (name);
	ch->desc->pending_message->message = NULL;
	ch->desc->pending_message->nVirtual = -2;
	ch->desc->pending_message->info = duplicateString ("");
	ch->desc->pending_message->subject = duplicateString (argument);
	ch->desc->pending_message->flags = MF_READ;

	ch->make_quiet();

	ch->send_to_char("Enter your note, terminate with an '@'\n\n");


	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->desc->proc = post_to_mysql_virtual_board;

	return 1;
}

void post_player_message (DESCRIPTOR_DATA * d)
{
	char date[AVG_STRING_LENGTH] = "";
	time_t current_time = 0;

	current_time = time (0);

	ctime_r (&current_time, date);

	/* asctime adds a \n to the end of the date string - remove it */

	if (strlen (date) > 1)
		date[strlen (date) - 1] = '\0';

	if (!d->pending_message->message)
		d->character->send_to_char("No message posted.\n");
	else
		add_message (1,		/* New message */
		d->pending_message->poster,	/* name */
		d->pending_message->nVirtual,	/* virtual */
		(d->character)->name,	/* poster */
		date,		/* date */
		d->pending_message->subject,	/* subject */
		d->pending_message->info,	/* info */
		d->pending_message->message,	/* message */
		d->pending_message->flags);

	free_mem(d->pending_message);

	d->pending_message = NULL;
}

int write_pc_board (CHAR_DATA * ch, char *name, char *argument)
{
	unsigned int i = 0;
	CHAR_DATA *who = NULL;

	if (strlen (name) > 15)
		return 0;

	for (i = 0; i <= strlen (name); i++)
		if (!isalpha (*name))
			return 0;

	*name = toupper (*name);

	if (!(who = load_pc (name)))
		return 0;

	who->unload_pc();

	if (!*argument)
	{
		ch->send_to_char("Please include a subject for your post.\n");
		return 0;
	}

	while (*argument == ' ')
		argument++;

	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;

	/* We need to borrow the poster slot to save the board name */

	ch->desc->pending_message->poster = duplicateString (name);
	ch->desc->pending_message->message = NULL;
	ch->desc->pending_message->nVirtual = -1;
	ch->desc->pending_message->info = duplicateString ("");
	ch->desc->pending_message->subject = duplicateString (argument);
	ch->desc->pending_message->flags = 0;

	ch->make_quiet();

	ch->send_to_char("Enter your note, terminate with an '@'\n\n");

	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->desc->proc = post_to_mysql_player_board;

	return 1;
}

	/// \todo merge with post_player_message
void post_journal (DESCRIPTOR_DATA * d)
{
	char date[AVG_STRING_LENGTH] = "";
	time_t current_time = 0;

	current_time = time (0);

	ctime_r (&current_time, date);

	/* asctime adds a \n to the end of the date string - remove it */

	if (strlen (date) > 1)
		date[strlen (date) - 1] = '\0';

	if (!d->pending_message->message)
		d->character->send_to_char("No journal entry added.\n");
	else
		add_message (5,		/* New message */
		d->pending_message->poster,	/* name */
		-3,		/* virtual */
		(d->character)->name,	/* poster */
		date,		/* date */
		d->pending_message->subject,	/* subject */
		d->pending_message->info,	/* info */
		d->pending_message->message,	/* message */
		d->pending_message->flags);

	free_mem(d->pending_message);

	d->pending_message = NULL;
}

int write_journal (CHAR_DATA * ch, char *argument)
{

	if (!*argument)
	{
		ch->send_to_char("Please include a subject for your post.\n");
		return 0;
	}

	while (*argument == ' ')
		argument++;

	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;

	/* We need to borrow the poster slot to save the board name */

	ch->desc->pending_message->poster = duplicateString (ch->name);
	ch->desc->pending_message->message = NULL;
	ch->desc->pending_message->nVirtual = -1;
	ch->desc->pending_message->info = duplicateString ("");
	ch->desc->pending_message->subject = duplicateString (argument);

	ch->make_quiet();

	ch->send_to_char("Type your journal entry; terminate with an '@'\n\n");

	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->desc->proc = post_to_mysql_journal;

	return 1;
}

void do_write (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *obj = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char title[MAX_STRING_LENGTH] = { '\0' };

	if (IS_NPC (ch))
	{
		ch->send_to_char("Write is only available to PCs.\n");
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char("What did you want to write on?\n");
		return;
	}

	if (!(obj = get_obj_in_list_vis (ch, buf, ch->room->contents)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
	{
		if ((!ch->get_trust()) || !write_pc_board (ch, buf, argument))
		{
			if ((!ch->get_trust()) || !write_virtual_board (ch, buf, argument))
				ch->send_to_char("You can't find that.\n");
		}
		return;
	}

	if (obj->obj_flags.type_flag == ITEM_PARCHMENT ||
		obj->obj_flags.type_flag == ITEM_BOOK)
	{
		do_scribe (ch, buf, 0);
		return;
	}

	if (!*argument)
	{
		ch->send_to_char("What would you like to write about?\n");
		return;
	}

	if (obj->obj_flags.type_flag != ITEM_BOARD)
	{
		ch->act("You can't write on $p.", false, obj, 0, TO_CHAR);
		return;
	}

	while (*argument == ' ')
		argument++;

	/* Get the name of the board */

	one_argument (obj->name, buf);

	strcpy (title, argument);

	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;

	/* We need to borrow the poster slot to save the board name */

	ch->desc->pending_message->poster = duplicateString (buf);
	ch->desc->pending_message->message = NULL;
	ch->desc->pending_message->nVirtual = 0;
	ch->desc->pending_message->info = duplicateString ("");
	ch->desc->pending_message->subject = duplicateString (title);

	ch->desc->pending_message->flags = 0;

	ch->send_to_char
		("\n#6Enter your message below. To terminate, use the '@' character. Please ensure\n"
		"you use proper linebreaks, and that your writing follows the acceptable posting\n"
		"policies for our in-game boards as outlined in HELP POSTING_POLICIES.#0\n\n"
		"1-------10--------20--------30--------40--------50--------60--------70--------80\n");

	ch->make_quiet();

	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->desc->proc = post_message;
}

void do_jwrite (CHAR_DATA * ch, char *argument, int cmd)
{

	if (IS_SET (ch->flags, FLAG_GUEST))
	{
		ch->send_to_char("Journals are unavailable to guests.\n");
		return;
	}

	if (IS_NPC (ch))
	{
		ch->send_to_char("Journals are only available to PCs.\n");
		return;
	}

	if (!*argument)
	{
		ch->send_to_char("What did you wish your subject to be?\n");
		return;
	}

	if (!write_journal (ch, argument))
	{
		ch->send_to_char("There seems to be a problem with your journal.\n");
		return;
	}
}

int read_journal_message (CHAR_DATA * ch, CHAR_DATA * reader, char *argument)
{
	MESSAGE_DATA *message = NULL;
	char name[MAX_STRING_LENGTH] = { '\0' };
	char temp_buf[MAX_STRING_LENGTH];
	
	sprintf (name, "%s", ch->name);

	if (!atoi (argument))
	{
		if (!reader)
			ch->send_to_char("Which entry?\n");
		else
			reader->send_to_char("Which entry?\n");
		return 1;
	}

	if (!(message = load_message (name, 8, atoi (argument))))
	{
		if (!reader)
			ch->send_to_char("No such journal entry.\n");
		else
			reader->send_to_char("No such journal entry.\n");
		return 1;
	}

	sprintf (temp_buf, "#6Date:#0    %s\n"
		"#6Subject:#0 %s\n\n%s", message->date, message->subject,
		message->message);

	if (!reader)
	{
		ch->send_to_char("\n");
		page_string (ch->desc, temp_buf);
	}
	else
	{
		reader->send_to_char("\n");
		page_string (reader->desc, temp_buf);
	}

	free_mem(message);

	return 1;
}

void do_jread (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *who = NULL;
	char name[MAX_STRING_LENGTH] = { '\0' };

	bool loaded_char = false;
	if (IS_SET (ch->flags, FLAG_GUEST))
	{
		ch->send_to_char("Journals are unavailable to guests.\n");
		return;
	}

	if (IS_NPC (ch))
	{
		ch->send_to_char("Journals are only available to PCs.\n");
		return;
	}

	if (ch->get_trust())
	{
		if (!*argument)
			who = ch;
		else if (*argument && isdigit (*argument))
			who = ch;
		else
		{
			argument = one_argument (argument, name);
			*name = toupper (*name);
			if (!(who = load_pc (name)))
			{
				ch->send_to_char("No such PC, I'm afraid.\n");
				return;
			}
			loaded_char = true;
		}
		if (!read_journal_message (who, ch, argument))
		{
			ch->send_to_char("There seems to be a problem with the journal.\n");
			if (loaded_char)
				who->unload_pc();
			return;
		}
	}
	else
	{
		if (!read_journal_message (ch, NULL, argument))
		{
			ch->send_to_char("There seems to be a problem with the journal.\n");
			if (loaded_char)
				who->unload_pc();
			return;
		}
	}
	if (loaded_char)
		who->unload_pc();
}

void post_message (DESCRIPTOR_DATA * d)
{
	if (!d->pending_message->message) {
		d->character->send_to_char("No message posted.\n");
		free_mem(d->pending_message);
		d->pending_message = NULL;
	}
	else {
		post_to_mysql_board (d);
	}
}

void add_board (int level, char *name, char *title)
{
	BOARD_DATA *board = NULL;
	BOARD_DATA *board_entry = NULL;

	/* Make sure this board doesn't already exist */

	if (board_lookup (name))
		return;

	board_entry = new BOARD_DATA;
	board_entry->next = NULL;
	board_entry->level = level;
	board_entry->name = duplicateString (name);
	board_entry->title = duplicateString (title);
	board_entry->next_virtual = 1;

	/* Add board_entry to end of full_board_list */

	if (!full_board_list)
		full_board_list = board_entry;

	else
	{
		board = full_board_list;

		while (board->next)
			board = board->next;

		board->next = board_entry;
	}
}

BOARD_DATA *board_lookup (const char *name)
{
	BOARD_DATA *board = NULL;

	for (board = full_board_list; board; board = board->next)
	{
		if (strcasecmp (board->name, name) == STR_MATCH)
			return board;
	}

	return NULL;
}



void add_message (int new_message, char *name, int nVirtual, const char *poster,
			 char *date, char *subject, char *info, char *message, long flags)
{
	int named = 0, day, i = 0;
	MESSAGE_DATA *msg = NULL;
	BOARD_DATA *board = NULL;
	CHAR_DATA *ch = NULL;
	DIR *dir = NULL;
	FILE *fp = NULL;
	bool found = false;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char msg_file_name[MAX_STRING_LENGTH] = { '\0' };
	char date_buf[MAX_STRING_LENGTH] = { '\0'};
	char *suf = '\0';
	time_t current_time = 0;

	if (!new_message)
	{
		system_log ("Add_mesage() called with new_message = 0", true);
		abort ();
	}

	if (!date)
	{
		/// \todo see who is responsible for deleteing the mem alloced by ctime
		current_time = time (0);
		date = ctime (&current_time);
		if (strlen (date) > 1)
			date[strlen (date) - 1] = '\0';
	}

	/* 1 means put a new message to a board (nVirtual == -1 to pc board */
	/* 2 means update a pc board message */

	if (new_message != 5 && new_message != 2 && nVirtual != -5 && nVirtual != -2
		&& nVirtual != -1 && !(board = board_lookup (name)))
	{
		printf ("No board for message; board:  '%s'.\n", name);
		return;
	}

	msg = new MESSAGE_DATA;

	if (nVirtual == -1)
	{

		if (!(ch = load_pc (name)))
		{
			system_log ("No such character by name in add_message()!", true);
			return;
		}

		nVirtual = ++ch->pc->staff_notes;
		named = 1;

		ch->unload_pc();
	}

	else if (nVirtual == -2)
	{
		new_message = 4;
	}

	else if (nVirtual == -5)
	{
		new_message = 3;
	}

	else if (nVirtual == -3)
	{
		if (!(dir = opendir (JOURNAL_DIR)))
		{
			perror ("opendir");
			printf ("Unable to open journal directory.");
			return;
		}
		for (i = 1; i <= 5000; i++)
		{
			found = false;
			sprintf (buf, "player_journals/%s.%06d", name, i);
			if (!(fp = fopen (buf, "r")))
				break;
			else
				fclose (fp);
		}
	}
	else if (new_message == 2 || new_message == 3 || new_message == 4
		|| new_message == 5)
		;
	else if (!nVirtual)
		nVirtual = board->next_virtual++;
	else if (nVirtual >= board->next_virtual)
		board->next_virtual = nVirtual + 1;
	else
		system_log
		("Virtual requested less than board's next virtual, add_message()",
		true);

	*date_buf = '\0';

	day = time_info.day + 1;
	if (day == 1)
		suf = "st";
	else if (day == 2)
		suf = "nd";
	else if (day == 3)
		suf = "rd";
	else if (day < 20)
		suf = "th";
	else if ((day % 10) == 1)
		suf = "st";
	else if ((day % 10) == 2)
		suf = "nd";
	else if ((day % 10) == 3)
		suf = "rd";
	else
		suf = "th";

	if (time_info.holiday == 0 &&
		!(time_info.month == 1 && day == 12) &&
		!(time_info.month == 4 && day == 10) &&
		!(time_info.month == 7 && day == 11) &&
		!(time_info.month == 10 && day == 12))
		sprintf (date_buf, "%d%s %s, %d SR", day, suf,
		month_name[time_info.month], time_info.year);
	else
	{
		if (time_info.holiday > 0)
		{
			sprintf (date_buf, "%s, %d SR",
				holiday_names[time_info.holiday], time_info.year);
		}
		else if (time_info.month == 1 && day == 12)
			sprintf (date_buf, "Erukyerme, %d SR", time_info.year);
		else if (time_info.month == 4 && day == 10)
			sprintf (date_buf, "Lairemerende, %d SR", time_info.year);
		else if (time_info.month == 7 && day == 11)
			sprintf (date_buf, "Eruhantale, %d SR", time_info.year);
		else if (time_info.month == 10 && day == 12)
			sprintf (date_buf, "Airilaitale, %d SR", time_info.year);
	}

	if (isalpha (*date_buf))
		*date_buf = toupper (*date_buf);

	if (new_message != 3 && new_message != 4 && new_message != 5)
		msg->nVirtual = nVirtual;
	else
		msg->nVirtual = i;
	msg->flags = flags;
	msg->poster = duplicateString (poster);
	msg->date = duplicateString (date);
	msg->subject = tilde_eliminator (subject);
	msg->info = duplicateString (info);
	msg->message = tilde_eliminator (message);
	msg->icdate = duplicateString (date_buf);

	if (named || new_message == 2)
		sprintf (msg_file_name, PLAYER_BOARD_DIR "/%s.%06d", name, nVirtual);
	else if (new_message != 2 && new_message != 3 && new_message != 4
		&& new_message != 5 && ch)
	{
		ch->desc->pending_message = msg;
		post_to_mysql_board (ch->desc);
		return;
	}
	else if (new_message == 5)
	{
		sprintf (msg_file_name, JOURNAL_DIR "/%s.%06d", name, i);
	}
	else if (new_message == 3)
	{
		add_message_to_mysql_vboard (name, poster, msg);
		return;
	}
	else if (new_message == 4)
	{
		add_message_to_mysql_player_notes (name, poster, msg);
		return;
	}

	if (!new_message)
	{
		free_mem(msg);
		return;
	}

	system_log ("Reached end of add_message()", true);

	free_mem(msg);
}

	//returns a single line from a file
char *read_a_line (FILE * fp)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };

	fgets (buf, MAX_STRING_LENGTH, fp);

	if (*buf)
		buf[strlen (buf) - 1] = '\0';

	return duplicateString (buf);
}

MESSAGE_DATA *load_message (char *msg_name, int pc_message, int msg_number)
{
	MESSAGE_DATA *message = NULL;
	FILE *fp_message = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' }, date_buf[MAX_STRING_LENGTH] =
	{
		'\0'}, *suf = '\0';
		int day = 0;
		bool tilde = false;

		*date_buf = '\0';

		if (pc_message == 1)
			sprintf (buf, PLAYER_BOARD_DIR "/%s.%06d", msg_name, msg_number);
		else if (pc_message == 2)
			sprintf (buf, VIRTUAL_BOARD_DIR "/%s.%06d", msg_name, msg_number);
		else if (pc_message == 3)
			sprintf (buf, JOURNAL_DIR "/%s.%06d", msg_name, msg_number);
		else
			sprintf (buf, BOARD_DIR "/%s.%06d", msg_name, msg_number);

		if (pc_message == 5)
		{
			message = load_mysql_message (msg_name, 1, msg_number);
			return message;
		}
		else if (pc_message == 6)
		{
			message = load_mysql_message (msg_name, 0, msg_number);
			return message;
		}
		else if (pc_message == 7)
		{
			message = load_mysql_message (msg_name, 2, msg_number);
			return message;
		}
		else if (pc_message == 8)
		{
			message = load_mysql_message (msg_name, 3, msg_number);
			return message;
		}

		if (!(fp_message = fopen (buf, "r+")))
		{
			printf ("Couldn't open %s\n", buf);
			return NULL;
		}

		message = new MESSAGE_DATA;

		message->nVirtual = msg_number;
		message->poster = read_a_line (fp_message);
		message->date = read_a_line (fp_message);
		message->subject = read_a_line (fp_message);
		message->info = read_a_line (fp_message);
		message->message = fread_string (fp_message);
		fscanf (fp_message, "%ld", &message->flags);

		day = time_info.day + 1;
		if (day == 1)
			suf = "st";
		else if (day == 2)
			suf = "nd";
		else if (day == 3)
			suf = "rd";
		else if (day < 20)
			suf = "th";
		else if ((day % 10) == 1)
			suf = "st";
		else if ((day % 10) == 2)
			suf = "nd";
		else if ((day % 10) == 3)
			suf = "rd";
		else
			suf = "th";

		sprintf (date_buf, "%s", read_a_line (fp_message));
		sprintf (date_buf, "%s", read_a_line (fp_message));

		if (!*date_buf)
		{
			if (time_info.holiday == 0 &&
				!(time_info.month == 1 && day == 12) &&
				!(time_info.month == 4 && day == 10) &&
				!(time_info.month == 7 && day == 11) &&
				!(time_info.month == 10 && day == 12))
				sprintf (date_buf, "%d%s %s, %d SR", day, suf,
				month_name[time_info.month], time_info.year);
			else
			{
				if (time_info.holiday > 0)
				{
					sprintf (date_buf, "%s, %d SR",
						holiday_names[time_info.holiday],
						time_info.year);
				}
				else if (time_info.month == 1 && day == 12)
					sprintf (date_buf, "Erukyerme, %d SR", time_info.year);
				else if (time_info.month == 4 && day == 10)
					sprintf (date_buf, "Lairemerende, %d SR", time_info.year);
				else if (time_info.month == 7 && day == 11)
					sprintf (date_buf, "Eruhantale, %d SR", time_info.year);
				else if (time_info.month == 10 && day == 12)
					sprintf (date_buf, "Airilaitale, %d SR", time_info.year);
			}
		}

		if (isalpha (*date_buf))
			*date_buf = toupper (*date_buf);

		if (*date_buf == '~')
			tilde = true;

		while (tilde)
		{
			sprintf (date_buf, "%s", read_a_line (fp_message));
			if (*date_buf != '~')
				tilde = false;
		}

		message->icdate = duplicateString (date_buf);

		fclose (fp_message);

		return message;
}

void do_notes (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *who = NULL;
	bool messages = false;
	char name[MAX_STRING_LENGTH] = { '\0' };

	argument = one_argument (argument, name);

	if (!*name)
	{
		ch->send_to_char
			("Which PC or virtual board did you wish to get a listing for?\n");
		return;
	}

	*name = toupper (*name);

	if (!(who = load_pc (name)))
	{
		messages = get_mysql_board_listing (ch, 1, name);
		if (!messages)
		{
			ch->send_to_char("No such PC or vboard.\n");
			return;
		}
		else
			return;
	}

	who->unload_pc();


	if (ch->pc && ch->pc->level < 3)
	  {
	    ch->send_to_char("You are not permitted to read player notes.\n");
	  }
	else
	  {
	    messages = get_mysql_board_listing (ch, 2, name);

	    if (!messages)
	      ch->send_to_char("That player does not have any notes.\n");
	  }
}

void do_journal (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *who = NULL;
	char name[MAX_STRING_LENGTH] = { '\0' };
	int messages = 0;
	bool loaded_char = false;

	if (IS_SET (ch->flags, FLAG_GUEST))
	{
		ch->send_to_char("Journals are unavailable to guests.\n");
		return;
	}

	if (ch->get_trust())
	{
		argument = one_argument (argument, name);
		if (!*name)
			who = ch;
		else
		{
			*name = toupper (*name);
			if (!(who = load_pc (name)))
			{
				ch->send_to_char("No such PC, I'm afraid.\n");
				return;
			}
			loaded_char = true;
		}
	}
	else
		who = ch;

	messages = get_mysql_board_listing (ch, 3, who->name);
	if (!messages)
		ch->send_to_char("No journal entries found.\n");

	if (who && who != ch && loaded_char)
		who->unload_pc();
}


void post_writing (DESCRIPTOR_DATA * d)
{
	OBJ_DATA *obj = NULL;
	OBJ_DATA *quill = NULL;
	CHAR_DATA *ch = NULL;
	OBJ_DATA *ink = NULL;
	WRITING_DATA *writing = NULL;
	unsigned int i = 0;
	time_t current_time = 0;
	char date[AVG_STRING_LENGTH] = "";
	char message[MAX_STRING_LENGTH] = "";
	float mod = 0;

	if (!d->pending_message || !d->pending_message->message || !*d->pending_message->message)
		return;

	ch = d->character;

	if (!(obj = ch->pc->writing_on))
	{
		ch->send_to_char("That object is no longer there!\n");
		return;
	}

	current_time = time (0);
	ctime_r (&current_time, date);
	if (strlen (date) > 1)
		date[strlen (date) - 1] = '\0';

	sprintf (message, "%s", d->pending_message->message);

	if (ch->right_hand && ch->right_hand->obj_flags.type_flag == ITEM_INK)
	{
		ink = ch->right_hand;
		if (ink->o.od.value[1] <= 0)
		{
			ch->send_to_char
				("\nHaving exhausted the last of your ink, you discard the now-empty vessel.\n");
			extract_obj (ink);
		}
	}

	if (ch->left_hand && ch->left_hand->obj_flags.type_flag == ITEM_INK)
	{
		ink = ch->left_hand;
		if (ink->o.od.value[1] <= 0)
		{
			ch->send_to_char
				("\nHaving exhausted the last of your ink, you discard the now-empty vessel.\n");
			extract_obj (ink);
		}
	}

		//removes ink from quill
	if ((quill = ch->right_hand))
	{
		if (quill->obj_flags.type_flag == ITEM_WRITING_INST)
		{
			if (quill->ink_color)
				quill->ink_color = NULL;
		}
		else
			quill = NULL;
	}

	if (!quill && ch->left_hand)
	{
		quill = ch->left_hand;
		if (quill->obj_flags.type_flag == ITEM_WRITING_INST)
		{
			if (quill->ink_color)
				quill->ink_color = NULL;
		}
		else
			quill = NULL;
	}

	mod = (skill_level(ch, ch->writes, 0) * 0.70) + (skill_level(ch, ch->speaks, 0) * 0.30);


	mod = (float) MIN (95, (int) mod);

	if (obj->obj_flags.type_flag == ITEM_PARCHMENT)
	{
		obj->writing = new WRITING_DATA;
		obj->writing->next_page = NULL;
		obj->writing->ink = duplicateString (ch->delay_who);
		obj->writing->author = duplicateString (ch->name);
		obj->writing->date = duplicateString (date);
		obj->writing->language = lookup_skill_id(ch->speaks);
		obj->writing->script = lookup_skill_id(ch->writes);
		obj->writing->message = tilde_eliminator (message);
		obj->writing->skill = (int) mod;
	}

	if (obj->obj_flags.type_flag == ITEM_BOOK)
	{
		for (i = 2, writing = obj->writing; i <= obj->open; i++)
		{
			if (obj->writing->next_page)
			{
				writing = writing->next_page;
			}
		}
		writing->ink = duplicateString (ch->delay_who);
		writing->author = duplicateString (ch->name);
		writing->date = duplicateString (date);
		writing->language = lookup_skill_id(ch->speaks);
		writing->script = lookup_skill_id(ch->writes);
		writing->message = tilde_eliminator (message);
		writing->skill = (int) mod;
	}

	ch->pc->writing_on = NULL;
	ch->delay_who = NULL;
	free_mem(d->pending_message);
	d->pending_message = NULL;

	skill_use (ch, ch->writes, 0);
	if (!number (0, 1))
		skill_use (ch, ch->speaks, 0);

	save_writing (obj);
}

void do_flip (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *obj = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buffer[MAX_STRING_LENGTH] = { '\0' };
	std::stringstream buffer2;

	argument = one_argument (argument, buf);

	if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->room->contents)))
	{
		ch->send_to_char("What did you want to flip?\n");
		return;
	}

	if (obj->obj_flags.type_flag != ITEM_BOOK)
	{
		ch->send_to_char("That isn't a book, unfortunately.\n");
		return;
	}

	if (!obj->open)
	{
		ch->send_to_char("You'll need to open it, first.\n");
		return;
	}

	if (!obj->o.od.value[0])
	{
		ch->send_to_char("It doesn't have any pages left to flip.\n");
		return;
	}

	if (*argument && !isdigit (*argument))
	{
		ch->send_to_char("Which page would you like to flip to?\n");
		return;
	}

	unsigned int page_arg=0;
	if (!*argument)
	{
		page_arg = obj->open + 1;
	}
	else
	{
		page_arg = strtol (argument, NULL, 10);
	}

	if (page_arg == obj->open)
	{
		ch->send_to_char("It's already open to that page.\n");
		return;
	}

	if (page_arg > obj->o.od.value[0])
	{
		sprintf (buf, "There are only %d pages in this book.\n",
			obj->o.od.value[0]);
		ch->send_to_char(buf);
		return;
	}

	if (page_arg > obj->open + 1)
		sprintf (buf,
		"You leaf carefully through #2%s#0 until you arrive at page %d.",
		obj->short_description, page_arg);
	else
		sprintf (buf, "You turn #2%s's#0 page.", obj->short_description);

	ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);

	if (page_arg > obj->open + 1)
		sprintf (buf,
		"%s#0 leafs carefully through #2%s#0 until %s arrives at the desired page.",
		 ch->char_short(), obj->short_description, HSSH (ch));
	else
		sprintf (buf, "%s#0 turns #2%s's#0 page.",  ch->char_short(),
		obj->short_description);
	
	buffer2.sync();
	buffer2 << "#5"  << CAP(buf);
	
	sprintf(buffer + strlen(buffer), "%s", buffer2.str().c_str());
	ch->act(buffer, false, obj, 0, TO_ROOM | _ACT_FORMAT);
	obj->open = page_arg;
}

void do_tear (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *obj = NULL;
	OBJ_DATA *parchment = NULL;
	WRITING_DATA *writing = NULL;
	WRITING_DATA *page = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buffer[MAX_STRING_LENGTH] = { '\0' };
	unsigned int i = 0;

	argument = one_argument (argument, buf);

	if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
	{
		ch->send_to_char("What did you wish to tear?\n");
		return;
	}

	if (ch->right_hand && ch->left_hand)
	{
		ch->send_to_char("You must have one hand free.\n");
		return;
	}

	if (obj->obj_flags.type_flag != ITEM_PARCHMENT
		&& obj->obj_flags.type_flag != ITEM_BOOK)
	{
		ch->send_to_char("I'm afraid that can't be torn.\n");
		return;
	}

	if (obj->obj_flags.type_flag == ITEM_BOOK)
	{
		if (!obj->writing_loaded)
			load_writing (obj);
		if (!obj->open)
		{
			ch->send_to_char("Open it to the page you wish to tear out, first.\n");
			return;
		}
		if (!obj->writing || !obj->o.od.value[0])
		{
			ch->send_to_char("It doesn't have any pages left!\n");
			return;
		}
		if (!(parchment = load_object (VNUM_PARCHMENT)))
		{
			ch->send_to_char
				("The parchment prototype (VNUM 61) appears to be missing. Please inform staff.\n");
			return;
		}
		for (i = 1, writing = obj->writing; i <= obj->open; i++)
		{
			if (i == 1 && i == obj->open)
			{
				sprintf (buf, "You carefully tear page %d from #2%s#0.", i,
					obj->short_description);
				page = writing;
				if (!obj->writing->next_page)
				{
					obj->writing = NULL;
				}
				else
					obj->writing = writing->next_page;
				break;
			}
			else if (i + 1 == obj->open && obj->open > 1)
			{
				sprintf (buf, "You carefully tear page %d from #2%s#0.", i + 1,
					obj->short_description);
				page = writing->next_page;
				if (!writing->next_page->next_page)
				{
					writing->next_page = NULL;
					obj->open--;
				}
				else
				{
					writing->next_page = writing->next_page->next_page;
				}
				break;
			}
			writing = writing->next_page;
		}

		ch->act(buf, false,  obj, 0, TO_CHAR | _ACT_FORMAT);
		sprintf (buf, "%s#0 carefully tears a page from #2%s#0.",
			 ch->char_short(), obj->short_description);
		
		sprintf(buffer, "#5%s", CAP (buf));
		ch->act(buffer, false, obj, 0, TO_ROOM | _ACT_FORMAT);

		if (*page->message && strcasecmp (page->message, "blank") != STR_MATCH)
		{
			parchment->writing = new WRITING_DATA;
			parchment->writing->next_page = NULL;
			parchment->writing->ink = duplicateString (page->ink);
			parchment->writing->author = duplicateString (page->author);
			parchment->writing->date = duplicateString (page->date);
			parchment->writing->language = page->language;
			parchment->writing->script = page->script;
			parchment->writing->message = duplicateString (page->message);
			parchment->writing->skill = page->skill;;
		}

		obj->o.od.value[0]--;
		obj_to_char (parchment, ch);
		save_writing (obj);
		save_writing (parchment);
		return;
	}

	sprintf (buf,
		"You rend #2%s#0 into small pieces, which you then meticulously discard.",
		obj->short_description);
	ch->act(buf, false,  obj, 0, TO_CHAR | _ACT_FORMAT);
	sprintf (buf,
		"%s rends #2%s#0 into small pieces, which %s then meticulously discards.",
		 ch->char_short(), obj->short_description, HSSH (ch));
	 sprintf(buffer, "#5%s", CAP (buf));
	ch->act(buffer, false, obj, 0, TO_ROOM | _ACT_FORMAT);

	extract_obj (obj);
}

void do_dip (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *quill = NULL, *ink = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	if (!
		(ch->left_hand
		&& ch->left_hand->obj_flags.type_flag == ITEM_WRITING_INST)
		&& !(ch->right_hand
		&& ch->right_hand->obj_flags.type_flag == ITEM_WRITING_INST))
	{
		ch->send_to_char("You need to be holding a writing implement.\n");
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		if (ch->right_hand
			&& ch->right_hand->obj_flags.type_flag == ITEM_WRITING_INST)
			quill = ch->right_hand;
		else
			quill = ch->left_hand;

		if (ch->right_hand && ch->right_hand->obj_flags.type_flag == ITEM_INK)
			ink = ch->right_hand;
		else if (ch->left_hand && ch->left_hand->obj_flags.type_flag == ITEM_INK)
			ink = ch->left_hand;
	}
	else
	{
		quill = get_obj_in_list_vis (ch, buf, ch->right_hand);
		if (!quill)
			quill = get_obj_in_list_vis (ch, buf, ch->left_hand);
		if (quill && quill->obj_flags.type_flag != ITEM_WRITING_INST)
		{
			ch->send_to_char
				("You must specify first the writing implement, and then the ink source.\n");
			return;
		}
		argument = one_argument (argument, buf);
		if (!*buf)
		{
			ch->send_to_char
				("You must specify both a writing implement and an ink source.\n");
			return;
		}
		ink = get_obj_in_list_vis (ch, buf, ch->right_hand);
		if (!ink)
			ink = get_obj_in_list_vis (ch, buf, ch->left_hand);
		if (!ink)
			ink = get_obj_in_list_vis (ch, buf, ch->room->contents);
		if (ink && ink->obj_flags.type_flag != ITEM_INK)
		{
			ch->send_to_char
				("You must specify first the writing implement, and then the ink source.\n");
			return;
		}
	}

	if (!ink)
	{
		ch->send_to_char
			("You need to have an ink source in one hand, or in the room.\n");
		return;
	}

	if (!quill)
	{
		ch->send_to_char("You need to be holding a writing implement.\n");
		return;
	}

	if (ink->o.od.value[0] <= 0)
	{
		ch->send_to_char("Your ink source seems to be empty.\n");
		return;
	}

	sprintf (buf,
		"You dip #2%s#0 carefully into #2%s#0, liberally coating its tip.",
		quill->short_description, ink->short_description);
	
	quill->ink_color = ink->ink_color;

	ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
	ink->o.od.value[0]--;
}

void do_scribe (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *obj = NULL;
	OBJ_DATA *quill = NULL;
	WRITING_DATA *writing = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buffer[MAX_STRING_LENGTH] = { '\0' };
	unsigned int i = 0;

	if (IS_NPC (ch))
	{
		ch->send_to_char("This is only available to PCs.\n");
		return;
	}

	
	if (!ch->writes || (ch->skill_map[ch->writes] < 1))
	{
		ch->send_to_char
			("In which script would you like to write? (See the SCRIBE command.)\n");
		return;
	}

	argument = one_argument (argument, buf);

	if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->room->contents)))
	{
		ch->send_to_char("You can't find that!\n");
		return;
	}

	if (!ch->left_hand && !ch->right_hand)
	{
		ch->send_to_char("You need to be holding a writing implement.\n");
		return;
	}

	if (ch->right_hand && ch->right_hand->obj_flags.type_flag == ITEM_WRITING_INST)
		quill = ch->right_hand;
	else if (ch->left_hand
		&& ch->left_hand->obj_flags.type_flag == ITEM_WRITING_INST)
		quill = ch->left_hand;
	else
	{
		ch->send_to_char("You need to be holding a writing implement.\n");
		return;
	}

	if (!quill->ink_color)
	{
		ch->send_to_char("The writing instrument must be dipped in ink, first.\n");
		return;
	}

	ch->delay_who = quill->ink_color;

	if (obj->obj_flags.type_flag != ITEM_PARCHMENT &&
		obj->obj_flags.type_flag != ITEM_BOOK)
	{
		ch->act("That cannot be written on.", false, obj, 0, TO_CHAR);
		return;
	}

	if (!obj->writing_loaded)
		load_writing (obj);

	if (obj->obj_flags.type_flag == ITEM_PARCHMENT && obj->writing)
	{
		ch->send_to_char("That has already been written on.\n");
		return;
	}

	else if (obj->obj_flags.type_flag == ITEM_BOOK)
	{
		if (!obj->open)
		{
			ch->send_to_char("You need to open it, first.\n");
			return;
		}
		if (!obj->writing || !obj->o.od.value[0])
		{
			ch->send_to_char("It doesn't have any pages left!\n");
			return;
		}
		for (i = 2, writing = obj->writing; i <= obj->open; i++)
		{
			if (obj->writing->next_page)
			{
				writing = writing->next_page;
			}
		}
		if (strcasecmp (writing->message, "blank") != STR_MATCH)
		{
			ch->send_to_char("That has already been written on.\n");
			return;
		}
	}

	sprintf(buffer, "#5%s#0 begins writing on #2%s#0.",  ch->char_short(),
		obj->short_description);
	buffer[3] = toupper (buffer[3]);
	ch->act(buffer, true, 0, 0, TO_ROOM | _ACT_FORMAT);

	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;

	ch->send_to_char
		("Scribe your message; terminate with an '@'. Please keep its length plausible\nfor the size of the writing object, since we have opted against coded limits.\n");
	sprintf (buf,
		"1-------10--------20--------30--------40--------50--------60--------70\n");
	ch->send_to_char(buf);

	ch->make_quiet();

	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->desc->proc = post_writing;
	ch->pc->writing_on = obj;
}

/***
do_evaluate

This command provides detailed information about a held object
including weight, cost and any skill affects, as well as other
object specific information such as time left for light objects
and liquid left in drinks containers.

- Valarauka

***/
void do_evaluate (CHAR_DATA *ch, char *argument, int cmd)
{


	char		arg1 [MAX_STRING_LENGTH] = { '\0' };
	OBJ_DATA	*obj = NULL;
	char		buffer [MAX_STRING_LENGTH] = { '\0' };

	/*** CHECK FOR POSTIONS AND CONDITONS FIRST ***/

	if ( ch->get_position() < SLEEP ) {
		ch->send_to_char("You are unconscious!\n");
		return;
	}

	if ( ch->get_position() == SLEEP ) {
		ch->send_to_char("You are asleep.\n");
		return;
	}

	if ( ch->is_blind() ) {
		ch->send_to_char("You are blind!\n");
		return;
	}

	argument = one_argument (argument, arg1);

	if ( !*arg1 ) {
		ch->send_to_char("Evaluate what?\n");
		return;
	}

	if ( !(obj = get_obj_in_dark (ch, arg1, ch->right_hand)) &&
		!(obj = get_obj_in_dark (ch, arg1, ch->left_hand)) &&
		!(obj = get_obj_in_dark (ch, arg1, ch->equip)) &&
		!(obj = get_obj_in_dark (ch, arg1, ch->room->contents))) {

			ch->send_to_char("You don't have that.\n");
			return;
	}

	/*** Describe the object ***/
	if (obj)
	{
		snprintf (buffer, MAX_STRING_LENGTH,  "\nIt is #2%s#0.", obj->short_description);
		ch->act(buffer, false, 0, 0, TO_CHAR | _ACT_FORMAT);
		*buffer = '\0';
	}
	else{
		ch->send_to_char("You cannot evaluate something unless you have it.\n");
		return;
	}

	ch->send_to_char("\n");

	show_evaluate_information(ch, obj);
	
	
	return;
}

void show_evaluate_information (CHAR_DATA *ch, OBJ_DATA	*obj)
{
	int 		variance = 0;
	int 		guess_weight = 0;
	int 		guess_value = 0;
	int			temp = 0;
	int			i = 0;
	char		buffer [MAX_STRING_LENGTH] = { '\0' };
	char		*temp_arg = NULL;
	AFFECTED_TYPE *af;

	/*** WEAR LOCATIONS FOR WEARABLE ITEMS ***/

	if (obj->obj_flags.wear_flags)
	{
		temp = 0;
		for (i = 0; (*wear_bits[i] != '\n'); i++)
		{
			if ((IS_SET (obj->obj_flags.wear_flags, (1 << i)))
				&& (strcmp (wear_bits[i],"Take"))
				&& (strcmp (wear_bits[i],"Unused"))) //dont want to show Take and Unused
			{
				//if found something to write and not already written inital string, write it
				if(temp == 0)
				{
					sprintf(buffer, "\nYou recognise that you could wear this item in the following locations:\n");
					temp = 1;
				}

				sprintf(buffer + strlen(buffer), "  #6%s#0\n", wear_bits[i]);
			}
		}

		ch->send_to_char(buffer);
		ch->send_to_char("\n");
	}

	/*** END WEAR LOCATIONS ***/

	
	
	/**** Guess at the Weight based on scan skill **/


	if (IS_SET (obj->obj_flags.wear_flags, ITEM_TAKE))
	{

		/*
		Rather than give the exact weight to the player, a random factor
		is introduced to replicate a rough estimate. The scan
		skill is used to determine how far from the actual weight the value given
		may deviate so that those with a high scan skill will be fairly accurate,
		where as those with a low scan skill might be quite far from the actual weight.
		The heavier an item, the harder it will be to get a very accurate estimate.

		In addition, a skill check must then be passed to allow the player to see the
		estimated weight.
		*/

		/*** Determine how far estimate may vary ***/
		if(ch->skill_map["Scan"]>70)
		{
			variance = number (95, 105);
		}
		else if(ch->skill_map["Scan"]>60)
		{
			variance = number (92, 108);
		}
		else if(ch->skill_map["Scan"]>50)
		{
			variance = number (90, 110);
		}
		else if(ch->skill_map["Scan"]>40)
		{
			variance = number (87, 113);
		}
		else
		{
			variance = number (85, 115);
		}

		/*** Calculate estimate ***/
		guess_weight = (int)((obj->obj_flags.weight + obj->contained_wt)/variance);

		if (skill_use(ch, "Scan", 0))
		{
			if (guess_weight <= 1)
			{
				sprintf(buffer, "\nYou would guess that this item weighs less than a pound");
			}
			else{  /** weighs more than a pound **/
				sprintf(buffer, "\nYou would guess that this item weighs about %d pounds", guess_weight);
			}
		}
		else
		{ /** failed skill check **/
			sprintf(buffer, "\nYou can't even begin to guess how much this weighs");
		}
	}
	else
	{ /** no way to check weight - message for non-takeable objects **/
		sprintf(buffer, "\nYou can't even begin to guess how much this weighs");
	}

		
	/*** end weight guess ***/

	/*** Guess at value based on barter skill ***/

	/*
	Rather than give the exact cost to the player, a random factor
	is introduced to replicate a rough estimate of the cost. The barter
	skill is used to determine how far from the actual cost the value given
	may deviate so that those with a high barter skill will be fairly accurate,
	where as those with a low barter skill might be quite far from the actual cost.
	The more expensive an item, the harder it will be to get a very accurate estimate.

	In addition, a skill check must then be passed to allow the player to see the cost.
	*/

	/*** Determine how far estimation may vary ***/
	if(ch->skill_map["Barter"]>70)
	{
		variance = number (95, 105);
	}
	else if(ch->skill_map["Barter"]>60)
	{
		variance = number (92, 108);
	}
	else if(ch->skill_map["Barter"]>50)
	{
		variance = number (90, 110);
	}
	else if(ch->skill_map["Barter"]>40)
	{
		variance = number (87, 113);
	}
	else
	{
		variance = number (85, 115);
	}

	/*** Calculate estimate ***/
	guess_value = (int)(((obj->coppers)*100)/variance);

	if (skill_use(ch, "Barter", 0))
	{//passed Skill Check 
		if (guess_value <= 1)
		{
			sprintf(buffer + strlen(buffer), " and you would guess that this item is worth less than 1 copper.");
		}
		else
		{
			sprintf(buffer + strlen(buffer), " and you would guess that this item is worth around %d coppers.", guess_value);
		}
	}
	else
	{//failed Skill Check 
		sprintf(buffer + strlen(buffer), " and you can't even begin to guess the value of this item.");
	}

		//overall quality of object
		//TODO: repalce this with different phrases for differnt item types
	
	sprintf(buffer + strlen(buffer), " ");
	
	if (obj->quality < 10)
	{
		sprintf(buffer + strlen(buffer), "It belongs in a midden heap.");
	}
	else if ((obj->quality >= 10) && (obj->quality < 20))
	{
		sprintf(buffer + strlen(buffer), "Barely recognizable, you are not certain what it really is supposed to be.");
	}
	else if ((obj->quality >= 20) && (obj->quality < 30))
	{
		sprintf(buffer + strlen(buffer), "It is useable, but it is not something to be proud of.");
	}
	else if ((obj->quality >= 30) && (obj->quality < 40))
	{
		sprintf(buffer + strlen(buffer), "While wholly useful, it lacks many requirements for high quality.");
	}
	else if ((obj->quality >= 40) && (obj->quality < 50))
	{
		sprintf(buffer + strlen(buffer), "At first glance, it appears to be of a fairly decent quality.");
	}
	else if ((obj->quality >= 50) && (obj->quality < 60))
	{
		sprintf(buffer + strlen(buffer), "Under closer inspection, only a few flaws can be found.");
	}
	else if ((obj->quality >= 60) && (obj->quality < 70))
	{
		sprintf(buffer + strlen(buffer), "While not flawless, it is of high value.");
	}
	else if ((obj->quality >= 70) && (obj->quality < 80))
	{
		sprintf(buffer + strlen(buffer), "It is superior to similar items.");
	}
	else if (obj->quality >= 80)
	{
		sprintf(buffer + strlen(buffer), "Under detailed examination, it exceeds expectations of high standards.");
	}
	
	
	ch->act(buffer, false, 0, 0, TO_CHAR | _ACT_FORMAT);
	*buffer = '\0';
	ch->send_to_char("\n");

	/*** end value guess ***/


	/***** Pslim lantern code ***/
	if ( obj->obj_flags.type_flag == ITEM_LIGHT ) {
		if ( obj->o.light.hours <= 0 ){
			snprintf (buffer, MAX_STRING_LENGTH,  "\nThe $o is empty.");
		}
		else {
			temp = obj->o.light.hours;
			snprintf (buffer, MAX_STRING_LENGTH,  "\nYou think the $o will last another %d hours.", temp);
		}
		ch->act(buffer, false, 0, 0, TO_CHAR | _ACT_FORMAT);
		*buffer = '\0';
		ch->send_to_char("\n");
	}
	/***** end Pslim lantern code *****/

	/**** SHOW CAPACITY OF CONTAINER ****/

	if (obj->obj_flags.type_flag == ITEM_CONTAINER)
	{

		sprintf(buffer, "\nYou estimate that it would hold around %d.%.2d lbs",
			obj->o.container.capacity / 100,
			obj->o.container.capacity % 100);
		ch->act(buffer, false, 0, 0, TO_CHAR | _ACT_FORMAT);
		*buffer = '\0';
		ch->send_to_char("\n");
	}

	else if ((obj->obj_flags.type_flag == ITEM_DRINKCON))
	{
		if (obj->o.drinkcon.capacity > 150)
			sprintf(buffer, "\nYou estimate that it would hold around %d gallons of fluid.", obj->o.drinkcon.capacity / 50);
		
		else if (obj->o.drinkcon.capacity == -1)
			sprintf(buffer, "\nYou cannot tell how much it holds.");
		
		else
			sprintf(buffer, "\nYou estimate that it would hold around %d fluid ounces.", obj->o.drinkcon.capacity * 12 / 5);
		ch->act(buffer, false, 0, 0, TO_CHAR | _ACT_FORMAT);
		*buffer = '\0';
		ch->send_to_char("\n");
	}

	else if ((obj->obj_flags.type_flag == ITEM_DRYCON))
	{
		if (obj->o.drycon.capacity > 150)
			sprintf(buffer, "\nYou estimate that it would hold around %d pounds of contents.", obj->o.drycon.capacity / 100);
		
		else if (obj->o.drycon.capacity == -1)
			sprintf(buffer, "\nYou cannot tell how much it holds.");
		
		else
			sprintf(buffer, "\nYou estimate that it would hold around %d dry ounces.", obj->o.drycon.capacity * 16 / 100);
		ch->act(buffer, false, 0, 0, TO_CHAR | _ACT_FORMAT);
		*buffer = '\0';
		ch->send_to_char("\n");
	}
	/*** END CAPACITY CODE ***/


	/**** LOOK INSIDE A DRINK/DRY CONTATINER ***/
	/**
	 capacity - od.value[0]
	 volume - od.value[1]
	 contents/liquid - od.value[2]
	 **/
	if (obj->obj_flags.type_flag == ITEM_DRINKCON || obj->obj_flags.type_flag == ITEM_DRYCON )
	{
		if ( obj->o.od.value[1] == 0 )
		{
			snprintf (buffer, MAX_STRING_LENGTH,  "\nYou can see that %s is empty.",
				obj->short_description);
		}
		else if ( obj->o.od.value[1] < 0 )
		{
			snprintf (buffer, MAX_STRING_LENGTH,  "\nYou can see that %s is full.",
					  obj->short_description);
		}
		else
		{
			if ( obj->o.od.value[0] )
			{
				temp = (obj->o.od.value[1] * 3) / obj->o.od.value[0];
			}
			else
			{
				temp = 1;
			}
			temp_arg = vnum_to_name (obj->o.od.value[2]);
			snprintf (buffer, MAX_STRING_LENGTH,  "\nYou can see that %s is %sfull of %s.",
				obj->short_description, fullness [temp], temp_arg);

		}
		ch->act(buffer, false, 0, 0, TO_CHAR | _ACT_FORMAT);
		*buffer = '\0';
		ch->send_to_char("\n");

	}
	/*** END DRINK CONTAINER ***/

	
	/**** LOOK INSIDE A NON-DRINK CONTAINER ***/

	if ( obj->obj_flags.type_flag == ITEM_CONTAINER ||
		obj->obj_flags.type_flag == ITEM_KEYRING ) {

			if ( IS_SET (obj->o.container.flags, CONT_CLOSED) ) {
				ch->send_to_char("\nIt is closed.");
				return;
			}

			ch->send_to_char("\nContents :\n");

			list_obj_to_char (obj->contains, ch, 1, true);
	}
	/* If not a container, no need to show any message here */

	/*** END NON_DRINK CONTAINER ***/

	/*** SHOW TIME TO DECAY FOR FOOD ***/

	if(obj->obj_flags.type_flag == ITEM_FOOD)
	{
		if (obj->morphTime)
		{
			int delta, days, hours, minutes;

			delta = obj->morphTime - time (0);

			days = delta / 86400;
			delta -= days * 86400;

			hours = delta / 3600;
			delta -= hours * 3600;

			minutes = delta / 60;

			//write appropriate message for length of time left till decay
			if(days > 1)
			{
				sprintf (buffer,
					"\nYou notice that %s appears fresh with no sign of decay.", obj->short_description);
			}
			else if(hours > 12)
			{
				sprintf (buffer,
					"\nYou notice that %s still appears fresh, though it is slowly beginning to lose its original unsullied appearance.",
					obj->short_description);
			}
			else if(hours > 1)
			{
				sprintf (buffer,
					"\nWhile still in good condition, %s no longer appears fresh.", obj->short_description);
			}
			else
			{
				sprintf (buffer,
					"\nYou notice that %s has already begun to decay in places and it will not be too long before it is completely rotten.",
					obj->short_description);
			}

			ch->act(buffer, false, 0, 0, TO_CHAR | _ACT_FORMAT);
			*buffer = '\0';
			ch->send_to_char("\n");
		}
	}

	/*** END FOOD DECAY TIME ***/

	/*** DETAIL ANY SKILLS AFFECTED BY THE OBJECT ***/

	for (af = obj->xaffected; af; af = af->next)
	{
		if (af->type == MAGIC_HIDDEN)
			continue;
		if (af->a.spell.location >= 10000)
		{
			if(af->a.spell.modifier < 0)
			{
				sprintf(buffer, "\nYou judge that this item would hinder your %s skill.",
					lookup_skill_name(af->a.spell.location - 10000));
			}
			else
			{
				sprintf(buffer, "\nYou judge that this item would improve your %s skill.",
					lookup_skill_name(af->a.spell.location - 10000));
			}

			ch->act(buffer, false, 0, 0, TO_CHAR | _ACT_FORMAT);
			*buffer = '\0';
		}
	}

	ch->send_to_char("\n");

	/*** END SKILL AFFECTS ***/

	return;
}


void do_origins (CHAR_DATA *ch, char *argument, int cmd)
{
	char output[MAX_STRING_LENGTH];
	OBJ_DATA * obj = 0;

	/* check hands then equip list then finally room */
	obj = get_obj_in_list_vis(ch,argument,ch->left_hand);
	if (!obj)
		obj = get_obj_in_list_vis(ch,argument,ch->right_hand);
	if (!obj)
		obj = get_obj_in_list_vis(ch,argument,ch->equip);
	if (!obj)
		obj = get_obj_in_list_vis (ch, argument, ch->room->contents);

	/* complain if it doesn't exist */
	if (!obj)
	{
		ch->send_to_char("I don't see what you're referring to.\n");
		return;
	}

	/* build the initial output */
	sprintf(output, "   It is #2%s#0.\n\n", OBJS (obj, ch));

	/* add the crafts list */
	char* originlist = origins_list(ch,obj);
	if (originlist != NULL)
	{
		sprintf(output + strlen(output), "%s", originlist);
		free_mem(originlist);
	}

	ch->send_to_char(output);
}


/* returns line formatted edition of crafts you can use something in */
/* this must be free_mem'd to prevent leaks */
char * origins_list(CHAR_DATA * ch, OBJ_DATA * obj)
{
	char* p = 0;
	char buf[MAX_STRING_LENGTH] = {'\0'};
	char output[MAX_STRING_LENGTH];
	int crafts_found = 0;
	AFFECTED_TYPE *af=0;
	SUBCRAFT_HEAD_DATA *tcraft;
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	
	/* if you're an imm, search all crafts regardless of those on your avvie */
	
	if (ch->get_trust())
	{
		for (tcraft_iterator = craft_map.begin(); tcraft_iterator != craft_map.end(); tcraft_iterator++)
		{
			tcraft = tcraft_iterator->second;
			if (!craft_produces (tcraft, obj->nVirtual))
				continue;
			if (crafts_found)
				sprintf (buf + strlen (buf), ", ");
			sprintf (buf + strlen (buf), "'%s %s'",tcraft->command,tcraft->subcraft_name);
			crafts_found += 1;
		}
	}
	else /* you're a mortal .. only list from those you know */
	{
		/* search all crafts you have and look for the indicated object */
		for (int i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
		{
			if (!(af = get_affect (ch, i)))
				continue;
			/* this shouldn't happen since it's in the craft range of affect nums, but might as well check */
			if (!af->a.craft || !af->a.craft->subcraft)
				continue;
			/* if the craft does not produce this item, ignore that craft */
			if (!craft_produces (af->a.craft->subcraft, obj->nVirtual))
				continue;

			/* format prettily */
			if (crafts_found)
				sprintf (buf + strlen (buf), ", ");

			/* print the command and name */
			sprintf (buf + strlen (buf), "'%s %s'",af->a.craft->subcraft->command,af->a.craft->subcraft->subcraft_name);
			crafts_found += 1;
		}
	}

	/* now whether mortal or not, check for any crafts found and stored in buf */
	if (crafts_found)
	{
		sprintf (output,
			"\n   You realize that you could make this item with the following craft%s: %s.\n",
			crafts_found != 1 ? "s" : "", buf);

		/* output reformats the long line into breaks for columns */
		reformat_string (output, &p);
	}

	/* return the duplicateString'd version */
	return p;
}

bool
point_diagonal (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char arg1[MAX_STRING_LENGTH] = { '\0' };
	char arg2[MAX_STRING_LENGTH] = { '\0' };
	char distance[MAX_STRING_LENGTH] = { '\0' };
	char buffer[MAX_STRING_LENGTH] = { '\0' };
	CHAR_DATA *target = NULL, *tch = NULL;
	ROOM_DATA *target_room = NULL;
	int dir = 0;
	
	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);
	
	dir = index_lookup (dirs, arg1);
	
	if (dir == -1)
	{
		ch->send_to_char("There isn't an exit in that direction.\n");
		return (false);
	}
	
	if (is_sunlight_restricted (ch))
		return(false);
	
	target_room = get_diagonal_move_room (ch, dir);
	
	if (!target_room)
	{
		ch->send_to_char("Your view is blocked.\n");
		return (false);
	}
		//we have a room. do we have a target? - For diagonals we only look 1 room deep
	target = get_char_room_vis2 (ch, target_room->nVirtual, arg2);	
	if (target && can_see_mob(ch, target) && has_been_sighted (ch, target))
	{
		sprintf (distance, "to the %s", dirs[dir]);
	}
	else 
	{
		ch->send_to_char("You don't see them within range.\n");
		return (false);
	}
	
	sprintf (buf, "You point at #5%s#0, %s.",  target->char_short(), distance);
	ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
	
	sprintf (buf, "%s#0 points at #5%s#0, %s.",  ch->char_short(),
			  target->char_short(), distance);
	*buf = toupper (*buf);
	
	sprintf(buffer, "#5%s", buf);
	sprintf (buf, "%s", buffer);
	ch->act(buf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
	
	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (!can_see_mob(tch, ch))
			continue;
		target_sighted (tch, target);
	}
	return(true);
}

void
look_gl_window(CHAR_DATA *ch)
{
	int nRoomVnum;
	int original_loc;
	bool again = true;
	bool abrt = false;
	bool change;
	ROOM_DATA *troom = NULL;
	CHAR_DATA *tch = NULL;
	
		//window is not availabe right now
	ch->send_to_char("You press your face to a window and potent optical magics billow about your eyes. You almost catch a lifelike glimpse of something, but it fades from view as quickly as you sense it.\n");
	return;
	
	while (again)
	{
			//select room at random
		nRoomVnum = number (1, 5000);
		
		if (!(troom = vtor (nRoomVnum)))
			continue;
		
		if (IS_SET (troom->room_flags, INDOORS))
			continue;
		
		if (troom->terrain_type == SECT_URBAN)
			continue;
		
		if (troom->terrain_type == SECT_CAVE)
			continue;
		
		if (IS_SET (troom->room_flags, STORAGE))
			continue;
		
		if (strlen (troom->description) < 256)
			continue;
		
		if (!strncmp (troom->description, "No Description Set", 17))
			continue;
		
		if (!strncmp (troom->description, "   No Description Set", 20))
			continue;
		
		abrt = false;
		for (tch = troom->people; tch; tch = tch->next_in_room)
			if (!IS_NPC (tch))
				abrt = true;
		
		if (!abrt)
		{
			again = false;
		}
	}
	
	if (!IS_SET (ch->affected_by, AFF_INFRAVIS))
	{
		ch->affected_by |= AFF_INFRAVIS;
		change = true;
	}
	original_loc = ch->in_room;
	ch->act
	("$n presses $s face to a window, gazing down at Arda's brilliant blue, far below.",
	 true, 0, 0, TO_ROOM | _ACT_FORMAT);
	ch->act
	("You press your face to a window, gazing down at Arda's brilliant blue, potent optical magics allowing you to catch a closer, lifelike glimpse of:",
	 false, 0, 0, TO_CHAR | _ACT_FORMAT);
	ch->char_from_room();
	ch->char_to_room(nRoomVnum);
	ch->send_to_char("\n");
	do_look (ch, "", 0);
	ch->char_from_room();
	ch->char_to_room(original_loc);
	if ((!ch->get_trust()))
		ch->roundtime = 15;
	if (change)
		ch->affected_by &= ~AFF_INFRAVIS;
	
	return;	
}

void
look_direction(CHAR_DATA *ch, char *arg1)
{
	int dir = 0;
	int temp_room = 0;
	bool visible_mob = false;
	bool visible_obj = false;
	ROOM_EXIT_DATA *exit = NULL;
	AFFECTED_TYPE *af = NULL;
	CHAR_DATA *tch = NULL;
	ROOM_DATA *troom = NULL;
	OBJ_DATA *tobj = NULL;
	
	dir = index_lookup (dirs, arg1);
	
	if (!(exit = is_exit(ch, dir)))
	{
		ch->send_to_char("There is no obvious exit that way.\n");
		return;
	}
	
	if (exit->fdesc && *exit->fdesc)
		find_portal_description(exit, ch);
	
	if ((af = get_affect (ch, AFFECT_SHADOW))
		&& af->a.shadow.edge == dir
		&& IS_SET (exit->port_flags, EX_ISDOOR)
		&& IS_SET (exit->port_flags, EX_CLOSED)
		&& !IS_SET (exit->port_flags, EX_ISGATE))
	{
		ch->send_to_char
		("Your field of view through that exit is obstructed.\n");
		return;
	}
	
		//standing at edge of room, can see people and objects
	else if ((af = get_affect (ch, AFFECT_SHADOW)) &&
			 af->a.shadow.edge == dir)
	{
		ch->send_to_char("You are close enough to see what is in the next "
					  "room:\n\n");
		
		temp_room = ch->in_room;
		
		ch->char_from_room();
		ch->char_to_room(exit->to_room);
		
		list_obj_to_char (ch->room->contents, ch, 0, false);
		list_char_to_char (ch->room->people, ch);
		
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
			if (can_see_mob(ch, tch))
				target_sighted (ch, tch);
		
		ch->char_from_room();
		ch->char_to_room(temp_room);
		return;
	}
		//standing in middle of room, all you see is people?
		//show people and objects, but based on a penalized scan test
	else
	{
		
		if (!(troom = vtor (exit->to_room)))
		{
			ch->send_to_char("The way in that direction is blocked.\n");
			return;
		}
		
		visible_mob = false;
		
		for (tch = troom->people; tch; tch = tch->next_in_room)
			if (could_see (ch, tch)
				&& (skill_use(ch, "Scan", 10)))
			{
				ch->act("You see $N.", false, 0, tch, TO_CHAR);
				visible_mob = true;
			}
		
		if (!visible_mob)
		{
			ch->send_to_char("\nDespite your best efforts, you cannot see anyone in that direction.\n");
		}
		
		for (tobj = troom->contents; tobj; tobj = tobj->next_content)
		{
						
			if (could_see_obj (ch, tobj)
				&& (skill_use(ch, "Scan", 10)))
			{
			show_obj_to_char (tobj, ch, 7, could_see_obj (ch, tobj));
			visible_obj = true;
			}
		}
		
		if (!visible_obj)
		{
			ch->send_to_char("\nDespite your best efforts, you cannot see anything in that direction.\n");
		}
		return;
	}
	return;
}

void
look_in_container (CHAR_DATA *ch, char *argument)
{
	int volume;
	OBJ_DATA *obj = NULL;
	OBJ_DATA *tobj = NULL;
	char arg1[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char tmp_buf[MAX_STRING_LENGTH] = { '\0' };
	
	argument = one_argument (argument, arg1);
	
	if (!*arg1)
	{
		ch->send_to_char("Look in what?\n");
		return;
	}
	
	if (!(obj = get_obj_in_dark (ch, arg1, ch->right_hand)) &&
		!(obj = get_obj_in_dark (ch, arg1, ch->left_hand)) &&
		!(obj = get_obj_in_dark (ch, arg1, ch->equip)) &&
		!(obj = get_obj_in_list_vis (ch, arg1, ch->room->contents)))
	{
		ch->send_to_char("You don't see that here.\n");
		return;
	}
	
	/**
	 capacity - od.value[0]
	 volume - od.value[1]
	 contents/liquid - od.value[2]
	 **/
	
	if (obj->obj_flags.type_flag == ITEM_DRINKCON || obj->obj_flags.type_flag == ITEM_DRYCON)
	{
		
		if (obj->o.od.value[1] == 0)
			ch->act("$o is empty.\n", false, obj, 0, TO_CHAR);
		
			// for infinte fountains of stew
		else if (obj->o.od.value[0] == -1)
		{
			sprintf(tmp_buf, "%s", obj->short_description);
			tmp_buf[0] = toupper(tmp_buf[0]);
			
			tobj = vtoo(obj->o.od.value[2]);
			if (tobj)
			{
			sprintf (buf, "#2%s#0 is full of #2%s#0.\n",
					 tmp_buf,
					 tobj->short_description);
			ch->send_to_char(buf);
			}
			else
			{
				sprintf (buf, "#2%s#0 is full.\n", tmp_buf);
				ch->send_to_char(buf);
			}

		}
		
		else
		{
			if (obj->o.od.value[0])
				volume = obj->o.od.value[1] * 3 / obj->o.od.value[0];
			else
				volume = 1;
			
			sprintf(tmp_buf, "%s", obj->short_description);
			tmp_buf[0] = toupper(tmp_buf[0]);
			
			tobj = vtoo(obj->o.od.value[2]);
			
			sprintf (buf, "#2%s#0 is %sfull of #2%s#0 and would hold about %d units.\n",
					 tmp_buf,
					 fullness[volume],
					 tobj->short_description,
					 obj->o.od.value[0]);
			ch->send_to_char(buf);
		}
		
		return;
	}
	
	else if (obj->obj_flags.type_flag == ITEM_LIGHT)
	{
		tobj = vtoo(obj->o.od.value[2]);
		
		if (obj->o.light.capacity == -1)
		{
			if (tobj)
			{
			sprintf (buf,
					 "$p looks like it is refilled with #2%s#0, on a regular basis.\n",
					 tobj->short_description);
			ch->act(buf, false,  obj, 0, TO_CHAR);
			}
			
			else 
			{
				sprintf (buf,
						 "$p looks like it is refilled on a regular basis.\n");
				ch->act(buf, false,  obj, 0, TO_CHAR);
			}

		}
		
		else if (obj->o.light.hours)
		{
			if (tobj)
			{
				sprintf (buf,
						 "$p looks like it will burn for about %d more hour%s, before it needs to be filled with %s.\n",
						 obj->o.light.hours,
						 (obj->o.light.hours > 1) ? "s" : "",
						 tobj->short_description);
				ch->act(buf, false,  obj, 0, TO_CHAR);
			}
			else
			{
				sprintf (buf,
						 "$p looks like it will burn for about %d more hour%s.\n",
						 obj->o.light.hours,
						 (obj->o.light.hours > 1) ? "s" : "");
				ch->act(buf, false,  obj, 0, TO_CHAR);
			}

			
		}
		else if (obj->o.light.hours == 0)
		{
			if (tobj)
			{
				sprintf (buf, "$o is spent, but it can be re-filled with %s.", tobj->short_description);
				ch->act(buf, false,  obj, 0, TO_CHAR);
			}
			else
				ch->act("$o is spent.", false, obj, 0, TO_CHAR);
		}
		else
		{
			ch->send_to_char
			("You can't tell how much longer that will last.\n");
		}
	}
	
	else if (obj->obj_flags.type_flag == ITEM_FUEL)
	{
		tobj = vtoo(obj->o.fuelcon.fuel);
		if (obj->o.fuelcon.volume)
		{
			sprintf (buf,
					 "$p looks like it will last about %d more hour%s.\n",
					 obj->o.fuelcon.volume,
					 (obj->o.fuelcon.volume > 1) ? "s" : "");
			ch->act(buf, false,  obj, 0, TO_CHAR);
		}
		else if (obj->o.light.hours == 0)
		{
			ch->act("$o has no fuel remaining.\n", false, obj, 0, TO_CHAR);
		}
		else
		{
			ch->send_to_char
			("You can't tell how much fuel is left.\n");
		}
	}
	
	else if (obj->obj_flags.type_flag == ITEM_CONTAINER ||
			 obj->obj_flags.type_flag == ITEM_KEYRING)
	{
		
		if (IS_SET (obj->o.container.flags, CONT_CLOSED))
		{
			ch->send_to_char("It is closed.\n");
			return;
		}
		
		ch->send_to_char(fname (obj->name));
		
		if (obj->in_room != NOWHERE)
			ch->send_to_char(" (here) : \n");
		else if (obj->carried_by)
			ch->send_to_char(" (carried) : \n");
		else
			ch->send_to_char(" (used) : \n");
		
		list_obj_to_char (obj->contains, ch, 1, true);
	}
	
	else
		ch->send_to_char("That is not a container.\n");
	
	return;
}

void
look_room(CHAR_DATA *ch, int cmd)
{
	int dir = 0;
	char rsize[2] = { '\0' };
	ROOM_DATA *troom = NULL;
	ROOM_EXIT_DATA *texit = NULL;
	AFFECTED_TYPE *af = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char *bitbuf;
	char * blizzard_description =
	"\n   A howling blanket of wind-driven snow almost obscures your vision.\n";

	if (ch->room->room_size == ROOM_SIZE_DETAIL)
		strcpy(rsize, "S");
	else if (ch->room->room_size == ROOM_SIZE_EXPLORE)
		strcpy(rsize, "M");
	else if (ch->room->room_size == ROOM_SIZE_VALLEY)
		strcpy(rsize, "L");
	else if (ch->room->room_size == ROOM_SIZE_STORAGE)
		strcpy(rsize, "T");
	else 
		strcpy(rsize, "-");
	
		//show the room name first
	if ((!ch->get_trust()))
	{
		sprintf (buf, "#6%s [%s]#0", ch->room->name, rsize);
		ch->send_to_char(buf);
	}
	else
	{
		bitbuf = strdup(sprintbit (ch->room->room_flags, room_bits));
		
		snprintf (buf, BUFFER_1KiB, "#6%s [%s-%d]#0 #2[%d: %s #6%s#2]#0",
				  ch->room->name,
				  rsize,
				  ch->room->zone,
				  ch->room->nVirtual,
				  bitbuf,
				  terrain_types[ch->room->terrain_type]);
		
		ch->send_to_char(buf);
		
		
	}
	
		//show default exits
		// if it is snowing, they can't see the exits 
	if (((weather_info[ch->room->wzone].state != HEAVY_SNOW
		  || get_affect (ch, MAGIC_AFFECT_INFRAVISION)
		  || IS_SET (ch->affected_by, AFF_INFRAVIS))
		 || IS_SET (ch->room->room_flags, INDOORS))
		|| ch->get_trust())
		
	{
		sprintf (buf, "#6Exits:#0 ");
			//only shows the default simple exits
		for (dir = 0; dir <= LAST_DIR; dir++)
		{
				//is there any exit at all in this direction
			if (!is_exit(ch, dir))
				continue;
			
				//allow default exits only
			if (!ch->room->dir_option[dir])
				continue;
			
			texit = ch->room->dir_option[dir];
			
			if (texit->to_room
				&& (troom = vtor (texit->to_room))
				&& IS_SET (troom->room_flags, FALL))
				sprintf (buf + strlen (buf), "#1%s#0 ", dirs[dir]);
			
			else if (texit->to_room
					 && (troom = vtor (texit->to_room))
					 && (troom->terrain_type == SECT_RIVER
						 || troom->terrain_type == SECT_LAKE
						 || troom->terrain_type == SECT_OCEAN
						 || troom->terrain_type == SECT_REEF
						 || troom->terrain_type == SECT_UNDERWATER))
				sprintf (buf + strlen (buf), "#4%s#0 ", dirs[dir]);
			
			else if (texit->to_room 
					 && (troom = vtor (texit->to_room)) 
					 && (ch->room->terrain_type == SECT_ROAD 
						 || ch->room->terrain_type == SECT_TRAIL 
						 || IS_SET(ch->room->room_flags, ROAD)) 
					 && (troom->terrain_type == SECT_ROAD 
						 || troom->terrain_type == SECT_TRAIL 
						 || IS_SET(troom->room_flags, ROAD)))
				sprintf (buf + strlen (buf), "#3%s#0 ", dirs[dir]);
			
			else
				sprintf (buf + strlen (buf), "#2%s#0 ", dirs[dir]);
			
			
			if ((texit->type == PORTAL_DOOR)
				|| (texit->type == PORTAL_GATE))
			{
				
				if ((texit->keyword) && (str_cmp(texit->keyword, "(none)")))
				{
					sprintf (buf + strlen(buf), "(%s %s) ",
							 (IS_SET (texit->port_flags, EX_CLOSED)) 
							 ? "closed" : "open", texit->keyword);
				}
			}
			else
			{
				if ((texit->keyword) && (str_cmp(texit->keyword, "(none)")))
				{
				sprintf (buf + strlen (buf), "(%s) ", texit->keyword);
				}

			}

			
		}
		
		ch->send_to_char("\n");
		ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
	}
	else
	{
		ch->send_to_char("\n");
	}
	
		//if it is dark, they can't see the exits either
	if (is_dark(ch->room) && (!IS_SET (ch->affected_by, AFF_INFRAVIS)))
	{
		ch->send_to_char("#6It is too dark to see the exits.#0\n\n");
	}
	else
	{
		ch->send_to_char("\n");
	}
	
	
		//now for the room description
		//adjusted for time of day, weather descriptions
		//add a line for heavy snow if needed
	if (weather_info[ch->room->wzone].state == HEAVY_SNOW
		&& !IS_SET (ch->room->room_flags, INDOORS)
		&& (!ch->get_trust())
		&& (!get_affect (ch, MAGIC_AFFECT_INFRAVISION)
			&& !IS_SET (ch->affected_by, AFF_INFRAVIS)))
	{
			//added one line for HEAY_SNOW chagnes
		ch->send_to_char(room_get_description (ch->room));
		ch->send_to_char("\n");
		ch->send_to_char(blizzard_description);
	}
	else
	{
		ch->send_to_char(room_get_description (ch->room));
		ch->send_to_char("\n");
	}
	
		//show a line with weather effects
	if (!IS_SET (ch->room->room_flags, INDOORS)
		&& ch->room->terrain_type != SECT_UNDERWATER)
	{
			//cmd = 0 - normal view
			//cmd = 1 - weather and time added to room description
		look_room_weather(ch, cmd);

	}
	
	
		// Every time a room is looked at, check for weather objects that need be loaded.
	if(IS_OUTSIDE (ch))
	{
		load_weather_obj(ch->room);
	}
	
	list_light_to_char(ch->room, ch);
	list_portals_to_char(ch->room, ch);
	list_obj_to_char (ch->room->contents, ch, 0, false);
	list_char_to_char (ch->room->people, ch);
	
	*buf = '\0';
	
	for (af = ch->room->affects; af; af = af->next)
	{
		
	}
	
	if (*buf)
	{
		ch->send_to_char("\n");
		ch->send_to_char(buf);
	}
	
	return;
}

void 
look_room_weather (CHAR_DATA *ch, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	*buf = '\0';
		
	if (cmd == 0)
	{
		//special effects
	if (IS_SET (ch->room->room_flags, STIFLING_FOG))
		sprintf (buf + strlen (buf),
				 "   #6A stifling fog blankets the area, minimizing visibility.#0\n");
	
		//Rain/snow
	else if (weather_info[ch->room->wzone].state == CHANCE_RAIN)
		sprintf (buf + strlen (buf),
				 "   #6The air here carries the subtle aroma of impending rain.\n#0");
	else if (weather_info[ch->room->wzone].state == LIGHT_RAIN)
		sprintf (buf + strlen (buf),
				 "   #6A light, cool rain is falling here.#0\n");
	else if (weather_info[ch->room->wzone].state == STEADY_RAIN)
		sprintf (buf + strlen (buf),
				 "   #6A copious amount of rain is falling steadily here.#0\n");
	else if (weather_info[ch->room->wzone].state == HEAVY_RAIN)
		sprintf (buf + strlen (buf),
				 "   #6Rain falls in heavy sheets, inundating the area with water.#0\n");
	else if (weather_info[ch->room->wzone].state == LIGHT_SNOW)
		sprintf (buf + strlen (buf),
				 "   #6A light dusting of snow is falling in the area.#0\n");
	else if (weather_info[ch->room->wzone].state == STEADY_SNOW)
		sprintf (buf + strlen (buf),
				 "   #6Snow is falling steadily here, beginning to blanket the area.#0\n");
	else if (weather_info[ch->room->wzone].state == HEAVY_SNOW)
		sprintf (buf + strlen (buf),
				 "   #6Snow is falling heavily here, piling into drifts.#0\n");
	
		//cloudy effects
	else if (weather_info[ch->room->wzone].clouds == LIGHT_CLOUDS)
		sprintf (buf + strlen (buf),
				 "   #6A scattering of light clouds can be spotted.#0\n");
	else if (weather_info[ch->room->wzone].clouds == HEAVY_CLOUDS)
		sprintf (buf + strlen (buf),
				 "   #6Thick, heavy clouds obscure the sky.#0\n");
	else if (weather_info[ch->room->wzone].clouds == OVERCAST)
		sprintf (buf + strlen (buf),
				 "   #6The skies are lightly overcast.#0\n");
	
	
	/*** Special effects set by IMMs***/
	if (weather_info[ch->room->wzone].special_effect != NO_EFFECT)
	{
			//show effect specific message
		if (weather_info[ch->room->wzone].special_effect == VOLCANIC_SMOKE)
		{
			sprintf (buf + strlen (buf),
					 "   #6A cloud of thick, dust filled volcanic smoke drifts through the air.\n#0");
		}
		else if (weather_info[ch->room->wzone].special_effect == FOUL_STENCH)
		{
			sprintf (buf + strlen (buf),
					 "   #6A foul stench permeates the area.\n#0");
		}
		else if (weather_info[ch->room->wzone].special_effect == LOW_MIST)
		{
			sprintf (buf + strlen (buf),
					 "  #6A low, eerie mist sits heavily upon the land.\n#0");
		}
	}
	
	if (strlen (buf) > 2)
		ch->send_to_char(buf);
		ch->send_to_char("\n");
	}
	
	else if (cmd == 1)
	{
		command_interpreter(ch, "weather");
		ch->send_to_char("\n");
	}
}


int 
scan_weather_penalty(ROOM_DATA* room)
{
	int penalty = 0;
	
	if (weather_info[room->wzone].fog == THIN_FOG)
		penalty += 5;
	if (weather_info[room->wzone].fog == THICK_FOG)
		penalty += 10;
	if (weather_info[room->wzone].state == STEADY_RAIN)
		penalty += 5;
	if (weather_info[room->wzone].state == HEAVY_RAIN)
		penalty += 10;
	if (weather_info[room->wzone].state == STEADY_SNOW)
		penalty += 10;
	if (weather_info[room->wzone].state == HEAVY_SNOW)
		penalty += 15;
	
	return (penalty);
}

int
scan_distance_penalty (int dist)
{
	int penalty = 0;

		//two large rooms, 10 explore rooms, 40 detail rooms
	if (dist > 220 )
		penalty += 40;
	
		//1.5 large rooms, ~7 explore rooms, ~28 detail rooms
	else if (dist > 155)
		penalty += 30;
	
		//one large room, 5 explore rooms, 20 detail rooms
	else if (dist > 110)
		penalty += 25;
	
		//half of large room, 2.5 explore rooms, 10 detail rooms
	else if (dist > 55)
		penalty += 20;
	
		//20% of large room, 1 explore room, 4 detail rooms
	else if (dist > 22)
		penalty += 10;
	
		// 2 detail rooms
	else if (dist > 11)
		penalty += 5;
	
		// 1 detail rooms
	else if (dist > 0)
		penalty += 0;
	
	return (penalty);
}

	//room light level is tracked in candlepower, 
	//so we need to adjsut to something easier to work with for levels.
	//noon day sun is 13,000 candle power or 1.3x10^4, so it is light_level 4.1139
int 
scan_light_penalty(ROOM_DATA* current_room, ROOM_DATA* target_room)
{
	int penalty = 0;
	float curr_room_level;
	float targ_room_level;
	float light_mod;
	
	curr_room_level = log10(current_room->light);
	targ_room_level = log10(target_room->light);
	light_mod = (curr_room_level - targ_room_level);
	
		//target room ranges from a lot darker to a lot brighter 
		//easier to see things if the target room is brighter than current room
	if (light_mod >= 6)		
		penalty += 15;		// on 6 or 7
	else if (light_mod >= 4)
		penalty += 10;		//on 4 or 5
	else if (light_mod >= 2)
		penalty += 5;		//on 2 or 3
	else if (light_mod >= -1)
		penalty += 0;		//on -1, 0, or 1
	else if (light_mod >= -3)
		penalty -= 5;		// on -2 or -3
	else if (light_mod >= -5)
		penalty -= 10;		//on -4 or -5
	else if (light_mod >= -7)
		penalty -= -15;		//on -6 or -7
	else 
		penalty -= 25;
	
	return (penalty);
}

char*
time_phrase(int high_sun)
{					   
	char* phrase = NULL;						
	
	if (time_info.hour == 0)
		phrase = strdup("midnight");
	else if (time_info.hour == 1)
		phrase = strdup("after midnight");
	else if (time_info.hour > 1
			 && time_info.hour < (sunrise[time_info.month] - 1))
		phrase = strdup("late at night");
	else if (time_info.hour == (sunrise[time_info.month] - 1))
		phrase = strdup("before dawn");
	else if (time_info.hour == sunrise[time_info.month])
		phrase = strdup("dawn");
	else if (time_info.hour == (sunrise[time_info.month] + 1))
		phrase = strdup("after dawn");
	else if (time_info.hour > (sunrise[time_info.month] + 1)
			 && time_info.hour < (high_sun - 1))
		phrase = strdup("morning");
	else if (time_info.hour == (high_sun - 1))
		phrase = strdup("late morning");
	else if (time_info.hour == high_sun)
		phrase = strdup("high sun");
	else if (time_info.hour == (high_sun + 1))
		phrase = strdup("early afternoon");
	else if (time_info.hour > (high_sun + 1)
			 && time_info.hour < (sunset[time_info.month] - 2))
		phrase = strdup("afternoon");
	else if (time_info.hour == (sunset[time_info.month] - 2))
		phrase = strdup("late afternoon");
	else if (time_info.hour == (sunset[time_info.month] - 1))
		phrase = strdup("dusk");
	else if (time_info.hour == sunset[time_info.month])
		phrase = strdup("sunset");
	else if (time_info.hour == (sunset[time_info.month] +1)) 
		phrase = strdup("evening");
	else if (time_info.hour > (sunset[time_info.month] +1) 
			 && time_info.hour < 23)
		phrase = strdup("night time");
	else if (time_info.hour == 23)
		phrase = strdup("before midnight");
	
	return (phrase);
	
}

	//they are looking at a given keyword
void
look_portal_key(CHAR_DATA* targ_ch, char* arg)
{
	ROOM_PORTAL_DATA * tport;
	ROOM_EXIT_DATA* texit;
	int iter;
	
	if (!arg)
		return;
	
		//loop through all portals in this room and check for keyword
	for (iter = 0; iter <= MAX_PORTALS; iter++)
	{
		tport = vtop(targ_ch->room->portals[iter]);
		if (!tport)
			continue;
		
		if (tport->room_1 == targ_ch->room->nVirtual) 
			texit = exit_from_portal(tport->ident, tport->room_2);
		
		else if (tport->room_2 == targ_ch->room->nVirtual) 
			texit = exit_from_portal(tport->ident, tport->room_1);
		
		else 
			continue;

		
		if (isname(arg, texit->keyword))
			find_portal_description(texit, targ_ch);
				
	}
	
}