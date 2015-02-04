//////////////////////////////////////////////////////////////////////////////
//
/// staff.cpp : Staff Command Module
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
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>

#include </usr/local/mysql/include/mysql.h>

#include "server.h"
#include "structs.h"
#include "net_link.h"
#include "protos.h"
#include "account.h"
#include "utils.h"
#include "decl.h"
#include "sys/stat.h"
#include "utility.h"

extern std::map<std::string, SUBCRAFT_HEAD_DATA*> craft_map;
extern std::map<int, ROOM_PORTAL_DATA*> portal_map;
extern std::map<std::string, SKILL_DATA*> skill_data_map;
extern std::map<int, VARIANT_VALUE*> gl_variant;
extern std::map<std::string, CLAN_DATA*> clan_data_map;

extern rpie::server engine;
extern const char *room_bits[];
extern std::list<second_affect*> second_affect_list;
extern const char *dirs[];
extern const char *verbal_speeds[];


const char *player_bits[11] = {
	"Brief",
	"NoShout",
	"Compact",
	"DONTSET",
	"Quiet",
	"Reboot",
	"Shutdown",
	"Build",
	"Approval",
	"Outlaw",
	"\n"
};

const int SPHERE_COUNT = 1;

/* flag is 1 << index */
SPHERE_INFO spheres[1] = {  //ONLY ADD TO THE END OF THIS OR YOU WILL SCREW UP CURRENT FLAGS
	{"all",true},
};

void _do_load (CHAR_DATA * ch, char *argument, int cmd);

#define IMOTE_OPCHAR '^'

void
do_refresh (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	ROOM_PORTAL_DATA *tport;
	ROOM_DATA *troom;
	std::map<int, OBJ_DATA*>::iterator obj_iterator;
	std::map<int, ROOM_PORTAL_DATA*>::iterator port_iterator;
	std::map<int, ROOM_DATA*>::iterator room_iterator;
	
	argument = one_argument (argument, buf);
	
	sprintf(buf2, "\n");
	
			//player builder restrictions
	if (ch->get_trust() < 1)
	{
		ch->send_to_char ("You must be an authorised staff builder to use this command.\n");
		return;
	}
	
		//save all objects in-game, just in case of problems with new data
	save_player_rooms ();
		//save all PCs in case we diddle with their gear
	autosave();
	
	if ((!*buf) || (!str_cmp (buf, "?")))
	{
		ch->send_to_char ("\n\nSyntax:\n");
		ch->send_to_char ("     refresh mob | object | portal | room | all\n\n");
		ch->send_to_char ("Will re-load items from the current approved database tables.\n");
		ch->send_to_char ("This may adversly affect loaded items, if their proto-type has been deleted.\n");
		return;
	}
	
	else if (!str_cmp (buf, "mob"))
	{
		proto_mob_map.clear();
		boot_mobiles();
		sprintf(buf2 + strlen(buf2), "Mobiles have been refreshed\n");
	}
	
	else if (!str_cmp (buf, "room"))
	{
		room_iterator = room_map.begin();
		troom = room_iterator->second;
		while (troom)
		{
			room_map.erase(room_iterator);
			delete (troom);
			troom = NULL;
			room_iterator = room_map.begin();
			troom = room_iterator->second;
			if (room_map.size() == 0)
				break;
		}
		
		room_map.clear();
		boot_rooms();
		sprintf(buf2 + strlen(buf2), "Rooms have been refreshed\n");
		reload_char();
	}
	
	else if (!str_cmp (buf, "portal"))
	{
		port_iterator = portal_map.begin();
		tport = port_iterator->second;
		while (tport)
		{
			portal_map.erase(port_iterator);
			delete (tport);
			tport = NULL;
			port_iterator = portal_map.begin();
			tport = port_iterator->second;
			if (portal_map.size() == 0)
				break;
		}
		
		portal_map.clear();
		boot_portals();
		sprintf(buf2 + strlen(buf2), "Portals have been refreshed\n");
	}
	else if (!str_cmp (buf, "object"))
	{
		
		obj_iterator = proto_obj_map.begin();
		obj = obj_iterator->second;
		while (obj)
		{
			proto_obj_map.erase(obj_iterator);
			delete (obj);
			obj = NULL;
			obj_iterator = proto_obj_map.begin();
			obj = obj_iterator->second;
			if (proto_obj_map.size() == 0)
				break;
		}
		proto_obj_map.clear();
		boot_objects();
		sprintf(buf2 + strlen(buf2), "Objects have been refreshed\n");
	}
	
	else if (!str_cmp (buf, "all"))
	{
		command_interpreter(ch, "refresh object");				
		command_interpreter(ch, "refresh mob");
		command_interpreter(ch, "refresh room");
		command_interpreter(ch, "refresh portal");
		
		sprintf(buf2 + strlen(buf2), "EVERYTHING has been refreshed!\n");
	}
	
	send_to_gods(buf2);

	return;
	
	
}


void
do_roster (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch = NULL;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char admin[MAX_STRING_LENGTH]= { '\0' };
	char title[MAX_STRING_LENGTH]= { '\0' };
	char depart[MAX_STRING_LENGTH]= { '\0' };
	int weight = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (!*argument)
	{
		ch->send_to_char ("Usage: roster (add | remove) <admin name> (<title> <department>)\n Usage: roster list\n");
		return;
	}

	argument = one_argument (argument, buf);

	if (str_cmp (buf, "add") && str_cmp (buf, "remove") && str_cmp (buf, "list"))
	{
		ch->send_to_char ("Usage: roster (add | remove) <admin name> (<title> <department>)\n Usage: roster list\n");
		return;
	}

	if (!str_cmp (buf, "list"))
	{
		sprintf(buf2, "Name: Title (Department)\n--------------------------\n");
		ch->send_to_char (buf2);
		mysql_safe_query
			("SELECT name, title, dept FROM staff_roster");

		if ((result = mysql_store_result (database)) != NULL)
		{
			//row = mysql_fetch_row (result);
			while ((row = mysql_fetch_row (result)))
			{
				sprintf (buf2, "%s:  %s (%s)\n", row[0], row[1], row[2]);
				ch->send_to_char (buf2);
			}
		}

		mysql_free_result (result);
		return;
	}

	argument = one_argument (argument, admin);
	argument = one_argument (argument, title);
	argument = one_argument (argument, depart);


	if (!*admin)
	{
		ch->send_to_char ("Usage: roster (add | remove) <admin name> (<title> <department>)\n Usage: roster list\n");
		return;
	}

	if (!*admin && !*title && !*depart && !str_cmp (buf, "add"))
	{
		ch->send_to_char ("Usage: roster (add | remove) <admin name> (<title> <department>)\n Usage: roster list\n");
		return;
	}

	if (islower (*admin))
		*admin = toupper (*admin);

	if (islower (*title))
		*title = toupper (*title);

	if (islower (*depart))
		*depart = toupper (*depart);

	if (!str_cmp (buf, "add") && !(tch = load_pc (admin)))
	{
		ch->send_to_char ("That PC could not be found in our database.\n");
		return;
	}

	if (!str_cmp (buf, "add") && tch->pc->level < 3)
	{
		ch->send_to_char
			("All staff listed on the roster must be at least level 3.\n");
		return;
	}

	if (tch)
		tch->unload_pc();

	if (!str_cmp (buf, "add"))
	{
		mysql_safe_query ("INSERT INTO staff_roster VALUES ('%s', '%s',  %d, '%s', %d)",
			admin, title, (int) time (0), depart, weight);
		ch->send_to_char
			("The specified individual has been added to the staff roster.\n");
		return;
	}
	else if (!str_cmp (buf, "remove"))
	{
		mysql_safe_query ("DELETE FROM staff_roster WHERE name = '%s'", admin);
		ch->send_to_char
			("The specified individual has been removed from the roster.\n");
		return;
	}

	ch->send_to_char ("Usage: roster (add | remove) <admin name> (<title> <department>)\n Usage: roster list\n");
	return;
}


void
do_register (CHAR_DATA * ch, char *argument, int cmd)
{
	account *acct;
	unsigned int i = 0;
	char name[MAX_STRING_LENGTH]= { '\0' };
	char email[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (argument, name);
	argument = one_argument (argument, email);

	if (!*name || !*email)
	{
		ch->send_to_char ("Usage: register <new account name> <email address>\n");
		return;
	}

	if (strlen (name) > 36)
	{
		ch->send_to_char ("The account name must be 36 characters or less.\n");
		return;
	}

	if (!isalpha (*name))
	{
		ch->send_to_char
			("The first character of the account name MUST be a letter.\n");
		return;
	}

	for (i = 0; i < strlen (name); i++)
	{
		if (!isalpha (name[i]) && !isdigit (name[i]))
		{
			ch->send_to_char
				("Account names may only contain letters and numbers.\n");
			return;
		}
	}

	*name = toupper (*name);

	if (!str_cmp (name, "Anonymous") || !str_cmp (name, IMPLEMENTOR_ACCOUNT))
	{
		ch->send_to_char ("That account name cannot be registered.\n");
		return;
	}
	acct = new account (name);
	if (acct->is_registered ())
	{
		delete acct;
		ch->send_to_char ("That account name has already been taken.\n");
		return;
	}
	delete acct;

	if (!strstr (email, "@") || !strstr (email, "."))
	{
		ch->send_to_char
			("Email addresses must be given in the form of \'address@domain.com\'.\n");
		return;
	}

	new_accounts++;
	mysql_safe_query
		("UPDATE newsletter_stats SET new_accounts=new_accounts+1");

	acct = new account;

	acct->set_email (email);
	acct->set_name (name);
	acct->created_on = time (0);

	setup_new_account (acct);

	ch->send_to_char
		("The new account has been registered, and notification has been emailed.\n");
}

void
post_log (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *ch;
	char report[MAX_STRING_LENGTH]= { '\0' };

	ch = d->character;
	if (!d->pending_message->message)
	{
		ch->send_to_char ("No newsletter report posted.\n");
		return;
	}

	sprintf (report, "%s", d->pending_message->message);

	if (!str_cmp (ch->delay_who, "blog"))
	{
		mysql_safe_query ("INSERT INTO building_log VALUES ('%s', '%s', %d)",
			ch->name, report, (int) time (0));
		ch->send_to_char ("Your building report has been filed.\n");
		return;
	}
	else if (!str_cmp (ch->delay_who, "clog"))
	{
		mysql_safe_query ("INSERT INTO coding_log VALUES ('%s', '%s', %d)",
			ch->name, report, (int) time (0));
		ch->send_to_char ("Your coding report has been filed.\n");
		return;
	}
	else if (!str_cmp (ch->delay_who, "plog"))
	{
		mysql_safe_query ("INSERT INTO plot_log VALUES ('%s', '%s', %d)",
			ch->name, report, (int) time (0));
		ch->send_to_char ("Your plot report has been filed.\n");
		return;
	}
	else if (!str_cmp (ch->delay_who, "alog"))
	{
		mysql_safe_query ("INSERT INTO announcements VALUES ('%s', '%s', %d)",
			ch->name, report, (int) time (0));
		ch->send_to_char ("Your announcement has been filed.\n");
		return;
	}
	
	else
		ch->send_to_char ("There seems to be a problem with this command.\n");

	ch->delay_who = NULL;
	free_mem(d->pending_message);
	d->pending_message = NULL;
}

void
do_clog (CHAR_DATA * ch, char *argument, int cmd)
{
	ch->send_to_char
		("Enter a coding report to be included in the next newsletter:\n");

	ch->make_quiet();

	free_mem(ch->desc->descStr);
	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;

	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->delay_who = duplicateString ("clog");

	ch->desc->proc = post_log;
}

void
do_alog (CHAR_DATA * ch, char *argument, int cmd)
{
	ch->send_to_char
		("Enter a general staff announcement to be included in the next newsletter:\n");

	ch->make_quiet();

	free_mem(ch->desc->descStr);
	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;

	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->delay_who = duplicateString ("alog");

	ch->desc->proc = post_log;
}

void
do_blog (CHAR_DATA * ch, char *argument, int cmd)
{
	ch->send_to_char
		("Enter a building report to be included in the next newsletter:\n");

	ch->make_quiet();

	free_mem(ch->desc->descStr);
	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;

	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->delay_who = duplicateString ("blog");

	ch->desc->proc = post_log;
}

void
do_plog (CHAR_DATA * ch, char *argument, int cmd)
{
	ch->send_to_char
		("Enter a plot report to be included in the next newsletter:\n");

	ch->make_quiet();

	free_mem(ch->desc->descStr);
	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;

	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->delay_who = duplicateString ("plog");

	ch->desc->proc = post_log;
}

void
do_wclone (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	OBJ_DATA *fbook, *tbook;
	WRITING_DATA *fwriting, *twriting;

	argument = one_argument (argument, buf);
	fbook = get_obj_in_list_vis (ch, buf, ch->right_hand);
	if (!fbook)
		fbook = get_obj_in_list_vis (ch, buf, ch->left_hand);
	if (!fbook)
		fbook = get_obj_in_list_vis (ch, buf, ch->room->contents);

	if (!fbook)
	{
		ch->send_to_char ("Which written object did you wish to clone from?\n");
		return;
	}
	if (fbook->obj_flags.type_flag != ITEM_PARCHMENT
		&& fbook->obj_flags.type_flag != ITEM_BOOK)
	{
		ch->send_to_char
			("The object you want to clone from isn't a parchment or book.\n");
		return;
	}

	if (!fbook->writing_loaded)
		load_writing (fbook);

	argument = one_argument (argument, buf);
	tbook = get_obj_in_list_vis (ch, buf, ch->right_hand);
	if (!tbook)
		tbook = get_obj_in_list_vis (ch, buf, ch->left_hand);
	if (!tbook)
		tbook = get_obj_in_list_vis (ch, buf, ch->room->contents);

	if (!tbook)
	{
		ch->send_to_char ("Which written object did you wish to clone to?\n");
		return;
	}
	if (fbook->obj_flags.type_flag != ITEM_PARCHMENT
		&& fbook->obj_flags.type_flag != ITEM_BOOK)
	{
		ch->send_to_char
			("The object you want to clone to isn't a parchment or book.\n");
		return;
	}

	if (fbook->obj_flags.type_flag != tbook->obj_flags.type_flag)
	{
		ch->send_to_char
			("This command only works with two written objects of the same type.\n");
		return;
	}

	if (!fbook->writing)
	{
		ch->send_to_char ("The specified source has no writing in it.\n");
		return;
	}

	if (fbook == tbook)
	{
		ch->send_to_char ("Source and destination can't be the same.\n");
		return;
	}

	for (fwriting = fbook->writing; fwriting; fwriting = fwriting->next_page)
	{
		if (fwriting == fbook->writing)
		{
			if (!tbook->writing) {
				tbook->writing = new WRITING_DATA;
			}
			twriting = tbook->writing;
			twriting->message = duplicateString (fwriting->message);
			twriting->author = duplicateString (fwriting->author);
			twriting->date = duplicateString (fwriting->date);
			twriting->ink = duplicateString (fwriting->ink);
			twriting->script = fwriting->script;
			twriting->skill = fwriting->skill;
			twriting->torn = false;
			twriting->language = fwriting->language;
			twriting->next_page = NULL;
		}
		else
			for (twriting = tbook->writing; twriting;
				twriting = twriting->next_page)
			{
				if (!twriting->next_page)
				{
					twriting->next_page = new WRITING_DATA;
					twriting->next_page->message = duplicateString (fwriting->message);
					twriting->next_page->author = duplicateString (fwriting->author);
					twriting->next_page->date = duplicateString (fwriting->date);
					twriting->next_page->ink = duplicateString (fwriting->ink);
					twriting->next_page->script = fwriting->script;
					twriting->next_page->skill = fwriting->skill;
					twriting->next_page->torn = false;
					twriting->language = fwriting->language;
					twriting->next_page->next_page = NULL;
					break;
				}
			}
	}

	tbook->o.od.value[0] = fbook->o.od.value[0];

	if (fbook->book_title && *fbook->book_title)
	{
		tbook->book_title = duplicateString (fbook->book_title);
		tbook->title_skill = fbook->title_skill;
		tbook->title_script = fbook->title_script;
		tbook->title_language = fbook->title_language;
	}

	sprintf (buf, "Writing cloned from #2%s#0 to #2%s#0.",
		obj_short_desc (fbook), obj_short_desc (tbook));
	ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);

	save_writing (tbook);
}

void
do_wizlock (CHAR_DATA * ch, char *argument, int cmd)
{
	if (maintenance_lock)
	{
		unlink ("maintenance_lock");
	}
	else
	{
		system ("touch maintenance_lock");
	}
	
	ch->send_to_char ("Wizlock toggled.\n");
	
}

void
do_stayput (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (!*argument)
	{
		ch->send_to_char ("Toggle who's stayput flag?\n");
		return;
	}

	argument = one_argument (argument, buf);

	if (!(tch = get_char_room_vis (ch, buf)))
	{
		ch->send_to_char ("They aren't here.\n");
		return;
	}

	if (!IS_NPC (tch))
	{
		ch->send_to_char ("This command is for use on NPCs only.\n");
		return;
	}

	
	if (IS_NPC (tch) && IS_SET (tch->mob->action, ACT_STAYPUT))
	{
		ch->send_to_char ("This mobile will no longer save over reboots.\n");
		tch->mob->action &= ~ACT_STAYPUT;
		return;
	}
	else
	{
		ch->send_to_char ("This mobile will now save over reboots.\n");
		tch->mob->action |= ACT_STAYPUT;
		save_stayput_mobiles ();
		return;
	}
}

void
do_clockout (CHAR_DATA * ch, char *argument, int cmd)
{
	if (!IS_NPC (ch))
	{
		ch->send_to_char ("Eh?\n");
		return;
	}

	
	if (!ch->mob->shop)
	{
		ch->send_to_char ("This command is only for NPC shopkeepers.\n");
		return;
	}

	ch->flags &= ~FLAG_KEEPER;
	ch->send_to_char ("You will no longer sell things until you 'clockin'.\n");
}

void
do_clockin (CHAR_DATA * ch, char *argument, int cmd)
{
	if (!IS_NPC (ch))
	{
		ch->send_to_char ("Eh?\n");
		return;
	}

	if (!ch->mob->shop)
	{
		ch->send_to_char ("This command is only for NPC shopkeepers.\n");
		return;
	}

	ch->flags |= FLAG_KEEPER;
	ch->send_to_char ("You are now open for business again.\n");
}

void
do_roll (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };

	*buf = '\0';

	if (strncasecmp (argument, "vs ", 3) == 0)
	{
		argument = one_argument (argument, buf);
		argument = one_argument (argument, buf);

		if (buf[3] == 0)
		{
			//roll 1-18 vs ATTR: attribute of 10 will have a 50% chance of sucess
			
			int attr = 0;
			if (strcasecmp (buf, "str") == 0)
			{
				attr = ch->str;
			}
			else if (strcasecmp (buf, "dex") == 0)
			{
				attr = ch->dex;
			}
			else if (strcasecmp (buf, "con") == 0)
			{
				attr = ch->con;
			}
			else if (strcasecmp (buf, "int") == 0)
			{
				attr = ch->intel;
			}
			else if (strcasecmp (buf, "wil") == 0)
			{
				attr = ch->wil;
			}
			
			else if (strcasecmp (buf, "agi") == 0)
			{
				attr = ch->agi;
			}
			else if (strcasecmp (buf, "luk") == 0)
			{
				attr = ch->luk;
			}
			else //in case of doubt, make it 50/50
			{
				strcpy (buf, "???");
				attr = 10;
			}

			int roll = dice (1, MAX(18, attr));
			int diff = attr - roll;
			char output_ch [AVG_STRING_LENGTH] = "";
			char output_room [AVG_STRING_LENGTH] = "";

			sprintf (output_ch, "#6OOC: Rolling vs. %s... you ", buf);
			sprintf (output_room, "#6OOC: $n #6rolls vs. %s and ", buf);

			if (diff >= 0)
			{
				if (diff < 4)
				{
					strcat (output_ch, "just barely pass.#0\n");
					strcat (output_room, "just barely passes.#0");
				}
				else if (diff > 10)
				{
					strcat (output_ch, "pass gracefully.#0\n");
					strcat (output_room, "passes gracefully.#0");
				}
				else
				{
					strcat (output_ch, "pass.#0\n");
					strcat (output_room, "passes.#0");
				}
			}
			else
			{
				if (diff > -5)
				{
					strcat (output_ch, "fall short of passing.#0\n");
					strcat (output_room, "falls short of passing.#0");
				}
				else if (diff < -10)
				{
					strcat (output_ch, "fail miserably.#0\n");
					strcat (output_room, "fails miserably.#0");
				}
				else
				{
					strcat (output_ch, "fail.#0\n");
					strcat (output_room, "fails.#0");
				}
			}
			ch->send_to_char (output_ch);
			argument = one_argument (argument, buf);
			if (strcasecmp (buf, "ooc") == 0) {
				ch->act(output_room, false,  0, 0, TO_ROOM);
			}
			return;

		}
		
		else 
		{
			int ind = lookup_skill_id(buf);
			if (ind >= 0)
			{
				int roll = number (1, MAX(100, skill_level(ch, buf, 0)));
				int diff = skill_level (ch, buf, 0) - roll;

				char output_ch [AVG_STRING_LENGTH] = "";
				char output_room [AVG_STRING_LENGTH] = "";

				sprintf (output_ch, "#6OOC: Rolling vs. %s... you ", buf);
				sprintf (output_room, "#6OOC: $n #6rolls vs. %s and ", buf);

				if (diff >= 0)
				{
					if (diff < 10)
					{
						strcat (output_ch, "just barely pass.#0\n");
						strcat (output_room, "just barely passes.#0");
					}
					else if (diff > 50)
					{
						strcat (output_ch, "pass gracefully.#0\n");
						strcat (output_room, "passes gracefully.#0");
					}
					else
					{
						strcat (output_ch, "pass.#0\n");
						strcat (output_room, "passes.#0");
					}
				}
				else
				{
					if (diff > -10)
					{
						strcat (output_ch, "fall short of passing.#0\n");
						strcat (output_room, "falls short of passing.#0");
					}
					else if (diff < -50)
					{
						strcat (output_ch, "fail miserably.#0\n");
						strcat (output_room, "fails miserably.#0");
					}
					else
					{
						strcat (output_ch, "fail.#0\n");
						strcat (output_room, "fails.#0");
					}
				}
				ch->send_to_char (output_ch);
				argument = one_argument (argument, buf);
				if (strcasecmp (buf, "ooc") == 0) {
					ch->act(output_room, false,  0, 0, TO_ROOM);
				}
				return;
			}
		}

	}
	else
	{
		char * p, * ooc;
		int rolls = strtol (argument, &p, 0);
		int die = strtol ((*p)?(p+1):(p), &ooc, 0);

		if ((rolls > 0 && rolls <= 100)
			&& (die > 0 && die <= 1000)
			&& (*p == ' ' || *p == 'd'))
		{

			unsigned int total = dice (rolls, die);

			bool ooc_output = false;
			if (strcasecmp (ooc, " ooc") == 0)
			{
				ooc_output = true;
			}

			sprintf (buf, "#6OOC: Rolling %dd%d... #2%d#0\n", rolls, die, total);
			ch->send_to_char (buf);

			if (ooc_output)
			{
				sprintf (buf, "#6OOC: $n #6rolls %dd%d, the result is #E%d#6.#0", rolls, die, total);
				ch->act(buf, false,  0, 0, TO_ROOM);
			}
			return;
		}
	}

	ch->send_to_char ("Usage: 'roll XdY', equivalent to rolling XdY.\n"
		"X and Y may be a numbers from 1 to 100.\n"
		"Add 'ooc' at the end to inform the rest of the room.\n"
		"You can also 'roll vs <stat>' or 'roll vs <skill>'.\n");
	return;

}

