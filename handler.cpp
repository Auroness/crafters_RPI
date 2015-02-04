//////////////////////////////////////////////////////////////////////////////
//
/// handler.cpp : Handler Module
//
//
// TODO: since it is mostly utility functions, combine with other files or move to appropriate class files?
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

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include "server.h"
#include "structs.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "group.h"
#include "utility.h"

extern std::map<std::string, SKILL_DATA*> skill_data_map;
extern int knockout;
extern rpie::server engine;
extern const char *dirs[];

const char *ignore[5] = {
	"the",
	"in",
	"on",
	"at",
	"\n"
};

const char *targets[2] = {
	"Self",
	"\n"
};

/* use_table_data is filled in at boot time from registry */

struct use_table_data use_table[] = {
	{8},				/* BRAWLING */
	{10},
	{10},
	{10},
	{16},
	{16},
	{16},
	{22},
	{22},
	{22},
	{25},
	{25},
	{0},
	{0},
	{0},
	{0}
};



	//returns a pointer to a copy of the first word in a list of words
char *fname (char *namelist)
{
	static char holder[30];
	char *point;

	if (!namelist)
		return NULL;

	for (point = holder; isalpha (*namelist); namelist++, point++)
		*point = *namelist;

	*point = '\0';

	return (holder);
}


int isname (const char *str, char *namelist)
{
	char *curname = '\0';
	const char *curstr = '\0';

	if (!str)
		return 0;

	if (!namelist)
		return 0;
	
	curname = namelist;
	for (;;)
	{
		for (curstr = str;; curstr++, curname++)
		{
			if ((!*curstr && !isalpha (*curname))
				|| is_abbrev (curstr, curname))
				return (1);

			if (!*curname)
				return (0);

			if (!*curstr || *curname == ' ')
				break;

			if (tolower (*curstr) != tolower (*curname))
				break;
		}

		/* skip to next name */

		for (; isalpha (*curname); curname++);
		if (!*curname)
			return (0);
		curname++;		/* first char of new name */
	}
}

// Case-sensitive version of isname().

int isnamec (char *str, char *namelist)
{
	char *curname = '\0';
	char *curstr = '\0';

	if (!str)
		return 0;

	if (!namelist)
		return 0;

	curname = namelist;
	for (;;)
	{
		for (curstr = str;; curstr++, curname++)
		{
			if ((!*curstr && !isalpha (*curname))
				|| is_abbrevc (curstr, curname))
				return (1);

			if (!*curname)
				return (0);

			if (!*curstr || *curname == ' ')
				break;

			if (*curstr != *curname)
				break;
		}

		/* skip to next name */

		for (; isalpha (*curname); curname++);
		if (!*curname)
			return (0);
		curname++;		/* first char of new name */
	}
}


void affect_modify (CHAR_DATA * ch, int type, int loc, int mod, int bitv,
			   int add, int sn)
{
	if (type >= JOB_1 && type <= JOB_3)
		return;

	if (type >= CRAFT_FIRST && type <= CRAFT_LAST)
		return;

	if (type >= MAGIC_FIRST_SOMA && type <= MAGIC_LAST_SOMA)
		return;
	
	if (type == MAGIC_SIT_TABLE)
		return;
	
	
	if (type == AFFECT_SHADOW)
		return;

	if (type == MUTE_EAVESDROP)
		return;

	if (bitv)
		ch->affected_by |= bitv;
	else
		ch->affected_by &= ~bitv;
		
	if (!add)
		mod = -mod;

	/*
	switch (type) {
	case SPELL_STRENGTH:		ch->tmp_str += mod;				return;
	case SPELL_DEXTERITY:		ch->tmp_dex += mod;				return;
	case SPELL_INTELLIGENCE:	ch->tmp_intel += mod;				return;
	case SPELL_AURA:			ch->tmp_aur += mod;				return;
	case SPELL_WILL:			ch->tmp_wil += mod;				return;
	case SPELL_CONSTITUTION:	ch->tmp_con += mod;				return;
	case SPELL_AGILITY:			ch->tmp_agi += mod;				return;
	default:														break;
	}
	*/
		//TO DO - CRAFT_FIRST is 8000, CRAFT_LAST is 18000
		//so there is some overlap in values
		//consider using APPLY_<SKILL> values instead 
	if ((loc >= 10001) && (loc <= (skill_data_map.size() + 10000)))
	{
		if (add)
			ch->skill_map[lookup_skill_name(loc - 10000)] += mod;
		else
		{
			ch->skill_map[lookup_skill_name(loc - 10000)] -= mod;
		}
		return;
	}

	else
		switch (loc)
	{

		case APPLY_NONE:
		case APPLY_CASH:
		case APPLY_SAVING_BREATH:
		case APPLY_SAVING_SPELL:
		case APPLY_AC:
			break;

		case APPLY_STR:
			ch->tmp_str += mod;
			break;
		case APPLY_DEX:
			ch->tmp_dex += mod;
			break;
		case APPLY_INT:
			ch->tmp_intel += mod;
			break;
		case APPLY_AUR:
			ch->tmp_aur += mod; //change is made to tmp_aur (shown in score but not charstat)
								 //ch->aur += mod; //to show it in charstat
			if (!IS_NPC (ch))
			{
				ch->assign_hit_points();
			}
			else
			{
				ch->max_hit += mod * 6;
			}
			
			break;
		case APPLY_WIL:
			ch->tmp_wil += mod;
			ch->max_move = calc_lookup (ch, REG_MISC, MISC_MAX_MOVE);
			break;
		case APPLY_CON:
			ch->tmp_con += mod;
			if (!IS_NPC (ch))
			{
				ch->assign_hit_points();
			}
			else
			{
				ch->max_hit += mod * 6;
				if (ch->hit > ch->max_hit)
					ch->hit = ch->max_hit;
			}
			ch->max_move = calc_lookup (ch, REG_MISC, MISC_MAX_MOVE);
			break;
		case APPLY_AGI:
			ch->tmp_agi += mod;
			break;
		case APPLY_AGE:
			ch->time_str.birth += mod;
			break;
		case APPLY_HIT:
			ch->max_hit += mod;
			break;
		case APPLY_MOVE:
			ch->max_move += mod;
			break;
		case APPLY_DAMROLL:
			if (IS_NPC (ch))
				ch->mob->damroll += mod;
			break;
		default:
			break;

	}				/* switch */
}


AFFECTED_TYPE *
get_obj_affect_location (OBJ_DATA * obj, int location)
{
	AFFECTED_TYPE *af;

	if (!obj->xaffected)
		return NULL;

	for (af = obj->xaffected; af; af = af->next)
		if (af->a.spell.location == location)
			return af;

	return NULL;
}

void
remove_obj_affect_location (OBJ_DATA * obj, int location)
{
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *taf;

	if (!obj->xaffected)
		return;

	if (obj->xaffected->a.spell.location == location)
	{
		af = obj->xaffected;
		obj->xaffected = obj->xaffected->next;
		free_mem (af);
		return;
	}

	for (af = obj->xaffected; af->next; af = af->next)
		if (af->next->a.spell.location == location)
		{
			taf = af->next;
			af->next = taf->next;
			free_mem (taf);
			return;
		}
}

void
affect_to_obj (OBJ_DATA * obj, AFFECTED_TYPE * af)
{
	AFFECTED_TYPE *taf;

	if (!obj->xaffected)
	{
		obj->xaffected = af;
		return;
	}

	for (taf = obj->xaffected; taf->next; taf = taf->next)
		;

	taf->next = af;
}

void
remove_obj_affect (OBJ_DATA * obj, int type)
{
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *free_af;

	if (!obj->xaffected)
		return;

	if (obj->xaffected->type == type)
	{
		af = obj->xaffected;
		obj->xaffected = af->next;
		free_mem (af);
		return;
	}

	for (af = obj->xaffected; af->next; af = af->next)
	{
		if (af->next->type == type)
		{
			free_af = af->next;
			af->next = free_af->next;
			free_mem (free_af);
			return;
		}
	}
}

