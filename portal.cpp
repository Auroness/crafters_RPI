//////////////////////////////////////////////////////////////////////////////
//
/// portal.cpp : Portal Building Module
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
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h>
#include <map>
#include <vector>
#include <list>
#include <utility>
#include </usr/local/mysql/include/mysql.h>

#include "structs.h"
#include "protos.h"
#include "utility.h"
#include "utils.h"

extern std::map<int, ROOM_PORTAL_DATA*> portal_map;
extern std::map<int, OBJECT_MATERIAL*> object_material_map;

extern const char *exit_dirs[];
extern const char *dirs[];
 



char * room_portal_data::sdesc_or_default(int side)
{
	std::string output;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	
	char *portal_type_bits[8] = {
		"Open-Space",
		"Door",
		"Gate",
		"Barrier",
		"Crossing",
		"Window",
		"Transport",
		"\n"
	};
	
	if (side == 1 && this->sdesc_1 && *this->sdesc_1)
		return this->sdesc_1;
	else if (side == 2 && this->sdesc_2 && *this->sdesc_2)
		return this->sdesc_2;
	else
	{	
		output.clear();
		
		sprintf(buf, "%s ", (strchr("aeiouyAEIOUY", tolower(portal_type_bits[this->type][0]))) ? "an" : "a");
		output.assign(buf);
		
		sprintf(buf, "%s", portal_type_bits[this->type]);
		*buf = tolower (*buf);
		output.append(buf);
		
		return ((char*)output.c_str());
	}
}
	
const char * room_portal_data::exit_dir(int side)
{
	int dir = 0;
	
	if (side == 1)
		dir = this->dir_1;
	else if (side == 2)
		dir = this->dir_2;
	else
		return "somewhere (bug me)"; //This should never ever happen.
	
	if (dir < 0)
		return "somewhere (A portal is missing an exit - pleast report this)"; //This should never happen.
		
	if (dir == UP)
		return "above";
	else if (dir == DOWN)
		return "below";
	else
		{
		std::string buf = "to the ";
		buf += dirs[dir];
		return buf.c_str();
		}
	}

bool room_portal_data::is_hidden()
{
	return IS_SET(this->port_flags, EX_SECRET);
}

void
portal_fdesc (CHAR_DATA * ch, char *argument)
{
	int side_num;
	char input[256];
	char buf[MAX_INPUT_LENGTH];
	ROOM_PORTAL_DATA *tport;
	
	tport = vtop(ch->pc->edit_portal);
	if (!tport)
	{
		ch->send_to_char("You are not currently editing a portal.\n");
		return;
	}

	argument = one_argument (argument, input);
	side_num = atoi(input);
	
	snprintf(buf, MAX_INPUT_LENGTH, "The old side 1 description was:\n%s\n\nThe old side 2 description was:\n%s\n", (tport->fdesc_1 ? tport->fdesc_1 : "(none)"), (tport->fdesc_2 ? tport->fdesc_2 : "(none)"));
	ch->send_to_char(buf);

	ch->act("$n begins editing a portal description.", false, 0, 0, TO_ROOM);
	
	free_mem(ch->desc->descStr);
	free_mem(ch->desc->pending_message);
	ch->desc->pending_message = new MESSAGE_DATA;
	ch->desc->descStr = ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;
	ch->delay_info1 = side_num;
	
	ch->send_to_char ("\nEnter a new description.  Terminate with an '@'\n");
	ch->make_quiet();
	
	ch->desc->proc = post_portal_fdesc;
	return;
}

void
post_portal_fdesc (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *ch = d->character;
	ROOM_PORTAL_DATA *tport = vtop(ch->pc->edit_portal);
	int side_num = ch->delay_info1;
	
	if (!tport)
	{
		ch->send_to_char("There was an error saving the description for this portal. Please report this to a coder.\n");
	}
	
	if (side_num == 1)
	{
		tport->fdesc_1 = duplicateString(d->descStr);
		
		std::stringstream tempstr;
		tempstr << reformat_desc(tport->fdesc_1);	
		tport->fdesc_1 = duplicateString(tempstr.str().c_str());
		
		ch->send_to_char("Descriptions installed on the first side of this portal.\n");
	}
	else if (side_num == 2)
	{
		tport->fdesc_2 = duplicateString(d->descStr);
		
		std::stringstream tempstr;
		tempstr << reformat_desc(tport->fdesc_2);	
		tport->fdesc_2 = duplicateString(tempstr.str().c_str());
		
		ch->send_to_char("Descriptions installed on second side of this portal.\n");
	}
	else
	{
		tport->fdesc_1 = duplicateString(d->descStr);
		tport->fdesc_2 = duplicateString(d->descStr);
		
		std::stringstream tempstr;
		tempstr << reformat_desc(tport->fdesc_1);	
		tport->fdesc_1 = duplicateString(tempstr.str().c_str());
		tempstr << reformat_desc(tport->fdesc_2);	
		tport->fdesc_2 = duplicateString(tempstr.str().c_str());
		
		
		ch->send_to_char("Descriptions installed on both sides of this portal.\n");
	}
	ch->delay_who = NULL;
	ch->delay_info1 = 0;
}

void
portal_key (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	int side;
	int key_num;
	int penalty;
	ROOM_PORTAL_DATA *tport;
	
		
	tport = vtop(ch->pc->edit_portal);
	if (!tport)
	{
		ch->send_to_char ("Sorry, could not find a portal.\n");
		return;
	}
	
	argument = one_argument (argument, buf);
	if (!isdigit (*buf))
	{
		ch->send_to_char ("Syntax:  pset key 1 | 2 <key-vnum> [pick-penalty]\n");
		return;
	}
	
	if (!isdigit (*buf) || atoi(buf) > 2 || atoi(buf) < 0)
	{
		ch->send_to_char ("Which side do you want to add a key to? (1 or 2)\n");
		return;
	}
	side = atoi(buf);
	
	argument = one_argument (argument, buf);
	if (!isdigit (*buf))
	{
		ch->send_to_char ("What is the vnum for the key?\n");
		return;
	}
	key_num = atoi(buf);
	
	argument = one_argument (argument, buf);
	if ((*buf) && !isdigit (*buf))
	{
		ch->send_to_char ("What is the pick penalty for this lock?\n");
		return;
	}
	
	if (!buf)
		penalty = 0;
	else 
	{
		penalty = atoi(buf);
	}
	
	//different lock on each side
	if (side == 1) 
	{
		tport->key_num_1 = key_num;
		tport->pick_key_pen_1 = penalty;
		sprintf(buf, "Key number %d with a %d point penalty was added to room %d.\n", key_num, penalty, tport->room_1);
	}
	else if (side == 2) 
	{
		tport->key_num_2 = key_num;
		tport->pick_key_pen_2 = penalty;
		sprintf(buf, "Key number %d with a %d penalty was added to room %d.\n", key_num, penalty, tport->room_2);
	}
	
	ch->send_to_char(buf);
	return;
}

