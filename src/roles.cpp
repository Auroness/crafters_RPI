//////////////////////////////////////////////////////////////////////////////
//
/// roles.c : coded support for the Role OLC system
//
//
/// Crafters RPI
/// Copyright (C) 2015 M. C. Huston
/// Authors: M.C Huston (auroness@gmail.com)
//
/// Shadows of Isildur RPI Engine++
/// Copyright (C) 2005-2007 C. W. McHenry
/// Authors: C. W. McHenry (traithe@middle-earth.us)
///          Jonathan W. Webb (sighentist@middle-earth.us)
///          Mary C. Huston (bristlecone@middle-earth.us)
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
/*----------------------------------------------------------------------
-- Table structure for table `special_roles`
--

CREATE TABLE IF NOT EXISTS `special_roles` (
`summary` varchar(255) default NULL,
`poster` varchar(255) default NULL,
`date` varchar(255) default NULL,
`cost` int(10) unsigned default NULL,
`body` text,
`timestamp` int(10) unsigned NOT NULL default '0'
) TYPE=MyISAM;

--
-- Table structure for table `special_roles_outfit`
--

CREATE TABLE IF NOT EXISTS `special_roles_outfit` (
`role_id` int(11) NOT NULL,
`outfit_id` int(11) auto_increment,
`role_desc` text,
`obj_vnum` int(11) NOT NULL default '0',
`obj_qty` int(11) NOT NULL default '1',
`clan_string` varchar(255) NOT NULL,
`clan_rank` varchar(255)NOT NULL,
`payday_num` int(11)  NOT NULL default '0',
`payday_obj_vnum` int(11) NOT NULL default '0',
`payday_obj_qty` int(11) NOT NULL default '1',
`payday_employer` int(11) NOT NULL default '0',
`payday_days` int(11) NOT NULL default '0',
`req_skill` varchar(255) NOT NULL,
`req_race` varchar(255) NOT NULL,
`skill_string` varchar(255) NOT NULL,
`skill_value` int(11)  NOT NULL default '0',
`craft` varchar(255) NOT NULL,
KEY `role_id` (`role_id`),
PRIMARY KEY  (`outfit_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
\------------------------------------------------------------------*/



#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
	
#include "structs.h"
#include "account.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"


extern void do_addcraft (CHAR_DATA * ch, char *argument, int cmd);
extern std::map<std::string, SUBCRAFT_HEAD_DATA*> craft_map;

void delete_role (CHAR_DATA * ch, char *argument);



