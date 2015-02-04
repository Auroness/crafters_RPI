//////////////////////////////////////////////////////////////////////////////
//
/// hash.cpp : Central Hash Module
//
//
// TODO: this is mostly loading fucntion - maybe move to game set up class?
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

#include "server.h"
#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"
#include "room.h"

extern rpie::server engine;
extern std::list<char_data*> character_list;
extern std::list<obj_data*> object_list;
extern std::map<int, VARIANT_VALUE*> gl_variant;


	//for loading a new mobile into the world
CHAR_DATA *
load_mobile (int vnum)
{
	CHAR_DATA *proto;
	CHAR_DATA *new_mobile;
	MOB_DATA *mob_info;

	if (!(proto = vtom (vnum)))
		return NULL;

	new_mobile = new_char (0);	/* NPC */

	mob_info = new_mobile->mob;

	new_mobile->deep_copy(proto);

	new_mobile->mob = mob_info;


	/* A mostly unique number.  Can be used to ensure
	the same mobile is being used between game plays.  */

	new_mobile->coldload_id = get_next_coldload_id (0);
	new_mobile->deleted = 0;

	
	if (new_mobile)
		character_list.push_front(new_mobile);

	new_mobile->time_str.birth = time (0);

	new_mobile->max_move = calc_lookup (new_mobile, REG_MISC, MISC_MAX_MOVE);

	if (!new_mobile->height)
		make_height (new_mobile);

	if (!new_mobile->frame)
		make_frame (new_mobile);

	if (IS_SET (new_mobile->affected_by, AFF_HIDE) && !get_affect (new_mobile, MAGIC_HIDDEN))
		magic_add_affect (new_mobile, MAGIC_HIDDEN, -1, 0, 0, 0, 0);


	if (IS_SET (new_mobile->flags, FLAG_VARIABLE))
	{
		new_mobile->str = proto->str;
		new_mobile->dex = proto->dex;
		new_mobile->intel = proto->intel;
		new_mobile->aur = proto->aur;
		new_mobile->agi = proto->agi;
		new_mobile->con = proto->con;
		new_mobile->wil = proto->wil;
		new_mobile->luk = proto->luk;
		randomize_mobile (new_mobile);
		new_mobile->flags &= ~FLAG_VARIABLE;
	}


	new_mobile->move = new_mobile->max_move;

	
	new_mobile->subdue = NULL;

	new_mobile->mob->owner = NULL;

	if ((new_mobile->mob->clock > 0) && (new_mobile->mob->morphto > 0))
	{
		new_mobile->mob->morph_time = time (0) + new_mobile->mob->clock * 15 * 60;
	}

	new_mobile->speaks = strdup(proto->speaks);

	if (IS_NPC (new_mobile) && !IS_SET (new_mobile->mob->action, ACT_STAYPUT))
	{
		new_mobile->mob->action |= ACT_STAYPUT;
		save_stayput_mobiles ();
	}
	
	return new_mobile;
}

	//replaces all instances of $name with "value assocaited with name"
	//and reutrns the altered object
