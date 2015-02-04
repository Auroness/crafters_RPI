//////////////////////////////////////////////////////////////////////////////
//
/// db.c : Database Module
//
//
// TODO: rename this. more general set up and game starting routines
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
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/time.h>
#include </usr/local/mysql/include/mysql.h>
#include <signal.h>
#include <stdexcept>

#include "server.h"
#include "structs.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"

/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */

extern rpie::server engine;

	
std::map<int, ROOM_DATA*> room_map;	//loaded rooms

std::map<int, ROOM_PORTAL_DATA*> portal_map;	//loaded portals

std::list<char_data*> character_list;		//added as they are loaded into the gameworld
std::map<int, CHAR_DATA*> proto_mob_map;	//map of protoypes

ROLE_DATA *role_list = NULL;

std::list<obj_data*> object_list;			//all objetcts loaded in the game world
std::map<int, OBJ_DATA*> proto_obj_map;	//map of protoypes

std::map<std::string, SUBCRAFT_HEAD_DATA*> craft_map;	//map of crafts by subcraft name

std::list<zone_data*> zone_table;

extern int weather_zone_table[MAX_WEATHER_ZONES];

NEWBIE_HINT *hint_list = NULL;
SITE_INFO *banned_site;
TEXT_DATA *text_list = NULL;
int booting = 1;
int mob_start_stat = 16;

/* do_hour messes up time, so we keep track of how many times do_hour
was used, so we can make time adjustments. */

int times_do_hour_called = 0;
int next_mob_coldload_id = 0;
int next_pc_coldload_id = 0;	/* get_next_pc_coldload_id () */
int next_obj_coldload_id = 0;

int count_max_online = 0;
char max_online_date [AVG_STRING_LENGTH] = "";

int OBJECT_MAX_ZONE = 100;
int second_affect_active = 0;
int check_objects;
int check_characters;
long mud_time;

int mp_dirty = 0;		/* 1 if mob programs need to be written out */
time_t next_hour_update;	/* Mud hour pulse (25 RL min) */
time_t next_minute_update;	/* RL minute (1 min) */

REGISTRY_DATA *registry[MAX_REGISTRY];
int advance_hour_now = 0;
char BOOT[AVG_STRING_LENGTH];

FILE *fp_log;
FILE *imm_log;
FILE *guest_log;
FILE *sys_log;

struct time_info_data time_info;

/* local procedures */
void setup_dir (FILE * fl, ROOM_DATA * room, int dir, int type);
ROOM_DATA *new_room (int nVirtual);
void reset_time (void);
void reload_hints (void);
int change;
bool memory_check = false;

std::map<int, std::string> mapWearByLoc;
std::map<std::string, int> mapWearByName;

	//translates from old room bits to new version
int translate_room_flag(int tflag);

/* external refs */
extern struct descriptor_data *descriptor_list;
extern struct msg_data *msg_list;
void boot_social_messages (void);

