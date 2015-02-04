//////////////////////////////////////////////////////////////////////////////
//
/// olc.cpp : OLC Module (online building or building in the game)
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
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include </usr/local/mysql/include/mysql.h>
#include </usr/local/include/mysql++/mysql++.h>


#include "server.h"
#include "structs.h"
#include "net_link.h"
#include "account.h"
#include "protos.h"
#include "decl.h"
#include "utils.h"
#include "utility.h"

#define ROOM_MAX		100000

extern rpie::server engine;
extern const char *dirs[];
extern const char *damage_severity[];
extern const char *weather_states[];
extern const char *verbal_speeds[];

extern std::map<std::string, SUBCRAFT_HEAD_DATA*> craft_map;
extern std::map<int, ROOM_PORTAL_DATA*> portal_map;
extern std::map<int, OBJECT_MATERIAL*> object_material_map;
extern std::map<std::string, SKILL_DATA*> skill_data_map;

extern int free_mem(char *&ptr);
extern int free_mem(void *ptr);

extern MYSQL *database;
extern mysqlpp::Connection dbo;

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

const struct constant_data constant_info[18] = {
	{"item-types", "OSET                   ", (void **) item_types},
	{"econ-flags", "OSET/MSET flag         ", (void **) econ_flags},
	{"wear-bits", "OSET flag              ", (void **) wear_bits},
	{"extra-bits", "OSET flag              ", (void **) extra_bits},
	{"position-types", "MSET                   ", (void **) position_types},
	{"sex-types", "MSET                   ", (void **) sex_types},
	{"action-bits", "MSET flag              ", (void **) action_bits},
	{"profession-bits", "MSET flag              ", (void **) profession_bits},
	{"affected-bits", "MSET flag              ", (void **) affected_bits},
	{"speeds", "MSET speed             ", (void **) verbal_speeds},
	{"room-bits", "RSET flag              ", (void **) room_bits},
	{"exit-bits", "PSET flag	    	  ", (void **) exit_bits},
	{"terrain-types", "RSET terrain             ", (void **) terrain_types},
	{"weather-room", "WEATHER                  ", (void **) weather_room},
	{"variable-races", "Variable Races           ", (void **) variable_races},
	{"weather-states", "Weather states           ", (void **) weather_states},
	{"damage-severity", "Damage severity          ", (void **) damage_severity},
	{"", "", NULL}
};


const char *weather_areas[3] = {
	"Angrenost",
	"Minas-Tirith",
	"\n"
};

void
usage_wearloc (CHAR_DATA * ch)
{
	ch->send_to_char ("Usage:\n   wearloc <wear-bit>\n\nValid values for <wear-bit> include:\n");
	do_tags(ch, "wear-bits", 0);
	ch->send_to_char ("\nWhich wear location did you wish to search for?\n");

	return;
}

void
do_wearloc (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *obj;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char temp_buf[MAX_STRING_LENGTH]= { '\0' };
	char loc[MAX_STRING_LENGTH]= { '\0' };
	int wear_loc, zone = 0, count = 0;
	std::string table_txt;
	std::map<int, OBJ_DATA*>::iterator obj_iterator;
	
	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char ("You did't specify a wear location.\n\n");
		usage_wearloc (ch);
		return;
	}
	else
		sprintf (loc, "%s", buf);

	if ((wear_loc = index_lookup (wear_bits, loc)) == -1)
	{
		ch->send_to_char ("You didn't specify a valid wear location.\n\n");
		usage_wearloc (ch);
		return;
	}

	*buf = '\0';

	argument = one_argument (argument, buf);

	if (*buf && isdigit (*buf))
		zone = atoi (buf);
	else
		zone = -1;

	table_txt.assign("Vnum       Name(s)           Short Desc"
		"               Clones\tLocation\n\n");

	for (obj_iterator = proto_obj_map.begin(); obj_iterator != proto_obj_map.end(); obj_iterator++)
	{
		obj = obj_iterator->second;
		if (!obj)
			continue;
		if (zone != -1 && obj->zone != zone)
			continue;
		if (!IS_SET (obj->obj_flags.wear_flags, 1 << wear_loc))
			continue;
		count++;
		sprintf (temp_buf + strlen (temp_buf),
			"%.5d %-16.16s %-28.28s %.3d\t(R) %05d\n", obj->nVirtual,
			obj->name, obj->short_description, 999, obj->in_room);
	}

	table_txt.append(temp_buf);
	
	page_string (ch->desc, table_txt.c_str());

}


void
save_affect_reset (FILE * fp, CHAR_DATA * tmp_mob, AFFECTED_TYPE * af)
{
	CHAR_DATA *proto;

	if (af->type == MAGIC_DRAGGER || 
		af->type == MAGIC_WATCH1 ||
		af->type == MAGIC_WATCH2 || 
		af->type == MAGIC_WATCH3 ||
		af->type == MAGIC_NOTIFY || 
		af->type == MAGIC_SIT_TABLE || 
		af->type == AFFECT_SHADOW ||
		af->type == MAGIC_CLAN_NOTIFY)
		return;

	if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST)
	{
		fprintf (fp, "C '%s'\n", af->a.craft->subcraft->subcraft_name);
		return;
	}

	proto = vtom (tmp_mob->mob->nVirtual);

	if (af->type == MAGIC_AFFECT_INFRAVISION &&
		IS_SET (tmp_mob->affected_by, AFF_INFRAVIS))
		return;

	if (af->type == MAGIC_AFFECT_INVISIBILITY &&
		IS_SET (tmp_mob->affected_by, AFF_INVISIBLE))
		return;

	if (af->type == MAGIC_HIDDEN && IS_SET (tmp_mob->affected_by, AFF_HIDE))
		return;

	fprintf (fp, "A %d %d %d %d %d %d %ld\n",
		af->type,
		af->a.spell.duration,
		af->a.spell.modifier,
		af->a.spell.location,
		af->a.spell.bitvector, af->a.spell.sn, af->a.spell.t);
}

void
fwrite_resets (ROOM_DATA * troom, FILE * fp)
{
	CHAR_DATA *tmp_mob;
	OBJ_DATA *to;
	OBJ_DATA *obj;
	AFFECTED_TYPE *af;
	int j;
	int w;

	/* Write room header information if we need to write room affects */

	if (troom->affects)
		fprintf (fp, "R %d \n", troom->nVirtual);

	for (af = troom->affects; af; af = af->next)
	{
				
			fprintf (fp, "r %d %d %d %d %d %d %ld \n",
			af->type,
			af->a.spell.duration,
			af->a.spell.modifier,
			af->a.spell.location,
			af->a.spell.bitvector, af->a.spell.sn, af->a.spell.t);
	
	}

	for (j = 0; j <= LAST_DIR; j++)
	{

		if (!troom->dir_option[j])
			continue;

		if (IS_SET (troom->dir_option[j]->port_flags, EX_PICKPROOF))
			fprintf (fp, "D 0 %d %d 2\n", troom->nVirtual, j);

		else if (IS_SET (troom->dir_option[j]->port_flags, EX_LOCKED))
			fprintf (fp, "D 0 %d %d 2\n", troom->nVirtual, j);

		else if (IS_SET (troom->dir_option[j]->port_flags, EX_ISDOOR))
			fprintf (fp, "D 0 %d %d 1\n", troom->nVirtual, j);

		else if (IS_SET (troom->dir_option[j]->port_flags, EX_ISGATE))
			fprintf (fp, "D 0 %d %d 3\n", troom->nVirtual, j);
	}

	if (troom->people)
	{
		for (tmp_mob = troom->people; tmp_mob; tmp_mob = tmp_mob->next_in_room)
		{

			if (!IS_NPC (tmp_mob))
				continue;

			if (IS_SET (tmp_mob->mob->action, ACT_STAYPUT))
				continue;

			if (tmp_mob->right_hand)
			{
				to = tmp_mob->right_hand;
				fprintf (fp, "G 1 %d 0\n", to->nVirtual);
				for (to = to->contains; to; to = to->next_content)
					fprintf (fp, "P 1 %d 0 %d\n", to->nVirtual,
					to->in_obj->nVirtual);
			}

			if (tmp_mob->left_hand)
			{
				to = tmp_mob->left_hand;
				fprintf (fp, "G 1 %d 0\n", to->nVirtual);
				for (to = to->contains; to; to = to->next_content)
					fprintf (fp, "P 1 %d 0 %d\n", to->nVirtual,
					to->in_obj->nVirtual);
			}

			for (w = 0; w < MAX_WEAR; w++)
			{
				if (!get_equip (tmp_mob, w))
					continue;

				obj = get_equip (tmp_mob, w);
				fprintf (fp, "E 1 %d 0 %d\n", obj->nVirtual, w);
				if (obj->contains
					&& (obj->obj_flags.type_flag == ITEM_CONTAINER
					|| obj->obj_flags.type_flag == ITEM_KEYRING))
					fprintf (fp, "s 1 %d %d\n", obj->contains->nVirtual,
					obj->contains->count);
			}

			for (af = tmp_mob->hour_affects; af; af = af->next)
				save_affect_reset (fp, tmp_mob, af);

			
		}
	}
}


FILE *
open_and_rename (CHAR_DATA * ch, char *name, int zone)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	FILE *fp;

	sprintf (buf, "%s/%s.%d", REGIONS, name, zone);

	if ((fp = fopen (buf, "w")) == NULL)
		return NULL;

	return fp;
}



int
save_rooms (CHAR_DATA * ch, int zone)
{
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *tmob;
	OBJ_DATA *tobj;
	ROOM_DATA *troom;
	std::map<int, ROOM_DATA*>::iterator room_iterator;
	FILE *fz;
	FILE *fr;
	int room_good;
	int n;
	int empty_rooms = 0;
	static int total_empty_rooms = 0;
	char *zone_name = NULL;
	char *zone_lead = NULL;
	std::list<zone_data*>::iterator zone_iterator;
	zone_data *zone_it;
	
	sprintf (buf, "Saving rooms in zone %d.", zone);
	system_log (buf, false);
	
	
		
	if (!(fr = open_and_rename (ch, "rooms", zone)))
		return 1;
	
	if (!(fz = open_and_rename (ch, "resets", zone)))
		return 1;
	
	for (zone_iterator = zone_table.begin(); zone_iterator != zone_table.end(); zone_iterator++)
	{
		zone_it = *zone_iterator;
		if (zone_it->number == zone )
		{
			sprintf(zone_name, "%s", zone_it->name);
			sprintf(zone_lead, "%s", zone_it->lead);
			break;
		}
	}
		
	fprintf (fz, "#%d\nLead: %s~\n%s~\n%d %d %d %f %d %d\n",
			 zone,
			 zone_lead,
			 zone_name,
			 0,
			 0,
			 0,
			 0.0,
			 0,
			 0);
	
	*buf = '\0';
	
	for (room_iterator = room_map.begin(); room_iterator != room_map.end(); room_iterator++)
	{
		troom = room_iterator->second;
		if (troom->zone == zone)
		{
			
			room_good = 0;
			
			for (n = 0; n <= LAST_DIR; n++)
				if (troom->dir_option[n] && troom->dir_option[n]->to_room > 0)
					room_good = 1;
			
			if (troom->contents || troom->people)
				room_good = 1;
			
			if (strncmp (troom->description, "No Description Set", 18))
				room_good = 1;
			
			if (room_good)
			{
				fwrite_resets (troom, fz);
			}
			else
			{
				empty_rooms++;
				total_empty_rooms++;
			}
		}
	}
	
	if (empty_rooms)
	{
		sprintf (buf, "%d empty rooms were not saved for zone %d.",
				 empty_rooms, zone);
		system_log (buf, false);
		
		strcat (buf, "\n");
		ch->send_to_char (buf);
	}
	
	std::map<int, CHAR_DATA*>::iterator mob_iterator;
	for (mob_iterator = proto_mob_map.begin(); mob_iterator != proto_mob_map.end(); mob_iterator++)
	{
		tmob = mob_iterator->second;
		if (tmob->mob->zone == zone && !tmob->deleted
			&& (IS_NPC(tmob) && !IS_SET (tmob->mob->action, ACT_STAYPUT)))
		{
			save_mysql_mob (NULL, tmob, BUILD_APPROVED); 
		}
	}
	
	std::map<int, OBJ_DATA*>::iterator obj_iterator;
	for (obj_iterator = proto_obj_map.begin(); obj_iterator != proto_obj_map.end(); obj_iterator++)
	{
		tobj = obj_iterator->second;
		if (tobj->zone == zone && !tobj->deleted)
		{
			save_mysql_object(NULL, tobj, BUILD_APPROVED);
		}
	}
	
	fprintf (fr, "$~\n");
	fprintf (fz, "S\n");
	
	fclose (fr);
	fclose (fz);
	
	
	return 0;
}

/*                                                                          *
* funtion: do_review                    < e.g.> review <name>              *
*                                                                          *
* 09/17/2004 [JWW] - Fixed an instances where mysql result was not freed   *
*                                                                          */
void
do_review (CHAR_DATA * ch, char *argument, int cmd)
{
	int silent_review = 0;
	CHAR_DATA *review_ch;
	MYSQL_RES *result;
	char buf[MAX_INPUT_LENGTH];
	char name[MAX_INPUT_LENGTH];

	argument = one_argument (argument, name);

	if (IS_NPC (ch))
	{
		ch->send_to_char ("You must use your staff avatar to review any applications.\n");
		return;
	}

	if (!isalpha (*name))
	{
		ch->send_to_char ("Illegal name.\n");
		return;
	}

	argument = one_argument (argument, buf);

	if (*buf == '!')
		silent_review = 1;

	if (!(review_ch = load_pc (name)))
	{
		ch->send_to_char ("There is no pfile associated with that name, "
			"sorry.\n");
		return;
	}

	if ((!ch->get_trust())
		&& (!is_newbie (review_ch)))
	{
		ch->send_to_char
			("There is no playerfile associated with that name, sorry.\n");
		review_ch->unload_pc();
		return;
	}

	if (review_ch->pc->create_state != 1)
	{
		sprintf (buf,
			"It appears that this application has already been reviewed and responded to.\n");
		ch->send_to_char (buf);
		review_ch->unload_pc();
		return;
	}

	if (is_being_reviewed (review_ch->name, ch->desc->acct->name.c_str ()))
	{
		sprintf (buf,
			"That application is currently checked out - try again later.\n");
		ch->send_to_char (buf);
		review_ch->unload_pc();
		return;
	}

	spitstat (review_ch, ch->desc);

	ch->delay_type = DEL_APP_APPROVE;
	ch->delay = 3 * 60;
	ch->delay_who = duplicateString (name);

	mysql_safe_query
		("SELECT * FROM virtual_boards WHERE board_name = 'Applications' AND subject LIKE '%% %s'",
		review_ch->name);

	if ((result = mysql_store_result (database)) != NULL)
	{
		if (mysql_num_rows (result) >= 1)
		{
			ch->send_to_char
				("\n#6Please type HISTORY to review previous responses to this application.#0\n");
		}
		mysql_free_result (result);
		result = NULL;
	}
	ch->send_to_char ("\n#2Please ACCEPT or DECLINE this application.#0\n");

	if (!is_yours (review_ch->name, ch->desc->acct->name.c_str ()))
	{
		mysql_safe_query
			("INSERT INTO reviews_in_progress VALUES ('%s', '%s', UNIX_TIMESTAMP())",
			review_ch->name, ch->desc->acct->name.c_str());
	}

	review_ch->unload_pc();
}



std::string
olist_show (CHAR_DATA *ch, OBJ_DATA * obj, int type, int header)
{
	char temp_buf[MAX_STRING_LENGTH] = {'\0'};
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char wear_loc[MAX_STRING_LENGTH]= { '\0' };
	char armor_type[MAX_STRING_LENGTH]= { '\0' };
	char fill_buf[MAX_STRING_LENGTH]= { '\0' };
	OBJ_DATA* filling;
	std::string head;
	std::string output;
	std::string final;
	
	switch (type)
	{
		case ITEM_DRINKCON:
		case ITEM_DRYCON:
		case ITEM_FOUNTAIN:
			if (header)
			{
				head.assign("#2+--------+---------+-------+-----+----------+---------------------------+#0\n");
				head.append("#2| Virt#  | Weight | Volume | Cap | Contents | Short Description         |#0\n");
				head.append("#2+--------+--------+--------+-----+----------+---------------------------+#0\n");
			}
			
			if (((obj->obj_flags.type_flag == ITEM_DRINKCON) || (obj->obj_flags.type_flag == ITEM_DRYCON))
				&& obj->o.od.value[1]
				&& obj->o.od.value[2]
				&& (filling = vtoo (obj->o.od.value[2])))
			{
				one_argument (filling->name, fill_buf);
			}
			else
			{
				sprintf(fill_buf, "");
			}
			
			if (obj->zone == 99)
				sprintf(temp_buf, " #3%.6d%s#0 ", obj->nVirtual, *obj->full_description ? " " : "*");
			else 
				sprintf(temp_buf, " %.6d%s ", obj->nVirtual, *obj->full_description ? " " : "*");
			
			sprintf(temp_buf + strlen(temp_buf), " %5d.%02d   %5d  %5d   %-6s     %-25.25s#0\n",
					obj->obj_flags.weight / 100, obj->obj_flags.weight % 100,
					obj->o.od.value[1],
					obj->o.od.value[0],
					fill_buf,
					obj->short_description);
			
			output.assign(temp_buf);
			break;
			
		case ITEM_FOOD:
			if (header)
			{
				head.assign("#2+--------+---------+-------+---------------------------------------------+#0\n");
				head.append("#2| Virt # | Foodval | Bites | Short Description                           |#0\n");
				head.append("#2+--------+---------+-------+---------------------------------------------+#0\n");
			}
			
			if (obj->zone == 99)
				sprintf(temp_buf, " #3%.6d%s#0 ", obj->nVirtual, *obj->full_description ? " " : "*");
			else 
				sprintf(temp_buf, " %.6d%s ", obj->nVirtual, *obj->full_description ? " " : "*");
			
			sprintf(temp_buf + strlen(temp_buf), "  %3d      %3d      %s#0\n",
					obj->o.food.food_value,
					obj->o.food.bites,
					obj->short_description);
			
			output.assign(temp_buf);
			break;
			
		case -1:
		default:
			
			if (header)
			{
				head.assign("#2+--------+-------+---------------------------+-------------------------------+#0\n");
				head.append("#2| Virt # |  Cost | Name                      | Short Description             |#0\n");
				head.append("#2+--------+-------+---------------------------+-------------------------------+#0\n");
			}
			
			
			if (((obj->obj_flags.type_flag == ITEM_DRINKCON) || (obj->obj_flags.type_flag == ITEM_DRYCON))
				&& obj->o.od.value[1]
				&& obj->o.od.value[2]
				&& (filling = vtoo (obj->o.od.value[2])))
			{
				one_argument (filling->name, fill_buf);
			}
			else
			{
				sprintf(fill_buf, "");
			}
			
			if (obj->zone == 99)
				sprintf(temp_buf, " #3%.6d%s#0 ", obj->nVirtual, *obj->full_description ? " " : "*");
			else 
				sprintf(temp_buf, " %.6d%s ", obj->nVirtual, *obj->full_description ? " " : "*");
			
			sprintf(temp_buf + strlen(temp_buf), "%7.2f  %-25.25s    %-25.25s %s#0\n",
						  obj->coppers,
						  obj->name,
						  obj->short_description,
						  fill_buf);
			
			output.assign(temp_buf);
			break;
	}
	
	if (!head.empty())
	{
		final.assign(head.c_str());
		final.append(output.c_str());
		return (final);
	}
	else
	{
		final.assign(output.c_str());
		return (final);
	}
	
}

void
mlist_show (std::string *output_string, CHAR_DATA *mob, bool header)
{
	std::string line;
	std::ostringstream conversion;
	std::ostringstream conversion2;
	std::list<CHAR_DATA*>::iterator tch_iterator;
	CHAR_DATA *tch;
	int loads = 0;
	
	if (header)
	{
		output_string->clear();
		output_string->append("#2+-------+----------------------------+-------------------------------+--------+#0\n");
		output_string->append("#2|#0 Vnum  #2|#0 Keywords                #2|#0 Short Description             #2|#0 Loaded #2|#0\n");
		output_string->append("#2+-------+----------------------------+-------------------------------+--------+#0\n");
	}

	line.assign("#2| ");
	conversion << mob->mob->nVirtual;

	for (int i = 0, j = (5 - conversion.str().length()); i < j; i++)
	{
		line.append("0");
	}

	line.append(conversion.str());
	line.append(" #2|#0 ");
	line.append(mob->keywords, MIN((int) strlen(mob->keywords), 26));
	for (int i = 0, j = (27 - MIN((int) strlen(mob->keywords), 26)); i < j; i++)
	{
		line.append(" ");
	}

	line.append("#2|#0 ");
	line.append(mob->short_descr, MIN((int) strlen(mob->short_descr), 29));

	for (int i = 0, j = (30 - MIN((int) strlen(mob->short_descr), 29)); i < j; i++)
	{
		line.append(" ");
	}

	line.append("#2|#0 ");
	
	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		tch = *tch_iterator;
		if (tch->deleted || !IS_NPC(tch))
			continue;

		if (tch->mob->nVirtual == mob->mob->nVirtual)
			loads++;

	}
	
	conversion2 << loads;
	line.append(conversion2.str());

	for (int i = 0, j = (7-conversion2.str().length()); i < j; i++)
	{
		line.append(" ");
	}

	line.append("#2|#0\n");
	while (line.find_first_of('\0') != std::string::npos)
	{
		line.erase(line.find_first_of('\0'), 1);
	}

	output_string->append(line);
	return;

}



void
do_mlist (CHAR_DATA *ch, char *input_argument, int cmd)
{
	std::string buf, key1, key2, key3, clan, argument;
	int race = -1, zone = -1, clan_flags;
	bool yes_key1 = false, yes_key2 = false, yes_key3 = false, inclusive = false;
	CHAR_DATA * mob;

	argument.assign(input_argument);
	argument = one_argument (argument, buf);

	if (buf.empty())
	{
		ch->send_to_char ("Selection Parameters:\n\n");
		ch->send_to_char ("    +/-<mobile keyword>     Include/exclude mobile keyword.\n");
		ch->send_to_char ("    <zone>                  Mobiles from specified zone only.\n");
		ch->send_to_char ("    '<race>'                Only mobiles of specified race.\n");
		ch->send_to_char ("    *clan                   Only mobiles of specified clan.\n");
		ch->send_to_char ("\n\nExample:    mlist +stocky -pale 'Fallohide Hobbit' *bakers_fellowship\n");

		ch->send_to_char ("\t...would list all stocky, non-pale Fallohide Hobbits who were in the Fellowship of Bakers.\n");

		return;
	}

	while (!(buf.empty()))
	{
		inclusive = true;
		race = lookup_race_id (buf.c_str());
		if (buf.length() > 1 && isalpha(buf[0]) && race != -1)
		{
			argument = one_argument (argument, buf);
			continue;
		}

		if (isdigit(buf[0]))
		{
			if ((zone = atoi(buf.c_str())) >= MAX_ZONE)
			{
				ch->send_to_char("Zone not in range 0...99\n");
				return;
			}
			argument = one_argument (argument, buf);
			continue;
		}

		switch (buf[0])
		{
		case '-':
			inclusive = 0;

		case '+':
			if (!buf[1])
			{
				ch->send_to_char ("Expected a keyword after (#2+/-#0).\n");
				return;
			}

			if (key1.empty())
			{
				yes_key1 = inclusive;
				buf.erase(0, 1);
				key1 = buf;
			}
			else if (key2.empty())
			{
				yes_key2 = inclusive;
				buf.erase(0, 1);
				key2 = buf;
			}
			else if (!key3.empty())
			{
				ch->send_to_char ("Sorry, at most three keywords.\n");
				return;
			}
			else
			{
				yes_key3 = inclusive;
				buf.erase(0, 1);
				key3 = buf;
			}
			break;

		case '*':
			if (!buf[1])
			{
				ch->send_to_char ("Expected a keyword after (#2*#0).\n");
				return;
			}
			buf.erase(0, 1);
			clan = buf;
			break;
		}
		argument = one_argument (argument, buf);
	}

	std::string out_string;
	int count = 0;
	bool header = true;
	std::map<int, CHAR_DATA*>::iterator mob_iterator;
	for (mob_iterator = proto_mob_map.begin(); mob_iterator != proto_mob_map.end(); mob_iterator++)
	{
		mob = mob_iterator->second;
		if (zone != -1 && mob->mob->zone != zone)
			continue;
		if (race != -1 && mob->race != race)
			continue;
		if (!clan.empty() && !get_clan (mob, clan.c_str(), clan_flags))
			continue;
		if (!key1.empty())
		{
			if (yes_key1 && !isname(key1.c_str(), mob->keywords))
				continue;
			else if (!yes_key1 && isname(key1.c_str(), mob->keywords))
				continue;
		}
		if (!key2.empty())
		{
			if (yes_key2 && !isname(key2.c_str(), mob->keywords))
				continue;
			else if (!yes_key2 && isname(key2.c_str(), mob->keywords))
				continue;
		}
		if (!key3.empty())
		{
			if (yes_key3 && !isname(key3.c_str(), mob->keywords))
				continue;
			else if (!yes_key3 && isname(key3.c_str(), mob->keywords))
				continue;
		}
		count++;
		if (count < 200)
			mlist_show (&out_string, mob, header);

		header = false;

	}
	if (count == 0)
	{
		ch->send_to_char("No results found.");
		return;
	}

	if (count > 200)
	{
		buf.assign("You have selected #6");
		std::ostringstream conversion;
		conversion << count;

		buf.append(conversion.str());
		buf.append("#0 mobiles, which is too many to display.\n");
		ch->send_to_char(buf.c_str());
		return;
	}

	buf.assign("Searching:");

	if (!clan.empty())
	{
		CLAN_DATA *clan_def = NULL;

		if ((clan_def = get_clandef (clan.c_str())))
		{
			buf.append(" Clan [#6");
			buf.append(clan_def->literal);
			buf.append("#0]");
		}
		else
		{
			buf.append(" Clan [#6");
			buf.append(clan);
			buf.append("#1 (Unregistered)#0]");
		}
	}

	if (zone != -1)
	{
		buf.append(" Zone [#6");
		std::ostringstream conversion;
		conversion << zone;
		buf.append(conversion.str());
		buf.append("#0]");
	}

	if (race != -1)
	{
		buf.append(" Race [#6");
		buf.append(lookup_race_variable(race, RACE_NAME));
		buf.append("#0]");
	}

	if (!key1.empty())
	{
		if (key2.empty())
		{
			buf.append(" Keyword [");
		}
		else
		{
			buf.append(" Keywords [");
		}
		
		if (yes_key1)
			buf.append("#2");
		else
			buf.append("#1");

		buf.append(key1);
		buf.append("#0");

		if (!key2.empty())
		{
			buf.append(" ");
			if (yes_key2)
				buf.append("#2");
			else
				buf.append("#1");

			buf.append(key2);
			buf.append("#0");
		}

		if (!key3.empty())
		{
			buf.append(" ");
			if (yes_key3)
				buf.append("#2");
			else
				buf.append("#1");

			buf.append(key3);
		}

		buf.append ("#0]");
	}
	buf.append("\n");

	ch->send_to_char(buf.c_str());

	out_string.append("#2+-------+----------------------------+-------------------------------+--------+#0\n");
	page_string (ch->desc, out_string.c_str());

}

void
do_olist (CHAR_DATA * ch, char *argument, int cmd)
{
	int header = 1;
	int type = -1;
	int inclusive;
	int zone = -1;
	int yes_key1 = 0;
	int yes_key2 = 0;
	int yes_key3 = 0;
	int count = 0;
	OBJ_DATA *obj;
	char key1[MAX_STRING_LENGTH] = { '\0' };
	char key2[MAX_STRING_LENGTH] = { '\0' };
	char key3[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };
	std::map<int, OBJ_DATA*>::iterator obj_iterator;
	std::string output;
	
	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char ("Selection Parameters:\n\n");
		ch->send_to_char
			("   +/-<object keyword>       Include/exclude object keyword.\n");
		ch->send_to_char ("   <zone>                    Objects from zone only.\n");
		ch->send_to_char
			("   <item-type>               Include items of item-type.\n");
		ch->send_to_char ("\nExample:   olist +bag -red container 10\n");
		ch->send_to_char
			("will only get non-red bags of type container from zone 10.\n");
		return;
	}

	while (*buf)
	{

		inclusive = 1;

		if (strlen (buf) > 1 && isalpha (*buf) &&
			(type = index_lookup (item_types, buf)) != -1)
		{
			argument = one_argument (argument, buf);
			continue;
		}

		if (isdigit (*buf))
		{

			if ((zone = atoi (buf)) >= OBJECT_MAX_ZONE)
			{
				ch->send_to_char ("Zone not in range 0..110\n");
				return;
			}

			argument = one_argument (argument, buf);
			continue;
		}

		switch (*buf)
		{

		case '-':
			inclusive = 0;
		case '+':

			if (!buf[1])
			{
				ch->send_to_char ("Expected keyname after 'k'.\n");
				return;
			}

			if (!*key1)
			{
				yes_key1 = inclusive;
				strcpy (key1, buf + 1);
			}
			else if (!*key2)
			{
				yes_key2 = inclusive;
				strcpy (key2, buf + 1);
			}
			else if (*key3)
			{
				ch->send_to_char ("Sorry, at most three keywords.\n");
				return;
			}
			else
			{
				yes_key3 = inclusive;
				strcpy (key3, buf + 1);
			}

			break;

		case 'z':

			argument = one_argument (argument, buf);

			if (!isdigit (*buf) || atoi (buf) >= OBJECT_MAX_ZONE)
			{
				ch->send_to_char ("Expected valid zone after 'z'.\n");
				return;
			}

			zone = atoi (buf);

			break;
		}

		argument = one_argument (argument, buf);
	}

		

	for (obj_iterator = proto_obj_map.begin(); obj_iterator != proto_obj_map.end(); obj_iterator++)
	{
		obj = obj_iterator->second;

		if (zone != -1 && obj->zone != zone)
			continue;

		if (type != -1 && obj->obj_flags.type_flag != type)
			continue;

		if (*key1)
		{
			if (yes_key1 && !isname (key1, obj->name))
				continue;
			else if (!yes_key1 && isname (key1, obj->name))
				continue;
		}

		if (*key2)
		{
			if (yes_key2 && !isname (key2, obj->name))
				continue;
			else if (!yes_key2 && isname (key2, obj->name))
				continue;
		}

		if (*key3)
		{
			if (yes_key3 && !isname (key3, obj->name))
				continue;
			else if (!yes_key3 && isname (key3, obj->name))
				continue;
		}

		count++;
		if (count == 1)
			header = 1;
		else 
			header = 0;

		if (count < 200)
			output.append(olist_show (ch, obj, type, header));
	}

	if (count > 200)
	{
		sprintf (buf, "You have selected %d objects. Only the first 200 will be displayed).\n",
			count);
		ch->send_to_char (buf);
		page_string (ch->desc, output.c_str());
	}
	else
		page_string (ch->desc, output.c_str());


	return;
}