void
portal_direction (CHAR_DATA * ch, char *argument)
{
	char input[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	int side;
	int dir;
	int room_num;
	ROOM_PORTAL_DATA *tport;
	
	tport = vtop(ch->pc->edit_portal);
	if (!tport)
	{
		ch->send_to_char("You are not currently editing a portal.\n");
		return;
	}
	
	argument = one_argument(argument, input);
	if (!isdigit(*input))
	{
		ch->send_to_char("Syntax:  pset dir 1 | 2 <direction> [room number]\n");
		return;
	}
	
	if (atoi(input) > 2 || atoi(input) < 0)
	{
		ch->send_to_char("Which side of the portal are you working with? (1 or 2)\n");
		return;
	}
	side = atoi(input);
	
	argument = one_argument(argument, input);
	dir = index_lookup(dirs, input);
	
	if (dir == -1)
	{
		ch->send_to_char ("What direction is that?\n");
		return;
	}
	
		
	argument = one_argument(argument, input);
	if (!*input || !isdigit(*input))
	{
		ch->send_to_char ("What is the room number?\n");
		return;
	}
	
	room_num = atoi(input);
	
	if (side == 1)
	{
		tport->room_1 = ch->in_room;
		if (room_num)
			tport->room_2 = room_num;
		tport->dir_1 = dir;
		tport->dir_2 = rev_dir[dir];
		snprintf(buf, MAX_INPUT_LENGTH, "You have set the %s side of this room to connect to the %s side of room %d.",
				dirs[tport->dir_1],
				dirs[tport->dir_2],
				room_num);
	}
	else if (side == 2)
	{
		if (room_num)
			tport->room_2 = room_num;
		tport->dir_1 = rev_dir[dir];
		tport->dir_2 = dir;
		snprintf(buf, MAX_INPUT_LENGTH, "You have set the %s side of room %d to connect to the %s side of this room.",
				dirs[tport->dir_2],
				tport->room_2,
				dirs[tport->dir_1]);
	}
	
	ch->send_to_char(buf);
	return;
}


	//argument will be "n", "##", "n !", or "## !"
void 
portal_new(CHAR_DATA * ch, char *argument)
{
	char input[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	ROOM_PORTAL_DATA *tport;
	ROOM_DATA *troom;
	MYSQL_RES *result;
	int port_num;
	int iter;
	bool placed = false;
	
	argument = one_argument(argument, input);
	//argument is now "" or "!"
	//input is now "n" or "##"
	
	if (ch->pc->edit_portal && *argument != '!')
	{
		snprintf(buf, MAX_INPUT_LENGTH, "You are still editing portal %d.\n Use 'pset save' to save your work. Otherwise, use #3'pset new %s !'#0, without the quotes, to discard old changes and start on a new portal.\n", ch->pc->edit_portal, input);
		ch->send_to_char (buf);
		return;
	}

	ch->pc->edit_portal = 0;
	
	if (!*input)
	{
		ch->send_to_char
		("You must supply a new portal number, or 'n' to use the next available slot.\n");
		return;
	}
	
	
	if (!isdigit(*input) && *input != 'n')
	{
		ch->send_to_char
		("The argument must be a digit, or 'n' to use the next available slot.\n");
		return;
	}
	
	if (isdigit(*input))
	{
		port_num = atoi(input);
	}

	if (vtop(port_num))
	{
		ch->send_to_char ("That portal number already exists.\n");
		return;
	}
	else if (*input == 'n')
	{
		snprintf(buf, MAX_INPUT_LENGTH, "INSERT INTO portals (room_1, last_editor, status) VALUES (%d, '%s', %d)", ch->in_room, ch->keywords, BUILD_APPROVED);
		
		mysql_safe_query(buf);
		
		if ((result = mysql_store_result(database)) == 0 &&
			mysql_field_count(database) == 0 &&
			mysql_insert_id(database) != 0)
		{
			port_num = mysql_insert_id(database);
		}
		
		if (port_num == '0' || !port_num)
		{
			ch->send_to_char ("There was a problem generating a portal number. Try again later.\n");
			return;
		}
		
	}
	
	troom = vtor(ch->in_room);
	
	for (iter = 0; iter < MAX_PORTALS; iter++)
	{
		if (troom->portals[iter] == -1)
		{
			troom->portals[iter] = port_num;
			placed = true;
			break;
		}
	}
	
	tport = new room_portal_data;
	tport->room_portal_init();
	tport->ident = port_num;
	tport->room_1 = troom->nVirtual;
	portal_map[tport->ident] = tport;
	
	if (placed)
	{
		snprintf (buf, MAX_INPUT_LENGTH, "Portal %d has been initialized in room %d. \n", port_num, ch->in_room);
	ch->send_to_char (buf);
	}
	else 
	{
		snprintf (buf, MAX_INPUT_LENGTH, "There was a problem placing a portal in room %d. \n", ch->in_room);
		ch->send_to_char (buf);
		return;
	}
	
	ch->pc->edit_portal = tport->ident; //set them up to edit it, since they created it
	return;
	

}

	//pset - (not editing a portal) message about not editing
	//pset - (editing a portal) pstat the portal
	//pset ? - displays help message
	//pset ## - (not editing a portal) starts editing
	//pset ## - (editing a portal) gives warning
	//pset ## ! - (editing a portal) dumps old portal an starts new one
	//pset <options> - does something special, depending on <option>
void
do_pset (CHAR_DATA * ch, char *argument, int cmd)
{
	char subcmd[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	ROOM_PORTAL_DATA *edit_port;
	ROOM_PORTAL_DATA *tport;

	if (IS_NPC (ch))
	{
		ch->send_to_char("Only PC can use this command.\n");
		return;
	}
	
	builder_log(ch, "pset", argument);
	
	argument = one_argument (argument, subcmd);
	
	if (!*subcmd)
	{
		if (ch->pc->edit_portal)
		{
			pstat(ch, ch->pc->edit_portal);
		}
		else
		{
			ch->send_to_char("This room has the following portals:\n");
			for (int i = 0; i <= MAX_PORTALS; i++)
			{
				if (ch->room->portals[i] && (tport = vtop(ch->room->portals[i])))
					{
					ch->send_to_char(plist_show(tport).c_str());
					}
				}
			}
		return;
	}
	
	else if (*subcmd == '?')
	{
		ch->send_to_char ("\n\nSyntax:\n");
		ch->send_to_char ("     pset new < \'n\' | number > \n");
		ch->send_to_char ("     pset delete\n");
		ch->send_to_char ("     pset save\n");
		ch->send_to_char ("     pset place\n");
		ch->send_to_char ("     pset portal remove\n");
		ch->send_to_char ("     pset type <portal type>\n");
		ch->send_to_char ("     pset bzone <value>\n");
		ch->send_to_char ("     pset room [1,2] <room number>\n");
		ch->send_to_char ("     pset dir [1,2] <direction> [room number]\n");
		ch->send_to_char ("     pset key [1,2] <vnum> [pick penalty]\n");
		ch->send_to_char ("     pset flag <portal flag>\n");
		ch->send_to_char ("     pset sdesc [1,2] <description> (omit 1/2 to set both)\n");
		ch->send_to_char ("     pset ldesc [1,2] <description> (omit 1/2 to set both)\n");
		ch->send_to_char ("     pset fdesc [1,2] (omit 1/2 to set both)\n");
		ch->send_to_char("(You are dropped into an editor to add this information.)\n");
		ch->send_to_char ("     pset keywords [1,2] <words> (omit 1/2 to set both)\n");
		ch->send_to_char ("     pset material <value>\n");
		ch->send_to_char ("     pset quality <value>\n");
		ch->send_to_char ("     pset diffculty <value>\n");
		ch->send_to_char ("     pset skill <value> or 'default'\n");
		ch->send_to_char ("     *pset fail <room num>\n");
		ch->send_to_char ("\nItems marked with an * are not functional yet\n");
		return;
	}
	
		//subcmd is a number
	else if (atoi(subcmd) != 0)
	{
		edit_port = vtop(atoi(subcmd));
		if (!edit_port)
		{
			ch->send_to_char("The portal you are attempting to edit does not exist.\n");
			return;
		}
			
		if (ch->pc->edit_portal && *argument != '!')
		{
			snprintf(buf, MAX_INPUT_LENGTH, "You are still editing portal %d.\nUse 'pset save' to save your work. Otherwise, use #3'pset %s !'#0, without the quotes, to discard old changes and start on a new portal.\n", ch->pc->edit_portal, subcmd);
			ch->send_to_char (buf);
		}
		else
		{
			ch->pc->edit_portal = edit_port->ident;
			snprintf(buf, MAX_INPUT_LENGTH, "You are now editing portal %d.\n", ch->pc->edit_portal);
			ch->send_to_char (buf);
		}
		return;
	}
	
	//subcmd is not a number
	else if (!str_cmp (subcmd, "new"))
	{
		portal_new(ch, argument);
		return;
	}
			
	else if ((edit_port = vtop(ch->pc->edit_portal)))
	{
		if (!str_cmp (subcmd, "type"))
		{
			portal_type(ch, argument);	
		}
		
		else if (!str_cmp (subcmd, "place"))
		{
			room_place_portal(ch, edit_port);	
		}
		
		else if (!str_cmp (subcmd, "portal"))
		{
			room_remove_portal(ch, argument);	
		}
		
		else if (!str_cmp (subcmd, "save"))
		{
			portal_save(ch);
		}
		
		else if (!str_cmp (subcmd, "bzone"))
		{
			argument = one_argument (argument, subcmd);
			if (atoi(subcmd))
			{
			edit_port->zone = atoi(subcmd);	
				snprintf(buf, MAX_INPUT_LENGTH, "The zone has been changed to %d.\n", edit_port->zone);
			}
			else
			{
				ch->send_to_char("Please specify a zone for this portal.\n");
				snprintf(buf, MAX_INPUT_LENGTH, "Current build zone for Portal %d: %d.", edit_port->ident, edit_port->zone);
				ch->send_to_char(buf);
			}
		}
		
		else if (!str_cmp (subcmd, "delete"))
		{
			if (edit_port->deleted == 1)
			{
				ch->send_to_char ("Portal will NOT be deleted.\n");
				edit_port->deleted = 0;
			}
			else
			{
				ch->send_to_char ("Portal will be deleted. Use \"pset delete\" again to stop deletion.\n");
				edit_port->deleted = 1;
			}
			
		}
		
		else if (!str_cmp (subcmd, "key"))
		{
			if (edit_port->type == PORTAL_SPACE)
			{
				sprintf(buf, "You cannot have a key for an open space.\n");
				ch->send_to_char(buf);
			}
			
			else
			{
			portal_key(ch,argument);	
			}
		}
		
		else if (!str_cmp (subcmd, "room"))
		{
			portal_room(ch,argument);	
		}
		
		else if (!str_cmp (subcmd, "material"))
		{
			if (edit_port->type == PORTAL_SPACE)
			{
				sprintf(buf, "You cannot set material for an opening.\n");
				ch->send_to_char(buf);
			}
			else
			{				
			portal_material(ch,argument);
			}
		}
		
		else if (!str_cmp (subcmd, "quality"))
		{
			if (edit_port->type == PORTAL_SPACE)
			{
				sprintf(buf, "You cannot set the quality for an opening.\n");
				ch->send_to_char(buf);
			}
			else
			{
				
				argument = one_argument (argument, subcmd);
				if (atoi(subcmd))
				{
					edit_port->quality = atoi(subcmd);	
					snprintf(buf, MAX_INPUT_LENGTH, "The quality has been changed to %d.\n", edit_port->quality);
					ch->send_to_char (buf);	
				}
				else
				{
					ch->send_to_char("Please specify a quality for this portal.\n");
					snprintf(buf, MAX_INPUT_LENGTH, "Current quality for Portal %d: %d.", edit_port->ident, edit_port->quality);
					ch->send_to_char(buf);
				}
			}
		}
		
		else if (!str_cmp (subcmd, "difficulty"))
		{
			argument = one_argument (argument, subcmd);
			if ((edit_port->type == PORTAL_DOOR) 
				||(edit_port->type == PORTAL_GATE)
				|| (edit_port->type == PORTAL_SPACE))
			{
				sprintf(buf, "You cannot set the difficulty on a door, gate or opening.\n");
				ch->send_to_char(buf);
			}
			
			else
			{
				if (atoi(subcmd))
				{
					edit_port->difficulty = atoi(subcmd);	
					snprintf(buf, MAX_INPUT_LENGTH, "The difficulty has been changed to %d.\n", edit_port->difficulty);
					ch->send_to_char(buf);
				}
				else
				{
					ch->send_to_char("Please specify a difficulty for this portal.\n");
					snprintf(buf, MAX_INPUT_LENGTH, "Current difficulty for portal %d: %d.", edit_port->ident, edit_port->difficulty);
					ch->send_to_char(buf);
				}
			}
		}
		
		else if (!str_cmp (subcmd, "slope"))
		{
			portal_slope(ch,argument);	
		}
		
		else if (!str_cmp (subcmd, "fail"))
		{
			if (edit_port->type != PORTAL_CROSSING)
			{
				sprintf(buf, "You can only set failure rooms on Crossings\n");
				ch->send_to_char(buf);
				return;
			}
			
			portal_fail(ch,argument);	
		}
		
		else if (!str_cmp (subcmd, "skill"))
		{
			if ((edit_port->type == PORTAL_DOOR) 
				||(edit_port->type == PORTAL_GATE)
				|| (edit_port->type == PORTAL_SPACE))
			{
				sprintf(buf, "You cannot set skill on a door, gate or opening.\n");
				ch->send_to_char(buf);
			}
			
			else
			{
			portal_skill(ch, argument);	
			}
		}
		
		else if (!str_cmp (subcmd, "direction") || !str_cmp (subcmd, "dir"))
		{
			portal_direction(ch, argument);	
		}
		
		else if (!str_cmp (subcmd, "flag"))
		{
			portal_flag(ch, argument);	
		}
		
		else if (!str_cmp (subcmd, "sdesc"))
		{
			portal_sdesc(ch, argument);	
			
		}	
		
		else if (!str_cmp (subcmd, "ldesc"))
		{
			if ((edit_port->type == PORTAL_DOOR) 
				||(edit_port->type == PORTAL_GATE)
				|| (edit_port->type == PORTAL_SPACE))
			{
				sprintf(buf, "You cannot set the long description on a door, gate or opening.\n");
				ch->send_to_char(buf);
			}
			
			else
			{
			portal_ldesc(ch, argument);	
			}
		}
		
		else if (!str_cmp (subcmd, "keywords"))
		{
			portal_keywords(ch, argument);	
		}
		
		else if (!str_cmp (subcmd, "fdesc"))
		{
			portal_fdesc(ch, argument);	
		}
		else 
		{
			ch->send_to_char("That command is not recognized. Try \"pset ?\" for help.\n");
		}
	}
	else
	{
		ch->send_to_char("You are not currently editing a portal.\n");
	}
}

/******
 * plist - will display all portals in the game. 
 *******/
void
do_plist (CHAR_DATA * ch, char *argument, int cmd)
{
	ROOM_PORTAL_DATA *tport;
	std::map<int, ROOM_PORTAL_DATA*>::iterator portal_iterator;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char key1[MAX_STRING_LENGTH] = { '\0' };
	char key2[MAX_STRING_LENGTH] = { '\0' };
	char key3[MAX_STRING_LENGTH] = { '\0' };
	std::string temp_buf;
	int count = 0;
	int inclusive;
	int port_zone = -1;
	int yes_key1 = 0;
	int yes_key2 = 0;
	int yes_key3 = 0;
	int flag_key1 = -2;
	int flag_key2 = -2;
	int flag_key3 = -2;
	int yes_flag1 = 0;
	int yes_flag2 = 0;
	int yes_flag3 = 0;
	int inc_flags;
	int port_type = -1;
	
	const char *exit_bits[8] = {
		"Closed",
		"Locked",
		"PickProof",
		"Secret",
		"Gate",
		"Door",
		"NoSneak",
		"\n"
	};
	
	char *portal_type_bits[8] = {
		"Open-Space",
		"Door",
		"Gate",
		"Barrier",
		"Crossing",
		"Window",
		"Transport",
		"\n"
	};
	
	if (portal_map.empty())
	{
		ch->send_to_char ("Portal Map is Empty\n\n");
		return;
	}
		
	
	temp_buf.assign("\n");
	
	argument = one_argument (argument, buf);
	
	if (!*buf || !str_cmp(buf, "?"))
	{
		ch->send_to_char ("Selection Parameters:\n\n");
		ch->send_to_char
		("   +/-<keyword>       Include/exclude keyword.\n");
		ch->send_to_char ("   z <zone>                Portals in zone only.\n");
		ch->send_to_char
		("   $<flag>                 Include portals with flag.\n");
		ch->send_to_char
		("   /<flag>                 Exclude portals with flag.\n");
		ch->send_to_char
		("   t  <type>             Include portals with type.\n");
		
		ch->send_to_char ("\nExample:   plist +water -fall $closed t barrier z 10\n");
		ch->send_to_char
		("will only get barrier portals with 'water', but not 'fall', that are closed, in zone 10.\n");
		return;
	}
	
	while (*buf)
	{
		inclusive = 1;
		inc_flags = 1;
		
		if (strlen (buf) > 1
			&& isalpha (*buf)
			&& (port_type != -1))
		{
			argument = one_argument (argument, buf);
			continue;
		}
		
		if (isdigit(*buf))
		{
			if ((port_zone = atoi (buf)) >= MAX_ZONE)
			{
				ch->send_to_char ("Zone not in range 0..99\n");
				return;
			}
			
			argument = one_argument (argument, buf);
			continue;
		}
		
		switch (*buf)
		{
				
			case '-':
				inclusive = 0;
				
			case '+':
				
				if (!buf[1])
				{
					ch->send_to_char("Expected keyword after '+/-'.\n");
					return;
				}
				
				if (!*key1)
				{
					yes_key1 = inclusive;
					strcpy (key1, buf + 1);
				}
				else if (!*key2)
				{
					yes_key2 = inclusive;
					strcpy (key2, buf + 1);
				}
				else if (*key3)
				{
					ch->send_to_char("Sorry, at most three keywords.\n");
					return;
				}
				else
				{
					yes_key3 = inclusive;
					strcpy(key3, buf + 1);
				}
				
				break;
				
			case 'z':
				
				argument = one_argument (argument, buf);
				
				if (!isdigit(*buf) || atoi(buf) >= MAX_ZONE)
				{
					ch->send_to_char("Expected valid zone after 'z'.\n");
					return;
				}
				
				port_zone = atoi(buf);
				
				break;
				
			case '/':
				inc_flags = 0;
				
			case '$':
	
				if (!buf[1])
				{
					ch->send_to_char("Expected flag name after '/ or $'.\n");
					return;
				}
				
				if (flag_key1 == -2)
				{
					yes_flag1 = inc_flags;
					flag_key1 = index_lookup (exit_bits, buf + 1);
				}
				else if (flag_key2 == -2)
				{
					yes_flag2 = inc_flags;
					flag_key2 = index_lookup (exit_bits, buf + 1);
				}
				else if (flag_key3 != -2)
				{
					ch->send_to_char("Sorry, at most three flags.\n");
					return;
				}
				else
				{
					yes_flag3 = inc_flags;
					flag_key3 = index_lookup (exit_bits, buf + 1);
				}
				
				break;
			
			case 't':
				argument = one_argument (argument, buf);
				if (!buf[1])
				{
					ch->send_to_char("Expected portal type name.\n");
					return;
				}
				else
					port_type = index_lookup (portal_type_bits, buf);

				break;				
		}
		
		argument = one_argument (argument, buf);
	} // while (*buf)

	
	for (portal_iterator = portal_map.begin(); portal_iterator != portal_map.end(); portal_iterator++)
	{
		tport = portal_iterator->second;
		
		if (port_zone != -1 && tport->zone != port_zone)
			continue;
		
		if (port_type != -1 && tport->type != port_type)
			continue;
		
		if (flag_key1 != -2)
		{
			if (yes_flag1 && !(IS_SET(tport->port_flags, (1 << flag_key1))))
				continue;
			else if (!yes_flag1 && (IS_SET(tport->port_flags, (1 << flag_key1))))
				continue;
		}
		
		if (flag_key2 != -2)
		{
			if (yes_flag2 && !(IS_SET(tport->port_flags, (1 << flag_key2))))
				continue;
			else if (!yes_flag2 && (IS_SET(tport->port_flags, (1 << flag_key2))))
				continue;
		}
		
		if (flag_key3 != -2)
		{
			if (yes_flag3 && !(IS_SET (tport->port_flags, (1 << flag_key3))))
				continue;
			else if (!yes_flag3 && (IS_SET (tport->port_flags, (1 << flag_key3))))
				continue;
		}
		
		
		if (*key1)
		{
			if (yes_key1 && (!isname (key1, tport->keywords_1) && !isname (key1, tport->keywords_2)))
				continue;
			else if (!yes_key1 && (isname (key1, tport->keywords_2) && isname (key1, tport->keywords_2)))
				continue;
		}
		
		if (*key2)
		{
			if (yes_key2 && (!isname (key2, tport->keywords_1) && !isname (key2, tport->keywords_2)))
				continue;
			else if (!yes_key2 && (isname (key2, tport->keywords_2) && isname (key2, tport->keywords_2)))
				continue;
		}
		
		if (*key3)
		{
			if (yes_key3 && (!isname (key3, tport->keywords_1) && !isname (key3, tport->keywords_2)))
				continue;
			else if (!yes_key3 && (isname (key3, tport->keywords_2) && isname (key3, tport->keywords_2)))
				continue;
		}

		/*******/
		count++;
		
		if (count < 200)
			temp_buf.append(plist_show (tport)); //prints the first 200 portals to temp_buf
		
	}
	
	if (count > 200)
	{
		snprintf (buf, MAX_STRING_LENGTH,
				 "You have selected %d portals (too many to print).\n",
				 count);
		ch->send_to_char (buf);
		return;
	}
	
	page_string (ch->desc, temp_buf.c_str());
}

std::string
plist_show (ROOM_PORTAL_DATA * tportal)
{
	char *bitbuf;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	std::string response;
	
	const char *exit_bits[8] = {
		"Closed",
		"Locked",
		"PickProof",
		"Secret",
		"Gate",
		"Door",
		"NoSneak",
		"\n"
	};
	
	const char *portal_type_bits[8] = {
		"Open-Space",
		"Door",
		"Gate",
		"Barrier",
		"Crossing",
		"Window",
		"Transport",
		"\n"
	};
	
	if (!tportal)
		return (NULL);
	
	response.empty();
	
	bitbuf = strdup(sprintbit (tportal->port_flags, exit_bits));
	snprintf (buf, MAX_STRING_LENGTH, "#2Portal:#0  %2d  connects rooms %d and %d (%s to %s)]  #2Type:#0  %s \n",
			  tportal->ident,
			  tportal->room_1,
			  tportal->room_2,
			  dirs[tportal->dir_1],
			  dirs[tportal->dir_2],
			  portal_type_bits[tportal->type]);
	response.append(buf);
	
	
	return response;
}

void
portal_type (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char ptype[MAX_STRING_LENGTH]= { '\0' };
	ROOM_PORTAL_DATA *tport;
	int flag = -1;
	
	char *portal_type_bits[8] = {
		"Open-Space",
		"Door",
		"Gate",
		"Barrier",
		"Crossing",
		"Window",
		"Transport",
		"\n"
	};
	
	argument = one_argument(argument, ptype);
	
	if (*ptype && ptype[0] != '?')
		flag = index_lookup(portal_type_bits, ptype);
		
	if (flag == -1)
	{
		ch->send_to_char("Syntax:  pset type <value>\n");
		ch->send_to_char("Possible types are:\n");
		// Display a current list of the available types so we can't forget to change this command.
		for (int i = 0; portal_type_bits[i][0] != '\n'; i++)
		{
			snprintf(buf, MAX_STRING_LENGTH, "  %s\n", portal_type_bits[i]);
			ch->send_to_char(buf);
		}
		return;
	}
	
	tport = vtop(ch->pc->edit_portal);
		
	if (!tport)
	{
		ch->send_to_char("You are not currently editing a portal.\n");
		return;
	}
	
	tport->type = flag;
	tport->port_flags = 0;
	
	if (tport->type == PORTAL_DOOR)
		tport->port_flags |= EX_ISDOOR;
	else if (tport->type == PORTAL_GATE)
		tport->port_flags |= EX_ISGATE;
		
	
	snprintf(buf, MAX_STRING_LENGTH, "Portal %d is now a(n) %s.\n", tport->ident, portal_type_bits[tport->type]);
	ch->send_to_char (buf);
}

void
pstat (CHAR_DATA* ch, int port_num)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char *buf2;
	ROOM_PORTAL_DATA *tport;
	
	char *portal_type_bits[8] = {
		"Open-Space",
		"Door",
		"Gate",
		"Barrier",
		"Crossing",
		"Window",
		"Transport",
		"\n"
	};
	
	const char *exit_bits[8] = {
		"Closed",
		"Locked",
		"PickProof",
		"Secret",
		"Gate",
		"Door",
		"NoSneak",
		"\n"
	};
	
	tport = vtop(port_num);
	if (!tport)
	{
		snprintf(buf, MAX_STRING_LENGTH, "Sorry, portal %d does not exist.\n", port_num);
		ch->send_to_char(buf);
		return;	
	}
	
	buf2 = strdup(sprintbit (tport->port_flags, exit_bits));
	snprintf (buf, MAX_STRING_LENGTH, "#6Portal:#0  %d  #2Zone:#0 %d  #2Type:#0 %s  #2Flags:#0  %s\n",
			 tport->ident,
			 tport->zone,
			 portal_type_bits[tport->type],
			  buf2 ? buf2:"");
	ch->send_to_char (buf);
	
	snprintf(buf, MAX_STRING_LENGTH, "#2Quality:#0  %d  #2Difficulty:#0  %d  #2Skill:#0  %s  #2Slope:#0  %-3.1f\n",
			tport->quality,
			tport->difficulty,
			lookup_skill_name(tport->skill),
			tport->slope);
	ch->send_to_char(buf);
	
	snprintf(buf, MAX_STRING_LENGTH, "#2Fail Room:#0  %d  #2Feet Fallen:#0  %d\n",
			 tport->fail_room,
			 tport->feet_fallen);
	ch->send_to_char(buf);
	
	/**/
	snprintf (buf, MAX_STRING_LENGTH, "#2Material: #0");
	if (!tport->material.empty())
	{
		std::list<std::string>::iterator materials_it;
		std::string tmp_str;
		for ( materials_it = tport->material.begin(); materials_it != tport->material.end(); materials_it++ )
		{
			tmp_str = *materials_it;
			snprintf(buf + strlen(buf), MAX_STRING_LENGTH, "%s ", tmp_str.c_str());
			tmp_str.clear();
		}
	}
	snprintf(buf + strlen(buf), MAX_STRING_LENGTH, "\n\n");
	ch->send_to_char (buf);
	/**/
	
	
	
	snprintf(buf, MAX_STRING_LENGTH, "#6Side 1:#0  %d (%s)\n", tport->room_1, dirs[tport->dir_1]);
	snprintf(buf + strlen(buf), MAX_STRING_LENGTH, "#2Short:#0  %s\n#2Long:#0  %s\n#2Description:#0  %s\n#2Keywords:#0  %s\n",
			tport->sdesc_1 ? tport->sdesc_1 : "(none)",
			tport->ldesc_1 ? tport->ldesc_1 : "(none)",
			tport->fdesc_1 ? tport->fdesc_1 : "(none)",
			tport->keywords_1 ? tport->keywords_1 : "(none)");
	
	snprintf(buf + strlen(buf), MAX_STRING_LENGTH, "#2Key:#0  %d  #2Pick Penalty:#0   %d\n\n",
			tport->key_num_1 ? tport->key_num_1 : 0,
			tport->pick_key_pen_1 ? tport->pick_key_pen_1 : 0);
	ch->send_to_char(buf);
	
	snprintf(buf, MAX_STRING_LENGTH, "#6Side 2:#0  %d (%s)\n", tport->room_2, dirs[tport->dir_2]);
	snprintf(buf + strlen(buf), MAX_STRING_LENGTH, "#2Short:#0  %s\n#2Long:#0  %s\n#2Description:#0  %s\n#2Keywords:#0  %s\n",
			tport->sdesc_2 ? tport->sdesc_2 : "(none)",
			tport->ldesc_2 ? tport->ldesc_2 : "(none)",
			tport->fdesc_2 ? tport->fdesc_2 : "(none)",
			tport->keywords_2 ? tport->keywords_2 : "(none)");
	
	snprintf(buf + strlen(buf), MAX_STRING_LENGTH, "#2Key:#0  %d  #2Pick Penalty:#0   %d\n\n",
			tport->key_num_2 ? tport->key_num_2 : 0,
			tport->pick_key_pen_2 ? tport->pick_key_pen_2 : 0);
	ch->send_to_char(buf);
	
	snprintf(buf, MAX_STRING_LENGTH, "The %s side of room %d is connected to the %s side of room %d.\n",
			dirs[tport->dir_1],
			tport->room_1,
			dirs[tport->dir_2],
			tport->room_2);
	ch->send_to_char (buf);
	
	if ((vtor(tport->room_1))
		&& (vtor(tport->room_2))
		&& is_portal_placed(tport, vtor(tport->room_1))
		&& is_portal_placed(tport, vtor(tport->room_2)))
	{
		snprintf(buf, MAX_STRING_LENGTH, "This portal has been placed in rooms %d and %d.\n", tport->room_1, tport->room_2);
		ch->send_to_char(buf);
	}
	else 
		ch->send_to_char("This portal has NOT been placed.\n");	
}

int
room_place_portal(CHAR_DATA * ch, ROOM_PORTAL_DATA *portal)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	ROOM_DATA *troom1;
	ROOM_DATA *troom2;
	int iter;
	bool placed_1 = false;	//is it placed in in first room
	bool placed_2 = false;	//is it placed in the second room
	bool located = false;	//do the rooms sharing a border in the right direction
	float delta_x;
	float delta_y;
	float delta_z;
	
	troom1 = vtor(portal->room_1);
	troom2 = vtor(portal->room_2);
	if (!troom1 || !troom2)
	{
	sprintf(buf, "Portal could not be placed. Plase check your information and try again.\n");
	ch->send_to_char(buf);
	return 0;
	}	
	/**** removed until we get coo-ords working in builder maps ****
	located = test_location_rooms(ch, portal);
	if (!located)
	{
		sprintf (buf, "Portal %d could not be placed, because the rooms are not properly spaced. Check your coordiantes and room sizes. \n", portal->ident);
		ch->send_to_char(buf);
		return 0;	
	}
	else
	{
		delta_x = troom2->x_coord - troom1->x_coord;
		delta_y = troom2->y_coord - troom1->y_coord;
		delta_z = troom2->z_coord - troom1->z_coord;
		portal->slope = ((delta_z)/(sqrt(pow(delta_x, 2) + pow(delta_y, 2))));
		save_mysql_portal (ch, portal, BUILD_APPROVED);
	}
		*****/
	
	for (iter = 0; iter < MAX_PORTALS; iter++)
	{ 
			//add portal to the first room
		if ((troom1->portals[iter] == -1) && (!placed_1))
		{
			troom1->portals[iter] = portal->ident;
			portals_to_dir_options(troom1);  //loads dir_options when we place the portal
			placed_1 = true;
		}
			//portal already in the list
		else if (troom1->portals[iter] == portal->ident)
		{
			placed_1 = true;
			continue;
		}
		
			//add portal to the second room
		if ((troom2->portals[iter] == -1) && (!placed_2))
		{
			troom2->portals[iter] = portal->ident;
			portals_to_dir_options(troom2);  //loads dir_options when we place the portal
			placed_2 = true;
		}
			//portal already in the list
		else if (troom2->portals[iter] == portal->ident)
		{
		placed_2 = true;
			continue;
		}
	}
	
	if (!portal_map[portal->ident])   //not loaded in game yet?
		portal_map[portal->ident]= portal;
	
	portal->to_place = true;
	save_mysql_portal(ch, portal, BUILD_APPROVED);
	save_mysql_room (NULL, troom1, BUILD_APPROVED);
	save_mysql_room (NULL, troom2, BUILD_APPROVED);
	
	if (portal->to_place)
	{
		sprintf (buf, "Portal %d has been placed in room %d and room %d. \n", portal->ident, portal->room_1, portal->room_2);
		ch->send_to_char(buf);
	}
	else 
	{
		sprintf (buf, "There was a problem placing portal %d. \n", portal->ident);
		ch->send_to_char(buf);
		return 0;
	}
	
	return 1;
	
}

	//for all portals in the rooms list, verify they have one side in that room, otherwise, remove them from the room's list
void
portal_belong(ROOM_DATA *troom)
{
	int iter;
	int port_num;
	ROOM_PORTAL_DATA *tport;
	
	for (iter = 0; iter < MAX_PORTALS; iter++)
	{
		port_num = troom->portals[iter];
		if ((tport = vtop(port_num)))
		{
			if (tport->room_1 == troom->nVirtual)			
				break;
			else if (tport->room_2 == troom->nVirtual)
				break;
			else
			{
				troom->portals[iter] = -1;
			}
		}
		else
		{
			troom->portals[iter] = -1;
		}
	}
	return;
}

	//does the room have the portal in its list?
bool
is_portal_placed(ROOM_PORTAL_DATA *tport, ROOM_DATA *troom)
{
	int iter;
	
	if ((troom->nVirtual == tport->room_1) || (troom->nVirtual == tport->room_2))
	{
		for (iter = 0; iter < MAX_PORTALS; iter++)
		{
			if (troom->portals[iter] == tport->ident) 
				return(true);
		}
	}
	else
		return(false);
	
	return(false);

}

	//pset room 1 | 2 <room number> - sets the room number
void
portal_room (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	int side;
	int room_num;
	ROOM_PORTAL_DATA *tport;
	
	
	tport = vtop(ch->pc->edit_portal);
	if (!tport)
	{
		ch->send_to_char ("Sorry, could not find a portal.\n");
		return;
	}
	
	argument = one_argument (argument, buf);
	if (!isdigit (*buf))
	{
		ch->send_to_char ("Syntax:  pset room 1 | 2 <room number> \n");
		return;
	}
	
	if (!isdigit (*buf) || atoi(buf) > 2 || atoi(buf) < 0)
	{
		ch->send_to_char ("Which side do you want to add a room to? (1 or 2)\n");
		return;
	}
	side = atoi(buf);
	
	argument = one_argument (argument, buf);
	if (!isdigit (*buf))
	{
		ch->send_to_char ("What is the room number for the side?\n");
		return;
	}
	room_num = atoi(buf);
	
	
	if (side == 1) 
	{
		tport->room_1 = room_num;
	}
	else if (side == 2) 
	{
		tport->room_2 = room_num;
		
	}
	
	sprintf(buf, "Room number %d is now on side %d.\n", room_num, side);
	ch->send_to_char(buf);
	
	return;
}

	//pset slope 1 | 2 <slope> - sets the slope gradient
void
portal_slope (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	int side;
	float exit_slope;
	ROOM_PORTAL_DATA *tport;
	
	
	tport = vtop(ch->pc->edit_portal);
	if (!tport)
	{
		ch->send_to_char ("Sorry, could not find a portal.\n");
		return;
	}
	
	argument = one_argument (argument, buf);
	if (!isdigit (*buf))
	{
		ch->send_to_char ("Syntax:  pset slope 1 | 2 <slope> (level is 0, vertical is 100) \n");
		return;
	}
	
	if (!isdigit (*buf) || atoi(buf) > 2 || atoi(buf) < 0)
	{
		ch->send_to_char ("Which side do you want to start the slope in? (1 or 2)\n");
		return;
	}
	side = atoi(buf);
	
	argument = one_argument (argument, buf);
	if (!isdigit (*buf))
	{
		ch->send_to_char ("What is the slope for this side?\n");
		return;
	}
	exit_slope = atof(buf);
	
	
	if (side == 1) 
	{
		tport->slope = exit_slope;
	}
	else if (side == 2) 
	{
		tport->slope = (-1 * exit_slope);
		
	}
	
	sprintf(buf, "Ths slope is now a gradient of %f going from side 1 to side 2.\n", exit_slope);
	ch->send_to_char(buf);
	
	return;
}

	//pset flag <portal flag>
void
portal_flag (CHAR_DATA * ch, char *argument)
{
	char input[BUFFER_128B];
	char buf[BUFFER_512B];
	char *bitbuf;
	ROOM_PORTAL_DATA *tport;
	int tflag;
	
	const char *exit_bits[8] = {
		"Closed",
		"Locked",
		"PickProof",
		"Secret",
		"Gate",
		"Door",
		"NoSneak",
		"\n"
	};
	
	argument = one_argument (argument, input);
	
	if (!*input || *input == '?')
	{
		command_interpreter(ch, "tags exit-bits");
		return;
	}
	
	tport = vtop(ch->pc->edit_portal);
	if (!tport)
		return;
	
	if (!isdigit (*input) && *input)
	{
		if (!str_cmp (input, "none") || !str_cmp (input, "clear"))
		{
			tport->port_flags = 0;
		}
		
		else
		{
			tflag = index_lookup (exit_bits, input);
			if (tflag == -1)
			{
				snprintf (buf, BUFFER_512B, "The exit flag '%s' does not exist.\n",
						  input);
				ch->send_to_char (buf);
				return;
			}
			else if ((1 << tflag) == EX_ISDOOR || (1 << tflag) == EX_ISGATE)
			{
				ch->send_to_char("Door and gate flags have been deprecated. You should set the portal type to door or gate instead.\n");
				return;
			}
			else
			{
				
				if (IS_SET (tport->port_flags, 1 << tflag))
					tport->port_flags &= ~(1 << tflag);
				else
					tport->port_flags |= (1 << tflag);
			}
		}
	}
	
	bitbuf = strdup(sprintbit (tport->port_flags, exit_bits));
	snprintf (buf, BUFFER_512B, "The exit is now #2[#0%s#2]#0.\n",
			 bitbuf);
	ch->send_to_char (buf);
}

	//pset material <material>
void
portal_material (CHAR_DATA * ch, char *arg)
{
	char buf[MAX_INPUT_LENGTH];
	ROOM_PORTAL_DATA *tport;
	std::map<int, OBJECT_MATERIAL*>::iterator it_material;		
	OBJECT_MATERIAL* tmaterial;
	std::list<std::string>::iterator materials_it;
	std::string tmp_str;
	bool found_material = false;
	int i;
	
	if (!*arg)
	{
		command_interpreter(ch, "tags materials");
		return;
	}
		
	tport = vtop(ch->pc->edit_portal);
	
	if (!isdigit (*arg) && *arg)
	{
		for (it_material = object_material_map.begin(); it_material != object_material_map.end(); it_material++)
		{
			if (!str_cmp(it_material->second->material_name, arg))
			{
					found_material = true;
					
					if (!tport->material.empty())
					{
						for (materials_it = tport->material.begin();
							 materials_it != tport->material.end();
							 materials_it++)
						{
							std::string tmp_mater;
							tmp_mater = *materials_it;
							
								//if it is already in the list, remove it
							if (!str_cmp(arg, tmp_mater.c_str()))
							{
								tport->material.remove(tmp_mater);
								ch->send_to_char("Material has been removed.\n");
								break;
							}
						}
							//list is not empty, and material is not already listed so add it
						if (materials_it == tport->material.end())
						{
							tport->material.push_front(arg);
							ch->send_to_char("Material has been added (1).\n");
							break;
						}
						
					}
						//it is an empty list, so add it
					else
					{
						tport->material.push_front(arg);
						ch->send_to_char("Material has been added. (2)\n");
					}
				}
			
		}
		
		if (!found_material)
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
	
	sprintf (buf, "The portal is now made of: ");
	if (!tport->material.empty())
	{
		
		for ( materials_it = tport->material.begin(); materials_it != tport->material.end(); materials_it++ )
		{
			tmp_str = *materials_it;
			sprintf(buf + strlen(buf), "%s ", tmp_str.c_str());
			tmp_str.clear();
		}
	}
	sprintf(buf + strlen(buf), "\n");
	ch->send_to_char (buf);
	
	return;
	
}

void
portals_to_dir_options(ROOM_DATA *troom)
{
	int dir;
	int side;
	int iter;
	ROOM_PORTAL_DATA *tport;
	ROOM_EXIT_DATA *texit;
	std::multimap<int,room_exit_data>::iterator mapiter;

		//using room_exit_data
		//reset all dir_options
	for (iter = 0; iter <= LAST_DIR; iter++)
	{
		troom->dir_option[iter] = NULL;
	}
	
		//add portals back to dir_options if needed
	if (troom->portals)
	{
			//loop through all portals in this room
		for (iter = 0; iter <= MAX_PORTALS; iter++)
		{
			tport = vtop(troom->portals[iter]);
			if (!tport)
				continue;
			
			if (tport->room_1 == troom->nVirtual)
			{
				side = 1;
				dir = tport->dir_1;
			}
			else if (tport->room_2 == troom->nVirtual)
			{
				side = 2;
				dir = tport->dir_2;
			}
			else 
				continue; //this room doesn't match this portal - ERROR

				//find first openings, doors and gates in major directions
				//add to the room's data
			if (tport
			 && (tport->type == PORTAL_SPACE
			  || tport->type == PORTAL_DOOR
			  || tport->type == PORTAL_GATE))
			{
					//skip if we already have a default exit in this direction
				if (troom->dir_option[dir])
					continue;
				
				if (side == 1)
				{
					troom->dir_option[dir] = new room_exit_data;
					troom->dir_option[dir]->fdesc = duplicateString(tport->fdesc_1);
					troom->dir_option[dir]->keyword = duplicateString(tport->keywords_1);
					troom->dir_option[dir]->port_flags = tport->port_flags;
					troom->dir_option[dir]->key = tport->key_num_1;
					troom->dir_option[dir]->to_room = tport->room_2;
					troom->dir_option[dir]->portal = tport->ident;
					troom->dir_option[dir]->direction = tport->dir_1;
					troom->dir_option[dir]->slope = tport->slope;
					troom->dir_option[dir]->type = tport->type;
					troom->dir_option[dir]->pick_penalty = tport->pick_key_pen_1;
					troom->dir_option[dir]->quality = tport->quality;
				}
				else if (side == 2)
				{
					troom->dir_option[dir] = new room_exit_data;
					troom->dir_option[dir]->fdesc = duplicateString(tport->fdesc_2);
					troom->dir_option[dir]->keyword = duplicateString(tport->keywords_2);
					troom->dir_option[dir]->port_flags = tport->port_flags;
					troom->dir_option[dir]->key = tport->key_num_2;
					troom->dir_option[dir]->to_room = tport->room_1;
					troom->dir_option[dir]->portal = tport->ident;
					troom->dir_option[dir]->direction = tport->dir_2;
					troom->dir_option[dir]->slope = -tport->slope;
					troom->dir_option[dir]->type = tport->type;
					troom->dir_option[dir]->pick_penalty = tport->pick_key_pen_2;
					troom->dir_option[dir]->quality = tport->quality;

				}
			}
			
			//remove old data and add all new ones to the exitmap
			texit = new room_exit_data;
			
			if (side == 1)
			{
				troom->exitmap.erase(tport->dir_1);
				
				texit->sdesc = duplicateString(tport->sdesc_1);
				texit->ldesc = duplicateString(tport->ldesc_1);
				texit->fdesc = duplicateString(tport->fdesc_1);
				texit->keyword = duplicateString(tport->keywords_1);
				texit->portal = tport->ident;
				texit->direction = tport->dir_1;
				texit->type = tport->type;
				texit->port_flags = tport->port_flags;
				texit->key = tport->key_num_1;
				texit->pick_penalty = tport->pick_key_pen_1;
				texit->to_room = tport->room_2;
				texit->slope = tport->slope;
				texit->quality = tport->quality;
				texit->difficulty = tport->difficulty;
				texit->skill = tport->skill;
				
				troom->exitmap.insert (std::make_pair(tport->dir_1,texit));
				
			}
			else if (side == 2)
			{
				troom->exitmap.erase(tport->dir_2);

				texit->sdesc = duplicateString(tport->sdesc_2);
				texit->ldesc = duplicateString(tport->ldesc_2);
				texit->fdesc = duplicateString(tport->fdesc_2);
				texit->keyword = duplicateString(tport->keywords_2);
				texit->portal = tport->ident;
				texit->direction = tport->dir_2;
				texit->type = tport->type;
				texit->port_flags = tport->port_flags;
				texit->key = tport->key_num_2;
				texit->pick_penalty = tport->pick_key_pen_2;
				texit->to_room = tport->room_1;
					//slope is opposite what it is for side 1
				texit->slope = -tport->slope; 
				texit->quality = tport->quality;
				texit->difficulty = tport->difficulty;
				texit->skill = tport->skill;
				
				troom->exitmap.insert (std::make_pair(tport->dir_2,texit));
			}
		}
	}
	
	save_mysql_room (NULL, troom, BUILD_APPROVED);
	
	return;
}

void
portal_sdesc(CHAR_DATA * ch, char *arg)
{
	ROOM_PORTAL_DATA *tport;
	char side[MAX_INPUT_LENGTH];
	std::ostringstream buf;
	
	arg = one_argument(arg, side);
	
	tport = vtop(ch->pc->edit_portal);
	
	if (!tport)
	{
		ch->send_to_char("You aren't currently editing a portal!\n");
		return;
	}
	
	if (atoi(side) == 1)
		tport->sdesc_1 = duplicateString(arg);
	else if (atoi(side) == 2)
		tport->sdesc_2 = duplicateString(arg);
	else
	{
		// They didn't give a valid number, so we'll just do both. Gotta "side" as it's a word in this case rather than a number.
		if (*arg)
		snprintf(side + strlen(side), MAX_INPUT_LENGTH, " %s", arg);
		tport->sdesc_1 = duplicateString(side);
		tport->sdesc_2 = duplicateString(side);
	}
		
	buf << "(Editing portal " << tport->ident << "):"
		<< "\nSide 1 sdesc: " << tport->sdesc_1
		<< "\nSide 2 sdesc: " << tport->sdesc_2 << "\n";
	ch->send_to_char(buf.str().c_str());
}

void
portal_ldesc(CHAR_DATA * ch, char *arg)
{
	ROOM_PORTAL_DATA *tport;
	char side[MAX_INPUT_LENGTH];
	std::ostringstream buf;
	
	arg = one_argument(arg, side);
	
	tport = vtop(ch->pc->edit_portal);
	
	if (!tport)
	{
		ch->send_to_char("You aren't currently editing a portal!\n");
		return;
	}
	
	if (atoi(side) == 1)
		tport->ldesc_1 = duplicateString(arg);
	else if (atoi(side) == 2)
		tport->ldesc_2 = duplicateString(arg);
	else
	{
		// They didn't give a valid number, so we'll just do both. Gotta include "side" as it's a word in this case rather than a number.
		side[0] = toupper(side[0]);
		snprintf(side + strlen(side), MAX_INPUT_LENGTH, " %s", arg);
		tport->ldesc_1 = duplicateString(side);
		tport->ldesc_2 = duplicateString(side);
	}
	
	buf << "(Editing portal " << tport->ident << "):"
		<< "\nSide 1 ldesc: " << tport->ldesc_1
		<< "\nSide 2 ldesc: " << tport->ldesc_2 << "\n";
	ch->send_to_char(buf.str().c_str());
}

void
portal_keywords(CHAR_DATA * ch, char *arg)
{
	ROOM_PORTAL_DATA *tport;
	char keywrd[MAX_INPUT_LENGTH];
	std::ostringstream buf;
	
	arg = one_argument(arg, keywrd);
	
	if(!arg)
		 arg= strdup("(none)");
	
		 
	tport = vtop(ch->pc->edit_portal);
	
	if (!tport)
	{
		ch->send_to_char("You aren't currently editing a portal!\n");
		return;
	}
	
	if (atoi(keywrd) == 1)
		tport->keywords_1 = strdup(arg);
	else if (atoi(keywrd) == 2)
		tport->keywords_2 = strdup(arg);
	else if (*keywrd)
	{
		// They didn't give a valid number, so we'll just do both. 
		tport->keywords_1 = strdup(keywrd);
		tport->keywords_2 = strdup(keywrd);
	}
	
	buf << "(Editing portal " << tport->ident << "):"
		<< "\nSide 1 keywords: " << tport->keywords_1
		<< "\nSide 2 keywords: " << tport->keywords_2 << "\n";
	ch->send_to_char(buf.str().c_str());
}

	//removes the portal being edited from both rooms
void
room_remove_portal(CHAR_DATA * ch, char *arg)
{
	char buf[MAX_INPUT_LENGTH];
	ROOM_DATA *troom1;
	ROOM_DATA *troom2;
	ROOM_PORTAL_DATA *portal;
	bool removed_1 = false;
	bool removed_2 = false;
		
	arg = one_argument(arg, buf);
	
	if (!*buf)
	{
		ch->send_to_char("If you want to remove this portal from both rooms, use \"pset portal remove\"\n");
		return;
	}
	
	portal = vtop(ch->pc->edit_portal);
	
	if (!portal)
	{
		ch->send_to_char("You are not editing a portal!\n");
		return;
	}
	else 
	{
		troom1 = vtor(portal->room_1);
		troom2 = vtor(portal->room_2);
	}
	
	for (int iter = 0; iter < MAX_PORTALS; iter++)
	{ 
			//remove portal from the first room
		if (troom1->portals[iter] == portal->ident)
		{
			troom1->portals[iter] = -1;
			removed_1 = true;
			portals_to_dir_options(troom1);
		}
		
			//remove portal from the second room
		if (troom2->portals[iter] == portal->ident)
		{
			troom2->portals[iter] = -1;
			removed_2 = true;
			portals_to_dir_options(troom2);
		}
	}
	
	portal->to_place = false;		
	save_mysql_portal (ch, portal, BUILD_APPROVED);
	
	if (removed_1 || removed_2)
	{
		snprintf(buf, MAX_INPUT_LENGTH, "Portal %d has been removed from:\n", portal->ident);
		if (removed_1)
			snprintf(buf + strlen(buf), MAX_INPUT_LENGTH, "room %d\n", troom1->nVirtual);
		if (removed_2)
			snprintf(buf + strlen(buf), MAX_INPUT_LENGTH, "room %d\n", troom2->nVirtual);
		
		ch->send_to_char(buf);
	}
	else 
	{
		snprintf(buf, MAX_INPUT_LENGTH, "There was a problem removing portal %d.\n", portal->ident);
		ch->send_to_char(buf);
		return;
	}
}

	//given the room we are standing in, and a direction,
	//is there a portal in that direction?
//If there is, what room does it lead to?
ROOM_DATA * get_portal_room(CHAR_DATA *ch, int dir)
{
	ROOM_DATA * troom;
	ROOM_PORTAL_DATA * tport;
	
	troom = ch->room;
	
		//loop through all portals in this room
	for (int iter = 0; iter <= MAX_PORTALS; iter++)
	{
		tport = vtop(troom->portals[iter]);
		if (!tport)
			continue;
		
			//find the room we are in, and test for the direction
		if ((tport->room_1 == troom->nVirtual) && (dir == tport->dir_1 ))
		{
			return vtor(tport->room_2);
		}
		else if ((tport->room_2 == troom->nVirtual) && (dir == tport->dir_2 ))
		{
			return vtor(tport->room_1);
		}
		else 
			continue; //this isn't the portal we want
		
	}
	return NULL;
}

	//using the room we are standing in, and a direction,
	//is there a portal in that direction with the specified keyword?
	//If there is, return the portal_ident.
int
keyword_to_portal(CHAR_DATA *ch, int dir, char* keyword)
{
	ROOM_DATA * troom;
	ROOM_PORTAL_DATA * tport;
	int iter;
	int holder = 0;
	
	if (!keyword)
		return 0;
	
	troom = ch->room;
	for (; isspace (*keyword); keyword++);	/* Get rid of whitespaces */
	
		//loop through all portals in this room
	for (iter = 0; iter <= MAX_PORTALS; iter++)
	{
		tport = vtop(troom->portals[iter]);
		if (!tport)
			continue;
		
			//find the room we are in, and test for the direction
		if ((tport->room_1 == troom->nVirtual) && (dir == tport->dir_1))
		{
			if (isname(keyword, tport->keywords_1))
			{
				holder = tport->ident;
			}
		}
		else if ((tport->room_2 == troom->nVirtual) && (dir == tport->dir_2))
		{
			if (isname(keyword, tport->keywords_2))
				{
				holder = tport->ident;
				}
		}
		else 
			continue; //this isn't the portal we want
	}
	return holder;
}

	//what skill is needed to get past a barrier or Crossing
	//pset skil <skill name>
void
portal_skill(CHAR_DATA * ch, char *arg)
{
	char skill[MAX_INPUT_LENGTH];
	char output[MAX_INPUT_LENGTH];
	ROOM_PORTAL_DATA *tport;
	int skill_val;

	tport = vtop(ch->pc->edit_portal);
	
	if (!tport)
	{
		ch->send_to_char("You are not currently editing a portal.\n");
		 return;
	}
		
	arg = one_argument (arg, skill);
	
	if (!*skill || isdigit(*skill))
	{
		ch->send_to_char("The specified 'skill' value must be a skill name.\n");
		return;
	}
	
	if (!str_cmp (skill, "default"))
	{
		ch->send_to_char ("The skill check has been removed and changed to Climb as the default.\n");
		tport->skill = lookup_skill_id("Climb");
		return;
	}
	
	if ((skill_val = lookup_skill_id(skill)) == -1)
	{
		snprintf(output, MAX_INPUT_LENGTH, "I could not find the '%s' skill in our registry.\n", skill);
		ch->send_to_char (output);
		return;
	}

		tport->skill = skill_val;
	
	snprintf (output, MAX_INPUT_LENGTH, "The '%s' skill has been added to this portal.\n", skill);
	ch->send_to_char (output);
	return;
}

	//what happens when a player and his followers try to get past a barrier
int
past_barrier(CHAR_DATA * ch, ROOM_PORTAL_DATA * tport, bool may_sneak)
{
	char buf[MAX_INPUT_LENGTH];
	char *skill_name;
	ROOM_DATA *troom;
	int side;
	bool sneak_check = true;
	
	
	troom = ch->room;
	skill_name = lookup_skill_name(tport->skill);
	side = port_side_room(troom, tport);
	
	
	if (!may_sneak 
		|| ((get_affect (ch, MAGIC_SNEAK))
			&& (!skill_use (ch, "Sneak", tport->difficulty))))
	{
		sneak_check = false;
		if(get_affect (ch, MAGIC_SNEAK))
			ch->send_to_char("You aren't certain if you are seen. ");
		
		remove_affect_type (ch, MAGIC_SNEAK);
		remove_affect_type (ch, MAGIC_HIDDEN);
	}
	
	if (skill_use (ch, skill_name, tport->difficulty) == 1)
	{
		if (side == 1)
		{
			snprintf(buf, MAX_INPUT_LENGTH, "As you reach %s, you decide you will have to %s to get past. Fortunately, you succeed.\n", tport->sdesc_or_default(1), lookup_skill_name(tport->skill));
			ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
			
			if (!get_affect (ch, MAGIC_SNEAK))
			{
				snprintf (buf, MAX_INPUT_LENGTH, "$n succeeds in getting past %s.", tport->sdesc_or_default(1));
				ch->act(buf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
			}
		}
		else
		{
			snprintf(buf, MAX_INPUT_LENGTH, "As you reach %s, you decide you will have to %s to get past. Fortunately, you succeed.\n", tport->sdesc_or_default(2), lookup_skill_name(tport->skill));
			ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
			
			if (!get_affect (ch, MAGIC_SNEAK))
			{
				snprintf (buf, MAX_INPUT_LENGTH, "$n succeeds in getting past %s.\n", tport->sdesc_or_default(2));
				ch->act(buf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
			}
		}
		
			
		return 1;
	}
	
			
			
	return 0;
}

	//what happens when a player tries to get past a Crossing
int
past_crossing(CHAR_DATA * ch, ROOM_PORTAL_DATA * tport, bool may_sneak)
{
	char buf[MAX_INPUT_LENGTH];
	char *skill_name;
	ROOM_DATA * troom;
	int side;
	bool sneak_check = true;
		
	troom = ch->room;	
	side = port_side_room(troom, tport);
	skill_name = lookup_skill_name(tport->skill);
	
	
	if (!may_sneak 
		|| ((get_affect (ch, MAGIC_SNEAK))
			&& (!skill_use (ch, "Sneak", tport->difficulty))))
		
	{
		sneak_check = false;
		if(get_affect (ch, MAGIC_SNEAK))
			ch->send_to_char("You aren't certain if you are seen.");
		remove_affect_type (ch, MAGIC_SNEAK);
		remove_affect_type (ch, MAGIC_HIDDEN);
	}
	
	if (skill_use (ch, skill_name, tport->difficulty) == 1)
	{
		if (side == 1)
		{
			sprintf(buf, "As you try to cross %s, you decide you will have to %s to get past. Keeping your eyes on %s, you succeed.\n", tport->sdesc_or_default(1), lookup_skill_name(tport->skill), vtor(tport->room_2)->name);
			ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
			
			if (!get_affect (ch, MAGIC_SNEAK))
			{
				sprintf (buf, "$n succeeds in getting past %s into %s.\n", tport->sdesc_or_default(1), vtor(tport->room_2)->name);
				ch->act(buf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
			}
			
		}
		else
		{
			sprintf(buf, "As you try to cross %s, you decide you will have to %s to get past. Keeping your eyes on %s, you succeed.\n", tport->sdesc_or_default(2), lookup_skill_name(tport->skill), vtor(tport->room_1)->name);
			ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
			
			if (!get_affect (ch, MAGIC_SNEAK))
			{
				sprintf (buf, "$n succeeds in getting past %s into %s.\n", tport->sdesc_or_default(2), vtor(tport->room_1)->name);
				ch->act(buf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
			}
		}
		
			
		return (1);
		
	}
			
	return 0;
}

	//what happens when a player activtes a transport
void
past_transport(CHAR_DATA * ch, ROOM_PORTAL_DATA * tport, bool may_sneak)
{
	char buf[MAX_INPUT_LENGTH];
	ROOM_DATA * troom;
	int side;
	bool sneak_check = true;
		
	troom = ch->room;	
	side = port_side_room(troom, tport);
	
	
	if (!may_sneak 
		|| ((get_affect (ch, MAGIC_SNEAK))
			&& (!skill_use (ch, "Sneak", tport->difficulty))))
	{
		sneak_check = false;
		if(get_affect (ch, MAGIC_SNEAK))
			ch->send_to_char("You aren't certain if you are seen.");
		
		remove_affect_type (ch, MAGIC_SNEAK);
		remove_affect_type (ch, MAGIC_HIDDEN);
	}
	
		//can't hide if they didn't sneak
		//move function checks for this in barriers and crossings
	if ((!get_affect (ch, MAGIC_SNEAK))
		&& (get_affect (ch, MAGIC_HIDDEN))) 
	{
		remove_affect_type (ch, MAGIC_HIDDEN);
	}
	
	if (side == 1)
	{
		sprintf(buf, "You step through %s leading to %s.", tport->sdesc_or_default(1), vtor(tport->room_2)->name);
		ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
		
		if (!get_affect (ch, MAGIC_SNEAK))
		{
			sprintf (buf, "$n passes through %s into %s.", tport->sdesc_or_default(1), vtor(tport->room_2)->name);
			ch->act(buf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
		}
		
		ch->char_from_room();
		
		ch->char_to_room(vtor(tport->room_2)->nVirtual);
		sprintf (buf, "$n arrives from %s to the %s.", tport->sdesc_or_default(2), dirs[tport->dir_2]);
		
		if (!get_affect (ch, MAGIC_SNEAK))
		{
			ch->act(buf, true,  0, 0, TO_ROOM);
		}
		
		do_look (ch, "", 0);
	}
	else
	{
		sprintf(buf, "You step through %s leading to %s.", tport->sdesc_or_default(2), vtor(tport->room_1)->name);
		ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
		
		if (!get_affect (ch, MAGIC_SNEAK))
		{
			sprintf (buf, "$n passes through %s into %s.", tport->sdesc_or_default(2), vtor(tport->room_1)->name);
			ch->act(buf, false,  0, 0, TO_ROOM | _ACT_FORMAT);
		}
		
		ch->char_from_room();
		
		ch->char_to_room(vtor(tport->room_1)->nVirtual);
		sprintf (buf, "$n arrives from %s to the %s.", tport->sdesc_or_default(1), dirs[tport->dir_1]);
		if (!get_affect (ch, MAGIC_SNEAK))
		{
			ch->act(buf, true,  0, 0, TO_ROOM);
		}
		
		do_look (ch, "", 0);
	}
		//there is no move penalty for using transports
	return;
	
}

	//given a room and a port, returns the side of the port in the specified room
int 
port_side_room (ROOM_DATA * troom, ROOM_PORTAL_DATA * tport)
{
	if ((!troom) || (!tport))
		return (0);
	
	if (tport->room_1 == troom->nVirtual)
	{
		return(1);
	}
	else if (tport->room_2 == troom->nVirtual)
	{
		return(2);
	}
	
	else
	{//the  portal are messed up, and this is an error
		return (0);
	}
	
}

	//tests if the two rooms in a portal are close enough together to be linked
bool
test_location_rooms(CHAR_DATA * ch, ROOM_PORTAL_DATA *portal)
{
		//find the boundaries of each room in the portal
		//test if the bourndary in the proper direction of the first room matches the corresponding boundary of the second room
		//if boundaries match, then return true so they can be linked
		//if the boundaries do not match, only type that will work is TRANSPORT
		//A smaller room can link to larger areas with Barriers, Crossings, windows or transports, as long as the exit side of the smaller room is inside the larger room.
	ROOM_DATA * room_1;
	ROOM_DATA * room_2;
	int dir_1;
	int dir_2;
	int rsize_1;
	int rsize_2;
	int one_match = -1;
	float range_1;
	float range_2;
	bool check;
	
	room_1 = vtor(portal->room_1);
	room_2 = vtor(portal->room_2);
	
	rsize_1 = room_1->room_size;
	rsize_2 = room_2->room_size;
	
	dir_1 = portal->dir_1;
	dir_2 = portal->dir_2;
	
		//get the size of the rooms
	switch (rsize_1)
	{
		case ROOM_SIZE_DETAIL:
			range_1 = DETAIL_SIZE;
			break;
		case ROOM_SIZE_EXPLORE:
			range_1 = EXPLORE_SIZE;
			break;
		case ROOM_SIZE_VALLEY:
			range_1 = VALLEY_SIZE;
			break;	
		case ROOM_SIZE_STORAGE:
			range_1 = STORAGE_SIZE;
			break;	
		default:
			range_1 = DETAIL_SIZE;
			break;
	}
	
	switch (rsize_2)
	{
		case ROOM_SIZE_DETAIL:
			range_2 = DETAIL_SIZE;
			break;
		case ROOM_SIZE_EXPLORE:
			range_2 = EXPLORE_SIZE;
			break;
		case ROOM_SIZE_VALLEY:
			range_2 = VALLEY_SIZE;
			break;
		case ROOM_SIZE_STORAGE:
			range_2 = STORAGE_SIZE;
			break;
		default:
			range_2 = DETAIL_SIZE;
			break;
	}
	
	//test if they have a match with one side
	switch (dir_1) 
	{
		case 0: //north edge
			if ((room_1->x_coord + range_1) == (room_2->x_coord - range_2))
				one_match = 0;
			break;
		case 1: //East edge
			if ((room_1->y_coord + range_1) == (room_2->y_coord - range_2))
				one_match = 1;
			break;
		case 2: //South edge
			if ((room_1->x_coord - range_1) == (room_2->x_coord + range_2))
				one_match = 2;
			break;	
		case 3: //west edge
			if ((room_1->y_coord - range_1) == (room_2->y_coord + range_2))
				one_match = 3;
			break;
		case 4: //upper edge
			if ((room_1->z_coord + range_1) == (room_2->z_coord - range_2))
				one_match = 4;
			break;
		case 5: //floor edge
			if ((room_1->z_coord - range_1) == (room_2->z_coord + range_2))
				one_match = 5;
			break;
		default:
			one_match = -1;
			break;
	}
	
		//they do not share an edge in the direction specified
		//Only allowed connection is Transport
	if (one_match < 0)
	{
		if(portal->type == PORTAL_TRANSPORT)
			return(true);
		else
			return (false);
	}
	
	//we share a side. now check to see if we are inside bounds
	else if (rsize_1 <= rsize_2)
	{
		switch (one_match)
		{
			case 0: //north edge
			case 2: //South edge
				if ((room_1->x_coord < room_2->x_coord + range_2) 
					&& (room_1->x_coord > room_2->x_coord - range_2))
					check = true;
				break;
			
			case 1: //East edge
			case 3: //west edge
				if ((room_1->y_coord < room_2->y_coord + range_2)
					&& (room_1->y_coord > room_2->y_coord - range_2))
					check = true;
				break;
				
			case 4: //upper edge
			case 5: //floor edge
				if ((room_1->z_coord < room_2->z_coord + range_2)
					&& (room_1->z_coord > room_2->z_coord - range_2))
					check = true;
				break;
			
			default:
				check = false;
				break;
		}
		return (check);	
	}
	else if (rsize_1 > rsize_2)
	{
		switch (one_match)
		{
			case 0: //north edge
			case 2: //South edge
				if ((room_2->x_coord < room_1->x_coord + range_1) 
					&& (room_2->x_coord > room_1->x_coord - range_1))
					check = true;
				break;
			
			case 1: //East edge
			case 3: //west edge
				if ((room_2->y_coord < room_1->y_coord + range_1)
					&& (room_2->y_coord > room_1->y_coord - range_1))
					check = true;
				break;
			
			case 4: //upper edge
			case 5: //floor edge
				if ((room_2->z_coord < room_1->z_coord + range_1)
					&& (room_2->z_coord > room_1->z_coord - range_1))
					check = true;
				break;
							
			default:
				check = false;
				break;
		}
		return (check);	
	}
	return(false);
}

void 
portal_save(CHAR_DATA * ch)
{
	char buf[MAX_INPUT_LENGTH];
	ROOM_PORTAL_DATA *tport;
	ROOM_DATA *room_1;
	ROOM_DATA *room_2;
	
	tport = vtop(ch->pc->edit_portal);
	if (tport)
	{
	save_mysql_portal (ch, tport, BUILD_APPROVED);
	sprintf(buf, "Portal %d is being saved!\n", ch->pc->edit_portal);
	ch->pc->edit_portal = 0;
	ch->send_to_char (buf);
		
		room_1 = vtor(tport->room_1);
		room_2 = vtor(tport->room_2);
			//now to refresh the exits for instant gratification
		if (room_1)
			portals_to_dir_options(room_1);
			
		if (room_2)
			portals_to_dir_options(room_2);
	}
	
	else
	{
		sprintf(buf, "There has been a problem saving Portal %d!\n", ch->pc->edit_portal);
		ch->send_to_char(buf);
		return;

	}

	return;	
}

	//IMP level function only
	//displays current room coordiantes, and connecting rooms in primary directions only
	//argument "fix" will adjust the connecting rooms to correct coords, 
	//based on current room and target room sizes
	//TODO: nonfunctional really, since builders can have multiple exits per side and ther is no way to tell how they are realted to each other.
	//Also, does not include STORAGE rooms
void
do_coords(CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_INPUT_LENGTH];
	ROOM_PORTAL_DATA *tport;
	ROOM_DATA *room_1, *room_2;
	float range_1;
	float range_2;
	bool check;
	float xcoord, ycoord, zcoord, xadj, yadj;
	int rsize_1, rsize_2;
	
	int dir = 0;
	ROOM_EXIT_DATA *exit = NULL;
		
	room_1 = vtor(ch->in_room);
	
	sprintf(buf, "Room %d: (%.2f, %.2f, %.2f)\n  Size : %s\n\n",
			room_1->nVirtual,
			room_1->x_coord,
			room_1->y_coord,
			room_1->z_coord,
			room_sizes[room_1->room_size]);
	
	for (dir = 0; dir <= LAST_DIR; dir++)
	{
		
		exit = is_exit(ch, dir);
		
		if (!exit || exit->to_room == NOWHERE)
			continue;
		
		room_2 = vtor(exit->to_room);
		sprintf(buf + strlen(buf), "Dir: %s - Room: %d  Size: %s\n    Before - (%.2f, %.2f, %.2f)\n",
				exit_dirs[dir],
				room_2->nVirtual,
				room_sizes[room_2->room_size],
				room_2->x_coord,
				room_2->y_coord,
				room_2->z_coord);
		
		
			//find out the proper location of the next room
		rsize_1 = room_1->room_size;
		rsize_2 = room_2->room_size;
		
		
			//get the size of the rooms
		switch (rsize_1)
		{
			case ROOM_SIZE_DETAIL:
				range_1 = DETAIL_SIZE;
				break;
			case ROOM_SIZE_EXPLORE:
				range_1 = EXPLORE_SIZE;
				break;
			case ROOM_SIZE_VALLEY:
				range_1 = VALLEY_SIZE;
				break;	
			case ROOM_SIZE_STORAGE:
				range_1 = STORAGE_SIZE;
				break;	
			default:
				range_1 = DETAIL_SIZE;
				break;
		}
		
		switch (rsize_2)
		{
			case ROOM_SIZE_DETAIL:
				range_2 = DETAIL_SIZE;
				break;
			case ROOM_SIZE_EXPLORE:
				range_2 = EXPLORE_SIZE;
				break;
			case ROOM_SIZE_VALLEY:
				range_2 = VALLEY_SIZE;
				break;
			case ROOM_SIZE_STORAGE:
				range_2 = STORAGE_SIZE;
				break;
			default:
				range_2 = DETAIL_SIZE;
				break;
		}

			//do we need to adjust for a DR conencted to larger size?
		if ((range_2 == DETAIL_SIZE) &&
			((range_1 == EXPLORE_SIZE) || (range_1 == VALLEY_SIZE)))
		{
			xadj = 2.75;
			yadj = 2.75;
		}
		else if ((range_1 == DETAIL_SIZE) &&
				 ((range_2 == EXPLORE_SIZE) || (range_2 == VALLEY_SIZE)))
		{
			xadj = -2.75;
			yadj = -2.75;
		}
		
		else
		{
			xadj = 0;
			yadj = 0;
		}

		switch (dir) 
		{
			case 0: //room to the north
				xcoord = room_1->x_coord - xadj;
				ycoord = room_1->y_coord + (range_1/2) + (range_2/2);
				zcoord = room_1->z_coord;
				break;
			case 1: //room to the East
				xcoord = room_1->x_coord + (range_1/2) + (range_2/2);
				ycoord = room_1->y_coord - yadj;
				zcoord = room_1->z_coord;
				break;
			case 2: //room to the South
				xcoord = room_1->x_coord - xadj;
				ycoord = room_1->y_coord - (range_1/2) - (range_2/2);
				zcoord = room_1->z_coord;
				break;	
			case 3: //room to the west
				xcoord = room_1->x_coord - (range_1/2) - (range_2/2);
				ycoord = room_1->y_coord - yadj;
				zcoord = room_1->z_coord;
				break;
				
			case 4: //room to the northeast
				xcoord = room_1->x_coord + (range_1/2) + (range_2/2);
				ycoord = room_1->y_coord + (range_1/2) + (range_2/2);
				zcoord = room_1->z_coord;
				break;
			case 5: //room to the southeast
				xcoord = room_1->x_coord + (range_1/2) + (range_2/2);
				ycoord = room_1->y_coord - (range_1/2) - (range_2/2);
				zcoord = room_1->z_coord;
				break;
			case 6: //room to the Southwest
				xcoord = room_1->x_coord - (range_1/2) - (range_2/2);
				ycoord = room_1->y_coord - (range_1/2) - (range_2/2);
				zcoord = room_1->z_coord;
				break;	
			case 7: //room to the northwest
				xcoord = room_1->x_coord - (range_1/2) - (range_2/2);
				ycoord = room_1->y_coord + (range_1/2) + (range_2/2);
				zcoord = room_1->z_coord;
				break;
			case 8: //room to the upper
				xcoord = room_1->x_coord;
				ycoord = room_1->y_coord;
				zcoord = room_1->z_coord + 15;
				break;
			case 9: //room to the floor
				xcoord = room_1->x_coord;
				ycoord = room_1->y_coord;
				zcoord = room_1->z_coord - 15;
				break;
			default:
				xcoord = 0;
				ycoord = 0;
				zcoord = 0;
				break;
		}
		
				
		if (str_cmp(argument, "fix"))
			sprintf(buf + strlen(buf), "    After - (%.2f, %.2f, %.2f)\n\n",
				xcoord,
				ycoord,
				zcoord);
		else
		{
			room_2->x_coord = xcoord;
			room_2->y_coord = ycoord;
			room_2->z_coord = zcoord;
			sprintf(buf + strlen(buf), "    After - (%.2f, %.2f, %.2f)\n\n",
					xcoord,
					ycoord,
					zcoord);
			
			save_mysql_room(ch, room_2, BUILD_APPROVED);
			
		}

		
	}
	
	ch->send_to_char(buf);
	return;
	
}

	//given a portal number and a direction
	//return the room in the direction specified of the portal
ROOM_DATA* room_from_port_dir(int port_num, int tdir)
{
	ROOM_PORTAL_DATA* tport = NULL;
	ROOM_DATA* troom = NULL;
	
	tport = vtop(port_num);
	
	if (!tport)
		return NULL;
	
	if (tport->dir_1 == tdir)
	{
		troom = vtor(tport->room_2);
		if (troom)
			return(troom);
	}
	else if (tport->dir_2 == tdir)
	{
		troom = vtor(tport->room_1);
		if (troom)
			return(troom);
	}
	
	else
	{//the  portal are messed up, and this is an error
		return NULL;
	}
	
	return NULL;
}

	//pset fail <destination> [<feet to fall>]
	//when they fail a skill check, they are moved to the destination room
	//optionally, they can be given damage, by specifying number of feet fallen
void
portal_fail (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	int dest_room;
	int feet_to_fall;
	ROOM_PORTAL_DATA *tport;
	
	
	tport = vtop(ch->pc->edit_portal);
	if (!tport)
	{
		ch->send_to_char ("Sorry, could not find a portal.\n");
		return;
	}
	
	argument = one_argument (argument, buf);
	if (!isdigit (*buf))
	{
		ch->send_to_char ("Syntax:  pset fail <destination room> <feet to fall> \n");
		return;
	}
	
	dest_room = atoi(buf);
	
	
	argument = one_argument (argument, buf);
	
	if (!*buf)
		feet_to_fall = 0;
	
	else if (!isdigit (*buf))
	{
		ch->send_to_char ("How many feet do they fall?\n");
		return;
	}
	else
		feet_to_fall = atof(buf);
	
	if (vtor(dest_room))
		tport->fail_room = dest_room;
	
	tport->feet_fallen = feet_to_fall;
		
	
	sprintf(buf, "Failing the skill check will now drop player in room %d, where they suffer damage from falling %d feet.\n", dest_room, feet_to_fall);
	
	ch->send_to_char(buf);
	
	return;
}
