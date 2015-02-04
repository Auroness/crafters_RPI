//////////////////////////////////////////////////////////////////////////////
//
/// objects.cpp : Object Module
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
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <set>
#include <map>
#include <list>
#include <vector>
#include "server.h"
#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"
#include "decl.h"
#include "comm.emote.h"

extern rpie::server engine;
extern std::map<int, OBJECT_MATERIAL*> object_material_map;


std::map<int, VARIANT_VALUE*> gl_variant; //global variant lists (colors, etc)

const char *locations[36] = {
	"",			//light?
	"hand",		//primary
	"hand",		//secondary
	"hands",	//both
	"head",
	"hair",
	"ears",
	"eyes",
	"face",
	"throat",
	"neck",
	"neck",
	"body",
	"body",
	"back",
	"shoulder",
	"shoulder",
	"upper arm",
	"upper arm",
	"arms",
	"wrist",
	"wrist",
	"hands",
	"finger",
	"finger",
	"waist",
	"belt",
	"belt",
	"legs",
	"ankle",
	"ankle",
	"feet",
	"hands",
	"hands",
	"\n"
};

OBJ_DATA *
new_object ()
{
	OBJ_DATA *obj = new obj_data;
	int i = 0;
	
	obj->writing = NULL;
	obj->writing_loaded = false;
	obj->xaffected = NULL;
	obj->zone = 0;
	obj->morphTime = 0;
	obj->morphto = 0;
	obj->name = NULL;
	obj->next = NULL;
	obj->next_content = NULL;
	obj->nVirtual = 0;
	obj->obj_timer = 0;
	obj->omote_str = NULL;
	obj->open = 0;
	obj->order_time = 0;
	obj->quality = 0;
	obj->short_description = NULL;
	obj->size = 0;
	obj->sold_at = 0;
	obj->sold_by = 0;
	obj->title_language = 0;
	obj->title_script = 0;
	obj->title_skill = -1;
	obj->tmp_flags = 0;
	obj->deleted = 0;
	obj->desc_keys = NULL;
	obj->description = NULL;
	obj->econ_flags = 0;
	obj->equiped_by = NULL;
	obj->coppers = 0.0;
	obj->max_hit_points = 0.0;
	obj->current_damage = 0;
	obj->full_description = NULL;
	obj->in_obj = NULL;
	obj->in_room = 0;
	obj->ink_color = NULL;
	obj->instances = 0;
	obj->item_wear = 100;
	obj->loaded = NULL;
	obj->location = 0;
	obj->activation = 0;
	obj->book_title = NULL;
	obj->carried_by = NULL;
	obj->clan_data = NULL;
	obj->clock = 0;
	obj->coldload_id = 0;
	obj->contained_wt = 0;
	obj->contains = NULL;
	obj->count = 0;
	obj->var_val.clear();
	obj->protect_loc.clear();
	obj->damage.clear();
	
	for (i=0; i< MAX_OBJ_MATERIALS; i++)
	{
		obj->materials[i] = "";
	}
	
	return obj;
}


void
obj_data::partial_deep_copy (OBJ_DATA *proto)
{
	
	int i;
	
	if (!IS_SET (this->obj_flags.extra_flags, ITEM_VARIABLE))
	{
		if (this->name)
		{
			free_mem(this->name);
		}
		this->name = duplicateString(proto->name);
	
		if (this->short_description)
		{
			free_mem(this->short_description);
		}
		this->short_description = duplicateString(proto->short_description);
	
		if (this->description)
		{
			free_mem(this->description);
		}
		this->description = duplicateString(proto->description);
	
		if (this->full_description)
		{
			free_mem(this->full_description);
		}
		this->full_description = duplicateString(proto->full_description);
	}

	if (!IS_SET (this->obj_flags.extra_flags, ITEM_VARIABLE))
		this->obj_flags.extra_flags = proto->obj_flags.extra_flags;
	else 
		insert_string_variables (this, "");
	
	this->obj_flags.type_flag = proto->obj_flags.type_flag;

	this->obj_flags.wear_flags = proto->obj_flags.wear_flags;

	this->o.od.value[0] = proto->o.od.value[0];

	if (obj_flags.type_flag != ITEM_KEY)
		this->o.od.value[1] = proto->o.od.value[1];

	this->o.od.value[2] = proto->o.od.value[2];
	this->o.od.value[3] = proto->o.od.value[3];
	this->o.od.value[4] = proto->o.od.value[4];
	this->o.od.value[5] = proto->o.od.value[5];

	this->zone = proto->zone;
	this->quality = proto->quality;
	this->econ_flags = proto->econ_flags;
	this->obj_flags.weight = proto->obj_flags.weight;
	this->coppers = proto->coppers;
	this->activation = proto->activation;

	if (!this->equiped_by)
		this->size = proto->size;

	this->item_wear = proto->item_wear;

	this->protect_loc = proto->protect_loc;
	
	for (i=0; i< MAX_OBJ_MATERIALS; i++)
	{
		this->materials[i] = proto->materials[i];
	}
	
}

void
obj_data::deep_copy (OBJ_DATA *copy_from)
{
	memcpy(this, copy_from, sizeof(obj_data));

	if (copy_from->short_description)
	{
		this->short_description = duplicateString(copy_from->short_description);
	}

	if (copy_from->name)
	{
		this->name = duplicateString(copy_from->name);
	}

	if (copy_from->description)
	{
		this->description = duplicateString(copy_from->description);
	}

	if (copy_from->full_description)
	{
		this->full_description = duplicateString(copy_from->full_description);
	}

	if (copy_from->omote_str)
	{
		this->omote_str = duplicateString(copy_from->omote_str);
	}

	if (copy_from->ink_color)
	{
		this->ink_color = duplicateString(copy_from->ink_color);
	}

	if (copy_from->desc_keys)
	{
		this->desc_keys = duplicateString(copy_from->desc_keys);
	}

	if (copy_from->book_title)
	{
		this->book_title = duplicateString(copy_from->book_title);
	}

	if (copy_from->protect_loc.size() != 0)
		this->protect_loc = copy_from->protect_loc;

	
}

obj_data::~obj_data ()
{
	if (this->xaffected)
	{
		free_mem(this->xaffected);
	}
	
	if (this->name)
	{
		free_mem(this->name);
	}
	
	if (this->description)
	{
		free_mem(this->description);
	}
	
	if (this->short_description)
	{
		free_mem(this->short_description);
	}
	
	if (this->full_description)
	{
		free_mem(this->full_description);
	}
	
	if (this->omote_str)
	{
		free_mem(this->omote_str);
	}
	
	if (this->carried_by)
	{
		free_mem(this->carried_by);
	}

	if (this->equiped_by)
	{
		free_mem(this->equiped_by);
	}
	
	if (this->in_obj)
	{
		free_mem(this->in_obj);
	}
	
	if (this->contains)
	{
		free_mem(this->contains);
	}
	
	if (this->next_content)
	{
		free_mem(this->next_content);
	}
	
	if (this->next)
	{
		free_mem(this->contains);
	}
		
	if (this->writing)
	{
		free_mem(this->writing);
	}
	
	if (this->ink_color)
	{
		free_mem(this->ink_color);
	}
	
	if (this->loaded)
	{
		free_mem(this->loaded);
	}
	
	
	if (this->desc_keys)
	{
		free_mem(this->desc_keys);
	}
	
	if (this->book_title)
	{
		free_mem(this->book_title);
	}
	
	if (this->indoor_desc)
	{
		this->indoor_desc = NULL;
	}
	
			
	if (this->clan_data)
	{
		free_mem(this->clan_data);
	}
	
	for (int i=0; i < MAX_OBJ_MATERIALS; i++)
	{
		if (this->materials[i])
		{
			free_mem(this->materials[i]);
		}
	}
	
	this->damage.clear();
	this->var_val.clear();
	this->protect_loc.clear();

}
	//determines if a specified material string is in the object_material_map
int
find_material(char* argument)
{
	std::map<int, OBJECT_MATERIAL*>::iterator it_material;
	
	for (it_material = object_material_map.begin(); it_material != object_material_map.end(); it_material++)
	{
		if (!str_cmp(it_material->second->material_name, argument))
		{
			return 1;
		}
	}
	return 0;
}

	//determines the armor value (hardness) of an object based on an average of all materials in the object
	
int 
get_material_hardness(OBJ_DATA* tobj)
{
	std::set<std::string>::iterator obj_it;
	std::map<int, OBJECT_MATERIAL*>::iterator it_material;
	std::string tmp_str;
	int summation = 0;
	int counter = 0;
	float average = 0.0;
	std::string value;
	
				
	for (int i=0; i < MAX_OBJ_MATERIALS; i++)
	{
		for (it_material = object_material_map.begin(); it_material != object_material_map.end(); it_material++)
		{
			
			if (!str_cmp(it_material->second->material_name, tobj->materials[i]))
			{
				summation += it_material->second->hardness;
				counter ++;
			}
			
		}
	}
		
	if (counter == 0)
		average = 0;
	else 
		average = summation / counter;
	
	return ((int)average);
}


