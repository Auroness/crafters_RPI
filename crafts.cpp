//////////////////////////////////////////////////////////////////////////////
//
/// crafts.c : Crafting Module
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
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "server.h"
#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "group.h"
#include "utility.h"



extern rpie::server engine;
extern const char *weather_states[];
extern std::map<std::string, SUBCRAFT_HEAD_DATA*> craft_map;


const char *item_default_flags[7] = {
	"in-room",
	"give",
	"held",
	"used",
	"produced",
	"final",
	"\n"
};

const char *mob_default_flags[6] = {
	"in-room",
	"used",
	"produced",
	"final",
	"owned",
	"\n"
};


const char *craft_mobile_flags[2] = {
	"set-owner",
	"\n"
};

const char *subcraft_flags[2] = {
	"hidden",
	"\n"
};

const char *attrs[9] = {
	"str",
	"dex",
	"con",
	"wil",
	"int",
	"agi",
	"luk",
	"pow",
	"\n"
};

	//TODO: add in checks for mob_items
int
missing_craft_items (CHAR_DATA * ch, AFFECTED_TYPE * af)
{
	char *p_char;
	bool missing_said = false;
	int i, j = 0;
	int item_required[MAX_ITEMS_PER_SUBCRAFT] = {0};
	int mob_required[MAX_MOBS_PER_SUBCRAFT] = {0};
	PHASE_DATA *phase = NULL;
	DEFAULT_ITEM_DATA *item;
	DEFAULT_MOB_DATA *mob_item;

	OBJ_DATA *obj_list[MAX_ITEMS_PER_SUBCRAFT] = {NULL};
	CHAR_DATA *mob_list[MAX_MOBS_PER_SUBCRAFT] = {NULL};


	if (!af->a.craft || !af->a.craft->subcraft)
		return 0;

	for (j = 1; j < MAX_PHASES_PER_SUBCRAFT; j++)
	{
		phase = af->a.craft->subcraft->phases[j];
		
		if (!phase)
			continue;
		
		for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
		{

			item = af->a.craft->subcraft->obj_items[i];

			if (!item)
			{
				continue;
			}

			if (!item->items[0])  /* No items in list.  Nothing required */
				continue;

			obj_list[i] = get_item_obj (ch, item, j);
		}

		
		for (i = 0; i < MAX_MOBS_PER_SUBCRAFT; i++)
		{
			
			mob_item = af->a.craft->subcraft->mob_items[i];
			
			if (!mob_item)
			{
				continue;
			}
			
			if (!mob_item->mobs[0])  /* No items in list.  Nothing required */
				continue;
			
			mob_list[i] = get_item_mob(ch, mob_item, j);
		}
		
		if (phase->first)
		{
			for (p_char = phase->first; *p_char; p_char++)
			{
				if (*p_char == '$')
				{
					p_char++;
					if (isdigit (*p_char) && atoi (p_char) < MAX_ITEMS_PER_SUBCRAFT)
						item_required[atoi (p_char)] = 1;
				}
				if (*p_char == '&')
				{
					p_char++;
					if (isdigit (*p_char) && atoi (p_char) < MAX_MOBS_PER_SUBCRAFT)
						mob_required[atoi (p_char)] = 1;
				}
			}
			
		}
	}

	for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{

		if (!item_required[i])
			continue;

		if (obj_list[i])
			continue;

		item = af->a.craft->subcraft->obj_items[i];

		if (!item)
		{
			continue;
		}

		if (item
			&& ((IS_SET (item->flags, SUBCRAFT_PRODUCED))
			|| (IS_SET (item->flags, SUBCRAFT_GIVE))
			|| (IS_SET (item->flags, SUBCRAFT_FINAL_PRODUCED))))
		{
			continue;
		}
		if (!missing_said)
		{
			missing_said = true;
			ch->send_to_char("\n#1You are missing one or more items:#0\n\n");
			ch->act("$n stops doing $s craft.", false, 0, 0, TO_ROOM);
			af->a.craft->timer = 0;
		}

		if (IS_SET (item->flags, SUBCRAFT_HELD))
			missing_item_msg (ch, item, "You must hold ");
		else if (IS_SET (item->flags, SUBCRAFT_IN_ROOM))
			missing_item_msg (ch, item, "In the room must be ");
		else
			missing_item_msg (ch, item, "You need ");
	}

	missing_said = false;
	for (i = 0; i < MAX_MOBS_PER_SUBCRAFT; i++)
	{
		
		if (!mob_required[i])
			continue;
		
		if (mob_list[i])
			continue;
		
		mob_item = af->a.craft->subcraft->mob_items[i];
		
		if (!mob_item)
		{
			continue;
		}
		
		if (mob_item
			&& (IS_SET (mob_item->flags, SUBCRAFT_MOB_PRODUCED)))
		{
			continue;
		}
		
		if (!missing_said)
		{
			missing_said = true;
			ch->send_to_char("\n#1You are missing one or more mobs:#0\n\n");
			ch->act("$n stops doing $s craft.", false, 0, 0, TO_ROOM);
			af->a.craft->timer = 0;
		}
		
		if (IS_SET (mob_item->flags, SUBCRAFT_IN_ROOM))
			missing_mob_msg (ch, mob_item, "In the room must be ");
		else
			missing_mob_msg (ch, mob_item, "You need ");
	}
	if (missing_said)
		return 1;

	return 0;
}

AFFECTED_TYPE *
is_craft_command (CHAR_DATA * ch, char *argument)
{
	int i;
	AFFECTED_TYPE *af;
	char command[MAX_STRING_LENGTH]= { '\0' };
	char subcraft_name[MAX_STRING_LENGTH]= { '\0' };
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	SUBCRAFT_HEAD_DATA *tcraft;
	
	if (IS_NPC (ch))
		return NULL;

	if ((!ch->get_trust()) && ch->room && IS_SET (ch->room->room_flags, OOC))
		return NULL;

	argument = one_argument (argument, command);
	sprintf (subcraft_name, "%s", argument);

	for (tcraft_iterator = craft_map.begin(); tcraft_iterator != craft_map.end(); tcraft_iterator++)
	{
		tcraft = tcraft_iterator->second;
		
		if (!str_cmp (command, tcraft->command) &&
			!str_cmp (subcraft_name, tcraft->subcraft_name))
			break;
	}
	if (tcraft_iterator == craft_map.end())
		return NULL;

	if (ch->get_trust())
	{
		for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
		{
			if (!(af = get_affect (ch, i)))
				break;
			else
			{
				if (af->a.craft->subcraft == tcraft)
				{
					return af;
				}
			}
		}
		magic_add_affect (ch, i, -1, 0, 0, 0, 0);
		af = get_affect (ch, i);
		af->a.craft = new affect_craft_type;
		af->a.craft->subcraft = tcraft;
		af->a.craft->phase_num = 0;
		af->a.craft->target_ch = NULL;
		af->a.craft->target_obj = NULL;
		af->a.craft->skill_check = 0;
		af->a.craft->timer = 0;
	}
	else
		for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
		{
			if (!(af = get_affect (ch, i)))
				continue;

			if (af->a.craft->subcraft == tcraft)
				break;
		}

		if (i > CRAFT_LAST)
			return NULL;

		return af;
}


	
void
craft_command (CHAR_DATA * ch, char *command_args,
			   AFFECTED_TYPE * craft_affect)
{
	char *argument;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char targ_buf[MAX_STRING_LENGTH]= { '\0' };
	SUBCRAFT_HEAD_DATA *subcraft;
	AFFECTED_TYPE *af;
	bool pass = false;
	int i = 0;

	subcraft = craft_affect->a.craft->subcraft;

	argument = one_argument (command_args, buf);	// Toss subcraft name 

	argument = one_argument (argument, targ_buf);	// Target object

		//can we see the target in the room
		
		//are we already crafting
	for (af = ch->hour_affects; af; af = af->next)
	{
		if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST
			&& af->a.craft->timer)
		{
			ch->send_to_char("You are already crafting something.\n");
			return;
		}
	}

	
		//Are we recovering from a prior craft
	if (get_affect (ch, MAGIC_CRAFT_DELAY)
		&& (craft_affect->a.craft->subcraft->delay) && (!ch->get_trust()))
	{
		ch->act
			("Sorry, but your OOC craft delay timer is still in place. You'll receive a notification when it expires and you're free to craft delayed items again.",
			false, 0, 0, TO_CHAR | _ACT_FORMAT);
		return;
	}

	/** terrains **/
	pass = true;
	for (i = 0; i < TERRAINSMAX; i++)
	{
		if ((craft_affect->a.craft->subcraft->terrains[i] > 0)
			&& (craft_affect->a.craft->subcraft->terrains[i] - 1 !=
			ch->room->terrain_type))
				pass = false;
		
	}
	if (!pass)
	{
		ch->send_to_char("That craft cannot be performed in this sort of terrain.\n");
		return;
	}
	

	/** seasons **/
	pass = true;
	for (i = 0; i < SEASONSMAX; i++)
	{
		if ((craft_affect->a.craft->subcraft->seasons[i])
			&& (craft_affect->a.craft->subcraft->seasons[i] - 1 !=
				time_info.season))
			pass = false;
	}
	if (!pass)
	{
		ch->send_to_char("That craft cannot be performed during this season.\n");
		return;
	}
	
	/** weather **/
	pass = true;
	for (i = 0; i < WEATHERMAX; i++)
	{
		if ((craft_affect->a.craft->subcraft->weather[i])
			&& (craft_affect->a.craft->subcraft->weather[i] - 1 !=
			weather_info[ch->room->wzone].state))
			pass = false;
	}
	if (!pass)
	{
		ch->send_to_char("That craft cannot be performed in this weather.\n");
		return;
	}
	

	/** other requirements **/
	if (missing_craft_items (ch, craft_affect))
		return;

	if (num_followers(ch) < craft_affect->a.craft->subcraft->followers)
	{
		ch->send_to_char("You do not have enough followers.\n");
		return;
	}

	craft_affect->a.craft->phase_num = 0;

	sprintf( buf, "%s %s", 
			craft_affect->a.craft->subcraft->command, 
			craft_affect->a.craft->subcraft->subcraft_name);
	player_log( ch, "[CRAFT]", buf );
	

	activate_phase (ch, craft_affect);
}

