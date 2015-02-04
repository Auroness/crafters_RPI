//////////////////////////////////////////////////////////////////////////////
//
/// object_damage.cpp : Object Damage Class
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
#include <stdlib.h>
#include <time.h>

#include "server.h"
#include "structs.h"
#include "utils.h"
#include "protos.h"
#include "utility.h"
#include <set>
#include <map>
#include <list>
#include <vector>
extern rpie::server engine;

std::map<int, OBJECT_MATERIAL*> object_material_map;
short skill_to_damage_name_index (ushort n_skill);

void
initialize_materials (void)
{
	load_materials();
	
}


/*------------------------------------------------------------------------\
|  new()                                                                  |
|                                                                         |
|  Returns a pointer to the newly allocated instance of object_damage.    |
\------------------------------------------------------------------------*/
OBJECT_DAMAGE *
object_damage__new ()
{
	OBJECT_DAMAGE *thisPtr = new object_damage;

	
	thisPtr->source = DAMAGE_NONE;
	thisPtr->type_material = 0;
	thisPtr->severity = 0;
	thisPtr->impact = 0;
	thisPtr->name = 0;
	thisPtr->when = 0;


	return thisPtr;
}


/*------------------------------------------------------------------------\
|  new_init()                                                             |
|                                                                         |
|  Returns a pointer to the newly allocated and initialized instance of   |
|  object damage.
	damage_severity[]	(impact/maxhit)
    0 - unnoticeable	0%
	1 - miniscule:		0% -4%
	2 - small:			4-8%
	3 - minor:			8-10%
	4 - moderate:		10-20%
	5 - large:			20-40%
	6 - deep:			40-60%
	7 - massive:		60-80%
	8 - terrible:		80+%
 
 
\------------------------------------------------------------------------*/
OBJECT_DAMAGE *
object_damage__new_init (int source, ushort impact,
						 int mtype, int hardness, int maxhit)
{
	OBJECT_DAMAGE *thisDamage = NULL;
	int effective_impact;
	float effective_severity;
	
	/* Exemption List (sketchy at best) */
	if ((source == DAMAGE_FIST)
		|| (source == DAMAGE_FREEZE)
		|| (source == DAMAGE_WATER))
	{
		return NULL;
	}

	if (!(thisDamage = object_damage__new ()))
	{
		return NULL;
	}

	effective_impact = impact - hardness;
	if (effective_impact < 0)
		effective_impact = 0;
	
	effective_severity = ((float)effective_impact / maxhit) * 100.0;
	if (effective_severity == 0)
		thisDamage->severity = 0;
	else if (effective_severity < 4)
		thisDamage->severity = 1;
	else if (effective_severity < 8)
		thisDamage->severity = 2;
	else if (effective_severity < 10)
		thisDamage->severity = 3;
	else if (effective_severity < 20)
		thisDamage->severity = 4;
	else if (effective_severity < 40)
		thisDamage->severity = 5;
	else if (effective_severity < 60)
		thisDamage->severity = 6;
	else if (effective_severity < 80)
		thisDamage->severity = 7;
	else 
		thisDamage->severity = 8;
	
		
	thisDamage->source = source;
	thisDamage->type_material = mtype; 
	
	
		
	
	thisDamage->impact = (source == DAMAGE_BLOOD) ? 0 : effective_impact;
	thisDamage->name = number (0, 3); //used to reference third index of damage_name[12][5][4]
	thisDamage->when = time (0);

	return thisDamage;
}



/*------------------------------------------------------------------------\
|  get_sdesc()                                                            |
|                                                                         |
|  Returns a short description of the damage instance.                    |
\------------------------------------------------------------------------*/
/**
 broad_damage_type: cloth=0, hide=1, metals=2, wood = 3, stone=4
 str_desc: a small puncture 
 **/
