//////////////////////////////////////////////////////////////////////////////
//
/// clan.c : Clan Module
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

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <cstring>

#include "server.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"


extern rpie::server engine;
extern std::map<std::string, CLAN_DATA*> clan_data_map;

CLAN_DATA *clan_list = NULL;


/***** ENFORCEMENT FUNCTIONS ******/
	//all of these are being bypassed for now
int get_next_leader (char **p, char *clan_name, int *clan_flags)
{
	return 0;
	/**
	char flag_names[MAX_STRING_LENGTH]= { '\0' };

	while (1)
	{

		*p = one_argument (*p, flag_names);

		*clan_flags = clan_rank_to_value (flag_names, clan_name);

		*p = one_argument (*p, clan_name);

		if (!*clan_name)
			return 0;

		if (IS_SET (*clan_flags, CLAN_LEADER) ||
			IS_SET (*clan_flags, CLAN_LEADER_OBJ) ||
			(*clan_flags >= CLAN_SERGEANT && *clan_flags <= CLAN_COMMANDER) ||
			IS_SET (*clan_flags, CLAN_MASTER))
			break;
	}

	return 1;
	 /***/
}

void update_enforcement_array (CHAR_DATA * ch)
{
	return;
	/**
	int flags;
	char *p;
	CLAN_DATA *clan_def;
	char clan_name[MAX_STRING_LENGTH]= { '\0' };

	p = ch->clans;

	while (get_next_clan (&p, clan_name, &flags))
	{
		if ((clan_def = get_clandef (clan_name)) && clan_def->zone)
		{
			ch->enforcement[clan_def->zone] = true;
		}
	}

	ch->enforcement[0] = true;
	 /**/
}

	//only used by do_accuse and do_pardon
int is_area_leader (CHAR_DATA * ch)
{
	return 0;
	/**
	int flags;
	char *p;
	CLAN_DATA *clan_def;
	char clan_name[MAX_STRING_LENGTH]= { '\0' };

	p = ch->clans;

	while (get_next_leader (&p, clan_name, &flags))
	{

		if (!(clan_def = get_clandef (clan_name)))
			continue;

		if (clan_def->zone != 0 && ch->room->zone == clan_def->zone)
			return 1;
	}

	return 0;
	 /**/
}




/***** END ENFORCEMENT FUNCTIONS *****/

/**** CONTROLLING NPCS ******/
	//limited to L4 for now
 
void
do_recruit (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *pal;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	
	argument = one_argument (argument, buf);
	
	if (!*buf)
	{
		for (pal = ch->room->people; pal; pal = pal->next_in_room)
		{
			if (IS_NPC (pal) && pal->following != ch && is_higher_rank (ch, pal))
				break;
		}
		
		if (!pal)
		{
			ch->send_to_char("Nobody else here will follow you.\n");
			return;
		}
		
		pal->following = ch;
			// ch->group->insert (pal);
		
		pal->act("$N nods to you to follow.", false, 0, ch,
			 TO_CHAR | _ACT_FORMAT);
		pal->act("$N motions to $S clanmates.", false, 0, ch,
			 TO_NOTVICT | _ACT_FORMAT);
		pal->act("$n falls in.", false, 0, ch, TO_ROOM | _ACT_FORMAT);
		return;
	}
	
	if (!str_cmp (buf, "all"))
	{
		ch->act("$n motions to $s clanmates.", false, 0, 0,
			 TO_ROOM | _ACT_FORMAT);
		for (pal = ch->room->people; pal; pal = pal->next_in_room)
		{
			if (IS_NPC (pal) && pal->following != ch && is_higher_rank (ch, pal))
			{
				pal->following = ch;
					// ch->group->insert (pal);
				pal->act("$N nods to you to follow.", false, 0, ch,
					 TO_CHAR | _ACT_FORMAT);
				pal->act("$n falls in.", false, 0, ch, TO_ROOM | _ACT_FORMAT);
			}
		}
		
		if (!pal)
		{
			ch->send_to_char("Nobody else here will follow you.\n");
			return;
		}
		
		return;
	}
	
	if (!(pal = get_char_room_vis (ch, buf)))
	{
		ch->send_to_char("Nobody is here by that name.\n");
		return;
	}
	
	if (pal == ch)
	{
		ch->send_to_char("Not yourself.\n");
		return;
	}
	
	if (!is_higher_rank (ch, pal))
	{
		ch->act("You don't have the authority to recruit $N.", false, 0, pal,
			 TO_CHAR);
		return;
	}
	
	if (pal->following == ch)
	{
		ch->act("$N is already following you.", false, 0, pal, TO_CHAR);
		return;
	}
	
	pal->following = ch;
		// ch->group->insert (pal);
	
	pal->act("$N motions to $S clanmates.", false, 0, ch,
		 TO_NOTVICT | _ACT_FORMAT);
	pal->act("$n falls in.", false, 0, ch, TO_VICT | _ACT_FORMAT);
	pal->act("$N nods to you to follow.", false, 0, ch, TO_CHAR);
}
 
void
do_disband (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *pal;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	
	argument = one_argument (argument, buf);
	
	if (!*buf || !str_cmp (buf, "all"))
	{
		
		ch->act("$n motions to $s clanmates.", false, 0, 0,
			 TO_ROOM | _ACT_FORMAT);
		ch->act("You motion to your clanmates.", false, 0, 0,
			 TO_CHAR | _ACT_FORMAT);
		
		for (pal = ch->room->people; pal; pal = pal->next_in_room)
		{
			
			if (pal->following != ch || !IS_NPC (pal))
				continue;
			
			if (is_higher_rank (ch, pal) && pal->is_awake())
			{
				pal->act("$N motions to you to stop following.",
					 false, 0, ch, TO_CHAR | _ACT_FORMAT);
				pal->act("$n falls out of step.", false, 0, ch,
					 TO_ROOM | _ACT_FORMAT);
				pal->following = 0;
				
			}
		}
		
		return;
	}
	
	if (!(pal = get_char_room_vis (ch, buf)))
	{
		ch->send_to_char("Nobody is here by that name.\n");
		return;
	}
	
	if (pal->following != ch)
	{
		ch->act("$N is not following you.", false, 0, pal, TO_CHAR);
		return;
	}
	
	if (!is_higher_rank (ch, pal))
	{
		ch->act("You can't give $N orders.", false, 0, pal, TO_CHAR);
		return;
	}
	
	if (!IS_NPC (pal))
	{
		ch->send_to_char("This command does not work on PCs.\n");
		return;
	}
	
	pal->following = 0;
	
	
	ch->act("You motion to $N.", false, 0, pal, TO_CHAR | _ACT_FORMAT);
	pal->act("$N motions to $n.", false, 0, ch, TO_NOTVICT | _ACT_FORMAT);
	pal->act("$N motions to you to stop following.", false, 0, ch, TO_CHAR);
	pal->act("$n falls out of step.", false, 0, ch, TO_ROOM | _ACT_FORMAT);
}
 /***** END CONTROLLING NPC *******/