void
initialize_location_map (void)
{
	mapWearByLoc[WEAR_LIGHT] = "light";
	mapWearByLoc[WEAR_FINGER_R] = "right finger";
	mapWearByLoc[WEAR_FINGER_L] = "left finger";
	mapWearByLoc[WEAR_NECK_1] = "neck1";
	mapWearByLoc[WEAR_NECK_2] = "neck2";
	mapWearByLoc[WEAR_BODY] = "body";
	mapWearByLoc[WEAR_HEAD] = "head";
	mapWearByLoc[WEAR_LEGS] = "legs";
	mapWearByLoc[WEAR_FEET] = "feet";
	mapWearByLoc[WEAR_HANDS] = "hands";
	mapWearByLoc[WEAR_ARMS] = "arms";
	mapWearByLoc[WEAR_ABOUT] = "about";
	mapWearByLoc[WEAR_WAIST] = "waist";
	mapWearByLoc[WEAR_WRIST_R] = "right wrist";
	mapWearByLoc[WEAR_WRIST_L] = "left wrist";
	mapWearByLoc[WEAR_PRIM] = "primary";
	mapWearByLoc[WEAR_SEC] = "secondary";
	mapWearByLoc[WEAR_BOTH] = "both";
	mapWearByLoc[WEAR_BELT_1] = "belt1";
	mapWearByLoc[WEAR_BELT_2] = "belt2";
	mapWearByLoc[WEAR_BACK] = "back";
	mapWearByLoc[WEAR_BLINDFOLD] = "blindfold";
	mapWearByLoc[WEAR_THROAT] = "throat";
	mapWearByLoc[WEAR_EAR] = "ear";
	mapWearByLoc[WEAR_SHOULDER_R] = "right shoulder";
	mapWearByLoc[WEAR_SHOULDER_L] = "left shoulder";
	mapWearByLoc[WEAR_ANKLE_R] = "right ankle";
	mapWearByLoc[WEAR_ANKLE_L] = "left ankle";
	mapWearByLoc[WEAR_HAIR] = "hair";
	mapWearByLoc[WEAR_FACE] = "face";
	mapWearByLoc[WEAR_CARRY_R] = "right hand";
	mapWearByLoc[WEAR_CARRY_L] = "left hand";
	mapWearByLoc[WEAR_ARMBAND_R] = "right armband";
	mapWearByLoc[WEAR_ARMBAND_L] = "left armband";
	
	for (std::map<int, std::string>::iterator it = mapWearByLoc.begin(); it != mapWearByLoc.end(); it++)
	{
		mapWearByName[it->second] = it->first;
	}
}


ROOM_DATA *
vtor(int roomVirtual)
{
	std::map<int, ROOM_DATA*>::iterator it;
	
	if (roomVirtual < 0)
		return NULL;
	
	it = room_map.find(roomVirtual);
	if (it != room_map.end())
	{
		return (it->second);
	}
	
	return NULL;
}


CHAR_DATA *
vtom (int nVirtual)
{
	std::map<int, CHAR_DATA*>::iterator it;
	
	if (nVirtual < 0)
		return NULL;

	it = proto_mob_map.find(nVirtual);
	if (it != proto_mob_map.end())
	{
		return (it->second);
	}
	
	return NULL;
}

ROOM_PORTAL_DATA *
vtop(int ident)
{
	std::map<int, ROOM_PORTAL_DATA*>::iterator it;
	
	if (ident < 0)
		return NULL;
	
	it = portal_map.find(ident);
	if (it != portal_map.end())
	{
		return (it->second);
	}
	
	return NULL;
}
/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */

