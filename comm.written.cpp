//////////////////////////////////////////////////////////////////////////////
//
/// comm.written.cpp : Written Communication Module
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

#include <string>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <memory>

#include "server.h"
#include "account.h"
#include "utility.h"
#include "character.h"
#include "group.h"


extern const char* dirs[];
extern const char* opposite_dirs[];

/* extern variables */

extern ROOM_DATA *world;
extern DESCRIPTOR_DATA *descriptor_list;
extern std::map<std::string, SKILL_DATA*> skill_data_map;
extern rpie::server engine;


void
do_select_script (CHAR_DATA * ch, char *argument, int cmd)
{
	char output[MAX_INPUT_LENGTH] = { '\0' };
	char* skill_name;
	std::map<std::string, SKILL_DATA*>::iterator skill_it;
	SKILL_DATA* tskill;
	
	if (!*argument)
	{
		sprintf(output, "You are currently writing in %s. Your possible choices are:\n", ch->writes);
		for (skill_it = skill_data_map.begin(); skill_it != skill_data_map.end(); skill_it++)
		{
			tskill = skill_it->second;
			skill_name = strdup(tskill->skill_name.c_str());
			if ((tskill->written == 1)
				&& (ch->skill_map[skill_name] > 0))
				sprintf(output + strlen(output), "  %s\n",skill_name);
		}
		ch->send_to_char (output);
		return;
	}
	
	skill_it = skill_data_map.find(argument);
	if (skill_it == skill_data_map.end())
	{	
		skill_it = skill_data_map.find(CAP(argument));
		if (skill_it == skill_data_map.end())
		{
			ch->send_to_char ("That isn't a script!\n");
			return;
		}
	}
	
	if ((ch->skill_map[argument] < 1)
		&& (ch->skill_map[CAP(argument)] < 1))
	{
		sprintf (output, "You are unfamiliar with %s.\n", CAP(argument));
		ch->send_to_char (output);
		return;
	}
	
	if (!str_cmp(ch->writes, CAP(argument)))
	{
		sprintf (output, "You are already writing in %s.\n", ch->writes);
		ch->send_to_char (output);
		return;
	}
	
	ch->writes = strdup(CAP(argument));
	
	sprintf (output, "You begin writing in %s.\n", ch->writes);
	ch->send_to_char (output);
	return;
	
}


void
do_plan (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[AVG_STRING_LENGTH * 2] = "";

	// change of plans
	if (argument && *argument)
	{
		while (isspace (*argument))
			argument++;

		// clear both strings
		if (strcasecmp (argument, "clear") == STR_MATCH )
		{
			if (ch->pc->plan)
			{
				delete ch->pc->plan;
				ch->pc->plan = 0;
			}
			if (ch->pc->goal)
			{
				delete ch->pc->goal;
				ch->pc->goal = 0;
			}
			ch->send_to_char ("All of your plans have been cleared.\n");
		}

		// change the short-term plan
		else if (strncmp (argument, "short ", 6) == STR_MATCH)
		{
			argument += 6;

			// clear the short term plan
			if (strcasecmp (argument, "clear") == STR_MATCH)
			{
				if (ch->pc->plan)
				{
					delete ch->pc->plan;
					ch->pc->plan = 0;
				}
				ch->send_to_char ("Your short-term plan has been cleared.\n");
			}

			// (re)set the short-term plan
			else
			{
				int plan_length = strlen(argument);
				if (plan_length && plan_length < 80)
				{
					if (ch->pc->plan)
					{
						delete ch->pc->plan;
						ch->pc->plan = 0;
					}

					ch->pc->plan = new std::string (argument);
					sprintf (buf, "Your short-term plan has been set to:\n"
						"#6%s#0\n", argument);
					ch->send_to_char (buf);
				}

				// bad plan message size
				else
				{
					ch->send_to_char ("Your short-term plan must be less than eighty characters in length.\nTo clear your plan, type #6plan short clear#0.\n");
				}
			}
		}

		// change the long-term plan
		else if (strncmp (argument, "long ", 5) == STR_MATCH)
		{
			argument += 5;

			// clear the long-term plan
			if (strcasecmp (argument, "clear") == STR_MATCH)
			{
				if (ch->pc->goal)
				{
					delete ch->pc->goal;
					ch->pc->goal = 0;
				}
				ch->send_to_char ("Your long-term plan has been cleared.\n");
			}

			// (re)set the long-term plan
			else
			{
				int goal_length = strlen(argument);
				if (goal_length && goal_length < 240)
				{
					if (ch->pc->goal)
					{
						delete ch->pc->goal;
						ch->pc->goal = 0;
					}

					ch->pc->goal = new std::string (argument);
					sprintf (buf, "Your long-term plan has been set to:\n\n"
						"   #6%s#0\n", argument);
					char *p;
					reformat_string (buf, &p);
					ch->send_to_char (p);
					free_mem (p);
				}

				// bad message size
				else
				{
					ch->send_to_char ("Your long-term plan must be at most three lines.\nTo clear your plan, type #6plan long clear#0.\n");
				}
			}
		}
		else
		{
			const char * usage =
				"To set your short-term plan, type:     #6plan short <message>#0\n"
				"To clear your short-term plan, type:   #6plan short clear#0\n"
				"To set your long-term plan, type:      #6plan long <message>#0\n"
				"To clear your long-term plan, type:    #6plan long clear#0\n"
				"To clear your all of your plans, type: #6plan clear#0\n";
			ch->send_to_char (usage);
		}
	}
	else
	{
		if ((ch->pc->plan && !ch->pc->plan->empty()) || (ch->pc->goal && !ch->pc->goal->empty()))
		{
			strcat (buf,"Your plans:\n");
			if (ch->pc->goal)
				sprintf (buf + strlen(buf), "\nLong-term:\n#6   %s#0\n", ch->pc->goal->c_str());

			if (ch->pc->plan)
				sprintf (buf + strlen(buf), "\nCurrently:\n#6%s#0\n", ch->pc->plan->c_str());

			ch->send_to_char (buf);
		}
		else
		{
			ch->send_to_char ("You do not have any plans.\n");
		}
	}

}

