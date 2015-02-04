//////////////////////////////////////////////////////////////////////////////
//
/// comm.verbal.cpp : Verbal Communication Module
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


#define VOICE_RESET "normal"	/* Users use this to unset their voices */

	//special indents and 72 characters long
void
reformat_say_string (char *source, char **target, CHAR_DATA * to)
{
	int token_value = 0;
	int first_line = 1;
	int line_len = 0;
	char *s = '\0';
	char *r = '\0';
	char token[MAX_STRING_LENGTH] = { '\0' };
	char result[MAX_STRING_LENGTH] = { '\0' };

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
			if (line_len + strlen (token) > FORMAT_SAY)
			{
				strcat (result, "\n    ");
				line_len = 0;
			}

			strcat (result, token);
			strcat (result, " ");
			line_len += strlen (token) + 1;
		}
	}

	result[strlen (result) - 1] = '\0';

	if (result[strlen (result) - 1] != '.' &&
		result[strlen (result) - 1] != '!' &&
		result[strlen (result) - 1] != '?')
		result[strlen (result)] = '.';

	*target = duplicateString (result);
}


void
do_ooc (CHAR_DATA * ch, char *argument, int cmd)
{
	int i = 0;
	char buf[MAX_STRING_LENGTH] = { '\0' };

	if (IS_SET (ch->flags, FLAG_GUEST) && IS_SET (ch->room->room_flags, OOC))
	{
		do_say (ch, argument, 0);
		return;
	}


	for (i = 0; *(argument + i) == ' '; i++);

	if (!*(argument + i))
		ch->send_to_char("Surely you can think of more to say than that!\n");

	else
	{
		std::string formatted = argument;
		formatted[0] = toupper (formatted[0]);
		char *p = 0;
		char *s = duplicateString (formatted.c_str ());
		reformat_say_string (s, &p, 0);
		sprintf (buf, "$n says, out of character,\n   \"%s\"", p + i);
		ch->act(buf, false, 0, 0, TO_ROOM);
		sprintf (buf, "You say, out of character,\n   \"%s\"\n", p + i);
		ch->send_to_char(buf);
		free_mem (s);
		free_mem (p);
	}
}



	//argument is language name
	//"speak atliduk" to change ch->speaks to Atliduk