void
do_invite (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch;
	CLAN_DATA *clan;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char *clan_name;
	int nLeaderRank;
	int ch_rank_lev;
	
		//rank level needed to invite clan members
	nLeaderRank = 4;
	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char("Invite who?\n");
		return;
	}

	if (!(tch = get_char_room_vis (ch, buf)))
	{
		ch->send_to_char("Nobody is here by that name.\n");
		return;
	}

	if (tch == ch)
	{
		ch->send_to_char("You can't invite yourself.\n");
		return;
	}


	argument = one_argument (argument, buf);
	if (!*buf)
	{
		ch->send_to_char("Which clan do you want invite them to join?\n");
		return;
	}
	else 
	{
		clan = get_clandef (buf);
		if (!clan)
		{
			ch->send_to_char("No such clan, I am afraid.\n");
			return;
		}
		
		clan_name = strdup(clan->name);
		
		if (!is_clan_member(ch, clan_name))
		{
			ch->send_to_char("You are not a member of that clan.\n");
			return;
		}
		
		ch_rank_lev = get_clan_rank_level(clan_name, ch->char_clan_map[clan_name]);
				
		if (ch_rank_lev < nLeaderRank)
		{
			ch->send_to_char("Your rank is not high enough to invite someone to join.\n");
			return;
		}
	}
	
	
	if (is_clan_member(tch, clan_name))
	{
		ch->act("$N is already a clan member.", false, 0, tch, TO_CHAR);
		return;
	}
	

	if (!tch->is_awake())
	{
		ch->act("$N isn't conscious right now.", false, 0, tch, TO_CHAR);
		return;
	}	
	

	tch->delay = 120;
	tch->delay_type = DEL_INVITE;
	tch->delay_ch = ch;
	tch->delay_who = duplicateString (buf);

	sprintf (buf, "You invite $N to join %s.",
		clan ? clan->literal : clan_name);

	ch->act(buf, false,  0, tch, TO_CHAR);

	sprintf (buf, "$N invites you to join %s.",
		clan ? clan->literal : clan_name);

	tch->act(buf, false,  0, ch, TO_CHAR);

	tch->act("$N whispers something to $n about joining a clan.",
		false, 0, ch, TO_NOTVICT | _ACT_FORMAT);

	return;
}

void
invite_accept (CHAR_DATA * ch, char *argument)
{
	CLAN_DATA *clan;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char clan_name[MAX_STRING_LENGTH]= { '\0' };

	if (!ch->delay_who || !*ch->delay_who)
		return;

	ch->delay = 0;

	strcpy (clan_name, ch->delay_who);
	free_mem (ch->delay_who);
	ch->delay_who = NULL;

	if (!is_he_here (ch, ch->delay_ch, 1))
	{
		ch->send_to_char("You don't see anyone to take your acceptance.\n");
		return;
	}

	char_clan_add (ch, clan_name, "Member");

	(ch->delay_ch)->act("$N accepts your invitation.", false, 0, ch, TO_CHAR);

	clan = get_clandef (clan_name);

	sprintf (buf, "You have been initiated into %s.",
		clan ? clan->literal : clan_name);

	ch->act(buf, false,  0, 0, TO_CHAR);
	
	return;
}



	
/************* NEW CLAN CODE  *******************/


	//castout <character name> <clan_name>
void
do_castout (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *victim;
	CLAN_DATA *clan;
	bool found = false;
	bool load_tag = false;
	char *clan_name;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	int nLeaderRank;
	
		//minimal rank level of character who can castout
	nLeaderRank = 9;
	
	argument = one_argument (argument, buf);
	
	
	if (!*buf)
	{
		ch->send_to_char("Castout whom?\n");
		return;
	}
	
	if ((victim = get_char_room_vis (ch, buf)))
	{
		found = true;
	}
	else if (!found && (victim = get_pc(buf)))
	{
		found = true;
	}
	else if (!found && (victim = load_pc(buf)))
	{
		found = true;
		load_tag = true;
	}
	else
	{
		sprintf (buf2,"There is no one with that name.\n");
	}
	
	if (!found)
	{
		ch->send_to_char(buf2);
		return;
	}
	
	
	argument = one_argument (argument, buf);
	if (!*buf)
	{
		ch->send_to_char("Which clan do you want cast them out of?\n");
		return;
	}
	else 
	{
		clan = get_clandef (buf);
		if (!clan)
		{
			ch->send_to_char("No such clan, I am afraid.\n");
			if (load_tag)
				victim->unload_pc();
			return;
		}
		
		clan_name = strdup(clan->name);
		
		if (!is_clan_member(ch, clan_name))
		{
			ch->send_to_char("You are not a member of that clan.\n");
			if (load_tag)
				victim->unload_pc();
			return;
		}
		
		if (!is_clan_member(victim, clan_name))
		{
			ch->send_to_char("They are not a member of the clan.\n");
			if (load_tag)
				victim->unload_pc();
			return;
		}
		
		if (!get_clan(ch, clan_name, nLeaderRank))
		{
			ch->send_to_char("You are not a leader in that clan.  You can't cast out anybody.\n");
			if (load_tag)
				victim->unload_pc();
			return;
		}
	}
	
	
	char_clan_remove(victim, clan_name);
	
	sprintf (buf, "$N is no longer a part of %s.", clan->literal);
	ch->act(buf, false,  0, victim, TO_CHAR);
	sprintf (buf, "$n has stripped you of your clan membership in %s.",
			 clan->literal);
	ch->act(buf, false,  0, victim, TO_VICT | _ACT_FORMAT);
	sprintf (buf, "$n has stripped $N of membership in %s.", clan->literal);
	ch->act("$n has stripped $N of clan membership.", false, 0, victim,
		 TO_NOTVICT | _ACT_FORMAT);
	
	if (load_tag)
		victim->unload_pc();
}