/*                                                                          *
* funtion: do_show                      < e.g.> show (k|a|l|v|m|o|q|r|u|z) *
*/
	//TODO: re-write most of these functions to make SQL queiers where possible
void
do_show (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf1[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char buf3[MAX_STRING_LENGTH]= { '\0' };
	char buf4[MAX_STRING_LENGTH]= { '\0' };
	char tmp[MAX_STRING_LENGTH]= { '\0' };
	std::ostringstream stream;
	std::list<CHAR_DATA*>::iterator tch_iterator;
	std::list<CHAR_DATA*>::iterator tch_iterator_two;
	std::list<zone_data*>::iterator zone_iterator;
	std::list<obj_data*>::iterator tobj_iterator;
	std::map<int, ROOM_DATA*>::iterator room_iterator;
	std::map<int, ROOM_PORTAL_DATA*>::iterator tport_iterator;
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	zone_data *zone_it;
	char *date;
	int mtop = 0, count, count2, otop = 0, n, i;
	int count_total_rooms, docs = 0;
	OBJ_DATA *tobj;
	CHAR_DATA *tch;
	ROOM_DATA *troom;
	ROOM_PORTAL_DATA *tport;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int keepers_only = 0;
	int got_line = 0;
	int craft_tot = 0;
	int big_counts_only = 0;
	int undesc_only = 0;
	int stayput = 0, chars = 0, accounts = 0;
	int search_type = 0;
	char *obj_name;
	char *mob_name;
	char *zone_str;
	char *port_key;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	time_t current_time;
	int timeframe = 0;  // 0 - all, 1 = day, 2 - week, 3 - month
		
		
	arg_splitter (4, argument, buf1, buf2, buf3, buf4);

	if ((!ch->get_trust()) && !IS_NPC (ch) && !ch->pc->is_guide)
	{
		ch->send_to_char ("Eh?\n");
		return;
	}

	if (ch->pc->is_guide && (!ch->get_trust()))
	{
		if (*buf1 != 'l' && *buf1 != 'L')
		{
			ch->send_to_char
				("Type SHOW L to get a list of applications in the queue.\n");
			return;
		}
	}

	if  (ch->get_trust() == 1)
	{
		if (*buf1 != 'a' &&
			*buf1 != 'o' &&
			*buf1 != 'p' &&
			*buf1 != 'r')

		{
			ch->send_to_char ("   a           area stats\n");
			ch->send_to_char ("   o           objects\n");
			ch->send_to_char ("   p           portals\n");
			ch->send_to_char ("   r           rooms\n");
			return;
		}
	}

	if  (ch->get_trust() == 2)
	{
		if (*buf1 != 'a' &&
			*buf1 != 'k' &&
			*buf1 != 'o' &&
			*buf1 != 'm' &&
			*buf1 != 'p' &&
			*buf1 != 'r' )

		{
			ch->send_to_char ("   a           area stats\n");
			ch->send_to_char ("   k           shopkeepers\n");
			ch->send_to_char ("   o           objects\n");
			ch->send_to_char ("   m           mobiles\n");
			ch->send_to_char ("   p           portals\n");
			ch->send_to_char ("   r           rooms\n");
			return;
		}
	}

	if  ((ch->get_trust() == 3) || (ch->get_trust() == 4))
	{
		if (*buf1 != 'a' &&
			*buf1 != 'c' &&
			*buf1 != 'k' &&
			*buf1 != 'l' &&
			*buf1 != 'm' &&
			*buf1 != 'o' &&
			*buf1 != 'p' &&
			*buf1 != 'r' &&
			*buf1 != 's' )

		{
			ch->send_to_char ("   a           area stats\n");
			ch->send_to_char ("   c           characters matching search\n");
			ch->send_to_char ("   k           shopkeepers\n");
			ch->send_to_char ("   l           applications\n");
			ch->send_to_char ("   o           objects\n");
			ch->send_to_char ("   m           mobiles\n");
			ch->send_to_char ("   p           portals\n");
			ch->send_to_char ("   r           rooms\n");
			ch->send_to_char ("   s           summary totals\n");
			return;
		}
	}

	if (!*buf1)
	{
		ch->send_to_char ("   a           area stats\n");
		ch->send_to_char ("   c           characters matching search\n");
		ch->send_to_char ("   k           shopkeepers\n");
		ch->send_to_char ("   l           applications\n");
		ch->send_to_char ("   m           mobiles\n");
		ch->send_to_char ("   o           objects\n");
		ch->send_to_char ("   p           portals\n");
		ch->send_to_char ("   r           rooms\n");
		ch->send_to_char ("   s           summary totals\n");
	}

		switch (*buf1)
	{

	

	case 'p': //portals
			
			port_key = buf3;
			zone_str = NULL;
			
			if (!buf2)
			{
				ch->send_to_char("  'show p 3' to show portals in zone 3\n");
				ch->send_to_char("  'show p stone' to show all portals with stone as a keyword\n");
				ch->send_to_char("  'show p 3 stone' to show all stone portals in zone 3\n");	
			}
			else if (*buf2 && !isdigit (*buf2))
				port_key = duplicateString (buf2);
			else if (*buf2)
				zone_str = buf2;
			
			if ((portal_map.size() > 50) && (!*buf2)) 
			{
				ch->send_to_char("  There are more than 50 portals. You should specify a zone, keyword or both.\n");
				ch->send_to_char("  'show p 3' to show portals in zone 3\n");
				ch->send_to_char("  'show p stone' to show all portals with stone as a keyword\n");
				ch->send_to_char("  'show p 3 stone' to show all stone portals in zone 3\n");
			}
			
			sprintf(tmp, "#2 +-------+----------------------------------+-----------------------------------+#0\n");
			sprintf (tmp+ strlen(tmp), " #2|#0 Ident #2|#0        Short desc (room 1)       #2|#0       Short desc (room 2)#2         |#0\n");
			sprintf(tmp + strlen(tmp), "#2 +-------+----------------------------------+-----------------------------------+#0\n");
			
			for (tport_iterator = portal_map.begin(); tport_iterator != portal_map.end(); tport_iterator++)
			{
				tport = tport_iterator->second;
				
				if (zone_str && tport->zone != atoi (zone_str))
					continue;
				
				if ((*port_key && strstr (tport->sdesc_1, port_key)) 
					|| (*port_key && strstr (tport->sdesc_2, port_key))
					|| !*port_key)
				{
											
					sprintf (tmp + strlen (tmp),
							 "#2 | %.5d |#0 %-26.26s(%-.4d) #2|#0  %-26.26s(%-.4d)#0 #2|#0\n",
							 tport->ident,
							 tport->sdesc_1 ? tport->sdesc_1 : "(none)",
							 tport->room_1,
							 tport->sdesc_2 ? tport->sdesc_2 : "(none)",
							 tport->room_2
							 );
				}
				
			}
			sprintf(tmp + strlen(tmp), "#2 +-------+----------------------------------+-----------------------------------+#0\n");
			page_string (ch->desc, tmp);
		break;

	case 'l':			/* Pending applications */

		current_time = time (0);
		date = new char[256];
		date[0] = '\0';
		if (asctime_r (localtime (&current_time), date) != NULL)
		{
			date[strlen (date) - 1] = '\0';
		}
		sprintf (buf, "\nApplication Queue, as of #2%s#0:\n\n", date);
		free_mem (date);
		ch->send_to_char (buf);

		mysql_safe_query
			("SELECT name, "
			"HOUR(TIMEDIFF(NOW(), FROM_UNIXTIME(birth))) AS birth,"
			"account, "
			"plrflags, "
			"sdesc "
			"FROM pfiles "
			"WHERE create_state = 1 "
			"ORDER BY birth DESC");
		result = mysql_store_result (database);

		if (result)
		{
			if (mysql_num_rows (result))
			{
				sprintf (buf, "  #2%-15s#0  #2%-15s#0  #2%-8s#0  #2#0\n",
					(!ch->get_trust()) ? "" : "Account", "Character",
					"In Queue");
				ch->send_to_char (buf);
			}
			while ((row = mysql_fetch_row (result)))
			{
				bool is_guide_rev = (!ch->get_trust());
				bool is_newb_app = is_newbie_acct(row[2]);
				

				if (is_guide_rev && (!is_newb_app))
					continue;

				got_line++;
				sprintf (buf,
					"  %-15s  %-15s  ",
					is_guide_rev ? "" : row[2],
					row[0]);
				current_time = strtol (row[1], NULL, 10);

				if (current_time >= 48)
					sprintf (buf + strlen (buf), "#1%3ldd %2ldh#0",
					current_time / 24, current_time % 24);
				else if (current_time >= 24)
					sprintf (buf + strlen (buf), "#3%3ldd %2ldh#0",
					current_time / 24, current_time % 24);
				else
					sprintf (buf + strlen (buf), "#2   %c %2dh#0",
					(!current_time) ? '<' : ' ',
					(int) MAX (1, (int)current_time));

				if (!IS_NPC (ch))
				{
					if (!is_guide_rev)
					{
						if (is_newb_app)
							sprintf (buf + strlen (buf), " #6(New)#0");

					}
					// combine the two following sql calls into one
					if (is_yours (row[0], ch->pc->account_name))
						sprintf (buf + strlen (buf), " #3(Yours)#0");
					else if (is_being_reviewed (row[0], ch->pc->account_name))
						sprintf (buf + strlen (buf), " #2(Checked Out)#0");

				}
				strcat (buf, "\n");
				ch->send_to_char (buf);
			}

			mysql_free_result (result);
			result = NULL;
		}

		if (!got_line)
			ch->send_to_char ("  None\n");

		break;

	case 'k':
		keepers_only = 1;

	case 'm':			/* Mobile */

		mob_name = buf3;
		zone_str = NULL;

		if (*buf2 && !isdigit (*buf2))
			mob_name = duplicateString (buf2);
		else if (*buf2)
			zone_str = buf2;
		
		if ((character_list.size() > 50) && (!*buf2))
		{
			ch->send_to_char("  There are more than 50 mobiles. You should specify a zone, keyword or both.\n");
			ch->send_to_char("  'show m 3' to show mobiles in zone 3\n");
			ch->send_to_char("  'show m zombie' to show all mobiles with zombie as a keyword\n");
			ch->send_to_char("  'show m 3 zombie' to show all zombies in zone 3\n");
			}

		sprintf (tmp, "Vnum  K            Name(s)                     Short desc               Room\n\n");
			
			for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
			{
				tch = *tch_iterator;	
			
			if (!IS_NPC (tch))
				continue;

			if (keepers_only && !tch->mob->shop)
				continue;

			if (zone_str && tch->mob->zone != atoi (zone_str))
				continue;

			if (!*tch->keywords)
				continue;

			if ((*mob_name && strstr (tch->keywords, mob_name)) || !*mob_name)
			{
				if (IS_SET (tch->flags, FLAG_KEEPER))
					sprintf (tmp + strlen (tmp),
					"%.5d * %-28.28s  %-31.31s#0   %.5d\n",
					tch->mob->nVirtual, tch->keywords, tch->short_descr, tch->in_room);
				else
				{
					sprintf (tmp + strlen (tmp),
						"%.5d   %-28.28s  %-31.31s#0   %.5d\n",
						tch->mob->nVirtual, tch->keywords, tch->short_descr, tch->in_room);
				}
				if (strlen (tmp) >= (MAX_STRING_LENGTH - 80)) //only 1 more line of room left
				{
					ch->send_to_char
					("Too many matches found - please use a more selective criterion!\n");
					return;
				}
			}
		}
		page_string (ch->desc, tmp);
		break;

	case '!':
		big_counts_only = 1;

	
	case 'o':			/* object */

		obj_name = buf3;
		zone_str = NULL;

		if (*buf2 && !isdigit (*buf2))
			obj_name = duplicateString (buf2);
		else if (*buf2)
			zone_str = buf2;
		
		if ((object_list.size() > 50) && (!*buf2))
		{
			ch->send_to_char("  There are more than 50 objects. You should specify a zone, keyword or both.\n");
			ch->send_to_char("  'show o 3' to show objects in zone 3\n");
			ch->send_to_char("  'show o tankard' to show all objects with tankard as a keyword\n");
			ch->send_to_char("  'show o 3 tankard' to show all tankards in zone 3\n");
		}
		
			sprintf (tmp, "Vnum       Name(s)           Short desc\n\n");
		
			for (tobj_iterator = object_list.begin(); tobj_iterator != object_list.end(); tobj_iterator++)
			{
				tobj = *tobj_iterator;
				
				if (zone_str && tobj->zone != atoi (zone_str))
					continue;
				
				if (big_counts_only && tobj->count < 100)
					continue;
				
				if ((*obj_name && strstr (tobj->name, obj_name)) || !*obj_name)
				{
					if (!*tobj->name)
						continue;
					
					if (tobj->in_obj)
						sprintf (tmp + strlen (tmp),
								 "%.5d  %-21.21s  %-28.28s#0  \t(I) %.5d\n",
								 tobj->nVirtual, tobj->name, tobj->short_description,
								 tobj->in_obj->nVirtual);
					
					else if (tobj->carried_by)
						sprintf (tmp + strlen (tmp),
								 "%.5d  %-21.21s  %-28.28s#0  \t(C) %.5d\n",
								 tobj->nVirtual, tobj->name, tobj->short_description,
								 tobj->carried_by->in_room);
					
					else if (tobj->equiped_by)
						sprintf (tmp + strlen (tmp),
								 "%.5d  %-21.21s  %-28.28s#0  \t(E) %.5d\n",
								 tobj->nVirtual, tobj->name, tobj->short_description, 
								 tobj->equiped_by->in_room);
					
					else
						sprintf (tmp + strlen (tmp),
								 "%.5d  %-21.21s  %-28.28s#0  \t(R) %.5d\n",
								 tobj->nVirtual, tobj->name, tobj->short_description, 
								 tobj->in_room);
					
					if (strlen (tmp) >= (MAX_STRING_LENGTH - 80)) //only 1 more line of room left
					{
						ch->send_to_char
						("Too many matches found - please use a more selective criterion!\n");
						return;
					}
				}
			}
		page_string (ch->desc, tmp);
		break;

					
	case 'c':
		if (!str_cmp (buf2, "keyword"))
			search_type = SEARCH_KEYWORD;
		else if (!str_cmp (buf2, "sdesc"))
			search_type = SEARCH_SDESC;
		else if (!str_cmp (buf2, "ldesc"))
			search_type = SEARCH_LDESC;
		else if (!str_cmp (buf2, "fdesc"))
			search_type = SEARCH_FDESC;
		else if (!str_cmp (buf2, "clan"))
			search_type = SEARCH_CLAN;
		else if (!str_cmp (buf2, "skill"))
			search_type = SEARCH_SKILL;
		else if (!str_cmp (buf2, "room"))
			search_type = SEARCH_ROOM;
		else if (!str_cmp (buf2, "race"))
			search_type = SEARCH_RACE;
		else if (!str_cmp (buf2, "stat"))
			search_type = SEARCH_STAT;
		else if (!str_cmp (buf2, "level") && ch->get_trust() >= 5)
			search_type = SEARCH_LEVEL;
		else
		{
			if ((ch->get_trust() > 1) && (ch->get_trust() < 5))
				ch->send_to_char
				("Search for: keyword, sdesc, ldesc, fdesc, clan, skill, race or room.\n");
			else
				ch->send_to_char
				("Search for: keyword, sdesc, ldesc, fdesc, clan, skill, race, room, or level.\n");
			return;
		}

		if (*buf4) {
			if (strcmp(buf4,"today") == 0) {
				timeframe = 1;
			}
			if (strcmp(buf4,"day") == 0) {
				timeframe = 1;
			}
			else if (strcmp(buf4,"week") == 0) {
				timeframe = 2;
			}
			else if (strcmp(buf4,"fortnight") == 0) {
				timeframe = 4;
			}
			else if (strcmp(buf4,"month") == 0) {
				timeframe = 3;
			}
		}
		result = mysql_player_search (search_type, buf3, timeframe);
		if (!result || !mysql_num_rows (result))
		{
			if (result)
				mysql_free_result (result);
			ch->send_to_char ("No playerfiles matching your search were found.\n");
			return;
		}
		sprintf (buf, "#6Playerfiles Matching Search: %d#0\n"
			"(#2approved#0, #1dead#0, #5suspended#0, #6pending#0)\n\n",
			(int) mysql_num_rows (result));
		i = 1;

		while ((row = mysql_fetch_row (result)))
		{
			// row[0] = account
			// row[1] = name
			// row[2] = sdsc
			// row[3] = create_state
			// opt row[4] = clan_rank

			const int color_state [5] = { 6, 6, 2, 5, 1 };
			int create_state = strtol (row[3], 0, 10);

			// safeguard wonky create_state values
			if (create_state < 0 || create_state > 4)
				create_state = 0;

			sprintf (buf2, "%4d. %-12s #%d%-10s",
				i, row[0], color_state[create_state], row[1]);
			if (search_type == SEARCH_CLAN)
			{
				sprintf (buf2 + strlen(buf2), " #0%-10s#%d",
					row[4], color_state[create_state]);
			}
			sprintf (buf2 + strlen(buf2), " %s#0\n", row[2]);
			if (strlen (buf) + strlen (buf2) >= MAX_STRING_LENGTH)
				break;
			else
				sprintf (buf + strlen (buf), "%s", buf2);
			i++;
			
			if (strlen (buf) >= (MAX_STRING_LENGTH - 80)) //only 1 more line of room left
			{
				ch->send_to_char
				("Too many matches found - please use a more selective criterion!\n");
				return;
			}
		}
		page_string (ch->desc, buf);
		mysql_free_result (result);
		break;

	case 'r':			// Room 

		*tmp = 0;
		for (room_iterator = room_map.begin(); room_iterator != room_map.end(); room_iterator++)
			{
				troom = room_iterator->second;

			if (*buf3 && atoi (buf3) > troom->nVirtual)
				continue;

			if (*buf4 && atoi (buf4) < troom->nVirtual)
				continue;

			if (undesc_only 
				&& !IS_SET (troom->room_flags, STORAGE) 
				&& troom->description)
				continue;

			if (IS_SET (troom->room_flags, STORAGE))
				strcat (tmp, "S");
			else
				strcat (tmp, " ");


			sprintf (tmp + strlen (tmp), "[%5d] %s", troom->nVirtual,
				troom->name);

					//Only shows openings, doors and gates. Other portals are not displayed
			for (n = 0; n <= LAST_DIR; n++)
			{
				if (troom->dir_option[n] && troom->dir_option[n]->to_room != -1)
				{
					switch (n)
					{
					case 0:
						sprintf (tmp + strlen (tmp), "  N [%5d]",
							troom->dir_option[n]->to_room);
						break;
					case 1:
						sprintf (tmp + strlen (tmp), "  E [%5d]",
							troom->dir_option[n]->to_room);
						break;
					case 2:
						sprintf (tmp + strlen (tmp), "  S [%5d]",
							troom->dir_option[n]->to_room);
						break;
					case 3:
						sprintf (tmp + strlen (tmp), "  W [%5d]",
							troom->dir_option[n]->to_room);
						break;
					case 4:
						sprintf (tmp + strlen (tmp), "  U [%5d]",
							troom->dir_option[n]->to_room);
						break;
					case 5:
						sprintf (tmp + strlen (tmp), "  D [%5d]",
							troom->dir_option[n]->to_room);
						break;
					}
				}
			}
			sprintf (tmp + strlen (tmp), "\n");
			if (strlen (tmp) >= (MAX_STRING_LENGTH - 80)) //only 1 more line of room left
			{
				ch->send_to_char
				("Too many matches found - please use a more selective criterion!\n");
				return;
			}
		}
		page_string (ch->desc, tmp);
		break;
	
	case 'a': 
		sprintf (tmp,
			"    Name                       Rooms     Plyrs     Mobs     Objs\n\n");
			
				for (i = 0; i < MAX_ZONE; i++)
				{
					mysql_safe_query("SELECT name FROM zones WHERE number = %d", i);
					result = mysql_store_result (database);
					row = mysql_fetch_row(result);
					
					if (mysql_num_rows (result) > 0)
					{
						sprintf (tmp + strlen (tmp), "%-24.24s", row[0]);
					}
					else 
					{
						continue;
					}
					
					mysql_free_result (result);
					
					
					count = 0;
					n = 0;
					for (room_iterator = room_map.begin(); room_iterator != room_map.end(); room_iterator++)
					{
						troom = room_iterator->second;
						if (troom->zone == i)
						{
							count++;
							if (troom->description)
								n++;
						}
					}
					sprintf (tmp + strlen (tmp), "%10d/%-4d", count, n);
					count = 0;
					count2 = 0;
					
					for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
					{
						tch = *tch_iterator;
						
							
						if (tch->in_room / 1000 == i)
							count++;
						
						if (!IS_NPC (tch) && tch->room && tch->room->zone == i)
							count2++;
					}

					sprintf (tmp + strlen (tmp), "%6d%10d", count2, count);
					count = 0;
					
					for (tobj_iterator = object_list.begin(); tobj_iterator != object_list.end(); tobj_iterator++)
					{
						tobj = *tobj_iterator;
						if (!tobj->deleted && tobj->zone == i)
							count++;
					}
					sprintf (tmp + strlen (tmp), "%10d\n", count);
					
				}
		page_string (ch->desc, tmp);
		break;

	case 's':
		n = 0;
		otop = 0;
		mtop = 0;

		for (count_total_rooms = 0, room_iterator = room_map.begin(); room_iterator != room_map.end(); room_iterator++, count_total_rooms++)
		{
			troom = room_iterator->second;
			
			if (troom->description)
				n++;
		}
			
		for (tobj_iterator = object_list.begin(); tobj_iterator != object_list.end(); tobj_iterator++)
		{
			tobj = *tobj_iterator;
			
			if (!tobj->deleted)
				otop++;
		}
		
		

		for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
		{
			tch = *tch_iterator;
			if (tch->deleted)
				continue;
			if (!tch->deleted && IS_NPC (tch))
				mtop++;
			
			if (IS_NPC (tch) && IS_SET (tch->mob->action, ACT_STAYPUT))
				stayput++;
		}


		mysql_safe_query ("SELECT count(name) from pfiles");
		result = mysql_store_result (database);
		if (result)
		{
			row = mysql_fetch_row (result);
			if (row)
				chars = atoi (row[0]);
			mysql_free_result (result);
			result = NULL;
		}

		mysql_safe_query ("SELECT count(id) from account");
		result = mysql_store_result (database);
		if (result)
		{
			row = mysql_fetch_row (result);
			if (row)
				accounts = atoi (row[0]);
			mysql_free_result (result);
			result = NULL;
		}

		mysql_safe_query ("SELECT count(*) from player_writing");
		result = mysql_store_result (database);
		if (result)
		{
			row = mysql_fetch_row (result);
			if (row)
				docs = atoi (row[0]);
			mysql_free_result (result);
			result = NULL;
		}

		craft_tot = craft_map.size();

		sprintf (tmp, "\n   Described Rooms:  %-5d\n"
			"   Total Rooms:      %-5d\n"
			"   Total Mobiles:    %-5d\n"
			"   Total Objects:    %-5d\n"
			"   Total Crafts:     %-5d\n\n"
			"   Total Characters: %-5d\n"
			"   Total Accounts:   %-5d\n"
			"   Player Writings:  %-5d\n\n"
			"   Stayput Mobiles:  %-5d\n",
			count_total_rooms - n, count_total_rooms, mtop, otop,
			craft_tot, chars, accounts, docs, stayput);
		ch->send_to_char (tmp);
		break;
			
	default:
		ch->send_to_char ("Not a valid show option.\n");
		break;
	}
	return;
}



void
post_rcret (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *ch;

	ch = d->character;
	ROOM_DATA* r = vtor(ch->in_room);
	
	if(!r->secrets[ch->delay_info1])
	{
		ch->send_to_char("There was an error saving the secret description. Please report this to a coder.\n");
	}
	else
	{
		r->secrets[ch->delay_info1]->stext = duplicateString(d->descStr);
		ch->send_to_char("Room secret was installed.\n");
		
		std::stringstream tempstr;
		tempstr << reformat_desc(r->secrets[ch->delay_info1]->stext);	
		r->secrets[ch->delay_info1]->stext = duplicateString(tempstr.str().c_str());
		
		
		builder_log(ch, "secret_desc", d->descStr);
	}

	ch->delay_who = NULL;
	ch->delay_info1 = 0;
}

void
room_secret (CHAR_DATA * ch, char *argument)
{
	char* buf1;
	char* buf2;
	int dir;
	int difficulty;
	struct secret *r_secret;
	ROOM_DATA *troom;
	
	argument = one_argument(argument, buf1);
	argument = one_argument(argument, buf2);
	
	dir = index_lookup (dirs, buf1);
	
	troom = vtor(ch->in_room);
	
	if (dir == -1)
	{
		ch->send_to_char ("What direction is that?\n");
		return;
	}
	
	if (troom->secrets[dir])
	{
		if (!strcmp(buf2, "remove"))
		{
			ch->send_to_char("Secret description removed.\n");
			troom->secrets[dir] = 0;
			return;
		}
	}
	
	difficulty = atoi(buf2);
	
	if (difficulty <= 0)
	{
		ch->send_to_char ("What do you want for the difficulty rating?\n");
		return;
	}
	
	if (vtor (ch->in_room)->secrets[dir])
	{
		ch->send_to_char ("The old secret description was: \n\n");
		ch->send_to_char (vtor (ch->in_room)->secrets[dir]->stext);
		r_secret = vtor (ch->in_room)->secrets[dir];
	}
	else
	{
		r_secret = new secret;
	}
	r_secret->diff = difficulty;
	vtor (ch->in_room)->secrets[dir] = r_secret;
	
	free_mem(ch->desc->descStr);
	ch->send_to_char ("\nEnter a new secret description.  Terminate with an '@'\n");

	ch->make_quiet();
	r_secret->stext = 0;
	ch->desc->max_str = 2000;
	ch->delay_info1 = dir;
	ch->desc->proc = post_rcret;

	ch->send_to_char ("Done.\n");
}


void
room_new (CHAR_DATA * ch, char *argument)
{
	int virt_nr;
	ROOM_DATA *troom;
	ROOM_DATA *tmp_room;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (!*argument)
	{
        sprintf(buf, "INSERT INTO rooms (name, description, last_editor, status) VALUES ('New Room', 'No Description Set', '%d', 'pending')",
            (ch->desc != NULL && ch->desc->acct != NULL) ? ch->desc->acct->get_id() : 0);
		
		mysql_safe_query (buf);
		
        if ( (virt_nr = mysql_insert_id(database)) < 1 )
		{
			ch->send_to_char ("There was a problem generating a room number. Try again later.\n");
			return;
		}
	}
	
	else  //we have a possible argument for new room number
	{
		tmp_room = vtor(atoi(argument));
		if (tmp_room)
		{
			ch->send_to_char ("That room already exists!\n");
			return;
		}
		else
		{
			virt_nr = atoi(argument);
			sprintf(buf, "INSERT INTO rooms (nVirtual, name, description, last_editor, status) VALUES ( '%d', 'New Room', 'No Description Set', '%d', 'pending')",
					virt_nr, 
					 (ch->desc != NULL && ch->desc->acct != NULL) ? ch->desc->acct->get_id() : 0);
			
			mysql_safe_query (buf);
			
			if ( (virt_nr = mysql_insert_id(database)) < 1 )
			{
				ch->send_to_char ("There was a problem generating your selected room number. Try again later.\n");
				return;
			}
		}

	}
	
	troom = new_room (virt_nr);
	

	sprintf (buf, "Room %d has been initialized. Moving you to the new room now.\n", virt_nr);
	ch->send_to_char (buf);
	
	sprintf(buf, "goto %d", virt_nr);
	command_interpreter (ch, buf);
	
	return;
	
}


void
post_rdesc (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *ch;
	ROOM_DATA *room;

	ch = d->character;
	room = vtor (ch->delay_info1);
	ch->delay_info1 = 0;

	if (!d->pending_message->message)
	{
		ch->send_to_char ("No room description posted.\n");
		return;
	}

	room->description = duplicateString (d->pending_message->message);
	room->extra->weather_desc[WR_NORMAL] = duplicateString (room->description);
	
	std::stringstream tempstr;
	tempstr << reformat_desc(room->description);	
	room->description = duplicateString(tempstr.str().c_str());
	
	
	builder_log(ch, "room_desc", d->pending_message->message);
		
	free_mem(d->pending_message);
	d->pending_message = NULL;
}

void
room_desc (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	ROOM_DATA *room;

	room = ch->room;
	
	/** removing leadership commands for the moment
	if (!IS_SET (ch->affected_by, AFF_LEADER_COMMAND)
		&& ch->get_trust() < 1)
	{
		ch->send_to_char ("You do not have approval for leadership commands");
		return;
	}
	 ***/
	
	argument = one_argument (argument, buf);

	
	if (room->description)
	{
		ch->send_to_char ("The old description was: \n");
		ch->send_to_char (room->description);
	}

	ch->act("$n begins editing a room description.", false, 0, 0, TO_ROOM);

	free_mem(ch->desc->descStr);
	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;
	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;
	ch->delay_info1 = room->nVirtual;

	
		ch->send_to_char
			("\nPlease enter the new description; terminate with an '@'\n\n");
		ch->make_quiet();
	

	ch->desc->proc = post_rdesc;
}

void
post_redesc (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *ch;
	int iter;
	
	ch = d->character;	
	iter = ch->delay_info1;

	if (d)
	{
		if (!str_cmp(ch->room->ex_description[iter]->keyword, ch->delay_who))
			{
				ch->room->ex_description[iter]->description = duplicateString(d->descStr);
				ch->send_to_char ("Virtual object description installed\n");
				
				std::stringstream tempstr;
				tempstr << reformat_desc(ch->room->ex_description[iter]->description);	
				ch->room->ex_description[iter]->description = duplicateString(tempstr.str().c_str());
				
			
				
				builder_log(ch, "room_extra_desc", d->descStr);
				ch->delay_who = NULL;
				ch->delay_info1 = 0;
				return;
			}
		
	}
	else 
	{
		ch->send_to_char("There was an error saving the extended description. Please report this to a coder.\n");
		ch->delay_who = NULL;
		ch->delay_info1 = 0;
		return;
	}

}

	//Adds keywords and an extra description for something in the room. After the command with the keywords is given, you will be prompted for a description. This is useful for describing signs and anything else that need not be an object or will not be taken from the room.
void
room_redesc (CHAR_DATA * ch, char *argument)
{
	char *tmp = '\0';
	EXTRA_DESCR_DATA *newdesc;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int iter;
	
	if (!*argument)
	{
		ch->send_to_char ("No argument specified\n");
		return;
	}

	argument = one_argument(argument, buf);

	
	if (!str_cmp (argument, "remove"))
	{
		for (iter = 0; iter <= MAX_EX_DESCR; iter++)
		{
			if (!ch->room->ex_description[iter])
				continue;
			
			if (!str_cmp(ch->room->ex_description[iter]->keyword, buf))
			{
				ch->room->ex_description[iter] = 0;
				ch->send_to_char ("Virtual object description removed\n");
				return;
			}
		}
		
		return;
	}


	for (iter = 0; iter < MAX_EX_DESCR; iter++)
	{
		if (!ch->room->ex_description[iter])
		{
			newdesc = new EXTRA_DESCR_DATA;
			newdesc->keyword = duplicateString (buf);
			ch->room->ex_description[iter] = newdesc;
			break;
		}
		
		if ((!str_cmp(ch->room->ex_description[iter]->keyword, buf))
			&& (ch->room->ex_description[iter]->description))
		{
			tmp = strdup(ch->room->ex_description[iter]->description);
			break;	
		}
		
	}
	
	
	ch->send_to_char ("The old description was: \n\n");
	ch->send_to_char (tmp);


	free_mem(ch->desc->descStr);

	
		ch->send_to_char ("\nEnter a new description.  Terminate with an '@'\n");
		ch->make_quiet();
		ch->room->ex_description[iter]->description = NULL;
		ch->desc->max_str = 2000;
		ch->desc->proc = post_redesc;
		ch->delay_who = duplicateString (buf);
		ch->delay_info1 = iter;
	
}

void
room_terrain (CHAR_DATA * ch, char *argument)
{
	int flag, no = 1, i;
	char buf[256], buf2[512], buf3[512];


	one_argument (argument, buf);
	if (!strcmp (buf, ""))
	{
		sprintf (buf3, "Current terrain type: %s\n",
			terrain_types[vtor (ch->in_room)->terrain_type]);
		ch->send_to_char (buf3);
		return;
	}

	if (buf[0] == '?')
	{
		sprintf (buf2, "The following terrain types are available:\n\t");
		for (i = 0; *terrain_types[i] != '\n'; i++)
		{
			sprintf (buf2 + strlen (buf2), "%-10s ", terrain_types[i]);
			if (!(no % 4))
				(strcat (buf2, "\n\t"));
			no++;
		}
		strcat (buf2, "\n");
		ch->send_to_char (buf2);
		return;
	}


	flag = parse_argument (terrain_types, buf);

	ch->room->terrain_type = flag;

	ch->send_to_char ("terrain is set.\n");
}


void
rflags (CHAR_DATA * ch, char *argument)
{
	std::map<int, ROOM_DATA*>::iterator room_iterator;
	int flag;
	int no = 1;
	int i;
	char buf[256];
	char *buf2;
	char buf3[512];

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		buf2 = strdup(sprintbit (ch->room->room_flags, room_bits));
		sprintf (buf3, "Current room flags: %s", buf2);
		strcat (buf3, "\n");
		ch->send_to_char (buf3);
		return;
	}

	
	if (buf[0] == '?')
	{
		sprintf (buf2, "The following room flags are available:\n\t");
		for (i = 0; *room_bits[i] != '\n'; i++)
		{
			sprintf (buf2 + strlen (buf2), "%-10s ", room_bits[i]);
			if (!(no % 4))
				(strcat (buf2, "\n\t"));
			no++;
		}
		strcat (buf2, "\n");
		ch->send_to_char (buf2);
		return;
	}

	if ((flag = index_lookup (room_bits, buf)) == -1)
	{
		ch->send_to_char ("No such room flag.\n");
		return;
	}

	if ((((1 << flag) == OOC)) && ch->get_trust() < 5)
	{
		ch->send_to_char ("Only a level 5 or above can set the OOC bit.\n");
		return;
	}

	if ((1 << flag) == TEMPORARY)
	{
		ch->send_to_char ("This flag cannot be set manually.\n");
		return;
	}

	if (!IS_SET (ch->room->room_flags, (1 << flag)))
		ch->room->room_flags |= (1 << flag);
	else
		ch->room->room_flags &= ~(1 << flag);

	ch->send_to_char ("Flag (re)set.\n");
}