void
display_outfitting_table (CHAR_DATA *ch, ROLE_DATA *role)
{
	MYSQL_RES *result = NULL;
	MYSQL_ROW row = NULL;
	CLAN_DATA *clan;
	SUBCRAFT_HEAD_DATA *craft;
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	char skill_name[AVG_STRING_LENGTH];
	int skill_level;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char output[MAX_STRING_LENGTH]= { '\0' };
	char skills_buf[MAX_STRING_LENGTH]= { '\0' };
	char races_buf[MAX_STRING_LENGTH]= { '\0' };
	int ind = -1;

	sprintf (output, "\n#6%s:#0\n"
		"\n", role->summary);

	//Starting Items
	sprintf (buf, "SELECT * FROM special_roles_outfit WHERE role_id = %d AND obj_vnum > 0 ORDER BY obj_vnum ASC", role->id);
	mysql_safe_query (buf);

	result = mysql_store_result (database);

	*buf = '\0';

	if ( !result || !mysql_num_rows (result) )
	{
		sprintf (output + strlen(output), "#6Starting Items:#0\nNone.\n");
		if ( result )
			mysql_free_result (result);
	}
	else
	{
		sprintf (buf, "#6Starting Items:#0\n");
		while ( (row = mysql_fetch_row(result)) )
		{
			if ( !vtoo(atoi(row[3])) )
				continue;
			if ( strlen(row[12]) > 0 || strlen(row[13]) > 0 )
				sprintf (skills_buf, " (%s%s%s)", row[13], (strlen(row[12]) > 0 && strlen(row[13]) > 0) ? " && " : "", row[12]);
			else *skills_buf = '\0';
			sprintf (buf + strlen(buf), "  - %d x #2%s#0 [%d]%s\n",
				atoi(row[4]), obj_short_desc(vtoo(atoi(row[3]))), atoi(row[3]), skills_buf);
		}
		sprintf (output + strlen(output), "%s", buf);
		if ( result )
			mysql_free_result (result);
	}

	//Starting Clanning
	sprintf (buf, "SELECT * FROM special_roles_outfit WHERE role_id = %d AND clan_string != '' ORDER BY clan_string ASC", role->id);
	mysql_safe_query (buf);

	result = mysql_store_result (database);

	if ( !result || !mysql_num_rows (result) )
	{
		sprintf (output + strlen(output), "\n#6Starting Clanning:#0\nNone.\n");
		if ( result )
			mysql_free_result (result);
	}
	else
	{
		sprintf (buf, "#6Starting Clanning:#0\n");
		while ( (row = mysql_fetch_row(result)) )
		{
			if ( !(clan = get_clandef (row[5])) )
				continue;
			sprintf (buf + strlen(buf), "  - #B%s#0 in #B'%s'#0 (%s)\n", row[6], clan->literal, row[5]);
		}
		sprintf (output + strlen(output), "\n%s", buf);
		if ( result )
			mysql_free_result (result);
	}

	//Starting Jobs and Paydays
	sprintf (buf, "SELECT * FROM special_roles_outfit WHERE role_id = %d AND payday_num > 0 ORDER BY payday_num ASC", role->id);
	mysql_safe_query (buf);

	result = mysql_store_result (database);

	if ( !result || !mysql_num_rows (result) )
	{
		sprintf (output + strlen(output), "\n#6Starting Paydays:#0\nNone.\n");
		if ( result )
			mysql_free_result (result);
	}
	else
	{
		sprintf (buf, "#6Starting Paydays:#0\n");
		while ( (row = mysql_fetch_row(result)) )
		{
			if ( !vtoo(atoi(row[8])) || !vtom(atoi(row[10])) )
				continue;
			sprintf (buf + strlen(buf), "  %d. %d x #2%s#0 from #5%s#0 every %d days.\n",
				atoi(row[7]), atoi(row[9]), obj_short_desc(vtoo(atoi(row[8]))), (vtom(atoi(row[10])))->char_short(),
				atoi(row[11]));
		}
		sprintf (output + strlen(output), "\n%s", buf);
		if ( result )
			mysql_free_result (result);
	}

	//Starting Skill Boosts
	sprintf (buf, "SELECT * FROM special_roles_outfit WHERE role_id = %d AND skill_string != '' ORDER BY skill_string ASC", role->id);
	mysql_safe_query (buf);

	result = mysql_store_result (database);

	if ( !result || !mysql_num_rows (result) )
	{
		sprintf (output + strlen(output), "\n#6Starting Skill Boosts:#0\nNone.\n");
		if ( result )
			mysql_free_result (result);
	}
	else
	{
		sprintf (buf, "#6Starting Skill Boosts:#0\n");
		while ( (row = mysql_fetch_row(result)) )
		{
			if (strlen(row[13]) > 0)
				sprintf(races_buf, "%s", row[13]);
			else 
				sprintf(races_buf, "");
			
			sprintf(skill_name, "%s", row[14]);
			skill_level = atoi(row[15]);
			
			
			if (ind = lookup_skill_id(skill_name) != -1 )
			{
				if (skill_level == 999)
				{
					sprintf (buf + strlen(buf), "  - #B%s#0 at opening value", skill_name);
				}
				
				else if ((skill_level > 0) && (skill_level != 999))
				{
					sprintf (buf + strlen(buf), "  - #B%s#0 boosted by #B%d#0 points", skill_name, skill_level);
				}
				else 
				{
					sprintf (buf + strlen(buf), "  - #B%s#0 set at #B%d#0", skill_name, abs(skill_level));
				}
				
				if (strlen(row[13]) > 0)
				{
					sprintf (buf + strlen(buf), " (%s).\n", races_buf);
					
				}
				else
				{
					sprintf (buf + strlen(buf), ".\n");
				}
				
			}
		}//end while

		sprintf (output + strlen(output), "\n%s", buf);
		if ( result )
			mysql_free_result (result);
	}

	//Starting Crafts
	sprintf (buf, "SELECT * FROM special_roles_outfit WHERE role_id = %d AND craft != '' ORDER BY craft ASC", role->id);
	mysql_safe_query (buf);

	result = mysql_store_result (database);

	if ( !result || !mysql_num_rows (result) )
	{
		sprintf (output + strlen(output), "\n#6Starting Crafts:#0\nNone.\n");
		if ( result )
			mysql_free_result (result);
	}
	else
	{
		sprintf (buf, "#6Starting Crafts:#0\n");
		while ( (row = mysql_fetch_row(result)) )
		{
			if (strlen(row[13]) > 0)
				sprintf(races_buf, "%s", row[13]);
			else 
				sprintf(races_buf, "");
			
			for (tcraft_iterator = craft_map.begin(); tcraft_iterator != craft_map.end(); tcraft_iterator++)
			{
				craft = tcraft_iterator->second;
				if (!str_cmp (craft->subcraft_name, row[16]))
					break;
			}

			if (craft)
			{
				sprintf (buf + strlen(buf), "  - #B%s#0", row[16]);
			}
			
			if (strlen(row[13]) > 0)
			{
				sprintf (buf + strlen(buf), " (%s).\n", races_buf);
				
			}
			else
			{
				sprintf (buf + strlen(buf), ".\n");
			}
		}

		sprintf (output + strlen(output), "\n%s", buf);
		if ( result )
			mysql_free_result (result);
	}

	//Additional Briefing for PC
	sprintf (buf, "SELECT role_desc FROM special_roles_outfit WHERE role_id = %d AND role_desc != ''", role->id);
	mysql_safe_query (buf);

	result = mysql_store_result (database);

	if ( !result || !mysql_num_rows (result) )
	{
		sprintf (output + strlen(output), "\n#6Role Briefing:#0\nNone.\n");
		if ( result )
			mysql_free_result (result);
	}
	else
	{
		sprintf (buf, "#6Role Briefing:#0\n");
		row = mysql_fetch_row(result);
		sprintf (buf + strlen(buf), "%s", row[0]);
		sprintf (output + strlen(output), "\n%s", buf);
		mysql_free_result (result);
	}

	ch->send_to_char(output);
}

