//////////////////////////////////////////////////////////////////////////////
//
/// portal.h - Portal Class Structures and Functions
// (replace doors and other exits)
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


#ifndef _rpie_portal_h
#define _rpie_portal_h

#include <map>
#include <string>

#include "structs.h"

class room_portal_data
{
public:
	int ident;		//unique identifier for each portal
	int deleted;	// if portal is to be deleted
	int zone;
	int type;			//opening, door, gate, wall, etc
	int port_flags;		//closed, locked, hidden etc
	int key_num_1;		//if it has a key, 0 if there is no key, for one side
	int key_num_2;		//if it has a key, 0 if there is no key, for the other side
	int pick_key_pen_1;		//penalty to pick lock on side 1
	int pick_key_pen_2;		//penalty to pick lock on side 2
	char *keywords_1;		//keywords for side 1
	char *keywords_2;		//keywords for side 2 - will be the same as 1 if this isn't used

	char *sdesc_1;		//sdesc on 1 side
	char *sdesc_2;		//sdesc on other side will be the same as 1 if this isn;'t used
	char *ldesc_1;		//ldesc on 1 side
	char *ldesc_2;		//ldesc on other side
	char *fdesc_1;		//full description on 1 side
	char *fdesc_2;		//full description on second side
	int room_1;			//room number on one side 
	int room_2;			//room number on the other side
	std::list<std::string> material;		//what is it made of 
	int dir_1;			//which direction is use to trigger this portal (in/out and normal directions)
	int dir_2;			//will be the opposite of dir_1 by default
	int quality;		//how good is the door 0-no door, 100-moria mines magic door
	int difficulty;		//how hard is it to pick the lock, or climb over
	float slope;		//gradient, or %slope - starts in side 1 and ends on side 2
	int skill;			//skill needed to pass this portal
	bool to_place;		//is this portal supposed to be placed?
	int fail_room;			//the destination room if they fail the skill check
	int feet_fallen;	//number of feet they fall, used to calcualte damage
	
	char *sdesc_or_default(int side);
	const char *exit_dir(int side);
	bool is_hidden();
	
	void room_portal_init()
	{
		ident = 0;
		deleted = 0;
		zone = 0;
		type = 0;			
		port_flags = 0;			
		key_num_1 = 0;		
		key_num_2 = 0;
		pick_key_pen_1 = 0;
		pick_key_pen_2 = 0;
		keywords_1 = NULL;		
		keywords_2 = NULL;	
		sdesc_1 = NULL;		
		sdesc_2 = NULL;		
		ldesc_1 = NULL;		
		ldesc_2 = NULL;		
		fdesc_1 = NULL;		
		fdesc_2 = NULL;		
		room_1 = 0;			 
		room_2 = 0;			
		dir_1 = -1;			
		dir_2 = -1;			
		quality = 0;		
		difficulty = 0;		
		slope = 0;
		skill = 0;
		to_place = 0;
		fail_room = 0;
		feet_fallen = 0;
	}
	
	~room_portal_data()
	{
		if(this->keywords_1)
			free_mem(this->keywords_1);
		
		if(this->keywords_2)
			free_mem(this->keywords_2);
		
		if(this->sdesc_1)
			free_mem(this->sdesc_1);
		
		if(this->sdesc_2)
			free_mem(this->sdesc_2);
		
		if(this->ldesc_1)
			free_mem(this->ldesc_1);
		
		if(this->ldesc_2)
			free_mem(this->ldesc_2);
		
		if(this->fdesc_1)
			free_mem(this->fdesc_1);
		
		if(this->fdesc_2)
			free_mem(this->fdesc_2);
		
	}
};





#endif