char *
object_damage__get_sdesc (OBJECT_DAMAGE * thisPtr)
{
	std::string str_sdesc;
	int broad_damage_type;
	char temp1[MAX_STRING_LENGTH]= { '\0' };
	char temp2[MAX_STRING_LENGTH]= { '\0' };
	char temp3[MAX_STRING_LENGTH]= { '\0' };
	extern const char* damage_severity[10];
	
	char *damage_none[5][4] = {
		{"none", "none", "none", "none"}, // vs cloth 
		{"none", "none", "none", "none"}, // vs hide 
		{"none", "none", "none", "none"}, // vs metal 
		{"none", "none", "none", "none"}, // vs wood 
		{"none", "none", "none", "none"}  // vs stone 
	};
	
	char *damage_stab[5][4] = {
		{"puncture", "hole", "perforation", "piercing"},	// vs cloth
		{"puncture", "hole", "perforation", "piercing"},	// vs hide
		{"puncture", "gouge", "perforation", "rupture"},	// vs metal
		{"hole", "split", "nick", "split"},					// vs wood
		{"chip", "scratch", "scrape", "nick"}				// vs stone
	};
	
	char *damage_pierce[5][4] = {
		{"puncture", "hole", "perforation", "piercing"},	// vs cloth
		{"puncture", "hole", "perforation", "piercing"},	// vs hide
		{"puncture", "gouge", "perforation", "rupture"},	// vs metal
		{"hole", "split", "nick", "split"},					// vs wood
		{"chip", "scratch", "scrape", "nick"}				// vs stone
	};
	
	char *damage_chop[5][4] = {
		{"slice", "cut", "slash", "gash"},		// vs cloth
		{"slice", "cut", "slash", "gash"},		// vs hide
		{"nick", "chip", "rivet", "gash"},		// vs metal
		{"split", "nick", "gouge", "hack"},		// vs wood
		{"chip", "nick", "scrape", "crack"}		// vs stone
	};
	
	char *damage_blunt[5][4] = {
		{"tear", "rip", "rent", "tatter"},		// vs cloth
		{"tear", "split", "rent", "tatter"},	// vs hide
		{"gouge", "rivet", "nick", "dent"},		// vs metal
		{"splinter", "crack", "split", "dent"}, // vs wood
		{"chip", "nick", "crack", "dent"}		// vs stone
	};
	
	char *damage_slash[5][4] = {
		{"slice", "cut", "slash", "gash"},	// vs cloth
		{"slice", "cut", "slash", "gash"},	// vs hide
		{"nick", "chip", "rivet", "gash"},	// vs metal
		{"split", "nick", "gouge", "hack"}, // vs wood
		{"chip", "nick", "scrape", "crack"} // vs stone
	};
	
	
	char *damage_freeze[5][4] = {
		{"stain", "tarnish", "tear", "tarnish"},	// vs cloth
		{"crack", "tarnish", "tarnish", "tear"},	// vs hide
		{"crack", "split", "dent", "tarnish"},		// vs metal
		{"warp", "split", "warp", "splint"},		// vs wood
		{"crack", "chip", "crack", "chip"}			// vs stone
	};
	
	char *damage_burn[5][4] = {
		{"scorching", "charring", "blackening", "hole"},		// vs cloth
		{"scorching", "charring", "blackening", "hole"},		// vs hide
		{"scorching", "charring", "blackening", "tarnish"},		// vs metal
		{"charring", "blackening", "scorching", "cindering"},	// vs wood
		{"blackening", "scorching", "blackening", "scorching"}	// vs stone
	};
	
	char *damage_tooth[5][4] = {
		{"tear", "rip", "rent", "tatter"},		// vs cloth
		{"tear", "split", "rent", "tatter"},	// vs hide
		{"gouge", "rivet", "nick", "dent"},		// vs metal
		{"dent", "splinter", "crack", "split"}, // vs wood
		{"chip", "nick", "crack", "dent"}			// vs stone
	};
	
	char *damage_claw[5][4] = {
		{"tear", "rip", "rent", "tatter"},		// vs cloth
		{"tear", "split", "rent", "tatter"},	// vs hide
		{"gouge", "rivet", "nick", "dent"},		// vs metal
		{"dent", "splinter", "crack", "split"}, // vs wood
		{"chip", "nick", "crack", "dent"}		// vs stone
	};
	
	char *damage_fist[5][4] = {
		{"dent", "deformation", "dimple", "impression"},	// vs cloth
		{"dent", "deformation", "dimple", "impression"},	// vs hide
		{"dent", "deformation", "dimple", "impression"},	// vs metal
		{"dent", "deformation", "dimple", "impression"},	// vs wood
		{"dent", "deformation", "dimple", "impression"}		// vs stone
	};
	
	char *damage_blood[5][4] = {
		{"bloodstain", "bloodstain", "blood-splatter", "stain"},	// vs cloth
		{"bloodstain", "bloodstain", "blood-splatter", "stain"},	// vs hide
		{"bloodstain", "bloodstain", "blood-splatter", "tarnish"},	// vs metal
		{"bloodstain", "bloodstain", "blood-splatter", "stain"},	// vs wood
		{"bloodstain", "bloodstain", "blood-splatter", "stain"}		// vs stone
	};
	
	char *damage_water[5][4] = {
		{"stain", "discoloration", "blemish", "splotch"},		// vs cloth
		{"stain", "discoloration", "blemish", "splotch"},		// vs hide
		{"tarnish", "corrosion", "flaking", "deterioration"},	// vs metal
		{"stain", "discoloration", "blemish", "splotch"},		// vs wood
		{"stain", "discoloration", "blemish", "splotch"}		// vs stone
	};
	
	char *damage_lightning[5][4] = {
		{"scorching", "charring", "blackening", "hole"},		// vs cloth
		{"scorching", "charring", "blackening", "hole"},		// vs hide
		{"scorching", "charring", "blackening", "tarnish"},		// vs metal
		{"charring", "blackening", "scorching", "cindering"},	// vs wood
		{"blackening", "scorching", "blackening", "scorching"}	// vs stone
	};
	
	char *damage_soiled[5][4] = {	
		{"stain", "soiled spot", "splatter", "stain"},		// vs cloth
		{"stain", "soiled spot", "splatter", "stain"},		// vs hide
		{"stain", "soiled spot", "splatter", "tarnish"},	// vs metal
		{"stain", "off colored", "splatter", "stain"},		// vs wood
		{"stain", "off colored", "splatter", "stain"}		// vs stone
	};
	
	
	if (!thisPtr)
		return NULL;
	
	str_sdesc.clear();
	
	broad_damage_type = thisPtr->type_material;

	if (is_vowel(damage_severity[thisPtr->severity][0]))
		sprintf(temp1, "an ");
	else 
		sprintf(temp1, "a ");
	
	str_sdesc.append(temp1);
				
	sprintf(temp2, "%s ",damage_severity[thisPtr->severity]);
	str_sdesc.append(temp2);
	
	if (thisPtr->source == DAMAGE_NONE )
		sprintf(temp3, "%s", damage_none[broad_damage_type][thisPtr->name]);
	
	else if (thisPtr->source == DAMAGE_STAB)
		sprintf(temp3, "%s", damage_stab[broad_damage_type][thisPtr->name]);
	
	else if (thisPtr->source == DAMAGE_PIERCE)
		sprintf(temp3, "%s", damage_pierce[broad_damage_type][thisPtr->name]);
	
	else if (thisPtr->source == DAMAGE_CHOP)
		sprintf(temp3, "%s", damage_chop[broad_damage_type][thisPtr->name]);
	
	else if (thisPtr->source == DAMAGE_BLUNT)
		sprintf(temp3, "%s", damage_blunt[broad_damage_type][thisPtr->name]);
	
	else if (thisPtr->source == DAMAGE_SLASH)
		sprintf(temp3, "%s", damage_slash[broad_damage_type][thisPtr->name]);
	
	else if (thisPtr->source == DAMAGE_FREEZE)
		sprintf(temp3, "%s", damage_freeze[broad_damage_type][thisPtr->name]);
	
	else if (thisPtr->source == DAMAGE_BURN)
		sprintf(temp3, "%s", damage_burn[broad_damage_type][thisPtr->name]);
	
	else if (thisPtr->source == DAMAGE_TOOTH)
		sprintf(temp3, "%s", damage_tooth[broad_damage_type][thisPtr->name]);
	
	else if (thisPtr->source == DAMAGE_CLAW)
		sprintf(temp3, "%s", damage_claw[broad_damage_type][thisPtr->name]);
	
	else if (thisPtr->source == DAMAGE_FIST)
		sprintf(temp3, "%s", damage_fist[broad_damage_type][thisPtr->name]);
	
	else if (thisPtr->source == DAMAGE_BLOOD)
		sprintf(temp3, "%s", damage_blood[broad_damage_type][thisPtr->name]);
	
	else if (thisPtr->source == DAMAGE_WATER)
		sprintf(temp3, "%s", damage_water[broad_damage_type][thisPtr->name]);
	
	else if (thisPtr->source == DAMAGE_LIGHTNING)
		sprintf(temp3, "%s", damage_lightning[broad_damage_type][thisPtr->name]);
	
	else if (thisPtr->source == DAMAGE_SOILED)
		sprintf(temp3, "%s", damage_soiled[broad_damage_type][thisPtr->name]);
	
	
	str_sdesc.append(temp3);
	
	
	return ((char*)str_sdesc.c_str());
}