void
post_brief (DESCRIPTOR_DATA * d)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2 [MAX_STRING_LENGTH]= { '\0' };

	if (!d->pending_message->message)
	{
		d->character->send_to_char("No role briefing was recorded.\n");
		d->pending_message = NULL;
		return;
	}

	mysql_real_escape_string (database, buf, d->pending_message->message, strlen(d->pending_message->message));
	d->pending_message = NULL;

	d->character->action &= ~PLR_QUIET;

	sprintf (buf2, "DELETE FROM special_roles_outfit WHERE role_id = %ld AND role_desc != ''",
		d->character->delay_info1);
	mysql_safe_query(buf2);

	sprintf (buf2, "INSERT INTO special_roles_outfit (role_id, role_desc) VALUES (%ld, '%s')",
		d->character->delay_info1, buf);
	mysql_safe_query(buf2);

	d->character->send_to_char("Your role briefing has been recorded.\n");

	d->character->delay_who = NULL;
	d->character->delay_info1 = 0;
}

void
post_body (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *ch;
	ROLE_DATA *role;

	ch = d->character;

	/*if (!*ch->pc->msg)
	{
		ch->send_to_char("Body update aborted.\n");
		return;
	}*/

	for (role = role_list; role; role = role->next)
	{
		if (role->id == ch->delay_info1)
			break;
	}

	//role->body = duplicateString (ch->pc->msg);
	role->body = duplicateString (d->pending_message->message);

	save_roles ();
}

