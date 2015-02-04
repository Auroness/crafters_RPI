//////////////////////////////////////////////////////////////////////////////
//
/// act.movement.c : Movement Module 
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>
#include <stdexcept>
#include <map>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "group.h"
#include "utility.h"

extern double round (double x);	// from math.h
extern const char* dirs[];
extern const char* relative_dirs[];
extern const char* extended_dirs[];
extern const char* opposite_dirs[];
extern const int rev_dir[];
extern const char* verbal_speeds[];


#define TRACK_LIMIT_PER_ROOM		15

int search_sequence = 0;
int high_water = 0;
int rooms_scanned = 0;

QE_DATA *quarter_event_list = NULL;



/* NOTICE:  Set the define ENCUMBERANCE_ENTRIES in structs.h if
 more entries are added to encumberance_info.  */
/* if wt <= str * str_mult, then element applies */
const struct encumberance_info enc_tab[6] = {
	{300,		0, 0, 0, "unencumbered"},
	{500,		1, 0, -15, "lightly encumbered"},
	{800,		2, 1, -25, "encumbered"},
	{1300,		3, 2, -45, "heavily encumbered"},
	{2000,		4, 2, -60, "critically encumbered"},
	{9999900,	4, 2, -100, "immobile"},
};

const struct fatigue_data fatigue[MAX_FATIGUE_LEVEL] = {
	{5,  "Completely Exhausted"},
	{15, "Exhausted"},
	{25, "Extremely Tired"},
	{35, "Tired"},
	{45, "Somewhat Tired"},
	{55, "Winded"},
	{75, "Somewhat Winded"},
	{90, "Relatively Fresh"},
	{999,"Completely Rested"}
};




void
track_from_room (ROOM_DATA * room, TRACK_DATA * track)
{
	TRACK_DATA *temptrack;

	if (!room || !track)
		return;

	if (room->tracks == track)
	{
		room->tracks = room->tracks->next;
	}
	else
	{
		for (temptrack = room->tracks; temptrack; temptrack = temptrack->next)
		{
			if (temptrack->next == track)
			{
				temptrack->next = temptrack->next->next;
			}
		}
	}

	free_mem (track); // TRACK_DATA*
}

void
track_expiration (ROOM_DATA * room, TRACK_DATA * track)
{
	int limit;

	if (room->terrain_type == SECT_TRAIL)
		limit = 8;
	else if (room->terrain_type == SECT_MOUNTAIN
		|| room->terrain_type == SECT_SWAMP)
		limit = 12;
	else
		limit = 16;

	if (track->hours_passed > limit)
	{
		track_from_room (room, track);
	}
}

void
update_room_tracks (void)
{
	ROOM_DATA *room;
	TRACK_DATA *track, *next_track;
	std::map<int, ROOM_DATA*>::iterator room_iterator;

	for (room_iterator = room_map.begin(); room_iterator != room_map.end(); room_iterator++)
	{
		room = room_iterator->second;
		
		for (track = room->tracks; track; track = next_track)
		{
			next_track = track->next;
			track->hours_passed++;
			if (weather_info[room->wzone].state == LIGHT_RAIN)
				track->hours_passed++;
			else if (weather_info[room->wzone].state == STEADY_RAIN)
				track->hours_passed += 2;
			else if (weather_info[room->wzone].state == HEAVY_RAIN)
				track->hours_passed += 3;
			else if (weather_info[room->wzone].state == LIGHT_SNOW)
				track->hours_passed += 2;
			else if (weather_info[room->wzone].state == STEADY_SNOW)
				track->hours_passed += 4;
			else if (weather_info[room->wzone].state == HEAVY_SNOW)
				track->hours_passed += 6;
			track_expiration (room, track);
		}
	}
}


void
leave_tracks (CHAR_DATA * ch, int to_dir, int from_dir)
{
	TRACK_DATA *track;
	int bleeding = 0, i = 0;

	if (!ch || !ch->room)
		return;

	if (ch->get_trust())
		return;

	if (IS_NPC(ch) && IS_SET (ch->mob->action, ACT_FLYING))
		return;

	if (ch->room->terrain_type == SECT_ROAD
		|| ch->room->terrain_type == SECT_URBAN)
		return;

	if (ch->race == 33
		|| ch->race == 35
		|| ch->race == 42
		|| ch->race == 66
		|| ch->race == 68
		|| ch->race == 74 || ch->race == 75 || ch->race == 81 || ch->race == 85)
	{
		/* Flying Creatures */
		return;
	}

	
	
	track = new TRACK_DATA;
	track->next = NULL;
	track->race = ch->race;
	track->to_dir = to_dir;
	track->from_dir = from_dir;
	track->speed = ch->speed;
	track->flags = 0;

	
	if (!IS_NPC (ch))
		track->flags |= PC_TRACK;

	if (ch->room->tracks)
		track->next = ch->room->tracks;

	ch->room->tracks = track;

	for (track = ch->room->tracks; track; track = track->next)
	{
		i++;
		if (i > TRACK_LIMIT_PER_ROOM)
		{
			while (track->next)
				track_from_room (ch->room, track->next);
			break;
		}
	}
}



void
add_to_qe_list (QE_DATA * qe)
{
	QE_DATA *tqe;

	if (!quarter_event_list || quarter_event_list->event_time > qe->event_time)
	{
		qe->next = quarter_event_list;
		quarter_event_list = qe;
		return;
	}

	for (tqe = quarter_event_list; tqe->next; tqe = tqe->next)
		if (tqe->next->event_time > qe->event_time)
			break;

	qe->next = tqe->next;
	tqe->next = qe;
}



void
enter_room (QE_DATA * qe)
{
	OBJ_DATA *obj;
	float encumb_percent = 0.0;
	int from_dir = 0;
	int temp;
	int roomnum;
	int echo = 0;
	int observed = 0;
	ROOM_DATA *prevroom;
	ROOM_DATA *target_room;
	ROOM_EXIT_DATA *room_exit;
	CHAR_DATA *ch;
	CHAR_DATA *tch = NULL;
	CHAR_DATA *tmp_ch;
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *drag_af;
	char buf[MAX_STRING_LENGTH];
	char travel_mssg[MAX_STRING_LENGTH];
	bool toward = false, away = false;

	ch = qe->ch;

	if (!ch)
	{
		if (qe->travel_str)
			free_mem (qe->travel_str);
		free_mem (qe); // QE_DATA*
		return;
	}

	if (!is_he_there (ch, qe->from_room))
	{

		if (is_he_somewhere (ch))
			ch->clear_moves();
		if (qe->travel_str)
			free_mem (qe->travel_str);
		free_mem (qe); // QE_DATA*

		return;
	}



	if ((af = get_affect (ch, AFFECT_SHADOW)))
		af->a.shadow.edge = -1;

		//travel string
	travel_mssg[0] = '\0';
	if (qe && qe->travel_str)
	{
		strcpy (travel_mssg, qe->travel_str);
	}
	else if (ch->travel_str != NULL)
	{
		sprintf (travel_mssg, ", %s", ch->travel_str);
	}

		//entering the next room string
	if (IS_SET (qe->flags, MF_ARRIVAL))
	{

		ch->flags &= ~FLAG_ENTERING;

		if (IS_SET (ch->flags, FLAG_LEAVING))
			printf ("Leaving still set!!\n");

		if ((af = get_affect (ch, MAGIC_DRAGGER)) &&
			is_he_here (ch, (CHAR_DATA *) af->a.spell.t, 0) && number (0, 1))
			do_wake (ch, ((CHAR_DATA *) af->a.spell.t)->name, 1);

		if (af)
			affect_remove (ch, af);

		remove_affect_type (ch, MAGIC_SNEAK);

		if (ch->moves)
			initiate_move(ch);

		if (qe->travel_str)
			free_mem (qe->travel_str);
		free_mem (qe); // QE_DATA*
		qe = NULL;

		return;
	}

		//moving to the edge of this room and stopping string
	if (IS_SET (qe->flags, MF_TOEDGE))
	{

		if (!(af = get_affect (ch, AFFECT_SHADOW)))
		{
			magic_add_affect (ch, AFFECT_SHADOW, -1, 0, 0, 0, 0);

			af = get_affect (ch, AFFECT_SHADOW);

			af->a.shadow.shadow = NULL;
		}

		af->a.shadow.edge = qe->dir;

		ch->flags &= ~FLAG_LEAVING;

		if (!af->a.shadow.shadow)
			;

		else if (!is_he_somewhere (af->a.shadow.shadow))
			ch->send_to_char ("You can no longer see who you are shadowing.\n");

		else if (!could_see (ch, af->a.shadow.shadow))
		{
			sprintf (buf, "You lose sight of #5%s#0.",
				af->a.shadow.shadow->char_short());
			ch->send_to_char (buf);
		}

		if (ch->moves)
		{
			if (qe->travel_str)
				free_mem (qe->travel_str);
			free_mem (qe); // QE_DATA*
			initiate_move(ch);
		}
		else
		{
			sprintf (buf, "$n stops just to the %s.", extended_dirs[qe->dir]);
			ch->act(buf, false,  0, 0, TO_ROOM);
			if (qe->travel_str)
				free_mem (qe->travel_str);
			free_mem (qe); // QE_DATA*
		}

		return;
	}

		//sets target room and port_flags
	target_room = qe->to_room;
	
	room_exit = exit_from_portal(qe->portal, target_room->nVirtual);
	if (!room_exit)
	{
		room_exit = new room_exit_data;
		room_exit->to_room = target_room->nVirtual;	
	}
		
	
		//only applies to normal ports
	if (!IS_SET (qe->flags, MF_PASSDOOR) &&
		IS_SET (room_exit->port_flags, EX_CLOSED))
	{

		if (room_exit->keyword && strlen (room_exit->keyword))
		{
			sprintf (buf, "The %s seems to be closed.\n", room_exit->keyword);
			ch->send_to_char (buf);
		}

		else
			ch->send_to_char ("It seems to be closed.\n");

		sprintf (buf, "$n stops at the closed %s.", room_exit->keyword);
		ch->act(buf, true,  0, 0, TO_ROOM);

		ch->clear_moves();
		ch->flags &= ~FLAG_LEAVING;
		if (qe->travel_str)
			free_mem (qe->travel_str);
		free_mem (qe); // QE_DATA*

		return;
	}

 
	ch->flags &= ~FLAG_LEAVING;

	
	
		//chance to wake person being dragged
	if ((drag_af = get_affect (ch, MAGIC_DRAGGER)) &&
		is_he_here (ch, (CHAR_DATA *) drag_af->a.spell.t, 1) &&
		((CHAR_DATA *) drag_af->a.spell.t)->get_position() <= SLEEP)
	{
		((CHAR_DATA *) drag_af->a.spell.t)->char_from_room();
		((CHAR_DATA *) drag_af->a.spell.t)->char_to_room(room_exit->to_room);
	}
	else
		drag_af = NULL;

		//subded mobs get moved and stay subdued
	if (ch->is_subduer())
	{
		(ch->subdue)->char_from_room();
		(ch->subdue)->char_to_room(room_exit->to_room);
	}
	
	temp = ch->get_position();
	ch->set_position(SLEEP);		/* Hack to make subduee not hear messages */

		//shadowers move to follow you
	shadowers_shadow (ch, room_exit->to_room, qe->dir);

	ch->set_position(temp);


	roomnum = ch->in_room;
	prevroom = vtor (roomnum);

	if (!IS_SET (qe->flags, MF_SNEAK))
	{
		leave_tracks (ch, qe->dir, ch->from_dir);
	}

	
		//actual movement is made
	ch->char_from_room();
	if (prevroom->terrain_type == SECT_PIT)
	{
		while (prevroom->contents)
		{
			obj = prevroom->contents;
			obj_from_room (&obj, 0);
			obj_to_room (obj, room_exit->to_room);
			if (echo == 0)
				ch->send_to_char ("You take your belongings with you as you leave.\n");
			echo = 1;
		}
	}
	ch->char_to_room(room_exit->to_room);
	
		//start setting up for messages
	if (qe->dir == 0)
		from_dir = 2;
	else if (qe->dir == 1)
		from_dir = 3;
	else if (qe->dir == 2)
		from_dir = 0;
	else if (qe->dir == 3)
		from_dir = 1;
	else if (qe->dir == 4)
		from_dir = 5;
	else
		from_dir = 4;

	ch->from_dir = from_dir;

		

	do_look (ch, "", 1);

	/* quick scan when they enter a room */
	if (IS_SET (ch->plr_flags, QUIET_SCAN))
		do_scan (ch, "", 1);


	while (ch->sighted)
		ch->sighted = ch->sighted->next;	/* Remove list of target sightings. */


	if (ch->is_subduer())
	{
		do_look (ch->subdue, "", 1);
		if (IS_SET (ch->plr_flags, QUIET_SCAN))
			do_scan (ch, "", 1);
	}

	
		//arrival messages and echos
	qe->flags |= MF_ARRIVAL;
	ch->flags |= FLAG_ENTERING;
	qe->event_time = qe->arrive_time;
	qe->from_room = ch->room;

	if (qe->event_time <= 0)
	{
		sprintf (buf, "$n arrives from %s.", opposite_dirs[qe->dir]);
		ch->act(buf, true,  0, 0, TO_ROOM);
		enter_room (qe);
		
	}
	else //echos and test for sneaking echos
	{
			//count number of non_friendly viewers in the room who could see the sneaker
		int population = 0;
		
		for (tmp_ch = ch->room->people; tmp_ch;
			 tmp_ch = tmp_ch->next_in_room)
		{
			if (tmp_ch != ch && can_see_mob(ch, tmp_ch)
				&& !are_grouped (tmp_ch, ch))
			{
				population += 1;
			}
		}
		
		
			// if sneaking and the room is nohide, or they fail the skillcheck remove the sneak affect
			//difficulty based on number of non_friendly viewers
		if (IS_SET (qe->flags, MF_SNEAK) 
			&& (IS_SET (ch->room->room_flags, NOHIDE)
				|| (ch->would_reveal() 
					&& (skill_level (ch, "Sneak", population*5)))))
		{
			
			observed = 0;
			for (tmp_ch = ch->room->people; tmp_ch;
				 tmp_ch = tmp_ch->next_in_room)
			{
				if (tmp_ch != ch && can_see_mob(ch, tmp_ch)
					&& !are_grouped (tmp_ch, ch))
				{
					ch->send_to_char ("You are observed.\n");
					tmp_ch->send_to_char ("You notice something moving out of the side of your vision.\n");
					break;
				}
			}
			
			remove_affect_type (ch, MAGIC_HIDDEN);
			remove_affect_type (ch, MAGIC_SNEAK);
			qe->flags &= ~MF_SNEAK;
		}
		
		if (!IS_SET (qe->flags, MF_SNEAK))
		{
			
			if (IS_NPC(ch) && IS_SET (ch->mob->action, ACT_FLYING))
				sprintf (buf, "$n flies in from %s%s.", opposite_dirs[qe->dir],
						 drag_af ? ", dragging $N" : "");
			else
				sprintf (buf, "$n is entering from %s%s.", opposite_dirs[qe->dir],
						 (drag_af ? ", dragging $N" : travel_mssg));
			
			ch->act(buf, true,  0, drag_af ?
					(CHAR_DATA *) drag_af->a.spell.t : 0, TO_NOTVICT);
		}
		else
		{
			sprintf (buf, "[%s sneaking in from %s.]", ch->name,
					 opposite_dirs[qe->dir]);
			ch->act(buf, true,  0, 0, TO_NOTVICT | TO_IMMS);
			sprintf (buf, "#5%s#0 sneaks into the area.",  ch->char_short());
			buf[2] = toupper (buf[2]);
			ch->act(buf, true,  0, 0, TO_NOTVICT | TO_GROUP);
		}
		
		
		add_to_qe_list (qe);
	}

	if (ch->is_subduer())
		ch->act("$n has $N in tow.", true, 0, ch->subdue, TO_NOTVICT);

	

	if (ch->following)
		follower_catchup (ch);

}

