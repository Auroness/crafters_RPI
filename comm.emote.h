//////////////////////////////////////////////////////////////////////////////
//
/// comm.emote.h : Non-Verbal Visual Communication Module
//
//
/// Crafters RPI
/// Copyright (C) 2015 M. C. Huston
/// Authors: M.C Huston (auroness@gmail.com)
//
// Shadows of Isildur RPI Engine++
// Copyright (C) 2005-2006 C. W. McHenry
// Authors: C. W. McHenry (traithe@middle-earth.us)
//          Jonathan W. Webb (sighentist@middle-earth.us)
// URL: http://www.middle-earth.us
//
// May includes portions derived from Harshlands
// Authors: Charles Rand (Rassilon)
// URL: http://www.harshlands.net
//
// May include portions derived under license from DikuMUD Gamma (0.0)
// which are Copyright (C) 1990, 1991 DIKU
// Authors: Hans Henrik Staerfeldt (bombman@freja.diku.dk)
//          Tom Madson (noop@freja.diku.dk)
//          Katja Nyboe (katz@freja.diku.dk)
//          Michael Seifert (seifert@freja.diku.dk)
//          Sebastian Hammer (quinn@freja.diku.dk)
//
//////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include "structs.h"

#define TRAVEL_RESET "normal"	/* use this to unset their travel string */

	//What do you look like when standing still
void do_pmote (CHAR_DATA * ch, char *argument, int cmd);


	//What do you look like when moving
void do_dmote (CHAR_DATA * ch, char *argument, int cmd);

	//adds character and object references to emotes	
void personalize_string (CHAR_DATA * src, CHAR_DATA * tar, char *emote);

void personalize_emote (CHAR_DATA * src, char *emote);

void do_emote (CHAR_DATA * ch, char *argument, int cmd);

void
do_travel (CHAR_DATA * ch, char *argument, int cmd);


bool evaluate_emote_string (CHAR_DATA * ch, std::string * first_person, std::string third_person, std::string argument);


void do_omote (CHAR_DATA * ch, char *argument, int cmd);

