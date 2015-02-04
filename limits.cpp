//////////////////////////////////////////////////////////////////////////////
//
/// limits.c : Gain Control Module
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
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "structs.h"
#include "account.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "math.h"
#include "utility.h"

extern bool pending_reboot;
extern std::map<std::string, SKILL_DATA*> skill_data_map;


	
void
check_linkdead (void)
{
	CHAR_DATA *tch;
	std::list<char_data*>::iterator tch_iterator;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	
	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		tch = *tch_iterator;
		
		if (!tch)
			continue;
		
		if (tch->deleted)
			continue;

		if (IS_NPC(tch))
			continue;
		
		if (!(tch->desc)
			&& ((tch->pc->time_last_activity + PLAYER_DISCONNECT_SECS) < mud_time))
		{
			if (tch->get_trust())
				break;

			if (tch->is_switched())
				break;
			
			sprintf (buf, "Link Dead Disconnect: %s.", tch->name);
			system_log (buf, false);
			
			if (tch->room)
			{
				do_quit (tch, "", 3);
				tch_iterator = character_list.begin();
				
			}
			
		}
	}
	
	return;	
}

void
check_idling (DESCRIPTOR_DATA * d)
{
	/* Unmark people who aren't really idle */
			 
	if (d->idle && (d->time_last_activity + PLAYER_IDLE_SECS) > mud_time)
	{
		d->idle = 0;
		return;
	}

	if (d->original)
		return;

	if (!d->character
		&& (d->time_last_activity + DESCRIPTOR_DISCONNECT_SECS) < mud_time
		&& (d->connected <= CON_ACCOUNT_MENU
		|| d->connected == CON_PENDING_DISC))
	{
		close_socket (d);
		return;
	}

	/* Disconnect those people idle for too long */

	if (d->idle && (d->time_last_activity + PLAYER_DISCONNECT_SECS) < mud_time)
	{
			//immortals can idle forever
			//even when they are animating
		if (d->character
			&& (IS_NPC (d->character) 
				|| (d->character)->get_trust()))
			return;

		if (d->original)
			return;

		/* Idle PCs in the chargen process */

		if (d->character && !d->character->room)
		{
			close_socket (d);
		}

		if (d->character 
			&& !d->connected 
			&& d->character->room)
			do_quit (d->character, "", 3);

		return;
	}

	/* Warn people who are just getting to be idle */

	if (!d->idle && (d->time_last_activity + PLAYER_IDLE_SECS) < mud_time)
	{

		if (d->character)
		{
			if (d->connected == CON_PLYNG)
				SEND_TO_Q ("Your thoughts begin to drift. #2(Idle)#0\n", d);
			else
				SEND_TO_Q
				("\nYour attention is required to prevent disconnection from the server. #2(Idle)#0\n",
				d);
		}

		d->idle = 1;
	}
}

void
check_idlers ()
{
	DESCRIPTOR_DATA *d, *d_next;

	for (d = descriptor_list; d; d = d_next)
	{
		d_next = d->next;
		check_idling (d);
	}
}