void
process_quarter_events (void)
{
	QE_DATA *qe;

	for (qe = quarter_event_list; qe; qe = qe->next)
		qe->event_time--;

	while (quarter_event_list && quarter_event_list->event_time <= 0)
	{

		qe = quarter_event_list;

		quarter_event_list = qe->next;

		enter_room (qe);
	}
}

void
exit_room (CHAR_DATA * ch, int dir, int flags, int leave_time,
		   int arrive_time, int speed_name, int needed_movement,
		   char *travel_mssg, ROOM_EXIT_DATA* target_exit)
{
	QE_DATA *qe;
	char buf[AVG_STRING_LENGTH];
	char *speed_type_names[6] = {
		"walking",
		"wandering",
		"slowly walking",
		"jogging",
		"running",
		"sprinting"
	};
	char buf1[AVG_STRING_LENGTH];
	

	ch->flags |= FLAG_LEAVING;

	qe = new QE_DATA;
	qe->next = NULL;
	qe->ch = ch;
	qe->dir = dir;  //used for directional echos
	qe->speed_type = speed_name;
	qe->flags = flags;
	qe->from_room = ch->room;
	qe->event_time = leave_time;
	qe->arrive_time = arrive_time;
	qe->move_cost = needed_movement;
	qe->travel_str = travel_mssg;
	qe->to_room = vtor(target_exit->to_room);
	qe->portal = target_exit->portal;
	
		//travel strings
	if (ch->speed == SPEED_IMMORTAL)
	{
		sprintf (buf1, "$n leaves in a flash %s.", dirs[dir]);
		sprintf (buf, "You blast %sward.\n", dirs[dir]);
		
		ch->send_to_char (buf);
		ch->act(buf1, true, 0, 0, TO_ROOM);
	}
	else
	{

		if (ch->is_subduer())
		{
			sprintf (buf, "$n starts %s %s, dragging $N along.",
				speed_type_names[ch->speed], dirs[dir]);
			ch->act(buf, true,  0, ch->subdue, TO_NOTVICT);
		}

		
		if (IS_SET (flags, MF_SNEAK))
		{
			sprintf (buf, "[%s starts sneaking %s.]", ch->name, dirs[dir]);
			ch->act(buf, true,  0, 0, TO_NOTVICT | TO_IMMS);

			sprintf (buf, "#5%s#0 begins sneaking %sward.",  ch->char_short(),
				dirs[dir]);
			
			buf[2] = toupper (buf[2]);
			ch->act(buf, true,  0, 0, TO_NOTVICT | TO_GROUP);
		}

	}

	if (qe->event_time <= 0)
	{
		enter_room (qe);
		return;
	}

	add_to_qe_list (qe);

	followers_follow (ch, dir, leave_time, arrive_time, target_exit->portal);
}

	
	//pulls out a room_exit object
	//using dir to find target_room
	//dir is tested for fall rooms restrictions
	//Walk_time is how many pulses are needed to move a unit distance
	//Wanted_time is how fast you move, based on set speed
	//walk_time for agi=10 (average) would be 10
	//so running will be  10 * 0.5 = 5 for wanted_time
	//while trudging will be 10 * 2.5 = 25 for wanted_time

int
calc_movement_charge (CHAR_DATA * ch, int dir, int wanted_time, int flags,
					  int *speed, int *speed_name, float *walk_time, ROOM_EXIT_DATA* room_exit)
{
	char buf[MAX_STRING_LENGTH];
	float needed_movement = 0.0;
	float cost_modifier = 1.00;
	float encumb_percent = 1.00;
	float slope_modifier = 1.00;
	int true_speed = 0;
	float t;
	AFFECTED_TYPE *dragger;
	ROOM_DATA *target_room;
	ROOM_DATA *curr_room;
	float size_adj;
	
	const float move_speeds[7] = {
		1.00,
		2.50,
		1.60,
		0.66,
		0.50,
		0.33,
		1.60
	};
	
	
	if (!(curr_room = vtor (ch->in_room)))
		return 0;
	
	if (!(target_room = vtor (room_exit->to_room)))
		return 0;

	if (dir == UP 
		&& IS_SET (target_room->room_flags, FALL) 
		&& !get_affect (ch, MAGIC_AFFECT_LEVITATE)
		&& (IS_NPC(ch) && !IS_SET (ch->mob->action, ACT_FLYING)))
		return 0;

	if (dir == UP 
		&& ch->is_encumbered()
		&& !get_affect (ch, MAGIC_AFFECT_LEVITATE)
		&& (IS_NPC(ch) && !IS_SET (ch->mob->action, ACT_FLYING)))
		return 0;

		//Find the movement needed, based on terrain and adjust for size of room
		// TOEDGE means were in the room and just want to get to the edge. 
	if (IS_SET (flags, MF_TOEDGE))
	{
		needed_movement = movement_loss[curr_room->terrain_type] / 2;
		if (curr_room->room_size == ROOM_SIZE_STORAGE)
			size_adj = STORAGE_SIZE/2;
		else if (curr_room->room_size == ROOM_SIZE_DETAIL)
			size_adj = DETAIL_SIZE/2;
		else if (curr_room->room_size == ROOM_SIZE_EXPLORE)
			size_adj = EXPLORE_SIZE/2;
		else if (curr_room->room_size == ROOM_SIZE_VALLEY)
			size_adj = VALLEY_SIZE/2;

	}

		// TONEXT_EDGE means we're starting on the edge, and about to walk into the room and cross it completely to the other edge. 
	else if (IS_SET (flags, MF_TONEXT_EDGE))
	{
		needed_movement = movement_loss[target_room->terrain_type];
		if (target_room->room_size == ROOM_SIZE_STORAGE)
			size_adj = STORAGE_SIZE;
		else if (target_room->room_size == ROOM_SIZE_DETAIL)
			size_adj = DETAIL_SIZE;
		else if (target_room->room_size == ROOM_SIZE_EXPLORE)
			size_adj = EXPLORE_SIZE;
		else if (target_room->room_size == ROOM_SIZE_VALLEY)
			size_adj = VALLEY_SIZE;
	}

		// Otherwise, we're just walking from midpoint to midpoint 
		//average the cost of this room and the next room, based on terrain
	else
	{
		needed_movement = (movement_loss[curr_room->terrain_type] +
						   movement_loss[target_room->terrain_type]) / 2;
			//half of first room
		if (curr_room->room_size == ROOM_SIZE_STORAGE)
			size_adj = STORAGE_SIZE/2;
		else if (curr_room->room_size == ROOM_SIZE_DETAIL)
			size_adj = DETAIL_SIZE/2;
		else if (curr_room->room_size == ROOM_SIZE_EXPLORE)
			size_adj = EXPLORE_SIZE/2;
		else if (curr_room->room_size == ROOM_SIZE_VALLEY)
			size_adj = VALLEY_SIZE/2;
		
			//now add half of the second room
		if (target_room->room_size == ROOM_SIZE_STORAGE)
			size_adj = STORAGE_SIZE/2;
		else if (target_room->room_size == ROOM_SIZE_DETAIL)
			size_adj += DETAIL_SIZE/2;
		else if (target_room->room_size == ROOM_SIZE_EXPLORE)
			size_adj += EXPLORE_SIZE/2;
		else if (target_room->room_size == ROOM_SIZE_VALLEY)
			size_adj += VALLEY_SIZE/2;
	}

	needed_movement = needed_movement * size_adj * REALITY_FACTOR;
	
		//based on needed_movement, there is a potential for damage to worn items
	movement_wear(ch, needed_movement);
	
		//consider modifiers for dragging, and subdue, and encumberance
	if ((dragger = get_affect (ch, MAGIC_DRAGGER)))
		cost_modifier *= 1.50;
	else if (ch->is_subduer())
		cost_modifier *= 1.50;

	encumb_percent =
	1.0 - (float) ((ch->can_carry_weight() -
				   ch->carrying()) / (float) ch->can_carry_weight());
	
	if (encumb_percent > 0.95)
	{
		printf ("Huh?  %f\n",
				(float) (ch->can_carry_weight() - ch->carrying()) /
				(float) ch->can_carry_weight());
		printf ("Very encumbered pc %s in room %d\n", ch->keywords, ch->in_room);
		sprintf (buf, "You are carrying too much to move.\n");
		ch->send_to_char (buf);
		ch->clear_moves();
		return -1;
	}

	if (room_exit->slope)
	{		
			//going downhill is easier
			//we need a positive value for slope_modifier
		if (room_exit->slope < 0)
			slope_modifier = slope_modifier * (-0.5 * room_exit->slope);
		else
			slope_modifier = (slope_modifier * room_exit->slope);

			//convert to percentage penalty (1.xxx)
		slope_modifier = 1.0 + (slope_modifier / 100.0);
		
	}
	
		//speed of a group is the speed of the slowest person
	if (is_with_group (ch))
	{
		true_speed = speed_group(ch);
	}
	else
	{
		true_speed = ch->speed;
	}

		//how many time units are need
		//adjsuted for magic
	
		//  At 2 pulses a second, 5 seconds/room for a 10.5 agi (average) N/PC 
		//walk_time is stored at the pointer location for later use
	*walk_time = 0;

	*walk_time = 2 * 10.5 * 5.0 / (ch->tmp_agi ? ch->tmp_agi : 10.5);

	if (!wanted_time)
		wanted_time = (int) round (*walk_time * move_speeds[true_speed]);

	if (*walk_time == 0)
		*walk_time = 9.0;

	if (get_affect (ch, MAGIC_AFFECT_SLOW))
		*walk_time *= 1.5;

	if (get_affect (ch, MAGIC_AFFECT_SPEED))
		*walk_time *= 0.5;

	

	
		//adjust for encumberance values to get the maximum speed the character 'can' move at
		//wanted_time = walk_time * speed for unadjusted movements
		//so t becomes adjusted speed
	t = wanted_time / *walk_time;

	if (t < 2.50 && encumb_percent > 0.80)
		t = 2.50;
	else if (t < 1.60 && encumb_percent > 0.70)
		t = 1.60;
	else if (t < 1.00 && encumb_percent > 0.50)
		t = 1.00;
	else if (t < 0.66 && encumb_percent > 0.40)
		t = 0.66;
	else if (t < 0.50 && encumb_percent > 0.30)
		t = 0.50;
	else if (t < 0.33)
		t = 0.33;

		//give a name to the adjsuted speed, and store it at the pointer location
	if (t > 1.60001)
		*speed_name = SPEED_CRAWL;
	else if (t > 1.00001)
		*speed_name = SPEED_PACED;
	else if (t > 0.66001)
		*speed_name = SPEED_WALK;
	else if (t > 0.50001)
		*speed_name = SPEED_JOG;
	else if (t > 0.33001)
		*speed_name = SPEED_RUN;
	else
		*speed_name = SPEED_SPRINT;

	
	
		//now we start calcualting the actual fatigue costs of the movement
		//convert encumberance with a exponential  increase
	if (encumb_percent > .30 && encumb_percent <= .40)
		encumb_percent *= 1.25;
	else if (encumb_percent > .40 && encumb_percent <= .50)
		encumb_percent *= 2.0;
	else if (encumb_percent > .50 && encumb_percent <= .60)
		encumb_percent *= 3.75;
	else if (encumb_percent > .60 && encumb_percent <= .75)
		encumb_percent *= 6.0;
	else if (encumb_percent > .75)
		encumb_percent *= 11.25;

		//cost_modifier also included dragging and subdue
	cost_modifier = cost_modifier / t;
	cost_modifier = cost_modifier *(1.0 + encumb_percent);
	*speed = (int) round (*walk_time * t);

	if (ch->get_trust() && ch->speed == SPEED_IMMORTAL)
	{
		*speed_name = SPEED_IMMORTAL;
		needed_movement = 0;
		*speed = 0;
	}

	
	if (*speed < 0)
		*speed = 1;
	
	
	int tot_cost = (int) round ((needed_movement) * cost_modifier * slope_modifier);

	
	return (tot_cost);
}


	//sets up travel strings
	//finds a target room
	//tests target room for problems
	//calls a function to calcualte the needed movement points
	//sets the actual number of pulses we wait for before physically moving the PC
	//sets up the echos to tell everyone we are moving