AFFECTED_TYPE *
get_obj_affect (OBJ_DATA * obj, int type)
{
	AFFECTED_TYPE *af;

	if (!obj->xaffected)
		return NULL;

	for (af = obj->xaffected; af; af = af->next)
		if (af->type == type)
			return af;

	return NULL;
}

AFFECTED_TYPE *
get_affect (const CHAR_DATA * ch, int affect_type)
{
	AFFECTED_TYPE *af;

	for (af = ch->hour_affects; af; af = af->next)
	{
		if (af && af->type)
		{
		if (af->type != affect_type)
			continue;
		else 
			return af;
		}
	}
	return NULL;
}


/* Insert an affect_type in a char_data structure
Automatically sets apropriate bits and apply's.
*/

void
affect_to_char (CHAR_DATA * ch, AFFECTED_TYPE * af)
{
	/* avoid hiding more than once */
	if ((af->type == MAGIC_HIDDEN) && get_affect(ch,MAGIC_HIDDEN))
		return;

	af->next = ch->hour_affects;
	ch->hour_affects = af;

	
	affect_modify (ch, af->type, af->a.spell.location, af->a.spell.modifier,
		af->a.spell.bitvector, true, af->a.spell.sn);
}

void
remove_affect_type (CHAR_DATA * ch, int type)
{
	AFFECTED_TYPE *af;

	af = get_affect (ch, type);
	
	if (af)
		affect_remove (ch, af);
}

/* Remove an affected_type structure from a char (called when duration
reaches zero).  Pointer *af must never be NIL!  Frees mem and calls
affect_location_apply.
*/

void
affect_remove (CHAR_DATA * ch, AFFECTED_TYPE * af)
{
	AFFECTED_TYPE *taf;
	
	
	affect_modify (ch, af->type, af->a.spell.location, af->a.spell.modifier,
					   af->a.spell.bitvector, false, af->a.spell.sn);
	
	/* remove structure *af from linked list */
	if (ch->hour_affects == af)
		ch->hour_affects = af->next;
	
	else
	{
		for (taf = ch->hour_affects; taf && taf->next != af; taf = taf->next)
			;
		
		if (!taf)
		{
			return;
		}
		
		taf->next = af->next;
	}
	
	if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST)
		free_mem (af->a.craft);
	
	free_mem (af);
	
}




	//puts obj in the hands of character 
void
obj_to_char (OBJ_DATA * obj, CHAR_DATA * ch)	/* STACKing */
{
	int dir = 0;
	OBJ_DATA *tobj;
	bool stacked_obj = false;

	// Make sure we have a count of at least 1
	obj->count = (obj->count > 0) ? obj->count : 1;

	// reset any value override
	obj->obj_flags.set_cost = 0;

	/* Do object stacking */

	if (IS_SET (obj->obj_flags.extra_flags, ITEM_STACK) &&
		!(obj->obj_flags.type_flag == ITEM_FOOD &&
		obj->o.food.food_value != obj->o.food.bites))
	{

		if (ch->right_hand 
			&& ch->right_hand->nVirtual == obj->nVirtual
			&& ch->right_hand->morphTime == obj->morphTime)
		{
			tobj = ch->right_hand;
			obj->count += tobj->count;
			extract_obj (tobj);
			stacked_obj = true;
			ch->right_hand = obj;
			obj->carried_by = ch;
			obj->location = -1;
			obj->in_room = NOWHERE;
			obj->in_obj = NULL;
			return;
		}

		else if (ch->left_hand 
				 && ch->left_hand->nVirtual == obj->nVirtual
				 && ch->left_hand->morphTime == obj->morphTime)
		{
			tobj = ch->left_hand;
			obj->count += tobj->count;
			extract_obj (tobj);
			stacked_obj = true;
			ch->left_hand = obj;
			obj->carried_by = ch;
			obj->location = -1;
			obj->in_room = NOWHERE;
			obj->in_obj = NULL;
			return;
		}
	}

	if (((ch->right_hand && ch->left_hand) && !stacked_obj)
		|| get_equip (ch, WEAR_BOTH))
	{
		ch->send_to_char("Since your hands are full, you set the object on the ground.\n");
		obj_to_room (obj, ch->in_room);
		obj->in_obj = NULL;
		return;
	}

	if (!ch->right_hand)
		ch->right_hand = obj;
	else
		ch->left_hand = obj;

	if (obj->obj_flags.type_flag == ITEM_KEY && IS_NPC (ch))
	{
		for (dir = 0; dir <= LAST_DIR; dir++)
		{
			if (is_exit(ch, dir)
				&& IS_SET (is_exit(ch, dir)->port_flags, EX_LOCKED)
				&& has_key (ch, NULL, is_exit(ch, dir)->key) && !ch->desc)
			{
				do_pmote (ch, "keeps a watchful eye on the entryways here.", 0);
				break;
			}
		}
	}

	obj->carried_by = ch;

	obj->next_content = NULL;

	obj->location = -1;

	obj->in_room = NOWHERE;

	obj->in_obj = NULL;

	room_light (ch->room);
}


void
obj_from_char (OBJ_DATA ** obj, int count)	/* STACKing */
{
	int contents = 0, volume = 0;
	int nMorphTime = 0;
	CHAR_DATA *ch;

	ch = (*obj)->carried_by;

	if (ch == NULL)
	{
		return;
	}

	/**
	 capacity - od.value[0]
	 volume - od.value[1]
	 contents/liquid - od.value[2]
	 **/
	if ((*obj)->obj_flags.type_flag == ITEM_DRINKCON 
		|| (*obj)->obj_flags.type_flag == ITEM_DRYCON)
	{
		contents = (*obj)->o.od.value[2];
		volume = (*obj)->o.od.value[1];
	}

	/* Take a partial number of objs? */

	if (count != 0 && count < (*obj)->count)
	{

		(*obj)->count -= count;

		nMorphTime = (*obj)->morphTime;

		*obj = load_object ((*obj)->nVirtual);
		(*obj)->count = count;
		(*obj)->morphTime = nMorphTime;
		(*obj)->carried_by = NULL;


		if ((*obj)->obj_flags.type_flag == ITEM_DRINKCON 
			|| (*obj)->obj_flags.type_flag == ITEM_DRYCON)
		{
			(*obj)->o.od.value[2] = contents;
			(*obj)->o.od.value[1] = volume;
		}

		return;
	}

	/* Remove object from inventory */

	if (ch->right_hand == *obj)
		ch->right_hand = NULL;
	else if (ch->left_hand == *obj)
		ch->left_hand = NULL;

	(*obj)->carried_by = NULL;
	(*obj)->next_content = NULL;
	(*obj)->equiped_by = NULL;
	(*obj)->in_room = NOWHERE;
	(*obj)->in_obj = NULL;

	room_light (ch->room);
}

void
equip_char (CHAR_DATA * ch, OBJ_DATA * obj, int pos)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (ch == 0)
	{
		sprintf (buf,
			"#1OBJECT MORPHING BUG! NULL ch pointer. Crash averted. Object vnum %d in room %d.#0\n",
			obj->nVirtual,
			obj->in_room);
		send_to_gods (buf);
		system_log (buf, true);
		return;
	}

	if (pos < 0)
	{
		sprintf (buf,
			"#1OBJECT MORPHING BUG! Crash averted. Position %d, vnum %d, character %s in room %d.#0\n",
			pos, obj->nVirtual, ch->name, obj->in_room);
		send_to_gods (buf);
		system_log (buf, true);
		return;
	}

	if (get_equip (ch, pos))
		return;


	if (obj->in_room != NOWHERE)
	{
		system_log ("EQUIP: Obj is in_room when equip.", true);
		return;
	}

	if (obj->item_wear < -10)
		return;
		
	obj->location = pos;

	if (pos != WEAR_PRIM && pos != WEAR_SEC && pos != WEAR_BOTH)
	{
		obj->next_content = ch->equip;
		ch->equip = obj;
	}

	if (pos == WEAR_PRIM || pos == WEAR_SEC || pos == WEAR_BOTH)
		obj->carried_by = ch;

	obj->equiped_by = ch;
}