/* Update both PC's & NPC's and objects*/
void
point_update (void)
{
	int cycle_count;
	int roll;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char buf3[MAX_STRING_LENGTH]= { '\0' };
	CHAR_DATA *ch;
	AFFECTED_TYPE *af;
	struct time_info_data healing_time;
	struct time_info_data bled_time;
	struct time_info_data playing_time;
	MESSAGE_DATA *deduct_message;
	char date[32] = "";
	time_t current_time = 0;
	std::list<char_data*>::iterator tch_iterator;
	

	cycle_count = 0;

	mud_time = time (NULL);
	
	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		ch = *tch_iterator;

		if (!ch)
			continue;

		if (ch->deleted)
			continue;

		if (!ch->room)
			continue;

		
		

		if (ch->move < ch->max_move)
		{
			ch->move = MIN ((ch->move + ch->move_gain()), ch->max_move);
		}
		
		

		//Application RPP cost
		if (!IS_NPC (ch) && ch->pc->app_cost && ch->desc)
		{
			playing_time = real_time_passed (time (0) - ch->time_str.logon + ch->time_str.played, 0);
			if (playing_time.hour >= 10 && ch->desc->acct)
			{
				//restoring the deduction of rpp for races -- Huan
				//writes a message to player notes when it happens
				current_time = time (0);
				ctime_r (&current_time, date);
				if (strlen (date) > 1)
					date[strlen (date) - 1] = '\0';
				
				deduct_message = new MESSAGE_DATA;
				deduct_message->poster = "System";
				deduct_message->subject = duplicateString ("AutoDeduct RPP.");
				
				if (ch->pc->app_cost == 1)
					sprintf(buf3, "%d point was deducted for application cost", 
ch->pc->app_cost );
				else 
					sprintf(buf3, "%d points were deducted for application cost", 
ch->pc->app_cost );
					

				deduct_message->message = duplicateString (buf3);
				deduct_message->date = duplicateString (date);
				
				add_message_to_mysql_player_notes (ch->name, "System", deduct_message);		
				ch->desc->acct->pay_application_cost (ch->pc->app_cost);
				ch->pc->app_cost = 0;
				save_char (ch, true);
			}
		}

		//Remove New Player Flag
		if (!IS_NPC (ch) && IS_SET (ch->plr_flags, NEW_PLAYER_TAG))
		{
			playing_time =
				real_time_passed (time (0) - ch->time_str.logon + ch->time_str.played, 0);
			if (playing_time.hour > 12)
			{
				ch->plr_flags &= ~NEW_PLAYER_TAG;
				ch->act
					("You've been playing for over 12 hours, now; the #2(new player)#0 tag on your long description has been removed. Once again, welcome - have fun, and best of luck in your travels!\n",
					false, 0, 0, TO_CHAR | _ACT_FORMAT);
			}
		}

		
		if ((af = get_affect (ch, MAGIC_CRAFT_DELAY)))
		{
			if (time (0) >= af->a.spell.modifier)
			{
				ch->send_to_char
					("#6OOC: Your craft delay timer has expired. You may resume crafting delayed items.#0\n");
				remove_affect_type (ch, MAGIC_CRAFT_DELAY);
			}
		}

		
	}				
}