void
boot_db (void)
{
	std::list<CHAR_DATA*>::iterator tch_iterator;
	
	mud_time = time (NULL);

	mob_start_stat = 16;
	
	system_log ("Beginning database initialization.", false);

	system_log ("Initialising registry.", false);
	setup_registry ();

	system_log ("Loading clan info.", false);
	load_clans_mysql ();

	system_log ("Reloading banned site list.", false);
	reload_sitebans ();

	system_log ("Loading special chargen roles.", false);
	reload_roles ();

	system_log ("Loading newbie hints.", false);
	reload_hints ();

	system_log ("Reading external text files.", false);
	add_text (&text_list, WIZLIST_FILE, "wizlist");
	add_text (&text_list, GREET_FILE, "greetings");
	add_text (&text_list, MAINTENANCE_FILE, "greetings.maintenance");
	add_text (&text_list, MENU1_FILE, "menu1");
	add_text (&text_list, QSTAT_FILE, "stat_message");
	add_text (&text_list, ACCT_APP_FILE, "account_application");
	add_text (&text_list, ACCT_EMAIL_FILE, "account_email");
	add_text (&text_list, ACCT_POLICIES_FILE, "account_policies");
	add_text (&text_list, THANK_YOU_FILE, "thankyou");
	add_text (&text_list, RACE_SELECT, "race_select");
	add_text (&text_list, SPECIAL_ROLE_SELECT, "special_role_select");
	add_text (&text_list, SEX_SELECT, "sex_select");
	add_text (&text_list, AGE_SELECT, "age_select");
	add_text (&text_list, PLDESC_FILE, "new_ldesc");
	add_text (&text_list, PSDESC_FILE, "new_sdesc");
	add_text (&text_list, PDESC_FILE, "new_desc");
	add_text (&text_list, NAME_FILE, "new_name");
	add_text (&text_list, PKEYWORDS_FILE, "new_keyword");
	add_text (&text_list, HEIGHT_FRAME, "height_frame");
	add_text (&text_list, LOCATION, "location");
	add_text (&text_list, COMMENT_HELP, "comment_help");
	add_text (&text_list, SKILL_SELECT, "skills_select");
	add_text (&text_list, PROFESSION_SELECT, "professions");


	system_log ("Reading race table.", false);
	load_race_table ();

	system_log ("Reading skills table.", false);
	load_skills_mysql ();
	
	system_log ("Initializing dynamic weather zones.", false);
	initialize_weather_zones ();

	system_log ("Resetting the game time:", false);
	reset_time ();

	system_log ("Resetting the weather state:", false);
	weather_and_time(0);
	
	initialize_materials();
	system_log ("Initializing materials.", false);
	
	object_list.clear();
	proto_obj_map.clear();
	system_log ("Generating hash table for object files.", false);
	boot_objects ();

	
	system_log ("Loading rooms.", false);
	boot_rooms ();
		
	system_log ("Loading portals.", false);
	boot_portals ();

	system_log("Loading room affects.", false);
	boot_zones_raffects();
	
	system_log("Loading build zone information.", false);
	boot_zones();
	
	system_log ("Loading auto-generated dwellings.", false);
	load_dwelling_rooms ();

	proto_mob_map.clear();
	character_list.clear();
	system_log ("Generating hash table for mobile files.", false);
	boot_mobiles ();

	
	load_variant_values();
	system_log ("Initializing variant values.", false);

	
	system_log ("Loading craft information.", false);
	boot_crafts ();

	system_log ("Loading social messages.", false);
	boot_social_messages ();

	system_log ("Reading online record.", false);
	load_online_stats ();

	system_log ("Reloading persistent tracks.", false);
	load_tracks ();

	
	system_log ("Initializing persistent mobiles...", false);
	init_stayput_mobiles ();
	
	system_log ("Stocking any new deliveries set since last reboot...",
		false);
	stock_new_deliveries ();


	// load timestamp of last sale from vnpc database
	load_vnpc_timestamp();

	system_log ("Initializing constants lists.", false);
	initialize_materials();
	initialize_location_map();
		//initialize_flag_values();
	
	booting = 0;

	system_log ("Boot db -- DONE.", false);
}

void
reload_hints (void)
{
	NEWBIE_HINT *hint, *thint;
	FILE *fp;
	char *string;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	hint_list = NULL;

	if (!(fp = fopen ("text/hints", "r")))
		return;

	fgets (buf, 256, fp);

	while (1)
	{
		string = fread_string (fp);
		if (!string || !*string)
			break;
		hint = new NEWBIE_HINT;
		hint->hint = string;
		hint->next = NULL;
		if (hint_list == NULL)
			hint_list = hint;
		else
		{
			thint = hint_list;
			while (thint->next) {
				thint = thint->next;
			}
			thint->next = hint;
		}
	}

	fclose (fp);
}

void
stock_new_deliveries (void)
{
	CHAR_DATA *tch;
	OBJ_DATA *obj;
	ROOM_DATA *room;
	int i = 0;
	std::list<CHAR_DATA*>::iterator tch_iterator;
	
	
	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		tch = *tch_iterator;
		if (tch->deleted)
			continue;
		if (!IS_NPC (tch) || !IS_SET (tch->flags, FLAG_KEEPER) || !tch->mob->shop)
			continue;
		room = vtor (tch->mob->shop->store_vnum);
		if (!room)
			continue;
		if (!room->psave_loaded)
			load_save_room (room);
		for (i = 0; i < MAX_TRADES_IN; i++)
		{
			if (!get_obj_in_list_num (tch->mob->shop->delivery[i], room->contents))
			{
				if(obj = load_object (tch->mob->shop->delivery[i]))
					obj_to_room (obj, room->nVirtual);
			}
		}
	}

	system_log ("New shopkeeper deliveries stocked.", false);
}

