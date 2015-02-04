//////////////////////////////////////////////////////////////////////////////
//
/// constants.h - General Pre-Defined Constants
//
//
/// Crafters RPI
/// Copyright (C) 2015 M. C. Huston
/// Authors: M.C Huston (auroness@gmail.com)
//
/// Shadows of Isildur RPI Engine++
/// Copyright (C) 2005-2006 C. W. McHenry
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


#ifndef _rpie_constants_h_
#define _rpie_constants_h_

#define STAFF_EMAIL		"auroness@gmail.com"

#define APP_EMAIL		"auroness@gmail.com"
#define CODE_EMAIL		"auroness@gmail.com"
#define PET_EMAIL		"auroness@gmail.com"
#define REPORT_EMAIL	"auroness@gmail.com"

#define IMPLEMENTOR_ACCOUNT	"God"
#define SERVER_LOCATION		"localhost"

#define MUD_NAME		"Crafters"
#define MUD_EMAIL		"auroness@gmail.com"

/* Be sure to define without trailing slashes! */

#define PATH_TO_SENDMAIL	"/usr/sbin/sendmail"

/* Other miscellaneous filepath defines; absolute filepaths only! */

#define PATH_TO_WEBSITE		""

#define B_BUF_SIZE				262144


/* Misc defines */

#define OOC_LOUNGE			1
#define MUSEUM_FOYER		1

#define JUNKYARD			1
#define LINKDEATH_HOLDING_ROOM		1

#define PREGAME_ROOM_PROTOTYPE		1
#define PREGAME_ROOM_NAME		"A Private Room in Club Endore"
#define	NEW_PLAYER_TRUNK		945
#define OOC_BRIEFING_SIGN		965
#define OOC_TERMINAL_OBJECT		125

#define START_LOC		1325
#define HEALER_KIT_VNUM			2155

#define LAST_PC_RACE			29
#define LAST_ROLE				9
#define MAX_SPECIAL_ROLES		50

#define NUM_ATTRIBUTES		8   //number of attributes

#define CONSTITUTION_MULTIPLIER		3	/* Damage absorption limit for any */
/* humanoid N/PC is 50 + (con+power) x multiplier */

/* color system */

#define BELL "\007"

/* main loop pulse control */

#define PULSES_PER_SEC		4

#define PULSE_AUTOSAVE		(SECOND_PULSE * 60)
#define PULSE_DELAY			4
#define PULSE_SMART_MOBS	(SECOND_PULSE * 1)
#define PULSE_VIOLENCE		8

/* 1500 = 10 RL hrs per IG day */
#define RL_SEC_PER_IG_HOUR	1500
#define IG_HOUR_PER_RL_HOUR	2.4

	//10 RL hours = 24 IG hours
	//1 RL hour = 2.4 IG hours
	//3600 RL seconds = 2.4 IG hours
	//1500 RL seconds = 1 IG hour
	//25 RL minutes = 1 IG hour

/* string stuff */

#define BUFFER_16B 16
#define BUFFER_32B 32
#define BUFFER_64B 64
#define BUFFER_128B 128
#define BUFFER_256B 256
#define BUFFER_512B 512
#define BUFFER_1KiB 1024
#define BUFFER_2KiB 2048
#define BUFFER_4KiB 4096
#define BUFFER_8KiB 8192
#define BUFFER_16KiB 16384
#define BUFFER_32KiB 32768
#define BUFFER_48KiB 49152
#define BUFFER_64KiB 65536

#define MAX_STRING_LENGTH BUFFER_48KiB
#define AVG_STRING_LENGTH BUFFER_256B	 // more useful string len
#define ERR_STRING_LENGTH BUFFER_512B
#define MAX_NAME_LENGTH (BUFFER_16B - 1)	// player character name

#define MAX_INPUT_LENGTH     8000
#define MAX_MESSAGES          60
#define MAX_ITEMS            153

#define MAX_TEXTS			 100


#define SECS_PER_REAL_MIN   60
#define SECS_PER_REAL_HOUR  (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY   (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR  (365*SECS_PER_REAL_DAY)

#define SECOND_PULSE		4
#define UPDATE_PULSE		(4 * SECOND_PULSE)

#define MAX_CONNECTIONS		400

#define CURRENCY_TIRITH		0
#define CURRENCY_MORGUL		1
#define CURRENCY_EDEN		2
#define CURRENCY_HARAD      3
#define CURRENCY_NORTHMAN   4


#endif // _rpie_constants_h_