void
hourly_update (void)
{
	int current_time;
	int hours;
	int nomsg = 0;
	DESCRIPTOR_DATA *d;
	CHAR_DATA *ch;
	OBJ_DATA *obj;
	OBJ_DATA *objj;
	OBJ_DATA *next_thing2;
	ROOM_DATA *room;
	NEGOTIATION_DATA *neg;
	NEGOTIATION_DATA *new_list = NULL;
	NEGOTIATION_DATA *tmp_neg;
	char your_buf[MAX_STRING_LENGTH]= { '\0' };
	char room_buf[MAX_STRING_LENGTH]= { '\0' };
	char room_msg_buf[MAX_STRING_LENGTH]= { '\0' };
	std::list<char_data*>::iterator tch_iterator;
	std::list<obj_data*>::iterator tobj_iterator;
	
	time_t test_time;
	
	current_time = time (NULL);
	test_time = current_time;
	
		//  tests hourday to the proper hour, so this only happens once a RL day.
	
	struct tm* tp = localtime(&test_time);
	int hourday;
	int minday;
	
	hourday = tp->tm_hour;
	minday = tp->tm_min;
	
	 
	
		//12:00 noon to 12:15pm GMT or 6am EST
	
	if ((hourday == 12) && (minday < 15) && (minday > 0))
	{
		for (d = descriptor_list; d; d = d->next)
			SEND_TO_Q ("\n#2Staff Announcement:#0: The daily refresh for the server is scheduled in 15 minutes.\n\n" , d);
		
	}
	
		//12:15pm to 12:30pm GMT or 6am EST
	if ((hourday == 12) && (minday < 30) && (minday > 20))
	{
		command_interpreter(NULL, "refresh all");
	}
	
	
	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		ch = *tch_iterator;
		
			//next_ch = ch->next;
		
		if (ch->deleted || !ch->room)
			continue;
		
		
		
		if (IS_NPC (ch))
		{
				// Mighty Morphing
			if ((ch->mob->morph_time) && ch->mob->morph_time < current_time)
			{
				morph_mob (ch);
			}
			
				// Delete Old Barters
			if (IS_SET (ch->flags, FLAG_KEEPER) && ch->mob->shop)
			{
				neg = ch->mob->shop->negotiations;
				new_list = NULL;
				
				while (neg)
				{
					tmp_neg = neg->next;
					
					if (neg->time_when_forgotten <= current_time)
					{
						free_mem (neg);
					}
					else
					{
						neg->next = new_list;
						new_list = neg;
					}
					neg = tmp_neg;
				}
				ch->mob->shop->negotiations = new_list;
			}
			
		}
		else 
		{
			if (!IS_SET (ch->room->room_flags, OOC))
				ch->hunger_thirst_process();
		}
		
	}
	
	
	for (tobj_iterator = object_list.begin(); tobj_iterator != object_list.end(); tobj_iterator++)
	{
		
		
		obj = *tobj_iterator;
		
		if (!obj)
			break;
		
		
		if (obj->deleted)
			continue;
		
			// Check for morphto first, or object can be extracted accidently before call 
		if ((obj->morphTime) && obj->morphTime < current_time)
		{
			morph_obj (obj);
				//we have to go back to the begining, so we can skip the 
				//potental extracted object when it morphed, or the addition of the
				//new object since object_list.end() is no longer valid
			tobj_iterator = object_list.begin();
			continue;
		}
		
		
		if (IS_SET (obj->obj_flags.extra_flags, ITEM_TIMER)
				 && --obj->obj_timer <= 0)
		{
			
			room = (obj->in_room == NOWHERE ? NULL : vtor (obj->in_room));
			
			if (obj->carried_by)
				(obj->carried_by)->act ("$p decays in your hands.",
					 false, obj, 0, TO_CHAR);
			
			else if (room && room->people)
			{
				(room->people)->act ("$p gradually decays away.",
					 true, obj, 0, TO_ROOM);
				(room->people)->act ("$p gradually decays away.",
					 true, obj, 0, TO_CHAR);
			}
			
			for (objj = obj->contains; objj; objj = next_thing2)
			{
					// Next in inventory
				next_thing2 = objj->next_content;	
				
				obj_from_obj (&objj, 0, 0);
				
				if (obj->in_obj)
					obj_to_obj (objj, obj->in_obj);
				else if (obj->carried_by)
					obj_to_room (objj, obj->carried_by->in_room);
				else if (obj->in_room != NOWHERE)
					obj_to_room (objj, obj->in_room);
				else
				{
					extract_obj (objj);
						//we have to go back to the begining, so we can skip the extracted object
						//since it is removed, object_list.end() is no longer valid
					tobj_iterator = object_list.begin();
				}
			}
			
			
			extract_obj (obj);
				//we have to go back to the begining, so we can skip the extracted object
				//since it is removed, object_list.end() is no longer valid
			tobj_iterator = object_list.begin();
			
		}
		
		else if (obj->obj_flags.type_flag == ITEM_LIGHT &&
				 obj->o.light.on && obj->o.light.hours)
		{
			
			obj->o.light.hours--;
			
			hours = obj->o.light.hours;
			
				//for permanent light sources
			if (hours < -1)
			{
				obj->o.light.hours = -1;
				continue;
			}
			
			if (!(ch = obj->carried_by))
				ch = obj->equiped_by;
			
			switch (hours)
			{
				case 0:
					strcpy (your_buf, "Your $o burns out.");
					strcpy (room_buf, "$n's $o burns out.");
					strcpy (room_msg_buf, "$p burns out.");
					break;
					
				case 1:
					strcpy (your_buf, "Your $o is just a dim flicker now.");
					strcpy (room_buf, "$n's $o is just a dim flicker now.");
					strcpy (room_msg_buf, "$p is just a dim flicker now.");
					break;
					
				case 2:
					strcpy (your_buf, "Your $o begins to burn low.");
					strcpy (room_buf, "$n's $o begins to burn low.");
					strcpy (room_msg_buf, "$p begins to burn low.");
					break;
					
				case 10:
					strcpy (your_buf, "Your $o sputters.");
					strcpy (room_buf, "$n's $o sputters.");
					strcpy (room_msg_buf, "$p sputters.");
					break;
					
				default:
					nomsg = 1;
					break;
			}
			
			if (hours == 0 ||
				(!is_name_in_list ("candle", obj->name) && !nomsg))
			{
				
				if (ch)
				{
					ch->act(your_buf, false, obj, 0, TO_CHAR);
					ch->act(room_buf, false, obj, 0, TO_ROOM);
				}
				
				if (obj->in_room &&
					(room = vtor (obj->in_room)) && room->people)
				{
					(room->people)->act(room_msg_buf, false,  obj, 0, TO_ROOM);
					(room->people)->act(room_msg_buf, false, obj, 0, TO_CHAR);
				}
			}
			
			if (obj->o.light.hours > 0)
				continue;
			
			if (is_name_in_list ("candle", obj->name))
			{
				extract_obj (obj);
					//we have to go back to the begining, so we can skip the extracted object
					//since it is removed, object_list.end() is no longer valid
				tobj_iterator = object_list.begin();
			}
		}
		
	}
	
	
}

