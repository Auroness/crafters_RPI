//////////////////////////////////////////////////////////////////////////////
//
/// save.c : World Save Module
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

#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

#ifndef MACOSX
	//#include <malloc.h>
#endif

#include <stdlib.h>
#include <list>

#include "structs.h"
#include "utils.h"
#include "utility.h"
#include "protos.h"
#include "decl.h"

#define POSSESS_UNDEF   0
#define	POSSESS_CARRY	1
#define POSSESS_WEAR	2
#define POSSESS_ROOM	3

extern std::map<std::string, SKILL_DATA*> skill_data_map;


static int last_wc = POSSESS_UNDEF;
static int last_eqloc;
static int last_eqlev;
static int file_eof_found = 0;
static int save_stop_obj_processing = 0;

int load_char_objects;

extern int fgetc (FILE * fp);
extern int fread_number (FILE * fp);
extern char *fread_word (FILE * fp);

void write_char_data (CHAR_DATA * ch, FILE * fp);
int write_obj_data (OBJ_DATA * obj, char *wc, int pos, int objstack, FILE * fp);
void read_obj_list (int cur_wc,
					int cur_eqloc,
					int cur_eqlev,
					CHAR_DATA * ch,
					OBJ_DATA * holder, FILE * fp, int room_num);
OBJ_DATA *fread_obj (FILE * fp);

char *
unspace (char *s)
{
	char *result;
	char *orig;

	if (!s)
	{
		((int *) -1)[0] = 1;
	}

	if (*s != ' ')
		return s;

	orig = s;

	result = new char[strlen(s) + 1];

	while (s && *s == ' ')
		s++;

	strncpy(result, s, strlen(s) + 1);

	free_mem (orig);

	return result;
}

	//returns 0 if there is a problem
	//returns 1 if successful