/*------------------------------------------------------------------------\
|  do_mend()                                                              |
|                                                                         |
|  User command to repair a damaged object.                               |
\------------------------------------------------------------------------*/
void
do_mend (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA * tch;
	OBJ_DATA *obj;
	OBJ_DATA *kit;
	int nItemType = 0;
	char buf[AVG_STRING_LENGTH];
	char buf2[AVG_STRING_LENGTH];
	int targ_cnt = 0;


	
	if (!*argument || strlen (argument) > AVG_STRING_LENGTH)
	{
		ch->send_to_char ("Mend what?\n");
		return;
	}
	argument = one_argument (argument, buf); //item to mend

		//for NPC repair
	if ((obj = get_obj_in_dark (ch, buf, ch->right_hand))
		|| (obj = get_obj_in_dark (ch, buf, ch->left_hand)))
	{
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			if (IS_NPC (tch) && IS_SET (tch->mob->profession, PROF_REPAIR))
			{
				npc_repair (ch, tch, obj, argument);
				return;
			}
		}
	}
	else
	{
		ch->send_to_char ("You're not holding anything like that.\n");
		return;
	}

		//for PC repairing
	/* Get the kit from our other hand */
	kit = ((obj == ch->right_hand) ? ch->left_hand : ch->right_hand);
	if (!kit || kit->obj_flags.type_flag != ITEM_REPAIR_KIT)
	{
		ch->act("You're not holding anything that you can mend $p with.",
			false,  obj, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (kit->o.od.value[0] == 0)
	{
		ch->send_to_char ("That repair kit no longer contains any useful materials.\n");
		return;
	}


	argument = one_argument (argument, buf); //which damage to mend
	targ_cnt = atoi(buf);

	if (targ_cnt > 0)
			{
		ch->delay_info1 = targ_cnt; //repair single damage mode

			}
	else 
		{
			ch->delay_info1 = 0; //repair all damage mode
		}


	if (kit->o.od.value[3] > 0 && kit->o.od.value[2] > ch->skill_map[lookup_skill_name(kit->o.od.value[3])])
	{
		ch->send_to_char
		("You do not have the skill required to use this repair kit.\n");
		return;
	}



	if ((nItemType = obj->obj_flags.type_flag))
	{

	 // Check if the kit works with the item 

	 if (!(kit->o.od.value[5] == 0)	// all items 
			&& !(kit->o.od.value[5] == nItemType))
		{
			ch->act("You're not holding anything that you can mend $p with.",
				 false,  obj, 0, TO_CHAR | _ACT_FORMAT);
			return;

		}

	}


	ch->act("You're begin to work on $p with $P.", false,  obj, kit,
		TO_CHAR | _ACT_FORMAT);

	sprintf (buf2, "$n begins working on $p with $P.", obj, kit);
	ch->act(buf2, false,  0, 0, TO_ROOM | _ACT_FORMAT);

	/** set up a delay, and do the actual repair in another function **/
	
		begin_repair(ch, obj, kit, ch->delay_info1); 
	

}

/*------------------------------------------------------------------------\
|  do_rend()                                                              |
|                                                                         |
|  User command to tear into an object                          |
\------------------------------------------------------------------------*/
void
do_rend (CHAR_DATA * ch, char *argument, int cmd)
{
	int i = 0;
	int num_args = 0;
	int max_avail = 0;
	unsigned int impact = 0;
	int type = -1;
	char buf[11][MAX_STRING_LENGTH / 11];
	char *str_damage_sdesc = NULL;
	OBJ_DATA *target_obj = NULL;
	OBJECT_DAMAGE *damage = NULL;
	CHAR_DATA *target_obj_ch = ch;	/* in possession of target_obj */
	CHAR_DATA *victim = NULL;
	char *error[] = {
		"\n"
		"Usage: rend [OPTIONS] object\n"
		"\n"
		"  -d IMPACT         - points of damage\n"
		"  -t TYPE           - See 'tag damage-types' for values.\n"
		"If  IMPACT or TYPE is left blank, a random value will be used.\n"
		"\n"
		"  -c CHARACTER      - Will damage the specified object on CHARACTER.\n"
		"\n"
		"Examples:\n"
		"  > rend tunic                             - Apply random damage to an obj\n"
		"  > rend -d 12 -t stab vest                - Specific damage to an obj.\n"
		"  > rend -t bloodstain -c traithe gloves   - Damage someone elses obj.\n",
		"#1Damage must be a number between 1 and the total hit points of the object.#0\n"
		"Hit points for an object is the sum of the item's Wear and Quality\n",
		"#1Unknown attack type, refer to 'tags damage-types' for a list of values.#0\n",
		"#1You don't see the subject of the -c option.#0\n",
		"#1You don't see that target object or victim.#0\n",
		"#1You can't use the -c option with a victim.#0\n"
	};
	extern const char *obj_dam_type[];
	
	if (!(num_args = sscanf (argument, "%s %s %s %s %s %s %s %s %s %s %s",
							 buf[0], buf[1], buf[2], buf[3], buf[4], buf[5],
							 buf[6], buf[7], buf[8], buf[9], buf[10])))
	{
		ch->send_to_char (error[0]);
		return;
	}
	
	for (i = 0; i < num_args; i++)
	{
		if (strcmp (buf[i], "-d") == 0)
		{
			if ((i == num_args - 1) || !sscanf (buf[i + 1], "%d", &impact)
				|| impact < 0 || impact > 100)
			{
				ch->send_to_char (error[1]);
				ch->send_to_char (error[0]);
				return;
			}
			else
			{
				buf[++i][0] = '\0';
			}
		}
		else if (strcmp (buf[i], "-t") == 0)
		{
			if ((i == num_args - 1)
				|| (type = index_lookup (obj_dam_type, buf[i + 1])) < 0)
			{
				ch->send_to_char (error[2]);
				ch->send_to_char (error[0]);
				return;
			}
			else
			{
				buf[++i][0] = '\0';
			}
		}
		else if (strcmp (buf[i], "-c") == 0)
		{
			if ((i == num_args - 1) || !*buf[i + 1]
				|| !(target_obj_ch = get_char_room_vis (ch, buf[i + 1])))
			{
				ch->send_to_char (error[3]);
				return;
			}
			else
			{
				buf[++i][0] = '\0';
			}
		}
		else
		{
			if (!*buf[i]
				|| !((victim = get_char_room_vis (ch, buf[i]))
					 || (target_obj =
						 get_obj_in_dark (target_obj_ch, buf[i],
										  target_obj_ch->right_hand))
					 || (target_obj =
						 get_obj_in_dark (target_obj_ch, buf[i],
										  target_obj_ch->left_hand))
					 || (target_obj =
						 get_obj_in_dark (target_obj_ch, buf[i],
										  target_obj_ch->equip))
					 || (target_obj =
						 get_obj_in_list_vis (target_obj_ch, buf[i],
											  target_obj_ch->room->contents))))
			{
				ch->send_to_char (error[4]);
				return;
			}
		}
	}
	if (!target_obj && !victim)
	{
		ch->send_to_char (error[4]);
		ch->send_to_char (error[0]);
		return;
	}
	else if (victim && target_obj_ch != ch)
	{
		ch->send_to_char (error[5]);
		ch->send_to_char (error[0]);
		return;
	}
	
	
		//random vaues for impact and type if they are not specified
	
	max_avail = (int)((target_obj->max_hit_points - target_obj->current_damage) + get_material_hardness(target_obj));
	
	impact = (impact <= 0) ? number (1, max_avail) : impact;
	
		//randomly chosen from first 10 types - physical/combat damage
	type = (type <= 0) ? number (0, 10) : type;
	
	damage = object__add_damage (target_obj, type, impact, ch, 0);
	
	if (damage)
		str_damage_sdesc = object_damage__get_sdesc (damage);
	
	if (damage && str_damage_sdesc)
	{
		
		sprintf (buf[0],
				 "You concentrate on %s until #1%s#0 appears on %s.",
				 (target_obj_ch != ch) ? "$N" : "$p",
				 str_damage_sdesc,
				 (target_obj_ch != ch) ? "$p" : "#3it#0");
		sprintf (buf[1], "You notice #1%s#0 on $p.", str_damage_sdesc);
		
	}
	else 
	{
		sprintf (buf[0],
				 "You concentrate on %s but nothing seems to happen to %s.",
				 (target_obj_ch != ch) ? "$N" : "$p",
				 (target_obj_ch != ch) ? "$p" : "#2it#0");
		buf[1][0] = '\0';
	}
	ch->act(buf[0], false,  target_obj, target_obj_ch,
		 TO_CHAR | _ACT_FORMAT);
	
	if ((target_obj_ch != ch) && buf[1][0])
		target_obj_ch->act(buf[1], false, target_obj, 0,
			 TO_CHAR | _ACT_FORMAT);
	
	
}


/*------------------------------------------------------------------------\
|  object__drench()                                                       |
|                                                                         |
|  Iterate through the object list and apply water damage where necessary |
\------------------------------------------------------------------------*/
void
object__drench (CHAR_DATA * ch, OBJ_DATA * _obj, bool isChEquip)
{
	OBJ_DATA *obj;

	if (_obj != NULL)
	{

		for (obj = _obj; obj != NULL; obj = obj->next_content)
		{

			/* Lights get extinguished in water */

			if (obj->obj_flags.type_flag == ITEM_LIGHT
				&& !IS_SET (obj->obj_flags.extra_flags, ITEM_MAGIC))
			{

				/* Lights out */
				if (obj->o.light.on)
				{
					ch->act("$p is extinguished.", false, obj, ch,
						TO_ROOM | TO_CHAR | _ACT_FORMAT);
					obj->o.light.on = false;
				}

				/* Spoil lanterns */
				obj->o.light.hours =
					(obj->o.light.hours <= obj->o.light.capacity
					&& obj->o.light.hours > 0) ? 0 : obj->o.light.hours;
			}

			/* Precious metals rust in water */

			if (!IS_SET (obj->obj_flags.extra_flags, ITEM_MAGIC)
				&& (strstr (obj->name, "steel") || strstr (obj->name, "iron")
				|| strstr (obj->full_description, "steel")
				|| strstr (obj->full_description, "iron")))
			{
				object__add_damage (obj, DAMAGE_WATER, 1, ch, 0);
			}


		}

	}
	if (isChEquip)
	{
		object__drench (ch, ch->left_hand, false);
		object__drench (ch, ch->right_hand, false);
	}
}


/*------------------------------------------------------------------------\
|  examine_damage()                                                       |
|                                                                         |
|  Iterate through the damage list and show the sdesc of each instance.   |
\------------------------------------------------------------------------*/
char *
object__examine_damage (OBJ_DATA * thisPtr, CHAR_DATA * ch)
{
	OBJECT_DAMAGE *tdamage = NULL;
	char *str_damage_sdesc;
	int dam_count;
	static char buf[MAX_STRING_LENGTH]= { '\0' };
	std::string tmpbuf;
	std::vector<OBJECT_DAMAGE *>::iterator odam_iterator;
	*buf = '\0';
	
	tmpbuf.clear();
	tmpbuf.assign("It bears ");
	
	/* Iterate through the damage instances attached to this object */
	
	for (odam_iterator = thisPtr->damage.begin(), dam_count=thisPtr->damage.size(); odam_iterator != thisPtr->damage.end(); odam_iterator++, dam_count--)
	{
		tdamage = *odam_iterator;
		if (!tdamage)
			break;
		
		if (tdamage->source != DAMAGE_NONE)
		{
			str_damage_sdesc = object_damage__get_sdesc (tdamage);
			if (str_damage_sdesc != NULL)
			{
				if(ch->get_trust() > 1)
				{
					sprintf(buf, "#3%s (%d)", str_damage_sdesc, tdamage->impact);
					
					if (dam_count == 1)
					{
						sprintf(buf + strlen(buf), ".#0");
					}
					else
					{
						sprintf(buf + strlen(buf), ", and ");
					}
					
					
				}
				else
				{
					sprintf(buf, "#3%s", str_damage_sdesc);
					if (dam_count == 1)
					{
						sprintf(buf + strlen(buf), ".#0");
					}
					else
					{
						sprintf(buf + strlen(buf), ", and ");
					}
					
				}
				
				tmpbuf.append(buf);
			}
		}
	}
	
	
	if (!tmpbuf.empty()
		&& tmpbuf.size() > 9)//accounts for 'It bears ' part of phrase
	{
		sprintf(buf, "%s",  tmpbuf.c_str());
	}
	
	return buf;
}


/**
 For objects with multiple materials, the object takes the full force of the blow, across an average of the hardness of it's components
**/

OBJECT_DAMAGE *
object__add_damage (OBJ_DATA * thisObj, int source,
					unsigned int impact, CHAR_DATA * ch, int flag )
{
	OBJECT_DAMAGE *damage = NULL;
	char buf[AVG_STRING_LENGTH];
	char *mater_name;
	int i = 0;
	float threshold;
	float max_loss;
	float tmp1;
	std::map<int, OBJECT_MATERIAL*>::iterator it_material;
	std::vector<OBJECT_DAMAGE *>::iterator it_damage;
	
		//it has no materials, so it can't take damage
	if (!str_cmp(thisObj->materials[0], "none"))
		return NULL;
	
		//don't want PC able to destroy everything
	if (IS_SET(thisObj->obj_flags.extra_flags, ITEM_NODESTROY))
		return NULL;
	
		//allow damage for testing purposes TODO:remove this comment for game play
	/****
		//skip damage to NPC items
	if ((thisObj->equiped_by && IS_NPC(thisObj->equiped_by)) ||
		(thisObj->carried_by && IS_NPC(thisObj->carried_by)))
		return NULL;
	/****/
	
		//find a random material in multi-material items
	int roll = number(0, MAX_OBJ_MATERIALS-1);
	bool found = false;
	
	while (!found)
	{
		for (i = 0; i <= roll; i++)
		{
			if (thisObj->materials[i] && str_cmp(thisObj->materials[i], ""))
			{
				found = true; 
				mater_name = strdup(thisObj->materials[i]);
			}
		}
	}
		
		//with multiple damages, a group of lesser damages may combine to become a single larger damage. 
		//we check for this first, to make room for new damage
	if (!flag)
		consolidate_damage(thisObj, ch);
	
	for (it_material = object_material_map.begin(); it_material != object_material_map.end(); it_material++)
	{
		if (!str_cmp(it_material->second->material_name, mater_name))
		{			
			damage = object_damage__new_init (source, impact, it_material->second->type_material, get_material_hardness(thisObj), thisObj->max_hit_points );
				//damage can be null for Exemption List in object_damage__new_init
			if (damage) 
			{
				it_damage = thisObj->damage.begin();
				thisObj->damage.insert(it_damage, damage);
				
				
					//damage increase by the amount of impact, but hardness reduced that in object_damage__new_init.
				thisObj->current_damage += damage->impact;
				
				threshold = (1.0 + (10.0 * thisObj->quality)/1000);
				max_loss = thisObj->max_hit_points * threshold;
				if (thisObj->current_damage > max_loss)
				{
						//object is not destroyed, just really, really mangled
						//remove final damage, and replace with modified damage
						//15% drop in quality
					damage_from_obj (thisObj, damage);
					
					int new_impact = (max_loss - thisObj->current_damage +1);
					
					damage = object_damage__new_init (source, new_impact, it_material->second->type_material, get_material_hardness(thisObj), thisObj->max_hit_points );
					thisObj->damage.push_back(damage);
					
					thisObj->current_damage = (max_loss - 1);
					thisObj->quality = (int)(thisObj->quality * 0.85);
					thisObj->item_wear = 1;
					
					int tmproom = thisObj->equiped_by->in_room;
					
					unequip_char (thisObj->equiped_by, thisObj->location);
					obj_from_char (&thisObj, 1);
					obj_to_room (thisObj, tmproom);
					sprintf(buf, "%d with quality %d and cond %d fell to the floor\n",  thisObj->nVirtual, thisObj->quality, thisObj->item_wear);

				}
				
				
			}
			else 
			{
				if (ch->get_trust() > 1)
					ch->send_to_char ("No damage was done!");
				return NULL;
			}
			
		}
}
	
	
	tmp1 = (10.0 * thisObj->current_damage)/(10.0 * thisObj->max_hit_points);
	thisObj->item_wear = (int)((1.0 - tmp1) * 100);
	
	return damage;
}



void
char__switch_item (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *right, *left;

	right = ch->right_hand;
	left = ch->left_hand;

	if (!right && !left)
	{
		ch->act("You have nothing to switch!", false,  0, 0,
			TO_CHAR | _ACT_FORMAT);
		return;
	}

	if ((right && right->location == WEAR_BOTH) ||
		(left && left->location == WEAR_BOTH))
	{
		ch->act("You must grip that in both hands!", false,  0, 0,
			TO_CHAR | _ACT_FORMAT);
		return;
	}

	ch->right_hand = NULL;
	ch->left_hand = NULL;

	if (right && right->location != WEAR_BOTH)
	{
		ch->act("You shift $p to your left hand.", false,  right, 0,
			TO_CHAR | _ACT_FORMAT);
		ch->left_hand = right;
	}

	if (left && left->location != WEAR_BOTH)
	{
		ch->act("You shift $p to your right hand.", false,  left, 0,
			TO_CHAR | _ACT_FORMAT);
		ch->right_hand = left;
	}

}


void
clear_omote (OBJ_DATA * obj)
{

	if (obj->omote_str)
	{
		free_mem (obj->omote_str);
		obj->omote_str = (char *) NULL;
	}
}

int
can_obj_to_container (OBJ_DATA * obj, OBJ_DATA * container, char **msg,
					  int count)
{
	OBJ_DATA *tobj;
	int i = 0;
	static char message[160];

	if (count > obj->count || count <= 0)
		count = obj->count;

	*msg = message;

	
	if (container->obj_flags.type_flag == ITEM_KEYRING)
	{
		if (obj->obj_flags.type_flag != ITEM_KEY)
		{
			sprintf (message, "Keyrings are only able to hold keys!\n");
			return 0;
		}
		for (tobj = container->contains, i = 1; tobj; tobj = tobj->next_content)
			i++;
		if (i + 1 > container->o.od.value[0])
		{
			sprintf (message,
				"There are too many keys on this keyring to add another.\n");
			return 0;
		}
		return 1;
	}

	

	if (container->obj_flags.type_flag == ITEM_DRYCON)
	{
		
		if (container->o.drycon.volume == -1 ||
			(container->o.drycon.volume > container->o.drycon.capacity))
		{
			sprintf (message,"It is full already.\n");
			return 0;
		}
		
		if (container->o.drycon.contents != 0)
		{
			if (container->o.drycon.volume + count > container->o.drycon.capacity)
			{
				sprintf (message, "There is too much to put in this container.\n");
				return 0;
			}
		}
		else
			return 1;

		return 1;
	}
	
	if (container->obj_flags.type_flag != ITEM_CONTAINER)
	{
		sprintf (message, "%s is not a container.\n",
			container->short_description);
		*message = toupper (*message);
		return 0;
	}

	if (container == obj)
	{
		sprintf (message, "You can't do that.\n");
		return 0;
	}

	if (IS_SET (container->o.od.value[1], CONT_CLOSED))
	{
		sprintf (message, "%s is closed.\n", container->short_description);
		*message = toupper (*message);
		return 0;
	}

	if (count > 1)
	{
		if (container->contained_wt + obj->obj_flags.weight * count >
			container->o.od.value[0])
		{
			sprintf (message, "That much won't fit.\n");
			return 0;
		}
	}
	else if (container->contained_wt + obj->obj_flags.weight >
		container->o.od.value[0])
	{

		sprintf (message, "%s won't fit.\n", obj->short_description);
		*message = toupper (*message);
		return 0;
	}

	return 1;
}

#define NO_TOO_MANY		1
#define NO_TOO_HEAVY	2
#define NO_CANT_TAKE	3
#define NO_CANT_SEE		4
#define NO_HANDS_FULL	5

	//returns 0, and an error message, if it cannot be taken
	//returns 1 if it can be taken
int
can_obj_to_inv (OBJ_DATA * obj, CHAR_DATA * ch, int *error, int count)
{
	*error = 0;

	if (!obj)
	{
		*error = NO_CANT_SEE;
		return 0;
	}
	
	if (count > obj->count || count <= 0)
		count = obj->count;

	if (!obj->in_obj && !can_see_obj(ch, obj))
	{
		*error = NO_CANT_SEE;
		return 0;
	}

	if (!CAN_WEAR (obj, ITEM_TAKE) && (!ch->get_trust()))
	{
		*error = NO_CANT_TAKE;
		return 0;
	}

	if (ch->right_hand 
		&& (ch->right_hand->nVirtual == obj->nVirtual)
		&& (ch->right_hand->obj_flags.type_flag == ITEM_MONEY))
		return 1;

	if (ch->left_hand 
		&& (ch->left_hand->nVirtual == obj->nVirtual)
		&& (ch->left_hand->obj_flags.type_flag == ITEM_MONEY))
		return 1;

	if ((ch->right_hand && ch->left_hand) || get_equip (ch, WEAR_BOTH))
	{
		*error = NO_HANDS_FULL;
		return 0;
	}

	/* Check out the weight */

	if (!(obj->in_obj 
		  && obj->in_obj->carried_by == ch))
	{

		if (((ch->carrying() + (count * obj->obj_flags.weight)) > ch->carrying())
			&& !ch->get_trust())
		{
			*error = NO_TOO_HEAVY;
			return 0;
		}

	}

	return 1;
}

int
obj_activate (CHAR_DATA * ch, OBJ_DATA * obj)
{
	if (!obj->activation)
		return 0;

	magic_affect (ch, obj->activation);

	/* Deal with one time activation on object */

	if (!IS_SET (obj->obj_flags.extra_flags, ITEM_MULTI_AFFECT))
		obj->activation = 0;

	/* Oops, I guess that killed him. */

	if (ch->get_position() == DEAD)
		return 1;

	return 0;
}

void
get (CHAR_DATA * ch, OBJ_DATA * obj, int count)
{
	OBJ_DATA *container;
	CHAR_DATA *i;

	if (IS_SET (obj->obj_flags.extra_flags, ITEM_TIMER))
	{
		obj->obj_flags.extra_flags &= ~ITEM_TIMER;
		obj->obj_timer = 0;
	}

	if (!obj->in_obj)
	{

		if (IS_SET (obj->tmp_flags, SA_DROPPED))
		{
			ch->send_to_char ("That can't be picked up at the moment.\n");
			return;
		}

		obj_from_room (&obj, count);

		clear_omote (obj);

		ch->act("You get $p.", false,  obj, 0, TO_CHAR | _ACT_FORMAT);
		ch->act("$n gets $p.", false,  obj, 0, TO_ROOM | _ACT_FORMAT);

		obj_to_char (obj, ch);

		if (obj->activation &&
			IS_SET (obj->obj_flags.extra_flags, ITEM_GET_AFFECT))
			obj_activate (ch, obj);

		return;
	}

	/* Don't activate object if it is in an object we're carrying */

	container = obj->in_obj;

	obj_from_obj (&obj, count, container);

	ch->act("You get $p from $P.", false,  obj, container,
		TO_CHAR | _ACT_FORMAT);
	ch->act("$n gets $p from $P.", true,  obj, container,
		TO_ROOM | _ACT_FORMAT);

	obj_to_char (obj, ch);

	if (obj->activation && IS_SET (obj->obj_flags.extra_flags, ITEM_GET_AFFECT))
		obj_activate (ch, obj);
}

void
do_junk (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *obj;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char ("What did you wish to junk?\n");
		return;
	}

	if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
	{
		ch->send_to_char ("You don't seem to be holding that.\n");
		return;
	}

	obj_from_char (&obj, 0);
	if(IS_SET (ch->flags, FLAG_ISADMIN))
		extract_obj (obj);
	else
		obj_to_room(obj, JUNKYARD);
	
	sprintf (buf, "You junk #2%s#0.", obj->short_description);
	ch->act(buf, false,  obj, 0, TO_CHAR | _ACT_FORMAT);
}


void
do_get (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *obj = NULL;
	OBJ_DATA *container = NULL;
	int container_loc = CONTAINER_LOC_UNKNOWN;
	int count = 0;
	int error;
	SECOND_AFFECT *sa;
	char arg1[MAX_STRING_LENGTH]= { '\0' };
	char arg2[MAX_STRING_LENGTH]= { '\0' };
	bool coldload_id = false;

	*arg1 = '\0';
	*arg2 = '\0';

		//TODO Temporaily allow Guest to do things
	/***
	if ((!ch->get_trust()) && IS_SET (ch->room->room_flags, OOC)
		&& str_cmp (ch->room->name, PREGAME_ROOM_NAME))
	{
		ch->send_to_char ("This command has been disabled in OOC zones.\n");
		return;
	}
***/
	argument = one_argument (argument, arg1);

	if (just_a_number (arg1))
	{
		count = atoi (arg1);
		argument = one_argument (argument, arg1);
	}
	else if (!str_cmp (arg1, ".c"))
	{
		coldload_id = true;
		argument = one_argument (argument, arg1);
	}

	argument = one_argument (argument, arg2);

	if (!str_cmp (arg2, "from") || !str_cmp (arg2, "in"))
		argument = one_argument (argument, arg2);

	if (!str_cmp (arg2, "ground") || !str_cmp (arg2, "room"))
	{
		argument = one_argument (argument, arg2);
		container_loc = CONTAINER_LOC_ROOM;
	}

	else if (!str_cmp (arg2, "worn") || !str_cmp (arg2, "my"))
	{
		argument = one_argument (argument, arg2);
		container_loc = CONTAINER_LOC_WORN;
	}

	else if (!strn_cmp (arg2, "inventory", 3))
	{
		argument = one_argument (argument, arg2);
		container_loc = CONTAINER_LOC_INVENTORY;
	}

	if (*arg2)
	{

		if (container_loc == CONTAINER_LOC_UNKNOWN &&
			!(container = get_obj_in_dark (ch, arg2, ch->right_hand)) &&
			!(container = get_obj_in_dark (ch, arg2, ch->left_hand)) &&
			!(container = get_obj_in_dark (ch, arg2, ch->equip)) &&
			!(container = get_obj_in_list_vis (ch, arg2, ch->room->contents)))
			container_loc = CONTAINER_LOC_NOT_FOUND;

		else if (container_loc == CONTAINER_LOC_ROOM &&
			!(container =
			get_obj_in_list_vis (ch, arg2, ch->room->contents)))
		{
			container_loc = CONTAINER_LOC_NOT_FOUND;
		}

		else if (container_loc == CONTAINER_LOC_INVENTORY &&
			!(container = get_obj_in_dark (ch, arg2, ch->right_hand)) &&
			!(container = get_obj_in_dark (ch, arg2, ch->left_hand)))
			container_loc = CONTAINER_LOC_NOT_FOUND;

		else if (container_loc == CONTAINER_LOC_WORN &&
			!(container = get_obj_in_dark (ch, arg2, ch->equip)))
			container_loc = CONTAINER_LOC_NOT_FOUND;

		if (container_loc == CONTAINER_LOC_NOT_FOUND)
		{
			ch->send_to_char ("You neither have nor see such a container.\n");
			return;
		}

		if (container->obj_flags.type_flag != ITEM_CONTAINER &&
			container->obj_flags.type_flag != ITEM_DRYCON &&
			container->obj_flags.type_flag != ITEM_KEYRING)
		{
			ch->act("$o isn't a container.", true,  container, 0, TO_CHAR);
			return;
		}
		
		if (container->obj_flags.type_flag != ITEM_DRYCON &&
			IS_SET (container->o.container.flags, CONT_CLOSED))
		{
			ch->send_to_char ("That's closed!\n");
			return;
		}


		if (container_loc == CONTAINER_LOC_UNKNOWN)
		{
			if (container->carried_by)
				container_loc = CONTAINER_LOC_INVENTORY;
			else if (container->equiped_by)
				container_loc = CONTAINER_LOC_WORN;
			else
				container_loc = CONTAINER_LOC_ROOM;
		}
	}

	if (!*arg1)
	{
		ch->send_to_char ("Get what?\n");
		return;
	}

	if (!container)
	{
		
		if (!obj && isdigit (*arg1) && coldload_id)
		{
			obj = get_obj_in_list_id (atoi (arg1), ch->room->contents);
			
			if (!obj || obj->in_room != ch->in_room)
			{
				ch->send_to_char ("You don't see that here.\n");
				return;
			}
		}
		
		
		if (!obj)
		{
			obj = get_obj_in_list_vis (ch, arg1, ch->room->contents);
			if (!obj)
			{
				ch->send_to_char ("You don't see that here.\n");
				return;
			}
		}
		
		if (!can_obj_to_inv (obj, ch, &error, count))
		{
			if (error == NO_CANT_TAKE)
			{
				ch->act("You can't take $p.", true,  obj, 0, TO_CHAR);
			}
			else if (error == NO_TOO_MANY)
			{
				ch->send_to_char("You can't handle so much.");
			}
			else if (error == NO_TOO_HEAVY)
			{
				ch->send_to_char("You can't carry so much weight.");
			}
			else if (error == NO_CANT_SEE)
			{
				ch->send_to_char("You don't see it.");
			}
			else if (error == NO_HANDS_FULL)
			{
				ch->send_to_char("Your hands are full!");
			}
			return;
		}
		
		if ((sa = get_second_affect (ch, SA_GET_OBJ, obj)))
			return;
		
		get (ch, obj, count);
		return;
	}
	
	if (!str_cmp (arg1, "all"))
	{
		if (!container)
		{
			ch->send_to_char ("There is isn't a container to take from.\n");
			return;
		}
		if (container->obj_flags.type_flag == ITEM_DRYCON)
		{
			obj = vtoo (container->o.drycon.contents);
			if (!obj)
			{
				ch->send_to_char ("There is nothing left you can take.\n");
				return;
			}		
			
			if (!can_obj_to_inv (obj, ch, &error, count))
			{
				if (error == NO_CANT_TAKE)
				{
					ch->act("You can't take $p.", true,  obj, 0, TO_CHAR);
				}
				else if (error == NO_TOO_MANY)
				{
					ch->send_to_char("You can't handle so much.");
				}
				else if (error == NO_TOO_HEAVY)
				{
					ch->send_to_char("You can't carry so much weight.");
				}
				else if (error == NO_CANT_SEE)
				{
					ch->send_to_char("You don't see it.");
				}
				else if (error == NO_HANDS_FULL)
				{
					ch->send_to_char("Your hands are full!");
				}

				return;
			}
			
			
			ch->delay_type = DEL_GET_ALL;
			ch->delay_who = (char *) container;
			ch->delay_info1 = container_loc;
			ch->delay = 4;
			
			return;
		}
		else 
		{
			ch->send_to_char ("You'll have to get things one at a time.\n");
			return;
		}
	}
		
	
	/* get obj from container */

	if (container->obj_flags.type_flag == ITEM_DRYCON)
		obj = vtoo(container->o.drycon.contents);
	else
		obj = get_obj_in_dark (ch, arg1, container->contains);
	
	if (!obj)
	{
		ch->act("You don't see that in $p.", true,  container, 0,
			TO_CHAR);
		return;
	}

	if (!can_obj_to_inv (obj, ch, &error, count))
	{
		if (error == NO_CANT_TAKE)
		{
			ch->act("You can't take $p.", true,  obj, 0, TO_CHAR);
		}
		else if (error == NO_TOO_MANY)
		{
			ch->send_to_char("You can't handle so much.");
		}
		else if (error == NO_TOO_HEAVY)
		{
			ch->send_to_char("You can't carry so much weight.");
		}
		else if (error == NO_CANT_SEE)
		{
			ch->send_to_char("You don't see it.");
		}
		else if (error == NO_HANDS_FULL)
		{
			ch->send_to_char("Your hands are full!");
		}
		
		return;
	}

	if (container && container != obj)
		obj->in_obj = container;
	
	get (ch, obj, count);
}