void
do_materials (CHAR_DATA * ch, char *argument, int cmd)
{
	int i, j;
	AFFECTED_TYPE *af;
	SUBCRAFT_HEAD_DATA *tcraft;
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	PHASE_DATA *phase;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	extern const char *attrs[9];

	
	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char("For which craft did you wish to obtain a materials list?\n");
		return;
	}

	for (tcraft_iterator = craft_map.begin(); tcraft_iterator != craft_map.end(); tcraft_iterator++)
	{
		tcraft = tcraft_iterator->second;
		if (!str_cmp (tcraft->subcraft_name, buf))
			break;
	}
	

	if (!tcraft)
	{
		ch->send_to_char("No such craft.  Type 'crafts' for a listing.\n");
		return;
	}

	for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
	{
		if (!(af = get_affect (ch, i)))
			continue;
		if (af->a.craft->subcraft == tcraft)
			break;
	}

	if (i > CRAFT_LAST && (!ch->get_trust()))
	{
		ch->send_to_char("That craft is not one your character has knowledge of.\n");
		return;
	}

	ch->send_to_char("\n");

	sprintf (buf, "#6%s:#0 %s %s\n", tcraft->craft_name, tcraft->command,
		tcraft->subcraft_name);
	buf[2] = toupper (buf[2]);

	ch->send_to_char(buf);

	if (ch->get_trust())
	{
		*buf = '\0';
		for (i = 0; i <= OPENINGMAX; i++)
		{
			if (tcraft->opening[i] > 0)
				sprintf (buf + strlen (buf), " %s", lookup_skill_name(tcraft->opening[i]));
		}
		if (*buf)
		{
			sprintf (buf2, "#6Opening Craft For:#0%s\n", buf);
			ch->send_to_char(buf2);
		}
	}

	/** races **/
	*buf = '\0';
	for (i = 0; i <= RACEMAX; i++)
	{
		if (tcraft->race[i]  > 0)
			sprintf (buf + strlen (buf), " %s",
					 lookup_race_variable (tcraft->race[i] - 1, RACE_NAME));
	}
	if (*buf)
	{
		sprintf (buf2, "#6Available To:#0%s\n", buf);
		ch->send_to_char(buf2);
	}

	/** clans **/
		//TODO: needs to be re-written to use new clan system
	*buf = '\0';
	if (tcraft->clans && *tcraft->clans)
	{
		/**********
		sprintf (buf, "#6Required Clans:#0 ");
		p = tcraft->clans;
		while (get_next_clan (&p, clan_name, &clan_flags))
		{
			clan_def = get_clandef (clan_name);
			if (!clan_def)
				continue;
			if (!first)
				sprintf (buf + strlen (buf), "                ");
			sprintf (buf + strlen (buf), "%-30s [%s]\n", clan_def->literal,
				rank_name_from_flags (clan_name, clan_flags));
			first = false;
		}
		 ***************/
		ch->send_to_char(buf);
	}

	/** terrains **/
	*buf = '\0';
	for (i = 0; i <= TERRAINSMAX; i++)
	{
		if (tcraft->terrains[i]  > 0)
			sprintf (buf + strlen (buf), " %s",
					 terrain_types[tcraft->terrains[i] - 1]);
	}
	if (*buf)
	{
		sprintf (buf2, "#6Only Usable In:#0%s\n", buf);
		ch->send_to_char(buf2);
	}
	
	
	/** seasons **/
	*buf = '\0';
	for (i = 0; i <= SEASONSMAX; i++)
	{
		if (tcraft->seasons[i]  > 0)
			sprintf (buf + strlen (buf), " %s",
					 seasons[tcraft->seasons[i] - 1]);
	}
	if (*buf)
	{
		sprintf (buf2, "#6Usable During:#0%s\n", buf);
		ch->send_to_char(buf2);
	}
	
	
	/** group **/
	if (tcraft->followers > 0)
	{
		sprintf (buf, "#6Requires#0 %d followers.\n",
			tcraft->followers);
		ch->send_to_char(buf);
	}

	/**weather*/
	*buf = '\0';
	for (i = 0; i <= WEATHERMAX; i++)
	{
		if (tcraft->weather[i]  > 0)
			sprintf (buf + strlen (buf), " %s",
					 weather_states[tcraft->weather[i] - 1]);
	}
	if (*buf)
	{
		sprintf (buf2, "#6Usable During:#0%s\n", buf);
		ch->send_to_char(buf2);
	}
	
	
	/** Phases information **/
	for (j = 1; j < MAX_PHASES_PER_SUBCRAFT; j++)
	{
		phase = tcraft->phases[j];
		if (!phase)
			continue;
		
		sprintf (buf, "#5Phase %d:#0  %d seconds",
			j, phase->phase_seconds);

		if ( phase->skill  > 0)
		{
			sprintf (buf + strlen (buf), ", %s skill utilized.\n",
				lookup_skill_name(phase->skill));
		}
		else if ( phase->attribute > -1 )
		{
			sprintf (buf + strlen (buf), ", %s attribute tested.\n",
				attrs[phase->attribute]);
		}
		else
		{
			sprintf (buf + strlen (buf), "\n");
		}

		ch->send_to_char(buf);


		if ( ch->get_trust() > 1 )
		{
			if (phase->attribute > -1)
			{
				sprintf (buf, "Attribute: %s vs %dd%d\n",
					attrs [phase->attribute],
					phase->dice,
					phase->sides);
				ch->send_to_char(buf);
			}

			if (phase->skill > 0)
			{
				sprintf (buf, "Skill: %s vs %dd%d\n",
					lookup_skill_name(phase->skill),
					phase->dice,
					phase->sides);
				ch->send_to_char(buf);
			}
		}
	}

	if (tcraft->delay > 0)
	{
		sprintf (buf, "#6OOC Delay Timer:#0 %d RL Minutes\n", tcraft->delay);
		ch->send_to_char(buf);
	}

	if (tcraft->key_first > 0)
	{
		sprintf (buf, "#6Primary Key:#0 %d\n", tcraft->key_first);
		ch->send_to_char(buf);
	}

	if (tcraft->key_end > 0)
	{
		sprintf (buf, "#6Product Key:#0 %d\n", tcraft->key_end);
		ch->send_to_char(buf);
	}

	
	for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{
		if (tcraft->obj_items[i])
			
		{
			if (IS_SET (tcraft->obj_items[i]->flags, SUBCRAFT_HELD))
				missing_item_msg (ch, tcraft->obj_items[i],
								  "Held (Reusable):  ");
			else if (IS_SET (tcraft->obj_items[i]->flags, SUBCRAFT_IN_ROOM))
				missing_item_msg (ch, tcraft->obj_items[i],
								  "In Room (Reusable):  ");
			else if (IS_SET (tcraft->obj_items[i]->flags, SUBCRAFT_USED))
				missing_item_msg (ch, tcraft->obj_items[i],
								  "Held or in Room (Consumed):  ");
			else if (IS_SET (tcraft->obj_items[i]->flags, SUBCRAFT_PRODUCED))
				missing_item_msg (ch, tcraft->obj_items[i],
								  "#6Produced:#0  ");
			else if (IS_SET (tcraft->obj_items[i]->flags, SUBCRAFT_FINAL_PRODUCED))
				missing_item_msg (ch, tcraft->obj_items[i],
								  "#6Final Produced:#0  ");
			else if (IS_SET (tcraft->obj_items[i]->flags, SUBCRAFT_GIVE))
				missing_item_msg (ch, tcraft->obj_items[i],
								  "#6Produced-to-hand:#0  ");
		}
		
	}
	
		
	for (i = 0; i < MAX_MOBS_PER_SUBCRAFT; i++)
	{
		if (tcraft->mob_items[i])
		{	
			if (IS_SET (tcraft->mob_items[i]->flags, SUBCRAFT_MOB_IN_ROOM))
			{
				missing_mob_msg (ch, tcraft->mob_items[i],
								 "#6Mob In Room (Reusable):#0  ");
			}
			else if (IS_SET (tcraft->mob_items[i]->flags, SUBCRAFT_MOB_USED))
			{
				missing_mob_msg (ch, tcraft->mob_items[i],
								 "#6Mob in Room (Consumed):#0  ");
			}
			else if (IS_SET (tcraft->mob_items[i]->flags, SUBCRAFT_MOB_PRODUCED))
			{
				missing_mob_msg (ch, tcraft->mob_items[i],
								 "#6Mob Produced:#0  ");
			}
			else if (IS_SET (tcraft->mob_items[i]->flags, SUBCRAFT_MOB_FINAL_PRODUCED))
			{
				missing_mob_msg (ch, tcraft->mob_items[i],
								 "#6Mob Final Produced:#0  ");
			}
			else if (IS_SET (tcraft->mob_items[i]->flags, SUBCRAFT_MOB_OWNED))
			{
				missing_mob_msg (ch, tcraft->mob_items[i],
								 "#6Mob Produced (Owned):#0  ");
				
			}
			
		}
		
	}

}

	//list crafts by catagory
	//lists the catagories
	//lists crafts within a catagory
	//mortal and immortal versions
	//**** NOTE: this does not start editing a craft. Use the cset commands
	//TODO: make certain "cset <craftname>" works and allows editing to start
void
do_crafts (CHAR_DATA * ch, char *argument, int cmd)
{
	int has_a_craft = 0;
	int i = 0, j = 0;
	AFFECTED_TYPE *af;
	SUBCRAFT_HEAD_DATA *craft;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char name[MAX_STRING_LENGTH]= { '\0' };
	std::string output;
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	
	if (IS_NPC (ch))
	{
		ch->send_to_char("You shouldn't be using this command...\n");
		return;
	}

	argument = one_argument (argument, buf);

		//"craft"
		//"craft all"
		//"craft woodworking"
	// Mortals - listing catagories
	if ((!ch->get_trust()))
	{
		//"craft"
		if (!*buf)
		{
			output.assign("You currently have crafts in the following areas:\n\n");
			for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
			{
				if ((af = get_affect (ch, i)))
				{
					sprintf(buf2, "  #6%-20s#0",
						af->a.craft->subcraft->craft_name);

					if (output.find(buf2, 0, strlen(buf2)) == std::string::npos)
					{
						j++;
						output.append(buf2);

						if (!(j % 3))
							output.append("\n");
					}

					has_a_craft = 1;
				}
			}

			if ((j % 3))
				output.append("\n");

			if (!has_a_craft)
				ch->send_to_char("You have no knowledge of any crafts.\n");
			else
				page_string (ch->desc, output.c_str());
		}
		
		//"craft <catagory>"
		//"craft all"
		//displayed in 3 columns
		else
		{
			j = 0;
			if (!str_cmp (buf, "all"))
				output.assign("You know the following crafts:\n\n");
			else
			{
				output.assign ("You know the following ");
				sprintf(buf2, "#6%s#0", buf);
				output.append (buf2);
				output.append (" crafts:\n\n");
			}

			for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
			{
				if ((af = get_affect (ch, i)))
				{
					if (str_cmp (buf, "all")
						&& str_cmp (buf, af->a.craft->subcraft->craft_name))
						continue;
					
					if (output.find(af->a.craft->subcraft->subcraft_name, 0, strlen(af->a.craft->subcraft->subcraft_name)) == std::string::npos)
					{
						j++;
						sprintf (name, "%s %s",
								 af->a.craft->subcraft->command,
								 af->a.craft->subcraft->subcraft_name);
						sprintf (buf2, "   #6%-30s#0", name);
						output.append(buf2);
						
						has_a_craft = 1;
						
						if (!(j % 2))
							output.append("\n");
					}
				}
			}
			
			if ((j % 2))

				output.append("\n");

			if (!has_a_craft)
				ch->send_to_char("You have no knowledge of any crafts.\n");
			else
				page_string (ch->desc, output.c_str());
		} 
		
		return;
	} //if ((!ch->get_trust()))

	// Immortal options
	// "craft"
	//list catagories
	if (!*buf)
	{
		sprintf (buf, "\nWe have the following crafts:\n\n");
		output.assign(buf);
		j = 0;
		
		for (tcraft_iterator = craft_map.begin(); tcraft_iterator != craft_map.end(); tcraft_iterator++)
		{
			craft = tcraft_iterator->second;
			sprintf(buf2, "  #6%-20s#0", craft->craft_name);
			
			if (output.find(buf2, 0, strlen(buf2)) == std::string::npos)
			{
				j++;
				output.append(buf2);
				
				if (!(j % 3))
					output.append("\n");
			}
		}
		
		
		page_string (ch->desc, output.c_str());
		return;
		
	}
	
	//"craft all" - list all crafts in all catagories
	else if (!str_cmp (buf, "all"))
	{
		list_all_crafts (ch);
		return;
	}

	//"craft <catagory>"
	else
	{
		sprintf (buf2, "\nWe have the following #6%s#0 crafts:\n\n", buf);
		output.assign(buf2);
		
		for (tcraft_iterator = craft_map.begin(); tcraft_iterator != craft_map.end(); tcraft_iterator++)
		{
			craft = tcraft_iterator->second;
			
			if (!str_cmp (craft->craft_name, buf))
			{
				sprintf (buf2,
						 "#6Subcraft:#0 %-24s #6Command:#0 %-20s\n",
						 craft->subcraft_name,
						 craft->command);
				output.append(buf2);
				
			}
		}
		
		page_string (ch->desc, output.c_str());
		return;
		
	}
		

	return;
}

void
do_remcraft (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *edit_mob;
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *next_affect;
	SUBCRAFT_HEAD_DATA *tcraft;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	bool craft_category = false;
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	
	argument = one_argument (argument, buf);

	if (*buf == '?')
	{
		ch->send_to_char("Start by using the mob command on a PC.\n");
		ch->send_to_char("\n");
		ch->send_to_char("remcraft <subcraft-name>            Use crafts to get names\n");
		ch->send_to_char("\n");
		ch->send_to_char("   remcraft swordblank \n");
		ch->send_to_char("\n");
		return;
	}

	if (IS_NPC (ch))
	{
		ch->send_to_char("This command cannot be used while switched.\n");
		return;
	}

	if (!(edit_mob = ch->pc->edit_player))
	{
		ch->send_to_char("Start by using the MOB command on a PC.\n");
		return;
	}

	for (tcraft_iterator = craft_map.begin(); tcraft_iterator != craft_map.end(); tcraft_iterator++)
	{
		tcraft = tcraft_iterator->second;
		if (!str_cmp (tcraft->subcraft_name, buf))
			break;
		if (!str_cmp (tcraft->craft_name, buf))
		{
			craft_category = true;
			break;
		}
	}

	if (!str_cmp (buf, "all"))
		craft_category = true;

	if (!tcraft && !craft_category)
	{
		ch->send_to_char("No such craft.  Type 'crafts' for a listing.\n");
		return;
	}

	for (af = edit_mob->hour_affects; af; af = next_affect)
	{

		next_affect = af->next;

		if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST)
		{
			if (!str_cmp (buf, "all")
				|| !str_cmp (af->a.craft->subcraft->craft_name, buf))
			{
				remove_affect_type (edit_mob, af->type);
				continue;
			}
			if (!str_cmp (af->a.craft->subcraft->subcraft_name, buf))
			{
				remove_affect_type (edit_mob, af->type);
				ch->send_to_char("Ok.\n");
				break;
			}
		}
	}

	if (craft_category)
		ch->send_to_char("Craft category removed.\n");
}

void
do_addcraft (CHAR_DATA * ch, char *argument, int cmd)
{
	int i;
	CHAR_DATA *edit_mob;
	AFFECTED_TYPE *af, *next_affect;
	SUBCRAFT_HEAD_DATA *craft;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	bool craft_category = false;
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	
	argument = one_argument (argument, buf);

	if (*buf == '?')
	{
		ch->send_to_char("Start by using the mob command on a PC.\n\n");
		ch->send_to_char("addcraft <subcraft-name>            Use crafts to get names\n");
		ch->send_to_char("Example:\n");
		ch->send_to_char("   addcraft swordblank\n\n");
		return;
	}

	if (IS_NPC (ch))
	{
		ch->send_to_char("This command cannot be used while switched.\n");
		return;
	}

	if (!(edit_mob = ch->pc->edit_player))
	{
		ch->send_to_char("Start by using the MOB command on a PC.\n");
		return;
	}

	for (tcraft_iterator = craft_map.begin(); tcraft_iterator != craft_map.end(); tcraft_iterator++)
	{
		craft = tcraft_iterator->second;
		
		if (!str_cmp (craft->subcraft_name, buf))
			break;
		if (!str_cmp (craft->craft_name, buf))
			craft_category = true;
	}

	if (!str_cmp (buf, "all"))
		craft_category = true;

	if (!craft && !craft_category)
	{
		ch->send_to_char("No such craft or subcraft.  Type 'crafts' for a listing.\n");
		return;
	}

	for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
		if (!get_affect (edit_mob, i))
			break;

	if (i > CRAFT_LAST)
	{
		ch->send_to_char("Sorry, this PC already has the maximum allowed crafts"
			".\n");
		return;
	}

	if (craft_category)
	{
		for (af = edit_mob->hour_affects; af; af = next_affect)
		{
			next_affect = af->next;
			if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST)
			{
				if (!str_cmp (af->a.craft->subcraft->craft_name, buf))
					remove_affect_type (edit_mob, af->type);
			}
		}
		for (tcraft_iterator = craft_map.begin(); tcraft_iterator != craft_map.end(); tcraft_iterator++)
		{
			craft = tcraft_iterator->second;
			
			if (!strcmp ("all", buf) || !strcmp (craft->craft_name, buf))
			{
				for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
					if (!get_affect (edit_mob, i))
						break;
				magic_add_affect (edit_mob, i, -1, 0, 0, 0, 0);
				af = get_affect (edit_mob, i);
				af->a.craft = new affect_craft_type;
				af->a.craft->subcraft = craft;
				af->a.craft->phase_num = 0;
				af->a.craft->target_ch = NULL;
				af->a.craft->target_obj = NULL;
				af->a.craft->skill_check = 0;
				af->a.craft->timer = 0;
			}
		}
	}
	else
	{
		magic_add_affect (edit_mob, i, -1, 0, 0, 0, 0);
		af = get_affect (edit_mob, i);
		af->a.craft = new affect_craft_type;
		af->a.craft->subcraft = craft;
		af->a.craft->phase_num = 0;
		af->a.craft->target_ch = NULL;
		af->a.craft->target_obj = NULL;
		af->a.craft->skill_check = 0;
		af->a.craft->timer = 0;
	}

	ch->send_to_char("Craft(s) added.\n");
}

	//adds General crafts and all crafts that are opening for skills the PC has
