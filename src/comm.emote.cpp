//////////////////////////////////////////////////////////////////////////////
//
/// comm.emote.cpp : Non-Verbal Visual Communication Module
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

#define TRAVEL_RESET "normal"	/* use this to unset their travel string */

	//What do you look like when standing still
void
do_pmote (CHAR_DATA * ch, char *argument, int cmd)
{
	char * result = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	while (isspace (*argument))
		argument++;

	if (!*argument)
	{
		sprintf (buf, "Your current pmote: (#2%s#0)\n",
			(ch->pmote_str) ? ch->pmote_str : "(#2none#0)");
		ch->send_to_char (buf);
	}

	else if (!strcmp (argument, "normal"))
	{
		ch->clear_pmote();
		ch->send_to_char ("Your current pmote string has been cleared.\n");

	}

	else if (IS_NPC(ch) && argument)
	{
		ch->pmote_str = duplicateString (argument);
	}

	else
	{
		if (ch && argument)
		{

			result = swap_xmote_target (ch, argument, 2);
			if(!result)
				return;
		}

		sprintf (buf, "You pmote: %s", result);

		ch->pmote_str = duplicateString (result);

		ch->act(buf, false, 0, 0, TO_CHAR | _ACT_FORMAT);
	}
}

	//What do you look like when moving
void
do_dmote (CHAR_DATA * ch, char *argument, int cmd)
{

	char buf[MAX_STRING_LENGTH] = { '\0' };

	while (isspace (*argument))
		argument++;

	if (!*argument)
	{
		sprintf (buf, "Your current dmote: (%s)\n",
			(ch->dmote_str) ? ch->dmote_str : "(none)");
		ch->send_to_char (buf);
	}

	else if (!strcmp (argument, "normal"))
	{
		ch->clear_dmote();
		ch->send_to_char ("Your current dmote string has been cleared.\n");

	}

	else
	{


		sprintf (buf, "You dmote: %s", argument);

		ch->dmote_str = duplicateString (argument);

		ch->act(buf, false, 0, 0, TO_CHAR | _ACT_FORMAT);
	}
}


	//adds character and object references to emotes	
void
personalize_string (CHAR_DATA * src, CHAR_DATA * tar, char *emote)
{
	char desc[MAX_STRING_LENGTH] = { '\0' };
	char output[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };
	CHAR_DATA *tch = NULL, *target = NULL;

	*output = '\0';

	while (*emote)
	{
		*desc = '\0';
		if (*emote == '#')
		{
			emote++;
			if (*emote == '5')
			{
				emote++;
				while (*emote != '#')
				{
					sprintf (desc + strlen (desc), "%c", *emote);
					emote++;
				}
				for (tch = tar->room->people; tch; tch = tch->next_in_room)
					if (strcasecmp ( tch->char_short(), desc) == STR_MATCH)
					{
						break;
					}
					emote++;
					emote++;
					if (*emote == '\'')
					{
						strcat (desc, "\'");
						emote++;
						if (*emote == 's')
						{
							strcat (desc, "s");
						}
						else
						{
							emote--;
						}
					}
					else
					{
						emote--;
						emote--;
					}
					if (!tch)
						continue;
					if (tch == tar)
					{
						sprintf (buf, "%c", desc[strlen (desc) - 1]);
						if (desc[strlen (desc) - 1] == '\''
							|| desc[strlen (desc) - 2] == '\'')
						{
							strcat (output, "#5your#0");
							emote--;
						}
						else
							strcat (output, "#5you#0");
						target = tch;
						emote++;
					}
					else
					{
						sprintf (buf, "#5%s#0",  tch->char_short());
						strcat (output, buf);
						emote--;
						if (*emote == '\'')
							emote--;
					}
			}
			else
				sprintf (output + strlen (output), "#%c", *emote);
		}
		else
			sprintf (output + strlen (output), "%c", *emote);
		emote++;
	}
	if (target)
	{
		if (*output == '#')
			output[2] = toupper (output[2]);
		src->act(output, false, 0, target, TO_VICT | _ACT_FORMAT);
		target->magic_sent = 1;
	}
}