OBJ_DATA *
replace_variant_string(OBJ_DATA * new_obj, VARIANT_VALUE* variant)
{
	
	int found;
	std::string origin;
	
	if (!variant)
		return(new_obj);
	
	
		//change sdesc
	origin.assign(new_obj->short_description);
	found = origin.find(variant->name);
	if (found !=std::string::npos)
	{
		origin.replace(found, strlen(variant->name), variant->var_value);
		free_mem (new_obj->short_description);
		new_obj->short_description = duplicateString (origin.c_str());
	}
	
		//change long description
	origin.assign(new_obj->description);
	found = origin.find(variant->name);
	if (found !=std::string::npos)
	{
	origin.replace(found, strlen(variant->name), variant->var_value);
	free_mem (new_obj->description);
	new_obj->description = duplicateString (origin.c_str());
	}
		//change full description
	origin.assign(new_obj->full_description);
	found = origin.find(variant->name);
	if (found !=std::string::npos)
	{
	origin.replace(found, strlen(variant->name), variant->var_value);
	free_mem (new_obj->full_description);
	new_obj->full_description = duplicateString (origin.c_str());
	}
	
		//change name/keywords
	origin.assign(new_obj->name);
	found = origin.find(variant->name);
	if (found !=std::string::npos)
	{
	origin.replace(found, strlen(variant->name), variant->var_value);
	free_mem (new_obj->name);
	new_obj->name = duplicateString (origin.c_str());
	}
	
	
		//change desc_keys for masks and armor
	if ((IS_SET (new_obj->obj_flags.extra_flags, ITEM_MASK)
			&& new_obj->obj_flags.type_flag == ITEM_WORN))
	{
		origin.assign(new_obj->desc_keys);
		found = origin.find(variant->name);
		if (found !=std::string::npos)
		{
		origin.replace(found, strlen(variant->name), variant->var_value);
		free_mem (new_obj->desc_keys);
		new_obj->desc_keys = duplicateString (origin.c_str());
		}
	}
				
		//assign ink color if needed
	if (new_obj->obj_flags.type_flag == ITEM_INK)
	{
		origin.assign(new_obj->ink_color);
		found = origin.find(variant->name);
		if (found !=std::string::npos)
		{
			origin.replace(found, strlen(variant->name), variant->var_value);
			free_mem (new_obj->ink_color);
			new_obj->ink_color = duplicateString (origin.c_str());
		}
	}
	
	new_obj->var_val.push_back(variant->ident);
	
	return(new_obj);
	
}

void
insert_string_variables (OBJ_DATA * new_obj, char *variant_string)
{
	char test_word[MAX_STRING_LENGTH]= { '\0' };
	char *work_variant_str;
	int i, limit;
	std::string origin, index_val, sdesc, ldesc, fdesc, keywords;
	std::string replacement;
	VARIANT_VALUE* tvariant;
	std::map<int, VARIANT_VALUE*>::iterator var_it;
	VARIANT_VALUE* tvariant2;
	std::map<int, VARIANT_VALUE*>::iterator var_it2;
	std::vector<VARIANT_VALUE*> var_vect;
	int tvar_save;
				

		//if there are no keywords, they don't get any variables parsed in
	if (!*new_obj->name)
		return;
		
	if (IS_SET (new_obj->obj_flags.extra_flags, ITEM_VARIABLE))
		new_obj->obj_flags.extra_flags &= ~ITEM_VARIABLE;

	//variant_string could be "(reddish blue) (green) (diamond)"
	
			
	if ((variant_string == 0) || (!str_cmp(variant_string, "")))
	{
		
		for (var_it = gl_variant.begin(); var_it != gl_variant.end(); var_it++)
		{
			tvariant = var_it->second;
			
				//find a "$value" that is in new_obj->name
			if (strstr (new_obj->name, tvariant->name)) 
			{
					//now find all of them, and add to the vector
				for (var_it2 = gl_variant.begin(); var_it2 != gl_variant.end(); var_it2++)
				{
					tvariant2 = var_it2->second;
					if (!str_cmp(tvariant->name, tvariant2->name))
					{
						var_vect.insert(var_vect.begin(), tvariant2);
					}
					
				}
				
					//pull one out of vector at random
				limit = var_vect.size() - 1;
				int tnum = number (0, limit);
				
				new_obj = replace_variant_string(new_obj, var_vect[tnum]);
			}
		}
	}
	
	else
	{
		work_variant_str = strdup(variant_string);
		i = 1;
		while (i < strlen(work_variant_str))
		{
			if (work_variant_str[i] == '\n')
			{
				break;
			}
			
			while (isspace (work_variant_str[i]))
			{
				i++;
			}
			
			if (work_variant_str[i] == '(')
			{
				i++;
			}
			
			while (work_variant_str[i] != ')')
			{
				sprintf (test_word + strlen (test_word), "%c", work_variant_str[i]);
				i++;
			}
			
			if (work_variant_str[i] == ')')
			{
				i++;
				
					//find a variable reference in keywords
					//find a variant name(with the $ at the front) in the gl_variant list and see if it is in the keywords
					//if it is, make up a vector of just variants with that name
					//pick one at random from the vector
					//replace the instances of it in new_object description strings
				
					//first test_word would be "reddish blue"
				if (*test_word)
				{
					for (var_it = gl_variant.begin(); var_it != gl_variant.end(); var_it++)
					{
						tvariant = var_it->second;
						if (!str_cmp(test_word, tvariant->var_value))
						{
							index_val = tvariant->name;
							tvar_save = tvariant->ident;
							
								//"red" is a $color, so replace this str with "red"
							new_obj = replace_variant_string(new_obj, tvariant);
							test_word[0] = '\0';
							break;					
						}
					}
				}
			}
		}
			
	}
			
}