OBJ_DATA *
unequip_char (CHAR_DATA * ch, int pos)
{
	OBJ_DATA *obj;
	OBJ_DATA *tobj;

	obj = get_equip (ch, pos);

	if (!obj)
		return NULL;

	if (IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) && ch->is_hooded())
		ch->affected_by &= ~AFF_HOODED;

	if (ch->equip == obj)
		ch->equip = ch->equip->next_content;
	else
	{
		for (tobj = ch->equip; tobj; tobj = tobj->next_content)
			if (tobj->next_content && obj == tobj->next_content)
			{
				tobj->next_content = obj->next_content;
				break;
			}
	}

	obj->location = -1;
	obj->equiped_by = NULL;
	obj->next_content = NULL;


	return (obj);
}


int
get_number (char **name)
{

	int i;
	char *ppos = '\0';
	char number[MAX_INPUT_LENGTH] = { '\0' };

	if ((ppos = (char *) strchr (*name, '.')))
	{
		*ppos++ = '\0';
		strcpy (number, *name);
		strcpy (*name, ppos);

		for (i = 0; *(number + i); i++)
			if (!isdigit (*(number + i)))
				return (0);

		return (atoi (number));
	}

	return (1);
}

	//are they carrying the item type in either hand?
OBJ_DATA *
get_carried_item (CHAR_DATA * ch, int item_type)
{
	if (!ch)
		return NULL;

	if (ch->right_hand && ch->right_hand->obj_flags.type_flag == item_type)
		return ch->right_hand;

	if (ch->left_hand && ch->left_hand->obj_flags.type_flag == item_type)
		return ch->left_hand;

	return NULL;
}

	// Search a given list for an object, and return a pointer to that object 
OBJ_DATA *
get_obj_in_list (char *name, OBJ_DATA * list)
{
	OBJ_DATA *i, *contents;
	int j, number;
	char tmpname[MAX_INPUT_LENGTH];
	char *tmp;

	strcpy (tmpname, name);
	tmp = tmpname;
	if (!(number = get_number (&tmp)))
		return (0);

	for (i = list, j = 1; i && (j <= number); i = i->next_content)
	{
		if (isname (tmp, i->name)
			|| (i->obj_flags.type_flag == ITEM_BOOK && i->book_title
				&& isname (tmp, i->book_title))
			|| (i->obj_flags.type_flag == ITEM_DRINKCON
				&& i->o.drinkcon.volume
				&& (contents = vtoo (i->o.drinkcon.liquid))
				&& isname (tmp, contents->name))
			|| (i->obj_flags.type_flag == ITEM_DRYCON
				&& i->o.drycon.volume
				&& (contents = vtoo (i->o.drycon.contents))
				&& isname (tmp, contents->name)))
		{
			if (j == number)
				return (i);
			j++;
		}
	}

	return (0);
}

	// Search a given list for an matching vnum and return a pointer to that object 
OBJ_DATA *
get_obj_in_list_num (int num, OBJ_DATA * list)
{
	OBJ_DATA *obj;

	for (obj = list; obj; obj = obj->next_content)
		if (obj->nVirtual == num)
			return obj;

	return NULL;
}

	// search the entire world for an object, and return a pointer
OBJ_DATA *
get_obj (char *name)
{
	OBJ_DATA *tobj,*contents;
	int j, number;
	char tmpname[MAX_INPUT_LENGTH];
	char *tmp;
	std::list<OBJ_DATA*>::iterator tobj_iterator;

	strcpy (tmpname, name);
	tmp = tmpname;
	if (!(number = get_number (&tmp)))
		return (0);

	for (tobj_iterator = object_list.begin(), j = 1; tobj_iterator != object_list.end() && (j <= number); tobj_iterator++)
	{

		tobj = *tobj_iterator;
		
		if (tobj->deleted)
			continue;

		if (isname (tmp, tobj->name)
			|| (tobj->obj_flags.type_flag == ITEM_BOOK && tobj->book_title
				&& isname (tmp, tobj->book_title))
			|| (tobj->obj_flags.type_flag == ITEM_DRINKCON
				&& tobj->o.drinkcon.volume
				&& (contents = vtoo (tobj->o.drinkcon.liquid))
				&& isname (tmp, contents->name))
			|| (tobj->obj_flags.type_flag == ITEM_DRYCON
				&& tobj->o.drycon.volume
				&& (contents = vtoo (tobj->o.drycon.contents))
				&& isname (tmp, contents->name)))
		{
			if (j == number)
				return (tobj);
			j++;
		}
	}

	return (0);
}

CHAR_DATA *
get_char_id (int coldload_id)
{
	CHAR_DATA *ch;
	std::list<char_data*>::iterator tch_iterator;
	
	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		ch = *tch_iterator;
		if (ch->deleted)
			continue;
		if (ch->coldload_id == coldload_id)
			return ch;
	}

	return NULL;
}

OBJ_DATA *
get_obj_in_list_id (int coldload_id, OBJ_DATA * list)
{
	OBJ_DATA *obj;

	for (obj = list; obj; obj = obj->next_content)
	{
		if (obj->deleted)
			continue;
		if (obj->coldload_id == coldload_id)
			return obj;
	}

	return NULL;
}

OBJ_DATA *
get_obj_id (int coldload_id)
{
	OBJ_DATA *obj;
	std::list<OBJ_DATA*>::iterator tobj_iterator;
	
	for (tobj_iterator = object_list.begin(); tobj_iterator != object_list.end(); tobj_iterator++)
	{
		obj = *tobj_iterator;
		
		if (obj->deleted)
			continue;
		if (obj->coldload_id == coldload_id)
			return obj;
	}

	return NULL;
}

/* search a room for a char, and return a pointer if found..  */
CHAR_DATA *
get_char_room (char *name, int room)
{
	CHAR_DATA *targ;
	int j, number;
	char tmpname[MAX_INPUT_LENGTH];
	char *tmp;

	strcpy (tmpname, name);
	tmp = tmpname;
	if (!(number = get_number (&tmp)))
		return (0);

	for (targ = vtor (room)->people, j = 1; targ && (j <= number);
		targ = targ->next_in_room)
		if (isname (tmp, targ->char_names()))
		{
			if (j == number)
				return (targ);
			j++;
		}

		return (0);
}

/* search all over the world for a char, and return a pointer if found */
CHAR_DATA *
get_char (char *name)
{
	CHAR_DATA *i;
	int j;
	int number;
	char tmpname[MAX_INPUT_LENGTH];
	char *tmp;
	std::list<char_data*>::iterator tch_iterator;
	
	strcpy (tmpname, name);
	tmp = tmpname;

	if (!(number = get_number (&tmp)))
		return 0;

	j = 1;
	for (tch_iterator = character_list.begin(); (tch_iterator != character_list.end()) && (j <= number); tch_iterator++)
	{
		i = *tch_iterator;

		if (i->deleted)
			continue;

		if (isname (tmp, i->char_names()))
		{
			if (j == number)
				return (i);
			j++;
		}
	}

	return 0;
}

/* search all over the world for a char, and return a pointer if found */
CHAR_DATA *
get_mob_vnum (int nVirtual)
{
	CHAR_DATA *i;
	std::list<char_data*>::iterator tch_iterator;
	
	//for (i = character_list; i; i = i->next)
	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		i = *tch_iterator;

		if (i->deleted)
			continue;

		if (!IS_NPC (i))
			continue;

		if (i->mob->nVirtual == nVirtual)
			return i;
	}

	return 0;
}

/* search all over the world for a char, and return a pointer if found */
CHAR_DATA *
get_char_nomask (char *name)
{
	CHAR_DATA *i;
	int j;
	int number;
	char tmpname[MAX_INPUT_LENGTH];
	char *tmp;
	std::list<char_data*>::iterator tch_iterator;
	strcpy (tmpname, name);
	tmp = tmpname;

	if (!(number = get_number (&tmp)))
		return 0;

	//for (i = character_list, j = 1; i && (j <= number); i = i->next)
	j = 1;
	for (tch_iterator = character_list.begin(); (tch_iterator != character_list.end()) && (j <= number); tch_iterator++)
	{
		i = *tch_iterator;

		if (i->deleted)
			continue;

		if (isname (tmp, i->keywords))
		{
			if (j == number)
				return (i);
			j++;
		}
	}

	return 0;
}

