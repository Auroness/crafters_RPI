//////////////////////////////////////////////////////////////////////////////
//
/// mysql.c : mySQL Interface Module
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

#include <sstream>
#include <iostream>
#include <iomanip>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include </usr/local/mysql/include/mysql.h>
#include </usr/local/include/mysql++/mysql++.h>
#include <dirent.h>
#include <signal.h>
#include <set>
#include <map>
#include <list>

#include "server.h"

#include "structs.h"
#include "net_link.h"
#include "account.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"
#include "decl.h"

extern RACE_TABLE_ENTRY *entry;
extern std::map<std::string, SUBCRAFT_HEAD_DATA*> craft_map;
extern std::map<int, ROOM_PORTAL_DATA*> portal_map;
extern std::map<int, OBJECT_MATERIAL*> object_material_map;
extern std::map<int, VARIANT_VALUE*> gl_variant;

extern const char *weather_states[];
extern const char *month_name[12];
extern const char *holiday_names[];

std::map<std::string, SKILL_DATA*> skill_data_map;
std::list<RACE_TABLE_ENTRY *>race_table;
std::map<std::string, CLAN_DATA*> clan_data_map;

extern bool mysql_logging;
extern int booting;
extern int finished_booting;
extern rpie::server engine;

MYSQL *database;
mysqlpp::Connection dbo(false);

int last_vnpc_sale=0;

void
init_mysql (void)
{
	if (!(database = mysql_init (database)))
	{
		fprintf (stderr, "The library call 'mysql_init' failed "
			"to initialize the MySQL handle for the following reason: %s\n",
			mysql_error (database));
		exit (1);
	}

	if (!(mysql_real_connect
		  (database,
		engine.get_config ("mysql_host").c_str (),
		engine.get_config ("mysql_user").c_str (),
		engine.get_config ("mysql_passwd").c_str (),
		   0, 0, 0, 0)))
	{
		fprintf (stderr, "mysql_real_connect: %s\n", mysql_error (database));
		exit (1);
	}

	if ((mysql_select_db (database, engine.get_config ("engine_db").c_str()) != 0))
	{
		fprintf (stderr, "mysql_select_db: %s\n", mysql_error (database));
		exit (1);
	}

	system_log ("MySQL connection initialized.", false);
}

void init_mysqlplus (void)
{
    dbo.set_option ( new mysqlpp::ConnectTimeoutOption(28800) );
  

    if ( dbo.connect (engine.get_config ("engine_db").c_str(),
            engine.get_config ("mysql_host").c_str(),
            engine.get_config ("mysql_user").c_str(),
            engine.get_config ("mysql_passwd").c_str()) ) {
        system_log ("MySQLPP connection initialized.", false);
    }	
	
}

void ping_mysqlplus (void)
{
    dbo.ping();
}

void
refresh_db_connection (void)
{
	mysql_close (database);

	init_mysql();
        init_mysqlplus();
}

// Route ALL mysql queries through this wrapper to ensure they are escaped
// properly, to thwart various SQL injection attacks.

int
mysql_safe_query (char *fmt, ...)
{
	va_list argp;
	int i = 0;
	double j = 0;
	char *s = 0, *out = 0, *p = 0;
	char safe[2*MAX_STRING_LENGTH];
	char query[2*MAX_STRING_LENGTH];
	
	*query = '\0';
	*safe = '\0';
	
	va_start (argp, fmt);
	
	for (p = fmt, out = query; *p != '\0'; p++)
	{
		if (*p != '%')
		{
			*out++ = *p;
			continue;
		}
		
		switch (*++p)
		{
			case 'c':
				i = va_arg (argp, int);
				out += sprintf (out, "%c", i);
				break;
			case 's':
				s = va_arg (argp, char *);
				if (!s)
				{
					out += sprintf (out, "");
					break;
				}
				mysql_real_escape_string (database, safe, s, strlen (s));
				out += sprintf (out, "%s", safe);
				break;
			case 'd':
				i = va_arg (argp, int);
				out += sprintf (out, "%d", i);
				break;
			case 'f':
				j = va_arg (argp, double);
				out += sprintf (out, "%f", j);
				break;
			case '%':
				out += sprintf (out, "%%");
				break;
		}
	}
	
	*out = '\0';
	
	va_end (argp);
	
	int result = mysql_real_query (database, query, strlen (query));
		
	return (result);
}




// Loads the master race table containing all race defines at boot

void
load_race_table (void)
{
	RACE_TABLE_ENTRY *entry_value;
	mysqlpp::Query query = dbo.query();
	mysqlpp::Row row;
	int i = 0;
	
	race_table.clear();	
	query << "SELECT * FROM races ORDER BY id ASC";
	
	mysqlpp::StoreQueryResult res = query.store();
	if ( res.num_rows() > 0 )
	{
		for (i = 0; i < res.num_rows(); i++)
		{
			row = res[i];
			
			entry_value = new RACE_TABLE_ENTRY;
			entry_value->next = NULL;
			entry_value->id = atoi (row["id"]);
			entry_value->name = duplicateString (row["name"]);
			entry_value->pc_race = atoi (row["pc_race"]);
			entry_value->starting_locs = atoi (row["starting_loc"]);
			entry_value->rpp_cost = atoi (row["rpp_cost"]);
			entry_value->created_by = duplicateString (row["created_by"]);
			entry_value->last_modified = atoi (row["last_modified"]);
			entry_value->race_size = atoi (row["size"]);
			entry_value->body_proto = atoi (row["body_proto"]);
			entry_value->innate_abilities = atoi (row["affects"]);
			entry_value->str_mod = atoi (row["str_mod"]);
			entry_value->dex_mod = atoi (row["dex_mod"]);
			entry_value->agi_mod = atoi (row["agi_mod"]);
			entry_value->con_mod = atoi (row["con_mod"]);
			entry_value->wil_mod = atoi (row["wil_mod"]);
			entry_value->int_mod = atoi (row["int_mod"]);
			entry_value->aur_mod = atoi (row["aur_mod"]);
			entry_value->native_tongue = atoi (row["native_tongue"]);
			entry_value->min_age = atoi (row["min_age"]);
			entry_value->max_age = atoi (row["max_age"]);
			entry_value->min_ht = atoi (row["min_height"]);
			entry_value->max_ht = atoi (row["max_height"]);
			entry_value->fem_ht_adj = atoi (row["fem_ht_diff"]);
			
			race_table.push_front(entry_value);
		}
	}
	
}

// Returns race database id, if any, for specified race name

int
lookup_race_id (const char *name)
{

	RACE_TABLE_ENTRY* ent = NULL;
	std::list<RACE_TABLE_ENTRY *>::iterator race_it;
	
	if (!name || !*name)
		return -1;
	if (race_table.empty())
		return -1;
	
	for (race_it = race_table.begin(); race_it != race_table.end(); ++race_it)
	  {
		  ent = *race_it;
		  
	    if (!str_cmp(ent->name,name))
	      {
		return ent->id;
	      }
	  }
	return -1;
}

// Returns the specified value for the specified race id

char *
lookup_race_variable (int id, int which_var)
{
	RACE_TABLE_ENTRY *entry_value;
	std::list<RACE_TABLE_ENTRY *>::iterator race_it;

	MYSQL_RES *result;
	MYSQL_ROW row;
	static char value[MAX_STRING_LENGTH]= { '\0' };
	int looked_up = 0;

	if (id < 0 || which_var < RACE_NAME || which_var > RACE_LAST_MODIFIED)
	{
		if (which_var == RACE_NAME)
			return "Unknown";
		else if (which_var == RACE_NATIVE_TONGUE)
			return "Westron";
		else
			return NULL;
	}

	for (race_it = race_table.begin(); race_it != race_table.end(); ++race_it)
	{
		entry_value = *race_it;
		
		if (entry_value->id != id)
			continue;
		if (which_var == RACE_NAME)
			return entry_value->name;
		else if (which_var == RACE_SIZE)
			looked_up = entry_value->race_size;
		else if (which_var == RACE_BODY_PROTO)
			looked_up = entry_value->body_proto;
		else if (which_var == RACE_AFFECTS)
			looked_up = entry_value->innate_abilities;
		else if (which_var == RACE_NATIVE_TONGUE)
			looked_up = entry_value->native_tongue;
		else if (which_var == RACE_MIN_AGE)
			looked_up = entry_value->min_age;
		else if (which_var == RACE_MAX_AGE)
			looked_up = entry_value->max_age;
		else if (which_var == RACE_MIN_HEIGHT)
			looked_up = entry_value->min_ht;
		else if (which_var == RACE_MAX_HEIGHT)
			looked_up = entry_value->max_ht;
		else if (which_var == RACE_FEM_HT_DIFF)
			looked_up = entry_value->fem_ht_adj;
		if (looked_up)
		{
			sprintf (value, "%d", looked_up);
			return value;
		}
		break;
	}

	*value = '\0';

	mysql_safe_query ("SELECT * FROM races WHERE id = %d", id);
	if ((result = mysql_store_result (database)))
	{
		if (mysql_num_rows (result))
		{
			if ((row = mysql_fetch_row (result)))
			{
				strcpy (value, row[which_var] != NULL ? row[which_var] : "");
			}
		}
		else
		{
			if (which_var == RACE_NAME)
			{
				strcpy (value, "Unknown");
			}
			else if (which_var == RACE_NATIVE_TONGUE)
			{
				strcpy (value, "Westron");
			}
		}
		mysql_free_result (result);
	}
	else
	{
		std::ostringstream error_message;
		error_message << "Error: " << std::endl
			<< __FILE__ << ':'
			<< __func__ << '('
			<< __LINE__ << "):" << std::endl
			<< mysql_error (database);
		const char *error_cstr = (error_message.str ()).c_str();
		send_to_gods (error_cstr);
		system_log (error_cstr, true);
	}
	return value;
}

MYSQL_RES *
mysql_player_search (int search_type, char *arg_string, int timeframe)
{
	char query[MAX_STRING_LENGTH]= { '\0' };
	static MYSQL_RES *result;
	int ind = 0;
	if (search_type == SEARCH_CLAN)
	{
		sprintf (query,
			"SELECT account, name, sdesc, create_state,"
			" TRIM(BOTH '\\'' FROM SUBSTRING_INDEX(LEFT(clans,LOCATE('%s',clans)-2),' ',-1)) AS rank"
				 " FROM pfiles WHERE ",
				 arg_string);
	}
	else
	{
		sprintf (query,
			"SELECT account, name, sdesc, create_state "
			" FROM pfiles WHERE ");
	}

	if (timeframe)
	{
		/* all, day, week, month, fortnight */
		int timeframes[5] = { 0, 86400, 604800, 2592000, 1209600 };
		sprintf (query + strlen (query), "lastlogon > (UNIX_TIMESTAMP() - %d) AND ",
			timeframes[timeframe]);
	}

	if (search_type != SEARCH_LEVEL)
	{
		strcpy (query + strlen (query), "level = 0 AND ");
	}

	if (search_type == SEARCH_KEYWORD)
		sprintf (query + strlen (query), "keywords LIKE '%%%s%%'", arg_string);
	else if (search_type == SEARCH_SDESC)
		sprintf (query + strlen (query), "sdesc LIKE '%%%s%%'", arg_string);
	else if (search_type == SEARCH_LDESC)
		sprintf (query + strlen (query), "ldesc LIKE '%%%s%%'", arg_string);
	else if (search_type == SEARCH_FDESC)
		sprintf (query + strlen (query), "description LIKE '%%%s%%'", arg_string);
	else if (search_type == SEARCH_CLAN)
		sprintf (query + strlen (query), "clans LIKE '%%%s%%'", arg_string);
	else if (search_type == SEARCH_SKILL)
		sprintf (query + strlen (query), "skills LIKE '%%%s%%'", arg_string);
	else if (search_type == SEARCH_ROOM)
		sprintf (query + strlen (query), "room = %s", arg_string);	
	else if (search_type == SEARCH_LEVEL)
		sprintf (query + strlen (query), "level = %s", arg_string);
	else if (search_type == SEARCH_RACE)
	{
		if ((ind = lookup_race_id (arg_string)) == -1)
			return NULL;
		sprintf (query + strlen (query), "race = %d", ind);
	}
	else
		return NULL;

	/* ORDER BY create_state, name */
	sprintf (query + strlen (query),
		" ORDER BY ABS(FLOOR((create_state - 2.1)*2)), name ASC");

	// Again, use real_query() here rather than safe_query() because the latter is
	// impractical and this command is for staff use only.
	fprintf(stderr, "%s\n", query);
	mysql_real_query (database, query, strlen (query));

	result = mysql_store_result (database);

	return result;
}

void
builder_log (CHAR_DATA * ch, char *command, char *str)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char tbuf[MAX_STRING_LENGTH]= { '\0' };
	
	if (!str || !*str || !command || !*command || !ch || ch->deleted)
		return;
	
	sprintf (buf, "%s", str);
	
	if (buf[strlen (buf) - 1] == '\n')
		buf[strlen (buf) - 1] = '\0';
	
	sprintf(tbuf,"INSERT INTO builder_access_log (account_id, name, room, immortal, command, argument) "
			"VALUES (%d, '%s', %d, %d, '%s', '%s')",
			ch->desc && ch->desc->acct ? ch->desc->acct->get_id() : 0,
			ch->desc && ch->desc->original ? ch->desc->original->name : ch->name,
			ch->in_room,
			ch->get_trust() > 0 && !IS_NPC (ch) ? 1 : 0,
			command, 
			buf);
	mysql_safe_query(tbuf);
	
}

void
system_log (const char *str, bool error)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buflong[2*MAX_STRING_LENGTH];
	
	if (!str || !*str || !mysql_logging)
		return;

	sprintf (buf, "%s", str);

	if (buf[strlen (buf) - 1] == '\n')
		buf[strlen (buf) - 1] = '\0';

	mysql_real_escape_string (database, buflong, buf, strlen(buf));
	sprintf(buf, "%s", buflong);
	
	mysql_safe_query
	("INSERT INTO system_log (error, entry) "
	 "VALUES (%d, '%s')",
	   (int) error, buf);
}

void
player_log (CHAR_DATA * ch, char *command, char *str)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	
	if (!str || !*str || !command || !ch)
		return;

	sprintf (buf, "%s", str);

	if (buf[strlen (buf) - 1] == '\n')
		buf[strlen (buf) - 1] = '\0';

		//TODO need to make this a define or soemthing global
	mysql_safe_query
		("INSERT INTO player_log (account_id, name, switched_into, room, guest, immortal, command, argument) "
		"VALUES (%d, '%s', '%s', %d, %d, %d, '%s', '%s')",
                ch->desc && ch->desc->acct ? ch->desc->acct->get_id() : 0,
		ch->desc && ch->desc->original ? ch->desc->original->name : ch->name,
		ch->desc && ch->desc->original ? ch->name : "",
		ch->in_room,
		IS_SET (ch->flags, FLAG_GUEST) ? 1 : 0,
		ch->get_trust() > 0 && !IS_NPC (ch) ? 1 : 0, 
		command, 
		buf);

	// feed to stdout for any on-server, realtime monitoring

	printf ("%s [%d]: %s %s\n", ch->name, ch->in_room, command, str);

	fflush (stdout);
}

void
add_profession_skills (CHAR_DATA * ch, char *skill_list)
{
	int skill_id = 0;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	while (*skill_list)
	{
		skill_list = one_argument (skill_list, buf);
		skill_id = lookup_skill_id(buf);
		if (skill_id == -1)
			continue;
		expose_skill(ch, skill_id);
	}
}

/*                                                                          *
* function: get_profession_name                                            *
*                                                                          */
char *
get_profession_name (int prof_id)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	static char prof[MAX_STRING_LENGTH]= { '\0' };
	int nProfessions = 0;

		
	if (prof_id == -1)
		return ("None");
	
	sprintf (prof, "None");

	mysql_safe_query ("SELECT name FROM professions WHERE id = %d", prof_id);

	if ((result = mysql_store_result (database)) == NULL)
	{
		sprintf (buf, "Warning: get_profession_name(): %s",
			mysql_error (database));
		system_log (buf, true);
	}
	else
	{
		nProfessions = mysql_num_rows (result);
		row = mysql_fetch_row (result);
		if (nProfessions > 0 && row != NULL)
		{
			sprintf (prof, "%s", row[0]);
		}
		mysql_free_result (result);
		result = NULL;
	}

	return prof;
}




/*                                                                         
* function: do_professions                                                 
*                                                                          
* 'delete' to show how many professions were deleted, or none, if that
*  were the case.              
**/
void
do_professions (CHAR_DATA * ch, char *argument, int cmd)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char temp_buf[MAX_STRING_LENGTH]= { '\0' };
	char skill_list[MAX_STRING_LENGTH]= { '\0' };
	char profession[MAX_STRING_LENGTH]= { '\0' };
	int i = 1, skill_num = 0;
	int nProfessions = 0;

	argument = one_argument (argument, buf);

	if (!str_cmp (buf, "delete"))
	{
		if (!*argument)
		{
			ch->send_to_char
				("Please include the name of the profession you wish to delete.\n");
			return;
		}

		argument = one_argument (argument, buf);

		mysql_safe_query ("DELETE FROM professions WHERE name = '%s'", buf);

		nProfessions = mysql_affected_rows (database);
		if (nProfessions)
		{
			sprintf (buf2,
				"%d profession%s matching the name '%s' %s deleted.\n",
				nProfessions, (nProfessions == 1) ? "" : "s", buf,
				(nProfessions == 1) ? "was" : "were");
			ch->send_to_char (buf2);
			return;
		}
		sprintf (buf2, "No professions matching the name '%s' were found.\n",
			buf);
		ch->send_to_char (buf2);
		return;
	}

	if (!str_cmp (buf, "list"))
	{
		mysql_safe_query ("SELECT * FROM professions ORDER BY name ASC");
		if ((result = mysql_store_result (database)) != NULL)
		{
			if (mysql_num_rows (result) > 0)
			{
				sprintf (temp_buf, "#2Currently Defined Professions:#0\n\n");
				while ((row = mysql_fetch_row (result)))
				{
					if (i < 10)
						sprintf (temp_buf + strlen (temp_buf),
						"   %d.  #2%15s#0: [%4d] %s\n", i, row[0],
						atoi (row[3]), row[1]);
					else
						sprintf (temp_buf + strlen (temp_buf),
						"   %d. #2%15s#0: [%4d] %s\n", i, row[0],
						atoi (row[3]), row[1]);
					i++;
				}
				page_string (ch->desc, temp_buf);
			}
			else
			{
				ch->send_to_char
					("There are no professions currently in the database.\n");
			}
			mysql_free_result (result);
			result = NULL;
			return;
		}
		else
		{
			sprintf (buf, "Warning: do_professions(): %s",
				mysql_error (database));
			system_log (buf, true);
		}
		ch->send_to_char ("An error occurred while trying to list professions.\n");
		return;
	}

	if (!str_cmp (buf, "add"))
	{

		if (!*argument)
		{
			ch->send_to_char ("Add which profession?\n");
			return;
		}

		argument = one_argument (argument, profession);

		if (!*argument)
		{
			ch->send_to_char ("Which skills should this profession start with?\n");
			return;
		}

		*skill_list = '\0';

		while (*argument)
		{
			skill_num++;
			if (skill_num > 6)
			{
				ch->send_to_char
					("You can only define a set of 6 skills per profession.\n");
				return;
			}
			argument = one_argument (argument, buf);
			if (lookup_skill_id(buf) == -1)
			{
				sprintf (buf2,
					"I couldn't find the '%s' skill in our database. Aborted.\n",
					buf);
				ch->send_to_char (buf2);
				return;
			}
			if (*skill_list)
				sprintf (skill_list + strlen (skill_list), " %s", buf);
			else
				sprintf (skill_list + strlen (skill_list), "%s", buf);
		}

		mysql_safe_query
			("INSERT INTO professions (name, skill_list) VALUES ('%s', '%s')",
			profession, skill_list);

		ch->send_to_char ("Profession has been added to the database.\n");
		return;
	}

	ch->send_to_char ("Usage: profession (add | delete | list) (<arguments>)\n");
}

void
set_hobbitmail_flags (int id, int flag)
{
	mysql_safe_query ("UPDATE hobbitmail SET flags = %d WHERE id = %d", flag,
		id);
}


std::string
resolved_host (char *ip)
{
	std::string resolved_hostname;

	mysql_safe_query ("SELECT ip, hostname, timestamp "
		"FROM resolved_hosts "
		"WHERE ip = '%s'", ip);

	MYSQL_RES *result = 0;
	if ((result = mysql_store_result (database)) != NULL)
	{
		MYSQL_ROW row = 0;
		if ((row = mysql_fetch_row (result)) != NULL)
		{
			int timestamp = int (strtol (row[2], 0, 10));
			if ((time (0) - timestamp) >= (60 * 60 * 24 * 30))
			{			// Update listing after it's a month old.
				mysql_safe_query ("DELETE FROM resolved_hosts "
					"WHERE ip = '%s'", ip);
			}
			else
			{
				resolved_hostname = row[1];
			}
		}
		mysql_free_result (result);
	}
	else
	{
		std::string mysql_error_message =  "Warning: resolved_host(): ";
		mysql_error_message += mysql_error (database);
		system_log (mysql_error_message.c_str (), true);
	}

	return resolved_hostname;
}

/*                                                                          *
* function: reload_sitebans                                                *
*                                                                          *
* 09/20/2004 [JWW] - Log a Warning on Failed MySql query                   *
*                                                                          */
void
reload_sitebans ()
{
	SITE_INFO *site, *tmp_site;
	MYSQL_RES *result;
	MYSQL_ROW row;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	mysql_safe_query ("SELECT * FROM banned_sites");
	if ((result = mysql_store_result (database)) == NULL)
	{
		sprintf (buf, "Warning: reload_sitebans(): %s", mysql_error (database));
		system_log (buf, true);
		return;
	}

	while ((row = mysql_fetch_row (result)))
	{
		site = new SITE_INFO;
		site->name = duplicateString (row[0]);
		site->banned_by = duplicateString (row[1]);
		site->banned_on = atoi (row[2]);
		site->banned_until = atoi (row[3]);
		site->next = NULL; //bug killer right here!
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
	}

	mysql_free_result (result);
	result = NULL;
}

void
save_banned_sites ()
{
	SITE_INFO *site = NULL;
	FILE *fp;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	
	mysql_safe_query ("DELETE FROM banned_sites");

	for (site = banned_site; site; site = site->next)
	{
		mysql_safe_query
			("INSERT INTO banned_sites VALUES ('%s', '%s', %d, %d)", site->name,
			site->banned_by, site->banned_on, site->banned_until);
	}

	if (!(fp = fopen ("online_stats", "w+")))
	{
		system_log (buf, true);
		return;
	}

	fprintf (fp, "%d\n%s~\n", count_max_online, max_online_date);
	fclose (fp);
}


/*                                                                          *
* function: load_mysql_save_rooms                                          *
*                                                                          *
* 09/20/2004 [JWW] - Log a Warning on Failed MySql query                   *
*                  - simplified the logic flow by nesting                  *
*                                                                          */
	//loads objects into the rooms
/******
void
load_mysql_save_rooms ()
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	int i = 0;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	mysql_safe_query ("SELECT * FROM saveroom_objs");

	if ((result = mysql_store_result (database)) == NULL)
	{
		sprintf (buf, "Warning: load_mysql_save_rooms(): %s",
			mysql_error (database));
		system_log (buf, true);
		return;
	}

	while ((row = mysql_fetch_row (result)))
	{
		sprintf (buf, "Object #%d: %s [%s]", i, row[5], row[0]);
		i++;
		if (atoi (row[0]) == -1)
			continue;
		obj_to_room (read_saved_obj (row), atoi (row[0]));
		system_log (buf, false);
	}

	mysql_free_result (result);
}
**********/
/*                                                                          *
* function: is_newbie                                                      *
*                                                                          *
*/
int
is_newbie (const CHAR_DATA* ch)
{
	MYSQL_RES *result;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int nPlayerFiles = 0;

	if (IS_NPC (ch))
		return 0;
	mysql_safe_query
		("SELECT name FROM pfiles WHERE accountId = %d AND create_state > 1", ch->pc->account_id);

	if ((result = mysql_store_result (database)) != NULL)
	{
		nPlayerFiles = mysql_num_rows (result);
		mysql_free_result (result);
		result = NULL;
		return ((nPlayerFiles > 0) ? 0 : 1);
	}
	else
	{
		sprintf (buf, "Warning: is_newbie(): %s", mysql_error (database));
		system_log (buf, true);
		return 0;			/* assume not a newbie ? */
	}
}

bool
is_newbie_acct(const char* account_name)
{
	mysql_safe_query ("SELECT COUNT(*) "
		"FROM pfiles "
		"WHERE account = '%s' "
		"AND create_state > 1",
		account_name);

	MYSQL_RES *result;
	bool is_brand_newbian = false;
	if ((result = mysql_store_result (database)) != NULL)
	{
		MYSQL_ROW row;
		if ((row = mysql_fetch_row (result)))
		{
			if (strtol (row[0],0,10) == 0)
				is_brand_newbian = true;
		}
		mysql_free_result (result);
	}
	else
	{
		std::ostringstream error_message;
		error_message << "Error: " << std::endl
			<< __FILE__ << ':'
			<< __func__ << '('
			<< __LINE__ << "):" << std::endl
			<< mysql_error (database);
		const char *error_cstr = (error_message.str ()).c_str();
		send_to_gods (error_cstr);
		system_log (error_cstr, true);
	}

	return is_brand_newbian;
}

int
is_yours (const char *name, const char *account_name)
{
	MYSQL_RES *result;
	int isYours = 0;

	if (!name || !*name || !account_name || !*account_name)
		return 0;

	mysql_safe_query
		("SELECT * FROM reviews_in_progress WHERE char_name = '%s' AND reviewer = '%s' AND (UNIX_TIMESTAMP() - timestamp) <= 60 * 20",
		name, account_name);
	if ((result = mysql_store_result (database)) != NULL)
	{

		if (mysql_num_rows (result) > 0)
		{
			isYours = 1;
		}
		mysql_free_result (result);
	}
	else if (result != NULL)
		mysql_free_result (result);

	return isYours;
}

int
is_being_reviewed (const char *name, const char *account_name)
{
	MYSQL_RES *result;
	int isBeingReviewed = 0;

	if (!name || !*name || !account_name || !*account_name)
		return 0;

	mysql_safe_query
		("SELECT *"
		" FROM reviews_in_progress"
		" WHERE char_name = '%s'"
		" AND reviewer != '%s'"
		" AND (UNIX_TIMESTAMP() - timestamp) <= 60 * 20",
		name, account_name);
	if ((result = mysql_store_result (database)) != NULL)
	{

		if (mysql_num_rows (result) > 0)
		{
			isBeingReviewed = 1;
		}
		mysql_free_result (result);
	}
	else if (result != NULL)
		mysql_free_result (result);

	return isBeingReviewed;
}

int
is_admin (const char *username)
{
	MYSQL_RES *result;

	if (!username || !*username)
		return 0;

	mysql_safe_query
		("SELECT name"
		" FROM pfiles"
		" WHERE level > 1"
		" AND account = '%s'",
		username);
	result = mysql_store_result (database);

	if (!result || !mysql_num_rows (result))
	{
		if (result)
			mysql_free_result (result);
		return 0;
	}

	if (result)
		mysql_free_result (result);

	return 1;

}