/** rollcall <clan name>
 ** Will list all clanmembers who have logged in during the
 ** last two weeks. Only the sdesc and rank is given.
 **/
void
do_rollcall (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char clanname[MAX_STRING_LENGTH]= { '\0' };
	int nLeaderRank;
	MYSQL_RES *result;
	MYSQL_ROW row;
	CLAN_DATA *tclan;
	int index;
	
		//minimal rank level for a leader who can use rollcall
	nLeaderRank = 9;
	
	argument = one_argument (argument, clanname);
	
	
	if (!*clanname)
	{
		ch->send_to_char("Which clan do you want a roster for?\n");
		return;
	}
	
	if (!is_clan_member(ch, clanname))
	{
		ch->send_to_char("You are not a member of that clan.\n");
		return;
	}
	
	if (!get_clan(ch, clanname, nLeaderRank))
	{
		ch->send_to_char("You do not have sufficent rank to know that information.\n");
		return;
	}
	
	/* Keep clan_name as the short name */
	if ((tclan = get_clandef_long (clanname)))
		strcpy (clanname, tclan->name);
	
	
	
	/* search last fornight */
	result = mysql_player_search (SEARCH_CLAN, clanname, 4);
	
	if (!result || !mysql_num_rows (result))
	{
		if (result)
			mysql_free_result (result);
		
		
		ch->send_to_char("No names were found.\n");
		return;
	}
	
	sprintf (buf, "#6Clan members:  %d#0\n\n",
			 (int) mysql_num_rows (result));
	
	index = 1;
	
	while ((row = mysql_fetch_row (result)))
	{
			// row[2] is sdesc
			// row[4] is rank
		sprintf (buf2, "%4d. %-13s %s\n", index, row[4], row[2]);
		
		if (strlen (buf) + strlen (buf2) >= MAX_STRING_LENGTH)
			break;
		else
			sprintf (buf + strlen (buf), "%s", buf2);
		
		index++;
	}
	
	page_string (ch->desc, buf);
	mysql_free_result (result);
	return;
}

void
do_promote (CHAR_DATA * ch, char *argument, int cmd)
{
	int nLeaderRank;
	int nLackeyRank = 0;
	CHAR_DATA *pLackey;
	CLAN_DATA *pClan;
	char strLackey[AVG_STRING_LENGTH] = "\0";
	char strClan[AVG_STRING_LENGTH] = "\0";
	char strRank[AVG_STRING_LENGTH] = "\0";
	char strOutput[AVG_STRING_LENGTH] = "\0";
	char strUsage[] = "Usage: promote <character> <clan> <rank>.\n\0";
	char *rank_name;
	
	argument = one_argument (argument, strLackey);
	
		//minimal rank level for a leader who can promote
		//TODO: consider changing to 2 or 3 ranks above target character
	nLeaderRank = 9;
	
	if (!*strLackey)
	{
		ch->send_to_char(strUsage);
		return;
	}
	
	if (!(pLackey = get_char_room_vis (ch, strLackey)))
	{
		ch->send_to_char("Nobody is here by that name.\n");
		ch->send_to_char(strUsage);
		return;
	}
	
	if (pLackey == ch)
	{
		switch (number (1, 3))
		{
			case 1:
				ch->send_to_char("Your delusions of grandeur go largely unnoticed.\n");
				break;
			case 2:
				ch->send_to_char("You give yourself a pat on the back.\n");
				break;
			default:
				ch->send_to_char("Unfortunately, promotions just don't work that way...\n");
				break;
		}
		ch->send_to_char(strUsage);
		return;
	}
	
	argument = one_argument (argument, strClan);
	
	if (!*strClan)
	{
		ch->send_to_char("Which clan did you wish to make the promotion in?\n");
		return;
	}
	
	if (!is_clan_member(ch, strClan))
	{
		ch->send_to_char("You are not a member of that clan!\n");
		return;
	}
	
	if (!is_clan_member(pLackey, strClan))
	{
		ch->send_to_char("They are not a member of the clan!\n");
		return;
	}
	
		//straight test for an absolute level for promotion
	if (!get_clan(ch, strClan, nLeaderRank))
	{
		ch->send_to_char("You are not high enough ranked to promote anyone.\n");
		return;
	}
	
	
	/* Keep clan_name as the short name */
	if ((pClan = get_clandef_long (strClan)))
		strcpy (strClan, pClan->name);
	
	argument = one_argument (argument, strRank);
	
	if (!*strRank)
	{
		ch->send_to_char(strUsage);
		return;
	}
	
	if (!(nLackeyRank = clan_rank_to_value (strRank, strClan)))
	{
		ch->send_to_char("I don't recognize the specified rank.\n");
		return;
	}
	
	
	if (nLackeyRank >= nLeaderRank)
	{
		ch->send_to_char("You do not have the authority to make this promotion.\n");
		return;
	}
	
	rank_name = (char*)rank_name_from_flags(strClan, nLackeyRank);
	
	sprintf (strOutput, "You promote $N to the rank of %s.", rank_name_from_flags(strClan, nLackeyRank));
	ch->act(strOutput, false, 0, pLackey, TO_CHAR | _ACT_FORMAT);
	
	sprintf (strOutput, "$n has promoted you to the rank of %s.", rank_name_from_flags(strClan, nLackeyRank));
	ch->act(strOutput, false, 0, pLackey, TO_VICT | _ACT_FORMAT);
	
	ch->act("$n has promoted $N.", false, 0, pLackey, TO_NOTVICT | _ACT_FORMAT);
	
	rank_name = strdup(rank_name_from_flags(strClan, nLackeyRank));
	char_clan_add (pLackey, strClan, rank_name);
}

	//Returns 1 if src outranks tar
	//returns 0 if src = tar, or targ is higher ranked