void
do_deduct (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch;
	account *acct;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	bool loaded = false;
	
	if (!*argument)
    {
		ch->send_to_char ("Who would you like to deduct?\n");
		return;
    }
	
	if (IS_NPC (ch))
    {
		ch->send_to_char ("This is a PC-only command.\n");
		return;
    }
	
	argument = one_argument (argument, buf);
	
		/// \todo Only lookup PCs, as there may be an NPC with the same keyword
		/// Or better yet make this command lookup by account name
	if (!(tch = get_char_room_vis (ch, buf)) && !(tch = get_char_vis (ch, buf)))
    {
		if (!(tch = load_pc (buf)))
		{
			ch->send_to_char ("No PC with that name was found. "
						  "Is it spelled correctly?\n");
			return;
		}
		else
			loaded = true;
    }
	
	if (!tch->pc || !tch->pc->account_name || !tch->pc->account_name[0])
    {
		ch->send_to_char ("No PC with that name was found. "
					  "Is it spelled correctly?\n");
		return;
    }
	
	acct = new account (tch->pc->account_name);
	
	if (!acct->is_registered ())
    {
		delete acct;
		ch->act
		("Hmm... there seems to be a problem with that character's account. Please notify the staff.",
		 false, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
    }
	
	argument = one_argument (argument, buf);
	
	if (!isdigit (*buf))
    {
		delete acct;
		ch->send_to_char
		("You must specify a number of roleplay points to deduct.\n");
		return;
    }
	
	int deduction = strtol(buf, 0, 10);
	if (deduction < 1 || deduction > acct->get_rpp ())
    {
		delete acct;
		ch->act
		("The specified number must be greater than 0 and less than or equal to the named character's RP point total.",
		 false, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
    }
	
	acct->deduct_rpp (deduction);
	delete acct;
	
	if (loaded)
		tch->unload_pc();
	
	ch->send_to_char
    ("#1The specified number of points have been removed from their account.\n");
	ch->send_to_char
    ("Please be sure to leave a note explaining why on their pfile.#0\n");
	
	sprintf (buf, "write %s Roleplay Point Deduction.", tch->name);
	command_interpreter (ch, buf);
	
}

void
do_award (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	account *acct;
	bool loaded = false;
	
	if (!*argument)
    {
		ch->send_to_char ("Who would you like to award?\n");
		return;
    }
	
	if (IS_NPC (ch))
    {
		ch->send_to_char ("This is a PC-only command.\n");
		return;
    }
	
	argument = one_argument (argument, buf);
	
	if (!(tch = get_char_room_vis (ch, buf)) && !(tch = get_char_vis (ch, buf)))
    {
		if (!(tch = load_pc (buf)))
		{
			ch->send_to_char
			("No PC with that name was found. Is it spelled correctly?\n");
			return;
		}
		else
			loaded = true;
    }
	
	if (IS_NPC (tch))
    {
		ch->send_to_char ("This command only works for player characters.\n");
		return;
    }
	
	acct = new account (tch->pc->account_name);
	
	if (!acct->is_registered ())
    {
		delete acct;
		ch->act
		("Hmm... there seems to be a problem with that character's account. Please notify the staff.",
		 false, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
    }
	
	if (!str_cmp (tch->pc->account_name, ch->pc->account_name))
    {
		delete acct;
		ch->send_to_char
		("Adding roleplay points to your own account is against policy.\n");
		return;
    }
	
	if (tch->desc && tch->desc->acct
		&& IS_SET (tch->desc->acct->flags, ACCOUNT_RPPDISPLAY))
		tch->send_to_char
		("#6Congratulations, you've just been awarded a roleplay point!#0\n");
	acct->award_rpp ();
	delete acct;
	
	if (loaded)
		tch->unload_pc();
	
	ch->send_to_char
    ("#2A roleplay point has been added to that character's account.\n");
	ch->send_to_char
    ("Please be sure to leave a note explaining why on their pfile.#0\n");
	
	sprintf (buf, "write %s Roleplay Point.", tch->name);
	command_interpreter (ch, buf);
	
}


void
post_email (DESCRIPTOR_DATA * d)
{
	account *acct;
	char from[MAX_STRING_LENGTH]= { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (!d->pending_message->message)
	{
		d->character->send_to_char ("Email aborted.\n");
		d->pending_message = NULL;
		return;
	}

	sprintf (buf, "%s", d->pending_message->message);
	d->pending_message = NULL;

	sprintf (from, "%s <%s>", d->character->name, d->acct->email.c_str ());

	acct = new account (d->character->delay_who);

	send_email (acct, d->acct->email.c_str (), from, d->character->delay_who2,
		buf);

	delete acct;
	d->character->action &= ~PLR_QUIET;

	d->character->send_to_char
		("Your email has been sent out. It has been copied to your address for reference.\n");

	d->character->delay_who = NULL;
	d->character->delay_who2 = NULL;
}

void
do_email (CHAR_DATA * ch, char *argument, int cmd)
{
	account *acct;
	CHAR_DATA *tch;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char ("To whom did you wish to send an email?\n");
		return;
	}

	if (!(tch = load_pc (buf)))
	{
		ch->send_to_char ("That PC's file could not be loaded.\n");
		return;
	}
	acct = new account (tch->pc->account_name);
	if (!acct->is_registered ())
	{
		ch->send_to_char ("That character's account could not be loaded.\n");
		tch->unload_pc();
		return;
	}

	ch->delay_who = duplicateString (acct->name.c_str ());
	tch->unload_pc();

	if (!*argument)
	{
		ch->send_to_char
			("What would you like the subject-line of the email to be?\n");
		return;
	}

	if (IS_NPC (ch))
	{
		ch->send_to_char ("Only PCs may use this command.\n");
		return;
	}

	ch->send_to_char ("\n#2Enter the message you'd like to email this player:#0\n");

	free_mem(ch->desc->descStr);
	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;

	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;
	ch->desc->proc = post_email;
	ch->delay_who2 = duplicateString (argument);
	ch->action |= PLR_QUIET;

}

void
do_unban (CHAR_DATA * ch, char *argument, int cmd)
{
	SITE_INFO *site, *next_site;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int i;

	if (!*argument || !isdigit (*argument))
	{
		ch->send_to_char
			("You must specify the number of the site from the banned sitelist.\n");
		return;
	}

	if (!banned_site)
	{
		ch->send_to_char ("There are currently no sites to unban!\n");
		return;
	}

	for (i = 1, site = banned_site; site; i++, site = next_site)
	{
		next_site = site->next;
		if (atoi (argument) == 1 && i == 1)
		{
			if (ch->get_trust() < 5
				&& str_cmp (site->banned_by, "Password Security System")
				&& str_cmp (ch->name, site->banned_by))
			{
				sprintf (buf,
					"Sorry, but you'll need to get %s or a level 5 admin to do that.\n",
					site->banned_by);
				ch->send_to_char (buf);
				return;
			}
			unban_site (site);
			return;
		}
		else if (i + 1 == atoi (argument) && site->next)
		{
			if (ch->get_trust() < 5
				&& str_cmp (ch->name, site->next->banned_by))
			{
				sprintf (buf,
					"Sorry, but you'll need to get %s or a level 5 admin to do that.\n",
					site->banned_by);
				ch->send_to_char (buf);
				return;
			}
			unban_site (site->next);
			return;
		}
	}

	ch->send_to_char ("That number is not on the banned site list.\n");

}

bool is_banned (DESCRIPTOR_DATA * d);
void
disconnect_banned_hosts ()
{
	SITE_INFO *site;
	DESCRIPTOR_DATA *d, *next_desc;

	for (site = banned_site; site; site = site->next)
	{
		for (d = descriptor_list; d; d = next_desc)
		{
			next_desc = d->next;
			if (!d->acct)
				continue;
			if (!IS_SET (d->acct->flags, ACCOUNT_NOBAN))
			{
				if (strstr (d->strClientHostname, site->name))
				{
					close_socket (d);
				}
			}
		}
	}
}

void
ban_host (char *host, char *banned_by, int length)
{
	SITE_INFO *site, *tmp_site;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	site = new SITE_INFO;
	site->name = duplicateString (host);
	site->banned_by = duplicateString (banned_by);
	site->banned_on = time (0);
	site->next = NULL;

	if (length != -1 && length != -2)
		site->banned_until = time (0) + (60 * 60 * 24 * length);
	else if (length == -2)
		site->banned_until = time (0) + (60 * 60);
	else if (length == -1)
		site->banned_until = -1;

	if (!banned_site)
		banned_site = site;
	else
		for (tmp_site = banned_site; tmp_site; tmp_site = tmp_site->next)
		{
			if (!tmp_site->next)
			{
				tmp_site->next = site;
				break;
			}
		}

		sprintf (buf, "%s has sitebanned %s.\n", banned_by, host);
		send_to_gods (buf);
		system_log (buf, false);

		save_banned_sites ();
}

void
do_disconnect (CHAR_DATA * ch, char *argument, int cmd)
{
	DESCRIPTOR_DATA *d;

	if (!argument || !*argument)
	{
		ch->send_to_char ("Usage: disconnect <account>\n");
	}

	for (d = descriptor_list; d; d = d->next)
	{

		if (d->acct &&
			d->acct->name.length () && !strcmp (d->acct->name.c_str (), argument))
		{

			close_socket (d);

		}
	}
}


/*                                                                          *
* funtion: do_ban                      < e.g.> ban [ <host> <duration> ]   *
*                                                                          *
* 09/17/2004 [JWW] - was freeing the static char[] that asctime() returns! *
*                    now switched to using alloc, asctime_r, and free_mem  *
*                                                                          */
void
do_ban (CHAR_DATA * ch, char *argument, int cmd)
{
	SITE_INFO *site;
	MYSQL_RES *result;
	MYSQL_ROW row;
	char *start_date, *end_date;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char host[MAX_STRING_LENGTH]= { '\0' };
	char length[MAX_STRING_LENGTH]= { '\0' };
	char accounts[MAX_STRING_LENGTH]= { '\0' };
	unsigned int i, j;
	time_t ban_time, end_ban_time;

	if (!*argument)
	{
		sprintf (buf, "\n#6Currently Banned Sites:#0\n\n");
		if (!banned_site)
		{
			sprintf (buf + strlen (buf), "   None.\n");
			ch->send_to_char (buf);
			return;
		}
		else
			for (i = 1, site = banned_site; site; i++, site = site->next)
			{
				*accounts = '\0';
				if (site->name[0] == '^')
				{
					mysql_safe_query
						("SELECT user_name, account_flags FROM user WHERE user_last_ip LIKE '%s%%'",
						site->name + 1);
				}
				else
				{
					mysql_safe_query
					("SELECT user_name, account_flags FROM user WHERE user_last_ip LIKE '%s%%'",
						site->name);
				}
				result = mysql_store_result (database);
				j = 1;
				if (result)
				{
					if (mysql_num_rows (result) >= 10)
						sprintf (accounts + strlen (accounts),
						" #110+ accounts affected#0");
					else
						while ((row = mysql_fetch_row (result)))
						{
							if (IS_SET (atoi (row[1]), ACCOUNT_NOBAN))
								continue;
							if (j != 1)
								sprintf (accounts + strlen (accounts), ",");
							sprintf (accounts + strlen (accounts), " %s", row[0]);
							j++;
						}
						mysql_free_result (result);
				}
				if (i != 1)
					sprintf (buf + strlen (buf), "\n");
				sprintf (buf + strlen (buf), "   %d. #6%s#0 [%s]\n", i,
					site->name, site->banned_by);
				if (i > 9)
					sprintf (buf + strlen (buf), " ");
				if (*accounts)
					sprintf (buf + strlen (buf), "      #2Accounts:#0%s\n",
					accounts);
				else
					sprintf (buf + strlen (buf),
					"      #2Accounts:#0 None Found\n");

				ban_time = site->banned_on;
				start_date = new char[256];
				if (asctime_r (localtime (&ban_time), start_date) != NULL)
				{
					start_date[strlen (start_date) - 1] = '\0';
				}
				else
				{
					start_date[0] = '\0';
				}

				if (site->banned_until != -1)
				{
					if (i > 9)
						sprintf (buf + strlen (buf), " ");
					sprintf (buf + strlen (buf), "      #2Ban Period:#0 %s to ",
						start_date);

					end_ban_time = site->banned_until;
					end_date = new char[256];
					if (asctime_r (localtime (&end_ban_time), end_date) != NULL)
					{
						end_date[strlen (end_date) - 1] = '\0';
					}
					else
					{
						end_date[0] = '\0';
					}

					sprintf (buf + strlen (buf), "%s\n", end_date);
					free_mem (end_date);
					end_date = NULL;
				}
				else
				{
					if (i > 9)
						sprintf (buf + strlen (buf), " ");
					sprintf (buf + strlen (buf),
						"      #2Permanently Banned On:#0 %s\n", start_date);
				}
				free_mem (start_date);
				start_date = NULL;
			}
			page_string (ch->desc, buf);
			return;
	}

	argument = one_argument (argument, host);
	argument = one_argument (argument, length);

	for (i = 0; i < strlen (host); i++)
	{
		if (!isalpha (host[i]) && !isdigit (host[i])
			&& host[i] != '.' && host[i] != '_'
			&& host[i] != '-' && host[i] != '*' && host[i] != '^')
		{
			sprintf (buf, "Illegal character '%c' in siteban entry.\n",
				host[i]);
			ch->send_to_char (buf);
			return;
		}
	}

	if (!*length || (!isdigit (*length) && *length != '!'))
	{
		ch->send_to_char
			("You need to specify a length of days; otherwise, '!' for a permanent ban.\n");
		return;
	}

	if (*length != '!' && atoi (length) < 1)
	{
		ch->send_to_char ("The minimum ban period is 1 day.\n");
		return;
	}

	if (*length == '!')
		ban_host (host, ch->name, -1);
	else
		ban_host (host, ch->name, atoi (length));

	disconnect_banned_hosts ();
}

void
do_echo (CHAR_DATA * ch, char *argument, int cmd)
{
	int i;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char *result = NULL;

	for (i = 0; *(argument + i) == ' '; i++);

	if (!*(argument + i))
	{
		ch->send_to_char ("That must be a mistake...\n");
		return;
	}

	//filter string to process use of tokens such as * and ~
	result = swap_xmote_target (ch, argument, 3);

	if (!result)
	{
		return;
	}

	sprintf (buf, "%s", result);

	send_to_room (buf, ch->in_room);
}

void
do_gecho (CHAR_DATA * ch, char *argument, int cmd)
{
	ROOM_DATA *troom;
	std::map<int, ROOM_DATA*>::iterator room_iterator;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	
	if (!*argument)
		ch->send_to_char ("That must be a mistake...\n");
	else
	{
		sprintf (buf, "%s\n", argument);

		for (room_iterator = room_map.begin(); room_iterator != room_map.end(); room_iterator++)
		{
			troom = room_iterator->second;
			if (troom->people)
			{
				send_to_room (buf, troom->nVirtual);
			}
		}
	}
}

void
do_broadcast (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char *p;
	DESCRIPTOR_DATA *d;

	if (!*argument)
		ch->send_to_char ("That must be a mistake...\n");
	else
	{
		sprintf (buf2, "%s\n", argument);
		sprintf (buf, "\a\n#2Staff Announcement:#0\n\n");
		reformat_string (buf2, &p);
		sprintf (buf + strlen (buf), "%s", p);

		for (d = descriptor_list; d; d = d->next)
			SEND_TO_Q (buf, d);

	}
}

void
do_zecho (CHAR_DATA * ch, char *argument, int cmd)
{
	int echo_zone = -1;
	int first_room = -1;
	int last_room = -1;
	ROOM_DATA *room;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char mess[MAX_STRING_LENGTH]= { '\0' };
	std::map<int, ROOM_DATA*>::iterator room_iterator;
	
	argument = one_argument (argument, buf);
	
	if (!*buf || !strcmp (buf, "?"))
    {
		ch->send_to_char ("  zecho <zone> Message        send message to specific "
					  "zone.\n");
		ch->send_to_char ("  zecho Message               send message to zone you "
					  "are in.\n");
		ch->send_to_char ("  zecho r <first room num> <last room num> Message   send message to rooms in a range.\n");      
		return;
    }
	
	while (*argument == ' ')
		argument++;
	
	if (isdigit (*buf) && atoi (buf) < 100)
    {
		echo_zone = atoi (buf);
		sprintf(mess, "%s", argument);
    }
	else if (!strcmp(buf, "r"))
    {
		argument = one_argument (argument, buf);
		if (isdigit (*buf))
			first_room = atoi (buf);
		else
		{
			ch->send_to_char ("  zecho r <first room num> <last room num> Message   send message to rooms in a range.\n");      
			return;
		}
		
		argument = one_argument (argument, buf);
		if (isdigit (*buf))
			last_room = atoi (buf);
		else
		{
			ch->send_to_char ("  zecho r <first room num> <last room num> Message   send message to rooms in a range.\n");      
			return;
		}
		
		sprintf(mess, "%s", argument);  
    }
	else
    {
		echo_zone = vtor(ch->in_room)->zone;
		sprintf(mess, "%s", buf);
	}
	
	strcat (mess, "\n");
	
	if (echo_zone >= 0)
    {
		sprintf (buf, "(to zone %d): %s", echo_zone, mess);
		ch->send_to_char (buf);
    }
	else
    {
		sprintf (buf, "(to rooms %d  to %d): %s", first_room, last_room, mess);
		ch->send_to_char (buf);
    }
    
		
	for (room_iterator = room_map.begin(); room_iterator != room_map.end(); room_iterator++)
	{
		room = room_iterator->second;
		/******/
		
		if (echo_zone >= 0)
		{
			if (room->people && room->zone == echo_zone)
			{
				send_to_room (mess, room->nVirtual);
			}
        }
		else if ((first_room > 0) && (last_room > 0))
		{
			if ((room->people) &&
				(room->nVirtual >= first_room) &&
				(room->nVirtual <= last_room))
			{
				send_to_room (mess, room->nVirtual);
			}
		}
	}
	
}

void
do_pecho (CHAR_DATA * ch, char *argument, int cmd)
{

	CHAR_DATA *vict;
	DESCRIPTOR_DATA *d;
	char name[MAX_STRING_LENGTH]= { '\0' };
	char message[MAX_STRING_LENGTH]= { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };

	half_chop (argument, name, message);

	if (!*name || !*message)
		ch->send_to_char ("Who do you wish to pecho to and what??\n");

	else if (!(vict = get_char_nomask (name)) || IS_NPC (vict) ||
		(!vict->get_trust() && IS_SET (vict->flags, FLAG_WIZINVIS)))
		ch->send_to_char ("There is nobody playing the mud by that name.\n");

	else if (ch == vict)
		ch->send_to_char ("You try to tell yourself something.\n");

	else if (IS_SET (vict->action, PLR_QUIET))
	{
		ch->send_to_char ("That player is editing, try again later.\n");
		return;
	}

	else
	{
		if ((!ch->get_trust()) && IS_SET (vict->flags, FLAG_ANON))
		{
			ch->send_to_char ("There is nobody playing the mud by that name.\n");
			return;
		}

		if (!vict->desc && !IS_NPC (vict))
		{
			for (d = descriptor_list; d; d = d->next)
			{
				if (d == vict->pc->owner)
					break;
			}

			if (!d)
			{
				ch->send_to_char ("That player has disconnected.\n");
				return;
			}
		}
		sprintf (buf, "You pecho to %s: '%s#0'\n", vict->name, message);
		strcat (message, "#0\n");
		vict->send_to_char(message);
		ch->send_to_char(buf);
	}
}


void
do_transfer (CHAR_DATA * ch, char *argument, int cmd)
{
	DESCRIPTOR_DATA *i;
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int target;

	argument = one_argument (argument, buf);
	if (!*buf)
		ch->send_to_char ("Who do you wish to transfer?\n");
	else if (str_cmp ("all", buf))
	{
		if (!(victim = get_char_vis (ch, buf)))
			ch->send_to_char ("No-one by that name around.\n");
		else
		{
			victim->act("$n disappears in a mushroom cloud.", false, 0, 0,
				TO_ROOM);
			target = ch->in_room;
			victim->char_from_room();
			victim->char_to_room(target);
			victim->act("$n arrives from a puff of smoke.", false, 0, 0,
				TO_ROOM);
			ch->act("$n has transferred you!", false,  0, victim, TO_VICT);
			do_look (victim, "", 1);
		}
	}
	else
	{				/* Trans All */
		if (ch->get_trust() < 5)
		{
			ch->send_to_char
				("Sorry, but TRANSFER ALL is a level 5-only command.\n");
			return;
		}

		if (!*argument || *argument != '!')
		{
			ch->send_to_char ("Please type 'transfer all !' to confirm.\n");
			return;
		}

		for (i = descriptor_list; i; i = i->next)
		{
			if (i->character != ch && !i->connected)
			{
				victim = i->character;
				victim->act("$n disappears in a mushroom cloud.", false, 0, 0,
					TO_ROOM);
				target = ch->in_room;
				victim->char_from_room();
				victim->char_to_room(target);
				victim->act("$n arrives from a puff of smoke.", false, 0, 0,
					TO_ROOM);
				ch->act("$n has transferred you!", false, 0, victim, TO_VICT);
				do_look (victim, "", 1);
			}
		}

		ch->send_to_char
			("All online PCs have been transferred to your location.\n");
		return;
	}
}

void
do_at (CHAR_DATA * ch, char *argument, int cmd)
{
	char command[MAX_INPUT_LENGTH];
	char loc_str[MAX_INPUT_LENGTH];
	int location;
	int original_loc;
	CHAR_DATA *target_mob;
	OBJ_DATA *target_obj;
	ROOM_DATA *target_room;

	half_chop (argument, loc_str, command);

	if (!*loc_str)
	{
		ch->send_to_char ("You must supply a room number or a name.\n");
		return;
	}


	if (isdigit (*loc_str))
	{
		if (!(target_room = vtor (atoi (loc_str))))
		{
			ch->send_to_char ("No room exists with that number.\n");
			return;
		}

		location = target_room->nVirtual;
	}

	else if ((target_mob = get_char_vis (ch, loc_str)))
		location = target_mob->in_room;

	else if ((target_obj = get_obj_vis (ch, loc_str)))
		if (target_obj->in_room != NOWHERE)
			location = target_obj->in_room;
		else
		{
			ch->send_to_char ("The object is not available.\n");
			return;
		}
	else
	{
		ch->send_to_char ("No such creature or object around.\n");
		return;
	}

	/* a location has been found. */

	original_loc = ch->in_room;
	ch->char_from_room();
	ch->char_to_room(location);
	command_interpreter (ch, command);

	/* check if the guy's still there */

	for (target_mob = vtor (location)->people;
		target_mob; target_mob = target_mob->next_in_room)
		if (ch == target_mob)
		{
			ch->char_from_room();
			ch->char_to_room(original_loc);
		}
}

void
do_goto (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	int location;
	CHAR_DATA *target_mob, *tch;
	OBJ_DATA *target_obj;
	ROOM_DATA *troom;

	ch->clear_pmote();

	argument = one_argument (argument, buf);
	if (!*buf)
	{
		ch->send_to_char ("You must supply a room number or a name.\n");
		return;
	}

	if (isdigit (*buf))
	{

		if (!(troom = vtor (atoi (buf))))
		{
			ch->send_to_char ("No room exists with that number.\n");
			return;
		}

		location = troom->nVirtual;
	}

	else if ((target_mob = get_char_vis (ch, buf)))
		location = target_mob->in_room;

	else if ((target_obj = get_obj_vis (ch, buf)))
	{
		if (target_obj->in_room != NOWHERE)
			location = target_obj->in_room;
		else
		{
			ch->send_to_char ("The object is not available.\n");
			return;
		}
	}
	else
	{
		ch->send_to_char ("No such creature or object around.\n");
		return;
	}

	/* a location has been found. */

	if (ch->in_room == location)
	{
		ch->send_to_char ("You're already there.\n");
		return;
	}

	if (ch->pc && ch->pc->level < 2)
	{
		if (ch->room->zone != vtor(location)->zone)
		{
			ch->send_to_char ("I'm sorry. That room is outside of your zone.\n");
			return;
		}
	}
		
		
		
	if (ch->pc)
	{
		if (!ch->pc->imm_leave || !strncmp (ch->pc->imm_leave, "(null)", 6))
			ch->pc->imm_leave = duplicateString ("");

		if (!ch->pc->imm_enter || !strncmp (ch->pc->imm_enter, "(null)", 6))
		{
			ch->pc->imm_enter = duplicateString ("");
		}

		if (*ch->pc->imm_leave)
			ch->act(ch->pc->imm_leave, true, 0, 0, TO_ROOM | _ACT_FORMAT);
		else
			ch->act("$n leaves the area.", true, 0, 0, TO_ROOM);
	}
	else
		ch->act("$n leaves the area.", true, 0, 0, TO_ROOM);

	if (!IS_SET (ch->flags, FLAG_WIZINVIS) && *argument != '!')
	{

		troom = vtor (location);

		for (tch = troom->people; tch; tch = tch->next_in_room)
		{
			if (!IS_NPC (tch) && (!tch->get_trust()))
			{
				ch->send_to_char
					("You're currently visible to players. Use '!' to confirm the goto.\n");
				return;
			}
		}
	}

	ch->char_from_room();
	ch->char_to_room(location);

	if (ch->pc && ch->pc->imm_enter && ch->pc->imm_enter[0])
	{
		ch->act(ch->pc->imm_enter, true, 0, 0, TO_ROOM | _ACT_FORMAT);
	}
	else
	{
		ch->act("$n enters the area.", true, 0, 0, TO_ROOM);
	}
	do_look (ch, "", 1);
}

void
pad_buffer (char *buf, int pad_stop)
{
	int to_pad;
	char *p;

	p = buf + strlen (buf);

	for (to_pad = pad_stop - strlen (buf); to_pad > 0; to_pad--, p++)
		*p = ' ';

	*p = 0;
}


void
charstat (CHAR_DATA * ch, char *name, bool bPCsOnly)
{

	int i, j = 0;
	int i2 = 0;
	int loaded_char = 0;
	int loads = 0;
	int instance = 1;
	int tskill = 0;
	int skill_num = 0;
	float value_pay = 0.0;
	int time_diff = 0, hours_remaining = 0, days_remaining = 0, mins_remaining = 0;
	CHAR_DATA *k = NULL;
	CHAR_DATA *tch = NULL;
	OBJ_DATA *vobj = NULL;
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *next_affect;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char tbuf[MAX_STRING_LENGTH]= { '\0' };
	std::string tskill_name;
	char tclan_name[MAX_STRING_LENGTH];
	std::list<second_affect*>::iterator sa;
	std::list<char_data*>::iterator tch_iterator;
	std::set<std::string>::iterator materials_it;
	std::map<std::string, int> ::iterator skill_iterator;
	std::map<int, CHAR_DATA*>::iterator mob_iterator;
	std::map<std::string, std::string>::iterator clan_it;
	std::list<second_affect*>::iterator sa_it;
	second_affect* tsa;
	
	tskill_name.clear();
	
		//player builder restrictions
	if (ch->get_trust() < 1)
	{
		ch->send_to_char ("You must be an authorised staff builder to use this command.\n");
		return;
	}
	
	if (name == NULL || !*name)
	{
		ch->send_to_char ("Stat who?\n");
		return;
	}
	
	if (bPCsOnly)
	{
		
		if ((k = get_char_room_vis (ch, name)) && !IS_NPC (k))
		{
		}
		else if ((k = get_char_vis (ch, name)) && !IS_NPC (k))
		{
		}
		else if ((k = load_pc (name)))
		{
			loaded_char = 1;
		}
	}
	if (k == NULL)
	{
		if (atoi (name) && !strstr (name, "."))
		{
			
			if (!(k = vtom (atoi (name))))
			{
				ch->send_to_char ("No such mobile with that vnum.\n");
				return;
			}
			instance = 0;
			
		}
		else if ((k = get_char_room_vis (ch, name)) && IS_NPC (k))
		{
		}
		else if ((k = get_char_vis (ch, name)) && IS_NPC (k))
		{
		}
		else
		{
			
			for (mob_iterator = proto_mob_map.begin(); mob_iterator != proto_mob_map.end(); mob_iterator++)
			{
				tch = mob_iterator->second;
				
				if (!*tch->keywords)
					continue;
				
				if (strstr (tch->keywords, name))
				{
					k = tch;
				}
			}
			
		}
		if (ch->get_trust() >= 3)
		{
			if (k == NULL && !bPCsOnly)
			{
				charstat (ch, name, true);
			}
		}
	}
	
	if (k == NULL)
	{
		ch->send_to_char ("No player or mobile with that name found.\n");
		return;
	}
	
	
	if (!IS_NPC (k))
		sprintf (buf, "\n#2Name:#0   %s, a %s PC", k->name,
				 sex_types[(int) k->sex]);
	else
		sprintf (buf, "\n#2VNUM:#0   %d, a %s %s", k->mob->nVirtual,
				 sex_types[(int) k->sex],
				 instance ? "[INSTANCE]" : "[PROTOTYPE]");
	
	if (IS_NPC (k))
	{
		loads = 0;
		
		for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
		{
			tch = *tch_iterator;
			if (tch->deleted || !IS_NPC (tch))
				continue;
			
			if (tch->mob->nVirtual == k->mob->nVirtual)
				loads++;
		}
		sprintf (buf + strlen (buf), " [Loaded Instances: %d]", loads);
	}
	else if (k->pc && k->pc->account_name)
	{
		sprintf (buf + strlen (buf), ", registered to account %s", k->pc->account_name);
	}
	
	sprintf (buf + strlen (buf), " [id %d]\n", k->coldload_id);
	
	ch->send_to_char (buf);
	
	if (IS_NPC(k))
	{
		sprintf (buf, "#2Zone: #0  %d\n", k->mob->zone);	
		ch->send_to_char (buf);
	}
	
	sprintf (buf, "#2Keys:#0   %s\n", k->keywords);
	
	sprintf (buf + strlen (buf), "#2SDesc:#0  %s\n",
			 k->short_descr ? k->short_descr : "None");
	
	sprintf (buf + strlen (buf), "#2LDesc:#0  %s\n",
			 k->long_descr ? k->long_descr : "None");
	
	sprintf(buf + strlen(buf), "#2Full Description:#0\n%s\n",k->description ? k->description : "None" );
	ch->send_to_char (buf);
	
	sprintf (buf, "#2Str:#0 %d/%d", k->tmp_str, k->str);
	pad_buffer (buf, 25);
	sprintf (buf + strlen (buf), "#2MaxHP:#0 %d", k->max_hit);
	pad_buffer (buf, 53);
	if (instance)
		sprintf (buf + strlen (buf), "#2Moves:#0  %d/%d", k->move, k->max_move);
	else
		sprintf (buf + strlen (buf), "#2Moves:#0  N/A");
	ch->send_to_char (buf);
	ch->send_to_char ("\n");
	
	sprintf (buf, "#2Dex:#0 %d/%d", k->tmp_dex, k->dex);
	pad_buffer (buf, 25);
	sprintf (buf + strlen (buf), "#2Race:#0  %s", lookup_race_variable (k->race, RACE_NAME));
	pad_buffer (buf, 53);
	sprintf (buf + strlen (buf), "#2Speak:#0  %s", k->speaks);
	ch->send_to_char (buf);
	ch->send_to_char ("\n");
	
	sprintf (buf, "#2Con:#0 %d/%d", k->tmp_con, k->con);
	pad_buffer (buf, 25);
	sprintf (buf + strlen (buf), "#2Age:#0   %dY %dM %dD %dH",
			 age(k).year, age(k).month, age(k).day, age(k).hour);
	pad_buffer (buf, 53);
	if (instance)
		sprintf (buf + strlen (buf), "#2Ht/Wt:#0  %d/%d", k->height, (k->get_weight()/100));
	else
		sprintf (buf + strlen (buf), "#2Ht/Wt:#0  N/A");
	ch->send_to_char (buf);
	ch->send_to_char ("\n");
	
	sprintf (buf, "#2Int:#0 %d/%d", k->tmp_intel, k->intel);
	pad_buffer (buf, 25);
	sprintf (buf + strlen (buf), "\n");
	ch->send_to_char (buf);
	
	sprintf (buf, "#2Wil:#0 %d/%d", k->tmp_wil, k->wil);
	pad_buffer (buf, 25);
	sprintf (buf + strlen (buf), "#2Armor:#0 %d", k->armor);
	pad_buffer (buf, 53);
	sprintf (buf + strlen (buf), "#2Speed:#0  %s", verbal_speeds[k->speed]);
	sprintf (buf + strlen (buf), "\n");
	ch->send_to_char (buf);
	
	sprintf (buf, "#2Agi:#0 %d/%d", k->tmp_agi, k->agi);
	pad_buffer (buf, 25);
	sprintf (buf + strlen (buf), "#2Room:#0  %d", k->in_room);
	pad_buffer (buf, 53);
	if (IS_NPC (k))
	{
		sprintf (buf + strlen (buf), "#2Spwn:#0   %d", k->mob->spawnpoint);
	}
	else
	{
		sprintf (buf + strlen (buf), "#2Cond:#0   %d,%d,%d",
				 k->intoxication, k->hunger, k->thirst);
	}
	sprintf (buf + strlen (buf), "\n");
	ch->send_to_char (buf);
	
	
	
	if (IS_NPC (k))
	{
		sprintf (buf, "#2Luk:#0 %d/%d", k->tmp_luk, k->luk);
		pad_buffer (buf, 25);
		pad_buffer (buf, 53);
		sprintf (buf + strlen (buf), "#2NDam:#0   %dd%d", k->mob->damnodice,
				 k->mob->damsizedice);
		sprintf (buf + strlen (buf), "   #2Delay:#0  %d", k->delay);
		sprintf (buf + strlen (buf), "\n");
		ch->send_to_char (buf);
		
		
		sprintf (buf, "#2Pow:#0 %d", k->aur);
		pad_buffer (buf, 25);
		pad_buffer (buf, 53);
		sprintf (buf + strlen (buf), "\n");
		ch->send_to_char (buf);
		
	}
	else if (k->pc)
	{
		*buf = '\0';
		sprintf (buf, "#2Luk:#0 %d/%d", k->tmp_luk, k->luk);
		pad_buffer (buf, 25);
		if (k->pc->level > 5)
		{
			strcat (buf, "#2Level:#0 Implementor");
		}
		else
		{
			sprintf (buf + strlen (buf), "#2Level:#0 %d", k->pc->level);
		}
		sprintf (buf + strlen (buf), "\n");
		ch->send_to_char (buf);
		
		sprintf (buf, "#2Pow:#0 %d", k->aur);
		sprintf (buf + strlen (buf), "\n");
		ch->send_to_char (buf);
	}
	
	sprintf (buf, "\n#2Clans:#0\n");
	int counter;
	std::string tclan;
	counter = k->char_clan_map.size();
	if (counter > 0)
	{
		for (clan_it = k->char_clan_map.begin(); clan_it !=  k->char_clan_map.end(); clan_it++)
		{
			if (clan_data_map.find(clan_it->first)!=clan_data_map.end())
			{
				tclan = clan_it->second;
				counter --;
				if(!tclan.empty())
				{
					sprintf(tclan_name, "%s", clan_it->first.c_str());
					
					sprintf(buf2, "%s", clan_data_map[tclan_name]->literal);
					sprintf(buf + strlen(buf), "[%s - %s]\n", buf2, clan_it->second.c_str());
				}
				if(counter == 0)
					break;
			}
		}
	}
	sprintf(buf + strlen(buf), "\n");
	ch->send_to_char (buf);

	
	sprintf (buf, "#2States:#0 [%s", position_types[k->get_position()]);
	
	if (k->intoxication > 0)
		sprintf (buf + strlen (buf), ", drunk");
	
	if (!k->hunger)
		sprintf (buf + strlen (buf), ", hungry");
	
	if (!k->thirst)
		sprintf (buf + strlen (buf), ", thirsty");
	
	if (!IS_NPC (k))
	{
		switch (k->pc->create_state)
		{
			case STATE_REJECTED:
				sprintf (buf + strlen (buf), ", REJECTED");
				break;
			case STATE_APPLYING:
				sprintf (buf + strlen (buf), ", APPLYING");
				break;
			case STATE_SUBMITTED:
				sprintf (buf + strlen (buf), ", SUBMITTED");
				break;
			case STATE_APPROVED:
				sprintf (buf + strlen (buf), ", APPROVED");
				break;
			case STATE_SUSPENDED:
				sprintf (buf + strlen (buf), ", SUSPENDED");
				break;
			case STATE_DIED:
				sprintf (buf + strlen (buf), ", DIED");
				break;
			case STATE_RETIRED:
				sprintf (buf + strlen (buf), ", RETIRED");
				break;
		}
	}
	
	if (GET_FLAG (k, FLAG_ENTERING))
		sprintf (buf + strlen (buf), ", ENTERING");
	
	if (GET_FLAG (k, FLAG_LEAVING))
		sprintf (buf + strlen (buf), ", LEAVING");
	
	sprintf (buf + strlen (buf), "]\n");
	ch->send_to_char (buf);
	
	
	if (k->mob && k->mob->access_flags)
	{
		
		sprintf (buf, "Access: [");
		
		for (i = 0; *room_bits[i] != '\n'; i++)
			if (IS_SET (k->mob->access_flags, 1 << i))
				sprintf (buf + strlen (buf), "%s ", room_bits[i]);
		
		strcat (buf, "]\n");
		ch->send_to_char (buf);
	}
	
	if (k->mob && k->mob->noaccess_flags)
	{
		
		sprintf (buf, "Noaccess: [");
		
		for (i = 0; *room_bits[i] != '\n'; i++)
			if (IS_SET (k->mob->noaccess_flags, 1 << i))
				sprintf (buf + strlen (buf), "%s ", room_bits[i]);
		
		strcat (buf, "]\n");
		ch->send_to_char (buf);
	}
	
	sprintf (buf, "#2Flags:#0  [");
	
	if (!IS_NPC (k))
	{
		for (i = 0; *player_bits[i] != '\n'; i++)
			if (IS_SET (k->action, 1 << i))
				sprintf (buf + strlen (buf), " %s", player_bits[i]);
		
	}
	else
	{
		for (i = 0; *action_bits[i] != '\n'; i++)
			if (IS_NPC (k) && IS_SET (k->mob->action, 1 << i))
				sprintf (buf + strlen (buf), " %s", action_bits[i]);
		
		for (i = 0; *profession_bits[i] != '\n'; i++)
			if (IS_NPC(k) && IS_SET (k->mob->profession, 1 << i))
				sprintf (buf + strlen (buf), " #3%s#0", profession_bits[i]);
		
	}
	
	if (GET_FLAG (k, FLAG_GUEST))
		strcat (buf, " Guest");
	
	if (IS_SET (k->plr_flags, NOPETITION))
		strcat (buf, " NoPetition");
	
	if (GET_FLAG (k, FLAG_BRIEF))
		strcat (buf, " Brief");
	
	if (GET_FLAG (k, FLAG_WIZINVIS))
		strcat (buf, " Wizinvis");
	
	if (GET_FLAG (k, FLAG_FLEE))
		strcat (buf, " Fleeing");
	
	if (GET_FLAG (k, FLAG_BINDING))
		strcat (buf, " Binding");
	
	if (GET_FLAG (k, FLAG_VARIABLE))
		strcat (buf, " Variable");
	
	if (GET_FLAG (k, FLAG_PACIFIST))
		strcat (buf, " Pacifist");
	
	if (IS_SET (k->plr_flags, NEWBIE_HINTS))
		strcat (buf, " Hints");
	
	if (IS_SET (k->plr_flags, MENTOR))
		strcat (buf, " Mentor");
	
	if (IS_SET (k->plr_flags, NEWBIE))
		strcat (buf, " Newbie");
	
	if (IS_SET (k->plr_flags, NEWBCHAT))
		strcat (buf, " Newbiechat");
	
	
	if (GET_FLAG (k, FLAG_SEE_NAME))
		strcat (buf, " See_name");
	
	if (GET_FLAG (k, FLAG_AUTOFLEE))
		strcat (buf, " Autoflee");
	
	if (GET_FLAG (k, FLAG_NOPROMPT))
		strcat (buf, " Noprompt");
	
	
	if (GET_FLAG (k, FLAG_ISADMIN))
		strcat (buf, " IsAdminPC");
	
	if (!IS_NPC (k) && k->pc->is_guide)
		strcat (buf, " IsGuide");
	
	
	if (IS_SET(k->plr_flags, IS_CRAFTER))
		strcat (buf, " IsCrafter");

	if (k->color)
		strcat (buf, " ANSI");
	
	if (IS_NPC(k))
	{
		
		if (IS_SET (k->mob->action, ACT_NOBIND))
			strcat (buf, " NoBind");
		
		if (IS_SET (k->mob->action, ACT_NOBLEED))
			strcat (buf, " NoBleed");
		
		if (IS_SET (k->mob->action, ACT_FLYING))
			strcat (buf, " Flying");
		
		if (IS_SET (k->mob->profession, PROF_PHYSICIAN))
			strcat (buf, " Physician");
		
		if (IS_SET (k->mob->profession, PROF_REPAIR)) 
			strcat (buf, " Repairman");
		
		if (IS_SET (k->mob->profession, PROF_PREY))
			strcat (buf, " Prey");	
	}
	
	sprintf (buf + strlen (buf), " ]\n");
	ch->send_to_char (buf);
	
	if (IS_NPC (k) && k->mob->owner)
	{
		sprintf (buf, "#2Owner:#0  [%s]\n", k->mob->owner);
		ch->send_to_char (buf);
	}
	
	if (IS_NPC (k) && IS_SET (k->mob->profession, PROF_ENFORCER))
	{
		sprintf (buf, "#2Jail Location:#0 [ %d ]\n", k->mob->jail);
		ch->send_to_char (buf);
	}
	
	if (IS_NPC (k) && IS_SET (k->mob->profession, PROF_JAILER))
	{
		sprintf (buf, "#2Jail Info:#0 [ Cell1: %d Cell2: %d Cell3: %d ].\n",
				 k->mob->cell_1, k->mob->cell_2, k->mob->cell_3);
		ch->send_to_char (buf);
	}
	
	if (k->desc && k->desc->snoop.snooping &&
		is_he_somewhere (k->desc->snoop.snooping))
	{
		sprintf (buf, "#2Snooping:#0  %s\n",
				 (k->desc->snoop.snooping)->name);
		ch->send_to_char (buf);
	}
	
	if (k->desc && k->desc->snoop.snoop_by &&
		is_he_somewhere (k->desc->snoop.snoop_by))
	{
		sprintf (buf, "#2Snooped by:#0  %s\n",
				 (k->desc->snoop.snoop_by)->name);
		ch->send_to_char (buf);
	}
	
	if (k->following && is_he_somewhere (k->following))
	{
		sprintf (buf, "#2Following:#0 %s\n", (k->following)->name);
		ch->send_to_char (buf);
	}
	
	if (k->affected_by)
	{
		sprintf (buf, "#2Affect:#0 [ ");
		for (i = 0; *affected_bits[i] != '\n'; i++)
			if (IS_SET (k->affected_by, 1 << i))
				sprintf (buf + strlen (buf), "%s ", affected_bits[i]);
		sprintf (buf + strlen (buf), "]\n");
		ch->send_to_char (buf);
	}
	
	
	for (af = k->hour_affects; af; af = next_affect)
	{
		
		if (af == k->hour_affects)
			ch->send_to_char ("\n");
		
		next_affect = af->next;	/* WatchX affect can delete af */
		
		if (af->type == AFFECT_SHADOW)
		{
			
			if (!af->a.shadow.shadow && af->a.shadow.edge != -1)
				sprintf (buf, "#2%5d#0   Standing", af->type);
			
			else if (!is_he_somewhere (af->a.shadow.shadow))
				continue;
			
			else if (IS_NPC (af->a.shadow.shadow))
				sprintf (buf, "#2%5d#0   Shadowing %s (%d)", af->type,
						 af->a.shadow.shadow->short_descr,
						 af->a.shadow.shadow->mob->nVirtual);
			else
				sprintf (buf, "#2%5d#0   Shadowing PC %s", af->type,
						 (af->a.shadow.shadow)->name);
			
			if (af->a.shadow.edge != -1)
				sprintf (buf + strlen (buf), " on %s edge.",
						 dirs[af->a.shadow.edge]);
			
			sprintf (buf + strlen (buf), "\n");
			
			ch->send_to_char (buf);
			
			continue;
		}
		
		
		else if (af->type == MAGIC_SIT_TABLE)
		{
			sprintf (buf, "#2%5d#0   Sitting at table (%s).\n", af->type, af->a.table.obj->short_description);
			ch->send_to_char (buf);
			continue;
		}
		
				
		if (af->type == MUTE_EAVESDROP)
		{
			sprintf (buf, "#2%5d#0    Muting Eavesdropping\n", af->type);
			ch->send_to_char (buf);
			continue;
		}
		
		if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST)
		{
			continue;
		}
		
		if (af->type >= JOB_1 && af->type <= JOB_3)
		{
			
			i = time_info.accum_days - af->a.job.pay_date;
			
			vobj = vtoo (af->a.job.object_vnum);
			
			if (!vobj)
				value_pay = af->a.job.cash;
			else
			{
				value_pay = vobj->coppers * af->a.job.count;
			}
			
			sprintf (buf,
					 "#2%5d#0   Job %d:  Pays %.0f coppers with %d of %d days until next normal payday\n",
					 af->type,
					 af->type - JOB_1 + 1,
					 value_pay,
					 i,
					 af->a.job.days);
			
			sprintf (buf + strlen(buf),
					 "             %d x %s (%d)",
					 af->a.job.count,
					 !vobj ? "UNDEFINED" : vobj->short_description,
					 af->a.job.object_vnum
					 );
			
			if ((af->a.job.employer) && (tch = vtom(af->a.job.employer)))
			{
				sprintf (buf + strlen(buf),
						 "\n             Employer: %s (%d)\n",
						 vtom(af->a.job.employer)->short_descr,
						 af->a.job.employer);
			}
			else
			{
				sprintf (buf+ strlen(buf),"\n");
			}
			
			ch->send_to_char (buf);
			continue;
		} //if (af->type >= JOB_1
		
		if (af->type == MAGIC_CRAFT_BRANCH_STOP)
		{
			sprintf (buf,
					 "#2%5d#0   May branch a craft again in %d RL minutes.\n",
					 af->type, af->a.spell.duration);
			ch->send_to_char (buf);
			continue;
		}
		
		if (af->type == MAGIC_PETITION_MESSAGE)
		{
			sprintf (buf,
					 "#2%5d#0   Will not receive petition message again for %d RL minutes.\n",
					 af->type, af->a.spell.duration);
			ch->send_to_char (buf);
			continue;
		}
		
		if (af->type == AFFECT_LOST_CON)
		{
			sprintf (buf,
					 "#2%5d#0   Will regain %d CON points in %d in-game hours.\n",
					 af->type, af->a.spell.sn, af->a.spell.duration);
			ch->send_to_char (buf);
			continue;
		}
		
				
		if (af->type == MAGIC_CRAFT_DELAY)
		{
			if (time (0) >= af->a.spell.modifier)
				continue;
			time_diff = af->a.spell.modifier - time (0);
			days_remaining = time_diff / (60 * 60 * 24);
			time_diff %= 60 * 60 * 24;
			hours_remaining = time_diff / (60 * 60);
			time_diff %= 60 * 60;
			mins_remaining = time_diff / 60;
			time_diff %= 60;
			sprintf (buf,
					 "#2%5d#0   OOC craft delay timer engaged. [%dd %dh %dm %ds remaining]\n",
					 af->type, days_remaining, hours_remaining, mins_remaining,
					 time_diff);
			ch->send_to_char (buf);
			continue;
		}
		
		if (af->type == MAGIC_SKILL_GAIN_STOP
			&& (ch->get_trust() > 4))
		{
			sprintf (buf, "#2%5d#0   May improve a skill again in %d RL minutes.\n",
					 af->type, af->a.spell.duration);
			ch->send_to_char (buf);
			continue;
		}
		
		
		if (af->type == MAGIC_RAISED_HOOD)
		{
			sprintf (buf,
					 "#2%5d#0   Mob has raised hood due to inclement weather.\n",
					 af->type);
			ch->send_to_char (buf);
			continue;
		}
		
		if (af->type >= MAGIC_CRIM_BASE && af->type <= MAGIC_CRIM_BASE + 100)
		{
			sprintf (buf, "#2%5d#0   Wanted in zone %d for %d hours.\n",
					 af->type, af->type - MAGIC_CRIM_BASE,
					 af->a.spell.duration);
			ch->send_to_char (buf);
			continue;
		}
		
		if (af->type == MAGIC_NOTIFY)
		{
			if (is_he_somewhere ((CHAR_DATA *) af->a.spell.t))
			{
				sprintf (buf, "#2%5d#0   Notify pending on %s for %d hours.\n",
						 af->type,
						 ((CHAR_DATA *) af->a.spell.t)->name,
						 af->a.spell.duration);
				ch->send_to_char (buf);
			}
			
			continue;
		}
		
		if (af->type == MAGIC_CLAN_NOTIFY)
		{
			if (is_he_somewhere ((CHAR_DATA *) af->a.spell.t))
			{
				sprintf (buf,
						 "#2%5d#0   CLAN Notify pending on %s for %d hours.\n",
						 af->type, ((CHAR_DATA *) af->a.spell.t)->name,
						 af->a.spell.duration);
				ch->send_to_char (buf);
			}
			
			continue;
		}
		
				
		if (af->type >= MAGIC_CRIM_HOODED && af->type < MAGIC_CRIM_HOODED + 100)
		{
			sprintf (buf,
					 "#2%5d#0   Hooded criminal charge in zone %d for %d RL seconds.\n",
					 af->type, af->type - MAGIC_CRIM_HOODED,
					 af->a.spell.duration);
			ch->send_to_char (buf);
			continue;
		}
		
		if (af->type == MAGIC_STARED)
		{
			sprintf (buf,
					 "#2%5d#0   Studied by an enforcer.  Won't be studied for "
					 "%d RL seconds.\n", af->type, af->a.spell.duration);
			ch->send_to_char (buf);
			continue;
		}
		
		if (af->type == MAGIC_WARNED)
		{
			sprintf (buf,
					 "#2%5d#0   Warned by an enforcer.  Won't be warned for "
					 "%d RL seconds.\n", af->type, af->a.spell.duration);
			ch->send_to_char (buf);
			continue;
		}
		
		if (af->type == MAGIC_HIDDEN)
		{
			sprintf (buf, "#2%5d#0   Hidden.\n", af->type);
			ch->send_to_char (buf);
			continue;
		}
		
		if (af->type == MAGIC_SNEAK)
		{
			sprintf (buf, "#2%5d#0   Currently trying to sneak.\n", af->type);
			ch->send_to_char (buf);
			continue;
		}
		
		
		if (af->type >= MAGIC_AFFECT_FIRST && af->type <= MAGIC_AFFECT_LAST)
		{
			sprintf (buf, "#2%5d#0   Spell effect, %d RL minutes remaining.\n",
					 af->type, af->a.spell.duration);
			ch->send_to_char (buf);
			continue;
		}
		
				
		if (af->type == MAGIC_WATCH1 ||
			af->type == MAGIC_WATCH2 || af->type == MAGIC_WATCH3)
			continue;
		
		
		ch->send_to_char (buf);
	}
	
	
	for (sa_it = second_affect_list.begin(); sa_it != second_affect_list.end(); sa_it++)
	{
		tsa = *sa_it;
		
		if (tsa->ch == ch)
		{
			sprintf (tbuf, "%s", lookup_string(tsa->type, REG_AFFECT).c_str());
			if (tbuf)
				sprintf (buf, "%s: Time remaining: %d seconds\n", tbuf, tsa->seconds);
			else
				sprintf (buf, "%d: Time remaining: %d seconds\n", tsa->type, tsa->seconds);
			
			
		}
		ch->send_to_char (buf);
	}
	
	ch->send_to_char ("\n");
	
	/**** morphing mobs ******/
	if (IS_NPC(k) && ((k->mob->clock > 0) || (k->mob->morphto > 0)))
	{
		int month, day, hour;
		
		month = k->mob->clock / (24 * 30);
		hour = k->mob->clock - month * 24 * 30;
		
		day = hour / 24;
		hour -= day * 24;
		
		sprintf (buf,
				 "\n#2Morphing Information:#0\n#2Clock:#0   %d %d %d    #2MorphTo:#0 %d    #2Morph Type:#0 %d\n\n",
				 month, day, hour, k->mob->morphto, k->mob->morph_type);
		ch->send_to_char (buf);
		
		if (k->mob->morph_time > 0)
		{
			int delta, days, hours, minutes;
			
			delta = k->mob->morph_time - time (0);
			
			days = delta / 86400;
			delta -= days * 86400;
			
			hours = delta / 3600;
			delta -= hours * 3600;
			
			minutes = delta / 60;
			
			sprintf (buf,
					 "This mob will morph in %d days, %d hours, and %d minutes\n",
					 days, hours, minutes);
			ch->send_to_char (buf);
		}
	}
	/**************************/
	if (k->mob && k->mob->shop && IS_SET (k->flags, FLAG_KEEPER))
	{
		sprintf (buf, "  #2Shop Rm:#0  %5d\n", k->mob->shop->shop_vnum);
		ch->send_to_char (buf);
		
		sprintf (buf, "  #2Store Rm:#0 %5d\n", k->mob->shop->store_vnum);
		ch->send_to_char (buf);
		
		if (k->mob->currency_type == CURRENCY_TIRITH)
		{
			sprintf (buf, "  #2Currency Type:#0  Minas Tirith\n");
		}	
		ch->send_to_char (buf);
		
		sprintf (buf, "  #2Markup:#0    %2.2f                \n",
				 k->mob->shop->markup);
		ch->send_to_char (buf);
		
		
		
		
		sprintf (buf, "  #2Discount:#0  %2.2f\n", k->mob->shop->discount);
		ch->send_to_char (buf);
		
		if (k->mob->merch_seven > 0)
		{
		sprintf (buf, "  #2Econ Markup1:#0 %2.2f", k->mob->shop->econ_markup1);
		pad_buffer (buf, 23);
		sprintf (buf + strlen (buf), "   #2Discount1:#0 %2.2f", k->mob->shop->econ_discount1);
		pad_buffer (buf, 40);
		sprintf (buf + strlen (buf), "  #2Econ1:#0 [");
		
		for (i = 0; str_cmp (econ_flags[i], "\n"); i++)
		{
			if (IS_SET (k->mob->shop->econ_flags1, 1 << i))
			{
				sprintf (buf + strlen (buf), "%s", econ_flags[i]);
				sprintf (buf + strlen (buf), " ");
			}
		}
		
		if (buf[strlen (buf) - 1] != '[')
			buf[strlen (buf) - 1] = ']';
		else
			sprintf (buf + strlen (buf), "]");
		
		sprintf (buf + strlen (buf), "\n");
		ch->send_to_char (buf);
		
		sprintf (buf, "  #2Econ Markup2:#0 %2.2f", k->mob->shop->econ_markup2);
		pad_buffer (buf, 23);
		sprintf (buf + strlen (buf), "   #2Discount2:#0 %2.2f", k->mob->shop->econ_discount2);
		pad_buffer (buf, 40);
		sprintf (buf + strlen (buf), "  #2Econ2:#0 [");
		
		for (i = 0; str_cmp (econ_flags[i], "\n"); i++)
		{
			if (IS_SET (k->mob->shop->econ_flags2, 1 << i))
			{
				sprintf (buf + strlen (buf), "%s", econ_flags[i]);
				sprintf (buf + strlen (buf), " ");
			}
		}
		
		if (buf[strlen (buf) - 1] != '[')
			buf[strlen (buf) - 1] = ']';
		else
			sprintf (buf + strlen (buf), "]");
		
		sprintf (buf + strlen (buf), "\n");
		ch->send_to_char (buf);
		
		sprintf (buf, "  #2Econ Markup3:#0 %2.2f", k->mob->shop->econ_markup3);
		pad_buffer (buf, 23);
		sprintf (buf + strlen (buf), "   #2Discount3:#0 %2.2f", k->mob->shop->econ_discount3);
		pad_buffer (buf, 40);
		sprintf (buf + strlen (buf), "  #2Econ3:#0 [");
		
		for (i = 0; str_cmp (econ_flags[i], "\n"); i++)
		{
			if (IS_SET (k->mob->shop->econ_flags3, 1 << i))
			{
				sprintf (buf + strlen (buf), "%s", econ_flags[i]);
				sprintf (buf + strlen (buf), " ");
			}
		}
		
		if (buf[strlen (buf) - 1] != '[')
			buf[strlen (buf) - 1] = ']';
		else
			strcat (buf, "]");
		
		strcat (buf, "\n");
		ch->send_to_char (buf);
		
		sprintf (buf, "  #2Econ Markup4:#0 %2.2f", k->mob->shop->econ_markup4);
		pad_buffer (buf, 23);
		sprintf (buf + strlen (buf), "   #2Discount4:#0 %2.2f", k->mob->shop->econ_discount4);
		pad_buffer (buf, 40);
		sprintf (buf + strlen (buf), "  #2Econ4:#0 [");
		
		for (i = 0; str_cmp (econ_flags[i], "\n"); i++)
		{
			if (IS_SET (k->mob->shop->econ_flags4, 1 << i))
			{
				strcat (buf, econ_flags[i]);
				strcat (buf, " ");
			}
		}
		
		if (buf[strlen (buf) - 1] != '[')
			buf[strlen (buf) - 1] = ']';
		else
			strcat (buf, "]");
		
		strcat (buf, "\n");
		ch->send_to_char (buf);
		
		
		sprintf (buf, "  #2Econ Markup5:#0 %2.2f", k->mob->shop->econ_markup5);
		pad_buffer (buf, 23);
		sprintf (buf + strlen (buf), "   #2Discount5:#0 %2.2f", k->mob->shop->econ_discount5);
		pad_buffer (buf, 40);
		sprintf (buf + strlen (buf), "  #2Econ5:#0 [");
		
		for (i = 0; str_cmp (econ_flags[i], "\n"); i++)
		{
			if (IS_SET (k->mob->shop->econ_flags5, 1 << i))
			{
				strcat (buf, econ_flags[i]);
				strcat (buf, " ");
			}
		}
		
		if (buf[strlen (buf) - 1] != '[')
			buf[strlen (buf) - 1] = ']';
		else
			strcat (buf, "]");
		
		strcat (buf, "\n");
		ch->send_to_char (buf);
		
		
		sprintf (buf, "  #2Econ Markup6:#0 %2.2f", k->mob->shop->econ_markup6);
		pad_buffer (buf, 23);
		sprintf (buf + strlen (buf), "   #2Discount6:#0 %2.2f", k->mob->shop->econ_discount6);
		pad_buffer (buf, 40);
		sprintf (buf + strlen (buf), "  #2Econ6:#0 [");
		
		for (i = 0; str_cmp (econ_flags[i], "\n"); i++)
		{
			if (IS_SET (k->mob->shop->econ_flags6, 1 << i))
			{
				strcat (buf, econ_flags[i]);
				strcat (buf, " ");
			}
		}
		
		if (buf[strlen (buf) - 1] != '[')
			buf[strlen (buf) - 1] = ']';
		else
			strcat (buf, "]");
		
		strcat (buf, "\n");
		ch->send_to_char (buf);
		
		
		sprintf (buf, "  #2Econ Markup7:#0 %2.2f", k->mob->shop->econ_markup7);
		pad_buffer (buf, 23);
		sprintf (buf + strlen (buf), "   #2Discount7:#0 %2.2f", k->mob->shop->econ_discount7);
		pad_buffer (buf, 40);
		sprintf (buf + strlen (buf), "  #2Econ7:#0 [");
		
		for (i = 0; str_cmp (econ_flags[i], "\n"); i++)
		{
			if (IS_SET (k->mob->shop->econ_flags7, 1 << i))
			{
				strcat (buf, econ_flags[i]);
				strcat (buf, " ");
			}
		}
		
		if (buf[strlen (buf) - 1] != '[')
			buf[strlen (buf) - 1] = ']';
		else
			strcat (buf, "]");
		
		strcat (buf, "\n");
		ch->send_to_char (buf);
		
		} //end of merch_seven stuff
		
		sprintf (buf, "  #2Buy Flags:#0    [");
		
		for (i = 0; str_cmp (econ_flags[i], "\n"); i++)
		{
			if (IS_SET (k->mob->shop->buy_flags, 1 << i))
			{
				strcat (buf, econ_flags[i]);
				strcat (buf, " ");
			}
		}
		
		if (buf[strlen (buf) - 1] != '[')
			buf[strlen (buf) - 1] = ']';
		else
			strcat (buf, "]");
		
		strcat (buf, "\n");
		ch->send_to_char (buf);
		
		sprintf (buf, "  #2NoBuy Flags:#0  [");
		
		for (i = 0; str_cmp (econ_flags[i], "\n"); i++)
		{
			if (IS_SET (k->mob->shop->nobuy_flags, 1 << i))
			{
				strcat (buf, econ_flags[i]);
				strcat (buf, " ");
			}
		}
		
		if (buf[strlen (buf) - 1] != '[')
			buf[strlen (buf) - 1] = ']';
		else
			strcat (buf, "]");
		
		strcat (buf, "\n");
		ch->send_to_char (buf);
		
		sprintf (buf, "  #2Trades in:#0    [");
		
		for (i = 0; i < MAX_TRADES_IN; i++)
			if (k->mob->shop->trades_in[i])
				sprintf (buf + strlen (buf), "%s ", item_types[k->mob->shop->trades_in[i]]);
		
		sprintf (buf + strlen (buf), "]\n");
		ch->send_to_char (buf);
		
		*buf2 = '\0';
		
		 for (materials_it = k->mob->shop->materials.begin(); materials_it != k->mob->shop->materials.end(); materials_it++ )
		 {
			 std::stringstream tmp_str;
			 tmp_str << *materials_it;
			 sprintf (buf2 + strlen (buf2), "%s ", tmp_str.str().c_str());
		 }
				
		if (*buf2)
			buf2[strlen (buf2) - 1] = ']';
		else
			sprintf (buf2, "]");
		
		sprintf (buf, "  #2Materials:#0    [%s\n\n", buf2);
		ch->send_to_char (buf);
		
		if (k->mob->shop->delivery[0])
		{
			sprintf (buf, "#2Deliveries:#0\n   ");
			j = 1;
			for (i = 0; i <= MAX_DELIVERIES; i++)
			{
				if (!k->mob->shop->delivery[i])
					break;
				sprintf (buf + strlen (buf), "#2%3d:#0 %-8d ", i,
						 k->mob->shop->delivery[i]);
				if (!(j % 5))
					strcat (buf, "\n   ");
				j++;
			}
			strcat (buf, "\n\n");
			ch->send_to_char (buf);
		}
	}

		
	/******** SKILLS ***********/
	i2 = 0;
		
	counter = k->skill_map.size();
	if (counter > 0)
	{
		for (skill_iterator = k->skill_map.begin(); skill_iterator != k->skill_map.end(); skill_iterator++)
		{
			
			tskill_name = skill_iterator->first.c_str();
			
			if (!tskill_name.empty())
			{
				tskill = skill_iterator->second;
				
				
				if (skill_data_map.find(tskill_name)!= skill_data_map.end())
				{
					
					counter --;
					if (tskill > 0)
					{
						skill_num = lookup_skill_id((char*)tskill_name.c_str());
						i2++;
						sprintf (buf, "#2%8.8s:#0 %03d/%03d  %s",
								 tskill_name.c_str(),
								 tskill,
								 calc_lookup (k, REG_CAP, skill_num),
								 i2 % 4 ? "" : "\n");
						ch->send_to_char (buf);
					}
				}
			}
			if (counter == 0)
				break;
		}
	}
	
	if (buf[strlen (buf) - 1] != '\n')
		ch->send_to_char ("\n");
	
	if (!IS_NPC(k) && ((k->pc->plan && !k->pc->plan->empty()) || (k->pc->goal && !k->pc->goal->empty())))
	{
		buf[0] = 0;
		if (k->pc->goal)
			sprintf (buf, "\n#2Long-term goal:#0\n   %s\n", k->pc->goal->c_str());
		
		if (k->pc->plan)
			sprintf (buf + strlen (buf), "\n#2Current Plan:#0\n%s\n", k->pc->plan->c_str());
		
		ch->send_to_char (buf);
		
	}
	
	if (loaded_char)
		k->unload_pc();
}

void
roomstat (CHAR_DATA * ch, char *name)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char *bitbuf;
	char buf3[MAX_STRING_LENGTH]= { '\0' };
	ROOM_DATA *rm;
	ROOM_PORTAL_DATA *tport;
	CHAR_DATA *k;
	OBJ_DATA *j;
	int i;
	int room_num;
	AFFECTED_TYPE *room_affect, *next_room_affect;

	if ((*name) && (!isdigit (*name)))
	{
		ch->send_to_char ("Please specify a room number.\n");
		return;
	}
	else if (!*name)
	{
		room_num = ch->in_room;
	}
	else if (isdigit (*name))
	{
		room_num = atoi(name);
	}
	
	rm = vtor (room_num);
	
	if (!rm)
	{
		ch->send_to_char ("Sorry. Could not find the room.\n");
		return;
	}
	
	
	bitbuf = strdup(sprintbit (rm->terrain_type, terrain_types));
	
	if (rm->capacity > 0)
		sprintf (buf3, " #2C:#0 %d ", rm->capacity);
	else 
		sprintf (buf3, " ");
		
	sprintf (buf, "#2[%6d]#0: %s [%s]%s[%s]\n",
			 rm->nVirtual,
			 rm->name,
			 room_sizes[rm->room_size],
			 buf3,
			 bitbuf);
	ch->send_to_char (buf);
	
	sprintf(buf, "#2Build Zone:#0 [%4d]    #2Enforcer Zone:#0 [XXXX]\n", rm->zone);
	ch->send_to_char (buf);
	
	

	bitbuf = strdup(sprintbit ((long) rm->room_flags, room_bits));
	sprintf (buf, "#2Rflags:#0 [%s]\n", bitbuf);
	ch->send_to_char (buf);

	ch->send_to_char ("#2Description:#0\n");
	ch->send_to_char (rm->description);
	
		
		//Weather descriptions follow normal descrip if they exist
		//don't display the last weather since it is the same as the main description
	for (i = 0; i < WR_LAST_DESCRIPTIONS; i++)
	{
		if (rm->extra->weather_desc[i].empty() 
			|| rm->extra->weather_desc[i].compare(""))
		{
			sprintf (buf, "");
		}
		else
		{
			sprintf (buf, "\n\n#2[%s]#0: %s\n", weather_room[i], rm->extra->weather_desc[i].c_str());
		}
		
		ch->send_to_char (buf);
	}
		
	if (rm->ex_description && rm->ex_description[0])
	{
		ch->send_to_char ("\n------- Extra items with descriptions -------\n");
		
		for (i = 0; i < MAX_EX_DESCR; i++)
		{ 
			if (rm->ex_description[i])
			{
				ch->send_to_char ("#2Ex-desc keyword(s)#0:\n");
				sprintf (buf, "[%s] %s\n", rm->ex_description[i]->keyword, rm->ex_description[i]->description);
				ch->send_to_char (buf);
			}
		}
	}
	
	if (rm->affects)
	{
		ch->send_to_char ("\n------- Affects on room -------\n");
		for ( room_affect = rm->affects; room_affect; room_affect = next_room_affect )
		{
			
			next_room_affect = room_affect->next;	 
			
		}
	}
	
	if (rm->portals)
	{
		sprintf(buf3, "");
		
		for (i = 0; i <= MAX_PORTALS; i++)
		{
			if (rm->portals[i] > 0)
			{
				tport = vtop(rm->portals[i]);
				if (tport)
				{
					sprintf (buf3 + strlen(buf3), "%s", plist_show(tport).c_str());
				}
			}
		}
		
		if (*buf3)
		{
			sprintf(buf, "\n------- Portal Summary -------\n%s", buf3);
			ch->send_to_char (buf);
		}
		
	}
	
	if (rm->extra)
	{		
		sprintf(buf3, "");
		
		for (i = 0; i <= LAST_DIR; i++) //dislay the alas descriptions
		{
			if (rm->extra 
				&& !rm->extra->alas[i].empty())
			{
				if(rm->extra->alas[i].compare(""))
				{
					sprintf (buf3 + strlen(buf3), "#2Direction:#0 %s . #2Alas:#0 %s#0\n",
							 dirs[i], rm->extra->alas[i].c_str());
				}
			}
		}
		if (*buf3)
		{
			sprintf(buf, "\n-- Alas Descriptions --\n%s", buf3);
			ch->send_to_char (buf);
		}
	}
	
	if (rm->people)
	{
	ch->send_to_char ("\n------- Chars present -------\n");
	for (k = rm->people; k; k = k->next_in_room)
	{
		if (IS_NPC (k))
		{
			sprintf (buf, "NPC: $N (%d)", k->mob->nVirtual);
			ch->act(buf, false,  0, k, TO_CHAR);
		}
		else
		{
			sprintf (buf, "PC: $N (%s)", k->name);
			ch->act(buf, false,  0, k, TO_CHAR);
		}
	}
	ch->send_to_char ("\n");
	}
	
	if (rm->contents)
	{
	ch->send_to_char ("\n--------- Contents ---------\n");
	for (j = rm->contents; j; j = j->next_content)
	{
		sprintf (buf, "%s (%d)\n", j->name, j->nVirtual);
		ch->send_to_char (buf);
	}
	ch->send_to_char ("\n");
	}
	
	if (rm->capacity > 0)
	{
		sprintf (buf, "#2Capacity:#0 %d #2Contents:#0 %d (people or equivalent weight)\n", rm->capacity, rm->occupants/100);
		ch->send_to_char (buf);
	}
	
	if (ch->pc->level >= 5)
	{
	sprintf (buf, "\n#2Location:#0 (%.2f, %.2f, %.2f)\n",
			 rm->x_coord,
			 rm->y_coord,
			 rm->z_coord);
	ch->send_to_char (buf);
	}
}

void
portstat (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char *bitbuf;
	ROOM_PORTAL_DATA *tport;
	
	const char *exit_bits[8] = {
		"Closed",
		"Locked",
		"PickProof",
		"Secret",
		"Gate",
		"Door",
		"NoSneak",
		"\n"
	};
	
	tport = vtop (atoi(argument));
	if (!tport)
	{
		ch->send_to_char ("That portal doesn't seem to exist.\n");
		return;
	}
	
	sprintf (buf, "[%4d]\n", tport->ident);
	ch->send_to_char (buf);
	
	bitbuf = strdup(sprintbit (tport->port_flags, exit_bits));
	sprintf(buf, "#2Zone:#0 %d  #2[%s]#0\n", tport->zone, bitbuf);
	ch->send_to_char (buf);
	
	
	
	sprintf (buf, "Room %d, %s side is connected to Room %d, %s side.\n",
			 tport->room_1,
			 dirs[tport->dir_1],
			 tport->room_2,
			 dirs[tport->dir_2]);
	ch->send_to_char (buf);
	
	return;
	
}

/*                                                                          *
* funtion: acctstat                    < e.g.> stat acct traithe           *
*                                                                          *
* 09/17/2004 [JWW] - Ensured that (result != NULL) in a few spots          *
*                  - was freeing the static char[] that asctime() returns! *
*                    now switched to using alloc, asctime_r, and free_mem  *
*                  - ordered the statement execution to match output flow  *
*                                                                          */
void
acctstat (CHAR_DATA * ch, char *name)
{
	account *acct;
	char *date;
	time_t account_time;
	char strTime[AVG_STRING_LENGTH] = "";
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char state[MAX_STRING_LENGTH]= { '\0' };
	char line[MAX_STRING_LENGTH]= { '\0' };
	MYSQL_RES *result;
	MYSQL_ROW row = NULL;
	int messages = 0, nDays = 0, nHours = 0, nColor = 0;

	if (!*name || atoi (name))
	{
		ch->send_to_char ("Please specify an account name.\n");
		return;
	}
	acct = new account (name);
	if (!acct->is_registered ())
	{
		delete acct;
		ch->send_to_char ("That account does not exist.\n");
		return;
	}


	*buf = '\0';
	*line = '\0';

	sprintf (buf, "\n   #2Account Name:#0             %s\n", acct->name.c_str ());
	sprintf (buf + strlen (buf), "   #2Registered Email:#0         %s\n",
		acct->email.c_str ());
	sprintf (buf + strlen (buf), "   #2Last Logged In From:#0      %s\n",
		acct->last_ip.c_str ());

	/* Retreive a list of other users from this IP ( default = no-op ) */
	mysql_safe_query
		("SELECT user_name FROM user WHERE user_last_ip = '%s' AND name != '%s'",
		acct->last_ip.c_str (), acct->name.c_str ());
	if ((result = mysql_store_result (database)) != NULL)
	{
		if (!IS_SET (acct->flags, ACCOUNT_IPSHARER)
			&& mysql_num_rows (result) > 0 && str_cmp (acct->name.c_str (), "Guest")
			&& acct->last_ip.compare ("(null)") != 0
			&& acct->last_ip.compare ("None") != 0
			&& acct->last_ip.find ("middle-earth.us") == std::string::npos)
		{
			sprintf (buf + strlen (buf), "   #1Accounts Sharing IP:#0     ");
			while ((row = mysql_fetch_row (result)))
			{
				sprintf (buf + strlen (buf), " %s", row[0]);
			}
			sprintf (buf + strlen (buf), "#0\n");
		}
		mysql_free_result (result);
		result = NULL;
	}
	else
	{
		system_log
			("Warning: MySql returned a NULL result in staff.c::acctstat() while fetching account sharing info.",
			true);
	}

	/* Retreive the timestamp for Last Activity ( default = "" ) */
	date = new char[256];
	date[0] = '\0';

	mysql_safe_query
		("SELECT lastlogon"
		" FROM pfiles"
		" WHERE account = '%s'"
		" ORDER BY lastlogon DESC LIMIT 1",
		acct->name.c_str ());
	if ((result = mysql_store_result (database)) != NULL)
	{
		row = mysql_fetch_row (result);
		if (row!= NULL && row[0] != NULL && *row[0])
		{
			account_time = (time_t) atoi (row[0]);
			if (asctime_r (localtime (&account_time), date) != NULL)
			{
				date[strlen (date) - 1] = '\0';	/* chop the newline asctime_r tacks on */
			}
		}
		mysql_free_result (result);
		result = NULL;
	}
	else
	{
		system_log
			("Warning: MySql returned a NULL result in staff.c::acctstat() while fetching lastlogin info.",
			true);
	}
	sprintf (buf + strlen (buf), "   #2Last PC Activity On:#0      %s\n",
		(date[0]) ? date : "None Recorded");
	free_mem (date);

	/* Account Registration Date ( default = "None Recorded" ) */
	date = new char[256];
	date[0] = '\0';
	account_time = acct->created_on;
	asctime_r (localtime (&account_time), date);
	sprintf (buf + strlen (buf), "   #2Account Registered On:#0    %s",
		(date[0]) ? date : "None Recorded");
	free_mem (date);

	sprintf (buf + strlen (buf), "   #2Accrued Roleplay Points:#0  %d\n",
		acct->get_rpp ());

	if (acct->get_last_rpp_date () > 0)
	{
		std::string date_str = acct->get_last_rpp_date_string ();
		sprintf (buf + strlen (buf), "   #2Last RPP Award On:#0        %s\n",
			date_str.c_str ());
	}

	/* Retreive the number of stored Hobbitmails ( default = 0 ) */
	messages = 0;
	mysql_safe_query ("SELECT COUNT(*) FROM hobbitmail WHERE account = '%s'",
		acct->name.c_str ());
	if ((result = mysql_store_result (database)) != NULL)
	{
		messages = ((row = mysql_fetch_row (result)) != NULL)
			? atoi (row[0]) : 0;
		mysql_free_result (result);
		result = NULL;
	}
	else
	{
		system_log
			("Warning: MySql returned a NULL result in staff.c::acctstat() while fetching hmail count",
			true);
	}
	sprintf (buf + strlen (buf), "   #2Stored Hobbitmails:#0       %d\n",
		messages);


	if (acct->flags)
	{
		sprintf (buf + strlen (buf), "   #2Account Flags Set:#0       ");
		if (IS_SET (acct->flags, ACCOUNT_NOPETITION))
			sprintf (buf + strlen (buf), " NoPetition");
		if (IS_SET (acct->flags, ACCOUNT_NOGUEST))
			sprintf (buf + strlen (buf), " NoGuest");
		if (IS_SET (acct->flags, ACCOUNT_NOPSI))
			sprintf (buf + strlen (buf), " NoPsi");
		if (IS_SET (acct->flags, ACCOUNT_NOBAN))
			sprintf (buf + strlen (buf), " NoBan");
		if (IS_SET (acct->flags, ACCOUNT_NORETIRE))
			sprintf (buf + strlen (buf), " NoRetire");
		if (IS_SET (acct->flags, ACCOUNT_NOVOTE))
			sprintf (buf + strlen (buf), " NoVoteReminder");
		if (IS_SET (acct->flags, ACCOUNT_IPSHARER))
			sprintf (buf + strlen (buf), " IPSharingOK");
		if (IS_SET (acct->flags, ACCOUNT_RPPDISPLAY))
			sprintf (buf + strlen (buf), " RPPDisplay");
		if (IS_SET (acct->flags, ACCOUNT_BETA))
			sprintf (buf + strlen (buf), " Beta Player");
		if (IS_SET (acct->flags, ACCOUNT_EMERITUS))
			sprintf (buf + strlen (buf), " Emeritus");
		sprintf (buf + strlen (buf), "\n");
	}
	sprintf (buf + strlen (buf), "   #2ANSI Color Setting:#0       %s\n",
		acct->color ? "On" : "Off");
	
	sprintf (buf + strlen (buf), "   #2Newsletter Status:#0        %s\n",
		acct->newsletter ? "Subscribed" : "Opted Out");

	mysql_safe_query
		("SELECT name,create_state,sdesc,played"
		" FROM pfiles"
		" WHERE account = '%s'"
		" ORDER BY birth ASC",
		acct->name.c_str ());
	result = mysql_store_result (database);

	if (result && mysql_num_rows (result) > 0)
	{
		sprintf (buf + strlen (buf), "\n");
		sprintf (buf + strlen (buf), "   #2Registered Characters:#0\n");
		while ((row = mysql_fetch_row (result)))
		{
			if (strlen (buf) + strlen (line) >= MAX_STRING_LENGTH)
				break;
			if (atoi (row[1]) < 1)
			{
				sprintf (state, "#3(Pending)#0");
				nColor = 3;
			}
			else if (atoi (row[1]) == 1)
			{
				sprintf (state, "#6(Submitted)#0");
				nColor = 6;
			}
			else if (atoi (row[1]) == 2)
			{
				sprintf (state, "#2(Active)#0");
				nColor = 2;
			}
			else if (atoi (row[1]) == 3)
			{
				sprintf (state, "#5(Suspended)#0");
				nColor = 5;
			}
			else if (atoi (row[1]) == 4)
			{
				sprintf (state, "#1(Deceased)#0");
				nColor = 1;
			}
			else
			{
				sprintf (state, "#1(Retried)#0");
				nColor = 1;
			}

			nDays = (strtol (row[3], NULL, 10) / (60 * 60 * 24));
			nHours = (strtol (row[3], NULL, 10) % (60 * 60 * 24));
			nHours = (nHours / (60 * 60));

			if (nDays > 0)
				sprintf (strTime, "%3dd", nDays);
			else
				strTime[0] = '\0';

			sprintf (strTime + strlen (strTime), " %2dh", nHours);
			if (!nDays && !nHours)
				strcpy (strTime, "<  1h");

			sprintf (line, "   #%d%-15s %8s#0  #5%-42s\n", nColor, row[0],
				strTime, row[2]);


			if (strlen (row[2]) > 32)
			{
				sprintf (line + 68, "...#0 %s\n", state);
			}
			else
			{
				sprintf (line + 71, "#0 %s\n", state);
			}
			sprintf (buf + strlen (buf), "%s", line);
		}
	}

	if (result)
		mysql_free_result (result);

	delete acct;

	page_string (ch->desc, buf);
}

void
objstat (CHAR_DATA * ch, char *name)
{
	OBJ_DATA *tobj;
	OBJ_DATA *j2;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char copper_buf[MAX_STRING_LENGTH]= { '\0' };
	char *sp = '\0';
	char *bow_type[] = { "Longbow", "Shortbow", "Crossbow", "Sling" };
	int i;
	int instance = 0;
	char mweight[MAX_STRING_LENGTH]= { '\0' };
	float cost = 0;
	AFFECTED_TYPE *af;
	std::vector<int>::iterator var_it;
	int tvar;
	extern const char *damage_severity[];
	
	if (!*name && !atoi (name))
	{
		ch->send_to_char ("Please specifiy an object name or vnum.\n");
		return;
	}

	if (just_a_number (name) && vtoo (atoi (name)))
		tobj = vtoo (atoi (name));

	else if ((tobj = get_obj_in_list_vis (ch, name, ch->right_hand)) ||
		(tobj = get_obj_in_list_vis (ch, name, ch->left_hand)) ||
		(tobj = get_obj_in_list_vis (ch, name, ch->room->contents)))
		instance = 1;

	else if (!IS_NPC (ch) && !*name)
		tobj = vtoo (ch->pc->edit_obj);

	if (!tobj)
	{
		ch->send_to_char ("No such object.\n");
		return;
	}

	if (!instance)
		sprintf (buf2, " [%d instances]", tobj->instances);
	else
		sprintf (buf2, " [id %d]", tobj->coldload_id);

	sprintf (buf, "\n#2VNUM:#0  %d [%s] [#2%s#0]%s\n", tobj->nVirtual,
		instance ? "#2Instance#0" : "#2Prototype#0",
		item_types[(int) tobj->obj_flags.type_flag], buf2);

	sprintf (buf + strlen (buf), "#2Zone: #0 %d\n", tobj->zone);
	ch->send_to_char (buf);

	sprintf (buf, "#2Keys: #0 %s\n", tobj->name);
	ch->send_to_char (buf);

	sprintf (buf, "#2SDesc:#0 %s\n#2LDesc:#0 %s\n", tobj->short_description,
		tobj->description);
	sprintf(buf + strlen(buf), "#2Full Description:#0\n%s\n",tobj->full_description ? tobj->full_description : "None" );
	ch->send_to_char (buf);

	sprintf (buf, "#2Omote:#0 %s\n\n", (tobj->omote_str) ? tobj->omote_str : "None");
	reformat_string (buf, &sp);
	sprintf (buf, "%s", sp);
	free_mem (sp);
	sp = NULL;
	ch->send_to_char (buf);

	if (tobj->clan_data) // owned by only 1 clan
	{
		sprintf (buf, "#2Clans:#0    ");
		sprintf (buf + strlen (buf), "'%s' %s  ",
				 tobj->clan_data->name,
				 tobj->clan_data->rank);
		
		strcat (buf, "\n");
		ch->send_to_char (buf);
	}
	
	if (tobj->obj_flags.type_flag == ITEM_INK)
	{
		sprintf (buf, "#2Ink String:#0  %s\n", tobj->ink_color);
		ch->send_to_char (buf);
	}

	if (tobj->obj_flags.wear_flags)
	{
		sprintf (buf, "#2Wearbits:#0    ");
		for (i = 0; (*wear_bits[i] != '\n'); i++)
			if (IS_SET (tobj->obj_flags.wear_flags, (1 << i)))
				sprintf (buf + strlen (buf), "%s ", wear_bits[i]);
		strcat (buf, "\n");
		ch->send_to_char (buf);
	}

	
	if (tobj->obj_flags.extra_flags)
	{
		sprintf (buf, "#2Extra Flags:#0 ");
		for (i = 0; (*extra_bits[i] != '\n'); i++)
			if (IS_SET (tobj->obj_flags.extra_flags, (1 << i)))
				sprintf (buf + strlen (buf), "%s ", extra_bits[i]);
		if (tobj->activation)
			sprintf (buf + strlen (buf), "; #2Activation:#0 %s\n",
			lookup_string (tobj->activation, REG_MAGIC_SPELLS).c_str());
		else
			strcat (buf, "\n");
		ch->send_to_char (buf);
	}

	if (tobj->econ_flags)
	{
		sprintf (buf, "#2Econ Flags:#0  ");
		for (i = 0; str_cmp (econ_flags[i], "\n"); i++)
		{
			if (IS_SET (tobj->econ_flags, 1 << i))
			{
				strcat (buf, econ_flags[i]);
				strcat (buf, " ");
			}
		}
		strcat (buf, "\n");
		ch->send_to_char (buf);
	}

	sprintf(buf, "#2Materials: #0");
	for (i = 0; i < MAX_OBJ_MATERIALS; i++)
	{
		if (str_cmp(tobj->materials[i], ""))
		{
			sprintf(buf + strlen(buf), "%s ", tobj->materials[i]);
		}
	}
	
	pad_buffer (buf, 32);
	sprintf(buf + strlen(buf), "#2Avg Hardness: #0%d\n", get_material_hardness(tobj));
	ch->send_to_char (buf);

		if (tobj->max_hit_points)
		{
		sprintf(buf, "#2Max Hit Points: #0%.2f", tobj->max_hit_points);
		pad_buffer (buf, 32);
		sprintf(buf + strlen(buf), "#2Current Damage: #0%d", tobj->current_damage);
		sprintf(buf + strlen(buf), "\n");
		
		if (*buf)
			ch->send_to_char (buf);
		}
	
	
	*buf = 0;
	
	if (tobj->size)
	{
		if (IS_WEARABLE (tobj))
			sprintf (buf, "#2Size:#0 %s (%s)\n",
			sizes_named[tobj->size], sizes[tobj->size]);
		else
			sprintf (buf, "#2Size:#0 %s (%s) #1[N/A]#0\n",
			sizes_named[tobj->size], sizes[tobj->size]);
	}

	else if (IS_WEARABLE (tobj))
		sprintf (buf, "#2Size:#0 Any\n");

	if (*buf)
		ch->send_to_char (buf);

	if (tobj->obj_flags.type_flag == ITEM_FLUID)
		sprintf (copper_buf,
		"%d.%02df (%d)", (int) tobj->coppers / 100,
		(int) tobj->coppers % 100, (int) tobj->coppers);
	else
		sprintf (copper_buf, "%df", (int) tobj->coppers);

	cost = tobj->coppers;

	sprintf (buf, "#2Weight:#0      %d.%02d", tobj->obj_flags.weight / 100,
		tobj->obj_flags.weight % 100);
	pad_buffer (buf, 32);
	sprintf (buf + strlen (buf), "#2Full Weight:#0  %d.%02d",
		obj_mass (tobj) / 100, obj_mass (tobj) % 100);
	pad_buffer (buf, 61);
	sprintf (buf + strlen (buf), "#2Timer:#0  %d\n", tobj->obj_timer);
	ch->send_to_char (buf);

	sprintf (buf, "#2Cost:#0        %.02f cp", cost);
	pad_buffer (buf, 32);
	sprintf (buf + strlen (buf), "#2Quality:#0 %d", tobj->quality);
	pad_buffer (buf, 61);
	sprintf (buf + strlen (buf), "#2Cond.:#0  %d%%\n", tobj->item_wear);

	ch->send_to_char (buf);

	switch (tobj->obj_flags.type_flag)
	{
		case ITEM_FUEL:
			sprintf (buf, "#2Oval0 - Capacity:#0 %d\n"
					 "#2Oval1 - Volume:#0   %d\n"
					 "#2Oval2 - Fuel:#0     %d\n"
					 "#2Oval3 - Refillable:#0     %d (0 is no, 1 is yes)\n",
					 tobj->o.fuelcon.capacity,
					 tobj->o.fuelcon.volume,
					 tobj->o.fuelcon.fuel,
					 tobj->o.fuelcon.refill);
			break;
			
		case ITEM_FLUID:
			sprintf (buf,
					 "#2Oval0 - Alcohol:#0   %d per sip\n"
					 "#2Oval1 - Water:#0     %d per sip\n"
					 "#2Oval2 - Calories:#0  %d per sip\n",
					 tobj->o.fluid.alcohol,
					 tobj->o.fluid.water,
					 tobj->o.fluid.food);
			break;
			
		case ITEM_LIGHT:
			
			if ((j2 = vtoo (tobj->o.light.fuel)))
			{
				sprintf (buf2, "%d (%s)", j2->nVirtual, j2->short_description);
				if (j2->obj_flags.type_flag != ITEM_FUEL)
					sprintf (buf2 + strlen (buf2), " [Need %s; this is %s]\n",
							 item_types[ITEM_FUEL],
							 item_types[(int) j2->obj_flags.type_flag]);
			}
			else
				sprintf (buf2, "%d (NO SUCH OBJECT EXISTS)", tobj->o.light.fuel);
			
			sprintf (buf,
					 "#2Oval0 - Capacity:#0   %d\n"
					 "#2Oval1 - Hours:#0      %d\n"
					 "#2Oval2 - Fuel:#0       %s\n"
					 "#2Oval3 - On:#0         %d (0 is no, 1 is yes)\n"
					 "#2Oval4 - Brightness:#0 %d\n",
					 tobj->o.light.capacity,
					 tobj->o.light.hours,
					 buf2,
					 tobj->o.light.on,
					 tobj->o.light.bright);
			break;
			
		case ITEM_FOOD:
			sprintf (buf,
					 "#2Oval0 - Calories overall:#0 %d\n"
					 "#2Oval1 - Water overall:#0    %d\n"
					 "#2Oval2 - Alchol overall:#0   %3d\n"
					 "#2Oval5 - Bites to consume:#0 %3d\n",
					 tobj->o.od.value[0],
					 tobj->o.od.value[1],
					 tobj->o.od.value[2],
					 tobj->o.od.value[5]);
			break;
		case ITEM_TOSSABLE:
			sprintf (buf,
					 "#2Oval0 - Facets:#0 %d\n"
					 "#2Oval1 -  Bonus:#0 %d\n",
					 tobj->o.od.value[0], tobj->o.od.value[1]);
			sprintf (buf + strlen (buf), "\n#2MKeys:#0 %s\n", tobj->desc_keys);
			break;
		case ITEM_WORN:
			sprintf (buf, "#2MKeys:#0 %s\n", tobj->desc_keys);
			break;
			
		
		case ITEM_NPC_OBJECT:
		{
			int npc_object_mob_id = tobj->o.od.value[0];
			CHAR_DATA* npc_object_mob = npc_object_mob_id ?
			vtom (npc_object_mob_id) : 0;
			sprintf (buf, "#2NPC VNUM:#0 %d [#5%s#0]\n", npc_object_mob_id,
					 (npc_object_mob) ? npc_object_mob->short_descr : "none");
		}
			break;
			
		case ITEM_CONTAINER:
			
			if (tobj->o.container.flags)
			{
			sprintf(buf2, "[");
			if (IS_SET(tobj->o.container.flags, CONT_CLOSEABLE))
				sprintf(buf2 + strlen(buf2), "Closeable ");
			
			if (IS_SET(tobj->o.container.flags, CONT_PICKPROOF))
				sprintf(buf2 + strlen(buf2), "Pickproof ");
			
			if (IS_SET(tobj->o.container.flags, CONT_CLOSED))
				sprintf(buf2 + strlen(buf2), "Closed ");
			
			if (IS_SET(tobj->o.container.flags, CONT_LOCKED))
				sprintf(buf2 + strlen(buf2), "Locked ");
			
			
			sprintf(buf2 + strlen(buf2), "]");
			}
			else
			{
				sprintf(buf2, "[none]");
			}

			if (!IS_SET (tobj->obj_flags.extra_flags, ITEM_TABLE))
				sprintf (buf, "#2Oval0 - Capacity:#0  %d  (%d.%.2d lbs)\n"
						 "#2Oval1 - Lockflags:#0 %d %s\n"
						 "#2Oval2 - Key#0:       %d\n"
						 "#2Oval3 - Pick Penl:#0 %d\n",
						 tobj->o.container.capacity,
						 tobj->o.container.capacity / 100,
						 tobj->o.container.capacity % 100,
						 tobj->o.container.flags,
						 buf2,
						 tobj->o.container.key,
						 tobj->o.container.pick_penalty);
			else
				sprintf (buf, "#2Oval0 - Capacity:#0    %d  (%d.%.2d lbs)\n"
						 "#2Oval1 - Lockflags:#0   %d %s\n"
						 "#2Oval2 - Key:#0         %d\n"
						 "#2Oval3 - Pick Penl:#0   %d\n"
						 "#2Oval5 - Seating Cap:#0 %d\n",
						 tobj->o.container.capacity,
						 tobj->o.container.capacity / 100,
						 tobj->o.container.capacity % 100,
						 tobj->o.container.flags,
						 buf2,
						 tobj->o.container.key,
						 tobj->o.container.pick_penalty,
						 tobj->o.container.table_max_sitting);
			
			break;
			
		case ITEM_DRINKCON:
		case ITEM_FOUNTAIN:
			
			if ((j2 = vtoo (tobj->o.drinkcon.liquid)))
			{
				sprintf (buf2, "%d (%s)", j2->nVirtual, j2->short_description);
				if (j2->obj_flags.type_flag != ITEM_FLUID)
					sprintf (buf2 + strlen (buf2), " [Need %s; this is %s]\n",
							 item_types[ITEM_FLUID],
							 item_types[(int) j2->obj_flags.type_flag]);
			}
			
			
			else 
				strcat (buf2, " [None set]");
			
			sprintf (buf, "#2Oval0 - Capacity:#0 %d\n"
					 "#2Oval1 - Contains:#0 %d\n"
					 "#2Oval2 - Liquid:#0   %s\n",
					 tobj->o.drinkcon.capacity, tobj->o.drinkcon.volume, buf2);
			break;
			
		case ITEM_DRYCON:
			
			if ((j2 = vtoo (tobj->o.drycon.contents)))
			{
				sprintf (buf2, "%d (%s)", j2->nVirtual, j2->short_description);
				if (j2->obj_flags.type_flag == ITEM_FLUID)
					sprintf (buf2 + strlen (buf2), " [Need non-fluid; this is %s]\n",
							 item_types[ITEM_FLUID]);
				
			}
			else 
				strcat (buf2, " [None set]");
			sprintf (mweight, "%d.%02d", tobj->o.drycon.max_weight / 100,
					 tobj->o.drycon.max_weight % 100);
			
			sprintf (buf, "#2Oval0 - Capacity:#0 %d\n"
					 "#2Oval1 - Contains:#0 %d\n"
					 "#2Oval2 - Contents:#0   %s\n"
					 "#2Oval3 - Max_weight:#0   %s\n",
					 tobj->o.drycon.capacity,
					 tobj->o.drycon.volume,
					 buf2,
					 mweight);
			break;
			
		case ITEM_KEY:
			sprintf (buf, "#2Oval0 - Keytype:#0  %d\n", tobj->o.od.value[0]);
			sprintf (buf + strlen (buf), "#2Oval1 - Keyed To:#0 %d\n",
					 tobj->o.od.value[1]);
			break;
			
		case ITEM_TICKET:
			sprintf (buf, "#2Oval0 - Ticket Number:#0 %d\n"
					 "#2Oval1 - Keeper VNUM:#0   %d\n",
					 tobj->o.ticket.ticket_num, tobj->o.ticket.keeper_vnum);
			break;
			
		case ITEM_MERCH_TICKET:
			sprintf (buf,
					 "#2Oval0 - Order ID##:#0    %d [see matching post on 'Orders' vboard]\n"
					 "#2Oval1 - Keeper VNUM:#0  %d\n", tobj->o.od.value[0],
					 tobj->o.od.value[1]);
			break;
			
			
		
		case ITEM_KEYRING:
			sprintf (buf, "#2Oval0 - Key Capacity:#0 %d\n", tobj->o.od.value[0]);
			break;
			
		
			
		case ITEM_INK:
			sprintf (buf, "#2Oval 0 -  Dips Left:#0  %d\n", tobj->o.od.value[0]);
			sprintf (buf + strlen (buf), "#2Oval 1 -  Capacity:#0   %d\n",
					 tobj->o.od.value[1]);
			break;
			
					
		case ITEM_REPAIR_KIT:
			sprintf (buf, "#2Oval 0 -  Uses Remaining:#0  %d\n", tobj->o.od.value[0]);
			sprintf (buf + strlen (buf), "#2Oval 1 -  Mending Bonus:#0   %d\n",
					 tobj->o.od.value[1]);
			sprintf (buf + strlen (buf), "#2Oval 2 -  Required Skill:#0  %d\n",
					 tobj->o.od.value[2]);
			sprintf (buf + strlen (buf), "#2Skill needed to use kit:#0   %s\n",
					 (tobj->o.od.value[3] > 0) ? lookup_skill_name(tobj->o.od.value[3]) : "#1No Skill needed#0");
			sprintf (buf + strlen (buf), "#2Max damage level:#0          %s\n",
					 damage_severity[tobj->o.od.value[4]]);	
			sprintf (buf + strlen (buf), "#2Repairs This Item Type:#0   ");
			if (tobj->o.od.value[5] == 0)
			{
				sprintf (buf + strlen (buf), "#1All#0\n");
				break;
			}
			else if (tobj->o.od.value[5] < 0)
			{
				sprintf (buf + strlen (buf), "#1None#0\n");
				break;
			}
			else
			{
				sprintf (buf + strlen (buf), "%s\n",
						 item_types[(tobj->o.od.value[5])]);
				break;
			}
			
			sprintf (buf + strlen (buf), "\n");
			break;
			
			
					
		default:
			if (tobj->nVirtual == 666)
				sprintf (buf, "#2Original Object VNUM:#0 %d\n", tobj->o.od.value[0]);
			else
				sprintf (buf, "#2Values 0-5:#0 [%d] [%d] [%d] [%d] [%d] [%d]\n",
						 tobj->o.od.value[0],
						 tobj->o.od.value[1],
						 tobj->o.od.value[2],
						 tobj->o.od.value[3], tobj->o.od.value[4], tobj->o.od.value[5]);
			break;
	}

	ch->send_to_char (buf);

	if (tobj->contains)
	{
		strcpy (buf, "\n#2Contains#0[ ");
		for (j2 = tobj->contains; j2; j2 = j2->next_content)
			sprintf (buf + strlen (buf), "%s ", fname (j2->name));
		strcat (buf, "]\n");
		ch->send_to_char (buf);
	}

	for (af = tobj->xaffected; af; af = af->next)
	{

		if (af->type == MAGIC_HIDDEN)
			continue;
		if (af->a.spell.location >= 10000)
		{
			sprintf (buf, "    #2Affects:#0 %s by %d\n",
				lookup_skill_name(af->a.spell.location - 10000),
				af->a.spell.modifier);
			ch->send_to_char (buf);
		}
	}

	if (tobj->clock || tobj->morphto)
	{
		int month, day, hour;

		month = tobj->clock / (24 * 30);
		hour = tobj->clock - month * 24 * 30;

		day = hour / 24;
		hour -= day * 24;

		sprintf (buf,
			"\n#2Morphing Information:#0\n#2Clock:#0   %d %d %d\n#2MorphTo:#0 %d\n",
			month, day, hour, tobj->morphto);
		ch->send_to_char (buf);

		if (tobj->morphTime)
		{
			int delta, days, hours, minutes;

			delta = tobj->morphTime - time (0);

			days = delta / 86400;
			delta -= days * 86400;

			hours = delta / 3600;
			delta -= hours * 3600;

			minutes = delta / 60;

			sprintf (buf,
				"This object will morph in %d days, %d hours, and %d minutes\n",
				days, hours, minutes);
			ch->send_to_char (buf);
		}
	}

	if (tobj->count > 1)
	{
		sprintf (buf, "\n#2Count:#0 %d\n", tobj->count);
		ch->send_to_char (buf);
	}
	if (tobj->obj_flags.set_cost)
	{
		sprintf (buf, "\n#2Sale Price:#0 %7.2f\n",
			((float)tobj->obj_flags.set_cost) / 100.0);
		ch->send_to_char (buf);
	}
	
	if (!tobj->var_val.empty())
	{
		sprintf(buf, "#2Variant Values:#0 \n");
		for (var_it = tobj->var_val.begin(); var_it != tobj->var_val.end(); var_it++)
		{
			tvar = *var_it;
			sprintf (buf + strlen(buf), "%s %s\n", gl_variant[tvar]->name, gl_variant[tvar]->var_value);
		}
		ch->send_to_char(buf);
	}
	
}

/*                                                                          *
* funtion: emailstat                    < e.g.> stat email null@bar.fu     *
*                                                                          *
* 09/17/2004 [JWW] - Fixed two instances where mysql result was not freed  *
*                    by consolodating if / else / return logic             *
*                                                                          */
void
emailstat (CHAR_DATA * ch, char *argument)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int i = 0;

	if (!*argument)
	{
		ch->send_to_char
			("You'll need to either specify a partial or full email address to search for.\n");
		return;
	}

	if (strchr (argument, ';'))
	{
		ch->send_to_char ("You have included an illegal character in your query.\n");
		return;
	}

	mysql_safe_query
		("SELECT user_name, user_email FROM user WHERE user_email LIKE '%%%s%%' ORDER BY user_name ASC",
		argument);
	result = mysql_store_result (database);
	if (result == NULL || mysql_num_rows (result) == 0)
	{
		ch->send_to_char
			("No login accounts were found with email addresses matching your query.\n");
	}
	else if (mysql_num_rows (result) > 255)
	{
		ch->send_to_char
			("More than 255 records were found matching this query. Aborting.\n");
	}
	else
	{

		*buf = '\0';
		sprintf (buf,
			"\n#6The following user accounts were found to match your query:#0\n\n");
		while ((row = mysql_fetch_row (result)) != NULL)
		{
			i++;
			sprintf (buf + strlen (buf), "   %-25s: %s\n", row[0], row[1]);
		}
		sprintf (buf + strlen (buf), "\n#6Matching Records Found: %d#0\n", i);
		page_string (ch->desc, buf);

	}

	if (result)
	{
		mysql_free_result (result);
	}
	else
	{
		system_log
			("Warning: MySql returned a NULL result in staff.c::emailstat() while fetching matching accounts.",
			true);
	}
	return;
}