int
remove_room_affect (ROOM_DATA * room, int type)
{
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *free_af;

	if (!room->affects)
		return 0;

	if (room->affects->type == type)
	{
		free_af = room->affects;
		room->affects = free_af->next;
		free_mem (free_af);
		save_room_affects (room->zone);
		return 1;
	}

	for (af = room->affects; af->next; af = af->next)
	{
		if (af->next->type == type)
		{
			free_af = af->next;
			af->next = free_af->next;
			free_mem (free_af);
			
			save_room_affects (room->zone);
			return 1;
		}
	}
		return 0;
}

void
room_affect_wearoff (ROOM_DATA * room, int type)
{
	if (remove_room_affect (room, type) == 0)
		return;

	switch (type)
	{
				
	case MAGIC_ROOM_CALM:
		if (room)
			send_to_room
			("Slowly, the sense of peace dissipates, and things return to normal.\n",
			room->nVirtual);
		else
			send_to_all ("The sense of peace everwhere fades.\n");
		break;

	case MAGIC_ROOM_LIGHT:
		if (room)
			send_to_room ("The unnatural light emanations fade.\n",
			room->nVirtual);
		else
			send_to_all
			("The unnatural light emanations fade from the land.\n");
		break;

	case MAGIC_ROOM_DARK:
		if (room)
			send_to_room ("The unnatural darkness fades away.\n",
			room->nVirtual);
		else
			send_to_all ("The unnatural darkness fades from the land.\n");
		break;

	case MAGIC_WORLD_SOLAR_FLARE:
		if (room)
			send_to_room ("The localized solar flare has ended.\n",
			room->nVirtual);
		else
			send_outside ("The ball of flame in the sky slowly dies out.\n");
		break;
	}
}
/*******************************************************
 * duration = -1 means it will never expire naturally
 * duration = 0 means it expires now
 * duration greater than 0 means it checks for wear-off
 * on the next tick
 *****************************************************/
void
room_update (void)
{
	ROOM_DATA * room;
	std::map<int, ROOM_DATA*>::iterator room_iterator;
	AFFECTED_TYPE *room_affect;
	
	

	for (room_iterator = room_map.begin(); room_iterator != room_map.end(); room_iterator++)
	{
		room = room_iterator->second;
		
			// Expire affects on rooms 
		for (room_affect = room->affects;
			 room_affect;
			 room_affect = room_affect->next )
			
		{
			if ((room->affects == NULL) || room_affect== NULL)
				break;
			
			if (room_affect->a.room.duration == 0 )
				room_affect_wearoff (room, room_affect->type);
			
			else if (room_affect->a.room.duration > 0 )
				room_affect->a.room.duration--;
			
			else 
				continue;
			
		}
		
			//update light levels
		room_light(room);
	}
	
}