int
fwrite_a_obj (OBJ_DATA * obj, FILE * fp)
{
	int affect_count = 0;
	int modifiers = 1;
	AFFECTED_TYPE *af = NULL;
	OBJECT_DAMAGE *damage;
	OBJ_DATA *proto = NULL;
	std::vector<int>::iterator var_it;
	int tvar;
	std::vector<object_damage *>::iterator odam_iterator;
	
	proto = vtoo (obj->nVirtual);

	if (!proto)
		return(0);

	for (af = obj->xaffected; af; af = af->next)
	{

		if (af->a.spell.location)
		{
			affect_count++;
			continue;
		}
	}

	if (!obj->var_val.empty())
		modifiers += obj->var_val.size();
		
	if (obj->clan_data)
		modifiers += 1;

	if (obj->omote_str)
		modifiers++;

	if (obj->loaded)
		modifiers += 1;

	if (!obj->damage.empty())
		modifiers += obj->damage.size();


	if (obj->size)
		modifiers++;

	if (obj->count > 1)
		modifiers++;

	if (obj->obj_flags.extra_flags)
		modifiers++;

	if (obj->obj_flags.set_cost)
		modifiers++;

	if (obj->obj_flags.type_flag == ITEM_DRINKCON)
		modifiers++;
	
	if (obj->obj_flags.type_flag == ITEM_DRYCON)
		modifiers++;

	if (obj->obj_timer)		/* obj_flags.timer */
		modifiers++;

	if (obj->clock && (obj->morphto >= 0) && obj->morphTime)
		modifiers += 3;

	if (obj->book_title && obj->title_skill)
		modifiers += 4;

	if (obj->item_wear)
		modifiers++;

	if ((obj->obj_flags.type_flag != ITEM_DRINKCON)
		&& (obj->obj_flags.type_flag != ITEM_DRYCON))
	{

		if (proto->name && obj->name && *obj->name)
			if (strcmp (obj->name, proto->name))
				modifiers++;

		if (proto->short_description && obj->short_description
			&& *obj->short_description)
			if (strcmp (obj->short_description, proto->short_description))
				modifiers++;

		if (proto->description && obj->description && *obj->description)
			if (strcmp (obj->description, proto->description))
				modifiers++;

		if (proto->full_description && obj->full_description
			&& *obj->full_description)
			if (strcmp (obj->full_description, proto->full_description))
				modifiers++;

	}

	/* if there is no proto type key but there is an object key, save it */
	bool saveMkey = false;
	if (!(proto->desc_keys))
	{
		if (!(obj->desc_keys))
		{
			saveMkey=false; // no proto, no current setting
		}
		else
		{
			saveMkey = strlen(obj->desc_keys) > 1; // no proto, verify length on current setting
		}
	}
	else
	{
		if (!(obj->desc_keys))
		{
			saveMkey=false; // currently no mkey despite on on proto. Let it revert to proto.
		}
		else // both valid so strcmp possible. Save if different.
		{
			// if strcmp is not zero, there is a non-zero difference between them and
			// thus the object's version should be saved
			saveMkey = ( strcmp(obj->desc_keys, proto->desc_keys) != 0 );
		}

	}
	if (saveMkey)
		modifiers++;

	if (obj->coldload_id)
		modifiers++;
	
	if (obj->current_damage > 0)
		modifiers++;
	
		
	fprintf (fp, "Id       %d %d %d\n", obj->nVirtual, affect_count, modifiers);

	if (obj->coldload_id)
		fprintf (fp, "coldload       %d\n", obj->coldload_id);

	if ((obj->obj_flags.type_flag != ITEM_DRINKCON)
		&& (obj->obj_flags.type_flag != ITEM_DRYCON))
	{

		if (proto->name && obj->name && *obj->name)
			if (strcmp (obj->name, proto->name))
				fprintf (fp, "name %s~\n", obj->name);

		if (proto->short_description && obj->short_description
			&& *obj->short_description)
			if (strcmp (obj->short_description, proto->short_description))
				fprintf (fp, "short %s~\n", obj->short_description);

		if (proto->description && obj->description && *obj->description)
			if (strcmp (obj->description, proto->description))
				fprintf (fp, "long %s~\n", obj->description);

		if (proto->full_description && obj->full_description
			&& *obj->full_description)
			if (strcmp (obj->full_description, proto->full_description))
				fprintf (fp, "full %s~\n", obj->full_description);

		if (saveMkey) // reuse value computed above
			fprintf (fp, "desc_keys %s~\n", obj->desc_keys);
	}


	if (obj->book_title && obj->title_skill)
	{
		fprintf (fp, "book_title     %s~\n", obj->book_title);
		fprintf (fp, "title_skill    %d\n", obj->title_skill);
		fprintf (fp, "title_language %d\n", obj->title_language);
		fprintf (fp, "title_script   %d\n", obj->title_script);
	}

	if (obj->loaded)
		fprintf (fp, "loaded %d\n", obj->loaded->nVirtual);

	if (obj->obj_flags.extra_flags)
		fprintf (fp, "extraflags %d\n", obj->obj_flags.extra_flags);

	if (obj->item_wear)
		fprintf (fp, "itemwear %d\n", obj->item_wear);

	if (obj->obj_flags.set_cost)
		fprintf (fp, "set_cost %d\n", obj->obj_flags.set_cost);

	fprintf (fp, "values %d %d %d %d %d %d\n", obj->o.od.value[0],
		obj->o.od.value[1], obj->o.od.value[2], obj->o.od.value[3],
		obj->o.od.value[4], obj->o.od.value[5]);

	if ((obj->obj_flags.type_flag == ITEM_DRINKCON)
		|| (obj->obj_flags.type_flag == ITEM_DRYCON))
		fprintf (fp, "weight   %d\n", obj->obj_flags.weight);

	if (obj->size)
		fprintf (fp, "size     %d\n", obj->size);

	if (obj->count > 1)
		fprintf (fp, "count    %d\n", obj->count);

	if (obj->obj_timer)
		fprintf (fp, "timer    %d\n", obj->obj_timer);

	if (obj->clock && (obj->morphto >= 0) && obj->morphTime)
	{
		fprintf (fp, "clock	%d\n", obj->clock);
		fprintf (fp, "morphto	%d\n", obj->morphto);
		fprintf (fp, "morphTime	%d\n", obj->morphTime);
	}

	if (obj->omote_str)
	{
		fprintf (fp, "OmoteStr   %s~\n", obj->omote_str);
	}

	
	if (!obj->damage.empty())
	{
		for (odam_iterator = obj->damage.begin(); odam_iterator != obj->damage.end(); odam_iterator++)
		{
			damage = *odam_iterator;
			if (!damage)
				break;
			
			object_damage__write_to_file (damage, fp);
		}
	}

	if (obj->clan_data)
	{
		fprintf (fp, "Clan\n%s~\n%s~\n",
			obj->clan_data->name,
			obj->clan_data->rank);
	}

	if (!obj->var_val.empty())
	{
		
		for (var_it = obj->var_val.begin(); var_it != obj->var_val.end(); var_it++)
		{
			tvar = *var_it;
			fprintf (fp, "Variant    %d\n", tvar);
		}
	}
	
	for (af = obj->xaffected; af; af = af->next)
	{
		if (af->a.spell.location)
		{
			fprintf (fp, "Afflocmod %d %d %d %d %ld %d %d\n",
				af->a.spell.location,
				af->a.spell.modifier,
				af->a.spell.duration,
				af->a.spell.bitvector,
				af->a.spell.t, af->a.spell.sn, af->type);
			continue;
		}
	}
	return (1);
}

	//returns 0 if there is a problem
	//returns 1 if successful
