//////////////////////////////////////////////////////////////////////////////
//
/// comm.chat.cpp : Chat and other OOC Communication Module
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
do_think (CHAR_DATA * ch, char *argument, int cmd) 
{
	char *p = '\0';
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf1[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };

	if (IS_SET (ch->room->room_flags, OOC) && ch->hasMortalBody())
	{
		ch->send_to_char ("This command has been disabled in OOC zones.\n");
		return;
	}

	while (isspace (*argument))
		argument++;

	if (!*argument)
	{
		ch->send_to_char ("What would you like to think?\n");
		return;
	}

	sprintf (buf, "You thought: %s", argument);
	sprintf (buf1, "%s thinks, \"%s\"", ch->name, argument);
	reformat_say_string (argument, &p, 0);
	sprintf (buf2, "#6You hear #5%s#6 think,\n   \"%s\"#0\n",  ch->char_short(), p);
	free_mem (p); // char*

	ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);

	/* Send thoughts to global Imm telepaths */
	for (descriptor_data *d = descriptor_list; d; d = d->next) {
		if (d->character && (d->connected == CON_PLYNG)) {
			if (!((d->character)->get_trust() > 1) || d->character == ch)
			{
				continue;
			}
			else {
				if (IS_SET (d->character->flags, FLAG_TELEPATH)) {
					d->character->act(buf1, false, 0, 0, TO_CHAR | _ACT_FORMAT);
				}
			}
		}
	}

	/* Send thoughts to in-room PC telepaths */
	for (CHAR_DATA *tch = ch->room->people; tch; tch = tch->next_in_room) {
		if (tch == ch) {	/* You don't get an echo of your own thought */
			continue;
		}
		else if (!(ch->hasMortalBody())) {	/* Imm thinks are not overheard */
			continue;
		}
		else if ((tch->get_trust() > 1) && !(tch->hasMortalBody())) { /* L5 PCs can use telepath but are not excluded from skill_telepathy */
			continue;
		}
		else if (skill_use (tch, "Telepathy", ch->skill_map["Telepathy"] / 3)
			|| (IS_NPC (ch) && tch->skill_map["Telepathy"])) {
				tch->send_to_char(buf2);
		}
	}
}


void
do_ichat (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	char *p = '\0';
	DESCRIPTOR_DATA *i = NULL;

	if (ch->get_trust() < 2)
	{
		ch->send_to_char ("Eh?\n");
		return;
	}
	
	if (!IS_SET (ch->flags, FLAG_ISADMIN) 
		&& !IS_SET (ch->desc->acct->flags, ACCOUNT_EMERITUS)
		&& (!(IS_NPC(ch) && ch->desc->original)))
	{
		ch->send_to_char ("Sorry, you are not an admin or Emeritus.\n");
		return;
	}
	
	for (; *argument == ' '; argument++);

	if (!(*argument))
	{
		ch->send_to_char ("What message would you like to send?\n");
		return;
	}
	else
	{
		/// Use the admin's wiznet flag (ignore the NPC's)
		bool ch_wiznet_set =
			(IS_NPC(ch) && ch->desc->original)
			? GET_FLAG (ch->desc->original, FLAG_WIZNET)
			: GET_FLAG (ch, FLAG_WIZNET);

		if (!ch_wiznet_set)
		{
			ch->send_to_char
				("You are not currently tuned into the wiznet. "
				"Type SET WIZNET to change this.\n");
			return;
		}

		if (IS_NPC (ch) && ch->desc->original)
		{
			sprintf (buf, "#1[Wiznet: %s (%s)]#0 %s\n",
				(ch->desc->original)->name,
				ch->name, CAP (argument));
		}
		else
		{
			sprintf (buf, "#1[Wiznet: %s]#0 %s\n",
				ch->name, CAP (argument));
		}

		reformat_string (buf, &p);
		p[0] = toupper (p[0]);

		for (i = descriptor_list; i; i = i->next)
		{
			if (i->character
				&& !i->connected
				&& (((i->character)->get_trust() > 1)
				|| IS_SET (i->character->flags, FLAG_ISADMIN)
				|| IS_SET (i->acct->flags, ACCOUNT_EMERITUS)))
			{
				
				bool tch_wiznet_set =
					(i->original)
					? GET_FLAG (i->original, FLAG_WIZNET)
					: GET_FLAG (i->character, FLAG_WIZNET);

				if (IS_SET (i->character->action, PLR_QUIET))
				{
					sprintf (buf2, "#2[%s is editing.]#0\n",
						(i->character)->name);
				}
				else if (!tch_wiznet_set)
				{
					if (i->character->get_trust())
					{
						CHAR_DATA *tch = (i->original)
							? (i->original)
							: (i->character);
						sprintf
							(buf2, "#2[%s is not listening to the wiznet.]#0\n",
							tch->name);
					}
				}
				else
				{
					i->character->send_to_char (p);
				}

				if (*buf2)
				{
					ch->send_to_char (buf2);
				}
			}
		}
		free_mem (p); // char*
	}
}