/*                                                                          *
* function: do_history                                                     *
*                                                                          *
*/
void
do_history (CHAR_DATA * ch, char *argument, int cmd)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char temp_buf[MAX_STRING_LENGTH]= { '\0' };
	char name[255];
	int nResponses = 0;

	if (!ch->delay || ch->delay_type != DEL_APP_APPROVE)
	{
		ch->send_to_char ("You are not currently reviewing an application.\n");
		return;
	}

	*name = '\0';

	strcpy (name, ch->delay_who);
	free_mem (ch->delay_who);
	ch->delay_who = NULL;
	ch->delay = 0;

	if (!*name)
	{
		ch->send_to_char ("You are not currently reviewing an application.\n");
		return;
	}

	mysql_safe_query
		("SELECT * FROM virtual_boards WHERE board_name = 'Applications' AND subject LIKE '%% %s' ORDER BY timestamp ASC",
		name);
	if ((result = mysql_store_result (database)) != NULL)
	{
		nResponses = mysql_num_rows (result);
		*temp_buf = '\0';
		while ((row = mysql_fetch_row (result)) != NULL)
		{
			sprintf (temp_buf + strlen (temp_buf),
				"\n#6Reviewed By:#0 %s\n"
				"#6Reviewed On:#0 %s\n\n"
				"%s\n--", row[3], row[4], row[5]);
		}
		mysql_free_result (result);
		result = NULL;

		if (nResponses)
		{
			strcat (temp_buf, "\n");
			page_string (ch->desc, temp_buf);
		}
		else
		{
			ch->send_to_char
				("This application has not yet had any administrator responses.\n");
		}
	}
	else
	{
		sprintf (buf, "Warning: do_history(): %s", mysql_error (database));
		system_log (buf, true);
	}
	return;
}

void
do_writings (CHAR_DATA * ch, char *argument, int cmd)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	OBJ_DATA *obj;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char temp_buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	int i = 0;

	if (!*argument)
	{
		ch->send_to_char ("How do you wish to access the writings database?\n");
		return;
	}

	argument = one_argument (argument, buf);

	if (!str_cmp (buf, "by"))
	{
		argument = one_argument (argument, buf);
		if (!*buf)
		{
			ch->send_to_char
				("For whom do you wish to search in the writings database?\n");
			return;
		}
		argument = one_argument (argument, buf2);
		if (!str_cmp (buf2, "containing"))
		{
			argument = one_argument (argument, buf2);
			if (!*buf2)
			{
				ch->send_to_char
					("Which strings did you wish to search for in these writings?\n");
				return;
			}
			mysql_safe_query
				("SELECT db_key FROM player_writing WHERE author = '%s' AND writing LIKE '%%%s%%' GROUP BY db_key ORDER BY db_key ASC",
				buf, buf2);
		}
		else
			mysql_safe_query
			("SELECT db_key FROM player_writing WHERE author = '%s' GROUP BY db_key ORDER BY db_key ASC",
			buf);
		result = mysql_store_result (database);
		if (!result || !mysql_num_rows (result))
		{
			if (result)
				mysql_free_result (result);
			ch->send_to_char
				("No writings were found in the database for that PC.\n");
			return;
		}

		sprintf (temp_buf,
			"\nThe database contains the following keys to writings authored by #6%s#0:\n\n",
			CAP (buf));
		i = 0;

		while ((row = mysql_fetch_row (result)))
		{
			sprintf (temp_buf + strlen (temp_buf), "   %-15d", atoi (row[0]));
			i++;
			if (!(i % 4))
				strcat (temp_buf, "\n");
		}

		if ((i % 4))
			strcat (temp_buf, "\n");

		page_string (ch->desc, temp_buf);

		mysql_free_result (result);
		result = NULL;
	}
	else if (!str_cmp (buf, "containing"))
	{
		argument = one_argument (argument, buf2);
		if (!*buf2)
		{
			ch->send_to_char
				("Which strings did you wish to search for in these writings?\n");
			return;
		}
		mysql_safe_query
			("SELECT db_key FROM player_writing WHERE writing LIKE '%%%s%%' GROUP BY db_key ORDER BY db_key ASC",
			buf2);
		result = mysql_store_result (database);
		if (!result || !mysql_num_rows (result))
		{
			if (result)
				mysql_free_result (result);
			ch->send_to_char
				("No writings were found in the database matching that string.\n");
			return;
		}

		*temp_buf = '\0';
		sprintf (temp_buf,
			"\nThe database contains the following keys to writings matching that string:\n\n");
		i = 0;

		while ((row = mysql_fetch_row (result)))
		{
			sprintf (temp_buf + strlen (temp_buf), "   %-15d", atoi (row[0]));
			i++;
			if (!(i % 4))
				strcat (temp_buf, "\n");
		}

		if ((i % 4))
			strcat (temp_buf, "\n");

		page_string (ch->desc, temp_buf);

		mysql_free_result (result);
		result = NULL;
	}
	else if (!str_cmp (buf, "display"))
	{
		argument = one_argument (argument, buf);
		if (!*buf)
		{
			ch->send_to_char
				("Which database key did you wish to view the writing for?\n");
			return;
		}
		if (!isdigit (*buf))
		{
			ch->send_to_char
				("You'll need to specify a numeric database key to view.\n");
			return;
		}

		mysql_safe_query
			("SELECT * FROM player_writing WHERE db_key = %d ORDER BY page ASC",
			atoi (buf));
		result = mysql_store_result (database);

		if (!result || !mysql_num_rows (result))
		{
			if (result)
				mysql_free_result (result);
			ch->send_to_char ("The database contains no writing under that key.\n");
			return;
		}

		*temp_buf = '\0';

		while ((row = mysql_fetch_row (result)))
		{
			sprintf (temp_buf + strlen (temp_buf), "\n#6Author:#0       %s\n"
				"#6Written On:#0   %s\n"
				"#6Written In:#0   %s and %s\n"
				"#6Page Number:#0  %d\n"
				"\n"
				"%s", row[1], row[3], lookup_skill_name(atoi (row[5])),
				lookup_skill_name(atoi (row[6])), atoi (row[2]), row[8]);
		}

		page_string (ch->desc, temp_buf);

		mysql_free_result (result);
		result = NULL;
	}
	else if (!str_cmp (buf, "key"))
	{
		argument = one_argument (argument, buf);
		if (!*buf)
		{
			ch->send_to_char ("Which object did you wish to key a writing to?\n");
			return;
		}
		if (!(obj = get_obj_in_dark (ch, buf, ch->right_hand)))
		{
			ch->send_to_char ("I don't see that object.\n");
			return;
		}
		if (obj->obj_flags.type_flag != ITEM_BOOK
			&& obj->obj_flags.type_flag != ITEM_PARCHMENT)
		{
			ch->send_to_char
				("You may only key writings to books or parchment objects.\n");
			return;
		}
		argument = one_argument (argument, buf);
		if (!*buf)
		{
			ch->send_to_char
				("Which writing entry did you wish to clone to this object?\n");
			return;
		}
		if (!isdigit (*buf))
		{
			ch->send_to_char
				("You'll need to specify a numeric database key to clone.\n");
			return;
		}
		mysql_safe_query
			("SELECT COUNT(*) FROM player_writing WHERE db_key = %d", atoi (buf));
		result = mysql_store_result (database);
		if (!result || !mysql_num_rows (result))
		{
			if (result)
				mysql_free_result (result);
			ch->send_to_char
				("The database contains no keys matching that number.\n");
			return;
		}
		row = mysql_fetch_row (result);
		if ((obj->obj_flags.type_flag == ITEM_PARCHMENT && atoi (row[0]) > 1) ||
			(obj->obj_flags.type_flag == ITEM_BOOK
			&& atoi (row[0]) > obj->o.od.value[0]))
		{
			ch->send_to_char
				("That writing key contains too many pages for this object to hold.\n");
			return;
		}
		if (obj->obj_flags.type_flag == ITEM_PARCHMENT)
			obj->o.od.value[0] = atoi (buf);
		else
			obj->o.od.value[1] = atoi (buf);
		load_writing (obj);
		if (obj->obj_flags.type_flag == ITEM_PARCHMENT)
			obj->o.od.value[0] = unused_writing_id ();
		else
			obj->o.od.value[1] = unused_writing_id ();
		save_writing (obj);
		ch->send_to_char
			("Writing has been successfully cloned and keyed to the object.\n");
		mysql_free_result (result);
		result = NULL;
		return;
	}
	else
		ch->send_to_char ("That isn't a recognized option for this command.\n");
}

void
load_all_writing (void)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	WRITING_DATA *writing;
	OBJ_DATA *obj;
	std::list<obj_data*>::iterator tobj_iterator;
	bool loaded = false;
	int i;

	mysql_safe_query ("SELECT * FROM player_writing");

	result = mysql_store_result (database);
	if (!result || !mysql_num_rows (result))
	{
		if (result)
			mysql_free_result (result);
		return;
	}

	while ((row = mysql_fetch_row (result)))
	{
		for (tobj_iterator = object_list.begin(); tobj_iterator != object_list.end(); tobj_iterator++)
		{
			obj = *tobj_iterator;
			
			if (obj->deleted)
				continue;

			if (obj->obj_flags.type_flag == ITEM_PARCHMENT
				&& obj->o.od.value[0] == atoi (row[0]))
			{
				if (!obj->writing) {
					obj->writing = new WRITING_DATA;
					obj->writing->next_page = NULL;
				}
				obj->writing->author = duplicateString (row[1]);
				obj->writing->date = duplicateString (row[3]);
				obj->writing->ink = duplicateString (row[4]);
				obj->writing->language = atoi (row[5]);
				obj->writing->script = atoi (row[6]);
				obj->writing->skill = atoi (row[7]);
				obj->writing->message = duplicateString (row[8]);
				obj->writing_loaded = true;
				break;
			}
			else if (obj->obj_flags.type_flag == ITEM_BOOK
				&& obj->o.od.value[1] == atoi (row[0]))
			{
				if (!obj->writing) {
					obj->writing = new WRITING_DATA;
					obj->writing->next_page = NULL;
				}
				writing = obj->writing;
				for (i = 1; i <= obj->o.od.value[0]; i++)
				{
					if (!writing->next_page && i + 1 <= obj->o.od.value[0])
					{
						writing->next_page = new WRITING_DATA;
						writing->next_page->next_page = NULL;
					}
					if (i == atoi (row[2]))
					{
						writing->author = duplicateString (row[1]);
						writing->date = duplicateString (row[3]);
						writing->ink = duplicateString (row[4]);
						writing->language = atoi (row[5]);
						writing->script = atoi (row[6]);
						writing->skill = atoi (row[7]);
						writing->message = duplicateString (row[8]);
						loaded = true;
						break;
					}
					writing = writing->next_page;
				}
				if (loaded)
				{
					obj->writing_loaded = true;
					loaded = false;
					break;
				}
			}
		}
	}

	if (result)
		mysql_free_result (result);
	result = NULL;
}

void
load_writing (OBJ_DATA * obj)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	WRITING_DATA *writing;
	int id, i;

	if (obj->writing_loaded)
	{
		return;
	}

	if (obj->obj_flags.type_flag != ITEM_BOOK && obj->obj_flags.type_flag != ITEM_PARCHMENT)
	{
		return;
	}

	obj->writing_loaded = true;

	if (obj->obj_flags.type_flag == ITEM_BOOK)
	{
		id = (obj)->o.od.value[1];
	}
	else
	{
		id = (obj)->o.od.value[0];
	}

	mysql_safe_query
		("SELECT * FROM player_writing WHERE db_key = %d ORDER BY page ASC",
		id);

	result = mysql_store_result (database);
	if (!result || !mysql_num_rows (result))
	{
		if (result)
		{
			mysql_free_result (result);
		}
		return;
	}

	if (obj->obj_flags.type_flag == ITEM_PARCHMENT)
	{
		row = mysql_fetch_row (result);
		if (!row)
		{
			return;
		}
		if (!obj->writing)
		{
			obj->writing = new WRITING_DATA;
			writing = obj->writing;
			writing->next_page = NULL;
			writing->author = duplicateString (row[1]);
			writing->date = duplicateString (row[3]);
			writing->ink = duplicateString (row[4]);
			writing->language = atoi (row[5]);
			writing->script = atoi (row[6]);
			writing->skill = atoi (row[7]);
			writing->message = duplicateString (row[8]);
		}
		if (result)
		{
			mysql_free_result (result);
		}
		return;
	}
	else if (obj->obj_flags.type_flag == ITEM_BOOK)
	{
		if (!obj->writing)
		{
			obj->writing = new WRITING_DATA;
			obj->writing->next_page = NULL;
		}
		/* iterate among all known pages, listed in ascending order */
		writing = obj->writing;
		row = mysql_fetch_row(result);
		/* iterate pages */
		for (i = 1; i <= obj->o.od.value[0]; i++)
		{
			/* if the next page should exist (for the last iteration next_page
			should be left null */
			if (!writing->next_page && ((i + 1) <= obj->o.od.value[0]))
			{
				writing->next_page = new WRITING_DATA;
				writing->next_page->next_page = NULL;
			}

			/* if the data specify info on this page, add in information */
			if (row && (i == atoi(row[2])))
			{
				//std::ostringstream oss;
				//oss << "Loading written page on book " << id << " for page " << i << "\n";
				//  send_to_gods (oss.str().c_str());

				writing->author = duplicateString (row[1]);
				writing->date = duplicateString (row[3]);
				writing->ink = duplicateString (row[4]);
				writing->language = atoi (row[5]);
				writing->script = atoi (row[6]);
				writing->skill = atoi (row[7]);
				writing->message = duplicateString (row[8]);
				// load the next row to be considered on next loop
				row = mysql_fetch_row(result);
			}
			/* if the data do not exist, mark pages blank as in load_object_full */
			else
			{
				//  std::ostringstream oss;
				//  oss << "Adding blank page on book " << id << " for page " << i << "\n";
				//  send_to_gods (oss.str().c_str());

				writing->message = duplicateString ("blank");
				writing->author = duplicateString ("blank");
				writing->date = duplicateString ("blank");
				writing->ink = duplicateString ("blank");
				writing->language = 0;
				writing->script = 0;
				writing->skill = 0;
				writing->torn = false;
			}
			writing = writing->next_page;
		} // for loop iterating pages
	} // if book

	if (result)
	{
		mysql_free_result (result);
	}
	result = NULL;
}




void
save_writing (OBJ_DATA * obj)
{
	WRITING_DATA *writing;
	int i = 1;

	if (!obj->writing)
	{
		return;
	}

	if (obj->obj_flags.type_flag != ITEM_PARCHMENT && obj->obj_flags.type_flag != ITEM_BOOK)
	{
		return;
	}

	if (obj->obj_flags.type_flag == ITEM_PARCHMENT)
	{
		writing = obj->writing;
		if (!writing || !writing->date || !str_cmp (writing->date, "blank")
			|| !writing->language || !writing->script)
		{
			return;
		}

		if (!obj->o.od.value[0])
		{
			obj->o.od.value[0] = unused_writing_id ();
		}

		mysql_safe_query ("DELETE FROM player_writing WHERE db_key = %d",
			obj->o.od.value[0]);
		mysql_safe_query
			("INSERT INTO player_writing VALUES (%d, '%s', 1, '%s', '%s', %d, "
			"%d, %d, '%s', %d)", obj->o.od.value[0], writing->author,
			writing->date, writing->ink, writing->language, writing->script,
			writing->skill, writing->message, (int) time (0));
	}

	if (obj->obj_flags.type_flag == ITEM_BOOK)
	{
		if (!obj->o.od.value[1])
		{
			obj->o.od.value[1] = unused_writing_id ();
		}

		mysql_safe_query ("DELETE FROM player_writing WHERE db_key = %d", obj->o.od.value[1]);

		for (writing = obj->writing; writing != NULL && i <= obj->o.od.value[0];
			writing = writing->next_page, i++)
		{
			if (!writing )
			{
				continue;
			}
			mysql_safe_query
				("INSERT INTO player_writing VALUES (%d, '%s', %d, '%s', '%s', %d, "
				"%d, %d, '%s', %d)", obj->o.od.value[1], writing->author, i,
				writing->date, writing->ink, writing->language, writing->script,
				writing->skill, writing->message, (int) time (0));
			if (!writing->next_page)
			{
				break;
			}
		}
	}

}

int
unused_writing_id (void)
{
	MYSQL_RES *result;
	bool again = true;
	int id;

	do
	{
		id = rand ();
		mysql_safe_query ("SELECT db_key FROM player_writing WHERE db_key = %d",
			id);
		result = mysql_store_result (database);
		if (!result || !mysql_num_rows (result))
		{
			if (result)
				mysql_free_result (result);
			return id;
		}
		mysql_free_result (result);
		result = NULL;
	}
	while (again);

	return id;
}



void
load_dreams (CHAR_DATA * ch)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	DREAM_DATA *dream, *dream_list = NULL;

	if (!ch || !ch->name)
		return;

	mysql_safe_query ("SELECT * FROM dreams WHERE name = '%s'", ch->name);
	result = mysql_store_result (database);
	if (result)
	{
		while ((row = mysql_fetch_row (result)))
		{
			dream = new DREAM_DATA;
			dream->dream = duplicateString (row[2]);
			dream->next = NULL;

			if (atoi (row[1]) > 0)
			{			// Already dreamed.
				if (ch->pc->dreamed)
					dream_list = ch->pc->dreamed;
				else
					ch->pc->dreamed = dream;
			}
			else
			{
				if (ch->pc->dreams)
					dream_list = ch->pc->dreams;
				else
					ch->pc->dreams = dream;
			}

			if (dream_list)
			{
				while (dream_list->next != NULL)
					dream_list = dream_list->next;
				dream_list->next = dream;
			}

			dream_list = NULL;
		}

		mysql_free_result (result);
	}
	else
	{
		fprintf (stderr, "load_dreams: The function call 'mysql_safe_query' "
			"failed to return a valid result\nfor the following reason: "
			"\n%s\n", mysql_error (database));
		exit (1);
	}
}


void
process_queued_review (MYSQL_ROW row)
{
	CHAR_DATA *tch, *tmp_ch;
	char buf[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];
	char email[255], subject[255];
	long int time_elapsed = 0;
	bool accepted = false;
	std::list<char_data*>::iterator tch_iterator;
	
	if (!row)
		return;

	if (!(tch = load_pc (row[0])))
	{
		return;
	}

	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		tmp_ch = *tch_iterator;
		if (tmp_ch->deleted)
			continue;
		if (tmp_ch->pc && tmp_ch->pc->edit_player
			&& !str_cmp (tmp_ch->pc->edit_player->name, tch->name))
		{
			tmp_ch->send_to_char
				("The PC in your editor has been closed for an application response.\n");
			tmp_ch->pc->edit_player = NULL;
		}
	}

	while (tch->pc->load_count > 1)
		tch->unload_pc();

	if (tch->pc->create_state != 1)
	{
		mysql_safe_query ("DELETE FROM queued_reviews WHERE char_name = '%s'",
			row[0]);
		tch->unload_pc();
		return;
	}

	accepted = atoi (row[7]);
	*buf = '\0';
	*subject = '\0';

	// Acceptance; process the new PC for entry into the game

	if (accepted)
	{
		tch->setup_new_character();

		mysql_safe_query
			("UPDATE newsletter_stats SET accepted_apps=accepted_apps+1");

		mysql_safe_query ("UPDATE professions SET picked=picked+1 WHERE id=%d",
			tch->pc->profession);

		sprintf (buf, "Greetings,\n"
			"\n"
			"   Thank you for your interest in %s! This is an automated\n"
			"system notification sent to inform you that your application for a character\n"
			"named %s has been ACCEPTED by the reviewer, and you that may\n"
			"enter the game at your earliest convenience. We'll see you there!\n"
			"\n"
			"%s left the following comments regarding your application:\n"
			"\n%s", MUD_NAME, tch->name, row[1], row[8]);
		sprintf (subject, "#2Accepted:#0 %s: %s", tch->pc->account_name, tch->name);
		sprintf (buf, "%s\n", row[8]);
		add_message (1, tch->name, -2, row[1], NULL, "Application Acceptance",
			"", buf, 0);
	}
	else
	{
		tch->pc->create_state = STATE_REJECTED;

		mysql_safe_query
			("UPDATE newsletter_stats SET declined_apps=declined_apps+1");

		sprintf (buf,
			"\n#6Unfortunately, your application was declined on its most recent review.\n\n%s left the following comment(s) explaining why:#0\n"
			"\n%s", row[1], row[8]);
		if (buf[strlen (buf) - 1] != '\n')
			strcat (buf, "\n");
		tch->pc->msg = duplicateString (buf);

		sprintf (buf, "Greetings,\n"
			"\n"
			"   Thank you for your interest in %s! This is an\n"
			"automated system notification to inform you that your application for\n"
			"a character named %s was deemed inappropriate by the reviewer, and\n"
			"therefore was declined. However, don't despair! This is a relatively\n"
			"common occurrence, and nothing to worry about. Your application has\n"
			"been saved on our server, and you may make the necessary changes simply\n"
			"by entering the game as that character. You will be dropped back\n"
			"into the character generation engine, where you may make corrections.\n"
			"\n"
			"%s left the following comments regarding your application:\n"
			"\n%s", MUD_NAME, tch->name, row[1], row[8]);
		sprintf (subject, "#1Declined:#0 %s: %s", tch->pc->account_name, tch->name);
	}

	account *acct = new account (tch->pc->account_name);
	if (acct->is_registered () && *buf)
	{
		sprintf (email, "%s <%s>", MUD_NAME, MUD_EMAIL);
		send_email (acct, NULL, email, "Re: Your Character Application",
			buf);
		if (accepted && tch->pc->special_role)
		{
			sprintf (email, "%s Player <%s>", MUD_NAME, acct->email.c_str ());
			delete acct;
			acct = new account (tch->pc->special_role->poster);
			if (acct->is_registered ())
			{
				sprintf (buf, "Hello,\n\n"
					"This email is being sent to inform you that a special role you placed\n"
					"in chargen has been applied for and accepted. The details are attached.\n"
					"\n"
					"Please contact this player at your earliest convenience to arrange a time\n"
					"to set up and integrate their new character. To do so, simply click REPLY;\n"
					"their email has been listed in this message as the FROM address.\n\n"
					"Character Name: %s\n"
					"Role Description: %s\n"
					"Role Post Date: %s\n"
					"\n"
					"%s\n\n", tch->name, tch->pc->special_role->summary,
					tch->pc->special_role->date,
					tch->pc->special_role->body);
				sprintf (buf2, "New PC: %s", tch->pc->special_role->summary);
				send_email (acct, APP_EMAIL, email, buf2, buf);
			}
		}
	}

	if (*subject)
	{
		sprintf (buf, "%s\n", row[8]);
		add_message (1, "Applications", -5, row[1], NULL, subject, "", buf, 0);
	}

	time_elapsed = time (0) - tch->time_str.birth;
	mysql_safe_query ("INSERT INTO application_wait_times (wait_time) "
		"VALUES (%d)",
		(int) time_elapsed);

	mysql_safe_query ("DELETE FROM queued_reviews WHERE char_name = '%s'",
		tch->name);

		//in case staff use mset to make changes to name, description or comment section, we need to compare to DB version.
	if (tch->keywords 
		&& *tch->keywords 
		&& str_cmp (row[2], tch->keywords))
	{
		free_mem (tch->keywords);
		tch->keywords = duplicateString (row[2]);
	}

	if (tch->short_descr 
		&& *tch->short_descr
		&& str_cmp (row[3], tch->short_descr))
	{
		free_mem (tch->short_descr);
		tch->short_descr = duplicateString (row[3]);
	}

	if (tch->long_descr 
		&& *tch->long_descr
		&& str_cmp (row[4], tch->long_descr))
	{
		free_mem (tch->long_descr);
		tch->long_descr = duplicateString (row[4]);
	}

	if (tch->description 
		&& *tch->description
		&& str_cmp (row[5], tch->description))
	{
		free_mem (tch->description);
		tch->description = duplicateString (row[5]);
		
		std::stringstream tempstr;
		tempstr << reformat_desc(tch->description);	
		tch->description = duplicateString(tempstr.str().c_str());
		
	}
	delete acct;
	tch->unload_pc();
}

void
process_reviews (void)
{
	MYSQL_RES *result = NULL;
	MYSQL_ROW row;


	mysql_safe_query ("SELECT * FROM queued_reviews");
	if ((result = mysql_store_result (database)) != NULL)
	{

		while ((row = mysql_fetch_row (result)))
		{
			process_queued_review (row);
		}

		mysql_free_result (result);
	}

}

// Return number of possible starting locations for PC of given race

int
num_starting_locs (int race)
{
	int flags = 0, start_loc = 0;

	if (race < 0)
		return 1;
	
	if (lookup_race_variable (race, RACE_START_LOC))
	{
		flags = atoi (lookup_race_variable (race, RACE_START_LOC));
		if (IS_SET (flags, RACE_HOME_ANGRENOST))
			start_loc++;
		return start_loc;
	}

	return 1;
}