int
write_obj_data (OBJ_DATA * obj, char *wc, int pos, int objstack, FILE * fp)
{
	char output_buf[MAX_STRING_LENGTH]= { '\0' };
	int saved_check = 0;
	
	if (!obj)
		return(0);
	
	if (!vtoo (obj->nVirtual))
		return(0);
	
	if (!fp)
	{
		sprintf (output_buf, "ERROR No file to write obj data. (%d)", obj->nVirtual);
		system_log (output_buf, true);
		return(0);
	}
	
	fprintf (fp, "%s %d %d\n", wc, pos, objstack);
	
		// Save object 
	saved_check = fwrite_a_obj (obj, fp);
	
	if (!saved_check)
	{
		sprintf (output_buf, "ERROR object (%d) not written.", obj->nVirtual);
		system_log (output_buf, true);
		return(0);
	}
	
		// Save everything this object contains 
	if (obj->contains)
		saved_check = write_obj_data (obj->contains, wc, pos, objstack + 1, fp);
	
	if (!saved_check)
	{
		sprintf (output_buf, "ERROR object contains (%d) not written.", obj->nVirtual);
		system_log (output_buf, true);
		return(0);	
	}
	
	
	
		// Save the next object in the list - recursive
	if (obj->location == -1 
		&& obj->next_content 
		&& obj != obj->next_content)
		saved_check = write_obj_data (obj->next_content, wc, pos, objstack, fp);
	
	if (!saved_check)
	{
		sprintf (output_buf, "ERROR next_contents not written.");
		system_log (output_buf, true);
	}
	
		//if this is the last obj in the contents, then end
	if (obj->next_content && obj->next_content == obj)
		obj->next_content = NULL;
	
	
	return(1);
	
}

	//reads in the specific saved object for loading to a players inventory 