void
do_fivenet (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	char *p = '\0';
	DESCRIPTOR_DATA *i = NULL;

	for (; *argument == ' '; argument++);

	if (!(*argument))
	{
		ch->send_to_char ("What message would you like to send?\n");
		return;
	}
	else
	{
		if (!GET_FLAG (ch, FLAG_WIZNET) && !IS_NPC (ch))
		{
			ch->send_to_char
				("You are not currently tuned into the wiznet. Type SET WIZNET to change this.\n");
			return;
		}

		if (IS_NPC (ch) && ch->desc->original)
			sprintf (buf, "#C[Fivenet: %s (%s)]#0 %s\n",
			(ch->desc->original)->name, ch->name, argument);
		else
			sprintf (buf, "#C[Fivenet: %s]#0 %s\n", ch->name,
			CAP (argument));

		reformat_string (buf, &p);

		for (i = descriptor_list; i; i = i->next)
			if (i->character && (i->character)->get_trust() >= 5)
			{
				if (!i->connected)
				{
					
					if (!IS_SET (i->character->action, PLR_QUIET)
						&& (GET_FLAG (i->character, FLAG_WIZNET) || i->original))
					{
						i->character->send_to_char (p);
					}
					else
					{
						if (IS_SET (i->character->action, PLR_QUIET))
							sprintf (buf2, "#2[%s is editing.]#0\n",
							(i->character)->name);
						else if (!GET_FLAG (i->character, FLAG_WIZNET))
							sprintf (buf2,
							"#2[%s is not listening to the wiznet.]#0\n",
							(i->character)->name);
						ch->send_to_char (buf2);
					}
				}
			}
			free_mem (p); // char*
	}

}


void
do_immtell (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *vict = NULL;
	DESCRIPTOR_DATA *d = NULL;
	char *p = '\0';
	char name[MAX_STRING_LENGTH] = { '\0' };
	char message[MAX_STRING_LENGTH] = { '\0' };
	char buf[MAX_STRING_LENGTH] = { '\0' };

	half_chop (argument, name, message);

	if (!*name || !*message)
	{
		ch->send_to_char ("Who do you wish to tell what??\n");
		return;
	}

	vict = get_char_nomask_nonpc (name);
	if (!vict)
	{
		ch->send_to_char ("They aren't here.\n");
		return;
	}
	
	if (ch == vict)
	{
		ch->send_to_char ("You try to tell yourself something.\n");
		return;
	}

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

		sprintf (buf, "#2[From %s]#0 %s\n",
			(IS_NPC (ch) ? ch->short_descr : ch->name),
			CAP (message));
		reformat_string (buf, &p);
		vict->send_to_char (p);
		free_mem (p); // char*

		sprintf (buf, "#5[To %s]#0 %s\n", vict->name, CAP (message));
		reformat_string (buf, &p);
		ch->send_to_char (p);
		free_mem (p); // char*
		
		return;
	}
}