/*------------------------------------------------------------------------\
|  write_to_file()                                                        |
|                                                                         |
|  Export a string that describes this damage instance.                   |
\------------------------------------------------------------------------*/
int
object_damage__write_to_file (OBJECT_DAMAGE * thisPtr, FILE * fp)
{
	return fprintf (fp, "Damage     %d %d %d %d %d %d\n",
		thisPtr->source, thisPtr->type_material,
		thisPtr->severity, thisPtr->impact,
		thisPtr->name,  thisPtr->when);
}


/*------------------------------------------------------------------------\
|  read_from_file()                                                       |
|                                                                         |
|  Import a string that describes a damage instance.                      |
\------------------------------------------------------------------------*/
OBJECT_DAMAGE *
object_damage__read_from_file (FILE * fp)
{
	OBJECT_DAMAGE *thisPtr = NULL;

	if (!(thisPtr = object_damage__new ()))
	{
		return NULL;
	}

	/* TODO: Make this an fscanf statement? */
	thisPtr->source = (int) fread_number (fp);
	thisPtr->type_material = fread_number (fp);
	thisPtr->severity = (int) fread_number (fp);
	thisPtr->impact = fread_number (fp);
	thisPtr->name = fread_number (fp);
	thisPtr->when = fread_number (fp);
	

	return thisPtr;
}