OBJ_DATA * load_object (int vnum)
{
	return load_object_full (vnum,true,1);
}

// if newWritingID is set, it will find an unused writing ID and assign it to the object
// This was done because sometimes load_object is called to get the prototype, then ovals
// are loaded afterwards. Thus it generates a new writing ID, then ignores it because it loads
// up the writing from the ovals later.
// all objects loaded in the initial fread from file already have ovals and call this
// function with false, to avoid duplicate work
OBJ_DATA *
load_object_full (int vnum, bool newWritingID, int l_count)
{
	OBJ_DATA *proto;
	OBJ_DATA *new_obj;
	OBJ_DATA *tobj;
	WRITING_DATA *writing;
	int i;
	bool keep_obj = true;
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *new_af;
	AFFECTED_TYPE *last_af = NULL;
	std::set<std::string>::iterator materials_it;
	std::list<OBJ_DATA*>::iterator tobj_iterator;
	
	if (!(proto = vtoo (vnum)))
		return NULL;

	if (proto->zone == 99)
		return NULL;
	
	new_obj = new_object ();

	new_obj->deep_copy(proto);

	new_obj->deleted = 0;

	new_obj->xaffected = NULL;

	new_obj->next_content = 0;
	
	
	for (af = proto->xaffected; af; af = af->next)
	{
		new_af = new AFFECTED_TYPE;
		memcpy (new_af, af, sizeof (AFFECTED_TYPE));
		new_af->next = NULL;
		
		if (!new_obj->xaffected)
			new_obj->xaffected = new_af;
		else
			last_af->next = new_af;

		last_af = new_af;
	}
	
	if (l_count)
		new_obj->count = l_count;
	else 
		new_obj->count = 1;
	
	if (new_obj->clock && new_obj->morphto)
		new_obj->morphTime = time (0) + new_obj->clock * 15 * 60;

	// do not generate and do the expensive unused id function if the ovals will be set
	// by another means, referencing existing writing.
	if (newWritingID)
	{
		//Since this can be called on any type of writing object, generate the new ID inside the ifs
		if (new_obj->obj_flags.type_flag == ITEM_BOOK)
		{	
			/* if there is no writing and it has more than 0 pages assigned to it */
			if (!new_obj->writing && new_obj->o.od.value[0] > 0)
			{
				new_obj->writing = new WRITING_DATA;
				new_obj->writing->next_page = NULL;

				for (i = 1, writing = new_obj->writing; i <= new_obj->o.od.value[0]; i++)
				{
					writing->message = duplicateString ("blank");
					writing->author = duplicateString ("blank");
					writing->date = duplicateString ("blank");
					writing->ink = duplicateString ("blank");
					writing->language = 0;
					writing->script = 0;
					writing->skill = 0;
					writing->torn = false;
					if (i != new_obj->o.od.value[0])
					{
						writing->next_page = new WRITING_DATA;
						writing->next_page->next_page = NULL;
						writing = writing->next_page;
					}
					
				}
				new_obj->o.od.value[1] = unused_writing_id(); // generate a new ID only once, all pages indexed by page ID off the key
				save_writing(new_obj);
			}
		} // if book

		if (new_obj->obj_flags.type_flag == ITEM_PARCHMENT)
		{
			new_obj->o.od.value[0] = unused_writing_id();
		}
	} // end if new writing id section

	if (IS_SET (new_obj->obj_flags.extra_flags, ITEM_VARIABLE))
	{
		insert_string_variables (new_obj, "");
	}
	
	new_obj->quality = proto->quality;
	new_obj->max_hit_points = (get_material_hardness(proto) * (proto->obj_flags.weight)/100.0);
	if (new_obj->max_hit_points > 0 && new_obj->max_hit_points < 1)
		new_obj->max_hit_points = 1;

	
	new_obj->contains = NULL;
	new_obj->equiped_by = NULL;
	new_obj->carried_by = NULL;
	new_obj->in_obj = NULL;
	
	if (!new_obj->item_wear)
		new_obj->item_wear = 100;

	new_obj->coldload_id = get_next_coldload_id (2);

	vtoo (new_obj->nVirtual)->instances++;

	for (tobj_iterator = object_list.begin(); tobj_iterator != object_list.end(); tobj_iterator++)
	{
		tobj = *tobj_iterator;
		if (tobj->coldload_id == new_obj->coldload_id)
			keep_obj = false;
	}
	if (keep_obj)
	{
		if (object_list.size() == 0)
			object_list.assign(1, new_obj);
		else 
			object_list.push_back(new_obj);
	}
	return new_obj;
}