/* search all over the world for a char, and return a pointer if found */
/* no NPCs here !*/
CHAR_DATA *
get_char_nomask_nonpc (char *name)
{
	CHAR_DATA *i;
	int j;
	int number;
	char tmpname[MAX_INPUT_LENGTH];
	char *tmp;
	std::list<char_data*>::iterator tch_iterator;
	strcpy (tmpname, name);
	tmp = tmpname;

	if (!(number = get_number (&tmp)))
		return 0;

	//for (i = character_list, j = 1; i && (j <= number); i = i->next)
	j = 1;
	for (tch_iterator = character_list.begin(); (tch_iterator != character_list.end()) && (j <= number); tch_iterator++)
	{
		i = *tch_iterator;

		if (i->deleted)
			continue;

		/* if this is switched into by an admin, run the check on the original char */
		if (IS_NPC(i) && i->desc && i->desc->original)
		{
			if (isname (tmp, (i->desc->original)->keywords))
			{
				/* return the desc on the NPC as that is where data would go to */
				if (j == number)
					return (i);
				j++;
			}
		}

		if (IS_NPC(i))
			continue;

		if (isname (tmp, i->keywords))
		{
			if (j == number)
				return (i);
			j++;
		}
	}

	return 0;
}


void
obj_to_room (OBJ_DATA * object, int roomnum)	/* STACKing */
{
	int add_at_top = 0;
	ROOM_EXIT_DATA *exit;
	ROOM_DATA *r, *troom = NULL;
	OBJ_DATA *tobj;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (!object)
		return;

	if (roomnum == NOWHERE)
		return;
	
	r = vtor (roomnum);

	if (!r)
		return;

	if (r->contents
		&& IS_SET (object->obj_flags.extra_flags, ITEM_STACK)
		&& !get_obj_affect (object, MAGIC_HIDDEN))
	{

		for (tobj = r->contents; tobj; tobj = tobj->next_content)
		{

			if (tobj->nVirtual != object->nVirtual
				|| get_obj_affect (tobj, MAGIC_HIDDEN)
				|| tobj->morphTime != object->morphTime)
				continue;

			tobj->count += object->count;
			extract_obj (object);
			room_light (r);
			return;
		}
	}

	if (add_at_top)
	{
		object->next_content = r->contents;
		r->contents = object;
	}
	else
	{
		if (!r->contents)
		{
			r->contents = object;
			object->next_content = NULL;
		}
		else
			for (tobj = r->contents; tobj; tobj = tobj->next_content)
			{
				if (!tobj->next_content)
				{
					tobj->next_content = object;
					object->next_content = NULL;
					break;
				}
			}
	}

	object->in_room = roomnum;
	object->carried_by = 0;
	object->equiped_by = NULL;
	object->in_obj = NULL;
	object->location = -1;

	room_light (r);

	exit = r->dir_option[DOWN];

	if (exit
		&& !IS_SET (exit->port_flags, EX_ISDOOR)
		&& !IS_SET (exit->port_flags, EX_ISGATE))
	{
		troom = vtor (exit->to_room);

		if (troom && IS_SET (r->room_flags, FALL))
		{
			sprintf (buf, "#2%s#0 plummets down!", obj_short_desc (object));
			buf[2] = toupper (buf[2]);
			send_to_room (buf, r->nVirtual);
			obj_from_room (&object, 0);
			obj_to_room (object, troom->nVirtual);
			sprintf (buf, "#2%s#0 falls from above.", obj_short_desc (object));
			buf[2] = toupper (buf[2]);
			send_to_room (buf, object->in_room);
		}
	}

}

/* Take an object from a room */
void
obj_from_room (OBJ_DATA ** obj, int count)	/* STACKing */
{
	int contents = 0, volume = 0;
	int nMorphTime = 0;
	OBJ_DATA *tobj;
	ROOM_DATA *room;

	room = vtor ((*obj)->in_room);

	if (!room)
		return;

	/**
	 capacity - od.value[0]
	 volume - od.value[1]
	 contents/liquid - od.value[2]
	 **/
	if ((*obj)->obj_flags.type_flag == ITEM_DRINKCON || (*obj)->obj_flags.type_flag == ITEM_DRYCON)
	{
		contents = (*obj)->o.od.value[2];
		volume = (*obj)->o.od.value[1];
	}

	

	/* Take a partial number of objs? */

	if (count != 0 && count < (*obj)->count)
	{

		(*obj)->count -= count;
		nMorphTime = (*obj)->morphTime;

		*obj = load_object ((*obj)->nVirtual);
		(*obj)->count = count;
		(*obj)->morphTime = nMorphTime;

		if ((*obj)->obj_flags.type_flag == ITEM_DRINKCON || (*obj)->obj_flags.type_flag == ITEM_DRYCON)
		{
			(*obj)->o.od.value[2] = contents;
			(*obj)->o.od.value[1] = volume;
		}

		room_light (room);

		(*obj)->in_room = NOWHERE;
		(*obj)->next_content = NULL;

		return;
	}

	/* Remove object from the room */

	if (room->contents == *obj)
		room->contents = (*obj)->next_content;
	else if (room->contents)
	{
		for (tobj = room->contents;
			tobj->next_content; tobj = tobj->next_content)
			if (tobj->next_content == *obj)
			{
				tobj->next_content = (*obj)->next_content;
				break;
			}
	}

	(*obj)->in_room = NOWHERE;
	(*obj)->next_content = NULL;

	room_light (room);

	remove_obj_affect (*obj, MAGIC_HIDDEN);
}

void
obj_to_obj (OBJ_DATA * obj, OBJ_DATA * container)
{
	OBJ_DATA *tobj;
	int room_remain;
	
	if (!obj || !container)
		return;

		//put out lights before putting them in containers other than tables
	if ((obj->obj_flags.type_flag == ITEM_LIGHT) 
		&& (!IS_SET (container->obj_flags.extra_flags, ITEM_TABLE)))
	{
		if (obj->o.light.on)
			obj->o.light.on = false;
	}
	
	if (container->obj_flags.type_flag != ITEM_DRYCON)
	{
		if (container->contains && IS_SET (obj->obj_flags.extra_flags, ITEM_STACK))
		{
			
			for (tobj = container->contains; tobj; tobj = tobj->next_content)
			{
				
				if (tobj->nVirtual != obj->nVirtual
					|| tobj->morphTime != obj->morphTime)
					continue;
				
				tobj->count += obj->count;
				tobj->in_obj->contained_wt += obj_mass(obj);
				extract_obj (obj);
				return;
			}
		}
		
		obj->in_obj = container;
		obj->equiped_by = NULL;
		obj->carried_by = NULL;
		if (container->contains && !is_obj_in_list (obj, container->contains))
			obj->next_content = container->contains;
		else if (!container->contains)
			container->contains = obj;
		else
		{
			extract_obj (obj);
			return;
		}
		
		container->contains = obj;
		obj->location = -1;
		
		for (tobj = container; tobj; tobj = tobj->in_obj)
			tobj->contained_wt += obj_mass (obj);
	}
	else
	{
		if (IS_SET (obj->obj_flags.extra_flags, ITEM_STACK))
		{
						
			if (container->o.drycon.volume == 0 || container->o.drycon.contents == 0)
			{
				container->o.drycon.capacity = (int)(container->o.drycon.max_weight / obj->obj_flags.weight);
				container->o.drycon.contents = obj->nVirtual;
			}
			
			room_remain = container->o.drycon.capacity - container->o.drycon.volume;
			
			if (obj->count > room_remain)
			{
				obj->count -= room_remain;
				container->o.drycon.volume = container->o.drycon.capacity;
				container->contained_wt += obj->count * obj->obj_flags.weight;
				obj->obj_flags.weight = obj_mass(obj);
			}
			else
			{
				container->o.drycon.volume += obj->count;
				container->contained_wt += obj->count * obj->obj_flags.weight;
				extract_obj(obj);
			}
		}
	}
}