void
initiate_move (CHAR_DATA * ch)
{
	int dir;
	int mflags;
	int port_chk;
	int needed_movement;
	int exit_speed;		/* Actually, time */
	int enter_speed;		/* Actually, time */
	float walk_time;
	int speed;
	int wanted_time;
	int speed_name;
	bool can_move;
	int mnt_spd;
	CHAR_DATA *tch;
	AFFECTED_TYPE *af;
	MOVE_DATA *move;
	ROOM_DATA *target_room = NULL;
	ROOM_EXIT_DATA *room_exit;
	char buf[MAX_STRING_LENGTH], suffix[25];
	char *move_names[8] = {
		"walking",
		"wandering",
		"slowly walking",
		"jogging",
		"running",
		"sprinting",
		"blasting"
	};
	char *move_names2[8] = {
		"walk",
		"wander",
		"pace",
		"jog",
		"run", 
		"sprint",
		"blast"
	};
	char buf1[MAX_STRING_LENGTH];
	char travel_mssg[MAX_STRING_LENGTH] = "";
	
	
	if (!ch->moves)
	{
		printf ("Nothing to initiate!\n");
		return;
	}
	
		//adjsut move_names and move_names2 for different races of mobs
	if (IS_NPC(ch) && IS_SET (ch->mob->profession, PROF_VEHICLE))
	{
		move_names[ch->speed] = "traveling";
		move_names2[ch->speed] = "travel";
	}
	
		//flyers
	if (ch->race == 33 || ch->race == 35 || ch->race == 66 ||
		ch->race == 68 || ch->race == 71 || ch->race == 74 ||
		ch->race == 75 || ch->race == 78 || ch->race == 81)
	{
		move_names[ch->speed] = "flying";
		move_names2[ch->speed] = "flie";	/* ("flies") */
	}
		//snakes
	else if (ch->race == 56 || ch->race == 58 || ch->race == 65)
	{
		move_names[ch->speed] = "slithering";
		move_names2[ch->speed] = "slither";
	}
	
	
	
	if (ch->get_position() != STAND)
	{
		ch->send_to_char ("You'll need to stand in order to move.\n");
		if (ch->moves)
			ch->clear_moves();
		return;
	}
	
		//now we are looking at the first move_data object in the list of moves for the character
		//pull infor from that move_data object
	
	move = ch->moves;
	ch->moves = move->next;
	
	dir = move->dir;
	mflags = move->flags;
	wanted_time = move->desired_time;
	port_chk = move->portal;
	
	if (move->travel_str)
	{
		if (!str_cmp(move->travel_str, "!"))
			sprintf (travel_mssg, ", ignoring obstacles of any kind", move->travel_str);
		else if (!str_cmp(move->travel_str, ""))
			sprintf (travel_mssg, "");
		else 
			sprintf (travel_mssg, ", %s", move->travel_str);
	}
	
		//now that we have everything, we get rid of the move_data object
	free_mem (move); // MOVE_DATA*
	move = NULL;
	
		//we create a room_exit_data object, based on the given direction	
	room_exit = is_exit_normal(ch, dir, 1); //works for normal portals
	
	if (room_exit 
		&& !(room_exit->portal == port_chk))
		room_exit = NULL;
	
		//we can move diagonal even if there is no direct connection
		//if we can scan the room, we can move there
	
		//check for diagonal normal and create a room_exit if we found a diagonal room
	if (!room_exit)
	{
			//not a normal exit, look for a diagonal
		target_room = get_diagonal_move_room (ch, dir);
		
			//we have a diagonal exit
		if (target_room)
		{
			room_exit = new room_exit_data;
			room_exit->to_room = target_room->nVirtual;			
		}
		
			//not a normal exit, not a diagonal, so we look for a special
		else 
		{
			target_room = room_from_port_dir(port_chk, dir);
			
				//we have a special exit
			if (target_room)
			{
				room_exit = new room_exit_data;
				room_exit->to_room = target_room->nVirtual;
				room_exit->portal = port_chk;
			}
			
				//we can't find any exit in that direction
			if (!target_room)
			{	
				if (ch->room->extra 
					&& !(ch->room->extra->alas[dir].empty()))
					ch->send_to_char (ch->room->extra->alas[dir].c_str());
				else
					ch->send_to_char ("No matter how hard you try, you cannot go that way.\n");
				
				ch->clear_moves();
				return;
			}
			
		}
		
	}
	
		//if we didn't find a diagonal, echo for closed doors and secrets
	else if ((IS_SET (room_exit->port_flags, EX_CLOSED) 
			  && (!ch->get_trust())))
	{
		
		if (room_exit->keyword && strlen (room_exit->keyword))
		{
			sprintf (buf, "The %s seems to be closed.\n", room_exit->keyword);
			ch->send_to_char (buf);
		}
		else
			ch->send_to_char ("It seems to be closed.\n");
		
		ch->clear_moves();
		return;
	}
		//we have a room_exit from EXIT (ch, dir);
	else
	{
		target_room = vtor (room_exit->to_room);
	}
	
	room_exit->portal = port_chk;
	
		//one way or antoher, we have target_room
		//now we test the target room for problems
		//if you are going up, is it a fall room or a climb room?
	if (dir == UP
		&& !get_affect (ch, MAGIC_AFFECT_LEVITATE)
		&& (IS_NPC(ch) && !IS_SET (ch->mob->action, ACT_FLYING)))
	{
		
		if (IS_SET (target_room->room_flags, FALL))
		{
			
			ch->send_to_char ("Too steep.  You can't climb up.\n");
			ch->clear_moves();
			return;
		}
		
	}
		//is there sufficent capacity to allow anohter to enter
		//if (!(target_room->capacity == 0))
		//{
		//	if (!room_avail(target_room, NULL, ch)
		//		&& !(ch->get_trust())
		//		&& !force_enter(ch, target_room))
		//	{
		//		ch->send_to_char("There isn't enough room for you.");
		//		ch->clear_moves();
		//		return;
		//	}
		//}
		// Case
	
	
	
		//need certain trail obejcts in the room to move
	if (IS_NPC(ch) && any_are_set(ch->mob->profession, PROF_VEHICLE)
		&& (target_room->terrain_type >= SECT_PIT
			|| target_room->terrain_type == SECT_SWAMP
			|| target_room->terrain_type == SECT_MOUNTAIN
			|| target_room->terrain_type == SECT_FOREST))
	{
		can_move = false;
		
		for (OBJ_DATA *tobj = target_room->contents; tobj; tobj = tobj->next_content)
		{
			if (tobj->nVirtual == VNUM_TRAIL_OBJECT_1 || tobj->nVirtual == VNUM_TRAIL_OBJECT_2 )
			{
				can_move = true;
				break;
			}
		}
		
		if (!can_move)
		{
			ch->act("You can't go there.", true, 0, 0, TO_CHAR);
			
			ch->clear_moves();
			return;
		}
	}
	
	
	
		//find actual number of pulses we need to wait before physically moving the PC
	
		//we send in the PC, the direction we want, flags that are needed and the function will change the values of speed, speed_name, and walk_time point to.
	needed_movement =
	calc_movement_charge (ch, dir, wanted_time, mflags, &speed, &speed_name,
						  &walk_time, room_exit);
	
		// Move failed for some reason, i.e. too encumbered, or moving too fast
		//actually never happens, values are always positive, but leave this in
		//for a sanity check in the future
	
	
	if (needed_movement == -1)
	{
		ch->clear_moves();
		return;
	}
	
	int exhausted;
	exhausted = (ch->move < needed_movement);
	
	if (exhausted)
	{
		ch->send_to_char ("You are too exhausted.\n");
		ch->clear_moves();
		return;
	}
	
		//speed was set when in calc_movement_charge
	if (speed == -1)
	{
		if (ch->is_subduer())
		{
			sprintf (buf, "You start %s %s, dragging $N.",
					 move_names[speed_name], dirs[dir]);
			ch->act(buf, true,  0, ch->subdue, TO_CHAR);
			sprintf (buf, "$N drags you with $M to the %s.", dirs[dir]);
			(ch->subdue)->act(buf, true, 0, ch, TO_CHAR);
		}
	}
	
	if (IS_SET (mflags, MF_TOEDGE))
	{
		exit_speed = speed;
		enter_speed = 0;
		sprintf (travel_mssg, ", then stopping");
	}
	
	else if (IS_SET (mflags, MF_TONEXT_EDGE))
	{
		exit_speed = 1;
		enter_speed = speed;
		sprintf (travel_mssg, ", then stopping");
	}
	
	else
	{
		exit_speed = (speed + 1) / 2;
		enter_speed = speed / 2;
	}
	
	
	if (IS_SET (mflags, MF_SNEAK))
	{
		sprintf (buf, "You cautiously begin sneaking %sward.\n", dirs[dir]);
		
		ch->send_to_char (buf);
		exit_speed += 4;
		if (ch->speed > 2)
			ch->speed = 2;
	}
	
	
	if (IS_NPC(ch) && IS_SET (ch->mob->profession, PROF_VEHICLE))
	{
		move_names[ch->speed] = duplicateString ("travelling");
		travel_mssg[0] = '\0';	/* override travel strings in vehicles */
	}
	else if ((strlen (travel_mssg) == 0) && (ch->travel_str != NULL))
	{
		sprintf (travel_mssg, ", %s", ch->travel_str);
	}
	
	sprintf (suffix, "ward");
	
		//adjust group speed
	int true_speed;
	if (is_with_group (ch))
	{
		true_speed = speed_group(ch);
	}
	else
	{
		true_speed = ch->speed;
	}
	
		//set up echos that you are now moving
	if (((IS_SET (ch->room->room_flags, INDOORS)
		  || (ch->room->terrain_type == SECT_CAVE))
		 && !IS_SET (mflags, MF_SNEAK)
		 && ch->speed != SPEED_IMMORTAL)
		|| (ch->get_trust()
			&& ch->speed != SPEED_IMMORTAL
			&& !IS_SET (mflags, MF_SNEAK)))
	{
		sprintf (buf, "You begin %s %s%s%s.\n", move_names[true_speed],
				 dirs[dir], suffix, travel_mssg);
		sprintf (buf1, "$n begins %s %s%s%s.", move_names[true_speed], dirs[dir],
				 suffix, travel_mssg);
		ch->send_to_char (buf);
		ch->act(buf1, true, 0, 0, TO_ROOM | _ACT_FORMAT);
	}
	
		//weather related changes to the announcement echo
	if ((!IS_SET (ch->room->room_flags, INDOORS)
		 && (ch->room->terrain_type != SECT_CAVE))
		&& (!ch->get_trust())
		&& !IS_SET (mflags, MF_SNEAK)
		&& !ch->is_subduer())
	{
		switch (weather_info[ch->room->wzone].state)
		{
			case LIGHT_RAIN:
				exit_speed += 2;
				needed_movement = (int) round (((float) needed_movement) * 1.2);
				sprintf (buf, "You %s %s%s through the light rain%s.\n",
						 move_names2[true_speed], dirs[dir], suffix, travel_mssg);
				sprintf (buf1, "$n %ss %s%s through the light rain%s.",
						 move_names2[true_speed], dirs[dir], suffix, travel_mssg);
				ch->send_to_char (buf);
				ch->act(buf1, true, 0, 0, TO_ROOM);
				break;
			case STEADY_RAIN:
				exit_speed += 4;
				needed_movement = (int) round (((float) needed_movement) * 1.5);
				sprintf (buf, "You %s %s%s through the rain%s.\n",
						 move_names2[true_speed], dirs[dir], suffix, travel_mssg);
				sprintf (buf1, "$n %ss %s%s through the rain%s.",
						 move_names2[true_speed], dirs[dir], suffix, travel_mssg);
				ch->send_to_char (buf);
				ch->act(buf1, true,  0, 0, TO_ROOM);
				break;
			case HEAVY_RAIN:
				exit_speed += 6;
				needed_movement *= 2;
				sprintf (buf,
						 "You %s %s%s, struggling through the lashing rain%s.\n",
						 move_names2[true_speed], dirs[dir], suffix, travel_mssg);
				sprintf (buf1,
						 "$n %ss %s%s, struggling through the lashing rain%s.",
						 move_names2[true_speed], dirs[dir], suffix, travel_mssg);
				ch->send_to_char (buf);
				ch->act(buf1, true,  0, 0, TO_ROOM);
				break;
			case LIGHT_SNOW:
				needed_movement = (int) round (((float) needed_movement) * 1.5);
				sprintf (buf, "You %s %s%s through the light snowfall%s.\n",
						 move_names2[true_speed], dirs[dir], suffix, travel_mssg);
				sprintf (buf1, "$n %ss %s%s through the light snowfall%s.",
						 move_names2[true_speed], dirs[dir], suffix, travel_mssg);
				ch->send_to_char (buf);
				ch->act(buf1, true,  0, 0, TO_ROOM);
				break;
			case STEADY_SNOW:
				exit_speed += 8;
				needed_movement *= 2;
				sprintf (buf, "You %s %s%s through the steadily falling snow%s.\n",
						 move_names2[true_speed], dirs[dir], suffix, travel_mssg);
				sprintf (buf1, "$n %ss %s%s through the steadily falling snow%s.",
						 move_names2[true_speed], dirs[dir], suffix, travel_mssg);
				ch->send_to_char (buf);
				ch->act(buf1, true,  0, 0, TO_ROOM);
				break;
			case HEAVY_SNOW:
				exit_speed += 12;
				needed_movement *= 4;
				sprintf (buf,
						 "You %s %s%s, struggling through the shrieking snow%s.\n",
						 move_names2[true_speed], dirs[dir], suffix, travel_mssg);
				sprintf (buf1,
						 "$n %ss %s%s, struggling through the shrieking snow%s.",
						 move_names2[true_speed], dirs[dir], suffix, travel_mssg);
				ch->send_to_char (buf);
				ch->act(buf1, true,  0, 0, TO_ROOM);
				break;
			default:
				sprintf (buf, "You begin %s %s%s%s.\n", move_names[true_speed],
						 dirs[dir], suffix, travel_mssg);
				sprintf (buf1, "$n begins %s %s%s%s.", move_names[true_speed],
						 dirs[dir], suffix, travel_mssg);
				ch->send_to_char (buf);
				ch->act(buf1, true,  0, 0, TO_ROOM);
				break;
		}
	}
	
		//vehicle announcments
	if (IS_NPC(ch) && IS_SET (ch->mob->profession, PROF_VEHICLE))
	{
		sprintf (buf, "%s",  ch->char_short());
		*buf = toupper (*buf);
		sprintf (buf1, "#5%s#0 begins %s %s%s.\n", buf, move_names[true_speed],
				 dirs[dir], suffix);
		send_to_room (buf1, ch->mob->nVirtual);
		needed_movement = 0;
	}
	
		//now we actually leave the room, and we need the PC, 
		//dir (so we know what room we are going to), speeds, movement costs 
		//and travel string, and the room exit data object with the room info.
	exit_room (ch, dir, mflags, exit_speed, enter_speed, speed_name,
			   needed_movement,
			   strlen (travel_mssg) ? duplicateString (travel_mssg) : NULL,
			   room_exit);
	return;
}
	//checks for flags, then creates a move_data object
	//argument is the travel string in parensthese
	//dir is the numerical direction from do_move comamnds
	//speed is used by followers
void
move (CHAR_DATA * ch, char *argument, int dir, int speed, int portal)
{
	MOVE_DATA *move;
	MOVE_DATA *tmove;
	QE_DATA *qe;
	CHAR_DATA *tch = NULL;
	char buf[MAX_STRING_LENGTH] = "";
	
	//create a move_data object using info we have so far
	move = new MOVE_DATA;
	move->next = NULL;
	move->dir = dir;
	move->desired_time = speed;
	move->travel_str = argument;
	move->portal = portal;
	
		// adding various tags
		//sneak tag
	if (get_affect (ch, MAGIC_SNEAK))
		move->flags = MF_SNEAK;
	else
	{
		move->flags = 0;
		if (!ch->mob || !IS_SET (ch->affected_by, AFF_HIDE))
			remove_affect_type (ch, MAGIC_HIDDEN);
	}
	
	argument = one_argument(argument, buf);
		
	
		//imm can ignore door restrictions
	if ((ch->get_trust()) && (!str_cmp (buf, "!")))
		move->flags |= MF_PASSDOOR;
	
		//just moving to the edge, not entering the other room at all
	if (!str_cmp (buf, "stand"))	
		move->flags = MF_TOEDGE;
	
		//if he doesn't have any movement yet, and he is not entering or leaving (from turning around) then add this move_data object and proceed with initiate_move()
	if (!ch->moves)
	{
		
		for (qe = quarter_event_list; qe && qe->ch != ch; qe = qe->next)
			;
		
		if (qe && qe->dir == rev_dir[dir] && GET_FLAG (ch, FLAG_LEAVING))
		{
			
			ch->send_to_char ("You turn around.\n");
			ch->act("$n changes directions and returns.", true, 0, 0,
				 TO_ROOM | _ACT_FORMAT);
			
			qe->dir = dir;
			qe->event_time = qe->arrive_time;
			
			qe->flags |= MF_ARRIVAL;
			ch->flags &= ~FLAG_LEAVING;
			ch->flags |= FLAG_ENTERING;
			
				//you turn around
			for (tch = ch->room->people; tch; tch = tch->next_in_room)
			{
				if (!IS_NPC (tch))
					continue;
				if (tch->following != ch)
					continue;
				for (qe = quarter_event_list; qe && qe->ch && qe->ch != tch;
					 qe = qe->next)
					;
				
				tch->send_to_char ("You turn around.\n");
				tch->act("$n changes directions and returns.", true, 0, 0,
					 TO_ROOM | _ACT_FORMAT);
				
				if (qe)
				{
					qe->dir = dir;
					qe->event_time = qe->arrive_time;
					
					qe->flags |= MF_ARRIVAL;
					tch->flags &= ~FLAG_LEAVING;
					tch->flags |= FLAG_ENTERING;
				}
			}
			
			return;
		}
		
		ch->moves = move;
		
		if (!GET_FLAG (ch, FLAG_LEAVING) && !GET_FLAG (ch, FLAG_ENTERING))
			initiate_move(ch);
		
		return;
	}
	
	
	for (tmove = ch->moves; tmove->next;)
		tmove = tmove->next;
	
	tmove->next = move;
	return;
}

	//dir is given as an int.
	//argument holds the travel string and any keywords
	//tests for things that can prevent movement
	//north "(travel string) red" --> do_move(ch, "(travel string) red", n)
	//move with a travel string to the north exit with red as a keyword
	//sent to move(ch, '(travel string)', dir, [portal with red as keyword])  