OBJ_DATA *
fread_obj (FILE * fp)
{
	int obj_vnum;
	int affect_count;
	int modifiers = 0;
	int i, page = 0;
	int old_money = 0;
	long nFilePosition = 0;
	char *p;
	OBJ_DATA *obj;
	OBJ_DATA *tobj;
	WRITING_DATA *writing = NULL;
	AFFECTED_TYPE *af;
	AFFECTED_TYPE *taf;
	OBJECT_DAMAGE *damage = NULL;
	int tvar = NULL;
	
	if (strcmp (p = fread_word (fp), "Id"))
	{
		abort ();
	}

	obj_vnum = fread_number (fp);
	affect_count = fread_number (fp);

	obj = load_object_full (obj_vnum, false, 1);
	
	if (!obj)
	{
		obj = load_object (666);
		
		if (!obj)
		{
			return NULL;
		}
	}

	//we will read the values later, instead of random value from load function
	obj->var_val.clear();
		
	
	
	while ((af = obj->xaffected))
	{
		obj->xaffected = af->next;
		free_mem (af);
	}
	

	fscanf (fp, "%d", &modifiers);

	for (; modifiers; modifiers--)
	{

		nFilePosition = ftell (fp);
		p = fread_word (fp);

		if (!strcmp (p, "name"))
		{
			fgetc (fp);
			obj->name = fread_string (fp);
			continue;
		}
		else if (!strcmp (p, "short"))
		{
			fgetc (fp);
			obj->short_description = fread_string (fp);
			continue;
		}
		else if (!strcmp (p, "loaded"))
		{
			obj->loaded = vtoo (fread_number (fp));
			continue;
		}
		else if (!strcmp (p, "coldload"))
		{
			obj->coldload_id = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "extraflags"))
		{
			obj->obj_flags.extra_flags = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "itemwear"))
		{
			obj->item_wear = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "set_cost"))
		{
			obj->obj_flags.set_cost = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "long"))
		{
			fgetc (fp);
			obj->description = fread_string (fp);
			continue;
		}
		else if (!strcmp (p, "full"))
		{
			fgetc (fp);
			obj->full_description = fread_string (fp);
			continue;
		}
		else if (!strcmp (p, "values"))
		{
			obj->o.od.value[0] = fread_number (fp);
			obj->o.od.value[1] = fread_number (fp);
			obj->o.od.value[2] = fread_number (fp);
			obj->o.od.value[3] = fread_number (fp);
			obj->o.od.value[4] = fread_number (fp);
			obj->o.od.value[5] = fread_number (fp);
			if (obj->obj_flags.type_flag == ITEM_BOOK
				|| obj->obj_flags.type_flag == ITEM_PARCHMENT)
				writing = obj->writing;
			continue;
		}
		else if (!strcmp (p, "weight"))
		{
			obj->obj_flags.weight = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "size"))
		{
			obj->size = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "count"))
		{
			obj->count = fread_number (fp);
			continue;
		}
		else if (!str_cmp (p, "timer"))
		{
			obj->obj_timer = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "clock"))
		{
			obj->clock = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "morphTime"))
		{
			obj->morphTime = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "morphto"))
		{
			obj->morphto = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "book_title"))
		{
			obj->book_title = unspace (fread_string (fp));
			continue;
		}
		else if (!str_cmp (p, "title_skill"))
		{
			obj->title_skill = fread_number (fp);
			continue;
		}
		else if (!str_cmp (p, "title_script"))
		{
			obj->title_script = fread_number (fp);
			continue;
		}
		else if (!str_cmp (p, "title_language"))
		{
			obj->title_language = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "page:"))
		{
			if (obj->obj_flags.type_flag == ITEM_BOOK)
				obj->o.od.value[1] = 0;
			else
				obj->o.od.value[0] = 0;
			writing = obj->writing;
			page = fread_number (fp);
			for (i = 1; i; i++)
			{
				if (i == page)
					break;
				else
					writing = writing->next_page;
			}
			continue;
		}
		else if (!strcmp (p, "message:"))
		{
			if (obj->obj_flags.type_flag == ITEM_BOOK)
				obj->o.od.value[1] = 0;
			else
				obj->o.od.value[0] = 0;
			fgetc (fp);
			writing->message = fread_string (fp);
			continue;
		}
		else if (!strcmp (p, "author:"))
		{
			if (obj->obj_flags.type_flag == ITEM_BOOK)
				obj->o.od.value[1] = 0;
			else
				obj->o.od.value[0] = 0;
			fgetc (fp);
			writing->author = fread_string (fp);
			continue;
		}
		else if (!strcmp (p, "date:"))
		{
			if (obj->obj_flags.type_flag == ITEM_BOOK)
				obj->o.od.value[1] = 0;
			else
				obj->o.od.value[0] = 0;
			fgetc (fp);
			writing->date = fread_string (fp);
			continue;
		}
		else if (!strcmp (p, "language:"))
		{
			writing->language = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "script:"))
		{
			writing->script = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "ink:"))
		{
			if (obj->obj_flags.type_flag == ITEM_BOOK)
				obj->o.od.value[1] = 0;
			else
				obj->o.od.value[0] = 0;
			fgetc (fp);
			writing->ink = fread_string (fp);
			continue;
		}
		else if (!strcmp (p, "skill:"))
		{
			writing->skill = fread_number (fp);
			continue;
		}
		else if (!strcmp (p, "desc_keys"))
		{
			fgetc (fp);
			obj->desc_keys = fread_string (fp);
			continue;
		}
		else if (!strcmp (p, "OmoteStr"))
		{
			obj->omote_str = unspace (fread_string (fp));
			continue;
		}
		
		else if (!strcmp (p, "Clan"))
		{
			obj->clan_data = new OBJ_CLAN_DATA;
			obj->clan_data->name = duplicateString (fread_string (fp));
			obj->clan_data->rank = duplicateString (fread_string (fp));
			continue;
		}
		
		else if (!strcmp (p, "Damage"))
		{

			if ((damage = object_damage__read_from_file (fp)))
			{
				std::vector<OBJECT_DAMAGE *>::iterator it_dam;
				it_dam = obj->damage.begin();
				obj->damage.insert(it_dam, damage);
				obj->current_damage += damage->impact;
			}
			continue;

		}
		
		else if (!strcmp (p, "Variant"))
		{
			tvar = fread_number (fp);
			obj->var_val.push_back(tvar);
			continue;
		}
		
		else if (!strcmp (p, "DONE"))
		{
			fseek (fp, nFilePosition, SEEK_SET);
			break;
		}
		else if (!strcmp (p, "Id"))
		{
			fseek (fp, nFilePosition, SEEK_SET);
			break;
		}

	}//for (; modifiers; modifiers--)

	if (old_money)
	{
		obj->count = obj->o.od.value[0];
		obj->o.od.value[0] = 0;
	}

	

	/* read_object probably created associated some affects with the
	new object.  Lets erase all those and put in saved affects */

	for (i = 0; obj && i < affect_count; i++)
	{

		if (strcmp (fread_word (fp), "Afflocmod"))
		{
			return (NULL);	/* Forfiet remainder of objects */
		}

		af = new AFFECTED_TYPE;

		af->next = NULL;

		fscanf (fp, "%d %d %d %d %ld %d %d\n",
			&af->a.spell.location,
			&af->a.spell.modifier,
			&af->a.spell.duration,
			&af->a.spell.bitvector,
			&af->a.spell.t, &af->a.spell.sn, &af->type);

		
			//Otherwise, load them into the object
			if (!obj->xaffected)
				obj->xaffected = af;
			else
			{
				taf = obj->xaffected;
				while (taf->next)
					taf = taf->next;
				taf->next = af;
			}
		
	}

		// Fluid check
	if (obj->obj_flags.type_flag == ITEM_DRINKCON ||
		obj->obj_flags.type_flag == ITEM_LIGHT ||
		obj->obj_flags.type_flag == ITEM_FOUNTAIN)
	{
		tobj = vtoo (obj->o.drinkcon.liquid);
		if (tobj && (tobj->obj_flags.type_flag == ITEM_FLUID))
			obj->o.drinkcon.liquid = tobj->nVirtual;
	}
	
	if (obj->obj_flags.type_flag == ITEM_DRYCON)
	{
		tobj = vtoo (obj->o.drycon.contents);
		if (tobj && 
			(tobj->obj_flags.type_flag != ITEM_FLUID))
			obj->o.drycon.contents = tobj->nVirtual;
	}


	/* Record what object previously was */

	if (obj->nVirtual == 666 && !obj->o.od.value[0])
		obj->o.od.value[0] = obj_vnum;
	
	return obj;
}