CHAR_DATA *
load_char_mysqlpp (char *name)
{
	int lev = 0, i = 0;
	char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH]= { '\0' };
    char *tbuf, *tbuf1, *tbuf2, *tbuf3;
	CHAR_DATA *ch = NULL;
	AFFECTED_TYPE *af;
	std::map <char*, int> col_name;
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	mysqlpp::Query query = dbo.query();
    mysqlpp::Row row;
	std::map<std::string, int> ::iterator skill_it;
	std::map<std::string, SKILL_DATA*>::iterator found_map_skill;
	std::string skillstr;

	if (!name || !*name)
		return NULL;

        query << "SELECT * FROM pfiles WHERE name = '" << name << "'";

	mysqlpp::StoreQueryResult res = query.store();
	if ( res.num_rows() > 0 )
	{
		row = res[0];
	}
        else return NULL;

	ch = new_char (1);

	ch->name = duplicateString (row["name"].c_str());
	ch->keywords = duplicateString (row["keywords"].c_str());
	ch->pc->account_name = duplicateString (row["account"].c_str());
    ch->pc->account_id = atoi(row["accountId"]);
	ch->short_descr = duplicateString (row["sdesc"].c_str());

	//correction for capital short_descr's;
	if (isupper(ch->short_descr[0]))
		ch->short_descr[0] = tolower(ch->short_descr[0]);

	ch->long_descr = duplicateString (row["ldesc"].c_str());

	ch->description = duplicateString (row["description"].c_str());
	ch->pc->msg = duplicateString (row["msg"].c_str());
	ch->pc->creation_comment = duplicateString (row["create_comment"].c_str());
	ch->pc->create_state = atoi (row["create_state"]);
	ch->pc->nanny_state = atoi (row["nanny_state"]);

	ch->pc->role = atoi (row["role_check"]);
	if ( row["role_summary"].length() > 3 )
	{
		if (!ch->pc->special_role)
		{
		ch->pc->special_role = new ROLE_DATA;
		ch->pc->special_role->next = NULL;
		
		ch->pc->special_role->summary = duplicateString (row["role_summary"].c_str());
		ch->pc->special_role->body = duplicateString (row["role_body"].c_str());
		ch->pc->special_role->date = duplicateString (row["role_date"].c_str());
		ch->pc->special_role->poster = duplicateString (row["role_poster"].c_str());
		ch->pc->special_role->cost = atoi (row["role_cost"]);
		ch->pc->special_role->id = atoi(row["role_id"]);
		}
	}

	ch->pc->app_cost = atoi (row["app_cost"]);
	ch->pc->level = atoi (row["level"]);
	ch->sex = atoi (row["sex"]);

	ch->race = atoi (row["race"]);
	ch->in_room = atoi (row["room"]);

	ch->str = atoi (row["str"]);
	ch->intel = atoi (row["intel"]);
	ch->wil = atoi (row["wil"]);
	ch->con = atoi (row["con"]);
	ch->dex = atoi (row["dex"]);
	ch->aur = atoi (row["aur"]);
	ch->agi = atoi (row["agi"]);
	ch->luk = atoi (row["luk"]);
	
	ch->pc->start_str = atoi (row["start_str"]);
	ch->pc->start_intel = atoi (row["start_intel"]);
	ch->pc->start_wil = atoi (row["start_wil"]);
	ch->pc->start_con = atoi (row["start_con"]);
	ch->pc->start_dex = atoi (row["start_dex"]);
	ch->pc->start_aur = atoi (row["start_aur"]);
	ch->pc->start_agi = atoi (row["start_agi"]);
	ch->pc->start_luk = atoi (row["start_luk"]);
	
	ch->tmp_str = ch->str;
	ch->tmp_intel = ch->intel;
	ch->tmp_wil = ch->wil;
	ch->tmp_con = ch->con;
	ch->tmp_dex = ch->dex;
	ch->tmp_aur = ch->aur;
	ch->tmp_agi = ch->agi;
	ch->tmp_luk = ch->luk;

	ch->time_str.played = atoi (row["played"]);
	ch->time_str.birth = atoi (row["birth"]);
	ch->time_str.logon = time (0);

	ch->hit = atoi (row["hit"]);
	ch->max_hit = atoi (row["maxhit"]);
	ch->move = atoi (row["move"]);
	ch->max_move = atoi (row["maxmove"]);

	ch->color = atoi (row["color"]);
	
		//speaks is a char* in game, but saved as an INT in SQL
	ch->speaks = lookup_skill_name(atoi (row["speaks"]));
	if (!ch->speaks)
		ch->speaks = "Westron";
	
	ch->flags = atoi(row["flags"]);
	ch->plr_flags = atoi (row["plrflags"]);

    ch->speed = atoi (row["speed"]);

    ch->coldload_id = atoi (row["coldload"]);
	ch->affected_by = atoi (row["affectedby"]);

	if ( row["affects"].length() > 3 )
	{
                tbuf = strdup(row["affects"].c_str());
		while (1)
		{
			get_line (&tbuf, buf);
			if (!*buf || !str_cmp (buf, "~") || !str_cmp (buf, " "))
				break;
			sscanf (buf, "%s ", buf2);
			if (!*buf2)
				break;


			if (!str_cmp (buf2, "Affect"))
			{
				af = new AFFECTED_TYPE;
				
				sscanf (buf, "Affect %d %d %d %d %d %d %ld\n",
						&af->type,
						&af->a.spell.duration,
						&af->a.spell.modifier,
						&af->a.spell.location,
						&af->a.spell.bitvector,
						&af->a.spell.sn, &af->a.spell.t);
				
					//first get rid of old affects we don't use any more
					//or apply them to the character if we do use them
				if (af->type == MAGIC_AFFECT_CURSE)
				{
					free_mem (af);
				}
				else if ((af->type > MAGIC_SKILL_GAIN_STOP) 
						 && (af->type <MAGIC_CRAFT_BRANCH_STOP))
				{
					free_mem (af);
				}
				else
				{
					af->next = NULL;
					if (af->a.spell.location <= APPLY_CON)
						affect_to_char (ch, af);
					else
					{
						af->next = ch->hour_affects;
						ch->hour_affects = af;
					}
				}
			}
			
			/***** skipping crats for now
			 else if (!str_cmp (buf2, "Subcraft"))
			 {
			 sscanf (buf, "Subcraft '%s'", buf2);
			 buf2[strlen (buf2) - 1] = '\0';
			 
			 for (tcraft_iterator = craft_map.begin(); tcraft_iterator != craft_map.end(); tcraft_iterator++)
			 {
			 craft = tcraft_iterator->second;
			 if (!str_cmp (craft->subcraft_name, buf2))
			 break;
			 }
			 
			 
			 if (!craft)
			 continue;
			 
			 for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
			 if (!get_affect (ch, i))
			 break;
			 if (i > CRAFT_LAST)
			 continue;
			 magic_add_affect (ch, i, -1, 0, 0, 0, 0);
			 af = get_affect (ch, i);
			 af->a.craft = new affect_craft_type;
			 af->a.craft->subcraft = craft;
			 af->a.craft->phase_num = 0;
			 af->a.craft->target_ch = NULL;
			 af->a.craft->target_obj = NULL;
			 af->a.craft->skill_check = 0;
			 af->a.craft->timer = 0;
			 }
			 /***************/
		}
	}

	ch->age = atoi (row["age"]);
	ch->intoxication = atoi (row["intoxication"]);
	ch->hunger = atoi (row["hunger"]);
	ch->thirst = atoi (row["thirst"]);
	ch->height = atoi (row["height"]);
	ch->frame = atoi (row["frame"]);
	ch->lastregen = atoi (row["lastregen"]);
	ch->last_room = atoi (row["lastroom"]);

    ch->pc->last_logon = atoi (row["lastlogon"]);
	ch->pc->last_logoff = atoi (row["lastlogoff"]);
	ch->pc->last_disconnect = atoi (row["lastdis"]);
	ch->pc->last_connect = atoi (row["lastconnect"]);
	ch->pc->last_died = atoi (row["lastdied"]);
	ch->pc->time_last_activity = atoi (row["lastactivity"]);
	
	if (atoi (row["hooded"]) > 0)
		ch->affected_by |= AFF_HOODED;

	if ( row["immenter"].length() > 1 )
		ch->pc->imm_enter = duplicateString (row["immenter"].c_str());
	if ( row["immleave"].length() > 1 )
		ch->pc->imm_leave = duplicateString (row["immleave"].c_str());
	if ( row["sitelie"].length() > 1 )
		ch->pc->site_lie = duplicateString (row["sitelie"].c_str());
	if ( row["voicestr"].length() > 1 && row["voicestr"].compare("(null)") != 0 )
		ch->voice_str = duplicateString (row["voicestr"]);

	if ( row["clans"].length() > 2 )
	{
        tbuf = strdup(row["clans"].c_str());
		while ( 1 )
		{
			tbuf = one_argument (tbuf, buf);
			tbuf = one_argument (tbuf, buf2);
			if (!*buf2)
				break;
			char_clan_add(ch, buf, buf2);
		}
	}


	/** skill map version of skills **/
	
	int skill_num;
	if ( row["skills"].length() > 2 )
	{
		tbuf = strdup(row["skills"].c_str());

		if (*tbuf)
		{	
			
			tbuf1 = strtok (tbuf," \n");
			if (tbuf1 != NULL)
			{
				tbuf2 = strdup(tbuf1);
			}
			
			tbuf1 = strtok (NULL, " \n");
			if (tbuf1 != NULL)
			{
				tbuf3 = strdup(tbuf1);
			}
			while ((tbuf2 != NULL) && (tbuf3 != NULL))
			{
				
				skillstr = strdup(tbuf2);
				lev = atoi(tbuf3);
				
								
				skill_num = lookup_skill_id(skillstr);
				if ((skill_num > 0) && (*tbuf3))
				{
					ch->skill_map[lookup_skill_name(skill_num)] = lev;
				}
				
				tbuf1 = strtok (NULL," \n");
				if (tbuf1 != NULL)
				{
					tbuf2 = strdup(tbuf1);
				}
				else 
					tbuf2 = NULL;
				
				tbuf1 = strtok (NULL, " \n");
				if (tbuf1 != NULL)
				{
					tbuf3 = strdup(tbuf1);
				}
				else
					tbuf3 = NULL;
				
				
			}
			
		}
	}
	
	
		//writes is a char* in game, but saved as an INT in SQL
	ch->writes = lookup_skill_name(atoi(row["writes"]));

	ch->pc->profession = atoi (row["profession"]);

	if ( row["travelstr"].length() > 2 )
		ch->travel_str = duplicateString (row["travelstr"].c_str());

	if ( row["dmotestr"].length() > 2 )
		ch->dmote_str = duplicateString (row["dmotestr"].c_str());
	
	ch->bmi = atoi (row["bmi"]);

	ch->guardian_mode = atoi (row["guardian_mode"]);

	if ( row["plan"].length() > 2 )
	{
		ch->pc->plan = new std::string(row["plan"]);
	}
			
	if ( row["goal"].length() > 2 )
	{
		ch->pc->goal = new std::string(row["goal"]);
	}
	
	ch->petition_flags = atoi(row["petition_flags"]);

	load_dreams (ch);

	if (!ch->coldload_id)
		ch->coldload_id = get_next_coldload_id (1);

	ch->assign_hit_points();

	if (ch->race == 28) //trolls - NPC
		ch->armor = 2;
	else if (ch->race == 86) //olog-hai - PC
		ch->armor = 3;
	else
		ch->armor = 0;



	ch->pc->is_guide = is_guide (ch->pc->account_name);

	if (ch->race < 0)
		ch->race = 0;

	if (lookup_race_variable (ch->race, RACE_BODY_PROTO) != NULL)
		ch->body_proto = atoi (lookup_race_variable (ch->race, RACE_BODY_PROTO));

	if (is_admin (ch->pc->account_name))
		ch->flags |= FLAG_ISADMIN;
	else
		ch->flags &= ~FLAG_ISADMIN;


	return ch;
}



char *
reference_ip (char *guest_name, char *host)
{
	static char gname[MAX_STRING_LENGTH]= { '\0' };
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (strstr (host, "127.0.0.1"))
		return guest_name;

	mysql_safe_query
		("SELECT name FROM user WHERE user_last_ip = '%s'", host);
	result = mysql_store_result (database);

	if (!result)
	{
		fprintf (stderr, "FATAL: reference_ip ():\n  %s\n",
			mysql_error (database));
		exit (1);
	}

	while ((row = mysql_fetch_row (result)))
	{
		sprintf (gname, "%s", row[0]);
		mysql_free_result (result);
		return gname;
	}

	mysql_free_result (result);

	return guest_name;
}

void
init_stayput_mobiles ()
{
	FILE *fp;
	CHAR_DATA *mob;
	ROOM_DATA *temp_room;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	std::list<char_data*>::iterator tch_iterator;
	std::map<int, ROOM_DATA*>::iterator room_iterator;
	mysqlpp::Query query = dbo.query();
    mysqlpp::Row row;

	query << "SELECT nVirtual, coldload_id, spawnpoint FROM stayput_mobs";
	

	 mysqlpp::StoreQueryResult res = query.store();
	 if ( res.num_rows() > 0 )
	 {
		 for (int i = 0; i < res.num_rows(); i++)
			 {
				 row = res[i];
				 
				 sprintf (buf, "save/mobiles/%d", atoi(row["coldload_id"]));
				 fp = fopen (buf, "r");
				 if (!fp)
					 continue;
				 
				 mob = load_a_saved_mobile (atoi (row["nVirtual"]), atoi(row["coldload_id"]), fp);
				 if (mob)
					mob->char_to_room(atoi (row["spawnpoint"]));
				 mob = NULL;
				 fclose (fp);
			 }
	 }

	system_log ("Loading stayput mobiles...", false);

	for (room_iterator = room_map.begin(); room_iterator != room_map.end(); room_iterator++)
	{
		temp_room = room_iterator->second;
		
		for (CHAR_DATA *temp_mob = temp_room->people; temp_mob; temp_mob = temp_mob->next_in_room)
		{
			bool mob_found = false;
			
			for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
			{
				if (temp_mob == *tch_iterator)
					mob_found = true;
			}
			if (!mob_found)
			{
				if (temp_mob)
					character_list.push_front(temp_mob);
			}
		}
	}

}


void
save_stayput_mobiles ()
{
	CHAR_DATA *mob;
	FILE *fp;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	std::list<char_data*>::iterator tch_iterator;
	
	if (!finished_booting)
		return;

	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		mob = *tch_iterator;
		if (mob->deleted)
			continue;
		if (!IS_NPC (mob))
			continue;
		if (mob->mob && !IS_SET (mob->mob->action, ACT_STAYPUT))
			continue;
		if (mob->desc && !mob->desc->original)
			continue;
				
		sprintf (buf, "save/mobiles/%d", mob->coldload_id);
		fp = fopen (buf, "w");
		
		save_mobile (mob, fp, "STAYPUT", 0);
		
		fclose (fp);

			//new save routine
		save_mysql_stayput_mob (mob);
	}
}


void
load_tracks (void)
{
	ROOM_DATA *room;
	TRACK_DATA *track, *tmp_track;
	MYSQL_RES *result;
	MYSQL_ROW row;

	mysql_safe_query ("SELECT * FROM tracks");
	result = mysql_store_result (database);

	if (!result)
		return;

	while ((row = mysql_fetch_row (result)))
	{
		if (!(room = vtor (atoi (row[0]))))
			continue;
		track = new TRACK_DATA;
		track->next = NULL;
		if (!room->tracks)
			room->tracks = track;
		else
			for (tmp_track = room->tracks; tmp_track; tmp_track = tmp_track->next)
			{
				if (!tmp_track->next)
				{
					tmp_track->next = track;
					break;
				}
			}
			track->race = atoi (row[1]);
			track->from_dir = atoi (row[2]);
			track->to_dir = atoi (row[3]);
			track->hours_passed = atoi (row[4]);
			track->speed = atoi (row[5]);
			track->flags = atoi (row[6]);
	}

	mysql_free_result (result);
	result = NULL;
}

void
save_tracks (void)
{
	ROOM_DATA *room;
	TRACK_DATA *track;
	std::map<int, ROOM_DATA*>::iterator room_iterator;
	
	mysql_safe_query ("DELETE FROM tracks");

	for (room_iterator = room_map.begin(); room_iterator != room_map.end(); room_iterator++)
	{
		room = room_iterator->second;
		
		if (!room->tracks)
			continue;
		for (track = room->tracks; track; track = track->next)
		{
			if (!track)
				continue;
			if (!IS_SET (track->flags, PC_TRACK))
				continue;
			mysql_safe_query
				("INSERT INTO tracks VALUES(%d, %d, %d, %d, %d, %d, %d)",
				room->nVirtual, track->race, track->from_dir, track->to_dir,
				track->hours_passed, track->speed, (int) track->flags);
		}
	}
}

void
save_hobbitmail_message (account * acct, MUDMAIL_DATA * message)
{
	if (!acct || acct->name.empty () || !message)
		return;

	mysql_safe_query
		("INSERT INTO hobbitmail (account, flags, from_line, from_account, sent_date, subject, message, timestamp, to_line)"
			" VALUES ('%s', %d, '%s', '%s', '%s', '%s', '%s', UNIX_TIMESTAMP(), '%s' )",
			acct->name.c_str (), (int) message->flags, message->from,
			message->from_account, message->date, message->subject, message->message,
			message->target);
}


void
mark_as_read (CHAR_DATA * ch, int number)
{
	mysql_safe_query
		("UPDATE player_notes SET flags = 1 WHERE name = '%s' AND post_number = %d",
		ch->name, number);
}

void
display_mysql_board_message (CHAR_DATA * ch, char *board_name, int msg_num,
							 bool bHideHeader)
{
	char query[MAX_STRING_LENGTH]= { '\0' };
	char temp_buf[MAX_STRING_LENGTH]= {'\0'};
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (msg_num < 1)
	{
		mysql_safe_query
			("SELECT count(post_number) FROM boards WHERE board_name = '%s'",
			board_name);
		if ((result = mysql_store_result (database)) != NULL)
		{
			row = mysql_fetch_row (result);
			msg_num = number (1, atoi (row[0]));
			mysql_free_result (result);
			mysql_safe_query
				("SELECT * FROM boards WHERE board_name = '%s' LIMIT %d, 1",
				board_name, msg_num - 1);
		}
		else
		{
			return;
		}
	}
	else
	{
		mysql_safe_query
			("SELECT * FROM boards WHERE board_name = '%s' AND post_number = %d",
			board_name, msg_num);
	}
	result = mysql_store_result (database);

	if (!result || !mysql_num_rows (result))
	{
		if (result)
			mysql_free_result (result);
		ch->send_to_char ("That message does not seem to exist in the database.\n");
		return;
	}

	row = mysql_fetch_row (result);

	*query = '\0';

	if (!bHideHeader)
	{
		if (*row[4])
			sprintf (query, " (%s)", row[4]);
		sprintf (temp_buf + strlen (temp_buf), "\n#6Date:#0    %s%s\n", row[5],
			ch->get_trust() && *query ? query : "");
		if (ch->get_trust())
		{
			sprintf (temp_buf + strlen (temp_buf), "#6Author:#0  %s\n", row[3]);
		}
		sprintf (temp_buf + strlen (temp_buf), "#6Subject:#0 %s\n\n", row[2]);
	}
	sprintf (temp_buf + strlen (temp_buf), "%s", row[6]);
	page_string (ch->desc, temp_buf);

	mysql_free_result (result);
}

void
retrieve_mysql_board_listing (CHAR_DATA * ch, char *board_name)
{
	mysql_safe_query ("SELECT CONCAT("
		"'#6', "
		"LPAD(post_number,5,' '), "
		"':#0 ', "
		"LPAD(ic_date, 24, ' '), "
		"' - ', "
		"SUBSTRING(subject,1,50)"
		") AS formatted "
		"FROM boards "
		"WHERE board_name = '%s' "
		"ORDER BY post_number DESC "
		"LIMIT 220",
		board_name);

	MYSQL_RES* result = mysql_store_result (database);

	if (result)
	{
		if (mysql_num_rows (result))
		{
			std::string listing;
			std::string subject;
			MYSQL_ROW row;

			while ((row = mysql_fetch_row (result)))
			{
				subject = row[0];
				if (subject.length () > 82)
				{
					subject.erase (79, std::string::npos);
					subject += "...";
				}

				listing += subject + "\n";
			}
			if (listing.length () > MAX_STRING_LENGTH)
			{
				listing.erase (MAX_STRING_LENGTH - 10, std::string::npos);
			}
			page_string (ch->desc, listing.c_str ());
		}
		else
		{
			ch->send_to_char ("No messages found for this listing.\n");
		}
		mysql_free_result (result);
	}
	else
	{
		std::ostringstream error_message;
		error_message << "Error: " << std::endl
			<< __FILE__ << ':'
			<< __func__ << '('
			<< __LINE__ << "):" << std::endl
			<< mysql_error (database);
		send_to_gods ((error_message.str ()).c_str());
	}
}

void
post_to_mysql_board (DESCRIPTOR_DATA * d)
{
	char date_buf[MAX_STRING_LENGTH]= { '\0' };
	char *date, *suf;
	int free_slot = 0, day;
	MYSQL_RES *result;
	MYSQL_ROW row;
	time_t current_time;

	if (!d->pending_message->message)
	{
		d->character->send_to_char ("No message posted.\n");
		free_mem(d->pending_message);
		d->pending_message = NULL;
		return;
	}

	current_time = time (0);
	date = asctime (localtime (&current_time));
	if (strlen (date) > 1)
		date[strlen (date) - 1] = '\0';

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

	mysql_safe_query
		("SELECT * FROM boards WHERE board_name = '%s' ORDER BY post_number DESC LIMIT 1",
		d->pending_message->poster);
	result = mysql_store_result (database);

	if (!result)
	{
		d->character->send_to_char
			("There seems to be a problem with the database listing.\n");
		return;
	}

	free_slot = 1;

	if ((row = mysql_fetch_row (result)))
	{
		free_slot = atoi (row[1]);
		free_slot++;
	}

	mysql_safe_query
		("INSERT INTO boards VALUES ('%s', %d, '%s', '%s', '%s', '%s', '%s', %d)",
		d->pending_message->poster, free_slot, d->pending_message->subject,
		d->character->name, date, date_buf, d->pending_message->message,
		(int) time (0));

	if (result)
		mysql_free_result (result);

	free_mem(d->pending_message);
	d->pending_message = NULL;
}

/* This is mainly a "test" command - not really functional for any one speciifc purpose! */

void
do_mysql (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char severity[MAX_STRING_LENGTH]= { '\0' };
	int time_passed = 0,  points = 0;
	struct time_info_data passed;
	std::list<char_data*>::iterator it;
	
	if (ch->get_trust() <= 5)
	{
		ch->send_to_char ("Eh?\n");
		return;
	}

	if (!*argument)
	{
		ch->send_to_char ("What query?\n");
		return;
	}
	

}

MESSAGE_DATA *
load_mysql_message (char *msg_name, int board_type, int msg_number)
{
	MESSAGE_DATA *message;
	MYSQL_RES *result;
	MYSQL_ROW row;

	result = NULL;

	if (board_type == 0)
	{				// In-game board system.
		mysql_safe_query
			("SELECT * FROM boards WHERE board_name = '%s' AND post_number = %d",
			msg_name, msg_number);
		result = mysql_store_result (database);
		if (!result || !mysql_num_rows (result))
		{
			if (result)
				mysql_free_result (result);
			return NULL;
		}
		if ((row = mysql_fetch_row (result)))
		{
			message = new MESSAGE_DATA;
			message->nVirtual = atoi (row[1]);
			message->poster = duplicateString (row[3]);
			message->date = duplicateString (row[4]);
			message->icdate = duplicateString (row[5]);
			message->subject = duplicateString (row[2]);
			message->message = duplicateString (row[6]);
			mysql_free_result (result);
			return message;
		}
	}
	else if (board_type == 1)
	{				// Virtual boards.
		mysql_safe_query
			("SELECT * FROM virtual_boards WHERE board_name = '%s' AND post_number = %d",
			msg_name, msg_number);
		result = mysql_store_result (database);
		if (!result || !mysql_num_rows (result))
		{
			if (result)
				mysql_free_result (result);
			return NULL;
		}
		if ((row = mysql_fetch_row (result)))
		{
			message = new MESSAGE_DATA;
			message->nVirtual = msg_number;
			message->poster = duplicateString (row[3]);
			message->date = duplicateString (row[4]);
			message->subject = duplicateString (row[2]);
			message->message = duplicateString (row[5]);
			message->nTimestamp = strtol (row[6], NULL, 10);
			mysql_free_result (result);
			return message;
		}
	}
	else if (board_type == 2)
	{				// Player notes.
		mysql_safe_query
			("SELECT * FROM player_notes WHERE name = '%s' AND post_number = %d",
			msg_name, msg_number);
		result = mysql_store_result (database);
		if (!result || !mysql_num_rows (result))
		{
			if (result)
				mysql_free_result (result);
			return NULL;
		}
		if ((row = mysql_fetch_row (result)))
		{
			message = new MESSAGE_DATA;
			message->nVirtual = msg_number;
			message->poster = duplicateString (row[3]);
			message->date = duplicateString (row[4]);
			message->subject = duplicateString (row[2]);
			message->message = duplicateString (row[5]);
			message->flags = atoi (row[6]);
			mysql_free_result (result);
			return message;
		}
	}
	else if (board_type == 3)
	{				// Player journal entries.
		mysql_safe_query
			("SELECT * FROM player_journals WHERE name = '%s' AND post_number = %d",
			msg_name, msg_number);
		result = mysql_store_result (database);
		if (!result || !mysql_num_rows (result))
		{
			if (result)
				mysql_free_result (result);
			return NULL;
		}
		if ((row = mysql_fetch_row (result)))
		{
			message = new MESSAGE_DATA;
			message->nVirtual = msg_number;
			message->poster = duplicateString (row[3]);
			message->date = duplicateString (row[4]);
			message->subject = duplicateString (row[2]);
			message->message = duplicateString (row[5]);
			mysql_free_result (result);
			return message;
		}
	}

	return NULL;
}

int
erase_mysql_board_post (CHAR_DATA * ch, char *name, int board_type,
						char *argument)
{
	MESSAGE_DATA *message;

	if (strlen (name) > 75)
		return 0;

	name[0] = toupper (name[0]);

	if (!isdigit (*argument))
		return 0;

	if (atoi (argument) < 1)
		return 0;

	if (board_type == 0)
	{
		if (!(message = load_message (name, 6, atoi (argument))))
			return 0;
		free_mem(message);
		mysql_safe_query
			("DELETE FROM boards WHERE post_number = %d AND board_name = '%s'",
			atoi (argument), name);
	}
	else if (board_type == 1)
	{
		if (!(message = load_message (name, 5, atoi (argument))))
			return 0;
		if ((!str_cmp (name, "Bugs") || !strncasecmp(name, "Typos",5)
			|| !str_cmp (name, "Ideas") || !strncasecmp (name, "Petitions",9)
			|| !str_cmp (name, "Submissions")) && ch->get_trust())
		{
			ch->send_to_char
				("Please enter what you did in response to this report:\n");
			free_mem(ch->desc->pending_message);
			ch->desc->pending_message = new MESSAGE_DATA;
			ch->desc->pending_message->message = NULL;
			ch->desc->pending_message->poster = duplicateString (message->poster);
			ch->desc->descStr = ch->desc->pending_message->message;
			ch->desc->max_str = MAX_STRING_LENGTH;
			ch->desc->proc = post_track_response;
			ch->delay_who = duplicateString (name);
			ch->delay_info1 = atoi (argument);
			ch->make_quiet();
			free_mem(message);
			return 1;
		}
		if (!str_cmp (name, "Prescience") && (ch->get_trust() > 1))
		{
			if (!(ch->delay_ch = load_pc (message->poster)))
			{
				ch->send_to_char
				("I couldn't find the PC that left the prescience request.\n");
				return 0;
			}
			ch->send_to_char
				("Enter the dream you'd like to give in response to this prescience request:\n");
			ch->desc->pending_message = new MESSAGE_DATA;
			ch->desc->pending_message->message = NULL;
			ch->desc->descStr = ch->desc->pending_message->message;
			ch->desc->max_str = STR_MULTI_LINE;
			ch->desc->proc = post_dream;
			ch->make_quiet();
		}
		mysql_safe_query
			("DELETE FROM virtual_boards WHERE post_number = %d AND board_name = '%s'",
			atoi (argument), name);
		free_mem(message);
	}
	if (board_type == 2)
	{
		if (!(message = load_message (name, 7, atoi (argument))))
			return 0;
		free_mem(message);
		mysql_safe_query
			("DELETE FROM player_notes WHERE post_number = %d AND name = '%s'",
			atoi (argument), name);
	}
	if (board_type == 3)
	{
		if (!(message = load_message (name, 8, atoi (argument))))
			return 0;
		free_mem(message);
		mysql_safe_query
			("DELETE FROM player_journals WHERE post_number = %d AND name = '%s'",
			atoi (argument), name);
	}

	return 1;
}

	//Type 1 board - all named boards, admin access only
	//type 2 board - player notes
	//type 3 boards - player journals