void
update_crafts (CHAR_DATA * ch)
{
		//skipping this for now, until crafts are completed
	return;
	
	SUBCRAFT_HEAD_DATA *craft;
	AFFECTED_TYPE *af;
	int i = 0;
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	
	for (tcraft_iterator = craft_map.begin(); tcraft_iterator != craft_map.end(); tcraft_iterator++)
	{
		craft = tcraft_iterator->second;
		
		if (((!strcmp (craft->craft_name, "general-fire")) ||
			(!strcmp (craft->craft_name, "general-labor")) ||
			(!strcmp (craft->craft_name, "general-food")) ||
			(!strcmp (craft->craft_name, "general-recreation")) ||
			(!strcmp (craft->craft_name, "general-serving")) ||
			(!strcmp (craft->craft_name, "general-cleaning")) ||
			(!strcmp (craft->craft_name, "general")))
			&& !has_craft (ch, craft)
			&& has_required_crafting_skills (ch, craft))
		{
			for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
				if (!get_affect (ch, i))
					break;
			magic_add_affect (ch, i, -1, 0, 0, 0, 0);
			af = get_affect (ch, i);
			af->a.craft = new affect_craft_type;
			af->a.craft->subcraft = craft;
			af->a.craft->phase_num = 0;
			af->a.craft->target_ch = NULL;
			af->a.craft->target_obj = NULL;
			af->a.craft->skill_check = 0;
			af->a.craft->timer = 0;
		}
		if (is_opening_craft (ch, craft)
			&& has_required_crafting_skills (ch, craft)
			&& !has_craft (ch, craft))
		{
			for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
				if (!get_affect (ch, i))
					break;
			magic_add_affect (ch, i, -1, 0, 0, 0, 0);
			af = get_affect (ch, i);
			af->a.craft = new affect_craft_type;
			af->a.craft->subcraft = craft;
			af->a.craft->phase_num = 0;
			af->a.craft->target_ch = NULL;
			af->a.craft->target_obj = NULL;
			af->a.craft->skill_check = 0;
			af->a.craft->timer = 0;
		}
	}
}

AFFECTED_TYPE *
is_crafting (CHAR_DATA * ch)
{
	AFFECTED_TYPE *af, *next_af;

	for (af = ch->hour_affects; af; af = next_af)
	{
		next_af = af->next;
		if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST
			&& af->a.craft->timer)
		{
			return af;
		}
	}

	return NULL;
}

/* Checks clan requirements for craft branch */

int
meets_clan_requirements (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
	char clan_name[MAX_STRING_LENGTH]= { '\0' };
	char *c1;
	int flags = 0, flags2 = 0;
	bool required_clan = false;

	if (craft->clans && strlen (craft->clans) > 3)
	{
		required_clan = true;
		for (c1 = craft->clans; get_next_clan (&c1, clan_name, &flags);)
		{
			if (get_clan (ch, clan_name, flags2) && flags2 >= flags)
				required_clan = false;
		}
	}

	if (required_clan)
		return 0;

	return 1;
}

/* Checks to see if race requirements for craft branch are met */

int
meets_race_requirements (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
	int i;
	bool required_race = false;

	for (i = 0; i <= 24; i++)
	{
		if (craft->race[i])
			required_race = true;
	}

	if (required_race)
	{
		for (i = 0; i <= 24; i++)
		{
			if (craft->race[i] && ch->race == craft->race[i] - 1)
				required_race = false;
		}
	}

	if (required_race)
		return 0;

	return 1;
}



/* Makes sure PC has all absolute baseline specs necessary for a given craft */

int
has_required_crafting_skills (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
	PHASE_DATA *phase;
	int i = 0;
	char* skill_name;

	if (!meets_race_requirements (ch, craft))
		return 0;

	if (!meets_clan_requirements (ch, craft))
		return 0;

	for (i = 0; i <= OPENINGMAX; i++)
	{
		skill_name = strdup(lookup_skill_name(craft->opening[i]));
		if ((craft->opening[i] > 0)
			&& (ch->skill_map[skill_name] < 1))
			return 0;
	}

	for (i = 1; i < MAX_PHASES_PER_SUBCRAFT; i++)
	{
		phase = craft->phases[i];
		skill_name = strdup(lookup_skill_name(phase->skill));
		if (phase->skill)
		{
			if ((ch->skill_map[skill_name] < 1))
				return 0;
		}
	}

	return 1;

}

int
is_opening_craft (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
	int i = 0;
	char* skill_name;
	
	for (i = 0; i <= 24; i++)
	{
		skill_name = strdup(lookup_skill_name(craft->opening[i]));
		if ((craft->opening[i] > 0)
			&& (ch->skill_map[skill_name] > 0))
			return 1;
	}

	return 0;

}

char *
craft_argument (char *argument, char *arg_first)
{
	char cEnd;

	if (argument == NULL)
		return "";

	while (isspace (*argument))
		argument++;

	if (*argument == '(')
	{
		argument++;
		strcpy (arg_first, "(");
		return argument;
	}

	if (*argument == ')')
	{
		argument++;
		strcpy (arg_first, ")");
		return argument;
	}

	cEnd = ' ';

	while (*argument != '\0')
	{

		if (*argument == '(' || *argument == ')')
			break;

		if (*argument == cEnd)
		{
			argument++;
			break;
		}

		if (cEnd == ' ')
			*arg_first = tolower (*argument);
		else
			*arg_first = *argument;

		arg_first++;
		argument++;
	}

	*arg_first = '\0';

	while (isspace (*argument))
		argument++;

	return argument;
}

void
add_to_default_list (DEFAULT_ITEM_DATA * items, char *flag_vnum)
{
	int ind;
	int ovnum;
	int i;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (!items->item_counts)
		items->item_counts = 1;

	if (*flag_vnum == '(' || *flag_vnum == ')')
		return;

	/* '-' means to erase all vnums */

	if (*flag_vnum == '-')
	{
		memset (items->items, 0, sizeof(int)*MAX_DEFAULT_ITEMS);
		return;
	}

	if ((*flag_vnum == 'x' || *flag_vnum == 'X') && isdigit (flag_vnum[1]))
	{
		sprintf (buf, "%c", flag_vnum[1]);
		if (flag_vnum[2] && isdigit (flag_vnum[2]))
			sprintf (buf + strlen (buf), "%c", flag_vnum[2]);
		if (flag_vnum[3] && isdigit (flag_vnum[3]))
			sprintf (buf + strlen (buf), "%c", flag_vnum[3]);
		if (flag_vnum[4] && isdigit (flag_vnum[4]))
			sprintf (buf + strlen (buf), "%c", flag_vnum[4]);
		items->item_counts = atoi (buf);
		return;
	}

	if (!isdigit (*flag_vnum))
	{
		if ((ind = index_lookup (item_default_flags, flag_vnum)) == -1)
		{
			;
		}
		else
			items->flags |= (1 << ind);

		return;
	}

	ovnum = atoi (flag_vnum);

	if (!ovnum || !vtoo (ovnum))
	{
		sprintf (buf, "NOTE:  vnum %s does not exist for CRAFTS!", flag_vnum);
		system_log (buf, true);
	}

	for (i = 0; i < MAX_DEFAULT_ITEMS; i++)
	{

		if (!items->items[i])
		{
			items->items[i] = ovnum;
			break;
		}

		if (items->items[i] == ovnum)
		{
			break;
		}
	}

	if (i >= MAX_DEFAULT_ITEMS)
	{
		system_log ("WARNING:  Too many items specified in default item list!",
			true);
		sprintf (buf, "Item %d not added.  Max allowed in list is %d items.",
			ovnum, MAX_DEFAULT_ITEMS);
		system_log (buf, true);
	}
}

void
add_to_mob_default_list (DEFAULT_MOB_DATA * mobs, char *flag_vnum)
{
	int ind;
	int mvnum;
	int i;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	
	if (!mobs->mob_counts)
		mobs->mob_counts = 1;
	
	if (*flag_vnum == '(' || *flag_vnum == ')')
		return;
	
	/* '-' means to erase all vnums */
	
	if (*flag_vnum == '-')
	{
		memset (mobs->mobs, 0, sizeof(int)*MAX_DEFAULT_MOBS);
		return;
	}
	
	if ((*flag_vnum == 'x' || *flag_vnum == 'X') && isdigit (flag_vnum[1]))
	{
		sprintf (buf, "%c", flag_vnum[1]);
		if (flag_vnum[2] && isdigit (flag_vnum[2]))
			sprintf (buf + strlen (buf), "%c", flag_vnum[2]);
		if (flag_vnum[3] && isdigit (flag_vnum[3]))
			sprintf (buf + strlen (buf), "%c", flag_vnum[3]);
		if (flag_vnum[4] && isdigit (flag_vnum[4]))
			sprintf (buf + strlen (buf), "%c", flag_vnum[4]);
		mobs->mob_counts = atoi (buf);
		return;
	}
	
	if (!isdigit (*flag_vnum))
	{
		if ((ind = index_lookup (mob_default_flags, flag_vnum)) == -1)
		{
			;
		}
		else
			mobs->flags |= (1 << ind);
		
		return;
	}
	
	mvnum = atoi (flag_vnum);
	
	if (!mvnum || !vtom(mvnum))
	{
		sprintf (buf, "NOTE:  Mobile vnum %s does not exist for CRAFTS!", flag_vnum);
		system_log (buf, true);
	}
	
	for (i = 0; i < MAX_DEFAULT_MOBS; i++)
	{
		
		if (!mobs->mobs[i])
		{
			mobs->mobs[i] = mvnum;
			break;
		}
		
		if (mobs->mobs[i] == mvnum)
		{
			break;
		}
	}
	
	if (i >= MAX_DEFAULT_MOBS)
	{
		system_log ("WARNING:  Too many mobs specified in default mob list!",
					true);
		sprintf (buf, "Mob %d not added.  Max allowed in list is %d mobs.",
				 mvnum, MAX_DEFAULT_MOBS);
		system_log (buf, true);
	}
}

char *
read_item_list (DEFAULT_ITEM_DATA ** items, char *list, int phase_num)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char *argument;
	DEFAULT_ITEM_DATA *deflt;

	*items = new DEFAULT_ITEM_DATA;

	deflt = *items;

	deflt->phase_num = phase_num;

	argument = craft_argument (list, buf);

	if (!*buf)
		return argument;

	if (*buf != '(')
	{
		add_to_default_list (deflt, buf);
		return argument;
	}

	do
	{
		argument = craft_argument (argument, buf);
		add_to_default_list (deflt, buf);
	}
	while (*buf != ')');

	return argument;
}

char *
read_mob_list (DEFAULT_MOB_DATA ** mobs, char *list, int phase_num)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char *argument;
	DEFAULT_MOB_DATA *deflt;
	
	*mobs = new DEFAULT_MOB_DATA;
	
	deflt = *mobs;
	
	deflt->phase_num = phase_num;
	
	argument = craft_argument (list, buf);
	
	if (!*buf)
		return argument;
	
	if (*buf != '(')
	{
		add_to_mob_default_list (deflt, buf);
		return argument;
	}
	
	do
	{
		argument = craft_argument (argument, buf);
		add_to_mob_default_list (deflt, buf);
	}
	while (*buf != ')');
	
	return argument;
}

char *
read_extended_text (FILE * fp, char *first_line)
{
	int continues;
	char line[MAX_STRING_LENGTH]= { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char *buf_end = '\0';
	char *buf_start = '\0';

	*buf = '\0';
	*line = '\0';

	strcpy (buf, first_line);

	*line = '\0';

	while (1)
	{

		continues = 0;

		for (buf_start = buf; *buf_start == ' ';)
			buf_start++;

		buf_end = buf_start + strlen (buf_start) - 1;

		while (1)
		{

			if (buf_end == buf_start)
				break;

			if (*buf_end == ' ' || *buf_end == '\n' || *buf_end == '\r')
				buf_end--;

			else if (*buf_end == '\\')
			{
				continues = 1;
				*buf_end = '\0';
				break;
			}

			else
				break;

			buf_end[1] = '\0';
		}

		strcat (line, buf_start);

		if (!continues)
			break;

		fgets (buf, MAX_STRING_LENGTH - 1, fp);
	}

	return duplicateString (line);
}



void
boot_crafts ()
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	MYSQL_RES *result;
	MYSQL_ROW row;
	
	mysql_safe_query ("SELECT craft_id FROM crafts WHERE status != 'declined'");
	result = mysql_store_result (database);
	
	if(result == false)
	{
		sprintf(buf, "boot_crafts: mysql error number: %s\n", mysql_error(database));
		system_log (buf, true);
		return;
	}
	
	if (result && mysql_num_rows (result) >= 1)
	{				
		while ((row = mysql_fetch_row(result)))
		{
			load_subcraft_mysql(atoi(row[0]));
		}
		mysql_free_result (result);
	}
	
	return;

}

void
craft_prepare_message (CHAR_DATA * ch, char *message, CHAR_DATA * n,
					   CHAR_DATA * N, CHAR_DATA * T, char *phase_msg,
					   OBJ_DATA ** obj_list,
					   CHAR_DATA ** mob_list)
{
	int ovnum;
	int mvnum;
	char *point;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	*buf = '\0';

	for (point = buf; *phase_msg;)
	{
		if (*phase_msg == '$' && isdigit (phase_msg[1]))
		{
			phase_msg++;
			if (isdigit (*phase_msg) &&
				(ovnum = atoi (phase_msg)) < MAX_ITEMS_PER_SUBCRAFT)
			{
				sprintf (point, "#2%s#0", OBJS (obj_list[ovnum], ch));
				point = point + strlen (point);
			}

			while (isdigit (*phase_msg))
				phase_msg++;
		}

		else if (*phase_msg == '@' && isdigit(phase_msg[1]))
		{
			phase_msg++;
			if (isdigit (*phase_msg) &&
				(mvnum = atoi (phase_msg)) < MAX_MOBS_PER_SUBCRAFT)
			{
				sprintf (point, "#5%s#0", PERS (mob_list[mvnum], ch));
				point = point + strlen (point);
			}
			
			while (isdigit(*phase_msg))
				phase_msg++;
		}
		
		else if (*phase_msg == '$')
		{

			switch (phase_msg[1])
			{
			case 'e':
				sprintf (point, "#5%s#0", n ? HSSH (n) : "HSSH-e");
				break;
			case 'm':
				sprintf (point, "#5%s#0", n ? HMHR (n) : "HMHR-m");
				break;
			case 's':
				sprintf (point, "#5%s#0", n ? HSHR (n) : "HSHR-s");
				break;
			case 'E':
				sprintf (point, "#5%s#0", N ? HSSH (N) : "HSSH-E");
				break;
			case 'M':
				sprintf (point, "#5%s#0", N ? HMHR (N) : "HMHR-M");
				break;
			case 'S':
				sprintf (point, "#5%s#0", N ? HSHR (n) : "HSHR-S");
				break;
			case 'T':
				sprintf (point, "#5%s#0", T ? PERS (T, ch) : "SOMEBODY-T");
				break;
			case 'n':
				sprintf (point, "#5%s#0", n ? PERS (n, ch) : "SOMEBODY-n");
				break;
			case 'N':
				sprintf (point, "#5%s#0", N ? PERS (N, ch) : "SOMEBODY-N");
				break;
			}

			phase_msg += 2;
			point = point + strlen (point);
		}

		else
		{
			*(point++) = *(phase_msg++);
			*point = '\0';
		}
	}

	strcpy (message, buf);
}