OBJ_DATA *
load_colored_object (int vnum, char *variant_values)
{
	OBJ_DATA *proto;
	OBJ_DATA *new_obj;
	OBJ_DATA *tobj;
	WRITING_DATA *writing;
	int i;
	bool keep_obj = true;
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *new_af;
	AFFECTED_TYPE *last_af = NULL;
	std::list<OBJ_DATA*>::iterator tobj_iterator;

	if (!(proto = vtoo (vnum)))
		return NULL;

		//since load_colored_object is called by _do_load,
		//it might not be a colored obejct
	if (!IS_SET (proto->obj_flags.extra_flags, ITEM_VARIABLE))
	{
		return(load_object(vnum));
	}
		
	new_obj = new_object ();

	new_obj->deep_copy(proto);

	new_obj->deleted = 0;

	new_obj->xaffected = NULL;


	for (af = proto->xaffected; af; af = af->next)
	{

		new_af = new AFFECTED_TYPE;

		memcpy (new_af, af, sizeof (AFFECTED_TYPE));

		new_af->next = NULL;

		if (!new_obj->xaffected)
			new_obj->xaffected = new_af;
		else
			last_af->next = new_af;

		last_af = new_af;
	}

	new_obj->count = 1;

	if (new_obj->clock && new_obj->morphto)
		new_obj->morphTime = time (0) + new_obj->clock * 15 * 60;

	if (new_obj->obj_flags.type_flag == ITEM_BOOK)
	{
		if (!new_obj->writing && new_obj->o.od.value[0] > 0)
		{
			new_obj->writing = new WRITING_DATA;
			new_obj->writing->next_page = NULL;

			for (i = 1, writing = new_obj->writing; i <= new_obj->o.od.value[0];
				i++)
			{
				writing->message = duplicateString ("blank");
				writing->author = duplicateString ("blank");
				writing->date = duplicateString ("blank");
				writing->ink = duplicateString ("blank");
				writing->language = 0;
				writing->script = 0;
				writing->skill = 0;
				writing->torn = false;
				if (i != new_obj->o.od.value[0])
				{
					writing->next_page = new WRITING_DATA;
					writing->next_page->next_page = NULL;
					writing = writing->next_page;
				}
			}
			// generate a new ID only once, all pages indexed by page ID off the key
			new_obj->o.od.value[1] = unused_writing_id(); 
			save_writing(new_obj);
		}
	}

	
	if (IS_SET (new_obj->obj_flags.extra_flags, ITEM_VARIABLE))
		insert_string_variables (new_obj, variant_values);
	
	new_obj->coldload_id = get_next_coldload_id (2);

	vtoo (new_obj->nVirtual)->instances++;

	for (tobj_iterator = object_list.begin(); tobj_iterator != object_list.end(); tobj_iterator++)
	{
		tobj = *tobj_iterator;
		if (tobj->coldload_id == new_obj->coldload_id)
			keep_obj = false;
	}
	if (keep_obj)
	{
		if (object_list.size() == 0)
			object_list.assign(1, new_obj);
		else 
			object_list.push_back(new_obj);
	}
	
	return new_obj;
}

