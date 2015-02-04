//////////////////////////////////////////////////////////////////////////////
//
/// structs.cpph - Structures and Access Functions
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

#include "structs.h"

default_mob_data::default_mob_data() {
	flags = 0;
	memset(mobs, 0, MAX_DEFAULT_MOBS * sizeof(int));
	mob_counts = 1;
	phase_num = 0;
	key_index = 0;
}

default_mob_data::default_mob_data(const default_mob_data &RHS) {
	flags = RHS.flags;
	memcpy(mobs, RHS.mobs, MAX_DEFAULT_MOBS * sizeof(int));
	mob_counts = RHS.mob_counts;
	phase_num = RHS.phase_num;
	key_index = RHS.key_index;
}

default_mob_data& default_mob_data::operator= (const default_mob_data &RHS) {
	if (this != &RHS) {
		flags = RHS.flags;
		memcpy(mobs, RHS.mobs, MAX_DEFAULT_MOBS * sizeof(int));
		mob_counts = RHS.mob_counts;
		phase_num = RHS.phase_num;
		key_index = RHS.key_index;
	}
	return *this;
}

default_mob_data::~default_mob_data() {
	
	
}

default_item_data::default_item_data() {
	flags = 0;
	memset(items, 0, MAX_DEFAULT_ITEMS * sizeof(int));
	item_counts = 1;
	phase_num = 0;
	key_index = 0;
	color = NULL;
}

default_item_data::default_item_data(const default_item_data &RHS) {
	flags = RHS.flags;
	memcpy(items, RHS.items, MAX_DEFAULT_ITEMS * sizeof(int));
	item_counts = RHS.item_counts;
	phase_num = RHS.phase_num;
	key_index = RHS.key_index;
	color = duplicateString(RHS.color);
}

default_item_data& default_item_data::operator= (const default_item_data &RHS) {
	if (this != &RHS) {
		flags = RHS.flags;
		memcpy(items, RHS.items, MAX_DEFAULT_ITEMS * sizeof(int));
		item_counts = RHS.item_counts;
		phase_num = RHS.phase_num;
		key_index = RHS.key_index;

		free_mem(color);
		color = duplicateString(RHS.color);
	}
	return *this;
}


default_item_data::~default_item_data() {
	free_mem(color);

}

phase_data::phase_data() {
	first = NULL;
	third = NULL;
	phase_seconds = 0;
	skill = 0;
	dice = 0;
	sides = 0;
	hit_cost = 0;
	move_cost = 0;
	attribute = -1;
	group_mess = NULL;
}

phase_data::phase_data(const phase_data &RHS) {
	first = duplicateString(RHS.first);
	third = duplicateString(RHS.third);
	phase_seconds = RHS.phase_seconds;
	skill = RHS.skill;
	dice = RHS.dice;
	sides = RHS.sides;
	hit_cost = RHS.hit_cost;
	move_cost = RHS.move_cost;
	attribute = RHS.attribute;
	group_mess = duplicateString(RHS.group_mess);
	
}

phase_data& phase_data::operator= (const phase_data &RHS) {
	if (this != &RHS) {
		free_mem(first);
		first = duplicateString(RHS.first);

		free_mem(third);
		third = duplicateString(RHS.third);

		phase_seconds = RHS.phase_seconds;
		skill = RHS.skill;
		dice = RHS.dice;
		sides = RHS.sides;
		hit_cost = RHS.hit_cost;
		move_cost = RHS.move_cost;
		attribute = RHS.attribute;
	
		free_mem(group_mess);
		group_mess = duplicateString(RHS.group_mess);
	}
	return *this;
}

phase_data::~phase_data() {
	free_mem(first);
	free_mem(third);
	free_mem(group_mess);

}

subcraft_head_data::subcraft_head_data() {
	craft_name = NULL;
	subcraft_name = NULL;
	command = NULL;
	clans = NULL;
	personal_string = NULL;

	memset(obj_items, 0, (MAX_ITEMS_PER_SUBCRAFT * sizeof(DEFAULT_ITEM_DATA*)));
	memset(mob_items, 0, (MAX_MOBS_PER_SUBCRAFT * sizeof(DEFAULT_MOB_DATA*)));
	memset(terrains, 0, TERRAINSMAX * sizeof(int));
	memset(seasons, 0, SEASONSMAX * sizeof(int));
	memset(opening, 0, OPENINGMAX * sizeof(int));
	memset(race, 0, RACEMAX * sizeof(int));
	memset(weather, 0, WEATHERMAX * sizeof(int));
	memset(phases, 0, (MAX_PHASES_PER_SUBCRAFT * sizeof(PHASE_DATA*)));
	
	subcraft_flags = 0;
	delay = 0;
	key_index = 0;
	key_first = 0;
	key_end = 0;
	followers = 0;

}