void
mobile_new (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int virt_nr;
	CHAR_DATA *newmob;
	MYSQL_RES *result;

	extern int mob_start_stat;

	if (IS_NPC (ch))
	{
		ch->send_to_char ("This is a PC only command.\n");
		return;
	}

		
	if (ch->pc->edit_mob && *argument != '!')
	{
		sprintf(buf, "You are still editing mobile %d.\n Use 'mset save' to save your work. Otherwise, use #3'mset new !'#0, without the quotes, to discard old changes and start on a new mobile.\n", ch->pc->edit_mob);
		ch->send_to_char (buf);
		return;
	}
	
	
	
	
		sprintf(buf, "INSERT INTO proto_mobiles (last_editor, approved) VALUES ('%s', %d)", ch->keywords, BUILD_APPROVED);
		
		mysql_safe_query (buf);
		
		if ((result = mysql_store_result(database)) == 0 &&
			mysql_field_count(database) == 0 &&
			mysql_insert_id(database) != 0)
		{
			virt_nr = mysql_insert_id(database);
		}
		
		if (virt_nr == '0' || !virt_nr)
		{
			ch->send_to_char ("There was a problem generating a mobile number. Try again later.\n");
			return;
		}
	
	
	
	

	newmob = new_char (0);	/* MOB */


	newmob->mob->nVirtual = virt_nr;
	newmob->mob->zone = 0;

	ch->pc->edit_mob = virt_nr;

	newmob->mob->action = 0;
	newmob->mob->action |= ACT_ISNPC;

	newmob->keywords = duplicateString ("mob new");
	newmob->short_descr = duplicateString ("a new mobile");

	newmob->speaks = strdup("Westron");
	newmob->skill_map["Westron"] = 100;
	newmob->max_hit = 80;
	newmob->hit = 80;
	newmob->max_move = 80;
	newmob->move = 80;
	newmob->armor = 0;
	newmob->mob->damroll = 0;

	newmob->str = mob_start_stat;
	newmob->dex = mob_start_stat;
	newmob->intel = mob_start_stat;
	newmob->wil = mob_start_stat;
	newmob->aur = 1; //changed for POWER
	newmob->con = mob_start_stat;
	newmob->agi = mob_start_stat;
	newmob->luk = mob_start_stat;
	newmob->tmp_str = mob_start_stat;
	newmob->tmp_dex = mob_start_stat;
	newmob->tmp_intel = mob_start_stat;
	newmob->tmp_wil = mob_start_stat;
	newmob->tmp_aur = 1;//changed for POWER
	newmob->tmp_con = mob_start_stat;
	newmob->tmp_agi = mob_start_stat;
	newmob->tmp_luk = mob_start_stat;
	newmob->mob->damnodice = 1;
	newmob->mob->damsizedice = 2;
	newmob->intoxication = 0;
	newmob->hunger = -1;
	newmob->thirst = -1;
	newmob->equip = NULL;

	
		//everyone, PC and NPC uses these skills almost daily
	open_skill (newmob, lookup_skill_id("Search"));
	open_skill (newmob, lookup_skill_id("Listen"));
	open_skill (newmob, lookup_skill_id("Scan"));
	open_skill (newmob, lookup_skill_id("Dodge"));
	open_skill (newmob, lookup_skill_id("Parry"));
	open_skill (newmob, lookup_skill_id("Block"));
	open_skill (newmob, lookup_skill_id("Brawling"));
	open_skill (newmob, lookup_skill_id("Climb"));
	open_skill (newmob, lookup_skill_id("Dodge"));
	open_skill (newmob, lookup_skill_id("Barter"));
	open_skill (newmob, lookup_skill_id("Tracking"));
	
	
	newmob->speed = SPEED_WALK;


	proto_mob_map[newmob->mob->nVirtual] = newmob;

	ch->act("$n creates a new mobile.\n", true, 0, 0, TO_ROOM);
	sprintf (buf, "Mobile %d has been initialized.\n", virt_nr);
	ch->send_to_char (buf);
}

void
object_new (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_STRING_LENGTH]= { '\0' };
	char arg2[MAX_STRING_LENGTH]= { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char temp_buf[MAX_STRING_LENGTH]= { '\0' };
	int virt_nr;
	int type;
	int num = 1;
	OBJ_DATA *newobj;
	MYSQL_RES *result;
	std::string material;
	
	argument = one_argument(argument, arg);
	argument = one_argument(argument, arg2);
	
		//arg = 'new'
		//arg2 = type
	
	if (ch->pc->edit_obj != 0 && *argument != '!')
	{
		sprintf(buf, "You are still editing object %d.\n Use #3oset save#0 to save your work. Otherwise, use #3oset new %s !#0, to discard old changes and start on a different object.\n", ch->pc->edit_obj, arg2);
		ch->send_to_char (buf);
		return;
		
	}
	
	if (!*arg2)
	{
		ch->send_to_char ("You must supply the object type.\n");
		return;
	}
	
	
	if (arg2[0] == '?')
	{
		sprintf (buf, "The following item types are available:\n");
		for (type = 0; *item_types[type] != '\n'; type++)
		{
			sprintf (buf + strlen (buf), "%-20s", item_types[type]);
			if (!(num % 4))
				strcat (buf, "\n");
			num++;
		}
		strcat (buf, "\n");
		ch->send_to_char (buf);
		return;
	}
	
	type = parse_argument (item_types, arg2);
	if (type == -1)
	{
		ch->send_to_char ("Not a valid type.\n");
		return;
	}
	
	
	sprintf(buf, "INSERT INTO proto_objects (last_editor, status) VALUES ('%s', %d)", ch->keywords, BUILD_APPROVED);
	
	mysql_safe_query (buf);
	
	if ((result = mysql_store_result(database)) == 0 &&
		mysql_field_count(database) == 0 &&
		mysql_insert_id(database) != 0)
	{
		virt_nr = mysql_insert_id(database);
	}
	
	if (virt_nr == '0' || !virt_nr)
	{
		ch->send_to_char ("There was a problem generating an object number. Try again later.\n");
		return;
	}
	
	newobj = new_object ();
	
	clear_object (newobj);
	
	newobj->nVirtual = virt_nr;
	newobj->zone = 0;
	
	newobj->full_description = 0;
	newobj->contains = 0;
	newobj->in_room = NOWHERE;
	newobj->in_obj = 0;
	newobj->next_content = 0;
	newobj->carried_by = 0;
	newobj->full_description = duplicateString ("");
	newobj->obj_flags.type_flag = index_lookup (item_types, arg2);
	newobj->quality = 30;  //corresponds to familar skill of NPC
	newobj->item_wear = 100;
	newobj->materials[0] = "none";
	

		//if the material listed below is not in the DB, it will be ignored
	switch (type)
	{
		case ITEM_FLUID:
			newobj->name = duplicateString ("water");
			newobj->short_description = duplicateString ("filled with water");
			newobj->description = duplicateString ("");
			newobj->obj_flags.weight = 100;
			newobj->materials[0] = "liquid";
			break;
			
		case ITEM_FUEL:
			newobj->name = duplicateString ("oil");
			newobj->short_description = duplicateString ("filled with oil");
			newobj->description = duplicateString ("");
			newobj->materials[0] = "liquid";
			newobj->obj_flags.weight = 100;
			break;
			
			
		case ITEM_LIGHT:
			newobj->name = duplicateString ("lantern");
			newobj->short_description = duplicateString ("a brass lantern");
			newobj->description = duplicateString ("an old dented lantern");
			newobj->obj_flags.wear_flags |= ITEM_TAKE;
			newobj->o.od.value[2] = 20;
			newobj->obj_flags.weight = 500;
			break;
			
					
		case ITEM_KEY:
			newobj->name = duplicateString ("key");
			newobj->short_description = duplicateString ("a key");
			newobj->description = duplicateString ("a small silver key");
			newobj->obj_flags.wear_flags |= ITEM_TAKE;
			newobj->materials[0] =  "metal";
			newobj->o.od.value[0] = 0;
			newobj->obj_flags.weight = 10;
			break;
			
		case ITEM_DRINKCON:
			newobj->name = duplicateString ("skin");
			newobj->short_description = duplicateString ("a water skin");
			newobj->description = duplicateString ("a leaky skin lies here.");
			newobj->obj_flags.wear_flags |= ITEM_TAKE;
			newobj->materials[0] =  "leather";
			newobj->o.od.value[0] = 10;
			newobj->o.od.value[1] = 0;
			newobj->o.od.value[2] = 0;
			newobj->o.od.value[3] = 0;
			newobj->obj_flags.weight = 200;
			break;
		
		case ITEM_DRYCON:
			newobj->name = duplicateString ("barrel");
			newobj->short_description = duplicateString ("a wooden barrel");
			newobj->description = duplicateString ("A wooden barrel sits here.");
			newobj->obj_flags.wear_flags |= ITEM_TAKE;
			newobj->materials[0] =  "wood";
			newobj->o.od.value[0] = 0;
			newobj->o.od.value[1] = 0;
			newobj->o.od.value[2] = 0;
			newobj->o.od.value[3] = 0;
			newobj->obj_flags.weight = 1000;
			break;
			
		case ITEM_FOOD:
			newobj->name = duplicateString ("bread");
			newobj->short_description = duplicateString ("a loaf of bread");
			newobj->description = duplicateString ("a loaf of bread");
			newobj->obj_flags.wear_flags |= ITEM_TAKE;
			newobj->materials[0] =  "meat";
			newobj->o.od.value[0] = 6;
			newobj->o.od.value[3] = 0;
			newobj->obj_flags.weight = 300;
			break;
			
		case ITEM_CONTAINER:
			newobj->name = duplicateString ("bag");
			newobj->short_description = duplicateString ("a rather large bag");
			newobj->description = duplicateString ("a large brown bag lies here");
			newobj->obj_flags.wear_flags |= ITEM_TAKE;
			newobj->materials[0] = "cloth";
			newobj->o.od.value[0] = 100;
			newobj->o.od.value[1] = 0;
			newobj->o.od.value[2] = -1;
			newobj->obj_flags.weight = 300;
			break;
			
		case ITEM_BOARD:
			newobj->name = duplicateString ("board");
			newobj->short_description = duplicateString ("a board");
			newobj->description = duplicateString ("a bulletin board is standing here.");
			newobj->o.od.value[2] = 1;
			newobj->obj_flags.weight = 100;
			break;
			
		case ITEM_USURY_NOTE:
			newobj->name = duplicateString ("note usury");
			newobj->short_description = duplicateString ("a usury note");
			newobj->description = duplicateString ("A usury note lies on the floor.");
			newobj->obj_flags.wear_flags |= ITEM_TAKE;
			newobj->materials[0] =  "parchement";
			newobj->o.od.value[0] = 1;
			newobj->o.od.value[1] = 1;
			newobj->o.od.value[2] = 20;
			newobj->o.od.value[3] = 40;
			newobj->obj_flags.weight = 100;
			break;
			
		
			
		case ITEM_KEYRING:
			newobj->name = duplicateString ("keyring sturdy iron");
			newobj->short_description = duplicateString ("a sturdy iron keyring");
			newobj->description =
			duplicateString ("A sturdy iron keyring lies here, forgotten.");
			newobj->obj_flags.wear_flags |= ITEM_TAKE;
			newobj->obj_flags.wear_flags |= ITEM_WEAR_BELT;
			newobj->materials[0] = "metal";
			newobj->o.od.value[0] = 10;
			newobj->obj_flags.weight = 50;
			break;
			
		
			
		case ITEM_INK:
			newobj->name = duplicateString ("ink black pot ceramic small");
			newobj->short_description =
			duplicateString ("a small ceramic pot of black ink");
			newobj->description =
			duplicateString ("A small ceramic pot has been left here.");
			newobj->ink_color = duplicateString ("black ink");
			newobj->obj_flags.wear_flags |= ITEM_TAKE;
			newobj->materials[0] = "glass";
			newobj->o.od.value[0] = 10;
			newobj->o.od.value[1] = 10;
			newobj->obj_flags.weight = 200;
			break;
			
		case ITEM_BOOK:
			newobj->name = duplicateString ("tome leather-bound small");
			newobj->short_description = duplicateString ("a small, leather-bound tome");
			newobj->description = duplicateString ("A small tome has been left here.");
			newobj->obj_flags.wear_flags |= ITEM_TAKE;
			newobj->materials[0] = "parchment";
			newobj->o.od.value[0] = 25;
			newobj->obj_flags.weight = 500;
			break;
			
		case ITEM_PARCHMENT:
			newobj->name = duplicateString ("paper parchment sheet");
			newobj->short_description = duplicateString ("a sheet of parchment");
			newobj->description =
			duplicateString ("A sheet of parchment has been discarded here.");
			newobj->obj_flags.wear_flags |= ITEM_TAKE;
			newobj->materials[0] = "parchement";
			newobj->o.od.value[0] = 0;
			newobj->obj_flags.weight = 10;
			break;
			
		case ITEM_WRITING_INST:
			newobj->name = duplicateString ("quill writing");
			newobj->short_description = duplicateString ("a writing quill");
			newobj->description =
			duplicateString ("A writing quill has been discarded here.");
			newobj->obj_flags.wear_flags |= ITEM_TAKE;
			newobj->o.od.value[0] = 0;
			newobj->obj_flags.weight = 25;
			break;
			
		case ITEM_HEALER_KIT:
			newobj->name = duplicateString ("kit healer");
			newobj->short_description = duplicateString ("a healer's kit");
			newobj->description = duplicateString ("A healer's kit lies on the floor.");
			newobj->obj_flags.wear_flags |= ITEM_TAKE;
			newobj->o.od.value[0] = 0;
			newobj->obj_flags.weight = 500;
			break;
			
		case ITEM_REPAIR_KIT:
			newobj->name = duplicateString ("kit repair mending");
			newobj->short_description = duplicateString ("a repair kit");
			newobj->description = duplicateString ("A repair kit lies on the floor.");
			newobj->obj_flags.wear_flags |= (ITEM_TAKE | ITEM_WEAR_BELT);
			newobj->o.od.value[0] = 0; //uses remaining
			newobj->o.od.value[4] = 1; // repairs miniscule or unnoticeable damage
			newobj->obj_flags.weight = 500;
			break;
			
					
		default:
			newobj->name = duplicateString ("thing generic");
			newobj->short_description = duplicateString ("a generic thing");
			newobj->description =
			duplicateString ("Some careless admin has left a generic thing here.");
			newobj->obj_flags.wear_flags |= ITEM_TAKE;
			newobj->o.od.value[0] = 0;
			newobj->obj_flags.weight = 100;
			break;
	}
	
	newobj->max_hit_points = (get_material_hardness(newobj) * (newobj->obj_flags.weight)/100.0);
	if (newobj->max_hit_points > 0 && newobj->max_hit_points < 1)
		newobj->max_hit_points = 1;
	
	if (IS_SET (newobj->obj_flags.extra_flags, ITEM_MAGIC))
		newobj->obj_flags.extra_flags &= ~ITEM_MAGIC;
	
	if (IS_SET (newobj->obj_flags.extra_flags, ITEM_INVISIBLE))
		newobj->obj_flags.extra_flags &= ~ITEM_INVISIBLE;
	
	if (ch->get_trust() < 5)
		newobj->obj_flags.extra_flags |= index_lookup (extra_bits, "ok");
	
	proto_obj_map[newobj->nVirtual] = newobj;
	sprintf(temp_buf, "%d %s (%s) has been initiated.\n", virt_nr, newobj->name, item_types[type]);
	ch->send_to_char(temp_buf);
	
	ch->pc->edit_obj = virt_nr;
	
	return;
	
}

int
index_lookup (const char* const* index, const char* const lookup)
{
	unsigned int i;
	char* temp_look;
	
		//runs through the list with the original argument. 
		//if it succeeds, it returned the value
	for (i = 0; *index[i] != '\n'; i++)
		if (!strn_cmp (index[i], lookup, strlen (lookup)))
			return i;
	
		//we have not succeed yet, so runs through the list with the modified argument. 
		//this only applies to abbreviated directions
	
	temp_look = strdup(convert_dir(strdup(lookup)));
	for (i = 0; *index[i] != '\n'; i++)
		if (!strn_cmp (index[i], temp_look, strlen (temp_look)))
			return i;
	
		//still didn't find it, so return with error value
	return -1;
}

void
do_tags (CHAR_DATA * ch, char *argument, int cmd)
{
	char tag_name[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int name_no;
	int i;
	std::map<int, OBJECT_MATERIAL*>::iterator it_material;
	
	argument = one_argument (argument, tag_name);

	if (!*tag_name)
	{
		for (name_no = 0; *constant_info[name_no].constant_name; name_no++)
		{
			sprintf (buf, "     %-20s    %s\n",
				constant_info[name_no].constant_name,
				constant_info[name_no].description);
			ch->send_to_char (buf);
		}
		
		ch->send_to_char("     materials               Object Materials\n");
		
		
		return;
	}

	for (name_no = 0; *constant_info[name_no].constant_name; name_no++)
		if (!strn_cmp (constant_info[name_no].constant_name, tag_name,
			strlen (tag_name)))
			break;

	if (!str_cmp (tag_name, "materials"))
	{
		sprintf(buf, "   ");
		for (it_material = object_material_map.begin(), i=0; it_material != object_material_map.end(); it_material++, i++)
		{
			sprintf (buf + strlen (buf), "%-23s ", it_material->second->material_name);
			if (!((i + 1) % 3))
				strcat (buf, "\n   ");
			
		}
		if (!((i + 1) % 3) || ((i + 1) % 3) == 2)
			strcat (buf, "\n");
		
		ch->send_to_char (buf);
		return;
	}
	
	if (!*constant_info[name_no].constant_name)
	{
		ch->send_to_char ("No such tag name, type 'tags' to get a "
			"listing.\n");
		return;
	}

	strcpy (buf, "   ");

	for (i = 0; *(char *) constant_info[name_no].index[i] != '\n'; i++)
	{
		sprintf (buf + strlen (buf), "%-23s ",
			(char *) constant_info[name_no].index[i]);
		if (!((i + 1) % 3))
			strcat (buf, "\n   ");
	}

	if (!((i + 1) % 3) || ((i + 1) % 3) == 2)
		strcat (buf, "\n");

	ch->send_to_char (buf);
}


int
redefine_objects (OBJ_DATA * proto)
{
	int change_count = 0;
	OBJ_DATA *obj;
	std::list<obj_data*>::iterator tobj_iterator;
	
	for (tobj_iterator = object_list.begin(); tobj_iterator != object_list.end(); tobj_iterator++)
	{
		obj = *tobj_iterator;

		if (obj->deleted || (obj->nVirtual != proto->nVirtual))
			continue;

		change_count++;

		obj->partial_deep_copy(proto);
	}

	return change_count;
}

int
redefine_mobiles (CHAR_DATA * proto)
{
	int change_count = 0;
	CHAR_DATA *mob;
	std::list<CHAR_DATA*>::iterator tch_iterator;

	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		mob = *tch_iterator;

		if (mob->deleted)
			continue;
		if (!IS_NPC (mob))
			continue;
		if (mob->mob->nVirtual != proto->mob->nVirtual)
			continue;

		change_count++;

		mob->partial_deep_copy(proto);
	}

	return change_count;
}

void
give_oset_help (CHAR_DATA * ch)
{
	page_string (ch->desc, 
		"Syntax:\n"
		"     oset [vnum]\n"
		"     oset new <item type>\n"
		"     oset delete \n"
		"     oset save \n"
		"     oset name \"<keywords>\" \n"
		"     oset sdesc \"<short description>\"\n"
		"     oset ldesc \"<long description>\"\n"
		"     oset desc\n"
		"  *This will drop you into an editor for the actual description.\n"
		"\n"
		"     oset delete\n"
		"     oset save\n"		 
		"\n"
		"     oset type <item-type>           See:  tags item-types\n"
		"     oset wear <wear-bits>           See:  tags wear-bits\n"
		"     oset extra <extra-bits>         See:  tags extra-bits\n"
		"     oset <econ number> <econ-flags> See:  tags econ-flags\n"
		"\n"
		"     oset weight <value>             #.##\n"
		"     oset cost <value>\n"
		"     oset size <value>               1 - XXsmall to 7 - XXlarge\n"
		"     oset quality <value> \n"
		"     oset oval# <value> \n"
		"     oset dam ##d# \n"
		"     oset clock <month day hour> \n"
		"     oset morphto <objnum> \n"
		"     oset timer <hours> \n"
		"     oset color <color>              For inks only. \n"
		"     oset material <material type>\n"
		"\n"
		"     oset mkeys \"<key words>\"\n"
		"         For masked (concealing) items only or tossable items (dice) - Keywords to use when mask or helm is worn or the dice is viewed. \n\n"
		"     oset treats <damage>\n"
		"         For healing kits - all, blunt, puncture, slash, burn, frost, or bleed. Can also be set with \"oset oval5 <damage>\"\n\n"
		"     oset mends <item type>\n"
		"         Mending items of that type (see tags item-types)\n" 
		"     oset mends all                  Will repair all types of items\n"
		"     oset mends none                 Will mend no items\n"
		"\n"
		"     oset mends <skill name>         Skill needed to use this kit\n"
		"     oset mends noskill              Requires no skill to use\n"
		"\n"
		"     oset mends <damage type>        Items with this level of damage or less\n"
		"                                    (see tags damage-severity)\n"
		
		
		"\n"
		"     oset clanadd <clan name> <clan rank>\n"
		"     oset clanremove\n"

		);
}

void
post_odesc (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *ch;
	OBJ_DATA *obj;

	ch = d->character;
	obj = vtoo (ch->delay_info1);
	ch->delay_info1 = 0;

	if (!d->pending_message->message)
	{
		ch->send_to_char ("No object description posted.\n");
		return;
	}

	obj->full_description = duplicateString (d->pending_message->message);

	std::stringstream tempstr;
	tempstr << reformat_desc(obj->full_description);	
	obj->full_description = duplicateString(tempstr.str().c_str());
	

	builder_log(ch, "odesc", d->pending_message->message);

	
	ch->delay_info2 = 0;

	d->pending_message = NULL;
	return;
}

	//oset - (not editing an object) not editing message
	//oset - (editing an object) ostat the object
	//oset ? - displays help message
	//oset ## - (not editing an object) starts editing
	//oset ## - (editing an object) gives warning
	//oset ## ! - (editing an object) dumps old object and starts new one
	//oset <options> - does something special, depending on <option>