void
do_stat (CHAR_DATA * ch, char *argument, int cmd)
{
	char arg1[MAX_STRING_LENGTH]= { '\0' };
	char arg2[MAX_STRING_LENGTH]= { '\0' };

	half_chop (argument, arg1, arg2);
	
	if  (ch->get_trust() < 2)
	{
		if (*arg1 != 'o'
			&& *arg1 != 'r'
			&& *arg1 != 'p')
		{
			ch->send_to_char ("Usage: stat <r|o|p> <name or number> \n\n");
			ch->send_to_char (" stat r       for room\n");
			ch->send_to_char (" stat o       for objects\n");
			ch->send_to_char (" stat p       for portals\n");

			return;
		}
	}

	if  (ch->get_trust() < 3)
	{
		if (*arg1 != 'o' &&
			*arg1 != 'r' &&
			*arg1 != 'm' &&
			*arg1 != 'p' &&
			*arg1 != 'f' )
		{
			ch->send_to_char ("Usage: stat <r|m|o|p> <name or number>\n\n");
			ch->send_to_char ("    r         room\n");
			ch->send_to_char ("    m         mobile\n");
			ch->send_to_char ("    o         object\n");
			ch->send_to_char ("    p         portals\n");
			return;
		}
	}


	if (!*arg1)
	{
		ch->send_to_char ("Usage: stat <r|p|c|m|o|a|e|f|k> name or number\n\n");
		ch->send_to_char ("    r         room\n");
		ch->send_to_char ("    p         portals\n");
		ch->send_to_char ("    c         character\n");
		ch->send_to_char ("    m         mobile\n");
		ch->send_to_char ("    o         object\n");
		ch->send_to_char ("    a         account\n");
		ch->send_to_char ("    e         email\n");
		ch->send_to_char ("    f         craft\n");
		ch->send_to_char ("    k         special craft\n");

		return;
	}
	else
	{
		switch (*arg1)
		{
		case 'r':
			roomstat (ch, arg2);
			return;
		case 'p':
				if (!isdigit (*arg2))
				{
					ch->send_to_char ("Please specify a portal number.\n");
					return;
				}	
			pstat (ch, atoi(arg2));
			return;
		case 'c':
			charstat (ch, arg2, true);
			return;
		case 'm':
			charstat (ch, arg2, false);
			return;
		case 'o':
			objstat (ch, arg2);
			return;
		case 'e':
			emailstat (ch, arg2);
			return;
		case 'a':
			acctstat (ch, arg2);
			return;
		case 'f':
			craftstat (ch, arg2);
			return;
		case 'k':
			spec_craftstat (ch, arg2);
			return;
		default:
			ch->send_to_char ("Usage: stat <room|portal|char|mob|obj|acct|email|craft> name\n\n");
			ch->send_to_char ("    r         room\n");
			ch->send_to_char ("    p         portals\n\n");
			ch->send_to_char ("    c         character\n");
			ch->send_to_char ("    m         mobile\n");
			ch->send_to_char ("    o         object\n");
			ch->send_to_char ("    a         account\n");
			ch->send_to_char ("    e         email\n");
			ch->send_to_char ("    f         craft\n");
			ch->send_to_char ("    k         special craft\n\n");
			return;
		}

	}
}