void
personalize_emote (CHAR_DATA * src, char *emote)
{
	char desc[MAX_STRING_LENGTH] = { '\0' };
	char copy[MAX_STRING_LENGTH] = { '\0' };
	CHAR_DATA *tch = NULL;

	sprintf (copy, "%s", emote);

	while (*emote)
	{
		*desc = '\0';
		if (*emote == '#')
		{
			emote++;
			if (*emote == '5')
			{
				emote++;
				while (*emote != '#')
				{
					sprintf (desc + strlen (desc), "%c", *emote);
					emote++;
				}
				tch = get_char_room_vis (src, desc);
				for (tch = src->room->people; tch; tch = tch->next_in_room)
					if (strcasecmp ( tch->char_short(), desc) == STR_MATCH)
						break;
				if (!tch)
					continue;
				if (!tch->magic_sent)
					personalize_string (src, tch, copy);
			}
		}
		emote++;
	}

	for (tch = src->room->people; tch; tch = tch->next_in_room)
	{
		if (tch == src)
			continue;
		if (tch->magic_sent)
			continue;

		if (IS_NPC(src))
		{
			tch->act(copy, false, 0, 0, TO_ROOM | TO_CHAR | _ACT_FORMAT);
			continue;
		}
		tch->act(copy, true, 0, 0, TO_CHAR | _ACT_FORMAT);
	}

	for (tch = src->room->people; tch; tch = tch->next_in_room)
	{
		if (tch->magic_sent)
			tch->magic_sent = 0;
	}
}

void
do_emote (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char copy[MAX_STRING_LENGTH] = { '\0' };
	char *result = NULL;
	char *p = '\0';

	while (isspace (*argument))
		argument++;


	if (strstr (argument, "\""))
	{
		ch->send_to_char
			("Conveying speech via emote is considered code abuse here. Please use SAY instead.\n");
		return;
	}

	if (!*argument)
		ch->send_to_char ("What would you like to emote?\n");
	else
	{
		p = copy;

		result = swap_xmote_target (ch, argument, 1);
		if (!result)
			return;

		sprintf (buf, "%s", result);

		personalize_emote (ch, buf); //adjusts for "you" if needed

		if (!strcmp(result, buf))
			ch->act(buf, false,  0, 0, TO_ROOM | TO_CHAR | _ACT_FORMAT);
		else
			ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);

	}
}


void
do_travel (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char *result_str = NULL;

	if (IS_NPC(ch))
	{
		ch->send_to_char ("I'm sorry, but travel strings for NPC don't work.\n");
		return;
	}
	while (isspace (*argument))
		argument++;

	if (!*argument)
	{
		if (ch->travel_str)
		{
			sprintf (buf, "Your current travel string: (#2%s#0)\n",
				ch->travel_str);
		}
		else
		{
			sprintf (buf, "You do not currently have a travel string set.\n");
		}
		ch->send_to_char (buf);
	}
	else
	{
		if (strcasecmp (argument, TRAVEL_RESET) == STR_MATCH)
		{
			ch->clear_travel();
			ch->send_to_char ("Your current travel string has been cleared.\n");
		}
		else
		{
			//filter string to process use of tokens such as * and ~
			result_str = swap_xmote_target (ch, argument, 3);

			if (!result_str)
			{
				return;
			}

			// We don't want the full top (period) that the code adds at the end of
			// this string so we remove it here
			result_str[strlen(result_str) - 1] = 0;

			sprintf (buf, "Your travel string has been set to: (%s)",
				result_str);
			ch->travel_str = duplicateString (result_str);
		}
		ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
	}
}