/* returns the situation-affected skill level */
int
skill_level (CHAR_DATA * ch, char * skill_name, int diff_mod)
{
	int ski_lev;
	OBJ_DATA *obj;
	ROOM_DATA *room;
	AFFECTED_TYPE *af;
	float condition_stat = 0.0;
	
	if (islower(skill_name[0]))
		CAP(skill_name);
	
	if (ch->skill_map[skill_name] > 0)
		ski_lev = ch->skill_map[skill_name];
	else 
		return 0;
	
		//use generic sneak in code split into two new sneak skills
		//based on terrain
	if (!str_cmp(skill_name, "Sneak"))
	{
		if ((ch->room->terrain_type == SECT_URBAN)
			|| (ch->room->terrain_type == SECT_ROAD)
			|| (ch->room->terrain_type == SECT_DOCK)
			|| (ch->room->terrain_type == SECT_CAVE))
			ski_lev = ch->skill_map["City-Sneak"];
		else
		{
			ski_lev = ch->skill_map["Scout-Sneak"];
		}
		
		if (ski_lev <= 0)
			return 0;
	}
		//if it is scout-sneak, it won't work in cities
	else if (!str_cmp(skill_name, "Scout-Sneak"))
	{
		if ((ch->room->terrain_type == SECT_URBAN)
			|| (ch->room->terrain_type == SECT_ROAD)
			|| (ch->room->terrain_type == SECT_DOCK)
			|| (ch->room->terrain_type == SECT_CAVE))
			return 0;
	}
		//if it is city-sneak, it won't work outside of the city terrains
	else if (!str_cmp(skill_name, "City-Sneak"))
	{
		if (!(ch->room->terrain_type == SECT_URBAN)
			&& !(ch->room->terrain_type == SECT_ROAD)
			&& !(ch->room->terrain_type == SECT_DOCK)
			&& !(ch->room->terrain_type == SECT_CAVE))
			return 0;
	}
	
	
	ski_lev -= diff_mod;
	
	
	if ((af = get_affect (ch, MAGIC_AFFECT_CURSE)))
		ski_lev -= af->a.spell.modifier;
	
		

	room = vtor(ch->in_room);
	
	/**
	 * Sunlight penalty applies 
	 *		if there is sunlight, outdoors 
	 *		If indoors, or no sunlgiht, then there is no penalty
	 **/
	if (get_affect(ch, AFF_SUNLIGHT_PEN) 
		&& (global_sun_light > SUN_TWILIGHT)
		&& !(ch->room->terrain_type == SECT_CAVE)
		&& !IS_SET (ch->room->room_flags, INDOORS))
	{
		float penalty = 0.0;
		
		if (weather_info[ch->room->wzone].clouds == CLEAR_SKY)
		{
			penalty = 30;
		}
		else if (weather_info[ch->room->wzone].clouds == LIGHT_CLOUDS)
		{
			penalty = 15;
		}
		else if (weather_info[ch->room->wzone].clouds == HEAVY_CLOUDS)
		{
			penalty = 10;
		}
		else if (weather_info[ch->room->wzone].clouds == OVERCAST)
		{
			penalty = 5;
		}
		
		if (ch->room->terrain_type == SECT_WOODS) {
			penalty *= 0.75;
		}
		else if (ch->room->terrain_type == SECT_FOREST) {
			penalty *= 0.5;
		}
		else if (ch->room->terrain_type == SECT_HILLS ||
				 ch->room->terrain_type == SECT_FIELD ||
				 ch->room->terrain_type == SECT_HEATH ||
				 ch->room->terrain_type == SECT_MOUNTAIN)
		{
			penalty += 5;
		}
		ski_lev -= (int)penalty;
	}
	
	
		//skill adjustments from objects
	for (obj = ch->equip; obj; obj = obj->next_content)
	{
		for (af = obj->xaffected; af; af = af->next)
		{
			if (af->a.spell.location - 10000 == (lookup_skill_id(skill_name)))
				ski_lev += af->a.spell.modifier;
		}
	}
	
	if ((obj = ch->right_hand))
	{
		for (af = obj->xaffected; af; af = af->next)
		{
			if (af->a.spell.location - 10000 == (lookup_skill_id(skill_name)))
				ski_lev += af->a.spell.modifier;
		}
	}
	
	if ((obj = ch->left_hand))
	{
		for (af = obj->xaffected; af; af = af->next)
		{
			if (af->a.spell.location - 10000 == (lookup_skill_id(skill_name)))
				ski_lev += af->a.spell.modifier;
		}
	}
	
	ski_lev = MAX (2, ski_lev);
	
	return ski_lev;
}

	//returns 0 for not having the skill at all
	//returns 0 if they do not learn from it (asleep, idle, timers)
	//returns 1 if they pass, (no learning)
	//returns 2 if they fail and learn