int
get_mysql_board_listing (CHAR_DATA * ch, int board_type, char *name)
{
	char query[MAX_STRING_LENGTH] = "";
	char temp_buf[MAX_STRING_LENGTH]= { '\0' };
	MYSQL_RES *result;
	MYSQL_ROW row;

	result = NULL;

	if (board_type == 1)
	{
		mysql_safe_query
			("SELECT * FROM virtual_boards WHERE board_name = '%s' ORDER BY post_number DESC LIMIT 100",
			name);
	}
	else if (board_type == 2)
	{
		mysql_safe_query
			("SELECT CONCAT('\\'',GROUP_CONCAT(aa.name SEPARATOR '\\', \\''),'\\'') AS characters "
			"FROM pfiles aa, pfiles bb "
			"WHERE bb.name = '%s' and aa.account = bb.account "
			"GROUP BY aa.account;", name);
		if ((result = mysql_store_result (database)) == NULL)
		{
			return 0;
		}
		if (mysql_num_rows (result) && (row = mysql_fetch_row (result)) != NULL)
		{
			sprintf (query, "SELECT name,post_number,subject,author,"
				" FROM_UNIXTIME(timestamp,'%%%%b %%%%d %%%%Y' ) AS date "
				"FROM player_notes "
				"WHERE name IN ( %s ) "
				"ORDER BY timestamp DESC, post_number DESC", row[0]);
		}
		mysql_free_result (result);
		if (!*query)
		{
			return 0;
		}
		mysql_safe_query (query);
	}
	else if (board_type == 3)
	{
		mysql_safe_query
			("SELECT * FROM player_journals WHERE name = '%s' ORDER BY post_number DESC",
			name);
	}

	result = mysql_store_result (database);
	if (!result || !mysql_num_rows (result))
	{
		if (result)
			mysql_free_result (result);
		return 0;
	}

	*temp_buf = '\0';
	while ((row = mysql_fetch_row (result)))
	{
		if (strlen (row[2]) > 34)
		{
			sprintf (query, "%s", row[2]);
			query[31] = '.';
			query[32] = '.';
			query[33] = '.';
			query[34] = '\0';
		}
		else
			sprintf (query, "%s", row[2]);
		if (board_type == 2)
		{
			sprintf (temp_buf + strlen (temp_buf),
				((!strcmp (name, row[0])) ?
				" #6%-10.10s %3d - %11s %-10.10s: %s#0\n" :
			" %-10.10s #6%3d#0 - %11s %-10.10s: %s\n"), row[0],
				atoi (row[1]), row[4], row[3], query);
		}
		else
		{
			sprintf (temp_buf + strlen (temp_buf), " #6%3d#0 - %16s %-10.10s: %s\n",
				atoi (row[1]), row[4], row[3], query);
		}
		if (strlen (temp_buf) > B_BUF_SIZE - AVG_STRING_LENGTH)
			break;
	}

	mysql_free_result (result);

	if (!*temp_buf)
		return 0;

	page_string (ch->desc, temp_buf);
	return 1;
}

void
post_to_mysql_virtual_board (DESCRIPTOR_DATA * d)
{
	char *date;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int free_slot = 0;
	time_t current_time;

	if (!d->pending_message->message)
	{
		d->character->send_to_char ("No message posted.\n");
		free_mem(d->pending_message);
		d->pending_message = NULL;
		return;
	}

	current_time = time (0);
	date = asctime (localtime (&current_time));
	if (strlen (date) > 1)
		date[strlen (date) - 1] = '\0';

	mysql_safe_query
		("SELECT post_number FROM virtual_boards WHERE board_name = '%s' ORDER BY post_number DESC LIMIT 1",
		d->pending_message->poster);
	result = mysql_store_result (database);

	if (!result || !mysql_num_rows (result))
	{
		d->character->send_to_char
			("There seems to be a problem with the database listing.\n");
		return;
	}

	free_slot = 1;
	if ((row = mysql_fetch_row (result)))
	{
		free_slot = atoi (row[0]);
		free_slot++;
	}

	mysql_safe_query
		("INSERT INTO virtual_boards VALUES ('%s', %d, '%s', '%s', '%s', '%s', %d)",
		d->pending_message->poster, free_slot, d->pending_message->subject,
		d->character->name, date, d->pending_message->message, (int) time (0));

	if (result)
		mysql_free_result (result);

	free_mem(d->pending_message);
	d->pending_message = NULL;
	free_mem (date);
}

void
post_to_mysql_journal (DESCRIPTOR_DATA * d)
{
	char *date;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int free_slot = 0;
	time_t current_time;

	if (!d->pending_message->message)
	{
		d->character->send_to_char ("No message posted.\n");
		free_mem(d->pending_message);
		d->pending_message = NULL;
		return;
	}

	current_time = time (0);
	date = asctime (localtime (&current_time));
	if (strlen (date) > 1)
		date[strlen (date) - 1] = '\0';

	mysql_safe_query
		("SELECT * FROM player_journals WHERE name = '%s' ORDER BY post_number DESC LIMIT 1",
		d->character->name);
	result = mysql_store_result (database);

	if (!result)
	{
		d->character->send_to_char
			("There seems to be a problem with the database listing.\n");
		return;
	}

	free_slot = 1;
	if ((row = mysql_fetch_row (result)))
	{
		free_slot = atoi (row[1]);
		free_slot++;
	}

	mysql_safe_query
		("INSERT INTO player_journals VALUES ('%s', %d, '%s', '%s', '%s', '%s')",
		d->pending_message->poster, free_slot, d->pending_message->subject,
		d->character->name, date, d->pending_message->message);

	if (result)
		mysql_free_result (result);

	free_mem(d->pending_message);
	d->pending_message = NULL;
}

void
post_to_mysql_player_board (DESCRIPTOR_DATA * d)
{
	char *date;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int free_slot = 0;
	time_t current_time;

	if (!d->pending_message->message)
	{
		d->character->send_to_char ("No message posted.\n");
		free_mem(d->pending_message);
		d->pending_message = NULL;
		return;
	}

	current_time = time (0);
	date = asctime (localtime (&current_time));
	if (strlen (date) > 1)
		date[strlen (date) - 1] = '\0';

	mysql_safe_query
		("SELECT * FROM player_notes WHERE name = '%s' ORDER BY post_number DESC LIMIT 1",
		d->pending_message->poster);
	result = mysql_store_result (database);

	if (!result)
	{
		d->character->send_to_char
			("There seems to be a problem with the database listing.\n");
		return;
	}

	free_slot = 1;
	if ((row = mysql_fetch_row (result)))
	{
		free_slot = atoi (row[1]);
		free_slot++;
	}

	mysql_safe_query
		("INSERT INTO player_notes VALUES ('%s', '%d', '%s', '%s', '%s', '%s', 0, %d)",
		d->pending_message->poster, free_slot, d->pending_message->subject,
		d->character->name, date, d->pending_message->message, (int) time (0));

	if (result)
		mysql_free_result (result);

	free_mem(d->pending_message);
	d->pending_message = NULL;
}

void
add_message_to_mysql_player_notes (const char *name, const char *poster,
								   MESSAGE_DATA * message)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	int free_slot = 0;

	mysql_safe_query
		("SELECT * FROM player_notes WHERE name = '%s' ORDER BY post_number DESC LIMIT 1",
		name);
	result = mysql_store_result (database);
	if (!result)
	{
		return;
	}

	free_slot = 1;
	if ((row = mysql_fetch_row (result)))
	{
		free_slot = atoi (row[1]);
		free_slot++;
	}

	mysql_safe_query
		("INSERT INTO player_notes VALUES ('%s', %d, '%s', '%s', '%s', '%s', %d, %d)",
		name, free_slot, message->subject, poster, message->date,
		message->message, (int) message->flags, (int) time (0));

	if (result)
		mysql_free_result (result);
}

void
add_message_to_mysql_vboard (const char *name, const char *poster, MESSAGE_DATA * message)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	int free_slot = 0;

	mysql_safe_query
		("SELECT * FROM virtual_boards WHERE board_name = '%s' ORDER BY post_number DESC LIMIT 1",
		name);
	result = mysql_store_result (database);

	if (!result)
	{
		return;
	}

	free_slot = 1;
	if ((row = mysql_fetch_row (result)))
	{
		free_slot = atoi (row[1]);
		free_slot++;
	}

	mysql_safe_query
		("INSERT INTO virtual_boards VALUES ('%s', %d, '%s', '%s', '%s', '%s', %d)",
		name, free_slot, message->subject, poster, message->date,
		message->message, (int) time (0));

	if (result)
		mysql_free_result (result);
}


void load_vnpc_timestamp()
{
	MYSQL_RES *result;
	MYSQL_ROW row;

	mysql_safe_query
		("SELECT * FROM shopdata LIMIT 1");

	result = mysql_store_result (database);

	if (!result)
	{
		/* if no result, set to now */
		last_vnpc_sale = time(0);
		return;
	}

	/* get one row */
	if ((row = mysql_fetch_row (result)))
	{
		last_vnpc_sale = atoi(row[0]);
	}

	if (result)
		mysql_free_result (result);
}

void save_vnpc_timestamp()
{
	mysql_safe_query("UPDATE shopdata SET last_vnpc_timestamp=%d",last_vnpc_sale);
}

	//This function will save a specified mobile prototype and the person responsible for makeing the save.
/**
 approved - 0  - pending approval (BUILD_APPROVED)
			1 - pproved (BUILD_PENDING)
			2 - declined (BUILD_DECLINED)
 
 **/
void
save_mysql_mob (CHAR_DATA *builder, CHAR_DATA * tmob, int edit_approval) 
{
	int i;
	int tskill;
	char tmp[MAX_STRING_LENGTH] = { '\0' };
	char tmp_names[MAX_STRING_LENGTH] = { '\0' };
	char tmp_values[MAX_STRING_LENGTH] = { '\0' };
	char tclan_name[MAX_STRING_LENGTH] = { '\0' };
	char tclan_rank[MAX_STRING_LENGTH] = { '\0' };
	char buflong[2*MAX_STRING_LENGTH];
	char *skill_name;
	std::map<std::string, int>::iterator skill_it;
	std::map<std::string, std::string>::iterator clan_it;
	std::string desc_temp;
	std::string sdesc_temp;
	std::string ldesc_temp;
	std::string clan_temp;
	std::string holding;
	std::string skill_buf;
	std::string shop_buf;
	std::string delivery_buf;
	std::string trades_in_buf;
	std::string material_buf;
	std::string material_temp;
	std::string travel_temp;
	std::string voice_temp;
	MYSQL_RES *result;
	

	if (!tmob || tmob->mob->nVirtual == 0)
	{
		return;
	}
	
	if (tmob->deleted == 1)
	{
		mysql_safe_query ("DELETE FROM proto_mobiles WHERE nVirtual = '%d'",
						  tmob->mob->nVirtual);
		return;	
	}
	
		//escape strings as needed
	if (tmob->description)
	{
	mysql_real_escape_string (database, buflong, tmob->description, strlen(tmob->description));
	desc_temp = duplicateString(buflong);
	}
	
	if(tmob->short_descr)
	{
	mysql_real_escape_string (database, buflong, tmob->short_descr, strlen(tmob->short_descr));
	sdesc_temp = duplicateString(buflong);
	}
	
	if(tmob->long_descr)
	{
	mysql_real_escape_string (database, buflong, tmob->long_descr, strlen(tmob->long_descr));
	ldesc_temp = duplicateString(buflong);
	}
	
	if (tmob->travel_str)
	{
		mysql_real_escape_string (database, buflong, tmob->travel_str, strlen(tmob->travel_str));
		travel_temp = duplicateString(buflong);
	}
	
	if (tmob->voice_str)
	{
		mysql_real_escape_string (database, buflong, tmob->voice_str, strlen(tmob->voice_str));
		voice_temp = duplicateString(buflong);
	}
	
	skill_buf.clear();
	shop_buf.clear();
	delivery_buf.clear();
	trades_in_buf.clear();
	clan_temp.clear();
	
		//Accumulate some values to store in a buffers for later use
	
	for (skill_it = tmob->skill_map.begin(); skill_it != tmob->skill_map.end(); skill_it++)
	{
		if (skill_data_map.find(skill_it->first) != skill_data_map.end())
		{
			tskill = skill_it->second;
			if (tskill > 0)
			{
			skill_name = strdup(skill_it->first.c_str());
			
			skill_buf.append(skill_name);
			skill_buf.append(" ");
			sprintf(tmp, "%d",tskill); 
			skill_buf.append(tmp);
			skill_buf.append(" ");
			}
			
		}
	}
		
		//CLANS
	for (clan_it = tmob->char_clan_map.begin(); clan_it != tmob->char_clan_map.end(); clan_it++)
	{
		if (clan_data_map.find(clan_it->first)!=clan_data_map.end())
		{
			sprintf(tclan_name, "%s", clan_it->first.c_str());
			sprintf(tclan_rank, "%s", clan_it->second.c_str());
			
			clan_temp.append(tclan_name);
			clan_temp.append(" \"");
			clan_temp.append(tclan_rank);
			clan_temp.append("\"");
			clan_temp.append(" ");
		}
	}
	
	if(tmob->mob->shop)
	{
			//Deliveries
		for (i = 0; i <= MAX_DELIVERIES; i++)
		{
			if (!tmob->mob->shop->delivery[i])
				break;
			sprintf(tmp, "%d",tmob->mob->shop->delivery[i]);
			delivery_buf.append(tmp);
			delivery_buf.append(" ");
		}
		
		if (!delivery_buf.empty())
		{
			delivery_buf.append("\n");
		}
		
			//trades in items
		for (i = 0; i <= MAX_TRADES_IN; i++)
		{
			if (!tmob->mob->shop->trades_in[i])
				break;
			sprintf(tmp, "%d",tmob->mob->shop->trades_in[i]);
			trades_in_buf.append(tmp);
			trades_in_buf.append(" ");
		}
		
		if (!trades_in_buf.empty())
		{
			trades_in_buf.append("\n");
		}
		
		
			//materials
		std::set<std::string>::iterator materials_it;
		for ( materials_it = tmob->mob->shop->materials.begin(); materials_it != tmob->mob->shop->materials.end(); materials_it++ )
		{
			std::stringstream tmp_str;
			tmp_str << *materials_it << "\n";
			
			material_buf.append(tmp_str.str().c_str());

		}
		if (!material_buf.empty())
		{
			material_buf.append("\n");
		}
	}
	
	if (!material_buf.empty())
	{
	mysql_real_escape_string (database, buflong, material_buf.c_str(), material_buf.length());
	material_temp = duplicateString(buflong);
	}
	
	mysql_safe_query ("SELECT nVirtual FROM proto_mobiles WHERE nVirtual = '%d'",
					  tmob->mob->nVirtual);
	result = mysql_store_result (database);
	
	if (result && mysql_num_rows (result) >= 1) // Update an existing mobile record.
	{				
		mysql_free_result (result);
		
			//general sting data 
		sprintf(tmp, "UPDATE proto_mobiles SET name = '%s', short_desc = '%s', clans = '%s', skills = '%s', long_desc = '%s', description = '%s', voicestr = '%s', travelstr = '%s' ",
				tmob->keywords,
				sdesc_temp.c_str(),
				clan_temp.c_str(),
				skill_buf.c_str(),
				ldesc_temp.c_str(),
				desc_temp.c_str(),
				voice_temp.c_str(),
				travel_temp.c_str());
		
			//Note: tmob->mob->profession and tmob->mob->actions are bitflag
			//so the format string is %ld
		sprintf(tmp + strlen(tmp), ", zone = '%d', str = '%d', intel = '%d', wil = '%d', aur = '%d', dex = '%d', con = '%d', agi = '%d', luk = '%d', speaks = '%d', act = '%ld', profession = '%ld', race = '%d', body_type = '%d', height = '%d', frame = '%d', armor = '%d', max_hit = '%d', damnodice = '%d', damroll = '%d', damsizedice = '%d',  position = '%d', default_pos = '%d', sex = '%d', speed = '%d', spawnpoint = '%d', access_flags = '%d', noaccess_flags = '%d', age = '%d', birth = '%d'",
				tmob->mob->zone,
				tmob->str,
				tmob->intel,
				tmob->wil,
				tmob->aur,
				tmob->dex,
				tmob->con,
				tmob->agi,
				tmob->luk,
				lookup_skill_id(tmob->speaks),
				tmob->mob->action,
				tmob->mob->profession,
				tmob->race,
				tmob->body_type,
				tmob->height,
				tmob->frame,
				tmob->armor,
				tmob->max_hit,
				tmob->mob->damnodice,
				tmob->mob->damroll,
				tmob->mob->damsizedice,
				tmob->position,
				tmob->default_pos,
				tmob->sex,
				tmob->speed,
				tmob->in_room,
				tmob->mob->access_flags,
				tmob->mob->noaccess_flags,
				tmob->age,
				(int)tmob->time_str.birth);
		
			//shopkeeper stuff
		if (IS_SET (tmob->flags, FLAG_KEEPER))
		{
			sprintf(tmp +strlen(tmp), ", keeper = '%d', shop_vnum = '%d', store_vnum = '%d', currency_type = '%d', markup = '%f', discount = '%f', shop_materials = '%s', buy_flags = '%d', nobuy_flags = '%d', delivery = '%s', trades_in ='%s'",
					1,
					tmob->mob->shop->shop_vnum,
					tmob->mob->shop->store_vnum,
					tmob->mob->currency_type,
					tmob->mob->shop->markup,
					tmob->mob->shop->discount,
					material_temp.c_str(),
					tmob->mob->shop->buy_flags,
					tmob->mob->shop->nobuy_flags,
					delivery_buf.c_str(),
					trades_in_buf.c_str(),
					tmob->mob->nVirtual);
		}
		
		if (tmob->mob->merch_seven > 0)
		{
			
			sprintf(tmp +strlen(tmp), ", merch_seven = '%d', markup1 = '%f', discount1 = '%f', econ_flags1 = '%d', markup2 = '%f', discount2 = '%f', econ_flags2 = '%d', markup3 = '%f', discount3 = '%f', econ_flags3 = '%d', markup4 = '%f', discount4 = '%f', econ_flags4 = '%d', markup5 = '%f', discount5 = '%f', econ_flags5 = '%d', markup6 = '%f', discount6 = '%f', econ_flags6 = '%d', markup7 = '%f', discount7 = '%f', econ_flags7 = '%d'",
					tmob->mob->merch_seven,
					tmob->mob->shop->econ_markup1,
					tmob->mob->shop->econ_discount1,
					tmob->mob->shop->econ_flags1,
					tmob->mob->shop->econ_markup2,
					tmob->mob->shop->econ_discount2,
					tmob->mob->shop->econ_flags2,
					tmob->mob->shop->econ_markup3,
					tmob->mob->shop->econ_discount3,
					tmob->mob->shop->econ_flags3,
					tmob->mob->shop->econ_markup4,
					tmob->mob->shop->econ_discount4,
					tmob->mob->shop->econ_flags4,
					tmob->mob->shop->econ_markup5,
					tmob->mob->shop->econ_discount5,
					tmob->mob->shop->econ_flags5,
					tmob->mob->shop->econ_markup6,
					tmob->mob->shop->econ_discount6,
					tmob->mob->shop->econ_flags6,
					tmob->mob->shop->econ_markup7,
					tmob->mob->shop->econ_discount7,
					tmob->mob->shop->econ_flags7);
		}
		else
		{
			sprintf(tmp +strlen(tmp), ", merch_seven = '%d'", tmob->mob->merch_seven);
		}

			//morphing data
		if (tmob->mob->clock)
		{
			sprintf(tmp +strlen(tmp), ", clock = '$d', morphto = '%d', morph_type = '%d'",
					tmob->mob->clock,
					tmob->mob->morphto,
					tmob->mob->morph_type);
		}
		
			//final query
		sprintf (tmp + strlen(tmp), ", last_editor = '%d', status='%s' WHERE nVirtual = '%d'", 
			 (builder != NULL && builder->desc != NULL && builder->desc->acct != NULL ) ? builder->desc->acct->get_id() : 0,
			 (edit_approval == BUILD_APPROVED ) ? "approved" : "pending",
				 tmob->mob->nVirtual);
		
		mysql_safe_query (tmp);
		result = mysql_store_result (database);
		if (!result || !mysql_num_rows (result))
		{
			if (result)
				mysql_free_result (result);
			
		}
		
	}
	else // New mobile record.
	{		
			
		if (result)
			mysql_free_result (result);
		mysql_safe_query ("DELETE FROM proto_mobiles WHERE nVirtual = '%d'",
						  tmob->mob->nVirtual);
		
			//Set up of query
		sprintf(tmp_names, "INSERT INTO proto_mobiles (");
		
			//general data
		sprintf(tmp_names + strlen(tmp_names), "nVirtual, name, short_desc, clans, skills, long_desc, description, voicestr, travelstr ");
		sprintf(tmp_values, " '%d', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s'", 
				tmob->mob->nVirtual,
				tmob->keywords, 
				sdesc_temp.c_str(),
				clan_temp.c_str(), 
				skill_buf.c_str(),
				ldesc_temp.c_str(), 
				desc_temp.c_str(),
				voice_temp.c_str(),
				travel_temp.c_str());
		
		
			//general numeric data	
		sprintf(tmp_names + strlen(tmp_names), ", zone, str, intel, wil, aur, dex, con, agi, luk, speaks, act, profession, race, body_type, height, frame, max_hit, damnodice, damroll, damsizedice, position, default_pos, sex, speed, spawnpoint, access_flags, noaccess_flags, birth");
		
			//Note: tmob->mob->act and tmob->mob->profession are bitflags
			// so the format string is %ld
		sprintf(tmp_values + strlen(tmp_values), ", '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%ld', '%ld', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d'",
				tmob->mob->zone,
				tmob->str,
				tmob->intel,
				tmob->wil,
				tmob->aur,
				tmob->dex,
				tmob->con,
				tmob->agi,
				tmob->luk,
				lookup_skill_id(tmob->speaks),
				tmob->mob->action,
				tmob->mob->profession,
				tmob->race,
				tmob->body_type,
				tmob->height,
				tmob->frame,
				tmob->armor,
				tmob->max_hit,
				tmob->mob->damnodice,
				tmob->mob->damroll,
				tmob->mob->damsizedice,
				tmob->position,
				tmob->default_pos,
				tmob->sex,
				tmob->speed,
				tmob->in_room,
				tmob->mob->access_flags,
				tmob->mob->noaccess_flags,
				(int)tmob->time_str.birth);
		
			//shopkeeper stuff
		if (IS_SET (tmob->flags, FLAG_KEEPER))
		{
			sprintf(tmp_names + strlen(tmp_names), ", keeper, shop_vnum, store_vnum, currency_type, markup, discount, shop_materials, buy_flags, nobuy_flags, delivery, trades_in");
			sprintf(tmp_values + strlen(tmp_values), ", '%d', '%d', '%d', '%d', '%f', '%f', '%s', '%d', '%d', '%s', '%s'",
					1,
					tmob->mob->shop->shop_vnum,
					tmob->mob->shop->store_vnum,
					tmob->mob->currency_type,
					tmob->mob->shop->markup,
					tmob->mob->shop->discount,
					material_temp.c_str(),
					tmob->mob->shop->buy_flags,
					tmob->mob->shop->nobuy_flags,
					delivery_buf.c_str(),
					trades_in_buf.c_str());
			
		}
		
		if (tmob->mob->merch_seven > 0)
		{
			sprintf(tmp_names + strlen(tmp_names), ", markup1, discount1, econ_flags1, markup2, discount2, econ_flags2, markup3, discount3, econ_flags3, markup4, discount4, econ_flags4, markup5, discount5, econ_flags5, markup6, discount6, econ_flags6, markup7, discount7, econ_flags7");
			sprintf(tmp_values + strlen(tmp_values), ", '%f', '%f', '%d' '%f', '%f', '%d', '%f', '%f', '%d', '%f', '%f', '%d', '%f', '%f', '%d', '%f', '%f', '%d', '%f', '%f', '%d'",
					tmob->mob->shop->econ_markup1,
					tmob->mob->shop->econ_discount1,
					tmob->mob->shop->econ_flags1,
					tmob->mob->shop->econ_markup2,
					tmob->mob->shop->econ_discount2,
					tmob->mob->shop->econ_flags2,
					tmob->mob->shop->econ_markup3,
					tmob->mob->shop->econ_discount3,
					tmob->mob->shop->econ_flags3,
					tmob->mob->shop->econ_markup4,
					tmob->mob->shop->econ_discount4,
					tmob->mob->shop->econ_flags4,
					tmob->mob->shop->econ_markup5,
					tmob->mob->shop->econ_discount5,
					tmob->mob->shop->econ_flags5,
					tmob->mob->shop->econ_markup6,
					tmob->mob->shop->econ_discount6,
					tmob->mob->shop->econ_flags6,
					tmob->mob->shop->econ_markup7,
					tmob->mob->shop->econ_discount7,
					tmob->mob->shop->econ_flags7);
		}
		
			//morphing data
		if (tmob->mob->clock)
		{
			sprintf(tmp_names + strlen(tmp_names), ", clock, morphto, morph_type");
			sprintf(tmp_values + strlen(tmp_values), ", '$d', '%d', '%d'",
					tmob->mob->clock, tmob->mob->morphto, tmob->mob->morph_type);
		}	
		
			//final query
		sprintf(tmp, "%s, last_editor, status) VALUES (%s, '%d', '%s')",
				tmp_names, 
				tmp_values,
				(builder != NULL && builder->desc != NULL && builder->desc->acct != NULL ) ? builder->desc->acct->get_id() : 0,
				(edit_approval == BUILD_APPROVED ) ? "approved" : "pending");

		mysql_safe_query (tmp);
		result = mysql_store_result (database);
		if (!result || !mysql_num_rows (result))
		{
			if (result)
				mysql_free_result (result);
			
		}
	}
	return;
	
	
}

	//This function will read in the prototype data for a mob
	//used by boot_mobile to start the game
	//also called by a function to load new or updated mobs