void
do_take (CHAR_DATA * ch, char *argument, int cmd)
{
	int worn_object = 0;
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	char obj_name[MAX_STRING_LENGTH]= { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };

		//TODO Temporaily allow Guest to do things
	/***
	 if ((!ch->get_trust()) && IS_SET (ch->room->room_flags, OOC)
	 && str_cmp (ch->room->name, PREGAME_ROOM_NAME))
	 {
	 ch->send_to_char ("This command has been disabled in OOC zones.\n");
	 return;
	 }
	 ***/
	

	argument = one_argument (argument, obj_name);

	if (!*obj_name)
	{
		ch->send_to_char ("Take what?\n");
		return;
	}

	argument = one_argument (argument, buf);

	if (!str_cmp (buf, "from"))
		argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char ("Take from whom?\n");
		return;
	}

	if (!(victim = get_char_room_vis (ch, buf)))
	{
		ch->send_to_char ("You don't see them.\n");
		return;
	}

	if (victim == ch)
	{
		ch->send_to_char ("Why take from yourself?\n");
		return;
	}

	if (!(obj = get_obj_in_list_vis (ch, obj_name, victim->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, obj_name, victim->left_hand)))
	{

		if (!(obj = get_obj_in_list_vis (ch, obj_name, victim->equip)))
		{
			ch->act("You don't see that on $N.", true,  0, victim, TO_CHAR);
			return;
		}

		if (ch->get_trust() > 1)
		{
			unequip_char (victim, obj->location);
			obj_to_char (obj, victim);
		}
		else
			worn_object = 1;
	}

	if (victim->get_position() == SLEEP && !ch->get_trust())
	{

		victim->forced_wakeup();

		if (victim->get_position() != SLEEP)
		{

			ch->act("$N awakens as you touch $M.", true,  0, victim, TO_CHAR);
			ch->act("$n awakens $N as $e touches $M.",
				false,  0, victim, TO_NOTVICT);

			if (!get_affect (victim, MAGIC_AFFECT_PARALYSIS))
				return;
		}
	}

	if (!(ch->get_trust() ||
		victim->get_position() <= SLEEP ||
		get_affect (victim, MAGIC_AFFECT_PARALYSIS) ||
		victim->is_subduee()))
	{
		ch->act("$N prevents you from taking $p.", true,  obj, victim, TO_CHAR);
		victim->act("$N unsuccessfully tries to take $p from you.",
			true, obj, ch, TO_CHAR);
		victim->act("$n prevents $N from taking $p.",
			false, obj, ch, TO_NOTVICT);
		return;
	}

	if (worn_object)
	{

		strcpy (buf2, locations[obj->location]);
		*buf2 = tolower (*buf2);

		sprintf (buf, "You begin to remove $p from $N's %s.", buf2);
		ch->act(buf, true,  obj, victim, TO_CHAR | _ACT_FORMAT);

		sprintf (buf, "$n begins removing $p from $N's %s.", buf2);
		ch->act(buf, false,  obj, victim, TO_NOTVICT | _ACT_FORMAT);

		sprintf (buf, "$N begins removing $p from your %s.", buf2);
		victim->act(buf, true, obj, ch, TO_CHAR | _ACT_FORMAT);

		ch->delay_info1 = (long int) obj;
		ch->delay_info2 = obj->location;
		ch->delay_ch = victim;
		ch->delay_type = DEL_TAKE;
		ch->delay = 15;

		return;
	}

	obj_from_char (&obj, 0);
	obj_to_char (obj, ch);

	if (!ch->get_trust())
	{
		victim->act("$n takes $p from you.", true, obj, ch,
			TO_CHAR | _ACT_FORMAT);
		ch->act("$n takes $p from $N.", false,  obj, victim,
			TO_NOTVICT | _ACT_FORMAT);
	}

	clear_omote (obj);

	ch->act("You take $p from $N.", true,  obj, victim, TO_CHAR);

	if (obj->activation && IS_SET (obj->obj_flags.extra_flags, ITEM_GET_AFFECT))
		obj_activate (ch, obj);
}


void
drop_all (CHAR_DATA * ch)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buffer[MAX_STRING_LENGTH]= { '\0' };
	OBJ_DATA *tobj, *obj;

	if (ch->right_hand)
	{
		ch->act("You drop:", false,  0, 0, TO_CHAR);
		ch->act("$n drops:", false,  0, 0, TO_ROOM);
	}

	while (ch->right_hand || ch->left_hand)
	{

		if (ch->right_hand)
			obj = ch->right_hand;
		else
			obj = ch->left_hand;

		ch->act("   $p", false,  obj, 0, TO_CHAR);
		ch->act("   $p", false,  obj, 0, TO_ROOM);

		obj_from_char (&obj, 0);
		obj_to_room (obj, ch->in_room);

	}
}

void
do_drop (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = "";
	char buffer[MAX_STRING_LENGTH] = "";
	OBJ_DATA *obj, *tobj;
	ROOM_DATA *room;
	int count = 0, old_count = 1;
	std::string first_person, third_person;

	argument = one_argument (argument, buf);

		
	if (just_a_number (buf))
	{
		count = atoi (buf);
		argument = one_argument (argument, buf);
	}

	if (!*buf)
	{
		ch->send_to_char ("Drop what?\n");
		return;
	}

	if (!str_cmp (buf, "all"))
	{

		argument = one_argument (argument, buf);

		if (*buf)
		{
			ch->send_to_char ("You can only 'drop all'.\n");
			return;
		}

		drop_all (ch);

		return;
	}

	if (!(obj = get_obj_in_dark (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_dark (ch, buf, ch->left_hand)))
	{
		ch->send_to_char ("You do not have that item.\n");
		return;
	}

	if (count > obj->count)
		count = obj->count;

	sprintf (buffer, "%s", ch->char_short());
	*buffer = toupper(*buffer);
	old_count = obj->count;
	if (count)
		obj->count = count;

	first_person.assign("You drop #2");
	first_person.append(obj_short_desc(obj));
	first_person.append("#0");
	third_person.assign("#5");
	third_person.append(buffer);
	third_person.append("#0 drops #2");
	third_person.append(obj_short_desc(obj));
	third_person.append("#0");

	obj->count = old_count;

	if (evaluate_emote_string (ch, &first_person, third_person, argument))
	{
		obj_from_char (&obj, count);
		obj_to_room (obj, ch->in_room);
		if (obj->activation &&
			IS_SET (obj->obj_flags.extra_flags, ITEM_DROP_AFFECT))
			obj_activate (ch, obj);

		if ( !(first_person.empty()) )
		{
			sprintf (buffer, "%s %s", buf, first_person.c_str());
			do_omote (ch, buffer, 0);
		}
	}
	room = ch->room;
}

void
put_on_char (CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj,
			 int count, char *argument)
{
	int location;


	if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_NECK))
		location = WEAR_NECK_1;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_BODY))
		location = WEAR_BODY;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_HEAD))
		location = WEAR_HEAD;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_BACK))
		location = WEAR_BACK;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_LEGS))
		location = WEAR_LEGS;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_FEET))
		location = WEAR_FEET;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_HANDS))
		location = WEAR_HANDS;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_ARMS))
		location = WEAR_ARMS;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_ABOUT))
		location = WEAR_ABOUT;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_WAIST))
		location = WEAR_WAIST;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_WRIST))
	{
		if (get_equip (victim, WEAR_WRIST_L))
			location = WEAR_WRIST_R;
		else
			location = WEAR_WRIST_L;
	}
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_HAIR))
		location = WEAR_HAIR;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_FACE))
		location = WEAR_FACE;
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_ANKLE))
	{
		if (get_equip (victim, WEAR_ANKLE_L))
			location = WEAR_ANKLE_R;
		else
			location = WEAR_ANKLE_L;
	}
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_SHOULDER))
	{
		if (get_equip (victim, WEAR_SHOULDER_L))
			location = WEAR_SHOULDER_R;
		else
			location = WEAR_SHOULDER_L;
	}
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_ARMBAND))
	{
		if (get_equip (victim, WEAR_ARMBAND_R))
			location = WEAR_ARMBAND_L;
		else
			location = WEAR_ARMBAND_R;
	}
	else if (IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_BLINDFOLD))
		location = WEAR_BLINDFOLD;
	else
	{
		ch->act("You can't put $p on $M.", true,  obj, victim, TO_CHAR);
		return;
	}

	
	if (get_equip (victim, location))
	{
		ch->act("$N is already wearing $p.",
			false,  get_equip (victim, location), victim, TO_CHAR);
		return;
	}

	if (victim->get_position() == SLEEP && !ch->get_trust())
	{

		victim->forced_wakeup();

		if (victim->get_position() != SLEEP)
		{

			ch->act("$N awakens as you touch $M.", true,  0, victim, TO_CHAR);
			ch->act("$n awakens $N as $e touches $M.",
				false,  0, victim, TO_NOTVICT);

			if (!get_affect (victim, MAGIC_AFFECT_PARALYSIS))
				return;
		}
	}

	if (!(ch->get_trust() ||
		victim->get_position() <= SLEEP ||
		get_affect (victim, MAGIC_AFFECT_PARALYSIS)))
	{
		ch->act("$N stops you from putting $p on $M.",
			true,  obj, victim, TO_CHAR);
		victim->act("$N unsuccessfully tries to put $p from you.",
			true, obj, ch, TO_CHAR);
		victim->act("$n stops $N from putting $p on $M.",
			false, obj, ch, TO_NOTVICT);
		return;
	}

	ch->delay_type = DEL_PUTCHAR;
	ch->delay = 7;
	ch->delay_ch = victim;
	ch->delay_info1 = (long int) obj;
	ch->delay_info2 = location;

	ch->act("$n begins putting $p on $N.", false,  obj, victim,
		TO_NOTVICT | _ACT_FORMAT);
	ch->act("$n begins putting $p on you.", true,  obj, victim,
		TO_VICT | _ACT_FORMAT);
	ch->act("You begin putting $p on $N.", true,  obj, victim,
		TO_CHAR | _ACT_FORMAT);
}



void
do_put (CHAR_DATA * ch, char *argument, int cmd)
{
	char buffer[MAX_STRING_LENGTH] = "";
	char arg[MAX_STRING_LENGTH] = "";
	char *error;
	OBJ_DATA *obj;
	OBJ_DATA *tar;
	int count = 0, old_count = 0, put_light_on_table = 0;
	CHAR_DATA *victim;
	std::string first_person, third_person;

	argument = one_argument (argument, arg);

	if (just_a_number (arg))
	{
		count = atoi (arg);
		argument = one_argument (argument, arg);
	}

	if (!arg)
	{
		ch->send_to_char ("Put what?\n");
		return;
	}

	if (!(obj = get_obj_in_dark (ch, arg, ch->right_hand)) &&
		!(obj = get_obj_in_dark (ch, arg, ch->left_hand)))
	{
		sprintf (buffer, "You don't have a %s.\n", arg);
		ch->send_to_char (buffer);
		return;
	}

	argument = one_argument (argument, arg);

	if (!str_cmp (arg, "in") || !str_cmp (arg, "into"))
		argument = one_argument (argument, arg);

	else if (!str_cmp (arg, "on"))
	{

		argument = one_argument (argument, arg);

		if (!(victim = get_char_room_vis (ch, arg)))
		{
			ch->act("Put $p on whom?", true,  obj, 0, TO_CHAR);
			return;
		}

		put_on_char (ch, victim, obj, count, argument);

		return;
	}

	if (!*arg)
	{
		ch->act("Put $o into what?", false,  obj, 0, TO_CHAR);
		return;
	}

	if (!(tar = get_obj_in_dark (ch, arg, ch->right_hand)) &&
		!(tar = get_obj_in_dark (ch, arg, ch->left_hand)) &&
		!(tar = get_obj_in_dark (ch, arg, ch->equip)) &&
		!(tar = get_obj_in_list_vis (ch, arg, ch->room->contents)))
	{

		if ((victim = get_char_room_vis (ch, arg)))
		{
			put_on_char (ch, victim, obj, count, argument);
			return;
		}

		sprintf (buffer, "You don't see a %s.\n", arg);
		ch->send_to_char (buffer);
		return;
	}

	
	// mod by Methuselah for table_lamp

	if (obj->obj_flags.type_flag == ITEM_LIGHT
		&& obj->o.light.on == true
		&& !IS_SET (obj->obj_flags.extra_flags, ITEM_MAGIC))
	{
		put_light_on_table++;
	}

	if (!can_obj_to_container (obj, tar, &error, count))
	{
		ch->send_to_char (error);
		return;
	}

	sprintf (buffer, "%s", ch->char_short());
	*buffer = toupper(*buffer);
	old_count = obj->count;
	if (count)
		obj->count = count;
	
	
	first_person.assign("You put #2");
	first_person.append(obj_short_desc(obj));
	third_person.assign("#5");
	third_person.append(buffer);
	third_person.append("#0 puts #2");
	third_person.append(obj_short_desc(obj));

	if (IS_SET (tar->obj_flags.extra_flags, ITEM_TABLE))
	{
		first_person.append("#0 on #2");
		third_person.append("#0 on #2");
	}
	else
	{
		first_person.append("#0 into #2");
		third_person.append("#0 into #2");
	}

	first_person.append(obj_short_desc(tar));
	first_person.append("#0");
	third_person.append(obj_short_desc(tar));
	third_person.append("#0");

	obj->count = old_count;
	
	
	if (evaluate_emote_string (ch, &first_person, third_person, argument))
	{
		obj_from_char (&obj, count);
		obj_to_obj (obj, tar);
	}
	
	if (put_light_on_table)
		room_light(ch->room);

	return;
}


void
do_give (CHAR_DATA * ch, char *argument, int cmd)
{
	char obj_name[MAX_INPUT_LENGTH];
	char vict_name[MAX_INPUT_LENGTH];
	CHAR_DATA *vict;
	OBJ_DATA *obj;
	int count = 0, error;

	argument = one_argument (argument, obj_name);

		//TODO Temporaily allow Guest to do things
	/***
	 if ((!ch->get_trust()) && IS_SET (ch->room->room_flags, OOC)
	 && str_cmp (ch->room->name, PREGAME_ROOM_NAME))
	 {
	 ch->send_to_char ("This command has been disabled in OOC zones.\n");
	 return;
	 }
	 ***/
	
	if (just_a_number (obj_name))
	{
		count = atoi (obj_name);
		argument = one_argument (argument, obj_name);
	}

	
	if (!*obj_name)
	{
		ch->send_to_char ("Give what?\n");
		return;
	}

	if (!(obj = get_obj_in_dark (ch, obj_name, ch->right_hand)) &&
		!(obj = get_obj_in_dark (ch, obj_name, ch->left_hand)))
	{
		ch->send_to_char ("You do not seem to have anything like that.\n");
		return;
	}

	argument = one_argument (argument, vict_name);

	if (!str_cmp (vict_name, "to"))
		argument = one_argument (argument, vict_name);

	if (!*vict_name)
	{

		ch->send_to_char ("Give to whom?\n");
		return;
	}

	if (!(vict = get_char_room_vis (ch, vict_name)))
	{
		ch->send_to_char ("No one by that name around here.\n");
		return;
	}

	if (vict == ch)
	{
		ch->send_to_char ("Give it to yourself? How generous...\n");
		return;
	}

	
	if (!can_obj_to_inv (obj, vict, &error, count))
	{
		if (error == NO_HANDS_FULL)
		{
			ch->act("$N's hands are currently occupied, I'm afraid.", true, 
				obj, vict, TO_CHAR | _ACT_FORMAT);
			ch->act("$n just tried to give you $o, but your hands are full.", true,
				obj, vict, TO_VICT | _ACT_FORMAT);
			return;
		}
		else if (error == NO_TOO_HEAVY)
		{
			ch->act
				("$N struggles beneath the weight of the object, and so you take it back.",
				true,  obj, vict, TO_CHAR | _ACT_FORMAT);
			ch->act
				("$n just tried to give you $o, but it is too heavy for you to carry.",
				true,  obj, vict, TO_VICT | _ACT_FORMAT);
			return;
		}
		else if (error == NO_CANT_TAKE)
		{
			ch->act("This item cannot be given.", false,  0, 0, TO_CHAR);
			return;
		}
	}


	if ((obj_mass (obj) + vict->carrying()) > vict->can_carry_weight())
	{
		ch->act("$E can't carry that much weight.", false, 0, vict, TO_CHAR);
		return;
	}

	obj_from_char (&obj, count);

	ch->act("$n gives $p to $N.", true, obj, vict, TO_NOTVICT | _ACT_FORMAT);
	ch->act("$n gives you $p.", false, obj, vict, TO_VICT | _ACT_FORMAT);
	ch->act("You give $p to $N.", true, obj, vict, TO_CHAR | _ACT_FORMAT);

	obj_to_char (obj, vict);


}