int
is_higher_rank (CHAR_DATA * src, CHAR_DATA * tar)
{
	int counter;
	int target_rank_value;
	int src_rank_value;
	std::map<std::string, std::string>::iterator clan_it;
	std::string strclan;
	std::string strrank;
	
	counter = src->char_clan_map.size();
	if (counter > 0)
	{
		for (clan_it = src->char_clan_map.begin(); clan_it !=  src->char_clan_map.end(); clan_it++)
		{
			if (clan_data_map.find(clan_it->first) != clan_data_map.end())
			{
				strclan = clan_it->first;
				strrank = clan_it->second;
				if (!strrank.empty())
				{
					counter --;
					if (is_clan_member (tar, (char *)strclan.c_str()))
					{
						target_rank_value = get_clan_rank_level(strclan, tar->char_clan_map[strclan]);
						
						src_rank_value = get_clan_rank_level(strclan, src->char_clan_map[strclan]);
						
						if (src_rank_value > target_rank_value)
							return 1;
					}
				}
				if (counter == 0)
					break;
			}
		}
	}
	
	
	
	
	if (!IS_NPC (src) && IS_NPC (tar))
	{
		if (tar->mob->owner && !str_cmp (src->name, tar->mob->owner))
			return 1;
	}
	
	return 0;
}


	//displays a string with ranks of the clans held in common
void 
display_clan_ranks (CHAR_DATA * ch, CHAR_DATA * observor)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	std::map<std::string, std::string>::iterator clan_it;
	std::map<std::string, CLAN_DATA*>::iterator clan_it2;
	std::string tclan;
	std::string trank;
	std::string target_rank;
	std::string full_name;
	int counter;
	int clan_num = 0;
	
	sprintf (buf, "You recognize that %s carries the rank of", HSSH (ch));
	
	counter = observor->char_clan_map.size();
	if (counter > 0)
	{
		for (clan_it = observor->char_clan_map.begin(); clan_it !=  observor->char_clan_map.end(); clan_it++)
		{
			clan_it2 = clan_data_map.find(clan_it->first);
			if (clan_it2 != clan_data_map.end())
			{
				tclan = clan_it->first;
				trank = clan_it->second;
				if (!trank.empty())
				{
					counter --;
					if (is_clan_member (ch, (char *)tclan.c_str()))
					{
						target_rank = ch->char_clan_map[tclan];
						full_name = clan_it2->second->literal;
						sprintf(buf + strlen(buf), " %s in %s", target_rank.c_str(), full_name.c_str());
						clan_num ++;
					}
					
					if (counter != 0)
						sprintf(buf + strlen(buf), ", and ");
				}
				if (counter == 0)
				{
					sprintf(buf + strlen(buf), ".\n");
					break;
				}
			}
		}
	}
	
	if (clan_num == 0)
		return;
	else
	{
		observor->send_to_char ("\n");
		*buf = toupper (*buf);
		
		std::stringstream tempstr;
		tempstr << reformat_desc(buf);	
		observor->send_to_char (tempstr.str().c_str());
		
	}
	
	return;
}

	//tests if the given name and flags are from a clan with the name as a long clan name
int
get_clan_long_short (CHAR_DATA * ch, char *clan_name, int clan_flags)
{
	if (get_clan_long (ch, clan_name, clan_flags))
		return 1;
	
	return get_clan (ch, clan_name, clan_flags);
}
	//test if the given clan name exist as a clan long name, and if that clan has the given flag value
int
get_clan_long (CHAR_DATA * ch, char *clan_name, int clan_flags)
{
	CLAN_DATA *clan;
	
	if (!(clan = get_clandef_long (clan_name)))
		return 0;
	
	if (!get_clan (ch, clan->name, clan_flags))
		return 0;
	
	return 1;
}


	//is he a member of the given clan? Shortname only
int
is_clan_member (CHAR_DATA * ch, char *clan_name)
{
	
	int counter;
	std::map<std::string, std::string>::iterator clan_it;
	std::string strclan;
	
	counter = ch->char_clan_map.size();
	if (counter > 0)
	{
		for (clan_it = ch->char_clan_map.begin(); clan_it !=  ch->char_clan_map.end(); clan_it++)
		{
			strclan = clan_it->second;
			
			if (!strclan.empty())
			{
					//does the clan exist?
				if (clan_data_map.find(clan_it->first)!=clan_data_map.end())
				{
					strclan = clan_it->first;
					
					if (!strclan.empty())
					{
						counter --;
						
						if (!str_cmp(strclan.c_str(), clan_name))
							return 1;	
						
					}
					if (counter == 0)
						break;
				}
			}
		}
	}
	
	return 0;
	
}

int
is_brother(CHAR_DATA * ch, CHAR_DATA *tch)
{
	
	int counter;
	std::map<std::string, std::string>::iterator clan_it;
	std::string strclan;
	
	counter = ch->char_clan_map.size();
	if (counter > 0)
	{
		for (clan_it = ch->char_clan_map.begin(); clan_it !=  ch->char_clan_map.end(); clan_it++)
		{
			strclan = clan_it->second;
			if (!strclan.empty())
			{
					//does the clan exist?
				if (clan_data_map.find(clan_it->first)!=clan_data_map.end())
				{
					strclan = clan_it->first;
					
					if (!strclan.empty())
					{
						counter --;
						
						if (is_clan_member(tch, (char*)strclan.c_str()))
							return 1;	
						
					}
					if (counter == 0)
						break;
				}
			}
		}
	}
	
	return 0;
	
}

/* the _player version of this routine is called to parse a clan name
 a player might type in.  Short and long names are both tested.
 */
int
is_clan_member_player (CHAR_DATA * ch, char *clan_name)
{
	CLAN_DATA *clan;
	
	if (is_clan_member (ch, clan_name))
	{				/* Short name match */
		if (get_clandef (clan_name))
			return 0;		/* Supplied short name when long existed */
		return 1;
	}
	
	if (!(clan = get_clandef_long (clan_name)))
		return 0;
	
	return is_clan_member (ch, clan->name);
}

	//Returns a clan struct for a clan with a given long name