void
load_mobile_mysql (int mob_num)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char *tbuf, *tbuf1, *tbuf2, *tbuf3;
	CHAR_DATA * tmob;
	int ind;
	int lev = 0;
	int skill_num;
	int keep_val;
	int merch_val;
	int tholder;
	std::map<std::string, int> ::iterator skill_it;
	std::map<std::string, SKILL_DATA*>::iterator found_map_skill;
	std::string skillstr;
	
	mysqlpp::Query query = dbo.query();
	
	query << "SELECT * FROM proto_mobiles WHERE nVirtual = '" << mob_num <<"'";
	
	mysqlpp::StoreQueryResult res = query.store();
	if ( res.num_rows() > 0 )
	{
		mysqlpp::Row row = res[0];
		
		if ( row["status"] != "approved" )
		{
				// if not approved, don't load
			sprintf(buf, "Mob %d not approved for loading.\n", mob_num);
			system_log(buf, true);
			return;
		}
		
		
		tmob = new_char (0);
		tmob->mob->nVirtual = mob_num;
		
		tmob->keywords = strdup(row["name"].c_str()); 
		one_argument (tmob->keywords, buf);
		tmob->name = duplicateString (CAP (buf));
		
		tmob->short_descr = strdup(row["short_desc"].c_str());
		
			//CLANS
		if ( row["clans"].length() > 2 )
		{
			tbuf = strdup(row["clans"].c_str());
			while ( 1 )
			{
				tbuf = one_argument (tbuf, buf);
				tbuf = one_argument (tbuf, buf2);
				if (!*buf2)
					break;
				char_clan_add(tmob, buf, buf2);
			}
		}
		
			//SKILLS
				
		/** skill map version of skills **/
		if ( row["skills"].length() > 2 )
		{
			tbuf = strdup(row["skills"].c_str());
			if (*tbuf)
			{	
				
				tbuf1 = strtok (tbuf," \n");
				if (tbuf1 != NULL)
				{
					tbuf2 = strdup(tbuf1);
				}
				
				tbuf1 = strtok (NULL, " \n");
				if (tbuf1 != NULL)
				{
					tbuf3 = strdup(tbuf1);
				}
				while ((tbuf2 != NULL) && (tbuf3 != NULL))
				{
					
					skillstr = strdup(tbuf2);
					lev = atoi(tbuf3);
					
							
					skill_num = lookup_skill_id(skillstr);
					if ((skill_num > 0) && (*tbuf3))
					{
					tmob->skill_map.insert(std::make_pair(lookup_skill_name(skill_num), lev));
					}
					
					tbuf1 = strtok (NULL," \n");
					if (tbuf1 != NULL)
					{
						tbuf2 = strdup(tbuf1);
					}
					else 
						tbuf2 = NULL;
					
					tbuf1 = strtok (NULL, " \n");
					if (tbuf1 != NULL)
					{
						tbuf3 = strdup(tbuf1);
					}
					else
						tbuf3 = NULL;
					
					
				}
				
			}
		}
				
		
		tmob->long_descr = strdup(row["long_desc"].c_str());
		tmob->description = strdup(row["description"].c_str());
		tmob->mob->zone = atoi(row["zone"]);
		tmob->str = atoi(row["str"]);
		tmob->intel = atoi(row["intel"]);
		tmob->wil = atoi(row["wil"]);
		tmob->aur = atoi(row["aur"]);
		tmob->dex = atoi(row["dex"]);
		tmob->con = atoi(row["con"]);
		tmob->agi = atoi(row["agi"]);
		tmob->luk = atoi(row["luk"]);

			//speaks is a char* in game, but saved as an INT in SQL
		tmob->speaks = lookup_skill_name(atoi(row["speaks"]));
		
		tmob->mob->action = atoi(row["act"]);
		tmob->mob->profession = atoi(row["profession"]);
		
		if (tmob->mob->action  || tmob->mob->profession)
			tmob->mob->action |= ACT_ISNPC;
				
		tmob->race = atoi(row["race"]);
		if ((tmob->race >= 0 && tmob->race <= 29 && tmob->race != 28)
			|| tmob->race == 94)
		{				/* Humanoid NPCs. */
			tmob->max_hit = 50 + tmob->con * CONSTITUTION_MULTIPLIER + (MIN(tmob->aur, 25) * 4); // Arbitrary power HP boost - Case
			tmob->hit = tmob->max_hit;
		}
		
		tmob->body_type = atoi(row["body_type"]);
		tmob->height = atoi(row["height"]);
		tmob->frame = atoi(row["frame"]);
		tmob->armor = atoi(row["armor"]);
		tmob->max_hit = atoi(row["max_hit"]);
		tmob->mob->damnodice = atoi(row["damnodice"]);
		tmob->mob->damroll = atoi(row["damroll"]);
		tmob->mob->damsizedice = atoi(row["damsizedice"]);
		tmob->position = atoi(row["position"]);
		tmob->default_pos = atoi(row["default_pos"]);
		tmob->sex = atoi(row["sex"]);
		tmob->speed = atoi(row["speed"]);
		tmob->mob->spawnpoint = atoi(row["spawnpoint"]);
		tmob->mob->access_flags = atoi(row["access_flags"]);
		tmob->mob->noaccess_flags = atoi(row["noaccess_flags"]);
		tmob->mob->clock = atoi(row["clock"]);
		tmob->mob->morphto = atoi(row["morphto"]);
		tmob->mob->morph_type = atoi(row["morph_type"]);
		tmob->age = atoi (row["age"]);
		tmob->time_str.birth = atoi (row["birth"]);
		
			//SHOPS
		keep_val = atoi(row["keeper"]); 
		if (keep_val)
		{
			tmob->flags |= FLAG_KEEPER;
			
			tmob->mob->shop = new SHOP_DATA;
			tmob->mob->shop->shop_vnum = atoi(row["shop_vnum"]); 
			tmob->mob->shop->store_vnum = atoi(row["store_vnum"]); 
			tmob->mob->currency_type = atoi(row["currency_type"]); 
			tmob->mob->shop->markup = atof(row["markup"]); 
			tmob->mob->shop->discount = atof(row["discount"]); 
			tmob->mob->shop->buy_flags = atoi(row["buy_flags"]); 
			tmob->mob->shop->nobuy_flags = atoi(row["nobuy_flags"]); 
			
			if ( row["delivery"] != mysqlpp::null )
			{
				tbuf = strdup(row["delivery"].c_str());
				if (strcmp(tbuf, ""))
				{
					for (ind = 0; ind <= MAX_DELIVERIES; ind++)
					{
						tbuf = one_argument (tbuf, buf);
						tmob->mob->shop->delivery[ind] = atoi(buf);
					}	
				}
			}
			
			if ( row["trades_in"] != mysqlpp::null )
			{
				tbuf = strdup(row["trades_in"].c_str());		
				if (strcmp(tbuf, ""))
				{
					for (ind = 0; ind <= MAX_TRADES_IN; ind++)
					{
						tbuf = one_argument (tbuf, buf);
						tmob->mob->shop->trades_in[ind] = atoi(buf);
					}	
				}
			}
			/**
			if ( row["shop_materials"] != mysqlpp::null )
			{
				tbuf = strdup(row["shop_materials"].c_str());
				if (*tbuf)
				{	
					argument = strtok (tbuf,"\n");
					if (argument != NULL)
					{
						arg1 = strdup(argument);
					}
					while (arg1 != NULL)
					{
						tmob->mob->shop->materials.push_front(arg1);
						argument = strtok (NULL,"\n");
						if (argument != NULL)
						{
							arg1 = strdup(argument);
						}
						else 
							arg1 = NULL;
						
					}
				}
				
			}
		to be restroed when materials are fully introdeuced**/
			merch_val = atoi(row["merch_seven"]);
			
			if (merch_val > 0)
			{
				tmob->mob->shop->econ_markup1 = atof(row["markup1"]);
				tmob->mob->shop->econ_discount1 = atof(row["discount1"]);
				tmob->mob->shop->econ_flags1 = atoi(row["econ_flags1"]);
				tmob->mob->shop->econ_markup2 = atof(row["markup2"]);
				tmob->mob->shop->econ_discount2 = atof(row["discount2"]);
				tmob->mob->shop->econ_flags2 = atoi(row["econ_flags2"]);
				tmob->mob->shop->econ_markup3 = atof(row["markup3"]);
				tmob->mob->shop->econ_discount3 = atof(row["discount3"]);
				tmob->mob->shop->econ_flags3 = atoi(row["econ_flags3"]);
				tmob->mob->shop->econ_markup4 = atof(row["markup4"]);
				tmob->mob->shop->econ_discount4 = atof(row["discount4"]);
				tmob->mob->shop->econ_flags4 = atoi(row["econ_flags4"]);
				tmob->mob->shop->econ_markup5 = atof(row["markup5"]);
				tmob->mob->shop->econ_discount5 = atof(row["discount5"]);
				tmob->mob->shop->econ_flags5 = atoi(row["econ_flags5"]);
				tmob->mob->shop->econ_markup6 = atof(row["markup6"]);
				tmob->mob->shop->econ_discount6 = atof(row["discount6"]);
				tmob->mob->shop->econ_flags6 = atoi(row["econ_flags6"]);
				tmob->mob->shop->econ_markup7 = atof(row["markup7"]);
				tmob->mob->shop->econ_discount7 = atof(row["discount7"]);
				tmob->mob->shop->econ_flags7 = atoi(row["econ_flags7"]);
			}
		}
	}
		//calcualted Values
	
	tmob->move = 50;
	tmob->max_move = 50;
	tmob->time_str.played = 0;
	tmob->time_str.logon = time (0);
	
	tmob->intoxication = 0;
	tmob->hunger = -1;
	tmob->thirst = -1;
	
	tmob->tmp_str = tmob->str;
	tmob->tmp_dex = tmob->dex;
	tmob->tmp_intel = tmob->intel;
	tmob->tmp_aur = tmob->aur;
	tmob->tmp_wil = tmob->wil;
	tmob->tmp_con = tmob->con;
	tmob->tmp_agi = tmob->agi;
	tmob->tmp_luk = tmob->luk;
	
	tmob->equip = NULL;
	
	tmob->desc = 0;
	
		//speaks is a char* in game, but saved as an INT in SQL
	if (tmob->speaks == 0)
	{
		tmob->skill_map["Westron"] = 100;
		tmob->speaks = strdup("Westron");
	}
	
	if (tmob->skill_map[tmob->speaks] > 0)
		tmob->skill_map[tmob->speaks] = 100;
	
	
	if (tmob->race < 0)
		tmob->race = 0;

	
	tholder = atoi (lookup_race_variable (tmob->race, RACE_BODY_PROTO));
	if (tholder)
		tmob->body_proto =tholder;
	
	tholder = atoi (lookup_race_variable (tmob->race, RACE_MIN_HEIGHT));
	if (tholder)
		tmob->mob->min_height =tholder;
	
	tholder = atoi (lookup_race_variable (tmob->race, RACE_MAX_HEIGHT));
	if (tholder)
		tmob->mob->max_height = tholder;
	
	tmob->apply_race_affects();
	
	
	proto_mob_map[tmob->mob->nVirtual] = tmob;
	
	
	return;
}

	//This function will save a specified object and the person responsible for makeing the save.
/**
 status - 
 0 - pending 
 1 - approved 
 2  - declined
 **/

void
save_mysql_object (CHAR_DATA *builder, OBJ_DATA * tobj, int edit_approval)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buflong[2*MAX_STRING_LENGTH];
	MYSQL_RES *result;
	std::string name_temp;
	std::string desc_temp;
	std::string sdesc_temp;
	std::string fdesc_temp;
	std::string xaffect_temp;
	std::string mater_temp;
	std::string clan_temp;
	int i = 0;
	std::set<std::string>::iterator obj_it;
	std::map<int, OBJECT_MATERIAL*>::iterator it_material;
	AFFECTED_TYPE *af;
	
	if (!tobj || tobj->nVirtual == 0)
	{
		return;
	}
	
	if (tobj->deleted == 1)
	{
		sprintf(buf, "DELETE FROM proto_objects WHERE nVirtual = '%d'", tobj->nVirtual);
		mysql_safe_query (buf);
		return;	
	}	
		//escape strings as needed
	if (tobj->name)
	{
	mysql_real_escape_string (database, buflong, tobj->name, strlen(tobj->name));
	name_temp = duplicateString(buflong);
	}
	
	if(tobj->description)
	{
	mysql_real_escape_string (database, buflong, tobj->description, strlen(tobj->description));
	desc_temp = duplicateString(buflong);
	}
	
	if(tobj->short_description)
	{
	mysql_real_escape_string (database, buflong, tobj->short_description, strlen(tobj->short_description));
	sdesc_temp = duplicateString(buflong);
	}
	
	if(tobj->full_description)
	{
	mysql_real_escape_string (database, buflong, tobj->full_description, strlen(tobj->full_description));
	fdesc_temp = duplicateString(buflong);
	}
	
	//convert xaffected into a string
	xaffect_temp.clear();
	for (af = tobj->xaffected; af; af = af->next)
	{
		sprintf(buf, "%d %d \n", af->a.spell.location, af->a.spell.modifier);
		xaffect_temp.append(buf);
	}
	if(!xaffect_temp.empty())
	{
		mysql_real_escape_string (database, buflong, xaffect_temp.c_str(), strlen(xaffect_temp.c_str()));
		xaffect_temp = duplicateString(buflong);
	}
	
	
		//Materials
	mater_temp.clear();
	for (i = 0; i < MAX_OBJ_MATERIALS; i++)
	{
		for (it_material = object_material_map.begin(); it_material != object_material_map.end(); it_material++)
		{
			if (!str_cmp(it_material->second->material_name, tobj->materials[i]))
			{
				mater_temp.append(it_material->second->material_name);
				mater_temp.append(" ");
			}
		}
		
	}
		//clan data
	clan_temp.clear();
	if (tobj->clan_data)
	{
		sprintf(buf, "\"%s\" \"%s\"",tobj->clan_data->name, tobj->clan_data->rank);
		clan_temp.assign(buf);
	}
	if(!clan_temp.empty())
	{
		mysql_real_escape_string (database, buflong, clan_temp.c_str(), strlen(clan_temp.c_str()));
		clan_temp = duplicateString(buflong);
	}
	
	sprintf(buf, "SELECT nVirtual FROM proto_objects WHERE nVirtual = '%d'", tobj->nVirtual);
	mysql_safe_query (buf);
	result = mysql_store_result (database);
	
	if (result && mysql_num_rows (result) >= 1)
	{				// Update an existing object record.
		mysql_free_result (result);
	
		sprintf(buf, "UPDATE proto_objects SET name = '%s', zone = %d, sdesc = '%s', ldesc = '%s', fdesc = '%s', desc_keys = '%s', obj_type = %d, extra_flags = %d, wear_flags = %d, econ_flags = %d, xaffected = '%s', oval0 = %d, oval1 = %d, oval2 = %d, oval3 = %d, oval4 = %d, oval5 = %d, obj_size = %d, weight = %d, quality = %d, ink_color = '%s', coppers = %f, activation = %d, obj_count = %d, clock = %d, morphto = %d, materials = '%s', last_editor = %d, status = '%s', clan_data = '%s' WHERE nVirtual = %d",
				name_temp.c_str(),
				tobj->zone,
				sdesc_temp.c_str(),
				desc_temp.c_str(),
				fdesc_temp.c_str(),
				tobj->desc_keys ? tobj->desc_keys : "",
				tobj->obj_flags.type_flag,
				tobj->obj_flags.extra_flags,
				tobj->obj_flags.wear_flags,
				tobj->econ_flags,
				xaffect_temp.c_str(),
				tobj->o.od.value[0],
				tobj->o.od.value[1],
				tobj->o.od.value[2],
				tobj->o.od.value[3],
				tobj->o.od.value[4],
				tobj->o.od.value[5],
				tobj->size,
				tobj->obj_flags.weight,
				tobj->quality,
				tobj->ink_color,
				tobj->coppers,
				tobj->activation,
				tobj->count,
				tobj->clock,
				tobj->morphto,
				mater_temp.c_str(),				
				( builder != NULL && builder->desc != NULL && builder->desc->acct != NULL ) ? builder->desc->acct->get_id() : 0,
				"approved",
				clan_temp.c_str(),
				tobj->nVirtual);
		mysql_safe_query (buf);

	}
	else
	{
		sprintf(buf, "INSERT INTO proto_objects (nVirtual, name, zone, sdesc, ldesc, fdesc, desc_keys, obj_type, extra_flags, wear_flags, econ_flags, xaffected, oval0, oval1, oval2, oval3, oval4, oval5,   obj_size, weight, quality, ink_color, coppers, activation, obj_count, clock, morphto, materials, last_editor, status, clan_data) VALUES (  %d, '%s', %d, '%s', '%s', '%s', '%s', %d, %d, %d, %d, '%s', %d, %d, %d, %d, %d, %d, %d, %d, %d, '%s', %f, %d, %d, %d, %d, '%s', %d, '%s', '%s')",
				tobj->nVirtual,
				name_temp.c_str(),
				tobj->zone,
				sdesc_temp.c_str(),
				desc_temp.c_str(),
				fdesc_temp.c_str(),
				tobj->desc_keys,					
				tobj->obj_flags.type_flag,
				tobj->obj_flags.extra_flags,
				tobj->obj_flags.wear_flags,
				tobj->econ_flags,
				xaffect_temp.c_str(),
				tobj->o.od.value[0],
				tobj->o.od.value[1],
				tobj->o.od.value[2],
				tobj->o.od.value[3],
				tobj->o.od.value[4],
				tobj->o.od.value[5],
				tobj->size,
				tobj->obj_flags.weight,
				tobj->quality,
				tobj->ink_color,
				tobj->coppers,					
				tobj->activation,
				tobj->count,
				tobj->clock,
				tobj->morphto,
				mater_temp.c_str(),
				( builder != NULL && builder->desc != NULL && builder->desc->acct != NULL ) ? builder->desc->acct->get_id() : 0,
				"approved",
				clan_temp.c_str());
		
		mysql_safe_query (buf);
	}
	
	return;
}

	//This function will read in the prototype data for an obj
	//used by boot_objects to start the game
	//also called by a function to load new or updated objs

void
load_object_mysql (int obj_num)
{
    char *xaffect;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char *tbuf;
	char arg1[MAX_STRING_LENGTH]= { '\0' };
	OBJ_DATA * tobj;
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *taf;
	int i = 1;
	int temp;
	
    mysqlpp::Query query = dbo.query();
	
	
    query << "SELECT * FROM proto_objects WHERE nVirtual = '" << obj_num << "'";
	
    mysqlpp::StoreQueryResult res = query.store();
	
    if ( res.num_rows() < 1 )
	{
		sprintf(buf, "Object %d not loaded.\n", obj_num);
		system_log(buf, true);
		return;		
	}
    else
	{
        mysqlpp::Row row = res[0];
		
        if ( row["status"] != "approved" )
		{
				// if not approved, don't load
			sprintf(buf, "Object %d not approved for loading.\n", obj_num);
			system_log(buf, true);
			return;
		}
		
		
		tobj = new_object();
		tobj->nVirtual = obj_num;
		
        tobj->name = strdup(row["name"]);
        tobj->zone = atoi(row["zone"]);
        tobj->short_description = strdup(row["sdesc"]);
        tobj->description = strdup(row["ldesc"]);
        tobj->full_description = strdup(row["fdesc"]);
        tobj->desc_keys = strdup(row["desc_keys"]);
        tobj->obj_flags.type_flag = (atoi(row["obj_type"]));
        tobj->obj_flags.extra_flags = (atoi(row["extra_flags"]));
        tobj->obj_flags.wear_flags = (atoi(row["wear_flags"]));
        tobj->econ_flags = (atoi(row["econ_flags"]));
		
		/***
        if ( row["xaffected"] != mysqlpp::null )
		{
			while (1)
			{
				af = new AFFECTED_TYPE;
				af->next = NULL;
				af->type = 0;
				af->a.spell.duration = -1;
				af->a.spell.bitvector = 0;
				af->a.spell.sn = 0;
                xaffect = strdup(row["xaffected"]);
                xaffect = one_argument (xaffect, buf);
                xaffect = one_argument (xaffect, buf2);
				if (!*buf2)
					break;
				af->a.spell.location = atoi(buf);
				af->a.spell.modifier = atoi(buf2);
				
				if (af->a.spell.location || af->a.spell.modifier)
				{
					if (!tobj->xaffected)
						tobj->xaffected = af;
					else
					{
						for (taf = tobj->xaffected; taf->next; taf = taf->next)
							;
						taf->next = af;
					}
				}
			}
		}
		***/
		
		tbuf = strdup(row["clan_data"].c_str());
		
			//CLANS
		if (tbuf != NULL && str_cmp (tbuf, "~") && str_cmp (tbuf, " "))
		{
			while (1)
			{
				tbuf = one_argument (tbuf, buf);
				tbuf = one_argument (tbuf, buf2);
				if (!*buf2)
					break;
				else
				{
				tobj->clan_data = new OBJ_CLAN_DATA;
				tobj->clan_data->name = duplicateString (buf);
				tobj->clan_data->rank = duplicateString (buf2);
				}
			}
		}
		
		
		temp = atoi(row["material1"]);
		if ( temp != 0 ) //allows for 'none ' as first material
			tobj->materials[0] = strdup(object_material_map[temp]->material_name);
		
		temp = atoi(row["material2"]);
		if ( temp > 1 ) //will not allow 'none' as second material
			tobj->materials[1] = strdup(object_material_map[temp]->material_name);
		
		temp = atoi(row["material3"]);
		if ( temp > 1 )//will not allow 'none' as third material
			tobj->materials[2] = strdup(object_material_map[temp]->material_name);
		
		tbuf = strdup(row["materials"].c_str());
		if (*tbuf && str_cmp(tbuf,"NULL"))
		{
			for (i = 0; i < MAX_OBJ_MATERIALS; i++)
			{
				tbuf = one_argument(tbuf, arg1);
				if(*arg1 && str_cmp(arg1, "null"))
					tobj->materials[i] = strdup(arg1);
			}
		}	
		
			
        tobj->o.od.value[0] = (atoi(row["oval0"]));
        tobj->o.od.value[1] = (atoi(row["oval1"]));
        tobj->o.od.value[2] = (atoi(row["oval2"]));
        tobj->o.od.value[3] = (atoi(row["oval3"]));
        tobj->o.od.value[4] = (atoi(row["oval4"]));
        tobj->o.od.value[5] = (atoi(row["oval5"]));
        tobj->size = (atoi(row["obj_size"]));
        tobj->obj_flags.weight = (atoi(row["weight"]));
        tobj->quality = (atoi(row["quality"]));
		
        tobj->ink_color = strdup(row["ink_color"]);
		if (atoi(tobj->ink_color) == 0)
			tobj->ink_color = strdup("black ink");
			
        tobj->coppers = (atoi(row["coppers"]));
        tobj->activation = (atoi(row["activation"]));
        tobj->count = (atoi(row["obj_count"]));
        tobj->clock = (atoi(row["clock"]));
        tobj->morphto = (atoi(row["morphto"]));
		
        tobj->last_editor = atoi(row["last_editor"]);
        		
		tobj->in_room = NOWHERE;
		tobj->next_content = 0;
		tobj->carried_by = 0;
		tobj->equiped_by = 0;
		tobj->in_obj = 0;
		tobj->contains = 0;
		tobj->obj_flags.set_cost = 0;
		tobj->current_damage = 0;
				
		tobj->max_hit_points = (get_material_hardness(tobj) * (tobj->obj_flags.weight)/100.0);
		if (tobj->max_hit_points > 0 && tobj->max_hit_points < 1)
			tobj->max_hit_points = 1;
		
		if (tobj->count == 0)
			tobj->count = 1;
		
		proto_obj_map[tobj->nVirtual] = tobj;
		return;
	}
}

void
save_mysql_room (CHAR_DATA *builder, ROOM_DATA * troom, int edit_approval)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buflong[2*MAX_STRING_LENGTH];
	int index;
	MYSQL_RES *result;
	std::string desc_temp;
	std::string port_desc;
	std::string name_temp;
	std::string weather_temp;
	std::string alas_temp;
	std::string dir_temp;
	std::string ex_descr_temp;
	std::string secret_temp;
	
	if (!troom || troom->nVirtual == 0)
	{
		return;
	}
	
	if (troom->deleted == 1)
	{
		sprintf(buf, "DELETE FROM rooms WHERE nVirtual = '%d'", troom->nVirtual);
		mysql_safe_query (buf);
		return;	
	}	
	
		//secret desciptions and values	
	secret_temp.empty();
	for (index = 0; index < LAST_DIR; index++)
	{
		if (troom->secrets[index])
		{
			sprintf(buf, "%d~%s~", 
					troom->secrets[index]->diff, 
					troom->secrets[index]->stext);
			secret_temp.append(buf);
		}
	}
	if (!secret_temp.empty())
	{
		mysql_real_escape_string (database, buflong, secret_temp.c_str(), secret_temp.length());
		secret_temp = duplicateString(buflong);
	}

	
		//Extended desciptions for virtual objects	
	ex_descr_temp.empty();
	for (index = 0; index < MAX_EX_DESCR; index++)
	{
		if (troom->ex_description[index])
		{
			sprintf(buf, "%s~%s~", 
					troom->ex_description[index]->keyword, 
					troom->ex_description[index]->description);
			ex_descr_temp.append(buf);
		}
	}
	if (!ex_descr_temp.empty())
	{
		mysql_real_escape_string (database, buflong, ex_descr_temp.c_str(), ex_descr_temp.length());
		ex_descr_temp = duplicateString(buflong);
	}
	
		//for extra descriptions based on weather
	weather_temp.empty();
	if (troom->extra && troom->extra->weather_desc)
	{
			//don't save WR_NORMAL whcih is the last description
		for (index = 0; index < WR_LAST_DESCRIPTIONS; index++)
		{
			if (troom->extra->weather_desc[index].empty())
			{
				sprintf(buf, "~");
			}
			else 
			{
				sprintf(buf, "%s~", troom->extra->weather_desc[index].c_str());
			}
			
			weather_temp.append(buf);
			
		}
	}
	else
	{
		for (index = 0; index < WR_LAST_DESCRIPTIONS; index++)
		{
			sprintf(buf, "~");
			weather_temp.append(buf);
		}
	}
	
	if(!weather_temp.empty())
	{
		mysql_real_escape_string (database, buflong, weather_temp.c_str(), weather_temp.length());
		weather_temp = duplicateString(buflong);
	}
	
		//Alas descriptions
	alas_temp.empty();
	if (troom->extra && troom->extra->alas)
	{
			//for alas directions
		for (index = 0; index <= LAST_DIR; index++)
		{
			if(troom->extra->alas[index].empty())
			{
				sprintf(buf, "~" );
			}
			else 
			{
				
				sprintf(buf, "%s~", troom->extra->alas[index].c_str());
			}
			alas_temp.append(buf);
		}
	}
	else
	{
		for (index = 0; index <= LAST_DIR; index++)
		{
			sprintf(buf, "~");
			alas_temp.append(buf);
		}
	}
	
	if(!alas_temp.empty())
	{
		mysql_real_escape_string (database, buflong, alas_temp.c_str(), alas_temp.length());
		alas_temp = duplicateString(buflong);
	}
	
		//additonal strings to be escaped
	if(troom->name)
	{
		mysql_real_escape_string (database, buflong, troom->name, strlen(troom->name));
		name_temp = duplicateString(buflong);
	}
	
	if(troom->description)
	{
		mysql_real_escape_string (database, buflong, troom->description, strlen(troom->description));
		desc_temp = duplicateString(buflong);
	}
	
	
	sprintf(buf, "SELECT nVirtual FROM rooms WHERE nVirtual = '%d'", troom->nVirtual);	
	mysql_safe_query (buf);
	result = mysql_store_result (database);
	
	if (result && mysql_num_rows (result) >= 1)
	{				// Update an existing room record.
		mysql_free_result (result);
		
		sprintf(buf, "UPDATE rooms SET name = '%s', build_zone = %d, weather_zone = %d, enforcer_zone = %d, x_coor = %f, y_coor = %f, z_coor = %f, size = %d, capacity = %d, description = '%s', terrain = %d, flags = %d, extra_weather = '%s', extra_alas = '%s', ex_description = '%s', secrets = '%s', last_editor = %d, status = '%s' WHERE nVirtual = %d",
				name_temp.c_str(),
				troom->zone,
				troom->wzone,
				troom->enforcezone,
				troom->x_coord, 
				troom->y_coord,
				troom->z_coord,
				troom->room_size,
				troom->capacity,
				desc_temp.c_str(),
				troom->terrain_type,
				troom->room_flags,
				weather_temp.c_str(),
				alas_temp.c_str(),
				ex_descr_temp.c_str(),
				secret_temp.c_str(),
				( builder != NULL && builder->desc != NULL && builder->desc->acct != NULL ) ? builder->desc->acct->get_id() : troom->last_editor,
				"approved",
				troom->nVirtual);
		mysql_safe_query (buf);
	}
	else
	{
		sprintf(buf, "INSERT INTO rooms (nVirtual, name, build_zone, weather_zone, enforcer_zone, x_coor, y_coor, z_coor, size, capacity, description, terrain, flags, extra_weather, extra_alas, ex_description, secrets, last_editor, status) VALUES (  %d, '%s', %d, %d, %d, %f, %f, %f,  %d, %d, '%s', %d, %d,'%s', '%s', '%s', '%s', %d, '%s') ;",
				troom->nVirtual,
				name_temp.c_str(),
				troom->zone,
				troom->wzone,
				troom->enforcezone,
				troom->x_coord, 
				troom->y_coord,
				troom->z_coord,
				troom->room_size,
				troom->capacity,
				desc_temp.c_str(),
				troom->terrain_type,
				troom->room_flags,
				weather_temp.c_str(),
				alas_temp.c_str(),
				ex_descr_temp.c_str(),
				secret_temp.c_str(),
				( builder != NULL && builder->desc != NULL && builder->desc->acct != NULL ) ? builder->desc->acct->get_id() : troom->last_editor,
				"approved",
				edit_approval);
		
		mysql_safe_query (buf);
	}
	
	return;
}