void
outfit_role (CHAR_DATA * ch, char *argument)
{
	CLAN_DATA	*clan = NULL;
	SUBCRAFT_HEAD_DATA *craft;
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char skills_buf [AVG_STRING_LENGTH];
	char races_buf[AVG_STRING_LENGTH];
	char skill_name[AVG_STRING_LENGTH];
	char craft_buf[AVG_STRING_LENGTH];
	char craft_name[AVG_STRING_LENGTH];
	int vnum = 0;
	int	qty = 1;
	int	jobnum = 0;
	int	days = 0;
	int	employer = 0;
	int	ind = 0;
	int	skill_level = 0;
	ROLE_DATA *role;
	bool remove = false;
	char role_num[AVG_STRING_LENGTH];
	char cmd[AVG_STRING_LENGTH];

	argument = one_argument (argument, role_num);

	//Find Role
	if (!just_a_number (role_num))
	{
		ch->send_to_char("Syntax: role outfit <role number> <cmd>\n");
		ch->send_to_char("<cmd> can be any of the following:\n");
		ch->send_to_char("delete - deletes the entire outfit entry\n");
		ch->send_to_char("brief  - A briefing for the PC taking the role.\n");
		ch->send_to_char("payday - set up to 3 paydays.\n");
		ch->send_to_char("object - Items loaded for new PC.\n");
		ch->send_to_char("clan   - Clan and ranks\n");
		ch->send_to_char("skill  - Skills and boosts\n");
		ch->send_to_char("craft  - Extra starting crafts\n");
		return;
	}

	ind = atoi(role_num);

	for (role = role_list; role; role = role->next)
	{
		if (role->id == ind)
			break;
	}

	if (!role)
	{
		ch->send_to_char("That role had no outfit table listed.\n");
		return;
	}

	argument = one_argument (argument, cmd);

	//Delete the outfitting table
	if ( !str_cmp (cmd, "delete") )
	{
		sprintf (buf, "DELETE FROM special_roles_outfit WHERE role_id = %d", ind);
		mysql_safe_query (buf);
		return;
	}


	//Briefing for Role
	if ( !str_cmp (cmd, "brief") )
	{
		ch->send_to_char("\n#2Enter a briefing for this role that you'd like to appear to starting characters:#0\n");
		free_mem(ch->desc->descStr);
		free_mem(ch->desc->pending_message);
		ch->desc->pending_message = new MESSAGE_DATA;
		ch->desc->descStr = ch->desc->pending_message->message;
		ch->desc->max_str = MAX_STRING_LENGTH;
		ch->desc->proc = post_brief;
		ch->delay_info1 = role->id;
		ch->action |= PLR_QUIET;
		return;
	}

	//Outfitting test
	if ( !str_cmp (cmd, "outfittest") )
	{
		outfit_new_char (ch, role);
		ch->send_to_char("Done.\n");
		return;
	}

	//Paydays
	//role outfit X payday <jobnum> <qty> <obj. vnum> <days> <employer>
	//role outfit X payday <jobnum> remove
	if ( !str_cmp (cmd, "payday") )
	{
		argument = one_argument (argument, buf);
		if ( !*buf || !just_a_number(buf) )
		{
			ch->send_to_char("Usage: role outfit X payday <jobnum> <qty> <obj. vnum> <days> <employer>\n");
			ch->send_to_char("Usage: role outfit X payday <jobnum> remove\n");
			return;
		}
		jobnum = atoi(buf);

		if ( jobnum < 1 || jobnum > 3 )
		{
			ch->send_to_char("Which payday (1, 2 or 3) did you want to outfit this role with?\n");
			return;
		}

		argument = one_argument (argument, buf);

		if ( !just_a_number(buf) && !str_cmp (buf, "remove") )
		{
			sprintf (buf, "DELETE FROM special_roles_outfit WHERE role_id = %d AND payday_num = %d", role->id, jobnum);
			mysql_safe_query (buf);

			sprintf (buf, "Payday ##%d has been removed from the outfitting tables for the role of #6'%s'#0.", jobnum, role->summary);
			ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}

		if ( !*buf )
		{
			ch->send_to_char("How many objects did you wish the payday to provide?\n");
			return;
		}
		qty = atoi(buf);

		argument = one_argument (argument, buf);
		if ( !*buf || !just_a_number(buf) )
		{
			ch->send_to_char("What is the VNUM of the object that this job pays in?\n");
			return;
		}
		vnum = atoi(buf);

		if ( !vtoo(vnum) )
		{
			ch->send_to_char("That object does not exist.\n");
			return;
		}

		argument = one_argument (argument, buf);
		if ( !*buf || !just_a_number(buf) )
		{
			ch->send_to_char("How many days between pay periods should there be?\n");
			return;
		}
		days = atoi(buf);

		argument = one_argument (argument, buf);
		if ( !*buf || !just_a_number(buf) )
		{
			ch->send_to_char("What is the VNUM of the mob you wish to be the employer for this payday?\n");
			return;
		}
		employer = atoi(buf);

		if ( !vtom(employer) )
		{
			ch->send_to_char("An NPC of that VNUM could not be found.\n");
			return;
		}

		if ( qty < 1 || employer < 1 || days < 1 || vnum < 1 )
		{
			ch->send_to_char("Positive values must be specified for this command.\n");
			return;
		}

		sprintf (buf, "DELETE FROM special_roles_outfit WHERE role_id = %d AND payday_num = %d", role->id, jobnum);
		mysql_safe_query (buf);

		sprintf (buf,	"INSERT INTO special_roles_outfit (role_id, payday_num, payday_obj_vnum, "
			"payday_obj_qty, payday_employer, payday_days) VALUES (%d, %d, %d, %d, %d, %d)",
				 role->id,
				 jobnum,
				 vnum,
				 qty,
				 employer,
				 days);
		mysql_safe_query (buf);

		sprintf (buf, "A payday for %d x #2%s#0 [%d] from #5%s#0 [%d] every %d days has been inserted "
			"as job ##%d into the outfitting tables for the role of #6'%s'#0.",
				 qty,
				 obj_short_desc(vtoo(vnum)),
				 vnum,
				 (vtom(employer))->char_short(),
				 employer,
				 days,
				 jobnum,
				 role->summary);
		ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	//Starting items
	//role outfit X object <vnum> <qty> <skill>
	//role outfit X object <vnum> <qty> <race>
	//role outfit X object <vnum> remove
	if ( !str_cmp (cmd, "object") )
	{
		*skills_buf = '\0';
		*races_buf = '\0';

		argument = one_argument (argument, buf); //vnum
		if ( !*buf || !just_a_number(buf) )
		{
			ch->send_to_char("Usage:  role outfit X object <vnum> <qty> \n");
			ch->send_to_char("Usage:  role outfit X object <vnum> remove \n");
			ch->send_to_char("Usage:  role outfit X object <vnum> <qty>  <skill>\n");
			ch->send_to_char("Usage:  role outfit X object <vnum> <qty> <race> \n");
			ch->send_to_char("Usage:  role outfit X object <vnum> remove\n");
			return;
		}
		vnum = atoi(buf);

		argument = one_argument (argument, buf);//qty or remove
		if ( *buf )
		{
			if ( just_a_number(buf) )
				qty = atoi(buf);
			else if ( !str_cmp (buf, "remove") )
				remove = true;
			else
			{
				ch->send_to_char("You must specify an object vnum, and optionally a quantity or REMOVE.\n");
				return;
			}
		}
		if ( qty <= 0 || vnum <= 0 )
		{
			ch->send_to_char("You must specify a positive object VNUM and a quantity greater than 1.\n");
			return;
		}
		if ( !vtoo(vnum) )
		{
			ch->send_to_char("That object VNUM does not exist in-game.\n");
			return;
		}

		argument = one_argument (argument, buf); //skill or race
		if ( *buf )
		{
			if ( (ind = lookup_skill_id(buf)) != -1 )
				sprintf (skills_buf, "%s", buf);
			else if ( (ind = lookup_race_id (buf)) != -1 )
				sprintf (races_buf, "%s",buf);
		}

		if ( remove )
		{
			sprintf (buf, "DELETE FROM special_roles_outfit WHERE role_id = %d AND obj_vnum = %d", role->id, vnum);
			mysql_safe_query (buf);
			sprintf (buf, "All instances of #2%s#0 have been removed from the outfitting tables for the role of #6'%s'#0.",
				obj_short_desc (vtoo(vnum)), role->summary);
			ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}
		else
		{
			sprintf (buf,   "INSERT INTO special_roles_outfit (role_id, obj_vnum, obj_qty, req_skill, "
				"req_race) VALUES (%d, %d, %d, '%s', '%s')", role->id, vnum, qty, skills_buf, races_buf);
			mysql_safe_query (buf);

			sprintf (buf, "%d x #2%s#0 ha%s been added to the outfitting tables for the role of #6'%s'#0",
				qty, obj_short_desc (vtoo(vnum)), qty != 1 ? "ve" : "s", role->summary);

			if ( *races_buf || *skills_buf )
			{
				sprintf (buf + strlen(buf), ", for all");
				if ( *races_buf )
					sprintf (buf + strlen(buf), " #B%s#0 characters", races_buf);
				if ( *skills_buf )
					sprintf (buf + strlen(buf), " with skill in #B%s#0", skills_buf);
			}
			sprintf (buf + strlen(buf), ".");

			ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}
	}

	//Clans
	if ( !str_cmp (cmd, "clan") )
	{
		argument = one_argument (argument, buf);
		if ( !*buf )
		{
			ch->send_to_char("Usage:  role outfit X clan <shortname> <rank> \n");
			ch->send_to_char("Usage:  role outfit X clan <shortname> remove \n");
			return;
		}
		if ( !(clan = get_clandef (buf)) )
		{
			ch->send_to_char("That clan does not exist in-game.\n");
			return;
		}
		argument = one_argument (argument, buf);
		if ( !*buf )
		{
			ch->send_to_char("You must also specify a rank, or REMOVE. When in doubt, use 'member.'\n");
			return;
		}
		if ( !str_cmp (buf, "remove") )
		{
			sprintf (buf2, "DELETE FROM special_roles_outfit WHERE role_id = %d AND clan_string = '%s'",
				role->id, clan->name);
			mysql_safe_query (buf2);

			sprintf (buf2,	"All instances of clanning for the clan #B'%s'#0 have been removed from the outfitting tables "
				"for the role of #6'%s'#0.", clan->literal, role->summary);
			ch->act(buf2, false, 0, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}
		if (str_cmp (buf, "member") && str_cmp (buf, "leader") && str_cmp (buf, "recruit") && str_cmp (buf, "private") &&
			str_cmp (buf, "corporal") && str_cmp (buf, "sergeant") && str_cmp (buf, "lieutenant") && str_cmp (buf, "captain") &&
			str_cmp (buf, "general") && str_cmp (buf, "commander") && str_cmp (buf, "apprentice") && str_cmp (buf, "journeyman") &&
			str_cmp (buf, "master") )
		{
			ch->send_to_char("That is not a recognized rank for clans.\n");
			return;
		}
		sprintf (buf2, "INSERT INTO special_roles_outfit (role_id, clan_string, clan_rank) VALUES (%d, '%s', '%s')",
			role->id, clan->name, buf);
		mysql_safe_query (buf2);

		sprintf (buf2,	"Clanning for the rank of #B%s#0 in the clan #B'%s'#0 has been inserted into the outfitting tables "
			"for the role of #6'%s'#0.", buf, clan->literal, role->summary);
		ch->act(buf2, false, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	//Skill Boost
	//role outfit X skill <skillname> <value>
	//role outfit X skill <skillname> remove
	if ( !str_cmp (cmd, "skill") )
	{
		*skills_buf = '\0';

		argument = one_argument (argument, skill_name); //skillname
		if ( !*skill_name || just_a_number(skill_name) )
		{
			ch->send_to_char("Usage:  role outfit X skill <skillname> [\"racename\"] \n");
			ch->send_to_char("Usage:  role outfit X skill <skillname> [value] [\"racename\"] \n");
			ch->send_to_char("Usage:  role outfit X skill <skillname> remove \n");
			ch->send_to_char("No value or value = 0, will give skill at opening value \n");
			ch->send_to_char("[value] will give a boost, if they already have the skill  \n");
			ch->send_to_char("-[value] will set the skill at that level, if they already have the skill, and it is lower.\n");
			ch->send_to_char("[racename] to specify a race that is need to receive this skill.\n");
			return;
		}

		argument = one_argument (argument, buf);//skill_level, race name or remove
		if ( *buf )
		{
			bool chk_flag = false;
			int chk_value = 0;
			chk_value = atoi(buf);
			if ((chk_value == 999) || (chk_value < 100))
				chk_flag = true;
	
			if (chk_flag)
			{
				skill_level = atoi(buf);
				argument = one_argument (argument, buf);//skill_level & race name
				if ((ind = lookup_race_id (buf)) != -1 )
					sprintf (races_buf, "%s", buf);
				else 
					sprintf (races_buf, "");
				
			}
			else if ((ind = lookup_race_id (buf)) != -1 ) //no skill level, just a race
				sprintf (races_buf, "%s", buf);
			
			else if ( !str_cmp (buf, "remove") )
				remove = true;
			else
			{
				ch->send_to_char("You must specify a skill name, and either a level, race or REMOVE.\n");
				return;
			}
		}

		if ( (ind = lookup_skill_id(skill_name)) != -1 )
			sprintf (skills_buf, "%s", skill_name);
		else
		{
			ch->send_to_char("That skill is not in out database.\n");
			return;
		}


		if ( remove )
		{
			sprintf (buf, "DELETE FROM special_roles_outfit WHERE role_id = %d AND skill_string = '%s'",
				role->id, skills_buf);
			mysql_safe_query (buf);
			sprintf (buf, "All instances of #2%s#0 have been removed from the outfitting tables for the role of #6'%s'#0.",
				skills_buf, role->summary);
			ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}
		else
		{
			if (skill_level == 0)
				skill_level = 999;
			
			sprintf (buf,   "INSERT INTO special_roles_outfit (role_id, skill_string, skill_value, req_race) VALUES (%d, '%s', %d, '%s')", role->id, skills_buf, skill_level, races_buf);
			mysql_safe_query (buf);

			if (skill_level == 999)
			{
				sprintf (buf, "'%s' will be given at opening values for the role of #6'%s'#0", skills_buf, role->summary);
			}
			else if (skill_level > 0)
			{
				sprintf (buf, "'%s' will be boosted by %d points, if they already have the skill, for the role of #6'%s'#0", skills_buf, skill_level, role->summary);
			}
			else if (skill_level < 0)
			{
				sprintf (buf, "'%s' will be set at %d, if they already have the skill and it is lower, for the role of #6'%s'#0", skills_buf, abs(skill_level), role->summary);
			}
			
			if ( *races_buf )
			{
				sprintf (buf + strlen(buf), ", for all  #B%s#0 characters", races_buf);
			}
			
			sprintf (buf + strlen(buf), ".\n");
			
			ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}
	} //end skill

	//Crafts
	//role outfit X craft <craftname>
	//role outfit X craft <craftname> <race>
	//role outfit X craft <craftname> remove
	if ( !str_cmp (cmd, "craft") )
	{
		*craft_buf = '\0';
		*races_buf = '\0';
		
		argument = one_argument (argument, craft_name); //subcraft name
		if ( !*craft_name )
		{
			ch->send_to_char("Usage:  role outfit X craft <craftname>\n");
			ch->send_to_char("Usage:  role outfit X craft <craftname> <racename>\n");
			ch->send_to_char("Usage:  role outfit X craft <craftname> remove \n");
			return;
		}

		argument = one_argument (argument, buf); //race or remove
		
		if (!str_cmp (buf, "remove"))
		{
			remove = true;
		}
		else if ( *buf )
		{
			if ( (ind = lookup_race_id (buf)) != -1 )
				sprintf (races_buf, "%s", buf);
		}		
		


		for (tcraft_iterator = craft_map.begin(); tcraft_iterator != craft_map.end(); tcraft_iterator++)
		{
			craft = tcraft_iterator->second;
			if (!str_cmp (craft->subcraft_name, craft_name))
				break;
		}

		if (!craft)
		{
			ch->send_to_char
				("No such craft.  Type 'crafts' for a listing.\n");
			return;
		}


		if ( remove )
		{
			sprintf (buf, "DELETE FROM special_roles_outfit WHERE role_id = %d AND craft = '%s'",
				role->id, craft_name);
			mysql_safe_query (buf);
			sprintf (buf, "All instances of #2%s#0 have been removed from the outfitting tables for the role of #6'%s'#0.",
				craft_name, role->summary);
			ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}
		else
		{
			sprintf (buf,   "INSERT INTO special_roles_outfit (role_id, craft, req_race) VALUES (%d, '%s', '%s')", role->id, craft_name, races_buf);
			mysql_safe_query (buf);

			sprintf (buf, "'%s' has been added to the outfitting tables for the role of #6'%s'#0", craft_name, role->summary);
			
			if ( *races_buf )
			{
				sprintf (buf + strlen(buf), ", for all");
				if ( *races_buf )
					sprintf (buf + strlen(buf), " #B%s#0 characters", races_buf);
				
			}
			sprintf (buf + strlen(buf), ".");
			
			ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
			return;
		}
	}// craft


	display_outfitting_table(ch, role);	//default command if there are no arguments
}

void
delete_role (CHAR_DATA * ch, char *argument)
{
	ROLE_DATA *role = NULL;
	ROLE_DATA *temp_role = NULL;
	bool deleted = false;
	char buf[MAX_STRING_LENGTH]= { '\0' };



	if (!isdigit (*argument))
	{
		ch->send_to_char("Syntax: role delete <role number>\n");
		return;
	}

	if (atoi (argument) == 1)
	{
		if (ch->get_trust() <= 4 && str_cmp (role_list->poster, ch->name))
		{
			ch->send_to_char
				("Sorry, but you are only allowed to delete your own roles.\n");
			return;
		}
		if (!role_list->next)
		{
			sprintf (buf, "DELETE FROM special_roles_outfit WHERE role_id = %d", atoi (argument));
			mysql_safe_query (buf);

			sprintf (buf, "DELETE FROM special_roles WHERE role_id = %d", atoi (argument));
			mysql_safe_query (buf);

			free_mem (role_list->summary);
			free_mem (role_list->poster);
			free_mem (role_list->body);
			role_list = NULL;
			ch->send_to_char
				("The specified special role has been removed as an option from chargen.\n");
			return;
		}
		else
		{
			temp_role = role_list->next;
			role_list->next = role_list->next->next;

			sprintf (buf, "DELETE FROM special_roles_outfit WHERE role_id = %d", temp_role->id);
			mysql_safe_query (buf);

			sprintf (buf, "DELETE FROM special_roles WHERE role_id = %d", temp_role->id);
			mysql_safe_query (buf);

			free_mem (temp_role->summary);
			free_mem (temp_role->poster);
			free_mem (temp_role->body);
			temp_role = NULL;
			ch->send_to_char
				("The specified special role has been removed as an option from chargen.\n");
			return;
		}
	}
	else
	{
		for (role = role_list; role && role->next; role = role->next)
		{
			if (role->next->id == atoi (argument))
			{
				if (ch->get_trust() == 4
					&& str_cmp (role->next->poster, ch->name))
				{
					ch->send_to_char
						("Sorry, but you are only allowed to delete your own roles.\n");
					return;
				}
				temp_role = role->next;
				role->next = role->next->next;
				deleted = true;
				break;
			}
		}//for


	}
	if (!deleted)
	{
		ch->send_to_char("That role number was not found.\n");
		return;
	}
	else
	{
		sprintf (buf, "DELETE FROM special_roles_outfit WHERE role_id = %d", temp_role->id);
		mysql_safe_query (buf);
		sprintf (buf, "DELETE FROM special_roles WHERE role_id = %d", temp_role->id);
		mysql_safe_query (buf);

		free_mem (temp_role->summary);
		free_mem (temp_role->poster);
		free_mem (temp_role->body);
		temp_role = NULL;
	}
	ch->send_to_char
		("The specified special role has been removed as an option from chargen.\n");
	save_roles ();
}

void
display_role (CHAR_DATA * ch, char *argument)
{
	char output[MAX_STRING_LENGTH]= { '\0' };
	ROLE_DATA *role;

	if (!isdigit (*argument))
	{
		ch->send_to_char("Syntax: role display <role number>\n");
		return;
	}

	for (role = role_list; role; role = role->next)
	{
		if (role->id == atoi (argument))
			break;
	}

	if (!role)
	{
		ch->send_to_char("That role number was not found.\n");
		return;
	}

	sprintf (output,
		"\n"
		"#2Role Summary:#0         %s\n"
		"#2Posting Admin:#0        %s\n"
		"#2Posted On:#0            %s\n"
		"#2Point Cost:#0           %d\n"
		"\n%s"
		"\n",
		role->summary,
		role->poster,
		role->date,
		role->cost,
		role->body);


	page_string (ch->desc, output);
}

void
list_roles (CHAR_DATA * ch)
{
	MYSQL_RES *result;
	char output[MAX_STRING_LENGTH]= { '\0' };
	ROLE_DATA *role;
	bool outfit = false;

	*output = '\0';

	for (role = role_list; role; role = role->next)
	{
		mysql_safe_query ("SELECT role_id FROM special_roles_outfit WHERE role_id = %d LIMIT 1", role->id);
		result = mysql_store_result (database);
		if ( result && mysql_num_rows(result) > 0 )
			outfit = true;
		else outfit = false;
		if ( result )
			mysql_free_result (result);
		sprintf (output + strlen (output), "%s%5d. %s\n", (outfit == false) ? "#9*#0" : " ", role->id, role->summary);
	}

	if (!*output)
	{
		ch->send_to_char("No special roles are currently available in chargen.\n");
		return;
	}

	sprintf (output + strlen(output), "\n#9*#0 = no outfitting directives have been created for this role,\n");

	ch->send_to_char
		("\n#2The following special roles have been made available in chargen:\n#0");
	page_string (ch->desc, output);
}

void
post_role (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *ch;
	ROLE_DATA *role;
	time_t current_time;
	char *date;
	int index_id = 0;

	ch = d->character;

	
	if (!role_list)
	{
		role_list = new ROLE_DATA;
		index_id = 0;
		role = role_list;
		role->next = NULL;
	}
	else
		for (role = role_list; role; role = role->next)
		{
			if (!role->next)
			{
				role->next = new ROLE_DATA;
				index_id = role->id; //get the id before we move the pointer
				role = role->next;
				role->next = NULL;
				break;
			}
		}

		/* Get a date string from current time ( default = "" ) */
		date = new char[256];
		date[0] = '\0';
		current_time = time (0);
		if (asctime_r (localtime (&current_time), date) != NULL)
		{
			date[strlen (date) - 1] = '\0';
		}

		role->cost = ch->delay_info1;
		role->summary = duplicateString (ch->delay_who);
		//role->body = duplicateString (ch->pc->msg);
		role->body = duplicateString (d->pending_message->message);
		role->date = duplicateString (date);
		role->poster = duplicateString (ch->pc->account_name);
		role->timestamp = (int) time (0);
		role->id = index_id + 1;

		ch->delay_who = NULL;
		ch->delay_info1 = 0;

		save_roles ();

		free_mem (date);
}

void
new_role (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int cost;

	argument = one_argument (argument, buf);

	if (IS_NPC (ch))
	{
		ch->send_to_char("Please don't post roles while switched. Thanks!\n");
		return;
	}

	if (!isdigit (*buf))
	{
		ch->send_to_char("Syntax: role new <point cost> <summary line>\n");
		return;
	}
	cost = atoi (buf);

	if (cost < 0 || cost > 50)
	{
		ch->send_to_char("Permissible costs are from 0-50, inclusive.\n");
		return;
	}

	if (!*argument)
	{
		ch->send_to_char("Syntax: role new <point cost> <summary line>\n");
		return;
	}

	ch->send_to_char
		("\n#2Enter a detailed summary of the role you wish to post, to\n"
		"give prospective players a better idea as to what sort of RP\n"
		"will be required to successfully portray it.#0\n");

	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;

	ch->make_quiet();
	ch->delay_info1 = cost;
	ch->delay_who = duplicateString(argument);
	ch->pc->msg = NULL;
	free_mem(ch->desc->descStr);
	ch->desc->max_str = MAX_STRING_LENGTH;
	ch->desc->proc = post_role;
}

void
update_role (CHAR_DATA * ch, char *argument)
{
	ROLE_DATA *role = NULL;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (argument, buf);

	if (!*buf || !isdigit (*buf))
	{
		ch->send_to_char("Please specify a role number to update.\n");
		ch->send_to_char("Syntax: role update <number> <cmd>.\n");
		return;
	}

	for (role = role_list; role; role = role->next)
	{
		if (role->id == atoi (buf))
			break;
	}

	if (!role)
	{
		ch->send_to_char("I could not find that role number to update.\n");
		return;
	}

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char("Please specify which field in the role to update\n");
		ch->send_to_char("Syntax: role update <number> contact <new name>\n");
		ch->send_to_char("Syntax: role update <number> cost <new point cost>\n");
		ch->send_to_char("Syntax: role update <number> summary <new summary string>\n");
		ch->send_to_char("Syntax: role update <number> body\n");
		return;
	}
	else if (!str_cmp (buf, "contact"))
	{
		argument = one_argument (argument, buf);
		if (!*buf)
		{
			ch->send_to_char
				("Please specify who the new point of contact should be.\n");
			return;
		}
		account *acct = new account (buf);
		if (!acct->is_registered ())
		{
			delete acct;
			ch->send_to_char
				("That is not the name of a currently registered MUD account.\n");
			return;
		}
		delete acct;
		free_mem (role->poster);
		role->poster = duplicateString (CAP (buf));
		ch->send_to_char("The point of contact for that role has been updated.\n");
		save_roles ();
	}
	else if (!str_cmp (buf, "cost"))
	{
		argument = one_argument (argument, buf);
		if (!*buf || !isdigit (*buf))
		{
			ch->send_to_char("Please specify what the new cost should be.\n");
			return;
		}
		role->cost = atoi (buf);
		ch->send_to_char("The cost for that role has been updated.\n");
		save_roles ();
	}

	else if (!str_cmp (buf, "summary"))
	{
		argument = one_argument (argument, buf);
		if (!*buf)
		{
			ch->send_to_char
				("Please specify the summary for this role.\n");
			return;
		}
		role->summary = duplicateString (buf);
		save_roles ();
	}

	else if (!str_cmp (buf, "body"))
	{
		ch->send_to_char("\nThe previous body was:\n");
		sprintf(buf, "%s\n", role->body);
		ch->send_to_char(buf);

		ch->send_to_char
			("\n#2Enter a detailed summary of the role you wish to post, to\n"
			"give prospective players a better idea as to what sort of RP\n"
			"will be required to successfully portray it.#0\n");

		free_mem(ch->desc->pending_message);
		ch->desc->pending_message = new MESSAGE_DATA;

		ch->make_quiet();
		ch->delay_info1 = role->id;
		ch->pc->msg = NULL;
		free_mem(ch->desc->descStr);
		ch->desc->max_str = MAX_STRING_LENGTH;
		ch->desc->proc = post_body;
	}



}

void
do_role (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (argument, buf);

	if (IS_NPC (ch))
	{
		ch->send_to_char("Sorry, but this can't be done while switched.\n");
		return;
	}

	if (!str_cmp (buf, "new"))
		new_role (ch, argument);
	else if (!str_cmp (buf, "list"))
		list_roles (ch);
	else if (!str_cmp (buf, "display"))
		display_role (ch, argument);
	else if (!str_cmp (buf, "outfit"))
		outfit_role (ch, argument);
	else if (!str_cmp (buf, "delete"))
		delete_role (ch, argument);
	else if (!str_cmp (buf, "update"))
		update_role (ch, argument);
	else if ( !str_cmp (buf, "save"))
	{
		ch->send_to_char("Saving all roles...\n");
		save_roles();
	}
	else
		ch->send_to_char
		("Syntax: role (new|list|display|delete|update|outfit|save) <argument(s)>\n");
}

void
save_roles (void)
{
	MYSQL_RES *result;
	char text[MAX_STRING_LENGTH]= { '\0' };
	char date_buf[MAX_STRING_LENGTH]= { '\0' };
	char summary[MAX_STRING_LENGTH]= { '\0' };
	ROLE_DATA *role;

	for (role = role_list; role; role = role->next)
	{
		*summary = '\0';
		*date_buf = '\0';
		*text = '\0';

		mysql_safe_query ("SELECT role_id FROM special_roles WHERE role_id = %d ORDER BY role_id ASC", role->id);
		result = mysql_store_result (database);


		if ( result && mysql_num_rows (result) > 0 )
		{
			mysql_safe_query
				("UPDATE special_roles SET summary = '%s', poster = '%s', date = '%s', cost = %d, body = '%s', timestamp = %d WHERE role_id = %d",
				role->summary, role->poster, role->date, role->cost, role->body, role->timestamp, role->id);

			mysql_free_result (result);
		}
		else
		{
			if ( result )
				mysql_free_result (result);
			mysql_safe_query
				("INSERT INTO special_roles (summary, poster, date, cost, body, timestamp, role_id) VALUES ('%s', '%s', '%s', %d, '%s', %d, %d)",
				role->summary, role->poster, role->date, role->cost, role->body,
				role->timestamp, role->id);
		}

	}
}

void
reload_roles (void)
{
	ROLE_DATA *role = NULL;
	MYSQL_RES *result;
	MYSQL_ROW row;

	mysql_safe_query ("SELECT * FROM special_roles ORDER BY role_id ASC");
	result = mysql_store_result (database);

	if (!result || !mysql_num_rows (result))
	{
		if (result)
			mysql_free_result (result);
		system_log ("No roles stored in database.", false);
		return;
	}

	while ((row = mysql_fetch_row (result)))
	{
		if (!role_list)
		{
			role_list = new ROLE_DATA;
			role = role_list;
		}
		else
		{
			role->next = new ROLE_DATA;
			role = role->next;
		}
		role->next = NULL;
		role->summary = duplicateString (row[0]);
		role->poster = duplicateString (row[1]);
		role->date = duplicateString (row[2]);
		role->cost = atoi (row[3]);
		role->body = duplicateString (row[4]);
		role->timestamp = atoi (row[5]);
		role->id = atoi(row[6]);
	}

	mysql_free_result (result);
}