void
obj_from_obj (OBJ_DATA ** obj, int count, OBJ_DATA *container)
{
	int adjust_wt;
	int volume = 0;
	OBJ_DATA *tobj;

	if (!(*obj))
		return;

	if (count < 0)
		count = 0;

	if (!count)
		count = (*obj)->count;
	
	if (!container)
		container = (*obj)->in_obj;

	/**
	 capacity - od.value[0]
	 volume - od.value[1]
	 contents/liquid - od.value[2]
	 **/
	if (container->obj_flags.type_flag == ITEM_DRINKCON || container->obj_flags.type_flag == ITEM_DRYCON)
	{
			//for drink or dry container with normal volumes
		if (container->o.od.value[0] >= 0)
		{
			volume = container->o.od.value[1];
			
			if (count >= volume)
			{
				if (container->obj_flags.type_flag == ITEM_DRYCON)
					container->o.od.value[0] = 1;
				
				container->o.od.value[1] = 0;
				container->o.od.value[2] = 0;
			}
			else 
			{
				container->o.od.value[1] = volume - count; 
			}
			adjust_wt = count * (*obj)->obj_flags.weight;
			container->contained_wt -= adjust_wt;
			
			return;
		}
		//capacity is negative to indicate infinite supply, so no adjustments to container
		//we do need to create and load the contents
		else 
		{
			tobj = *obj;
			
			*obj = load_object (tobj->nVirtual);
			(*obj)->count = count;
			(*obj)->morphTime = tobj->morphTime;
			(*obj)->next_content = NULL;
			(*obj)->in_obj = NULL;
			return;
		}

		
	}

	/* Removing only part of obj from container */

	if (count < (*obj)->count)
	{

		tobj = *obj;

		*obj = load_object (tobj->nVirtual);

		tobj->count -= count;

		(*obj)->count = count;
		(*obj)->morphTime = tobj->morphTime;
		(*obj)->in_obj = tobj->in_obj;

		adjust_wt = (*obj)->count * (*obj)->obj_flags.weight;
	}

		
		// Removing obj completely from  other containers 
	else
	{

		adjust_wt = obj_mass (*obj);

		if ((*obj)->in_obj->contains == *obj)
			(*obj)->in_obj->contains = (*obj)->next_content;
		else
		{
			for (tobj = (*obj)->in_obj->contains;
				tobj->next_content; tobj = tobj->next_content)
				if (tobj->next_content == *obj)
				{
					tobj->next_content = (*obj)->next_content;
					break;
				}
		}
	}

	for (tobj = (*obj)->in_obj; tobj; tobj = tobj->in_obj)
		tobj->contained_wt -= adjust_wt;

	(*obj)->next_content = NULL;
	(*obj)->in_obj = NULL;
}



void
remove_object_affect (OBJ_DATA * obj, AFFECTED_TYPE * af)
{
	AFFECTED_TYPE *taf;

	if (af == obj->xaffected)
	{
		obj->xaffected = af->next;
		free_mem (af);
		return;
	}

	for (taf = obj->xaffected; taf->next; taf = taf->next)
	{
		if (af == taf->next)
		{
			taf->next = af->next;
			free_mem (af);
			return;
		}
	}
}


void
extract_obj (OBJ_DATA * obj)
{
	OBJ_DATA *tobj;

	while (obj->contains)
		extract_obj (obj->contains);

	if (obj->equiped_by != NULL)
	{
		tobj = unequip_char (obj->equiped_by, obj->location);
		extract_obj (tobj);
	}

	if (obj->carried_by != NULL)
		obj_from_char (&obj, 0);
	else if (obj->in_obj != NULL)
		obj_from_obj (&obj, 0, 0);
	else if (obj->in_room != NOWHERE)
		obj_from_room (&obj, 0);

	while (obj->xaffected)
		remove_object_affect (obj, obj->xaffected);

	vtoo (obj->nVirtual)->instances--;
	
	obj->carried_by = NULL;
	obj->equiped_by = NULL;
	obj->in_obj = NULL;

	obj->deleted = 1;

	object_list.remove(obj);
	knockout = 1;			/* Get obj out of object_list ASAP */
}

void
morph_obj (OBJ_DATA * obj)
{

	char buf[MAX_STRING_LENGTH]= { '\0' };
	OBJ_DATA *newObj;

	if (obj->deleted)
		return;

	if (obj->in_room && vtor (obj->in_room)
		&& IS_SET (vtor (obj->in_room)->room_flags, STORAGE))
		return;

	if (obj->morphto)
	{
		if (obj->morphto == 86)
		{
				//if (obj->equiped_by)
				//unequip_char (obj->equiped_by, obj->location);
			
			extract_obj (obj);
			
			return;
		}
		else
		{
			newObj = load_object (obj->morphto);
				//	if (obj->equiped_by)
				//{
				//tobj = unequip_char (obj->equiped_by, obj->location);
				//equip_char(tobj->equiped_by, newObj, tobj->location);
				//}
		}
	}
	else
	{
		sprintf (buf, "Object %d has a morph clock, but no morph Objnum\n",
			obj->nVirtual);
		system_log (buf, true);
		return;
	}

	if (!newObj)
	{
		sprintf (buf, "Attempt to load morph obj %d for object %d failed\n",
			obj->morphto, obj->nVirtual);
		system_log (buf, true);
		return;
	}

	newObj->count = obj->count;
	int location = obj->location;
	CHAR_DATA* equipped_by = obj->equiped_by;

	/* if the object is equipped, unequip it. This takes it into nowhere */
	if (obj->equiped_by)
		unequip_char (obj->equiped_by, obj->location);

	/* if the object is carried, remove it from the user */
	/* this will clear it out of any hand */
	/* then, load it into a hand again -- this may move something from left to right etc */
	if (obj->carried_by)
	{
		CHAR_DATA *ch;
		ch = obj->carried_by;
		obj_from_char (&obj, 0);
		obj_to_char(newObj, ch);
	}

	/* if the item had been equipped, move it to the proper location
	/* for items that were not in hands, they just magically reappear in the same spot
	/* without use of hands */
	if (equipped_by)
	{
		equip_char(equipped_by,newObj,location);
	}


	if (obj->in_obj)
	{
		OBJ_DATA *container;

		container = obj->in_obj;
		obj_from_obj (&obj, 0, 0);

		obj_to_obj (newObj, container);
	}

	if (obj->in_room != NOWHERE)
	{
		int room;

		room = obj->in_room;
		obj_from_room (&obj, 0);

		obj_to_room (newObj, room);
	}

	while (obj->xaffected)
		remove_object_affect (obj, obj->xaffected);

	obj->deleted = 1;
}

void
remove_sighting (CHAR_DATA * ch, CHAR_DATA * target)
{
	SIGHTED_DATA *sighted;

	if (!ch || !target)
		return;

	if (ch->sighted && ch->sighted->target == target)
		ch->sighted = ch->sighted->next;

	for (sighted = ch->sighted; sighted; sighted = sighted->next)
	{
		if (sighted->next && sighted->next->target == target)
			sighted->next = sighted->next->next;
	}
}




	//Returns a pointer to the character in the same room we are in and he is visible
CHAR_DATA *
get_char_room_vis (CHAR_DATA * ch, const char *name)
{
	CHAR_DATA *tch;
	int j = 1;
	int number;
	char tmpname[MAX_STRING_LENGTH]= { '\0' };
	char immkeys[AVG_STRING_LENGTH];
	char *tmp;
	char *tchtemp;

	
	if (!strcmp (name, "self") || !strcmp (name, "me"))
		return ch;

	strcpy (tmpname, name);

	tmp = tmpname;

	if (!(number = get_number (&tmp)))
		return NULL;

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{

		sprintf (immkeys, "%s %s person", tch->char_names(), tch->keywords);

		if (are_grouped (ch, tch) && (!ch->get_trust()))
			tchtemp = tch->keywords;
		else if ((!ch->get_trust()))
			tchtemp = duplicateString(tch->char_names());
		
		else
			tchtemp = tch->keywords;


		// if (isname (tmp, (!ch->get_trust()) ? tch->char_names() : immkeys))
		if (isname (tmp, tchtemp) || isname (tmp, tch->char_names()))
		{

			if (can_see_mob(ch, tch) || ch == tch)
			{
				if (j == number)
					return tch;
				
				j++;
			}
		}
	}

	return NULL;
}