CLAN_DATA *
get_clandef_long (char *clan_long_name)
{
	CLAN_DATA *clan;
	int counter;
	std::map<std::string, CLAN_DATA *>::iterator clan_it;
	std::string strclan;
	
	counter = clan_data_map.size();
	if (counter > 0)
	{
		for (clan_it = clan_data_map.begin(); clan_it !=  clan_data_map.end(); clan_it++)
		{
			strclan = clan_it->first;
			if (!strclan.empty())
			{
				counter --;
				clan = clan_it->second;
				
				if (!str_cmp(clan->literal, clan_long_name))
					return (clan);	
				
			}
			if (counter == 0)
				break;
		}
	}
	return NULL;
}

	//Returns a clan struct for a clan with the given short-name
CLAN_DATA *
get_clandef (const char *clan_name)
{
	std::map<std::string, CLAN_DATA *>::iterator clan_it;
	std::string strclan;
	
	strclan = strdup(clan_name);
	clan_it = clan_data_map.find(strclan);
	
	if (clan_it !=clan_data_map.end())
		return (clan_it->second);
	
	return NULL;
}


	//tests if the character has the given clan and clan flags
int
get_clan (CHAR_DATA * ch, const char *clan_name, int rank)
{
	int counter;
	int rank_value;
	std::map<std::string, std::string>::iterator clan_it;
	std::string tclan;
	std::string trank;
	
	counter = ch->char_clan_map.size();
	if (counter > 0)
	{
		for (clan_it = ch->char_clan_map.begin(); clan_it !=  ch->char_clan_map.end(); clan_it++)
		{
				//does the clan exist?
			if (clan_data_map.find(clan_it->first)!=clan_data_map.end())
			{
				tclan = clan_it->first;
				trank = clan_it->second;
				
				if (!trank.empty())
				{
					counter --;
					rank_value = clan_rank_to_value(trank.c_str(), clan_name);
					
					if ((!str_cmp(tclan.c_str(), clan_name))
						&& (rank_value == rank))
						return 1;	
					
				}
				if (counter == 0)
					break;
			}
		}
	}
	
	return 0;
}

	//converts the rank name from the clan, into a level number
int
clan_rank_to_value (const char *rank_names, const char *clan_name)
{
	int rank_value = 0;
	int i;
	char *tname;
	CLAN_DATA* tclan;
	std::string clan_string;
	std::map<std::string, CLAN_DATA*>::iterator clan_it;
	
	tname = strdup(clan_name);		
	clan_it = clan_data_map.find(tname);
	if (clan_it != clan_data_map.end())
	{
		tclan = clan_it->second;
	}
	else
	{
			//switch case if needed
		if (islower(tname[0]))
			tname[0] = toupper(tname[0]);
		else
			tname[0] = tolower(tname[0]);
		
				
		clan_it = clan_data_map.find(tname);
		if (clan_it != clan_data_map.end())
		{
			tclan = clan_it->second;
		}
		else
		{
			return 0;
		}

	}
	
	if (tclan)
	{
		for (i = 0; i < MAX_CLAN_RANKS; i++)
		{
			if (!str_cmp(rank_names, tclan->rank[i]))
			{
				rank_value = i;
				break;	
			}
		}
	}
	return rank_value;
}

	//TODO: this may need re-writing at a later date
	//iterates through a string to get the next clan-name
	//used by enforcement, and craft code
int
get_next_clan (char **p, char *clan_name, int *clan_flags)
{
	char flag_names[MAX_STRING_LENGTH]= { '\0' };
	
	*p = one_argument (*p, flag_names);
	
	*clan_flags = clan_rank_to_value (flag_names, clan_name);
	
	*p = one_argument (*p, clan_name);
	
	if (!*clan_name)
		return 0;
	
	return 1;
}

	//tests if the given string contains a clan and if it is, does the rank exist
	//string will be in format "<clan rank> <clan name>"
	//returns 1 if the clan/rank combo is in the string, 0 otherwise
int
is_clan_in_string (char *thestring, char *clan, int clan_flags)
{
	char *val_check;
	char *rank_name;
	char test_value[MAX_STRING_LENGTH]= { '\0' };
	
		//does the clan exist and rank exist?
	if (lookup_clan_id (clan))
		rank_name = strdup(rank_name_from_flags(clan, clan_flags));
	else 
		return 0;
	
		//if clan and rank both exist, combine them
	if (rank_name)
		sprintf(test_value, "%s %s", rank_name, clan);
	else 
		return 0;
	
		//check if combined they are in the string	
	val_check = strstr (thestring, test_value);
	
	if (val_check != NULL)
		return 1;
	else 
		return 0;
}
	//combines clan and rank into a single string segment
char *
add_clan_to_string (char *thestring, char *new_clan_name, int new_rank)
{
	char *argument;
	char clan_name[MAX_STRING_LENGTH]= { '\0' };
	char rank_name[MAX_STRING_LENGTH]= { '\0' };
	static char buf[MAX_STRING_LENGTH]= { '\0' };
	
	argument = thestring;
	
		// if we're changing flags, we need to remove the old data 
	
	while (1)
	{
		
		argument = one_argument (argument, rank_name);
		argument = one_argument (argument, clan_name);
		
		if (!*clan_name)
			break;
		
		if (!str_cmp (clan_name, new_clan_name))
		{
			remove_clan_from_string (thestring, new_clan_name);
			break;
		}
	}
	
		//Now we add the clan and ranks to buffer
		// <new rank> <new clan> <old string>
	if (thestring && *thestring)
		sprintf (buf, "\"%s\" %s %s",
				 rank_name_from_flags (new_clan_name, new_rank),
				 new_clan_name,
				 thestring);
	else
		sprintf (buf, "\"%s\" %s",
				 rank_name_from_flags (new_clan_name, new_rank),
				 new_clan_name);
	
	while (buf[strlen (buf) - 1] == ' ')
		buf[strlen (buf) - 1] = '\0';
	
	if (thestring && *thestring)
	{
		free_mem (thestring);
		thestring = NULL;
	}
	
	return buf;
}