void
reset_time (void)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };

	long beginning_of_time = GAME_SECONDS_BEGINNING;

	int qz[] = { 6, 6, 6, 6, 5, 5, 4, 4, 5, 6, 6, 7 };
	int sunrise_time;

	int i = 0;

	struct time_info_data mud_time_passed (time_t t2, time_t t1);

	next_hour_update = time (0) + ((time (0) - beginning_of_time) % RL_SEC_PER_IG_HOUR);
	next_minute_update = time (0);

	time_info = mud_time_passed (time (0), beginning_of_time);

	sunrise_time = qz[(int) time_info.month];	/* sunrise is easy */
	
	sunrise_sunset();
	for (i = 0; i <= MAX_WEATHER_AREAS; i++)
	{
		weather_info[i].light = global_sun_light;
	}

	sprintf (buf, "   Current Gametime: %dH %dD %dM %dY.",
		time_info.hour, time_info.day, time_info.month, time_info.year);
	system_log (buf, false);

	if (time_info.month == 0 || time_info.month == 1 || time_info.month == 11)
		time_info.season = WINTER;
	else if (time_info.month < 11 && time_info.month > 7)
		time_info.season = AUTUMN;
	else if (time_info.month < 8 && time_info.month > 4)
		time_info.season = SUMMER;
	else
		time_info.season = SPRING;

	for (i = 0; i <= MAX_WEATHER_AREAS; i++)
	{
		weather_info[i].trend = number (0, 15);
		weather_info[i].clouds = number (0, 3);
		if (time_info.season == SUMMER)
			weather_info[i].clouds = number (0, 1);
		if (weather_zone_table[i] == WEATHER_DESERT)
			weather_info[i].clouds = 0;
		weather_info[i].fog = 0;
		if (weather_info[i].clouds > 0 && weather_zone_table[i] != WEATHER_DESERT)
			weather_info[i].state = number (0, 1);
		else if (weather_zone_table[i] == WEATHER_DESERT)
			weather_info[i].state = NO_RAIN;
		weather_info[i].temperature =
			seasonal_temp[weather_zone_table[i]][time_info.month];
		weather_info[i].wind_speed = number (0, 2);
		
		if (time_info.hour >= sunrise_time
			&& time_info.hour < sunset[time_info.month])
		{
			weather_info[i].temperature += 15;
		}
		else
			weather_info[i].temperature -= 15;
	}

	time_info.holiday = 0;
}

void
create_room_zero (void)
{
	ROOM_DATA *room;

	room = new_room (0);
	room->zone = 0;
	room->name = duplicateString ("Heaven");
	room->description = duplicateString ("You are in heaven.\n");
	
	
}


#define MAX_PREALLOC_ROOMS		14000


ROOM_DATA *
new_room (int nVirtual)
{
	ROOM_DATA *new_room;

	new_room = new ROOM_DATA(nVirtual);
	new_room->psave_loaded = true;
	new_room->name = duplicateString("New Room");
	new_room->description = duplicateString("No Description Set");
		
	if (!new_room->extra)
		new_room->extra = new ROOM_EXTRA_DATA;
	
	room_map[new_room->nVirtual] = new_room;
	
	return new_room;
}


/*************************************************************************
*  stuff related to the save/load player system								  *
*********************************************************************** */

/* Load a char, true if loaded, false if not */

int
load_char_objs (CHAR_DATA * ch, char *name)
{
	FILE *pf;
	char fbuf[265];
	
	if (!name)
	{
		system_log ("BUG: name NULL in load_char_objs: db.c", true);
		return 0;
	}
	else if (!*name)
	{
		system_log ("BUG: name empty in load_char_objs: db.c\n", true);
		return 0;
	}
	
	sprintf (fbuf, "save/objs/%c/%s", tolower (*name), name);
	if (!(pf = fopen (fbuf, "r")))
	{
		ch->equip_newbie();
		return 0;
	}
	else
	{
		read_obj_suppliment (ch, pf);
		fclose (pf);
	}
	
	return 1;
}