void
do_speak (CHAR_DATA * ch, char *argument, int cmd)
{
	char output[MAX_INPUT_LENGTH] = { '\0' };
	char* skill_name;
	std::map<std::string, SKILL_DATA*>::iterator skill_it;
	SKILL_DATA* tskill;

	if (!*argument)
	{
		sprintf(output, "You are currently speaking %s. Your possible choices are:\n", ch->speaks);
		for (skill_it = skill_data_map.begin(); skill_it != skill_data_map.end(); skill_it++)
		{
			tskill = skill_it->second;
			skill_name = strdup(tskill->skill_name.c_str());
			if ((tskill->spoken == 1)
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
			ch->send_to_char ("That isn't a language!\n");
			return;
		}
	}
	
	if (skill_it->second->spoken != 1)
	{
		ch->send_to_char ("That isn't a language!\n");
		return;
	}
		
	if ((ch->skill_map[argument] < 1)
		&& (ch->skill_map[CAP(argument)] < 1))
	{
		sprintf (output, "You are unfamiliar with %s.\n", CAP(argument));
		ch->send_to_char (output);
		return;
	}
	
	if (!str_cmp(ch->speaks, CAP(argument)))
	{
		sprintf (output, "You are already speaking %s.\n", ch->speaks);
		ch->send_to_char (output);
		return;
	}
	
	ch->speaks = strdup(CAP(argument));

	sprintf (output, "You begin speaking %s.\n", ch->speaks);
	ch->send_to_char (output);
	return;
	
}


void
do_mute (CHAR_DATA * ch, char *argument, int cmd)
{

	char buf[MAX_STRING_LENGTH] = { '\0' };
	AFFECTED_TYPE *af = NULL;

	if (ch->skill_map["Listen"] < 1)
	{
		sprintf (buf,
			"You don't have any skill to try and overhear conversations, so you are already muted by default.\n");
		ch->send_to_char (buf);
		return;
	}

	while (isspace (*argument))
		argument++;

	if (!*argument)
	{
		sprintf (buf, "You %s listening to others' conversations.\n",
			get_affect (ch, MUTE_EAVESDROP) ? "aren't" : "are");
		ch->send_to_char (buf);
		return;
	}

	if (strcasecmp (argument, "on") == STR_MATCH)
	{

		if (!get_affect (ch, MUTE_EAVESDROP))
		{
			af = new AFFECTED_TYPE;

			af->type = MUTE_EAVESDROP;
			af->a.listening.duration = -1;
			af->a.listening.on = 1;

			affect_to_char (ch, af);
		}
		sprintf (buf, "You will now not listen to others' conversations.\n");
		ch->send_to_char (buf);
	}
	else if (strcasecmp (argument, "off") == STR_MATCH)
	{

		if (get_affect (ch, MUTE_EAVESDROP))
		{
			remove_affect_type (ch, MUTE_EAVESDROP);
		}
		sprintf (buf, "You will now listen to others' conversations.\n");
		ch->send_to_char (buf);
	}
	else
	{
		sprintf (buf,
			"You can change your mute status by 'mute on' or 'mute off'.  To see what your mute status is use 'mute'\n");
		ch->send_to_char (buf);
	}
}


void
do_voice (CHAR_DATA * ch, char *argument, int cmd)
{

	char buf[MAX_STRING_LENGTH] = { '\0' };

	while (isspace (*argument))
		argument++;

	if (strchr (argument, '~'))
	{
		ch->send_to_char
			("Sorry, but you can't use tildae when setting a voice string.\n");
		return;
	}

	if (!*argument)
	{
		if (ch->voice_str)
		{
			sprintf (buf, "Your current voice string: (#2%s#0)\n",
				ch->voice_str);
		}
		else
			sprintf (buf, "You do not currently have a voice string set.\n");
		ch->send_to_char (buf);
	}
	else
	{
		if (strcasecmp (argument, VOICE_RESET) == STR_MATCH)
		{
			ch->clear_voice();
			sprintf (buf, "Your voice string has been cleared.");
		}
		else
		{
			sprintf (buf, "Your voice string has been set to: (#2%s#0)",
				argument);
			ch->voice_str = duplicateString (argument);
		}

		ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
	}
}

int
decipher_speaking (CHAR_DATA * ch, char *skill_name, int skill)
{
	int check = 0;

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

	skill_use (ch, skill_name, 0);

	if (ch->skill_map[skill_name] >= check)
		return 1;
	else
		return 0;
}

const char *
accent_desc (CHAR_DATA * ch, int skill)
{
	if (skill < 10)
		return "with very crude enunciation";
	else if (skill >= 10 && skill < 20)
		return "with crude enunciation";
	else if (skill >= 20 && skill < 30)
		return "with awkward enunciation";
	else if (skill >= 30 && skill < 40)
		return "with slightly awkward enunciation";
	else if (skill >= 40 && skill < 50)
		return "with a very faintly awkward enunciation";
	else
		return "with a faint accent";
}

void do_say (CHAR_DATA * ch, char *argument, int cmd)
{
	int talked_to_another = 0, i = 0, key_e = 0;
	CHAR_DATA *tch = NULL;
	CHAR_DATA *target = NULL;
	OBJ_DATA *obj = NULL;
	AFFECTED_TYPE *tongues = NULL;
	AFFECTED_TYPE *af_table = NULL;
	bool deciphered = false, allocd = false;
	char key[MAX_STRING_LENGTH] = { '\0' };

	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf4[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	char buf3[MAX_STRING_LENGTH] = { '\0' };
	char buf5[MAX_STRING_LENGTH] = { '\0' };
	char target_key[MAX_STRING_LENGTH] = { '\0' };
	char voice[MAX_STRING_LENGTH] = { '\0' };
	char *utters[] = { "say", "sing", "tell", "murmur", "wouldbewhisper" };
	bool bIsWithGroup = false;


	if (ch->room->terrain_type == SECT_UNDERWATER)
	{
		ch->send_to_char ("You can't do that underwater!\n");
		return;
	}

	

	/* We modify *argument, make sure we don't */
	/*  have a problem with const arguments   */
	//  strcpy (argbuf, argument);
	//  argument = argbuf;

	while (isspace (*argument))
		argument++;

	if (!*argument)
	{
		ch->send_to_char ("What would you like to say?\n");
		return;
	}

	if (cmd == 5) //group
	{
		cmd = 3;
		bIsWithGroup = true;
	}

	if (cmd == 2) //tell
	{
		*target_key = '\0';
		argument = one_argument (argument, target_key);
	}

	if (ch->voice_str && ch->voice_str[0])
	{
		strcpy (voice, ch->voice_str);
	}

	//Get the intro phrase and the message
	if (*argument == '(')
	{
		sprintf (buf, "%s", argument);
		i = 1;

		while (isspace (buf[i])) {
			i++;
		}

		if (buf[i] == ')') {
			ch->send_to_char ("What did you wish to say?\n");
			return;
		}

		while (buf[i] != ')') {
			if (buf[i] == '\0')
			{
				ch->send_to_char ("What did you wish to say?\n");
				return;
			}
			if (buf[i] == '*')
			{
				i++;
				while (isalpha (buf[i]))
					key[key_e++] = buf[i++];
				key[key_e] = '\0';
				key_e = 0;

				if (!get_obj_in_list_vis (ch, key, ch->room->contents) &&
					!get_obj_in_list_vis (ch, key, ch->right_hand) &&
					!get_obj_in_list_vis (ch, key, ch->left_hand) &&
					!get_obj_in_list_vis (ch, key, ch->equip))
				{
					sprintf (buf, "I don't see %s here.\n", key);
					ch->send_to_char (buf);
					return;
				}

				obj = get_obj_in_list_vis (ch, key, ch->right_hand);
				if (!obj)
					obj = get_obj_in_list_vis (ch, key, ch->left_hand);
				if (!obj)
					obj = get_obj_in_list_vis (ch, key, ch->room->contents);
				if (!obj)
					obj = get_obj_in_list_vis (ch, key, ch->equip);
				sprintf (buf2 + strlen (buf2), "#2%s#0", obj_short_desc (obj));
				*key = '\0';
				continue;
			}
			if (buf[i] == '~')
			{
				i++;
				while (isalpha (buf[i]))
					key[key_e++] = buf[i++];
				key[key_e] = '\0';
				key_e = 0;

				if (!get_char_room_vis (ch, key))
				{
					sprintf (buf, "I don't see %s here.\n", key);
					ch->send_to_char (buf);
					return;
				}

				sprintf (buf2 + strlen (buf2), "#5%s#0",
					(get_char_room_vis (ch, key))->char_short());
				*key = '\0';
				continue;
			}
			sprintf (buf2 + strlen (buf2), "%c", buf[i]);
			i++;
		}
		strcpy (voice, buf2);
		while (*argument != ')') {
			argument++;
			if (*argument == '\0') {
				ch->send_to_char ("What did you wish to say?\n");
				return;
			}
		}

		if (*(++argument) == '\0') {
			ch->send_to_char ("What did you wish to say?\n");
			return;
		}
		else if (*(++argument) == '\0') {
			ch->send_to_char ("What did you wish to say?\n");
			return;
		}

		i = 0;
		*buf = '\0';
		if (cmd == 2 && *target_key)
			sprintf (buf, "%s %s", target_key, argument);
		else
			sprintf (buf, "%s", argument);
		*argument = '\0';
		argument = buf;
		if (!*argument)
		{
			ch->send_to_char ("What did you wish to say?\n");
			return;
		}
	}
	else if (cmd == 2 && *target_key)
	{
		sprintf (buf, "%s %s", target_key, argument);
		sprintf (argument, "%s", buf);
	}

	if (!*argument)
	{
		ch->send_to_char ("What did you wish to say?\n");
		return;
	}

	if (cmd == 2)
	{				/* Tell */

		argument = one_argument (argument, buf);

		reformat_say_string (argument, &argument, 0);

		if (!*argument)
		{
			ch->send_to_char ("What did you wish to tell?\n");
			return;
		}

		if (!(target = get_char_room_vis (ch, buf)))
		{
			ch->send_to_char ("Tell who?\n");
			return;
		}

		if (target == ch)
		{
			ch->send_to_char ("You want to tell yourself?\n");
			return;
		}

		while (isspace (*argument))
			argument++;
	}
	else
		reformat_say_string (argument, &argument, 0);

	if (!*argument)
	{
		if (cmd == 1)
			ch->send_to_char ("What are the words to the song?\n");
		else if (cmd == 2)
			ch->send_to_char ("What would you like to tell?\n");
		else
			ch->send_to_char ("What would you like to say?\n");

		return;
	}


	if (cmd == 3)
	{
		if ((af_table = get_affect (ch, MAGIC_SIT_TABLE)) != NULL)
		{
			bIsWithGroup = false;
		}
	}
	if (ch->skill_map[ch->speaks] < 1)
	{
		ch->send_to_char("You can't even make a guess at the language you want to speak.\n");
		return;
	}

	sprintf (buf4, "%s", argument);	/* The intended message, sent to the player. */
	sprintf (buf5, "%s",argument);
	sprintf (buf2, "%s",argument);

	if (cmd == 0)
	{
		if (buf4[strlen (buf4) - 1] == '?')
		{
			utters[cmd] = duplicateString ("ask");
			allocd = true;
		}
		else if (buf4[strlen (buf4) - 1] == '!')
		{
			utters[cmd] = duplicateString ("exclaim");
			allocd = true;
		}
	}
	else if (cmd == 2)
	{
		if (buf4[strlen (buf4) - 1] == '?')
		{
			utters[cmd] = duplicateString ("ask");
			allocd = true;
		}
		else if (buf4[strlen (buf4) - 1] == '!')
		{
			utters[cmd] = duplicateString ("emphatically tell");
			allocd = true;
		}
	}

	skill_use (ch, ch->speaks, 0);

	// Now decided what the target person hears
	ROOM_DATA* r = vtor(ch->in_room);
	if (r!=NULL)
	{
		ch->room=r;
	}
	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{

		if (tch == ch)		/* Don't say it to ourselves */
			continue;

		if (!tch->desc)	/* NPC don't hear anything */
			continue;

		if ((af_table && !is_at_table (tch, af_table->a.table.obj))
			|| (bIsWithGroup
			&& (!are_grouped (ch, tch)
			|| get_affect (tch, MAGIC_SIT_TABLE))))
		{

			/* If the guy is muting, punt */
			if (get_affect (tch, MUTE_EAVESDROP))
				continue;

			sprintf (buf2, "%s",argument);

			if (!whisper_it (tch, "Listen", buf2, buf2))
				continue;

			if ((tch->skill_map[ch->speaks]
			&& decipher_speaking (tch, ch->speaks, ch->skill_map[ch->speaks]))
				|| IS_SET (ch->room->room_flags, OOC))
			{
				if (!IS_SET (ch->room->room_flags, OOC))
				{
					sprintf (buf, "You overhear $N say in %s,", ch->speaks);
					if (tch->skill_map[ch->speaks] >= 50
						&& ch->skill_map[ch->speaks] < 50)
						sprintf (buf + strlen (buf), " %s,",
						accent_desc (ch, ch->skill_map[ch->speaks]));
				}
				else
				{
					sprintf (buf, "You overhear $N say,");
				}
				if (voice && *voice)
					sprintf (buf + strlen (buf), " %s,", voice);
				deciphered = true;
			}
			else
			{
				if (tch->skill_map[ch->speaks])
					sprintf (buf, "You overhear $N say something in %s,",
					ch->speaks);
				else
					sprintf (buf, "You overhear $N say something,");
				if (tch->skill_map[ch->speaks] >= 50
					&& ch->skill_map[ch->speaks] < 50)
					sprintf (buf + strlen (buf), " %s,",
					accent_desc (ch, ch->skill_map[ch->speaks]));
				if (voice && *voice)
					sprintf (buf + strlen (buf), " %s,", voice);
				sprintf (buf + strlen (buf),
					" but you are unable to decipher %s words.",
					HSHR (ch));
				deciphered = false;
			}

			tch->act(buf, false, 0, ch, TO_CHAR | _ACT_FORMAT);

			if (tch->desc && deciphered)
			{
				*buf2 = toupper (*buf2);
				sprintf (buf, "   \"%s\"\n", buf2);
				tch->send_to_char (buf);
			}

			continue;
		}

		if (tch->desc)
			talked_to_another = 1;

		if ((tch->get_trust() > 1) 
			&& !IS_NPC (tch) 
			&& GET_FLAG (tch, FLAG_SEE_NAME))
			sprintf (buf3, " (%s)", ch->name);
		else
			*buf3 = '\0';

		if (cmd == 0 || cmd == 1)
		{
			if (!IS_SET (ch->room->room_flags, OOC)
				&& decipher_speaking (tch, ch->speaks, ch->skill_map[ch->speaks]))
			{
				sprintf (buf, "$N%s %ss in %s,", buf3, utters[cmd],
					(tch->skill_map[ch->speaks]
				|| tongues) ? ch->speaks : "an unknown tongue");
				if (tch->skill_map[ch->speaks] >= 50
					&& ch->skill_map[ch->speaks] < 50)
					sprintf (buf + strlen (buf), " %s,",
					accent_desc (ch, ch->skill_map[ch->speaks]));
				deciphered = true;
				if (voice && *voice)
					sprintf (buf + strlen (buf), " %s,", voice);
			}
			else if (!IS_SET (ch->room->room_flags, OOC))
			{
				sprintf (buf, "$N%s %ss something in %s,", buf3, utters[cmd],
					(tch->skill_map[ch->speaks]
				|| tongues) ? ch->speaks : "an unknown tongue");
				if (voice && *voice)
					sprintf (buf + strlen (buf), " %s,", voice);
				sprintf (buf + strlen (buf),
					" but you are unable to decipher %s words.",
					HSHR (ch));
				deciphered = false;
			}
			else if (IS_SET (ch->room->room_flags, OOC))
			{
				sprintf (buf, "$N%s %ss,", buf3, utters[cmd]);
				if (voice && *voice)
					sprintf (buf + strlen (buf), " %s,", voice);
				deciphered = true;
			}
		}

		else if (cmd == 2)
		{
			if (tch == target)
			{
				if (!IS_SET (ch->room->room_flags, OOC)
					&& decipher_speaking (tch, ch->speaks,
					ch->skill_map[ch->speaks]))
				{
					sprintf (buf, "$N%s %ss you in %s,", buf3,
						utters[cmd], (tch->skill_map[ch->speaks] || tongues) ?
						ch->speaks : "an unknown tongue");
					deciphered = true;
					if (tch->skill_map[ch->speaks] >= 50
						&& ch->skill_map[ch->speaks] < 50)
						sprintf (buf + strlen (buf), " %s,",
						accent_desc (ch, ch->skill_map[ch->speaks]));
					if (voice && *voice)
						sprintf (buf + strlen (buf), " %s,", voice);
				}
				else if (!IS_SET (ch->room->room_flags, OOC))
				{
					sprintf (buf, "$N%s %ss you something in %s,", buf3,
						utters[cmd], (tch->skill_map[ch->speaks] || tongues) ?
						ch->speaks : "an unknown tongue");
					if (voice && *voice)
						sprintf (buf + strlen (buf), " %s,", voice);
					sprintf (buf + strlen (buf),
						" but you are unable to decipher %s words.",
						HSHR (ch));
					deciphered = false;
				}
				else
				{
					sprintf (buf, "$N%s %ss you,", buf3, utters[cmd]);
					deciphered = true;
				}
			}
			else
			{
				if (!IS_SET (ch->room->room_flags, OOC)
					&& decipher_speaking (tch, ch->speaks,
					ch->skill_map[ch->speaks]))
				{
					sprintf (buf, "$N%s %ss %s in %s,", buf3,
						utters[cmd],  target->char_short(),
						(tch->skill_map[ch->speaks]
					|| tongues) ? ch->speaks :
					"an unknown tongue");
					deciphered = true;
					if (tch->skill_map[ch->speaks] >= 50
						&& ch->skill_map[ch->speaks] < 50)
						sprintf (buf + strlen (buf), " %s,",
						accent_desc (ch, ch->skill_map[ch->speaks]));
					if (voice && *voice)
						sprintf (buf + strlen (buf), " %s,", voice);
				}
				else if (!IS_SET (ch->room->room_flags, OOC))
				{
					sprintf (buf, "$N%s %ss %s something in %s,", buf3,
						utters[cmd],  target->char_short(),
						(tch->skill_map[ch->speaks] || tongues) ?
						ch->speaks : "an unknown tongue");
					if (voice && *voice)
						sprintf (buf + strlen (buf), " %s,", voice);
					sprintf (buf + strlen (buf),
						" but you are unable to decipher %s words.",
						HSHR (ch));
					deciphered = false;
				}
				else
				{
					sprintf (buf, "$N%s %ss %s,", buf3, utters[cmd],
						 target->char_short());
					if (voice && *voice)
						sprintf (buf + strlen (buf), " %s,", voice);
					deciphered = true;
				}
			}
		}

		else if (cmd == 3)
		{
			if (!IS_SET (ch->room->room_flags, OOC)
				&& decipher_speaking (tch, ch->speaks, ch->skill_map[ch->speaks]))
			{
				sprintf (buf, "$N%s %ss in %s,", buf3,
					utters[cmd],
					(tch->skill_map[ch->speaks] || tongues) ?
					ch->speaks : "an unknown tongue");
				deciphered = true;
				if (tch->skill_map[ch->speaks] >= 50
					&& ch->skill_map[ch->speaks] < 50)
					sprintf (buf + strlen (buf), " %s,",
					accent_desc (ch, ch->skill_map[ch->speaks]));
				if (voice && *voice)
					sprintf (buf + strlen (buf), " %s,", voice);
			}
			else if (!IS_SET (ch->room->room_flags, OOC))
			{
				sprintf (buf, "$N%s %ss something in %s,", buf3,
					utters[cmd],
					(tch->skill_map[ch->speaks] || tongues) ?
					ch->speaks : "an unknown tongue");
				if (voice && *voice)
					sprintf (buf + strlen (buf), " %s,", voice);
				sprintf (buf + strlen (buf),
					" but you are unable to decipher %s words.",
					HSHR (ch));
				deciphered = false;
			}
			else
			{
				sprintf (buf, "$N%s %ss,", buf3, utters[cmd]);
				deciphered = true;
			}
		}

		tch->act(buf, false,  0, ch, TO_CHAR | _ACT_FORMAT);

		if (tch->desc && deciphered)
		{
			*buf4 = toupper (*buf4);
			sprintf (buf, "   \"%s\"\n", buf4);
			tch->send_to_char (buf);
		}

		sprintf (argument, "%s",buf5);
		deciphered = false;
	}

	*buf4 = toupper (*buf4);

	if (cmd == 2)
	{
		if (voice && *voice)
		{
			if (!IS_SET (ch->room->room_flags, OOC))
				sprintf (buf, "You %s #5%s#0 in %s, %s,\n   \"%s\"\n",
				utters[cmd],  target->char_short(), ch->speaks,
				voice, buf4);
			else
				sprintf (buf, "You %s #5%s#0, %s,\n   \"%s\"\n", utters[cmd],
				 target->char_short(), voice, buf4);
		}
		else
		{
			if (!IS_SET (ch->room->room_flags, OOC))
				sprintf (buf, "You %s #5%s#0 in %s,\n   \"%s\"\n",
				utters[cmd],  target->char_short(), ch->speaks,
				buf4);
			else
				sprintf (buf, "You %s #5%s#0,\n   \"%s\"\n", utters[cmd],
				 target->char_short(), buf4);
		}
	}
	else
	{
		if (voice && *voice)
		{
			if (!IS_SET (ch->room->room_flags, OOC))
				sprintf (buf, "You %s in %s, %s,\n   \"%s\"\n",
				utters[cmd], ch->speaks, voice, buf4);
			else
				sprintf (buf, "You %s, %s,\n   \"%s\"\n", utters[cmd], voice,
				buf4);
		}
		else
		{
			if (!IS_SET (ch->room->room_flags, OOC))
				sprintf (buf, "You %s in %s,\n   \"%s\"\n", utters[cmd],
				ch->speaks, buf4);
			else
				sprintf (buf, "You %s,\n   \"%s\"\n", utters[cmd], buf4);
		}
	}

	ch->send_to_char (buf);
	
	if (cmd == 0)
	{
		if (allocd)
			free_mem (utters[cmd]); // char[]
	}
}

void
do_sing (CHAR_DATA * ch, char *argument, int cmd)
{
	do_say (ch, argument, 1);	/* 1 = sing */
}


void
do_tell (CHAR_DATA * ch, char *argument, int cmd)
{
	do_say (ch, argument, 2);
}


int
whisper_it (CHAR_DATA * ch, char* skill_name, char *source, char *target)
{
	int missed = 0;
	int got_one = 0;
	int bonus = 0;
	char *in = '\0';
	char *out = '\0';
	char buf[MAX_STRING_LENGTH] = { '\0' };

	if (ch->skill_map[skill_name] < 1)
		return 0;

	skill_use (ch, skill_name, 0);

	if (!str_cmp(skill_name, "Telepathy"))
		bonus = 20;

	in = source;
	out = buf;
	*out = '\0';

	while (*in)
	{

		while (*in == ' ')
		{
			in++;
			*out = ' ';
			out++;
		}

		*out = '\0';

		if (ch->skill_map[skill_name] + bonus < number (1, SKILL_CEILING))
		{
			if (!missed)
			{
				strcat (out, " . . .");
				out += strlen (out);
			}

			missed = 1;

			while (*in && *in != ' ')
				in++;
		}

		else
		{
			while (*in && *in != ' ')
			{
				*out = *in;
				out++;
				in++;
			}

			got_one = 1;
			missed = 0;
		}

		*out = '\0';
	}

	strcpy (target, buf);

	return got_one;
}

void
do_whisper (CHAR_DATA * ch, char *argument, int cmd)
{
	int heard_something = 0, key_e = 0;
	CHAR_DATA *vict = NULL;
	CHAR_DATA *tch = NULL;
	AFFECTED_TYPE *tongues = NULL;
	char name[MAX_STRING_LENGTH] = { '\0' };
	char message[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	char buf3[MAX_STRING_LENGTH] = { '\0' };
	char buf4[MAX_STRING_LENGTH] = { '\0' };
	char voice[MAX_STRING_LENGTH] = { '\0' };
	int i = 1;
	OBJ_DATA *obj = NULL;
	char key[MAX_STRING_LENGTH] = { '\0' };


	if (ch->room->terrain_type == SECT_UNDERWATER)
	{
		ch->send_to_char ("You can't do that underwater!\n");
		return;
	}

	half_chop (argument, name, message);

	*message = toupper (*message);

	
	if (!*name || !*message)
	{
		ch->send_to_char ("Who do you want to whisper to.. and what?\n");
		return;
	}

	if (!(vict = get_char_room_vis (ch, name)) )
	{
		ch->send_to_char ("No-one by that name here.\n");
		return;
	}
	
	else if (vict == ch)
	{
		ch->act("$n whispers quietly to $mself.", false, 0, 0, TO_ROOM);
		ch->send_to_char ("You whisper to yourself.\n");
		return;
	}


	if ((ch->skill_map[ch->speaks] < 1)
		&& !IS_SET (ch->room->room_flags, OOC))
	{
		ch->send_to_char ("You don't know the language you want to "
			"whisper\n");
		return;
	}

	//set voice string to character voice string if set
	if (ch->voice_str && ch->voice_str[0])
	{
		strcpy (voice, ch->voice_str);
	}

	/* Process bracketed emote if present */

	//check for a bracket at the start of the message
	if (*message == '(')
	{
		// To begin with, set the voice string to be the entire message
		// we will then parse this until we find the end of the emote and
		// cut the remaining characters
		strcpy (voice, message);

		//result = swap_xmote_target (ch, argument, 1)
		*buf = '\0';
		sprintf (buf, "%s", voice);
		i = 1;
		*buf2 = '\0';
		while (buf[i] != ')')
		{
			if (buf[i] == '\0')
			{
				ch->send_to_char ("What did you wish to say?\n");
				return;
			}
			if (buf[i] == '*')
			{
				i++;
				while (isalpha (buf[i]))
					key[key_e++] = buf[i++];
				key[key_e] = '\0';
				key_e = 0;

				if (!get_obj_in_list_vis (ch, key, ch->room->contents) &&
					!get_obj_in_list_vis (ch, key, ch->right_hand) &&
					!get_obj_in_list_vis (ch, key, ch->left_hand) &&
					!get_obj_in_list_vis (ch, key, ch->equip))
				{
					sprintf (buf, "I don't see %s here.\n", key);
					ch->send_to_char (buf);
					return;
				}

				obj = get_obj_in_list_vis (ch, key, ch->right_hand);
				if (!obj)
					obj = get_obj_in_list_vis (ch, key, ch->left_hand);
				if (!obj)
					obj = get_obj_in_list_vis (ch, key, ch->room->contents);
				if (!obj)
					obj = get_obj_in_list_vis (ch, key, ch->equip);
				sprintf (buf2 + strlen (buf2), "#2%s#0", obj_short_desc (obj));
				*key = '\0';
				continue;
			}
			if (buf[i] == '~')
			{
				i++;
				while (isalpha (buf[i]))
					key[key_e++] = buf[i++];
				key[key_e] = '\0';
				key_e = 0;

				if (!get_char_room_vis (ch, key))
				{
					sprintf (buf, "I don't see %s here.\n", key);
					ch->send_to_char (buf);
					return;
				}

				sprintf (buf2 + strlen (buf2), "#5%s#0",
					(get_char_room_vis (ch, key)->char_short()));
				*key = '\0';
				continue;
			}
			sprintf (buf2 + strlen (buf2), "%c", buf[i]);
			i++;


		}

		//copy the string to the voice
		strcpy (voice, buf2);

		//incriment up to and then past the ending bracket
		i = i+2;

		//strip the emote from the start of the message
		strcpy (message, message + i);

		*message = toupper (*message);

	}
	/* End processing of emote */

	char *p = '\0';
	reformat_say_string (message, &p, 0);

	if (voice && *voice) //add voice string if one is set (either explicitly or via bracketed emote)
	{
		if (!IS_SET (ch->room->room_flags, OOC))
			sprintf (buf, "You whisper to $N in %s, %s,\n   \"%s\"",
			ch->speaks, voice, p);
		else
			sprintf (buf, "You whisper to $N, %s,\n   \"%s\"", voice, p);
	}
	else
	{
		if (!IS_SET (ch->room->room_flags, OOC))
			sprintf (buf, "You whisper to $N in %s,\n   \"%s\"",
			ch->speaks, p);
		else
			sprintf (buf, "You whisper to $N,\n   \"%s\"", p);
	}

	ch->act(buf, true, 0, vict, TO_CHAR);

	sprintf (buf4, "%s", p);
	*buf4 = toupper (*buf4);
	sprintf (buf3, "%s", p);
	*buf3 = toupper (*buf3);

	skill_use (ch, ch->speaks, 0);

	for (tch = vtor (ch->in_room)->people; tch; tch = tch->next_in_room)
	{

		if (tch == ch)		/* Don't say it to ourselves */
			continue;

		if ( tch != vict && cmd == 83 ) /* Coded shopkeep whisper - skip to reduce spam */
			continue;

		sprintf (buf2, "%s",p);

		heard_something = 1;

		if (tch != vict)
		{
			if (get_affect (tch, MUTE_EAVESDROP))
				continue;
			heard_something = whisper_it (tch, "Listen", buf2, buf2);
		}
		if (!heard_something)
		{
			if (voice && *voice) //add voice string if one is set (either explicitly or via bracketed emote)
			{
				sprintf (buf, "$n whispers something to $3, %s, but you can't quite make out the words.", voice);
			}
			else
			{
				sprintf (buf, "$n whispers something to $3, but you can't quite make out the words.");
			}
			ch->act(buf, true, (OBJ_DATA *) vict, tch, TO_VICT | _ACT_FORMAT);
			continue;
		}

		if (tch == vict)//send message to target of whisper
		{
			if (voice && *voice) //add voice string if one is set (either explicitly or via bracketed emote)
			{
				if (!IS_SET (ch->room->room_flags, OOC)
					&& decipher_speaking (tch, ch->speaks, ch->skill_map[ch->speaks]))
				{
					sprintf (buf, "$3 whispers to you in %s, %s,",
						(tch->skill_map[ch->speaks] || tongues) ?
						ch->speaks : "an unknown tongue", voice);
					if (tch->skill_map[ch->speaks] >= 50
						&& ch->skill_map[ch->speaks] < 50)
						sprintf (buf + strlen (buf), " %s,",
						accent_desc (ch, ch->skill_map[ch->speaks]));
					tch->act(buf, false,  (OBJ_DATA *) ch, 0,
						TO_CHAR | _ACT_FORMAT);
					sprintf (buf, "   \"%s\"", buf3);
					tch->act(buf, false,  0, 0, TO_CHAR);
				}
				else if (!IS_SET (ch->room->room_flags, OOC))
				{
					sprintf (buf,
						"$3 whispers something to you in %s, %s, but you cannot decipher %s words.",
						(tch->skill_map[ch->speaks]
					|| tongues) ? ch->speaks : "an unknown tongue", voice,
						HSHR (ch));
					tch->act(buf, false,  (OBJ_DATA *) ch, 0,
						TO_CHAR | _ACT_FORMAT);
				}
				else
				{
					sprintf (buf, "$3 whispers to you, %s,\n   \"%s\"", voice, buf4);
					tch->act(buf, false,  (OBJ_DATA *) ch, 0, TO_CHAR);
				}
				
			}
			else
			{
				if (!IS_SET (ch->room->room_flags, OOC)
					&& decipher_speaking (tch, ch->speaks, ch->skill_map[ch->speaks]))
				{
					sprintf (buf, "$3 whispers to you in %s,",
						(tch->skill_map[ch->speaks] || tongues) ?
						ch->speaks : "an unknown tongue");
					if (tch->skill_map[ch->speaks] >= 50
						&& ch->skill_map[ch->speaks] < 50)
						sprintf (buf + strlen (buf), " %s,",
						accent_desc (ch, ch->skill_map[ch->speaks]));
					tch->act(buf, false,  (OBJ_DATA *) ch, 0,
						TO_CHAR | _ACT_FORMAT);
					sprintf (buf, "   \"%s\"", buf3);
					tch->act(buf, false,  0, 0, TO_CHAR);
				}
				else if (!IS_SET (ch->room->room_flags, OOC))
				{
					sprintf (buf,
						"$3 whispers something to you in %s, but you cannot decipher %s words.",
						(tch->skill_map[ch->speaks]
					|| tongues) ? ch->speaks : "an unknown tongue",
						HSHR (ch));
					tch->act(buf, false,  (OBJ_DATA *) ch, 0,
						TO_CHAR | _ACT_FORMAT);
				}
				else
				{
					sprintf (buf, "$3 whispers to you,\n   \"%s\"", buf4);
					tch->act(buf, false,  (OBJ_DATA *) ch, 0, TO_CHAR);
				}
				
			}
		}

		else //overhear listeners who passed the listen check
		{
			if (voice && *voice) //add voice string if one is set (either explicitly or via bracketed emote)
			{
				if (!IS_SET (ch->room->room_flags, OOC)
					&& decipher_speaking (tch, ch->speaks, ch->skill_map[ch->speaks]))
				{
					sprintf (buf, "You overhear $3 whispering in %s to $N, %s,",
						(tch->skill_map[ch->speaks] || tongues) ?
						ch->speaks : "an unknown tongue", voice);
					if (tch->skill_map[ch->speaks] >= 50
						&& ch->skill_map[ch->speaks] < 50)
						sprintf (buf + strlen (buf), " %s,",
						accent_desc (ch, ch->skill_map[ch->speaks]));
					tch->act(buf, false,  (OBJ_DATA *) ch, vict,
						TO_CHAR | _ACT_FORMAT);
					sprintf (buf, "   \"%s\"", buf2);
					tch->act(buf, false,  0, 0, TO_CHAR);
				}
				else if (!IS_SET (ch->room->room_flags, OOC))
				{
					sprintf (buf,
						"You overhear $3 whispering something in %s to $N, %s, but you cannot decipher %s words.",
						(tch->skill_map[ch->speaks]
					|| tongues) ? ch->speaks : "an unknown tongue", voice,
						HSHR (ch));
					tch->act(buf, false,  (OBJ_DATA *) ch, vict,
						TO_CHAR | _ACT_FORMAT);
				}
				else
				{
					sprintf (buf, "You overhear $3 whisper to $N, %s,\n   \"%s\"", voice,
						buf2);
					tch->act(buf, false,  (OBJ_DATA *) ch, vict, TO_CHAR);
				}
			}
			else
			{
				if (!IS_SET (ch->room->room_flags, OOC)
					&& decipher_speaking (tch, ch->speaks, ch->skill_map[ch->speaks]))
				{
					sprintf (buf, "You overhear $3 whispering in %s to $N,",
						(tch->skill_map[ch->speaks] || tongues) ?
						ch->speaks : "an unknown tongue");
					if (tch->skill_map[ch->speaks] >= 50
						&& ch->skill_map[ch->speaks] < 50)
						sprintf (buf + strlen (buf), " %s,",
						accent_desc (ch, ch->skill_map[ch->speaks]));
					tch->act(buf, false,  (OBJ_DATA *) ch, vict,
						TO_CHAR | _ACT_FORMAT);
					sprintf (buf, "   \"%s\"", buf2);
					tch->act(buf, false,  0, 0, TO_CHAR);
				}
				else if (!IS_SET (ch->room->room_flags, OOC))
				{
					sprintf (buf,
						"You overhear $3 whispering something in %s to $N, but you cannot decipher %s words.",
						(tch->skill_map[ch->speaks]
					|| tongues) ? ch->speaks : "an unknown tongue",
						HSHR (ch));
					tch->act(buf, false,  (OBJ_DATA *) ch, vict,
						TO_CHAR | _ACT_FORMAT);
				}
				else
				{
					sprintf (buf, "You overhear $3 whisper to $N,\n   \"%s\"",
						buf2);
					tch->act(buf, false,  (OBJ_DATA *) ch, vict, TO_CHAR);
				}
			}
		}

		sprintf (p, "%s",buf3);
	}

	free_mem (p); // char*

}


void
do_ask (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *vict = NULL;
	char name[MAX_STRING_LENGTH] = { '\0' };
	char message[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };

	half_chop (argument, name, message);

	if (!*name || !*message)
		ch->send_to_char ("Who do you want to ask something.. and what??\n");
	else if (!(vict = get_char_room_vis (ch, name)))
		ch->send_to_char ("No-one by that name here.\n");
	else if (vict == ch)
	{
		ch->act("$n quietly asks $mself a question.", false, 0, 0, TO_ROOM);
		ch->send_to_char ("You think about it for a while...\n");
	}
	else
	{
		sprintf (buf, "$n asks you '%s'", message);
		ch->act(buf, false,  0, vict, TO_VICT);
		ch->send_to_char ("Ok.\n");
		ch->act("$n asks $N a question.", false, 0, vict, TO_NOTVICT);
	}
}

void
do_talk (CHAR_DATA * ch, char *argument, int cmd)
{
	if (is_at_table (ch, NULL))
	{
		do_say (ch, argument, 3);
	}
	else if (is_with_group (ch))
	{
		do_say (ch, argument, 5);
	}
	else
	{
		do_say (ch, argument, 0);
	}
}


void
do_shout (CHAR_DATA *ch, char * arg, int cmd)
{
	std::string argument;
	CHAR_DATA *tch = NULL;
	AFFECTED_TYPE *tongues = NULL;
	std::map<int, ROOM_DATA*>::iterator room_iterator;
	int door = 0;
	

	argument = arg;
	
	if (ch->room->terrain_type == SECT_UNDERWATER)
	{
		ch->send_to_char ("You can't do that underwater!\n");
		return;
	}

	
	if (argument.empty())
	{
		ch->send_to_char ("What would you like to shout?\n");
		return;
	}

	while (isspace(argument[0]))
		argument.erase(0,1);

	argument[0] = toupper(argument[0]);
	char *buf;
	char *orig = duplicateString((char *) argument.c_str());
	reformat_say_string (orig, &buf, 0);
	free_mem(orig);
	argument = buf;

	std::string toplayer = argument;

	for (tch = vtor (ch->in_room)->people; tch; tch = tch->next_in_room)
	{
		std::string output;
		if (tch == ch)
		{
			if (!IS_SET(ch->room->room_flags, OOC))
			{
				output = MAKE_STRING("You shout in ") + MAKE_STRING(ch->speaks) + ",";
			}
			else
			{
				output = "You shout, ";
			}
			output += MAKE_STRING("\n    \"") + toplayer + "\"\n";
			ch->send_to_char(output.c_str());
			continue;
		}

		if (!tch->desc)
		{
			continue;
		}

		output = MAKE_STRING("#5") + MAKE_STRING(ch->char_short()) + MAKE_STRING("#0 ");
		output[2] = toupper(output[2]);
		if ((tch->get_trust() > 1) && GET_FLAG(tch, FLAG_SEE_NAME))
		{
			output += MAKE_STRING("(") + MAKE_STRING(ch->name) + ") ";
		}
		output += "shouts, ";
		if (!IS_SET(ch->room->room_flags, OOC))
		{
			if (!tch->skill_map[ch->speaks] && !tongues)
			{
				output += MAKE_STRING("in an unknown tongue,");
			}
			else
			{
				output += MAKE_STRING("in ") +  MAKE_STRING(ch->speaks) + MAKE_STRING(", ");
			}
		}

		if (tch->skill_map[ch->speaks] >= 50 && ch->skill_map[ch->speaks] < 50)
		{
			output += MAKE_STRING(" ") + MAKE_STRING(accent_desc(ch, ch->skill_map[ch->speaks])) + ",";
		}

		if (!decipher_speaking (tch, ch->speaks, ch->skill_map[ch->speaks]) && !IS_SET (ch->room->room_flags, OOC))
		{
			output += "something that you fail to decipher.\n";
		}
		else
		{
			output += MAKE_STRING("\n    \"") + toplayer + "\"\n";
		}
		tch->send_to_char(output.c_str());
	}

	
	for (door = 0; door <= LAST_DIR; door++)
	{
		ROOM_EXIT_DATA *texit;
		texit = is_exit(ch, door);
		if (texit && (texit->to_room != -1))
		{
			for (tch = vtor(is_exit(ch, door)->to_room)->people; tch; tch = tch->next_in_room)
			{
				if (!tch->desc)
				{
					continue;
				}
				std::string output, sex_string;
				if (ch->sex == SEX_MALE)
				{
					sex_string = "male ";
				}
				else if (ch->sex == SEX_FEMALE)
				{
					sex_string = "female ";
				}
				else
				{
					sex_string = "";
				}
				output = MAKE_STRING("You hear a ") + sex_string + MAKE_STRING("voice ");
				if ((tch->get_trust() > 1) && GET_FLAG(tch, FLAG_SEE_NAME))
				{
					output += MAKE_STRING("(") + MAKE_STRING(ch->name) + ") ";
				}
				if (!IS_SET(ch->room->room_flags, OOC))
				{
					if (!tch->skill_map[ch->speaks])
					{
						output += MAKE_STRING("shout in an unknown tongue from ") + opposite_dirs[door];
					}
					else
					{
						output += MAKE_STRING("shout in ") + MAKE_STRING(ch->speaks) + MAKE_STRING(" from ") + opposite_dirs[door];
					}
					if (tch->skill_map[ch->speaks] >= 50 && ch->skill_map[ch->speaks] < 50)
					{
						output += MAKE_STRING(" ") + MAKE_STRING(accent_desc(ch, ch->skill_map[ch->speaks])) + ",";
					}

					if (!decipher_speaking (tch, ch->speaks, ch->skill_map[ch->speaks]))
					{
						output += " though you cannot decipher the words.\n";
					}
					else
					{
						output += MAKE_STRING("\n    \"") + toplayer + MAKE_STRING("\"\n");
					}
				}
				else
				{
					output += MAKE_STRING("shout from ") + MAKE_STRING(opposite_dirs[door]) + MAKE_STRING("\n    \"") + toplayer + MAKE_STRING("\"\n");
				}
				tch->send_to_char(output.c_str());
				continue;
			}
				//TODO needs to have a check for distance and location to only give shouts to nearby rooms
			/********
			for (room_iterator = room_map.begin(); room_iterator != room_map.end(); room_iterator++)
			{
				room = room_iterator->second;
				
				for (tch = room->people; tch; tch = tch->next_in_room)
				{
					if (!tch->desc)
					{
						continue;
					}
					std::string output, sex_string;
					if (ch->sex == SEX_MALE)
					{
						sex_string = "male ";
					}
					else if (ch->sex == SEX_FEMALE)
					{
						sex_string = "female ";
					}
					else
					{
						sex_string = "";
					}
					output = MAKE_STRING("You hear a ") + sex_string + MAKE_STRING("voice ");
					
					if ((ch->get_trust()(tch) > 1) && GET_FLAG(tch, FLAG_SEE_NAME))
					{
						output += MAKE_STRING("(") + MAKE_STRING(ch->name) + ") ";
					}
					
					if (!IS_SET(ch->room->room_flags, OOC))
					{
						if (!tch->skill_map[ch->speaks])
						{
							output += MAKE_STRING("shout in an unknown tongue from the outside");
						}
						else
						{
							output += MAKE_STRING("shout in ") + MAKE_STRING(ch->speaks) + MAKE_STRING(" from the outside.");
						}
						if (tch->skill_map[ch->speaks] >= 50 && ch->skill_map[ch->speaks] < 50)
						{
							output += MAKE_STRING(" ") + MAKE_STRING(accent_desc(ch, ch->skill_map[ch->speaks])) + ",";
						}

						if (!decipher_speaking (tch, ch->speaks, ch->skill_map[ch->speaks]))
						{
							output += " though you cannot decipher the words.\n";
						}
						else
						{
							output += MAKE_STRING("\n    \"") + toplayer + MAKE_STRING("\"\n");
						}
					}
					else
					{
						output += MAKE_STRING("shout from the outside ") + MAKE_STRING("\n    \"") + toplayer + MAKE_STRING("\"\n");
					}
					
					tch->send_to_char(output.c_str());
					continue;
				}
			}
			 *******/
		}
	}

}