void
save_character_state (CHAR_DATA * tch)
{
	AFFECTED_TYPE *af;
	SIGHTED_DATA *sighted;
	char craft_name[MAX_STRING_LENGTH]= { '\0' };
	int phase_num;

	
	if (tch->subdue && GET_FLAG (tch, FLAG_SUBDUER))
	{
		mysql_safe_query ("INSERT INTO copyover_subduers VALUES (%d, %d)",
			tch->coldload_id, tch->subdue->coldload_id);
	}
	
	if (tch->following)
	{
		mysql_safe_query ("INSERT INTO copyover_followers VALUES (%d, %d)",
			tch->coldload_id, tch->following->coldload_id);
	}

	if ((af = is_crafting (tch)))
	{
		
		for (phase_num = 1; phase_num < MAX_PHASES_PER_SUBCRAFT; phase_num++)
		{
			if (phase_num == af->a.craft->phase_num)
				break;
		}
		sprintf (craft_name, "%s %s", af->a.craft->subcraft->command,
			af->a.craft->subcraft->subcraft_name);
		mysql_safe_query
			("INSERT INTO copyover_crafts VALUES (%d, '%s', %d, %d)",
			tch->coldload_id, craft_name, phase_num, af->a.craft->timer);
	}
}


void
do_shutdown (CHAR_DATA * ch, char *argument, int cmd)
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char arg[MAX_STRING_LENGTH]= { '\0' };
	FILE *fs;
	bool block = false;


	argument = one_argument (argument, arg);

	if (!*arg)
	{
		ch->send_to_char ("Specify a shutdown mode please.");
		return;
	}

	if (!str_cmp (arg, "die"))
	{
		system ("touch booting");
		sprintf (buf, "%s is shutting down the server...\n", ch->name);
		send_to_gods (buf);
		system_log (buf, false);
		sprintf (buf,
			"\n#1--- ANNOUNCEMENT ---#0\n#1We are shutting the server down for maintenance. Please check\nback later; our apologies for the inconvenience.#0\n#1--------------------\n#0");
		send_to_all_unf (buf);
		shutd = 1;
		if ((fs = fopen (".reboot", "w")) == NULL)
		{
			system_log ("Error creating shutdown file.", true);
			return;
		}
		if (ch->desc->original)
			sprintf (buf, "%s\n", (ch->desc->original)->name);
		else
			sprintf (buf, "%s\n", ch->name);
		fputs (buf, fs);
		fclose (fs);
	}
	else if (!str_cmp (arg, "stop"))
	{
		if (!pending_reboot)
		{
			ch->send_to_char ("No reboot is currently pending.\n");
			return;
		}
		pending_reboot = false;
		sprintf (buf, "%s has cancelled the pending reboot.", ch->name);
		send_to_gods (buf);
	}
	else if (!str_cmp (arg, "reboot"))
	{
		
			if (ch->get_trust() < 5)
			{
				ch->send_to_char ("You'll need to ask senior staff for a reboot.\n");
				return;
			}

		for (d = descriptor_list; d; d = d->next)
		{
			if (!d->character)
				continue;
			if (d->character && IS_NPC (d->character))
			{
				continue;
			}
			if (d->connected != CON_PLYNG && d->character->pc->nanny_state)
				block = true;
			if (d->character->pc->create_state == STATE_APPLYING)
				block = true;
			if (IS_SET (d->character->action, PLR_QUIET))
				block = true;
			if (d->character->subdue && IS_NPC (d->character->subdue))
				block = true;
			if (d->character->following && IS_NPC (d->character->following))
				block = true;
		}

		argument = one_argument (argument, arg);
		if (block && *arg != '!')
		{
			if (pending_reboot)
			{
				ch->send_to_char ("There is already a reboot pending.\n");
				return;
			}
			ch->send_to_char ("\n");
			sprintf (buf, "%s has queued a server reboot.", ch->name);
			send_to_gods (buf);
			pending_reboot = true;
			return;
		}
		if (!(fs = fopen (".copyover_data", "w")))
		{
			ch->send_to_char
				("#1Copyover file not writeable; reboot aborted. Please notify the staff.#0\n");
			system_log ("Error creating copyover file.", true);
		}
		if (fs)
			fclose (fs);
		if (!(fs = fopen (".reboot", "w")))
		{
			system_log ("Error creating reboot file.", true);
			return;
		}
		if (ch->desc->original)
			sprintf (buf, "%s\n", (ch->desc->original)->name);
		else
			sprintf (buf, "%s\n", ch->name);
		fputs (buf, fs);
		fclose (fs);
		unlink ("booting");
		prepare_copyover (0);
	}
	else
		ch->send_to_char ("Specify shutdown parameter:  Die or Reboot ?\n");
}