//Returns a pointer to the character in designated room and he is visible
CHAR_DATA *
get_char_room_vis2 (CHAR_DATA * ch, int vnum, char *name)
{
	ROOM_DATA *room;
	CHAR_DATA *tch;
	int j = 1;
	int number;
	char tmpname[MAX_STRING_LENGTH]= { '\0' };
	char immkeys[AVG_STRING_LENGTH];
	char *tmp;

	if (!(room = vtor (vnum)))
		return NULL;

	if (!strcmp (name, "self") || !strcmp (name, "me"))
		return ch;

	strcpy (tmpname, name);

	tmp = tmpname;

	if (!(number = get_number (&tmp)))
		return NULL;

	for (tch = room->people; tch; tch = tch->next_in_room)
	{

		sprintf (immkeys, "%s %s person", tch->char_names(), tch->keywords);

		if (isname (tmp, (!ch->get_trust()) ? tch->char_names() : immkeys))
		{
			if (can_see_mob(ch, tch) || ch == tch)
			{

				if (j == number)
					return tch;
				
				j++;
			}
		}
	}

	return NULL;
}
//Returns a pointer to the character in any room and he is visible
CHAR_DATA *
get_char_vis (CHAR_DATA * ch, const char *name)
{
	CHAR_DATA *i;
	int j, number;
	char tmpname[MAX_INPUT_LENGTH];
	char *tmp;
	std::list<char_data*>::iterator tch_iterator;
	
	if (!strcmp (name, "self") || !strcmp (name, "me"))
		return ch;

	/* check location */
	if ((i = get_char_room_vis (ch, name)))
		return (i);

	strcpy (tmpname, name);
	tmp = tmpname;
	if (!(number = get_number (&tmp)))
		return (0);

	
	j = 1;
	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end() && (j <= number); tch_iterator++)
	{
		i = *tch_iterator;

		if (i->deleted)
			continue;

		if (isname (tmp, i->keywords))
			if (can_see_mob(ch, i))
			{
				if (j == number)
					return i;
				j++;
			}
	}

	return 0;
}

OBJ_DATA *
get_obj_in_list_vis_not_money (CHAR_DATA * ch, char *name, OBJ_DATA * list)
{
	OBJ_DATA *obj,*contents;
	int j = 1;
	int number;
	char tmpname[MAX_INPUT_LENGTH];
	char *tmp;

	strcpy (tmpname, name);
	tmp = tmpname;

	if (!(number = get_number (&tmp)))
		return NULL;

	if (isdigit (*name))
	{

		if (!(number = atoi (name)))
			return NULL;

		for (obj = list; number && obj; obj = obj->next_content)
		{

			if (obj->obj_flags.type_flag == ITEM_MONEY)
				continue;

			if (can_see_obj(ch, obj) && !(--number))
				return obj;
		}

		return NULL;
	}

	for (obj = list; obj && (j <= number); obj = obj->next_content)
	{
		if (obj->obj_flags.type_flag == ITEM_MONEY)
			continue;

		if (isname (tmp, obj->name)
			|| (obj->obj_flags.type_flag == ITEM_BOOK && obj->book_title
				&& isname (tmp, obj->book_title))
			|| (obj->obj_flags.type_flag == ITEM_DRINKCON
				&& obj->o.drinkcon.volume
				&& (contents = vtoo (obj->o.drinkcon.liquid))
				&& isname (tmp, contents->name))
			|| (obj->obj_flags.type_flag == ITEM_DRYCON
				&& obj->o.drycon.volume
				&& (contents = vtoo (obj->o.drycon.contents))
				&& isname (tmp, contents->name)))
			if (can_see_obj(ch, obj))
			{
				if (j == number)
					return (obj);
				j++;
			}
	}

	return 0;
}

OBJ_DATA *
get_obj_in_list_vis (CHAR_DATA * ch, const char *name, OBJ_DATA * list)
{
	OBJ_DATA *obj;
	int j = 1;
	int number;
	char tmpname[MAX_INPUT_LENGTH];
	char *tmp;

	if (!name || !*name)
		return NULL;

	if (!list)
		return NULL;

	strcpy (tmpname, name);
	tmp = tmpname;

	if (!(number = get_number (&tmp)))
		return NULL;

	if (*name == '#')
	{

		if (!(number = atoi (&name[1])))
			return NULL;

		for (obj = list; number && obj; obj = obj->next_content)
			if (can_see_obj(ch, obj) && !(--number))
				return obj;

		return NULL;
	}

	for (obj = list; obj && (j <= number); obj = obj->next_content)
	{
		if (isname (tmp, obj->name)
			|| (obj->obj_flags.type_flag == ITEM_BOOK && obj->book_title
			&& isname (tmp, obj->book_title)))
			if (can_see_obj(ch, obj))
			{
				if (j == number)
					return (obj);
				j++;
			}
	}

	return 0;
}

OBJ_DATA *
get_obj_in_dark (CHAR_DATA * ch, char *name, OBJ_DATA * list)
{
	OBJ_DATA *obj;
	int j = 1;
	int number;
	char tmpname[MAX_INPUT_LENGTH];
	char *tmp = '\0';

	*tmpname = '\0';

	strcpy (tmpname, name);
	tmp = tmpname;

	if (!(number = get_number (&tmp)))
		return 0;

	if (*name == '#')
	{

		if (!(number = atoi (&name[1])))
			return NULL;

		for (obj = list; number && obj; obj = obj->next_content)
			if (IS_OBJ_VIS (ch, obj) && !(--number))
				return obj;

		return NULL;
	}

	for (obj = list; obj && (j <= number); obj = obj->next_content)
	{
		if (isname (tmp, obj->name)
			|| (obj->obj_flags.type_flag == ITEM_BOOK && obj->book_title
			&& isname (tmp, obj->book_title)))
			if (IS_OBJ_VIS (ch, obj))
			{
				if (j == number)
					return obj;
				j++;
			}
	}

	return 0;
}

/* search the entire world for an object, and return a pointer  */
OBJ_DATA *
get_obj_vis (CHAR_DATA * ch, char *name)
{
	OBJ_DATA *tobj;
	std::list<OBJ_DATA*>::iterator tobj_iterator;
	int j, number;
	char tmpname[MAX_INPUT_LENGTH];
	char *tmp;

	/* scan items carried */
	if ((tobj = get_obj_in_list_vis (ch, name, ch->right_hand)))
		return (tobj);

	if ((tobj = get_obj_in_list_vis (ch, name, ch->left_hand)))
		return (tobj);

	/* scan room */
	if ((tobj = get_obj_in_list_vis (ch, name, vtor (ch->in_room)->contents)))
		return (tobj);

	strcpy (tmpname, name);
	tmp = tmpname;
	if (!(number = get_number (&tmp)))
		return (0);

	/* ok.. no luck yet. scan the entire obj list   */
	for (tobj_iterator = object_list.begin(); tobj_iterator != object_list.end(); tobj_iterator++)
	{
		tobj = *tobj_iterator;
		
		if (tobj->deleted)
			continue;

		if (isname (tmp, tobj->name)
			|| (tobj->obj_flags.type_flag == ITEM_BOOK && tobj->book_title
			&& isname (tmp, tobj->book_title)))
			if (can_see_obj(ch, tobj))
			{
				if (j == number)
					return (tobj);
				j++;
			}
	}
	return (0);
}



/* Generic Find, designed to find any object/character                    */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar_ch Will be NULL if no character was found, otherwise points     */
/* **tar_obj Will be NULL if no object was found, otherwise points        */
/*                                                                        */
/* The routine returns a pointer to the next word in *arg (just like the  */
/* one_argument routine).                                                 */