void
do_move (CHAR_DATA * ch, char *argument, int dir)
{
	AFFECTED_TYPE *af;
	CHAR_DATA *tch;
	char buf[AVG_STRING_LENGTH];
	char tbuf[AVG_STRING_LENGTH];
	char *travel = NULL;
	char *keyword = NULL;
	bool travel_check = false;
	bool keyword_check = false;
	bool may_sneak = true;		//default is able to sneak
	int i, j;
	int port_ident;
	int side;
	ROOM_PORTAL_DATA * tport;
	std::list<CHAR_DATA*>::iterator tch_iterator;
	char command[AVG_STRING_LENGTH];  //used for fall room message
	std::multimap<int,ROOM_EXIT_DATA*>::iterator iter;
	
		//is he paralzyed, asleep, or restrained?
	if (!ch->can_move())
	{
		if (get_affect (ch, MAGIC_SNEAK))
			remove_affect_type (ch, MAGIC_SNEAK);
		return;
	}
	
		//tests if PC are crafting - NPC should not be able to craft currently
	if ( !IS_NPC(ch) )
	{
		for (af = ch->hour_affects; af; af = af->next)
		{
			if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST
				&& af->a.craft->timer)
			{
				ch->send_to_char ("You'll need to stop crafting first.\n");
				return;
			}
		}
	}
	
		// If you are trying to move, bag the pmote 
	ch->clear_pmote();
	
		//check for travel strings and keyword	
	if (*argument == '(')
	{
		travel = new char[strlen(argument) + 1];
		sprintf (buf, "%s", argument);
		i = 1;
		j = 0;
		travel[j] = '\0';
		while (buf[i] != ')')
		{
			travel_check = true;
			if (buf[i] == '\0')
			{
				ch->send_to_char
				("Exactly how is it that you are trying to move?\n");
				ch->clear_moves();
				return;
			}
			if (buf[i] == '*')
			{
				ch->send_to_char
				("Unfortunately, the use of *object references is not allowed while moving.\n");
				ch->clear_moves();
				if (get_affect (ch, MAGIC_SNEAK))
					remove_affect_type (ch, MAGIC_SNEAK);
				return;
			}
			if (buf[i] == '~')
			{
				ch->send_to_char
				("Unfortunately, the use of ~character references is not allowed while moving.\n");
				ch->clear_moves();
				if (get_affect (ch, MAGIC_SNEAK))
					remove_affect_type (ch, MAGIC_SNEAK);
				return;
			}
			travel[j++] = buf[i++];
			travel[j] = '\0';
		}
			//pull off rest of buf for keyword
		j = 0;
		keyword = new char[strlen(buf) + 1];
		keyword[j] = '\0';
		while (buf[i] != '\0')
		{
			keyword_check = true;
			if (buf[i] == ')')
			{
				i++;
				continue;
			}
			keyword[j++] = buf[i++];
			keyword[j] = '\0';
		}
	
	}
	else
	{
		keyword = argument;
	}
	
		

		//is it a fall room?
	sprintf(command, "%s", dirs[dir]);
	iter = ch->room->exitmap.find(dir);
			
	if (iter != ch->room->exitmap.end()
		&& vtor(iter->second->to_room)
		&& IS_SET (vtor(iter->second->to_room)->room_flags, FALL)
		&& (IS_NPC(ch) && !IS_SET (ch->mob->action, ACT_FLYING))
		&& *argument != '!')
	{
		sprintf (buf,
				 "#6Moving in that direction will quite likely result in a rather nasty fall. If you're sure about this, type \'%s !\' to confirm.#0",
				 command);
		ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
		if (get_affect (ch, MAGIC_SNEAK))
			remove_affect_type (ch, MAGIC_SNEAK);
		return;
	}

		
	
		//test for barriers and crossings here. 
		//if they succeed, they continue on with movement
		//portal data is passed to move function as the last argument
		//normal, non-portal movment - move (ch, argument, dir, 0, 0);
		//special portal movement - move (ch, argument, dir, 0, port_ident);
	
	port_ident = keyword_to_portal(ch, dir, keyword);
	tport = vtop(port_ident);
	side = port_side_room(ch->room, tport);
	if (tport && IS_SET (tport->port_flags, EX_NOSNEAK))
		may_sneak = false;
	
		//we didn't specify a portal
	if (!tport)
	{
		
			//if a portal exists that needs a keyword, ask for the keyword
			//we know it needs a keyword because it is not in dir_option
		iter = ch->room->exitmap.find(dir);
		if (iter != ch->room->exitmap.end() && !ch->room->dir_option[dir])
		{
			
			ch->send_to_char("Where are you trying to go?\n");
			return;
		}
			//there is a normal exit that does not need a keyword
		else if (ch->room->dir_option[dir])
		{
			port_ident = ch->room->dir_option[dir]->portal;
			if (IS_SET (ch->room->dir_option[dir]->port_flags, EX_NOSNEAK))
				may_sneak = false;

		}
			//there is no portal at all, so give alas echos
		else 
		{
			if (ch->room->extra 
				&& !(ch->room->extra->alas[dir].empty()))
				ch->send_to_char (ch->room->extra->alas[dir].c_str());
			else
				ch->send_to_char ("No matter how hard you try, you cannot go that way.\n");
			return;
		}
		
	}
		//we specified a portal, now find it's type and react to it
	else if ((tport->type == PORTAL_SPACE) 
			 || (tport->type == PORTAL_DOOR)
			 || (tport->type == PORTAL_GATE))
	{
		if (IS_SET (tport->port_flags, EX_NOSNEAK))
			may_sneak = false;
		
	}
	else if (tport->type == PORTAL_BARRIER)
	{
		if (!*keyword)
		{
			ch->send_to_char("What are you trying to get past?\n");
			return;
		}
		
		if (IS_SET (tport->port_flags, EX_NOSNEAK))
			may_sneak = false;
		
		if (past_barrier(ch, tport, may_sneak))
		{
			
			for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
			{
				tch = *tch_iterator;
				if (tch->deleted)
					continue;
				
				if (tch->following == ch)
				{
					if (!past_barrier(tch, tport, may_sneak))
					{
						if (side == 1)
						{
							sprintf(tbuf, "You decide you will have to %s to get past %s. However, you cannot make it.\n", lookup_skill_name(tport->skill), tport->sdesc_1);
							tch->act(tbuf, false, 0, 0, TO_CHAR | _ACT_FORMAT);
							
							sprintf (tbuf, "$n fails to %s to get past %s.", lookup_skill_name(tport->skill),  tport->sdesc_1);
							tch->act(tbuf, false, 0, 0, TO_ROOM | _ACT_FORMAT);
						}
						
						else if (side == 2)
						{
							sprintf(tbuf, "You decide you will have to %s to get past %s. However, you cannot make it.\n", lookup_skill_name(tport->skill), tport->sdesc_2);
							tch->act(tbuf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
							
							sprintf (tbuf, "$n fails to %s to get past %s.", lookup_skill_name(tport->skill),  tport->sdesc_2);
							tch->act(tbuf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
						}
						else
						{
							sprintf(tbuf, "You decide you will have to %s to get past. However, you cannot make it.\n", lookup_skill_name(tport->skill));
							tch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
							
							sprintf (tbuf, "$n fails to %s to get past the barrier.", lookup_skill_name(tport->skill));
							tch->act(tbuf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
						}
						tch->clear_moves();
						tch->clear_current_move();
						if (tch->following == ch)
							tch->following = 0;			
						
					}
				}
			}
		}
		else 
		{
			if (side == 1)
			{
				sprintf(tbuf, "You decide you will have to %s to get past %s. However, you cannot make it.\n", lookup_skill_name(tport->skill), tport->sdesc_1);
				ch->act(tbuf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
				
				sprintf (tbuf, "$n fails to %s to get past %s.", lookup_skill_name(tport->skill),  tport->sdesc_1);
				ch->act(tbuf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
			}
			else if (side == 2)
			{
				sprintf(tbuf, "You decide you will have to %s to get past %s. However, you cannot make it.\n", lookup_skill_name(tport->skill), tport->sdesc_2);
				ch->act(tbuf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
				
				sprintf (tbuf, "$n fails to %s to get past %s.", lookup_skill_name(tport->skill),  tport->sdesc_2);
				ch->act(tbuf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
			}
			else
			{
				sprintf(tbuf, "You decide you will have to %s to get past. However, you cannot make it.\n", lookup_skill_name(tport->skill));
				ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
				
				sprintf (tbuf, "$n fails to %s to get past the barrier.", lookup_skill_name(tport->skill));
				ch->act(tbuf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
			}
			return;
		}
		
	}

			
	else if (tport->type == PORTAL_CROSSING)
	{
		if (!*keyword)
		{
			ch->send_to_char("What are you trying to get past?\n");
			return;
		}
		
		if (IS_SET (tport->port_flags, EX_NOSNEAK))
			may_sneak = false;
		
		if (past_crossing(ch, tport, may_sneak))
		{
			for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
			{
				tch = *tch_iterator;
				if (tch->deleted)
					continue;
				
				if (tch->following == ch)
				{
					if (!past_crossing(tch, tport, may_sneak))
					{
						
						if (side == 1)
						{
							sprintf(tbuf, "You decide you will have to %s to get past %s. However, you cannot make it.\n", lookup_skill_name(tport->skill), tport->sdesc_1);
							tch->act(tbuf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
							
							sprintf (tbuf, "$n fails to %s to get past %s.", lookup_skill_name(tport->skill),  tport->sdesc_1);
							tch->act(tbuf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
						}
						
						else if (side == 2)
						{
							sprintf(tbuf, "You decide you will have to %s to get past %s. However, you cannot make it.\n", lookup_skill_name(tport->skill), tport->sdesc_2);
							tch->act(tbuf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
							
							sprintf (tbuf, "$n fails to %s to get past %s.", lookup_skill_name(tport->skill),  tport->sdesc_2);
							tch->act(tbuf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
						}
						else
						{
							sprintf(tbuf, "You decide you will have to %s to get past. However, you cannot make it.\n", lookup_skill_name(tport->skill));
							tch->act(tbuf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
							
							sprintf (tbuf, "$n fails to %s to get across.", lookup_skill_name(tport->skill));
							tch->act(tbuf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
						}
						
						tch->clear_moves();
						tch->clear_current_move();
						if (tch->following == ch)
							tch->following = 0;
					}
				}
			}
		}
		else
		{
			if (tport->fail_room > 0 && vtor(tport->fail_room))
			{
				sprintf(tbuf, "\nYou decide you will have to %s to get past %s. However, you slip and fall.\n\n", lookup_skill_name(tport->skill), tport->sdesc_1);
				ch->act(tbuf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
				
				sprintf(tbuf, "$n slips and falls as they try to get past %s.\n", tport->sdesc_1);
				ch->act(tbuf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
				
				fallen_result(ch, tport);
				return;
				
			}
			else if (side == 1)
			{
				sprintf(tbuf, "You decide you will have to %s to get past %s. However, you cannot make it.\n", lookup_skill_name(tport->skill), tport->sdesc_1);
				ch->act(tbuf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
				
				sprintf (tbuf, "$n fails to %s to get past %s.", lookup_skill_name(tport->skill),  tport->sdesc_1);
				ch->act(tbuf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
			}
			
			else if (side == 2)
			{
				sprintf(tbuf, "You decide you will have to %s to get past %s. However, you cannot make it.\n", lookup_skill_name(tport->skill), tport->sdesc_2);
				ch->act(tbuf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
				
				sprintf (tbuf, "$n fails to %s to get past %s.", lookup_skill_name(tport->skill),  tport->sdesc_2);
				ch->act(tbuf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
			}
			else
			{
				sprintf(tbuf, "You decide you will have to %s to get past. However, you cannot make it.\n", lookup_skill_name(tport->skill));
				ch->act(tbuf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
				
				sprintf (tbuf, "$n fails to %s to get across.", lookup_skill_name(tport->skill));
				ch->act(tbuf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
			}
			
			return;
			
		}
	}
	
	else if (tport->type == PORTAL_TRANSPORT)
	{
		if (!*keyword)
		{
			ch->send_to_char("Where are you trying to go?\n");
			return;
		}

		if (IS_SET (tport->port_flags, EX_NOSNEAK))
			may_sneak = false;
		
		for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
		{
			tch = *tch_iterator;
			if (tch->deleted)
				continue;
			
			if (tch->following == ch || tch == ch)
			{				
				past_transport(tch, tport, may_sneak);
			}
		}
		return;	
		
	}
	
	if (str_cmp(argument, "!"))
	{
		if(keyword)
		{
				
				//initiate_move generate a proper travel string
			if (!str_cmp(keyword, "stand"))
				argument = duplicateString("then stopping.");
			else
				argument = duplicateString(travel);
		
		}
		
		if (travel_check && travel)
			free_mem(travel);
		if (keyword_check && keyword)
			free_mem(keyword);
		
	}
	
	if (!may_sneak 
		|| ((get_affect (ch, MAGIC_SNEAK))
			&& (!skill_level (ch, "Sneak", 0))))
	{
		if(get_affect (ch, MAGIC_SNEAK))
			ch->send_to_char("You aren't certain if you are seen. ");
		
		remove_affect_type (ch, MAGIC_SNEAK);
		remove_affect_type (ch, MAGIC_HIDDEN);
	}
		
		
	move (ch, argument, dir, 0, port_ident);
	
	return;	
}


void
do_east (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, EAST);
}

void
do_west (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, WEST);
}

void
do_north (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, NORTH);
}

void
do_south (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, SOUTH);
}
void
do_northeast (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, NORTHEAST);
}

void
do_northwest (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, NORTHWEST);
}

void
do_southeast (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, SOUTHEAST);
}

void
do_southwest (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, SOUTHWEST);
}

void
do_up (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, UP);
}

void
do_down (CHAR_DATA * ch, char *argument, int cmd)
{
	do_move (ch, argument, DOWN);
}

int
find_door (CHAR_DATA * ch, char *type, char *dir)
{
	char buf[MAX_STRING_LENGTH];
	int door;

	dir = strdup(convert_dir(dir));
	
	if (*dir)			// a direction was specified 
	{
			//finds a direction value based on the direction char given by *dir
		if ((door = search_block (dir, dirs, false)) == -1)	// Partial Match
		{
			ch->send_to_char ("That's not a direction.\n");
			return (-1);
		}

		if (ch->room->dir_option[door])
		{
			if (ch->room->dir_option[door]->keyword && strlen (ch->room->dir_option[door]->keyword))
			{
				if (isname (type, ch->room->dir_option[door]->keyword))
				{
					return (door);
				}
				else
				{
					sprintf (buf, "I see no %s there.\n", type);
					ch->send_to_char (buf);
					return (-1);
				}
			}
			else
			{
				return (door);
			}
		}
		else
		{
			ch->send_to_char ("There is nothing to open or close there.\n");
			return (-1);
		}
	}
	else				/* try to locate the keyword */
	{
		for (door = 0; door <= LAST_DIR; door++)
		{
			if (ch->room->dir_option[door])
			{
				if (ch->room->dir_option[door]->keyword && strlen (ch->room->dir_option[door]->keyword))
				{
					if (isname (type, ch->room->dir_option[door]->keyword))
					{
						return (door);
					}
				}
			}
		}
		sprintf (buf, "I see no %s here.\n", type);
		ch->send_to_char (buf);
		return (-1);
	}
}


void
do_open (CHAR_DATA * ch, char *argument, int cmd)
{
	int door, other_room;
	char buffer[MAX_STRING_LENGTH];
	char type[MAX_INPUT_LENGTH];
	char dir[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	ROOM_EXIT_DATA *back_exit;
	OBJ_DATA *obj;
	CHAR_DATA *victim;

	argument_interpreter (argument, type, dir);

	if (!*type)
	{
		ch->send_to_char ("Open what?\n");
	}

	if (generic_find (type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
	{

		/* this is an object */

		if (obj->obj_flags.type_flag == ITEM_BOOK)
		{
			if (obj->open)
			{
				ch->send_to_char ("It's already opened.\n");
				return;
			}
			if (!*dir || atoi (dir) <= 1)
			{
				/* load on demand */
				if (obj->o.od.value[0] && !obj->writing)
				{
					load_writing(obj);
				}

				if (!obj->writing || !obj->o.od.value[0])
				{
					sprintf (buf,
						"You open #2%s#0, and notice that it has no pages.\n",
						obj->short_description);
					ch->send_to_char (buf);
					sprintf (buf, "%s#0 opens #2%s#0.",  ch->char_short(),
						obj->short_description);
					sprintf (buffer, "#5%s", CAP (buf));
					ch->act(buffer, false, obj, 0, TO_ROOM | _ACT_FORMAT);
					obj->open = 1;
					return;
				}
				sprintf (buf, "You open #2%s#0 to the first page.\n",
					obj->short_description);
				ch->send_to_char (buf);
				sprintf (buf, "%s#0 opens #2%s#0 to the first page.",
					 ch->char_short(), obj->short_description);
				sprintf (buffer, "#5%s", CAP (buf));
				ch->act(buffer, false, obj, 0, TO_ROOM | _ACT_FORMAT);
				obj->open = 1;
				return;
			}
			else if (atoi (dir) > obj->o.od.value[0])
			{
				ch->send_to_char ("There aren't that many pages in the book.\n");
				return;
			}
			else
			{
				sprintf (buf, "You open #2%s#0 to page %d.\n",
					obj->short_description, atoi (dir));
				ch->send_to_char (buf);
				sprintf (buf, "%s#0 opens #2%s#0.",  ch->char_short(),
					obj->short_description);
				sprintf (buffer, "#5%s", CAP (buf));
				ch->act(buffer, false, obj, 0, TO_ROOM | _ACT_FORMAT);
				obj->open = atoi (dir);
				return;
			}
		}
		
		if (obj->obj_flags.type_flag != ITEM_CONTAINER)
			ch->send_to_char ("That's not a container.\n");
		else if (!IS_SET (obj->o.container.flags, CONT_CLOSED))
			ch->send_to_char ("But it's already open!\n");
		else if (!IS_SET (obj->o.container.flags, CONT_CLOSEABLE))
			ch->send_to_char ("You can't do that.\n");
		else if (IS_SET (obj->o.container.flags, CONT_LOCKED))
			ch->send_to_char ("It seems to be locked.\n");
		else
		{
			obj->o.container.flags &= ~CONT_CLOSED;
			ch->act("You open $p.", false, obj, 0, TO_CHAR | _ACT_FORMAT);
			ch->act("$n opens $p.", false, obj, 0, TO_ROOM | _ACT_FORMAT);
		}
	}
	else if ((door = find_door (ch, type, dir)) >= 0)
	{

		/* perhaps it is a door/gate */

		
		if (!IS_SET (ch->room->dir_option[door]->port_flags, EX_ISDOOR)
			&& !IS_SET(ch->room->dir_option[door]->port_flags, EX_ISGATE))
			ch->send_to_char ("That's impossible, I'm afraid.\n");
		else if (!IS_SET (ch->room->dir_option[door]->port_flags, EX_CLOSED))
			ch->send_to_char ("It's already open!\n");
		else if (IS_SET (ch->room->dir_option[door]->port_flags, EX_LOCKED))
			ch->send_to_char ("It seems to be locked.\n");
		else
		{
			ch->room->dir_option[door]->port_flags &= ~EX_CLOSED;
			vtop(ch->room->dir_option[door]->portal)->port_flags &= ~EX_CLOSED;
			portals_to_dir_options(ch->room);
			
			if (ch->room->dir_option[door]->keyword && strlen (ch->room->dir_option[door]->keyword))
			{
				sprintf (buf, "You open the %s $T.", relative_dirs[door]);
				ch->act(buf, false,  0, ch->room->dir_option[door]->keyword,
					TO_CHAR | _ACT_FORMAT);
				sprintf (buf, "$n opens the %s $T.", relative_dirs[door]);
				ch->act(buf, false,  0, ch->room->dir_option[door]->keyword,
					TO_ROOM | _ACT_FORMAT);
			}
			else
			{
				sprintf (buf, "You open the %s door.", relative_dirs[door]);
				ch->act("You open the door.", false, 0, 0,
					TO_CHAR | _ACT_FORMAT);
				sprintf (buf, "$n opens the %s door.", relative_dirs[door]);
				ch->act("$n opens the door.", false, 0, 0,
					TO_ROOM | _ACT_FORMAT);
			}
			
				// now for opening the OTHER side of the door! 
			other_room = ch->room->dir_option[door]->to_room;
			
			if (other_room != NOWHERE)
			{
				int back_dir = rev_dir[door];
				ROOM_DATA *back_room = vtor(other_room);
				
				if (back_room)
				{
				back_exit = back_room->dir_option[back_dir];
				if (back_exit && (back_exit->to_room == ch->in_room))
				{
					back_exit->port_flags &= ~EX_CLOSED;
					vtop(back_exit->portal)->port_flags &= ~EX_CLOSED;
					portals_to_dir_options(back_room);
					
					if (back_exit->keyword && strlen (back_exit->keyword))
					{
						sprintf (buf,
								 "The %s %s %s opened from the other side.\n",
								 relative_dirs[rev_dir[door]], back_exit->keyword,
								 (back_exit->keyword[strlen (back_exit->keyword) - 1] ==
								  's') ? "are" : "is");
						send_to_room (buf, is_exit(ch, door)->to_room);
					}
					else
					{
						sprintf (buf,
								 "The %s door is opened from the other side.",
								 relative_dirs[rev_dir[door]]);
						send_to_room (buf, is_exit(ch, door)->to_room);
					}
				}
				}
			}
				
		}
	}
}


void
do_close (CHAR_DATA * ch, char *argument, int cmd)
{
	int door, other_room;
	char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	char buffer[MAX_STRING_LENGTH];
	struct room_exit_data *back;
	OBJ_DATA *obj;
	CHAR_DATA *victim;

	argument_interpreter (argument, type, dir);

	if (!*type)
		ch->send_to_char ("Close what?\n");
	else
		if (generic_find (type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
		{

			/* this is an object */

			if (obj->obj_flags.type_flag == ITEM_BOOK)
			{
				if (!obj->open)
				{
					ch->send_to_char ("That isn't currently open.\n");
					return;
				}
				sprintf (buf, "You close #2%s#0.\n", obj->short_description);
				ch->send_to_char (buf);
				sprintf (buf, "%s#0 closes #2%s#0.",  ch->char_short(),
					obj->short_description);
				sprintf (buffer, "#5%s", CAP (buf));
				ch->act(buffer, false, obj, 0, TO_ROOM | _ACT_FORMAT);
				obj->open = 0;
				return;
			}
			
			if (obj->obj_flags.type_flag != ITEM_CONTAINER)
				ch->send_to_char ("That's not a container.\n");
			else if (IS_SET (obj->o.container.flags, CONT_CLOSED))
				ch->send_to_char ("But it's already closed!\n");
			else if (!IS_SET (obj->o.container.flags, CONT_CLOSEABLE))
				ch->send_to_char ("That's impossible.\n");
			else
			{
				obj->o.container.flags |= CONT_CLOSED;
				ch->act("You close $p.", false, obj, 0, TO_CHAR | _ACT_FORMAT);
				ch->act("$n closes $p.", false, obj, 0, TO_ROOM);
			}
		}
		else if ((door = find_door (ch, type, dir)) >= 0)
		{

			/* Or a door */

			if (!IS_SET (ch->room->dir_option[door]->port_flags, EX_ISDOOR)
				&& !IS_SET (ch->room->dir_option[door]->port_flags, EX_ISGATE))
				ch->send_to_char ("That's absurd.\n");
			else if (IS_SET (ch->room->dir_option[door]->port_flags, EX_CLOSED))
				ch->send_to_char ("It's already closed!\n");
			else
			{
				ch->room->dir_option[door]->port_flags |= EX_CLOSED;
				vtop(ch->room->dir_option[door]->portal)->port_flags |= EX_CLOSED;
				portals_to_dir_options(ch->room);

				if (ch->room->dir_option[door]->keyword && strlen (ch->room->dir_option[door]->keyword))
				{
					sprintf (buf, "You close the %s $T.", relative_dirs[door]);
					ch->act(buf, false, 0, ch->room->dir_option[door]->keyword,
						TO_CHAR | _ACT_FORMAT);
					sprintf (buf, "$n closes the %s $T.", relative_dirs[door]);
					ch->act(buf, false, 0, ch->room->dir_option[door]->keyword,
						TO_ROOM | _ACT_FORMAT);
				}
				else
				{
					sprintf (buf, "You close the %s door.", relative_dirs[door]);
					ch->act(buf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
					sprintf (buf, "$n closes the %s door.", relative_dirs[door]);
					ch->act(buf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
				}
				
					// now for closing the other side, too 
				
				if ((other_room = ch->room->dir_option[door]->to_room) != NOWHERE)
					if ((back = vtor (other_room)->dir_option[rev_dir[door]]))
						if (back->to_room == ch->in_room)
						{
							back->port_flags |= EX_CLOSED;
							vtop(back->portal)->port_flags |= EX_CLOSED;
							portals_to_dir_options(vtor (other_room));

							if (back->keyword && strlen (back->keyword))
							{
								sprintf (buf,
									"The %s %s closes quietly.\n",
									relative_dirs[rev_dir[door]], back->keyword);
								send_to_room (buf, is_exit(ch, door)->to_room);
							}
							else
							{
								sprintf (buf,
									"The %s door closes quietly.\n",
									relative_dirs[rev_dir[door]]);
								send_to_room (buf, is_exit(ch, door)->to_room);
							}
						}
			}
		}
}


OBJ_DATA *
has_key (CHAR_DATA * ch, OBJ_DATA * obj, int key)
{
	OBJ_DATA *tobj;

	if (ch->right_hand && ch->right_hand->nVirtual == key)
	{
		tobj = ch->right_hand;
		if (obj && tobj->o.od.value[1]
		&& tobj->o.od.value[1] == obj->coldload_id)
			return tobj;
		else if (!obj || !tobj->o.od.value[1])
			return tobj;
	}

	if (ch->left_hand && ch->left_hand->nVirtual == key)
	{
		tobj = ch->left_hand;
		if (obj && tobj->o.od.value[1]
		&& tobj->o.od.value[1] == obj->coldload_id)
			return tobj;
		else if (!obj || !tobj->o.od.value[1])
			return tobj;
	}

	if (ch->right_hand && ch->right_hand->obj_flags.type_flag == ITEM_KEYRING)
	{
		for (tobj = ch->right_hand->contains; tobj; tobj = tobj->next_content)
			if (tobj->nVirtual == key)
			{
				if (obj && tobj->o.od.value[1]
				&& tobj->o.od.value[1] == obj->coldload_id)
					return tobj;
				else if (!obj || !tobj->o.od.value[1])
					return tobj;
			}
	}

	if (ch->left_hand && ch->left_hand->obj_flags.type_flag == ITEM_KEYRING)
	{
		for (tobj = ch->left_hand->contains; tobj; tobj = tobj->next_content)
			if (tobj->nVirtual == key)
			{
				if (obj && tobj->o.od.value[1]
				&& tobj->o.od.value[1] == obj->coldload_id)
					return tobj;
				else if (!obj || !tobj->o.od.value[1])
					return tobj;
			}
	}

	return 0;
}


void
do_lock (CHAR_DATA * ch, char *argument, int cmd)
{
	int door, other_room;
	char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	struct room_exit_data *back;
	OBJ_DATA *obj = NULL, *key = NULL;
	CHAR_DATA *victim;


	argument_interpreter (argument, type, dir);

	if (!*type)
		ch->send_to_char ("Lock what?\n");
	else if (generic_find (type, FIND_OBJ_INV | FIND_OBJ_ROOM,
		ch, &victim, &obj))
	{

		/* this is an object */

		if (obj->obj_flags.type_flag != ITEM_CONTAINER)
			ch->send_to_char ("That's not a container.\n");
		else if (!IS_SET (obj->o.container.flags, CONT_CLOSED))
			ch->send_to_char ("Maybe you should close it first...\n");
		else if (obj->o.container.key <= 0)
			ch->send_to_char ("That thing can't be locked.\n");
		else if (!(key = has_key (ch, obj, obj->o.container.key)))
			ch->send_to_char ("You don't seem to have the proper key.\n");
		else if (IS_SET (obj->o.container.flags, CONT_LOCKED))
			ch->send_to_char ("It is locked already.\n");
		else
		{
			if (key && !key->o.od.value[1])
				key->o.od.value[1] = obj->coldload_id;
			obj->o.container.flags |= CONT_LOCKED;
			ch->act("You lock $p.", false, obj, 0, TO_CHAR | _ACT_FORMAT);
			ch->act("$n locks $p.", false, obj, 0, TO_ROOM | _ACT_FORMAT);
		}
	}
	else if ((door = find_door (ch, type, dir)) >= 0)
	{

		/* a door, perhaps */

		if (!IS_SET (is_exit(ch, door)->port_flags, EX_ISDOOR)
			&& !IS_SET (is_exit(ch, door)->port_flags, EX_ISGATE))
			ch->send_to_char ("That's absurd.\n");
		else if (!IS_SET (is_exit(ch, door)->port_flags, EX_CLOSED))
			ch->send_to_char ("You have to close it first, I'm afraid.\n");
		else if (is_exit(ch, door)->key < 0)
			ch->send_to_char ("There does not seem to be any keyholes.\n");
		else if (!has_key (ch, NULL, is_exit(ch, door)->key))
			ch->send_to_char ("You don't have the proper key.\n");
		else if (IS_SET (is_exit(ch, door)->port_flags, EX_LOCKED))
			ch->send_to_char ("It's already locked!\n");
		else
		{
			is_exit(ch, door)->port_flags |= EX_LOCKED;
			vtop(is_exit(ch, door)->portal)->port_flags |= EX_LOCKED;
			
			if (is_exit(ch, door)->keyword && strlen (is_exit(ch, door)->keyword))
			{
				
				sprintf (buf, "You lock the %s $T.", relative_dirs[door]);
				ch->act(buf, false, 0, is_exit(ch, door)->keyword,
					TO_CHAR | _ACT_FORMAT);

				sprintf (buf, "$n locks the %s $T.", relative_dirs[door]);
				ch->act(buf, false, 0, is_exit(ch, door)->keyword,
					TO_ROOM | _ACT_FORMAT);
			}
			else
			{
				sprintf (buf, "You lock the %s door.", relative_dirs[door]);
				ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);

				if (ch->in_room < 100000)
					sprintf (buf, "$n locks the %s door.", relative_dirs[door]);
				else
					sprintf (buf, "$n locks the door.");


				ch->act(buf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
			}
			/* now for locking the other side, too */
			if ((other_room = is_exit(ch, door)->to_room) != NOWHERE)
				if ((back = vtor (other_room)->dir_option[rev_dir[door]]))
					if (back->to_room == ch->in_room)
					{
						back->port_flags |= EX_LOCKED;
						vtop(back->portal)->port_flags |= EX_LOCKED;
						
							//if they are hidden, there are no echos to the room
						if (!get_affect (ch, MAGIC_HIDDEN))
						{
							if (back->keyword && strlen (back->keyword))
							{
								
								sprintf (buf,
									"The %s %s is locked from the other side.\n",
									relative_dirs[rev_dir[door]],
									back->keyword);
								send_to_room (buf, is_exit(ch, door)->to_room);
							}
							else
							{
								sprintf (buf,
									"The %s door is locked from the other side.\n",
									relative_dirs[rev_dir[door]]);
								send_to_room (buf, is_exit(ch, door)->to_room);
							}
						}

					}
		}
	}
}


void
do_unlock (CHAR_DATA * ch, char *argument, int cmd)
{
	int door, other_room;
	char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	struct room_exit_data *back;
	OBJ_DATA *obj = NULL, *key = NULL;
	CHAR_DATA *victim;


	argument_interpreter (argument, type, dir);

	if (!*type)
		ch->send_to_char ("Unlock what?\n");

	else if (generic_find (type, FIND_OBJ_INV | FIND_OBJ_ROOM,
		ch, &victim, &obj))
	{

		/* this is an object */

		if (obj->obj_flags.type_flag != ITEM_CONTAINER)
			ch->send_to_char ("That's not a container.\n");
		else if (!IS_SET (obj->o.container.flags, CONT_CLOSED))
			ch->send_to_char ("It isn't closed.\n");
		else if (obj->o.container.key <= 0)
			ch->send_to_char ("Odd - you can't seem to find a keyhole.\n");
		else if (!(key = has_key (ch, obj, obj->o.container.key)))
			ch->send_to_char ("You don't have the proper key.\n");
		else if (!IS_SET (obj->o.container.flags, CONT_LOCKED))
			ch->send_to_char ("Oh.. it wasn't locked, after all.\n");
		else
		{
			if (key && !key->o.od.value[1])
				key->o.od.value[1] = obj->coldload_id;
			obj->o.container.flags &= ~CONT_LOCKED;
			sprintf(buf, "You unlock $p with %s.", key->short_description);
			ch->act(buf, false,  obj, 0, TO_CHAR | _ACT_FORMAT);
			sprintf(buf, "$n unlocks $p with %s.", key->short_description);
			ch->act(buf, false,  obj, 0, TO_ROOM | _ACT_FORMAT);
		}

	}
	else if ((door = find_door (ch, type, dir)) >= 0)
	{

		/* it is a door */

		if (!IS_SET (is_exit(ch, door)->port_flags, EX_ISDOOR)
			&& !IS_SET (is_exit(ch, door)->port_flags, EX_ISGATE))
			ch->send_to_char ("That's absurd.\n");
		else if (!IS_SET (is_exit(ch, door)->port_flags, EX_CLOSED))
			ch->send_to_char ("Heck.. it ain't even closed!\n");
		else if (is_exit(ch, door)->key < 0)
			ch->send_to_char ("You can't seem to spot any keyholes.\n");
		else if (!(key = has_key(ch, NULL, is_exit(ch, door)->key)))
			ch->send_to_char ("You do not have the proper key for that.\n");
		else if (!IS_SET (is_exit(ch, door)->port_flags, EX_LOCKED))
			ch->send_to_char ("It's already unlocked, it seems.\n");
		else
		{
			is_exit(ch, door)->port_flags &= ~EX_LOCKED;
			vtop(is_exit(ch, door)->portal)->port_flags &= ~EX_LOCKED;
			
			if (is_exit(ch, door)->keyword && strlen (is_exit(ch, door)->keyword))
			{
				sprintf (buf, "You unlock the %s $T with %s.", relative_dirs[door], key->short_description);
				ch->act(buf, false, 0, is_exit(ch, door)->keyword,
					TO_CHAR | _ACT_FORMAT);
				sprintf (buf, "$n unlocks the %s $T with %s.", relative_dirs[door], key->short_description);
				ch->act(buf, false, 0, is_exit(ch, door)->keyword,
					TO_ROOM | _ACT_FORMAT);
			}
			else
			{
				
				sprintf (buf, "You unlock the %s door with %s.", relative_dirs[door], key->short_description);
				ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
				sprintf (buf, "$n unlocks the %s door with %s.", relative_dirs[door], key->short_description);
				ch->act(buf, false,  0, 0, TO_ROOM);
			}
			/* now for unlocking the other side, too */
			if ((other_room = is_exit(ch, door)->to_room) != NOWHERE)
				if ((back = vtor (other_room)->dir_option[rev_dir[door]]))
					if (back->to_room == ch->in_room)
					{
						back->port_flags &= ~EX_LOCKED;
						vtop(back->portal)->port_flags &= ~EX_LOCKED;
						
						
						if (!get_affect (ch, MAGIC_HIDDEN))
						{
							if (back->keyword && strlen (back->keyword))
							{
								sprintf (buf,
									"The %s %s is unlocked from the other side.\n",
									relative_dirs[rev_dir[door]],
									back->keyword);
								send_to_room (buf, is_exit(ch, door)->to_room);
							}
							else
							{
								sprintf (buf,
									"The %s door is unlocked from the other side.\n",
									relative_dirs[rev_dir[door]]);
								send_to_room (buf, is_exit(ch, door)->to_room);
							}
						}

					}
		}
	}
}


void
do_enter (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	char tmp[MAX_STRING_LENGTH];
	int door;
	CHAR_DATA *ent_mob;

	one_argument (argument, buf);

	
	
	if (*buf)			/* an argument was supplied, search for door keyword */
	{
		for (door = 0; door <= LAST_DIR; door++)
			if (is_exit(ch, door))
				if (is_exit(ch, door)->keyword && strlen (is_exit(ch, door)->keyword))
					if (!str_cmp (is_exit(ch, door)->keyword, buf))
					{
						do_move (ch, "", door);
						return;
					}
					sprintf (tmp, "There is no %s here.\n", buf);
					ch->send_to_char (tmp);
	}
	else if (IS_SET (vtor (ch->in_room)->room_flags, INDOORS))
		ch->send_to_char ("You are already indoors.\n");
	else
	{
		/* try to locate an entrance */
		for (door = 0; door <= LAST_DIR; door++)
			if (is_exit(ch, door))
				if (is_exit(ch, door)->to_room != NOWHERE)
					if (!IS_SET (is_exit(ch, door)->port_flags, EX_CLOSED) &&
						IS_SET (vtor (is_exit(ch, door)->to_room)->room_flags, INDOORS))
					{
						do_move (ch, "", door);
						return;
					}
					ch->send_to_char ("You can't seem to find anything to enter.\n");
	}
}


void
leave_vehicle (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *vehicle;
	ROOM_DATA *troom;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (ch->vehicle)
	{

		vehicle = ch->vehicle;

		if (vehicle->mob->nVirtual != ch->in_room)
		{
			ch->send_to_char ("You cannot exit the craft from here.\n");
			return;
		}

		ch->act("$n disembarks.", true, 0, 0, TO_ROOM);

		troom = vtor (vehicle->in_room);

		if (troom->terrain_type == SECT_REEF ||
			troom->terrain_type == SECT_OCEAN ||
			is_room_affected (troom->affects, MAGIC_ROOM_FLOOD))
		{
			if (*buf != '!')
			{
				ch->act("$N is at sea.", false, 0, vehicle, TO_CHAR);
				ch->send_to_char ("If you jump ship into the water, type:\n"
					"   #3leave !#0\n");
				return;
			}
		}

		ch->char_from_room();

		ch->char_to_room(vehicle->in_room);

		ch->vehicle = NULL;
		ch->coldload_id = 0;

		ch->act("$n disembarks from $N.", true, 0, vehicle, TO_ROOM);

		do_look (ch, "", 1);
		return;
	}
	else if (IS_SET (ch->room->room_flags, VEHICLE))
	{
		if (!(vehicle = get_mob_vnum (ch->room->nVirtual)))
		{
			ch->send_to_char ("You cannot exit the craft from here.\n");
			return;
		}
		ch->act("$n disembarks.", true, 0, 0, TO_ROOM);

		ch->char_from_room();

		ch->char_to_room(vehicle->in_room);

		ch->vehicle = NULL;
		ch->coldload_id = 0;

		ch->act("$n disembarks from $N.", true, 0, vehicle, TO_ROOM);

		do_look (ch, "", 1);
		return;
	}

	ch->send_to_char ("You cannot exit the craft from here.\n");
}

void
do_leave (CHAR_DATA * ch, char *argument, int cmd)
{

	if (ch->vehicle || IS_SET (ch->room->room_flags, VEHICLE))
	{
		leave_vehicle (ch, argument);
		return;
	}

	ch->send_to_char ("You cannot exit from here.\n");
}

void
do_stand (CHAR_DATA * ch, char *argument, int cmd)
{
	int already_standing = 0;
	int dir;
	AFFECTED_TYPE *af;
	ROOM_EXIT_DATA* room_exit;
	char buf[MAX_STRING_LENGTH] = "";
	std::string first_person, third_person;

	if (!ch->can_move())
	{
		ch->send_to_char ("You can't move.\n");
		return;
	}

	if(ch->delay_type == DEL_HIDE)
	{
		ch->send_to_char ("You cannot move while looking for a place to hide.\n");
		return;
	}

	if (*argument != '(')
		argument = one_argument (argument, buf);

	if (*buf)
	{
		if ((dir = index_lookup (dirs, buf)) == -1)
		{
			ch->send_to_char ("Stand where?\n");
			return;
		}
		
		room_exit = is_exit_normal(ch, dir, 1); //works for normal portals
		
		if (room_exit)
		{
			if (!(af = get_affect (ch, AFFECT_SHADOW)))
			{
				
				magic_add_affect (ch, AFFECT_SHADOW, -1, 0, 0, 0, 0);
				
				af = get_affect (ch, AFFECT_SHADOW);
				
				af->a.shadow.shadow = NULL;
				af->a.shadow.edge = -1;	/* Not on an edge */
			}
			
			move (ch, "stand", dir, 0, room_exit->portal);
			return;
		}
		else
		{
			ch->send_to_char ("Stand where?\n");
			return;
		}
	}

	if (get_second_affect (ch, SA_STAND, NULL))
		return;

	if (ch->get_position() == STAND)
		already_standing = 1;

	switch (ch->get_position())
	{
	case STAND:
		ch->act("You are already standing.", false, 0, 0, TO_CHAR);
		return;
		break;

	case SIT:
		if ((af = get_affect (ch, MAGIC_SIT_TABLE)) &&
			is_obj_in_list ((OBJ_DATA *) af->a.spell.t, ch->room->contents))
		{
			first_person.assign("You get up from #2");
			first_person.append(obj_short_desc((OBJ_DATA *) af->a.spell.t));
			first_person.append("#0");
			third_person.assign("#5");
			third_person.append(ch->char_short());
			third_person[2] = toupper (third_person[2]);
			third_person.append("#0 gets up from #2");
			third_person.append(obj_short_desc((OBJ_DATA *) af->a.spell.t));
			third_person.append("#0");

		}
		else
		{
			first_person.assign("You stand up");
			third_person.assign("#5");
			third_person.append(ch->char_short());
			third_person[2] = toupper (third_person[2]);
			third_person.append("#0 clambers on #5");
			third_person.append(HSHR(ch));
			third_person.append("#0 feet");
		}
		if (evaluate_emote_string(ch, &first_person, third_person, argument))
			ch->set_position(STAND);
		else
			return;

		break;

	case REST:

		first_person.assign("You stop resting, and stand up");
		third_person.assign("#5");
		third_person.append(ch->char_short());
		third_person[2] = toupper (third_person[2]);
		third_person.append("#0 stops resting, and clambers on #5");
		third_person.append(HSHR(ch));
		third_person.append("#0 feet");

		if (evaluate_emote_string(ch, &first_person, third_person, argument))
			ch->set_position(STAND);
		else
			return;

		break;

	case SLEEP:
		do_wake (ch, argument, 0);
		break;


	default:
		ch->act("You stop floating around, and put your feet on the ground.",
			false, 0, 0, TO_CHAR);
		ch->act("$n stops floating around, and puts $s feet on the ground.",
			true, 0, 0, TO_ROOM);
		break;
	}

	if (!already_standing && ch->get_position() == STAND)
		follower_catchup (ch);

	if (ch->get_position() != SIT && ch->get_position() != SLEEP &&
		(af = get_affect (ch, MAGIC_SIT_TABLE)))
		affect_remove (ch, af);
}


void
do_sit (CHAR_DATA * ch, char *argument, int cmd)
{
	int count;
	CHAR_DATA *tch;
	SECOND_AFFECT *sa;
	OBJ_DATA *obj = NULL;
	AFFECTED_TYPE *af;
	char buf[MAX_STRING_LENGTH]="";
	std::string first_person, third_person;

	if (*argument != '(')
		argument = one_argument (argument, buf);

	if ((sa = get_second_affect (ch, SA_STAND, NULL)))
		remove_second_affect (sa);

	if (*buf && !(obj = get_obj_in_list_vis (ch, buf, ch->room->contents)))
	{
		ch->send_to_char ("You don't see that to sit at.\n");
		return;
	}

	if (obj)
	{

		if (!IS_SET (obj->obj_flags.extra_flags, ITEM_TABLE))
		{
			ch->act("You cannot sit at $p.", false, obj, 0, TO_CHAR);
			return;
		}

		if (ch->get_position() != STAND)
		{
			ch->act("You must be standing before you can sit at $p.",
				false, obj, 0, TO_CHAR);
			return;
		}

		count = 0;

		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{

			if (tch == ch)
				continue;

			if ((af = get_affect (tch, MAGIC_SIT_TABLE)) &&
				af->a.table.obj == obj)
				count++;
		}

		if (obj->o.container.table_max_sitting != 0 &&
			count >= obj->o.container.table_max_sitting)
		{
			ch->act("There is no available space at $p.", false, obj, 0,
				TO_CHAR);
			return;
		}

		remove_affect_type (ch, MAGIC_HIDDEN);
		remove_affect_type (ch, MAGIC_SNEAK);
		
		first_person.assign("You sit at #2");
		first_person.append(obj_short_desc(obj));
		first_person.append("#0");
		third_person.assign("#5");
		third_person.append(ch->char_short());
		third_person[2] = toupper (third_person[2]);
		third_person.append("#0 sits at #2");
		third_person.append(obj_short_desc(obj));
		third_person.append("#0");

		if (evaluate_emote_string (ch, &first_person, third_person, argument))
		{
				//magic_add_affect (ch, MAGIC_SIT_TABLE, -1, 0, 0, 0, 0);

			if ((af = get_affect(ch, MAGIC_SIT_TABLE)))
				affect_remove (ch, af);

			table_add_affect (ch, obj, MAGIC_SIT_TABLE);
			
				//af->a.table.obj = obj;

			ch->set_position(SIT);
		}

		return;
	}

	switch (ch->get_position())
	{
	case STAND:
		{
			first_person.assign("You sit down");
			third_person.assign("#5");
			third_person.append(ch->char_short());
			third_person[2] = toupper (third_person[2]);
			third_person.append("#0 sits down");

			if (evaluate_emote_string (ch, &first_person, third_person, argument))
			{
				ch->set_position(SIT);
			}
			else
				return;
		}
		break;
	case SIT:
		{
			ch->send_to_char ("You're sitting already.\n");
		}
		break;
	case REST:
		{
			first_person.assign("You stop resting, and sit up");
			third_person.assign("#5");
			third_person.append(ch->char_short());
			third_person[2] = toupper (third_person[2]);
			third_person.append("#0 stops resting");
			if (evaluate_emote_string (ch, &first_person, third_person, argument))
				ch->set_position(SIT);
			else
				return;
		}
		break;
	case SLEEP:
		{
			ch->act("You have to wake up first.", false, 0, 0, TO_CHAR);
		}
		break;
	
	default:
		{
			ch->act("You stop floating around, and sit down.", false, 0, 0,
				TO_CHAR);
			ch->act("$n stops floating around, and sits down.", true, 0, 0,
				TO_ROOM);
			ch->set_position(SIT);
		}
		break;
	}
	ch->clear_pmote();
}


void
do_rest (CHAR_DATA * ch, char *argument, int cmd)
{
	std::string first_person, third_person;

	if (!ch->can_move())
	{
		ch->send_to_char ("You can't move.\n");
		return;
	}

		
	switch (ch->get_position())
	{
	case STAND:
		{
			first_person.assign("You sit down and rest your tired bones");
			third_person.assign("#5");
			third_person.append(ch->char_short());
			third_person[2] = toupper(third_person[2]);
			third_person.append("#0 sits down and rests");

			if (evaluate_emote_string(ch, &first_person, third_person, argument))
				ch->set_position(REST);
			else
				return;
		}
		break;
	case SIT:
		{
			first_person.assign("You rest your tired bones");
			third_person.assign("#5");
			third_person.append(ch->char_short());
			third_person[2] = toupper(third_person[2]);
			third_person.append("#0 rests");

			if (evaluate_emote_string(ch, &first_person, third_person, argument))
				ch->set_position(REST);
			else
				return;
		}
		break;
	case REST:
		{
			ch->act("You are already resting.", false, 0, 0, TO_CHAR);
		}
		break;
	case SLEEP:
		{
			ch->act("You have to wake up first.", false, 0, 0, TO_CHAR);
		}
		break;
	
	default:
		{
			ch->act("You stop floating around, and stop to rest your tired bones.",
				false, 0, 0, TO_CHAR);
			ch->act("$n stops floating around, and rests.", false, 0, 0,
				TO_ROOM);
			ch->set_position(SIT);
		}
		break;
	}

	ch->clear_pmote();
}


void
do_sleep (CHAR_DATA * ch, char *argument, int cmd)
{
	std::string first_person, third_person;
	ch->clear_pmote();

		
	switch (ch->get_position())
	{
	case STAND:
	case SIT:
	case REST:

		first_person.assign("You fall asleep");
		third_person.assign("#5");
		third_person.append(ch->char_short());
		third_person[2] = toupper (third_person[2]);

		third_person.append("#0 falls asleep");

		if (evaluate_emote_string (ch, &first_person, third_person, argument))
			ch->set_position(SLEEP);
		else
			return;

		break;

	case SLEEP:
		ch->send_to_char ("You are already sound asleep.\n");
		break;

	default:
		ch->act("You stop floating around, and lie down to sleep.",
			false, 0, 0, TO_CHAR);
		ch->act("$n stops floating around, and lie down to sleep.",
			true, 0, 0, TO_ROOM);
		ch->set_position(SLEEP);
		break;
	}
}


void
do_wake (CHAR_DATA * ch, char *argument, int not_noisy)
{
	CHAR_DATA *tch = NULL;
	AFFECTED_TYPE *af;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (*buf)
	{
		if (!(tch = get_char_room_vis (ch, buf)))
		{
			if (not_noisy)
				ch->send_to_char ("They aren't here.\n");
			return;
		}
	}

	/* Awaken someone else */

	if (*buf && tch != ch)
	{

		if (ch->get_position() == SLEEP)
		{
			ch->send_to_char ("You must be awake yourself to do that.\n");
			return;
		}

		if (tch->get_position()!= SLEEP)
		{
			ch->act("$E isn't asleep.", true, 0, tch, TO_CHAR);
			return;
		}

		if (get_affect (tch, MAGIC_AFFECT_SLEEP))
		{
			if (not_noisy)
				ch->act("$E doesn't respond.", false, 0, tch, TO_CHAR);
			return;
		}

		if (get_second_affect (tch, SA_KNOCK_OUT, NULL))
		{
			if (not_noisy)
				ch->act("$E can't be roused.", false, 0, tch, TO_CHAR);
			return;
		}

		ch->act("You wake $M up.", false, 0, tch, TO_CHAR);
		tch->set_position(REST);

		ch->act("You are awakened by $n.", false, 0, tch, TO_VICT);

		if ((tch = ch->being_dragged ()))
		{
			ch->act("You awaken to discover $N was dragging you!",
				false, 0, tch, TO_CHAR);
			if ((af = get_affect (tch, MAGIC_DRAGGER)))
				affect_remove (tch, af);
		}

		return;
	}

	/* Awaken yourself */

	if (get_affect (ch, MAGIC_AFFECT_SLEEP))
	{
		ch->send_to_char ("You can't wake up!\n");
		return;
	}

	if (ch->get_position() > SLEEP)
		ch->send_to_char ("You are already awake...\n");

	else if (ch->get_position() == UNCON)
		ch->send_to_char ("You're out cold, I'm afraid.\n");

	else if (get_second_affect (ch, SA_KNOCK_OUT, NULL))
		ch->send_to_char ("Your body is still recovering from trauma.");

	else
	{
		ch->set_position(REST);

		if ((tch = ch->being_dragged()))
		{
			if (ch->get_position() == REST)
			{
				ch->act("You awake to discover $N is dragging you!",
					false, 0, tch, TO_CHAR);
			}
			else
			{
				ch->act("You awaken to discover $N was dragging you!",
					false, 0, tch, TO_CHAR);
				if ((af = get_affect (tch, MAGIC_DRAGGER)))
					affect_remove (tch, af);
			}
		}
		else
		{
			if (ch->get_position() == REST)
				ch->act("You open your eyes.", false, 0, 0, TO_CHAR);
			else
				ch->act("You awaken and stand.", false, 0, 0, TO_CHAR);
		}

		ch->act("$n awakens.", true, 0, 0, TO_ROOM);
	}
}



/*	track finds the shortest path between a character and a room somewhere
in the mud.
*/


int
track (CHAR_DATA * ch, int to_room)
{
	int room_set_1[5000];		/* These hardly need to be more than */
	int room_set_2[5000];		/*  about 200 elements...5000 tb safe */
	int *rooms;
	int *torooms;
	int *tmp;
	int targets;
	int options;
	int i;
	int dir;
	int count = 0;
	char buf[MAX_STRING_LENGTH];	/* KILLER CDR */
	ROOM_DATA *room;
	ROOM_DATA *opt_room;
	ROOM_DATA *target_room;

	if (!(target_room = vtor (to_room)))
		return -1;

	if (to_room == ch->in_room)
		return -1;

	search_sequence++;

	rooms = room_set_1;
	torooms = room_set_2;

	options = 1;

	rooms[0] = to_room;

	high_water = 0;

	while (options)
	{

		targets = 0;

		for (i = 0; i < options; i++)
		{

			opt_room = vtor (rooms[i]);

			for (dir = 0; dir <= LAST_DIR; dir++)
			{

				if (!opt_room->dir_option[dir] ||
					opt_room->dir_option[dir]->to_room == NOWHERE)
					continue;

				room = vtor (opt_room->dir_option[dir]->to_room);

				if (!room)
				{
					sprintf (buf, "Huh?  Room %d -> %d (bad)\n",
						opt_room->nVirtual,
						opt_room->dir_option[dir]->to_room);
					ch->send_to_char (buf);
					continue;
				}

				if (!room->dir_option[rev_dir[dir]] ||
					room->dir_option[rev_dir[dir]]->to_room !=
					opt_room->nVirtual)
				{
					/*                      printf ("Room %d only connects '%s' with room %d.\n",
					opt_room->nVirtual, dirs [dir], room->nVirtual);
					*/ continue;
				}

				if (room->search_sequence == search_sequence)
					continue;

				rooms_scanned++;

				room->search_sequence = search_sequence;

				if (ch->get_trust())
				{
					if (!strncmp (room->description, "No Description Set", 18))
					{
						if (count % 12 == 11)
							ch->send_to_char ("\n");
						count++;

						sprintf (buf, "%5d ", room->nVirtual);
						ch->send_to_char (buf);
					}
				}

				if (room->nVirtual == ch->in_room)
					return rev_dir[dir];

				torooms[targets++] = room->nVirtual;

				if (targets > high_water)
					high_water = targets;
			}
		}

		tmp = rooms;
		rooms = torooms;
		torooms = tmp;		/* Important - must point at other structure. */

		options = targets;
	}

	if (ch->get_trust())
	{
		sprintf (buf, "\nTotal rooms: %d\n", count);
		ch->send_to_char (buf);
	}

	return -1;
}

char *
crude_name (int race)
{
	char *ptrBodyProto = NULL;
	
	if (race < 0)
		race = 0;
	
	if ((ptrBodyProto = lookup_race_variable (race, RACE_BODY_PROTO)) != NULL)
	{
		switch (strtol (ptrBodyProto, NULL, 10))
		{
		case PROTO_HUMANOID:
			return "bipedal creature";
		case PROTO_FOURLEGGED_PAWS:
		case PROTO_FOURLEGGED_FEET:
			return "four-legged creature";
		case PROTO_FOURLEGGED_HOOVES:
			return "four-legged, cloven-hoofed creature";
		case PROTO_SERPENTINE:
		case PROTO_WINGED_TAIL:
		case PROTO_WINGED_NOTAIL:
		default:
			;
		}
	}
	return "unknown creature";
}

char *
specific_name (int race)
{
	std::string buf;
	char *ptrBodyProto = NULL;
	char *ptrSizeProto = NULL;
	
	if (race < 0)
		race = 0;
	
	if ((ptrSizeProto = lookup_race_variable (race, RACE_SIZE)) != NULL)
	{
		switch (strtol (ptrSizeProto, NULL, 10))
		{
		case -3:
			buf.append("miniscule ");
			break;
		case -2:
			buf.append("tiny ");
			break;
		case -1:
			buf.append("small ");
			break;
		case 0:
			buf.append("medium-sized ");
			break;
		case 1:
			buf.append("large ");
			break;
		case 2:
			buf.append("enormous ");
			break;
		case 3:
			buf.append("colossal ");
			break;
		default:
			break;
		}
	}
	if ((ptrBodyProto = lookup_race_variable (race, RACE_BODY_PROTO)) != NULL)
	{
		switch (strtol (ptrBodyProto, NULL, 10))
		{
		case PROTO_WINGED_TAIL:
			buf.append("tailed, bipedal creature");
			break;
		case PROTO_WINGED_NOTAIL:
		case PROTO_HUMANOID:
			buf.append("bipedal creature");
			break;
		case PROTO_FOURLEGGED_PAWS:
		case PROTO_FOURLEGGED_FEET:
			buf.append("four-legged creature");
			break;
		case PROTO_FOURLEGGED_HOOVES:
			buf.append("four-legged, cloven-hoofed creature");
			break;
		case PROTO_SERPENTINE:
			buf.append("serpentine creature");
			break;
		default:
			buf.append("unknown creature");
		}
	}

	if (buf.empty())
		return "unknown creature";

	return (char *) buf.c_str();

}

char *
track_age (int hours_passed)
{
	if (hours_passed <= 1)
		return "within the hour";
	else if (hours_passed <= 4)
		return "recently";
	else if (hours_passed <= 6)
		return "within hours";
	else if (hours_passed <= 12)
		return "within a half-day";
	else if (hours_passed <= 24)
		return "within a day";
	else if (hours_passed <= 48)
		return "within a couple days";
	else if (hours_passed <= 72)
		return "within a few days";
	else
		return "within days";
}

char *
speed_adj (int speed)
{
	if (speed == 0)
		return "a brisk walk";
	else if (speed == 1)
		return "a faltering stagger";
	else if (speed == 2)
		return "a deliberate pace";
	else if (speed == 3)
		return "a swift jog";
	else if (speed == 4)
		return "a loping run";
	else
		return "a haphazard sprint";
}


void
skill_tracking (CHAR_DATA * ch, char *argument, int cmd)
{
	TRACK_DATA *track;
	char buf[MAX_STRING_LENGTH];

	if (ch->room->terrain_type == SECT_ROAD
		|| ch->room->terrain_type == SECT_URBAN)
	{
		ch->send_to_char ("Tracking in such an area is all but impossible.\n");
		return;
	}

	if (ch->get_trust())
	{
		if (!ch->room->tracks)
		{
			ch->send_to_char ("There are no tracks here.\n");
			return;
		}
		sprintf (buf, "The following tracks are here:\n\n");
		for (track = ch->room->tracks; track; track = track->next)
		{
			if (track->from_dir)
				sprintf (buf + strlen (buf),
				"%s tracks, from the %s, heading %s at a %s, left %d hours ago.",
				lookup_race_variable (track->race, RACE_NAME),
				dirs[track->from_dir], dirs[track->to_dir],
				verbal_speeds[track->speed], track->hours_passed);
			else
				sprintf (buf + strlen (buf),
				"%s tracks heading to the %s at a %s, left %d hours ago.",
				lookup_race_variable (track->race, RACE_NAME),
				dirs[track->to_dir], verbal_speeds[track->speed],
				track->hours_passed);
			if (IS_SET (track->flags, PC_TRACK))
				strcat (buf, " #2(PC)#0");
			if (IS_SET (track->flags, BLOODY_TRACK))
				strcat (buf, " #1(bloody)#0");
			if (IS_SET (track->flags, FLEE_TRACK))
				strcat (buf, " #6(fled)#0");
			strcat (buf, "\n");
		}
		ch->send_to_char (buf);
		return;
	}

	if (ch->skill_map["Tracking"] < 1)
	{
		ch->send_to_char ("You don't know how to track!\n");
		return;
	}

	if (is_dark (ch->room) && (!ch->get_trust())
		&& !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
		&& !IS_SET (ch->affected_by, AFF_INFRAVIS))
	{
		ch->send_to_char ("You can't see well enough to pick up any tracks.\n");
		return;
	}

	ch->delay = 8;
	ch->delay_type = DEL_TRACK;
	ch->send_to_char ("You survey the area carefully, searching for tracks...\n");
	ch->act("$n surveys the area slowly, searching for something.", true, 0, 0,
		TO_ROOM | _ACT_FORMAT);
}

void
do_track (CHAR_DATA * ch, char *argument, int cmd)
{
	int to_room;
	int dir;
	char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
	CHAR_DATA *tch;
	bool nodded = false;

	// If an npc was commanded by a player, disallow the track to succeed.
	if (cmd > 0 && IS_NPC (ch) && ((ch->get_trust()) >= (cmd - 1)))
		return;

	if (!IS_NPC (ch))
	{
		if ((!ch->get_trust()) || !*argument)
		{
			skill_tracking (ch, argument, cmd);
			return;
		}
	}

	argument = one_argument (argument, buf);

	if (isdigit (*buf))
		to_room = atoi (buf);
	else if ((tch = get_char_vis (ch, buf)))
		to_room = tch->in_room;
	else
	{
		ch->send_to_char ("You can't locate them.\n");
		return;
	}

	if (ch->in_room == to_room)
		return;

	dir = track (ch, to_room);

	if (dir == -1)
	{
		ch->send_to_char ("Unknown direction.\n");
		return;
	}

	if (ch->get_trust())
	{
		sprintf (buf, "[High: %d Scan: %d] Move %s\n",
			high_water, rooms_scanned, dirs[dir]);
		ch->send_to_char (buf);
	}

	high_water = 0;
	rooms_scanned = 0;

	one_argument (ch->room->dir_option[dir]->keyword, buf2);
	sprintf (buf, "%s %s", buf2, dirs[dir]);

	if (IS_SET (ch->room->dir_option[dir]->port_flags, EX_LOCKED))
	{
		if (ch->room->dir_option[dir]->key)
		{
			for (tch = ch->room->people; tch; tch = tch->next_in_room)
			{
				if (get_obj_in_list_num
					(ch->room->dir_option[dir]->key, tch->right_hand)
					|| get_obj_in_list_num (ch->room->dir_option[dir]->key,
					tch->left_hand))
				{
					name_to_ident (tch, buf2);
					sprintf (buf, "%s", buf2);
					do_nod (ch, buf, 0);
					nodded = true;
					break;
				}
			}
		}
		if (!nodded)
			do_knock (ch, buf, 0);
	}
	else if (IS_SET (ch->room->dir_option[dir]->port_flags, EX_CLOSED))
	{
		do_open (ch, buf, 0);
	}

	do_move (ch, "", dir);
}

void
do_drag (CHAR_DATA * ch, char *argument, int cmd)
{
	int dir;
	CHAR_DATA *victim = NULL;
	OBJ_DATA *obj = NULL;
	AFFECTED_TYPE *af;
	char buf[MAX_STRING_LENGTH];
	char name[MAX_STRING_LENGTH];

	argument = one_argument (argument, name);

	if (!(victim = get_char_room_vis (ch, name)))
	{
		if (!(obj = get_obj_in_list_vis (ch, name, ch->room->contents)))
		{
			ch->send_to_char ("Drag who or what?\n");
			return;
		}
	}

	if (victim)
	{
		if (ch == victim)
		{
			ch->send_to_char ("Drag yourself?\n");
			return;
		}

	}

	argument = one_argument (argument, buf);

	if ((dir = index_lookup (dirs, buf)) == -1)
	{
		ch->send_to_char ("Drag where?\n");
		return;
	}

	if (victim && victim->is_awake())
	{
		ch->act("$N is conscious, you can't drag $M.",
			false, 0, victim, TO_CHAR);
		return;
	}

	if (victim && number (1, 4) == 1)
	{
		do_wake (ch, name, 1);
		if (victim->get_position() > SLEEP)
			return;
	}

	if (victim && ch->is_subduer())
	{
		ch->send_to_char
			("You can't drag anything while you have someone in tow.\n");
		return;
	}

	if (obj && !IS_SET (obj->obj_flags.wear_flags, ITEM_TAKE))
	{
		ch->act("$p is firmly attached.  You can't drag it.",
			false, obj, 0, TO_CHAR);
		return;
	}

	magic_add_affect (ch, MAGIC_DRAGGER, -1, 0, 0, 0, 0);

	if ((af = get_affect (ch, MAGIC_DRAGGER)))
		af->a.spell.t = obj ? (long int) obj : (long int) victim;

	do_move (ch, "", dir);
}


void
do_shadow (CHAR_DATA * ch, char *argument, int cmd)
{
	AFFECTED_TYPE *af;
	CHAR_DATA *tch;
	char buf[MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		if (!(af = get_affect (ch, AFFECT_SHADOW)) || !af->a.shadow.shadow)
		{
			ch->send_to_char ("You aren't shadowing anybody.\n");
			return;
		}

		ch->send_to_char ("Ok.\n");

		af->a.shadow.shadow = NULL;

		return;
	}

	if (!(tch = get_char_room_vis (ch, buf)))
	{
		ch->send_to_char ("They aren't here to shadow.\n");
		return;
	}

	if (tch == ch)
	{
		if ((af = get_affect (ch, AFFECT_SHADOW)))
		{
			ch->send_to_char ("Ok, you stop shadowing.\n");
			af->a.shadow.shadow = NULL;
			return;
		}

		ch->send_to_char ("Ok.\n");
		return;
	}

	magic_add_affect (ch, AFFECT_SHADOW, -1, 0, 0, 0, 0);

	af = get_affect (ch, AFFECT_SHADOW);

	af->a.shadow.shadow = tch;
	af->a.shadow.edge = -1;	/* Not on an edge */

	ch->act("You will attempt to shadow $N.", false, 0, tch, TO_CHAR);
}

void
shadowers_shadow (CHAR_DATA * ch, int to_room, int move_dir)
{
	int dir;
	CHAR_DATA *tch;
	CHAR_DATA *next_in_room;
	AFFECTED_TYPE *af;
	MOVE_DATA *move;
	ROOM_EXIT_DATA *exit;
	ROOM_DATA *room;

	/* ch is leaving in direction dir.  Pick up people in same room and
	have them move to the edge.  Pick up people in surrounding rooms on
	edges and have them move to edge of ch's current room too. */

	/* Handle current room shadowers first */

	for (tch = ch->room->people; tch; tch = next_in_room)
	{

		next_in_room = tch->next_in_room;

		if (ch == tch)
			continue;

		if (GET_FLAG (tch, FLAG_LEAVING) || GET_FLAG (tch, FLAG_ENTERING))
			continue;

		if (tch->moves)
			continue;

		if (!(af = get_affect (tch, AFFECT_SHADOW)))
			continue;

		if (af->a.shadow.shadow != ch)
			continue;

		if (!can_see_mob(tch, ch))
			continue;

		move = new MOVE_DATA;

		move->dir = move_dir;
		move->flags = MF_TOEDGE;
		move->desired_time = 0;
		move->next = NULL;
		move->travel_str = NULL;

		tch->moves = move;

		initiate_move(tch);
	}

	/* Handle people who are on the edge of this room.  Those people
	will head to the edge of this room joining the room that ch is
	about to enter. */

	for (dir = 0; dir <= LAST_DIR; dir++)
	{

		if (!(exit = is_exit(ch, dir)))
			continue;

		if (exit->to_room == to_room)	/* The shadowee is returning */
			continue;

		if (!(room = vtor (exit->to_room)))
			continue;

		for (tch = room->people; tch; tch = next_in_room)
		{

			next_in_room = tch->next_in_room;

			if (ch == tch)
				continue;		/* Hopefully not possible */

			if (GET_FLAG (tch, FLAG_LEAVING) || GET_FLAG (tch, FLAG_ENTERING))
				continue;

			if (tch->moves)
				continue;

			if (!(af = get_affect (tch, AFFECT_SHADOW)))
				continue;

			if (af->a.shadow.shadow != ch)
				continue;

			if (af->a.shadow.edge != rev_dir[dir])
				continue;

			if (!could_see (tch, ch))	/* Make sure shadowee is visable */
				continue;

			/* Make N/PC enter room of ch as ch leaves */

			move = new MOVE_DATA;

			move->dir = af->a.shadow.edge;
			move->flags = MF_TONEXT_EDGE;
			move->desired_time = 0;
			move->next = NULL;
			move->travel_str = NULL;

			tch->moves = move;

			/* Make N/PC move to edge joining room ch just entered */

			move = new MOVE_DATA;

			move->dir = move_dir;
			move->flags = MF_TOEDGE;
			move->desired_time = 0;
			move->next = NULL;
			move->travel_str = NULL;

			tch->moves->next = move;

			initiate_move(tch);
		}
	}
}

int
room_avail(ROOM_DATA *troom, OBJ_DATA *tobj, CHAR_DATA *tch)
{
	int count = 0;
	int obj_wt = 0;
	int tot_wt = 0;
	float wt_count = 0.0;
	CHAR_DATA *temp_char = NULL;
	OBJ_DATA *temp_obj = NULL;

	if (troom->capacity == 0)
		return(1);

	if (tobj)
		obj_wt = obj_mass(tobj)/100;

	if (tch && (IS_NPC (tch) || (!tch->get_trust())))
	{
		if (IS_NPC(tch))
			tot_wt = (tch->carrying()/100) + 200;
		else
			tot_wt = (tch->carrying()/100) + (tch->get_weight()/100);
	}

		//obj_wt is weight in pounds of objects in room
	for (temp_obj = troom->contents; temp_obj; temp_obj = temp_obj->next_content)
	{
		obj_wt = obj_wt + (obj_mass(temp_obj)/100);
	}

		//tot_wt is weight of people (200 pounds each) plus stuff they are carrying
	for (temp_char = troom->people; temp_char; temp_char = temp_char->next_in_room)
	{
		if (IS_NPC (temp_char) || (!temp_char->get_trust()))
		{
			if (IS_NPC(temp_char))
				tot_wt = tot_wt + (temp_char->carrying()/100) + 200;
			else
				tot_wt = tot_wt + (temp_char->carrying()/100) + (temp_char->get_weight()/100);
			count++;
		}
	}

	wt_count = (tot_wt + obj_wt);

	//total weight in pounds (room->contents already used, so using 'occupants' here)
	troom->occupants = (int)(wt_count); 

		//room->capacity is number of people (at 200 pounds each)
	if (wt_count >= (troom->capacity * 200))
	{
		return (0); //there is no room
	}
	else
	{
		return (1); //there is room
	}

	return(0);

}


int
force_enter (CHAR_DATA *tch, ROOM_DATA *troom)
{
	int mod = 0;
	int attr_str = 0;
	int attr_agi = 0;
	int num_roll = 0;
	int check_roll = 0;
	int forced = 0;
	float occup;
	float capac;

	attr_str = tch->tmp_str;
	attr_agi = tch->tmp_agi;

	occup = troom->occupants;
	capac = troom->capacity;


	if (capac > 0)
	{
		if (occup > (capac) && (occup <= (1.25 * capac)))
			mod = 1;
		else if (occup > (1.25 * capac) && (occup <= (1.50 * capac)))
			mod = 2;
		else if (occup > (1.50 * capac) && (occup <= (1.75 * capac)))
			mod = 3;
		else if (occup > (1.75 * capac) && (occup <= (2.00 * capac)))
			mod = 4;
		else if (occup > (2.00 * capac))
			mod = 5;

		num_roll = 12 + (mod * 3);
		check_roll = (number (1, num_roll));

		if ( check_roll < attr_str)
		{
			tch->send_to_char ("You force your way inside.\n");
			forced = 1; 
		}
		else if ((number (1, num_roll) < attr_agi))
		{
			tch->send_to_char ("You manage to squeeze your way in.\n");
			forced = 1; 
		}
		else
			forced = 0; 
	}

	else //treat as infinite capacity when troom->capacity <= 0
	{
		forced = 1;
	}

	if (forced)
	{
			//TODO: good chance to remove random objects and/or people from room if the room is over capacity now
	}
	
	return (forced); 
}

void
movement_wear(CHAR_DATA* ch, int needed_movement)
{

	return;
}

	//moves character to the destination room,
	//and gives them damage based on height
	//used to be failure to climb and travel through FALL rooms
void
fallen_result(CHAR_DATA* ch, ROOM_PORTAL_DATA* tport)
{
	
	int died;
	int rooms_fallen;
	
	ch->clear_moves();
	ch->clear_current_move();
	
	ch->char_from_room();
	ch->char_to_room(tport->fail_room);
	
	do_look (ch, "", 1);
	
	
	ch->send_to_char ("\n\nYou land with a thud.\n");
		ch->act("$n lands with a thud!", false, 0, 0, TO_ROOM);
	
	
		//assumes rooms are about 16 high, so figure damage based on falling 16 feet
	rooms_fallen = tport->feet_fallen / 16;
	
			
	if (!died)
		knock_out (ch, 15);
	
	return;
}

void
do_stop (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch;
	AFFECTED_TYPE *af;
	
	if (ch->moves)
	{
		
		ch->clear_moves();
		ch->clear_current_move();
		
		ch->send_to_char ("Movement commands cancelled.\n");
		
		return;
	}
	
	if ((af = is_crafting (ch)))
	{
		ch->act("$n stops doing $s craft.", false, 0, 0, TO_ROOM);
		ch->send_to_char ("You stop doing your craft.\n");
		af->a.craft->timer = 0;
		return;
	}
	
	
	if (ch->clear_current_move())
		return;
	
	if (ch->delay)
	{
		ch->break_delay();
		return;
	}
	
	ch->action |= PLR_STOP;	/* Same as ACT_STOP */
	
	
}