void
autosave (void)
{
	int save_count = 0;
	CHAR_DATA *t;
	std::list<char_data*>::iterator tch_iterator;
	
	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		t = *tch_iterator;

		if (t->deleted || IS_NPC (t))
			continue;

		if (t->desc && t->desc->connected == CON_PLYNG && (t->in_room!=NOWHERE))
		{
			save_char (t, true);
			save_count++;
		}
	}
}


/************************************************************************
*  procs of a (more or less) general utility nature			*
********************************************************************** */
inline bool
is_valid_string_char (char c)
{
	return ((c >= 32 && c <= 126)
		|| (c >= 9 && c <= 13));
	/// \todo Allow valid extended characters (c >= 0xC0 && c <= 0xFC)
}

char *
fread_string (FILE * fp)
{
	char c;
	char string_space[MAX_STRING_LENGTH]= { '\0' };
	char *plast;

	plast = string_space;

	while (isspace ((c = getc (fp))))
	{
		*plast++ = c;
		if (c != '\t' && c != ' ')
			plast = string_space;
	}

	if ((*plast++ = c) == '~')
		return "";

	for (;;)
	{
		switch (*plast = getc (fp))
		{
		default:
			if (is_valid_string_char(*plast))
				plast++;
			break;

		case EOF:
			*plast++ = '\0';
			system_log ("Fread_string() error.", true);
			exit (1);
			break;

		case '~':
			*plast = '\0';
			return duplicateString (string_space);
		}
	}
	return NULL;
}

char *
read_string (char *string)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };

	*buf = '\0';

	while (isspace (*string))
	{
		*string++;
	}

	if (*string == '~')
		return "";

	for (;;)
	{
		switch (*string)
		{
		default:
			*string++;
			sprintf (buf + strlen (buf), "%c", *string);
			break;
		case '~':
			return duplicateString (buf);
		}
	}
}


void
free_obj (OBJ_DATA * obj)
{
	AFFECTED_TYPE *af;
	OBJ_DATA *tobj;
	WRITING_DATA *writing;

	if (obj->nVirtual == 0)
		return;
	else
		tobj = vtoo (obj->nVirtual);
	
	/* Make sure these arn't duplicate fields of the prototype */

	if (!tobj || tobj->name != obj->name)
		free_mem (obj->name);

	if (!tobj || tobj->short_description != obj->short_description)
		free_mem (obj->short_description);

	if (!tobj || tobj->description != obj->description)
		free_mem (obj->description);

	if (!tobj || tobj->full_description != obj->full_description)
		free_mem (obj->full_description);

	if (!tobj || tobj->desc_keys != obj->desc_keys)
		free_mem (obj->desc_keys);

	clear_omote (obj);

	obj->short_description = NULL;
	obj->description = NULL;
	obj->full_description = NULL;
	obj->name = NULL;
	obj->desc_keys = NULL;

	while ((af = obj->xaffected))
	{
		obj->xaffected = af->next;
		free_mem (af);
	}

	while (obj->writing)
	{
		writing = obj->writing;
		obj->writing = writing->next_page;
		if (writing->message)
			free_mem (writing->message);
		if (writing->author)
			free_mem (writing->author);
		if (writing->date)
			free_mem (writing->date);
		if (writing->ink)
			free_mem (writing->ink);
		free_mem (writing);
	}

	memset (obj, 0, sizeof (OBJ_DATA));

	free_mem (obj);

}

/* read contents of a text file, and place in buf */

char *
file_to_string (char *name)
{
	FILE *fl;
	char tmp[MAX_STRING_LENGTH]= { '\0' };	/* max size on the string */
	char *string;
	int num_chars;

	if (!(fl = fopen (name, "r")))
	{
		sprintf (tmp, "file_to_string(%s)", name);
		perror (tmp);
		string = duplicateString('\0');
		return (string);
	}

	num_chars = fread (tmp, 1, MAX_STRING_LENGTH - 1, fl);
	tmp[num_chars] = '\0';
	std::string tempString = tmp;
	tempString += "\r\0";
	string = duplicateString(tempString.c_str());

	fclose (fl);

	return (string);
}