/**
 // What object do they have in the given location
 */
OBJ_DATA *
get_equip (CHAR_DATA * ch, int location)
{
	OBJ_DATA *obj;


	if (ch->right_hand && ch->right_hand->location == location)
		return ch->right_hand;

	if (ch->left_hand && ch->left_hand->location == location)
		return ch->left_hand;

	for (obj = ch->equip; obj; obj = obj->next_content)
	{
		if (obj->location == location)
		{
			return obj;
		}
	}

	return NULL;
}

void
do_drink (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *container;
	OBJ_DATA *drink;
	char buf[MAX_STRING_LENGTH] = "";
	std::string first_person, third_person;
	int sips = 1, range = 0;
	const char *verbose_liquid_amount [5] = {
		"", 
		"some of the ",
		"a lot of the ",
		"most of the ",
		"all of the "};

	argument = one_argument (argument, buf);

	if (!(container = get_obj_in_dark (ch, buf, ch->right_hand)) &&
		!(container = get_obj_in_list_vis (ch, buf, ch->left_hand)) &&
		!(container = get_obj_in_list_vis (ch, buf, ch->room->contents)))
	{
		ch->act("You can't find it.", false,  0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (container->obj_flags.type_flag != ITEM_DRINKCON &&
		container->obj_flags.type_flag != ITEM_FOUNTAIN)
	{
		ch->act("You cannot drink from $p.", false,  container, 0, TO_CHAR);
		return;
	}

	if (container->o.drinkcon.volume == 0)
	{
		ch->act("$p is empty.", false,  container, 0, TO_CHAR);
		return;
	}

	if (!(drink = vtoo (container->o.drinkcon.liquid)))
	{
		ch->act("$p appears empty.", false,  container, 0, TO_CHAR);
		return;
	}

	if (*argument != '(' && *argument)
	{
		argument = one_argument (argument, buf);
		if (just_a_number(buf))
		{
			if (container->o.drinkcon.volume < atoi(buf) && container->o.drinkcon.volume != -1)
			{
				ch->send_to_char ("There simply isn't that much to drink!\n");
				return;
			}

			if (atoi(buf) < 1)
			{
				ch->send_to_char ("As amusing as regurgitation can be, you may not drink negative amounts.\n");
			}

			sips = atoi(buf);
		}
		else
		{
			ch->send_to_char ("The correct syntax is #3drink <container> [<amount>] [(emote)]#0.\n");
			return;
		}
	}
	if (sips > 1)
		range = 1;
	if (sips > (container->o.od.value[0] / 3))
		range = 2;
	if (sips > (container->o.od.value[0] * 2 / 3))
		range = 3;
	if (sips == container->o.od.value[0])
		range = 4;
	if (container->o.od.value[1] == -1 && sips > 1)
		range = 1;
	if (container->o.od.value[1] == -1 && sips == 1)
		range = 0;

	sprintf (buf, "%s", ch->char_short());
	*buf = toupper(*buf);
	first_person.assign("You drink ");
	first_person.append(verbose_liquid_amount[range]);
	first_person.append("#2");
	first_person.append(fname(drink->name));
	first_person.append("#0 from #2");
	first_person.append(obj_short_desc(container));
	first_person.append("#0");
	
	third_person.assign("#5");
	third_person.append(buf);
	third_person.append("#0 drinks ");
	third_person.append(verbose_liquid_amount[range]);
	third_person.append("#2");
	third_person.append(fname(drink->name));
	third_person.append("#0 from #2");
	third_person.append(obj_short_desc(container));
	third_person.append("#0");


	if (evaluate_emote_string(ch, &first_person, third_person, argument))
	{
		if (ch->intoxication != -1)
		{
			if (number (6, 20) > ((ch->con + ch->wil) / 2))
				ch->intoxication += (drink->o.fluid.alcohol * sips);
			else
				ch->intoxication += ((drink->o.fluid.alcohol * sips) / 2);
		}

		if (ch->thirst != -1 && ch->thirst != MAX_WATER)
			ch->thirst += (drink->o.fluid.water * sips);

		if (ch->hunger != -1 && ch->hunger != MAX_CALORIE)
			ch->hunger += (drink->o.fluid.food * sips);

		if (ch->thirst < 0 && (!ch->get_trust()))
			ch->thirst = 0;
		if (ch->hunger < 0 && (!ch->get_trust()))
			ch->hunger = 0;
		//if (ch->fatigue < 0 && (!ch->get_trust()))
		//  ch->fatigue = 0;
		if (ch->thirst > MAX_WATER)
			ch->thirst = MAX_WATER;
		if (ch->hunger > MAX_CALORIE)
			ch->hunger = MAX_CALORIE;

		if (ch->thirst == MAX_WATER && drink->o.fluid.water)
			ch->send_to_char("You are completely satiated.\n");

		if (ch->hunger == MAX_CALORIE && drink->o.fluid.food)
			ch->send_to_char("You are absolutely stuffed.\n");

		
		if (container->o.drinkcon.volume != -1)
			container->o.drinkcon.volume -= sips;

		if (!container->o.drinkcon.volume)
		{
			container->o.drinkcon.liquid = 0;
		}

	}
}

void
do_eat (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	std::string first_person, third_person;
	int bites = 1, range = 0, old_count = 1;
	const char *verbose_bites_amount [6] = {
		"",
		"a bite of ",
		"a few bites of ",
		"a lot of ",
		"most of ",
		"all of "};

	argument = one_argument (argument, buf);

	if (!(obj = get_obj_in_dark (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_dark (ch, buf, ch->left_hand)) &&
		!(obj = get_obj_in_dark (ch, buf, ch->equip)))
	{
		ch->send_to_char ("You can't find it.\n");
		return;
	}

	if (obj->obj_flags.type_flag != ITEM_FOOD && (!ch->get_trust()))
	{
		ch->send_to_char ("That isn't food.  You can't eat it.\n");
		return;
	}

	if (obj->equiped_by && obj->obj_flags.type_flag != ITEM_FOOD)
	{
		ch->send_to_char ("You must remove that item before destroying it.\n");
		return;
	}

		
	if (ch->hunger == MAX_CALORIE)
	{
		ch->act("You are already absolutely stuffed!", false,  0, 0, TO_CHAR);
	}

	if (*argument != '(' && *argument)
	{
		argument = one_argument (argument, buf);
		if (just_a_number(buf))
		{
			if (obj->o.food.bites < atoi(buf) && obj->o.food.bites != -1)
			{
				ch->send_to_char ("There simply isn't that much to eat!\n");
				return;
			}

			if (atoi(buf) < 1)
			{
				ch->send_to_char ("As amusing as regurgitation can be, you may not eat negative amounts.\n");
				return;
			}

			bites = atoi(buf);
		}
		else if (!strcmp (buf, "all"))
		{
			bites = MAX (1, obj->o.food.bites);
		}
		else
		{
			ch->send_to_char ("The correct syntax is #3eat <food> [<amount>] [(emote)]#0.\n");
			return;
		}

	}

	if (obj->o.food.bites > 1)
		range = 1;
	if (bites > 1)
		range = 2;
	if (bites > (obj->o.food.bites / 3) && bites != 1 && obj->o.food.bites > 4)
		range = 3;
	if (bites > (obj->o.food.bites * 2 / 3) && bites != 1)
		range = 4;
	if (bites == obj->o.food.bites && bites != 1)
		range = 5;
	if (obj->o.food.bites == -1)
		range = 1;

	sprintf(buf, "%s", ch->char_short());
	*buf = toupper(*buf);

	old_count = obj->count;
	obj->count = 1;

	first_person.assign("You eat ");
	first_person.append(verbose_bites_amount[range]);
	first_person.append("#2");
	first_person.append(obj_short_desc(obj));
	first_person.append("#0");
	third_person.assign("#5");
	third_person.append(buf);
	third_person.append("#0 eats ");
	third_person.append(verbose_bites_amount[range]);
	third_person.append("#2");
	third_person.append(obj_short_desc(obj));
	third_person.append("#0");

	obj->count = old_count;

	if (evaluate_emote_string (ch, &first_person, third_person, argument))
	{
		if (obj->equiped_by)
			unequip_char (ch, obj->location);

		
		for (int i = 0; i < bites; i++)
		{
			if (ch->hunger < MAX_CALORIE)
				ch->hunger += get_bite_value (obj);
			
			if (ch->hunger > MAX_CALORIE)
				ch->act("You are full.", false,  0, 0, TO_CHAR);
			if (ch->hunger > MAX_CALORIE)
				ch->hunger = MAX_CALORIE;
			
			if (ch->intoxication != -1)
			{
				if (number (6, 20) > ((ch->con + ch->wil) / 2))
					ch->intoxication += (obj->o.food.alcohol_value * bites);
				else
					ch->intoxication += ((obj->o.food.alcohol_value * bites) / 2);
			}
			
			if (ch->thirst != -1 && ch->thirst != MAX_WATER)
				ch->thirst += (obj->o.food.water_value * bites);
			
			obj->o.food.bites--;
			
			if (obj->count > 1 && obj->o.food.bites < 1)
			{
				obj->count--;
				obj->o.food.bites = vtoo (obj->nVirtual)->o.food.bites;
			}
			else if (obj->o.food.bites < 1 && obj->count <= 1)
				extract_obj (obj);
			else if (ch->get_trust())
			{
					//immortals can destroy anything completely by eating it
					//option to using junk or purge
					//since it gives no echoes when the object is extracted
				extract_obj (obj);
				return;
			}
		}


	}
}

void
do_fill (CHAR_DATA * ch, char *argument, int cmd)
{
	int volume_to_transfer;
	OBJ_DATA *from;
	OBJ_DATA *to;
	OBJ_DATA *fuel;
	OBJ_DATA *tmp_obj;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char ("FILL <object> from/with <object>\n");
		ch->send_to_char ("Example:  fill bucket from well\n");
		ch->send_to_char ("          fill bucket well      (same thing)\n");
		return;
	}

	if (!(to = get_obj_in_dark (ch, buf, ch->right_hand)) &&
		!(to = get_obj_in_list_vis (ch, buf, ch->left_hand)) &&
		!(to = get_obj_in_list_vis (ch, buf, ch->room->contents)))
	{

		if (get_obj_in_dark (ch, buf, ch->room->contents))
		{
			ch->send_to_char ("It is too dark for that.\n");
			return;
		}

		ch->send_to_char ("Fill what?\n");

		return;
	}

	if (to->obj_flags.type_flag != ITEM_DRINKCON
		&& to->obj_flags.type_flag != ITEM_LIGHT
		&& to->obj_flags.type_flag != ITEM_DRYCON
		&& to->obj_flags.type_flag != ITEM_FUEL)
	{
		ch->act("You can't fill $p.", false,  to, 0, TO_CHAR);
		return;
	}

	if (to->obj_flags.type_flag == ITEM_LIGHT &&
		is_name_in_list ("candle", to->name))
	{
		ch->act("You can't do that with $p.", false,  to, 0, TO_CHAR);
		return;
	}

	/**
	 capacity - od.value[0]
	 volume/hours - od.value[1]
	 contents/liquid - od.value[2]
	 **/
	if (to->o.od.value[1] == -1 ||
		(to->o.od.value[1] >= to->o.od.value[0]))
	{
		ch->act("$p is full already.", false,  to, 0, TO_CHAR);
		return;
	}

	argument = one_argument (argument, buf);

	if (!str_cmp (buf, "with") || !str_cmp (buf, "from"))
		argument = one_argument (argument, buf);

	if (!*buf)
	{

		for (from = ch->room->contents; from; from = from->next_content)
			if (from->obj_flags.type_flag == ITEM_FOUNTAIN && can_see_obj(ch, from))
				break;

		if (!from)
		{
			ch->act("Fill $p from what?", false,  to, 0, TO_CHAR);
			return;
		}
	}

	else if (!(from = get_obj_in_dark (ch, buf, ch->room->contents)))
	{
		ch->act("Fill $p from what?", false,  to, 0, TO_CHAR);
		return;
	}

	if ((from->obj_flags.type_flag != ITEM_FOUNTAIN
		  && from->obj_flags.type_flag != ITEM_DRINKCON
		  && from->obj_flags.type_flag != ITEM_FUEL)
		&& (to->obj_flags.type_flag == ITEM_DRINKCON
			|| to->obj_flags.type_flag == ITEM_LIGHT))
	{
		ch->act("There is no way to fill $p from $P.",
			false,  to, from, TO_CHAR);
		return;
	}

	if ((to->obj_flags.type_flag == ITEM_DRYCON)
		&& (from->obj_flags.type_flag == ITEM_FOUNTAIN 
			|| from->obj_flags.type_flag == ITEM_DRINKCON))
	{
		ch->act("There is no way to fill $p from $P.",
			 false,  to, from, TO_CHAR);
		return;
	}

	
	if (!from->o.od.value[1])
	{
		ch->act("$p is empty.", false,  from, 0, TO_CHAR);
		return;
	}

	if ((to->obj_flags.type_flag == ITEM_LIGHT)
		&& (to->o.light.fuel != from->o.od.value[2]))
	{

		if (!(fuel = vtoo (to->o.light.fuel)))
		{
			ch->act("$p is broken.", false,  to, 0, TO_CHAR);
			return;
		}

		ch->act("$p only burns $O.", false,  to, fuel, TO_CHAR);

		if ((fuel = vtoo (from->o.od.value[2])))
			ch->act("$p contains $O.", false,  from, fuel, TO_CHAR);

		return;
	}

	if (to->obj_flags.type_flag == ITEM_DRINKCON
		&& to->o.drinkcon.liquid 
		&& to->o.drinkcon.volume
		&& (to->o.drinkcon.liquid != from->o.drinkcon.liquid))
	{
		ch->send_to_char ("You shouldn't mix fluids.\n");
		return;
	}

	else if (to->obj_flags.type_flag == ITEM_DRYCON
			 && to->o.drycon.contents
			 && to->o.drycon.volume
			 && to->o.drycon.contents != from->o.drycon.contents)
	{
		ch->send_to_char ("You shouldn't mix dry items.\n");
		return;
	}
	
	else if (to->obj_flags.type_flag == ITEM_FUEL
			 && to->o.fuelcon.capacity
			 && to->o.fuelcon.volume
			 && to->o.fuelcon.fuel != from->o.fuelcon.fuel)
	{
		ch->send_to_char ("You shouldn't mix fuels.\n");
		return;
	}
	
	sprintf (buf, "You fill $p from $P with %s.",
		vnum_to_name (from->o.od.value[2]));
	ch->act(buf, false,  to, from, TO_CHAR | _ACT_FORMAT);

	sprintf (buf, "$n fills $p from $P with %s.",
		vnum_to_name (from->o.od.value[2]));
	ch->act(buf, true,  to, from, TO_ROOM | _ACT_FORMAT);

	/**
	 capacity - od.value[0]
	 volume - od.value[1]
	 contents/liquid - od.value[2]
	 **/
	
	if ((to->obj_flags.type_flag == ITEM_DRINKCON) && !to->o.od.value[1])
	{
		to->o.od.value[2] = from->o.od.value[2];
		
		volume_to_transfer = from->o.od.value[1];
		
		if (from->o.od.value[1] == -1)
			volume_to_transfer = to->o.od.value[0];
		
		if (volume_to_transfer > (to->o.od.value[0] - to->o.od.value[1]))
			volume_to_transfer = to->o.od.value[0] - to->o.od.value[1];
		
		if (from->o.od.value[1] != -1)
			from->o.od.value[1] -= volume_to_transfer;
		
		if (to->o.od.value[1] != -1)
			to->o.od.value[1] += volume_to_transfer;
	}

	if ((to->obj_flags.type_flag == ITEM_DRYCON)
		&& !to->o.drycon.volume)
	{
		
		to->o.drycon.contents = from->o.drycon.contents;
		
		tmp_obj = vtoo(to->o.drycon.contents);
		
		to->o.drycon.capacity = (int)(to->o.drycon.max_weight / tmp_obj->obj_flags.weight);
		
		volume_to_transfer = from->o.drycon.volume;
		
		if (from->o.drycon.volume == -1)
			volume_to_transfer = to->o.drycon.capacity;
		
		if (volume_to_transfer > (to->o.drycon.capacity - to->o.drycon.volume))
			volume_to_transfer = to->o.drycon.capacity - to->o.drycon.volume;
		
		if (from->o.drycon.volume != -1)
			from->o.drycon.volume -= volume_to_transfer;
		
		if (from->o.drycon.volume == 0)
		{
			from->o.drycon.capacity = 1;
			from->o.drycon.contents = 0;
		}
			
		if (to->o.drycon.volume != -1)
			to->o.drycon.volume += volume_to_transfer;
	}
	
	if (from->obj_flags.type_flag == ITEM_LIGHT &&
		!from->o.light.hours && from->o.light.on)
	{
		ch->act("$p is extinguished.", false,  from, 0, TO_CHAR);
		ch->act("$p is extinguished.", false,  from, 0, TO_ROOM);
		from->o.light.on = 0;
	}
	
		//filling the light item with wood or oil
	if (to->obj_flags.type_flag == ITEM_LIGHT)
	{
		if (from->count > 1)
			volume_to_transfer = (from->o.fuelcon.volume * from->count);
		else 
			volume_to_transfer = from->o.fuelcon.volume;
		
		if (from->o.fuelcon.volume == -1)
			volume_to_transfer = to->o.light.capacity;
		
		if (volume_to_transfer > (to->o.light.capacity - to->o.light.hours))
			volume_to_transfer = to->o.light.capacity - to->o.light.hours;
		
		if (from->count > 1) 
		{
			from->count = from->count - (volume_to_transfer/from->o.fuelcon.volume);
		}
		else if (from->o.fuelcon.volume != -1)
		{
			from->o.fuelcon.volume -= volume_to_transfer;
		}
		
		if (to->o.light.hours != -1)
			to->o.light.hours += volume_to_transfer;
		
		if ((from->o.fuelcon.volume == 0)
			&& (from->o.fuelcon.refill == 0))
		{
			extract_obj(from);
		}
	}
	
		//filling a wood box with wood
	if (to->obj_flags.type_flag == ITEM_FUEL)
	{
		if (from->count > 1)
			volume_to_transfer = (from->o.fuelcon.volume * from->count);
		else 
			volume_to_transfer = from->o.fuelcon.volume;
		
		if (from->o.fuelcon.volume == -1)
			volume_to_transfer = to->o.fuelcon.capacity;
		
		if (volume_to_transfer > (to->o.fuelcon.capacity - to->o.fuelcon.volume))
			volume_to_transfer = to->o.fuelcon.capacity - to->o.fuelcon.volume;
		
		if (from->count > 1) 
		{
			from->count = from->count - (volume_to_transfer/from->o.fuelcon.volume);
		}
		else if (from->o.fuelcon.volume != -1)
		{
			from->o.fuelcon.volume -= volume_to_transfer;
		}
		
		if (to->o.fuelcon.volume != -1)
			to->o.fuelcon.volume += volume_to_transfer;
		
		if ((from->o.fuelcon.volume == 0)
			&& (from->o.fuelcon.refill == 0))
		{
			extract_obj(from);
		}
		
	}
	
}

void
do_pour (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *from;
	OBJ_DATA *to;
	int volume_to_transfer;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char ("What do you want to pour?\n");
		return;
	}

	if (!(from = get_obj_in_dark (ch, buf, ch->right_hand)) &&
		!(from = get_obj_in_dark (ch, buf, ch->left_hand)))
	{
		ch->send_to_char ("You can't find it.\n");
		return;
	}

	if (from->obj_flags.type_flag != ITEM_DRINKCON
		&& from->obj_flags.type_flag != ITEM_LIGHT
		&& from->obj_flags.type_flag != ITEM_DRYCON)
	{
		ch->act("You can't pour from $p.\n", false,  from, 0, TO_CHAR);
		return;
	}

	if (from->obj_flags.type_flag == ITEM_LIGHT &&
		is_name_in_list ("candle", from->name))
	{
		ch->send_to_char ("You can't pour wax from a candle (yet).\n");
		return;
	}

	if (from->obj_flags.type_flag == ITEM_LIGHT && !from->o.light.hours)
	{
		ch->act("$p contains no fuel.", false,  from, 0, TO_CHAR);
		return;
	}

	/**
	 capacity - od.value[0]
	 volume - od.value[1]
	 contents/liquid - od.value[2]
	 **/
	if ((from->obj_flags.type_flag == ITEM_DRINKCON || from->obj_flags.type_flag == ITEM_DRYCON)
		&& !from->o.od.value[1])
	{
		ch->act("$p is empty.", false,  from, 0, TO_CHAR);
		return;
	}

	argument = one_argument (argument, buf2);

	if (!str_cmp (buf2, "out"))
	{
		do_empty (ch, buf, 0);
		return;
	}

	if (!*buf2)
	{
		ch->send_to_char ("What do you want to pour it into?");
		return;
	}

	if (!(to = get_obj_in_dark (ch, buf2, ch->right_hand)) &&
		!(to = get_obj_in_list_vis (ch, buf2, ch->left_hand)) &&
		!(to = get_obj_in_list_vis (ch, buf2, ch->room->contents)))
	{
		ch->act("You can't find it to pour $p into.", false,  from, 0,
			TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (from->coldload_id == to->coldload_id)
	{
		ch->send_to_char("You can't pour things into themselves!");
		return;
	}
		
	if (to->obj_flags.type_flag != ITEM_DRINKCON
		&& to->obj_flags.type_flag != ITEM_LIGHT
		&& to->obj_flags.type_flag != ITEM_DRYCON)
	{
		ch->act("You can't pour into $p.", false,  to, 0, TO_CHAR);
		return;
	}

	if (to->obj_flags.type_flag == ITEM_LIGHT &&
		!to->o.light.hours && to->o.light.on)
		to->o.light.on = 0;

	if (to->obj_flags.type_flag == ITEM_LIGHT)
	{

		if (is_name_in_list ("candle", to->name))
		{
			ch->send_to_char ("You can't pour it into a candle.");
			return;
		}

		if (from->o.light.fuel != to->o.od.value[2])
		{
			sprintf (buf, "$p only burns %s.",
				vnum_to_name (to->o.light.fuel));
			ch->act(buf, false,  to, 0, TO_CHAR);
			return;
		}
	}

	if (to->o.od.value[1] &&
		from->o.od.value[2] != to->o.od.value[2])
	{
		ch->act("If you want to fill $p, you must empty it first.",
			false,  to, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (to->o.od.value[0] == to->o.od.value[1])
	{
		ch->act("$p is already full.", false,  to, 0, TO_CHAR);
		return;
	}

	sprintf (buf, "You pour %s from $p into $P.",
		vnum_to_name (from->o.od.value[2]));
	ch->act(buf, false,  from, to, TO_CHAR | _ACT_FORMAT);

	sprintf (buf, "$n pours %s from $p into $P.",
		vnum_to_name (from->o.od.value[2]));
	ch->act(buf, true,  from, to, TO_ROOM | _ACT_FORMAT);

	if ((to->obj_flags.type_flag == ITEM_DRINKCON || to->obj_flags.type_flag == ITEM_DRYCON)
		&& !to->o.od.value[1])
		to->o.od.value[2] = from->o.od.value[2];

	volume_to_transfer = from->o.od.value[1];
	if (volume_to_transfer > (to->o.od.value[0] - to->o.od.value[1]))
		volume_to_transfer = to->o.od.value[0] - to->o.od.value[1];

	from->o.od.value[1] -= volume_to_transfer;

	to->o.od.value[1] += volume_to_transfer;

	
	if (from->obj_flags.type_flag == ITEM_LIGHT &&
		!from->o.light.hours && from->o.light.on)
	{
		ch->act("$p is extinguished.", false,  from, 0, TO_CHAR);
		ch->act("$p is extinguished.", false,  from, 0, TO_ROOM);
		from->o.light.on = 0;
	}
}

/* functions related to wear */

void
perform_wear (CHAR_DATA * ch, OBJ_DATA * obj, int keyword)
{
	switch (keyword)
	{

	case KEYWEAR_LIGHT:

		if (obj->o.light.hours < 1)
		{
			ch->act("You hold $p and realize it is spent.",
				true,  obj, 0, TO_CHAR);
		}

		else
		{
			if (!obj->o.light.on)
			{
				ch->act("You light $p and hold it.", false,  obj, 0, TO_CHAR);
				ch->act("$n lights $p and holds it.", false,  obj, 0, TO_ROOM);
			}
			else
			{
				ch->act("You hold $p.", false,  obj, 0, TO_CHAR);
				ch->act("$n holds $p.", false,  obj, 0, TO_ROOM);
			}
		}
		break;

	case KEYWEAR_FINGER:
		ch->act("$n wears $p on $s finger.", true,  obj, 0,
			TO_ROOM | _ACT_FORMAT);
		break;
	case KEYWEAR_NECK:
		ch->act("$n wears $p around $s neck.", true,  obj, 0,
			TO_ROOM | _ACT_FORMAT);
		ch->act("You wear $p around your neck.", true,  obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case KEYWEAR_BODY:
		ch->act("$n wears $p on $s body.", true,  obj, 0,
			TO_ROOM | _ACT_FORMAT);
		ch->act("You wear $p on your body.", true,  obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case KEYWEAR_HEAD:
		ch->act("$n wears $p on $s head.", true,  obj, 0,
			TO_ROOM | _ACT_FORMAT);
		ch->act("You wear $p on your head.", true,  obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case KEYWEAR_LEGS:
		ch->act("$n wears $p on $s legs.", true,  obj, 0,
			TO_ROOM | _ACT_FORMAT);
		ch->act("You wear $p on your legs.", true,  obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case KEYWEAR_FEET:
		ch->act("$n wears $p on $s feet.", true,  obj, 0,
			TO_ROOM | _ACT_FORMAT);
		ch->act("You wear $p on your feet.", true,  obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case KEYWEAR_HANDS:
		ch->act("$n wears $p on $s hands.", true,  obj, 0,
			TO_ROOM | _ACT_FORMAT);
		ch->act("You wear $p on your hands.", true,  obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case KEYWEAR_ARMS:
		ch->act("$n wears $p on $s arms.", true,  obj, 0,
			TO_ROOM | _ACT_FORMAT);
		ch->act("You wear $p on your arms.", true,  obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case KEYWEAR_ABOUT:
		ch->act("$n wears $p about $s body.", true,  obj, 0,
			TO_ROOM | _ACT_FORMAT);
		ch->act("You wear $p about your body.", true,  obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case KEYWEAR_WAIST:
		ch->act("$n wears $p about $s waist.", true,  obj, 0,
			TO_ROOM | _ACT_FORMAT);
		ch->act("You wear $p about your waist.", true,  obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case KEYWEAR_WRIST:
		ch->act("$n wears $p around $s wrist.", true,  obj, 0,
			TO_ROOM | _ACT_FORMAT);
		break;
	case KEYWEAR_BELT:
		ch->act("$n affixes $p to $s belt.", true,  obj, 0,
			TO_ROOM | _ACT_FORMAT);
		ch->act("You affix $p to your belt.", true,  obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case KEYWEAR_BACK:
		ch->act("$n stores $p across $s back.", true,  obj, 0,
			TO_ROOM | _ACT_FORMAT);
		ch->act("You store $p across your back.", true,  obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case KEYWEAR_THROAT:
		ch->act("$n wears $p around $s neck.", true,  obj, 0,
			TO_ROOM | _ACT_FORMAT);
		ch->act("You wear $p around your neck.", false,  obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case KEYWEAR_EARS:
		ch->act("$n wears $p on $s ears.", true,  obj, 0,
			TO_ROOM | _ACT_FORMAT);
		ch->act("You wear $p on your ears.", false,  obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case KEYWEAR_SHOULDER:
		ch->act("$n slings $p over $s shoulder.", true,  obj, 0,
			TO_ROOM | _ACT_FORMAT);
		break;
	case KEYWEAR_ANKLE:
		ch->act("$n wears $p around $s ankle.", true,  obj, 0,
			TO_ROOM | _ACT_FORMAT);
		break;
	case KEYWEAR_HAIR:
		ch->act("$n wears $p in $s hair.", true,  obj, 0,
			TO_ROOM | _ACT_FORMAT);
		ch->act("You wear $p in your hair.", true,  obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case KEYWEAR_FACE:
		ch->act("$n wears $p on $s face.", true,  obj, 0,
			TO_ROOM | _ACT_FORMAT);
		ch->act("You wear $p on your face.", true,  obj, 0,
			TO_CHAR | _ACT_FORMAT);
		break;
	case KEYWEAR_ARMBAND:
		ch->act("$n wears $p around $s upper arm.", true,  obj, 0,
			TO_ROOM | _ACT_FORMAT);
		/* TO_CHAR act occurs in wear() */
		break;
	}
}

void
wear (CHAR_DATA * ch, OBJ_DATA * obj_object, int keyword)
{
	char buffer[MAX_STRING_LENGTH]= { '\0' };

	
	if (obj_object->obj_flags.type_flag == ITEM_LIGHT
		&& obj_object->o.light.on == true
		&& !IS_SET (obj_object->obj_flags.extra_flags, ITEM_MAGIC))
	{
		ch->send_to_char ("You'll need to snuff that, first.\n");
		return;
	}

		
	switch (keyword)
	
	{
		case KEYWEAR_FINGER:
			if (CAN_WEAR (obj_object, ITEM_WEAR_FINGER))
			{
				
				if (get_equip (ch, WEAR_FINGER_L) && get_equip (ch, WEAR_FINGER_R))
					ch->send_to_char ("You are already wearing something on your "
								  "ring fingers.\n");
				
				else
				{
					
					perform_wear (ch, obj_object, keyword);
					
					if (get_equip (ch, WEAR_FINGER_L))
					{
						sprintf (buffer,
								 "You slip the %s on your right ring finger.\n",
								 fname (obj_object->name));
						ch->send_to_char (buffer);
						if (obj_object == ch->right_hand)
							ch->right_hand = NULL;
						else if (obj_object == ch->left_hand)
							ch->left_hand = NULL;
						equip_char (ch, obj_object, WEAR_FINGER_R);
					}
					
					else
					{
						sprintf (buffer,
								 "You slip the %s on your left ring finger.\n",
								 fname (obj_object->name));
						ch->send_to_char (buffer);
						if (obj_object == ch->right_hand)
							ch->right_hand = NULL;
						else if (obj_object == ch->left_hand)
							ch->left_hand = NULL;
						equip_char (ch, obj_object, WEAR_FINGER_L);
					}
				}
			}
			else
				ch->send_to_char ("You can't wear that on your ring finger.\n");
			break;
			
		case KEYWEAR_NECK:
			if (CAN_WEAR (obj_object, ITEM_WEAR_NECK))
			{
				if (get_equip (ch, WEAR_NECK_1) || get_equip (ch, WEAR_NECK_2))
					ch->send_to_char ("You can only wear one thing around your "
								  "neck.\n");
				else
				{
					perform_wear (ch, obj_object, keyword);
					if (get_equip (ch, WEAR_NECK_1))
					{
						if (obj_object == ch->right_hand)
							ch->right_hand = NULL;
						else if (obj_object == ch->left_hand)
							ch->left_hand = NULL;
						equip_char (ch, obj_object, WEAR_NECK_2);
					}
					else
					{
						if (obj_object == ch->right_hand)
							ch->right_hand = NULL;
						else if (obj_object == ch->left_hand)
							ch->left_hand = NULL;
						equip_char (ch, obj_object, WEAR_NECK_1);
					}
				}
			}
			else
				ch->send_to_char ("You can't wear that around your neck.\n");
			break;
			
		case KEYWEAR_BODY:
			if (CAN_WEAR (obj_object, ITEM_WEAR_BODY))
			{
				if (get_equip (ch, WEAR_BODY))
					ch->send_to_char ("You already wear something on your body."
								  "\n");
				else if (obj_object->size && obj_object->size != ch->get_size())
				{
					if (obj_object->size > ch->get_size())
						ch->act("$p won't fit, it is too large.",
							 true,  obj_object, 0, TO_CHAR);
					else
						ch->act("$p won't fit, it is too small.",
							 true,  obj_object, 0, TO_CHAR);
				}
				
				else
				{
					perform_wear (ch, obj_object, keyword);
					if (obj_object == ch->right_hand)
						ch->right_hand = NULL;
					else if (obj_object == ch->left_hand)
						ch->left_hand = NULL;
					equip_char (ch, obj_object, WEAR_BODY);
				}
			}
			else
				ch->send_to_char ("You can't wear that on your body.\n");
			break;
			
		case KEYWEAR_HEAD:
			if (CAN_WEAR (obj_object, ITEM_WEAR_HEAD))
			{
				if (get_equip (ch, WEAR_HEAD))
					ch->send_to_char ("You already wear something on your head."
								  "\n");
				else
				{
					perform_wear (ch, obj_object, keyword);
					if (obj_object == ch->right_hand)
						ch->right_hand = NULL;
					else if (obj_object == ch->left_hand)
						ch->left_hand = NULL;
					equip_char (ch, obj_object, WEAR_HEAD);
				}
			}
			else
				ch->send_to_char ("You can't wear that on your head.\n");
			break;
			
		case KEYWEAR_LEGS:
			if (CAN_WEAR (obj_object, ITEM_WEAR_LEGS))
			{
				if (get_equip (ch, WEAR_LEGS))
					ch->send_to_char ("You already wear something on your legs."
								  "\n");
				else if (obj_object->size && obj_object->size != ch->get_size())
				{
					if (obj_object->size > ch->get_size())
						ch->act("$p won't fit, it is too large.",
							 true,  obj_object, 0, TO_CHAR);
					else
						ch->act("$p won't fit, it is too small.",
							 true,  obj_object, 0, TO_CHAR);
				}
				
				else
				{
					perform_wear (ch, obj_object, keyword);
					if (obj_object == ch->right_hand)
						ch->right_hand = NULL;
					else if (obj_object == ch->left_hand)
						ch->left_hand = NULL;
					equip_char (ch, obj_object, WEAR_LEGS);
				}
			}
			else
				ch->send_to_char ("You can't wear that on your legs.\n");
			break;
			
		case KEYWEAR_FEET:
			if (CAN_WEAR (obj_object, ITEM_WEAR_FEET))
			{
				if (get_equip (ch, WEAR_FEET))
					ch->send_to_char ("You already wear something on your feet."
								  "\n");
				else
				{
					perform_wear (ch, obj_object, keyword);
					obj_from_char (&obj_object, 0);
					equip_char (ch, obj_object, WEAR_FEET);
				}
			}
			else
				ch->send_to_char ("You can't wear that on your feet.\n");
			break;
			
		case KEYWEAR_HANDS:
			if (CAN_WEAR (obj_object, ITEM_WEAR_HANDS))
			{
				if (get_equip (ch, WEAR_HANDS))
					ch->send_to_char ("You already wear something on your hands."
								  "\n");
				else
				{
					perform_wear (ch, obj_object, keyword);
					if (obj_object == ch->right_hand)
						ch->right_hand = NULL;
					else if (obj_object == ch->left_hand)
						ch->left_hand = NULL;
					equip_char (ch, obj_object, WEAR_HANDS);
				}
			}
			else
				ch->send_to_char ("You can't wear that on your hands.\n");
			break;
			
		case KEYWEAR_ARMS:
			if (CAN_WEAR (obj_object, ITEM_WEAR_ARMS))
			{
				if (get_equip (ch, WEAR_ARMS))
					ch->send_to_char ("You already wear something on your arms."
								  "\n");
				else if (obj_object->size && obj_object->size != ch->get_size())
				{
					if (obj_object->size > ch->get_size())
						ch->act("$p won't fit, it is too large.",
							 true,  obj_object, 0, TO_CHAR);
					else
						ch->act("$p won't fit, it is too small.",
							 true,  obj_object, 0, TO_CHAR);
				}
				
				else
				{
					perform_wear (ch, obj_object, keyword);
					if (obj_object == ch->right_hand)
						ch->right_hand = NULL;
					else if (obj_object == ch->left_hand)
						ch->left_hand = NULL;
					equip_char (ch, obj_object, WEAR_ARMS);
				}
			}
			else
				ch->send_to_char ("You can't wear that on your arms.\n");
			break;
			
		case KEYWEAR_ABOUT:
			if (CAN_WEAR (obj_object, ITEM_WEAR_ABOUT))
			{
				if (get_equip (ch, WEAR_ABOUT))
				{
					ch->send_to_char ("You already wear something about your body.\n");
				}
				else
				{
					perform_wear (ch, obj_object, keyword);
					if (obj_object == ch->right_hand)
						ch->right_hand = NULL;
					else if (obj_object == ch->left_hand)
						ch->left_hand = NULL;
					equip_char (ch, obj_object, WEAR_ABOUT);
				}
			}
			else
			{
				ch->send_to_char ("You can't wear that about your body.\n");
			}
			break;
		case KEYWEAR_WAIST:
		{
			if (CAN_WEAR (obj_object, ITEM_WEAR_WAIST))
			{
				if (get_equip (ch, WEAR_WAIST))
				{
					ch->send_to_char
					("You already wear something about your waist.\n");
				}
				else
				{
					perform_wear (ch, obj_object, keyword);
					if (obj_object == ch->right_hand)
						ch->right_hand = NULL;
					else if (obj_object == ch->left_hand)
						ch->left_hand = NULL;
					equip_char (ch, obj_object, WEAR_WAIST);
				}
			}
			else
			{
				ch->send_to_char ("You can't wear that about your waist.\n");
			}
		}
			break;
		case KEYWEAR_WRIST:
		{
			if (CAN_WEAR (obj_object, ITEM_WEAR_WRIST))
			{
				if (get_equip (ch, WEAR_WRIST_L) && get_equip (ch, WEAR_WRIST_R))
				{
					ch->send_to_char
					("You already wear something around both your wrists.\n");
				}
				else
				{
					perform_wear (ch, obj_object, keyword);
					if (obj_object == ch->right_hand)
						ch->right_hand = NULL;
					else if (obj_object == ch->left_hand)
						ch->left_hand = NULL;
					if (get_equip (ch, WEAR_WRIST_L))
					{
						sprintf (buffer,
								 "You wear the %s around your right wrist.\n",
								 fname (obj_object->name));
						ch->send_to_char (buffer);
						equip_char (ch, obj_object, WEAR_WRIST_R);
					}
					else
					{
						sprintf (buffer,
								 "You wear the %s around your left wrist.\n",
								 fname (obj_object->name));
						ch->send_to_char (buffer);
						equip_char (ch, obj_object, WEAR_WRIST_L);
					}
				}
			}
			else
			{
				ch->send_to_char ("You can't wear that around your wrist.\n");
			}
		}
			break;
			
					
		case KEYWEAR_BELT:
			if (!CAN_WEAR (obj_object, ITEM_WEAR_BELT))
				ch->send_to_char ("You cannot wear that on your belt.\n");
			
			else if (!get_equip (ch, WEAR_WAIST))
				ch->send_to_char ("You need a belt to wear that.\n");
			
			else if (get_equip (ch, WEAR_BELT_1) && get_equip (ch, WEAR_BELT_2))
				ch->send_to_char ("Your belt is full.\n");
			
			else
			{
				int belt_loc;
				
				/* Mostly I expect pouches to be equiped here.
				 put them in the second belt loc first */
				
				if (!get_equip (ch, WEAR_BELT_2))
					belt_loc = WEAR_BELT_2;
				else
					belt_loc = WEAR_BELT_1;
				
				perform_wear (ch, obj_object, keyword);
				if (obj_object == ch->right_hand)
					ch->right_hand = NULL;
				else if (obj_object == ch->left_hand)
					ch->left_hand = NULL;
				equip_char (ch, obj_object, belt_loc);
			}
			break;
			
		case KEYWEAR_BACK:
			if (!CAN_WEAR (obj_object, ITEM_WEAR_BACK))
				ch->send_to_char ("You cannot wear that across your back.\n");
			
			else if (get_equip (ch, WEAR_BACK))
				ch->send_to_char ("You are already wearing something there.\n");
			
			else
			{
				perform_wear (ch, obj_object, keyword);
				if (obj_object == ch->right_hand)
					ch->right_hand = NULL;
				else if (obj_object == ch->left_hand)
					ch->left_hand = NULL;
				equip_char (ch, obj_object, WEAR_BACK);
			}
			break;
			
		case KEYWEAR_BLINDFOLD:
			if (!CAN_WEAR (obj_object, ITEM_WEAR_BLINDFOLD))
				ch->send_to_char ("You cannot wear that over your eyes.\n");
			
			else if (get_equip (ch, WEAR_BLINDFOLD))
				ch->send_to_char ("Something already covers your eyes.\n");
			
			else
			{
				perform_wear (ch, obj_object, keyword);
				if (obj_object == ch->right_hand)
					ch->right_hand = NULL;
				else if (obj_object == ch->left_hand)
					ch->left_hand = NULL;
				equip_char (ch, obj_object, WEAR_BLINDFOLD);
			}
			
			break;
			
		case KEYWEAR_THROAT:
			if (!CAN_WEAR (obj_object, ITEM_WEAR_THROAT))
				ch->send_to_char ("You cannot wear that around your throat.\n");
			
			else if (get_equip (ch, WEAR_THROAT))
				ch->act("You are already wearing $p around your throat.",
					 false,  get_equip (ch, WEAR_THROAT), 0, TO_CHAR);
			
			else
			{
				perform_wear (ch, obj_object, keyword);
				if (obj_object == ch->right_hand)
					ch->right_hand = NULL;
				else if (obj_object == ch->left_hand)
					ch->left_hand = NULL;
				equip_char (ch, obj_object, WEAR_THROAT);
			}
			
			break;
			
		case KEYWEAR_EARS:
			if (!CAN_WEAR (obj_object, ITEM_WEAR_EAR))
				ch->send_to_char ("You cannot wear that on your ears.\n");
			
			else if (get_equip (ch, WEAR_EAR))
				ch->act("You are already wearing $p on your ears.",
					 false,  get_equip (ch, WEAR_EAR), 0, TO_CHAR);
			
			else
			{
				perform_wear (ch, obj_object, keyword);
				if (obj_object == ch->right_hand)
					ch->right_hand = NULL;
				else if (obj_object == ch->left_hand)
					ch->left_hand = NULL;
				equip_char (ch, obj_object, WEAR_EAR);
			}
			
			break;
			
		case KEYWEAR_SHOULDER:
		{
			if (CAN_WEAR (obj_object, ITEM_WEAR_SHOULDER))
			{
				if (get_equip (ch, WEAR_SHOULDER_L) &&
					get_equip (ch, WEAR_SHOULDER_R))
				{
					ch->send_to_char
					("You already wear something on both shoulders.\n");
				}
				else
				{
					perform_wear (ch, obj_object, keyword);
					if (obj_object == ch->right_hand)
						ch->right_hand = NULL;
					else if (obj_object == ch->left_hand)
						ch->left_hand = NULL;
					if (get_equip (ch, WEAR_SHOULDER_L))
					{
						sprintf (buffer,
								 "You sling the %s over your right shoulder.\n",
								 fname (obj_object->name));
						ch->send_to_char (buffer);
						equip_char (ch, obj_object, WEAR_SHOULDER_R);
					}
					else
					{
						sprintf (buffer,
								 "You sling the %s over your left shoulder.\n",
								 fname (obj_object->name));
						ch->send_to_char (buffer);
						equip_char (ch, obj_object, WEAR_SHOULDER_L);
					}
				}
			}
			else
			{
				ch->send_to_char ("You can't wear that on your shoulder.\n");
			}
		}
			break;
			
		case KEYWEAR_ANKLE:
		{
			if (CAN_WEAR (obj_object, ITEM_WEAR_ANKLE))
			{
				if (get_equip (ch, WEAR_ANKLE_L) && get_equip (ch, WEAR_ANKLE_R))
				{
					ch->send_to_char
					("You already wear something around both your ankles.\n");
				}
				else
				{
					perform_wear (ch, obj_object, keyword);
					if (obj_object == ch->right_hand)
						ch->right_hand = NULL;
					else if (obj_object == ch->left_hand)
						ch->left_hand = NULL;
					if (get_equip (ch, WEAR_ANKLE_L))
					{
						sprintf (buffer,
								 "You wear the %s around your right ankle.\n",
								 fname (obj_object->name));
						ch->send_to_char (buffer);
						equip_char (ch, obj_object, WEAR_ANKLE_R);
					}
					else
					{
						sprintf (buffer,
								 "You wear the %s around your left ankle.\n",
								 fname (obj_object->name));
						ch->send_to_char (buffer);
						equip_char (ch, obj_object, WEAR_ANKLE_L);
					}
				}
			}
			else
			{
				ch->send_to_char ("You can't wear that around your ankle.\n");
			}
		}
			break;
			
		case KEYWEAR_HAIR:
			if (!CAN_WEAR (obj_object, ITEM_WEAR_HAIR))
				ch->send_to_char ("You cannot wear that in your hair.\n");
			
			else if (get_equip (ch, WEAR_HAIR))
				ch->act("You are already wearing $p in your hair.",
					 false,  get_equip (ch, WEAR_HAIR), 0, TO_CHAR);
			
			else
			{
				perform_wear (ch, obj_object, keyword);
				if (obj_object == ch->right_hand)
					ch->right_hand = NULL;
				else if (obj_object == ch->left_hand)
					ch->left_hand = NULL;
				equip_char (ch, obj_object, WEAR_HAIR);
			}
			
			break;
			
		case KEYWEAR_FACE:
		{
			if (!CAN_WEAR (obj_object, ITEM_WEAR_FACE))
				ch->send_to_char ("You cannot wear that on your face.\n");
			
			else if (get_equip (ch, WEAR_FACE))
				ch->act("You are already wearing $p on your face.",
					 false,  get_equip (ch, WEAR_FACE), 0, TO_CHAR);
			
			else
			{
				perform_wear (ch, obj_object, keyword);
				if (obj_object == ch->right_hand)
					ch->right_hand = NULL;
				else if (obj_object == ch->left_hand)
					ch->left_hand = NULL;
				equip_char (ch, obj_object, WEAR_FACE);
			}
			
			break;
		}
			
			/* ARMBANDS, PATCHES, ARMLETS, ETC */
		case KEYWEAR_ARMBAND:
			
		{
			if (!CAN_WEAR (obj_object, ITEM_WEAR_ARMBAND))
			{
				ch->send_to_char ("You can't wear that around your upper arm.\n");
				return;
			}
			
			if (get_equip (ch, WEAR_ARMBAND_R) && get_equip (ch, WEAR_ARMBAND_L))			{
				ch->send_to_char
				("You already wearing something around both arms.\n");
				return;
			}
			
			else
			{
				perform_wear (ch, obj_object, keyword);
				
				if (obj_object == ch->right_hand)
				{
					ch->right_hand = NULL;
				}
				else if (obj_object == ch->left_hand)
				{
					ch->left_hand = NULL;
				}
				
				if (get_equip (ch, WEAR_ARMBAND_L))
				{
					sprintf (buffer, "You wear the %s around your upper right arm.\n",
							 fname (obj_object->name));
					ch->send_to_char (buffer);
					equip_char (ch, obj_object, WEAR_ARMBAND_R);
				}
				else
				{
					sprintf (buffer, "You wear the %s around your upper left arm.\n",
							 fname (obj_object->name));
					ch->send_to_char (buffer);
					equip_char (ch, obj_object, WEAR_ARMBAND_L);
				}
			}
			break;
		}
		case -1:
		{
			sprintf (buffer, "Wear %s where?.\n", fname (obj_object->name));
			ch->send_to_char (buffer);
		}
			break;
		case -2:
		{
			sprintf (buffer, "You can't wear the %s.\n",
					 fname (obj_object->name));
			ch->send_to_char (buffer);
		}
			break;
		default:
		{
			sprintf (buffer, "Unknown type called in wear, obj VNUM %d.",
					 obj_object->nVirtual);
			system_log (buffer, true);
		}
			break;
	}
}

void
do_wear (CHAR_DATA * ch, char *argument, int cmd)
{
	char arg1[MAX_STRING_LENGTH]= { '\0' };
	char arg2[MAX_STRING_LENGTH]= { '\0' };
	char buf[256];
	char buffer[MAX_STRING_LENGTH]= { '\0' };
	OBJ_DATA *obj_object;
	int keyword;
	
	*arg1 = '\0';
	*arg2 = '\0';

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);

	if (*arg1)
	{
		obj_object = get_obj_in_dark (ch, arg1, ch->right_hand);
		if (!obj_object)
			obj_object = get_obj_in_dark (ch, arg1, ch->left_hand);
		
		if (obj_object)
		{
			if (*arg2 && !isdigit (*arg2))
			{
				keyword = index_lookup (wear_bits, arg2);	/* Partial Match */
				if (keyword == -1 ||keyword == 0 )
				{
					sprintf (buf, "%s is an unknown body location.\n", arg2);
					ch->send_to_char (buf);
					return;
				}
				else
				{
					wear (ch, obj_object, keyword);
					return;
				}
			}
			else
			{
				if (CAN_WEAR (obj_object, ITEM_WEAR_FINGER))
					keyword = KEYWEAR_FINGER;
				if (CAN_WEAR (obj_object, ITEM_WEAR_NECK))
					keyword = KEYWEAR_NECK;
				if (CAN_WEAR (obj_object, ITEM_WEAR_WRIST))
					keyword = KEYWEAR_WRIST;
				if (CAN_WEAR (obj_object, ITEM_WEAR_WAIST))
					keyword = KEYWEAR_WAIST;
				if (CAN_WEAR (obj_object, ITEM_WEAR_ARMS))
					keyword = KEYWEAR_ARMS;
				if (CAN_WEAR (obj_object, ITEM_WEAR_HANDS))
					keyword = KEYWEAR_HANDS;
				if (CAN_WEAR (obj_object, ITEM_WEAR_FEET))
					keyword = KEYWEAR_FEET;
				if (CAN_WEAR (obj_object, ITEM_WEAR_LEGS))
					keyword = KEYWEAR_LEGS;
				if (CAN_WEAR (obj_object, ITEM_WEAR_ABOUT))
					keyword = KEYWEAR_ABOUT;
				if (CAN_WEAR (obj_object, ITEM_WEAR_HEAD))
					keyword = KEYWEAR_HEAD;
				if (CAN_WEAR (obj_object, ITEM_WEAR_BODY))
					keyword = KEYWEAR_BODY;
				if (CAN_WEAR (obj_object, ITEM_WEAR_BELT))
					keyword = KEYWEAR_BELT;
				if (CAN_WEAR (obj_object, ITEM_WEAR_BACK))
					keyword = KEYWEAR_BACK;
				if (CAN_WEAR (obj_object, ITEM_WEAR_BLINDFOLD))
					keyword = KEYWEAR_BLINDFOLD;
				if (CAN_WEAR (obj_object, ITEM_WEAR_THROAT))
					keyword = KEYWEAR_THROAT;
				if (CAN_WEAR (obj_object, ITEM_WEAR_EAR))
					keyword = KEYWEAR_EARS;
				if (CAN_WEAR (obj_object, ITEM_WEAR_SHOULDER))
					keyword = KEYWEAR_SHOULDER;
				if (CAN_WEAR (obj_object, ITEM_WEAR_ANKLE))
					keyword = KEYWEAR_ANKLE;
				if (CAN_WEAR (obj_object, ITEM_WEAR_HAIR))
					keyword = KEYWEAR_HAIR;
				if (CAN_WEAR (obj_object, ITEM_WEAR_FACE))
					keyword = KEYWEAR_FACE;
				if (CAN_WEAR (obj_object, ITEM_WEAR_ARMBAND))
					keyword = KEYWEAR_ARMBAND;

				if (keyword == -1 ||keyword == 0 )
				{
					sprintf (buf, "You can't wear %s.\n", arg1);
					ch->send_to_char (buf);
					return;
				}
				
				if (obj_object->activation &&
					IS_SET (obj_object->obj_flags.extra_flags,
					ITEM_WEAR_AFFECT))
					obj_activate (ch, obj_object);

				wear (ch, obj_object, keyword);
				return;
			}
		}
		else
		{
			sprintf (buffer, "You do not seem to have the '%s'.\n", arg1);
			ch->send_to_char (buffer);
			return;
		}
	}
	else
	{
		ch->send_to_char ("Wear what?\n");
		return;
	}
	return;
}



/**
 // Can they hold or wield the object
 */
int
can_handle (OBJ_DATA * obj, CHAR_DATA * ch)
{
	int wear_count = 0;

	if (get_equip (ch, WEAR_BOTH))
		wear_count = wear_count + 2;
	if (get_equip (ch, WEAR_LIGHT))
		wear_count++;
	if (get_equip (ch, WEAR_PRIM))
		wear_count++;
	if (get_equip (ch, WEAR_SEC))
		wear_count++;

	if (wear_count > 2)
	{
		return 0;
	}

	if (wear_count == 2)
		return 0;

	if (obj->o.od.value[0] == 0 && get_equip (ch, WEAR_PRIM))
		return 0;

	return 1;
}





void
do_empty (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *obj;
	OBJ_DATA *container;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (argument, buf);

	if (IS_SET (ch->room->room_flags, OOC) && (!ch->get_trust())
		&& str_cmp (ch->room->name, PREGAME_ROOM_NAME))
	{
		ch->send_to_char ("That is not allowed in OOC areas.\n");
		return;
	}

	if (!(container = get_obj_in_dark (ch, buf, ch->right_hand)) &&
		!(container = get_obj_in_dark (ch, buf, ch->left_hand)))
	{
		ch->send_to_char ("You don't have that object.\n");
		return;
	}

	if (container->obj_flags.type_flag == ITEM_CONTAINER)
	{

		if (container->contains)
		{
			ch->act("You turn $p upside down, spilling its contents:",
				false,  container, 0, TO_CHAR);
			ch->act("$n turns $p upside down, spilling its contents:",
				true,  container, 0, TO_ROOM);
		}

		while (container->contains)
		{
			obj = container->contains;
			obj_from_obj (&obj, 0, 0);
			obj_to_room (obj, ch->in_room);
			ch->act("    $p", false,  obj, 0, TO_CHAR);
			ch->act("    $p", false,  obj, 0, TO_ROOM);
		}

		return;
	}

	else if (container->obj_flags.type_flag == ITEM_POTION)
	{
		ch->act("You spill the contents of $p on the ground.",
			false,  container, 0, TO_CHAR);
		ch->act("$n spills the contents of $p on the ground.",
			false,  container, 0, TO_ROOM);
		extract_obj (container);
		return;
	}

	/**
	 capacity - od.value[0]
	 volume - od.value[1]
	 contents/liquid - od.value[2]
	 **/
	
	else if (container->obj_flags.type_flag == ITEM_DRYCON)
	{

		if (!container->o.od.value[1])
		{
			ch->act("$o is already empty.", false,  container, 0, TO_CHAR);
			return;
		}

		ch->act("You spill the contents of $p on the ground.",
			false,  container, 0, TO_CHAR);
		ch->act("$n spills the contents of $p on the ground.",
			false,  container, 0, TO_ROOM);

		obj = vtoo(container->o.drycon.contents);
		obj->count = container->o.drycon.volume;
		obj_to_room (obj, ch->in_room);
		
		container->o.drycon.capacity = 1;
		container->o.drycon.volume = 0;
		container->o.drycon.contents = 0;
		return;
	}

	else if (container->obj_flags.type_flag == ITEM_DRINKCON)
	{
		
		if (!container->o.od.value[1])
		{
			ch->act("$o is already empty.", false,  container, 0, TO_CHAR);
			return;
		}
		
		ch->act("You spill the contents of $p on the ground where it disappears.",
			 false,  container, 0, TO_CHAR);
		ch->act("$n spills the contents of $p on the ground where it dissappears.",
			 false,  container, 0, TO_ROOM);
		
		container->o.od.value[2] = 0;
		container->o.od.value[1] = 0;
		return;
	}
	
	else if (container->obj_flags.type_flag == ITEM_FUEL)
	{
		
		if (!container->o.od.value[1])
		{
			ch->act("$o is already empty.", false,  container, 0, TO_CHAR);
			return;
		}
		
		ch->act("You spill the contents of $p on the ground.",
			 false,  container, 0, TO_CHAR);
		ch->act("$n spills the contents of $p on the ground.",
			 false,  container, 0, TO_ROOM);
		
		obj = vtoo(container->o.fuelcon.fuel);
		obj->count = container->o.fuelcon.volume;
		obj_to_room (obj, ch->in_room);
		
		container->o.fuelcon.volume = 0;
		return;
	}
	
	else if (container->obj_flags.type_flag == ITEM_LIGHT &&
		!is_name_in_list ("candle", container->name))
	{

		if (!container->o.light.hours)
		{
			ch->act("$o is already empty.", false,  container, 0, TO_CHAR);
			return;
		}

		if (container->o.light.on)
			light (ch, container, false, true);

		sprintf (buf, "You empty the remains of %s from $p on the ground.",
			vnum_to_name (container->o.light.fuel));
		ch->act(buf, false,  container, 0, TO_CHAR);

		sprintf (buf, "$n empties the remains of %s from $p on the ground.",
			vnum_to_name (container->o.light.fuel));
		ch->act(buf, false,  container, 0, TO_ROOM);

		container->o.light.hours = 0;

		return;
	}

	ch->act("You can't figure out how to empty $p.",
		false,  container, 0, TO_CHAR);
}

void
do_blindfold (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (argument, buf);

	if (!(victim = get_char_room_vis (ch, buf)))
	{
		ch->send_to_char ("There's no such person here.\n");
		return;
	}

	if (victim == ch)
	{
		ch->send_to_char ("Wear a blindfold if you want to blindfold yourself.\n");
		return;
	}

	if (get_equip (victim, WEAR_BLINDFOLD))
	{
		ch->act("$N is already blindfolded.", false,  0, victim, TO_CHAR);
		return;
	}

	if (ch->right_hand
		&& IS_SET (ch->right_hand->obj_flags.wear_flags, ITEM_WEAR_BLINDFOLD))
		obj = ch->right_hand;

	else if (ch->left_hand
		&& IS_SET (ch->left_hand->obj_flags.wear_flags,
		ITEM_WEAR_BLINDFOLD))
		obj = ch->left_hand;

	if (!obj)
	{
		ch->send_to_char ("You don't have a blindfold available.\n");
		return;
	}

	if (!victim->is_awake() && number (0, 4))
	{
		if (victim->forced_wakeup())
		{
			ch->act("You've awoken $N.", false,  0, victim, TO_CHAR);
			ch->act("$N wakes you up while trying to blindfold you!",
				false,  0, victim, TO_VICT);
			ch->act("$n wakes $N up while trying to bindfold $M.",
				false,  0, victim, TO_NOTVICT);
		}
	}

	if (victim->is_awake()
		  ||get_affect (victim, MAGIC_AFFECT_PARALYSIS)
		  || victim->is_subduee())
	{
		ch->act("$N won't let you blindfold $M.", false,  0, victim, TO_CHAR);
		return;
	}

	if (obj->carried_by)
		obj_from_char (&obj, 0);

	victim->act("$N blindfolds you!", true, 0, ch, TO_CHAR);
	ch->act("You place $p over $N's eyes.", false,  obj, victim, TO_CHAR);
	ch->act("$n places $p over $N's eyes.", false,  obj, victim, TO_NOTVICT);

	equip_char (victim, obj, WEAR_BLINDFOLD);
}


void
light (CHAR_DATA * ch, OBJ_DATA * obj, int on, int on_off_msg)
{
	/* Automatically correct any problems with on/off status */

	if (obj->o.light.hours <= 0)
		obj->o.light.on = 0;

	if (!on && !obj->o.light.on)
		return;

	if (on && obj->o.light.hours <= 0)
		return;

	if (on && obj->o.light.on)
		return;

	obj->o.light.on = on;

	if (on && get_affect (ch, MAGIC_HIDDEN))
	{
		if (ch->would_reveal())
			ch->act("You reveal yourself.", false,  0, 0, TO_CHAR);
		else
			ch->act("The light will reveal your hiding place.",
			false,  0, 0, TO_CHAR);

		remove_affect_type (ch, MAGIC_HIDDEN);
	}

	if (on)
	{
		room_light (ch->room);	/* lighten before messages */
		if (on_off_msg)
		{
			ch->act("You light $p.", false,  obj, 0, TO_CHAR | _ACT_FORMAT);
			ch->act("$n lights $p.", false,  obj, 0, TO_ROOM | _ACT_FORMAT);
		}
	}
	else
	{
		if (on_off_msg)
		{
			ch->act("You put out $p.", false,  obj, 0, TO_CHAR | _ACT_FORMAT);
			ch->act("$n puts out $p.", false,  obj, 0, TO_ROOM | _ACT_FORMAT);
		}
		room_light (ch->room);	/* darken after messages */
	}
}

void
do_light (CHAR_DATA * ch, char *argument, int cmd)
{
	int on = 1;
	int room_only = 0;
	OBJ_DATA *obj;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (argument, buf);

	if (!str_cmp (buf, "room"))
	{
		room_only = 1;
		argument = one_argument (argument, buf);
	}

	if (!*buf)
	{
		ch->send_to_char ("Light what?\n");
		return;
	}

	if (room_only)
	{
		obj = get_obj_in_dark (ch, buf, ch->room->contents);
	}
	else
	{
		obj = get_obj_in_dark (ch, buf, ch->right_hand);
		if (!obj)
			obj = get_obj_in_dark (ch, buf, ch->left_hand);
		if (!obj && (obj = get_obj_in_dark (ch, buf, ch->equip))
			&& !IS_SET (obj->obj_flags.extra_flags, ITEM_MAGIC))
		{
			ch->act("You can't light $p while you're still wearing it.\n", false,
				obj, 0, TO_CHAR);
			return;
		}
	}

	if (!obj)
	{
		ch->send_to_char ("You don't see that light source.\n");
		return;
	}

	if (obj->obj_flags.type_flag != ITEM_LIGHT)
	{
		ch->act("You cannot light $p.", false,  obj, 0, TO_CHAR);
		return;
	}

	argument = one_argument (argument, buf);

	if (!str_cmp (buf, "off"))
		on = 0;

	if (!on && !obj->o.light.on)
	{
		ch->act("$p isn't lit.", false,  obj, 0, TO_CHAR);
		return;
	}

	if (on && obj->o.light.hours <= 0)
	{
		ch->act("$p will no longer light.", false,  obj, 0, TO_CHAR);
		return;
	}

	if (on && obj->o.light.on)
	{
		ch->act("$p is already lit.", false,  obj, 0, TO_CHAR);
		return;
	}

	light (ch, obj, on, true);
}



/****
 mode = 0 repair all damage
 mode = # repair just the one damage
 ****/
void
begin_repair (CHAR_DATA * ch, OBJ_DATA *obj, OBJ_DATA *kit, int mode)
{
	OBJECT_DAMAGE *damage;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char *damage_sdesc;
	int count = 1; 
	std::vector<object_damage *>::iterator odam_iterator;
	
	if ((mode==0) && (!obj->damage.empty())) //repairing all damage so we start with first available
	{
		damage = *obj->damage.begin();
		damage_sdesc = object_damage__get_sdesc(damage);
		snprintf(buf, MAX_STRING_LENGTH, "You turn your attention to %s.",
				 damage_sdesc);
		snprintf(buf2, MAX_STRING_LENGTH, "$n turns $s attention to %s.",
				 damage_sdesc);
		
		ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
		ch->act(buf2, false,  0, 0, TO_ROOM | _ACT_FORMAT);
		ch->delay_who = duplicateString (obj->name);
		ch->delay_type = DEL_MEND_OBJECT;
		
		if (kit->o.od.value[3] > 0)
		{
			ch->delay = 10 - (ch->skill_map[lookup_skill_name(kit->o.od.value[3])] / 10);
		}
		else 
		{
			ch->delay = 10;
		}
		
		ch->delay = MAX (ch->delay, 10);
		ch->delay_info1 = mode;
		return;
	}
	
	else if (mode > 0)
	{
		for (odam_iterator = obj->damage.begin(); odam_iterator != obj->damage.end(); odam_iterator++)
		{
			damage = *odam_iterator;
			if (!damage)//damage_from_obj removes damage, so we must check here too
				break;
						
			if (count == mode)
			{
				damage_sdesc = object_damage__get_sdesc(damage);
				snprintf(buf, MAX_STRING_LENGTH, "You turn your attention to %s.",
						 damage_sdesc);
				snprintf(buf2, MAX_STRING_LENGTH, "$n turns $s attention to %s.",
						 damage_sdesc);
				ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
				ch->act(buf2, false,  0, 0, TO_ROOM | _ACT_FORMAT);
				ch->delay_who = duplicateString (obj->name);
				ch->delay_type = DEL_MEND_OBJECT;
				
				if (kit->o.od.value[3] > 0)
				{
					ch->delay = 10 - (ch->skill_map[lookup_skill_name(kit->o.od.value[3])] / 10);
				}
				else 
				{
					ch->delay = 10;
				}
				
				ch->delay = MAX (ch->delay, 10);
				ch->delay_info1 = count;
				return;
			}
			count ++;
		}
	}
	sprintf (buf,
			 "No other damage on this item can benefit from your attention.");
	ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
	ch->delay = 0;
	ch->delay_type = 0;
	ch->delay_who = 0;
	ch->delay_info1 = 0;
	
	
	return;
}



OBJECT_DAMAGE *
damage_from_obj (OBJ_DATA * obj, OBJECT_DAMAGE * damage)
{
	OBJECT_DAMAGE *tempdamage;
	float tmp1;
	std::vector<OBJECT_DAMAGE*>::iterator odam_iterator;
	
	if (!obj)
		return NULL;
	
	if (!damage)
		return NULL;
	
	if (obj->damage.empty())
		return NULL;
	
	for (odam_iterator = obj->damage.begin(); odam_iterator != obj->damage.end(); odam_iterator++)
	{
		tempdamage = *odam_iterator;
		if(tempdamage == damage)
		{
				//remove damage from list
			obj->damage.erase(odam_iterator);
			obj->current_damage = obj->current_damage - tempdamage->impact;
				//recalculate item wear
			tmp1 = (10.0 * obj->current_damage)/(10.0 * obj->max_hit_points);
			obj->item_wear = (int)((1.0 - tmp1) * 100);
			break;
		}
	}
	
	return (tempdamage);
}

int
skill_mend(CHAR_DATA * ch, OBJ_DATA *kit, OBJECT_DAMAGE * damage)
{
	int repair_skill;
	int severity;
	char * skill_name;
	
		
	severity = damage->severity;
	repair_skill = kit->o.od.value[3];
	if (repair_skill)
	{
		skill_name = strdup(lookup_skill_name(repair_skill));
	}
	
	if ((repair_skill > 0) && (skill_name))
	{
		switch (severity) {
			case 1:
				if (ch->skill_map[skill_name] > 15)
					return(true);
				else
					return(false);
				break;
				
			case 2:
				if (ch->skill_map[skill_name] > 20)
					return(true);
				else
					return(false);
				break;
				
			case 3:
				if (ch->skill_map[skill_name] > 25)
					return(true);
				else
					return(false);
				break;
				
			case 4:
				if (ch->skill_map[skill_name] > 30)
					return(true);
				else
					return(false);
				break;
				
			case 5:
				if (ch->skill_map[skill_name] > 35)
					return(true);
				else
					return(false);
				break;
				
			case 6:
				if (ch->skill_map[skill_name] > 40)
					return(true);
				else
					return(false);
				break;
				
			case 7:
				if (ch->skill_map[skill_name] > 50)
					return(true);
				else
					return(false);
				break;
				
			case 8:
				if (ch->skill_map[skill_name] > 60)
					return(true);
				else
					return(false);
				break;
				
				
			default:
				return(false);
				break;
		}
	}
	
	else if (repair_skill == 0) //any skill will work
	{
		return (true);
	}
	else 
	{
		return (false);
	}

}
/*******************
 *
 * Cost is per percentage point of damage times 10% of vaue of item times repeat discount
 * Discount:
 * Repair one thing - 15% higher
 * Repair multiple items - first one is 15% off and each one after that 5% off
 * 
 * Entire item repaired X times =  cost of new item where X depends on quality of item
 * Quality = 30, means the item can be repaired 3 times, before it equals cost of new item
 * higher quality items are cheaper to repair, since they are easier to fix. They are
 * also more expensive, so it is a balance for players choice.
 *******************/
void
npc_repair (CHAR_DATA * ch, CHAR_DATA * mob, OBJ_DATA *obj, char *argument)
{
	OBJECT_DAMAGE * damage;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	int count;
	int item_num;
	float cost = 0;
	float obj_cost = 0;
	std::vector<object_damage *>::iterator odam_iterator;
	
	obj_cost = (obj->coppers/100) / (obj->quality/10); 
	
	if (!mob || ((IS_NPC(ch) && !IS_SET (mob->mob->profession, PROF_REPAIR))))
	{
		ch->send_to_char ("I don't see a repairman here.\n");
		return;
	}
	
	if (mob->delay)
	{
		ch->act("$n appears to be busy.", true,  0, mob, TO_CHAR | _ACT_FORMAT);
		return;
	}
	
	name_to_ident (ch, buf2);
	if (obj->damage.empty())
	{
		sprintf (buf, "whisper %s I don't see any damage on the item!",
				 buf2);
		command_interpreter (mob, buf);
		return;
	}
	
	
	argument = one_argument (argument, buf);//mend sword -value/#-
	
	
	if (!*buf)
	{
		ch->send_to_char ("Do you want an estimate or do you want me to fix it?\n");
		return;
	}
	
	if (!str_cmp (buf, "value"))
	{
		if (!*argument)  //mend sword value -#- (# is argument)
		{
			ch->send_to_char
			("Which damage did you wish to get an appraisal for?\n");
			return;
		}
		one_argument (argument, buf);

		item_num = atoi(buf);

	
		if (!strn_cmp (buf, "all", strlen (buf)))
		{
			for (odam_iterator = obj->damage.begin(); odam_iterator != obj->damage.end(); odam_iterator++)
			{
				damage = *odam_iterator;
				if (!damage)
					break;
				
				if (odam_iterator != obj->damage.end())
				{
					cost += damage->impact  * obj_cost  * 0.85 ;
				}
				else
				{
					cost += damage->impact  * obj_cost  * 0.95 ;
				}
			}
			if (mob->mob->shop)
				cost *= mob->mob->shop->markup;
		}
		else
		{
			count = 1;
			for (odam_iterator = obj->damage.begin(); odam_iterator != obj->damage.end(); odam_iterator++)
			{
				damage = *odam_iterator;
				if (!damage)
					break;
				
				if (count == item_num)
				{
					cost += damage->impact  * 1.15 * obj_cost;
					if (mob->mob->shop)
						cost *= mob->mob->shop->markup;
					break;
				}
				count ++;
			}
			
			if (!damage)
			{
				sprintf (buf, "whisper %s I don't see any damage there to repair.",
						 buf2);
				command_interpreter (mob, buf);
				return;
			}
			
		}
		
		if (cost < 1)
			cost = 1;
		
		if (cost < 1)
		{
			sprintf (buf,
					 "whisper %s There's nothing I can do about that damage.",
					 buf2);
			command_interpreter (mob, buf);
			return;
		}
		
		sprintf (buf,
				 "whisper %s I'll fix it all up for a total of %d coppers.",
				 buf2, (int) cost);
		command_interpreter (mob, buf);
		return;
	}
	
	if (!strn_cmp (buf, "all", strlen (buf)))
	{
		item_num = -1;
		for (odam_iterator = obj->damage.begin(); odam_iterator != obj->damage.end(); odam_iterator++)
		{
			damage = *odam_iterator;
			if (!damage)
				break;
			
			if (odam_iterator != obj->damage.end())
			{
				cost += damage->impact * 0.85 * obj_cost ;
			}
			else
			{
				cost += damage->impact  * 0.95 * obj_cost;
			}
		}
		if (mob->mob->shop)
			cost *= mob->mob->shop->markup;
	}
	else 
	{
		item_num = atoi(buf);
		count = 1;
		for (odam_iterator = obj->damage.begin(); odam_iterator != obj->damage.end(); odam_iterator++)
		{
			damage = *odam_iterator;
			if (!damage)
				break;
			
			if (count == item_num)
			{
				cost += damage->impact  * 1.15 * obj_cost;
				if (mob->mob->shop)
					cost *= mob->mob->shop->markup;
				break;
			}
			count ++;
		}
		if (!damage)
		{
			sprintf (buf, "whisper %s I don't see any damage there to repair.",
					 buf2);
			command_interpreter (mob, buf);
			return;
		}
		
	}
	
	if (cost < 1)
		cost = 1;
	
	if (cost < 1)
	{
		sprintf (buf,
				 "whisper %s All of the damage has been taken care of - there's nothing I can do.",
				 buf2);
		command_interpreter (mob, buf);
		return;
	}
	
	if (!is_brother (ch, mob))
	{
		
		if (!can_subtract_money (ch, (int) cost, mob->mob->currency_type))
		{
			sprintf (buf, "%s You seem to be a little short on coin.", buf2);
			do_whisper (mob, buf, 83);
			return;
		}
		
		subtract_money (ch, (int) cost, mob->mob->currency_type);
		if (mob->mob->shop && mob->mob->shop->store_vnum)
			money_to_storeroom (mob, (int) cost);
		
		ch->send_to_char ("\n");
	}
	else
	{
		sprintf (buf, "whisper %s There is no cost to you for this treatment.",
				 buf2);
		command_interpreter (mob, buf);
	}
	
	ch->act("$N promptly repairs the damage.", true,  0, mob,
		 TO_CHAR | _ACT_FORMAT);
	ch->act("$N promptly repairs the damage to $n's item.", true,  0, mob,
		 TO_ROOM | _ACT_FORMAT);

	/*** now to do the actually repair stuff**/
	if (!obj->damage.empty())
	{
		count = 1;
		sprintf(buf, "start - count is %d num is %d\n", count, item_num);
				
		for (odam_iterator = obj->damage.begin(); odam_iterator != obj->damage.end(); odam_iterator++)
		{
			damage = *odam_iterator;
						
			if (item_num > 0) //repairs one specific damage
			{
				if (count == item_num)
				{
				obj->item_wear = obj->item_wear + damage->impact;
				damage = damage_from_obj(obj, damage);	
				return;	
				}
								
			}
			else //repair all of it
			{
				obj->item_wear = obj->item_wear + damage->impact;
				damage = damage_from_obj(obj, damage);
			}
			
			count ++;
			
		}
	}	
}

OBJ_DATA*
vtoo(int objectVirtual) 
{
	std::map<int, OBJ_DATA*>::iterator it;
	
	
	if (objectVirtual > 0)
	{
		it = proto_obj_map.find(objectVirtual);
		if (it != proto_obj_map.end())
		{
			return (it->second);
		}
		else 
		{
			return (NULL);
		}
	}
	else 
	{
		return (NULL);
	}
}

void
do_variant (CHAR_DATA * ch, char *argument, int cmd)
{
	char tag_name[MAX_INPUT_LENGTH] = {'\0'};
	char output[MAX_STRING_LENGTH] = {'\0'};
	int found;
	std::string holder;
	VARIANT_VALUE* tvariant;
	std::map<int, VARIANT_VALUE*>::iterator var_it;
	
	if(gl_variant.empty())
	{
		ch->send_to_char("Sorry, but the variant list is empty.\n");
		return;
	}
	
	argument = one_argument (argument, tag_name);
	
	if (!str_cmp(tag_name, "list"))
	{
		for (var_it = gl_variant.begin(); var_it != gl_variant.end(); var_it++)
		{
			tvariant = var_it->second;
			found = holder.find(tvariant->name);
			
			if (found==std::string::npos)
			{
				holder.append(tvariant->name);
				holder.append("\n");
			}
			
		}
		sprintf(output, "%s \n", holder.c_str());
	}
	else
	{
		for (var_it = gl_variant.begin(); var_it != gl_variant.end(); var_it++)
		{
			tvariant = var_it->second;
			if(!str_cmp (tag_name, tvariant->name))
			{
				sprintf(output + strlen(output), "%s \n", tvariant->var_value);
			}
			else if (!*tag_name)
				sprintf(output + strlen(output), "%s    %s \n", tvariant->name, tvariant->var_value);
			
		}
	}
	
	if (!*output)
	{
		ch->send_to_char("  Sorry, there were no results. Did you use the right variant name and include the $? Use  'variant list' to get a listing of available variants.\n");
		return;
	}
	else
		page_string (ch->desc, output);
	
	
}

void 
obj_type(CHAR_DATA* ch, char*argument)
{
	int ind;
	OBJ_DATA* tobj;
	char buf[MAX_STRING_LENGTH] = {'\0'};;
	
	if (!*argument)
	{
		ch->send_to_char("Which item type are you trying to set?\n");
		return;
	}
	
	tobj = vtoo(ch->pc->edit_obj);
	
	if (!tobj)
	{
		ch->send_to_char("You are not currently editing an object.\n");
		return;
	}
	
	
	
	if ((ind = index_lookup (item_types, argument)) != -1)
	{
		tobj->obj_flags.type_flag = ind;
		sprintf (buf, "Type %s set.\n", item_types[ind]);
		ch->send_to_char (buf);
	}
	
	return;
}

void
obj_wear (CHAR_DATA* ch, char* argument)
{
	int ind;
	OBJ_DATA* tobj;
	
	if (!*argument)
	{
		ch->send_to_char("Which wear location are you trying to set?\n");
		return;
	}
	
	tobj = vtoo(ch->pc->edit_obj);
	
	if (!tobj)
	{
		ch->send_to_char("You are not currently editing an object.\n");
		return;
	}

	if ((ind = index_lookup (wear_bits, argument)) != -1)
	{
		if (IS_SET (tobj->obj_flags.wear_flags, 1 << ind))
			tobj->obj_flags.wear_flags &= ~(1 << ind);
		else
			tobj->obj_flags.wear_flags |= (1 << ind);
		
		ch->send_to_char ("Wear bit set.\n");
	}
	
}

void
obj_timer (CHAR_DATA* ch, char* argument)
{
	OBJ_DATA* tobj;
	
	if (!*argument)
	{
		ch->send_to_char
		("What did you want to set the object's timer to?\n");
		return;
	}
	
	tobj = vtoo(ch->pc->edit_obj);
	if (!tobj)
	{
		ch->send_to_char("You are not currently editing an object.\n");
		return;
	}
	
	if (!isdigit (*argument))
	{
		ch->send_to_char ("Expected a numeric value for the new timer.\n");
		return;
	}
	tobj->obj_timer = atoi (argument);
	
	if (!tobj->obj_timer
		&& IS_SET (tobj->obj_flags.extra_flags, ITEM_TIMER))
		tobj->obj_flags.extra_flags &= ~ITEM_TIMER;
	
	else if (tobj->obj_timer
			&& !IS_SET (tobj->obj_flags.extra_flags, ITEM_TIMER))
			tobj->obj_flags.extra_flags |= ITEM_TIMER;
			
	ch->send_to_char ("The object's timer has been set.\n");
	return;
}

void 
obj_extra (CHAR_DATA* ch, char* argument)
{
	int ind;
	OBJ_DATA* tobj;
	
	if (!*argument)
	{
		ch->send_to_char("Which extra tag are you trying to set?\n");
		return;
	}
	
	tobj = vtoo(ch->pc->edit_obj);
	if (!tobj)
	{
		ch->send_to_char("You are not currently editing an object.\n");
		return;
	}
	
		
	if ((ind = index_lookup (extra_bits, argument)) != -1)
	{
		
		if ((1 << ind) == ITEM_VNPC)
		{
			ch->send_to_char ("This flag cannot be set manually.\n");
			return;
		}
		
		if (((tobj->obj_flags.type_flag != ITEM_WORN)
			 && (!str_cmp (argument, "mask")))
			|| ((IS_SET (tobj->obj_flags.wear_flags, ITEM_WEAR_HEAD))
				&& (IS_SET (tobj->obj_flags.wear_flags, ITEM_WEAR_FACE))
				&& (!str_cmp (argument, "mask"))))
			ch->send_to_char
			("A mask must be worn HEAD, or FACE and must be WORN.\n");
		
		else if ((!IS_SET (tobj->obj_flags.wear_flags, ITEM_WEAR_HEAD))
				 && (!IS_SET (tobj->obj_flags.wear_flags, ITEM_WEAR_FACE))
				  && (!str_cmp (argument, "mask")))
			ch->send_to_char
			("HEAD or FACE can be a mask, but not ABOUT.\n");
		
		else if (IS_SET (tobj->obj_flags.extra_flags, 1 << ind))
			tobj->obj_flags.extra_flags &= ~(1 << ind);
		else
			tobj->obj_flags.extra_flags |= (1 << ind);
	}
	
	ch->send_to_char ("The object's extra flag has been set.\n");
	return;
}

void
obj_affect (CHAR_DATA* ch, char* argument)
{
	int ind;
	OBJ_DATA* tobj;
	char* buf;
	AFFECTED_TYPE *af;
	
	if (!*argument)
	{
		ch->send_to_char("Which skill are you trying to affect?\n");
		return;
	}
	
	tobj = vtoo(ch->pc->edit_obj);
	if (!tobj)
	{
		ch->send_to_char("You are not currently editing an object.\n");
		return;
	}
	
		//the skill
	argument = one_argument (argument, buf);
	if (lookup_skill_id(buf) == -1)
	{
		ch->send_to_char ("No such skill.\n");
		return;
	}
	
	
		//skill value
	if (!*argument)
	{
		ch->send_to_char("What is the skill value?\n");
		return;
	}
	argument = one_argument (argument, buf);
	if (*buf != '0' && !atoi (buf))
	{
		ch->send_to_char ("Expected a skill value.\n");
		return;
	}
	
	if (get_obj_affect_location (tobj, ind))
		remove_obj_affect_location (tobj, ind);
	
	if (atoi (buf))
	{
		ch->send_to_char ("Object skill affect set.\n");
		af = new AFFECTED_TYPE;
		
		af->type = 0;
		af->a.spell.duration = -1;
		af->a.spell.bitvector = 0;
		af->a.spell.sn = 0;
		af->a.spell.location = ind + 10000;
		af->a.spell.modifier = atoi (buf);
		af->next = NULL;
		
		affect_to_obj (tobj, af);
	}
	else
	{
		ch->send_to_char ("Object skill affect removed.\n");
		remove_obj_affect_location (tobj, ind + 10000);
	}
	
	return;
}
 
void
obj_delete (CHAR_DATA* ch, char* argument)
{
	int loads = 0;
	OBJ_DATA* tobj;
	OBJ_DATA* tmp_obj;
	std::list<obj_data*>::iterator tobj_iterator;
	
	tobj = vtoo(ch->pc->edit_obj);
	
	if (!tobj)
	{
		ch->send_to_char("You are not currently editing an object.\n");
		return;
	}
	
	if (tobj->deleted)
	{
		tobj->deleted = 0;
		ch->send_to_char ("This object is no longer marked for deletion."
					  "\n");
		return;
	}
	
	
	for (tobj_iterator = object_list.begin(); tobj_iterator != object_list.end(); tobj_iterator++)
	{
		tmp_obj = *tobj_iterator;
		
		if (tmp_obj->deleted)
			continue;
		
		if (tmp_obj->nVirtual == tobj->nVirtual)
			loads++;
	}
	
	if (loads > 0)
	{
		ch->send_to_char ("Clear this object from the world first.\n");
		return;
	}
	
	ch->send_to_char ("WARNING:  This object is cleared from the world.  "
				  "However, the prototype\n"
				  "          cannot be removed until the mud is rebooted.\n"
				  "          Use the DELETE option again to undo "
				  "deletion.\n");
	
	tobj->deleted = 1;
	
	return;
}

void 
obj_material (CHAR_DATA* ch, char* argument)
{
	char* buf;
	int i;
	OBJ_DATA* tobj;
	std::list<obj_data*>::iterator tobj_iterator;
	std::set<std::string>::iterator obj_mater_it;
	std::map<int, OBJECT_MATERIAL*>::iterator it_material;
	bool found_material = false;

	if (!*argument)
	{
		ch->send_to_char("What material are you trying to set?\n");
		return;
	}
	
	tobj = vtoo(ch->pc->edit_obj);
	
	if (!tobj)
	{
		ch->send_to_char("You are not currently editing an object.\n");
		return;
	}
	
		//is this a real material
		//does this material exist in array already?
		//if it does, remove it
		//else add it
		//--when adding material, remove "none"
	
	
		
		if (find_material(argument))
		{
			found_material = false;
			for (i = 0; i < MAX_OBJ_MATERIALS; i++)
			{
				if (!str_cmp(argument, tobj->materials[i]))
				{
						//found it so we remove it
					tobj->materials[i] = strdup("");
					found_material = true;
					break;
				}
			}
			
			for (i = 0; i < MAX_OBJ_MATERIALS; i++)
			{
				if ((!found_material) && (!str_cmp(tobj->materials[i], "none")))
				{
						//did not find it so we replace "none"
					tobj->materials[i] = strdup(argument);
					found_material = true;
					break;
				}
				if ((!found_material) && (!str_cmp(tobj->materials[i], "")))
				{
						//did not find it so we add it to first empty spot
					tobj->materials[i] = strdup(argument);
					found_material = true;
					break;
				}
			}
			
			if (!found_material)
			{
				ch->send_to_char("You have too many materials. You cannot add more.\n");
				return;
			}
		}
		else 
		{
			ch->send_to_char("That material does not exist. Did you spell it wrong?.\n");
			return;
		}

	
	
		//we changed material, so we need to re-calcualte the hit points
	if (found_material)
	{
		tobj->max_hit_points = (get_material_hardness(tobj) * (tobj->obj_flags.weight)/100.0);
		if (tobj->max_hit_points > 0 && tobj->max_hit_points < 1)
			tobj->max_hit_points = 1;
		
	}
	else //we didn't find a matching material, so list the possibilities
	{
		ch->send_to_char("That is not a valid material. You may select one of the following:\n");
		strcpy (buf, "   ");
		
		for (it_material = object_material_map.begin(), i=0; it_material != object_material_map.end(); it_material++, i++)
		{
			sprintf (buf + strlen (buf), "%-23s ", it_material->second->material_name);
			if (!((i + 1) % 3))
				strcat (buf, "\n   ");
			
		}
		if (!((i + 1) % 3) || ((i + 1) % 3) == 2)
			strcat (buf, "\n");
		
		ch->send_to_char (buf);
		return;
	}
	
}


	//damages can add up and become worse. 
	//A number of tiny scratches can become a small gouge.
	//Function loops through all damage to the object
	//removes a few (1-4?) damages of similar severity
	//replaces them with a single instance of severity + 1
	//only one consolidation per function call
int
consolidate_damage(OBJ_DATA* thisobj, CHAR_DATA* ch)
{
	OBJECT_DAMAGE *tdamage;
	int item_count = 0; 
	int check_value = 0;
	int num_remove = 0;
	int temp_source = 0;
	int	temp_impact = 0;
	int true_impact = 0;
	bool done_flag = false;
	
	num_remove = number(2, 4);
	
	std::vector<object_damage *>::iterator odam_iterator;

		//find out if we have enough of the same size or smaller to consolidate
		//max severity - 1 to allow for replacing with greater damage
	for (check_value = 0; check_value < 9; check_value++)
	{
		item_count = 0;
		
		for (odam_iterator = thisobj->damage.begin(); odam_iterator != thisobj->damage.end(); odam_iterator++)
		{
			tdamage = *odam_iterator;
			if (!tdamage)
			{
				done_flag = true;
				break;
			}
			
			if (tdamage->severity <= check_value)
				item_count ++;
			
			if (item_count == num_remove)
			{
				done_flag = true;
				break;
			}
		}
		
		if (done_flag)
			break;
	}
	
		//not enough was found, so we return with nothing being consolidated
	if (!done_flag)
		return 0;
	
	done_flag = false;
	for (odam_iterator = thisobj->damage.begin(); odam_iterator != thisobj->damage.end(); odam_iterator++)
	{
		tdamage = *odam_iterator;
		if (!tdamage)
		{
			break;
		}
		
		
			//remove some damages
		if (tdamage->severity <= check_value)
		{
				//save the values for latest damage so we can bump it up later			
			temp_source = tdamage->source;
			temp_impact = tdamage->impact;
			
			
			done_flag = true;
			damage_from_obj(thisobj, tdamage);
			odam_iterator = thisobj->damage.begin();
			item_count --;
		}
		
		if (!item_count)
			break;
		
	}
	
		//nothing was saved for later increase
	if (!done_flag)
		return 0;	
	
		//add one damage back in that is a little bit worse
	true_impact = temp_impact + get_material_hardness(thisobj);
	tdamage = object__add_damage (thisobj, temp_source, true_impact+1, ch, 1);
	
	
	ch->act("The damage to $p just got worse!",
		 false,  thisobj, 0, TO_CHAR | _ACT_FORMAT);
	
	
	return 1;
	
}

void
do_remove (CHAR_DATA * ch, char *argument, int cmd)
{
	OBJ_DATA *arrow = NULL;
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buffer[MAX_STRING_LENGTH];
	char location[MAX_STRING_LENGTH];
	CHAR_DATA *tch = NULL;
	OBJ_DATA *obj = NULL;
	OBJ_DATA *eq;
	int removed = 0, target_found = 0;
	int target_obj = 0, target_char = 0;
	
		
	argument = one_argument (argument, arg1);
	
	if (!*arg1)
    {
		ch->send_to_char ("Remove what?\n");
		return;
    }
	
		
	for (obj = ch->equip; obj; obj = obj->next_content)
		if (IS_OBJ_VIS (ch, obj) && isname (arg1, obj->name))
			break;
	
	if (!obj)
    {
		if (ch->right_hand)
		{
			if (isname (arg1, ch->right_hand->name))
				obj = ch->right_hand;
		}
		if (ch->left_hand)
		{
			if (isname (arg1, ch->left_hand->name))
				obj = ch->left_hand;
		}
    }
	
	if ((get_equip (ch, WEAR_BOTH) && obj != get_equip (ch, WEAR_BOTH))
		|| ((ch->right_hand && obj != ch->right_hand)
			&& (ch->left_hand && obj != ch->left_hand)))
    {
		ch->send_to_char ("Your hands are otherwise occupied, at the moment.\n");
		return;
    }
	
	if (!obj)
    {
		ch->send_to_char ("Remove what?\n");
		return;
    }
	
	if (obj->location == -1)
    {
		ch->send_to_char ("You don't need to remove that!\n");
		return;
    }
	
		
	if (obj->location == WEAR_WAIST)
    {
		if ((eq = get_equip (ch, WEAR_BELT_1)))
		{
			ch->act("$p falls to the floor.", true,  eq, 0, TO_CHAR);
			ch->act("$n drops $p.", true,  eq, 0, TO_ROOM);
			obj_to_room (unequip_char (ch, WEAR_BELT_1), ch->in_room);
		}
		
		if ((eq = get_equip (ch, WEAR_BELT_2)))
		{
			ch->act("$p falls to the floor.", true,  eq, 0, TO_CHAR);
			ch->act("$n drops $p.", true,  eq, 0, TO_ROOM);
			obj_to_room (unequip_char (ch, WEAR_BELT_2), ch->in_room);
		}
    }
	
		
	if (IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
		IS_SET (ch->affected_by, AFF_HOODED))
		do_hood (ch, "", 0);
	
	if (obj->location == WEAR_LIGHT && obj->obj_flags.type_flag == ITEM_LIGHT)
		light (ch, obj, false, true);
	
	if (obj->location == WEAR_PRIM || obj->location == WEAR_SEC ||
		obj->location == WEAR_BOTH)
		unequip_char (ch, obj->location);
	else
		obj_to_char (unequip_char (ch, obj->location), ch);
	
	ch->act("You stop using $p.", false,  obj, 0, TO_CHAR);
	ch->act("$n stops using $p.", true,  obj, 0, TO_ROOM);
	
}
 