subcraft_head_data::subcraft_head_data(const subcraft_head_data &RHS) {
	craft_name = duplicateString(RHS.craft_name);
	subcraft_name = duplicateString(RHS.subcraft_name);
	command = duplicateString(RHS.command);
	clans = duplicateString(RHS.clans);
	personal_string = duplicateString(RHS.personal_string);

	for (int i = 0; i < MAX_PHASES_PER_SUBCRAFT; i++)
	{
		if (RHS.phases[i] != NULL)
		{
			phases[i] = new phase_data(*(RHS.phases[i]));
		}
		else
		{
			phases[i] = NULL;
		}
	}
	

	for (int i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{
		if (RHS.obj_items[i] != NULL)
		{
			obj_items[i] = new default_item_data(*(RHS.obj_items[i]));
		}
		else
		{
			obj_items[i] = NULL;
		}
	}

	for (int i = 0; i < MAX_MOBS_PER_SUBCRAFT; i++)
	{
		if (RHS.mob_items[i] != NULL) 
		{
			mob_items[i] = new default_mob_data(*(RHS.mob_items[i]));
		}
		else
		{
			mob_items[i] = NULL;
		}
		
	}
	
	subcraft_flags = RHS.subcraft_flags;

	memcpy(terrains, RHS.terrains, TERRAINSMAX * sizeof(int));
	memcpy(seasons, RHS.seasons, SEASONSMAX * sizeof(int));
	memcpy(opening, RHS.opening, OPENINGMAX * sizeof(int));
	memcpy(race, RHS.race, RACEMAX * sizeof(int));
	memcpy(weather, RHS.weather, WEATHERMAX * sizeof(int));

	delay = RHS.delay;
	key_index = RHS.key_index;
	key_first = RHS.key_first;
	key_end = RHS.key_end;
	followers = RHS.followers;

}

subcraft_head_data& subcraft_head_data::operator= (const subcraft_head_data &RHS) {
	if (this != &RHS) 
	{
		free_mem(craft_name);
		craft_name = duplicateString(RHS.craft_name);

		free_mem(subcraft_name);
		subcraft_name = duplicateString(RHS.subcraft_name);

		free_mem(command);
		command = duplicateString(RHS.command);

		free_mem(clans);
		clans = duplicateString(RHS.clans);
		
		free_mem(personal_string);
		personal_string = duplicateString(RHS.personal_string);


		for (int i = 0; i < MAX_PHASES_PER_SUBCRAFT; i++) {
			if (phases[i] != NULL) {
				delete phases[i];
				phases[i] = NULL;
			}
			
			if (RHS.phases[i] != NULL) {
				phases[i] = new phase_data(*(RHS.phases[i]));
			}
			else
			{
				phases[i] = NULL;
			}
			
		}

		for (int i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++) {
			if (obj_items[i] != NULL) {
				delete obj_items[i];
				obj_items[i] = NULL;
			}

			if (RHS.obj_items[i] != NULL) {
				obj_items[i] = new default_item_data(*(RHS.obj_items[i]));
			}
			else {
				obj_items[i] = NULL;
			}

		}

		for (int i = 0; i < MAX_MOBS_PER_SUBCRAFT; i++) {
			if (mob_items[i] != NULL) {
				delete mob_items[i];
				mob_items[i] = NULL;
			}
			
						
			if (RHS.mob_items[i] != NULL) {
				mob_items[i] = new default_mob_data(*(RHS.mob_items[i]));
			}
			else {
				mob_items[i] = NULL;
			}
			
		}
		
		
		subcraft_flags = RHS.subcraft_flags;

		memcpy(terrains, RHS.terrains, TERRAINSMAX * sizeof(int));
		memcpy(seasons, RHS.seasons, SEASONSMAX * sizeof(int));
		memcpy(opening, RHS.opening, OPENINGMAX * sizeof(int));
		memcpy(weather, RHS.weather, WEATHERMAX * sizeof(int));
		memcpy(race, RHS.race, RACEMAX * sizeof(int));
		

		delay = RHS.delay;
		key_index = RHS.key_index;
		key_first = RHS.key_first;
		key_end = RHS.key_end;
		followers = RHS.followers;
	}
	return *this;
}

subcraft_head_data::~subcraft_head_data() {
	free_mem(craft_name);
	free_mem(subcraft_name);
	free_mem(command);
	free_mem(personal_string);
	free_mem(clans);

	for (int i = 0; i < MAX_PHASES_PER_SUBCRAFT; i++) {
		if (phases[i] != NULL) {
			delete phases[i];
			phases[i] = NULL;
		}
	}
	
	for (int i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++) {
		if (obj_items[i] != NULL) {
			delete obj_items[i];
			obj_items[i] = NULL;
		}
	}

	for (int i = 0; i < MAX_MOBS_PER_SUBCRAFT; i++) {
		if (mob_items[i] != NULL) {
			delete mob_items[i];
			mob_items[i] = NULL;
		}
	}
}