void
clear_object (OBJ_DATA * obj)
{
	int i;
	memset (obj, 0, sizeof (OBJ_DATA));

	obj->coppers = 0;
	obj->nVirtual = -1;
	obj->in_room = NOWHERE;
	obj->description = NULL;
	obj->short_description = NULL;
	obj->full_description = NULL;
	obj->equiped_by = NULL;
	obj->carried_by = NULL;
	
	
	for (i=0; i< MAX_OBJ_MATERIALS; i++)
	{
		obj->materials[i] = strdup("");
	}
}

void
save_char_objs (CHAR_DATA * ch, char *name)
{
	FILE *of;
	char fbuf[MAX_STRING_LENGTH]= { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };

	sprintf (fbuf, "save/objs/%c/%s", tolower (*name), name);

	if (!IS_NPC (ch) && ch->pc->create_state == STATE_DIED)
	{
		sprintf (buf, "mv %s %s.died", fbuf, fbuf);
		system (buf);
	}

	if (!IS_NPC (ch) && ch->pc->create_state == STATE_RETIRED)
	{
		sprintf (buf, "mv %s %s.retired", fbuf, fbuf);
		system (buf);
	}
	
	if (!(of = fopen (fbuf, "w")))
	{
		sprintf (buf, "ERROR: Opening obj save file. (%s)", ch->name);
		system_log (buf, true);
		return;
	}

	write_obj_suppliment (ch, of);

	fclose (of);
}


/*
* Read a number from a file.
*/
int
fread_number (FILE * fp)
{
	char c;
	long int number;
	bool sign;

	do
	{
		c = getc (fp);
	}
	while (isspace (c));

	number = 0;

	sign = false;
	if (c == '+')
	{
		c = getc (fp);
	}
	else if (c == '-')
	{
		sign = true;
		c = getc (fp);
	}

	if (!isdigit (c))
	{
		system_log ("Fread_number(): bad format.", true);
		return 0;
		//abort ();
	}

	while (isdigit (c))
	{
		number = number * 10 + c - '0';
		c = getc (fp);
	}

	if (sign)
		number = 0 - number;

	if (c == '|')
		number += fread_number (fp);
	else if (c != ' ')
		ungetc (c, fp);

	return number;
}


/*
* Read one word (into static buffer).
*/
char *
fread_word (FILE * fp)
{
	static char word[MAX_INPUT_LENGTH];
	char *pword;
	char cEnd;

	do
	{
		cEnd = getc (fp);
	}
	while (isspace (cEnd));

	if (cEnd == '\'' || cEnd == '"')
	{
		pword = word;
	}
	else
	{
		word[0] = cEnd;
		pword = word + 1;
		cEnd = ' ';
	}

	for (; pword < word + MAX_INPUT_LENGTH; pword++)
	{
		*pword = getc (fp);
		if (cEnd == ' ' ? isspace (*pword) : *pword == cEnd)
		{
			if (cEnd == ' ')
				ungetc (*pword, fp);
			*pword = '\0';
			return word;
		}
	}

	system_log ("Fread_word(): word too long.", true);
	return NULL;
	//abort ();
}

	
void
boot_mobiles ()
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	MYSQL_RES *result;
	MYSQL_ROW row;
	
	mysql_safe_query ("SELECT nVirtual FROM proto_mobiles WHERE status != 'declined'");
	result = mysql_store_result (database);
	
	if(result == false)
	{
		sprintf(buf, "boot_mobiles: mysql error number: %s\n", mysql_error(database));
		system_log (buf, true);
		return;
	}
	
	if (result && mysql_num_rows (result) >= 1)
	{				
		while ((row = mysql_fetch_row(result)))
		{
			load_mobile_mysql(atoi(row[0]));
		}
		mysql_free_result (result);
	}
	
	return;
}

