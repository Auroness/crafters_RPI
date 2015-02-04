//////////////////////////////////////////////////////////////////////////////
//
/// dwellings.c : auto-genned player dwellings/areas
//
// TODO: do we need temporary housing units created on the fly? What to do with unused rooms?
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

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "server.h"
#include "structs.h"
#include "protos.h"
#include "utils.h"

	//pregame rooms for now
	//possibly expand to rental units later
void
clone_room (ROOM_DATA * source_room, ROOM_DATA * targ_room, bool clone_exits)
{
	
		//	EXTRA_DESCR_DATA *exptr = NULL, *extra = NULL;
	int loop_count = 0;

	if (!source_room || !targ_room)
		return;

	targ_room->room_flags = source_room->room_flags;
	targ_room->terrain_type = source_room->terrain_type;

	targ_room->name = duplicateString (source_room->name);
	targ_room->description = duplicateString (source_room->description);

	targ_room->zone = source_room->zone;

	if (clone_exits)
	{
			//TODO: add code to duplicate portals and exits
	}

	
	for (loop_count = 0; loop_count <= MAX_EX_DESCR; loop_count++)
	{
		if (!source_room->ex_description[loop_count])
			continue;
		
		else
		{
			targ_room->ex_description[loop_count] = new EXTRA_DESCR_DATA;
			targ_room->ex_description[loop_count]->keyword = strdup(source_room->ex_description[loop_count]->keyword);
			targ_room->ex_description[loop_count]->description = strdup(source_room->ex_description[loop_count]->description);
		}
		
	}
	

	if (source_room->extra)
	{
		targ_room->extra = new ROOM_EXTRA_DATA;
		for (loop_count = 0; loop_count <= WR_LAST_DESCRIPTIONS; loop_count++)
		{
			targ_room->extra->weather_desc[loop_count].assign(source_room->extra->weather_desc[loop_count]);
			if (targ_room->extra->weather_desc[loop_count].empty())
				targ_room->extra->weather_desc[loop_count].assign("");
		}
		
		for (loop_count = 0; loop_count <= LAST_DIR; loop_count++)
		{
			if(source_room->extra->alas[loop_count].empty())
			{
				targ_room->extra->alas[loop_count] = strdup("~");
			}
			else 
			{
				
				targ_room->extra->alas[loop_count].assign(source_room->extra->alas[loop_count]);
			}
		}
		
	}
	else
		targ_room->extra = NULL;
}

int
delete_contiguous_rblock (ROOM_DATA * start_room, int from_dir,
						  int outside_vnum)
{
	int i = 0;

	if (!start_room)
		return -1;

	for (i = 0; i <= LAST_DIR; i++)
	{

		if (i == from_dir || !start_room->dir_option[i])
			continue;

		
		if (vtor (start_room->dir_option[i]->to_room))
			delete_contiguous_rblock (vtor (start_room->dir_option[i]->to_room),
			rev_dir[i], outside_vnum);
	}

	// If this is a temporary dwelling type room that's being destroyed, let's make sure
	// anyone logged off in this room is set to the room just outside before we delete.

	// If there's no outside room designated, we'll set them to NOWHERE just to be safe.
	mysql_safe_query ("UPDATE pfiles SET room = %d WHERE room = %d", outside_vnum, start_room->nVirtual);

	room_delete (start_room);

	return 0;
}

int
clone_contiguous_rblock (ROOM_DATA * start_room, int from_dir)
{
	ROOM_DATA *room;
	int i = 0, rnum = 0, dest_room = 0;

	if (!start_room)
		return -1;

	// Determine unused vnum for the new cloned room.

	for (rnum = 100000; rnum < 110000; rnum++) {
		if (!vtor (rnum))
			break;
	}

	// Clone master room to new room, including all exit information.

	room = new_room (rnum);
	clone_room (start_room, room, true);

	// Link up exit to next room in block.

	for (i = 0; i <= LAST_DIR; i++)
	{

		if (i == from_dir || !room->dir_option[i])
			continue;

		if (vtor (start_room->dir_option[i]->to_room))
			dest_room =
			clone_contiguous_rblock (vtor (start_room->dir_option[i]->to_room),
			rev_dir[i]);

		if (dest_room == -1)
			continue;

		room->dir_option[i]->to_room = dest_room;
		vtor (dest_room)->dir_option[rev_dir[i]]->to_room = rnum;

	}

	// Return the VNUM of the new cloned room.

	return rnum;
}

	//generates a temporary dwelling room, but it has no connection to the 'real' world. There is no outside exit
ROOM_DATA *
generate_dwelling_room (OBJ_DATA * dwelling)
{
	ROOM_DATA *room;
	int target = 0;

	
	if (dwelling->o.od.value[0] && (room = vtor (dwelling->o.od.value[0])))
	{
		return room;
	}
	else
	{
		if (!vtor (dwelling->o.od.value[5]))
			return NULL;

		target = clone_contiguous_rblock (vtor (dwelling->o.od.value[5]), -1);
		room = vtor (target);

		if (!room)
			return NULL;

		dwelling->o.od.value[0] = room->nVirtual;
	}

	if (!room)
		return NULL;

	return room;
}