void
load_room_mysql (int room_num)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char *arg1;
	char *arg2;
	char *argument;
	char *tbuf;
	int index;
	ROOM_DATA *troom;
    mysqlpp::Query query = dbo.query();
	std::string temp_str;
	
	query << "SELECT nVirtual, name, build_zone, weather_zone, enforcer_zone, x_coor, y_coor, "
	"z_coor, size, capacity, description, terrain, flags, extra_weather, extra_alas, "
	"ex_description, affects, secrets, last_editor, status FROM rooms WHERE nVirtual = '" << room_num << "'";
	
	mysqlpp::StoreQueryResult res = query.store();
	if ( res.num_rows() > 0 ) 
	{
		mysqlpp::Row row = res[0];
		
		if ( row["status"] != "approved" )
		{
				// if not approved, don't load
			sprintf(buf, "Room %d not approved for loading.\n", room_num);
			system_log(buf, true);
			return;
		}
		
		troom = new_room(room_num);
		troom->nVirtual = atoi(row["nVirtual"]);
		troom->zone = atoi(row["build_zone"]);
		troom->wzone = atoi(row["weather_zone"]);
		troom->enforcezone = atoi(row["enforcer_zone"]);
		troom->x_coord = atof(row["x_coor"]);
		troom->y_coord = atof(row["y_coor"]);
		troom->z_coord = atof(row["z_coor"]);
		troom->room_size = atoi(row["size"]);
		troom->capacity = atoi(row["capacity"]);
		troom->terrain_type = atoi(row["terrain"]);
		troom->room_flags = atoi(row["flags"]);
		troom->last_editor = atoi(row["last_editor"]);
		troom->name = strdup(row["name"]);
		troom->description= strdup(row["description"]);
		
		
		if (row["extra_weather"] != mysqlpp::null)
		{
			argument = (char *)row["extra_weather"].c_str();
			if (*argument)
			{
				*buf2 = '\0';
				
				if (!troom->extra)
				{
					troom->extra = new ROOM_EXTRA_DATA;
				}
				index = 0;			
				while (*argument)
				{
					switch (*argument)
					{
						case '~':
							troom->extra->weather_desc[index] = duplicateString(buf2);
							index++;
							*buf2 = '\0';
							break;
						default:
							sprintf (buf2 + strlen (buf2), "%c", *argument);
							break;
					}
					
					*argument++;
				}
			}
		}
		else
		{
			troom->extra = new ROOM_EXTRA_DATA;
		}
		
		troom->extra->weather_desc[WR_NORMAL] = duplicateString(troom->description);
		
		
		
		if ( row["extra_alas"] != mysqlpp::null )
		{
			argument = (char *)row["extra_alas"].c_str();
			if (*argument)
			{
				*buf2 = NULL;
				
				if (!troom->extra)
				{
					troom->extra = new ROOM_EXTRA_DATA;
				}
				index = 0;			
				while (*argument)
				{
					switch (*argument)
					{
						case '~':
							troom->extra->alas[index] = duplicateString(buf2);
							index++;
							*buf2 = NULL;
							break;
						default:
							sprintf (buf2 + strlen (buf2), "%c", *argument);
							break;
					}
					
					*argument++;
				}
			}
		}
		else
		{
			troom->extra = new ROOM_EXTRA_DATA;
		}
		
			//arg1 is the key word
			//arg2 is the description
			//breaks between ex_desc is a ~
		if ( row["ex_description"] != mysqlpp::null )
		{
			index = 0;
			tbuf = (char *)row["ex_description"].c_str();
			if (*tbuf)
			{	
				
				argument = strtok (tbuf,"~");
				if (argument != NULL)
				{
					arg1 = strdup(argument);
				}
				
				argument = strtok (NULL, "~");
				if (argument != NULL)
				{
					arg2 = strdup(argument);
				}
				while ((arg1 != NULL) && (arg2 != NULL) && (index < MAX_EX_DESCR))
				{
					if ((arg1 != NULL) && (arg2 != NULL))
					{
						troom->ex_description[index] = new EXTRA_DESCR_DATA;
						troom->ex_description[index]->keyword = strdup(arg1);
						troom->ex_description[index]->description = strdup(arg2);
						index ++;
						argument = strtok (NULL,"~");
						if (argument != NULL)
						{
							arg1 = strdup(argument);
						}
						else 
							arg1 = NULL;
						
						argument = strtok (NULL, "~");
						if (argument != NULL)
						{
							arg2 = strdup(argument);
						}
						else
							arg2 = NULL;
						
					}
				}
				
			}
		}
		
		/*** secrets ***/
			//arg1 is the difficulty rating - an integer
			//arg2 is the descriptive string
			//breaks between secrets is a ~
		if ( row["secrets"] != mysqlpp::null )
		{
			index = 0;
			tbuf = (char *)row["secrets"].c_str();
			if (*tbuf)
			{	
				
				argument = strtok (tbuf,"~");
				if (argument != NULL)
				{
					arg1 = strdup(argument);
				}
				
				argument = strtok (NULL, "~");
				if (argument != NULL)
				{
					arg2 = strdup(argument);
				}
				while ((arg1 != NULL) && (arg2 != NULL) && (index < LAST_DIR))
				{
					if ((arg1 != NULL) && (arg2 != NULL))
					{
						troom->secrets[index] = new secret;
						troom->secrets[index]->diff = atoi(arg1);
						troom->secrets[index]->stext = strdup(arg2);
						index ++;
						argument = strtok (NULL,"~");
						if (argument != NULL)
						{
							arg1 = strdup(argument);
						}
						else 
							arg1 = NULL;
						
						argument = strtok (NULL, "~");
						if (argument != NULL)
						{
							arg2 = strdup(argument);
						}
						else
							arg2 = NULL;
						
					}
				}
				
			}
		}
		
		
		/*** end secrets ***/
		
			//affects - needs to be parsed properly - row["affects"]
		troom->affects = NULL;
		
			//portals
		
		query << "SELECT ident FROM portals LEFT JOIN rooms ON rooms.nVirtual = portals.room_1 OR rooms.nVirtual = portals.room_2 WHERE portals.to_place = '1' AND rooms.nVirtual = ' " << room_num << "'";
		
		mysqlpp::UseQueryResult subRes = query.use();
		
		index = 0;
		while ( mysqlpp::Row subRow = subRes.fetch_row() )
		{
			if ( index >= MAX_PORTALS )
				break;
			troom->portals[index] = subRow["ident"];
			index++;
		}
		
		portals_to_dir_options(troom);
		
		room_map[troom->nVirtual] = troom;
	}
	else
	{
			// if no result, panic?
		sprintf(buf, "Error loading room %d.\n", room_num);
		system_log(buf, true);
		return;
	}
	
		//load the stuff in the room
	load_save_room(troom);
}


void
load_portals_mysql (int port_num)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char *tbuf;
	char *argument;
	char *arg1;
	std::string line;
	ROOM_PORTAL_DATA *tport;
	mysqlpp::Query query = dbo.query();
	
	query << "SELECT * FROM portals WHERE ident = '" << port_num << "'";
	
	mysqlpp::StoreQueryResult res = query.store();
	if ( res.num_rows() > 0 )
	{
		mysqlpp::Row row = res[0];
		
		if ( row["status"] != "approved" )
		{
				// if not approved, don't load
			sprintf(buf, "Portal %d not approved for loading.\n", port_num);
			system_log(buf, true);
			return;
		}
		
		tport = new room_portal_data;
		
		tport->ident = atoi(row["ident"]);
		tport->zone = atoi(row["zone"]);
		tport->type = atoi(row["type"]);
		tport->port_flags = atoi(row["port_flags"]);
		tport->key_num_1 = atoi(row["key_num_1"]);
		tport->key_num_2 = atoi(row["key_num_2"]);
		tport->sdesc_1 = strdup(row["sdesc_1"].c_str());
		tport->sdesc_2 = strdup(row["sdesc_2"].c_str());
		tport->ldesc_1 = strdup(row["ldesc_1"].c_str());
		tport->ldesc_2 = strdup(row["ldesc_2"].c_str());
		tport->fdesc_1 = strdup(row["fdesc_1"].c_str());
		tport->fdesc_2 = strdup(row["fdesc_2"].c_str());
		tport->keywords_1 = strdup(row["keywords_1"].c_str());
		tport->keywords_2 = strdup(row["keywords_2"].c_str());
		tport->room_1 = atoi(row["room_1"]);
		tport->room_2 = atoi(row["room_2"]);
		
		
		if (row["material"] != mysqlpp::null)
		{
			tbuf = strdup(row["material"].c_str());
			if (*tbuf)
			{	
				tport->material.clear();
				argument = strtok (tbuf," \n\r");
				if (argument != NULL)
				{
					arg1 = strdup(argument);
				}
				while (arg1 != NULL)
				{
					if(find_material(arg1))
					{
						tport->material.push_front(arg1);
					}
					
					argument = strtok (NULL," \n\r");
					if (argument != NULL)
					{
						arg1 = strdup(argument);
					}
					else 
						arg1 = NULL;
					
					
				}
			}
		}
		
		tport->dir_1 = atoi(row["dir_1"]);
		tport->dir_2 = atoi(row["dir_2"]);
		tport->pick_key_pen_1 = atoi(row["pick_key_pen_1"]);
		tport->pick_key_pen_2 = atoi(row["pick_key_pen_2"]);
		tport->quality = atoi(row["quality"]);
		tport->difficulty = atoi(row["difficulty"]);
		tport->slope = atol(row["slope"]);
		tport->skill = atoi(row["skill"]);
		tport->to_place = atoi(row["to_place"]);
		
		tport->fail_room = atoi(row["fail_room"]);
		tport->feet_fallen = atoi(row["feet_fall"]);
		
		portal_map[tport->ident] = tport;
		
		if (tport->to_place == 1)
		{
			if ((vtor(tport->room_1)) 
				&& (is_portal_placed(tport, vtor(tport->room_1))))
				portals_to_dir_options(vtor(tport->room_1));
			else 
			{
				tport->to_place = 0;
			}
			
			if ((vtor(tport->room_2)) 
				&& (is_portal_placed(tport, vtor(tport->room_2))))
				portals_to_dir_options(vtor(tport->room_2));
			else 
			{
				tport->to_place = 0;
			}
		}
		
		
	}
	return;
}

void
save_mysql_portal (CHAR_DATA *builder, ROOM_PORTAL_DATA * tport, int edit_approval)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buflong[2*MAX_STRING_LENGTH];
	MYSQL_RES *result;
	std::string sdesc_1;
	std::string sdesc_2;
	std::string ldesc_1;
	std::string ldesc_2;
	std::string fdesc_1;
	std::string fdesc_2;
	std::string keywords_1;
	std::string keywords_2;
	std::string port_material_temp;
	std::string port_material_buf;
	std::list<std::string>::iterator material_it;
	
	
	if (!tport || tport->ident == 0)
	{
		return;
	}
	
	if (tport->deleted == 1)
	{
		sprintf(buf, "DELETE FROM portals WHERE ident = '%d'", tport->ident);
		mysql_safe_query (buf);
		return;	
	}	
	
		//escape strings
	if (tport->sdesc_1)
	{
	mysql_real_escape_string (database, buflong, tport->sdesc_1, strlen(tport->sdesc_1));
	sdesc_1 = duplicateString(buflong);
	}
	
	if (tport->sdesc_2)
	{
		mysql_real_escape_string (database, buflong, tport->sdesc_2, strlen(tport->sdesc_2));
	sdesc_2 = duplicateString(buflong);
	}
	
	if (tport->ldesc_1)
	{
	mysql_real_escape_string (database, buflong, tport->ldesc_1, strlen(tport->ldesc_1));
	ldesc_1 = duplicateString(buflong);
	}
	
	if (tport->ldesc_2)
	{
		mysql_real_escape_string (database, buflong, tport->ldesc_2, strlen(tport->ldesc_2));
	ldesc_2 = duplicateString(buflong);
	}
	
	if (tport->fdesc_1)
	{
	mysql_real_escape_string (database, buflong, tport->fdesc_1, strlen(tport->fdesc_1));
	fdesc_1 = duplicateString(buflong);
	}
	
	if (tport->fdesc_2)
	{
	mysql_real_escape_string (database, buflong, tport->fdesc_2, strlen(tport->fdesc_2));
	fdesc_2 = duplicateString(buflong);
	}
	
	if (tport->keywords_1)
	{
	mysql_real_escape_string (database, buflong, tport->keywords_1, strlen(tport->keywords_1));
	keywords_1 = duplicateString(buflong);
	}
	
	if (tport->keywords_2)
	{
	mysql_real_escape_string (database, buflong, tport->keywords_2, strlen(tport->keywords_2));
	keywords_2 = duplicateString(buflong);
	}
	
		//Materials
	port_material_buf.clear();
	std::stringstream tmp_str;
	std::string mater_str;
	if (!tport->material.empty())
	{
		for (material_it = tport->material.begin();
			 material_it != tport->material.end();
			 material_it++ )
		{
			mater_str = *material_it;
			if (find_material((char*)mater_str.c_str()))
			{
				tmp_str << *material_it << "\n";
				port_material_buf.append(tmp_str.str().c_str());	
			}
		}
	}
	
	if(!port_material_buf.empty())
	{
		mysql_real_escape_string (database, buflong, port_material_buf.c_str(), strlen(port_material_buf.c_str()));
		port_material_temp = duplicateString(buflong);
	}
	
	sprintf(buf, "SELECT ident FROM portals WHERE ident = '%d'", tport->ident);
	mysql_safe_query (buf);
	result = mysql_store_result (database);
	
	if (result && mysql_num_rows (result) >= 1)
	{				// Update an existing portal record.
		mysql_free_result (result);
		
		sprintf(buf, "UPDATE portals SET zone = %d, type = %d, port_flags = %d, key_num_1 = %d, key_num_2 = %d, sdesc_1 = '%s', sdesc_2 = '%s', ldesc_1 = '%s', ldesc_2 = '%s', fdesc_1 = '%s', fdesc_2 = '%s', keywords_1 = '%s', keywords_2 = '%s', room_1 = %d, room_2 = %d, material = '%s', dir_1 = %d, dir_2 = %d, quality = %d, difficulty = %d, pick_key_pen_1 = %d, pick_key_pen_2 = %d, slope = %f, skill = %d, last_editor = %d, status = '%s', to_place = %d, fail_room = %d, feet_fall = %d WHERE ident = %d", 
				tport->zone,
				tport->type,
				tport->port_flags,
				tport->key_num_1,
				tport->key_num_2,
				sdesc_1.c_str(),
				sdesc_2.c_str(),
				ldesc_1.c_str(),
				ldesc_2.c_str(),
				fdesc_1.c_str(),
				fdesc_2.c_str(),
				keywords_1.c_str(),
				keywords_2.c_str(),
				tport->room_1,
				tport->room_2,
				port_material_temp.c_str(),
				tport->dir_1,
				tport->dir_2,
				tport->quality,
				tport->difficulty,
				tport->pick_key_pen_1,
				tport->pick_key_pen_2,
				tport->slope,
				tport->skill,
				( builder != NULL && builder->desc != NULL && builder->desc->acct != NULL ) ? builder->desc->acct->get_id() : 0,
				"approved",
				tport->to_place ? 1 : 0,
				tport->fail_room,
				tport->feet_fallen,
				tport->ident);
		mysql_safe_query (buf);
	}
	else
	{
		sprintf(buf, "INSERT INTO portals (ident, zone, type, port_flags, key_num_1, key_num_2, sdesc_1, sdesc_2, ldesc_1, ldesc_2, fdesc_1, fdesc_2, keywords_1, keywords_2, room_1, room_2, material, dir_1, dir_2, quality, difficulty, pick_key_pen_1, pick_key_pen_2, slope, skill, last_editor, status, to_place, fail_room, feet_fall) VALUES (%d, %d, %d, %d, %d, %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d, %d, '%s', %d, %d, %d, %d, %d, %d, %f, %d, %d, '%s', %d, %d, %d)",
				tport->ident,
				tport->zone,
				tport->type,
				tport->port_flags,
				tport->key_num_1,
				tport->key_num_2,
				sdesc_1.c_str(),
				sdesc_2.c_str(),
				ldesc_1.c_str(),
				ldesc_2.c_str(),
				fdesc_1.c_str(),
				fdesc_2.c_str(),
				keywords_1.c_str(),
				keywords_2.c_str(),
				tport->room_1,
				tport->room_2,
				port_material_temp.c_str(),
				tport->dir_1,
				tport->dir_2,
				tport->quality,
				tport->difficulty,
				tport->pick_key_pen_1,
				tport->pick_key_pen_2,
				tport->slope,
				tport->skill,
				( builder != NULL && builder->desc != NULL && builder->desc->acct != NULL ) ? builder->desc->acct->get_id() : 0,
				"approved",
				tport->to_place ? 1 : 0,
				tport->fail_room,
				tport->feet_fallen);
		mysql_safe_query (buf);

	}
	
	return;
	
}