int
generic_find (char *arg, int bitvector, CHAR_DATA * ch,
			  CHAR_DATA ** tar_ch, OBJ_DATA ** tar_obj)
{
	

	int i;
	char name[256];
	bool found;

	found = false;


	/* Eliminate spaces and "ignore" words */
	while (*arg && !found)
	{

		for (; *arg == ' '; arg++);

		for (i = 0; (name[i] = *(arg + i)) && (name[i] != ' '); i++);
		name[i] = 0;
		arg += i;
		if (search_block (name, ignore, true) > -1)
			found = true;

	}

	if (!name[0])
		return (0);

	*tar_ch = 0;
	*tar_obj = 0;

	if (IS_SET (bitvector, FIND_CHAR_ROOM))
	{				/* Find person in room */
		if ((*tar_ch = get_char_room_vis (ch, name)))
		{
			return (FIND_CHAR_ROOM);
		}
	}

	if (IS_SET (bitvector, FIND_CHAR_WORLD))
	{
		if ((*tar_ch = get_char_vis (ch, name)))
		{
			return (FIND_CHAR_WORLD);
		}
	}

	if (IS_SET (bitvector, FIND_OBJ_EQUIP))
	{
		for (found = false, i = 0; i < MAX_WEAR && !found; i++)
			if (get_equip (ch, i) && !str_cmp (name, get_equip (ch, i)->name))
			{
				*tar_obj = get_equip (ch, i);
				found = true;
			}
			if (found)
			{
				return (FIND_OBJ_EQUIP);
			}
	}

	if (IS_SET (bitvector, FIND_OBJ_INV))
	{
		if ((*tar_obj = get_obj_in_list (name, ch->right_hand)))
		{
			return (FIND_OBJ_INV);
		}

		if ((*tar_obj = get_obj_in_list (name, ch->left_hand)))
		{
			return (FIND_OBJ_INV);
		}

		if ((*tar_obj = get_obj_in_list (name, ch->equip)))
		{
			return (FIND_OBJ_INV);
		}
	}

	if (IS_SET (bitvector, FIND_OBJ_ROOM))
	{
		if ((*tar_obj =
			get_obj_in_list_vis (ch, name, vtor (ch->in_room)->contents)))
		{
			return (FIND_OBJ_ROOM);
		}
	}

	if (IS_SET (bitvector, FIND_OBJ_WORLD))
	{
		if ((*tar_obj = get_obj_vis (ch, name)))
		{
			return (FIND_OBJ_WORLD);
		}
	}

	return (0);
}

/* Return true if obj with vnum is equipt */

bool
get_obj_in_equip_num (CHAR_DATA * ch, long vnum)
{
	OBJ_DATA *eq;

	for (eq = ch->equip; eq; eq = eq->next_content)
		if (eq->nVirtual == vnum)
			return 1;

	return 0;
}

void
update_delays (void)
{
	AFFECTED_TYPE *af;
	CHAR_DATA *ch;
	CHAR_DATA *tch;
	CHAR_DATA *target;
	std::list<char_data*>::iterator tch_iterator;
	
	
	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		ch = *tch_iterator;

		if (!ch || ch->deleted || !ch->room || ch->in_room == NOWHERE)
			continue;

		if (ch->roundtime)
		{
			ch->roundtime--;
			if (ch->roundtime <= 0)
			{
				ch->act("You can perform another action, now.", false, 0, 0,
					TO_CHAR | _ACT_FORMAT);
			}
		}
		if (ch->balance < 0)
		{
			ch->balance++;
			if (ch->balance == 0)
			{
				ch->send_to_char("You feel as if you have fully regained your balance.\n");
			}
		}
		

		if (ch->deleted || !ch->delay || --ch->delay)
			continue;

		switch (ch->delay_type)
		{
		case DEL_APP_APPROVE:
			ch->delay += 10;
			break;
		case DEL_COUNT_COIN:
			ch->delayed_count_coin();
			break;
		case DEL_GET_ALL:
			ch->delayed_get();
			break;
		case DEL_SEARCH:
			ch->delayed_search();
			break;
		case DEL_ALERT:
			ch->delayed_alert();
			break;
		case DEL_INVITE:
			ch->break_delay();
			break;
		case DEL_CAMP1:
			ch->delayed_camp1();
			break;
		case DEL_CAMP2:
			ch->delayed_camp2();
			break;
		case DEL_CAMP3:
			ch->delayed_camp3();
			break;
		case DEL_CAMP4:
			ch->delayed_camp4();
			break;
		case DEL_TAKE:
			ch->delayed_take();
			break;
		case DEL_PUTCHAR:
			ch->delayed_putchar();
			break;
		case DEL_HIDE:
			ch->delayed_hide();
			break;
		case DEL_SCAN:
			ch->delayed_scan();
			break;
		case DEL_QUICK_SCAN:
			ch->delayed_quick_scan();
			break;
		case DEL_HIDE_OBJ:
			ch->delayed_hide_obj();
			break;
		case DEL_TRACK:
			ch->delayed_track();
			break;
		case DEL_MEND_OBJECT:  
			ch->delayed_mend();
			break;
		case DEL_SCAN_ALL:
			ch->delayed_scan_all();
			break;		
		}
	}
}


AFFECTED_TYPE *
is_room_affected (AFFECTED_TYPE * af, int type)
{
	while (af)
	{
		if (af->type == type)
			return af;

		af = af->next;
	}

	return NULL;
}

void
increase_room_affect (AFFECTED_TYPE ** af, int type, int intensity)
{
	AFFECTED_TYPE *raffect;

	if ((raffect = is_room_affected (*af, type)))
	{
		raffect->a.room.intensity += intensity;
		return;
	}
}

void
add_room_affect (AFFECTED_TYPE ** af, int type, int duration, int intensity)
{
	AFFECTED_TYPE *raffect;

	if ((raffect = is_room_affected (*af, type)))
	{
		if (duration)
		raffect->a.room.duration += duration;
		
		if (intensity)
			raffect->a.room.intensity += intensity;
		return;
	}


	raffect = new AFFECTED_TYPE;

	raffect->type = type;
		
	if (duration != 0)
		raffect->a.room.duration = duration;
	else
		raffect->a.room.duration = 1;
		
	if (intensity > 1)
		raffect->a.room.intensity = intensity;
	else
		raffect->a.room.intensity = 1;
	
	if (*af)
	raffect->next = *af;
	else 
		raffect->next = NULL;


	*af = raffect;
	
}

int
is_in_room (CHAR_DATA * ch, CHAR_DATA * target)
{
	CHAR_DATA *tch;

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{
		if (tch == target)
			return 1;
	}

	return 0;
}

int add_registry (int reg_index, int int_value, const char* str_value)
{
	REGISTRY_DATA *new_reg;
	static int init = 0;

		//for (; init < MAX_REGISTRY; init++)	/* Only executes once in */
		//registry[init] = NULL;	/* the life of the game  */

	if (int_value == -1)
	{
		abort ();
	}

	new_reg = new REGISTRY_DATA;
	new_reg->str_value.assign(str_value);
	new_reg->int_value = int_value;
	new_reg->next = registry[reg_index];
	registry[reg_index] = new_reg;

	return 0;
}

void
free_registry (void)
{
	int i = 0;

	while (registry[i])
		i++;
	i--;
	while (i)
	{
		delete registry[i];
		i--;
	}

}

int lookup_value (std::string str_value, int reg_index)
{
	REGISTRY_DATA *reg;

	if (str_value.empty())
		return -1;

	for (reg = registry[reg_index]; reg; reg = reg->next)
		if (str_value.compare(reg->str_value))
			return reg->int_value;

	return -1;
}

std::string lookup_string (int value, int reg_index)
{
	REGISTRY_DATA *reg;

	for (reg = registry[reg_index]; reg; reg = reg->next)
		if (value == reg->int_value)
			return reg->str_value;

	return NULL;
}