int
skill_use (CHAR_DATA * ch, char* skill_name, int diff_mod)
{
	double roll;
	int lv;
	int cap;
	int skill;
	int min, max;
	int timer_length;
	int skill_level_val;
	int skill_change;
	float max_gain = 0;
	
		//they failed the skill test, because there is no skill named
	if (!skill_name)
		return 0;
	
	skill = lookup_skill_id(skill_name);
	
		//they failed the skill test, because the skill doesn't exist
	if (skill < 0)
		return 0;
	
		//they failed the skill test, because they don't have the skill
	if (ch->skill_map[skill_name] < 1)
		return 0;
	
	if (!ch->is_awake())
		return 0;
	
		//no skill check for idle/disconencted PC
	if ( (!IS_NPC(ch))
		&& (!ch->desc || ch->desc->idle))	
		return 0;
	
	if (IS_SET (ch->room->room_flags, OOC))	/* No skill gain in OOC areas. */
		return 0;
	
	lv = calc_lookup (ch, REG_LV, skill);
	cap = calc_lookup (ch, REG_CAP, skill);
	
	max_gain = lookup_max_gain(skill);
	
		//if diff_mod is positive, skill_level_val is lower than the skill
		//if diff_mod is negative, skill_level_val is higher than the skill
	skill_level_val = skill_level (ch, skill_name, diff_mod);
	
	
		//the higher the skill_level_val, the easier it is to pass the test
	roll = number (1, MAX(100, skill_level_val));
	
	
		//if they roll less than thier level, they PASS the test, but don't learn from it
	if (roll <= skill_level_val)
		return 1;
	
		//they failed the skill, so they get a -chance- to learn from it
	
		//skill_change is the difference between the adjusted skill value
		// and the original skill value
		//skill_change will be positive if diff_mod is positive
		// this means a challenge
	
		//skill_change will be negative if diff_mod is negative
		//then it is not a challenge, it is easier than normal
	skill_change = ch->skill_map[skill_name] - skill_level_val;
	
		//returns a 1 if the difficulty was not high enough for them to learn
	if ((ch->skill_map[skill_name] > 50) && (skill_change < 20))
		return 1;
	
	
		//new roll to see if they actually learn
	roll = number (1, 100);
	
	if ((ch->skill_map[skill_name] < cap) && (roll <= lv))
	{
		
		if (!get_affect (ch, MAGIC_SKILL_GAIN_STOP))
		{
				//they gain 1 point
			ch->skill_map[skill_name]++;
			
				//set timers for next skill gain
				//24 hours +/- 1 hours or so
			min = 1320;
			
			max = 1440 + ch->skill_map[skill_name] + number (1, 60) - (ch->tmp_intel * 2);
			max = MIN (1600, max);
			
			timer_length = number (min, max);
			skill_record (ch, skill_name, timer_length);
			
			magic_add_affect (ch, MAGIC_SKILL_GAIN_STOP,
							  timer_length, 0, 0, 0, 0);
			
				//they failed, AND they learned
			return 2;
			
		}
		else 
		{
				//they failed, but did not learn
			return 0;
		}
		
	}
	
	return 0;
}

void
offline_stamina (CHAR_DATA * ch, int since)
{
	time_t recover_time = 0;
	int i = 0;
	int old_position;
	
	old_position = ch->get_position();
	
		//every 4 seconds, just like point_update for stamina recovery
	recover_time = (time (0) - since) /4;
	
	ch->set_position(SLEEP);
	
	for (i = 0; i < recover_time; i++)
	{
		if (ch->move < ch->max_move)
		{
			ch->move = MIN ((ch->move + ch->move_gain()), ch->max_move);
		}
	}
	
	ch->set_position(old_position);
}

void
offline_skill_train(CHAR_DATA * ch, int since)
{
	time_t train_time = 0;
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *next_af;
	
		//minutes of trainging time sepnt offline
	train_time = ((time (0) - since) / 60);
	
	for (af = ch->hour_affects; af; af = next_af)
	{
		
		next_af = af->next;
		
		if (af->type == MAGIC_SKILL_GAIN_STOP)
		{
			af->a.spell.duration = af->a.spell.duration - train_time;
			
			if (af->a.spell.duration <= 0)
				affect_remove (ch, af);
		}	
		
	}
	
	
}