const char *
rank_name_from_flags (char* clan_name, int flags)
{
	CLAN_DATA* tclan;
	std::string clan_string;
	std::map<std::string, CLAN_DATA*>::iterator clan_it;
	std::string buffer;
	
	buffer.assign("");
	clan_string.assign(clan_name);
	
	clan_it = clan_data_map.find(clan_string);
	if (clan_it != clan_data_map.end())
	{
		tclan = clan_it->second;
		buffer.append(tclan->rank[flags]);
	}
	else
	{
			//switch case if needed
		if (islower(clan_name[0]))
			clan_name[0] = toupper(clan_name[0]);
		else
			clan_name[0] = tolower(clan_name[0]);
		
		clan_string.assign(clan_name);
		
		clan_it = clan_data_map.find(clan_string);
		if (clan_it != clan_data_map.end())
		{
			tclan = clan_it->second;
			buffer.append(tclan->rank[flags]);
		}
	}
	
	
	return (buffer.c_str());
}

	// Returns clan database id, if any, for specified clan name
	// returns -1 if clan does not exist
int
lookup_clan_id (char *clan_name)
{
	std::string clan_string;
	std::map<std::string, CLAN_DATA*>::iterator clan_it;
	CLAN_DATA* tclan;
	
	clan_string.assign(clan_name);
	
	clan_it = clan_data_map.find(clan_string);
	if (clan_it != clan_data_map.end())
	{
		tclan = clan_it->second;
		return tclan->id;
	}
	else
	{
			//switch case if needed
		if (islower(clan_name[0]))
			clan_name[0] = toupper(clan_name[0]);
		else
			clan_name[0] = tolower(clan_name[0]);
		
		clan_string.assign(clan_name);
		
		clan_it = clan_data_map.find(clan_string);
		if (clan_it != clan_data_map.end())
		{
			tclan = clan_it->second;
			return tclan->id;
		}
	}
	
	return -1;	
}


char *
lookup_clan_name (int clan_num)
{
	std::map<std::string, CLAN_DATA*>::iterator clan_it;
	CLAN_DATA* tclan;
	
	if (clan_num == 0)
		return NULL;
	
	for (clan_it = clan_data_map.begin(); clan_it != clan_data_map.end(); clan_it++)
	{
		tclan = clan_it->second;
		if (tclan->id == clan_num)
			return (tclan->name);
	}
	
	return NULL;	
}

/* called by db.c:boot_objects */
void
clan__assert_member_objs ()
{
	char buf[AVG_STRING_LENGTH];
	int counter = 0;
	CLAN_DATA *tclan;
	OBJ_DATA *obj;
	std::map<std::string, CLAN_DATA*>::iterator clan_it;
	
	counter = clan_data_map.size();
	if (counter > 0)
	{
		for (clan_it = clan_data_map.begin(); clan_it !=  clan_data_map.end(); clan_it++)
		{
			tclan = clan_it->second;
			
			if (tclan->leader_obj)
			{
				
				if (!(obj = vtoo (tclan->leader_obj)))
				{
					sprintf (buf,
							 "Note:  Clan leader obj %d does not exist for %s.",
							 tclan->leader_obj, tclan->name);
					system_log (buf, true);
				}
				else
				{
					obj->obj_flags.extra_flags |= ITEM_LEADER;
				}
				
			}
			
			if (tclan->member_obj)
			{
				
				if (!(obj = vtoo (tclan->member_obj)))
				{
					sprintf (buf,
							 "Note:  Clan member obj %d does not exist for %s.",
							 tclan->member_obj, tclan->name);
					system_log (buf, true);
				}
				else
				{
					obj->obj_flags.extra_flags |= ITEM_MEMBER;
				}
				
			}
		}
		
	}
	
	return;
}

	//TODO: needs massive re-write or consider removal
	//methods to create, edit and list clans
	//use "mset clan" commands to work with clans for characters
