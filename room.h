//////////////////////////////////////////////////////////////////////////////
//
/// room.h - Room Class Structures and Functions
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

#ifndef _rpie_room_h
#define _rpie_room_h

#include <list>
#include <map>
#include "net_link.h"
#include "portal.h"
enum exit_state
{
	unlocked_and_open,
	unlocked_and_closed,
	locked_and_closed,
	gate_unlocked_and_open,
	gate_unlocked_and_closed,
	gate_locked_and_closed
};


typedef struct room_data ROOM_DATA;

class room_data {
public:
	int nVirtual;			
	int zone;		//for building and sorting by staff
	int wzone;		//weather zones
	int enforcezone;	//for enforcements
	int terrain_type;
	int deleted;	//is this room to be removed from the database?
	float light;		//how much light is in the room?
	int search_sequence;		// used in track ()
	char *name;
	char *description;
	float x_coord;
	float y_coord;
	float z_coord;
	int room_size;  //interior, explore, wilderness
	EXTRA_DESCR_DATA *ex_description[MAX_EX_DESCR];  //signs or other virtual object descriptins
	ROOM_EXIT_DATA *dir_option[LAST_DIR + 1]; //exits in a limited version
	int portals[MAX_PORTALS];
	struct secret *secrets[LAST_DIR + 1];  //hidden descriptions located in a direction
	int room_flags;
	OBJ_DATA *contents;
	CHAR_DATA *people;
	AFFECTED_TYPE *affects;
	ROOM_EXTRA_DATA *extra; //special keywords for extra descriptions of room features
	bool psave_loaded;	//saves and loads rooms with contents
	TRACK_DATA *tracks;
	int occupants;
	int capacity; //max capacity for room
        int last_editor;
	std::multimap<int,ROOM_EXIT_DATA*> exitmap;

	room_data()
	{
		nVirtual = 0;
		zone = 0;
		wzone = 0;
		enforcezone = 0;
		terrain_type = 0;  //'urban' terrain
		deleted = 0;
		light = 0;
		search_sequence = 0;
		name = NULL;
		description = NULL;
		for (int i = 0; i < MAX_EX_DESCR; i++)
		{
			ex_description[i] = NULL;
		}
		x_coord = 0.0;
		y_coord = 0.0;
		z_coord = 0.0;
		room_size = 0; 
		for (int i = 0; i < (LAST_DIR + 1); i++)
		{
			dir_option[i] = NULL;
			secrets[i] = NULL;
		}
		for (int i = 0; i < (MAX_PORTALS); i++)
		{
			portals[i] = -1;
		}
		
		room_flags = 0;
		contents = NULL;
		people = NULL;
		affects = NULL;
		extra = NULL;
		psave_loaded = false;
		tracks = NULL;
		occupants = 0;
		capacity = 0;
		exitmap.clear();
	}
	
	room_data(int new_room_num)
	{
		nVirtual = new_room_num;
		zone = 0;
		wzone = 0;
		enforcezone = 0;
		terrain_type = 0;
		deleted = 0;
		light = 0;
		search_sequence = 0;
		name = NULL;
		description = NULL;
		for (int i = 0; i < MAX_EX_DESCR; i++)
		{
			ex_description[i] = NULL;
		}
		x_coord = 0.0;
		y_coord = 0.0;
		z_coord = 0.0;
		room_size = 0; 
		for (int i = 0; i < (LAST_DIR + 1); i++) {
			dir_option[i] = NULL;
			secrets[i] = NULL;
		}
		for (int i = 0; i < (MAX_PORTALS); i++) {
			portals[i] = -1;
		}
		room_flags = 0;
		contents = NULL;
		people = NULL;
		affects = NULL;
		extra = NULL;
		psave_loaded = false;
		tracks = NULL;
		occupants = 0;
		capacity = 0;
	}
	
	~room_data()
	{
		if(this->name)
			free_mem(this->name);
		
		if(this->description)
			free_mem(this->description);
		
		for (int i = 0; i < MAX_EX_DESCR; i++)
		{
			if(this->ex_description[i])
				free_mem(this->ex_description[i]);
		}
		
		for (int i = 0; i < (LAST_DIR + 1); i++)
		{
			if(this->dir_option[i])
				free_mem(this->dir_option[i]);
			
			if(this->secrets[i])
				free_mem(this->secrets[i]);
		}
		
		if(this->affects)
			free_mem(this->affects);
		
		if(this->extra)
			free_mem(this->extra);
		
		if(this->tracks)
			free_mem(this->tracks);
		
		exitmap.clear();
	}
	
	
};

extern std::map<int, ROOM_DATA*> room_map;
ROOM_DATA *vtor(int roomVirtual);
bool is_overcast (ROOM_DATA * room);
bool is_sunlight_restricted (CHAR_DATA * ch, ROOM_DATA * room = 0);
ROOM_DATA *generate_dwelling_room (OBJ_DATA * dwelling);
int remove_room_affect (ROOM_DATA * room, int type);
void load_save_room (ROOM_DATA * room);
void save_player_rooms ();
void load_mysql_save_rooms ();
int track_room (ROOM_DATA * from_room, int to_room);
int is_dark (ROOM_DATA * room);

void setup_dir (FILE * fl, ROOM_DATA * room, int dir, int type);

ROOM_DATA *new_room (int nVirtual);
void clone_room (ROOM_DATA * source_room, ROOM_DATA * targ_room, bool clone_exits);
int clone_contiguous_rblock (ROOM_DATA * start_room, int from_dir);
int delete_contiguous_rblock (ROOM_DATA * start_room, int from_dir, int outside_vnum);


int room_delete (ROOM_DATA * room);
void room_light (ROOM_DATA * room);

void room_affect_wearoff (ROOM_DATA * room, int type);

int is_he_there (CHAR_DATA * ch, ROOM_DATA * room);

inline bool
set_door_state (int room_number, int direction, exit_state state)
{
	ROOM_DATA* room = 0;
	if ((room = vtor (room_number)))
	{
		ROOM_EXIT_DATA *door;
		if ((door = room->dir_option[direction]))
		{
			switch (state)
			{
			case unlocked_and_open:
				door->port_flags &= ~(EX_LOCKED | EX_CLOSED);
				break;

			case unlocked_and_closed:
				door->port_flags |= EX_CLOSED;
				door->port_flags &= ~EX_LOCKED;
				break;

			case locked_and_closed:
				door->port_flags |= (EX_LOCKED | EX_CLOSED);
				break;

			case gate_unlocked_and_open:
				door->port_flags &= ~(EX_LOCKED | EX_CLOSED);
				door->port_flags |= EX_ISGATE;
				break;

			case gate_unlocked_and_closed:
				door->port_flags |= EX_CLOSED;
				door->port_flags &= ~EX_LOCKED;
				door->port_flags |= EX_ISGATE;
				break;

			case gate_locked_and_closed:
				door->port_flags |= (EX_LOCKED | EX_CLOSED);
				door->port_flags |= EX_ISGATE;
				break;
			}
			return true;
		}
	}
	return false;
}

#endif // _rpie_room_h