OBJ_DATA *
obj_list_vnum (CHAR_DATA * ch, OBJ_DATA * list, int vnum)
{
	for (; list; list = list->next_content)
		if (can_see_obj(ch, list))
			return list;

	return NULL;
}

CHAR_DATA *
mob_list_vnum (CHAR_DATA * ch, CHAR_DATA * mlist, int vnum)
{
	CHAR_DATA * tch;
	
	for (tch = mlist; tch; tch = tch->next_in_room)
	{
		if (tch == ch)
			continue;
		if (get_char_room_vis(ch, tch->keywords) && (tch->mob->nVirtual == vnum))
			return tch;
	}		
	return NULL;
}

OBJ_DATA *
obj_list_vnum_dark (OBJ_DATA * list, int vnum)
{
	for (; list; list = list->next_content)
		if (list->nVirtual == vnum)
			return list;

	return NULL;
}


int
craft__count_available_objs (CHAR_DATA * ch, int vnum) {
	int nTally = 0;
	OBJ_DATA *ptrObj = NULL;

	nTally += (ch->right_hand) ? ch->right_hand->count : 0;
	nTally += (ch->left_hand) ? ch->left_hand->count : 0;

	for (ptrObj = ch->room->contents; ptrObj != NULL; ptrObj = ptrObj->next_content)
	{
		nTally += (ptrObj->count) ? ptrObj->count : 0;
	}

	return nTally;
}