int
fread_look_obj_header (int *wc, int *eqloc, int *eqlev, FILE * fp)
{
	char *ptr;

	if (last_wc != POSSESS_UNDEF)
	{
		*wc = last_wc;
		*eqloc = last_eqloc;
		*eqlev = last_eqlev;
		return (1);
	}

	if (file_eof_found || save_stop_obj_processing)
		return (0);

	ptr = fread_word (fp);
	
	if (!ptr)			//probaly an error in the text file
		return(0);		

	if (*ptr == '\0')		// Probably the end of file mark 
		return (0);

	else if (!strcmp (ptr, "CARRY"))
		*wc = POSSESS_CARRY;

	else if (!strcmp (ptr, "WEAR"))
		*wc = POSSESS_WEAR;

	else if (!strcmp (ptr, "ROOM"))
		*wc = POSSESS_ROOM;

	else if (!strcmp (ptr, "DONE"))
	{
		save_stop_obj_processing = 1;
		return (0);
	}

	else if (!strcmp (ptr, "END"))
	{
		file_eof_found = 1;
		return (0);
	}

	else
		return (0);

	*eqloc = fread_number (fp);
	*eqlev = fread_number (fp);

	last_wc = *wc;
	last_eqloc = *eqloc;
	last_eqlev = *eqlev;

	return (1);
}

void
read_obj_list (int cur_wc, int cur_eqloc, int cur_eqlev, CHAR_DATA * ch,
			   OBJ_DATA * holder, FILE * fp, int room_num)
{
	int eqloc = 0;
	int eqlev = 0;
	int wc = 0;
	OBJ_DATA *obj = NULL;

	while (1)
	{

		if (!fread_look_obj_header (&wc, &eqloc, &eqlev, fp))
			return;

		if (cur_eqlev > eqlev)
			return;

		if ((cur_wc != wc || cur_eqloc != eqloc) && eqloc != -1
			&& eqloc != WEAR_PRIM && eqloc != WEAR_SEC && eqloc != WEAR_BOTH)
		{
			return;
		}

		if ((cur_eqlev < eqlev))
			read_obj_list (wc, eqloc, eqlev, ch, obj, fp, room_num);
		else
		{
			last_wc = POSSESS_UNDEF;
			
			obj = fread_obj (fp);
			
			if (!obj)
				return;
			
			if (holder)
			{
				obj_to_obj (obj, holder);
				
			}
			else if (wc == POSSESS_ROOM)
			{
				if (!vtor (room_num))
					printf ("Room %d does not exist for object %d.\n",
							room_num, obj->nVirtual);
				else 
					obj_to_room (obj, room_num);
				
			}
			else if (cur_eqloc == -1)
			{
				obj->in_room = NOWHERE;
				obj_to_char (obj, ch);
				if (eqloc == WEAR_PRIM
					|| eqloc == WEAR_SEC
					|| eqloc == WEAR_BOTH)
					equip_char (ch, obj, eqloc);
			}
			else
			{
				obj->carried_by = ch;
				obj->in_room = NOWHERE;
				equip_char (ch, obj, eqloc); 
			}			
		}			/* cur_eqlev < eqlev */
	}				/* while (1) */
}

OBJ_DATA *
get_equipped (CHAR_DATA * ch, int location)
{
	OBJ_DATA *obj;

	for (obj = ch->equip; obj; obj = obj->next_content)
		if (obj->location == location)
			return obj;

	return NULL;
}

void
write_obj_suppliment (CHAR_DATA * ch, FILE * fp)
{
	int i;
	int save_check = 1;
	char output_buf[MAX_STRING_LENGTH]= { '\0' };
	
	if (ch->right_hand)
	{
		save_check = write_obj_data (ch->right_hand, "CARRY", ch->right_hand->location, 0,
			fp);
	}

	if (!save_check)
	{
		sprintf (output_buf, "ERROR right hand object not saved for %s", ch->name);
		system_log (output_buf, true);
	}
	
	if (ch->left_hand)
	{
		save_check = write_obj_data (ch->left_hand, "CARRY", ch->left_hand->location, 0, fp);
	}
	
	if (!save_check)
	{
		sprintf (output_buf, "ERROR left hand object not saved for %s", ch->name);
		system_log (output_buf, true);
	}
	
		//position 0 is WEAR_LIGHT which doesn't exist for anyone, so we skip it
	for (i = 1; i < MAX_WEAR; i++)
	{
			//reset to test each position in this loop
		save_check = 1;
		if (i == WEAR_PRIM || i == WEAR_BOTH || i == WEAR_SEC)
			continue;
		if (get_equipped (ch, i))
			save_check = write_obj_data (get_equipped (ch, i), "WEAR", i, 0, fp);
		
		if (!save_check)
		{
			sprintf (output_buf, "ERROR worn item not saved for %s in position %d", ch->name, i);
			system_log (output_buf, true);
		}
	}

	fprintf (fp, "END\n");
}