void
do_clan (CHAR_DATA * ch, char *argument, int cmd)
{
	
	int clan_zone = 0;
	int counter;
	int leader_obj_vnum = 0;
	int member_obj_vnum = 0;
	int clan_id;
	OBJ_DATA *obj;
	CLAN_DATA *tclan;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char clan_name[MAX_STRING_LENGTH]= { '\0' };
	char newname[MAX_STRING_LENGTH]= { '\0' };
	char clan_literal[MAX_STRING_LENGTH]= { '\0' };
	std::list<char_data*>::iterator tch_iterator;
	std::map<std::string, CLAN_DATA*>::iterator clan_it;
	
	if (IS_NPC (ch))
	{
		ch->send_to_char("Sorry, but you can't do this while switched...\n");
		return;
	}
	
	argument = one_argument (argument, buf);
	
		//lists all current clans
	if (!*buf)
	{
		
		sprintf (buf,"\nClan Name        Full Clan Name\n");
		sprintf (buf + strlen(buf),"===============  =================================\n");
		
		counter = clan_data_map.size();
		if (counter > 0)
		{
			for (clan_it = clan_data_map.begin();
				 clan_it !=  clan_data_map.end();
				 clan_it++)
			{
				tclan = clan_it->second;
				if (tclan)
				{
					counter --;
					sprintf (buf + strlen(buf), "%-15s  %s\n",
							 clan_it->first.c_str(),
							 tclan->literal);
					
					if (tclan->zone)
					{
						sprintf (buf + strlen(buf), "                 Enforcement Zone %d\n",
								 tclan->zone);
					}
					
					if (tclan->member_obj)
					{
						obj = vtoo (tclan->member_obj);
						sprintf (buf + strlen(buf), "                 Member Object (%06d):  %s\n",
								 tclan->member_obj,
								 obj ? obj->short_description : "UNDEFINED");
					}
					
					if (tclan->leader_obj)
					{
						obj = vtoo (tclan->leader_obj);
						sprintf (buf + strlen(buf), "                 Leader Object (%06d):  %s\n",
								 tclan->leader_obj,
								 obj ? obj->short_description : "UNDEFINED");
					}
					
					
				}
				if (counter == 0)
					break;
				
				sprintf (buf + strlen(buf), "\n");
			}
		}
		page_string (ch->desc, buf);
		return;
	}
	
		//gives syntax help
	else if (*buf == '?')
	{
		ch->send_to_char("\nSyntax:\n");
		ch->send_to_char("   clan\n  (with no paramters, will list all current clans)\n");
		
		ch->send_to_char("   clan create <short name> <long name> [<enforcement zone>] "
					  "[<leader obj>] [<member obj>]\n");
		ch->send_to_char("   clan remove <short name>\n");
		ch->send_to_char("   clan rename <short name> <new long name>\n  You are only changing the Long name, visible to players. The short-name will remain unchanged.\n");
		ch->send_to_char("\nThe obj vnums are optional.\n\nExamples:\n");
		
		ch->send_to_char("  > clan create osgilwatch 'Osgiliath Watch' 10\n");
		ch->send_to_char("  > clan remove osgilwatch\n");
		return;
	}
	
		//removes a clan from the datbase - L5 required
	else if (!str_cmp (buf, "remove") || !str_cmp (buf, "delete"))
	{
		
		if (IS_NPC (ch))
		{
			ch->send_to_char("This command cannot be used while switched.\n");
			return;
		}
		
		if (ch->get_trust() < 5)
		{
			ch->send_to_char("You must be level 5 to delete clans from the database.\n");
			return;
		}
		
		clan_it = clan_data_map.find(argument);
		
		if (clan_it != clan_data_map.end())
		{
			mysql_safe_query("DELETE FROM clans WHERE name = '%s'",
							 argument);
			
			clan_id = clan_it->second->id;
			mysql_safe_query("DELETE FROM clan_ranks WHERE id = '%d'",
							 clan_id);

			
			clan_data_map.erase(argument);
			
			ch->send_to_char("That clan has been removed from the database and the game.\n");
			return;
		}
		
		else
		{
			ch->send_to_char("That clan name is not recognized. Remember, you must use the short name.\n");
			return;
		}
		
	}
	
		//changes the long name of an existing clan
	else if (!str_cmp (buf, "rename"))
	{
		
		argument = one_argument (argument, clan_name);
		argument = one_argument (argument, newname);
		
		clan_it = clan_data_map.find(clan_name);
		if (clan_it != clan_data_map.end())
			tclan = clan_it->second;
		
		if (tclan)
		{
			tclan->literal = strdup(newname);
			mysql_safe_query("UPDATE `clans` SET `long_name` =  '%s' WHERE `name` = '%s';",
							 tclan->literal,
							 tclan->name);
			ch->send_to_char("That long name has been changed.\n");
			return;
		}
		else
		{
			ch->send_to_char("That clan name is not recognized. Remember, you must use the short name.\n");
			return;
		}
		
	}
	
		//creates a new clan
		//clan create <short name> <long name> [<enforcement zone>] [<leader obj>] [<member obj>]
	else if (!str_cmp (buf, "create"))
	{
		
		argument = one_argument (argument, clan_name);
		argument = one_argument (argument, clan_literal);
		
		if (!*clan_name || !*clan_literal)
		{
			ch->send_to_char("You need to specify a clan short name and long name. \n");
			return;
		}
		
		clan_it = clan_data_map.find(clan_name);
		if (clan_it != clan_data_map.end())
		{
			ch->send_to_char("That clan short name already exists. Did you mean to make changes or create a new clan? \n");
			return;
		}
		
		argument = one_argument (argument, buf);
		if (atoi(buf) > 0)
			clan_zone = atoi(buf);
		
		argument = one_argument (argument, buf);
		if (atoi(buf) > 0)
		{
			leader_obj_vnum = atoi(buf);
			if (!(obj = vtoo(leader_obj_vnum)))
			{
				ch->send_to_char("The leader object does not exist.\n");
				return;
			}
			
		}
		
		argument = one_argument (argument, buf);
		if (atoi(buf) > 0)
		{
			member_obj_vnum = atoi(buf);
			if (!(obj = vtoo(member_obj_vnum)))
			{
				ch->send_to_char("The member object does not exist.\n");
				return;
			}
		}
		
		sprintf (buf, "INSERT INTO clans (name, long_name, zone, leader_obj, member_obj) VALUES ('%s','%s', %d, %d, %d);",
				 clan_name,
				 clan_literal,
				 clan_zone,
				 leader_obj_vnum,
				 member_obj_vnum);
		
		mysql_safe_query (buf);
		
			//loads the clan data into the game
		load_clans_mysql ();
		
			//now that the clan is in the game with an id, we can create initial ranks
		clan_it = clan_data_map.find(clan_name);
		
		if (clan_it != clan_data_map.end())
		{
			
		clan_id = clan_it->second->id;
		sprintf(buf, "INSERT INTO clan_ranks (clan_num, rank_name, rank_num, is_used) VALUES ( '%d', '%s', '%d', '%d');",
					clan_id, 
					"Member",
					1,
					1);
			
			mysql_safe_query (buf);
			
			
		}
		
			//refresh clan ranks and clans
		load_clans_mysql ();
		
		ch->send_to_char("The clan and initial rank of Member has been created in the database and is now live.\n");
		return;

		
	}
	
	else
	{
		ch->send_to_char("What do you want to do again?  Type clan ? for help.\n");
		return;
	}
	
}

void
clan__do_score (CHAR_DATA * ch)
{
	char buf[AVG_STRING_LENGTH] = "";
	char buf2[AVG_STRING_LENGTH] = "";
	std::map<std::string, std::string>::iterator clan_it;
	std::string tclan;
	int counter;
	
	sprintf (buf, "\nYou are associated with the following Clans:\n");
	
	counter = ch->char_clan_map.size();
	if (counter > 0)
	{
		for (clan_it = ch->char_clan_map.begin(); clan_it !=  ch->char_clan_map.end(); clan_it++)
		{
			if (clan_data_map.find(clan_it->first)!=clan_data_map.end())
			{
				tclan = clan_it->second;
				if (!tclan.empty())
				{
					counter --;
					sprintf(buf2, "%s", clan_data_map[clan_it->first.c_str()]->literal);
					sprintf(buf + strlen(buf), "    [%s - %s]\n", buf2, clan_it->second.c_str());
				}
				if (counter == 0)
					break;
			}
		}
	}
	ch->send_to_char(buf);
	
}

	//Adds the clan to the map of clans for the player
	//checks for rank and calls char_clan_rank_add if needed
	//default of 'Member' is used if rank is not specified or does not exist
	//returns 1 if clan exists, -1 if clan does not exist