void
save_mysql_crafts(CHAR_DATA *builder, SUBCRAFT_HEAD_DATA *craft, int edit_approval)
{
	SUBCRAFT_HEAD_DATA *tcraft;
	DEFAULT_MOB_DATA *mobs;
	PHASE_DATA *phase;
	int i, j;
	char flag[MAX_STRING_LENGTH]= { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buflong[2*MAX_STRING_LENGTH];
	std::string races;
	std::string clans;
	std::string opening_for;
	std::string terrain_craft;
	std::string season_craft;
	std::string weather_craft;
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	MYSQL_RES *result;
	
	if (!craft)
		return;
	
	races.clear();
	if (craft->race)
	{
		for (i = 0; i < RACEMAX; i++)
		{
			if (craft->race[i] > 0)
			{
				sprintf(buf, "\"%s\" ", lookup_race_variable (craft->race[i], RACE_NAME));
				races.append(buf);
			}
		}
	}
	
	clans.clear();
	if (craft->clans)
	{
		sprintf(buf, "%s", craft->clans);
		clans.append(buf);
	}
	
	opening_for.clear();
	if (craft->opening)
	{
		for (i = 0; i < OPENINGMAX; i++)
		{
			if (craft->opening[i] > 0)
			{
				sprintf(buf, "%s ", lookup_skill_name(craft->opening[i]));
				opening_for.append(buf);
			}
		}
	}
	
	terrain_craft.clear();
	if (craft->terrains)
	{
		for (i = 0; i < TERRAINSMAX; i++)
		{
			if (craft->terrains[i] > 0)
			{
				sprintf(buf, "%s ", terrain_types[craft->terrains[i] - 1]);
				terrain_craft.append(buf);
			}
		}
	}
	
	season_craft.clear();
	if (craft->seasons)
	{
		for (i = 0; i < SEASONSMAX; i++)
		{
			if (craft->seasons[i] > 0)
			{
				sprintf(buf, "%s ", seasons[craft->seasons[i] - 1]);
				season_craft.append(buf);
			}
		}
	}
	
	weather_craft.clear();
	if (craft->weather)
	{
		for (i = 0; i < WEATHERMAX; i++)
		{
			if (craft->weather[i] > 0)
			{
				sprintf(buf, "%s ", weather_states[craft->weather[i] - 1]);
				weather_craft.append(buf);
			}
		}
	}
	
	
	
	
	sprintf(buf, "SELECT subcraft_name FROM crafts WHERE subcraft_name = '%s'", craft->subcraft_name);
	mysql_safe_query (buf);
	result = mysql_store_result (database);
	
	if (result && mysql_num_rows (result) >= 1)
	{				// Update an existing portal record.
		mysql_free_result (result);
		
		sprintf(buf, "UPDATE crafts SET craft_name = '%s', command = '%s', open_for_skill = '%s', race = '%s', clans = '%s', terrains = '%s', seasons = '%s', weather = '%s', subcraft_flags = %d, delay = %d, key_first = %d, key_end = %d, followers = %d, message = '%s', last_editor = %d, status = '%s' WHERE subcraft_name = '%s'",
				craft->craft_name,
				craft->command,
				opening_for.c_str(),
				races.c_str(),
				clans.c_str(),
				terrain_craft.c_str(),
				season_craft.c_str(),
				weather_craft.c_str(),
				craft->subcraft_flags,
				craft->delay,
				craft->key_first,
				craft->key_end,
				craft->followers,
				craft->personal_string,	
				(builder != NULL && builder->desc != NULL && builder->desc->acct != NULL ) ? builder->desc->acct->get_id() : 0,
				(edit_approval == BUILD_APPROVED ) ? "approved" : "pending",
				craft->subcraft_name);
		
		mysql_safe_query (buf);
	}
	else
	{
		sprintf(buf, "INSERT INTO crafts (craft_name,	subcraft_name, command, open_for_skill, race, clans, terrains, seasons, weather, subcraft_flags, delay, key_first, key_end, followers, message, last_editor, status) VALUES ('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d, %d, %d, %d, %d, '%s', %d, '%s')",
				craft->craft_name,
				craft->subcraft_name,
				craft->command,
				opening_for.c_str(),
				races.c_str(),
				clans.c_str(),
				terrain_craft.c_str(),
				season_craft.c_str(),
				weather_craft.c_str(),
				craft->subcraft_flags,
				craft->delay,
				craft->key_first,
				craft->key_end,
				craft->followers,
				craft->personal_string,
				(builder != NULL && builder->desc != NULL && builder->desc->acct != NULL ) ? builder->desc->acct->get_id() : 0,
				(edit_approval == BUILD_APPROVED ) ? "approved" : "pending");
		mysql_safe_query (buf);
	}
	
		//for each phase call save_mysql_phase(craft, phase, phase_num)
	
	for (i = 1; i < MAX_PHASES_PER_SUBCRAFT; i++)
	{
		phase = craft->phases[i];
		if (phase)
			save_mysql_phase(craft, phase, i);
		
		
			//save each of the default_item_data and default_mob_data
			//items[0] does not exist. hard for normal people to understand object0
		for (j = 1; j < MAX_ITEMS_PER_SUBCRAFT; j++)
		{
			if ((craft->obj_items[j] > 0) && (craft->obj_items[j]->phase_num == i))
				save_mysql_craft_element(craft, craft->obj_items[j], NULL, "obj", j);
		}
		
		for (j = 1; j < MAX_MOBS_PER_SUBCRAFT; j++)
		{
			if ((craft->mob_items[j] > 0) &&
				(craft->mob_items[j]->phase_num == i))
				save_mysql_craft_element(craft, NULL, craft->mob_items[j], "mob", j);
		}
		
	}
	
	
	return;
	
}	
	
void
save_mysql_phase(SUBCRAFT_HEAD_DATA *craft, PHASE_DATA *phase, int phase_num)
{
	
	int i, j;
	char flag[MAX_STRING_LENGTH]= { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };
	MYSQL_RES *result;
	MYSQL_ROW row;
	int tcraft_id = 0;
	
	
	
		//get the subcraft_id
	sprintf(buf, "SELECT craft_id FROM crafts WHERE subcraft_name = '%s'", craft->subcraft_name);
	mysql_safe_query (buf);
	result = mysql_store_result (database);
	if (result && mysql_num_rows (result) >= 1)
	{
		row = mysql_fetch_row(result);
		tcraft_id = atoi(row[0]);
	}
	else
	{
		sprintf(buf, "Error saving craft! - Craft_id not found");
		system_log(buf, true);
		return;
	}

		//get the phase
	sprintf(buf, "SELECT craft_id FROM craft_phases WHERE craft_id = %d AND phase_num = %d", tcraft_id, phase_num);
	mysql_safe_query (buf);
	result = mysql_store_result (database);
	
		//if the phase exist, update it
	if (result && mysql_num_rows (result) >= 1)
	{				// Update an existing portal record.
		mysql_free_result (result);
		
		sprintf(buf, "UPDATE craft_phases SET first = '%s', third = '%s', group_mess = '%s', phase_seconds = %d, skill = %d, attribute = %d, dice = %d, sides = %d, move_cost = %d, hit_cost = %d WHERE craft_id = %d AND phase_num = %d",
				phase->first,
				phase->third,
				phase->group_mess,
				phase->phase_seconds,
				phase->skill,
				phase->attribute,
				phase->dice,
				phase->sides,
				phase->move_cost,
				phase->hit_cost,
				tcraft_id,
				phase_num);
		
		mysql_safe_query (buf);
	}
	else
	{
		sprintf(buf, "INSERT INTO craft_phases (first, third, group_mess, phase_seconds, skill, attribute, dice, sides, move_cost, hit_cost, craft_id, phase_num) VALUES ('%s', '%s', '%s', %d, %d, %d, %d, %d, %d, %d, %d, %d)",
			phase->first,
			phase->third,
			phase->group_mess,
			phase->phase_seconds,
			phase->skill,
			phase->attribute,
			phase->dice,
			phase->sides,
			phase->move_cost,
			phase->hit_cost,
			tcraft_id,
			phase_num);
		
		mysql_safe_query (buf);
	}
	
	return;
}

void
save_mysql_craft_element(SUBCRAFT_HEAD_DATA *craft, DEFAULT_ITEM_DATA *obj_item, DEFAULT_MOB_DATA *mob_item, char* elem_type, int elem_numb)
{
	int i, j;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int tcraft_id;
	int icount;
	int iflags;
	int phasenum;
	std::string vnums;
	MYSQL_RES *result;
	MYSQL_ROW row;

		//get the craft_id
	sprintf(buf, "SELECT craft_id FROM crafts WHERE subcraft_name = '%s'", craft->subcraft_name);
	mysql_safe_query (buf);
	result = mysql_store_result (database);
	if (result && mysql_num_rows (result) >= 1)
	{
		row = mysql_fetch_row(result);
		tcraft_id = atoi(row[0]);
	}
	else
	{
		sprintf(buf, "Error saving craft! - Craft_id not found");
		system_log(buf, true);
		return;
	}

	if (!str_cmp(elem_type, "obj"))
		 {
			 sprintf(buf, "%d ", obj_item->items[0]);
			 vnums.assign(buf);
			 for (i = 1; i < MAX_DEFAULT_ITEMS; i++)
			 {
				 if (obj_item->items[i] > 0)
				 {
				 sprintf(buf, "%d ", obj_item->items[i]);
				 vnums.append(buf);
				 }
				 else
				 {
					 break;	 
				 }

			 }
			 icount = obj_item->item_counts;
			 iflags = obj_item->flags;
			 phasenum = obj_item->phase_num;
		 }
		 
	else if (!str_cmp(elem_type, "mob"))
		 {
			 sprintf(buf, "%d ", mob_item->mobs[0]);
			 vnums.assign(buf);
			 for (i = 1; i < MAX_DEFAULT_MOBS; i++)
			 {
				 if (mob_item->mobs[i] > 0)
				 {
				 sprintf(buf, "%d ", mob_item->mobs[i]);
				 vnums.append(buf);
				 }
				 else
				 {
					 break;	 
				 }

			 } 
			 icount = mob_item->mob_counts;
			 iflags = mob_item->flags;
			 phasenum = mob_item->phase_num;
		 }
			  
			  
		//is the element in the table
	sprintf(buf, "SELECT craft_id FROM craft_elements WHERE craft_id = %d AND  element_numb = %d AND element_type = '%s'", tcraft_id, elem_numb, elem_type);
	
	
	mysql_safe_query (buf);
	result = mysql_store_result (database);
	
	if (result && mysql_num_rows (result) >= 1)
	{				// Update an existing craft element record.
		mysql_free_result (result);
		
		sprintf(buf, "UPDATE craft_elements SET flags = '%d', count = '%d', vnum_list = '%s', phase_num = '%d' WHERE craft_id = '%d' AND element_numb = '%d' AND element_type = '%s'",
				iflags,
				icount,
				vnums.c_str(),
				phasenum,
				tcraft_id,
				elem_numb,
				elem_type);
		
		mysql_safe_query (buf);
	}
	else
	{
		sprintf(buf, "INSERT INTO craft_elements (element_type, flags, count, vnum_list, craft_id, element_numb, phase_num) VALUES ('%s', '%d', '%d', '%s', '%d',  '%d', '%d')",
				elem_type,
				iflags,
				icount,
				vnums.c_str(),
				tcraft_id,
				elem_numb,
				phasenum);
		
		mysql_safe_query (buf);
	}
	
	
	return;
}

void 
load_subcraft_mysql (int craft_id)
{
	int ind;
	int items_num;
	int mobs_num;
	int ski_att, i;
	char *argument = '\0';
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char tbuf[MAX_STRING_LENGTH]= { '\0' };
	PHASE_DATA *phase = NULL;
	SUBCRAFT_HEAD_DATA *tcraft = NULL;
	MYSQL_RES *result;
	mysqlpp::Query query = dbo.query();
	
		
		//load header inforamtion

	query << "SELECT craft_name, subcraft_name, command, open_for_skill, race, clans, terrains, seasons, weather,subcraft_flags, delay, key_first, key_end, followers, status FROM crafts WHERE craft_id = '" << craft_id << "'";
	
	mysqlpp::StoreQueryResult res = query.store();
	if ( res.num_rows() > 0 )
	{
		mysqlpp::Row row = res[0];
		
		if ( row["status"] != "approved" )
		{
				// if not approved, don't load
			sprintf(buf, "Craft %d not approved for loading.\n", craft_id);
			system_log(buf, true);
			return;
		}
		
		tcraft = new SUBCRAFT_HEAD_DATA;
		memset (tcraft, 0, sizeof(SUBCRAFT_HEAD_DATA));	
		
		tcraft->craft_name = strdup(row["craft_name"].c_str());
		tcraft->subcraft_name = strdup(row["subcraft_name"].c_str());
		tcraft->command = strdup(row["command"].c_str());
		tcraft->clans = strdup(row["clans"].c_str());
		
			//Terrain
		argument = strdup(row["terrains"].c_str());
		argument = one_argument (argument, tbuf);
		
		while (*tbuf)
		{
			if ((ind = index_lookup (terrain_types, tbuf)) != -1)
			{
				for (i = 0; i <= 24; i++)
				{
					if (!tcraft->terrains[i])
					{
						tcraft->terrains[i] = ind + 1;
						break;
					}
				}
			}
			else
			{
				system_log ("Illegal subcraft terrain:", true);
				system_log (tbuf, true);
			}
			
			argument = one_argument (argument, tbuf);
			
		}

			//Seasons
		argument = strdup(row["seasons"].c_str());
		argument = one_argument (argument, tbuf);
		
		while (*tbuf)
		{
			if ((ind = index_lookup (seasons, tbuf)) != -1)
			{
				for (i = 0; i <= 24; i++)
				{
					if (!tcraft->seasons[i])
					{
						tcraft->seasons[i] = ind + 1;
						break;
					}
				}
			}
			else
			{
				system_log ("Illegal subcraft season:", true);
				system_log (tbuf, true);
			}
			
			argument = one_argument (argument, tbuf);
			
		}
		
			//Weather
		argument = strdup(row["weather"].c_str());
		argument = one_argument (argument, tbuf);
		
		while (*tbuf)
		{
			if ((ind = index_lookup (weather_states, tbuf)) != -1)
			{
				for (i = 0; i <= 24; i++)
				{
					if (!tcraft->weather[i])
					{
						tcraft->weather[i] = ind + 1;
						break;
					}
				}
			}
			else
			{
				system_log ("Illegal subcraft weather:", true);
				system_log (tbuf, true);
			}
			
			argument = one_argument (argument, tbuf);
			
		}
		
			//Opening skills
		argument = strdup(row["open_for_skill"].c_str());
		argument = one_argument (argument, tbuf);
		
		while (*tbuf)
		{
			
			if (atoi (tbuf) <= skill_data_map.size())
			{
				for (i = 0; i <= 24; i++)
				{
					if (!tcraft->opening[i])
					{
						tcraft->opening[i] = atoi (tbuf);
						break;
					}
				}
			}
			argument = one_argument (argument, tbuf);
		}
		
			//Race
		argument = strdup(row["race"].c_str());
		argument = one_argument (argument, tbuf);
		while (*tbuf)
		{
			for (i = 0; i <= 24; i++)
			{
				if (!tcraft->race[i])
				{
					tcraft->race[i] = atoi (tbuf);
					break;
				}
			}
			argument = one_argument (argument, tbuf);
		}
		
		

		
		tcraft->delay = atoi(row["delay"]);
		tcraft->key_first = atoi(row["key_first"]);
		tcraft->key_end = atoi(row["key_end"]);
		tcraft->followers = atoi(row["followers"]);
	
		craft_map[tcraft->subcraft_name] = tcraft;
		
		//load phase information
		load_phases_mysql(craft_id, tcraft);
		
		//load element inforamtion
		load_elements_mysql(craft_id, tcraft);
	}

		return;
}

void
load_phases_mysql(int craft_id, SUBCRAFT_HEAD_DATA *craft)
{
	char *argument = '\0';
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char tbuf[MAX_STRING_LENGTH]= { '\0' };
	PHASE_DATA *tphase = NULL;
	PHASE_DATA *phase = NULL;
	SUBCRAFT_HEAD_DATA *tcraft = NULL;
	MYSQL_RES *result;
	mysqlpp::Query query = dbo.query();
	int phase_num;
	int i;
	
	query << "SELECT craft_id, phase_num, first, third, group_mess, phase_seconds, skill, attribute, dice, sides, move_cost, hit_cost FROM craft_phases WHERE craft_id = '" << craft_id <<"'";
	
	mysqlpp::StoreQueryResult res = query.store();
	if ( res.num_rows() > 0 )
	{
		for (i = 0; i < res.num_rows(); i++)
		{
		mysqlpp::Row row = res[i];
		
		tphase = new PHASE_DATA;
		memset (tphase, 0, sizeof(PHASE_DATA));	
		
		tphase->first = strdup(row["first"].c_str());
		tphase->third = strdup(row["third"].c_str());
		tphase->group_mess = strdup(row["group_mess"].c_str());
		
		tphase->phase_seconds = atoi(row["phase_seconds"]);
		tphase->skill = atoi(row["skill"]);
		tphase->dice = atoi(row["dice"]);
		tphase->sides = atoi(row["sides"]);
		tphase->hit_cost = atoi(row["hit_cost"]);
		tphase->move_cost = atoi(row["move_cost"]);
		tphase->attribute = atoi(row["attribute"]);

		phase_num = atoi (row["phase_num"]);
		craft->phases[phase_num] = tphase;
		}	
	}
	
	return;
}

void
load_elements_mysql(int craft_id, SUBCRAFT_HEAD_DATA *craft)
{
	int i, j;
	char *argument = '\0';
	char buf[MAX_STRING_LENGTH]= { '\0' };

	MYSQL_RES *result;
	mysqlpp::Query query = dbo.query();
	mysqlpp::Row row;
	char * elem_type;
	int elem_numb;
	int phase_num;
	DEFAULT_MOB_DATA *tmob;
	DEFAULT_ITEM_DATA *tobj;
	
	query << "SELECT craft_id, element_numb, element_type, flags, count, vnum_list, phase_num FROM craft_elements WHERE craft_id = '" << craft_id <<"'";
	
	mysqlpp::StoreQueryResult res = query.store();
	if ( res.num_rows() > 0 )
	{
		
		for (j = 0; j < res.num_rows(); j++)
		{
			row = res[j];
			
			elem_type = strdup(row["element_type"].c_str());
			elem_numb = atoi(row["element_numb"]);
			
			if (!strcmp(elem_type, "obj"))
			{
				tobj = new DEFAULT_ITEM_DATA;
				tobj->flags = atoi(row["flags"]);
				tobj->item_counts = atoi(row["count"]);
				tobj->phase_num = atoi(row["phase_num"]);
				
				argument = strdup(row["vnum_list"].c_str());
				argument = one_argument(argument, buf);
				for (i = 0; i < MAX_DEFAULT_ITEMS; i++)
				{
					tobj->items[i] = atoi(buf);
					argument = one_argument(argument, buf);
				}
				
				if (!strcmp(elem_type, "obj"))
				{
					craft->obj_items[elem_numb] = tobj;
				}
			}
			
			else if (!strcmp(elem_type, "mob"))
			{
				tmob = new DEFAULT_MOB_DATA;
				tmob->flags = atoi(row["flags"]);
				tmob->mob_counts = atoi(row["count"]);
				tmob->phase_num = atoi(row["phase_num"]);
				
				argument = strdup(row["vnum_list"].c_str());
				argument = one_argument(argument, buf);
				for (i = 0; i < MAX_DEFAULT_ITEMS; i++)
				{
					tmob->mobs[i] = atoi(buf);
					argument = one_argument(argument, buf);
				}
				
				if (!strcmp(elem_type, "mob"))
				{
					craft->mob_items[elem_numb] = tmob;
				}
			}
			
		}
	}
	return;
			
}

void
load_materials(void)
{
	int i;
	OBJECT_MATERIAL *tmaterial;
	mysqlpp::Query query = dbo.query();
	mysqlpp::Row row;
	int id_num;
	
	query << "SELECT ident, name, hardness, type_material, quality_0, quality_1, quality_2, quality_3, quality_4, quality_5, quality_6 FROM object_materials";
	
	mysqlpp::StoreQueryResult res = query.store();
	if ( res.num_rows() > 0 )
	{
		
		for (i = 0; i < res.num_rows(); i++)
		{
			row = res[i];
			
			tmaterial = new object_material();
			tmaterial->material_name = strdup(row["name"].c_str());
			tmaterial->hardness = atoi(row["hardness"]);
			tmaterial->type_material = atoi(row["type_material"]);
			tmaterial->ident = atoi(row["ident"]);
			tmaterial->qual_strings[0] = strdup(row["quality_0"].c_str());
			tmaterial->qual_strings[1] = strdup(row["quality_1"].c_str());
			tmaterial->qual_strings[2] = strdup(row["quality_2"].c_str());
			tmaterial->qual_strings[3] = strdup(row["quality_3"].c_str());
			tmaterial->qual_strings[4] = strdup(row["quality_4"].c_str());
			tmaterial->qual_strings[5] = strdup(row["quality_5"].c_str());
			tmaterial->qual_strings[6] = strdup(row["quality_6"].c_str());
			id_num = atoi(row["ident"]);

			object_material_map[id_num] = tmaterial;
		}
	}
	return;
	
}

void
load_variant_values(void)
{
	int i;
	VARIANT_VALUE *tvariant;
	mysqlpp::Query query = dbo.query();
	mysqlpp::Row row;
	
	query << "SELECT ident, name, value, description FROM variant_values";
	
	mysqlpp::StoreQueryResult res = query.store();
	if ( res.num_rows() > 0 )
	{
		
		for (i = 0; i < res.num_rows(); i++)
		{
			row = res[i];
			
			tvariant = new variant_value();
			tvariant->name = strdup(row["name"].c_str());
			tvariant->var_value = strdup(row["value"].c_str());
			tvariant->description = strdup(row["description"].c_str());
			tvariant->ident = atoi(row["ident"]);
						
			gl_variant.insert(std::make_pair(tvariant->ident, tvariant));
		}
	}
	return;
	
}


	// Returns skill database id, if any, for specified skill name
	// returns -1 if skill does not exist
	// checks for CAP or non-CAP values
int
lookup_skill_id (std::string name)
{
	std::map<std::string, SKILL_DATA*>::iterator skill_it;
	SKILL_DATA* tskill;
	
	if (name.empty())
		return -1;
	
	skill_it = skill_data_map.find(name);
	if (skill_it != skill_data_map.end())
	{
		tskill = skill_it->second;
		return tskill->skill_id;
	}
	else
	{
		name[0] = toupper(name[0]);
		skill_it = skill_data_map.find(name);
		if (skill_it != skill_data_map.end())
		{
			tskill = skill_it->second;
			return tskill->skill_id;
		}
	}

	return -1;	
}

char *
lookup_skill_name (int skill_num)
{
	std::map<std::string, SKILL_DATA*>::iterator skill_it;
	SKILL_DATA* tskill;
	
	if (skill_num == 0)
		return NULL;
	
	for (skill_it = skill_data_map.begin(); skill_it != skill_data_map.end(); skill_it++)
	{
		tskill = skill_it->second;
		if (tskill->skill_id == skill_num)
			return ((char*)tskill->skill_name.c_str());
	}
	
	return NULL;	
}

float
lookup_max_gain(int skill_num)
{
	std::map<std::string, SKILL_DATA*>::iterator skill_it;
	SKILL_DATA* tskill;

	for (skill_it = skill_data_map.begin(); skill_it != skill_data_map.end(); skill_it++)
	{
		tskill = skill_it->second;
		return (tskill->max_gain);
	}
	
	return 0;
}


void
load_skills_mysql (void)
{
	int i;
	SKILL_DATA* tskill;
	mysqlpp::Row row;
	mysqlpp::Query query = dbo.query();
	
	query << "SELECT id, name, rpp_required, restricted_races, ov, lv, cap, max_gain, func_info, delay, created_by, last_modified, is_used, spoken, written, innate FROM skills";
	
	mysqlpp::StoreQueryResult res = query.store();
	if ( res.num_rows() > 0 )
	{
		for (i = 0; i < res.num_rows(); i++)
		{
			row = res[i];
		
			if ( row["is_used"] == "0" )
			{
					// if not used, don't load
				continue;
			}
			
			tskill = new skill_data;
			
			tskill->skill_id = atoi(row["id"]);
			tskill->rpp_required = atoi(row["rpp_required"]);
			tskill->last_modified = atoi(row["last_modified"]);
			tskill->max_gain = atof(row["max_gain"]);
			tskill->spoken = atoi(row["spoken"]);
			tskill->written = atoi(row["written"]);
			tskill->innate = atoi(row["innate"]);
			tskill->skill_name.assign(row["name"]);
			tskill->restricted_races.assign(row["restricted_races"]);
			tskill->open_value.assign(row["ov"]);
			tskill->level_value.assign(row["lv"]);
			tskill->max_cap.assign(row["cap"]);
			tskill->func_info.assign(row["func_info"]);
			
			
			use_table[tskill->skill_id].delay = atoi(row["delay"]);
			
			add_registry (REG_SKILLS, tskill->skill_id, tskill->skill_name.c_str());
			add_registry (REG_OV, tskill->skill_id, tskill->open_value.c_str());
			add_registry (REG_LV, tskill->skill_id, tskill->level_value.c_str());
			add_registry (REG_CAP, tskill->skill_id, tskill->max_cap.c_str());
			
			skill_data_map[tskill->skill_name] = tskill;
		}
	}
}

	//mostly identical to save_mysql_mob that saves prototype, with additonal items for stayputs
void
save_mysql_stayput_mob (CHAR_DATA * tmob) 
{
	int i;
	int tskill;
	char tmp[MAX_STRING_LENGTH] = { '\0' };
	char tmp_names[MAX_STRING_LENGTH] = { '\0' };
	char tmp_values[MAX_STRING_LENGTH] = { '\0' };
	char tclan_name[MAX_STRING_LENGTH] = { '\0' };
	char tclan_rank[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buflong[2*MAX_STRING_LENGTH];
	char *skill_name;
	std::map<std::string, int>::iterator skill_it;
	std::map<std::string, std::string>::iterator clan_it;
	std::string desc_temp;
	std::string sdesc_temp;
	std::string ldesc_temp;
	std::string clan_temp;
	std::string holding;
	std::string skill_buf;
	std::string shop_buf;
	std::string delivery_buf;
	std::string trades_in_buf;
	std::string material_buf;
	std::string material_temp;
	std::string travel_temp;
	std::string voice_temp;
	std::string temp_str;
	MYSQL_RES *result;
	
	
	if (!tmob || tmob->mob->nVirtual == 0)
	{
		return;
	}
	/***
		//saves carried and worn items
	 sprintf (buf, "save/mobiles/%d", tmob->coldload_id);
	 fp = fopen (buf, "w");
	 
	 save_mobile (tmob, fp, "STAYPUT", 0);
	 
	 fclose (fp);
	 ****/ 
	 
	
		//escape strings as needed
	if (tmob->description)
	{
		mysql_real_escape_string (database, buflong, tmob->description, strlen(tmob->description));
		desc_temp = duplicateString(buflong);
	}
	
	if(tmob->short_descr)
	{
		mysql_real_escape_string (database, buflong, tmob->short_descr, strlen(tmob->short_descr));
		sdesc_temp = duplicateString(buflong);
	}
	
	if(tmob->long_descr)
	{
		mysql_real_escape_string (database, buflong, tmob->long_descr, strlen(tmob->long_descr));
		ldesc_temp = duplicateString(buflong);
	}
	
	if(tmob->travel_str)
	{
		mysql_real_escape_string (database, buflong, tmob->travel_str, strlen(tmob->travel_str));
		travel_temp = duplicateString(buflong);
	}
	
	if(tmob->voice_str)
	{
		mysql_real_escape_string (database, buflong, tmob->voice_str, strlen(tmob->voice_str));
		voice_temp = duplicateString(buflong);
	}
	
	skill_buf.clear();
	shop_buf.clear();
	delivery_buf.clear();
	trades_in_buf.clear();
	clan_temp.clear();

		//Accumulate some values to store in a buffers for later use
	
	int counter = tmob->skill_map.size();
	
	for (skill_it = tmob->skill_map.begin(); skill_it != tmob->skill_map.end(); skill_it++)
	{
		if (counter <= 0)
			break;
		
			tskill = skill_it->second;
			if (tskill > 0)
			{
				skill_name = strdup(skill_it->first.c_str());
				
				skill_buf.append(skill_name);
				skill_buf.append(" ");
				sprintf(tmp, "%d",tskill); 
				skill_buf.append(tmp);
				skill_buf.append(" ");
			}
		counter --;
		
		
	}
	

		//CLANS
	counter = tmob->char_clan_map.size();
	for (clan_it = tmob->char_clan_map.begin(); clan_it != tmob->char_clan_map.end(); clan_it++)
		{
			if (counter <= 0)
				break;
			
				sprintf(tclan_name, "%s", clan_it->first.c_str());
				sprintf(tclan_rank, "%s", clan_it->second.c_str());
					
				clan_temp.append(tclan_name);
				clan_temp.append(" \"");
				clan_temp.append(tclan_rank);
				clan_temp.append("\"");
				clan_temp.append(" ");

			counter --;
		}
	
	
		
	if(tmob->mob->shop)
	{
			//Deliveries
		for (i = 0; i <= MAX_DELIVERIES; i++)
		{
			if (!tmob->mob->shop->delivery[i])
				break;
			sprintf(tmp, "%d",tmob->mob->shop->delivery[i]);
			delivery_buf.append(tmp);
			delivery_buf.append(" ");
		}
		
		if (!delivery_buf.empty())
		{
			delivery_buf.append("\n");
		}
		
			//trades in items
		for (i = 0; i <= MAX_TRADES_IN; i++)
		{
			if (!tmob->mob->shop->trades_in[i])
				break;
			sprintf(tmp, "%d",tmob->mob->shop->trades_in[i]);
			trades_in_buf.append(tmp);
			trades_in_buf.append(" ");
		}
		
		if (!trades_in_buf.empty())
		{
			trades_in_buf.append("\n");
		}
		
		
			//materials
		std::set<std::string>::iterator materials_it;
		for ( materials_it = tmob->mob->shop->materials.begin(); materials_it != tmob->mob->shop->materials.end(); materials_it++ )
		{
			std::stringstream tmp_str;
			tmp_str << *materials_it << "\n";
			
			material_buf.append(tmp_str.str().c_str());
			
		}
		if (!material_buf.empty())
		{
			material_buf.append("\n");
		}
	}
	if (!material_buf.empty())
	{
		mysql_real_escape_string (database, buflong, material_buf.c_str(), material_buf.length());
		material_temp = duplicateString(buflong);
	}
	
	sprintf(tmp,"SELECT coldload_id FROM stayput_mobs WHERE coldload_id = '%d'",
				 tmob->coldload_id); 
	mysql_safe_query (tmp);
	result = mysql_store_result (database);
	
	if (result && mysql_num_rows (result) >= 1) // Update an existing stayput record.
	{				
		mysql_free_result (result);
		
			//general sting data 
		sprintf(tmp, "UPDATE stayput_mobs SET name = '%s', short_desc = '%s', clans = '%s', skills = '%s', long_desc = '%s', description = '%s',  voicestr = '%s', travelstr = '%s' ",
				tmob->keywords,
				sdesc_temp.c_str(),
				clan_temp.c_str(),
				skill_buf.c_str(),
				ldesc_temp.c_str(),
				desc_temp.c_str(),
				voice_temp.c_str(),
				travel_temp.c_str());
		
			//Note: tmob->mob->action and tmob->mob->profession are bitflag
			//so the format string is %ld
		sprintf(tmp + strlen(tmp), ", zone = '%d', str = '%d', intel = '%d', wil = '%d', aur = '%d', dex = '%d', con = '%d', agi = '%d', luk = '%d', speaks = '%d', act = '%ld', profession = '%ld', race = '%d', body_type = '%d', height = '%d', frame = '%d', armor = '%d', hit = '%d', max_hit = '%d', move = '%d', intoxication = '%d', hunger = '%d', thirst = '%d', damnodice = '%d', damroll = '%d', damsizedice = '%d', position = '%d', default_pos = '%d', sex = '%d', speed = '%d', spawnpoint = '%d', access_flags = '%d', noaccess_flags = '%d', birth = '%d', age='%d'",
				tmob->mob->zone,
				tmob->str,
				tmob->intel,
				tmob->wil,
				tmob->aur,
				tmob->dex,
				tmob->con,
				tmob->agi,
				tmob->luk,
				lookup_skill_id(tmob->speaks),
				tmob->mob->action,
				tmob->mob->profession,
				tmob->race,
				tmob->body_type,
				tmob->height,
				tmob->frame,
				tmob->armor,
				tmob->hit,
				tmob->max_hit,
				tmob->move,
				tmob->intoxication,
				tmob->hunger,
				tmob->thirst,
				tmob->mob->damnodice,
				tmob->mob->damroll,
				tmob->mob->damsizedice,
				tmob->position,
				tmob->default_pos,
				tmob->sex,
				tmob->speed,
				tmob->in_room,
				tmob->mob->access_flags,
				tmob->mob->noaccess_flags,
				(int)tmob->time_str.birth,
				tmob->age);
		
			//shopkeeper stuff
		if (IS_SET (tmob->flags, FLAG_KEEPER))
		{
			sprintf(tmp +strlen(tmp), ", keeper = '%d', shop_vnum = '%d', store_vnum = '%d', currency_type = '%d', markup = '%f', discount = '%f', shop_materials = '%s', buy_flags = '%d', nobuy_flags = '%d', delivery = '%s', trades_in ='%s'",
					1,
					tmob->mob->shop->shop_vnum,
					tmob->mob->shop->store_vnum,
					tmob->mob->currency_type,
					tmob->mob->shop->markup,
					tmob->mob->shop->discount,
					material_temp.c_str(),
					tmob->mob->shop->buy_flags,
					tmob->mob->shop->nobuy_flags,
					delivery_buf.c_str(),
					trades_in_buf.c_str(),
					tmob->mob->nVirtual);
		}
		
		if (tmob->mob->merch_seven > 0)
		{
			
			sprintf(tmp +strlen(tmp), ", merch_seven = '%d', markup1 = '%f', discount1 = '%f', econ_flags1 = '%d', markup2 = '%f', discount2 = '%f', econ_flags2 = '%d', markup3 = '%f', discount3 = '%f', econ_flags3 = '%d', markup4 = '%f', discount4 = '%f', econ_flags4 = '%d', markup5 = '%f', discount5 = '%f', econ_flags5 = '%d', markup6 = '%f', discount6 = '%f', econ_flags6 = '%d', markup7 = '%f', discount7 = '%f', econ_flags7 = '%d'",
					tmob->mob->merch_seven,
					tmob->mob->shop->econ_markup1,
					tmob->mob->shop->econ_discount1,
					tmob->mob->shop->econ_flags1,
					tmob->mob->shop->econ_markup2,
					tmob->mob->shop->econ_discount2,
					tmob->mob->shop->econ_flags2,
					tmob->mob->shop->econ_markup3,
					tmob->mob->shop->econ_discount3,
					tmob->mob->shop->econ_flags3,
					tmob->mob->shop->econ_markup4,
					tmob->mob->shop->econ_discount4,
					tmob->mob->shop->econ_flags4,
					tmob->mob->shop->econ_markup5,
					tmob->mob->shop->econ_discount5,
					tmob->mob->shop->econ_flags5,
					tmob->mob->shop->econ_markup6,
					tmob->mob->shop->econ_discount6,
					tmob->mob->shop->econ_flags6,
					tmob->mob->shop->econ_markup7,
					tmob->mob->shop->econ_discount7,
					tmob->mob->shop->econ_flags7);
		}
		else
		{
			sprintf(tmp +strlen(tmp), ", merch_seven = '%d'", tmob->mob->merch_seven);
		}
		
			//morphing data
		if (tmob->mob->clock)
		{
			sprintf(tmp +strlen(tmp), ", clock = '%d', morphto = '%d', morph_type = '%d', morph_time = '%d'",
					tmob->mob->clock,
					tmob->mob->morphto,
					tmob->mob->morph_type,
					tmob->mob->morph_time);
		}
		
			//final query
		sprintf (tmp + strlen(tmp), " WHERE coldload_id = '%d'", tmob->coldload_id);
		
		mysql_safe_query (tmp);
		result = mysql_store_result (database);
		if (!result || !mysql_num_rows (result))
		{
			if (result)
				mysql_free_result (result);
			
		}
		
	}
	else // New mobile record.
	{		
		//Set up of query
		sprintf(tmp_names, "INSERT INTO stayput_mobs (");
		
			//general data
		sprintf(tmp_names + strlen(tmp_names), "nVirtual, coldload_id, name, short_desc, clans, skills, long_desc, description, voicestr, travelstr ");
		sprintf(tmp_values, " '%d', '%d', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s'", 
				tmob->mob->nVirtual,
				tmob->coldload_id,
				tmob->keywords, 
				sdesc_temp.c_str(),
				clan_temp.c_str(), 
				skill_buf.c_str(),
				ldesc_temp.c_str(), 
				desc_temp.c_str(),
				voice_temp.c_str(),
				travel_temp.c_str());
		
		
			//general numeric data	
		sprintf(tmp_names + strlen(tmp_names), ", zone, str, intel, wil, aur, dex, con, agi, luk, speaks, act, profession, race, body_type, height, frame, armor, hit,  max_hit, move, intoxication, hunger, thirst, damnodice, damroll, damsizedice,  position, default_pos, sex, speed, spawnpoint, access_flags, noaccess_flags, birth, age");
		
			//Note: tmob->mob->act and tmob->mob->profession are bitflags
			//so the format string is %ld
		sprintf(tmp_values + strlen(tmp_values), ", '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%ld', '%ld', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d'",
				tmob->mob->zone,
				tmob->str,
				tmob->intel,
				tmob->wil,
				tmob->aur,
				tmob->dex,
				tmob->con,
				tmob->agi,
				tmob->luk,
				lookup_skill_id(tmob->speaks),
				tmob->mob->action,
				tmob->mob->profession,
				tmob->race,
				tmob->body_type,
				tmob->height,
				tmob->frame,
				tmob->armor,
				tmob->hit,
				tmob->max_hit,
				tmob->move,
				tmob->intoxication,
				tmob->hunger,
				tmob->thirst,
				tmob->mob->damnodice,
				tmob->mob->damroll,
				tmob->mob->damsizedice,
				tmob->position,
				tmob->default_pos,
				tmob->sex,
				tmob->speed,
				tmob->in_room,
				tmob->mob->access_flags,
				tmob->mob->noaccess_flags,
				(int)tmob->time_str.birth,
				tmob->age);
		
			//shopkeeper stuff
		if (IS_SET (tmob->flags, FLAG_KEEPER))
		{
			sprintf(tmp_names + strlen(tmp_names), ", keeper, shop_vnum, store_vnum, currency_type, markup, discount, shop_materials, buy_flags, nobuy_flags, delivery, trades_in");
			sprintf(tmp_values + strlen(tmp_values), ", '%d', '%d', '%d', '%d', '%f', '%f', '%s', '%d', '%d', '%s', '%s'",
					1,
					tmob->mob->shop->shop_vnum,
					tmob->mob->shop->store_vnum,
					tmob->mob->currency_type,
					tmob->mob->shop->markup,
					tmob->mob->shop->discount,
					material_temp.c_str(),
					tmob->mob->shop->buy_flags,
					tmob->mob->shop->nobuy_flags,
					delivery_buf.c_str(),
					trades_in_buf.c_str());
			
		}
		
		if (tmob->mob->merch_seven > 0)
		{
			sprintf(tmp_names + strlen(tmp_names), ", markup1, discount1, econ_flags1, markup2, discount2, econ_flags2, markup3, discount3, econ_flags3, markup4, discount4, econ_flags4, markup5, discount5, econ_flags5, markup6, discount6, econ_flags6, markup7, discount7, econ_flags7");
			sprintf(tmp_values + strlen(tmp_values), ", '%f', '%f', '%d' '%f', '%f', '%d', '%f', '%f', '%d', '%f', '%f', '%d', '%f', '%f', '%d', '%f', '%f', '%d', '%f', '%f', '%d'",
					tmob->mob->shop->econ_markup1,
					tmob->mob->shop->econ_discount1,
					tmob->mob->shop->econ_flags1,
					tmob->mob->shop->econ_markup2,
					tmob->mob->shop->econ_discount2,
					tmob->mob->shop->econ_flags2,
					tmob->mob->shop->econ_markup3,
					tmob->mob->shop->econ_discount3,
					tmob->mob->shop->econ_flags3,
					tmob->mob->shop->econ_markup4,
					tmob->mob->shop->econ_discount4,
					tmob->mob->shop->econ_flags4,
					tmob->mob->shop->econ_markup5,
					tmob->mob->shop->econ_discount5,
					tmob->mob->shop->econ_flags5,
					tmob->mob->shop->econ_markup6,
					tmob->mob->shop->econ_discount6,
					tmob->mob->shop->econ_flags6,
					tmob->mob->shop->econ_markup7,
					tmob->mob->shop->econ_discount7,
					tmob->mob->shop->econ_flags7);
		}
		
			//morphing data
		if (tmob->mob->clock)
		{
			sprintf(tmp_names + strlen(tmp_names), ", clock, morphto, morph_type, morph_time");
			sprintf(tmp_values + strlen(tmp_values), ", '$d', '%d', '%d' '%d'",
					tmob->mob->clock, tmob->mob->morphto, tmob->mob->morph_type, tmob->mob->morph_time);
		}	
		
			//final query
		sprintf(tmp, "%s) VALUES (%s)",
				tmp_names, 
				tmp_values);
		
		mysql_safe_query (tmp);
		result = mysql_store_result (database);
		if (!result || !mysql_num_rows (result))
		{
			if (result)
				mysql_free_result (result);
			
		}
	}
	return;
	
	
}
/******* CLAN STUFF ****/



	//clears the current clan_data_map and loads it with info from the database.
void
load_clans_mysql (void)
{
	int i;
	CLAN_DATA* tclan;
	mysqlpp::Row row;
	mysqlpp::Query query = dbo.query();
	
	clan_data_map.empty();
	query << "SELECT id, name, long_name, parent, zone, member_obj, leader_obj, pay_master, friend, foe, is_used FROM clans";
	
	mysqlpp::StoreQueryResult res = query.store();
	if ( res.num_rows() > 0 )
	{
		for (i = 0; i < res.num_rows(); i++)
		{
			row = res[i];
			
			if ( row["is_used"] == "0" )
			{
					// if not used, don't load
				continue;
			}
			
			tclan = new clan_data;
			
			tclan->id = atoi(row["id"]);
			tclan->name = strdup(row["name"]);
			tclan->literal = strdup(row["long_name"]);
			tclan->parent = atoi(row["parent"]);
			tclan->zone = atoi(row["zone"]);
			tclan->member_obj = atoi(row["member_obj"]);
			tclan->leader_obj = atoi(row["leader_obj"]);
			tclan->pay_master = atoi(row["pay_master"]);
						
			load_clan_ranks_mysql(tclan);	
			
			clan_data_map[tclan->name] = tclan;
		}
	}
}

	//fills the array with rank names and returns it to load_clan_mysql
void
load_clan_ranks_mysql (CLAN_DATA* tclan)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int i;
	char *clan_rank_name;
	int clan_rank_num;
	mysqlpp::Row row;
	mysqlpp::Query query = dbo.query();
	
	for (int i = 0; i < MAX_CLAN_RANKS; i++)
		tclan->rank[i] = "none";	
	
	sprintf(buf, "SELECT rank_name, rank_num, is_used FROM clan_ranks WHERE clan_num = %d", tclan->id);
	
	query << buf;
	
	mysqlpp::StoreQueryResult res = query.store();
	if ( res.num_rows() > 0 )
	{
		for (i = 0; i < res.num_rows(); i++)
		{
			row = res[i];
			
			if ( row["is_used"] == "0" )
			{
					// if not used, don't load
				continue;
			}
			
					
			clan_rank_name = strdup(row["rank_name"]);
			clan_rank_num = atoi(row["rank_num"]);
			
			tclan->rank[clan_rank_num] = strdup(clan_rank_name);
		}
	}
}


	//loads stayput mobs from database
	//similar to load_mobile_mysql with additonal items for stayputs
CHAR_DATA *
load_stayput_mobile_mysql (int mob_num, int coldload)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char *tbuf, *tbuf1, *tbuf2, *tbuf3;
	int damage = 0;
	int bleeding = 0;
	int poison = 0;
	CHAR_DATA * tmob;
	int ind;
	int lev = 0;
	int keep_val;
	int merch_val;
	int tholder;
	std::map<std::string, int> ::iterator skill_it;
	std::map<std::string, SKILL_DATA*>::iterator found_map_skill;
	std::string skillstr;
	mysqlpp::Row row;
	mysqlpp::Query query = dbo.query();
	
	query << "SELECT * FROM stayput_mobs WHERE nVirtual = '" << mob_num <<"' AND coldload_id = '" << coldload << "'";
	
	mysqlpp::StoreQueryResult res = query.store();
	if ( res.num_rows() > 0 )
	{
		
		row = res[0];
		
		tmob = new_char (0);
		tmob->mob->nVirtual = mob_num;
		tmob->coldload_id = atoi(row["coldload_id"]);
		tmob->keywords = strdup(row["name"].c_str()); 
		one_argument (tmob->keywords, buf);
		tmob->name = duplicateString (CAP (buf));
		
		tmob->short_descr = strdup(row["short_desc"].c_str());
		tmob->voice_str = strdup(row["voicestr"].c_str());
		tmob->travel_str = duplicateString(row["travelstr"].c_str());
		
			//CLANS
		if ( row["clans"].length() > 2 )
		{
			tbuf = strdup(row["clans"].c_str());
			while ( 1 )
			{
				tbuf = one_argument (tbuf, buf);
				tbuf = one_argument (tbuf, buf2);
				if (!*buf2)
					break;
				char_clan_add(tmob, buf, buf2);
			}
		}
		
			//SKILLS
		
		int skill_num;
		if ( row["skills"].length() > 2 )
		{
			tbuf = strdup(row["skills"].c_str());
			
			if (*tbuf)
			{	
				
				tbuf1 = strtok (tbuf," \n");
				if (tbuf1 != NULL)
				{
					tbuf2 = strdup(tbuf1);
				}
				
				tbuf1 = strtok (NULL, " \n");
				if (tbuf1 != NULL)
				{
					tbuf3 = strdup(tbuf1);
				}
				while ((tbuf2 != NULL) && (tbuf3 != NULL))
				{
					
					skillstr = strdup(tbuf2);
					lev = atoi(tbuf3);
					
					
					
					skill_num = lookup_skill_id(skillstr);
					if ((skill_num > 0) && (*tbuf3))
					{
						tmob->skill_map[lookup_skill_name(skill_num)] = lev;
					}
					
					tbuf1 = strtok (NULL," \n");
					if (tbuf1 != NULL)
					{
						tbuf2 = strdup(tbuf1);
					}
					else 
						tbuf2 = NULL;
					
					tbuf1 = strtok (NULL, " \n");
					if (tbuf1 != NULL)
					{
						tbuf3 = strdup(tbuf1);
					}
					else
						tbuf3 = NULL;
					
					
				}
				
			}
		}
		
		
		tmob->long_descr = strdup(row["long_desc"].c_str());
		tmob->description = strdup(row["description"].c_str());
		tmob->mob->zone = atoi(row["zone"]);
		tmob->str = atoi(row["str"]);
		tmob->intel = atoi(row["intel"]);
		tmob->wil = atoi(row["wil"]);
		tmob->aur = atoi(row["aur"]);
		tmob->dex = atoi(row["dex"]);
		tmob->con = atoi(row["con"]);
		tmob->agi = atoi(row["agi"]);
		tmob->luk = atoi(row["luk"]);
		
			//speaks is a char* in game, but saved as an INT in SQL
		tmob->speaks = lookup_skill_name(atoi(row["speaks"]));
		
		tmob->mob->action = atoi(row["act"]);
		tmob->mob->profession = atoi(row["profession"]);
		
		if (tmob->mob->action  || tmob->mob->profession)
			tmob->mob->action |= ACT_ISNPC;
		
		
		tmob->race = atoi(row["race"]);
		if ((tmob->race >= 0 && tmob->race <= 29 && tmob->race != 28)
			|| tmob->race == 94)
		{				/* Humanoid NPCs. */
			tmob->max_hit = 50 + tmob->con * CONSTITUTION_MULTIPLIER + (MIN(tmob->aur, 25) * 4); // Arbitrary power HP boost - Case
			tmob->hit = tmob->max_hit;
		}
		
		tmob->move = atoi(row["move"]);
		tmob->body_type = atoi(row["body_type"]);
		tmob->height = atoi(row["height"]);
		tmob->frame = atoi(row["frame"]);
		tmob->armor = atoi(row["armor"]);
		tmob->max_hit = atoi(row["max_hit"]);
		tmob->mob->damnodice = atoi(row["damnodice"]);
		tmob->mob->damroll = atoi(row["damroll"]);
		tmob->mob->damsizedice = atoi(row["damsizedice"]);
		tmob->position = atoi(row["position"]);
		tmob->default_pos = atoi(row["default_pos"]);
		tmob->sex = atoi(row["sex"]);
		tmob->speed = atoi(row["speed"]);
		tmob->mob->spawnpoint = atoi(row["spawnpoint"]);
		tmob->mob->access_flags = atoi(row["access_flags"]);
		tmob->mob->noaccess_flags = atoi(row["noaccess_flags"]);
		tmob->mob->clock = atoi(row["clock"]);
		tmob->mob->morphto = atoi(row["morphto"]);
		tmob->mob->morph_type = atoi(row["morph_type"]);
		tmob->mob->morph_time = atoi(row["morph_time"]);
		tmob->age = atoi (row["age"]);
		tmob->time_str.birth = atoi (row["birth"]);
		
			//SHOPS
		keep_val = atoi(row["keeper"]); 
		if (keep_val)
		{
			tmob->flags |= FLAG_KEEPER;
			
			tmob->mob->shop = new SHOP_DATA;
			tmob->mob->shop->shop_vnum = atoi(row["shop_vnum"]); 
			tmob->mob->shop->store_vnum = atoi(row["store_vnum"]); 
			tmob->mob->currency_type = atoi(row["currency_type"]); 
			tmob->mob->shop->markup = atof(row["markup"]); 
			tmob->mob->shop->discount = atof(row["discount"]); 
			tmob->mob->shop->buy_flags = atoi(row["buy_flags"]); 
			tmob->mob->shop->nobuy_flags = atoi(row["nobuy_flags"]); 
			
			if ( row["delivery"] != mysqlpp::null )
			{
				tbuf = strdup(row["delivery"].c_str());
				if (strcmp(tbuf, ""))
				{
					for (ind = 0; ind <= MAX_DELIVERIES; ind++)
					{
						tbuf = one_argument (tbuf, buf);
						tmob->mob->shop->delivery[ind] = atoi(buf);
					}	
				}
			}
			
			if ( row["trades_in"] != mysqlpp::null )
			{
				tbuf = strdup(row["trades_in"].c_str());		
				if (strcmp(tbuf, ""))
				{
					for (ind = 0; ind <= MAX_TRADES_IN; ind++)
					{
						tbuf = one_argument (tbuf, buf);
						tmob->mob->shop->trades_in[ind] = atoi(buf);
					}	
				}
			}
			/**
			 if ( row["shop_materials"] != mysqlpp::null )
			 {
			 tbuf = strdup(row["shop_materials"].c_str());
			 if (*tbuf)
			 {	
			 argument = strtok (tbuf,"\n");
			 if (argument != NULL)
			 {
			 arg1 = strdup(argument);
			 }
			 while (arg1 != NULL)
			 {
			 tmob->mob->shop->materials.push_front(arg1);
			 argument = strtok (NULL,"\n");
			 if (argument != NULL)
			 {
			 arg1 = strdup(argument);
			 }
			 else 
			 arg1 = NULL;
			 
			 }
			 }
			 
			 }
			 to be restroed when materials are fully introdeuced**/
			merch_val = atoi(row["merch_seven"]);
			
			if (merch_val > 0)
			{
				tmob->mob->shop->econ_markup1 = atof(row["markup1"]);
				tmob->mob->shop->econ_discount1 = atof(row["discount1"]);
				tmob->mob->shop->econ_flags1 = atoi(row["econ_flags1"]);
				tmob->mob->shop->econ_markup2 = atof(row["markup2"]);
				tmob->mob->shop->econ_discount2 = atof(row["discount2"]);
				tmob->mob->shop->econ_flags2 = atoi(row["econ_flags2"]);
				tmob->mob->shop->econ_markup3 = atof(row["markup3"]);
				tmob->mob->shop->econ_discount3 = atof(row["discount3"]);
				tmob->mob->shop->econ_flags3 = atoi(row["econ_flags3"]);
				tmob->mob->shop->econ_markup4 = atof(row["markup4"]);
				tmob->mob->shop->econ_discount4 = atof(row["discount4"]);
				tmob->mob->shop->econ_flags4 = atoi(row["econ_flags4"]);
				tmob->mob->shop->econ_markup5 = atof(row["markup5"]);
				tmob->mob->shop->econ_discount5 = atof(row["discount5"]);
				tmob->mob->shop->econ_flags5 = atoi(row["econ_flags5"]);
				tmob->mob->shop->econ_markup6 = atof(row["markup6"]);
				tmob->mob->shop->econ_discount6 = atof(row["discount6"]);
				tmob->mob->shop->econ_flags6 = atoi(row["econ_flags6"]);
				tmob->mob->shop->econ_markup7 = atof(row["markup7"]);
				tmob->mob->shop->econ_discount7 = atof(row["discount7"]);
				tmob->mob->shop->econ_flags7 = atoi(row["econ_flags7"]);
			}
		}
		
			//calcualted Values
		
		
		tmob->max_move = 50;
		tmob->time_str.played = 0;
		tmob->time_str.logon = time (0);
		
		tmob->intoxication = 0;
		tmob->hunger = -1;
		tmob->thirst = -1;
		
		tmob->tmp_str = tmob->str;
		tmob->tmp_dex = tmob->dex;
		tmob->tmp_intel = tmob->intel;
		tmob->tmp_aur = tmob->aur;
		tmob->tmp_wil = tmob->wil;
		tmob->tmp_con = tmob->con;
		tmob->tmp_agi = tmob->agi;
		tmob->tmp_luk = tmob->luk;
		
		tmob->equip = NULL;
		
		tmob->desc = 0;
		
			//speaks is a char* in game, but saved as an INT in SQL
		if (tmob->speaks == 0)
		{
			tmob->skill_map["Westron"] = 100;
			tmob->speaks = strdup("Westron");
		}
		
		if (tmob->skill_map[tmob->speaks] > 0)
			tmob->skill_map[tmob->speaks] = 100;
		
				
		if (tmob->race < 0)
			tmob->race = 0;
		
		
		tholder = atoi (lookup_race_variable (tmob->race, RACE_BODY_PROTO));
		if (tholder)
			tmob->body_proto =tholder;
		
		tholder = atoi (lookup_race_variable (tmob->race, RACE_MIN_HEIGHT));
		if (tholder)
			tmob->mob->min_height =tholder;
		
		tholder = atoi (lookup_race_variable (tmob->race, RACE_MAX_HEIGHT));
		if (tholder)
			tmob->mob->max_height = tholder;
		
		tmob->apply_race_affects();
		
		
		return (tmob);
		
	}
	else 
		return NULL;
}

	//adds skils, crafts, paydays, etc for characters with roles