void
do_oset (CHAR_DATA * ch, char *argument, int cmd)
{
	char *tmp_arg;
	OBJ_DATA *edit_obj;
	OBJ_DATA *tmp_obj;
	int ind;
	int i;
	int weight_int;
	int weight_dec;
	int bonus;
	int parms;
	int sides;
	int dice;
	int mend_type;
	int new_cflags;
	int full_description = 0, indoor_description = 0;
	char subcmd[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char *buf2;
	char clan_name[MAX_STRING_LENGTH]= { '\0' };
	char clan_rank[MAX_STRING_LENGTH]= { '\0' };
	
		
	builder_log(ch, "oset", argument);
	
	if (IS_NPC (ch))
	{
		ch->send_to_char ("Only PC can use this command.\n");
		return;
	}
	
	buf2 = duplicateString(argument); //duplicate of cull command for warning message
	
	argument = one_argument (argument, subcmd); 
	
	
	
	
		//"oset" (by itself) - displays current object
	if (!*subcmd) 
	{
		if (ch->pc->edit_obj && vtoo (ch->pc->edit_obj))
		{
			sprintf (buf, "object %d", ch->pc->edit_obj);
			do_stat (ch, buf, 0);
		}
		else
			ch->send_to_char ("You're not editing an object.\n");
		return;
	}
	
		//"oset ?"
	if (*subcmd == '?') 
	{
		give_oset_help (ch);
		return;
	}
	
		//is it a new object?
	else if (!str_cmp (subcmd, "new"))
	{
		object_new(ch, buf2); 
		return;
	}
	
		//Deciding what object we are editing
		//subcmd is a number of an existing object
		//'oset ###'
		//'oset ### !'
	if (atoi (subcmd) != 0 && (edit_obj = vtoo (atoi (subcmd))))
	{
		if (ch->pc->edit_obj == 0)
		{
			ch->pc->edit_obj = edit_obj->nVirtual;
			sprintf (buf, "You are now editing object %d.\n", edit_obj->nVirtual);
			ch->send_to_char (buf);
			return;
		}
		else if (ch->pc->edit_obj != 0 && *argument != '!')
		{
			sprintf(buf, "You are still editing object %d.\n Use #3oset save#0 to save your work. Otherwise, use #3oset %s !#0, to discard old changes and start on a different object.\n", ch->pc->edit_obj, subcmd);
			ch->send_to_char (buf);
			return;
			
		}
		else if ((ch->pc->edit_obj != 0) && (*argument == '!'))
		{
			ch->pc->edit_obj = edit_obj->nVirtual;
			sprintf(buf, "You are now editing object %d\n",ch->pc->edit_obj);
			ch->send_to_char (buf);
			return;
		}
	}
	
		//or is the name of an existing object
		//'oset pouch'
	else if ((edit_obj = get_obj_in_list_vis (ch, subcmd, ch->right_hand)) ||
			 (edit_obj = get_obj_in_list_vis (ch, subcmd, ch->left_hand)) ||
			 (edit_obj = get_obj_in_list_vis (ch, subcmd, ch->room->contents)))
			//the assignemnet of edit_obj is in the logic test
	{
		if (ch->pc->edit_obj == 0)
		{
			ch->pc->edit_obj = edit_obj->nVirtual;
			sprintf (buf, "You are now editing object %d.\n", edit_obj->nVirtual);
			ch->send_to_char (buf);
			return;
		}
		else if (ch->pc->edit_obj != 0 && *argument != '!')
		{
			sprintf(buf, "You are still editing object %d.\n Use #3oset save#0 to save your work. Otherwise, use #3oset %s !#0, to discard old changes and start on a different object.\n", ch->pc->edit_obj, subcmd);
			ch->send_to_char (buf);
			return;
			
		}
		else if ((ch->pc->edit_obj != 0) && (*argument == '!'))
		{
			ch->pc->edit_obj = edit_obj->nVirtual;
			sprintf(buf, "You are now editing object %d\n",ch->pc->edit_obj);
			ch->send_to_char (buf);
			return;
		}
	}
	
		//subcmd isn't a number or name, so it must be a command
	
		//make certain we are editing an object
	if (ch->pc->edit_obj)
	{
		edit_obj = vtoo (ch->pc->edit_obj);
	}
	else
	{
		ch->send_to_char ("You aren't editing anything!\n");
		ch->pc->edit_obj = 0;
		return;
	}
	
		//Some objects cannot be edited
	if ((edit_obj->nVirtual == VNUM_TICKET)
		|| (edit_obj->nVirtual == VNUM_ORDER_TICKET))
	{
		ch->send_to_char ("You cannot make changes to this item.\n");
		ch->pc->edit_obj = 0;
		return;
	}
	
		//they are editing an object
		//subcmd is - <cmd> <options>
	
	
	if (ch->get_trust() < 5)
		edit_obj->obj_flags.extra_flags |= ITEM_OK;
	
	
		//save
	if (!str_cmp (subcmd, "save"))
	{
		ch->send_to_char ("The changes you made to this object have been saved to the database.\n");
		
		save_mysql_object(ch, edit_obj, BUILD_APPROVED); 
		proto_obj_map[edit_obj->nVirtual] = edit_obj;
		
		if (ch->pc->edit_obj)
		{
			ch->pc->edit_obj = 0;
		}
		
		return;
		
	}
	
		//item types
	else if (!str_cmp (subcmd, "type"))
	{
		argument = one_argument (argument, buf);
		obj_type(ch, buf);
		return;
	}
	
		//wear bits
	else if (!str_cmp (subcmd, "wear"))
	{
		obj_wear(ch, argument);
		return;
		
	}
	
		//timer to decay
	else if (!str_cmp (subcmd, "timer"))
	{
		obj_timer(ch, argument);
		return;
	}
	
		//extra bits
	
	else if (!str_cmp (subcmd, "extra"))
	{
		obj_extra(ch, argument);
		return;
	}
	
		//affects
	else if (!str_cmp (subcmd, "affect"))
	{
		obj_affect(ch, argument);
		return;
	}
	
		//delete
	else if (ch->get_trust() >= 4 && !str_cmp (subcmd, "delete"))
	{
		obj_delete(ch, argument);
		return;
	}
			
		//material
	else if (!str_cmp (subcmd, "material"))
	{
		
		obj_material (ch, argument);
		return;
	}
		
	
		//Oval 0 -  Uses Remaining
		//Oval 1 -  Mending Bonus
		//Oval 2 -  Required Skill Level
		//Oval 3 -  Skill needed to use kit
		//Oval 4 -  Max Severity level
		//Oval 5 -  Item Type
	
		//MENDS
	else if (!str_cmp (subcmd, "mends"))
	{
		if (edit_obj->obj_flags.type_flag != ITEM_REPAIR_KIT)
		{
			ch->send_to_char ("That item is not a repair kit.\n");
			return;
		}
		if (!str_cmp (argument, "noskill"))
		{
			edit_obj->o.od.value[3] = 0;
			ch->send_to_char ("This repair kit requires no skill to use.\n");
			return;
		}
		else if (!str_cmp (argument, "all-types"))
		{
			edit_obj->o.od.value[5] = 0;
			ch->send_to_char
			("This repair kit will repair all types of item.\n");
			return;
		}
		else if (!str_cmp (argument, "none"))
		{
			edit_obj->o.od.value[3] = -1;
			edit_obj->o.od.value[5] = -1;
			ch->send_to_char
			("This repair kit will NOT mend ANY items!\n");
			return;
		}
		
			//requires a skill to use
		else if ((mend_type = lookup_skill_id(argument)) > 0)
		{
			edit_obj->o.od.value[3] = mend_type;
			sprintf (buf, "A repairman now needs the '%s' skill to repair items with this kit.\n", argument);
			ch->send_to_char (buf);
			
			return;
			
		}
		
			//will repair a single type of item
		else if ((mend_type = parse_argument (item_types, argument)) > 0)
		{
			edit_obj->o.od.value[5] = mend_type;
			sprintf (buf, "This repair kit will now mend items of type '%s'.\n",
					 argument);
			ch->send_to_char (buf);
			
			return;
		}
		
			//limt to damage severity
		else if ((mend_type = index_lookup (damage_severity, argument)) > 0)
		{
			if (mend_type >= 8)
			{
				edit_obj->o.od.value[4] = 8;
				ch->send_to_char
				("This repair kit will mend any level of damage.\n");
			}
			else
			{
				edit_obj->o.od.value[4] = mend_type;
				sprintf (buf,
						 "This repair kit will mend only items of '%s' damage or less.\n",
						 damage_severity[mend_type]);
				ch->send_to_char (buf);
			}
			return;
		}
		
		ch->send_to_char
		("\n"
		 "oset mends <item type>   - mending items of that type\n"
		 "                           (see tags item-types)\n" 
		 "oset mends all-types     - will repair all types of items\n"
		 "oset mends none          - will mend no items at all\n"
		 "\n"
		 "oset mends <skill name>  - skill needed to use this kit\n"
		 "oset mends noskill       - requires no skill to use\n"
		 "\n"
		 "oset mends <damage type> - items with this level of damage or less\n"
		 "                           (see tags damage-severity)\n" 
		 "\n");
		return;
	}
	
		//ink color
	else if (!str_cmp (subcmd, "ink-color"))
	{
		
		if (edit_obj->obj_flags.type_flag != ITEM_INK)
		{
			ch->send_to_char ("This object is not an ink!\n");
			return;
		}
		else
		{
			edit_obj->ink_color = duplicateString (argument);
			sprintf(buf, "Ink color string set to %s.\n", edit_obj->ink_color);
			ch->send_to_char (buf);
		}
		
		if ((i = redefine_objects (edit_obj)) > 0)
		{
			sprintf (buf, "%d objects in world redefined.\n", i);
			ch->send_to_char (buf);
		}
		return;
	}
	
		//weight
	else if (!str_cmp (subcmd, "weight"))
	{
		
		argument = one_argument (argument, buf);
		
		i = sscanf (buf, "%d.%d", &weight_int, &weight_dec);
		
		if (i == 0)
		{
			ch->send_to_char ("Expected weight value.\n");
			return;
		}
		
		if (i == 2 && (weight_dec < 0 || weight_dec >= 99))
		{
			ch->send_to_char ("Decimal portion of weight not in 0..99\n");
			return;
		}
		
		if (i == 1)
			edit_obj->obj_flags.weight = weight_int * 100;
		else
			edit_obj->obj_flags.weight = weight_int * 100 + weight_dec;
		
			//we changed weight, so we need to re-calcualte the hit points
		
		edit_obj->max_hit_points = (get_material_hardness(edit_obj) * (edit_obj->obj_flags.weight)/100.0);
		if (edit_obj->max_hit_points > 0 && edit_obj->max_hit_points < 1)
			edit_obj->max_hit_points = 1;
		
		
		
		ch->send_to_char ("Weight value set.\n");
	}
	
		//cost - base value in coppers
	else if (!str_cmp (subcmd, "cost"))
	{
		
		argument = one_argument (argument, buf);
		
		if (!isdigit (*buf))
		{
			ch->send_to_char ("Expected a cost value.\n");
			return;
		}
		
		if (atoi (buf) < 0)
		{
			ch->send_to_char ("Must be a number greater than -1.\n");
			return;
		}
		
		sscanf (buf, "%f", &edit_obj->coppers);
		
		ch->send_to_char ("Cost value set.\n");
		
	}
	
		//size 
	else if (!str_cmp (subcmd, "size"))
	{
		
		argument = one_argument (argument, buf);
		
		if (!isdigit (*buf))
		{
			ch->send_to_char ("Expected a size.\n");
			ch->send_to_char ("1 - XX-Small      2 - X-Small    3 - Small\n");
			ch->send_to_char ("4 - Medium        5 - Large     6 - X-large.\n");
			ch->send_to_char ("7 - XX-Large.\n");
			return;
		}
		
		edit_obj->size = atoi (buf);
		ch->send_to_char ("Size set.\n");
	}
	
		//quality
	else if (!str_cmp (subcmd, "quality"))
	{
		
		argument = one_argument (argument, buf);
		
		if (!isdigit (*buf))
		{
			ch->send_to_char ("Expected quality value. 30 is the default for NPC quality goods.\n");
			return;
		}
		
		edit_obj->quality = atoi (buf);
		ch->send_to_char ("Quality set.\n");
	}
	
		//name which includes keywords
	else if (!str_cmp (subcmd, "name")
			 || !str_cmp (subcmd, "keys")
			 || !str_cmp (subcmd, "key"))
	{
		
		
		if (!*argument)
		{
			ch->send_to_char ("What, no name?\n");
			return;
		}
		
		
		edit_obj->name = duplicateString (argument);
		ch->send_to_char ("Name set. \n");
	}
	
		//description - drops into editor at end of function
	else if (!str_cmp (subcmd, "desc") 
			 || !str_cmp (subcmd, "descr")
			 || !str_cmp (subcmd, "description"))
	{
		full_description = 1;
	}
	
		//sdesc
	else if (!str_cmp (subcmd, "sdesc"))
	{
		
		if (!*argument)
		{
			ch->send_to_char ("What, no short description?\n");
			return;
		}
		
		edit_obj->short_description = duplicateString (argument);
		ch->send_to_char ("sdesc set. \n");
	}
	
		//ldesc
	else if (!str_cmp (subcmd, "ldesc"))
	{
		
		if (!*argument)
		{
			ch->send_to_char ("What, no long description?\n");
			return;
		}
		
		edit_obj->description = duplicateString (argument);
		ch->send_to_char ("Ldesc set. \n");
	}
	

		//oval0  - parchement, NPC_object and dry_containers types restricted
	else if (!str_cmp (subcmd, "oval") || !str_cmp (subcmd, "oval0"))
	{
		if (edit_obj->obj_flags.type_flag == ITEM_PARCHMENT)
		{
			ch->send_to_char ("This oval cannot be edited.\n");
			return;
		}
		
		if (edit_obj->obj_flags.type_flag == ITEM_DRYCON)
		{
			argument = one_argument (argument, buf);
			
			OBJ_DATA* tmp_obj = vtoo(edit_obj->o.od.value[2]);
			if (!tmp_obj)
			{
				ch->send_to_char ("You need to set the Contents (oval2) first\n");
				return;
			}
			
			edit_obj->o.od.value[0] = atoi(buf);
			
				//adjust contains to match Capacity
			if (edit_obj->o.od.value[1] > edit_obj->o.od.value[0])
				edit_obj->o.od.value[1] = edit_obj->o.od.value[0];
			
				//adjust max_weight to fit capacity
			int tmp_weight = edit_obj->o.od.value[0]*tmp_obj->obj_flags.weight;
			
			if (edit_obj->o.od.value[3] < tmp_weight)
				edit_obj->o.od.value[3] = tmp_weight;
			
			ch->send_to_char ("Capacity, contains and max weight adjusted as needed.\n");
			return;
			
		}
		
		for (i = 0; i <= LAST_DIR; i++)
		{
			tmp_arg = one_argument (argument, buf);
			int new_value = strtol (buf,0,10);
			
			if (*buf == '0' || new_value)
			{
				argument = tmp_arg;
				if (!i && edit_obj->obj_flags.type_flag == ITEM_NPC_OBJECT)
				{
					if (new_value && !vtom(new_value)) {
						ch->send_to_char ("You must create that mobile first.\n");
						return;
					}
				}
				edit_obj->o.od.value[i] = new_value;
			}
			else if (*buf != '-')
				return;
		}
	}
	
		//oval1 - book, key types, and dry containers restricted
	else if (!str_cmp (subcmd, "oval1"))
	{
		if (edit_obj->obj_flags.type_flag == ITEM_BOOK
			|| edit_obj->obj_flags.type_flag == ITEM_KEY)
		{
			ch->send_to_char ("This oval cannot be edited.\n");
			return;
		}
		
		if (edit_obj->obj_flags.type_flag == ITEM_DRYCON
			|| edit_obj->obj_flags.type_flag == ITEM_DRINKCON)
		{
			argument = one_argument (argument, buf);
			
			if (*buf == '0' || atoi (buf))
				edit_obj->o.od.value[1] = atoi (buf);
			else
			{
				ch->send_to_char ("Expected capacity value.\n");
				return;
			}
			
				//adjust contains to match Capacity
			if (edit_obj->o.od.value[1] > edit_obj->o.od.value[0])
			{
				edit_obj->o.od.value[1] = edit_obj->o.od.value[0];
				ch->send_to_char ("Contains adjusted to match Capacity.\n");
				return;
			}
			else 
				return;
			
		}
		
		argument = one_argument (argument, buf);
		
		if (*buf == '0' || atoi (buf))
			edit_obj->o.od.value[1] = atoi (buf);
		else
		{
			ch->send_to_char ("Expected oval1 value.\n");
			return;
		}
	}
	
		//oval 2 - no type restrictions 
	else if (!str_cmp (subcmd, "oval2"))
	{
		argument = one_argument (argument, buf);
		
		if (*buf == '0' || atoi (buf))
			edit_obj->o.od.value[2] = atoi (buf);
		else
		{
			ch->send_to_char ("Expected oval2 value.\n");
			return;
		}
		
			//we have the max capacity and we know the item it contains
			//now we calculate capacity
			//re-set volume to 1 if there were too many
		if ((edit_obj->obj_flags.type_flag == ITEM_DRYCON) 
			&& (edit_obj->o.drycon.contents > 0)
			&& (edit_obj->o.drycon.max_weight > 0))
		{
			tmp_obj = vtoo(edit_obj->o.drycon.contents);
			edit_obj->o.drycon.capacity = (int)(edit_obj->o.drycon.max_weight/tmp_obj->obj_flags.weight);
			if (edit_obj->o.drycon.volume * tmp_obj->obj_flags.weight > edit_obj->o.drycon.capacity)
				edit_obj->o.drycon.volume = 1;
		}
	}
	
		//oval 3 - restrictions for dry_containers
	else if (!str_cmp (subcmd, "oval3"))
	{
		argument = one_argument (argument, buf);
		
		if (*buf == '0' || atoi (buf))
			edit_obj->o.od.value[3] = atoi (buf);
		else
		{
			ch->send_to_char ("Expected oval3 value.\n");
			return;
		}
			//we have the max capacity and we know the item it contains
			//now we calculate capacity
			//re-set volume to 1 if there were too many
		if ((edit_obj->obj_flags.type_flag == ITEM_DRYCON) 
			&& (edit_obj->o.drycon.contents > 0)
			&& (edit_obj->o.drycon.max_weight > 0))
		{
			tmp_obj = vtoo(edit_obj->o.drycon.contents);
			edit_obj->o.drycon.capacity = (int)(edit_obj->o.drycon.max_weight/tmp_obj->obj_flags.weight);
			if (edit_obj->o.drycon.volume * tmp_obj->obj_flags.weight > edit_obj->o.drycon.capacity)
				edit_obj->o.drycon.volume = 1;
		}
	}
	
		//oval 4 - no type restricted
	else if (!str_cmp (subcmd, "oval4"))
	{
		
		argument = one_argument (argument, buf);
		
		if (*buf == '0' || atoi (buf))
			edit_obj->o.od.value[4] = atoi (buf);
		else
		{
			ch->send_to_char ("Expected oval4 value.\n");
			return;
		}
	}
	
		//oval 5 - no type restrictions
	else if (!str_cmp (subcmd, "oval5"))
	{
		argument = one_argument (argument, buf);
		
		
		if (*buf == '0' || atoi (buf))
			edit_obj->o.od.value[5] = atoi (buf);
		else
		{
			ch->send_to_char ("Expected oval5 value.\n");
			return;
		}
		
	}
	
	
		//zones - for builders
	else if (!str_cmp (subcmd, "bzone"))
	{
		argument = one_argument (argument, buf);
		
		
		if (*buf == '0' || atoi (buf))
			edit_obj->zone = atoi (buf);
		else
		{
			ch->send_to_char ("Expected build zone value.\n");
			return;
		}
		ch->send_to_char ("Bzone set. \n");
	}
	
		//keywords for mask, full-helms and dice
	else if (!str_cmp (subcmd, "mkeys"))
	{
		
		argument = one_argument (argument, buf);
		
		if (edit_obj->obj_flags.type_flag != ITEM_TOSSABLE
			&& !IS_SET (edit_obj->obj_flags.extra_flags, ITEM_MASK))
		{
			ch->send_to_char
			("Use <oset mask> to set the mask flag before entering keywords.\nOr change the type to TOSSABLE\n");
			return;
		}
		
		if (!*buf)
		{
			ch->send_to_char ("You must enter a keyword list.\n");
			return;
		}
		
		if (edit_obj->obj_flags.type_flag == ITEM_WORN
			|| edit_obj->obj_flags.type_flag == ITEM_TOSSABLE)
		{
			ch->send_to_char
			("Remember, the first keyword will be used in the wearer's short description.\n");
			edit_obj->desc_keys = duplicateString (buf);
		}
		ch->send_to_char ("Keywords for a mask, full helm or die are now set. \n");
	}
	
		//clock for morph timer - (months days hours)
	else if (!str_cmp (subcmd, "clock"))
	{
		
		int month, day, hour;
		
		argument = one_argument (argument, buf);
		
		if (!isdigit (*buf))
		{
			ch->send_to_char ("Expected a number of months.\n");
			return;
		}
		month = atoi (buf);
		
		argument = one_argument (argument, buf);
		
		if (!isdigit (*buf))
		{
			ch->send_to_char ("Expected a number of days.\n");
			return;
		}
		day = atoi (buf);
		
		argument = one_argument (argument, buf);
		
		if (!isdigit (*buf))
		{
			ch->send_to_char ("Expected a number of hours.\n");
			return;
		}
		hour = atoi (buf);
		
		edit_obj->clock = ((month * 30 * 24) + (day * 24) + hour);
		ch->send_to_char ("Morph time set. \n");
	}
	
		//morphto vnum
	else if (!str_cmp (subcmd, "morphto"))
	{
		
		argument = one_argument (argument, buf);
		
		if (!isdigit (*buf))
		{
			ch->send_to_char ("Expected an object number.\n");
			return;
		}
		
		edit_obj->morphto = atoi (buf);
		ch->send_to_char ("Morph-to set. \n");
	}
	
		
		//econ - see 'econ-tags'
	else if (!str_cmp (subcmd, "econ"))
	{
		argument = one_argument (argument, buf);
		
		if (!*buf)
		{
			ch->send_to_char ("Expected an econ value (see \'tags econ-flags\').\n");
			return;
		}
		
		
		if ((ind = index_lookup (econ_flags, buf)) == -1)
		{
			ch->send_to_char ("I could not find that econ flag.\n");
			return;
		}
		TOGGLE (edit_obj->econ_flags, 1 << ind);
		ch->send_to_char ("Econ tag set. \n");
	}
	
	
	
	
	/** clanning for objects **/
	/**
	 clanadd <clan name> [<clan rank>]
	 **/
	else if (!str_cmp (subcmd, "clanadd"))
	{
		argument = one_argument (argument, clan_name);
		
		if (!*clan_name)
		{
			ch->send_to_char ("Expected a clan name\n");
			return;
		}
		
		argument = one_argument (argument, clan_rank);
		if (!*clan_rank)
			sprintf(clan_rank, "member");
		
		new_cflags = clan_rank_to_value (clan_rank, clan_name);
		
		if (new_cflags == 0)
			sprintf(clan_rank, "member");
		
		if (edit_obj->clan_data) //removes old clan
			clan_rem_obj (edit_obj, edit_obj->clan_data);
		
		edit_obj->clan_data = new OBJ_CLAN_DATA;
		edit_obj->clan_data->name = duplicateString (clan_name);
		edit_obj->clan_data->rank = duplicateString (clan_rank);
		
		ch->send_to_char("Clan/rank have been added.\n");
	}//if clannadd
	
	
	/**
	 clanremove
	 **/
	else if (!str_cmp (subcmd, "clanremove"))
	{
		clan_rem_obj (edit_obj, edit_obj->clan_data);
		ch->send_to_char ("Clanning removed from object. \n");
	}
	
	
	else if (subcmd)
	{
		sprintf (buf, "Unknown option: %s\n", subcmd);
		ch->send_to_char (buf);
		return;
	}
	
	
	
	if ((i = redefine_objects (edit_obj)) > 0)
	{
		sprintf (buf, "%d objects in world redefined.\n", i);
		ch->send_to_char (buf);
	}
	
	if (full_description)
	{
		free_mem(ch->desc->descStr);
		free_mem(ch->desc->pending_message);
		ch->desc->pending_message = new MESSAGE_DATA;
		ch->desc->descStr = ch->desc->pending_message->message;
		ch->desc->proc = post_odesc;
		ch->delay_info1 = edit_obj->nVirtual;
		if (indoor_description)
			ch->delay_info2 = 1;
		
		
		ch->send_to_char ("\nOld description:\n\n");
		ch->send_to_char (edit_obj->full_description);
		
		ch->desc->max_str = STR_MULTI_LINE;
		ch->send_to_char
		("\nPlease enter the new description; terminate with an '@'\n");
		ch->make_quiet();
		
		
		return;
	}
}

void
mobile_edit (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	char tmp_buf[MAX_INPUT_LENGTH];
	CHAR_DATA *edit_mobile;

	argument = one_argument (argument, buf);

	if (*buf)
	{
		if ((ch->pc->edit_player || ch->pc->edit_mob) && *argument != '!')
		{
			sprintf(tmp_buf, "  You are still editing. Use 'mset save' to save your work. Otherwise, use #3'mset new %s !'#0, without the quotes, to discard old changes and start on a new mobile or player.\n", buf);
			ch->send_to_char (tmp_buf);
			return;
		}
		
		if (ch->pc->edit_player)
		{
			(ch->pc->edit_player)->unload_pc();
			ch->pc->edit_player = NULL;
		}

		ch->pc->edit_mob = 0;
	}

	

	if (!(edit_mobile = load_pc (buf)))
	{

		if ((!ch->get_trust()) || (!(edit_mobile = vtom (atoi (buf))) &&
			!(edit_mobile = get_char_room_vis (ch, buf))))
		{
			ch->send_to_char ("Couldn't find that mobile.\n");
			ch->pc->edit_mob = 0;
			return;
		}

		if (edit_mobile &&
			edit_mobile->mob &&
			edit_mobile != vtom (edit_mobile->mob->nVirtual))
			ch->pc->edit_mob = edit_mobile->mob->nVirtual;
		else
			ch->pc->edit_mob = NULL;

		/* If PC, then only a parital name was specified, do a load_pc */

		if (!IS_NPC (edit_mobile))
		{
			if (!(edit_mobile = load_pc (edit_mobile->name)))
			{
				ch->send_to_char ("Unable to load online-PC.\n");
				ch->pc->edit_mob = 0;
				return;
			}
		}
	}

	if (!IS_NPC (edit_mobile))
	{

		if (!str_cmp (ch->pc->account_name, edit_mobile->pc->account_name)
			&& edit_mobile != ch)
		{
			ch->send_to_char
				("Sorry, but you'll need to get another staff member to edit your character.\n"
				"Editing of one's own PCs is not permitted, for a variety of reasons.\n");
			edit_mobile->unload_pc();
			return;
		}

		if ((!ch->get_trust()) && edit_mobile->pc->create_state != STATE_SUBMITTED)
		{
			ch->send_to_char
				("You may only open PCs in the application queue to edit.\n");
			edit_mobile->unload_pc();
			return;
		}

		if (ch->get_trust() < 3)
		{
			ch->send_to_char
				("You are not authorized to work with mobiles.\n");
			edit_mobile->unload_pc();
			return;
		}

		ch->pc->edit_mob = 0;
		ch->pc->edit_player = edit_mobile;

		sprintf (buf, "Editing PLAYER %s\n", edit_mobile->name);
		ch->send_to_char (buf);
		return;
	}

	if (IS_NPC(edit_mobile) && IS_SET (edit_mobile->mob->action, ACT_STAYPUT))
	{
		ch->act
			("Loaded Stayput mobs should NOT be edited. If you need to make a change to this particular individual, remove the stayput flag and then make changes. Old mobs will keep the old data. If you want to make a change to the proto-type, please edit it by number and not by name.\n Mobs must be loaded for changes to be seen. ",
			false, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	ch->pc->edit_mob = edit_mobile->mob->nVirtual;

	sprintf (buf, "Editing mobile %s\n", edit_mobile->keywords);
	ch->send_to_char (buf);
}


void
give_mset_help (CHAR_DATA * ch)
{
	page_string (ch->desc,
		"Syntax:\n"
		"     mset \n"  
		"     mset [mob-vnum]\n"
		"     mset new \n"
        "     mset delete \n"
		"     mset save \n"
		"     mset name <keywords>\n"
		"     mset sdesc <short description>\n"
		"     mset ldesc <long description>\n"
		"     mset desc\n"
		"\n"
		"        The following options need a simple numberic value:\n"
		"\n"
		"     armor | ac, moves, state (-1..4)\n"
		"     natural, str, dex, int, wil, con, aur, bite,\n"
		"     height, frame, room (pc's only), sleep\n"
		"     For example, \'mset str 15\'\n"
		"\n"
		"     mset access <room flags>\n"
		"     mset noaccess <room flags>\n"
		"     mset conds <drunk #> <full #> <thirst #> \n"
		"     mset dam <number>d<number>+/-<number> (1d4+2)\n"
		"     mset hits <hit-points>\n"
		"     mset clock <months> <days> <hours>	\n"
		"     mset morphto <objnum> \n"
		"     mset attack <attack-type>\n"
		"     mset [allskills | noskills]\n"
		"\n"
		"     mset <position-types>\n"
		"     mset <race>\n"
		"     mset <skills> <percent>\n"
		"     mset <action-bits>\n"
		"     mset <affected-bits>\n"
		"     mset dpos <position-types>\n"
		"     mset [trudge | pace | walk | jog | run | sprint]\n"
		"\n"
		"     mset keeper\n"
		"     mset ostler\n"
        "     mset merch_seven <0,1> \n"
		"     mset markup <number>\n"
		"     mset discount <number>\n"
		"     mset econ_markup[1-7] <number>\n"
		"     mset econ_discount[1-7] <number> \n"
		"     mset nobuy <econ flag>\n"
		"     mset econ[1-7] <econ flag>\n"
		"     mset shop\n"
		"     mset store\n"
		"     mset trades <item type>\n"
		"     mset materials <material>\n"
		"     mset delivery <ovnum> \n"
		"\n");
}

void
post_mdesc (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *ch;
	CHAR_DATA *mob;

	ch = d->character;
	
	if (ch->delay_info1)
		mob = vtom (ch->delay_info1);
	else
		mob = load_pc (ch->delay_who);
	
	ch->delay_info1 = 0;
	
	if (ch->delay_who)
		free_mem (ch->delay_who);

	if (!mob)
	{
		ch->send_to_char ("NULL mob pointer... aborting...\n");
		return;
	}

	if (!d->pending_message->message)
	{
		ch->send_to_char ("No mobile description posted.\n");
		if (!IS_NPC(mob))
			mob->unload_pc();
		return;
	}

	mob->description = duplicateString (d->pending_message->message);
	
	std::stringstream tempstr;
	tempstr << reformat_desc(mob->description);	
	mob->description = duplicateString(tempstr.str().c_str());
	

	builder_log(ch, "mdesc", d->pending_message->message);

	
	d->pending_message = NULL;
	if (!IS_NPC(mob))
		mob->unload_pc();
}



	//cset - (not editing a craft) message about not editing
	//cset - (editing a craft) stat the craft
	//cset ? - displays help message
	//cset <craft-name> - (not editing a craft) starts editing
	//cset <craft-name> - (editing a craft) gives warning
	//cset <craft-name> ! - (editing a craft) dumps old craft and starts new one
	//cset <options> - does something special, depending on <option>
void
do_cset (CHAR_DATA * ch, char *argument, int cmd)
{
	char subcmd[MAX_STRING_LENGTH]= { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	SUBCRAFT_HEAD_DATA *tcraft;
	
	if (IS_NPC (ch))
	{
		ch->send_to_char ("This is a PC-only command.\n");
		return;
	}
	
	if (!IS_SET(ch->plr_flags, IS_CRAFTER))
	{
		ch->send_to_char ("You must be an authorised crafter to use this command.\n");
		return;
	}
	
	argument = one_argument (argument, subcmd);
	
	if (!*subcmd)
	{
		if (ch->pc->edit_craft)
		{
			craftstat(ch, ch->pc->edit_craft->subcraft_name);
			return;
			
		}
		else
			ch->send_to_char("You're not editing a craft.\n");
		return;
	}
	
		//"cset ?"
	if (*subcmd == '?')
	{
		
		ch->send_to_char ("\n\nSyntax:\n");
		ch->send_to_char ("     cset new <craft> <subcraft> <command> \n");
		ch->send_to_char ("     cset delete\n");
		ch->send_to_char ("     cset save\n");
		ch->send_to_char ("-----------------------\n");
		ch->send_to_char ("     cset craft\n");
		ch->send_to_char ("     cset subcraft\n");
		ch->send_to_char ("     cset command\n");
		ch->send_to_char ("     cset hidden\n");
		ch->send_to_char ("     cset obscure\n");
		ch->send_to_char ("     cset terrains\n");
		ch->send_to_char ("     cset seasons\n");
		ch->send_to_char ("     cset weather\n");
		ch->send_to_char ("     cset opening\n");
		ch->send_to_char ("     cset race\n");
		ch->send_to_char ("     cset clan\n");
		ch->send_to_char ("     cset delay\n");
		ch->send_to_char ("-----------------------\n");
		ch->send_to_char ("     cset failure\n");
		ch->send_to_char ("     cset failobjs\n");
		ch->send_to_char ("     cset failmobs\n");
		ch->send_to_char ("     cset key_start\n");
		ch->send_to_char ("     cset key_end\n");
		ch->send_to_char ("     cset followers\n");
		ch->send_to_char ("-----------------------\n");
		ch->send_to_char ("     cset phase # 1st <message>\n");
		ch->send_to_char ("     cset phase # 3rd <message>\n");
		ch->send_to_char ("     cset phase # failure-1st <message>\n");
		ch->send_to_char ("     cset phase # failure-3rd <message>\n");
		ch->send_to_char ("     cset phase # group <message>\n");
		ch->send_to_char ("     cset phase # failure-group <message>\n");
		ch->send_to_char ("     cset phase # delete\n");
		ch->send_to_char ("     cset phase # time <value>\n");
		ch->send_to_char ("     cset phase # skill <skill name>\n");
		ch->send_to_char ("     cset phase # skill none\n");
		ch->send_to_char ("     cset phase # check XdY\n");
		ch->send_to_char ("     cset phase # attribute <attribute name>\n");
		ch->send_to_char ("     cset phase # attribute none\n");
		ch->send_to_char ("     cset phase # checkattribute XdY\n");
		ch->send_to_char ("     cset phase # fobj# [X(count#)] <list of vnums>\n");
		ch->send_to_char ("     cset phase # object# [X(count#)] <list of vnums> <flag>\n");
		ch->send_to_char ("     cset phase # mobile# <list of vnums> <flag>\n");
		
		return;
	}
	
		//subcmd is something besides ?
		//try to find a subcraft name that matches argument
		//"cset <craft-name>"
	for (tcraft_iterator = craft_map.begin(); tcraft_iterator != craft_map.end(); tcraft_iterator++)
	{
		tcraft = tcraft_iterator->second;
		
		if (!str_cmp (subcmd, tcraft->subcraft_name))
			break;
	}
	
		//We found a matching craft, and we are not editing another craft
	if ((tcraft_iterator != craft_map.end()) && (ch->pc->edit_craft == NULL))
	{
		ch->pc->edit_craft = tcraft;
		ch->send_to_char ("Craft has been opened for editing.\n");
		return;
	}
	
		//we found a craft, but we are already editing a craft
	else if ((tcraft_iterator != craft_map.end()) && (ch->pc->edit_craft != NULL) && *argument != '!')
	{
		
		sprintf(buf2, "You are still editing the %s craft.\n Use 'cset save' to save your work. Otherwise, use #3'cset %s !'#0, without the quotes, to discard your changes and work on a different craft.\n", ch->pc->edit_craft->subcraft_name, subcmd);
		ch->send_to_char (buf2);
		return;
	}
		//we found a craft, and we are already editing a craft, but we want to work on a different craft.
	else if (tcraft && (ch->pc->edit_craft != NULL) && *argument == '!')
	{
		ch->pc->edit_craft = tcraft;
		sprintf(buf, "You are now editing %s\n",ch->pc->edit_craft->subcraft_name);
		ch->send_to_char (buf);
		return;
	}
		
		//we didn't find a craft, so continue on tocheck for argument as an option name
		//"cset new"
		//"cset new <catagory> <subcraft> <command>"
	if (!str_cmp (subcmd, "new"))
	{
		craft_new(ch, argument);
		return;	
	}
	
	else if (!str_cmp (subcmd, "save"))
	{
		craft_save(ch);
		return;
	}
	
	//"cset <option> <values>"
	else if ((!str_cmp (subcmd, "craft"))||
		(!str_cmp (subcmd, "subcraft")) ||
		(!str_cmp (subcmd, "command")) ||
		(!str_cmp (subcmd, "hidden")) ||
		(!str_cmp (subcmd, "obscure")))
	{
		craft_setup (ch, argument, subcmd);
		return;
	}

	else if (!str_cmp (subcmd, "terrains"))
	{
		craft_terrains(ch, argument, subcmd);
		return;
	}

	else if (!str_cmp (subcmd, "seasons"))
	{
		craft_seasons(ch, argument, subcmd);
		return;
	}

	else if (!str_cmp (subcmd, "weather"))
	{
		craft_weather(ch, argument, subcmd);
		return;
	}

	else if (!str_cmp (subcmd, "opening"))
	{
		craft_opening(ch, argument, subcmd);
		return;
	}

	else if (!str_cmp (subcmd, "race"))
	{
		craft_race(ch, argument, subcmd);
		return;
	}

	else if (!strn_cmp (subcmd, "clan", 4))
	{
		craft_clan(ch, argument, subcmd);
		return;
	}

	else if (!str_cmp (subcmd, "delete"))
	{
		craft_delete(ch, argument, subcmd);
		return;
	}

	else if (!strn_cmp (subcmd, "delay", 5))
	{
		craft_delay(ch, argument, subcmd);
		return;
	}

	
	/*** key item **/
	else if (!strn_cmp (subcmd, "key_start", 9))
	{
		craft_key(ch, argument, subcmd);
		return;
	}
	/*** key-production list **/
	else if (!strn_cmp (subcmd, "key_end", 7))
	{
		craft_key_product(ch, argument, subcmd);
		return;
	}

	/*** followers **/
	else if (!strn_cmp (subcmd, "followers", 9))
	{
		craft_group(ch, argument, subcmd);
		return;
	}

	
	/***  PHASES  **/
	else if (!strn_cmp (subcmd, "phase", 5))
	{
		craft_phases(ch, argument, subcmd);
		return;
	}
	

	/** error message **/
	else
	{
		ch->send_to_char ("I'm afraid that isn't a recognized cset command or a recognized craft name.\n");
		return;
	}
}

void
mset_account_flag (CHAR_DATA* ch, const char *account_name, const char *subcmd)
{
	account acct (account_name);
	
	if (acct.is_registered ())
	{
		if (!strcasecmp (subcmd, "nopetition"))
		{
			if (acct.toggle_petition_ban ())
				ch->send_to_char("A petition ban has been placed on that PC's account.\n");
			else
				ch->send_to_char("The petition ban on that PC's account has been removed.\n");
			
		}
		
		else if (!strcasecmp (subcmd, "beta"))
		{
			if (acct.toggle_beta ())
				ch->send_to_char("That account will be able to login during BETA.\n");
			else 
				ch->send_to_char("That account will no longer be able to log in to game during BETA.\n");
		}
		
		else if (!strcasecmp (subcmd, "emeritus"))
		{
			if (acct.toggle_emeritus ())
				ch->send_to_char("That account will have Emeritus status and use of wiznet.\n");			
			else 
				ch->send_to_char("That account will no longer have Emeritus status or use of wiznet.\n"); 
			
		}
		
		else if (!strcasecmp (subcmd, "noban"))
		{
			if (acct.toggle_ban_pass ())
				ch->send_to_char("That PC's account is no longer exempt from any applicable sitebans.\n");
			else
				ch->send_to_char("That PC's account is now exempt from any applicable sitebans.\n");
		}
		
		else if (!strcasecmp (subcmd, "noguest"))
		{
			if (acct.toggle_guest_ban ())
				ch->send_to_char("The guest ban on that PC's account has been removed.\n");
			else 
				ch->send_to_char("A guest ban has been placed on that PC's account.\n");
		}
		
		else if (!strcasecmp (subcmd, "nopsi"))
		{
			if (acct.toggle_psionics_ban ())
				ch->send_to_char("A psionics ban has been placed on that PC's account.\n");
			else
				ch->send_to_char("The psionics ban on that PC's account has been removed.\n");
		}
		
		else if (!strcasecmp (subcmd, "noretire"))
		{
			if (acct.toggle_retirement_ban ())
				ch->send_to_char("A retirement ban has been placed on that PC's account.\n");
			else 
				ch->send_to_char("The retirement ban on that PC's account has been removed.\n");
		}
		
		else if (!str_cmp (subcmd, "ipsharingok"))
		{
			if (acct.toggle_ip_sharing ())
				ch->send_to_char("That player will no longer be able to share IPs with others.\n");
			else 
				ch->send_to_char("The IP sharing restrictions have been lifted on that account.\n");
		}
	}
}



	//mset - (not editing a mobile) not editing message
	//mset - (editing a mobile) stat the mobile
	//mset ? - displays help message
	//mset ## - (not editing a mobile) starts editing existing mob
	//mset ## - (already editing a mobile) gives warning
	//mset ## ! - (editing a mobile) dumps old mobile an starts new one
	//mset <options> - does something special, depending on <option>

void
do_mset (CHAR_DATA * ch, char *argument, int cmd)
{
	char subcmd[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char clanname[MAX_STRING_LENGTH]= { '\0' };
	std::string skillstr;
	CHAR_DATA *edit_mob;
	CHAR_DATA *tch;
	AFFECTED_TYPE *af;
	SKILL_DATA *tskill;
	std::list<CHAR_DATA*>::iterator tch_iterator;
	std::map<int, OBJECT_MATERIAL*>::iterator material_type_it;
	std::list<std::string>::iterator materials_it;
	std::map<std::string, SKILL_DATA*> ::iterator skill_map_it;
	int i;
	int j;
	int delta;
	int ind;
	int full_description = 0;
	int dice;
	int sides;
	int bonus;
	int parms;
	int loads = 0;
	int error = 0;
	char c;
	char *p;
	char *skill_name;
	
	builder_log(ch, "mset", argument);
	
	if (IS_NPC (ch))
	{
		ch->send_to_char ("This is a PC-only command.\n");
		return;
	}
	
	if (!IS_NPC (ch) 
		&& !ch->pc->is_guide
		&& (ch->get_trust() < 3))
	{
		ch->send_to_char ("Eh?\n");
		return;
	}
	
	
	argument = one_argument (argument, subcmd);
		//used by guides to approve online applications
	if ((!ch->get_trust()) && str_cmp (subcmd, "sdesc") && str_cmp (subcmd, "ldesc")
		&& str_cmp (subcmd, "name") && str_cmp (subcmd, "desc"))
	{
		ch->send_to_char
		("Uses: MSET NAME, MSET SDESC, MSET LDESC, or MSET DESC.\n");
		return;
	}
	
	if (!*subcmd)
	{
		if (ch->pc->edit_mob)
		{
			sprintf (buf, "m %d", ch->pc->edit_mob);
			do_stat (ch, buf, 0);
		}
		else if (ch->pc->edit_player)
		{
			sprintf (buf, "c %s", (ch->pc->edit_player)->name);
			do_stat (ch, buf, 0);
		}
		else
			ch->send_to_char ("You're not editing a mobile.\n");
		return;
	}
	
	else if (*subcmd == '?')
	{
		give_mset_help (ch);
		return;
	}
	
	else if (!str_cmp (subcmd, "new"))
	{
		mobile_new(ch, argument);
		return;
	}
	
	
	if (isdigit (*subcmd) && (edit_mob = vtom (atoi (subcmd))))
	{
		if (!ch->pc->edit_mob || ((ch->pc->edit_mob && *argument == '!')))
		{
			ch->pc->edit_mob = edit_mob->mob->nVirtual;
			sprintf (buf, "You are now editing mobile %d.\n", edit_mob->mob->nVirtual);
			ch->send_to_char (buf);
			return;
		}
		else if (ch->pc->edit_mob && *argument != '!')
		{
			sprintf(buf, "You are still editing mobile %d.\n Use 'mset save' to save your work. Otherwise, use #3'mset %s !'#0, without the quotes, to discard old changes and start on a new mobile or player.\n", ch->pc->edit_mob, subcmd);
			ch->send_to_char (buf);
			return;
			
		}
	}
	
	else if (isdigit (*subcmd))
	{
		sprintf (buf, "No such mobile vnum %s\n", subcmd);
		ch->send_to_char (buf);
		ch->send_to_char ("Not editing a mobile now.\n");
		ch->pc->edit_mob = 0;
		return;
	}
	
	else if (!(edit_mob = vtom (ch->pc->edit_mob)) &&
			 !(edit_mob = ch->pc->edit_player))
	{
		mobile_edit(ch, subcmd);
		return;
	}
	
	if (ch->pc->edit_player)
		edit_mob = ch->pc->edit_player;
	
	
	if (IS_SET (edit_mob->flags, FLAG_VARIABLE)
		&& str_cmp (subcmd, "variable"))
	{
		ch->send_to_char
		("Don't mset this mob, unless you're removing the variable flag.\n");
		return;
	}
	
		
	else if (!str_cmp(subcmd,"crafter"))
	{
		if (ch->get_trust() < 5)
		{
			ch->send_to_char ("You must be level 5 to toggle the IsCrafter flag.\n");
			return;
		}
		
		if (!IS_SET (edit_mob->plr_flags, IS_CRAFTER))
		{
			edit_mob->plr_flags |= IS_CRAFTER;
			ch->send_to_char
			("This admin is now authorised to use cset.\n");
			return;
		}
		
		edit_mob->plr_flags &= ~IS_CRAFTER;
		ch->send_to_char
		("This admin is no longer authorised to use cset.\n");
		return;
	}
	
	else if (!str_cmp (subcmd, "currency"))
	{
		if (!IS_NPC (edit_mob))
		{
			ch->send_to_char ("Currency type may only be set on mobiles.\n");
			return;
		}
		argument = one_argument (argument, buf);
		if (!*buf)
		{
			ch->send_to_char
			("Which currency type should they deal in? (Copper is the default)\n");
			return;
		}
		*buf = tolower (*buf);
		if (!str_cmp (buf, "gondorian") || !str_cmp (buf, "copper"))
		{
			edit_mob->mob->currency_type = CURRENCY_TIRITH;
			ch->send_to_char
			("That mobile will now deal in Gondorian currency (Coppers).\n");
		}
			//Gondor is the default, but we may add other later
		else
		{
			edit_mob->mob->currency_type = CURRENCY_TIRITH;
			return;
		}
	}
	
	
	else if (!str_cmp (subcmd, "level"))
	{
		
		if (ch->get_trust() < 5)
		{
			ch->send_to_char ("You cannot assign levels.\n");
			return;
		}
		
		if (IS_NPC (ch))
		{
			ch->send_to_char ("Change levels on PCs only.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
			
		if (!isdigit (*buf) || atoi (buf) > 5)
		{
			ch->send_to_char ("Expected level 0..5\n");
			return;
		}
		edit_mob->pc->level = atoi (buf);
		save_char (edit_mob, true);
		ch->send_to_char ("Player level set and saved.\n");
		return;
	}
	
	else if (!str_cmp (subcmd, "account"))
	{
		if (ch->get_trust() < 5)
		{
			ch->send_to_char ("This option is only for level 5 admins.\n");
			return;
		}
		
		if (IS_NPC (edit_mob))
		{
			ch->send_to_char ("This command is for PCs only.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		if (!*buf)
		{
			ch->send_to_char
			("Which account did you wish to move this PC to?\n");
			return;
		}
				
		if (edit_mob->pc->account_name && *edit_mob->pc->account_name)
			free_mem (edit_mob->pc->account_name);
		buf[0] = toupper (buf[0]);
		edit_mob->pc->account_name = duplicateString (buf);
		ch->send_to_char ("Account set and saved.\n");
		save_char (edit_mob, false);
		return;
	}
	
	else if (!str_cmp (subcmd, "nogain"))
	{
		if (ch->get_trust() < 4)
		{
			ch->send_to_char
			("You must be at least a level four admin to use this flag.\n");
			return;
		}
		argument = one_argument (argument, buf);
		if (!*buf)
		{
			ch->send_to_char ("Toggle the nogain flag in which skill?\n");
			return;
		}
		
		ind = lookup_skill_id(buf);
		if (ind == -1)
		{
			ch->send_to_char ("Unknown skill.\n");
			return;
		}
		
		af = get_affect (edit_mob, MAGIC_FLAG_NOGAIN + ind);
		if (!af)
		{
			af = new AFFECTED_TYPE;
			af->next = NULL;
			af->type = MAGIC_FLAG_NOGAIN + ind;
			affect_to_char (edit_mob, af);
			ch->send_to_char
			("Skill advancement has been halted in the specified skill.\n");
			if (!IS_NPC (edit_mob))
			{
				ch->send_to_char
				("\n#2Please leave a note explaining the situation for inclusion in their pfile.#0\n");
				sprintf (buf, "%s Advancement Halted: %s.", edit_mob->name,
						 lookup_skill_name(ind));
				do_write (ch, buf, 0);
			}
		}
		else
		{
			affect_remove (edit_mob, af);
			ch->send_to_char
			("The character may now resume advancement in the specified skill.\n");
			if (!IS_NPC (edit_mob))
			{
				ch->send_to_char
				("\n#2Please leave a note explaining the situation for inclusion in their pfile.#0\n");
				sprintf (buf, "%s Advancement Resumed: %s.",
						 edit_mob->name, lookup_skill_name(ind));
				do_write (ch, buf, 0);
			}
		}
		return;
	}
	
	else if (!str_cmp (subcmd, "sleep"))
	{
		
		if (!edit_mob->pc)
		{
			ch->send_to_char ("The sleep keyword can only be used against "
						  "PCs.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (!isdigit (*buf))
		{
			ch->send_to_char ("Sleep is in seconds and less than 600 (10 "
						  "minutes).\n");
			return;
		}
		
	}
	
	else if (!str_cmp (subcmd, "nopetition")
			 || !str_cmp (subcmd, "noban")
			 || !str_cmp (subcmd, "beta")
			 || !str_cmp (subcmd, "noguest")
			 || !str_cmp (subcmd, "nopsi")
			 || !str_cmp (subcmd, "noretire")
			 || !str_cmp (subcmd, "ipsharingok")
			 || !str_cmp (subcmd, "emeritus"))
	{
		if (!IS_NPC (edit_mob) 
			&& edit_mob->pc 
			&& edit_mob->pc->account_name)
		{
			
			mset_account_flag (ch, edit_mob->pc->account_name, subcmd);
		}
		else
		{
			ch->send_to_char ("I couldn't access that character's account.\n");
		}
		return;
	}
	
		
	else if (!str_cmp (subcmd, "state"))
	{
		
		if (IS_NPC (edit_mob))
		{
			ch->send_to_char ("Mobiles don't have a create state.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (!atoi (buf) && !isdigit (*buf))
		{
			ch->send_to_char ("Expected a creation state value -1 to 5.\n");
			ch->send_to_char("-1 - Rejected application\n"
						 " 0 - Applying\n"
						 " 1 - Submitted application\n"
						 " 2 - Approved\n"
						 " 3 - Suspended\n"
						 " 4 - Dead\n"
						 " 5 - Retired\n");
			
			return;
		}
		
		if (atoi (buf) > 5 || atoi (buf) < -1)
		{
			ch->send_to_char ("Expected a creation state value -1 to 5.\n");
			ch->send_to_char("-1 - Rejected application\n"
						 " 0 - Applying\n"
						 " 1 - Submitted application\n"
						 " 2 - Approved\n"
						 " 3 - Suspended\n"
						 " 4 - Dead\n"
						 " 5 - Retired\n");
			return;
		}
		
		edit_mob->pc->create_state = atoi (buf);
		if (edit_mob->pc->create_state == STATE_DIED)
		{
			if (IS_SET(edit_mob->flags, FLAG_RETIRED))
				edit_mob->flags &= ~FLAG_RETIRED;
				
			edit_mob->flags |= FLAG_DEAD;
		}
		
		if (edit_mob->pc->create_state == STATE_RETIRED)
		{
			if (IS_SET(edit_mob->flags, FLAG_DEAD))
				edit_mob->flags &= ~FLAG_DEAD;

			edit_mob->flags |= FLAG_RETIRED;
		}
		
		
			//make certain we have a tname sicne we may have skipped this step with the state change
		if (edit_mob->pc->create_state != 1)
		{
			strcpy (subcmd, edit_mob->name);
			*subcmd = toupper (*subcmd);
		}
		else
		{
			strcpy (subcmd, edit_mob->name);
			*subcmd = toupper (*subcmd);
		}
		
		ch->send_to_char ("State set and saved.\n");
		save_char (edit_mob, false);
		return;
	}
	
	
	/** morphing mobs **/
	else if (!str_cmp (subcmd, "morphto"))
	{
		argument = one_argument (argument, buf);
		
		if (!isdigit (*buf))
		{
			ch->send_to_char ("Expected a mob Vnum.\n");
			return;
		}
		
		edit_mob->mob->morphto = atoi (buf);
	}
	
	else if (!str_cmp (subcmd, "clock"))
	{
		
		int month, day, hour;
		
		argument = one_argument (argument, buf);
		if (!isdigit (*buf))
		{
			ch->send_to_char ("Expected a number of months.\n");
			return;
		}
		month = atoi (buf);
		
		argument = one_argument (argument, buf);
		if (!isdigit (*buf))
		{
			ch->send_to_char ("Expected a number of days.\n");
			return;
		}
		day = atoi (buf);
		
		argument = one_argument (argument, buf);
		if (!isdigit (*buf))
		{
			ch->send_to_char ("Expected a number of hours.\n");
			return;
		}
		hour = atoi (buf);
		
		edit_mob->mob->clock = ((month * 30 * 24) + (day * 24) + hour);
	}
	/** does this even work?
	else if (!str_cmp (subcmd, "morphtype"))
	{
		int mtype;
		
		argument = one_argument (argument, buf);
		if (!isdigit (*buf))
		{
			ch->send_to_char ("Expected 1 for physical morph, 2 for skill morph.\n");
			return;
		}
		mtype = atoi (buf);
		if (mtype > 2 || mtype < 0)
			mtype = 1;
		edit_mob->mob->morph_type = mtype;
		
	}
	 
	 
	 ****/
	
	
		//SHOPKEEPER
	else if (!str_cmp (subcmd, "openinghour"))
	{
		argument = one_argument (argument, buf);
		if (!*buf || !isdigit (*buf) || atoi (buf) > 24 || atoi (buf) < 1)
		{
			ch->send_to_char ("Expected a value from 1 to 24.\n");
			return;
		}
		if (!IS_SET (edit_mob->flags, FLAG_KEEPER))
		{
			ch->send_to_char ("This mob isn't a shopkeeper!\n");
			return;
		}
		else
			edit_mob->mob->shop->opening_hour = atoi (buf);
		ch->send_to_char ("Done.\n");
		return;
	}
	
	else if (!str_cmp (subcmd, "closinghour"))
	{
		argument = one_argument (argument, buf);
		if (!*buf || !isdigit (*buf) || atoi (buf) > 24 || atoi (buf) < 1)
		{
			ch->send_to_char ("Expected a value from 1 to 24.\n");
			return;
		}
		if (!IS_SET (edit_mob->flags, FLAG_KEEPER))
		{
			ch->send_to_char ("This mob isn't a shopkeeper!\n");
			return;
		}
		else
			edit_mob->mob->shop->closing_hour = atoi (buf);
		ch->send_to_char ("Done.\n");
		return;
	}
	
	else if (!str_cmp (subcmd, "keeper"))
	{
		if (IS_SET (edit_mob->flags, FLAG_KEEPER))
		{
			edit_mob->flags &= ~FLAG_KEEPER;
			ch->send_to_char ("Note:  Keeper flag removed.  Shop data will "
						  "be deleted when the zone\n"
						  "       is saved.\n");
		}
		
		else
		{
			edit_mob->flags |= FLAG_KEEPER;
			
			if (!edit_mob->mob->shop) {
				edit_mob->mob->shop = new SHOP_DATA;
				edit_mob->mob->shop->negotiations = NULL;
			}
			
			edit_mob->mob->shop->discount = 1.0;
			edit_mob->mob->shop->markup = 1.0;
		}
	}
	
	else if (!str_cmp (subcmd, "markup"))
	{
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting a markup."
						  "\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (isdigit (*buf))
			sscanf (buf, "%f", &edit_mob->mob->shop->markup);
		else
		{
			ch->send_to_char ("Expected a markup multiplier.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "discount"))
	{
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting a "
						  "discount.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (*buf == '.' || isdigit (*buf))
			sscanf (buf, "%f", &edit_mob->mob->shop->discount);
		else
		{
			ch->send_to_char ("Expected a discount multiplier.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "merch_seven"))
	{
		if (!IS_NPC (edit_mob))
		{
			ch->send_to_char ("You can't set that on a PC.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (*buf != '\0')
			edit_mob->mob->merch_seven = atoi (buf);
		else
		{
			ch->send_to_char ("Expected a zero or one for merch_seven\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "econ-discount1") ||
			 !str_cmp (subcmd, "econ_discount1") ||
			 !str_cmp (subcmd, "discount1"))
	{
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting a "
						  "economic discount.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (*buf == '.' || isdigit (*buf))
			sscanf (buf, "%f", &edit_mob->mob->shop->econ_discount1);
		else
		{
			ch->send_to_char ("Expected a economic discount multiplier.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "econ-markup1") ||
			 !str_cmp (subcmd, "econ_markup1") ||
			 !str_cmp (subcmd, "markup1"))
	{
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting a "
						  "economic markup.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (*buf == '.' || isdigit (*buf))
			sscanf (buf, "%f", &edit_mob->mob->shop->econ_markup1);
		else
		{
			ch->send_to_char ("Expected a economic markup multiplier.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "econ-discount2") ||
			 !str_cmp (subcmd, "econ_discount2") ||
			 !str_cmp (subcmd, "discount2"))
	{
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting a "
						  "economic discount.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (*buf == '.' || isdigit (*buf))
			sscanf (buf, "%f", &edit_mob->mob->shop->econ_discount2);
		else
		{
			ch->send_to_char ("Expected a economic discount multiplier.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "econ-markup2") ||
			 !str_cmp (subcmd, "econ_markup2") ||
			 !str_cmp (subcmd, "markup2"))
	{
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting a "
						  "economic markup.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (*buf == '.' || isdigit (*buf))
			sscanf (buf, "%f", &edit_mob->mob->shop->econ_markup2);
		else
		{
			ch->send_to_char ("Expected a economic markup multiplier.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "econ-discount3") ||
			 !str_cmp (subcmd, "econ_discount3") ||
			 !str_cmp (subcmd, "discount3"))
	{
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting a "
						  "economic discount.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (*buf == '.' || isdigit (*buf))
			sscanf (buf, "%f", &edit_mob->mob->shop->econ_discount3);
		else
		{
			ch->send_to_char ("Expected a economic discount multiplier.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "econ-markup3") ||
			 !str_cmp (subcmd, "econ_markup3") ||
			 !str_cmp (subcmd, "markup3"))
	{
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting a "
						  "economic markup.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (*buf == '.' || isdigit (*buf))
			sscanf (buf, "%f", &edit_mob->mob->shop->econ_markup3);
		else
		{
			ch->send_to_char ("Expected a economic markup multiplier.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "econ-discount4") ||
			 !str_cmp (subcmd, "econ_discount4") ||
			 !str_cmp (subcmd, "discount4"))
	{
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting a "
						  "economic discount.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (*buf == '.' || isdigit (*buf))
			sscanf (buf, "%f", &edit_mob->mob->shop->econ_discount4);
		else
		{
			ch->send_to_char ("Expected a economic discount multiplier.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "econ-markup4") ||
			 !str_cmp (subcmd, "econ_markup4") ||
			 !str_cmp (subcmd, "markup4"))
	{
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting a "
						  "economic markup.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (*buf == '.' || isdigit (*buf))
			sscanf (buf, "%f", &edit_mob->mob->shop->econ_markup4);
		else
		{
			ch->send_to_char ("Expected a economic markup multiplier.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "econ-discount5") ||
			 !str_cmp (subcmd, "econ_discount5") ||
			 !str_cmp (subcmd, "discount5"))
	{
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting a "
						  "economic discount.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (*buf == '.' || isdigit (*buf))
			sscanf (buf, "%f", &edit_mob->mob->shop->econ_discount5);
		else
		{
			ch->send_to_char ("Expected a economic discount multiplier.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "econ-markup5") ||
			 !str_cmp (subcmd, "econ_markup5") ||
			 !str_cmp (subcmd, "markup5"))
	{
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting a "
						  "economic markup.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (*buf == '.' || isdigit (*buf))
			sscanf (buf, "%f", &edit_mob->mob->shop->econ_markup5);
		else
		{
			ch->send_to_char ("Expected a economic markup multiplier.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "econ-discount6") ||
			 !str_cmp (subcmd, "econ_discount6") ||
			 !str_cmp (subcmd, "discount6"))
	{
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting a "
						  "economic discount.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (*buf == '.' || isdigit (*buf))
			sscanf (buf, "%f", &edit_mob->mob->shop->econ_discount6);
		else
		{
			ch->send_to_char ("Expected a economic discount multiplier.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "econ-markup6") ||
			 !str_cmp (subcmd, "econ_markup6") ||
			 !str_cmp (subcmd, "markup6"))
	{
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting a "
						  "economic markup.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (*buf == '.' || isdigit (*buf))
			sscanf (buf, "%f", &edit_mob->mob->shop->econ_markup6);
		else
		{
			ch->send_to_char ("Expected a economic markup multiplier.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "econ-discount7") ||
			 !str_cmp (subcmd, "econ_discount7") ||
			 !str_cmp (subcmd, "discount7"))
	{
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting a "
						  "economic discount.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (*buf == '.' || isdigit (*buf))
			sscanf (buf, "%f", &edit_mob->mob->shop->econ_discount7);
		else
		{
			ch->send_to_char ("Expected a economic discount multiplier.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "econ-markup7") ||
			 !str_cmp (subcmd, "econ_markup7") ||
			 !str_cmp (subcmd, "markup7"))
	{
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting a "
						  "economic markup.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (*buf == '.' || isdigit (*buf))
			sscanf (buf, "%f", &edit_mob->mob->shop->econ_markup7);
		else
		{
			ch->send_to_char ("Expected a economic markup multiplier.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "econ"))
	{
		argument = one_argument (argument, buf);
		
		if ((ind = index_lookup (econ_flags, buf)) != -1)
		{
			
			if (!edit_mob->mob->shop)
			{
				ch->send_to_char ("Assign the KEEPER flag before setting econ "
							  "flags.\n");
				return;
			}
			
			TOGGLE (edit_mob->mob->shop->econ_flags1, 1 << ind);
			
			if (!str_cmp (econ_flags[ind], "nobarter"))
				ch->send_to_char
				("Nobarter set as an econ flag.  Did you mean to "
				 "set it as a nobuy flag?\nAs in 'mset nobuy "
				 "nobarter'?\n");
		}
		else
			ch->send_to_char ("I couldn't locate that econ flag.\n");
	}
	
	else if (!str_cmp (subcmd, "nobuy"))
	{
		
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting econ "
						  "flags.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if ((ind = index_lookup (econ_flags, buf)) == -1)
		{
			ch->send_to_char ("No such econ flag.\n");
			return;
		}
		
		TOGGLE (edit_mob->mob->shop->nobuy_flags, 1 << ind);
	}
	
	else if (!str_cmp (subcmd, "buy"))
	{
		
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting econ "
						  "flags.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if ((ind = index_lookup (econ_flags, buf)) == -1)
		{
			ch->send_to_char ("No such econ flag.\n");
			return;
		}
		
		TOGGLE (edit_mob->mob->shop->buy_flags, 1 << ind);
	}
	
	else if (!str_cmp (subcmd, "econ1"))
	{
		
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting econ "
						  "flags.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if ((ind = index_lookup (econ_flags, buf)) == -1)
		{
			ch->send_to_char ("No such econ1 flag.\n");
			return;
		}
		
		TOGGLE (edit_mob->mob->shop->econ_flags1, 1 << ind);
	}
	
	else if (!str_cmp (subcmd, "econ2"))
	{
		
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting econ "
						  "flags.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if ((ind = index_lookup (econ_flags, buf)) == -1)
		{
			ch->send_to_char ("No such econ2 flag.\n");
			return;
		}
		
		TOGGLE (edit_mob->mob->shop->econ_flags2, 1 << ind);
	}
	
	else if (!str_cmp (subcmd, "econ3"))
	{
		
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting econ "
						  "flags.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if ((ind = index_lookup (econ_flags, buf)) == -1)
		{
			ch->send_to_char ("No such econ3 flag.\n");
			return;
		}
		
		TOGGLE (edit_mob->mob->shop->econ_flags3, 1 << ind);
	}
	
	else if (!str_cmp (subcmd, "econ4"))
	{
		
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting econ "
						  "flags.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if ((ind = index_lookup (econ_flags, buf)) == -1)
		{
			ch->send_to_char ("No such econ4 flag.\n");
			return;
		}
		
		TOGGLE (edit_mob->mob->shop->econ_flags4, 1 << ind);
	}
	
	else if (!str_cmp (subcmd, "econ5"))
	{
		
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting econ "
						  "flags.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if ((ind = index_lookup (econ_flags, buf)) == -1)
		{
			ch->send_to_char ("No such econ5 flag.\n");
			return;
		}
		
		TOGGLE (edit_mob->mob->shop->econ_flags5, 1 << ind);
	}
	
	else if (!str_cmp (subcmd, "econ6"))
	{
		
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting econ "
						  "flags.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if ((ind = index_lookup (econ_flags, buf)) == -1)
		{
			ch->send_to_char ("No such econ6 flag.\n");
			return;
		}
		
		TOGGLE (edit_mob->mob->shop->econ_flags6, 1 << ind);
	}
	
	else if (!str_cmp (subcmd, "econ7"))
	{
		
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting econ "
						  "flags.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if ((ind = index_lookup (econ_flags, buf)) == -1)
		{
			ch->send_to_char ("No such econ7 flag.\n");
			return;
		}
		
		TOGGLE (edit_mob->mob->shop->econ_flags7, 1 << ind);
	}
	
	else if (!str_cmp (subcmd, "shop"))
	{
		
		argument = one_argument (argument, buf);
		
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting a shop."
						  "\n");
			return;
		}
		
		if (*buf != '-' && !isdigit (*buf))
		{
			ch->send_to_char ("Expected a room vnum or -1 (any room) after "
						  " the 'shop' keyword.\n");
			return;
		}
		
		if (atoi (buf) > 0 && !vtor (atoi (buf)))
		{
			ch->send_to_char ("The shop room vnum specified doesn't exist.\n");
			return;
		}
		
		edit_mob->mob->shop->shop_vnum = atoi (buf);
	}
	
	else if (!str_cmp (subcmd, "store"))
	{
		
		argument = one_argument (argument, buf);
		
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before setting a store."
						  "\n");
			return;
		}
		
		if (*buf != '-' && !isdigit (*buf))
		{
			ch->send_to_char ("Expected a room vnum or -1 (none) after "
						  " the 'store' keyword.\n");
			return;
		}
		
		if (atoi (buf) > 0 && !vtor (atoi (buf)))
		{
			ch->send_to_char ("The store room vnum specified doesn't exist.\n");
			return;
		}
		
		edit_mob->mob->shop->store_vnum = atoi (buf);
	}
	
	else if (!str_cmp (subcmd, "delivery"))
	{
		
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before using delivery."
						  "\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		p = one_argument (buf, buf2);
		
		do
		{
			if (!isdigit (*buf2))
			{
				error = 1;
				sprintf (buf, "%d is not a delivery object vnum.\n",
						 atoi (buf2));
				ch->send_to_char (buf);
				return;
			}
			
			if (!vtoo ((j = atoi (buf2))))
			{
				error = 1;
				sprintf (buf, "%d is not defined.\n", atoi (buf2));
				ch->send_to_char (buf);
				return;
			}
			
			for (ind = -1, i = MAX_DELIVERIES - 1; i >= 0; i--)
			{
				
				if (edit_mob->mob->shop->delivery[i] == -1)
					edit_mob->mob->shop->delivery[i] = 0;
				
				if (!edit_mob->mob->shop->delivery[i])
					ind = i;
				
				else if (edit_mob->mob->shop->delivery[i] == j)
				{
					ind = -2;
					return;
				}
			}
			
			if (ind == -2)
				edit_mob->mob->shop->delivery[i] = 0;
			else if (i == -1 && ind == -1)
				ch->send_to_char ("Delivery table is full, sorry.\n");
			else if (i == -1 && ind != -1)
				edit_mob->mob->shop->delivery[ind] = j;
			
			p = one_argument (p, buf2);
			
		}
		while (*buf2);
	}
	
	else if (!str_cmp (subcmd, "newbie"))
	{
		if (!IS_SET (edit_mob->plr_flags, NEWBIE))
		{
			edit_mob->plr_flags |= NEWBIE;
			ch->send_to_char ("Newbie bit enabled.\n");
		}
		else
		{
			edit_mob->plr_flags &= ~NEWBIE;
			ch->send_to_char ("Newbie bit removed.\n");
		}
	}
	
	else if (!str_cmp (subcmd, "allskills"))
	{
		for (skill_map_it = skill_data_map.begin(); skill_map_it != skill_data_map.end(); skill_map_it++)
		{
			tskill = skill_map_it->second;
			skill_name = strdup(tskill->skill_name.c_str());
			edit_mob->skill_map[skill_name] = 100;
		}
		
	}
	
	else if (!str_cmp (subcmd, "noskills"))
	{
		for (skill_map_it = skill_data_map.begin(); skill_map_it != skill_data_map.end(); skill_map_it++)
		{
			tskill = skill_map_it->second;
			skill_name = strdup(tskill->skill_name.c_str());
			if(!strcmp("Westron", skill_name))
			{
				ch->send_to_char ("NOTE:  Leaving Westron set.\n");
				continue;
			}

			else
				edit_mob->skill_map[skill_name] = 0;
		}
		
		
	}
	
	else if (!str_cmp (subcmd, "speaks") || !str_cmp (subcmd, "speak"))
	{
		std::map<std::string, SKILL_DATA*>::iterator found_map_skill;
		argument = one_argument (argument, subcmd);
		
		found_map_skill = skill_data_map.find(subcmd);
		if (found_map_skill == skill_data_map.end())
		{
			ch->send_to_char ("Unknown language.\n");
			return;
		}
		
		if (edit_mob->skill_map[subcmd] < 1)
			ch->send_to_char ("NOTE:  Mob doesn't know that language.\n");
		
		edit_mob->speaks = strdup(subcmd);
	}

	/*** TODO: adjsut this to work with object_material_map
	else if (!str_cmp (subcmd, "material")
			 || !str_cmp (subcmd, "materials"))
	{
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("That mob isn't a shopkeeper.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		p = one_argument (buf, buf2);
		
		if (!*buf2)
		{
			ch->send_to_char ("Which materials did you wish to set?\n");
			return;
		}
		
		
		for (material_type_it = object_material_map.begin(); material_type_it != object_material_map.end(); material_type_it++)
		{
			if (!material_type_it->second.compare(buf2))
			{
				found_material = true;
				if (!edit_mob->mob->shop->materials.empty())
				{
					for (materials_it = edit_mob->mob->shop->materials.begin();
						 materials_it != edit_mob->mob->shop->materials.end();
						 materials_it++)
					{
						if (material_type_it->second == *materials_it)
						{
							edit_mob->mob->shop->materials.remove(*materials_it);
						}
						else
						{
							edit_mob->mob->shop->materials.push_front(material_type_it->second);
						}
						
					}
				}
				else
				{
					edit_mob->mob->shop->materials.push_front(material_type_it->second);
				}
				
			}
			
		}
		 
		
		if (!found_material)
		{
			ch->send_to_char("That is not a valid material.\n");
			return;
		}
		else
		{
			ch->send_to_char("Material set.\n");
			return;
		}
				
		
	}
*******/
		
	
	else if (!str_cmp (subcmd, "trades") || !str_cmp (subcmd, "tradesin"))
	{
		
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("That mob isn't a shopkeeper.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		p = one_argument (buf, buf2);
		
		if ((ind = index_lookup (item_types, buf2)) == -1)
			{
				sprintf (buf, "Unknown item-type: %s\n", buf2);
				ch->send_to_char (buf);
				return;
			}
			
			for (j = -1, i = MAX_TRADES_IN - 1; i >= 0; i--)
			{
				
				if (!edit_mob->mob->shop->trades_in[i])
					j = i;
				
				else if (edit_mob->mob->shop->trades_in[i] == ind)
				{
					edit_mob->mob->shop->trades_in[i] = 0;
					break;
				}
			}
			
			if (j == -1 && i == -1)
			{
				sprintf (buf, "Trades table full, sorry: %s\n", buf2);
				ch->send_to_char (buf);
				return;
			}
			
			else if (j != -1 && i == -1)
				edit_mob->mob->shop->trades_in[j] = ind;
	}
	
	else if (!str_cmp (subcmd, "access"))
	{
		
		if (!IS_NPC (edit_mob))
		{
			ch->send_to_char ("You can't set that on a PC.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		p = one_argument (buf, buf2);
		
		
			if ((ind = index_lookup (room_bits, buf2)) == -1)
			{
				sprintf (buf, "Unknown room-flag: %s\n", buf2);
				ch->send_to_char (buf);
				return;
			}
			
			if (IS_SET (edit_mob->mob->access_flags, 1 << ind))
				edit_mob->mob->access_flags &= ~(1 << ind);
			else
				edit_mob->mob->access_flags |= (1 << ind);

	}
	
	else if (!str_cmp (subcmd, "noaccess"))
	{
		
		if (!IS_NPC (edit_mob))
		{
			ch->send_to_char ("You can't set that on a PC.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		p = one_argument (buf, buf2);
		
			if ((ind = index_lookup (room_bits, buf2)) == -1)
			{
				sprintf (buf, "Unknown room-flag: %s\n", buf2);
				ch->send_to_char (buf);
				return;
			}
			
			if (IS_SET (edit_mob->mob->noaccess_flags, 1 << ind))
				edit_mob->mob->noaccess_flags &= ~(1 << ind);
			else
				edit_mob->mob->noaccess_flags |= (1 << ind);
			
		
	}
	
	/** TODO: re-work the encforcemtn sytem - not currently supported
	else if (!str_cmp (subcmd, "jailer"))
	{
		if (IS_SET (edit_mob->mob->profession, PROF_JAILER))
		{
			edit_mob->mob->profession &= ~PROF_JAILER;
			ch->send_to_char ("Creature is no longer flagged as a jailer.\n",
						  ch);
		}
		else
		{
			edit_mob->mob->profession |= PROF_JAILER;
			ch->send_to_char
			("Creature is now flagged as a jailer. Be SURE to set\n"
			 "the cell1, cell2, and cell3 values.\n");
		}
	}
	
	else if (!str_cmp (subcmd, "jail"))
	{
		argument = one_argument (argument, buf);
		
		if (*buf == '0' || (atoi (buf) && atoi (buf) >= 0))
		{
			
			sprintf (buf2, "Old jail VNUM: %d.\nNew jail VNUM: %d.\n",
					 edit_mob->mob->jail, atoi (buf));
			edit_mob->mob->jail = atoi (buf);
			ch->send_to_char (buf2);
		}
		
		else if (atoi (buf) && atoi (buf) < 0)
		{
			ch->send_to_char ("Expected a value of at least 0.\n");
			return;
		}
		
		else
		{
			ch->send_to_char
			("Expected the integer value of the target room's VNUM.\n");
			return;
		}
		
	}
	
	 ***********/
	
	else if (!str_cmp (subcmd, "hits") || !str_cmp (subcmd, "hp"))
	{
		argument = one_argument (argument, buf);
		
		if (*buf == '0' || atoi (buf))
		{
			edit_mob->hit = atoi (buf);
			if (IS_NPC (edit_mob))
				edit_mob->max_hit = edit_mob->hit;
		}
		else
		{
			ch->send_to_char ("Expected hitpoints value.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "bzone"))
	{
		argument = one_argument (argument, buf);
		
		if (*buf == '0' || atoi (buf))
		{
			edit_mob->mob->zone = atoi (buf);
		}
		else
		{
			ch->send_to_char ("Expected Build Zone value.\n");
			return;
		}
	}
	
		//this one needs further processing before leaving do_mset
	else if (!str_cmp (subcmd, "desc") ||
			 !str_cmp (subcmd, "descr") || !str_cmp (subcmd, "description"))
	{
		full_description = 1;
		
	}
	
	else if (!str_cmp (subcmd, "intoxication"))
	{
		
		argument = one_argument (argument, buf);
		
		if (!just_a_number (buf))
		{
			ch->send_to_char ("Expected ... intoxication <num>\n");
			return;
		}
		
		edit_mob->intoxication = atoi (buf);
	}
	
	else if (!str_cmp (subcmd, "hunger"))
	{
		
		argument = one_argument (argument, buf);
		
		if (!just_a_number (buf) && *buf != '-')
		{
			ch->send_to_char ("Expected ... hunger <num>\n");
			return;
		}
		
		edit_mob->hunger = atoi (buf);
	}
	
	else if (!str_cmp (subcmd, "thirst"))
	{
		
		argument = one_argument (argument, buf);
		
		if (!just_a_number (buf) && *buf != '-')
		{
			ch->send_to_char ("Expected ... thirst <num>\n");
			return;
		}
		
		edit_mob->thirst = atoi (buf);
	}
	
	else if (!str_cmp (subcmd, "conds"))
	{
		for (i = 0; i < 3; i++)
		{
			argument = one_argument (argument, buf);
			
			if (!isdigit (*buf) && !atoi (buf))
			{
				ch->send_to_char ("You should enter 3 numbers after "
							  "conds.\n");
				ch->send_to_char ("Note:  Use mset intoxication, hunger, "
							  "thirst instead.\n");
				return;
			}
			
			if (i == 0)
				edit_mob->intoxication = atoi (buf);
			else if (i == 1)
				edit_mob->hunger = atoi (buf);
			else
				edit_mob->thirst = atoi (buf);
		}
		
		if (!isdigit (*buf))
			return;
	}
	
	else if (!str_cmp (subcmd, "age"))
	{
		
		if (!IS_NPC (edit_mob))
		{
			ch->send_to_char("This only works on NPCs.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (!isdigit (*buf))
		{
			ch->send_to_char ("Age must be a number.\n");
			return;
		}
		
		edit_mob->age = atoi (buf);
	}
	
	else if (!str_cmp (subcmd, "travel"))
	{
		if (!*argument)
		{
			ch->send_to_char ("The travel string expected after \'travel\'.\n");
			return;
		}
		
		edit_mob->travel_str = duplicateString (argument);
		
	}
	
	else if (!str_cmp (subcmd, "voice"))
	{
		if (!*argument)
		{
			ch->send_to_char ("The voice string expected after \'voice\'.\n");
			return;
		}
		
		edit_mob->voice_str = duplicateString (argument);
		
	}
	
	
	else if (!str_cmp (subcmd, "room"))
	{
		
		if (IS_NPC (edit_mob))
		{
			ch->send_to_char
			("This only works on PCs.  For an NPC, switch into the target mob, then\nuse GOTO.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (!isdigit (*buf) || !vtor (atoi (buf)))
		{
			ch->send_to_char ("No such room.\n");
			return;
		}
		
		if (edit_mob->room)
		{
			edit_mob->char_from_room ();
			edit_mob->char_to_room (atoi (buf));
		}
		else
			edit_mob->in_room = atoi (buf);
	}
	
	
	else if (!str_cmp (subcmd, "nobleed"))
	{
		if (IS_NPC(edit_mob) && IS_SET (edit_mob->mob->action, ACT_NOBLEED))
		{
			ch->send_to_char ("This mobile will now bleed from heavy wounds.\n");
			edit_mob->mob->action &= ~ACT_NOBLEED;
		}
		else
		{
			edit_mob->mob->action |= ACT_NOBLEED;
			ch->send_to_char
			("This mobile will no longer bleed from heavy wounds.\n");
		}
	}
	
	else if (!str_cmp (subcmd, "aggro"))
	{
		if (IS_NPC(edit_mob) && IS_SET (edit_mob->mob->action, ACT_AGGRESSIVE))
		{
			ch->send_to_char ("This mobile will no longer act aggressively.\n");
			edit_mob->mob->action &= ~ACT_AGGRESSIVE;
		}
		else
		{
			edit_mob->mob->action |= ACT_AGGRESSIVE;
			ch->send_to_char
			("This mobile will now act aggressively.\n");
		}
	}
	
	else if (!str_cmp (subcmd, "pursue"))
	{
		if (IS_NPC(edit_mob) && IS_SET (edit_mob->mob->action, ACT_PURSUE))
		{
			ch->send_to_char ("This mobile will not pursue victims.\n");
			edit_mob->mob->action &= ~ACT_PURSUE;
		}
		else
		{
			edit_mob->mob->action |= ACT_PURSUE;
			ch->send_to_char
			("This mobile will pursue victims.\n");
		}
	}
	
	else if (!str_cmp (subcmd, "memory"))
	{
		if (IS_NPC(edit_mob) && IS_SET (edit_mob->mob->action, ACT_MEMORY))
		{
			ch->send_to_char ("This mobile will not remember who he is fighting.\n");
			edit_mob->mob->action &= ~ACT_MEMORY;
		}
		else
		{
			edit_mob->mob->action |= ACT_MEMORY;
			ch->send_to_char
			("This mobile will remember who he is fighting.\n");
		}
	}
	
	else if (!str_cmp (subcmd, "nocommand"))
	{
		if (IS_NPC(edit_mob) && IS_SET (edit_mob->mob->action, ACT_NOCOMMAND))
		{
			ch->send_to_char ("This mobile will now accept commands.\n");
			edit_mob->mob->action &= ~ACT_NOCOMMAND;
		}
		else
		{
			edit_mob->mob->action |= ACT_NOCOMMAND;
			ch->send_to_char
			("This mobile will not accept commands.\n");
		}
	}
	
	else if (!str_cmp (subcmd, "novnpc"))
	{
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before using allowing vNPC sales."
						  "\n");
			return;
		}
		
		if (IS_NPC(edit_mob) && IS_SET (edit_mob->mob->action, ACT_NOVNPC))
		{
			ch->send_to_char ("This shopkeeper will not have vNPC sales.\n");
			edit_mob->mob->action &= ~ACT_NOVNPC;
		}
		else
		{
			edit_mob->mob->action |= ACT_NOVNPC;
			ch->send_to_char
			("This mobile will now receive vNPC sales.\n");
		}
	}

	
	
	else if (!str_cmp (subcmd, "noorder"))
	{
		if (!edit_mob->mob->shop)
		{
			ch->send_to_char ("Assign the KEEPER flag before using allowing vNPC sales."
						  "\n");
			return;
		}
		if (IS_NPC(edit_mob) && IS_SET (edit_mob->mob->action, ACT_NOORDER))
		{
			ch->send_to_char ("This shopkeeper will no longer accept orders.\n");
			edit_mob->mob->action &= ~ACT_NOORDER;
		}
		else
		{
			edit_mob->mob->action |= ACT_NOORDER;
			ch->send_to_char
			("This shopkeeper will now accept orders.\n");
		}
	}
	
	else if (!str_cmp (subcmd, "wimpy"))
	{
		if (IS_NPC(edit_mob) && IS_SET (edit_mob->mob->action, ACT_WIMPY))
		{
			ch->send_to_char ("This mobile is no longer a coward.\n");
			edit_mob->mob->action &= ~ACT_WIMPY;
		}
		else
		{
			edit_mob->mob->action |= ACT_WIMPY;
			ch->send_to_char
			("This mobile is a coward and will run from battles.\n");
		}
	}
	
	else if (!str_cmp (subcmd, "pcowned"))
	{
		if (IS_NPC(edit_mob) && IS_SET (edit_mob->mob->action, ACT_PCOWNED))
		{
			ch->send_to_char ("This mobile is can no longer be owned by a PC.\n");
			edit_mob->mob->action &= ~ACT_PCOWNED;
		}
		else
		{
			edit_mob->mob->action |= ACT_PCOWNED;
			ch->send_to_char
			("This mobile can now be owned by a PC.\n");
		}
	}
	
	else if (!str_cmp (subcmd, "stealthy"))
	{
		if (IS_NPC(edit_mob) && IS_SET (edit_mob->mob->action, ACT_STEALTHY))
		{
			ch->send_to_char ("This mobile is can no longer be stealthy.\n");
			edit_mob->mob->action &= ~ACT_STEALTHY;
		}
		else
		{
			edit_mob->mob->action |= ACT_STEALTHY;
			ch->send_to_char
			("This mobile can now be stealthy.\n");
		}
	}
	
	else if (!str_cmp (subcmd, "passive"))
	{
		if (IS_NPC(edit_mob) && IS_SET (edit_mob->mob->action, ACT_PASSIVE))
		{
			ch->send_to_char ("This mobile will not help allies in combat.\n");
			edit_mob->mob->action &= ~ACT_PASSIVE;
		}
		else
		{
			edit_mob->mob->action |= ACT_PASSIVE;
			ch->send_to_char
			("This mobile will now aid his comrades in combat.\n");
		}
	}
	
		
	else if (!str_cmp (subcmd, "flying"))
	{
		if (IS_NPC(edit_mob) && IS_SET (edit_mob->mob->action, ACT_FLYING))
		{
			ch->send_to_char ("This mobile will no longer fly.\n");
			edit_mob->mob->action &= ~ACT_FLYING;
		}
		else
		{
			edit_mob->mob->action |= ACT_FLYING;
			ch->send_to_char ("This mobile will now fly.\n");
		}
	}
	
		//Supported Professions
	else if (!str_cmp (subcmd, "physician"))
	{
		if (IS_SET (edit_mob->mob->profession, PROF_PHYSICIAN))
		{
			ch->send_to_char ("This mobile will no longer treat wounds.\n");
			edit_mob->mob->profession &= ~PROF_PHYSICIAN;
		}
		else
		{
			edit_mob->mob->profession |= PROF_PHYSICIAN;
			ch->send_to_char ("This mobile will now treat wounds.\n");
		}
	}
	
	else if (!str_cmp (subcmd, "prey"))
	{
		if (IS_SET (edit_mob->mob->profession, PROF_PREY))
		{
			ch->send_to_char
			("This mobile will no longer flee from any approaching creature.\n");
			edit_mob->mob->profession &= ~PROF_PREY;
		}
		else
		{
			ch->send_to_char
			("This mobile will now flee from any approaching creature.\n");
			edit_mob->mob->profession |= PROF_PREY;
		}
	}
	
	else if (!str_cmp (subcmd, "repairman"))
	{
		if (IS_SET (edit_mob->mob->profession, PROF_REPAIR))
		{
			ch->send_to_char ("This mobile will no longer repair items.\n");
			edit_mob->mob->profession &= ~PROF_REPAIR;
		}
		else
		{
			edit_mob->mob->profession |= PROF_REPAIR;
			ch->send_to_char ("This mobile will now repair items.\n");
		}
	}
	
	else if (!str_cmp (subcmd, "sentinel"))
	{
		if (IS_SET (edit_mob->mob->profession, PROF_SENTINEL))
		{
			ch->send_to_char ("This mobile will no longer remain in this location and may wander.\n");
			edit_mob->mob->profession &= ~PROF_SENTINEL;
		}
		else
		{
			edit_mob->mob->profession |= PROF_SENTINEL;
			ch->send_to_char ("This mobile will now stay in this spot and not wander away.\n");
		}
	}
		//end of supported Professions
	
	else if (!str_cmp (subcmd, "nobind"))
	{
		if (IS_NPC(edit_mob) && IS_SET (edit_mob->mob->action, ACT_NOBIND))
		{
			ch->send_to_char ("This mobile will now bind its own wounds.\n");
			edit_mob->mob->action &= ~ACT_NOBIND;
		}
		else
		{
			edit_mob->mob->action |= ACT_NOBIND;
			ch->send_to_char
			("This mobile will no longer bind its own wounds.\n");
		}
	}
	
	else if (!str_cmp (subcmd, "moves"))
	{
		argument = one_argument (argument, buf);
		
		if (*buf == '0' || atoi (buf))
		{
			edit_mob->move = atoi (buf);
		}
		else
		{
			ch->send_to_char ("Expected moves value.\n");
			return;
		}
	}
	
	
	else if (!str_cmp (subcmd, "maxmoves"))
	{
		argument = one_argument (argument, buf);
		
		if (*buf == '0' || atoi (buf))
		{
			edit_mob->max_move = atoi (buf);
		}
		else
		{
			ch->send_to_char ("Expected moves value.\n");
			return;
		}
	}
	
	else if (ch->get_trust() >= 4 && !str_cmp (subcmd, "delete"))
	{
		
		if (!IS_NPC (edit_mob))
		{
			ch->send_to_char ("NPC only setting:  delete\n");
			return;
		}
		
		if (edit_mob->deleted)
		{
			edit_mob->deleted = 0;
			ch->send_to_char ("This mobile is no longer marked for deletion."
						  "\n");
			return;
		}
		
		if (IS_NPC(edit_mob))
		{
			loads = 0;
			
			for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
			{
				tch = *tch_iterator;
				
				if (tch->deleted)
					continue;
				
				if (!IS_NPC (tch))
					continue;
				
				if (tch->mob->nVirtual == edit_mob->mob->nVirtual)
					loads++;
			}
		}
		
			//if (tch)
		if(loads > 0)
		{
			ch->send_to_char ("Clear this mobile from the world first.\n");
			return;
		}
		
		ch->send_to_char ("WARNING:  This mobile is marked for deletion.  "
					  "However, the prototype\n"
					  "          cannot be removed until the mud is rebooted.\n"
					  "          Use the DELETE option again to undo "
					  "deletion.\n");
		
		edit_mob->deleted = 1;
	}
	
		
	else if (!str_cmp (subcmd, "str"))
	{
		argument = one_argument (argument, buf);
		
		if (atoi (buf) < 1)
		{
			ch->send_to_char ("Expected a positive value.\n");
			return;
		}
		
		if (*buf == '0' || atoi (buf))
		{
			delta = edit_mob->str - atoi (buf);
			edit_mob->str = atoi (buf);
			edit_mob->tmp_str -= delta;
		}
		else
		{
			ch->send_to_char ("Expected a value for str.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "dex"))
	{
		argument = one_argument (argument, buf);
		
		if (atoi (buf) < 1)
		{
			ch->send_to_char ("Expected a positive value.\n");
			return;
		}
		
		if (*buf == '0' || atoi (buf))
		{
			delta = edit_mob->dex - atoi (buf);
			edit_mob->dex = atoi (buf);
			edit_mob->tmp_dex -= delta;
		}
		else
		{
			ch->send_to_char ("Expected a value for dex.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "con"))
	{
		argument = one_argument (argument, buf);
		
		if (atoi (buf) < 1)
		{
			ch->send_to_char ("Expected a positive value.\n");
			return;
		}
		
		if (*buf == '0' || atoi (buf))
		{
			delta = edit_mob->con - atoi (buf);
			edit_mob->con = atoi (buf);
			edit_mob->tmp_con -= delta;
		}
		else
		{
			ch->send_to_char ("Expected a value for con.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "int"))
	{
		argument = one_argument (argument, buf);
		
		if (atoi (buf) < 1)
		{
			ch->send_to_char ("Expected a positive value.\n");
			return;
		}
		
		if (*buf == '0' || atoi (buf))
		{
			delta = edit_mob->intel - atoi (buf);
			edit_mob->intel = atoi (buf);
			edit_mob->tmp_intel -= delta;
		}
		else
		{
			ch->send_to_char ("Expected a value for int.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "pow"))
	{
		argument = one_argument (argument, buf);
		
		if (ch->get_trust() >= 5) {
			if (atoi (buf) < 1)
			{
				ch->send_to_char ("Expected a positive value.\n");
				return;
			}
			
			if (*buf == '0' || atoi (buf))
			{
				delta = edit_mob->aur - atoi (buf);
				edit_mob->aur = atoi (buf);
				edit_mob->tmp_aur -= delta;
			}
			else
			{
				ch->send_to_char ("Expected a value for aur.\n");
				return;
			}
		}
		else {
			ch->send_to_char("Your admin level is not high enough to edit power.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "agi"))
	{
		argument = one_argument (argument, buf);
		
		if (atoi (buf) < 1)
		{
			ch->send_to_char ("Expected a positive value.\n");
			return;
		}
		
		if (*buf == '0' || atoi (buf))
		{
			delta = edit_mob->agi - atoi (buf);
			edit_mob->agi = atoi (buf);
			edit_mob->tmp_agi -= delta;
		}
		else
		{
			ch->send_to_char ("Expected a value for agi.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "luk"))
	{
		argument = one_argument (argument, buf);
		
		if (atoi (buf) < 1)
		{
			ch->send_to_char ("Expected a positive value.\n");
			return;
		}
		
		if (*buf == '0' || atoi (buf))
		{
			delta = edit_mob->luk - atoi (buf);
			edit_mob->luk = atoi (buf);
			edit_mob->tmp_luk -= delta;
		}
		else
		{
			ch->send_to_char ("Expected a value for luk.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "wil"))
	{
		argument = one_argument (argument, buf);
		
		if (*buf == '0' || atoi (buf))
		{
			delta = edit_mob->wil - atoi (buf);
			edit_mob->wil = atoi (buf);
			edit_mob->tmp_wil -= delta;
		}
		else
		{
			ch->send_to_char ("Expected a value for wil.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "dam"))
	{
		
		if (!IS_NPC (edit_mob))
		{
			ch->send_to_char ("NPC only setting:  dam\n");
			return;
		}
		
		bonus = 0;
		
		argument = one_argument (argument, buf);
		
		parms = sscanf (buf, "%d%[dD]%d%d", &dice, &c, &sides, &bonus);
		
		if (parms < 3)
		{
			ch->send_to_char ("The dam parameter format is <number>d<number>+/-<number>.\n");
			return;
		}
		
		edit_mob->mob->damnodice = dice;
		edit_mob->mob->damsizedice = sides;
		edit_mob->mob->damroll = bonus;
	}
	
	else if (!str_cmp (subcmd, "armor") || !str_cmp (subcmd, "ac"))
	{
		argument = one_argument (argument, buf);
		
		if (*buf == '0' || atoi (buf))
			edit_mob->armor = atoi (buf);
		else
		{
			ch->send_to_char ("Expected armor/ac value.\n");
			return;
		}
	}
	
	
	
	else if (!str_cmp (subcmd, "autoflee"))
	{			/* Need {}'s for macro */
		TOGGLE (edit_mob->flags, FLAG_AUTOFLEE);
	}
	
	else if ((ind = index_lookup (sex_types, subcmd)) != -1)
		edit_mob->sex = ind;
	
	else if (!str_cmp (subcmd, "variable"))
	{
		if (ch->get_trust() < 4)
		{
			ch->send_to_char
			("You must be a level four admin to set this bit.\n");
			return;
		}
		if (!IS_SET (edit_mob->flags, FLAG_VARIABLE))
		{
			edit_mob->flags |= FLAG_VARIABLE;
			ch->send_to_char ("This mob has been saved and will now randomize at load-up.\n\n");
						
			if (ch->pc->edit_mob)
			{
				save_mysql_mob (ch, edit_mob, BUILD_APPROVED); 
			}
			
			ch->pc->edit_mob = 0;
			return;
		}
		else
		{
			edit_mob->flags &= ~FLAG_VARIABLE;
			ch->send_to_char ("This mob will no longer randomize at load-up.\n");
			return;
		}
	}
	
	else if (!str_cmp (subcmd, "isadmin"))
	{
		if (ch->get_trust() < 5)
		{
			ch->send_to_char ("You'll need a level 5 admin to set this bit.\n");
			return;
		}
		if (IS_NPC (edit_mob) || edit_mob->get_trust())
		{
			ch->send_to_char
			("This bit is meant only to be set on mortal PCs.\n");
			return;
		}
		if (IS_SET (edit_mob->flags, FLAG_ISADMIN))
			edit_mob->flags &= ~FLAG_ISADMIN;
		else
			edit_mob->flags |= FLAG_ISADMIN;
		ch->send_to_char ("IsAdminPC flag toggled on character.\n");
		return;
	}
	
	else if (!str_cmp (subcmd, "race"))
	{
		
		argument = one_argument (argument, buf);
		
		if ((ind = lookup_race_id (buf)) == -1)
			ch->send_to_char ("That race isn't in the database, I'm afraid.\n");
		else
			edit_mob->race = ind;
		
		argument = one_argument (argument, buf);
		
		if (!str_cmp (buf, "defaults") && ind <= LAST_PC_RACE)
		{
			randomize_mobile (edit_mob);
			ch->send_to_char
			("Stats and skills re-initialized to race defaults.\n");
			return;
		}
		
	}
	
	else if (!str_cmp (subcmd, "frame"))
	{
		
		argument = one_argument (argument, buf);
		
		ind = index_lookup (frames, buf);
		
		if ((ind == -1))
		{
			ch->send_to_char ("Expected a frame: scant, light, ... \n");
			return;
		}
		
		edit_mob->frame = ind;
	}
	
	else if (!str_cmp (subcmd, "height"))
	{
		
		argument = one_argument (argument, buf);
		
		if (!isdigit (*buf))
		{
			ch->send_to_char ("Expected height.\n");
			return;
		}
		
		edit_mob->height = atoi (buf);
	}
	
	else if (!str_cmp (subcmd, "dpos"))
	{
		
		argument = one_argument (argument, buf);
		
		if (!*buf)
		{
			ch->send_to_char ("Expected mob position after dpos.\n");
			return;
		}
		
		if ((ind = index_lookup (position_types, buf)) == -1)
		{
			ch->send_to_char ("Invalid position.\n");
			return;
		}
		
		edit_mob->default_pos = ind;
	}
	
	else if (!str_cmp (subcmd, "name"))
	{
		if (!*argument)
		{
			ch->send_to_char ("Expected a name, and keywords after name.\n");
			return;
		}
		
		if (!IS_NPC (edit_mob) && !isname (edit_mob->name, argument))
		{
			ch->send_to_char ("MAKE SURE THE PLAYERS REAL NAME IS INCLUDED!!!\n"
						  "You don't realize how many problems you are trying to create!\n");
			return;
		}
		
		edit_mob->keywords = duplicateString (argument);
	}
	
	else if (!str_cmp (subcmd, "clanadd"))
	{
		if (!*argument)
		{
			ch->send_to_char ("Expected a clan name, and optional rank.\n");
			return;
		}
		
		argument = one_argument(argument, clanname);
		if (lookup_clan_id(clanname) < 0)
		{
			ch->send_to_char ("That clan does not exist!\n");
			return;
		}
		
		char_clan_add(edit_mob, clanname, argument);
	}
	
	else if (!str_cmp (subcmd, "clanremove"))
	{
		if (!*argument)
		{
			ch->send_to_char ("Expected a clan name.\n");
			return;
		}
				
		if (lookup_clan_id(argument) < 0)
		{
			ch->send_to_char ("That clan does not exist!\n");
			return;
		}
		
		char_clan_remove(edit_mob, argument);
	}
	
	else if (!str_cmp (subcmd, "sdesc"))
	{
				
		if (!*argument)
		{
			ch->send_to_char ("The short description is expected after \'sdesc\'.\n");
			return;
		}
		
		edit_mob->short_descr = duplicateString (argument);
	}
	
	else if (!str_cmp (subcmd, "ldesc"))
	{
			
		if (!*argument)
		{
			ch->send_to_char ("The long description expected after \'ldesc\'.\n");
			return;
		}
		
		edit_mob->long_descr = duplicateString (argument);
	}
	
	else if (!str_cmp (subcmd, "archive"))
	{
		
		if (ch->get_trust() < 5)
		{
			ch->send_to_char ("Only level 5 can archive pfiles.\n");
			return;
		}
		
		if (IS_NPC (edit_mob))
		{
			ch->send_to_char ("You can't archive a mobile.");
			return;
		}
		
		if (edit_mob->pc->load_count != 1)
		{
			ch->send_to_char
			("Pfile is being accessed by more than just you.\n");
			ch->send_to_char
			("Player is online or someone else has him/her mob'ed.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		
		if (str_cmp (buf, edit_mob->name))
		{
			ch->send_to_char ("You are trying to archive a pfile.\n");
			ch->send_to_char
			("If you were trying to archive the pfile for 'Marvin', you would type:\n\n   mset archive marvin\n");
			return;
		}
		
		edit_mob->unload_pc();
		edit_mob = NULL;
		ch->pc->edit_player = NULL;
		
		*buf = toupper (*buf);
		
		sprintf (subcmd, "save/player/archive/%s", buf);
		sprintf (buf2, "save/player/%c/%s", tolower (*buf), buf);
		
		if ((error = rename (buf2, subcmd)))
		{
			perror ("archive");
			sprintf (buf, "Failed, system error %d.\n", error);
			ch->send_to_char (buf);
			return;
		}
		
		ch->send_to_char ("Pfile archived.\n");
		
		return;
	}
	
	else if (!str_cmp (subcmd, "save"))
	{
		ch->send_to_char ("The changes you made have been saved to the database.\n");
		
		if (ch->pc->edit_mob)
		{
			save_mysql_mob (ch, edit_mob, BUILD_APPROVED); 
		}
		
		if (ch->pc->edit_player)
		{
			save_char (ch->pc->edit_player, true);
			(ch->pc->edit_player)->unload_pc();
			ch->pc->edit_player = NULL;
		}
		
		ch->pc->edit_mob = 0;
		return;
	}
	
		
	else if (!str_cmp (subcmd, "position"))
	{	
		argument = one_argument (argument, buf);
		
		ind = index_lookup (position_types, buf);
		if (ind > 0)
			edit_mob->position = ind;
		
		if (edit_mob->default_pos == 0)
			edit_mob->default_pos = ind;
		
	}
	
	else if (!str_cmp (subcmd, "skill"))
	{	
		argument = one_argument (argument, buf);
		ind = lookup_skill_id(buf);
		if (ind < 0)
			return;
	
		argument = one_argument (argument, buf);
		if (isdigit (*buf))
		{
			if (atoi (buf) < 0)
			{
				ch->send_to_char ("Please specify a positive value.\n");
				return;
			}
			open_skill (edit_mob, ind);
			
				//subcmd could be lowercase, so we need to use the id number
			edit_mob->skill_map[lookup_skill_name(ind)] = atoi (buf);
			
			
		}
		else
		{
			ch->send_to_char
			("Expected a value after the skill, from 0 to 100.\n");
			return;
		}
		
	}
	
	else if (!strcmp (subcmd, "stayput"))
	{
		
		if (IS_NPC(edit_mob) && IS_SET (edit_mob->mob->action, ACT_STAYPUT))
		{
			edit_mob->mob->action &= ~ACT_STAYPUT;
			ch->send_to_char ("Mob is now stayput.\n");
		}
		else
		{
			edit_mob->mob->action  |= ACT_STAYPUT;
			ch->send_to_char ("Mob is no longer stayput.\n");
		}
	}
	
	else if (!strcmp (subcmd, "affect"))
	{
		argument = one_argument(argument, buf);
		
		ind = index_lookup (affected_bits, buf);
	
		if (ind < 0)
			return;
		
		if(ch->get_trust() > 3)
		{
			if (IS_SET (edit_mob->affected_by, 1 << ind))
			{
				ch->send_to_char ("Affect has been removed.\n");
				edit_mob->affected_by &= ~(1 << ind);
			}
			else
			{
				ch->send_to_char ("Affect has been added.\n");
				edit_mob->affected_by |= (1 << ind);
			}
		}
		else
		{
			ch->send_to_char ("You need to be level 4 or higher to set Affects.\n");
			return;
			
		}
		
	}
	
	else if (!strcmp (subcmd, "speed"))
	{
		argument = one_argument(argument, buf);
		
		ind = index_lookup (verbal_speeds, buf);
	
		if (ind < 0)
			return;
		
		edit_mob->speed = ind;
	}

	
	else
	{
		sprintf (buf, "Unknown keyword: %s\n", subcmd);
		ch->send_to_char (buf);
		return;
	}
	
	
	if (!IS_NPC (edit_mob))
		ch->send_to_char ("Player modified.\n");
	
	else if ((i = redefine_mobiles (edit_mob)) > 0)
	{
		sprintf (buf, "%d mobile(s) in world redefined.\n", i);
		ch->send_to_char (buf);
	}
	
	
	 if (full_description)
	 {
		 free_mem(ch->desc->descStr);
		 free_mem(ch->desc->pending_message);
		 ch->desc->pending_message = new MESSAGE_DATA;
		 ch->desc->descStr = ch->desc->pending_message->message;
		 ch->desc->proc = post_mdesc;
		 if (IS_NPC (edit_mob))
			 ch->delay_info1 = edit_mob->mob->nVirtual;
		 else
			 ch->delay_who = duplicateString (edit_mob->name);
		 
		 
		 ch->send_to_char ("\nOld description:\n\n");
		 ch->send_to_char (edit_mob->description);
		 
		 ch->desc->max_str = STR_MULTI_LINE;
		 ch->send_to_char
		 ("\nPlease enter the new description; terminate with an '@'\n");
		 ch->make_quiet();
		 	 
		 return;
	 }
	
	return;
}


void
consider_wearing (CHAR_DATA * ch, OBJ_DATA * obj)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };

	obj_from_room (&obj, 0);

	one_argument (obj->name, buf);

	equip_char (ch, obj, obj->obj_flags.wear_flags);
}

void
do_outfit (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	char name[MAX_STRING_LENGTH]= { '\0' };
	CHAR_DATA *mob;
	OBJ_DATA *next_obj;
	OBJ_DATA *tobj;

	/****TODO: Leadership commands??
	 
	if (!IS_SET (ch->affected_by, AFF_LEADER_COMMAND)
		&& ch->get_trust() < 1)
	{
		ch->send_to_char ("You do not have approval for leadership commands");
		return;
	}
	*********/
	
	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char ("Outfit whom?\n");
		return;
	}

	if (!(mob = get_char_room_vis (ch, buf)))
	{
		ch->send_to_char ("Mobile not available for outfitting.\n");
		return;
	}

	if (mob == ch)
		ch->send_to_char ("Outfit yourself?  Ok.\n");

	for (tobj = mob->room->contents; tobj; tobj = next_obj)
	{

		next_obj = tobj->next_content;

		one_argument (tobj->name, name);

		sprintf (buf, "get %s", name);
		command_interpreter (mob, buf);

		sprintf (buf, "wear %s", name);

		command_interpreter (mob, buf);
	}
}


void
advance_spaces (char **s)
{
	while (**s && (**s == ' ' || **s == '\t' || **s == '\r'))
		(*s)++;
}

int
get_token (char **s, char *token)
{
	static int start_sentence = 0;

	if (start_sentence)
	{
		start_sentence = 0;
		return TOK_SENTENCE;
	}

	*token = '\0';

	if (**s == '\n')
		(*s)++;

	if (!**s)
		return TOK_END;

	if (**s == '\n')
		return TOK_NEWLINE;

	if (**s == ' ')
	{
		advance_spaces (s);
		return TOK_PARAGRAPH;
	}

	while (**s && **s != ' ' && **s != '\t' && **s != '\n')
	{
		if (**s == '\\' && (*s)[1] == '\'')
			(*s)++;
		*token = **s;
		start_sentence = (**s == '.');
		token++;
		(*s)++;
	}

	*token = '\0';

	advance_spaces (s);

	return TOK_WORD;
}

	//indented and 65 characters long
std::string reformat_desc (char *source)
{
	int token_value = 0;
	int first_line = 1;
	int line_len;
	char *s;
	char *r;
	char token[MAX_STRING_LENGTH]= { '\0' };
	char result[MAX_STRING_LENGTH]= { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (source == NULL)
		return("");

	if (strn_cmp (source, "   ", 3))
	{
		sprintf (buf, "   %s", source);
		source = strdup(buf);
	}

	s = source;
	r = result;
	*result = '\0';

	line_len = 0;

	while (token_value != TOK_END)
	{

		token_value = get_token (&s, token);

		if (token_value == TOK_PARAGRAPH)
		{

			if (first_line)
				first_line = 0;
			else
				strcat (result, "\n");

			strcat (result, "   ");
			line_len = 3;
			continue;
		}

		if (token_value == TOK_SENTENCE)
		{
			line_len += 1;
			continue;
		}

		if (token_value == TOK_NEWLINE)
		{
			if (line_len != 0)
				strcat (result, "\n");	/* Catch up */
			strcat (result, "\n");
			line_len = 0;
			continue;
		}

		if (token_value == TOK_WORD)
		{
			if (line_len + strlen (token) > FORMAT_DESC)
			{
				strcat (result, "\n");
				line_len = 0;
			}

			strcat (result, token);
			strcat (result, " ");

			line_len += strlen (token) + 1;
		}
	}

	if (result[strlen (result) - 1] != '\n')
		strcat (result, "\n");

	return(result);
}

	//indented and 79 characters long
void
reformat_string (char *source, char **target)
{
	int token_value = 0;
	int first_line = 1;
	int line_len;
	char *s;
	char *r;
	char token[MAX_STRING_LENGTH]= { '\0' };
	char result[MAX_STRING_LENGTH]= { '\0' };

	if (source == NULL)
		return;
	
	s = source;
	r = result;
	*result = '\0';

	line_len = 0;

	while (token_value != TOK_END)
	{

		token_value = get_token (&s, token);

		if (token_value == TOK_PARAGRAPH)
		{

			if (first_line)
				first_line = 0;
			else
				strcat (result, "\n");

			strcat (result, "   ");
			line_len = 3;
			continue;
		}

		if (token_value == TOK_SENTENCE)
		{
			line_len += 1;
			continue;
		}

		if (token_value == TOK_NEWLINE)
		{
			if (line_len != 0)
				strcat (result, "\n");
			strcat (result, "\n");
			line_len = 0;
			continue;
		}

		if (token_value == TOK_WORD)
		{
			if ((line_len + strlen (token) > FORMAT_STRING)
				&& !(*token == '#' && strlen (token) == 2))
			{
				strcat (result, "\n");
				line_len = 0;
			}

			strcat (result, token);
			strcat (result, " ");
			line_len += strlen (token) + 1;
		}
	}

	if (result[strlen (result) - 1] != '\n')
		strcat (result, "\n");

	*target = strdup(result);
}

/*******
	//returns 1 on successful deleting
int
free_room (ROOM_DATA * room)
{
	int dir;
	ROOM_DATA *troom;
	std::map<int, ROOM_DATA*>::iterator room_iterator;

	while (room->affects)
		remove_room_affect (room, room->affects->type);

	while (room->people)
		(room->people)->extract_char();

	while (room->contents)
		extract_obj (room->contents);

	for (room_iterator = room_map.begin(); room_iterator != room_map.end(); room_iterator++)
	{
		troom = room_iterator->second;
		
		for (dir = 0; dir <= LAST_DIR; dir++)
		{
			if (troom->dir_option[dir]
			&& troom->dir_option[dir]->to_room == room->nVirtual)
			{
				free_exit (troom->dir_option[dir]);
				troom->dir_option[dir] = NULL;
			}
		}
	}

	for (dir = 0; dir <= LAST_DIR; dir++)
	{
		if (room->dir_option[dir])
		{
			free_exit (room->dir_option[dir]);
			room->dir_option[dir] = NULL;
		}
	}

	if (room->name && *room->name)
	{
		free_mem (room->name);
		room->name = NULL;
	}

	if (room->description && *room->description)
	{
		free_mem (room->description);
		room->description = NULL;
	}

	return 1;
}
******/
/**
 * returns 0 for unable to delete - PC in the room, or the room doesn't exist
 * returns 1 for sucessful deletion
 * Room removed from IG map, and marked for deletion in DB
 **/
int
room_delete (ROOM_DATA * room)
{
	char buf[100];
	CHAR_DATA *tch;
	
	if (!room)
		return 0;
	
	for (tch = room->people; tch; tch = tch->next_in_room)
	{
		if (!IS_NPC (tch))
			return 0;
	}
	
	sprintf (buf, "save/rooms/%d", room->nVirtual);
	unlink (buf);
	
	room->zone = -1;
	room->deleted = 1;
	
	save_mysql_room(NULL, room, BUILD_APPROVED);
	room_map.erase(room->nVirtual);
	
	return 1;
}

void
rdelete (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int room_num;

	argument = one_argument (argument, buf);

	if (!isdigit (*buf))
	{
		ch->send_to_char ("Expected a room virtual number.\n");
		return;
	}

	room_num = atoi (buf);

	if (room_num == 0 || !vtor (room_num))
	{
		ch->send_to_char ("No such room.\n");
		return;
	}

	if (ch->in_room == room_num)
	{
		ch->send_to_char ("You cannot delete the room you are in!\n");
		return;
	}
	
	if (!room_delete (vtor (room_num)))
	{
		ch->send_to_char ("ROOM NOT DELETED!\n");
		return;
	}
	else
	{
		sprintf (buf, "Room %d removed and marked for deletion.\n", room_num);
		ch->send_to_char (buf);

	}
}


void
do_zset (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char subcmd[MAX_STRING_LENGTH]= { '\0' };
	int zone;
	std::string zone_name;
	std::string zone_lead;
	std::list<zone_data*>::iterator zone_iterator;
	zone_data *zone_it;
	MYSQL_RES *result;
	
	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char ("\n\nSyntax:\n");
		ch->send_to_char ("     zset <zone number>               - display the zone \n");
		ch->send_to_char ("     zset <zone number> lead <name>   - set the lead\n");
		ch->send_to_char ("		zset <zone number> name <name>   - set the zone name\n");
	}
	if (!isdigit (*buf))
	{
		ch->send_to_char ("Expected a zone number\n");
		return;
	}

	zone = atoi (buf);

	for (zone_iterator = zone_table.begin(); zone_iterator != zone_table.end(); zone_iterator++)
	{
		zone_it = *zone_iterator;
		if (zone_it->number == zone )
		{
			zone_name.assign(zone_it->name ? zone_it->name : "");
			zone_lead.assign(zone_it->lead ? zone_it->lead : "");
			break;
		}
	}
	
	argument = one_argument (argument, subcmd);

	if (!*subcmd)
	{

		sprintf (buf, "Zone:    [%2d]    %s\n", zone, zone_name.c_str() ? zone_name.c_str() : "Un-named Zone");
		ch->send_to_char (buf);

		sprintf (buf, "Project Lead: %s\n", zone_lead.c_str() ? zone_lead.c_str() : "No Lead");
		ch->send_to_char (buf);
		return;
	}

	while (*subcmd)
	{

		argument = one_argument (argument, buf);

		
		if (!str_cmp (subcmd, "name"))

		{
			if (!*buf)
			{
				ch->send_to_char ("Pick a name for your zone.\n");
				return;
			}

			zone_name.assign(buf);
		}

		else if (!str_cmp (subcmd, "lead"))
		{
			if (ch->get_trust() < 5)
			{
				ch->send_to_char ("You cannot assign project leads.\n");
				return;
			}
			if (!*buf)
			{
				ch->send_to_char ("Who is the Lead.\n");
				return;
			}
			zone_lead.assign(CAP (buf));
			
		}
		argument = one_argument (argument, subcmd);
	}
	
	
	 sprintf(buf, "SELECT number FROM zones WHERE number = '%d'", zone);
	 mysql_safe_query (buf);
	 result = mysql_store_result (database);
	 
	 if (result && mysql_num_rows (result) >= 1)
	 {				// Update an existing build zone record.
	 mysql_free_result (result);
	 
	 sprintf(buf, "UPDATE zones SET name = '%s', lead = '%s' WHERE number = '%d'",
		zone_name.c_str(),
		zone_lead.c_str(),
		zone);
	 send_to_gods(buf);

	 mysql_safe_query (buf);
	 }
	 else
	 {
	 sprintf(buf, "INSERT INTO zones (number, name, lead) VALUES (%d, '%s', '%s')",
		zone,
		zone_name.c_str(),
		zone_lead.c_str());
		
	 send_to_gods(buf);
	 mysql_safe_query (buf);
	 }
	
		//updating the zones
	for (zone_iterator = zone_table.begin(); zone_iterator != zone_table.end(); zone_iterator++)
	{
		zone_it = *zone_iterator;
		if (zone_it->number == zone )
		{
			zone_it->name = duplicateString(zone_name.c_str());
			zone_it->lead = duplicateString(zone_lead.c_str());
			break;
		}
	}
}

	//TODO: use for loading groups of NPCs for Events

CHAR_DATA *
mcopy (CHAR_DATA * mob)
{
	int container_container = 0;
	CHAR_DATA *new_mob;
	OBJ_DATA *obj;
	OBJ_DATA *obj2;
	OBJ_DATA *new_obj;
	OBJ_DATA *new_obj2;

	new_mob = load_mobile (mob->mob->nVirtual);

	new_mob->action = mob->action;

	for (obj = mob->equip; obj; obj = obj->next_content)
	{

		new_obj = load_object (obj->nVirtual);
		new_obj->count = obj->count;

		for (obj2 = obj->contains; obj2; obj2 = obj2->next_content)
		{

			new_obj2 = load_object (obj2->nVirtual);
			new_obj2->count = obj2->count;

			obj_to_obj (new_obj2, new_obj);

			if (obj2->contains)
				container_container = 1;
		}

		equip_char (new_mob, new_obj, obj->location);
	}

	if (mob->right_hand)
	{
		obj = mob->right_hand;
		new_obj = load_object (obj->nVirtual);
		new_obj->count = obj->count;
		
		for (obj2 = obj->contains; obj2; obj2 = obj2->next_content)
		{
			new_obj2 = load_object (obj2->nVirtual);
			new_obj2->count = obj2->count;
			obj_to_obj (new_obj2, new_obj);
			if (obj2->contains)
				container_container = 1;
		}
		obj_to_char (new_obj, new_mob);
	}

	if (mob->left_hand)
	{
		obj = mob->left_hand;
		new_obj = load_object (obj->nVirtual);
		new_obj->count = obj->count;
		
		for (obj2 = obj->contains; obj2; obj2 = obj2->next_content)
		{
			new_obj2 = load_object (obj2->nVirtual);
			new_obj2->count = obj2->count;
			obj_to_obj (new_obj2, new_obj);
			if (obj2->contains)
				container_container = 1;
		}
		obj_to_char (new_obj, new_mob);
	}

	return new_mob;
}

	//TODO: use for loading groups of NPCs for Events
void
do_mcopy (CHAR_DATA * ch, char *argument, int cmd)
{
	int room_num;
	CHAR_DATA *mob;
	CHAR_DATA *new_mob;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (argument, buf);

	if (!*buf || *buf == '?')
	{
		ch->send_to_char ("mcopy [rvnum] [mob #.]mobname\n");
		return;
	}

	room_num = ch->in_room;

	if (isdigit (buf[strlen (buf) - 1]))
	{

		room_num = atoi (buf);

		if (room_num <= 0 || room_num > 99999)
		{
			ch->send_to_char ("Illegal room number.\n");
			return;
		}

		if (!vtor (room_num))
		{
			ch->send_to_char ("No such room.\n");
			return;
		}

		argument = one_argument (argument, buf);
	}

	if (!(mob = get_char_room (buf, room_num)))
	{
		ch->send_to_char ("No such mobile.\n");
		return;
	}

	if (!IS_NPC (mob))
	{
		ch->send_to_char ("You specified a PC.\n");
		return;
	}

	new_mob = mcopy (mob);

	new_mob->char_to_room(ch->in_room);

	ch->act("$N copied and outfitted.", false, 0, new_mob, TO_CHAR);
}

void
post_alas (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *ch;

	ch = d->character;
	
	ch->room->extra->alas[ch->delay_info1].assign(reformat_desc((char*)ch->room->extra->alas[ch->delay_info1].c_str()));
	
	
	builder_log(ch, "alas_desc", d->descStr);
	
	ch->send_to_char("Alas description set.\n");

	ch->delay_who = NULL;
	ch->delay_info1 = 0;
	ch->desc->proc = NULL;
}

void
post_weather_desc (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *ch;

	ch = d->character;
	
	ch->room->extra->weather_desc[ch->delay_info1] = duplicateString(d->descStr);
	
	ch->room->extra->weather_desc[ch->delay_info1].assign(reformat_desc((char*)ch->room->extra->weather_desc[ch->delay_info1].c_str()));
	
	builder_log(ch, "weather_desc", d->descStr);
	
	ch->send_to_char("Description set.\n");

	ch->delay_who = NULL;
	ch->delay_info1 = 0;
	ch->desc->proc = NULL;
}

void
do_rset (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };

	builder_log(ch, "rset", argument);
	
	argument = one_argument (argument, buf);

	if (!*buf || *buf == '?')
	{
		ch->send_to_char ("\n\nSyntax:\n");
		ch->send_to_char ("     rset new < \'n\' | number > \n");
		ch->send_to_char ("     rset delete <room number>\n");
		ch->send_to_char ("     rset save\n");
		ch->send_to_char ("     rset name <room name>\n");
		ch->send_to_char ("     *rset desc\n");
		ch->send_to_char ("     rset bzone <value>\n");
		ch->send_to_char (" (This is for the builders zone.)\n");
		ch->send_to_char ("     rset wzone <value>\n");
		ch->send_to_char (" (This is for the weather zone.)\n");
		ch->send_to_char ("     rset cap <value>\n");
		ch->send_to_char ("     rset flag <room flag>\n");
		ch->send_to_char ("     rset terrain <terrain>\n");
		ch->send_to_char ("     rset size <value>\n");
		ch->send_to_char ("     rset location <x coor> <y coord> <z coord>\n");
		ch->send_to_char ("     rset link <direction> <room number>\n");
		ch->send_to_char ("     *rset alas <direction> 'remove'\n");
		ch->send_to_char ("     *rset extra_desc [<keyword> | 'remove']\n");
		ch->send_to_char ("     *rset secret <direction> <[difficulty | 'remove']>\n");
		ch->send_to_char ("     *rset weather <weather-room> ['remove']\n");
		ch->send_to_char ("     rset save\n");
		ch->send_to_char (" * These commands will drop you into an editor for the actual descriptions, unless you use the 'remove' option.\n");
		ch->send_to_char ("Type 'tags weather-room' for possible weather states.\n");
		ch->send_to_char ("Type 'tags room-bits' for possible room flags.\n");
		ch->send_to_char ("Type 'tags terrain-types' for possible terrains.\n");
		
		
		return;
		
	}

	
	
		if (!str_cmp (buf, "alas"))
		{
			room_alas_desc(ch, argument);
		}
		
		else if (!str_cmp (buf, "new"))
		{
			room_new(ch, argument);
		}
		
		else if (!str_cmp (buf, "location"))
		{
			room_location(ch, argument);
		}
		
		else if (!str_cmp (buf, "link"))
		{
			room_link(ch, argument);
		}
		
		else if (!str_cmp (buf, "bzone"))
		{
			room_bzone(ch, argument);
		}
		
		else if (!str_cmp (buf, "cap"))
		{
			room_capacity(ch, argument);
		}

	
		else if (!str_cmp (buf, "wzone"))
		{
			room_wzone(ch, argument);
		}
		else if (!str_cmp (buf, "size"))
		{
			room_size(ch, argument);
		}
		
		else if (!str_cmp (buf, "weather"))
		{
			room_weather_desc(ch, argument);
		}
		
		else if (!str_cmp (buf, "secret"))
		{
			room_secret (ch, argument);
		}
		
		else if (!str_cmp (buf, "delete"))
		{
			rdelete (ch, argument);
		}
		
		else if (!str_cmp (buf, "flag"))
		{
			rflags(ch, argument);
		}
		
		else if (!str_cmp (buf, "terrain"))
		{
			room_terrain(ch, argument);
		}
		
		else if (!str_cmp (buf, "extra_desc"))
		{
			room_redesc (ch, argument);
		}
		
		else if (!str_cmp (buf, "name"))
		{
			room_name(ch, argument);
		}
		
		else if (!str_cmp (buf, "save"))
		{
			room_light(ch->room);		
			save_mysql_room(ch, ch->room, BUILD_APPROVED);
			sprintf(buf, "Room %d has been saved!\n", ch->room->nVirtual);
			ch->send_to_char (buf);
		}
		else if (!str_cmp (buf, "desc") || !str_cmp (buf, "description"))
		{
			room_desc(ch, argument);
		}
		return;
	
}

void
do_job (CHAR_DATA * ch, char *argument, int cmd)
{
	int job;
	int days = 0;
	int pay_date;
	int count = 0;
	int ovnum = 0;
	int employer = 0;
	int cash = 0;
	CHAR_DATA *edit_mob;
	AFFECTED_TYPE *af;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (argument, buf);

	if (*buf == '?')
	{
		ch->send_to_char ("Start by using the mob command on a PC.\n");
		ch->send_to_char ("\n");
		ch->send_to_char ("job [1, 2, or 3]\n");
		ch->send_to_char ("    pay <amount>              pay PC in cash by amount\n");
		ch->send_to_char ("    days <pay-period-days>    # days between payday\n");
		ch->send_to_char ("    employer <mob-vnum>       A mob pays PC instead of autopay\n");
		ch->send_to_char ("    objects <count> <ovnum>   Pay PC a number of objects for pay\n");
		ch->send_to_char ("    delete                    Delete the job\n");
		ch->send_to_char ("\n");
		return;
	}

	if (IS_NPC(ch))
	{
		ch->send_to_char ("This command is for PC's only.\n");
		return;
	}

	if (!(edit_mob = ch->pc->edit_player))
	{
		ch->send_to_char ("Start by using the MOB command on a PC.\n");
		return;
	}

	if (atoi (buf) < 1 || atoi (buf) > 3)
	{
		ch->send_to_char ("Specify job 1, 2 or 3.\n");
		return;
	}

	job = atoi (buf) + JOB_1 - 1;

	while (*argument)
	{

		argument = one_argument (argument, buf);

		if (!str_cmp (buf, "delete"))
		{
			if ((af = get_affect (edit_mob, job)))
			{
				affect_remove (edit_mob, af);
				ch->send_to_char ("Job removed.\n");
				return;
			}
			else
				ch->send_to_char ("That job wasn't assigned.\n");
		}

		if (!str_cmp (buf, "pay") || !str_cmp (buf, "money") ||
			!str_cmp (buf, "cash"))
		{

			argument = one_argument (argument, buf);
			cash = atoi (buf);

			if (cash < 0)
				cash = 0;

			if (cash > 500)
				ch->send_to_char ("That cash amount is a bit high.\n");

			if (cash > 10000)
			{
				ch->send_to_char ("Cash > 10000.  Disallowed.\n");
				return;
			}
		}

		else if (!str_cmp (buf, "days"))
		{

			argument = one_argument (argument, buf);

			days = atoi (buf);

			if (days < 0)
				days = 0;
		}

		else if (!str_cmp (buf, "employer"))
		{

			argument = one_argument (argument, buf);

			employer = atoi (buf);

			if (!vtom (employer))
			{
				ch->send_to_char ("Employer mob is not defined.\n");
				return;
			}
		}

		else if (!str_cmp (buf, "objects"))
		{

			argument = one_argument (argument, buf);

			count = atoi (buf);

			if (count < 1 || count > 50)
			{
				ch->send_to_char ("Count should be between 1 and 50.\n");
				return;
			}

			argument = one_argument (argument, buf);

			ovnum = atoi (buf);

			if (!vtoo (ovnum))
			{
				ch->send_to_char ("Specified object vnum doesn't exist.\n");
				return;
			}
		}
	}

	if (days <= 0)
	{
		ch->send_to_char
			("Don't forget to specify the number of days until payday.\n");
		return;
	}

	if (!count && !cash)
	{
		ch->send_to_char ("Make the payment in cash or some number of objects.\n");
		return;
	}

	pay_date = time_info.month * 30 + time_info.day +
		time_info.year * 12 * 30 + days;

	if ((af = get_affect (edit_mob, job)))
		affect_remove (edit_mob, af);

	job_add_affect (edit_mob, job, days, pay_date, cash, count, ovnum,
		employer);

	ch->send_to_char ("Ok.\n");
}


std::string
rlist_show (ROOM_DATA * troom, int terrain, int header)
{
	char *bitbuf;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char rsize[2] = { '\0' };
	
	if (troom->room_size == ROOM_SIZE_DETAIL)
		strcpy(rsize, "S");
	else if (troom->room_size == ROOM_SIZE_EXPLORE)
		strcpy(rsize, "M");
	else if (troom->room_size == ROOM_SIZE_VALLEY)
		strcpy(rsize, "L");
	else if (troom->room_size == ROOM_SIZE_STORAGE)
		strcpy(rsize, "T");
	else 
		strcpy(rsize, "-");
	
	bitbuf = strdup(sprintbit (troom->room_flags, room_bits));
	sprintf (buf, "#6%s[%s]#0 #2[%d: %s#6%s#2]#0",
			 troom->name,
			 rsize,
			 troom->nVirtual,
			 bitbuf,
			 terrain_types[troom->terrain_type]);
	
	sprintf (buf + strlen (buf), "\n");
	
	return(buf);
}

/******
* rlist +/-<keyword> z<zone number> t <terrain type> /$<roomflag>
*******/
void
do_rlist (CHAR_DATA * ch, char *argument, int cmd)
{
	int header = 1;
	int terrain = -2;
	int inclusive;
	int rsize = -1;
	int zone = -1;
	int yes_key1 = 0;
	int yes_key2 = 0;
	int yes_key3 = 0;
	int flag_key1 = -2;
	int flag_key2 = -2;
	int flag_key3 = -2;
	int count = 0;
	int yes_flag1 = 0;
	int yes_flag2 = 0;
	int yes_flag3 = 0;
	int inc_flags;
	ROOM_DATA *troom;
	std::map<int, ROOM_DATA*>::iterator room_iterator;
	char key1[MAX_STRING_LENGTH] = { '\0' };
	char key2[MAX_STRING_LENGTH] = { '\0' };
	char key3[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };
	std::string temp_buf;

	temp_buf.assign("\n");
	
	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char ("Selection Parameters:\n\n");
		ch->send_to_char
			("   +/-<room keyword>       Include/exclude room keyword.\n");
		ch->send_to_char ("   z <zone>                Rooms from zone only.\n");
		ch->send_to_char
			("   $<flag>                 Include rooms with rflag.\n");
		ch->send_to_char
			("   /<flag>                 Exclude rooms with rflag.\n");
		ch->send_to_char
			("   t  <terrain>             Include rooms with terrain-type.\n");
		ch->send_to_char
		("   s  <size>             Only include rooms of <size> S, M, L, T)\n");

		ch->send_to_char ("\nExample:   rlist +bedroom -poor $scum s urban z 10\n");
		ch->send_to_char
			("will only get bedrooms of non-poor rooms, that are scum, in the urban of zone 10.\n");
		return;
	}


	while (*buf)
	{

		inclusive = 1;
		inc_flags = 1;
		
		switch (*buf)
		{

		case '-':
			inclusive = 0;

		case '+':

			if (!buf[1])
			{
				ch->send_to_char ("Expected keyname after '+/-'.\n");
				return;
			}

			if (!*key1)
			{
				yes_key1 = inclusive;
				strcpy (key1, buf + 1);
			}
			else if (!*key2)
			{
				yes_key2 = inclusive;
				strcpy (key2, buf + 1);
			}
			else if (*key3)
			{
				ch->send_to_char ("Sorry, at most three keywords.\n");
				return;
			}
			else
			{
				yes_key3 = inclusive;
				strcpy (key3, buf + 1);
			}

			break;

		case 'z':

			argument = one_argument (argument, buf);

			if (!isdigit (*buf) || atoi (buf) >= MAX_ZONE)
			{
				ch->send_to_char ("Expected valid zone after 'z'.\n");
				return;
			}

			zone = atoi (buf);

			break;
				
		case 's':
			
			argument = one_argument (argument, buf);
			
			if (!*buf || isdigit (*buf))
			{
				ch->send_to_char ("Expected a size after 's' (S for small, M for medium, L for large, T for tiny).\n");
				return;
			}
			
			if (!str_cmp(buf, "S"))
				rsize = ROOM_SIZE_DETAIL;
			else if (!str_cmp(buf, "M"))
				rsize = ROOM_SIZE_EXPLORE;
			else if (!str_cmp(buf, "L"))
				rsize = ROOM_SIZE_VALLEY;
			else if (!str_cmp(buf, "T"))
				rsize = ROOM_SIZE_STORAGE;
			else 
				rsize = ROOM_SIZE_DETAIL;
		break;		

		case 't':
				
				argument = one_argument (argument, buf);
				
				if (!*buf || isdigit (*buf))
				{
					ch->send_to_char ("Expected a terrain type after 't'.\n");
					return;
				}
				terrain = parse_argument (terrain_types, buf);
		break;
				
		case '/':
			inc_flags = 0;

		case '$':
			//argument = one_argument (argument, buf);

			if (!buf[1])
			{
				ch->send_to_char ("Expected flag name after '/ or $'.\n");
				return;
			}

			if (flag_key1 == -2)
			{
				yes_flag1 = inc_flags;
				flag_key1 = index_lookup (room_bits, buf + 1);
			}
			else if (flag_key2 == -2)
			{
				yes_flag2 = inc_flags;
				flag_key2 = index_lookup (room_bits, buf + 1);
			}
			else if (flag_key3 != -2)
			{
				ch->send_to_char ("Sorry, at most three flags.\n");
				return;
			}
			else
			{
				yes_flag3 = inc_flags;
				flag_key3 = index_lookup (room_bits, buf + 1);
			}

			break;
		}

		argument = one_argument (argument, buf);
	} // while (*buf)

	
	for (room_iterator = room_map.begin(); room_iterator != room_map.end(); room_iterator++)
	{
		troom = room_iterator->second;
		
		if ((zone != -1) && (troom->zone != zone))
			continue;

		if ((rsize != -1) && (troom->room_size != rsize))
			continue;
		
		if ((terrain != -2) && (troom->terrain_type != terrain))
			continue;

		if (flag_key1 != -2)
		{
			if (yes_flag1 && !(IS_SET (troom->room_flags, (1 << flag_key1))))
				continue;
			else if (!yes_flag1 && (IS_SET (troom->room_flags, (1 << flag_key1))))
				continue;
		}

		if (flag_key2 != -2)
		{
			if (yes_flag2 && !(IS_SET (troom->room_flags, (1 << flag_key2))))
				continue;
			else if (!yes_flag2 && (IS_SET (troom->room_flags, (1 << flag_key2))))
				continue;
		}

		if (flag_key3 != -2)
		{
			if (yes_flag3 && !(IS_SET (troom->room_flags, (1 << flag_key3))))
				continue;
			else if (!yes_flag3 && (IS_SET (troom->room_flags, (1 << flag_key3))))
				continue;
		}


		if (*key1)
		{
			if (yes_key1 && !isname (key1, troom->name))
				continue;
			else if (!yes_key1 && isname (key1, troom->name))
				continue;
		}

		if (*key2)
		{
			if (yes_key2 && !isname (key2, troom->name))
				continue;
			else if (!yes_key2 && isname (key2, troom->name))
				continue;
		}

		if (*key3)
		{
			if (yes_key3 && !isname (key3, troom->name))
				continue;
			else if (!yes_key3 && isname (key3, troom->name))
				continue;
		}

		count++;

		if (count < 200)
			temp_buf.append(rlist_show (troom, terrain, header)); //prints the first 200 rooms to temp_buf

		header = 0;
	}

	if (count > 200)
	{
		sprintf (buf,
			"You have selected %d rooms (too many to print).\n",
			count);
		ch->send_to_char (buf);
		return;
	}

	page_string (ch->desc, temp_buf.c_str());
}

void
room_capacity (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	ROOM_DATA *room;

	room = vtor (ch->in_room);
	argument = one_argument (argument, buf);


	if (!isdigit (*buf))
	{
		ch->send_to_char ("Syntax:  rset cap [capacity]\n");
		return;
	}

	room->capacity = atoi (buf);
	sprintf(buf, "Room capacity is now %d individuals.", room->capacity);

	ch->send_to_char (buf);
}

void
do_name (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA *tch = NULL;
	std::string strArgument = argument, ThisArg;

	strArgument = one_argument (strArgument, ThisArg);
	if (ThisArg.empty())
	{
		ch->send_to_char("Name who?\n");
		return;
	}
	if (!ThisArg.compare("self"))
	{
		tch = ch;
	}
	else
	{
		tch = get_char_room_vis(ch, (char *) ThisArg.c_str());
	}

	if (!tch)
	{
		ch->send_to_char("You do not see that person to rename.\n");
		return;
	}

	if (tch != ch && !IS_NPC(tch) && (!ch->get_trust()))
	{
		ch->act("Mortals may only change their own keywords (via #6self#0), or the keywords of NPCs they own.", false, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if ((!ch->get_trust()) && IS_NPC(tch) && (!tch->mob->owner || strn_cmp(tch->mob->owner, ch->name, strlen(tch->mob->owner))))
	{
		ch->act("Mortals may only change their own keywords (via #6self#0), or the keywords of NPCs they own.", false, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	strArgument = one_argument (strArgument, ThisArg);
	if (ThisArg.empty())
	{
		ch->send_to_char ("Do you wish to add, delete, or view names?\n");
		return;
	}
	if (!ThisArg.compare("add"))
	{
		strArgument = one_argument (strArgument, ThisArg);
		if (ThisArg.empty())
		{
			ch->send_to_char("What keyword would you like to add?\n");
			return;
		}
		std::string keywords = tch->keywords;
		if (keywords.find(ThisArg) != std::string::npos)
		{
			ch->send_to_char("That is already a keyword.\n");
			return;
		}
		if (keywords.length() > 120)
		{
			ch->send_to_char("You cannot have more than 120 characters of keywords, please remove some first.\n");
			return;
		}
		keywords += " " + ThisArg;
		free_mem(tch->keywords);
		tch->keywords = duplicateString ((char *) keywords.c_str());
		if (!IS_NPC(tch))
			save_char(tch, true);
		else
			save_stayput_mobiles();
		std::string sshort = (tch == ch) ? "you" : tch->char_short();
		std::string output = "Keyword: #2" + ThisArg + "#0 installed for #5" + sshort + "#0.\n";
		ch->send_to_char((char *) output.c_str());
		return;
	}
	else if (!ThisArg.compare("delete"))
	{
		strArgument = one_argument (strArgument, ThisArg);
		if (ThisArg.empty())
		{
			ch->send_to_char ("What keyword would you like to delete?\n");
			return;
		}
		std::string name = tch->name;
		name[0] = tolower(name[0]);
		if (name.find(ThisArg) != std::string::npos)
		{
			ch->send_to_char("You cannot remove the name from the keywords.\n");
			return;
		}
		std::string keywords = tch->keywords;
		if (keywords.find(MAKE_STRING(" ") + ThisArg) == std::string::npos && keywords.find(ThisArg + " ") == std::string::npos)
		{
			ch->send_to_char("That isn't a keyword.\n");
			return;
		}
		std::string sdesc = tch->short_descr;
		if (sdesc.find(ThisArg) != std::string::npos)
		{
			ch->send_to_char("You cannot remove keywords that are a part of the description.\n");
			return;
		}
		std::string NewKeywords;
		while (!keywords.empty())
		{
			keywords = one_argument(keywords, sdesc);
			if (sdesc.compare(ThisArg))
			{
				if (name.find(sdesc) != std::string::npos)
					sdesc[0] = toupper(sdesc[0]);
				if (NewKeywords.empty())
					NewKeywords += sdesc;
				else
					NewKeywords += " " + sdesc;
			}
		}
		free_mem(tch->keywords);
		tch->keywords = duplicateString((char *) NewKeywords.c_str());
		if (!IS_NPC(tch))
			save_char(tch, true);
		else
			save_stayput_mobiles();
		std::string sshort = (tch == ch) ? "you" : tch->char_short();
		std::string output = "Keyword: #2" + ThisArg + "#0 removed for #5" + sshort + "#0.\n";
		ch->send_to_char(output.c_str());
		return;

	}
	else if (!ThisArg.compare("view") || !ThisArg.compare("list"))
	{
		std::string output = MAKE_STRING(((tch == ch) ? "#5You#0 have" : (MAKE_STRING(tch->char_short()) + "has"))) + " the following keywords: " + MAKE_STRING(tch->keywords) + ".\n";
		ch->send_to_char(output.c_str());
		return;
	}
	else
	{
		ch->send_to_char("Do you wish to add, delete, or view names?\n");
		return;
	}
}

int
save_room_affects (int zone)
{
	char buf[MAX_INPUT_LENGTH];
	ROOM_DATA *troom;
	std::map<int, ROOM_DATA*>::iterator room_iterator;
	FILE *fr;
	
	sprintf (buf, "Saving rooms affects in zone %d.", zone);
	system_log (buf, false);
	
	sprintf (buf, "%s/raffects/resets.%d", SAVE_DIR, zone);	
	
	
	if ((fr = fopen (buf, "w+")) == NULL)
		return (0);
		
		
	for (room_iterator = room_map.begin(); room_iterator != room_map.end(); room_iterator++)
	{
		troom = room_iterator->second;
		if ((troom->zone == zone) && (troom->affects))
		{
			fwrite_room_affects (troom, fr);
		}
	}
	fprintf (fr, "$~\n");
	fclose (fr);
	
	
	return (0);
}

void
fwrite_room_affects (ROOM_DATA * troom, FILE * fp)
{
	
	AFFECTED_TYPE *af;
	
	if (troom->affects)
	{
		fprintf (fp, "#%d\n", troom->nVirtual );
		for (af = troom->affects; af; af = af->next)
		{
			fprintf (fp, "r %d %d %d\n", 
					 af->type,
					 af->a.room.duration,
					 af->a.room.intensity);
		}
	}
	
	fprintf (fp, "S\n");
}

void
room_weather_desc(CHAR_DATA * ch, char *argument)
{
	int ind;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	
	argument = one_argument (argument, buf);
	
	if (!str_cmp (buf, "day"))
	{
		room_desc (ch, argument);
		return;
	}
	
	else if (!str_cmp (buf, "?"))
	{
		sprintf(buf, "tags weather-room");
		command_interpreter (ch, buf);
		return;
	}
	
	else if ((ind = index_lookup (weather_room, buf)) == -1)
	{
		ch->send_to_char ("No such weather-room description.\n");
		return;
	}
	
	argument = one_argument (argument, buf);
	
	if (!str_cmp (buf, "remove"))
	{
		if (!ch->room->extra
			|| ch->room->extra->weather_desc[ind].empty())
		{
			sprintf (buf, "There is no current %s description.\n", weather_room[ind]);
			ch->send_to_char (buf);
			return;
		}
		ch->room->extra->weather_desc[ind] = "";
		sprintf (buf, "The weather description has been removed.\n");
		ch->send_to_char (buf);
		return;
		
	}
	
	else
	{
		if (!ch->room->extra)
		{
			ch->room->extra = new ROOM_EXTRA_DATA;
			printf ("Creating extra room data.\n");
			fflush (stdout);
		}
		
		if (!ch->room->extra->weather_desc[ind].empty())
		{
			ch->send_to_char ("The old description was: \n");
			ch->send_to_char (ch->room->extra->weather_desc[ind].c_str());
		}
		
		ch->act("$n begins editing a room description.", false, 0, 0, TO_ROOM);
		
		free_mem(ch->desc->descStr);
		
		ch->send_to_char ("\nEnter a new description.  Terminate with an '@'\n");
		ch->make_quiet();
		ch->room->extra->weather_desc[ind].clear();
		ch->desc->max_str = 2000;
		ch->delay_info1 = ind;
		ch->desc->proc = post_weather_desc;
	}
	return;
}

	//"alas <dir> <remove>"
void
room_alas_desc(CHAR_DATA * ch, char *argument)
{
	int ind;
	char* buf;
	char rem_buf[MAX_STRING_LENGTH]= { '\0' };
	
	argument = one_argument (argument, buf);
	
		//buf should be a direction
	if ((ind = index_lookup (dirs, buf)) == -1)
	{
		ch->send_to_char ("Expected north, south, east, west, etc.\n");
		return;
	}
	
	argument = one_argument (argument, rem_buf);
	
		//if there is anything left, it should be 'delete' or 'remove'
		//but if it is anything at all, it will delete the alas
	if (!*rem_buf)
	{
		if (!ch->room->extra)
			ch->room->extra = new ROOM_EXTRA_DATA;
		
		if (!ch->room->extra->alas[ind].empty())
		{
			ch->send_to_char ("The alas description was:\n");
			ch->send_to_char (ch->room->extra->alas[ind].c_str());
		}
		
		ch->act("$n begins editing an alas description.", false, 0, 0,
			 TO_ROOM);
		
		free_mem(ch->desc->descStr);
		
		ch->send_to_char ("\nEnter a new description.  Terminate with an '@'\n");
		ch->send_to_char ("1-------10--------20--------30--------40--------50--------60---65\n");
		ch->make_quiet();
		ch->room->extra->alas[ind].clear();
		ch->desc->max_str = 2000;
		ch->delay_info1 = ind;
		ch->desc->proc = post_alas;
		return;
	}	
	else
	{
		if ( ch->room->extra 
			&& !ch->room->extra->alas[ind].empty())
			ch->room->extra->alas[ind].clear();
		ch->send_to_char ("The alas description was removed.\n");
		return;
		
	}

}

void
room_location (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	char x_value[MAX_STRING_LENGTH]= { '\0' };
	char y_value[MAX_STRING_LENGTH]= { '\0' };
	char z_value[MAX_STRING_LENGTH]= { '\0' };
	int overlap = 0;
	
	argument = one_argument (argument, x_value);
	argument = one_argument (argument, y_value);
	argument = one_argument (argument, z_value);
	
					   
	if (!is_real_number(x_value) || !is_real_number(y_value) || !is_real_number(z_value))
	{
		ch->send_to_char ("Syntax:  rset location [x coor] [y coor] [z coor]\n");
		return;
	}
	
	ch->room->x_coord = atof (x_value);
	ch->room->y_coord = atof (y_value);
	ch->room->z_coord = atof (z_value);
	
	if (!((ch->room->x_coord == 0) && (ch->room->y_coord == 0) && (ch->room->z_coord == 0)))
		overlap = test_overlap(ch->room, ch);
	
	if (overlap == 0)
	{
		sprintf(buf, "Room is now located at (%f, %f, %f).\n",
			ch->room->x_coord,
			ch->room->y_coord,
			ch->room->z_coord);
	}
	
	else
	{
		sprintf(buf, "Your room overlaps %d other rooms! \n", overlap);
		ch->room->x_coord = 0;
		ch->room->y_coord = 0;
		ch->room->z_coord = 0;
		sprintf(buf, "Room has been moved back to (%f, %f, %f).\n",
				ch->room->x_coord,
				ch->room->y_coord,
				ch->room->z_coord);
	}

	
	ch->send_to_char (buf);
}

void
room_bzone (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	char zone_value[MAX_STRING_LENGTH]= { '\0' };
	
	
	
	argument = one_argument (argument, zone_value);
		
	
	if (!isdigit (*zone_value))
	{
		ch->send_to_char ("Syntax:  rset bzone <value>\n");
		return;
	}
	
	ch->room->zone = atol (zone_value);
	
	sprintf(buf, "Room is now located in Zone %d.\n",
			ch->room->zone);
	
	ch->send_to_char (buf);
}

	//sets the weather zone for the room
void
room_wzone (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];
	int tzone;
	int i;
	
	argument = one_argument (argument, buf);
	
	if ((!*buf) || !str_cmp(buf, "?") ||(isdigit (*buf)) )
		{
			ch->send_to_char ("Syntax:  rset wzone <value>\n");
			sprintf(buf2, "Possible Weather zones values are:\n");
			for (i = 0; i < MAX_WEATHER_AREAS; i++)
			{
				sprintf(buf2 + strlen(buf2), "%s \n", weather_areas[i]);
			}
			ch->send_to_char(buf2);
			return;
		}
	
	
	tzone = index_lookup(weather_areas, buf);
	
	if (tzone == -1)
	{
		ch->send_to_char ("That is not an supported weather area.\n");
		return;
	}
	
	ch->room->wzone = tzone;
	
	sprintf(buf, "Room will now have weather for %s.\n",
			weather_areas[ch->room->wzone]);
	
	ch->send_to_char (buf);
}

	//test to see if the room overlaps other rooms of similar size
int
test_overlap(ROOM_DATA *troom, CHAR_DATA *ch)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	ROOM_DATA *check_room;
	float east_limit;
	float west_limit;
	float north_limit;
	float south_limit;
	float top_limit;
	float bottom_limit;
	int room_num;
	int num_rows;
	int count_row;
	int conflict;
	int rsize;
	float limit_range;
	
	room_num = troom->nVirtual;
	rsize = troom->room_size;
	
	switch (rsize) {
		case ROOM_SIZE_STORAGE:
			limit_range = STORAGE_SIZE/2;
			break;
		case ROOM_SIZE_DETAIL:
			limit_range = DETAIL_SIZE/2;
			break;
		case ROOM_SIZE_EXPLORE:
			limit_range = EXPLORE_SIZE/2;
			break;
		case ROOM_SIZE_VALLEY:
			limit_range = VALLEY_SIZE/2;
			break;	
		default:
			limit_range = DETAIL_SIZE/2;
			break;
	}
		//west_limit --- x_coor --- east_limit
	east_limit = troom->x_coord +  limit_range;
	west_limit = troom->x_coord - limit_range;
	north_limit = troom->y_coord + limit_range;
	south_limit = troom->y_coord - limit_range;
	top_limit = troom->z_coord + limit_range;
	bottom_limit = troom->z_coord - limit_range;
	
	sprintf(buf, "SELECT nVirtual FROM rooms WHERE ((x_coor <= %f) AND (x_coor >= %f) AND (y_coor <= %f) AND (y_coor >= %f) AND (z_coor <= %f) AND (z_coor >= %f))",
			east_limit,
			west_limit,
			north_limit,
			south_limit,
			top_limit,
			bottom_limit);
	
	mysql_safe_query(buf);
	
	
	if (!(result = mysql_store_result (database)))
	{
			// if no result, panic?
		sprintf(buf, "Error placing room %d.\n", room_num);
		system_log(buf, true);
		return(1);
	}
	
	num_rows = mysql_num_rows (result);
	
	if (num_rows > 0) //more than the base room is in the prohibited area
	{
		row = mysql_fetch_row(result);
		sprintf(buf, "Rooms that conflict:\n\n");
		for (count_row = 0; count_row < num_rows; count_row ++)
		{
			conflict = atoi(row[0]);
			check_room = vtor(conflict);
			
			if (check_room->room_size == rsize)
			{
				sprintf(buf + strlen(buf), "%d (same size)\n ", conflict);
			}
			else 
			{
				sprintf(buf + strlen(buf), "%d (different size)\n ", conflict);
			}
			
			row = mysql_fetch_row(result);
		}
		ch->send_to_char(buf);
		return (num_rows);
	}
	else
		return(0);
	
	return (1);
}

void
room_size(CHAR_DATA * ch, char *argument)
{
	
		char buf[MAX_INPUT_LENGTH];
		char size_value[MAX_STRING_LENGTH]= { '\0' };
		
		argument = one_argument (argument, size_value);
		
		
		if (!isdigit (*size_value))
		{
			ch->send_to_char ("Syntax:  rset size <value>\n");
			ch->send_to_char ("Detail          0 \n");
			ch->send_to_char ("Exploration   1\n");
			ch->send_to_char ("Valley        2\n");
			ch->send_to_char ("Storage       3\n");

			return;
		}
		
		ch->room->room_size = atoi(size_value);
		
	if (ch->room->room_size > 3 || ch->room->room_size < 0)
	{
		ch->send_to_char ("Size must be 0, 1, 2, or 3\n");
		ch->room->room_size = 0;
		
	}
		sprintf(buf, "This room is now %s sized.\n",
				room_sizes[ch->room->room_size]);
		
		ch->send_to_char (buf);
	
	
}

void
room_name (CHAR_DATA * ch, char *argument)
{
	/** removing leadership commands for the moment 
	if (!IS_SET (ch->affected_by, AFF_LEADER_COMMAND)
		&& ch->get_trust() < 1)
	{
		ch->send_to_char ("You do not have approval for leadership commands");
		return;
	}
	**/
	
	for (; isspace (*argument); argument++);	/* Get rid of whitespaces */
	
	vtor (ch->in_room)->name = duplicateString (argument);
	
	ch->send_to_char ("Room has been named.\n");
}

	//rset link <dir> <room num>
void
room_link (CHAR_DATA * ch, char *argument)
{
	char *buf;
	ROOM_DATA *side_1, *side_2;
	int dir;
	int room_num;
	MYSQL_RES *result;
	int port_num;
	ROOM_PORTAL_DATA *tport;
	
	argument = one_argument (argument, buf);
	
	dir = index_lookup (dirs, buf);
	
	if (dir == -1)
	{
		ch->send_to_char ("What direction is that?\n");
		return;
	}
	
	argument = one_argument (argument, buf);
	if ((*buf) && !isdigit (*buf))
	{
		ch->send_to_char ("What is the room number on the other side?\n");
		return;
	}
	
	if (*buf)
		room_num = atoi(buf);
	else
	{
		ch->send_to_char ("That is not a valid number.\n");
		return;
	}

	side_1 = vtor(ch->in_room);
	side_2 = vtor(room_num);
	
	if (side_1 && side_2)
	{
		sprintf(buf, "INSERT INTO portals (room_1, last_editor, status) VALUES (%d, '%s', %d)", ch->in_room, ch->keywords, BUILD_APPROVED);
		
		mysql_safe_query (buf);
		
		if ((result = mysql_store_result(database)) == 0 &&
			mysql_field_count(database) == 0 &&
			mysql_insert_id(database) != 0)
		{
			port_num = mysql_insert_id(database);
		}
		
		if (port_num == '0' || !port_num)
		{
			ch->send_to_char ("There was a problem generating a portal number. Please alert a coder or try again later.\n");
			return;
		}

		
		tport = new room_portal_data;
		tport->room_portal_init();
		tport->ident = port_num;
		tport->type = 0;
		
		tport->room_1 = ch->in_room;
		tport->room_2 = room_num;
		
		tport->dir_1 = dir;
		tport->dir_2 = rev_dir[dir];
		
		portal_map[tport->ident] = tport;	
		
			//includes a room save
		if (room_place_portal(ch, tport))
		{
			save_mysql_portal (ch, tport, BUILD_APPROVED);
			sprintf(buf, "Portal %d now connects rooms %d and %d\n", port_num, tport->room_1, tport->room_2);
			ch->send_to_char (buf);
			return;
		}

	}
	
	return;
}

void
initialize_flag_values(void)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int name_no, i;
	
	
	mysql_safe_query("TRUNCATE TABLE `constant_values`");

	
	for (name_no = 0; *constant_info[name_no].constant_name; name_no++)
	{
		if (!str_cmp (constant_info[name_no].constant_name, "skills"))
			continue;
		
		for (i = 0; *(char *) constant_info[name_no].index[i] != '\n'; i++)
		{
			
			sprintf(buf, "INSERT INTO constant_values (name, value, description) VALUES ('%s', '%s', '%s')",
					constant_info[name_no].constant_name,
					(char *) constant_info[name_no].index[i],
					constant_info[name_no].description);
			
			mysql_safe_query(buf);
		}
	}
	
	
	return;
	
}