void
read_obj_suppliment (CHAR_DATA * ch, FILE * fp)
{
	int i;

	file_eof_found = 0;
	save_stop_obj_processing = 0;
	last_wc = POSSESS_UNDEF;
	load_char_objects = 1;	/* Global variable */

	ch->right_hand = NULL;
	ch->left_hand = NULL;
	ch->equip = NULL;

	read_obj_list (POSSESS_CARRY, -1, 0, ch, NULL, fp, 0);

	for (i = 0; i < MAX_WEAR; i++)
	{
		if (i == WEAR_PRIM || i == WEAR_SEC || i == WEAR_BOTH)
			continue;
		read_obj_list (POSSESS_WEAR, i, 0, ch, NULL, fp, 0);
	}

	load_char_objects = 0;
}

#define		TYPE_STRING			1
#define		TYPE_INT			2
#define		TYPE_SKILL			3
#define		TYPE_PVARIC_SPELL		4
#define		TYPE_END			5
#define		TYPE_AFFECT			6
#define		TYPE_SHORTINT			7
#define		TYPE_DREAM			8
#define		TYPE_OBSOLETE			9
#define		TYPE_LONG			10
#define		TYPE_SUBCRAFT			11
#define		TYPE_ALIAS			12
#define		TYPE_STORED_PC			14
#define		TYPE_SPELL			16
#define		TYPE_ENCHANTMENT		17
#define		TYPE_VOICE			18
#define		TYPE_HOODED			19
#define		TYPE_NANNY			21
#define		TYPE_APPCOST			22
#define		TYPE_ROLE			23
#define		TYPE_ROLE_SUMMARY		24
#define		TYPE_ROLE_BODY			25
#define		TYPE_ROLE_DATE			26
#define		TYPE_ROLE_POSTER		27
#define		TYPE_ROLE_COST			28
#define		TYPE_MAIL_MESSAGE		29
#define		TYPE_IP				30
#define		TYPE_AFFECTEDBY			31
#define		TYPE_NEWSLETTER			32
#define		TYPE_OWNER			33
#define		TYPE_HAS_INV			35
#define		TYPE_SPAWN			36
#define		TYPE_DONE			40

struct key_data
{
	char key[AVG_STRING_LENGTH];
	int key_type;
	void *ptr;
};


int
save_char (CHAR_DATA * ch, int save_objs)
{
	if (IS_NPC (ch) || IS_SET (ch->flags, FLAG_GUEST) || !ch->name
		|| !*ch->name)
	{
		return 1;
	}

	ch->save_char_mysql();

	if (save_objs)
		save_char_objs (ch, ch->name);

	if (ch->room)
		save_attached_mobiles (ch, 0);

	return 0;
}

void
load_online_stats ()
{
	FILE *fp;

	if (!(fp = fopen ("online_stats", "r")))
	{
		system_log ("Error opening online_stats!", true);
		return;
	}
	fscanf (fp, "%d\n", &count_max_online);
	char *tmpdate = fread_string (fp);
	strcpy (max_online_date, tmpdate);
	free_mem (tmpdate); // char*
	fclose (fp);
}

void
save_player_rooms ()
{
	FILE *fp;
	ROOM_DATA *room;
	std::map<int, ROOM_DATA*>::iterator room_iterator;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	for (room_iterator = room_map.begin(); room_iterator != room_map.end(); room_iterator++)
	{
		room = room_iterator->second;
		if (room->psave_loaded && room->contents)
		{
			sprintf (buf, "save/rooms/%d", room->nVirtual);
			fp = fopen (buf, "w");
			if (!fp)
				continue;
			write_obj_data (room->contents, "ROOM", 0, 0, fp);
			fprintf (fp, "DONE\n");
			fclose (fp);
		}
		else if (room->psave_loaded && !room->contents)
		{
			sprintf (buf, "save/rooms/%d", room->nVirtual);
			unlink (buf);
		}
	}
}

void
do_saverooms (CHAR_DATA * ch, char *argument, int cmd)
{
	save_player_rooms ();
	ch->send_to_char ("Ok.\n");
}


void
load_save_room (ROOM_DATA * room)
{
	FILE *rp;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (!room)
		return;

	room->psave_loaded = 1;

	sprintf (buf, "save/rooms/%d", room->nVirtual);

	if (!(rp = fopen (buf, "r")))
	{
		return;
	}

	file_eof_found = 0;
	save_stop_obj_processing = 0;
	last_wc = POSSESS_UNDEF;

	read_obj_list (POSSESS_ROOM, 0, 0, NULL, NULL, rp, room->nVirtual);

	room_light(room);
	fclose (rp);
}

	//loads the mobiles attached to a player by vnum when he logged off 