void
boot_rooms ()
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	MYSQL_RES *result;
	MYSQL_ROW row;
	
	mysql_safe_query ("SELECT nVirtual FROM rooms WHERE status = 'approved'");
	result = mysql_store_result (database);
	
	if(result == false)
	{
		sprintf(buf, "boot_rooms: mysql error number: %s\n", mysql_error(database));
		system_log (buf, true);
		return;
	}
	
	if (result && (mysql_num_rows (result) > 0))
	{	
		while ((row = mysql_fetch_row(result)))
		{
			load_room_mysql(atoi(row[0]));
		}
		mysql_free_result (result);
	}
	
	return;
}

void
boot_portals ()
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	MYSQL_RES *result;
	MYSQL_ROW row;
	
	mysql_safe_query ("SELECT ident FROM portals WHERE status = 'approved'");
	result = mysql_store_result (database);
	
	if(result == false)
	{
		sprintf(buf, "boot_portals: mysql error number: %s\n", mysql_error(database));
		system_log (buf, true);
		return;
	}
	
	if (result && mysql_num_rows (result) >= 1)
	{				
		while ((row = mysql_fetch_row(result)))
		{
			load_portals_mysql(atoi(row[0]));
		}
		mysql_free_result (result);
	}
	
	return;
}



void
create_ticket_proto ()
{
	OBJ_DATA *obj;

	if (vtoo (VNUM_TICKET) != NULL)
		return;

	obj = new_object ();

	clear_object (obj);

	obj->nVirtual = VNUM_TICKET;

	obj->name = duplicateString ("ticket small number paper");
	obj->short_description = duplicateString ("a small ostler's ticket");
	obj->description = duplicateString ("A small paper ticket with a number "
		"is here.");
	obj->full_description = "";

	obj->obj_flags.weight = 1;
	obj->obj_flags.type_flag = ITEM_TICKET;
	obj->obj_flags.wear_flags = ITEM_TAKE;

	obj->in_room = NOWHERE;
	proto_obj_map[obj->nVirtual] = obj;
}

void
create_order_ticket_proto ()
{
	OBJ_DATA *obj;

	if (vtoo (VNUM_ORDER_TICKET) != NULL)
		return;

	obj = new_object ();

	clear_object (obj);

	obj->nVirtual = VNUM_ORDER_TICKET;

	obj->name = duplicateString ("ticket small paper merchandise");
	obj->short_description = duplicateString ("a small merchandise ticket");
	obj->description =
		duplicateString ("A small merchandise ticket has been carelessly left here.");
	obj->full_description = "";

	obj->obj_flags.weight = 1;
	obj->obj_flags.type_flag = ITEM_MERCH_TICKET;
	obj->obj_flags.wear_flags = ITEM_TAKE;

	obj->in_room = NOWHERE;
	proto_obj_map[obj->nVirtual] = obj;
}


void
create_statue_proto ()
{
	OBJ_DATA *obj;

	if (vtoo (VNUM_STATUE) != NULL)
		return;

	obj = new_object ();

	clear_object (obj);

	obj->nVirtual = VNUM_STATUE;

	obj->name = duplicateString ("statue");
	obj->short_description = duplicateString ("a remarkably lifelike statue");
	obj->description = duplicateString ("A remarkably lifelike statue looms here.");
	obj->full_description = "";

	obj->obj_flags.weight = 1000;
	obj->o.container.capacity = 0;
	obj->obj_flags.type_flag = ITEM_CONTAINER;

	obj->in_room = NOWHERE;
	proto_obj_map[obj->nVirtual] = obj;
}

void
boot_objects ()
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	MYSQL_RES *result;
	MYSQL_ROW row;
	
	
	mysql_safe_query ("SELECT nVirtual FROM proto_objects WHERE status = 'approved'");
	result = mysql_store_result (database);
	
	if(result == false)
	{
		sprintf(buf, "boot_objects: mysql error number: %s\n", mysql_error(database));
		system_log (buf, true);
		return;
	}
	
	if (result && mysql_num_rows (result) >= 1)
	{				
		while ((row = mysql_fetch_row(result)))
		{
			load_object_mysql(atoi(row[0]));
		}
		mysql_free_result (result);
	}
	 
}