int
char_clan_add(CHAR_DATA* ch, char *clan_name, char* clan_rank)
{
	int swap = -1;
	
	if (lookup_clan_id(clan_name) > 0)
	{
		ch->char_clan_map[clan_name] = strdup("Member");
		
		if (str_cmp(clan_rank, "Member"))
		{
			swap = char_clan_rank_change(ch, clan_name, clan_rank);
			return (swap);
		}
	}
	
	return (-1);
		
}

	//Removes the clan from the map of clans for the player
	//returns 1 removal succeed, -1 if it did not
int
char_clan_remove(CHAR_DATA* ch, char *clan_name)
{
	if (lookup_clan_id(clan_name) >= 0)
	{
		ch->char_clan_map.erase(clan_name);
		return (1);
	}
	
	return (-1);
	
}

	//given the clan exists, and rank exists, the rank is changed from 'Member'
	//if the rank does not exist, no change is made
	//returns 1 if rank was changed, -1 if rank was not changed
int
char_clan_rank_change(CHAR_DATA* ch, char *clan_name, char* clan_rank)
{
	int i;
	std::map<std::string, CLAN_DATA*>::iterator clan_it;
	CLAN_DATA* tclan;
	std::string clan_name_str;
	
	clan_it = clan_data_map.find(clan_name);
	
	if (!str_cmp(clan_rank, ""))
		clan_rank = strdup("Member");
	
	if (clan_it != clan_data_map.end())
	{
		tclan = clan_it->second;
		for (i = 0; i < MAX_CLAN_RANKS; i++)
		{
			if (tclan->rank[i])
			{
				clan_name_str = strdup(clan_name);
				if (!str_cmp(tclan->rank[i], clan_rank))
				{				
					ch->char_clan_map[clan_name_str] = strdup(clan_rank);
					return (1);
				}
			}
		}
	}
	else
		return (-1);
	
	return (-1);		
}

	// returns the numerical value for the given rank
	// returns -1 if the rank dose not exist
int
get_clan_rank_level(std::string clan_name, std::string clan_rank)
{
	int i;
	std::map<std::string, CLAN_DATA*>::iterator clan_it;
	CLAN_DATA* tclan;
	std::string clan_name_str;
	
	clan_it = clan_data_map.find(clan_name);
	
	if (clan_it != clan_data_map.end())
	{
		tclan = clan_it->second;
		for (i = 0; i < MAX_CLAN_RANKS; i++)
		{
			if (tclan->rank[i])
			{
				if (!str_cmp(tclan->rank[i], clan_rank.c_str()))
				{				
					return (i);
				}
			}
		}
	}
	
	return (-1);
	
	
}

	// removes a clan from the object 
	// The obejct no longer "belongs" to the clan
	// this has nothing to do with clan member_obj or leader_obj values
void
clan_rem_obj (OBJ_DATA *obj, OBJ_CLAN_DATA * targ)
{
	obj->clan_data = NULL;
	free_mem (targ->name);
	free_mem (targ->rank);
	free_mem(targ);
	return;
}

	//adds clanning when they equip the object
void
clan_object_equip (CHAR_DATA * ch, OBJ_DATA * obj)
{
	char* clan_flags;
	int counter = 0;
	CLAN_DATA *tclan;
	std::map<std::string, CLAN_DATA*>::iterator clan_it;
	
	counter = clan_data_map.size();
	if (counter > 0)
	{
		for (clan_it = clan_data_map.begin(); clan_it !=  clan_data_map.end(); clan_it++)
		{
			tclan = clan_it->second;
			
			if (tclan->leader_obj == obj->nVirtual)
				clan_flags = duplicateString("Leader");
			
			if (tclan->member_obj == obj->nVirtual)
				clan_flags = duplicateString("Member");
			
			char_clan_add (ch, tclan->name, clan_flags);
		}
	}
	return;
}

	//removes the clanning due to the object
void
clan_object_unequip (CHAR_DATA * ch, OBJ_DATA * obj)
{
	int counter = 0;
	CLAN_DATA *tclan;
	std::map<std::string, CLAN_DATA*>::iterator clan_it;
	
	counter = clan_data_map.size();
	if (counter > 0)
	{
		for (clan_it = clan_data_map.begin(); clan_it !=  clan_data_map.end(); clan_it++)
		{
			tclan = clan_it->second;
			
			if ((tclan->leader_obj == obj->nVirtual)
				|| (tclan->member_obj == obj->nVirtual))
				char_clan_remove(ch, tclan->name);
		}
	}
	return;
}

	//removes two arguments from thestring by copying thestring into buf argument by argument, except for the amtching arguements
char *
remove_clan_from_string (char *thestring, char *old_clan_name)
{
	char *argument;
	static char buf[MAX_STRING_LENGTH]= { '\0' };
	char clan_flags[MAX_STRING_LENGTH]= { '\0' };
	char clan_name[MAX_STRING_LENGTH]= { '\0' };
	
	if (!*thestring)
		return NULL;
	
	argument = thestring;
	
	*buf = '\0';
	
	while (1)
	{
		argument = one_argument (argument, clan_flags);
		argument = one_argument (argument, clan_name);
		
		if (!*clan_name)
			break;
		
		if (str_cmp (clan_name, old_clan_name))
			sprintf (buf + strlen (buf), "'%s' %s ", clan_flags, clan_name);
	}
	
	free_mem (thestring);
	thestring = NULL;
	
	if (*buf && buf[strlen (buf) - 1] == ' ')
		buf[strlen (buf) - 1] = '\0';
	
	return buf;
}

	//used in rprogs for clanrank test
bool
outranks (const char *has_rank, const char *compared_rank, const char *clan)
{
	int rank, compare;
	rank = clan_rank_to_value(has_rank, clan);
	compare = clan_rank_to_value(compared_rank, clan);
	
	if (rank > compare)
		return true;
	else
		return false;
	
}