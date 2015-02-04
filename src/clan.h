//////////////////////////////////////////////////////////////////////////////
//
// clan.h - Character Clan Structures and Utility Functions
//
//
/// Crafters RPI
/// Copyright (C) 2015 M. C. Huston
/// Authors: M.C Huston (auroness@gmail.com)
//
// Shadows of Isildur RPI Engine++
// Copyright (C) 2005-2006 C. W. McHenry
// Authors: C. W. McHenry (traithe@middle-earth.us)
//          Jonathan W. Webb (sighentist@middle-earth.us)
// URL: http://www.middle-earth.us
//
// May includes portions derived from Harshlands
// Authors: Charles Rand (Rassilon)
// URL: http://www.harshlands.net
//
// May include portions derived under license from DikuMUD Gamma (0.0)
// which are Copyright (C) 1990, 1991 DIKU
// Authors: Hans Henrik Staerfeldt (bombman@freja.diku.dk)
//          Tom Madson (noop@freja.diku.dk)
//          Katja Nyboe (katz@freja.diku.dk)
//          Michael Seifert (seifert@freja.diku.dk)
//          Sebastian Hammer (quinn@freja.diku.dk)
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _rpie_clan_h_
#define _rpie_clan_h_

#include <stdio.h>
#include "structs.h"

#define CLAN_MEMBER		(1 << 0)
#define CLAN_LEADER		(1 << 1)
#define CLAN_MEMBER_OBJ		(1 << 2)
#define CLAN_LEADER_OBJ		(1 << 3)
#define CLAN_RECRUIT		(1 << 4)
#define CLAN_PRIVATE		(1 << 5)
#define CLAN_CORPORAL		(1 << 6)
#define CLAN_SERGEANT		(1 << 7)
#define CLAN_LIEUTENANT		(1 << 8)
#define CLAN_CAPTAIN		(1 << 9)
#define CLAN_GENERAL		(1 << 10)
#define CLAN_COMMANDER		(1 << 11)
#define CLAN_APPRENTICE		(1 << 12)
#define CLAN_JOURNEYMAN		(1 << 13)
#define CLAN_MASTER		(1 << 14)

#define MAX_CLANS		100


/* player interface */
void do_clan (CHAR_DATA * ch, char *argument, int cmd);

char *get_shared_clan (CHAR_DATA * ch, CHAR_DATA * other);
int get_clan (CHAR_DATA * ch, const char *clan, int clan_flags);
int is_clan_in_string (char *string, char *clan, int clan_flags);
int get_next_clan (char **p, char *clan_name, int *clan_flags);
char *add_clan_to_string (char *string, char *new_clan_name, int clan_flags);
char *remove_clan_from_string (char *string, char *old_clan_name);
const char *rank_name_from_flags (char* clan_name, int flags);
int clan_rank_to_value (const char *flag_names, const char *clan_name);
int get_clan_rank_level(std::string clan_name, std::string clan_rank);
int get_next_leader (char **p, char *clan_name, int *clan_flags);
bool outranks(const char *has_rank, const char *compared_rank, const char *clan);
int is_brother(CHAR_DATA * ch, CHAR_DATA *tch);
int is_area_leader (CHAR_DATA * ch);
CLAN_DATA *get_clandef (const char *clan_name);
CLAN_DATA *get_clandef_long (char *clan_long_name);
int is_clan_member (CHAR_DATA * ch, char *clan_name);
void clan_object_equip (CHAR_DATA * ch, OBJ_DATA * obj);
void clan_object_unequip (CHAR_DATA * ch, OBJ_DATA * obj);
int is_clan_member_player (CHAR_DATA * ch, char *clan_name);
int get_clan_long (CHAR_DATA * ch, char *clan_name, int clan_flags);
int get_clan_long_short (CHAR_DATA * ch, char *clan_name, int clan_flags);

void do_rollcall (CHAR_DATA * ch, char *argument, int cmd);

void clan__do_score (CHAR_DATA * ch);


void display_clan_ranks (CHAR_DATA * ch, CHAR_DATA * observer);
void clan__assert_member_objs ();	/* caller: db.c:boot_objects()                  */

/** NEW CLAN CODE **/
int char_clan_add(CHAR_DATA* ch, char *clan_name, char* clan_rank);
int char_clan_rank_change(CHAR_DATA* ch, char *clan_name, char* clan_rank);
int char_clan_remove(CHAR_DATA* ch, char *clan_name);

extern CLAN_DATA *clan_list;

	//TODO: change this when we re-do enforcer zones
inline int is_area_enforcer (CHAR_DATA * ch)
{
		// this func gets called lots but the result rarely changes
	/**
	if (!ch->enforcement[0])
		update_enforcement_array (ch);
	
	return ch->enforcement[ch->room->zone];
	 **/
	return 0;
}

#endif // _rpie_clan_h_