OBJ_DATA *
get_item_obj (CHAR_DATA * ch, DEFAULT_ITEM_DATA * item, int phase_num)
{
	int j;
	int vnum;
	OBJ_DATA *tobj;

	for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
	{

		if (!(vnum = item->items[j]))
			continue;

		if ((IS_SET (item->flags, SUBCRAFT_PRODUCED))
			|| (IS_SET (item->flags, SUBCRAFT_FINAL_PRODUCED))
			|| (IS_SET (item->flags, SUBCRAFT_GIVE) && (item->phase_num != phase_num)))
		{
			/* if produced later, add to list, so it can be used by another phase */
			return vtoo (vnum);
		}

		if (IS_SET (item->flags, SUBCRAFT_USED))
		{
			if ((tobj = obj_list_vnum (ch, ch->right_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->left_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				if (item->item_counts > craft__count_available_objs (ch, vnum))
					continue;
				else
				{
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_IN_ROOM))
		{
			if ((tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				if (item->item_counts > tobj->count)
					continue;
				else
				{
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_HELD))
		{
			if ((tobj = obj_list_vnum_dark (ch->right_hand, vnum))
				|| (tobj = obj_list_vnum_dark (ch->left_hand, vnum)))
			{
				if (item->item_counts > tobj->count)
					continue;
				else
				{
					return tobj;
				}
			}
		}

	

		if (IS_SET (item->flags, SUBCRAFT_HELD)
			|| IS_SET (item->flags, SUBCRAFT_IN_ROOM)
			|| IS_SET (item->flags, SUBCRAFT_USED))
			continue;

		/* Grab this item wherever it is */

		if (((tobj = obj_list_vnum_dark (ch->right_hand, vnum))) ||
			((tobj = obj_list_vnum (ch, ch->left_hand, vnum))) ||
			((tobj = obj_list_vnum (ch, ch->room->contents, vnum))))
		{
			if (item->item_counts > tobj->count)
				continue;
			else
			{
				return tobj;
			}
		}
	}

	return NULL;
}

CHAR_DATA *
get_item_mob (CHAR_DATA * ch, DEFAULT_MOB_DATA * mob, int phase_num)
{
	int j;
	int vnum;
	CHAR_DATA *tch;
	
	for (j = 0; j < MAX_DEFAULT_MOBS; j++)
	{
		
		if (!(vnum = mob->mobs[j]))
			continue;
		
		if (IS_SET (mob->flags, SUBCRAFT_MOB_PRODUCED) && (mob->phase_num != phase_num))
		{
			/* if produced later, add to list, so it can be used by another phase */
			return vtom(vnum);
		}
		
		if ((IS_SET (mob->flags, SUBCRAFT_MOB_USED)) ||
			(IS_SET (mob->flags, SUBCRAFT_MOB_IN_ROOM)))
		{
			if ((tch = mob_list_vnum (ch, ch->room->people, vnum)))
			{
				return tch;
			}
		}
		
				
		
		if (IS_SET (mob->flags, SUBCRAFT_MOB_IN_ROOM)
			|| IS_SET (mob->flags, SUBCRAFT_MOB_USED))
			continue;
		
		/* Grab this mob wherever it is */
		
		if ((tch = mob_list_vnum (ch, ch->room->people, vnum)))
		{
			return tch;
			
		}
	}
	
	return NULL;
}


OBJ_DATA *
get_key_start_obj (CHAR_DATA * ch, DEFAULT_ITEM_DATA * item, int phase_num, int index)
{
	int j;
	int vnum = 0;
	OBJ_DATA *tobj;

	for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
	{
		vnum = item->items[j];
		if (vnum < 1)
			continue;

		if (item->phase_num != phase_num)
			continue;

		if (IS_SET (item->flags, SUBCRAFT_USED))
		{
			if ((tobj = obj_list_vnum (ch, ch->right_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->left_hand, vnum))
				|| (tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				if (item->item_counts <= craft__count_available_objs (ch, vnum))
				{
					ch->craft_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_IN_ROOM))
		{
			if ((tobj = obj_list_vnum (ch, ch->room->contents, vnum)))
			{
				if (item->item_counts > tobj->count)
					continue;
				else
				{
					ch->craft_index = j;
					return tobj;
				}
			}
		}

		if (IS_SET (item->flags, SUBCRAFT_HELD))
		{
			if ((tobj = obj_list_vnum_dark (ch->right_hand, vnum))
				|| (tobj = obj_list_vnum_dark (ch->left_hand, vnum)))
			{
				if (item->item_counts > tobj->count)
					continue;
				else
				{
					ch->craft_index = j;
					return tobj;
				}
			}
		}

		
		if (IS_SET (item->flags, SUBCRAFT_HELD)
			|| IS_SET (item->flags, SUBCRAFT_IN_ROOM)
			|| IS_SET (item->flags, SUBCRAFT_USED))
			continue;

		/* Grab this item wherever it is */

		if (((tobj = obj_list_vnum_dark (ch->right_hand, vnum))) ||
			((tobj = obj_list_vnum (ch, ch->left_hand, vnum))) ||
			((tobj = obj_list_vnum (ch, ch->room->contents, vnum))))
		{
			if (item->item_counts > tobj->count)
				continue;
			else
			{
				ch->craft_index = j;
				return tobj;
			}
		}
	}

	return NULL;
}

OBJ_DATA *
get_key_end_obj (CHAR_DATA * ch, DEFAULT_ITEM_DATA * item, int phase_num, int index)
{
	int j;
	int vnum;


	for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
	{
		vnum = item->items[j];
		if (vnum < 1)
			continue;
		
		if (index == j)
		{
			return vtoo (vnum);
		}
		
	}

	return NULL;
}

void
missing_item_msg (CHAR_DATA * ch, DEFAULT_ITEM_DATA * item, char *header)
{
	int i;
	int choice_count = 0;
	OBJ_DATA *proto_obj;
	char *p;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (item->item_counts > 1)
		sprintf (buf, "%s%d of ", header, item->item_counts);
	else
		sprintf (buf, "%s", header);

	for (i = 0; i < MAX_DEFAULT_ITEMS; i++)
		if (item->items[i])
			choice_count++;

	for (i = 0; i < MAX_DEFAULT_ITEMS; i++)
	{

		if (!item->items[i])
			continue;

		choice_count--;

		proto_obj = vtoo (item->items[i]);

		if (!proto_obj)
			continue;

		sprintf (buf + strlen (buf), "#2%s#0", proto_obj->short_description);
		if (choice_count)
			strcat (buf, ", ");

		if (choice_count == 1)
			strcat (buf, "or ");

		if (!choice_count)
			strcat (buf, ".");
	}

	reformat_string (buf, &p);
	ch->send_to_char(p);

	free_mem (p);
}

void
missing_mob_msg (CHAR_DATA * ch, DEFAULT_MOB_DATA * item, char *header)
{
	int i;
	int choice_count = 0;
	CHAR_DATA *proto_mob;
	char *p;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	
	if (item->mob_counts > 1)
		sprintf (buf, "%s%d of ", header, item->mob_counts);
	else
		sprintf (buf, "%s", header);
	
	for (i = 0; i < MAX_DEFAULT_MOBS; i++)
		if (item->mobs[i])
			choice_count++;
	
	for (i = 0; i < MAX_DEFAULT_MOBS; i++)
	{
		
		if (!item->mobs[i])
			continue;
		
		choice_count--;
		
		proto_mob = vtom(item->mobs[i]);
		
		if (!proto_mob)
			continue;
		
		sprintf (buf + strlen (buf), "#5%s#0", proto_mob->short_descr);
		if (choice_count)
			strcat (buf, ", ");
		
		if (choice_count == 1)
			strcat (buf, "or ");
		
		if (!choice_count)
			strcat (buf, ".");
	}
	
	reformat_string (buf, &p);
	ch->send_to_char(p);
	
	free_mem (p);
}

	//can this object be used in this craft?
int
craft_uses (SUBCRAFT_HEAD_DATA * craft, int vnum)
{
	int i = 0, j = 0;

	for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{
		if (!craft->obj_items[i])
			continue;
		if (!IS_SET (craft->obj_items[i]->flags, SUBCRAFT_USED) &&
			!IS_SET (craft->obj_items[i]->flags, SUBCRAFT_HELD) &&
			!IS_SET (craft->obj_items[i]->flags, SUBCRAFT_IN_ROOM))
			continue;
		for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
		{
			if (craft->obj_items[i]->items[j]) 
			{
				return 1;
			}
		}
	}

	return 0;
}

	//does this craft produce this object
int
craft_produces (SUBCRAFT_HEAD_DATA * craft, int vnum)
{
	int i = 0, j = 0;

	for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{
		if (!craft->obj_items[i])
			continue;
		if (!
			((IS_SET (craft->obj_items[i]->flags, SUBCRAFT_PRODUCED))
			 || (IS_SET (craft->obj_items[i]->flags, SUBCRAFT_FINAL_PRODUCED))
			|| (IS_SET (craft->obj_items[i]->flags, SUBCRAFT_GIVE))))
			continue;
		
		for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
		{
			if (craft->obj_items[i]->items[j])
				return 1;
		}
	}

	return 0;
}

	//does this craft use this mob
int
craft_mob_uses (SUBCRAFT_HEAD_DATA * craft, int vnum)
{
	int i = 0, j = 0;
	
	for (i = 0; i < MAX_MOBS_PER_SUBCRAFT; i++)
	{
		if (!craft->mob_items[i])
			continue;
		if (!IS_SET (craft->obj_items[i]->flags, SUBCRAFT_MOB_USED) &&
			!IS_SET (craft->obj_items[i]->flags, SUBCRAFT_MOB_IN_ROOM))
			continue;
		for (j = 0; j < MAX_DEFAULT_MOBS; j++)
		{
			if (craft->mob_items[i]->mobs[j]) 
			{
				return 1;
			}
		}
	}
	
	return 0;
}

	//does this craft produe this mob
int
craft_mob_produces (SUBCRAFT_HEAD_DATA * craft, int vnum)
{
	int i = 0, j = 0;
	
	for (i = 0; i < MAX_MOBS_PER_SUBCRAFT; i++)
	{
		if (!craft->mob_items[i])
			continue;
		if (!IS_SET (craft->obj_items[i]->flags, SUBCRAFT_MOB_PRODUCED))
			continue;
		
		for (j = 0; j < MAX_DEFAULT_MOBS; j++)
		{
			if (craft->mob_items[i]->mobs[j])
				return 1;
		}
	}
	
	return 0;
}



int
has_craft (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
	AFFECTED_TYPE *af;

	for (af = ch->hour_affects; af; af = af->next)
	{
		if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST)
		{
			if (!str_cmp
				(af->a.craft->subcraft->subcraft_name, craft->subcraft_name))
				return 1;
		}
	}

	return 0;
}


int
requires_skill_check (SUBCRAFT_HEAD_DATA * craft)
{
	PHASE_DATA *phase;
	int i;
	
	for (i = 1; i < MAX_PHASES_PER_SUBCRAFT; i++)
	{
		phase = craft->phases[i];
		if (phase->skill)
			return 1;
	}

	return 0;
}


int
figure_craft_delay (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft)
{
	PHASE_DATA *phase;
	int seconds = 0, skill = 0, numskills = 0;
	int i;
	
	seconds = craft->delay * 60;

	for (i = 1; i < MAX_PHASES_PER_SUBCRAFT; i++)
	{
		phase = craft->phases[i];
		if (phase->skill)
		{
			numskills++;
			skill += ch->skill_map[lookup_skill_name(phase->skill)];
		}
	}

	if (!numskills)
		numskills = 1;

	skill /= numskills;

	if (skill >= 30 && skill < 50)
		seconds = (int) ((float) seconds * 0.85);
	else if (skill >= 50 && skill < 70)
		seconds = (int) ((float) seconds * 0.72);
	else if (skill >= 70)
		seconds = (int) ((float) seconds * 0.61);

	return seconds;
}

void
activate_phase (CHAR_DATA * ch, AFFECTED_TYPE * af)
{
	int nObjectTally = 0;
	int nObjectVnum = 0;
	int nMobTally = 0;
	int nMobVnum = 0;
	int i, j;
	int phase_num;
	int attr_value = 0;
	int dice_value = 0;
	int delay_time = 0;
	bool missing_said = false;
	int phase_failed = 0;
	int index = 0;
	int dice_val;
	int ch_level;
	char color[MAX_STRING_LENGTH]= { '\0' };
	char *p;
	char *first;
	PHASE_DATA *phase = NULL;
	PHASE_DATA *tphase = NULL;
	DEFAULT_ITEM_DATA *item = NULL;
	DEFAULT_MOB_DATA *dmob = NULL;
	CHAR_DATA *tch = NULL;
	OBJ_DATA *obj_list[MAX_ITEMS_PER_SUBCRAFT];
	CHAR_DATA *mob_list[MAX_MOBS_PER_SUBCRAFT];
	OBJ_DATA *ptrObj = NULL, *ptrObjNext = NULL;
	CHAR_DATA *ptrMob = NULL;
	SUBCRAFT_HEAD_DATA *subcraft = NULL;
	int item_required[MAX_ITEMS_PER_SUBCRAFT];
	int mob_required[MAX_ITEMS_PER_SUBCRAFT];

	char buf[MAX_STRING_LENGTH]= { '\0' };

		//skips phase 0
	if (af->a.craft->phase_num == 0)
	{
		af->a.craft->phase_num++;
	}	
		// A craft is active as long as af->a.craft->timer is non-zero. 
		//so we have to wait until it is zero to activate this phase 
		//phase is the current phase we are going to activate
	if (af->a.craft->timer == 0)
		phase = af->a.craft->subcraft->phases[af->a.craft->phase_num];

	/* Shorthand variables */

	subcraft = af->a.craft->subcraft;
	phase_num = af->a.craft->phase_num;
	
	/* Point affect at next phase, and setup timer for that phase */

	af->a.craft->phase_num++;

		//tphase is the next phase that will be activated
	tphase = af->a.craft->subcraft->phases[af->a.craft->phase_num];
	if (tphase)
		af->a.craft->timer = tphase->phase_seconds;
	else 
		af->a.craft->timer = 0;

		
	if (ch->get_trust())
		af->a.craft->timer = 0;

		//we have an empty phase
	if (!phase)
		return;
	
	memset (item_required, 0, MAX_ITEMS_PER_SUBCRAFT * sizeof (int));
	memset (mob_required, 0, MAX_MOBS_PER_SUBCRAFT * sizeof (int));
	memset (obj_list, 0, MAX_ITEMS_PER_SUBCRAFT * sizeof (OBJ_DATA *));
	memset (mob_list, 0, MAX_ITEMS_PER_SUBCRAFT * sizeof (OBJ_DATA *));

	/* load object lists */
	for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{
		item = af->a.craft->subcraft->obj_items[i];

		if (!item)
			continue;

		if (!item->items[0])	/* No items in list.  Nothing required */
			continue;

		// one item from obj_items
		if (i == subcraft->key_first)
		{
			obj_list[i] = get_key_start_obj (ch, item, phase_num, ch->craft_index);
			continue;
		}

		else if (i == subcraft->key_end)
		{
			obj_list[i] = get_key_end_obj (ch, item, phase_num, ch->craft_index);
			continue;
		}

		else
		{
			obj_list[i] = get_item_obj (ch, item, phase_num);
		}
	}

		//TODO: calcualte average quality for tools here based on obj_list[]
	
		//load mobile lists
	for (i = 0; i < MAX_MOBS_PER_SUBCRAFT; i++)
	{
		dmob = af->a.craft->subcraft->mob_items[i];
		
		if (!dmob)
			continue;
		
		if (!dmob->mobs[0])	/* No mob in list.  Nothing required */
			continue;
		
		mob_list[i] = get_item_mob(ch, dmob, phase_num);
		
	}
	//TODO: calcualte average quality for tools here based on mob_list[]
	
		
		// Determine what is required for this phase based on first person 
		// p refers to the 3 in object3, or $3 in messages - @3 for mobs
	if (phase->first)
	{
		for (p = phase->first; *p; p++)
		{
			if (*p == '$')
			{
				p++;
				if (isdigit (*p) && atoi (p) < MAX_ITEMS_PER_SUBCRAFT)
				{
					item = af->a.craft->subcraft->obj_items[atoi (p)];
					item->phase_num = phase_num;
					item_required[atoi (p)] = 1;
				}
				
			}
			if (*p == '@')
			{
				p++;
				if (isdigit (*p) && atoi (p) < MAX_MOBS_PER_SUBCRAFT)
				{
					dmob = af->a.craft->subcraft->mob_items[atoi (p)];
					dmob->phase_num = phase_num;
					mob_required[atoi (p)] = 1;
				}
			}
		}
	}

	
		// group check 
	if ((subcraft->followers > 0) && !(num_followers (ch) >= subcraft->followers))
	{

		ch->send_to_char("You need more followers.\n");
		af->a.craft->timer = 0;
		return;

	}


		// Make sure all required objects are accounted for 
	for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{
		if (!item_required[i])
			continue;
		
		if (obj_list[i])
			continue;
		
		item = af->a.craft->subcraft->obj_items[i];
		
		if (!item)
			continue;

		if (item
			&& ((IS_SET (item->flags, SUBCRAFT_PRODUCED))
				|| (IS_SET (item->flags, SUBCRAFT_FINAL_PRODUCED))
				|| (IS_SET (item->flags, SUBCRAFT_GIVE))))
			continue;
		
		
		if (!missing_said)
		{
			missing_said = true;
			ch->send_to_char("\n#1You are missing one or more items:#0\n\n");
			ch->act("$n stops doing $s craft.", false, 0, 0, TO_ROOM);
			af->a.craft->timer = 0;
		}

		if (IS_SET (item->flags, SUBCRAFT_HELD))
			missing_item_msg (ch, item, "You must hold ");
		else if (IS_SET (item->flags, SUBCRAFT_IN_ROOM))
			missing_item_msg (ch, item, "In the room must be ");
		else
			missing_item_msg (ch, item, "You need ");
	}

	//make certain all required mobiles are accounted for
	for (i = 0; i < MAX_MOBS_PER_SUBCRAFT; i++)
	{
		if (!mob_required[i])
			continue;
		
		if (mob_list[i])
			continue;
		
		dmob = af->a.craft->subcraft->mob_items[i];
		
		if (!dmob)
			continue;
		
		if (dmob 
			&& (IS_SET (dmob->flags, SUBCRAFT_MOB_PRODUCED)))
			continue;
		
		if (!missing_said)
		{
			missing_said = true;
			ch->send_to_char("\n#1You are missing one or more mobiles:#0\n\n");
			ch->act("$n stops doing $s craft.", false, 0, 0, TO_ROOM);
			af->a.craft->timer = 0;
		}
		
		if (IS_SET (dmob->flags, SUBCRAFT_MOB_IN_ROOM))
			missing_mob_msg (ch, dmob, "In the room must be ");
		
	}
					
	if (missing_said)
		return;

	/* Check for skill requirements */

		//TODO: verfiy they get a chance to boost skill, but the craft level is not affected
	if (phase->skill)
	{
		skill_use(ch, lookup_skill_name(phase->skill), 0);
		dice_val = dice (phase->dice, phase->sides);
		ch_level = skill_level (ch, lookup_skill_name(phase->skill), 0); //includes object effects

		if (dice_val > ch_level)
		{
			phase_failed = 1;
		}
	}

	/* Check to see if attribute check succeeded.  This really is
	against the theme of this mud since it is skill based, not
	attribute based, but it was asked for anyway. */
		//TODO: verfiy they get a chance to boost skill, but the craft level is not affected

	if ( phase->attribute > -1 )
	{
		switch ( phase->attribute )
		{
		case 0:  attr_value = ch->tmp_str; break;
		case 1:  attr_value = ch->tmp_dex; break;
		case 2:  attr_value = ch->tmp_con; break;
		case 3:  attr_value = ch->tmp_wil; break;
		case 4:  attr_value = ch->tmp_intel; break;
		case 5:  attr_value = ch->tmp_aur; break;
		case 6:  attr_value = ch->tmp_agi; break;
		case 7:  attr_value = ch->tmp_luk; break;
		}

		dice_value = dice (phase->dice, phase->sides);
		if ( attr_value < dice_value )
		{
			phase_failed = 1;
		}
	}

	/* Removed USED items from wherever they are */
	for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{
		item = af->a.craft->subcraft->obj_items[i];

		if (!item)
			continue;

		if (item->phase_num != phase_num || !IS_SET (item->flags, SUBCRAFT_USED))
			continue;

		if (!obj_list[i])
		{
			ch->act
				("You are not using enough of a particular item to finish this craft.",
				false, 0, 0, TO_CHAR);

			af->a.craft->timer = 0;
			return;
		}

			//TODO: some way to use variant colros to make produced items with the variants of consumed items

		/* Purge Consumed Craft Items */
		nObjectTally = item->item_counts;
		if (nObjectTally == 0)
			nObjectTally = 1;
		nObjectVnum = obj_list[i]->nVirtual;
		
			//first we remove normal items from hands
		if (nObjectTally && ch->right_hand && ch->right_hand->nVirtual == nObjectVnum)
		{
			if (ch->right_hand->count <= nObjectTally)
			{
				nObjectTally -= ch->right_hand->count;
				extract_obj (ch->right_hand);
			}
			else
			{
				ch->right_hand->count -= nObjectTally;
				nObjectTally = 0;
			}
		}
		
		if (nObjectTally && ch->left_hand && ch->left_hand->nVirtual == nObjectVnum)
		{
			if (ch->left_hand->count <= nObjectTally)
			{
				nObjectTally -= ch->left_hand->count;
				extract_obj (ch->left_hand);
			}
			else
			{
				ch->left_hand->count -= nObjectTally;
				nObjectTally = 0;
			}
		}
		
						//finally we check the room for normal items
		if (nObjectTally && ch->room->contents)
		{
			nObjectVnum = obj_list[i]->nVirtual; //look for normal items first
				//first test for normal objects
			for (ptrObj = ch->room->contents; nObjectTally && ptrObj != NULL;)
			{
				if (ptrObj->nVirtual == nObjectVnum)
				{
					if (ptrObj->count <= nObjectTally)
					{
						nObjectTally -= ptrObj->count;
						ptrObjNext = ptrObj->next_content;
						extract_obj (ptrObj);
						ptrObj = ptrObjNext;
					}
					else
					{
						ptrObj->count -= nObjectTally;
						nObjectTally = 0;
						ptrObj = ptrObj->next_content;
					}
				}
				else
				{
					ptrObj = ptrObj->next_content;
				}
			}
			
		}//if (nObjectTally
	}//if (!obj_list[i])

	/* Removed USED mobiles from wherever they are */
	for (i = 0; i < MAX_MOBS_PER_SUBCRAFT; i++)
	{
		dmob = af->a.craft->subcraft->mob_items[i];
		
		if (!dmob)
			continue;
		
		if (dmob->phase_num != phase_num || !IS_SET (dmob->flags, SUBCRAFT_MOB_USED))
			continue;
		
		if (!mob_list[i])
		{
			ch->act
			("There are not enough of a particular mob to finish this craft.",
			 false, 0, 0, TO_CHAR);
			
			af->a.craft->timer = 0;
			return;
		}
		
		
		/* Purge Consumed mobile Items */
		nMobTally = dmob->mob_counts;
		if (nMobTally == 0)
			nMobTally = 1;
		nMobVnum = mob_list[i]->mob->nVirtual;
		
		
			//we check the room for normal items
		if (nMobTally && ch->room->people)
		{
			nMobVnum = mob_list[i]->mob->nVirtual; 
			for (ptrMob = ch->room->people; nMobTally && ptrMob != NULL;)
			{
				if (IS_NPC(ptrMob) && (ptrMob->mob->nVirtual == nMobVnum))
				{
					ptrMob->extract_char();
					nMobTally --;
					ptrMob = ptrMob->next_in_room;
				}
				else
				{
					ptrMob = ptrMob->next_in_room;
				}
			}
			
		}
	}

	
	/* Create objects made by this phase */

	for (i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++)
	{
		item = af->a.craft->subcraft->obj_items[i];

		if (!item)
			continue;

		if (!item->items[0])
			continue;

		if (item->phase_num != phase_num)
			continue;
		
		if (!((IS_SET (item->flags, SUBCRAFT_PRODUCED))
			  || (IS_SET (item->flags, SUBCRAFT_FINAL_PRODUCED))
			|| (IS_SET (item->flags, SUBCRAFT_GIVE))))
			continue;

		/* We need to account for the quantity of an item */

		*color = '\0';

		for (j = 0; j < MAX_ITEMS_PER_SUBCRAFT; j++)
		{
			item = af->a.craft->subcraft->obj_items[j];

			if (item && item->color && *item->color)
			{
				sprintf (color, "%s", item->color);
				free_mem (item->color);
				break;
			}
		}

		item = af->a.craft->subcraft->obj_items[i];

		if (!item)
			continue;

		index = ch->craft_index;
		if ((i == subcraft->key_end) && !(i == 0))
			index = ch->craft_index;
		else
			index = 0;

		if (item->items[index] && vtoo (item->items[index]))
		{
			if (color
				&& IS_SET (vtoo (item->items[index])->obj_flags.extra_flags, ITEM_VARIABLE))
				obj_list[i] = load_colored_object (item->items[index], color);
			else
			{
				obj_list[i] = load_object (item->items[index]);
			}
		}

		if (!obj_list[i])
		{
			ch->act
				("An object produced by this craft was not found. Please inform the staff.",
				false, 0, 0, TO_CHAR);

			af->a.craft->timer = 0;
			return;
		}

		obj_list[i]->count = item->item_counts;
		if (IS_SET (item->flags, SUBCRAFT_GIVE))
		{
			obj_to_char (obj_list[i], ch);
		}
		else
		{
			obj_to_room (obj_list[i], ch->in_room);
		}
	}//for (i = 0;

	for (i = 0; i < MAX_MOBS_PER_SUBCRAFT; i++)
	{
		dmob = af->a.craft->subcraft->mob_items[i];
		
		if (!dmob)
			continue;
		
		if (!dmob->mobs[0])
			continue;
		
		if (dmob->phase_num != phase_num)
			continue;
		
		if (!(IS_SET (dmob->flags, SUBCRAFT_MOB_PRODUCED)))
			continue;
		
		dmob = af->a.craft->subcraft->mob_items[i];
		
		if (!dmob)
			continue;
		
		if (dmob->mobs[0] && vtom(dmob->mobs[0]))
		{
			mob_list[i] = load_mobile(dmob->mobs[0]);
		}
		
		if (!mob_list[i])
		{
			ch->act
			("A mobile produced by this craft was not found. Please inform the staff.", false, 0, 0, TO_CHAR);
			
			af->a.craft->timer = 0;
			return;
		}
		
		mob_list[i]->char_to_room (ch->in_room);
		mob_list[i]->mob->action |= ACT_STAYPUT;
		save_stayput_mobiles ();
		
			//need to add a check for number of mobs to laod		
		
	}
	
	
	/* First person message */
	if (phase->first)
	{

		first = phase->first;
		
		craft_prepare_message (ch, buf, ch, NULL, NULL,
			first, obj_list, mob_list);

		if (*buf == '#')
			buf[2] = toupper (buf[2]);
		else
			*buf = toupper (*buf);
		ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
	}

		// Third person message 
	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{

		if (!phase->third)
			break;

		if (tch == ch)
			continue;

		craft_prepare_message (tch, buf, ch, NULL, tch,
			phase->third, obj_list, mob_list);
		if (*buf == '#')
			buf[2] = toupper (buf[2]);
		else
			*buf = toupper (*buf);
		tch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
	}

		// Group members message 

	if ((subcraft->followers > 0)
		&& (num_followers (ch) >= subcraft->followers)
		&& phase->group_mess)
	{
		craft_prepare_message (ch, buf, ch, NULL, tch,
			phase->group_mess, obj_list, mob_list);
		if (*buf == '#')
			buf[2] = toupper (buf[2]);
		else
			*buf = toupper (*buf);
		ch->act(buf, false,  0, 0, TO_GROUP | _ACT_FORMAT);
	}

		

			// Immediately activate next phase after a phase with 0 timer 
	if (!af->a.craft->timer && af->a.craft->phase_num)
		activate_phase (ch, af);
	else if (!af->a.craft->phase_num
		&& requires_skill_check (af->a.craft->subcraft))
	{
			//TODO: replace this with craft skill boost
	}

	if (af->a.craft->subcraft->delay && !af->a.craft->phase_num)
	{
		delay_time = time (0) + figure_craft_delay (ch, af->a.craft->subcraft);
		magic_add_affect (ch, MAGIC_CRAFT_DELAY, -1, delay_time, 0, 0, 0);
	}


}

void
craft_setup (CHAR_DATA * ch, char *argument, char *subcmd)
{

	char output[MAX_STRING_LENGTH]= { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;


	if (!str_cmp (subcmd, "craft"))
	{
		argument = one_argument (argument, buf);
		if (!isalpha (*buf))
		{
			ch->send_to_char("Craft name contains illegal characters.\n");
			return;
		}
		craft->craft_name = duplicateString (buf);
		sprintf (output, "Craft name changed to '%s'.\n", buf);
		ch->send_to_char(output);
	}

	else if (!str_cmp (subcmd, "subcraft"))
	{
		argument = one_argument (argument, buf);
		if (!isalpha (*buf))
		{
			ch->send_to_char("Subcraft name contains illegal characters.\n");
			return;
		}
		craft->subcraft_name = duplicateString (buf);
		sprintf (output, "Subcraft name changed to '%s'.\n", buf);
		ch->send_to_char(output);
	}

	else if (!str_cmp (subcmd, "command"))
	{
		argument = one_argument (argument, buf);
		if (!isalpha (*buf))
		{
			ch->send_to_char("Command name contains illegal characters.\n");
			return;
		}
		craft->command = duplicateString (buf);
		sprintf (output, "Craft command changed to '%s'.\n", buf);
		ch->send_to_char(output);
	}

	else if (!str_cmp (subcmd, "hidden") || !str_cmp (subcmd, "obscure"))
	{
		TOGGLE_BIT (craft->subcraft_flags, SCF_OBSCURE);
		if (IS_SET (craft->subcraft_flags, SCF_OBSCURE))
		{
			sprintf (output,
				"DELETE FROM new_crafts WHERE subcraft = '%s'",
				craft->subcraft_name);
			mysql_safe_query (output);
			sprintf (output, "DELETE FROM crafts WHERE subcraft = '%s'",
				craft->subcraft_name);
			mysql_safe_query (output);
			ch->send_to_char("This craft will not be displayed on the website or in the Palantir Weekly.\n");
		}
		else
		{
			ch->send_to_char("This craft will be displayed on the website after the next reboot.\n");
		}
		return;
	}

}

void
craft_terrains (CHAR_DATA * ch, char *argument, char *subcmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int index;
	int j;
	int i;
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;

	argument = one_argument (argument, buf);

	if (*buf)
		index = index_lookup (terrain_types, buf);
	else
		index = -1;

	if (index == -1)
	{
		ch->send_to_char("That isn't a recognized terrain type.\n");
		return;
	}

	for (j = -1, i = 24; i >= 0; i--)
	{
		if (!craft->terrains[i])
			j = i;
		else if (craft->terrains[i] - 1 == index)
		{
			j = -2;
			break;
		}
	}

	if (j == -2)
	{
		craft->terrains[i] = 0;
		ch->send_to_char("Craft no longer requires the specified terrain.\n");
		return;
	}

	else if (i == -1 && j == -1)
	{
		ch->send_to_char("That craft's list of required terrains is currently full.\n");
		return;
	}

	else if (i == -1 && j != -1)
	{
		craft->terrains[j] = index + 1;
		ch->send_to_char("The specified terrain has been added to the list of required   terrain types.\n");
		return;
	}
}

void
craft_seasons (CHAR_DATA * ch, char *argument, char *subcmd)
{

	char buf[MAX_STRING_LENGTH]= { '\0' };
	int index;
	int j;
	int i;
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;

	argument = one_argument (argument, buf);

	if (*buf)
		index = index_lookup (seasons, buf);
	else
		index = -1;

	if (index == -1)
	{
		ch->send_to_char("That isn't a recognized seasons.\n");
		return;
	}

	for (j = -1, i = 5; i >= 0; i--)
	{
		if (!craft->seasons[i])
			j = i;
		else if (craft->seasons[i] - 1 == index)
		{
			j = -2;
			break;
		}
	}

	if (j == -2)
	{
		craft->seasons[i] = 0;
		ch->send_to_char("Craft no longer requires the specified season.\n");
		return;
	}

	else if (i == -1 && j == -1)
	{
		ch->send_to_char("That craft's list of required seasons is currently full.\n");
		return;
	}

	else if (i == -1 && j != -1)
	{
		craft->seasons[j] = index + 1;
		ch->send_to_char("The specified season has been added to the list of required seasons.\n");
		return;
	}

	return;
}

void
craft_opening (CHAR_DATA * ch, char *argument, char *subcmd)
{

	char buf[MAX_STRING_LENGTH]= { '\0' };
	int index;
	int j;
	int i;
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;

	argument = one_argument (argument, buf);

	if (*buf)
		index = lookup_skill_id(buf);
	else
		index = -1;

	if (index == -1)
	{
		ch->send_to_char("That isn't a recognized skill name.\n");
		return;
	}

	for (j = -1, i = 24; i >= 0; i--)
	{
		if (!craft->opening[i])
			j = i;
		else if (craft->opening[i] == index)
		{
			j = -2;
			break;
		}
	}

	if (j == -2)
	{
		craft->opening[i] = 0;
		ch->send_to_char("Craft no longer an opening craft for the specified skill.\n");
		return;
	}

	else if (i == -1 && j == -1)
	{
		ch->send_to_char("That craft's list of skills it opens for is currently full.\n");
		return;
	}

	else if (i == -1 && j != -1)
	{
		craft->opening[j] = index;
		ch->send_to_char("This craft will now open for the specified skill.\n");
		return;
	}

	return;
}

void
craft_race (CHAR_DATA * ch, char *argument, char *subcmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int index;
	int j;
	int i;
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;

	argument = one_argument (argument, buf);

	if (*buf)
		index = lookup_race_id(buf);
	else
		index = -1;

	if (index == -1)
	{
		ch->send_to_char("That isn't a recognized race name.\n");
		return;
	}

	for (j = -1, i = 24; i >= 0; i--)
	{
		if (!craft->race[i])
			j = i;
		else if (craft->race[i] - 1 == index)
		{
			j = -2;
			break;
		}
	}

	if (j == -2)
	{
		craft->race[i] = 0;
		ch->send_to_char("That race is no longer required for this craft.\n");
		return;
	}

	else if (i == -1 && j == -1)
	{
		ch->send_to_char("That craft's list of required races is currently full.\n");
		return;
	}

	else if (i == -1 && j != -1)
	{
		craft->race[j] = index + 1;
		ch->send_to_char("This craft will now require the specified race.\n");
		return;
	}
	return;
}


void
craft_clan (CHAR_DATA * ch, char *argument, char *subcmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	int flags = 0;
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;

	argument = one_argument (argument, buf); //clan
	argument = one_argument (argument, buf2); //clan rank

	if (!get_clandef (buf))
	{
		ch->send_to_char("That is not a recognized clan.\n");
		return;
	}

	if (is_clan_in_string (craft->clans, buf, flags))
	{
		if (!*buf2)
		{
			craft->clans =
				duplicateString (remove_clan_from_string (craft->clans, buf));
			ch->send_to_char("The specified clan requirement has been removed.\n");
			return;
		}

		else if (!(flags = clan_rank_to_value (buf2, buf)))
		{
			ch->send_to_char("That is not a recognized clan rank.\n");
			return;
		}

		else
		{
			craft->clans =
				duplicateString (remove_clan_from_string (craft->clans, buf));
			craft->clans =
				duplicateString (add_clan_to_string (craft->clans, buf, flags));
			ch->send_to_char("The rank requirement for the specified clan has been updated.\n");
			return;
		}
	}

	else
	{
		if (!*buf2)
			sprintf (buf2, "member");

		if (*buf2 && !(flags = clan_rank_to_value (buf2, buf)))
		{
			ch->send_to_char("That is not a recognized clan rank.\n");
			return;
		}

		if (flags)
		{
			craft->clans =
				duplicateString (add_clan_to_string (craft->clans, buf, flags));
		}
		else
		{
			craft->clans =
				duplicateString (add_clan_to_string (craft->clans, buf, CLAN_MEMBER));
		}

		ch->send_to_char("The specified clan and rank have been added as requirements.\n");
		return;
	}
	return;
}


void
craft_delete (CHAR_DATA * ch, char *argument, char *subcmd)
{
	char output[MAX_STRING_LENGTH]= { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };
	SUBCRAFT_HEAD_DATA *tcraft;
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	
	tcraft = ch->pc->edit_craft;

	argument = one_argument (argument, buf);

	if (*buf != '!')
	{
		ch->send_to_char("If you are SURE you'd like to delete this craft, please use 'cset delete !'.\n");
		return;
	}

	mysql_safe_query
		("DELETE FROM new_crafts WHERE subcraft = '%s' AND craft = '%s'",
		tcraft->subcraft_name,
		tcraft->craft_name);

	sprintf (output,
		"The %s craft has been deleted.\n",
		tcraft->subcraft_name);

	tcraft_iterator = craft_map.find(tcraft->subcraft_name);
	craft_map.erase (tcraft_iterator);

	ch->send_to_char(output);
	ch->pc->edit_craft = NULL;
	return;
}


void
craft_delay (CHAR_DATA * ch, char *argument, char *subcmd)
{
	SUBCRAFT_HEAD_DATA *craft;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (argument, buf);

	craft = ch->pc->edit_craft;


	if (!*buf)
	{
		ch->send_to_char("How many OOC minutes did you want to set the delay timer to?\n");
		return;
	}

	if (!isdigit (*buf))
	{
		ch->send_to_char("You must specify a number of RL minutes to set the timer to.\n");
		return;
	}

	else
	{
		craft->delay = atoi (buf);
		ch->send_to_char("The craft's OOC delay timer has been set.\n");
		return;
	}
}

void
craft_key (CHAR_DATA * ch, char *argument, char *subcmd)
{
	char output[MAX_STRING_LENGTH]= { '\0' };
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;

	if (!*argument)
	{
		ch->send_to_char("Which object list did you want to use as the key?\n");
		return;
	}

	if (!isdigit (*argument))
	{
		ch->send_to_char("You must specify the number of the object list that you wish to use as the key.\n");
		return;
	}

	craft->key_first = atoi (argument);
	if (craft->key_first == 0)
	{
		ch->send_to_char("The craft's key objects lists have been deleted.\n");
		craft->key_end = -1;
		craft->key_first = -1;
		return;
	}
	else
	{
		sprintf
			(output,
			"The craft's key objects list has been set to %d.\n",
			craft->key_first );
		ch->send_to_char(output);
		return;
	}
	return;
}

void
craft_key_product (CHAR_DATA * ch, char *argument, char *subcmd)
{
	char output[MAX_STRING_LENGTH]= { '\0' };
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;

	if (!*argument)
	{
		ch->send_to_char("Which object list did you want to use for key-production?\n");
		return;
	}

	if (!isdigit (*argument))
	{
		ch->send_to_char("You must specify the number (1-9) of the object list that you wish to use for key-production. \n");
		return;
	}

	craft->key_end = atoi (argument);

	if (craft->key_end == 0)
	{
		ch->send_to_char("The craft's key objects lists have been deleted.\n");
		craft->key_end = -1;
		craft->key_first = -1;
		return;
	}
	else
	{
		sprintf
			(output,
			"The craft's key-production list has been set to %d.\n",
			craft->key_end );
		ch->send_to_char(output);
		return;
	}
	return;
}

void
craft_weather (CHAR_DATA * ch, char *argument, char *subcmd)
{

	char buf[MAX_STRING_LENGTH]= { '\0' };
	int index;
	int j;
	int i;
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;

	argument = one_argument (argument, buf);

	if (*buf)
		index = index_lookup (weather_states, buf);
	else
		index = -1;

	if (index == -1)
	{
		ch->send_to_char("That isn't a recognized weather state.\n");
		return;
	}

	for (j = -1, i = 8; i >= 0; i--)
	{
		if (!craft->weather[i])
			j = i;
		else if (craft->weather[i] - 1 == index)
		{
			j = -2;
			break;
		}
	}

	if (j == -2)
	{
		craft->weather[i] = 0;
		ch->send_to_char("Craft no longer requires the specified weather.\n");
		return;
	}

	else if (i == -1 && j == -1)
	{
		ch->send_to_char("That craft's list of required weather is currently full.\n");
		return;
	}

	else if (i == -1 && j != -1)
	{
		craft->weather[j] = index + 1;
		ch->send_to_char("The specified weather has been added to the list of required weather.\n");
		return;
	}

	return;
}

void
craft_group (CHAR_DATA * ch, char *argument, char *subcmd)
{
	char output[MAX_STRING_LENGTH]= { '\0' };
	SUBCRAFT_HEAD_DATA *craft;

	craft = ch->pc->edit_craft;

	if (!*argument)
	{
		ch->send_to_char("How many followers do you need in your group?\n");
		return;
	}

	if (!isdigit (*argument))
	{
		ch->send_to_char("You must specify the number of followers that you wish to require.\n");
		return;
	}

	craft->followers = atoi (argument);
	if (craft->followers == 0)
	{
		ch->send_to_char("The craft's group requiremnt have been deleted.\n");
		return;
	}
	else
	{
		sprintf
			(output,
			"This craft now requires %d followers in the leaders group.\n",
			craft->followers );
		ch->send_to_char(output);
		return;
	}
	return;
}

void
craftstat (CHAR_DATA * ch, char *argument)
{
	int i;
	SUBCRAFT_HEAD_DATA *craft;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char temp_buf[MAX_STRING_LENGTH]= { '\0' };
	std::string table_buf;
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	
	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char("Which craft did you wish to view?");
		return;
	}

	for (tcraft_iterator = craft_map.begin(); tcraft_iterator != craft_map.end(); tcraft_iterator++)
	{
		craft = tcraft_iterator->second;
		if (!str_cmp (craft->subcraft_name, buf))
			break;
	}
	

	if (!craft)
	{
		ch->send_to_char("No such craft.  Type 'crafts' for a listing.\n");
		return;
	}

	

	/** displays information **/
	
	sprintf (temp_buf, "#6Craft:#0 %s #6Subcraft:#0 %s #6Command:#0 %s\n",
		craft->craft_name, craft->subcraft_name, craft->command);

	table_buf.assign(temp_buf);
	
	if (IS_SET (craft->subcraft_flags, SCF_OBSCURE))
		table_buf.append("#1Hidden#0\n");

	/** restricted to branching by race **/
	table_buf.append("#6Race:#0 ");
	for (i = 0; i <= 24; i++)
	{
		if (craft->race[i] > 0)
		{
			table_buf.append(lookup_race_variable (craft->race[i] - 1, RACE_NAME));
		}
	}
	table_buf.append("\n");
		
	/** restricted to branching by clan **/
	table_buf.append("  #6Clans:#0 ");
	if (craft->clans && strlen (craft->clans) > 3)
	{
		table_buf.append(craft->clans);
		table_buf.append(" ");
	}

	table_buf.append("\n");
		/** Opens for skill **/
	table_buf.append("  #6Base:#0 ");
	for (i = 0; i <= 24; i++)
	{
		if (craft->opening[i] > 0)
		{
			table_buf.append(lookup_skill_name(craft->opening[i]));
			table_buf.append(" ");
		}
	}
	table_buf.append("\n");
	
	/** terrains **/
	table_buf.append("  #6Terrains:#0 ");
			
		for (i = 0; i <= 24; i++)
			if (craft->terrains[i])
			{
				table_buf.append(terrain_types[craft->terrains[i] - 1]);
				table_buf.append(" ");	
			}
	
		table_buf.append("\n");
	

	/** seasons **/
			table_buf.append("  #6Seasons:#0 ");
		for (i = 0; i <= 5; i++)
			if (craft->seasons[i])
			{
				table_buf.append(seasons[craft->seasons[i] - 1]);
				table_buf.append(" ");	
			}
		
		table_buf.append("\n");
	

	/** weather **/
	
		table_buf.append("  #6Weather:#0 ");
		for (i = 0; i <= 8; i++)
			if (craft->weather[i])
			{
				table_buf.append(weather_states[craft->weather[i] - 1]);
				table_buf.append(" ");	
			}
		
		table_buf.append("\n");
	
	/** group **/
	if (craft->followers > 0)
	{
		sprintf (temp_buf, "  #6Followers:#0 %d\n", craft->followers);
		table_buf.append(temp_buf);	
	}
		
	/** delay **/
	if (craft->delay)
	{
		sprintf (temp_buf, "  #6OOC Delay Timer:#0 %d RL Minutes\n",
				 craft->delay);
		table_buf.append(temp_buf);
	}
	

	if (craft->key_first > 0)
	{
		sprintf (temp_buf, "  #6Primary Key:#0 %d \n", craft->key_first);
		table_buf.append(temp_buf);
	}

	if (craft->key_end > 0)
	{
		sprintf (temp_buf, "  #6Product Key:#0 %d \n", craft->key_end);
		table_buf.append(temp_buf);
	}

	ch->send_to_char(table_buf.c_str());
	display_craft (ch, craft);

	return;

}

//list the crafts of a PC
/** 
 craftspc <name> - lists all of <name>'s crafts
 craftspc <name> sets - lists the craft sets only
 craftspc <name> <craftset> - lsits the individual crafts in <craftset>.
 **/
void
do_craftspc (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch;
	int i, j;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	std::string output;
	AFFECTED_TYPE *af;
	
	argument = one_argument(argument, buf);

	if (!strcmp(argument, "?") || (!*buf))
	{
		ch->send_to_char("\nThe syntax is as follows:\n");
		ch->send_to_char("  craftspc <name>              lists all of <name>'s crafts\n");
		ch->send_to_char("  craftspc <name> sets         lists the craft sets only\n");
		ch->send_to_char("  craftspc <name> <craftset>   lists the individual crafts in <craftset>.\n");
		
	}
	
	tch = load_pc(buf);
	if (tch == NULL)
		return;
		
	if (!strcmp(argument, "sets"))
	{
		sprintf(buf2, "#5%s#0 currently has crafts in the following areas:\n\n", tch->name);
		output.assign(buf2);
		for (i = CRAFT_FIRST; i <= CRAFT_LAST; i++)
		{
			if ((af = get_affect (tch, i)))
			{
				sprintf(buf2, "  #6%-24s#0",
						af->a.craft->subcraft->craft_name);
				
				if (output.find(buf2, 0, strlen(buf2)) == std::string::npos)
				{
					j++;
					output.append(buf2);
					
					if (!(j % 3))
						output.append("\n");
				}
			}
		}
		
	}
	
	else if (*argument) 
	{
		sprintf (buf2,
				 "\n#5%s#0 has the following crafts in #6%s#0:\n\n",
				 tch->name, argument);
		
		output.assign(buf2);
		
			
		for (af = tch->hour_affects; af; af = af->next)
		{
			if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST)
			{
				if (!strcmp(argument, af->a.craft->subcraft->craft_name))
				{
					sprintf(buf2, "  %-20s\n",
							af->a.craft->subcraft->subcraft_name);
					output.append(buf2);
				}
			}
		}
	}
	else 
	{
		sprintf (buf2,
				 "\n#5%s#0 has the following crafts on %s pfile:\n\n",
			tch->name,
			HSHR (tch));

		output.assign(buf2);

		for (af = tch->hour_affects; af; af = af->next)
		{
			if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST)
			{
				sprintf (buf2,
						 "  #6Craft:#0 %-24s #6Subcraft:#0 %-20s\n",
					af->a.craft->subcraft->craft_name,
						 af->a.craft->subcraft->subcraft_name);

				output.append(buf2);
			}
		}
	}

		tch->unload_pc();
		page_string (ch->desc, output.c_str());

	return;
}

//displays only header info and economic inforamtion
void
spec_craftstat (CHAR_DATA * ch, char *argument)
{
	SUBCRAFT_HEAD_DATA *craft;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	
	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char("Which craft did you wish to view?");
		return;
	}



	for (tcraft_iterator = craft_map.begin(); tcraft_iterator != craft_map.end(); tcraft_iterator++)
	{
		craft = tcraft_iterator->second;
		if (!str_cmp (craft->subcraft_name, buf))  //individual craft
		{
			display_spec_craft (ch, craft);
			return;
		}
		if (!str_cmp (craft->craft_name, buf)) //craft catagory
		{
			display_spec_craft (ch, craft);
		}
	}


	return;
}

void
display_spec_craft (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA *craft)
{
	PHASE_DATA *phase;
	std::string table_buf;
	char temp_buf[MAX_STRING_LENGTH]= { '\0' }; 
	DEFAULT_ITEM_DATA *items;
	int i, j, phasenum = 1;
	float low_consumed_expense = 0;
	float high_consumed_expense = 0;
	float low_reusable_expense = 0;
	float high_reusable_expense = 0;
	float low_produced_value = 0;
	float high_produced_value = 0;
	float low_cost = 0;
	float high_cost = 0;
	float obj_value;

	/** displays information **/
	table_buf.assign("#6Craft:#0 ");
	table_buf.append(craft->craft_name);
	table_buf.append(" #6Subcraft:#0 ");
	table_buf.append(craft->subcraft_name);
	table_buf.append(" #Command:#0 ");
	table_buf.append(craft->command);
	table_buf.append("\n");
	

	/** delay **/
	if (craft->delay)
	{
		sprintf (temp_buf, "  #6OOC Delay Timer:#0 %d RL Minutes\n",
				 craft->delay);
		table_buf.append("temp_buf");
	}
	

	
	/** evaluates objects for each phase **/

	for (phasenum = 1; phasenum < MAX_PHASES_PER_SUBCRAFT; phasenum++)
	{

		phase = craft->phases[phasenum];
		
		if (craft->obj_items)
		{
			for (i = 1; craft->obj_items[i]; i++)
			{
				items = craft->obj_items[i];

				if (items->phase_num != phasenum)
					continue;

				if (items->items && items->items[0])
				{
					low_cost = 0;
					high_cost = 0;
					for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
					{
						if (items->items[j]
						&& items->items[j] != items->item_counts)
						{
							OBJ_DATA *obj = vtoo (items->items[j]);

							if (obj)
							{
								obj_value = obj->coppers;
								if (!low_cost || (low_cost > obj_value))
								{
									low_cost = obj_value;
								}
								if (high_cost < obj_value)
								{
									high_cost = obj_value;
								}
							}//if (obj)
						}//if (items->items[j])
					} //for (j = 0;)

					low_cost *= items->item_counts;
					high_cost *= items->item_counts;

					if (IS_SET (items->flags, SUBCRAFT_USED))
					{
						low_consumed_expense += low_cost;
						high_consumed_expense += high_cost;
					}

					else if ((IS_SET (items->flags, SUBCRAFT_PRODUCED))
							 || (IS_SET (items->flags, SUBCRAFT_PRODUCED))
							 || (IS_SET (items->flags, SUBCRAFT_GIVE)))
					{
						low_produced_value += low_cost;
						high_produced_value += high_cost;
					}

					else
					{
						low_reusable_expense += low_cost;
						high_reusable_expense += high_cost;
					}
				}
			}
		}
	}

	sprintf (temp_buf,
		"#6Reusable Material Costs:#0 % 7.2f - % 7.2f bits\n"
		"#6Expended Material Costs:#0 % 7.2f - % 7.2f bits\n"
		"#6Produced Material Value:#0 % 7.2f - % 7.2f bits\n\n",
		low_reusable_expense, high_reusable_expense,
		low_consumed_expense, high_consumed_expense,
		low_produced_value, high_produced_value);

	table_buf.append(temp_buf);
	page_string (ch->desc, table_buf.c_str());

	return;
}

	//cset new <craft name> <subcraft name> <command>
	//cset new <craft name> <subcraft name> <command> !
void
craft_new(CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char craft_name[MAX_STRING_LENGTH]= { '\0' };
	char subcraft[MAX_STRING_LENGTH]= { '\0' };
	char command[MAX_STRING_LENGTH]= { '\0' };
	std::map<std::string, SUBCRAFT_HEAD_DATA*>::iterator tcraft_iterator;
	SUBCRAFT_HEAD_DATA *tcraft;
	SUBCRAFT_HEAD_DATA *craft;
	
	
	argument = one_argument (argument, buf);
	sprintf (craft_name, "%s", buf);
	
	argument = one_argument (argument, buf);
	sprintf (subcraft, "%s", buf);
	
	argument = one_argument (argument, buf);
	sprintf (command, "%s", buf);
	
	if (ch->pc->edit_craft && *buf != '!')
	{
		sprintf(buf2, "You are still editing the %s craft.\n Use 'cset save' to save your work. Otherwise, use #3'cset new %s %s %s !'#0, without the quotes, to discard old changes and work on a new craft.\n", ch->pc->edit_craft->subcraft_name, craft_name, subcraft, command);
		ch->send_to_char(buf2);
		return;
	}
	
	ch->pc->edit_craft = NULL;
	
	if (!*craft_name || !*subcraft || !*command)
	{
		ch->send_to_char("The syntax is as follows: cset new <craft> <subcraft> <command>\n");
		return;
	}
	
	for (tcraft_iterator = craft_map.begin(); tcraft_iterator != craft_map.end(); tcraft_iterator++)
	{
		tcraft = tcraft_iterator->second;
		if(!str_cmp (tcraft->subcraft_name, subcraft))
		{
			ch->send_to_char("Sorry, but that subcraft name is already in use.\n");
			return;
		}
	}
	
	craft = new SUBCRAFT_HEAD_DATA;
	craft->craft_name = duplicateString (craft_name);
	craft->subcraft_name = duplicateString (subcraft);
	craft->command = duplicateString (command);
	craft->key_first = -1;
	craft->key_end = -1;
	ch->pc->edit_craft = craft;
	craft_map[craft->subcraft_name] = craft;
	ch->send_to_char("New craft initialized and opened for editing.\n");
	
	mysql_safe_query
	("INSERT INTO new_crafts VALUES ('%s', '%s', '%s', '%s')",
	 craft->command,
	 craft->subcraft_name,
	 craft->craft_name,
	 ch->name);
	return;
} 


void
craft_save(CHAR_DATA * ch)
{

	if (ch->pc->edit_craft && (IS_SET(ch->plr_flags, IS_CRAFTER)))
	{
		save_mysql_crafts(ch, ch->pc->edit_craft, BUILD_APPROVED);	
		ch->pc->edit_craft = NULL;
		ch->send_to_char("Craft was saved.");
		return;
	}
	else 
	{
		ch->send_to_char("Craft was NOT saved!");
		return;
	}

	return;
}

void
craft_phases (CHAR_DATA * ch, char *argument, char *subcmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char output[MAX_STRING_LENGTH]= { '\0' };
	int phasenum, i, j, dice, sides;
	int objnum = 0;
	int mobnum = 0;
	SUBCRAFT_HEAD_DATA *craft;
	PHASE_DATA *phase;
	DEFAULT_ITEM_DATA *items;
	DEFAULT_MOB_DATA * mobs;
	extern const char *attrs[9];

	craft = ch->pc->edit_craft;
	
	if (!craft)
	{
		ch->send_to_char("You are not editing a craft!\n");
		return;	
	}
	
	if (!isdigit (subcmd[5]))
	{
		ch->send_to_char("Please specify a phase number, e.g. 'cset phase2'.\n");
		return;
	}
	sprintf (buf, "%c", subcmd[5]);
	if (isdigit (subcmd[6]))
		sprintf (buf + strlen (buf), "%c", subcmd[6]);
	
	phasenum = atoi (buf);
	
	phase = craft->phases[phasenum];
	
	if (!phase)
	{
		phase = new PHASE_DATA;
		craft->phases[phasenum] = phase;
	}
	
	argument = one_argument (argument, subcmd);
	if (!*subcmd)
	{
		ch->send_to_char("What would you like to edit in this phase?\n");
		return;
	}
	
	/** Phase Echos **/
	if (!str_cmp (subcmd, "1st"))
	{
		argument = one_argument (argument, buf);
		phase->first = strdup(buf);
		sprintf (output,
				 "The first person echo for phase %d has been modified.\n",
				 phasenum);
		ch->send_to_char(output);
	}
	
	else if (!str_cmp (subcmd, "3rd"))
	{
		argument = one_argument (argument, buf);
		phase->third = duplicateString (buf);
		sprintf (output,
				 "The third person echo for phase %d has been modified.\n",
				 phasenum);
		ch->send_to_char(output);
	}
	
	else if (!str_cmp (subcmd, "group"))
	{
		argument = one_argument (argument, buf);
		phase->group_mess = duplicateString (buf);
		sprintf (output,
				 "The group echo for phase %d has been modified.\n",
				 phasenum);
		ch->send_to_char(output);
	}
	
	else if (!str_cmp (subcmd, "delete"))
	{
		if (phasenum == 1)
		{
			ch->send_to_char("If you want to delete the craft, use 'cset delete'.\n");
			return;
		}
		else
			for (i = 1; i < MAX_PHASES_PER_SUBCRAFT-1; i++)
			{
				phase = craft->phases[i];
				if (phasenum <= i)
					phase[i] = phase[i+1];
			}
		
		sprintf (output, "Phase %d has been deleted.\n", phasenum);
		ch->send_to_char(output);
	}
	
	else if (!str_cmp (subcmd, "time"))
	{
		argument = one_argument (argument, buf);
		if (!isdigit (*buf))
		{
			ch->send_to_char("The specified value for 'time' must be an integer.\n");
			return;
		}
		if (atoi (buf) < 1)
		{
			ch->send_to_char("The specified value for 'time' must be greater than one.\n");
			return;
		}
		phase->phase_seconds = atoi (buf);
		sprintf (output,
				 "The time delay for phase %d has been set to %d seconds.\n",
				 phasenum, atoi (buf));
		ch->send_to_char(output);
	}
	
	else if (!str_cmp (subcmd, "skill"))
	{
		if (phase->attribute > -1)
		{
			ch->send_to_char("Remove the attribute check if you wish to check against a skill\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		if (isdigit (*buf) || !(*buf))
		{
			ch->send_to_char("The specified 'skill' value must be a skill name.\n");
			return;
		}
		if (!str_cmp (buf, "none"))
		{
			phase->skill = 0;
			ch->send_to_char("The phase's skill check has been removed.\n");
			return;
		}
		if ((i = lookup_skill_id(buf)) == -1)
		{
			sprintf (output,
					 "I could not find the '%s' skill in our registry.\n",
					 buf);
			ch->send_to_char(output);
			return;
		}
		phase->skill = i;
		for (size_t iter = 0; iter <= strlen (buf); iter++)
			buf[iter] = toupper (buf[iter]);
		sprintf (output, "Phase %d will now check against the %s skill.\n",
				 phasenum, buf);
		ch->send_to_char(output);
	}
	
	else if (!str_cmp (subcmd, "check"))
	{
		argument = one_argument (argument, buf);
		if (!isdigit (*buf))
		{
			ch->send_to_char("The specified 'check' value must be in the format XdY, e.g. 2d35.\n");
			return;
		}
		if (!phase->skill)
		{
			ch->send_to_char("You must first specify a skill to check against for this phase.\n");
			return;
		}
		sscanf (buf, "%dd%d", &dice, &sides);
		if (dice < 1 || sides < 0)
		{
			ch->send_to_char("Both values specified must be greater than zero.\n");
			return;
		}
		phase->dice = dice;
		phase->sides = sides;
		sprintf (output,
				 "The phase's skill check has been changed to %dd%d.\n",
				 dice, sides);
		ch->send_to_char(output);
	}
	
	else if ( !str_cmp (subcmd, "attribute") )
	{
		if (phase->skill > 0)
		{
			ch->send_to_char("Remove the skill check if you wish to check an attribute.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		if (isdigit (*buf) || !(*buf))
		{
			ch->send_to_char("The specified 'attribute' value must be an attribute name.\n");
			return;
		}
		if ( !str_cmp (buf, "none") ) {
			phase->attribute = -1;
			ch->send_to_char("The phase's attribute test has been removed.\n");
			return;
		}
		if ( (i = index_lookup (attrs, buf)) == -1 ) {
			snprintf (output, MAX_STRING_LENGTH,  "I could not find the '%s' attribute in our files.\n", buf);
			ch->send_to_char(output);
			return;
		}
		phase->attribute = index_lookup (attrs, buf);
		
		
		for ( i = 0; i <= strlen(buf); i++ )
			buf[i] = toupper(buf[i]);
		snprintf (output, MAX_STRING_LENGTH,  "Phase %d will now test  against the %s attribute.\n", phasenum, buf);
		ch->send_to_char(output);
	}
	
	else if ( !str_cmp (subcmd, "checkattribute"))
	{
		argument = one_argument (argument, buf);
		if ( !isdigit (*buf) ) {
			ch->send_to_char("The specified 'check' value must be in the format XdY, e.g. 2d35.\n");
			return;
		}
		if ( phase->attribute == -1 ) {
			ch->send_to_char("You must first specify an attribute to test against for this phase.\n");
			return;
		}
		sscanf (buf, "%dd%d", &dice, &sides);
		if ( dice < 1 || sides < 0 ) {
			ch->send_to_char("Both values specified must be greater than zero.\n");
			return;
		}
		phase->dice = dice;
		phase->sides = sides;
		snprintf (output, MAX_STRING_LENGTH,  "The phase's attribute check has been changed to %dd%d.\n", dice, sides);
		ch->send_to_char(output);
	}
	
	
	else if (!strn_cmp (subcmd, "object", 6))
	{
		if (!isdigit (subcmd[6]))
		{
			ch->send_to_char("An object number must be specified, e.g. object6.\n");
			return;
		}
		sprintf (buf, "%c", subcmd[6]);
		
		if (isdigit (subcmd[7]))
			sprintf (buf + strlen (buf), "%c", subcmd[7]);
		
		objnum = atoi (buf);
		
		if (objnum < 1)
		{
			ch->send_to_char("The specified object set must be greater than 1, e.g. object6.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		if (!*buf)
		{
			ch->send_to_char("What object did you wish to set?\n");
			return;
		}
		
		if (!craft->obj_items[0])
		{
			craft->obj_items[0] = new DEFAULT_ITEM_DATA;
			craft->obj_items[0]->phase_num = phasenum;
		}
		
		for (i = 0; i < MAX_DEFAULT_ITEMS; i++)
		{
			items = craft->obj_items[i];
			if (i == objnum)
				break;
			if (!craft->obj_items[i + 1])
			{
				if (i + 1 != objnum)
				{
					sprintf (output,
							 "Please use the next available slot to add a new\nobject set; in this case, object%d.\n",
							 i + 1);
					ch->send_to_char(output);
					return;
				}
				else
				{
					craft->obj_items[i + 1] = new DEFAULT_ITEM_DATA;
					craft->obj_items[i + 1]->phase_num = phasenum;
				}
			}
		}//for (i = 0;
		memset (items->items, 0, sizeof(int)*MAX_DEFAULT_ITEMS);
		items->item_counts = 0;
		if (*buf == 'X' || *buf == 'x')
		{
			*buf2 = '\0';
			for (j = 1; j <= strlen (buf); j++)
				sprintf (buf2 + strlen (buf2), "%c", buf[j]);
			
			items->item_counts = atoi (buf2);
		}
		
		else if (isdigit (*buf))
		{
			for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
			{
				if (!items->items[j])
				{
					if (!vtoo (atoi (buf)) && atoi (buf) != 0)
					{
						sprintf (output,
								 "Sorry, but object VNUM %d could not be loaded for inclusion.\n",
								 atoi (buf));
						ch->send_to_char(output);
						break;
					}
					
					if (atoi (buf) == 0)
					{
						sprintf (output,
								 "Object set %d has been removed.\n",
								 objnum);
						ch->send_to_char(output);
						
						for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
							items->items[j] = 0;
						
						break;
					}
					sprintf (output,
							 "#2%s#0 has been added to object set %d.\n",
							 vtoo (atoi (buf))->short_description, objnum);
					output[2] = toupper (output[2]);
					ch->send_to_char(output);
					items->items[j] = atoi (buf);
					break;
				}
			}
		}
		
		while ((argument = one_argument (argument, buf)))
		{
			if (!isdigit (*buf))
				break;
			for (j = 0; j < MAX_DEFAULT_ITEMS; j++)
			{
				if (!items->items[j])
				{
					if (!vtoo (atoi (buf)) && atoi (buf) != 0)
					{
						sprintf (output,
								 "Sorry, but object VNUM %d could not be loaded for inclusion.\n",
								 atoi (buf));
						ch->send_to_char(output);
						break;
					}
					
					if (atoi (buf) == 0)
					{
						sprintf (output,
								 "Object set %d has been removed.\n",
								 objnum);
						ch->send_to_char(output);
						items->items[j] = 0;
						break;
					}
					
					sprintf (output,
							 "#2%s#0 has been added to object set %d.\n",
							 vtoo (atoi (buf))->short_description, objnum);
					output[2] = toupper (output[2]);
					ch->send_to_char(output);
					items->items[j] = atoi (buf);
					break;
				}
			}
		}
		
		if (!isdigit (*buf) && *buf)
		{
			if ((j = index_lookup (item_default_flags, buf)) == -1)
			{
				sprintf (output, "The item flag '%s' does not exist.\n",
						 buf);
				ch->send_to_char(output);
				return;
			}
			else
			{
				items->flags = 0;
				items->flags |= (1 << j);
				sprintf (output, "The item's '%s' flag has been set.\n",
						 item_default_flags[j]);
				ch->send_to_char(output);
			}
		}
		
		items->phase_num = phasenum;
	}
	
	else if (!strn_cmp (subcmd, "mobile", 6))
	{
		if (!isdigit (subcmd[6]))
		{
			ch->send_to_char("An mobile number must be specified, e.g. mobile9.\n");
			return;
		}
		sprintf (buf, "%c", subcmd[6]);
		
		if (isdigit (subcmd[7]))
			sprintf (buf + strlen (buf), "%c", subcmd[7]);
		
		mobnum = atoi (buf);
		
		if (mobnum < 1)
		{
			ch->send_to_char("The specified mobile set must be greater than 1, e.g. mobile9.\n");
			return;
		}
		
		argument = one_argument (argument, buf);
		if (!*buf)
		{
			ch->send_to_char("What mobile did you wish to set?\n");
			return;
		}
		
		if (!craft->mob_items[0])
		{
			craft->mob_items[0] = new DEFAULT_MOB_DATA;
			craft->mob_items[0]->phase_num = phasenum;
		}
		
		
		for (i = 0; i < MAX_DEFAULT_MOBS; i++)
		{
			mobs = craft->mob_items[i];
			if (i == mobnum)
				break;
			if (!craft->mob_items[i + 1])
			{
				if (i + 1 != mobnum)
				{
					sprintf (output,
							 "Please use the next available slot to add a new mobile set; in this case, mobile%d.\n",
							 i + 1);
					ch->send_to_char(output);
					return;
				}
				else
				{
					craft->mob_items[i+1] = new DEFAULT_MOB_DATA;
					craft->mob_items[i+1]->phase_num = phasenum;
				}
			}
		}
		
		memset (mobs->mobs, 0, MAX_DEFAULT_MOBS);
		
		if (isdigit (*buf))
		{
			for (j = 0; j < MAX_DEFAULT_MOBS; j++)
			{
				if (!mobs->mobs[j])
				{
					if (!vtom (atoi (buf)) && atoi (buf) != 0)
					{
						sprintf (output,
								 "Sorry, but mobile VNUM %d could not be loaded for inclusion.\n", atoi (buf));
						ch->send_to_char(output);
						break;
					}
					
					if (atoi (buf) == 0)
					{
						sprintf (output,
								 "Mobile set %d has been removed.\n",
								 objnum);
						ch->send_to_char(output);
						
						for (j = 0; j < MAX_DEFAULT_MOBS; j++)
							mobs->mobs[j] = 0;
						
						break;
					}
					sprintf (output,
							 "#5%s#0 has been added to mobile set %d.\n",
							 vtom (atoi (buf))->short_descr, mobnum);
					output[2] = toupper (output[2]);
					ch->send_to_char(output);
					mobs->mobs[j] = atoi (buf);
					break;
				}
			}
		}
		
		while ((argument = one_argument (argument, buf)))
		{
			if (!isdigit (*buf))
				break;
			
			for (j = 0; j < MAX_DEFAULT_MOBS; j++)
			{
				if (!mobs->mobs[j])
				{
					if (!vtom (atoi (buf)) && atoi (buf) != 0)
					{
						sprintf (output,
								 "Sorry, but Mbile VNUM %d could not be loaded for inclusion.\n",
								 atoi (buf));
						ch->send_to_char(output);
						break;
					}
					
					if (atoi (buf) == 0)
					{
						sprintf (output,
								 "Mobile set %d has been removed.\n",
								 mobnum);
						ch->send_to_char(output);
						mobs->mobs[j] = 0;
						break;
					}
					
					sprintf (output,
							 "#5%s#0 has been added to mobile set %d.\n",
							 vtom (atoi (buf))->short_descr, mobnum);
					output[2] = toupper (output[2]);
					ch->send_to_char(output);
					mobs->mobs[j] = atoi (buf);
					break;
				}
			}
		}
		
		if (!isdigit (*buf) && *buf)
		{
			if ((j = index_lookup (mob_default_flags, buf)) == -1)
			{
				sprintf (output, "The item flag '%s' does not exist.\n",
						 buf);
				ch->send_to_char(output);
				return;
			}
			else
			{
				mobs->flags = 0;
				mobs->flags |= (1 << j);
				sprintf (output, "The item's '%s' flag has been set.\n",
						 mob_default_flags[j]);
				ch->send_to_char(output);
			}
		}
		
		mobs->phase_num = phasenum;
	} 
	
	/** Phase error message **/
	else
	{
		ch->send_to_char("I'm afraid that isn't a recognized phase command.\n");
		return;
	}
	
	return;
}