void
do_watch (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	AFFECTED_TYPE *af;
	CHAR_DATA *tch;

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->clear_watch();
		ch->send_to_char ("Ok.\n");
		return;
	}

	if (!(tch = get_pc (buf)))
	{
		ch->send_to_char ("PC not found.  Did you enter the full name?\n");
		return;
	}

	if ((af = get_affect (tch, MAGIC_WATCH1)) && af->a.spell.t == (long int) ch)
	{
		ch->send_to_char ("Watch removed.\n");
		affect_remove (tch, af);
		return;
	}

	if ((af = get_affect (tch, MAGIC_WATCH2)) && af->a.spell.t == (long int) ch)
	{
		ch->send_to_char ("Watch removed.\n");
		affect_remove (tch, af);
		return;
	}

	if ((af = get_affect (tch, MAGIC_WATCH3)) && af->a.spell.t == (long int) ch)
	{
		ch->send_to_char ("Watch removed.\n");
		affect_remove (tch, af);
		return;
	}

	if (!get_affect (tch, MAGIC_WATCH1))
	{
		magic_add_affect (tch, MAGIC_WATCH1, -1, 0, 0, 0, 0);
		get_affect (tch, MAGIC_WATCH1)->a.spell.t = (long int) ch;
		ch->send_to_char ("Watch added.\n");
	}

	else if (!get_affect (tch, MAGIC_WATCH2))
	{
		magic_add_affect (tch, MAGIC_WATCH2, -1, 0, 0, 0, 0);
		get_affect (tch, MAGIC_WATCH2)->a.spell.t = (long int) ch;
		ch->send_to_char ("Watch added.\n");
	}

	else if (!get_affect (tch, MAGIC_WATCH3))
	{
		magic_add_affect (tch, MAGIC_WATCH3, -1, 0, 0, 0, 0);
		get_affect (tch, MAGIC_WATCH3)->a.spell.t = (long int) ch;
		ch->send_to_char ("Watch added.\n");
	}

	else
	{
		ch->send_to_char ("Three other people watching...try snooping.\n");
		return;
	}
}

void
do_snoop (CHAR_DATA * ch, char *argument, int cmd)
{
	char arg[MAX_STRING_LENGTH]= { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };
	CHAR_DATA *victim;

	if (!ch->desc)
		return;

	one_argument (argument, arg);

	if (!*arg)
		strcpy (arg, ch->name);

	if (IS_NPC (ch))
	{
		ch->send_to_char ("Mixing snoop and switch is bad for your health.\n");
		return;
	}

	if (!(victim = get_char_vis (ch, arg)))
	{
		ch->send_to_char ("No such person around.\n");
		return;
	}

	if (!victim->desc)
	{
		ch->send_to_char ("There's no link.. nothing to snoop.\n");
		return;
	}

	if (!victim->pc)
	{
		ch->send_to_char ("Sorry... can only snoop PCs!\n");
		return;
	}

	if (victim == ch)
	{

		ch->send_to_char ("You stop snooping.\n");

		if (ch->desc->snoop.snooping)
		{
			if (ch->desc->snoop.snooping->pc->level)
			{
				sprintf (buf, "%s stops snooping you.\n", ch->name);
				ch->desc->snoop.snooping->send_to_char (buf);
			}
			ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
			ch->desc->snoop.snooping = 0;
		}

		return;
	}

	if (victim->desc->snoop.snoop_by)
	{
		sprintf (buf, "%s is snooping already.\n",
			(victim->desc->snoop.snoop_by)->name);
		ch->send_to_char (buf);
		return;
	}

	if (victim->pc->level)
	{
		sprintf (buf, "%s is snooping you.\n", ch->name);
		victim->send_to_char (buf);
	}

	ch->send_to_char ("Ok.\n");

	if (ch->desc->snoop.snooping)
		ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;

	ch->desc->snoop.snooping = victim;
	victim->desc->snoop.snoop_by = ch;
	return;
}

void
do_summon (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *victim;
	AFFECTED_TYPE *af;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char ("Summon whom?\n");
		return;
	}

	victim = get_char (buf);

	if (!victim)
	{
		ch->send_to_char ("Couldn't find that character.\n");
		return;
	}

	if (victim == ch)
	{
		ch->send_to_char ("You cannot summon yourself.\n");
		return;
	}

	if (victim->in_room == ch->in_room)
	{
		ch->act("$N is already here.", false, 0, victim, TO_CHAR);
		return;
	}

	if ((af = get_affect (victim, MAGIC_STAFF_SUMMON)))
		af->a.spell.t = victim->in_room;
	else
	{
		magic_add_affect (victim, MAGIC_STAFF_SUMMON, 24, 0, 0, 0, 0);
		af = get_affect (victim, MAGIC_STAFF_SUMMON);
		af->a.spell.t = victim->in_room;
	}

	victim->char_from_room();
	victim->char_to_room(ch->in_room);
	
	
	if (victim->hit <= 0)
		victim->hit = 1;

	if (!victim->is_awake())
	{
		victim->set_position(STAND);
	}

	ch->act("$N has been summoned.", false, 0, victim, TO_CHAR);
	ch->act("$n summons $N.", false, 0, victim, TO_NOTVICT);
	victim->act("You have been summoned by $N.", true, 0, ch, TO_CHAR);

	victim->send_to_char ("Use the RETURN command to return to your former location "
		"when you are ready.\n");
	
}

void
do_switch (CHAR_DATA * ch, char *argument, int cmd)
{
	char arg[MAX_STRING_LENGTH]= { '\0' };
	CHAR_DATA *victim;

	one_argument (argument, arg);

	if (!*arg)
	{
		char__switch_item (ch, argument, cmd);
		return;
	}

	if (IS_NPC (ch))
		return;

	if ((!ch->get_trust()))
	{
		ch->send_to_char ("Eh?\n");
		return;
	}

	if (ch->pc && (ch->pc->create_state == STATE_DIED || ch->pc->create_state == STATE_RETIRED))
	{
		ch->send_to_char ("Into a dead body? Ewwwwhh!\n");
		return;
	}

	if (!(victim = get_char_room_vis (ch, arg)))
		if (!(victim = get_char (arg)))
		{
			ch->send_to_char ("They aren't here.\n");
			return;
		}

		if (ch == victim)
		{
			ch->send_to_char
				("Seek some help bud, no multi-personality disorders allowed here.\n");
			return;
		}

		if (!ch->desc || ch->desc->snoop.snoop_by || ch->desc->snoop.snooping)
		{
			ch->send_to_char ("Mixing snoop & switch is bad for your health.\n");
			return;
		}

		if (victim->desc || (!IS_NPC (victim) && !victim->pc->admin_loaded))
			ch->send_to_char ("You can't do that, the body is already in use!\n");
		else
		{
			ch->send_to_char ("Ok.\n");

			ch->desc->character = victim;
			ch->desc->original = ch;
			if (ch->desc->original->color)
				victim->color = 1;

			victim->desc = ch->desc;
			victim->petition_flags = ch->petition_flags; /* copy over display of flags */
			ch->desc = 0;
		}
}

void
do_return (CHAR_DATA * ch, char *argument, int cmd)
{
	AFFECTED_TYPE *af;

		if (ch->desc && ch->desc->original && is_he_somewhere (ch->desc->original))
	{
		ch->send_to_char ("You return to your original character.\n");
		ch->desc->character = ch->desc->original;
		ch->desc->original = NULL;
		ch->desc->character->desc = ch->desc;
		ch->desc = NULL;
		return;
	}

	if ((af = get_affect (ch, MAGIC_STAFF_SUMMON)))
	{
		ch->act("$n unsummons $mself.", false, 0, 0, TO_ROOM);
		ch->char_from_room();
		ch->char_to_room(af->a.spell.t);
		ch->act("$n appears.", false, 0, 0, TO_ROOM);
		do_look (ch, "", 0);
		affect_remove (ch, af);
		return;
	}

	if ((!ch->get_trust()))
		ch->send_to_char ("You are where you should be.\n");
	else
		ch->send_to_char ("You are what you should be.\n");
}


	//TODO: once we can see who leads a group, so we can use "as", we won't need this.
	//Use only to force players, and only by name.
void
do_force (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *vict;
	char name[100], to_force[100], buf[100];

	half_chop (argument, name, to_force);

	if (!*name || !*to_force)
		ch->send_to_char ("Who do you wish to force to do what?\n");
	
	else 
	{
		if (!(vict = get_char_room_vis (ch, name)))
			ch->send_to_char ("No-one by that name here.\n");
		else
		{
			if (vict == ch)
			{
				ch->send_to_char ("Force yourself?\n");
				return;
			}
			if (ch->get_trust() < vict->get_trust())
				vict = ch;
			sprintf (buf, "$n has forced you to '%s'.", to_force);
			ch->act(buf, false,  0, vict, TO_VICT);
			ch->send_to_char ("Ok.\n");
			command_interpreter (vict, to_force);
		}
	}
	
	/**************
	else if (!str_cmp ("room", name))
	{
		for (vict = ch->room->people; vict; vict = vict->next_in_room)
		{
			if (ch->get_trust() (vict) < ch->get_trust())
				command_interpreter (vict, to_force);
		}
	}
	 
	else
	 {				// force all 
		for (i = descriptor_list; i; i = i->next)
		{
			if (i->character != ch && !i->connected)
			{
				vict = i->character;
				if (ch->get_trust() < ch->get_trust() (vict))
					ch->send_to_char ("Oh no you don't!!\n");
				else
				{
					sprintf (buf, "$n has forced you to '%s'.", to_force);
					ch->act(buf, false,  0, vict, TO_VICT);
					command_interpreter (vict, to_force);
				}
			}
		}
		ch->send_to_char ("Ok.\n");
	}
	 /*************/
}

void
do_as (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	DESCRIPTOR_DATA *save_d;

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char ("Syntax:  as <character> <command>\n");
		return;
	}

	if (!(tch = get_char_room_vis (ch, buf)))
	{
		ch->send_to_char ("They aren't here.\n");
		return;
	}

	if (!IS_NPC (tch) && (tch->get_trust() > 5))
	{
		tch = ch;
	}

	if (!IS_NPC (tch) && !IS_NPC (ch))
	{
		if (tch->pc->level > ch->pc->level)
		{
			ch->send_to_char ("I'm sure they wouldn't appreciate that.\n");
			return;
		}
	}

	one_argument (argument, buf);

	if (!str_cmp (buf, "switch"))
	{
		ch->send_to_char ("Naughty genie...\n");
		return;
	}

	if (!str_cmp (buf, "quit"))
	{
		ch->send_to_char ("Oh, not quit...that would crash the mud...use force "
			"instead.\n");
		return;
	}

	save_d = tch->desc;
	tch->desc = ch->desc;

	command_interpreter (tch, argument);

	tch->desc = save_d;
}


OBJ_DATA *
get_eq_by_type_and_name (CHAR_DATA * ch, int type, char *name)
{
	int i = 0;
	OBJ_DATA *eq = NULL;

	if (!type && !*name)
	{
		return NULL;
	}

	for (i = 0; i < MAX_WEAR; i++)
	{

		if (!(eq = get_equip (ch, i)))
		{
			continue;
		}

		/* If we are looking for a particular type */
		if (type && (eq->obj_flags.type_flag != type))
		{
			continue;
		}

		/* If we are looking for a specific name */
		if (name && *name)
		{
			if (isname (name, eq->name))
			{

				return eq;
			}
			else
			{
				continue;
			}
		}
		else
		{

			return eq;
		}

	}

	return NULL;
}


void
do_load (CHAR_DATA * ch, char *argument, int cmd)
{
	if ((!ch->get_trust()))
	{
		return;
	}
	else if (argument && (*argument == 'o' || *argument == 'm'))
	{
		_do_load (ch, argument, cmd);
	}
	
}

void
_do_load (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *mob = NULL;
	OBJ_DATA *obj = NULL;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char load_buf[MAX_STRING_LENGTH]= { '\0' };
	char type[MAX_STRING_LENGTH]= { '\0' };
	char num[MAX_STRING_LENGTH]= { '\0' };
	char second_value[MAX_STRING_LENGTH]= { '\0' };
	int vnumber, i;
	int count = 1;
	
		//type  is 'm' or 'o'
	argument = one_argument (argument, type);
	
		//num is either vnum or a count
	argument = one_argument (argument, num);
	
		//argument now holds remainder of args

	
	if (!*type || !*num || *type == '?')
	{
		ch->send_to_char ("Syntax:\n"
					  "   load 'mob' <number>\n"
					  "   load 'obj' <count> <vnum> [target] (variants)\n variant must be include parenthese\n");
		return;
	}
	
	vnumber = atoi (num);
	
		//load m <vnum>
	if (vnumber < 0)
	{
		ch->send_to_char ("Vnum must be greater than 0\n");
		return;
	}
	
		//all mobiles loaded are automatically stayputted
	else if (is_abbrev (type, "mobile"))
	{
		if (!(mob = load_mobile (vnumber)))
		{
			ch->send_to_char ("There is no mobile with that number.\n");
			return;
		}
		
		mob->char_to_room(ch->in_room);
		mob->mob->action |= ACT_STAYPUT;
		mob->mob->spawnpoint = ch->in_room;
		save_stayput_mobiles ();
		
		ch->act("$N immediately appears!", false, 0, mob, TO_ROOM);
		ch->act("$N appears.", false, 0, mob, TO_CHAR);
		
	}
	
	else if (is_abbrev (type, "obj"))
	{
		
			//we know we have at least one number, and may have more args
			//which could include a keyword
			//load o <count> <vnum>
			//load o <count> <vnum> <variant values>
			//load o <count> <vnum> <keyword> <variants values>
			//load o <vnum> <keyword> <variants values>
			//load o <vnum> <variants values>
		
		argument = one_argument (argument, second_value);
		
			//if we have two numbers in a row,
			//first will be count and the second is the vnum
		if (isdigit (*second_value))
		{
			count = vnumber;
			vnumber = atoi (second_value);
			argument = one_argument(argument, buf);
		}
		else
		{
			count = 1;
			sprintf(buf, "%s", second_value);
		}
		
			//argument is empty
			//---- no more infomation, load basic object in room
			// or load random variants
		if (!*buf)
		{
			obj = vtoo(vnumber);
			
			if (!obj)
			{
				ch->send_to_char ("There is no object with that number.\n");
				return;
			}
			
			if (obj->zone == 99)
			{
				ch->send_to_char ("That object has not been evaluated yet, and cannot be loaded.\n");
				return;
			}
			
			for (i = 1; i <= count; i++)
			{
				obj = load_colored_object(vnumber, 0);
				obj_to_room (obj, ch->in_room);
				ch->act("$p immediately appears!", false, obj, 0, TO_ROOM);
				ch->act("$p appears.", false, obj, 0, TO_CHAR);
			}
			
		}
			//buf could be mob keyname
			//----give basic object to the mob
			//buf could be variant value
			//----load random variant object to room
			//load each one individually, since they are random variants
		else if (*buf && !*argument)
		{
			mob = get_char_room_vis (ch, buf);
			if (!mob)
			{
					//buf is single variant value
					//we load to the room if it exists
					//test to see if the object exists
				obj = vtoo(vnumber);
				if (!obj)
				{
					ch->send_to_char ("There is no object with that number or variant value.\n");
					return;
				}
				
				if (obj->zone == 99)
				{
					ch->send_to_char ("That object has not been evaluated yet, and cannot be loaded.\n");
					return;
				}
				
				for (i = 1; i <= count; i++)
				{
					obj = load_colored_object (vnumber, buf);
					obj_to_room (obj, ch->in_room);
					ch->act("$p immediately appears!", false, obj, 0, TO_ROOM);
					ch->act("$p appears.", false, obj, 0, TO_CHAR);
				}

			}
			else
			{
					//buf is the mob
					//obj has no variants
					//we load basic object to mob or send random variant
					//test if the obj exists
				obj = vtoo(vnumber);
				if (!obj)
				{
					ch->send_to_char ("There is no object with that number or variant value.\n");
					return;
				}
				
				if (obj->zone == 99)
				{
					ch->send_to_char ("That object has not been evaluated yet, and cannot be loaded.\n");
					return;
				}
				
				if (IS_SET (obj->obj_flags.extra_flags, ITEM_STACK))
				{
					obj = load_object_full(vnumber, 0, count);
					mob->act("$p appears in your hands.", true, obj, ch, TO_CHAR);
					if (ch != mob)
						ch->act("$p was created in $N's hands.",
							 true, obj, mob, TO_CHAR);
					obj_to_char (obj, mob);
				}
				else
				{
					for (i = 1; i <= count; i++)
					{
					obj = load_object_full(vnumber, 0, 1);
					mob->act("$p is given to you.", true, obj, ch, TO_CHAR);
					if (ch != mob)
						ch->act("$p was given to $N.",
							 true, obj, mob, TO_CHAR);
					obj_to_char (obj, mob);					
					}
				}
			}
		}
			//argument exist
			//buf is mob keyname, argument is variant values list
			//----give specified variant object to the mob
			//buf is NOT a mob
			//----load specified variant object to room
			//if count, then all items wll be the same variant value
		else 
		{
			obj = vtoo(vnumber);
			if (!obj)
			{
				ch->send_to_char ("There is no object with that number or that variant values.\n");
				return;
			}
			
			if (obj->zone == 99)
			{
				ch->send_to_char ("That object has not been evaluated yet, and cannot be loaded.\n");
				return;
			}
			
			mob = get_char_room_vis (ch, buf);
			if (!mob)
			{
					//make a single variant arg list since buf is part of list
				
				sprintf(load_buf, "%s %s", buf, argument);
			}
			else 
			{
				sprintf(load_buf, "%s", argument);
			}

				//load to the mob if we have one, else load to room
			if (mob)
			{
				for (i = 1; i <= count; i++)
				{
					obj = load_colored_object (vnumber, load_buf); 
					mob->act("$p appears in your hands.", true, obj, ch, TO_CHAR);
					if (ch != mob)
						ch->act("$p was created in $N's hands.",
							 true, obj, mob, TO_CHAR);
					obj_to_char (obj, mob);
				}
			}
			else
			{
				for (i = 1; i <= count; i++)
				{
					obj = load_colored_object (vnumber, load_buf); 
					obj_to_room (obj, ch->in_room);
					ch->act("$p immediately appears!", false, obj, 0, TO_ROOM);
					ch->act("$p appears.", false, obj, 0, TO_CHAR);
				}
			}
		}
	}
	
	else
	{
		ch->send_to_char ("Syntax:\n"
					  "   load 'mob' <number>\n"
					  "   load 'obj' <count> <vnum> ");
		return;
	}	
}