CHAR_DATA *
load_saved_mobiles (CHAR_DATA * ch, char *name)
{
	int nVirtual;
	int coldload;
	CHAR_DATA *mob;
	CHAR_DATA *last_mob;
	CHAR_DATA *return_mob = NULL;
	FILE *fp;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char hookup[MAX_STRING_LENGTH]= { '\0' };

	if (!(fp = fopen (name, "r")))
	{
		return NULL;
	}

	last_mob = ch;

	while (fgets (buf, 256, fp))
	{

		if (*buf == ' ' || *buf == '\n')
			fgets (buf, 255, fp);

		if (sscanf (buf, "%d %s %d", &nVirtual, hookup, &coldload) != 3)
		{
			fclose (fp);
			return return_mob;
		}

		mob = load_a_saved_mobile (nVirtual, coldload, fp);

		if (!return_mob)
			return_mob = mob;

		mob->char_to_room(mob->in_room);

		if (!last_mob)
			;

		last_mob = mob;
	}

	fclose (fp);

	return return_mob;
}

	//loads specific data from text file 
CHAR_DATA *
load_a_saved_mobile (int nVirtual, int coldload, FILE * fp)
{
	int i;
	int n = 0;
	int sn;
	int last_key = 0;
	int num_keys;
	struct time_info_data healing_time;
	char *p, *p2;
	AFFECTED_TYPE *af;
	CHAR_DATA *mob, *tch;

	struct key_data key_table[] = {
		{"Room", TYPE_INT, NULL},
		{"SDesc", TYPE_STRING, NULL},
		{"LDesc", TYPE_STRING, NULL},
		{"Keys", TYPE_STRING, NULL},
		{"FightMode", TYPE_INT, NULL},
		{"Clans", TYPE_STRING, NULL},
		{"ColdloadId", TYPE_INT, NULL},
		{"Flags", TYPE_INT, NULL},
		{"Moves", TYPE_INT, NULL},
		{"Hits", TYPE_INT, NULL},
		{"Speaks", TYPE_STRING, NULL},
		{"AffectedBy", TYPE_INT, NULL},
		{"Position", TYPE_INT, NULL},
		{"Act", TYPE_INT, NULL},
		{"Prof", TYPE_INT, NULL},
		{"Cond0", TYPE_INT, NULL},
		{"Cond1", TYPE_INT, NULL},
		{"Cond2", TYPE_INT, NULL},
		{"Sex", TYPE_INT, NULL},
		{"Height", TYPE_INT, NULL},
		{"Frame", TYPE_INT, NULL},
		{"AccessFlags", TYPE_INT, NULL},
		{"Morph_time", TYPE_INT, NULL},
		{"SpawnLocation", TYPE_SPAWN, NULL},
		{"Skill", TYPE_SKILL, NULL},
		{"Owner", TYPE_OWNER, NULL},
		{"Affect", TYPE_AFFECT, NULL},
		{"HasInventory", TYPE_HAS_INV, NULL},
		{"DONE", TYPE_DONE, NULL},
		{"End", TYPE_END, NULL},
		{"\0", TYPE_INT, NULL}
	};
	
		//loads a copy of the proto-type

		//if (!(mob = load_mobile (nVirtual)))
	if (!(mob = load_stayput_mobile_mysql(nVirtual, coldload)))
	{
		return NULL;
	}

	
		//now get anything that has changed between proto-type and the text files of changes
	key_table[n++].ptr = &mob->in_room;
	key_table[n++].ptr = &mob->short_descr;
	key_table[n++].ptr = &mob->long_descr;
	key_table[n++].ptr = &mob->keywords;
	key_table[n++].ptr = &mob->coldload_id;
	key_table[n++].ptr = &mob->flags;
	key_table[n++].ptr = &mob->move;
	key_table[n++].ptr = &mob->hit;
	key_table[n++].ptr = &mob->speaks;
	key_table[n++].ptr = &mob->affected_by;
	key_table[n++].ptr = &mob->position;
	key_table[n++].ptr = &mob->mob->action;
	key_table[n++].ptr = &mob->mob->profession;
	key_table[n++].ptr = &mob->intoxication;
	key_table[n++].ptr = &mob->hunger;
	key_table[n++].ptr = &mob->thirst;
	key_table[n++].ptr = &mob->sex;
	key_table[n++].ptr = &mob->height;
	key_table[n++].ptr = &mob->frame;
	key_table[n++].ptr = &mob->mob->access_flags;
	key_table[n++].ptr = &mob->mob->morph_time;
	key_table[n++].ptr = NULL;
	key_table[n++].ptr = NULL;
	key_table[n++].ptr = NULL;
	key_table[n++].ptr = NULL;
	key_table[n++].ptr = NULL;
	key_table[n++].ptr = NULL;

	for (num_keys = 0; *key_table[num_keys].key;)
		num_keys++;

	for (; !feof (fp);)
	{

		if (!(p = fread_word (fp)))
		{
			free_char (mob);
			return NULL;
		}

			//look for a match to 'p'
		for (i = last_key;
			i < last_key + num_keys &&
			str_cmp (key_table[i % num_keys].key, p);)
			i++;

		i = i % num_keys;

			//didn't find one, get next word
		if (str_cmp (key_table[i].key, p))
		{
			continue;
		}

		if (key_table[i].key_type == TYPE_SKILL ||
			key_table[i].key_type == TYPE_AFFECT ||
			key_table[i].key_type == TYPE_OWNER ||
			key_table[i].key_type == TYPE_HAS_INV ||
			key_table[i].key_type == TYPE_SPAWN)
			last_key = i;
		else
			last_key = i + 1;

		if (key_table[i].key_type == TYPE_INT)
			*(int *) key_table[i].ptr = fread_number (fp);

		else if (key_table[i].key_type == TYPE_SHORTINT)
			*(shortint *) key_table[i].ptr = fread_number (fp);

		else if (key_table[i].key_type == TYPE_STRING)
			*(char **) key_table[i].ptr = unspace (fread_string (fp));

		else if (key_table[i].key_type == TYPE_LONG)
			*(long *) key_table[i].ptr = fread_number (fp);

		else if (key_table[i].key_type == TYPE_OBSOLETE)
			sn = fread_number (fp);

		else if (key_table[i].key_type == TYPE_SPAWN)
			mob->mob->spawnpoint = fread_number (fp);


		else if (key_table[i].key_type == TYPE_HAS_INV)
			read_obj_suppliment (mob, fp);

		else if (key_table[i].key_type == TYPE_SKILL)
		{

			sn = lookup_skill_id(p = fread_word (fp));

			if (sn == -1)
			{
				return NULL;
			}

			if (sn > MAX_SKILLS)
				printf ("Skill Num # %d (learned %d) out of range.\n",
				sn, fread_number (fp));
			else
				mob->skill_map[lookup_skill_name(sn)] = fread_number (fp);
		}

		else if (key_table[i].key_type == TYPE_OWNER)
		{
			mob->mob->owner = unspace (fread_string (fp));
		}

		else if (key_table[i].key_type == TYPE_AFFECT)
		{

			af = new AFFECTED_TYPE;

			fscanf (fp, "%d %d %d %d %d %d %ld\n",
				&af->type,
				&af->a.spell.duration,
				&af->a.spell.modifier,
				&af->a.spell.location,
				&af->a.spell.bitvector, &af->a.spell.sn, &af->a.spell.t);
			af->next = NULL;

			affect_to_char (mob, af);
		}

		
		
		else if (key_table[i].key_type == TYPE_DONE)
		{

			mob->flags &= ~(FLAG_ENTERING | FLAG_LEAVING);


			if (get_affect (mob, MAGIC_AFFECT_SLEEP))
				mob->set_position(SLEEP);

			mob->time_str.logon = time (0);

			return mob;
		}

		else if (key_table[i].key_type == TYPE_END)
		{
			return mob;
		}
	}

	free_char (mob);

	return NULL;
}

