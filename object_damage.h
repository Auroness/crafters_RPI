//////////////////////////////////////////////////////////////////////////////
//
/// object_damage.h - Object Damage Class
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


#ifndef _rpie_object_damage_h_
#define _rpie_object_damage_h_

/*--------------------------------------. 
|  OBJECT_DAMAGE                         \
|------------------------------------------------------------------------.
|                                                                         |
|  public members:                                                        |
|    obj_damage_type[] : array of types of damage                             |
|                                                                         |
|  public methods:                                                        |
|    object_damage__new :            alloc and return an instance.        |
|    object_damage__new_init :       as above plus initialization.        |
|    object_damage__get_sdesc :      return a short desc of the damage    |
|    object_damage__write_to_file :  write the object to a file           |
|    object_damage__read_from_file : read the object from a file          |
|                                                                         |
`------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------.
|                                                                         |
|  IMPORTANT:                                                             |
|                                                                         |
|  You can globally erase the damage from objects by shutting down the    |
|  server and running the following UNIX command:                         |
|                                                                         |
|  $ grep -R -l "^Damage" ./lib | xargs sed -i.bak '/^Damage/d'           |
|                                                                         |
`------------------------------------------------------------------------*/

#include <stdio.h>









#endif // _rpie_object_damage_h_