// This leaves a private room designed for staff to load things for an offline player. This special function is required, so that the room is deleted.
void do_leaveprivate(CHAR_DATA* ch, char* argument, int cmd)
{
	if (ch->in_room < 100000)
	{
		ch->send_to_char ("You are not in a private room.\n");
		return;
	}

	ROOM_DATA* currentRoom = vtor(ch->in_room);
	if (currentRoom==NULL)
	{
		ch->send_to_char ("Error accessing your current room.\n");
		return;
	}

	if (currentRoom->dir_option[0]==NULL)
	{
		ch->send_to_char ("You do not seem to have an exit storing your old location.\n");
		return;
	}

	int targetRoom = currentRoom->dir_option[0]->to_room;
	if (targetRoom<0)
	{
		ch->send_to_char ("Error loading the room to which you are to be restored.\n");
		return;
	}

	// Move the character to the room specified in the exit and 
	ch->char_from_room();
	ch->char_to_room(targetRoom);
	
		// Force them to look
	do_look(ch,NULL,0);
	ch->act("$n enters the area.", true, 0, 0, TO_ROOM);

	// Evict others from the room, taking them to wherever the player went
	CHAR_DATA *tch = NULL, *tch_next=NULL;
	for (tch = currentRoom->people; tch; tch = tch_next)
	{
		tch_next = tch->next_in_room;
		if (!IS_NPC (tch))
		{
			tch->char_from_room();
			tch->char_to_room(targetRoom);
			// Force them to look
			do_look(tch,NULL,0);
		}
	}

	// Delete the room they came from (currentRoom)
	room_delete(currentRoom);

	//Update list immediately
	save_dwelling_rooms();
}

// This makes a private room for the player and stores their previous location. The intent is to run this on an offline player
// and load things in this room for them. They can get them, and then leaveprivate to go back to where they came from. The exit
// is blocked off so they have to use leaveprivate, which is necessary to get this temporary room to delete.
void do_makeprivate(CHAR_DATA* ch, char* argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *tch;

	if (IS_NPC (ch))
	{
		ch->send_to_char ("This is a PC only command.\n");
		return;
	}

	
	// Load the PC by proper name only
	argument = one_argument (argument, buf);
	if (!(tch = load_pc (buf)))
	{
		ch->send_to_char ("That PC was not found.\n");
		return;
	}

	// Create a private room copy
	int new_room = -1;
	
	if ((new_room =clone_contiguous_rblock (vtor (PREGAME_ROOM_PROTOTYPE), -1)) < 0)
	{
		ch->send_to_char ("Error creating a private room.\n");
		return;
	}

	ROOM_DATA* to_room = vtor(new_room);
	if (to_room==NULL)
	{
		ch->send_to_char ("Error creating a private room.\n");
		return;
	}

	// Link the new room to their current location
	if (to_room->dir_option[0] != NULL)
		free_mem(to_room->dir_option[0]);

		// start with the door closed and locked.
		// This is to allow the exit to save the room link but force them to use leaveprivate to leave the room
		// The purpose of that is to allow deletion of the room when they've left it.
		// No possible key to circumvent this

	to_room->dir_option[0] = new room_exit_data;
	to_room->dir_option[0]->keyword = duplicateString("door");
	to_room->dir_option[0]->port_flags = EX_ISDOOR | EX_CLOSED | EX_LOCKED;
	to_room->dir_option[0]->to_room = tch->in_room;


	// Transfer the admin into that private room
	ch->char_from_room();
	ch->char_to_room(to_room->nVirtual);

	// Clear its contents
	OBJ_DATA *obj=NULL, *next_o=NULL;
	for (obj = to_room->contents; obj; obj = next_o)
	{
		next_o = obj->next_content;
		extract_obj (obj);
	}

	// Turn OOC off, so they can get things
	to_room->room_flags &= ~OOC;

	// Replace desc
	free_mem(to_room->description);
	to_room->description = duplicateString("You were placed in this private room while you were offline so an admin could\nprovide you with the following item(s). Please take them and use\n#6leaveprivate#0 when done.\n");

	// Replace title to avoid auto messages of "Welcome" and "look sign" etc
	free_mem(to_room->name);
	to_room->name = duplicateString("An offline loading room");

	// Place the player's record in that room
	tch->in_room = to_room->nVirtual;
	tch->unload_pc();

	// Inform the admin
	std::ostringstream oss;
	oss << "\n#6OOC: This room has been created for the use of " << buf << "." << std::endl
		<< "Load anything you wish for them to have into here, and they may use" << std::endl
		<< "#2leaveprivate#6 to exit back to where they came from" << std::endl
		<< "in the game world.#0\n" << std::endl;
	ch->send_to_char(oss.str().c_str());

	// Force them to look
	do_look(ch,NULL,0);

	//Update list immediately
	save_dwelling_rooms();
}

/* clean a room of all mobiles and objects */
void
do_purge (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *vict, *next_v;
	OBJ_DATA *obj, *next_o;
	char name[100];

	if (IS_NPC (ch))
		return;

	one_argument (argument, name);

	if (*name)			/* argument supplied. destroy single object or char */
	{
		if ((vict = get_char_room_vis (ch, name)))
		{
			if (!IS_NPC (vict) && !vict->pc->admin_loaded)
			{
				ch->send_to_char ("\nNow, now. Let's not be hostile, mmmkay?\n");
				return;
			}
			ch->act("$n disintegrates $N.", false, 0, vict, TO_ROOM);
			ch->act("You disintegrate $N.", false, 0, vict, TO_CHAR);
			vict->extract_char();
		}

		else if ((obj = get_obj_in_list_vis (ch, name,
			vtor (ch->in_room)->contents)))
		{
			ch->act("$n destroys $p.", false, obj, 0, TO_ROOM);
			ch->act("You destroy $p.", false, obj, 0, TO_CHAR);
			extract_obj (obj);
		}
		else
		{
			ch->send_to_char ("I don't know anyone or anything by that name.\n");
			return;
		}

	}
	else				/* no argument. clean out the room */
	{
		if (IS_NPC (ch))
		{
			ch->send_to_char ("Don't... You would only kill yourself..\n");
			return;
		}

		if (!ch->room->contents)
		{
			ch->send_to_char
				("There aren't any objects that need purging in this room.\n");
			return;
		}

		ch->act
			("$n briefly narrows $s eyes. A moment later, the clutter in the area dissipates away in a glimmering haze of binary code.",
			false, 0, 0, TO_ROOM | _ACT_FORMAT);
		ch->act
			("You narrow your eyes, concentrating. A moment later, the clutter in the area dissipates away in a glimmering haze of binary code.",
			false, 0, 0, TO_CHAR | _ACT_FORMAT);

		for (vict = vtor (ch->in_room)->people; vict; vict = next_v)
		{
			next_v = vict->next_in_room;
			if (IS_NPC (vict))
			{
			ch->act("$N is a master mobile.  Kill it if you really want "
					"it destroyed.",
					false, 0, vict, TO_CHAR | _ACT_FORMAT);
				
			}
		}

		for (obj = vtor (ch->in_room)->contents; obj; obj = next_o)
		{
			next_o = obj->next_content;
			extract_obj (obj);
		}
	}
}

void
do_restore (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *victim;
	char buf[100];

	one_argument (argument, buf);
	if (!*buf)
		ch->send_to_char ("Who do you wish to restore?\n");
	else
	{
		if (!(victim = get_char_room_vis (ch, buf)))
			ch->send_to_char ("I don't see that person here.\n");
		else
		{
			victim->move = victim->max_move;
			ch->send_to_char ("Done.\n");
			victim->act("You have been fully healed by $N!",
				false, 0, ch, TO_CHAR);
			victim->set_position(STAND);
		}
	}
}

void
do_invis (CHAR_DATA * ch, char *argument, int cmd)
{
	/* This command is now open to all, but generates the typical failures if
	not of race 77 - Istari. This race is used to assign to PP characters not
	given immortality */

	if ((!ch->get_trust()) && ch->race != 77)
	{
		int echo = number (1, 9);
		if (echo == 1)
			ch->send_to_char ("Eh?\n");
		else if (echo == 2)
			ch->send_to_char ("Huh?\n");
		else if (echo == 3)
			ch->send_to_char ("I'm afraid that just isn't possible...\n");
		else if (echo == 4)
			ch->send_to_char ("I don't recognize that command.\n");
		else if (echo == 5)
			ch->send_to_char ("What?\n");
		else if (echo == 6)
			ch->send_to_char
			("Perhaps you should try typing it a different way?\n");
		else if (echo == 7)
			ch->send_to_char
			("Try checking your typing - I don't recognize it.\n");
		else if (echo == 8)
			ch->send_to_char
			("That isn't a recognized command, craft, or social.\n");
		else
			ch->send_to_char ("Hmm?\n");
		return;
	}

	if (!IS_SET (ch->flags, FLAG_WIZINVIS))
	{
		ch->flags |= FLAG_WIZINVIS;
		ch->send_to_char ("You dematerialize.\n");
	}
	else
	{
		ch->send_to_char ("You are already invisible.\n");
	}
}

void
do_vis (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	AFFECTED_TYPE *af;

	if ((af = get_affect (ch, MAGIC_AFFECT_INVISIBILITY)))
		affect_remove (ch, af);

	if (IS_SET (ch->flags, FLAG_WIZINVIS))
	{
		if (ch->race == 77)
		{
			ch->send_to_char("That wouldn't be wise!\n");
			return;
		}

		ch->flags &= ~FLAG_WIZINVIS;
		remove_affect_type (ch, MAGIC_HIDDEN);

		strcpy (buf, "$n materializes before your eyes.");
		ch->act(buf, false,  0, 0, TO_ROOM);

		ch->send_to_char ("You materialize.\n");
	}

	else if (get_affect (ch, MAGIC_HIDDEN))
	{
		remove_affect_type (ch, MAGIC_HIDDEN);
		ch->send_to_char ("You reveal yourself.\n");
		ch->act("$n reveals $mself.", false, 0, 0, TO_ROOM);
	}

	else
		ch->send_to_char ("You are already visible.\n");
}


void
read_race_data (CHAR_DATA * ch)
{
	FILE *fp;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char *p;
	int vnum;
	char race[MAX_STRING_LENGTH]= { '\0' };
	int count = 0;
	CHAR_DATA *mob;
	int ind;

	if (!(fp = fopen ("race.done", "r")))
	{
		perror ("lib/race.done");
		return;
	}

	while (fgets (buf, 256, fp))
	{

		p = one_argument (buf, race);

		vnum = atoi (race);

		p = one_argument (p, race);

		ind = lookup_race_id (race);

		if (ind == -1)
			printf ("%05d  %15s; INVALID RACE NAME.", vnum, race);
		else
			printf ("%05d  %15s = %s", vnum, race,
			lookup_race_variable (ind, RACE_NAME));

		if (!(mob = vtom (vnum)))
			printf ("INVALID VNUM!\n");
		else
		{
			if (ind != -1)
				mob->race = ind;
			printf ("\n");
		}

		count++;
	}

	fclose (fp);

	printf ("%d mobs processed.\n", count);
}

void
debug_info (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *target;
	CHAR_DATA *other;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char ("debug info <target n/pc> [<other n/pc>]\n");
		ch->send_to_char ("   Perception is of target on other.\n");
		return;
	}

	if (!(target = get_char_room (buf, ch->in_room)))
	{
		ch->send_to_char ("No such target.\n");
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf)
		other = ch;
	else if (!(other = get_char_room (buf, ch->in_room)))
	{
		ch->send_to_char ("No such other.\n");
		return;
	}

	sprintf (buf, "is_brother (target, other):   %s\n",
		is_brother (target, other) ? "true" : "false");
	ch->send_to_char (buf);

	sprintf (buf, "is_higher_rank (target, other):     %s\n",
		is_higher_rank (target, other) ? "true" : "false");
	ch->send_to_char (buf);

	sprintf (buf, "is_area_enforcer (target):    %s\n",
		is_area_enforcer (target) ? "true" : "false");
	ch->send_to_char (buf);

	sprintf (buf, "is_area_leader (target):      %s\n",
		is_area_leader (target) ? "true" : "false");
	ch->send_to_char (buf);
}



void
do_debug (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	char output[MAX_INPUT_LENGTH] = {'\0'};
	CHAR_DATA * tmob;
	VARIANT_VALUE* tvariant;
	CLAN_DATA* tclan_data;
	std::map<int, VARIANT_VALUE*>::iterator var_it;
	std::map<std::string, CLAN_DATA*>::iterator clan_it;
	std::list<CHAR_DATA*>::iterator tch_iterator;
	char tempbuf[MAX_INPUT_LENGTH];
	
	
	argument = one_argument (argument, buf);
	
		//if (!str_cmp(buf, "lua"))
			//lua_test(ch);
	
	if (!str_cmp (buf, "charlist"))
	{
		for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
		{
			tmob = *tch_iterator;
			sprintf(tempbuf, "name - %s\n", tmob->name);
			send_to_imps(tempbuf);
		}
	}
	if (!str_cmp (buf, "info"))
	{
		debug_info(ch, argument);
	}
	
	if (!str_cmp (buf, "ranks"))
	{
		
		if(clan_data_map.empty())
		{
			send_to_imps("clan_data_map is empty\n");
			return;
		}
		else 
		{			
			for (clan_it = clan_data_map.begin(); clan_it != clan_data_map.end(); clan_it++)
			{
				tclan_data = clan_it->second;
				sprintf(output+strlen(output), "%s (%d)\n",
						tclan_data->name,
						tclan_data->id);
				
				for(int i = 0; i<10; i++)
				{
					sprintf(output+strlen(output), "%d - %s\n",
							i,
							tclan_data->rank[i] ? tclan_data->rank[i] : "none");
				}
			}
		}
		page_string (ch->desc, output);
		return;
	}
	
	if (!str_cmp (buf, "variant"))
	{
		
		argument = one_argument (argument, tempbuf);

		if(gl_variant.empty())
		{
			send_to_imps("variant list is empty\n");
		}
		else 
		{			
			for (var_it = gl_variant.begin(); var_it != gl_variant.end(); var_it++)
			{
				tvariant = var_it->second;
				if(!str_cmp (tempbuf, tvariant->name))
				{
					sprintf(output+strlen(output), "%s \n", tvariant->var_value);
				}
				else if (!*tempbuf)
					sprintf(output+strlen(output), "%s    %s \n", tvariant->name, tvariant->var_value);
				
			}
			page_string (ch->desc, output);
		}
		return;
	}
	

}

void
do_hour (CHAR_DATA * ch, char *argument, int cmd)
{
	next_hour_update = time (0);
	next_minute_update = time (0);

	times_do_hour_called++;
}

void
do_day (CHAR_DATA * ch, char *argument, int cmd)
{
	int i = 0, times = 0;

	if (*argument && isdigit (*argument))
		times = atoi (argument);

	if (!times)
		times = 1;

	for (i = 1; i <= times; i++)
	{

		time_info.hour = 24;

		next_hour_update = time (0);
		next_minute_update = time (0);

		weather_and_time (1);
	}
}

void
char__unbalance (CHAR_DATA * ch, float nMultiplier)
{

	float nPenalty = 0.0;

	if (ch->agi <= 9)
		nPenalty = 15;
	else if (ch->agi > 9 && ch->agi <= 13)
		nPenalty = 13;
	else if (ch->agi > 13 && ch->agi <= 15)
		nPenalty = 11;
	else if (ch->agi > 15 && ch->agi <= 18)
		nPenalty = 9;
	else
		nPenalty = 7;

	ch->balance =
		MAX (ch->balance - (int) (nPenalty * nMultiplier), -50);
}



void
do_set (CHAR_DATA * ch, char *argument, int cmd)
{
	int ind;
	char buf[MAX_INPUT_LENGTH];
	char subcmd[MAX_INPUT_LENGTH];

	argument = one_argument (argument, subcmd);

		

		//Informative
	if (!str_cmp (subcmd, "scan"))
	{
		if (!IS_SET (ch->plr_flags, QUIET_SCAN))
		{
			ch->plr_flags |= QUIET_SCAN;
			ch->send_to_char
				("You will now scan surrounding rooms.\n");
			return;
		}

		ch->plr_flags &= ~QUIET_SCAN;
		ch->send_to_char
			("You will no longer scan surrounding rooms.\n");
		return;
	}

	else if (!str_cmp (subcmd, "hints"))
	{
		if (!IS_SET (ch->plr_flags, NEWBIE_HINTS))
		{
			ch->plr_flags |= NEWBIE_HINTS;
			ch->send_to_char
				("You will now periodically receive game- and syntax-related hints.\n");
			return;
		}
		ch->plr_flags &= ~NEWBIE_HINTS;
		ch->send_to_char
			("You will no longer receive game- and syntax-related hints.\n");
		return;
	}

	else if (!str_cmp (subcmd, "newbchat"))
	{
		if (IS_SET (ch->plr_flags, NEW_PLAYER_TAG)
			||IS_SET (ch->plr_flags, MENTOR))
		{
			if (!IS_SET (ch->plr_flags, NEWBCHAT))
			{
				ch->plr_flags |= NEWBCHAT;
				ch->send_to_char("You are now tuned into the newbie chat channel.\n");
				return;
			}
			ch->plr_flags &= ~NEWBCHAT;
			ch->send_to_char("You will no longer tuned into the newbie chat channel.\n");
			return;
		}
		else 
		{
			ch->send_to_char("You need to be a new player or a Mentor to use this channel.\n");
			return;
		}

	}
	

	else if (!str_cmp (subcmd, "rpp") && ch->desc && ch->desc->acct)
	{
		if (IS_SET (ch->flags, FLAG_GUEST))
		{
			ch->send_to_char ("This option is unavailable to guest logins.\n");
			return;
		}

		if (ch->desc->acct->toggle_rpp_visibility ())
		{
			ch->send_to_char ("The output to SCORE will now show "
				"your number of roleplay points.\n");
		}
		else
		{
			ch->send_to_char ("Your roleplay points will no longer "
				"be visible in SCORE.\n");
		}
		return;
	}

	else if (!str_cmp (subcmd, "mute"))
	{
		if (!IS_SET (ch->plr_flags, MUTE_BEEPS))
		{
			ch->plr_flags |= MUTE_BEEPS;
			ch->send_to_char
			("You will now no longer hear system beeps upon receiving notifies.\n");
			return;
		}
		ch->plr_flags &= ~MUTE_BEEPS;
		ch->send_to_char
		("You will now hear system beeps upon receiving notifies.\n");
		return;
	}
	
	else if (!str_cmp (subcmd, "newbie"))
	{
		if (IS_SET (ch->plr_flags, NEW_PLAYER_TAG))
		{
			ch->plr_flags &= ~NEW_PLAYER_TAG;
			ch->send_to_char
			("The #2(new player)#0 tag has been removed from your long description.\n");
			return;
		}
		if (ch->get_trust())
		{
			ch->plr_flags |= NEW_PLAYER_TAG;
			ch->send_to_char ("New player tag has been enabled.\n");
			return;
		}
		ch->send_to_char
		("Shame on you - trying to pass yourself off as a clueless newbie?\n");
		return;
	}
	
	else if (!str_cmp (subcmd, "mentor"))
	{
		if (IS_SET (ch->flags, FLAG_GUEST))
		{
			ch->send_to_char ("Guests cannot toggle this flag.\n");
			return;
		}
		if (!IS_SET (ch->plr_flags, MENTOR))
		{
			ch->act
			("Your mentor flag has been enabled. Be SURE to read the policies listed in #6HELP MENTOR#0 - you WILL be held accountable to them. If you don't agree to them, please type SET MENTOR again now to remove the flag.",
			 false, 0, 0, TO_CHAR | _ACT_FORMAT);
			ch->plr_flags |= MENTOR;
			return;
		}
		ch->send_to_char ("Your mentor flag has been disabled.\n");
		ch->plr_flags &= ~MENTOR;
		
		if (IS_SET (ch->plr_flags, NEWBCHAT))
		{
			ch->send_to_char ("You are no longer tuned into the newbie chat channel.\n");
			ch->plr_flags &= ~NEWBCHAT;
		}
		
		return;
	}
	
	else if (!str_cmp (subcmd, "prompt"))
	{
		if (!GET_FLAG (ch, FLAG_NOPROMPT))
		{
			ch->flags |= FLAG_NOPROMPT;
			ch->send_to_char
			("Prompt disabled. Use the HEALTH command to check your character's welfare.\n");
			return;
		}
		else
			ch->flags &= ~FLAG_NOPROMPT;
		ch->send_to_char ("Informative prompt enabled.\n");
		return;
	}
		//Other
	else if (!str_cmp (subcmd, "ansi"))
	{
		if (!IS_SET (ch->flags, FLAG_GUEST))
		{
			ch->send_to_char ("This switch is only for guest users.\n");
			return;
		}
		if (ch->desc->color)
		{
			ch->desc->color = 0;
			ch->send_to_char ("ANSI color has been disabled.\n");
			return;
		}
		ch->desc->color = 1;
		ch->send_to_char ("#2ANSI color has been enabled.#0\n");
		return;
	}
	else if (ch->desc && !str_cmp (subcmd, "lines"))
	{
		
		argument = one_argument (argument, buf);
		
		if (!isdigit (*buf) || atoi (buf) > 79)
			ch->send_to_char ("Expected a number 1..79");
		else
			ch->desc->max_lines = atoi (buf);
	}
	
	else if (ch->desc && !str_cmp (subcmd, "columns"))
	{
		
		argument = one_argument (argument, buf);
		
		if (!isdigit (*buf) || atoi (buf) > 199)
			ch->send_to_char ("Expected a number 1..199");
		else
			ch->desc->max_columns = atoi (buf);
	}
	
	
		//Staff only
	else if (!str_cmp (subcmd, "wiznet")
		&& ((ch->pc && ch->pc->level)
		|| IS_SET (ch->flags, FLAG_ISADMIN)
		|| (IS_NPC (ch) && ch->desc->original)))
	{
		CHAR_DATA* real_ch =
			(IS_NPC(ch) && ch->desc->original)
			? ch->desc->original
			: ch;

		if (GET_FLAG (real_ch, FLAG_WIZNET))
		{
			real_ch->flags &= ~FLAG_WIZNET;
			ch->send_to_char ("You are no longer tuned into the wiznet.\n");
			sprintf (buf, "%s has left the wiznet channel.\n", real_ch->name);
			send_to_gods (buf);
		}

		else
		{
			real_ch->flags |= FLAG_WIZNET;
			ch->send_to_char ("You are now tuned into the wiznet.\n");
			sprintf (buf, "%s has rejoined the wiznet channel.\n",
				real_ch->name);
			send_to_gods (buf);
		}
		return;
	}

	else if (ch->pc && ch->pc->level && !str_cmp (subcmd, "names"))
	{
		if (GET_FLAG (ch, FLAG_SEE_NAME))
		{
			ch->flags &= ~FLAG_SEE_NAME;
			ch->send_to_char ("'SAY' will no longer show player names.\n");
		}

		else
		{
			ch->flags |= FLAG_SEE_NAME;
			ch->send_to_char ("'SAY' will show player names.\n");
		}

		return;
	}

	else if (ch->get_trust() && !str_cmp (subcmd, "telepath"))
	{
		if (GET_FLAG (ch, FLAG_TELEPATH))
		{
			ch->flags &= ~FLAG_TELEPATH;
			ch->send_to_char ("You will no longer receive PC thoughts.\n");
		}
		else
		{
			ch->send_to_char
				("You are now attuned to PC thoughts -- may the gods help you.\n");
			ch->flags |= FLAG_TELEPATH;
		}
	}
		//TODO- needs to be listed on info screen
	else if (ch->get_trust() && !str_cmp (subcmd, "guardian"))
	{

		argument = one_argument (argument, buf);

		if (!*buf)
		{
			/* Toggle ON | OFF */
			if (ch->guardian_mode)
			{
				ch->guardian_mode = 0;
			}
			else
			{
				ch->guardian_mode = 0xFFFF;
			}
		}

		else if (!str_cmp (buf, "on") || !str_cmp (buf, "all"))
		{
			ch->guardian_mode = 0xFFFF;
		}

		else if (!str_cmp (buf, "off") || !str_cmp (buf, "none"))
		{
			ch->guardian_mode = 0;
		}

		else if (!str_cmp (buf, "pc"))
		{
			TOGGLE_BIT (ch->guardian_mode, GUARDIAN_PC);
		}
		else if (!str_cmp (buf, "npc"))
		{
			if ((ch->guardian_mode & ~GUARDIAN_PC))
				ch->guardian_mode &= GUARDIAN_PC;
			else
			{
				ch->guardian_mode |= ~GUARDIAN_PC;
			}
		}
		else if (!str_cmp (buf, "humanoids"))
		{
			TOGGLE_BIT (ch->guardian_mode, GUARDIAN_NPC_HUMANOIDS);
		}
		else if (!str_cmp (buf, "wildlife"))
		{
			TOGGLE_BIT (ch->guardian_mode, GUARDIAN_NPC_WILDLIFE);
		}
		else if (!str_cmp (buf, "shopkeeps"))
		{
			TOGGLE_BIT (ch->guardian_mode, GUARDIAN_NPC_SHOPKEEPS);
		}
		else if (!str_cmp (buf, "sentinel"))
		{
			TOGGLE_BIT (ch->guardian_mode, GUARDIAN_NPC_SENTINELS);
		}
		else if (!str_cmp (buf, "keyholders"))
		{
			TOGGLE_BIT (ch->guardian_mode, GUARDIAN_NPC_KEYHOLDER);
		}
		else if (!str_cmp (buf, "enforcers"))
		{
			TOGGLE_BIT (ch->guardian_mode, GUARDIAN_NPC_ENFORCERS);
		}

		if (ch->guardian_mode == 0xFFFF)
		{
			ch->send_to_char
				("You will be informed of any significant PC initiated attacks.\n");
		}
		else if (ch->guardian_mode)
		{
			strcpy (buf, "You will be informed of PC attacks on:");

			if ((ch->guardian_mode & GUARDIAN_PC))
				strcat (buf, " PCs,");

			if ((ch->guardian_mode & ~GUARDIAN_PC) == 0xFFFE)
				strcat (buf, " NPCs.");
			else if ((ch->guardian_mode & ~GUARDIAN_PC))
			{
				strcat (buf, " NPC");
				if ((ch->guardian_mode & GUARDIAN_NPC_HUMANOIDS))
					strcat (buf, " Humanoids,");
				if ((ch->guardian_mode & GUARDIAN_NPC_WILDLIFE))
					strcat (buf, " Wildlife,");
				if ((ch->guardian_mode & GUARDIAN_NPC_SHOPKEEPS))
					strcat (buf, " Shopkeeps,");
				if ((ch->guardian_mode & GUARDIAN_NPC_SENTINELS))
					strcat (buf, " Sentinels,");
				if ((ch->guardian_mode & GUARDIAN_NPC_KEYHOLDER))
					strcat (buf, " Keyholders,");
				if ((ch->guardian_mode & GUARDIAN_NPC_ENFORCERS))
					strcat (buf, " Enforcers,");
			}
			buf[(strlen (buf) - 1)] = '\n';
			ch->send_to_char (buf);
		}
		else
		{
			ch->send_to_char
				("You will not be informed of any PC initiated attacks.\n");
		}

	}

	else if (ch->get_trust() && !str_cmp (subcmd, "immwalk"))
		ch->speed = SPEED_IMMORTAL;

	else if (ch->get_trust() && !str_cmp (subcmd, "available"))
	{
		if (IS_SET (ch->flags, FLAG_AVAILABLE))
		{
			ch->flags &= ~FLAG_AVAILABLE;
			ch->send_to_char
				("You will no longer register as available for petitions.\n");
		}

		else
		{
			ch->flags |= FLAG_AVAILABLE;
			ch->send_to_char
				("You will now be listed as available for petitions.\n");
		}

		return;
	}

	
	else if (ch->pc && ch->pc->level && !str_cmp (subcmd, "mortal"))
	{
		ch->pc->mortal_mode = 1;
		ch->send_to_char ("Now playing in mortal mode.\n");
	}

	else if (ch->pc && ch->pc->level
		&& !str_cmp (subcmd, "immortal"))
	{
		ch->pc->mortal_mode = 0;
		ch->send_to_char ("Returning to immortal mode.\n");
	}

	
	else if (ch->pc && ch->pc->level && !str_cmp (subcmd, "immenter"))
	{

		while (*argument && *argument == ' ')
			argument++;

		if (ch->pc->imm_enter)
			free_mem (ch->pc->imm_enter);

		if (!*argument)
		{
			ch->send_to_char ("You will use the standard immenter message.\n");
			ch->pc->imm_enter = duplicateString ("");
			return;
		}

		if (*argument == '"' || *argument == '\'')
			ch->send_to_char ("Note:  You probably didn't mean to use quotes.\n");

		ch->pc->imm_enter = duplicateString (argument);
	}

	else if (ch->pc && ch->pc->level && !str_cmp (subcmd, "immleave"))
	{

		while (*argument && *argument == ' ')
			argument++;

		if (ch->pc->imm_leave)
			free_mem (ch->pc->imm_leave);

		if (!*argument)
		{
			ch->send_to_char ("You will use the standard immleave message.\n");
			ch->pc->imm_leave = duplicateString ("");
			return;
		}

		if (*argument == '"' || *argument == '\'')
			ch->send_to_char ("Note:  You probably didn't mean to use quotes.\n");

		ch->pc->imm_leave = duplicateString (argument);
	}
	
		
	else if ((ind = index_lookup (verbal_speeds, subcmd)) != -1 && *subcmd)
	{

		
		if (ind == SPEED_IMMORTAL && !ch->get_trust())
		{
			ch->send_to_char
				("Your muscles would rip away from your bones if you tried that.\n");
			return;
		}

		ch->speed = ind;
		sprintf (buf, "From now on you will %s.\n", verbal_speeds[ind]);
		ch->send_to_char (buf);
		ch->clear_travel();

	}

	else
	{
		ch->send_to_char ("\n   #6Movement:#0\n");
		ch->send_to_char ("   Walk speeds  - trudge, pace, walk, jog, run, sprint\n");
		ch->send_to_char ("\n");
	
		ch->send_to_char ("   #6Informative:#0\n");
		ch->send_to_char ("   Hints      - Toggle receipt of game-related hints\n");
		ch->send_to_char ("   Newbchat   - Toggle access to newbie chat channel\n");
		ch->send_to_char ("   Mentor     - Please see #6HELP MENTOR#0 for details\n");
		ch->send_to_char ("   Mute       - Toggle system beeps upon being notified\n");
		ch->send_to_char ("   Newbie     - Turn off your #2(new player)#0 ldesc tag\n");
		ch->send_to_char ("   Prompt     - Toggle informative prompt on and off\n");
		ch->send_to_char ("   Scan       - Toggle quick scan upon entering rooms\n");
		ch->send_to_char ("   Rpp        - Toggle output of RPP total in SCORE\n");

		if (ch->get_trust())
		{
			ch->send_to_char ("\n   #6Staff-Only Commands:#0\n");
			ch->send_to_char ("   Available - Toggle your availability for petitions\n");
			ch->send_to_char ("   Immenter <message>  - Set \"goto\" enter message\n");
			ch->send_to_char ("   Immleave <message>  - Set \"goto\" leave message\n");
			ch->send_to_char ("   Immortal  - Return to immortal mode\n");
			ch->send_to_char ("   Immwalk   - Removes the walking delay when you move\n");
			ch->send_to_char ("   Mortal    - Become a player (for testing)\n");
			ch->send_to_char ("   Names     - Causes 'SAY' to show player names\n");
			ch->send_to_char ("   Telepath  - Allows you to 'listen in' on PC thoughts\n");
			ch->send_to_char ("   Wiznet    - Toggles your presence on the wiznet\n");
		}
	}
}