void
save_mobile (CHAR_DATA * mob, FILE * fp, char *save_reason, int extract)
{
	
	AFFECTED_TYPE *af;
	

	if (!IS_NPC (mob))
		return;

	fprintf (fp, "%d %s %d\n", mob->mob->nVirtual, save_reason, mob->coldload_id);

	fprintf (fp, "Room             %d\n", mob->in_room);
	
	
	if (mob->mob->owner)
		fprintf (fp, "Owner            %s~\n", mob->mob->owner);
/**** also saved in mob-pfiles
	for (af = mob->hour_affects; af; af = af->next)
		if (af->type != MAGIC_CLAN_NOTIFY &&
			af->type != MAGIC_NOTIFY &&
			af->type != MAGIC_WATCH1 &&
			af->type != MAGIC_WATCH2 &&
			af->type != MAGIC_WATCH3 )
			fprintf (fp, "Affect       %d %d %d %d %d %d %ld\n",
			af->type,
			af->a.spell.duration,
			af->a.spell.modifier,
			af->a.spell.location,
			af->a.spell.bitvector, af->a.spell.sn, af->a.spell.t);

	
*********/
	
	if (mob->equip || mob->right_hand || mob->left_hand)
	{
		fprintf (fp, "HasInventory\n");
		write_obj_suppliment (mob, fp);
	}

	fprintf (fp, "Done\n");

	fprintf (fp, "End\n");

	if (extract)
		mob->extract_char();
}

void
save_attached_mobiles (CHAR_DATA * ch, int extract)
{
	FILE *fp;
	char save_name[MAX_STRING_LENGTH]= { '\0' };

	sprintf (save_name, "save/player/%c/%s.a", tolower (*ch->name), ch->name);

	if (!(fp = fopen (save_name, "w")))
	{
		return;
	}

	
	fprintf (fp, "end\n");
	fclose (fp);
}