int
calculate_race_height (CHAR_DATA * tch)
{
	int min = 0, max = 0;
	float percentile = 0.0;

	if (tch->race < 0)
		tch->race = 0;
	
	if (tch->mob && tch->mob->min_height && tch->mob->max_height)
	{
		min = tch->mob->min_height;
		max = tch->mob->max_height;
		if (max < min)
			max += number (1, 10);
	}
	else if (!lookup_race_variable (tch->race, RACE_MIN_HEIGHT)
		|| !lookup_race_variable (tch->race, RACE_MAX_HEIGHT))
		return -1;
	else
	{
		min = atoi (lookup_race_variable (tch->race, RACE_MIN_HEIGHT));
		max = atoi (lookup_race_variable (tch->race, RACE_MAX_HEIGHT));
	}

	if (!IS_NPC (tch))
	{
		if (tch->height == 1)	// short
			percentile = (float) (number (1, 45) / 100.0);
		else if (tch->height == 2)	// average
			percentile = (float) (number (45, 55) / 100.0);
		else
			percentile = (float) (number (55, 100) / 100.0);	// tall
	}
	else
		percentile = (float) (number (25, 85) / 100.0);

	return ((int) (min + ((max - min) * percentile)));
}

int
calculate_size_height (CHAR_DATA * tch)
{
	if (tch->size == -3)		// XXS  (insect)
		return (number (1, 3));
	else if (tch->size == -2)	// XS   (rodent, rabbit, etc.)
		return (number (3, 7));
	else if (tch->size == -1)
	{				// S    (small humanoid, dog, cat, etc.)
		if (tch->body_proto == PROTO_HUMANOID)
			return (number (36, 60));
		else
			return (number (36, 54));
	}
	else if (tch->size == 0)
	{				// M    (average humanoid, livestock, etc.)
		if (tch->body_proto == PROTO_HUMANOID)
			return (number (60, 72));
		else
			return (number (36, 54));
	}
	else if (tch->size == 1)	// L    (large humanoid, troll)
		return (number (120, 156));
	else if (tch->size == 2)	// XL   (ent, giant)
		return (number (180, 240));
	else				// XXL  (dragon)
		return (number (300, 480));
}

void
make_height (CHAR_DATA * mob)
{
	if ((mob->height = calculate_race_height (mob)) == -1)
		mob->height = calculate_size_height (mob);

	return;
}

void
make_frame (CHAR_DATA * mob)
{
	if (mob->race < 0)
		mob->race = 0;
	
	if (!lookup_race_variable (mob->race, RACE_ID))
	{
		mob->frame = 3;
		return;
	}

	if (mob->sex == SEX_MALE)
		mob->frame = 3 + number (-1, 3);
	else
		mob->frame = 3 + number (-3, 1);
}


void
cleanup_the_dead (int mode)
{
	OBJ_DATA *tobj;
	CHAR_DATA *ch;
	std::list<char_data*>::iterator tch_iterator;
	std::list<obj_data*>::iterator tobj_iterator;
	
	if (mode == 1 || mode == 0)
	{
		for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
		{
			ch = *tch_iterator;
			if (ch)
			{
				if (!ch->deleted)
				{
					continue;
				}
				
				
				character_list.remove(ch);
					//we have to go back to the begining,
					//so we can skip the deleted character
					//since it is removed, character_list.end() is no longer valid
				tch_iterator = character_list.begin();
				
				if (!IS_NPC (ch))
				{
					ch->unload_pc();
					continue;
				}
				
				free_char (ch);
			}
		}
	}
	
	if (mode == 2 || mode == 0)
	{
		for (tobj_iterator = object_list.begin(); tobj_iterator != object_list.end(); tobj_iterator++)
		{
			tobj = *tobj_iterator;
			if (tobj)
			{
				if (!tobj->deleted)
				{
					continue;
				}
				
				object_list.remove(tobj);
					//we have to go back to the begining, so we can skip the deleted object
					//since it is removed, object_list.end() is no longer valid
				tobj_iterator = object_list.begin();
				
			}
		}
	}
}