void
do_passwd (CHAR_DATA * ch, char *argument, int cmd)
{
	char account_name[MAX_INPUT_LENGTH];
	char new_pass[MAX_INPUT_LENGTH];

	argument = one_argument (argument, account_name);

	if (!*account_name)
	{
		ch->send_to_char ("Syntax:  passwd <account-name> <password>\n");
		return;
	}

	argument = one_argument (argument, new_pass);

	if (!*new_pass)
	{
		ch->send_to_char ("Syntax:  passwd <account-name> <password>\n");
		return;
	}

	if (strlen (new_pass) > 10 || strlen (new_pass) < 4)
	{
		ch->send_to_char ("Make the password between 4 and 10 characters.\n");
		return;
	}

	account acct (account_name);
	if (!acct.is_registered ())
	{
		ch->send_to_char ("No such login account.\n");
		return;
	}

	char* epass = encrypt_buf(new_pass);
	acct.update_password (epass);
	free_mem (epass);
	ch->send_to_char ("Password set.\n");
}


void
do_wanted (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf1[MAX_STRING_LENGTH]= { '\0' };
	char temp_buf[MAX_STRING_LENGTH]= { '\0' };
	CHAR_DATA *tch;
	AFFECTED_TYPE *af;
	int criminals = 0;
	int zone = -1;
	std::list<char_data*>::iterator tch_iterator;
	
	argument = one_argument (argument, buf);

	if (*buf && isdigit (*buf))
		zone = atoi (buf);


	if ((!ch->get_trust()) && !is_area_enforcer (ch))
	{
		ch->send_to_char ("You don't know who is wanted around here.\n");
		return;
	}

	if ((!ch->get_trust()))
		zone = ch->room->zone;

	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		tch = *tch_iterator;

		if (tch->deleted)
			continue;

		for (af = tch->hour_affects; af; af = af->next)
		{

			if (af->type < MAGIC_CRIM_BASE || af->type > MAGIC_CRIM_BASE + 100)
				continue;

			/*
			If ch is not an imm, and the criminal is in the
			same zone, then if the criminal is not wanted in
			the zone, skip him.
			*/
			if (zone != -1)
			{
				if (!get_affect (tch, MAGIC_CRIM_BASE + zone))
						continue;
				
			}

			criminals++;

			if ((!ch->get_trust()))
			{
				if (af->a.spell.duration == -1)
					sprintf (buf, "Forever %s\n", tch->short_descr);
				else
					sprintf (buf, "  %d    %s\n", af->a.spell.duration,
					tch->short_descr);
			}

			else
			{

				if (IS_NPC (tch))
					sprintf (buf1, "[%d] ", tch->mob->nVirtual);
				else
					sprintf (buf1, "[%s] ", tch->name);

				sprintf (buf, "%5d  %2d      %2d  %s%s\n",
					tch->in_room, af->a.spell.duration,
					af->type - MAGIC_CRIM_BASE, buf1, tch->short_descr);
			}

			strcat (temp_buf, buf);
		}
	}

	if (!criminals)
		ch->send_to_char ("No criminals.\n");
	else
	{
		if (ch->get_trust())
			ch->desc->header = duplicateString ("Room  Hours  Zone  Name\n");
		else if (criminals)
			ch->desc->header = duplicateString ("Hours   Description\n");

		page_string (ch->desc, temp_buf);
	}
}

void
report_exits (CHAR_DATA * ch, int zone)
{
	int dir;
	ROOM_DATA *room;
	ROOM_DATA *troom;
	std::map<int, ROOM_DATA*>::iterator room_iterator;
	char tempbuf[MAX_STRING_LENGTH] = "\0";
	

	for (room_iterator = room_map.begin(); room_iterator != room_map.end(); room_iterator++)
	{
		room = room_iterator->second;

		if (zone != -1 && room->zone != zone)
			continue;

		for (dir = 0; dir <= LAST_DIR; dir++)
		{

			if (!room->dir_option[dir])
				continue;

			if (!(troom = vtor (room->dir_option[dir]->to_room)))
			{
				sprintf (tempbuf + strlen (tempbuf),
					"Room %5d %5s doesn't go to room %d\n",
					room->nVirtual, dirs[dir],
					room->dir_option[dir]->to_room);
			}

			else if (!troom->dir_option[rev_dir[dir]])
			{
				sprintf (tempbuf + strlen (tempbuf),
					"Room %5d %5s is one-way to room %d\n",
					room->nVirtual, dirs[dir],
					room->dir_option[dir]->to_room);
			}

			else if (room->nVirtual != troom->dir_option[rev_dir[dir]]->to_room)
			{
				sprintf (tempbuf + strlen (tempbuf),
					"Room %5d %5s->%5d  BUT  %5d <- %5s %5d\n",
					room->nVirtual, dirs[dir], troom->nVirtual,
					troom->dir_option[rev_dir[dir]]->to_room,
					dirs[rev_dir[dir]], troom->nVirtual);
			}
		}
	}

	page_string (ch->desc, tempbuf);
}

void
report_mobiles (CHAR_DATA * ch)
{
	int i;
	int zone_counts[100];
	int total_mobs = 0;
	int recount = 0;
	CHAR_DATA *tch;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	std::list<char_data*>::iterator tch_iterator;

	for (i = 0; i < 100; i++)
		zone_counts[i] = 0;

	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		tch = *tch_iterator;

		if (tch->deleted || !IS_NPC (tch))
			continue;

		total_mobs++;
	}

	for (i = 0; i < 25; i++)
	{
		sprintf (buf,
			"[%2d]: %3d     [%2d]: %3d      [%2d]: %3d      [%2d]: %3d\n",
			i, zone_counts[i], i + 25, zone_counts[i + 25], i + 50,
			zone_counts[i + 50], i + 75, zone_counts[i + 75]);
		ch->send_to_char (buf);

		recount += zone_counts[i] + zone_counts[i + 25] +
			zone_counts[i + 50] + zone_counts[i + 75];
	}

	sprintf (buf, "  Total: %d; recount: %d\n", total_mobs, recount);
	ch->send_to_char (buf);
}

void
report_objects (CHAR_DATA * ch)
{
	int i = 0;
	int j;
	FILE *fp;
	OBJ_DATA *obj;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	std::map<int, OBJ_DATA*>::iterator obj_iterator;
	
	if (ch->get_trust() < 6)
		ch->send_to_char("Please ask an Implementor for this information\n");

	if (!(fp = fopen ("objects", "w+")))
		perror ("Couldn't open objects");

	fprintf (fp,
		"Virtual\tQuality\tSize\tCondition\tWeight\tCost\tItemType\tOV1\tOV2\tOV3\tOV4\tOV5\tFlags\tKeywords\tShort\tOneLine\n");

	for (obj_iterator = proto_obj_map.begin(); obj_iterator != proto_obj_map.end(); obj_iterator++)
	{
		obj = obj_iterator->second;

		*buf2 = '\0';

		for (j = 0; str_cmp (econ_flags[j], "\n"); j++)
		{
			if (IS_SET (obj->econ_flags, 1 << j))
			{
				strcat (buf2, econ_flags[j]);
				strcat (buf2, " ");
			}
		}

		/*          virt qual sz   cnd   wt  cost  it  v0  v1  v2  v3  v4  v5  fl  ky  sd  ld */

		fprintf (fp,
			"%05d\t%2d\t%5d\t%2d\t%5d\t%5.2f\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%s\t%s\t%s\t%s\n",
			obj->nVirtual, obj->quality, obj->size, obj->item_wear,
			obj->obj_flags.weight, obj->coppers,
			item_types[(int) obj->obj_flags.type_flag], obj->o.od.value[0],
			obj->o.od.value[1], obj->o.od.value[2], obj->o.od.value[3],
			obj->o.od.value[4], obj->o.od.value[5], buf2, obj->name,
			obj->short_description, obj->description);

		i++;
	}

	sprintf (buf, "Write %d records.\n", i);

	ch->send_to_char (buf);

	fclose (fp);
}

void
revise_objects (CHAR_DATA * ch)
{
	int ind;
	int vnum;
	FILE *fp;
	OBJ_DATA *obj;
	char *p;
	char *argument;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };

	if (!(fp = fopen ("objects.revised", "r")))
		perror ("Couldn't open objects");

	while (fgets (buf, 256, fp))
	{

		p = buf;
		while (*p)
		{
			if (*p == ',')
				*p = ' ';
			p++;
		}

		argument = one_argument (buf, buf2);

		vnum = atoi (buf2);

		if (!vnum || !(obj = vtoo (vnum)))
		{
			printf ("Vnum %d doesn't work.\n", vnum);
			continue;
		}

		argument = one_argument (argument, buf2);
		argument = buf2;

		obj->econ_flags = 0;

		while (1)
		{
			argument = one_argument (argument, buf);

			if (!*buf)
				break;

			if ((ind = index_lookup (econ_flags, buf)) == -1)
			{
				printf ("Unknown flag name: %s\n", buf);
				continue;
			}

			obj->econ_flags |= (1 << ind);
		}
	}

	fclose (fp);
	ch->send_to_char ("Done.\n");
}

void
report_races (CHAR_DATA * ch)
{
	int i = 0;
	FILE *fp;
	CHAR_DATA *mob;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	std::map<int, CHAR_DATA*>::iterator mob_iterator;

	if (ch->get_trust() < 6)
		ch->send_to_char("Please ask an Implementor for this information\n");

	if (!(fp = fopen ("races", "w+")))
		perror ("Couldn't create races");

	for (mob_iterator = proto_mob_map.begin(); mob_iterator != proto_mob_map.end(); mob_iterator++)
	{
		mob = mob_iterator->second;
		fprintf (fp, "%05d  %-7.7s %-60.60s\n", mob->mob->nVirtual,
			lookup_race_variable (mob->race, RACE_NAME), mob->keywords);
		i++;
	}

	sprintf (buf, "Wrote %d records.\n", i);
	ch->send_to_char (buf);

	fclose (fp);
}

void
report_shops (CHAR_DATA * ch)
{
	int write_count = 0;
	int i;
	FILE *fp;
	CHAR_DATA *mob;
	OBJ_DATA *tobj;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	std::map<int, CHAR_DATA*>::iterator mob_iterator;

	/*
	I have a request for you...  merchants listing, tab delimited, with:
	vnum, short desc, store room, markup, discount, trades in, deliveries
	0-9, and if possible, a listing of what is in their storerooms.

	I'd like to give you back vnum, all the markups, discounts, and nobuy
	*/

	if (ch->get_trust() < 6)
		ch->send_to_char("Please ask an Implementor for this information\n");

	if (!(fp = fopen ("shops", "w+")))
		perror ("Couldn't create shops");

	fprintf (fp,
		"Vnum \tShop \tStore\tMrkup\tDscnt\tDel-0\tName0\tDel-1\tName1\tDel-2\tName2\tDel-3\tName3\tDel-4\tName4\tDel-5\tName5\tDel-6\tName6\tDel-7\tName7\tDel-8\tName8\tDel-9\tName9\tKeeper Name\ttrade-flags\n");

	for (mob_iterator = proto_mob_map.begin(); mob_iterator != proto_mob_map.end(); mob_iterator++)
	{
		mob = mob_iterator->second;

		if (!mob->mob->shop)
			continue;

		sprintf (buf, "%05d\t%5d\t%5d\t%3.2f\t%3.2f\t",
			mob->mob->nVirtual, mob->mob->shop->shop_vnum,
			mob->mob->shop->store_vnum, mob->mob->shop->markup, mob->mob->shop->discount);

		for (i = 0; i < MAX_DELIVERIES; i++)
		{

			sprintf (buf + strlen (buf), "%5d\t", mob->mob->shop->delivery[i]);

			if (mob->mob->shop->delivery[i] > 0 &&
				(tobj = vtoo (mob->mob->shop->delivery[i])))
				sprintf (buf + strlen (buf), "%s\t", tobj->short_description);
			else
				sprintf (buf + strlen (buf), "\t");
		}

		sprintf (buf + strlen (buf), "%s\t", mob->short_descr);

		for (i = 0; i < MAX_TRADES_IN; i++)
			if (mob->mob->shop->trades_in[i])
				sprintf (buf + strlen (buf), "%s ",
				item_types[mob->mob->shop->trades_in[i]]);

		fprintf (fp, "\t%s\n", buf);

		write_count++;
	}

	sprintf (buf, "Wrote %d records.\n", write_count);
	ch->send_to_char (buf);

	fclose (fp);
}

void
get_float (float *f, char **argument)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };

	*argument = one_argument (*argument, buf);

	sscanf (buf, "%f", f);
}

void
get_flags (int *flags, char **argument)
{
	int ind;
	char *p;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };

	*argument = one_argument (*argument, buf);

	*flags = 0;

	p = buf;

	while (1)
	{
		p = one_argument (p, buf2);

		if (!*buf2)
			break;

		if ((ind = index_lookup (econ_flags, buf2)) == -1)
		{
			printf ("Unknown flag name: %s\n", buf2);
			continue;
		}

		*flags |= (1 << ind);
	}
}

void
revise_shops (CHAR_DATA * ch)
{
	int vnum;
	FILE *fp;
	CHAR_DATA *mob;
	char *p;
	char *argument;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };

	if (!(fp = fopen ("shops.revised", "r")))
		perror ("Couldn't open shops.revised");

	while (fgets (buf, 256, fp))
	{

		p = buf;
		while (*p)
		{
			if (*p == ',')
				*p = ' ';
			p++;
		}

		/*
		"Vnum ","Shop ","Store","Mrkup","Dscnt","Mrkup1","Dscnt1","Flags1","Mrkup2","Dscnt2","Flags2","Mrkup3","Dscnt3","Flags3","Nobuy"
		1173,14173,2951,1.15,0.15,2.30,0.30,"fine khuzan rare valuable magical sindarin noble foreign",1.44,0.19,"ntharda stharda orbaal",0.58,0.07,"poor","junk gargun"
		*/
		argument = one_argument (buf, buf2);	/* Keeper vnum */
		vnum = atoi (buf2);

		if (!vnum || !(mob = vtom (vnum)))
		{
			printf ("Vnum %d doesn't work.\n", vnum);
			continue;
		}

		argument = one_argument (argument, buf2);	/* Shop */
		argument = one_argument (argument, buf2);	/* Store */

		get_float (&mob->mob->shop->markup, &argument);
		get_float (&mob->mob->shop->discount, &argument);
		get_float (&mob->mob->shop->econ_markup1, &argument);
		get_float (&mob->mob->shop->econ_discount1, &argument);
		get_flags (&mob->mob->shop->econ_flags1, &argument);
		get_float (&mob->mob->shop->econ_markup2, &argument);
		get_float (&mob->mob->shop->econ_discount2, &argument);
		get_flags (&mob->mob->shop->econ_flags2, &argument);
		get_float (&mob->mob->shop->econ_markup3, &argument);
		get_float (&mob->mob->shop->econ_discount3, &argument);
		get_flags (&mob->mob->shop->econ_flags3, &argument);
		get_float (&mob->mob->shop->econ_markup4, &argument);
		get_float (&mob->mob->shop->econ_discount4, &argument);
		get_flags (&mob->mob->shop->econ_flags4, &argument);
		get_float (&mob->mob->shop->econ_markup5, &argument);
		get_float (&mob->mob->shop->econ_discount5, &argument);
		get_flags (&mob->mob->shop->econ_flags5, &argument);
		get_float (&mob->mob->shop->econ_markup6, &argument);
		get_float (&mob->mob->shop->econ_discount6, &argument);
		get_flags (&mob->mob->shop->econ_flags6, &argument);
		get_float (&mob->mob->shop->econ_markup7, &argument);
		get_float (&mob->mob->shop->econ_discount7, &argument);
		get_flags (&mob->mob->shop->econ_flags7, &argument);
		get_flags (&mob->mob->shop->nobuy_flags, &argument);
	}

	fclose (fp);
	ch->send_to_char ("Done.\n");
}


void
do_report (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char temp[MAX_STRING_LENGTH]= { '\0' };
	int zone = -1;

	argument = one_argument (argument, buf);

	if (!*buf || *buf == '?')
	{
		ch->send_to_char ("   objects   - object list to file 'objects'.\n");
		ch->send_to_char ("   shops     - keeper list to file 'shops'.\n");
		ch->send_to_char ("   race      - race report.\n");
		ch->send_to_char ("   mobiles   - Mob population by zone.\n");
		ch->send_to_char ("   exits [zone]    - Faulty links in exits\n");
	}

	else if (!str_cmp (buf, "exits"))
	{
		argument = one_argument (argument, temp);

		if (isdigit (*temp))
		{
			if ((zone = atoi (temp)) >= MAX_ZONE)
			{
				ch->send_to_char ("Zone not in range 0..99\n");
				return;
			}
			report_exits(ch, zone);
		}
		else
			report_exits (ch, -1);
	}

	else if (!str_cmp (buf, "revshops"))
		revise_shops (ch);
	else if (!str_cmp (buf, "revobjs"))
		revise_objects (ch);
	else if (!str_cmp (buf, "shops") || !str_cmp (buf, "keepers"))
		report_shops (ch);
	else if (!str_cmp (buf, "objs") || !str_cmp (buf, "objects"))
		report_objects (ch);
	else if (!str_cmp (buf, "mobiles") || !str_cmp (buf, "mobs"))
		report_mobiles (ch);
	else if (!str_cmp (buf, "race") || !str_cmp (buf, "races"))
		report_races (ch);
	else
		ch->send_to_char ("report ?   - for help\n");
}