void setup_registry (void)
{
	int i;

	add_registry (REG_REGISTRY, REG_REGISTRY, "Registry");
	add_registry (REG_REGISTRY, REG_SPELLS, "Spells");
	add_registry (REG_REGISTRY, REG_DURATION, "Duration");
	add_registry (REG_REGISTRY, REG_OV, "OV");
	add_registry (REG_REGISTRY, REG_LV, "LV");
	add_registry (REG_REGISTRY, REG_CAP, "Cap");
	add_registry (REG_REGISTRY, REG_MISC, "Misc");
	add_registry (REG_REGISTRY, REG_MAGIC_SPELLS, "Magic");
	add_registry (REG_REGISTRY, REG_MAX_RATES, "Rates");
	add_registry (REG_REGISTRY, REG_CRAFT_MAGIC, "Craftmagic");

	/* Add in all skills to craft magic */

	for (i = 1; i < skill_data_map.size(); i++)
		add_registry (REG_CRAFT_MAGIC, MAGIC_SKILL_MOD_FIRST + i, lookup_skill_name(i));
	
	add_registry (REG_MISC_NAMES, MISC_MAX_MOVE, "Maxmove");
	add_registry (REG_MISC_NAMES, MISC_STEAL_DEFENSE, "Stealdefense");

		//average of con and wil, times 5, plus 25
	add_registry (REG_MISC, MISC_MAX_MOVE, "(con + wil) / 2 * 5 + 25");
	add_registry (REG_MISC, MISC_STEAL_DEFENSE, "(int + dex) / 2");

}


	//No provision is made to morph shopkeepers, or other specialized mobs
void
morph_mob (CHAR_DATA * old_mob)
{
	char nbuf[MAX_STRING_LENGTH] = {'\0'};
	CHAR_DATA *newMob = NULL;
	OBJ_DATA *nobj;
	int temp_vnum = 0;
	int troom = 0;
	int jdex = 0;
	int flag = 0;
	char* skill_name;
	std::map<std::string, SKILL_DATA*> ::iterator skill_map_it;
	SKILL_DATA* tskill;
	
	if (old_mob->deleted)
		return;

	if (old_mob->is_subduer()
		|| old_mob->is_subduee()
		|| old_mob->following > 0
		|| !IS_NPC (old_mob))
		return;

	temp_vnum = old_mob->mob->morphto;
	troom = old_mob->in_room;
	flag = old_mob->mob->morph_type;

	if (temp_vnum <= 0)
	{
		sprintf (nbuf, "Mob %d has a morph clock, but no morph Mobvnum\n",
			old_mob->mob->nVirtual);
		system_log (nbuf, true);
		return;
	}

	if (troom <= 0)
	{
		sprintf (nbuf, "Mob %d has a morph clock, and will morph to %d but there is an error with the room number\n",
			old_mob->mob->nVirtual, old_mob->mob->morphto);
		system_log (nbuf, true);
		return;
	}

	if (temp_vnum == 86)
	{
		old_mob->extract_char();
		return;
	}
	else
	{
		newMob = load_mobile (temp_vnum);
	}

	if (!newMob)
	{
		sprintf (nbuf, "Attempt to load target morph mob %d from mob %d failed\n",
			old_mob->mob->morphto, old_mob->mob->nVirtual);
		system_log (nbuf, true);
		send_to_gods(nbuf);
		return;
	}

	/********************
	morphtype = 1 will simply transfer gear to the new mob.
	morphtype = 2 will keep the same description, just change the skills

	*********************/
	if (flag == 1)
		// physical morph includes new skills from new prototype
	{
		newMob->last_room = old_mob->last_room;
		newMob->hour_affects = old_mob->hour_affects;
	}

	else if (flag == 2) // skill morph - Physical remains the same
	{

		newMob->flags = old_mob->flags;

		newMob->hit = old_mob->hit;
		
		newMob->agi = old_mob->agi;
		newMob->aur = old_mob->aur;
		newMob->con = old_mob->con;
		newMob->dex = old_mob->dex;
		newMob->intel = old_mob->intel;
		newMob->str = old_mob->str;
		newMob->wil = old_mob->wil;
		newMob->luk = old_mob->luk;
		
		newMob->tmp_agi = old_mob->tmp_agi;
		newMob->tmp_agi  = old_mob->tmp_agi;
		newMob->tmp_con = old_mob->tmp_con;
		newMob->tmp_dex = old_mob->tmp_dex;
		newMob->tmp_intel = old_mob->tmp_intel;
		newMob->tmp_str = old_mob->tmp_str;
		newMob->tmp_wil = old_mob->tmp_wil;
		newMob->tmp_luk = old_mob->tmp_luk;

		newMob->frame = old_mob->frame;
		newMob->height = old_mob->height;
		newMob->bmi = old_mob->bmi;
		newMob->body_proto = old_mob->body_proto;
		newMob->body_type = old_mob->body_type	;
		newMob->sex = old_mob->sex;
		newMob->size = old_mob->size;

		
		newMob->mob->cell_1 = old_mob->mob->cell_1;
		newMob->mob->cell_2 = old_mob->mob->cell_2;
		newMob->mob->cell_3 = old_mob->mob->cell_3;
		newMob->in_room = old_mob->in_room;
		newMob->last_room = old_mob->last_room;
		newMob->max_hit = old_mob->max_hit;
		newMob->max_move = old_mob->max_move;
		newMob->move = old_mob->move;
		newMob->speaks = old_mob->speaks;
		newMob->speed = old_mob->speed;
		newMob->race = old_mob->race;
		newMob->affected_by = old_mob->affected_by;
		newMob->action = old_mob->action;

		
		if (old_mob->keywords)
		{
			sprintf (nbuf, "%s", old_mob->keywords);
			newMob->keywords = duplicateString (nbuf);
		}

		if (old_mob->name)
		{
			sprintf (nbuf, "%s", old_mob->name);
			newMob->name = duplicateString (nbuf);
		}

		if (old_mob->short_descr)
		{
			sprintf (nbuf, "%s", old_mob->short_descr);
			newMob->short_descr = duplicateString (nbuf);
		}

		if (old_mob->long_descr)
		{
			sprintf (nbuf, "%s", old_mob->long_descr);
			newMob->long_descr = duplicateString (nbuf);
		}

		if (old_mob->pmote_str)
		{
			sprintf (nbuf, "%s", old_mob->pmote_str);
			newMob->pmote_str = duplicateString (nbuf);
		}

		if (old_mob->voice_str)
		{
			sprintf (nbuf, "%s", old_mob->voice_str);
			newMob->voice_str = duplicateString (nbuf);
		}

		if (old_mob->description)
		{
			sprintf (nbuf, "%s", old_mob->description);
			newMob->description = duplicateString (nbuf);
		}

		if (old_mob->travel_str)
		{
			sprintf (nbuf, "%s", old_mob->travel_str);
			newMob->travel_str = duplicateString (nbuf);
		}

		if (old_mob->dmote_str)
		{
			sprintf (nbuf, "%s", old_mob->dmote_str);
			newMob->dmote_str = duplicateString (nbuf);
		}

		newMob->hour_affects = old_mob->hour_affects;

		/*
		The new mob will take the best skill level between his old skill and the new skill level.
		*/

		for (skill_map_it = skill_data_map.begin(); skill_map_it != skill_data_map.end(); skill_map_it++)
		{
			tskill = skill_map_it->second;
			skill_name = strdup(tskill->skill_name.c_str());
			
			if (!(old_mob->skill_map.find(skill_name) == old_mob->skill_map.end())
				&& !(newMob->skill_map.find(skill_name) == newMob->skill_map.end())
				&& (old_mob->skill_map[skill_name] > newMob->skill_map[skill_name]))
			{
				newMob->skill_map[skill_name] = old_mob->skill_map[skill_name];
			}
		}
		
		
	} //flag ==2

	/******** objects and equip for all ***/
	for (jdex = 1; jdex < MAX_WEAR; jdex++)
	{
		if (get_equip (old_mob, jdex))
		{
			nobj = unequip_char (old_mob, jdex);
			obj_to_char (nobj, newMob);
			equip_char (newMob, nobj, jdex);
		}
	}

	if (old_mob->right_hand)
	{
		nobj = old_mob->right_hand;
		old_mob->right_hand = NULL;
		nobj->equiped_by = newMob;
		nobj->carried_by = newMob;
		newMob->right_hand = nobj;
	}

	if (old_mob->left_hand)
	{
		nobj = old_mob->left_hand;
		old_mob->left_hand = NULL;
		nobj->equiped_by = newMob;
		nobj->carried_by = newMob;
		newMob->left_hand = nobj;
	}


	newMob->mob->action |= ACT_STAYPUT;
	old_mob->extract_char();
	newMob->char_to_room(troom);


}