void
outfit_new_char (CHAR_DATA *ch, ROLE_DATA *role)
{
	mysqlpp::Query query = dbo.query();
    mysqlpp::Row row;
	int i;
	OBJ_DATA *obj = NULL;
	OBJ_DATA *sign_obj = NULL;
	CLAN_DATA *clan;
	char *tclan_name;
	char *tclan_rank;
	char *tcraft;
	bool obj_load = true;
	bool craft_load = true;
	bool skill_load = true;
	SUBCRAFT_HEAD_DATA *craft;
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	AFFECTED_TYPE *af;
	char output[MAX_STRING_LENGTH]= { '\0' };
	char* buf;
	char craft_name[AVG_STRING_LENGTH];
	char* skill_name;
	int	skill_level = 0;
	int	pay_date = 0;
	int	job = 0;
	int	ind = 0;
	std::map<std::string, int>::iterator skill_it;
	
		//Load Items	
	query << "SELECT * FROM special_roles_outfit WHERE role_id = " << role->id;
	
	mysqlpp::StoreQueryResult res = query.store();
	if ( res.num_rows() > 0 )
	{
		for (i = 0; i < res.num_rows(); i++)
		{
			row = res[i];
			
				// Is there a required race the PC must be before we load this object, craft or skill.
			if (strlen(row["req_race"]) > 0 
				&& (ind = lookup_race_id (row["req_race"]) != -1))
			{
				if (ch->race != ind )
				{
					obj_load = false;
					craft_load = false;
					skill_load = false;
				}
			}	
			
			
				// Is there a required skill the PC must have before we load this object/craft?
			if (strlen(row["req_skill"]) > 0)
			{
				skill_name = lookup_skill_name(atoi(row["skill_value"]));
				skill_it = ch->skill_map.find(skill_name);
				
				if (skill_it == ch->skill_map.end())
				{
					obj_load = false;
					craft_load = false;	
					skill_load = false;
				}
				
			}
			
			
				//OBJECTS
				
			if (obj_load)
			{ 
				
				ind = atoi(row["obj_vnum"]);
				
				obj = load_object(ind);
				if (obj)
				{
					obj->count = atoi(row["obj_qty"]);
				obj_to_room (obj, ch->room->nVirtual);	
				}
				
			}
			
				//CRAFTS
			/************
			tcraft = strdup(row["craft"]);
			if (*tcraft)
			{
				for (tcraft_iterator = craft_map.begin(); tcraft_iterator != craft_map.end(); tcraft_iterator++)
				{
					craft = tcraft_iterator->second;
					if (!str_cmp (craft->subcraft_name, tcraft))
						break;
				}
			}
			
			if (craft_load && craft && !has_craft(ch, craft))
			{
					//find an empty affect location
				for (index = CRAFT_FIRST; index <= CRAFT_LAST; index++)
					if (!get_affect (ch, index))
						break;
				
				magic_add_affect (ch, index, -1, 0, 0, 0, 0);
				af = get_affect (ch, index);
				af->a.craft = new affect_craft_type;
				af->a.craft->subcraft = craft;
				af->a.craft->phase_num = 0;
				af->a.craft->target_ch = NULL;
				af->a.craft->target_obj = NULL;
				af->a.craft->skill_check = 0;
				af->a.craft->timer = 0;
				ch->send_to_char ("Craft added.\n");
			}
			/****************/
			
				//SKILLS
			/*****************************************
			 * skill_level < 0		skill set at value.
			 *							If they do not have the skill, no bonus 
			 * skill_level == 999	skill given at opening skill value
			 *							If they already have it, nothing is done
			 * skill_level > 0		bonus points to skill
			 *							If they don't have skill, no bonus 
			 * skill == 999 is needed to allow skills levels to be sorted, 
			 *				with opening value given first
			 *				then any bonuses awarded beyond that
			 * Multiple values are possible for a character
			 *  ride 999
			 *	ride +10
			 *	ride - 40
			 *	Will give the player the ride skill at opening value, 
			 *	give him 10 more points in it
			 *	and if he still has less than 40, he is boosted to 40
			 ******************************************/	
			if (skill_load
				&& (strlen(row["skill_string"]) > 0))
			{
				
								
				skill_name = strdup(row["skill_string"]);
				skill_level = atoi(row["skill_value"]);
				
				if (lookup_skill_id(skill_name) != -1)
				{
					if (skill_level < 0)
					{
						skill_level = (-1 * skill_level);
						if (ch->skill_map[skill_name] > 0) //they have the skill
						{
							if (ch->skill_map[skill_name] < skill_level) //they need the boost to the set level
								ch->skill_map[skill_name] = skill_level;				
						}
					}
					else if (skill_level == 999)
					{
						if (ch->skill_map[skill_name] == 0) //they don't have the skill
						{
							open_skill (ch, ind);  //give them opening values only
						}
					}
					else if (skill_level > 0)
					{
						if (ch->skill_map[skill_name] > 0) //they have the skill
						{
							ch->skill_map[skill_name] += skill_level;  //they get a boost							
						}
					}
					
				}
			}	
			
				//CLANS
			tclan_name = strdup(row["clan_string"]); 
			if(*tclan_name)
			{
				if (!(clan = get_clandef(tclan_name)))
					continue;
				
				tclan_rank = strdup(row["clan_rank"]);
				if(!*tclan_rank)
					sprintf (tclan_rank, "Member");
				
				char_clan_add (ch, tclan_name, tclan_rank);
			}
			
				//PAYDAYS
			if (atoi(row["payday_num"]) > 0)
			{
				job = atoi(row["payday_num"]) + JOB_1 - 1;
				
				if ((af = get_affect (ch, job)))
					affect_remove (ch, af);
				
				pay_date = time_info.accum_days;
				
				job_add_affect (ch,
								job,
								atoi(row["payday_days"]),
								pay_date,
								0,
								atoi(row["payday_obj_qty"]),
								atoi(row["payday_obj_vnum"]),
								atoi(row["payday_employer"]));
			}
			
				//BRIEFING SIGN
			buf = strdup(row["role_desc"].c_str());
			sign_obj = load_object(OOC_BRIEFING_SIGN);
			
			if (*buf && sign_obj)
			{
				sprintf (output,	"#6The strange, glowing text was perhaps left by a helpful Istar, seeking\n"
						 "to help educate you in the intricacies of your new lot in life:#0\n\n%s", buf);
				sign_obj->full_description = duplicateString (output);
				obj_to_room (sign_obj, ch->room->nVirtual);
			}
		}
	}
	
	ch->pc->role = 0;
}


void
init_body_loc(CHAR_DATA *ch)
{
	mysqlpp::Query query = dbo.query();
    mysqlpp::Row row;
	int i;
	BODY_LOC_INFO* tloc;
	std::vector<BODY_LOC_INFO*>::iterator body_it;
	
		//fill locations	
	query << "SELECT * FROM body_locations WHERE prototype = " << ch->body_proto;
	
	mysqlpp::StoreQueryResult res = query.store();
	if ( res.num_rows() > 0 )
	{
		for (i = 0; i < res.num_rows(); i++)
		{
			row = res[i];
			
			tloc = new BODY_LOC_INFO();
			tloc->priority_order = atoi(row["priority_id"]);
			tloc->long_name = strdup(row["long_name"]);
			tloc->short_name = strdup(row["short_name"]);
			body_it = ch->body_coverage.begin();
			body_it = ch->body_coverage.insert (body_it, tloc);
		};
	}
	
}
void
skill_record (CHAR_DATA* ch, char* skill_name, int timer_length)
{
	char query_buf[MAX_STRING_LENGTH]= { '\0' };
	int flag = 0;
	
	if (IS_NPC(ch))
		flag = 1;
	
	if (ch && ch->name)
	{
		sprintf(query_buf, "INSERT INTO skill_record (name, skill, timer, flag) VALUES ('%s', '%s', '%d', '%d')", ch->name, skill_name, timer_length, flag);
		
		mysql_safe_query(query_buf);
	}
	
	ch->send_to_char("You think you just learned something!\n");
	return;
}