bool evaluate_emote_string (CHAR_DATA * ch, std::string * first_person, std::string third_person, std::string argument)
{
	OBJ_DATA * object;
	CHAR_DATA * tch = NULL;
	int i = 1;
	std::string output_string = "", key_string = "", error_string = "";

	if (argument[0] == '(')
	{
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			i = 1;
			output_string.clear();
			while (argument[i] != ')')
			{
				if (argument[i] == '\0')
				{
					ch->send_to_char ("Incorrect usage of emote string - please see HELP EMOTE\n");
					return false;
				}

				if (argument[i] == '*')
				{
					i++;
					key_string.clear();
					object = NULL;
					while (isalpha(argument[i]) || argument[i] == '-')
						key_string.push_back(argument[i++]);
					if (!(object = get_obj_in_list_vis(ch, key_string.c_str(), ch->right_hand)) &&
						!(object = get_obj_in_list_vis(ch, key_string.c_str(), ch->left_hand)) &&
						!(object = get_obj_in_list_vis(ch, key_string.c_str(), ch->room->contents)) &&
						!(object = get_obj_in_list_vis(ch, key_string.c_str(), ch->equip)) )
					{
						error_string = "You cannot find an object with the keyword [#2";
						error_string.append(key_string);
						error_string.append("]#0\n");
						ch->send_to_char (error_string.c_str());
						return false;
					}

					output_string.append("#2");
					output_string.append(obj_short_desc(object));
					output_string.append("#0");
					continue;
				}

				if (argument[i] == '~')
				{
					i++;
					key_string.clear();
					while (isalpha(argument[i]) || argument[i] == '-')
						key_string.push_back(argument[i++]);
					if (!get_char_room_vis(ch, key_string.c_str()))
					{
						error_string = "You cannot find a person with the keyword [#5";
						error_string.append(key_string);
						error_string.append("#0]\n");
						ch->send_to_char (error_string.c_str());
						return false;
					}

					if (get_char_room_vis(ch, key_string.c_str()) == tch)
					{
						output_string.append("#5you#0");
					}
					else
					{
						output_string.append("#5");
						output_string.append((get_char_room_vis(ch, key_string.c_str()))->char_short());
						output_string.append("#0");
					}
					continue;
				}

				output_string.push_back(argument[i++]);
			}

			if (tch == ch)
			{
				ch->send_to_char (first_person->c_str());
			}
			else
			{
				tch->send_to_char (third_person.c_str());
			}
			output_string.push_back('.');
			output_string.push_back('\n');
			tch->send_to_char (", ");
			tch->send_to_char (output_string.c_str());
			continue;
		}
	}

	else
	{

		ch->act((char*)(first_person->append(".").c_str()), false, 0, 0, TO_CHAR);
		ch->act((char*)(third_person.append(".").c_str()), false, 0, 0, TO_ROOM);
	}

	error_string.clear();
	first_person->clear();
	if (argument[0] == '(')
	{
		for ( i++; argument[i] != '\0'; i++)
		{
			error_string.push_back(argument[i]);
		}
		first_person->assign(error_string);
	}
	else
		first_person->assign(argument);

	return true;

}

void
do_omote (CHAR_DATA * ch, char *argument, int cmd)
{
	
	char buf[AVG_STRING_LENGTH * 4] = { '\0' };
	char arg1[MAX_STRING_LENGTH] = { '\0' };
	char * result = NULL;
	OBJ_DATA *obj = NULL;
	
	argument = one_argument (argument, arg1);
	
	if (!*arg1)
	{
		ch->send_to_char ("What would you like to omote on?\n");
		return;
	}
	
	if (!(obj = get_obj_in_list_vis (ch, arg1, ch->room->contents)))
	{
		ch->send_to_char ("You don't see that here.\n");
		return;
	}
	
	if (!CAN_WEAR (obj, ITEM_TAKE) && (!IS_SET (ch->flags, FLAG_ISADMIN)))
	{
		ch->send_to_char ("You can't omote on that.\n");
		return;
	}
	
	if (IS_SET (ch->room->room_flags, OOC))
	{
		ch->send_to_char ("You can't do this in an OOC area.\n");
		return;
	}
	
	if (!*argument)
	{
		ch->send_to_char ("What will you omote?\n");
		return;
	}
	
	if (!strncmp(argument,"normal",6))
	{
		clear_omote(obj);
		ch->send_to_char ("Omote cleared.\n");
		return;
	}
	
	result = swap_xmote_target (ch, argument, 3);
	if (!result)
		return;
	
	if (strlen (result) >= AVG_STRING_LENGTH * 4)
	{
		ch->send_to_char ("Your omote needs to be more succinct.\n");
		return;
	}
	
	sprintf (buf, "%s%s%s",
			 result,
			 (result[strlen (result) - 1] != '.') ? "." : "",
			 (obj->short_description[0] == '#') ? "#0" : "");
	
	/* free old string */
	clear_omote(obj);
	/* add new one */
	obj->omote_str = duplicateString (buf);
	sprintf (buf, "You omote: %s %s", obj->short_description, obj->omote_str);
	
	if (obj->short_description[0] == '#')
	{
		buf[13] = toupper (buf[13]);
	}
	
	ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);

}