void
do_openskill (CHAR_DATA * ch, char *argument, int cmd)
{
	std::string skill_name;
	std::string Cap_skill_name;
	CHAR_DATA *victim;
	char buf1[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char *buf3;

	argument = one_argument (argument, buf1);
	argument = one_argument (argument, buf2);

	buf3 = duplicateString(CAP(buf1));
	skill_name = strdup(buf3);
	
	if (skill_data_map.find(skill_name) == skill_data_map.end())
		
	{
		buf3 = strdup(CAP(buf2));
		skill_name = strdup(buf3);
		if (skill_data_map.find(skill_name) == skill_data_map.end())
		{
			ch->send_to_char ("No such skill.\n");
			return;
		}
		victim = get_char_room_vis (ch, buf1);
	}
	else
	{
		victim = get_char_room_vis (ch, buf2);
	}
		
	

	if (!victim)
	{
		ch->send_to_char
			("Either use 'at <person> openskill <skillname>' or go to his/her room and use 'openskill <skillname> <person>'.\n");
		return;
	}

	if (victim->skill_map[skill_name] > 0)
	{
		sprintf (buf1, "Skill %s is already at %d on that character.\n",
			skill_name.c_str(), victim->skill_map[skill_name]);
		ch->send_to_char(buf1);
		return;
	}
		//TODO: are we going to keep these??
	/***
	 if (sn == lookup_skill_id("Clairvoyance") 
	 || sn == lookup_skill_id("Dager-sense")
	 || sn == lookup_skill_id("Empathic-heal") 
	 || sn == lookup_skill_id("Hex")
	 || sn == lookup_skill_id("Mental-bolt")
	 || sn == lookup_skill_id("Prescience")
	 || sn == lookup_skill_id("Sensitivity") 
	 || sn == lookup_skill_id("Telepathy"))
	{
		if (ch->get_trust() <= 5)
		{
			ch->send_to_char
				("Psionics may only be rolled randomly at character creation.\n",
				ch);
			return;
		}
	}
***/
	
	open_skill (victim, lookup_skill_id(skill_name));

	sprintf (buf1, "$N's %s has opened at %d.", skill_name.c_str(), victim->skill_map[skill_name]);

	ch->act(buf1, false, 0, victim, TO_CHAR);
	 
}


/***********
Affects for Characters and objects are commented out, until they can be fully evaluated and de-bugged.
 ************/
void
do_affect (CHAR_DATA * ch, char *argument, int cmd)
{
	int power_specified = 0;
	int duration;
	int power = 0;
	int affect_no;
	AFFECTED_TYPE *af = NULL;
	CHAR_DATA *tch = NULL;
	OBJ_DATA *obj = NULL;
	ROOM_DATA *room;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	
	argument = one_argument (argument, buf);
	
	if (!*buf || *buf == '?')
	{
		
		ch->send_to_char ("           char <mob> <affect no/name>   delete\n"
						  "   affect  obj  <obj> <affect no/name>   <duration> [power]\n"
						  "           room       <affect no/name> <duration> [intensity]\n"
						  "\n"
						  "Example:\n\n"
						  "   affect room 'aklash odor' delete\n"
						  "   affect char 2311 5000 1000   (2311 = Incense Smoke)\n\n"
						  "If an affect doesn't already exist, one is created by this "
						  "command.\n\n"
						  "   affect char 2311 4000        (Only modify duration)\n\n"
						  "Most affect durations are in mud hours, some are in RL seconds.\n"
						  "CAUTION: Room affects are the only ones functional at this time!");
		
		return;
	}
	
	room = ch->room;
	
	if (is_abbrev (buf, "character"))
	{
		
		argument = one_argument (argument, buf);
		
		if (!(tch = get_char_room_vis (ch, buf)))
		{
			ch->send_to_char ("Couldn't find that character in the room.\n");
			return;
		}
	}
	
	else if (is_abbrev (buf, "object"))
	{
		
		argument = one_argument (argument, buf);
		
		if (!(obj = get_obj_in_dark (ch, buf, ch->room->contents)))
		{
			ch->send_to_char ("Couldn't find that object in the room.\n");
			return;
		}
	}
	
	else if (!is_abbrev (buf, "room"))
	{
			
		ch->send_to_char ("Expected 'character', 'object', or 'room' after affect.");
		return;
	}
	
	argument = one_argument (argument, buf);
	
	if (just_a_number (buf))
		affect_no = atoi (buf);
	else
	{
		if ((affect_no = lookup_value (buf, REG_MAGIC_SPELLS)) == -1 &&
			(affect_no = lookup_value (buf, REG_CRAFT_MAGIC)) == -1 &&
			(affect_no = lookup_value (buf, REG_SPELLS)) == -1 &&
			(affect_no = lookup_value (buf, REG_AFFECT)) == -1)
		{
			ch->send_to_char ("No such affect or spell.\n");
			return;
		}
	}
	
	argument = one_argument (argument, buf);
	
	if (is_abbrev (buf, "delete"))
	{
		if (obj)
		{
			af = get_obj_affect (obj, affect_no);
			if (!af)
			{
				ch->send_to_char ("No such affect on that object.\n");
				return;
			}
			
			remove_obj_affect (obj, affect_no);
			ch->send_to_char ("Affected deleted from object.\n");
			return;
		}
		
		else if (tch)
		{
			af = get_affect (tch, affect_no);
			if (!af)
			{
				ch->send_to_char ("No such affect on that character.\n");
				return;
			}
			
			affect_remove (tch, af);
			ch->send_to_char ("Affect deleted from character.\n");
			return;
		}
	
		/* It must be a room affect */
		else
		{
			if (!(af = is_room_affected (room->affects, affect_no)))
			{
				ch->send_to_char ("No such affect on this room.\n");
				return;
			}
			
			remove_room_affect (room, affect_no);
			ch->send_to_char ("Affect deleted from room.\n");
			return;
		}
	}
	
	/* We're not deleting, get the duration and optional power so we
	 can either update an affect or create a new one.
	 */
	
	if (!(duration = atoi (buf)))
	{
		ch->send_to_char ("Duration expected.\n");
		return;
	}
	
	argument = one_argument (argument, buf);
	
	if (*buf)
	{
		if (!just_a_number (buf))
		{
			ch->send_to_char ("Power or intentsity should be a number.\n");
			return;
		}
		
		power_specified = 1;
		power = atoi (buf);
	}
	
	/* Affect already exist? */
	if (tch)
		af = get_affect (tch, affect_no);
	else if (obj)
		af = get_obj_affect (obj, affect_no);
	else
		af = is_room_affected (room->affects, affect_no);
	
	if (af)
	{
		
		af->a.spell.duration = duration;
		
		if (power_specified)
			af->a.spell.modifier = power;
		
		ch->send_to_char ("Affect modified.\n");
		return;
	}
	
	/* Add a new affect */
	
	af = new AFFECTED_TYPE;
	
	af->type = affect_no;
	
	af->a.spell.duration = duration;
	
	if (power_specified)
	{
		af->a.spell.modifier = power;
	}
	if (tch)
		affect_to_char (tch, af);
	else if (obj)
		affect_to_obj (obj, af);
	else
	{
		if (power_specified)
			add_room_affect (&room->affects, af->type, duration, power );
		else
			add_room_affect (&room->affects, af->type, duration, 1);
		
		save_room_affects (room->zone);
	}
	
	ch->send_to_char ("Affect created.\n");
}

void
list_all_crafts (CHAR_DATA * ch)
{
	SUBCRAFT_HEAD_DATA *craft;
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	std::ostringstream craftList;
	char craftLine[1024];

	craftList << "We currently have the following crafts available:\n\n";

	for (tcraft_iterator = craft_map.begin(); tcraft_iterator != craft_map.end(); tcraft_iterator++)
	{
		craft = tcraft_iterator->second;
		sprintf (craftLine, "#6Craft:#0 %-20s #6Sub:#0 %-24s #6Cmd:#0 %-10s\n", 
			craft->craft_name, craft->subcraft_name, craft->command);
		craftList << craftLine;
	}

	ch->send_to_char ("\n");
	page_string (ch->desc, craftList.str().c_str());
}

void
display_craft (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
	PHASE_DATA *phase;
	DEFAULT_ITEM_DATA *items;
	DEFAULT_MOB_DATA *mobs;
	int i, j, phasenum = 1;
	char flag[MAX_STRING_LENGTH]= { '\0' };
	char tempbuf[MAX_STRING_LENGTH]="\0";
	
	float low_consumed_expense = 0;
	float high_consumed_expense = 0;
	float low_reusable_expense = 0;
	float high_reusable_expense = 0;
	float low_produced_value = 0;
	float high_produced_value = 0;
	extern const char *attrs[9];

	
	if (craft->phases[1] == NULL)
	{
		ch->send_to_char("Sorry. No phase data is available\n");
		return;
	}
	
	for (phasenum = 1; phasenum < MAX_PHASES_PER_SUBCRAFT; phasenum++)
	{
		if (craft->phases[phasenum] == NULL)
			continue;
		
		phase = craft->phases[phasenum];
		
		sprintf (tempbuf + strlen (tempbuf), "  #6Phase %d:#0\n", phasenum);

		if (phase->first)
			sprintf (tempbuf + strlen (tempbuf), "    1st:  %s\n",
			phase->first);

		if (phase->third)
			sprintf (tempbuf + strlen (tempbuf), "    3rd:  %s\n",
			phase->third);

		if (phase->group_mess)
			sprintf (tempbuf + strlen (tempbuf), "  Group:  %s\n",
			phase->group_mess);

		if (phase->phase_seconds)
			sprintf (tempbuf + strlen (tempbuf), "      T:  %d\n",
			phase->phase_seconds);

		if (phase->skill > 0)
			sprintf (tempbuf + strlen (tempbuf), "  Skill:  %s vs %dd%d\n",
			lookup_skill_name(phase->skill), phase->dice, phase->sides);

		if ( phase->attribute > -1)
		{
			sprintf (tempbuf + strlen(tempbuf), "  Attri:  %s vs %dd%d\n",
				attrs[phase->attribute], phase->dice, phase->sides);
		}

		if (phase->move_cost)
			sprintf (tempbuf + strlen (tempbuf), "   Cost:  moves %d\n",
			phase->move_cost);

		if (phase->hit_cost)
			sprintf (tempbuf + strlen (tempbuf), "   Cost:  hits %d\n",
			phase->hit_cost);

	
	
	if (craft->obj_items)
	{
		for (i = 1; craft->obj_items[i]; i++)
		{
			items = craft->obj_items[i];
			
			if ((items->phase_num == phasenum) && items->items && items->items[0])
			{
				if (IS_SET (items->flags, SUBCRAFT_GIVE))
					sprintf (flag, "give");
				else if (IS_SET (items->flags, SUBCRAFT_HELD))
					sprintf (flag, "held");
				else if (IS_SET (items->flags, SUBCRAFT_USED))
					sprintf (flag, "used");
				else if (IS_SET (items->flags, SUBCRAFT_PRODUCED))
					sprintf (flag, "produced");
				else if (IS_SET (items->flags, SUBCRAFT_FINAL_PRODUCED))
					sprintf (flag, "final");
				else
					sprintf (flag, "in-room");
				sprintf (tempbuf + strlen (tempbuf), "      %d:  (%s", i, flag);
				if (items->item_counts > 1)
					sprintf (tempbuf + strlen (tempbuf), " x%d",
							 items->item_counts);
				
				float low_cost = 0;
				float high_cost = 0;
				for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
				{
					if (items->items[j]
						&& items->items[j] != items->item_counts)
					{
						sprintf (tempbuf + strlen (tempbuf), " %d",
								 items->items[j]);
						OBJ_DATA *obj = vtoo (items->items[j]);
						if (obj)
						{
							float obj_value = obj->coppers;
							if (!low_cost || (low_cost > obj_value))
							{
								low_cost = obj_value;
							}
							if (high_cost < obj_value)
							{
								high_cost = obj_value;
							}
						}
					}
				}
				low_cost *= items->item_counts;
				high_cost *= items->item_counts;
				
				if (IS_SET (items->flags, SUBCRAFT_USED))
				{
					low_consumed_expense += low_cost;
					high_consumed_expense += high_cost;
				}
				else if ((IS_SET (items->flags, SUBCRAFT_PRODUCED))
						 || (IS_SET (items->flags, SUBCRAFT_PRODUCED))
						 || (IS_SET (items->flags, SUBCRAFT_GIVE)))
				{
					low_produced_value += low_cost;
					high_produced_value += high_cost;
				}
				else
				{
					low_reusable_expense += low_cost;
					high_reusable_expense += high_cost;
				}
				
				sprintf (tempbuf + strlen (tempbuf), ")\n");
			}
		}
	}
	
		
	if (craft->mob_items)
	{
		for (i = 1; craft->mob_items[i]; i++)
		{
			mobs = craft->mob_items[i];
			
			if ((mobs->phase_num == phasenum) && mobs->mobs && mobs->mobs[0])
			{
				if (IS_SET (mobs->flags, SUBCRAFT_MOB_USED))
					sprintf (flag, "used");
				else if (IS_SET (mobs->flags, SUBCRAFT_MOB_PRODUCED))
					sprintf (flag, "produced");
				else if (IS_SET (mobs->flags, SUBCRAFT_MOB_FINAL_PRODUCED))
					sprintf (flag, "final");
				else if (IS_SET (mobs->flags, SUBCRAFT_MOB_OWNED))
					sprintf (flag, "owned");
				else
					sprintf (flag, "in-room");
				
				sprintf (tempbuf + strlen (tempbuf), "Mobile %d:  (%s ", i, flag);
				
				for (j = 0; j < MAX_DEFAULT_MOBS; j++)
				{
					if (mobs->mobs[j])
					{
						sprintf (tempbuf + strlen (tempbuf), " %d",
								 mobs->mobs[j]);
						
					}
				}
				sprintf (tempbuf + strlen (tempbuf), ")\n");
				
			}
		}
	}
	
	
	}
	sprintf (tempbuf + strlen (tempbuf),
		"\n"
		"#6Reusable Material Costs:#0 % 7.2f - % 7.2f bits\n"
		"#6Expended Material Costs:#0 % 7.2f - % 7.2f bits\n"
		"#6Produced Material Value:#0 % 7.2f - % 7.2f bits\n",
		low_reusable_expense, high_reusable_expense,
		low_consumed_expense, high_consumed_expense,
		low_produced_value, high_produced_value);

	page_string (ch->desc, tempbuf);

}

int
is_name_legal (char *buf)
{
	unsigned int i;

	if (strlen (buf) > 15)
		return 0;

	for (i = 0; i < strlen (buf); i++)
	{
		if (!isalpha (buf[i]))
		{
			return 0;
		}

		if (i)
			buf[i] = tolower (buf[i]);
		else
			buf[i] = toupper (buf[i]);
	}

	return 1;
}

void
do_pfile (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch;
	char pfile[MAX_STRING_LENGTH]= { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char new_name[MAX_STRING_LENGTH]= { '\0' };
	struct stat stat_buf;
	std::list<char_data*>::iterator tch_iterator;
	
	if (ch->get_trust() != 5)
	{
		ch->send_to_char ("No way.\n");
		return;
	}

	argument = one_argument (argument, pfile);

	if (!*pfile || *pfile == '?')
	{
		ch->send_to_char
			("pfile <pc-name>   activate [<new-name>] - return from cold storage\n"
			"                  archive               - place in cold storage\n"
			"                  copy <new-name>       - make a copy of a pfile, including\n"
			"                                          objects.\n"
			"                  delete <pfile>        - delete an active pfile\n"
			"                  rename <new-name>     - rename a pfile\n\n");
		return;
	}

	if (!is_name_legal (pfile))
	{
		ch->send_to_char ("Illegal pfile name.\n");
		return;
	}

	argument = one_argument (argument, buf);

	if (str_cmp (buf, "activate") &&
		str_cmp (buf, "archive") &&
		str_cmp (buf, "copy") &&
		str_cmp (buf, "delete") && str_cmp (buf, "rename"))
	{
		ch->send_to_char ("Unrecognized keyword, type 'pfile ?' for help.\n");
		return;
	}

	if (!str_cmp (buf, "activate"))
	{

		argument = one_argument (argument, new_name);

		if (!*new_name)
			strcpy (new_name, pfile);

		if (!is_name_legal (new_name))
		{
			ch->send_to_char ("Illegal new-name.\n");
			return;
		}

		sprintf (buf, "save/player/%c/%s", tolower (*new_name), new_name);

		/* Stat returns 0 if successful, -1 if it fails */

		if (stat (buf, &stat_buf) == 0)
		{
			ch->send_to_char ("Target pfile (new-name) already exists.\n");
			return;
		}

		sprintf (buf, "save/archive/%s", new_name);

		if (stat (buf, &stat_buf) == -1)
		{
			ch->send_to_char ("Archive pfile (pc-name) does not exist in the "
				"archive directory.\n");
			return;
		}

		/* Pfile */

		sprintf (buf, "mv save/archive/%s save/player/%c/%s",
			new_name, tolower (*new_name), new_name);
		system (buf);

		/* Mobs saved with pfile */

		sprintf (buf, "mv save/archive/%s.a save/player/%c/%s.a",
			new_name, tolower (*new_name), new_name);
		system (buf);

		/* Player's objects */

		sprintf (buf, "mv save/archive/%s.objs save/objs/%c/%s",
			new_name, tolower (*new_name), new_name);
		system (buf);

		ch->send_to_char ("Player character re-activated.\n");
		return;
	}

	if (!str_cmp (buf, "delete"))
	{

		ch->send_to_char ("Delete operation.\n");
		ch->send_to_char (pfile);
		ch->send_to_char (" being deleted.\n");
		tch = load_pc (pfile);

		if (!tch)
		{
			ch->send_to_char ("That pfile doesn't exist or isn't active.\n");
			return;
		}

		sprintf (buf, "Load count is %d\n", tch->pc->load_count);
		ch->send_to_char (buf);

		if (tch->pc->load_count != 1)
		{
			ch->act("The player can't be online and the pfile can't be loaded by "
				"another admin when it is deleted.  You or someone else may "
				"have the pfile 'mob'ed.", false,
				0, 0, TO_CHAR | _ACT_FORMAT);
			tch->unload_pc();
			return;
		}

		tch->unload_pc();

		sprintf (buf, "rm save/player/%c/%s", tolower (*pfile), pfile);
		ch->send_to_char (buf);
		ch->send_to_char ("\n");

		strcat (buf, ".a");
		ch->send_to_char (buf);
		ch->send_to_char ("\n");

		sprintf (buf, "rm save/objs/%c/%s", tolower (*pfile), pfile);
		ch->send_to_char (buf);
		ch->send_to_char ("\n");

		strcat (buf, ".died");
		ch->send_to_char (buf);
		ch->send_to_char ("\n");

		sprintf (buf, "rm save/player/submit/%s", pfile);
		ch->send_to_char (buf);
		ch->send_to_char ("\n");

		ch->send_to_char ("Ok.\n");

		return;
	}

	if (!str_cmp (buf, "archive"))
	{

		for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
		{
			tch = *tch_iterator;

			if (tch->deleted || !tch->pc)
				continue;

			if (!str_cmp (new_name, tch->name))
			{
				ch->send_to_char ("That character is currently loaded.  Figure out "
					"a way to unload it.\n");
				return;
			}
		}
	}

	ch->send_to_char ("End of procedure\n");
}

void
do_last (CHAR_DATA * ch, char *argument, int cmd)
{
	time_t localtime;
	CHAR_DATA *tch;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (argument, buf);

	if (!(tch = load_pc (buf)))
	{
		ch->send_to_char ("No such PC.\n");
		return;
	}

	if (tch->pc->last_connect)
	{
		sprintf (buf, "Last connect:     %s", ctime (&tch->pc->last_connect));
		ch->send_to_char (buf);
	}

	if (tch->pc->last_logon)
	{
		sprintf (buf, "Last logon:       %s", ctime (&tch->pc->last_logon));
		ch->send_to_char (buf);
	}

	if (tch->pc->last_logoff)
	{
		sprintf (buf, "Last logoff:      %s", ctime (&tch->pc->last_logoff));
		ch->send_to_char (buf);
	}

	if (tch->pc->last_disconnect)
	{
		sprintf (buf, "Last disconnect:  %s",
			ctime (&tch->pc->last_disconnect));
		ch->send_to_char (buf);
	}

	if (tch->pc->last_died)
	{
		sprintf (buf, "Last died:        %s", ctime (&tch->pc->last_died));
		ch->send_to_char (buf);
	}

	localtime = time (0);

	ch->send_to_char ("\nCurrent Server Time: ");

	ch->send_to_char (ctime (&localtime));

	tch->unload_pc();
}

TEXT_DATA *
find_text (CHAR_DATA * ch, TEXT_DATA * list, char *buf)
{
	while (list)
	{
		
		if (!str_cmp (list->name, buf))
			return list;
		
		list = list->next;
	}
	
	return NULL;
}


char *
get_text_buffer (CHAR_DATA * ch, TEXT_DATA * list, char *text_name)
{
	TEXT_DATA *entry;
	
	if ((entry = find_text (ch, list, text_name)))
		return entry->text;
	
	return "(no info)\n";
}

#define MAP_MAX_RADIUS 3
#define MAP_GRID_WIDTH ((MAP_MAX_RADIUS * 2) + 1)
#define MAP_GRID_DEPTH 5
/******
 map[0][x][y]	room number
 map[1][x][y]	terrain type
 map[2][x][y]	doors on east wall
 map[3][x][y]	doors on south wall
 map[4][x][y]	doors up/down
 
 ******/

void
fill_map (ROOM_DATA * ptrRoom, int x, int y,
		  int map[MAP_GRID_DEPTH][MAP_GRID_WIDTH][MAP_GRID_WIDTH])
{
	int n = 0, e = 0, s = 0, w = 0;
	static unsigned char radius = 0;
	ROOM_EXIT_DATA *ptrNExit = NULL;
	ROOM_EXIT_DATA *ptrEExit = NULL;
	ROOM_EXIT_DATA *ptrSExit = NULL;
	ROOM_EXIT_DATA *ptrWExit = NULL;

	if (!ptrRoom)
		return;

	if (!map[0][x][y])
	{
		map[0][x][y] = ptrRoom->nVirtual;
	}
	
	map[1][x][y] = (ptrRoom->terrain_type >= 0
		&& ptrRoom->terrain_type < 21) ? ptrRoom->terrain_type : 21;
	if ((ptrEExit = ptrRoom->dir_option[1]) != NULL)
	{
		map[2][x][y] = (IS_SET (ptrEExit->port_flags, (EX_ISDOOR || EX_ISGATE))) ? 2 : 1;
	}
	
	if ((ptrSExit = ptrRoom->dir_option[2]) != NULL)
	{
		map[3][x][y] = (IS_SET (ptrSExit->port_flags, (EX_ISDOOR || EX_ISGATE))) ? 2 : 1;
	}
	
	if ((ptrRoom->dir_option[UP]) != NULL)
	{
		map[4][x][y] = 1;
	}
	
	if ((ptrRoom->dir_option[DOWN]) != NULL)
	{
		map[4][x][y] = (map[4][x][y]) ? 3 : 2;
	}

	
	
	if ((y > 0) && !map[0][x][y - 1] && (ptrNExit = ptrRoom->dir_option[0]))
	{
		n = ptrNExit->to_room;
	}
	if (!map[0][x + 1][y] && (x + 1 < MAP_GRID_WIDTH) && (ptrEExit != NULL))
	{
		e = ptrEExit->to_room;
	}
	if (!map[0][x][y + 1] && (y + 1 < MAP_GRID_WIDTH) && (ptrSExit != NULL))
	{
		s = ptrSExit->to_room;
	}
	if (!map[0][x - 1][y] && (x - 1 >= 0)
		&& (ptrWExit = ptrRoom->dir_option[3]))
	{
		w = ptrWExit->to_room;
	}

	if (radius > MAP_MAX_RADIUS + 1)
		return;

	radius++;

	if (n)
	{
		fill_map (vtor (n), x, y - 1, map);
	}
	if (e)
	{
		fill_map (vtor (e), x + 1, y, map);
	}
	if (s)
	{
		fill_map (vtor (s), x, y + 1, map);
	}
	if (w)
	{
		fill_map (vtor (w), x - 1, y, map);
	}
	radius--;
}



void
do_map (CHAR_DATA * ch, char *argument, int cmd)
{
	int map[MAP_GRID_DEPTH][MAP_GRID_WIDTH][MAP_GRID_WIDTH];
	char buf[AVG_STRING_LENGTH * 2] = "";
	char buf2[AVG_STRING_LENGTH] = "";
	char arg1[AVG_STRING_LENGTH] = "";
	char bogy[4][2][6] = {
		{" ", " "},
		{"#b(#0", "#b)#0"},
		{"#d(#0", "#d)#0"},
		{"#f(#0", "#f)#0"}
	};
	const char strEWall[3][7] = {
		"#1X#0",
		" ",
		"#1|#0"
	};
	const char strSWall[3][18] = {
		"#1XXXXXXXXXXX#0",
		"          #1X#0",
		"#1----------X#0"
	};
	const char strVExit[4][7] = {
		" ",
		"#6'#0",
		"#6,#0",
		"#6%#0"
	};
	const char *strSect[] = {
		"", "#7", "#7", "#7", "#a",
		"#2", "#2", "#3", "#b", "#e",
		"#6", "#e", "#a", "#3", "#d",
		"", "#4", "#4", "#4", "#6",
		"#c", "#9"
	};
	
	unsigned char i = 0, j = 0, x = 0, y = 0, nInRoom = 0, bSearch = 0;
	int r = 0;
	CHAR_DATA *rch;
	ROOM_DATA *room;
	
	if (*argument)
	{
		bSearch = 1;
		argument = one_argument (argument, arg1);
	}
	
	for (i = 0; i < MAP_GRID_WIDTH; i++)
	{
		for (j = 0; j < MAP_GRID_WIDTH; j++)
		{
			map[0][i][j] = 0;	/* room */
			map[1][i][j] = 0;	/* sect */
			map[2][i][j] = 0;	/* e links */
			map[3][i][j] = 0;	/* s links */
			map[4][i][j] = 0;	/* u/d links */
			
		}
	}
	x = MAP_MAX_RADIUS;
	y = MAP_MAX_RADIUS;
	
	fill_map (ch->room, x, y, map);
	
	strcpy (buf, "#0\n");
	for (j = 0; j < MAP_GRID_WIDTH; j++)
	{
		for (i = 0; i < MAP_GRID_WIDTH; i++)
		{
			if ((r = map[0][i][j]) > 0)
			{
				if ((room = vtor (r)))
					
				{
					if (i == MAP_MAX_RADIUS && j == MAP_MAX_RADIUS)
					{
						sprintf (buf + strlen (buf), " #d<#0%6d#d>#0%s%s",
								 r,
								 strVExit[map[4][i][j]],
								 strEWall[map[2][i][j]]);
					}
					else
					{
						
						sprintf (buf + strlen (buf), " %8d#0%s%s",
								 r,
								 strVExit[map[4][i][j]],
								 strEWall[map[2][i][j]]);
					}
				}
				
				strcat (buf2, strSWall[map[3][i][j]]);
				
			}
			else
			{
				sprintf (buf + strlen (buf), "          %s",
						 ((i < MAP_GRID_WIDTH - 1)
						  && (map[0][i + 1][j])) ? strEWall[0] : " ");
				if (j < MAP_GRID_WIDTH - 1 && map[0][i][j + 1])
				{
					strcat (buf2, strSWall[0]);
				}
				else
				{
					sprintf (buf2 + strlen (buf2), "          %s",
							 ((i < MAP_GRID_WIDTH - 1)
							  && (map[0][i + 1][j])) ? strEWall[0] : "#1+#0");
				}
			}
		}
		strcat (buf, "#0\n");
		strcat (buf2, "#0\n");
		strcat (buf, buf2);
		ch->send_to_char (buf);
		buf[0] = '\0';
		buf[1] = '\0';
		buf2[0] = '\0';
		buf[2] = '\0';
	}
	
	
	strcpy (buf, "\n  #6'#0 Up       #6,#0 Down     #6%#0 Up/Down\n  ");
	
	for (i = 0; terrain_types[i][0] != '\n'; i++)
	{
		sprintf (buf + strlen (buf), "%s%-10s#0 ", strSect[i], terrain_types[i]);
		if (!((i + 1) % 7))
			strcat (buf, "\n  ");
	}
	
	
	strcat (buf, "\n  ");
	ch->send_to_char (buf);
}

void
do_aecho (CHAR_DATA * ch, char *argument, int cmd)
{


}

#include <fstream>
bool
csv_obj (CHAR_DATA* ch)
{
	std::string wear_buf;
	std::string extra_buf;
	std::string econ_buf;
	std::string oval_buf;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int i;
	bool success = false;
	std::ofstream fp ("objects.csv");
	OBJ_DATA *tobj;
	std::map<int, OBJ_DATA*>::iterator obj_iterator;

	
	if (fp)
	{
		for (obj_iterator = proto_obj_map.begin(); obj_iterator != proto_obj_map.end(); obj_iterator++)
		{
			wear_buf.clear();
			extra_buf.clear();
			econ_buf.clear();
			oval_buf.clear();
			tobj = obj_iterator->second;
			for (i = 0; (*wear_bits[i] != '\n'); i++)
				if (IS_SET (tobj->obj_flags.wear_flags, (1 << i)))
				{
					wear_buf.append(wear_bits[i]);
					wear_buf.append(" ");
				}
			
			for (i = 0; (*extra_bits[i] != '\n'); i++)
				if (IS_SET (tobj->obj_flags.extra_flags, (1 << i)))
				{
					extra_buf.append(extra_bits[i]);
					extra_buf.append(" ");
				}
			
			for (i = 0; str_cmp (econ_flags[i], "\n"); i++)
				if (IS_SET (tobj->econ_flags, 1 << i))
				{
					econ_buf.append(econ_flags[i]);
					econ_buf.append(" ");
				}
			
			sprintf (buf, "%d, %d, %d, %d, %d, %d",
					 tobj->o.od.value[0],
					 tobj->o.od.value[1],
					 tobj->o.od.value[2],
					 tobj->o.od.value[3],
					 tobj->o.od.value[4],
					 tobj->o.od.value[5]);
			oval_buf.append(buf);

			
			fp
				<< tobj->zone << ", "
				<< tobj->nVirtual << ", "
				<< item_types[(size_t)tobj->obj_flags.type_flag] << ", "
				<< wear_buf.c_str() << ", "
				<< extra_buf.c_str() << ", "
				<< econ_buf.c_str() << ", "
				<< oval_buf.c_str() << ", "
				<< tobj->name << ", "
				<< tobj->short_description << ", "
				<< tobj->description << ", "
				<< (((float)tobj->obj_flags.weight) * 0.01) << ", "
				<< tobj->coppers << ';' << std::endl;
		}
		fp.close ();
		success = true;
	}
	else
	{
		ch->send_to_char("Error: Cannot open objects.csv for writing\n");
	}
	return success;
}

void
do_becho (CHAR_DATA * ch, char *argument, int cmd)
{
	int i;
	std::string output;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char* result = NULL;

	for (i = 0; *(argument + i) == ' '; i++);

	if (!*(argument + i))
	{
		ch->send_to_char ("What did you want to echo?\n");
		return;
	}

	output.assign ("#9<<<****************************************>>>#0\n");
	output.append ("#B<<<---------------------------------------->>>#0\n\n");

	sprintf (buf, "%s", argument + i);
	result = swap_xmote_target (ch, buf, 3);
	if (!result)
		return;
	output.append (CAP(result));
	output.append ("\n\n");

	output.append ("#B<<<---------------------------------------->>>#0\n");
	output.append ("#9<<<****************************************>>>#0\n");


	send_to_room ((char*)output.c_str(), ch->in_room);

}

void
do_csv (CHAR_DATA* ch, char *argument, int cmd)
{
	if (argument[0] == 'o')
	{
		ch->send_to_char ("Begining Object Save...");
		if (csv_obj (ch))
			ch->send_to_char ("Done\n");
	}
	else
	{
		ch->send_to_char ("Usage:\no: objects\n");
	}
}

void do_wmotd(CHAR_DATA * ch, char *argument, int cmd)
{

	std::string msg_line;
	std::string output;

	std::ifstream fin("MOTD");

	if(!fin)
	{
		system_log ("The MOTD could not be found", true);
		ch->send_to_char("The MOTD could not be found");
		return;
	}

	while(getline(fin, msg_line))
	{
		output.append(msg_line);
	}

	fin.close();

	if (!output.empty())
	{
		ch->send_to_char ("The old MOTD was: \n");
		ch->send_to_char (output.c_str());
		ch->send_to_char ("\n");
	}

	ch->act("$n begins editing the MOTD.", false, 0, 0, TO_ROOM);

	free_mem(ch->desc->descStr);
	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;

	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;
	ch->delay_info1 = ch->room->nVirtual;

	ch->send_to_char
		("\nPlease enter the new MOTD; terminate with an '@'\n\n");
	ch->send_to_char
		("1-------10--------20--------30--------40--------50--------60--------70--------80\n");
	ch->make_quiet();

	ch->desc->proc = post_motd;

	return;
}


void
post_motd (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *ch;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	std::ofstream fout("MOTD");

	ch = d->character;
	ch->delay_info1 = 0;

	if (!d->pending_message->message)
	{
		ch->send_to_char ("No MOTD posted.\n");
		return;
	}

	if(!fout)
	{
		system_log ("MOTD could not be found", true);
		ch->send_to_char("MOTD could not be found");
		return;
	}

	sprintf (buf, "%s", d->pending_message->message);

	fout << buf << std::endl;
	fout.close();

	d->pending_message = NULL;


}

void do_subscribe (CHAR_DATA * ch, char *argument, int cmd)
{
	int sphereIndex = -1;
	/* ch is the character to send to, twiddle the one to adjust flags on */
	CHAR_DATA* twiddlechar = ch;


	/* check if body is switched */
	if (ch->desc && ch->desc->original)
	{
		twiddlechar = ch->desc->original;
	}


	if (IS_NPC (twiddlechar) || (ch->get_trust() < 2))
	{
		ch->send_to_char ("Eh?\n");
		return;
	}

	/* find the sphere they want to subscribe to */
	for (int i=0; i<SPHERE_COUNT; i++)
	{
		if (!strcasecmp(argument,spheres[i].name) && spheres[i].available)
		{
			sphereIndex = i;
			break;
		}
	}

	/* fail if the sphere is not recognised */
	if (sphereIndex == -1)
	{
		ch->send_to_char("That sphere was not recognised.\n");
		ch->send_to_char("Current spheres:#5\n");
		for (int j=0; j<SPHERE_COUNT; j++)
		{
			if (spheres[j].available)
			{
				ch->send_to_char(spheres[j].name);
				ch->send_to_char("\n");
			}
		}
		ch->send_to_char("#0");
		return;
	}

	/* fail if sphere already set */
	if (IS_SET(twiddlechar->petition_flags,(1<<sphereIndex)))
	{
		ch->send_to_char("You are already subscribed to that sphere's petitions.\n");
		return;
	}

	/* subscribe */
	twiddlechar->petition_flags |= (1<<sphereIndex);
	ch->send_to_char("You have subscribed to petitions and hobbitmails for #5");
	ch->send_to_char(spheres[sphereIndex].name);
	ch->send_to_char("#0.\n");
	twiddlechar->save_char_mysql();

	/* replicate flags */
	ch->petition_flags = twiddlechar->petition_flags;

}

void do_unsubscribe (CHAR_DATA * ch, char *argument, int cmd)
{
	int sphereIndex = -1;
	/* ch is the character to send to, twiddle the one to adjust flags on */
	CHAR_DATA* twiddlechar = ch;


	/* check if body is switched */
	if (ch->desc && ch->desc->original)
	{
		twiddlechar = ch->desc->original;
	}


	if (IS_NPC (twiddlechar) || !ch->get_trust())
	{
		ch->send_to_char ("Eh?\n");
		return;
	}

	/* find the sphere they want to unsubscribe from */
	for (int i=0; i<SPHERE_COUNT; i++)
	{
		if (!strcasecmp(argument,spheres[i].name))
		{
			sphereIndex = i;
			break;
		}
	}

	/* fail if the sphere is not recognised */
	if (sphereIndex == -1)
	{
		ch->send_to_char("That sphere was not recognised.\n");
		return;
	}

	/* fail if sphere not set */
	if (!IS_SET(twiddlechar->petition_flags,(1<<sphereIndex)))
	{
		ch->send_to_char("You are not subscribed to that sphere's petitions.\n");
		return;
	}

	/* unsubscribe */
	twiddlechar->petition_flags &= ~(1 << sphereIndex);
	ch->send_to_char("You have unsubscribed from petitions and hobbitmails for #5");
	ch->send_to_char(spheres[sphereIndex].name);
	ch->send_to_char("#0.\n");
	twiddlechar->save_char_mysql();

	/* replicate flags */
	ch->petition_flags = twiddlechar->petition_flags;
}



	//after rooms are refreshed, the room is empty. This function will take all loaded characters from load_list and put themback in the rooms they belong
void
reload_char(void)
{
	CHAR_DATA *tch;
	int troom;
	std::list<char_data*>::iterator tch_iterator;

	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		tch = *tch_iterator;
		troom = tch->in_room;
		tch->char_to_room(troom);
	}
	return;
}