OBJ_DATA *
find_dwelling_obj (int dwelling_room)
{
	ROOM_DATA *room;
	OBJ_DATA *tobj;
	std::map<int, ROOM_DATA*>::iterator room_iterator;

	for (room_iterator = room_map.begin(); room_iterator != room_map.end(); room_iterator++)
	{
		room = room_iterator->second;
		
		for (tobj = room->contents; tobj; tobj = tobj->next_content)
		{
			
			if (tobj->o.od.value[0] == dwelling_room)
				return tobj;
		}
	}

	return NULL;
}


int
save_dwelling_rooms ()
{
	ROOM_DATA *troom;
	std::map<int, ROOM_DATA*>::iterator room_iterator;
	FILE *fr;
	int room_good;
	int n;
	int empty_rooms = 0;
	static int total_empty_rooms = 0;

  system_log("Saving auto-generated temprooms to disk.", false);

	if (!(fr = fopen ("temprooms", "w+")))
		return -1;

	for (room_iterator = room_map.begin(); room_iterator != room_map.end(); room_iterator++)
	{
		troom = room_iterator->second;
		
		if (troom->nVirtual >= 100000 && troom->nVirtual <= 200000)
		{

			room_good = 0;

			for (n = 0; n <= LAST_DIR; n++)
				if (troom->dir_option[n] && troom->dir_option[n]->to_room > 0)
					room_good = 1;

			if (troom->contents || troom->people)
				room_good = 1;

			if (strncmp (troom->description, "No Description Set", 18))
				room_good = 1;

			
				empty_rooms++;
				total_empty_rooms++;
			
		}
	}

	fprintf (fr, "$~\n");
	fclose (fr);

	return 0;
}

	//TODO: do we really want to use this for temp dwelling rooms?
void
load_dwelling_rooms ()
{
	return;
	
	FILE *fl;
	ROOM_DATA *room;
	int virtual_nr = 0, zon = 100, i = 0, flag = 0, tmp, sdir;
	char *temp, chk[50];
	struct secret *r_secret;

	if (!(fl = fopen ("temprooms", "r")))
		return;


	do
	{
		fscanf (fl, " #%d\n", &virtual_nr);
		temp = fread_string (fl);

		if (!temp)
			continue;

		if ((flag = (*temp != '$')))
		{
			room = new_room (virtual_nr);
			room->zone = zon;
			room->name = temp;
			room->description = fread_string (fl);
			fscanf (fl, "%d", &tmp);

			fscanf (fl, " %d ", &tmp);
			room->room_flags = tmp;

			/* The STORAGE bit is set when loading in shop keepers */

			room->room_flags &= ~(STORAGE);

			fscanf (fl, " %d ", &tmp);
			room->terrain_type = tmp;

			fscanf (fl, "%d\n", &tmp);
				//room->deity = tmp; //depreciated

			room->contents = 0;
			room->people = 0;
			room->light = 0;

			for (tmp = 0; tmp <= LAST_DIR; tmp++)
				room->dir_option[tmp] = 0;

			for (tmp = 0; tmp <= MAX_EX_DESCR; tmp++)
				room->ex_description[tmp] = NULL;

			for (tmp = 0; tmp <= LAST_DIR; tmp++)
				room->secrets[tmp] = 0;

			for (;;)
			{
				fscanf (fl, " %s \n", chk);

				if (*chk == 'D')	/* direction field */
					setup_dir (fl, room, atoi (chk + 1), 0);

				else if (*chk == 'H')	/* Secret (hidden) */
					setup_dir (fl, room, atoi (chk + 1), 1);

				else if (*chk == 'T')	/* Trapped door */
					setup_dir (fl, room, atoi (chk + 1), 2);

				else if (*chk == 'B')	/* Trapped hidden door */
					setup_dir (fl, room, atoi (chk + 1), 3);

				else if (*chk == 'Q')
				{		/* Secret search desc */
					r_secret = new secret;
					sdir = atoi (chk + 1);
					fscanf (fl, "%d\n", &tmp);
					r_secret->diff = tmp;
					r_secret->stext = fread_string (fl);
					room->secrets[sdir] = r_secret;
				}

				else if (*chk == 'E')	/* extra description field */
				{
					
					for (tmp = 0; tmp < MAX_EX_DESCR; tmp++)
					{
						if (!room->ex_description[tmp])
						{
							room->ex_description[tmp] = new EXTRA_DESCR_DATA;
							room->ex_description[tmp]->keyword = fread_string (fl);
							room->ex_description[tmp]->description = fread_string (fl);
						}
					}
					
				}

				

				else if (*chk == 'A')
				{		/* Additional descriptions */

					room->extra = new ROOM_EXTRA_DATA;

					for (i = 0; i <= WR_LAST_DESCRIPTIONS; i++)
					{
						room->extra->weather_desc[i] = fread_string (fl);
						if (room->extra->weather_desc[i].empty())
							room->extra->weather_desc[i].assign("");
					}

					for (i = 0; i <= LAST_DIR; i++)
					{
						room->extra->alas[i] = fread_string (fl);
						if (room->extra->alas[i].empty())
							room->extra->alas[i].assign("");
					}
				}

				else if (*chk == 'S')	/* end of current room */
					break;
			} //for loop
		} // if flag
		else
		{
			//the dynamic memory that temp points to is still used by room->name when the if is true,
			//but is discarded without deletion otherwise. Now we delete it. Free_mem handles the null checking and setting, etc.
			free_mem(temp); 
		}

	}
	while (flag);

	fclose (fl);
}