struct hash_data
{
	int len;
	char *string;
	struct hash_data *next;
};

	//loads room affects from text files 
void
boot_zones_raffects (void)
{
	FILE *fl;
	int zon;
	bool flag;
	int room_num;
	char c;
	char zfile[MAX_STRING_LENGTH]= { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };
	AFFECTED_TYPE *af;
	ROOM_DATA * troom;
	
	
	for (zon = 0; zon < 99; zon++)
	{
		
		sprintf (zfile, "%s/raffects/resets.%d", SAVE_DIR, zon);
		
		if (!(fl = fopen (zfile, "r")))
		{
			continue;
		}
		
		fscanf (fl, "%c", &c);
		
		if (c == '$')
		{
			fclose (fl);
			continue;
		}
		
		for (;;)
		{
			if (c == '#')
			{
				fscanf (fl, "%d\n", &room_num); //room number				
				if (!(troom = vtor (room_num))) //if the room doesn't exist
				{
					flag = false;
					sprintf (buf, "Room %d does not exist.", room_num);
					system_log (buf, true);
				}
				else {
					flag = true;
				}

			}
			
			fscanf (fl, "%c", &c);
			
			if (c == 'r' && flag)
			{
				
				af = new AFFECTED_TYPE;
				
				fscanf (fl, "%d %d %d",
						&af->type,
						&af->a.room.duration,
						&af->a.room.intensity);
				if (flag)
					add_room_affect (&troom->affects, af->type, af->a.room.duration, af->a.room.intensity);
				
			}
			
			if (c == 'S')
			{
				flag = true;
				continue;
			}
			
			if (c == '$')
			{
				fclose (fl);
				break;
			}
			
		}
		
		
	}
	
	return;
	
}

void
boot_zones(void)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	MYSQL_RES *result;
	MYSQL_ROW row;
	zone_data * tzone;
	

	mysql_safe_query("SELECT number, name, lead FROM zones ORDER BY number ASC");
	
	
	if (!(result = mysql_store_result (database)))
	{
			// if no result, panic?
		sprintf(buf, "Error loading zone names.\n");
		system_log(buf, true);
		return;
	}
	
	
	row = mysql_fetch_row(result);
	
	if (!result || !mysql_num_rows (result))
	{
		sprintf(buf, "Zones not loaded.\n");
		system_log(buf, true);
		return;		
	}
	
	do
	{
		tzone = new zone_data;
		tzone->number = atoi(row[0]);
		tzone->name = duplicateString(row[1]);
		tzone->lead = duplicateString(row[2]);
		zone_table.push_back(tzone);
	}
	while ((row = mysql_fetch_row(result)));
	
	return;
}


	//translate the old object material number into a basic string for material type
std::string
translate_material(int value)
{
	
	if (value == 0)
		return "none";
	else if (value == 1)
		return "burlap";
	else if (value == 2)
		return "linen";
	else if (value == 3)
		return "wool";
	else if (value == 4)
		return "cotton";
	else if (value == 5)
		return "silk";
	else if (value == 6)
		return "velvet";
	else if (value == 7)
		return "cloth";
	else if (value == 8)
		return "thin-leather";
	else if (value == 9)
		return "plain-leather";
	else if (value == 10)
		return "thick-leather";
	else if (value == 11)
		return "courboulli";
	else if (value > 11 && value <= 14)
		return "leather";
	else if (value == 15)
		return "copper";
	else if (value == 16)
		return "bronze";
	else if (value == 17)
		return "iron";
	else if (value == 18)
		return "steel";
	else if (value == 19)
		return "brass";
	else if (value == 20)
		return "tin";
	else if (value == 21)
		return "lead";
	else if (value == 22)
		return "mithril";
	else if (value > 22 && value <= 28)
		return "metal";
	else if (value == 29)
		return "ceramic";
	else if (value == 30)
		return "stone";
	else if (value == 31)
		return "ivory";
	else if (value == 32)
		return "wood";
	
	return "none";
		
}