void
do_petition (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *admin = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char *p = '\0';
	bool sent = false;
	const char * sphereName;
	bool emergencyPetition = false;
	std::list<CHAR_DATA*>::iterator tch_iterator;

	if (IS_NPC(ch))
		return;

	argument = one_argument (argument, buf);

	while (isspace (*argument))
		argument++;

	if ( !ch->desc )
		return;

	if (!*argument)
	{
		ch->send_to_char ("Petition what message?\n");
		return;
	}

	if (IS_SET (ch->plr_flags, NOPETITION))
	{
		ch->act("Your ability to petition/think has been revoked by an admin.",
			false, 0, 0, TO_CHAR);
		return;
	}


	if (ch->desc->acct
		&& IS_SET (ch->desc->acct->flags, ACCOUNT_NOPETITION))
	{
		ch->act("Your ability to petition has been revoked by an admin.", false,
			 0, 0, TO_CHAR);
		return;
	}

	// Spheres are back for sending petitions, but not for logging

	int sphereIndex = -1;
	for (int i=0; i<SPHERE_COUNT; i++)
	{
		if (!strcasecmp(buf,spheres[i].name) && spheres[i].available)
		{
			sphereIndex = i;
			sphereName = spheres[i].name;
			break;
		}
	}

	if (!strcasecmp(buf,"emergency"))
	{
		sphereName="#1Emergency#0";
		emergencyPetition = true;
	}
	

	if (sphereIndex>=0 || emergencyPetition)  /* Left sphere index  */
	{
		std::stringstream petitionStream;
		
		sprintf (buf, "%s[Petition(#5%s#6): %s]#0 %s\n", petitionStream.str().c_str(),sphereName, IS_NPC (ch) ? ch->short_descr : ch->name, CAP (argument));
		reformat_string (buf, &p);

		for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
		{
			admin = *tch_iterator;

			if (admin->deleted)
				continue;

			/* skip those switched/LD as they have no socked to send to */
			if (!admin->desc)
				continue;

			/* check original character for admin and petition subscription */
			if (admin->desc 
				&& admin->desc->original 
				&& (admin->get_trust() > 1))
			{
				/* skip if orig char is not subscribed */
				if (!emergencyPetition 
					&& !IS_SET(admin->desc->original->petition_flags,(1<<sphereIndex)))
					continue;

				/* send to switched in target as that has the socket now */
				admin->send_to_char(p);

				/* use switched in idle too, as original is LD */
				if (!admin->desc->idle)
					sent = true;

				continue;
			}

			if (admin->get_trust() < 2)
				continue;

			/* do not send to this admin if they are not assigned to this sphere */
			if (!emergencyPetition && !IS_SET(admin->petition_flags,(1<<sphereIndex)))
				continue;

			admin->send_to_char(p);

			if (!admin->desc->idle)
				sent = true;
		} // end of loop over all characters

		
		/* clean up and sent echo back to user */
		free_mem (p); // char*

		sprintf (buf, "You petitioned: %s\n", CAP (argument));
		reformat_string (buf, &p);
		ch->send_to_char (p);
		free_mem (p); // char*

		
		return;

	}
	/* not a sphere...first check if it's an admin, otherwise splat them the sphere list */

	/* word was not a sphere name. could be an available admin name */
	admin = load_pc (buf);

	if (!admin)
	{
		admin->unload_pc();
		ch->send_to_char("Unable to send your petition.\n");
		ch->send_to_char("Please see HELP PETITION for information on how to use this command.\n");
		return;
	}

	if (admin == ch)
	{
		ch->send_to_char ("Petition yourself? I see...\n");
		admin->unload_pc();
		return;
	}

	if (!is_he_somewhere (admin) 
		|| !admin->pc->level)
	{
		ch->send_to_char ("Sorry, but that person is currently unavailable.\n");
		admin->unload_pc();
		return;
	}

	if (IS_SET (admin->action, PLR_QUIET))
	{
		ch->send_to_char ("That admin is editing.  Please try again in a minute.\n");
	}

	std::stringstream petitionStream;


	sprintf (buf, "%s#5[Private Petition: %s]#0 %s\n", petitionStream.str().c_str(),
		IS_NPC (ch) ? ch->short_descr : ch->name, CAP (argument));

	reformat_string (buf, &p);
	admin->send_to_char(p);
	free_mem (p); // char*
	sprintf (buf, "You petitioned %s: %s\n", admin->name, CAP (argument));
	reformat_string (buf, &p);
	ch->send_to_char (p);
	free_mem (p); // char*

	/* successfully sent to that named staff member */
	admin->unload_pc();
	return;
}


void
do_newbchat (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char *p = '\0';
	DESCRIPTOR_DATA *i = NULL;
	
	if (!IS_SET (ch->plr_flags, NEW_PLAYER_TAG)
		&& !IS_SET (ch->plr_flags, MENTOR))
	{
		ch->send_to_char ("Eh?\n");
		return;
	}
	
	if (!IS_SET (ch->plr_flags, NEWBCHAT))
	{
		ch->send_to_char ("You are not tuned in to the newbie chat channel.\n");
		return;
	}
	
	for (; *argument == ' '; argument++);
	
	if (!(*argument))
	{
		ch->send_to_char ("What message would you like to send?\n");
		return;
	}
	else
	{
		sprintf (buf, "#3[newbchat: %s]#0 %s\n", ch->name, CAP (argument));
		reformat_string (buf, &p);
		p[0] = toupper (p[0]);
		
		for (i = descriptor_list; i; i = i->next)
		{
			if (i->character
				&& !i->connected
				&& (IS_SET (i->character->plr_flags, NEW_PLAYER_TAG)
					|| IS_SET (i->character->plr_flags, MENTOR))
				&& IS_SET(i->character->plr_flags, NEWBCHAT))
			{
				i->character->send_to_char (p);
				
			}
		}
		free_mem (p); // char*
	}
}
