//////////////////////////////////////////////////////////////////////////////
//
/// structs.h - Data Structures
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

#ifndef _rpie_structs_h_
#define _rpie_structs_h_

#include <string>
#include <string.h>
#include <set>
#include <map>
#include <list>
#include <vector>
#include </usr/local/mysql/include/mysql.h>
#include <sys/types.h>
#include <sys/time.h>

#include "constants.h"
#include "weather.h"

extern void replaceString(char *&destination, const char *source);
extern char* duplicateString(const char *source);
extern int free_mem(char *&ptr);
extern int free_mem(void *ptr);

typedef struct clan_data CLAN_DATA;
typedef class track_data TRACK_DATA;
typedef class threat_data THREAT_DATA;
typedef class attacker_data ATTACKER_DATA;
typedef class newbie_hint NEWBIE_HINT;
typedef class role_data ROLE_DATA;
typedef class writing_data WRITING_DATA;
typedef class site_info SITE_INFO;
typedef class affected_type AFFECTED_TYPE;
typedef class board_data BOARD_DATA;
typedef struct char_ability_data CHAR_ABILITY_DATA;
typedef class char_data CHAR_DATA;
typedef struct delayed_affect_data DELAYED_AFFECT_DATA;
typedef class dream_data DREAM_DATA;
typedef class extra_descr_data EXTRA_DESCR_DATA;
typedef class default_item_data DEFAULT_ITEM_DATA;
typedef class default_mob_data DEFAULT_MOB_DATA;
typedef struct memory_t MEMORY_T;
typedef class mudmail_data MUDMAIL_DATA;
typedef class mob_data MOB_DATA;
typedef class mobprog_data MOBPROG_DATA;
typedef class move_data MOVE_DATA;
typedef class name_switch_data NAME_SWITCH_DATA;
typedef class negotiation_data NEGOTIATION_DATA;
typedef class pc_data PC_DATA;
typedef class obj_data OBJ_DATA;
typedef class phase_data PHASE_DATA;
typedef class qe_data QE_DATA;
typedef struct race_data RACE_TABLE_ENTRY;
typedef class reset_affect RESET_AFFECT;
typedef struct reset_time_data RESET_TIME_DATA;
typedef class room_portal_data ROOM_PORTAL_DATA;
typedef class room_exit_data ROOM_EXIT_DATA;
typedef struct room_extra_data ROOM_EXTRA_DATA;
typedef class shop_data SHOP_DATA;
typedef class second_affect SECOND_AFFECT;
typedef struct skill_data SKILL_DATA;
typedef class social_data SOCIAL_DATA;
typedef struct spell_data SPELL_DATA;
typedef class subcraft_head_data SUBCRAFT_HEAD_DATA;
typedef struct time_info_data TIME_INFO_DATA;
typedef class variant_value VARIANT_VALUE;
typedef class object_material OBJECT_MATERIAL;
typedef struct body_info BODY_INFO;
typedef class sighted_data SIGHTED_DATA;
typedef struct sphere_info SPHERE_INFO;
typedef class text_data TEXT_DATA;
typedef class obj_clan_data OBJ_CLAN_DATA;
typedef class body_loc_info BODY_LOC_INFO;
typedef class object_damage OBJECT_DAMAGE;
typedef struct affect_soma_type SOMA_TYPE;

#ifndef __cplusplus
typedef char bool;
#endif
typedef char byte;
typedef unsigned long bitflag;
typedef signed char shortint;

#define TREAT_ALL		(1 << 0)
#define TREAT_SLASH		(1 << 1) //stab, chop, slash, claw damage
#define TREAT_BLUNT		(1 << 2) //blunt, fist damage 
#define TREAT_PUNCTURE	(1 << 3) //pierce, tooth damage 
#define TREAT_BURN		(1 << 6) //burns
#define TREAT_FROST		(1 << 7) //freeze
#define TREAT_BLEED		(1 << 8)
#define TREAT_INFECTION		(1 << 9) //for future expansion

#define TOK_WORD		0
#define	TOK_NEWLINE		1
#define TOK_PARAGRAPH	2
#define TOK_END			3
#define TOK_SENTENCE	4

#define SEARCH_KEYWORD		1
#define SEARCH_SDESC		2
#define SEARCH_LDESC		3
#define SEARCH_FDESC		4
#define SEARCH_RACE		5
#define SEARCH_CLAN		6
#define SEARCH_SKILL		7
#define SEARCH_ROOM		8
#define SEARCH_LEVEL		9
#define SEARCH_STAT		10

/* Second Affects */
#define SA_STAND		1
#define SA_GET_OBJ		2
#define SA_WEAR_OBJ		3
#define SA_CLOSE_DOOR		4
#define SA_WORLD_SWAP		5
#define SA_WALK			6
#define SA_RUN			7
#define SA_FOLLOW		8
#define SA_SHADOW		9
#define SA_FLOOD		10
#define SA_KNOCK_OUT		11
#define SA_ESCAPE		12
#define SA_MOVE			13
#define SA_COMMAND 	 	14
#define SA_WARNED		15
#define SA_ALREADY_WARNED 16
#define SA_FLEEING_WARNED 17
#define SA_DOANYWAY	18
#define SA_GFOLLOW 19
#define SA_LEAD 20
#define SA_FLEE 21


#define VNUM_TICKET		44

#define VNUM_PARCHMENT	61
#define VNUM_SPEAKER_TOKEN	175
#define VNUM_STATUE		195
#define VNUM_ORDER_TICKET	575



#define VNUM_TRAIL_OBJECT_1			91686
#define VNUM_TRAIL_OBJECT_2			91687

#define VNUM_BASE_MOBILE		5995

#define VNUM_DEFAULT_LOAD_ROOM	1		//where they go if problem with the room number
#define IMPORT_ZONE		99		//zone where all imports are loaded


	//CURRENCY_EDEN
#define EDEN_FIRST		66900

#define EDEN_1			66900
#define EDEN_5			66901
#define EDEN_50			66902
#define EDEN_200		66903
#define EDEN_K			66904
#define EDEN_10K		66905

#define EDEN_LAST		66905

	//CURRENCY_TIRITH
#define GONDOR_FIRST	1538  

#define GONDOR_1		1538	
#define GONDOR_5		1539	
#define GONDOR_50		1540
#define GONDOR_200		1541	
#define GONDOR_K		1542	
#define GONDOR_10K		1543  

#define GONDOR_LAST		1543

	//keeping non-gondorian coins so they can be used as objects, but not as money
	//CURRENCY_HARAD
#define HARAD_FIRST		80010

#define HARAD_1			80010
#define HARAD_5			80011
#define HARAD_50		80012
#define HARAD_200		80013
#define HARAD_K			80014
#define HARAD_10K		80015

#define HARAD_LAST		80015

	//CURRENCY_MORGUL
#define MORGUL_FIRST	5030

#define MORGUL_1		5030
#define MORGUL_5		5031
#define MORGUL_50		5032
#define MORGUL_200		5033
#define MORGUL_K		5034
#define MORGUL_10K		5035

#define MORGUL_LAST		5035

	//CURRENCY_NORTHMAN
#define NORTH_FIRST		42131

#define NORTH_1			42131
#define NORTH_4			42132
#define NORTH_24		42133
#define NORTH_960		42134

#define NORTH_LAST		42134


#define HOLIDAY_METTARE		1
#define HOLIDAY_YESTARE		2
#define HOLIDAY_TUILERE		3
#define HOLIDAY_LOENDE		4
#define HOLIDAY_ENDERI		5
#define HOLIDAY_YAVIERE		6

#define SPRING	        0
#define SUMMER  	    1
#define AUTUMN		    2
#define WINTER	        3

#define BASE_SPELL		1000

#define BLEEDING_INTERVAL 1	/* 1 minute per "bleeding pulse". */
#define BASE_SPECIAL_HEALING 30	/* Increased healing rate, for special mobs/PCs. */
#define BASE_PC_ELF_HEALING 15      // -
#define BASE_PC_DWARF_HEALING 20    //  | November 21st, Power changes - Case
#define BASE_PC_ORCTROLL_HEALING 30 // -
#define BASE_PC_STANDARD_HEALING 40	/* Number of minutes per healing pulse for standard PCs. */

/* debug parameters */

#define DEBUG_MISC		2
#define DEBUG_SKILLS	4
#define DEBUG_SUMMARY	8
#define DEBUG_SUBDUE	16

/* registry */

#define REG_REGISTRY		0
#define REG_MAXBONUS		1
#define REG_SPELLS			2
#define REG_DURATION		3
#define REG_OV				4
#define REG_LV				5
#define REG_CAP				6
#define REG_SKILLS			7	/* Link to DURATION, OV, LV, CAP */
#define REG_MISC			8
#define REG_MISC_NAMES		9	/* Link to MISC */
#define REG_MISC_VALUES		10
#define REG_MAGIC_SPELLS	11
#define REG_MAGIC_SPELLS_D	12	/* Linked to MAGIC_SPELLS */
#define REG_MAX_RATES		13	/* Learning rates table skill vs formula */
#define REG_CRAFT_MAGIC		14
#define REG_AFFECT          15 /* misc affect values */

#define MISC_MAX_MOVE		3
#define MISC_STEAL_DEFENSE	4

#define CRIM_STEALING	3


/* generic find defines */

#define FIND_CHAR_ROOM    	(1<<0)
#define FIND_CHAR_WORLD    	(1<<1)
#define FIND_OBJ_INV      	(1<<2)
#define FIND_OBJ_ROOM     	(1<<3)
#define FIND_OBJ_WORLD   	(1<<4)
#define FIND_OBJ_EQUIP   	(1<<5)

/* mob/object hash */

#define GEN_MOB 	1
#define GEN_OBJ 	2
#define GEN_WLD 	3
#define MAX_HASH    1000	/* 100 vnums per hash */

#ifndef SIGCHLD
#define SIGCHLD SIGCLD
#endif


#define MESS_ATTACKER 1
#define MESS_VICTIM   2
#define MESS_ROOM     3

#define MAX_WEATHER_AREAS	2
	//room->wzone
#define WEATHER_ANGRENOST	0
#define WEATHER_TIRITH		1
	//add in separate weather for mountians, swamps, or other areas

#define MAX_WEATHER_ZONES	6 //number of different weather zones

#define WEATHER_TEMPERATE	0
#define WEATHER_COOL		1
#define WEATHER_COLD		2
#define WEATHER_ARCTIC		3
#define WEATHER_WARM		4
#define WEATHER_HOT			5
#define WEATHER_DESERT		6

/* Weather-room descrition constants - see weather_room in constants.c */



#define		WR_FOGGY		0
#define		WR_CLOUDY		1
#define		WR_RAINY		2
#define		WR_STORMY		3
#define		WR_SNOWY		4
#define		WR_BLIZARD		5
#define		WR_NIGHT		6
#define		WR_NIGHT_FOGGY		7
#define		WR_NIGHT_RAINY		8
#define		WR_NIGHT_STORMY		9
#define		WR_NIGHT_SNOWY		10
#define		WR_NIGHT_BLIZARD	11
#define		WR_NIGHT_CLOUDY     12
#define		WR_SPRING			13
#define		WR_SUMMER			14
#define		WR_AUTUMN			15
#define		WR_WINTER			16
#define		WR_NIGHT_SPRING		17
#define		WR_NIGHT_SUMMER		18
#define		WR_NIGHT_AUTUMN		19
#define		WR_NIGHT_WINTER		20
#define		WR_NORMAL			21
#define		WR_LAST_DESCRIPTIONS		21	//extra weather descriptions

/* For 'type_flag' */

#define ITEM_LIGHT          1
#define ITEM_DRYCON         2
#define ITEM_TREASURE       3
#define ITEM_POTION    	    4 
#define ITEM_WORN      	    5
#define ITEM_OTHER     	    6
#define ITEM_TRASH     	    7 
#define ITEM_TRAP      	    8 
#define ITEM_CONTAINER 	    9
#define ITEM_NOTE      	    10 
#define ITEM_DRINKCON   	11
#define ITEM_KEY       	    12
#define ITEM_FOOD      	    13
#define ITEM_MONEY     	    14
#define ITEM_ORE       	    15 
#define ITEM_BOARD     	    16
#define ITEM_FOUNTAIN  	    17
#define ITEM_GRAIN		    18
#define ITEM_PERFUME	    19
#define ITEM_POTTERY	    20
#define ITEM_PLANT          21
#define ITEM_COMPONENT	    22
#define ITEM_HERB		    23
#define ITEM_SALVE		    24
#define ITEM_POISON		    25
#define ITEM_LOCKPICK	    26
#define ITEM_INST_WIND      27
#define ITEM_INST_PERCU     28
#define ITEM_INST_STRING 	29
#define ITEM_FUR        	30
#define ITEM_WOODCRAFT  	31
#define ITEM_SPICE		    32
#define ITEM_TOOL		    33
#define ITEM_USURY_NOTE		34
#define ITEM_TICKET			35
#define ITEM_DYE			36
#define ITEM_CLOTH			37
#define ITEM_INGOT			38
#define ITEM_TIMBER			39 
#define ITEM_FLUID			40
#define ITEM_FUEL			41
#define ITEM_HEALER_KIT		42
#define ITEM_PARCHMENT		43
#define ITEM_BOOK			44
#define ITEM_WRITING_INST	45
#define ITEM_INK			46
#define ITEM_KEYRING		47
#define ITEM_NPC_OBJECT		48
#define ITEM_REPAIR_KIT		49
#define ITEM_TOSSABLE		50
#define ITEM_MERCH_TICKET	51
#define ITEM_ROOM_RENTAL	52

/* Bitvector For 'wear_flags' */

#define ITEM_TAKE           ( 1 << 0 )
#define ITEM_WEAR_FINGER    ( 1 << 1 )
#define ITEM_WEAR_NECK      ( 1 << 2 )
#define ITEM_WEAR_BODY      ( 1 << 3 )
#define ITEM_WEAR_HEAD      ( 1 << 4 )
#define ITEM_WEAR_LEGS      ( 1 << 5 )
#define ITEM_WEAR_FEET      ( 1 << 6 )
#define ITEM_WEAR_HANDS     ( 1 << 7 )
#define ITEM_WEAR_ARMS      ( 1 << 8 )
#define ITEM_WEAR_ABOUT		( 1 << 9 )
#define ITEM_WEAR_WAIST   	( 1 << 10 )
#define ITEM_WEAR_WRIST    	( 1 << 11 )
#define ITEM_WEAR_BELT		( 1 << 12 )
#define ITEM_WEAR_BACK		( 1 << 13 )
#define ITEM_WEAR_BLINDFOLD	( 1 << 14 )
#define ITEM_WEAR_THROAT	( 1 << 15 )
#define ITEM_WEAR_EAR		( 1 << 16 )
#define ITEM_WEAR_SHOULDER	( 1 << 17 )
#define ITEM_WEAR_ANKLE		( 1 << 18 )
#define ITEM_WEAR_HAIR		( 1 << 19 )
#define ITEM_WEAR_FACE		( 1 << 20 )
#define ITEM_WEAR_ARMBAND	( 1 << 21 )	/* Shoulder Patches */

/** flags 24 to 31 available for expansion **/

/* Keywords for wearing items used in do_wear()*/
#define KEYWEAR_TAKE		0
#define KEYWEAR_LIGHT		0	//need both of the 0 value defines to keep code clear
#define KEYWEAR_FINGER		1
#define KEYWEAR_NECK		2
#define KEYWEAR_BODY		3
#define KEYWEAR_HEAD		4
#define KEYWEAR_LEGS		5
#define KEYWEAR_FEET		6
#define KEYWEAR_HANDS		7
#define KEYWEAR_ARMS		8
#define KEYWEAR_ABOUT		9
#define KEYWEAR_WAIST		10
#define KEYWEAR_WRIST		11
#define KEYWEAR_BELT		12
#define KEYWEAR_BACK		13
#define KEYWEAR_BLINDFOLD	14
#define KEYWEAR_THROAT		15
#define KEYWEAR_EARS		16
#define KEYWEAR_SHOULDER	17
#define KEYWEAR_ANKLE		18
#define KEYWEAR_HAIR		19
#define KEYWEAR_FACE		20
#define KEYWEAR_ARMBAND		21

/* NOTE: UPDATE wear_bits in constants.c */

/* Bitvector for 'extra_flags' */

#define ITEM_DESTROYED      ( 1 << 0 )
#define ITEM_INVISIBLE      ( 1 << 1 )
#define ITEM_MAGIC          ( 1 << 2 )
#define ITEM_NODROP         ( 1 << 3 )
#define ITEM_GET_AFFECT	    ( 1 << 4 )
#define ITEM_DROP_AFFECT    ( 1 << 5 )
#define ITEM_MULTI_AFFECT   ( 1 << 6 )
#define ITEM_WEAR_AFFECT    ( 1 << 7 )
#define ITEM_HOLD_AFFECT   ( 1 << 8 )
#define ITEM_HIT_AFFECT	    ( 1 << 9 )
#define ITEM_OK	      		( 1 << 10 )
#define ITEM_LEADER	        ( 1 << 11 )
#define ITEM_MEMBER	        ( 1 << 12 )
#define ITEM_OMNI	        ( 1 << 13 )
#define ITEM_ILLEGAL	    ( 1 << 14 )
#define ITEM_RESTRICTED	    ( 1 << 15 )
#define ITEM_MASK	        ( 1 << 16 )
#define ITEM_TABLE			( 1 << 17 )
#define ITEM_STACK			( 1 << 18 )	/* Item stack with same vnum objs */
#define ITEM_VARIABLE		( 1 << 19 )
#define ITEM_TIMER			( 1 << 20 )	/* Will decay */
#define ITEM_PC_SOLD		( 1 << 21 )	/* Sold to shopkeep by PC. */
#define ITEM_VNPC			( 1 << 22 )	/* Item exists but isn't visible to players */
#define ITEM_NODESTROY		( 1 << 23 ) // item cannot be damaged by PCs
	//values 26 to 31 available for expansion


#define DAMAGE_NONE			0
#define DAMAGE_STAB			1
#define DAMAGE_PIERCE		2	//peck
#define DAMAGE_CHOP			3
#define DAMAGE_BLUNT		4
#define DAMAGE_SLASH		5
#define DAMAGE_FREEZE		6	
#define DAMAGE_BURN			7
#define DAMAGE_TOOTH		8	//bite
#define DAMAGE_CLAW			9
#define DAMAGE_FIST			10	//punch
#define DAMAGE_BLOOD		11		
#define DAMAGE_WATER		12
#define DAMAGE_LIGHTNING	13
#define DAMAGE_SOILED		14
#define DAMAGE_PERMANENT	15	
#define DAMAGE_REPAIR		16
#define DAMAGE_STUN			17
#define DAMAGE_NATURAL_FLAG	18

#define DAMAGE_TYPE_MAX		18


/* for containers  - value[1] */

#define CONT_CLOSEABLE  ( 1 << 0 )
#define CONT_PICKPROOF  ( 1 << 1 )
#define CONT_CLOSED     ( 1 << 2 )
#define CONT_LOCKED     ( 1 << 3 )
#define CONT_BEHEADED	( 1 << 4 )


#define OBJ_NOTIMER	-7000000
#define NOWHERE    	-1

/* Bitvector for obj 'tmp_flags' */

#define SA_DROPPED	( 1 << 0 )

/* Bitvector For 'room_flags' */
	//works no matter what terrain the room is

#define DARK			( 1 << 0 )
#define LIGHT			( 1 << 1 )
#define RUINS			( 1 << 2 )
#define NO_MOB			( 1 << 3 )
#define NO_MERCHANT		( 1 << 4 )
#define NOHIDE			( 1 << 5 )
#define INDOORS			( 1 << 6 )
#define LAWFUL			( 1 << 7 )
#define TUNNEL			( 1 << 8 )
#define CAVE			( 1 << 9 )	 //not used
#define SAFE_Q			( 1 << 10 )
#define STORAGE			( 1 << 11 )
#define NOINVLIMIT		( 1 << 12 )
#define FALL			( 1 << 13 )	
#define CLIMB			( 1 << 14 )	//not used
#define	VEHICLE			( 1 << 15 )
#define STIFLING_FOG	( 1 << 16 )
#define ROAD			( 1 << 17 )
#define WEALTHY			( 1 << 18 )	//for vnpc buying
#define POOR			( 1 << 19 )	//for vnpc buying
#define SCUM			( 1 << 20 )	//for vnpc buying
#define TEMPORARY		( 1 << 21 )	//used for pre-commence rooms
#define OOC				( 1 << 22 )
	//room_flags for future expansion (1<<24) to (1<<31) (9 flags)


/* For 'dir_option' */

#define NORTH		0
#define EAST		1
#define SOUTH		2
#define WEST		3
#define NORTHEAST	4
#define SOUTHEAST	5
#define SOUTHWEST	6
#define NORTHWEST	7
#define UP			8
#define DOWN		9

#define LAST_DIR	DOWN

/* exit_info */
	//types of connections between rooms
#define PORTAL_SPACE		0	//plain opening
#define PORTAL_DOOR			1	//can blocks vision, can block passage
#define PORTAL_GATE			2	//can block passage, allows vision
#define PORTAL_BARRIER		3	//hinders passage, no vision, pass with skill
#define PORTAL_CROSSING		4	//allows vison, passage, with skill
#define PORTAL_WINDOW		5	//no passage, allows vision
#define PORTAL_TRANSPORT	6	//transport to shops or farms, no lock, no vision

#define EX_CLOSED	    ( 1 << 0 )	//doors, windows, gates
#define EX_LOCKED	    ( 1 << 1 )	//doors, gates
#define EX_PICKPROOF	( 1 << 2 )	//doors, gates
#define EX_SECRET   	( 1 << 3 )	//doors, windows, gates, barriers
#define EX_ISGATE		( 1 << 4 )	//portal is a gate in dir_options
#define EX_ISDOOR		( 1 << 5 )	//portal is a door in dir_options
#define EX_NOSNEAK		( 1 << 6 )	//cannot sneak through the portal

#define MAX_PORTALS	20		//randomly chosen value to limit number of portals per room

/* For 'Terrain types' */

#define SECT_URBAN	    0
#define SECT_CITY      	1		//no longer used
#define SECT_ROAD      	2
#define SECT_TRAIL	    3
#define SECT_FIELD	    4
#define SECT_WOODS	    5
#define SECT_FOREST	    6
#define SECT_HILLS	    7
#define SECT_MOUNTAIN  	8
#define SECT_SWAMP     	9
#define SECT_DOCK	    10
#define SECT_CAVE	    11
#define SECT_PASTURE	12
#define SECT_HEATH	    13
#define SECT_PIT		14
#define SECT_LAKE		15
#define SECT_RIVER		16
#define SECT_OCEAN		17
#define SECT_REEF		18
#define SECT_UNDERWATER	19

/* For room scale type */
#define ROOM_SIZE_DETAIL			0	//5.5 yards per side
#define ROOM_SIZE_EXPLORE   	1	//22 yards per side
#define ROOM_SIZE_VALLEY		2	//110 yards per side
#define ROOM_SIZE_STORAGE		3	//1 yard per side
#define ROOM_SIZE_OTHER			4	//use for expansion
#define ROOM_SIZE_CREATION		5	//rooms being created with no defined size yet

	//size is in yards
#define DETAIL_SIZE			5.5
#define EXPLORE_SIZE		22
#define VALLEY_SIZE			110
#define STORAGE_SIZE		1

#define MAX_SCAN_DIST		200.0	//maximum distance that can be scanned
#define QUICK_SCAN_DIST		10.0	//yards for quick scanning distance
#define SCAN_DELAY			4
#define REALITY_FACTOR		0.033	//allows a con & wil of 15 to move reasonably

/* For 'equip' */

#define WEAR_LIGHT		0
#define WEAR_PRIM		1
#define WEAR_SEC		2
#define WEAR_BOTH		3
#define WEAR_HEAD		4
#define WEAR_HAIR		5
#define WEAR_EAR		6
#define WEAR_BLINDFOLD	7
#define WEAR_FACE		8
#define WEAR_THROAT		9
#define WEAR_NECK_1		10
#define WEAR_NECK_2		11
#define WEAR_ABOUT		12
#define WEAR_BODY		13
#define WEAR_BACK		14
#define WEAR_SHOULDER_R	15
#define WEAR_SHOULDER_L	16
#define WEAR_ARMBAND_R	17
#define WEAR_ARMBAND_L	18
#define WEAR_ARMS		19
#define WEAR_WRIST_R	20
#define WEAR_WRIST_L	21
#define WEAR_HANDS		22
#define WEAR_FINGER_R	23
#define WEAR_FINGER_L	24
#define WEAR_WAIST		25
#define WEAR_BELT_1		26
#define WEAR_BELT_2		27
#define WEAR_LEGS		28
#define WEAR_ANKLE_R	29
#define WEAR_ANKLE_L	30
#define WEAR_FEET		31

#define WEAR_CARRY_R    32
#define WEAR_CARRY_L    33


#define MAX_WEAR        34

#define MAX_SKILLS		150		//highest id number of skill from DB table 
#define MAX_CRAFTS		100
#define MAX_AFFECT		25

/* conditions */

#define DRUNK			0
#define FULL			1
#define THIRST			2

/* Bitvector for 'affected_by' */

#define AFF_UNDEF1              ( 1 << 0 )
#define AFF_INVISIBLE	        ( 1 << 1 )
#define AFF_INFRAVIS        	( 1 << 2 )
#define AFF_LEADER_COMMAND		( 1 << 3 ) 
#define AFF_GROUP            	( 1 << 4 )
#define AFF_SCAN				( 1 << 5 )
#define AFF_SNEAK				( 1 << 6 )	
#define AFF_HIDE				( 1 << 7 )	
#define AFF_FOLLOW           	( 1 << 8 )	
#define AFF_HOODED              ( 1 << 9 )	
#define AFF_SUNLIGHT_PEN		( 1 << 10 )
#define AFF_SUNLIGHT_PET		( 1 << 11 )
#define AFF_BREATHE_WATER		( 1 << 12 )

	//(1 << 13) up to (1<<31) are available for expansion

/* modifiers to char's abilities */

#define APPLY_NONE          0
#define APPLY_STR           1
#define APPLY_DEX           2
#define APPLY_INT		    3
#define APPLY_CHA		    4
#define APPLY_AUR		    5
#define APPLY_WIL           6
#define APPLY_CON           7
#define APPLY_SEX           8
#define APPLY_AGE           9
#define APPLY_CHAR_WEIGHT	10
#define APPLY_CHAR_HEIGHT	11
#define APPLY_DEFENSE		12	/* Free - APPLY_DEFENSE not used */
#define APPLY_HIT		    13
#define APPLY_MOVE		    14
#define APPLY_CASH		    15
#define APPLY_AC		    16
#define APPLY_ARMOR		    16
#define APPLY_OFFENSE		17	/* Free - APPLY_OFFENSE not used */
#define APPLY_DAMROLL		18
#define APPLY_SAVING_PARA	19
#define APPLY_SAVING_ROD	20
#define APPLY_SAVING_PETRI	21
#define APPLY_SAVING_BREATH	22
#define APPLY_SAVING_SPELL	23
#define APPLY_AGI			24


/* NOTE:  Change affect_modify in handler.c if new APPLY's are added */

	//TODO: remove these values once crafts are in a proper map
#define CRAFT_FIRST	1
#define CRAFT_LAST  9999

/* sex */

#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2

/* RACE */
#define RACE_DEFAULT	0		//id for default race

/* positions */
#define DEAD		0
#define MORT		1
#define UNCON		2
#define STUN		3
#define SLEEP		4
#define REST		5
#define SIT			6
#define STAND		7



/**************** For mobile AI expansion ******************/
	//mobile professions - from actions_bit
#define PROF_SENTINEL		( 1 << 0 )
#define PROF_REPAIR			( 1 << 1 )  
#define PROF_SOLDIER		( 1 << 2 )  
#define PROF_ENFORCER		( 1 << 3 )
#define PROF_VEHICLE	    ( 1 << 4 )
#define PROF_CRIMINAL		( 1 << 5 )
#define PROF_PET			( 1 << 6 )	
#define PROF_WILDLIFE		( 1 << 7 )	
#define PROF_JAILER			( 1 << 8 )	
#define PROF_PHYSICIAN		( 1 << 9 )
#define PROF_PREY			( 1 << 10 )	

	//mobile_actions - from actions_bit
#define ACT_MEMORY	    ( 1 << 0 )
#define ACT_NOCOMMAND   ( 1 << 1 )
#define ACT_ISNPC       ( 1 << 2 )
#define ACT_NOVNPC	    ( 1 << 3 )	
#define ACT_AGGRESSIVE 	( 1 << 4 )
#define ACT_WIMPY     	( 1 << 5 )
#define ACT_PURSUE		( 1 << 6 )
#define ACT_NOORDER	    ( 1 << 7 )
#define ACT_NOBUY	    ( 1 << 8 )
#define ACT_STEALTHY	( 1 << 9 )	
#define ACT_PCOWNED		( 1 << 10 )
#define ACT_STAYPUT		( 1 << 11 )	
#define ACT_PASSIVE		( 1 << 12 )	
#define ACT_NOBIND		( 1 << 13 )
#define ACT_NOBLEED		( 1 << 14 )
#define ACT_FLYING		( 1 << 15 )
/*********************************************************/

/* For players : specials.act */

#define PLR_QUIET	    ( 1 << 19 )	/* WAS ( 1 << 4 ) - conflicts with ACT_NOVNPC */
#define PLR_STOP	    ( 1 << 15 )


/** Affects defines range from 600 to 20000+ **/
#define JOB_1					600
#define JOB_2					601
#define JOB_3					602

/* Affect for toggling listen/mute */
#define MUTE_EAVESDROP				620


#define AFFECT_SHADOW			900	/* Affected n/pc shadows another n/pc */

#define AFFECT_LOST_CON			980


#define MAGIC_CRIM_BASE		    1000	/* Criminal tags reservd from 1000..1100 */
#define MAGIC_CRIM_RESERVED	    1100	/* 1000..1100 are reserved */


#define MAGIC_SKILL_GAIN_STOP		1500	// Skill use 
#define MAGIC_CRAFT_BRANCH_STOP		2000
#define MAGIC_CRAFT_DELAY		2010

#define MAGIC_HIDDEN			2011
#define MAGIC_SNEAK				2012
#define MAGIC_NOTIFY			2013
#define MAGIC_CLAN_NOTIFY		2014

#define MAGIC_DRAGGER			2017
#define MAGIC_WATCH1			2019
#define MAGIC_WATCH2			2020
#define MAGIC_WATCH3			2021
#define MAGIC_STAFF_SUMMON		2022
#define AFFECT_GROUP_RETREAT	2023


#define AFFECT_FLEE_NORTH		2150	//fleeing to the north
#define AFFECT_FLEE_EAST		2151
#define AFFECT_FLEE_SOUTH		2152
#define AFFECT_FLEE_WEST		2153
#define AFFECT_FLEE_NORTHEAST	2154	
#define AFFECT_FLEE_SOUTHEAST	2155
#define AFFECT_FLEE_SOUTHWEST	2156
#define AFFECT_FLEE_NORTHWEST	2157
#define AFFECT_FLEE_UP			2158
#define AFFECT_FLEE_DOWN		2159
#define AFFECT_FLEE_ANY 		2160


#define MAGIC_CRIM_HOODED		2600	/* Hooded criminal observed in zone .. */

#define MAGIC_STARED			2700	/* Don't stare again until this expires */

#define MAGIC_SKILL_MOD_FIRST	3000	/* Reserve 200 for skill mod affects */
#define MAGIC_SKILL_MOD_LAST	3200


/** 3400 - 4999 reserved for MAGIC_EFFECTs **/
#define MAGIC_AFFECT_FIRST			3400

#define MAGIC_AFFECT_DMG			3400
#define MAGIC_AFFECT_HEAL			3401
#define MAGIC_AFFECT_PROJECTILE_IMMUNITY	3402
#define MAGIC_AFFECT_INFRAVISION		3403
#define MAGIC_AFFECT_CONCEALMENT		3404
#define MAGIC_AFFECT_INVISIBILITY		3405
#define MAGIC_AFFECT_SEE_INVISIBLE		3406
#define MAGIC_AFFECT_SENSE_LIFE			3407
#define MAGIC_AFFECT_TONGUES			3408
#define MAGIC_AFFECT_LEVITATE			3409
#define MAGIC_AFFECT_SLOW			3410
#define MAGIC_AFFECT_SPEED			3411
#define MAGIC_AFFECT_SLEEP			3412
#define MAGIC_AFFECT_PARALYSIS			3413
#define MAGIC_AFFECT_FEAR			3414
#define MAGIC_AFFECT_REGENERATION		3415
#define MAGIC_AFFECT_CURSE			3416
#define MAGIC_AFFECT_DIZZINESS			3417
#define MAGIC_AFFECT_FURY			3418
#define MAGIC_AFFECT_INVULNERABILITY		3419
#define MAGIC_AFFECT_ARMOR			3420
#define MAGIC_AFFECT_BLESS			3421
#define MAGIC_AFFECT_LAST			4999

/** 5000 - 5399 reserved for Room/world special effects **/
#define MAGIC_ROOM_CALM			5000
#define MAGIC_ROOM_LIGHT		5001
#define MAGIC_ROOM_DARK			5002
#define MAGIC_ROOM_DEBUG		5003
#define MAGIC_ROOM_FLOOD		5004
#define MAGIC_WORLD_CLOUDS		5005	/* Blocks the sun */
#define MAGIC_WORLD_SOLAR_FLARE	5006	/* Creates an artificial sun */
#define MAGIC_WORLD_MOON		5006	/* Moonlight in all rooms */
	
#define MAGIC_BUY_ITEM			5400

#define MAGIC_ROOM_FIGHT_NOISE		5500

#define MAGIC_PETITION_MESSAGE		5600


#define MAGIC_FLAG_NOGAIN		6000

#define MAGIC_WARNED			6500

#define MAGIC_RAISED_HOOD		6600

#define MAGIC_SIT_TABLE			6700	/* PC acquires this affect when at a table */

#define MAGIC_FIRST_POISON		7000
#define POISON_LETHARGY			7001
#define MAGIC_LAST_POISON		7001


/** 20000 and up for SOMA effects **/
#define MAGIC_FIRST_SOMA                20000	/* SOMATIC EFFECTS TBA */
#define MAGIC_LAST_SOMA                 MAGIC_FIRST_SOMA + 108

#define TYPE_UNDEFINED		    -1

#define TYPE_SUFFERING		    200	/* KILLER CDR: Eliminate this line somehow */

#define SKILL_CEILING		80	// Top limit that skill checks are made against.

#define CONTAINER_LOC_NOT_FOUND	0
#define CONTAINER_LOC_ROOM		1
#define CONTAINER_LOC_INVENTORY	2
#define CONTAINER_LOC_WORN		3
#define CONTAINER_LOC_UNKNOWN	4


/* ========================== SKILLS ============================================== */



class skill_data {
public:
	int skill_id;
	int skill_type;
	int rpp_required;
	int last_modified;
	float max_gain;
	bool is_used;
	bool spoken;
	bool written;
	bool innate;
	std::string skill_name;
	std::string restricted_races;
	std::string open_value;
	std::string level_value;
	std::string max_cap;
	std::string func_info;
	std::string created_by;
	
	
	skill_data() {
		skill_name.clear();
		restricted_races.clear();
		open_value.clear();
		level_value.clear();
		max_cap.clear();
		func_info.clear();
		created_by.clear();
		skill_id = 0;
		skill_type = 0;
		rpp_required = 0;
		max_gain = 0;
		last_modified = 0;
		is_used = 0;
		spoken = 0;
		written = 0;
		
	}
	
	~skill_data() {

	}
};

/* =========================== WEATHER ============================================= */

/* How much light is in the land ? measured in foot-candles */

#define SUN_DARK		.0002		//no sun or moonlight
#define SUN_NOON		13000		//full noon
#define SUN_RISE_SET	40		//at sunrise/setting
#define SUN_TWILIGHT	30		//dawn/dusk
#define DEFAULT_LIGHT	10		//normal dining or living rooms
#define FAINT_LIGHT		1		//small candles
#define MOON_FULL		.0027	//full moon
#define MOON_GIBOUS		.0020	//waxing/waning moon
#define MOON_CRESCENT	.0010	//crescent moon
#define NO_MOON			.0002   //no moon is visibile


class moon_data {
public:
	int phase;
	int age;
	int moonrise;
	int moonset;
	float light;
};



/* And how is the sky ? */
#define SKY_CLOUDLESS	0
#define SKY_CLOUDY   	1
#define SKY_RAINING  	2
#define SKY_LIGHTNING	3
#define SKY_STORMY		4
#define SKY_FOGGY 		5

#define MAX_OBJ_SAVE	15

/* Delay types */

#define DEL_PICK			1
#define DEL_SEARCH			2
#define DEL_APP_APPROVE		3
#define DEL_COUNT_COIN		4
#define DEL_GATHER			5
#define DEL_COMBINE			6
#define DEL_GET_ALL			7
#define DEL_AWAKEN			8
#define DEL_ALERT			9
#define DEL_INVITE			10
#define DEL_CAMP1			11
#define DEL_CAMP2			12
#define DEL_CAMP3			13
#define DEL_CAMP4			14
#define DEL_TAKE			15
#define DEL_PUTCHAR			16
#define DEL_STARE			17
#define DEL_HIDE			18
#define DEL_SCAN			19
#define DEL_QUICK_SCAN		20
#define DEL_HIDE_OBJ		21
#define DEL_PICK_OBJ		22
#define DEL_OOC				23
#define DEL_TRACK			24
#define DEL_PURCHASE_ITEM	25
#define DEL_ORDER_ITEM		26
#define DEL_MEND_OBJECT		27
#define DEL_SCAN_ALL		28


#define MAX_EX_DESCR	20
class extra_descr_data {
public:
	char *keyword;
	char *description;
	EXTRA_DESCR_DATA *next;
	
	extra_descr_data() {
		keyword = NULL;
		description = NULL;
	}
	
};

class variant_value {
public:
	int ident;
	char* name;
	char* var_value;
	char* description;
	
	variant_value(){
		name = NULL;
		var_value = NULL;
		description = NULL;
	}
};


struct time_data_str {
	time_t birth;			/* This represents the characters age                */
	time_t logon;			/* Time of the last logon (used to calculate played) */
	long played;			/* This is the total accumulated time played in secs */
};


class writing_data {
public:
	char *message;
	char *author;
	char *date;
	char *ink;
	int language;
	int script;
	bool torn;
	int skill;
	WRITING_DATA *next_page;

	writing_data() {
		message = NULL;
		author = NULL;
		date = NULL;
		ink = NULL;
		language = -1;
		script = -1;
		torn = false;
		skill = -1;
		next_page = NULL;
	}
};


class obj_flag_data {
public:
	byte type_flag;
	int wear_flags;
	int extra_flags;
	int weight;
	int set_cost; // set by an npc-shopkeeper or imm
	long bitvector;
	
	obj_flag_data() {
		type_flag = '\0';
		wear_flags = 0;
	}
};


class obj_clan_data {
public:
	char *name;
	char *rank;
	
	obj_clan_data() {
		name = NULL;
		rank = NULL;
	}
};

	//start of union listing
/**/
struct light_data
{
	int capacity;
	int hours;
	int fuel;
	int on;
	int bright;// new value - to be used in light intensity in rooms
	int v5;
};

struct fuel_con_data  //units of fuel for light objects
{
	int capacity;
	int volume;
	int fuel;
	int refill;	//can it be re-filled or is it a one-shot fuel
	int v4;
	int v5;
};

struct drink_con_data  //can only contain liquid contents
{
	int capacity;
	int volume;		//measured in 'sips'
	int liquid;
	int v3;
	int v4;
	int v5;
};

struct dry_con_data  //like liquid, but only for non-liquid contents
{
	int capacity;
	int volume;
	int contents;
	int max_weight;
	int v4;
	int v5;
};

struct fountain_data
{
	int capacity;
	int volume;
	int liquid;
	int v3;
	int v4;
	int v5;
};

struct container_data
{
	int capacity;
	int flags;
	int key;
	int pick_penalty;
	int v4;
	int table_max_sitting;
};

struct clan_container_data
{
	int capacity;
	int flags;
	int key;
	int pick_penalty;
	int v4;
	int v5;
};

struct ticket_data
{
	int ticket_num;
	int keeper_vnum;
	int v2;
	int v3;
	int v4;
	int v5;
};

/**/

	//set to multiples of 24 so they can decrease evenly each hour
#define MAX_CALORIE		2400
#define MAX_WATER		24
#define MAX_ALCHOL		24

/**/
struct food_data
{
	int food_value;		//measured in calories
	int water_value;
	int alcohol_value;
	int v3;
	int v4;
	int bites;
};


struct fluid_data
{
	int alcohol;
	int water;		
	int food;		//measured in calories
	int v3;
	int v4;
	int v5;
};

struct default_obj_data
{
	int value[6];
};

union obj_info
{
	struct default_obj_data od;
	struct drink_con_data drinkcon;
	struct dry_con_data drycon;
	struct fuel_con_data fuelcon;
	struct container_data container;
	struct light_data light;
	struct ticket_data ticket;
	struct food_data food;
	struct fluid_data fluid;
	struct fountain_data fountain;
	struct clan_container_data locker;
};

 /**/
	//materials per object - if changed, make changes in mysql.cpp
#define MAX_OBJ_MATERIALS	3 

/* ======================== Structure for object ========================= */
/***/
class obj_data {
public:
	int deleted;
	int nVirtual;
	int zone;
	int in_room;
	int instances;		// Proto-only field; keeps track of loaded instances 
	int order_time;		// Proto-only field to track time required to order item 
	struct obj_flag_data obj_flags;
	union obj_info o;
	AFFECTED_TYPE *xaffected;
	char *name;
	char *description;		//really long description
	char *short_description;
	char *full_description;
	
	char *omote_str;
	
	CHAR_DATA *carried_by;
	CHAR_DATA *equiped_by;
	OBJ_DATA *in_obj;
	OBJ_DATA *contains;
	OBJ_DATA *next_content;
	OBJ_DATA *next;
	int clock;
	int morphTime;
	int morphto;
	int location;
	int contained_wt;
	int activation;		// Affect applied when picked up 
	int quality;
	int econ_flags;		// Flag means used enhanced prices 
	int size;
	int count;			// How many this obj represents 
	int obj_timer;
	float coppers;		// Partial value in farthings    
	float max_hit_points;		//weight * average hardness of materials
	int current_damage;
	int item_wear;		// Percentile; 100%, brand new 
	WRITING_DATA *writing;
	char *ink_color;
	unsigned int open;		// 0 closed, 1+ open/page number
	OBJ_DATA *loaded;
	char *desc_keys;
	char *book_title;
	int title_skill;
	int title_language;
	int title_script;
	char *materials[MAX_OBJ_MATERIALS];
	int tmp_flags;
	bool writing_loaded;
	int coldload_id;
	std::vector<OBJECT_DAMAGE *> damage;
	char *indoor_desc;
	int sold_at;
	int sold_by;
    int last_editor;
	OBJ_CLAN_DATA *clan_data;
	

	std::vector<int> var_val;
	std::vector<char *> protect_loc;
	void deep_copy (OBJ_DATA *copy_from);
	void partial_deep_copy (OBJ_DATA *copy_from);
	
	~obj_data ();
};

 /***/

class object_damage {
public:
	int source;		// what was the cause of damage 
	int type_material;		//0-cloth, 1-hide, 2-metal, 3-wood, 4-stone
	int severity;			// index to impact adjective 
	ushort impact;			// damage inflicted on the object 
	ushort name;			// index to a damage name 
	int when;
	
	object_damage(){
		source = DAMAGE_NONE;
		type_material = 0;
		severity = 0;
		impact = 0;
		name = 0;
		when = 0;
	}
	
};

class object_material {
public:
	char *material_name;		//leather, cobwebs, metal, etc
	int type_material;			//0-cloth, 1-hide, 2-metal, 3-wood, 4-stone - for damage name
	int ident;			//used by SQL
	int hardness;		//deteremines hit points
	char *qual_strings[7];	//strings for quality
	
	
	object_material() {
		material_name = NULL;
		ident = 0;
		hardness = 0;
		for (int i = 0; i < 7; i++)
		{
			this->qual_strings[i] = NULL;
		}
		
	}
};


class body_loc_info {
public:
	int priority_order;
	char* long_name;
	char* short_name;
	
	body_loc_info() {
		priority_order = 0;
		short_name = NULL;
		long_name = NULL;
	}
	
	~body_loc_info() {
		free_mem(short_name);
		free_mem(long_name);
	}
};

const int rev_dir[12] = {
	2,		//opposite of south(2) is north[0]
	3,	
	0,	
	1,	
	6,	
	7, 	
	4,	
	5,	
	9,	
	8,	
	11,	
	10	
};

//includes all forms of portals. saved in each room and data is based on that room as a reference point. Not saved to the database, but filled when room is loaded. Saved in a multimap by <int dir, room_exit_data> 


class room_exit_data {
public:
	char *sdesc;
	char *ldesc;
	char *fdesc;
	char *keyword;
	int portal;			//which portal is this exit based on
	int direction;		//what direction we go to use this exit
	int type;			//opening, door, gate, barrier, crossing, etc
	int port_flags;		//flags - closed, locked, pickproof, door, gate - 
	int key;			// vnum for the key
	int pick_penalty;	//penalty for lock picks
	int to_room;		//what room you go -to-
	float slope;			//angle of the exit
	int material;		//what is this portal made from
	int quality;		//how good is the quality - how hard is it to break or destroy
	int difficulty;		//how hard is it to get past barriers or crossings?
	int skill;			//skill need to get past barriers and crossings
	
	room_exit_data() {
		sdesc = NULL;
		ldesc = NULL;
		fdesc = NULL;
		keyword = NULL;
		portal = 0;
		direction = -1;
		type = 0;
		port_flags = 0;
		key = 0;
		pick_penalty = 0;
		to_room = 0;
		slope = 0;
		material = 0;
		quality = 0;
		difficulty = 0;
		skill = 0;
	}
	
};


#define TS_ROOM			1

#include "room.h"

struct room_extra_data
{
	std::string alas[LAST_DIR+1];
	std::string weather_desc[WR_LAST_DESCRIPTIONS+1];
	
	room_extra_data() 
	{
		for (int i=0; i <= LAST_DIR; i++)
		{
			alas[i].clear();
		}
		
		for(int i=0; i <= WR_LAST_DESCRIPTIONS; i++)
		{
			weather_desc[i].clear();
		}
	}
};


#include "character.h"

class prog_vars {
public:
	char *name;
	int value;
	struct prog_vars *next;

	prog_vars() {
		name = NULL;
		value = 0;
		next = NULL;
	}

	~prog_vars() {
		free_mem(name);
	}
};


	//For secret descriptions only - does not hide portals at all

class secret {
public:
	int diff;			// difficulty (search skill abil) 
	char *stext;

	secret() {
		diff = 0;
		stext = NULL;
	}

};

#define MAX_DELIVERIES			200
#define MAX_TRADES_IN			100 // DO NOT CHANGE THIS WITHOUT CHANGING THE FREAD FOR IT TO BE DYNAMIC. CURRENTLY DOES 10 ONLY - CASE

#define MAX_SHOP_MATERIALS	20 //materials that a shopkeeper use for inventory

class negotiation_data {
public:
	int ch_coldload_id;
	int obj_vnum;
	int time_when_forgotten;
	int price_delta;
	int transactions;
	int true_if_buying;
	struct negotiation_data *next;

	negotiation_data() {
		ch_coldload_id = 0;
		obj_vnum = 0;
		time_when_forgotten = 0;
		price_delta = 0;
		transactions = 0;
		true_if_buying = 0;
		next = NULL;
	}
};

class shop_data {
public:
	float markup;			/* Objects sold are multiplied by this  */
	float discount;		/* Objects bought are muliplied by this */
	int shop_vnum;		/* Rvnum of shop                                                */
	int store_vnum;		/* Rvnum of inventory                                   */
	char *no_such_item1;		/* Message if keeper hasn't got an item */
	char *no_such_item2;		/* Message if player hasn't got an item */
	char *missing_cash1;		/* Message if keeper hasn't got cash    */
	char *missing_cash2;		/* Message if player hasn't got cash    */
	char *do_not_buy;		/* If keeper dosn't buy such things.    */
	char *message_buy;		/* Message when player buys item                */
	char *message_sell;		/* Message when player sells item               */
	int delivery[MAX_DELIVERIES];	/* Merchant always sells these      */
	int trades_in[MAX_TRADES_IN];	/* item_type that can buy       */
	std::set <std::string> materials;	//materials shop will deal with
	int econ_flags1;		/* Bits which enhance price             */
	int econ_flags2;
	int econ_flags3;
	int econ_flags4;
	int econ_flags5;
	int econ_flags6;
	int econ_flags7;
	float econ_markup1;		/* Sell markup for flagged items        */
	float econ_markup2;
	float econ_markup3;
	float econ_markup4;
	float econ_markup5;
	float econ_markup6;
	float econ_markup7;
	float econ_discount1;		/* Buy markup for flagged items         */
	float econ_discount2;
	float econ_discount3;
	float econ_discount4;
	float econ_discount5;
	float econ_discount6;
	float econ_discount7;
	int buy_flags;		/* Any econ flags set here are traded */
	int nobuy_flags;		/* Any econ flags set here aren't traded */
	NEGOTIATION_DATA *negotiations;	/* Haggling information                         */
	int opening_hour;
	int closing_hour;
	float avg_price;

	shop_data() {
		markup = 0;
		discount = 0;
		buy_flags = 0;
		nobuy_flags = 0;
		opening_hour = 0;
		closing_hour = 0;
		econ_flags1 = 0;
		econ_flags2 = 0;
		econ_flags3 = 0;
		econ_flags4 = 0;
		econ_flags5 = 0;
		econ_flags6 = 0;
		econ_flags7 = 0;
		econ_markup1 = 0;
		econ_markup2 = 0;
		econ_markup3 = 0;
		econ_markup4 = 0;
		econ_markup5 = 0;
		econ_markup6 = 0;
		econ_markup7 = 0;
		econ_discount1 = 0;
		econ_discount2 = 0;
		econ_discount3 = 0;
		econ_discount4 = 0;
		econ_discount5 = 0;
		econ_discount6 = 0;
		econ_discount7 = 0;
		store_vnum = 0;
		shop_vnum = 0;
		no_such_item1 = NULL;
		no_such_item2 = NULL;
		missing_cash1 = NULL;
		missing_cash2 = NULL;
		do_not_buy = NULL;
		message_buy = NULL;
		message_sell = NULL;
		avg_price = 0.0;
		
		memset(delivery, 0, (MAX_DELIVERIES * sizeof(int)));
		memset(trades_in, 0, (MAX_TRADES_IN * sizeof(int)));
		negotiations = NULL;
	}

	~shop_data(){
		free_mem(no_such_item1);
		free_mem(no_such_item2);
		free_mem(missing_cash1);
		free_mem(missing_cash2);
		free_mem(do_not_buy);
		free_mem(message_buy);
		free_mem(message_sell);
	}

};

#define MAX_CLAN_RANKS 10

struct clan_data
{
	int id;
	char *name;		//short name
	int parent;		//id of parent clan
	char *rank[MAX_CLAN_RANKS];	//rank names
	int zone;
	char *literal;	//long name
	int member_obj;	//a single vnum for an object that makes wearer a member
	int leader_obj;	//a single vnum for an object that makes wearer a leader
	int pay_master;	//mvnum of recognized paymaster
	
	clan_data()
	{
		id = 0;
		name = NULL;
		parent = 0;
		zone = 0;
		literal = NULL;
		member_obj = 0;
		leader_obj = 0;
		pay_master = 0;
		
		for (int i = 0; i < MAX_CLAN_RANKS; i++)
			rank[i] = "none";	
		
	}
	~clan_data() {
		free_mem(name);
		free_mem(literal);
		for (int i = 0; i < MAX_CLAN_RANKS; i++)
			free_mem(rank[i]);
	}
};


#define PC_TRACK	( 1 << 0 )
#define BLOODY_TRACK	( 1 << 1 )
#define FLEE_TRACK	( 1 << 2 )

class track_data {
public:
	shortint race;
	shortint from_dir;
	shortint to_dir;
	shortint hours_passed;
	shortint speed;
	bitflag flags;
	TRACK_DATA *next;

	track_data() {
		race = RACE_DEFAULT;
		from_dir = 0;
		to_dir = 0;
		hours_passed = 0;
		speed = 0;
		flags = 0;
		next = NULL;
	}
	
};

struct reset_time_data
{
	int month;
	int day;
	int minute;
	int hour;
	int second;
	int flags;
};

struct time_info_data
{
	int hour;
	int day;
	int month;
	int year;
	int season;
	int minute;
	int holiday;
	int accum_days;
};

class memory_data {
public:
	char *name;
	struct memory_data *next;

	memory_data() {
		name = NULL;
		next = NULL;
	}
	~memory_data() {
		free_mem(name);	
	}
	
};

struct char_ability_data
{
	int str;
	int intel;
	int wil;
	int dex;
	int con;
	int aur;
	int agi;
};

class newbie_hint {
public:
	char *hint;
	NEWBIE_HINT *next;

	newbie_hint() {
		hint = NULL;
		next = NULL;
	}

	
};

class role_data {
public:
	char *summary;
	char *body;
	char *poster;
	char *date;
	int cost;
	int timestamp;
	int id;
	ROLE_DATA *next;
	
	role_data() {
		summary = NULL;
		body = NULL;
		poster = NULL;
		date = NULL;
		cost = 0;
		timestamp = 0;
		id = 0;
		next = NULL;
	}

	~role_data() {
		free_mem(summary);
		free_mem(body);
		free_mem(poster);
		free_mem(date);
		}
	
};


struct affect_attribute_type
{
	int duration;
	int intensity;
	int attrib;			//index to const char *attrs[]
	int uu4;
	int uu5;
	int uu6;
};


struct affect_spell_type
{
	int duration;			/* For how long its effects will last           */
	int modifier;			/* This is added to apropriate ability          */
	int location;			/* Tells which ability to change(APPLY_XXX)     */
	int bitvector;		/* Tells which bits to set (AFF_XXX)            */
	long t;			/* Extra information                        */
	int sn;			/* Acquired by spell number                                     */
};



struct affect_job_type
{
	int days;			//number of days in normal period
	int pay_date;		//last date paid
	int cash;			//amount to pay in normal period
	int count;
	int object_vnum;
	int employer;
};

struct affect_table_type
{
	int uu1;
	int uu2;
	int uu3;
	int uu4;
	OBJ_DATA *obj;
	int uu6;
};


struct affect_paralyze
{
	int duration;
	int minutes_until_paralyzed;
	int uu3;
	int uu4;
	int uu5;
	int sn;
};

struct affect_shadow_type
{
	CHAR_DATA *shadow;		/* target begin shadowed              */
	int edge;			/* -1, center.  0-5 edge by direction */
};

struct affect_hidden_type
{
	int duration;
	int hidden_value;
	int coldload_id;
	int uu4;
	int uu5;
	int uu6;
};


struct affect_room_type
{
	int duration;
	int intensity;
	int uu3;
	int uu4;
	int uu5;
	int uu6;
};

struct affect_craft_type
{
	int timer;
	int skill_check;
	int phase_num;
	SUBCRAFT_HEAD_DATA *subcraft;
	CHAR_DATA *target_ch;
	OBJ_DATA *target_obj;
};

struct affect_listen_type
{				/* For muting and later, directed listening */
	int duration;			/* Always on if it exists */
	int on;			/* nonzero is on */
};

/* Agent FX - let the suffering begin (soon)
enum AGENT_FORM {
AGENT_NONE=0,
AGENT_FUME,
AGENT_POWDER,
AGENT_SOLID,
AGENT_SALVE,
AGENT_LIQUID,
AGENT_SPRAY
};
enum AGENT_METHOD {
AGENT_NONE=0,
AGENT_INJECTED,
AGENT_INHALED,
AGENT_INGESTED,
AGENT_INWOUND,
AGENT_TOUCHED
};
****************************************/


union affected_union
{
	struct affect_attribute_type attr_aff;
	struct affect_spell_type spell;
	struct affect_job_type job;
	struct affect_table_type table;
	struct affect_shadow_type shadow;
	struct affect_paralyze paralyze;
	struct affect_hidden_type hidden;
	struct affect_room_type room;
	struct affect_craft_type *craft;
	struct affect_listen_type listening;
	
};

class affected_type {
public:
	int type;
	union affected_union a;
	AFFECTED_TYPE *next;

	affected_type() {
		memset(this, 0, sizeof(affected_type));
	}
};

//#ifdef HAHA
//struct affected_type
//{
//	int type;			/* The type of spell that caused this           */
//	int duration;			/* For how long its effects will last           */
//	int modifier;			/* This is added to apropriate ability          */
//	int location;			/* Tells which ability to change(APPLY_XXX)     */
//	int bitvector;		/* Tells which bits to set (AFF_XXX)            */
//	int t;			/* Extra information                        */
//	int sn;			/* Acquired by spell number                                     */
//	AFFECTED_TYPE *next;
//};
//#endif

class second_affect {
public:
	int type;
	int seconds;
	CHAR_DATA *ch;
	OBJ_DATA *obj;
	char *info;
	int info2;

	second_affect() {
		type = 0;
		seconds = 0;
		ch = NULL;
		obj = NULL;
		info = NULL;
		info2 = 0;
	}

	~second_affect() {
		free_mem(info);
		free_mem(ch);
		free_mem(obj);
	}

	bool operator== (second_affect &rhs)
	{
		if (this->type != rhs.type
			|| this->seconds != rhs.seconds
			|| this->ch != rhs.ch
			|| this->obj != rhs.obj
			|| this->info2 != rhs.info2
			|| this->info != rhs.info)
		{
			return false;
		}
		return true;
	}
};

class dream_data {
public:
	char *dream;
	DREAM_DATA *next;

	dream_data() {
		dream = NULL;
		next = NULL;
	}

	~dream_data() {
		free_mem(dream);
	}
};

class site_info {
public:
	char *name;
	char *banned_by;
	int banned_on;
	int banned_until;
	SITE_INFO *next;

	site_info() {
		name = NULL;
		banned_by = NULL;
		banned_on = 0;
		banned_until = 0;
		next = NULL;
	}

	~site_info() {
		free_mem(name);
		free_mem(banned_by);
	}
};


class threat_data {
public:
	CHAR_DATA *source;
	int level;
	THREAT_DATA *next;

	threat_data() {
		source = NULL;
		level = 0;
		next = NULL;
	}
	
	~threat_data() {
		free_mem(source);
	}
};

class attacker_data {
public:
	CHAR_DATA *attacker;
	ATTACKER_DATA *next;

	attacker_data() {
		attacker = NULL;
		next = NULL;
	}
	
	~attacker_data() {
		free_mem(attacker);
	}
	
};

struct sighted_data
{
	CHAR_DATA *target;
	SIGHTED_DATA *next;
};

/* ======================================================================== */



struct constant_data
{
	char constant_name[AVG_STRING_LENGTH];
	char description[AVG_STRING_LENGTH];
	void **index;
};


#define STR_ONE_LINE		2001
#define STR_MULTI_LINE		2000


/* Max number of race defines in table */

#define MAX_NUM_RACES		500

class race_data {
public:
	int id;
	char *name;
	bool pc_race;
	int starting_locs;
	int rpp_cost;
	char *created_by;
	int last_modified;
	int race_size;
	int body_proto;
	int innate_abilities;
	int str_mod;
	int con_mod;
	int dex_mod;
	int agi_mod;
	int int_mod;
	int wil_mod;
	int aur_mod;
	int native_tongue;
	int min_age;
	int max_age;
	int min_ht;
	int max_ht;
	int fem_ht_adj;
	RACE_TABLE_ENTRY *next;

	race_data() {
		name = NULL;
		next = NULL;
	}
	
	~race_data() {
		free_mem(name);
		free_mem(created_by);
	}

};

/* Body prototypes for wound location definitions */

#define PROTO_HUMANOID			0
#define PROTO_FOURLEGGED_PAWS		1
#define PROTO_FOURLEGGED_HOOVES		2
#define PROTO_FOURLEGGED_FEET		3
#define PROTO_WINGED_TAIL		4
#define PROTO_WINGED_NOTAIL		5
#define PROTO_SERPENTINE		6

/* Racial define innate abilities */

#define INNATE_INFRA			( 1 << 0 )
#define INNATE_FLYING			( 1 << 1 )
#define INNATE_WAT_BREATH		( 1 << 2 )
#define INNATE_NOBLEED			( 1 << 3 )
#define INNATE_SUN_PEN			( 1 << 4 )
#define INNATE_SUN_PET			( 1 << 5 )

/* Possible starting locations for race defines */

#define RACE_HOME_GONDOR		( 1 << 0 )		//not used
#define RACE_HOME_MORDOR		( 1 << 1 )     //not used
#define RACE_HOME_HARAD			( 1 << 2 )     //not used
#define RACE_HOME_CAOLAFON		( 1 << 3 )     //not used
#define RACE_HOME_MORDOR_ORC	( 1 << 4 )     //not used
#define RACE_HOME_BALCHOTH		( 1 << 5 )     //not used
#define RACE_HOME_ANGRENOST		( 1 << 6 )

/* These should match the order of the rows in the races */
/* database, since they're read in ascending order. */

#define RACE_NAME		0
#define RACE_ID			1
#define RACE_RPP_COST		2
#define RACE_DESC		3
#define RACE_START_LOC		4
#define RACE_PC			5
#define RACE_AFFECTS		6
#define RACE_BODY_PROTO		7
#define RACE_SIZE		8
#define RACE_STR_MOD		9
#define RACE_CON_MOD		10
#define RACE_DEX_MOD		11
#define RACE_AGI_MOD		12
#define RACE_INT_MOD		13
#define RACE_WIL_MOD		14
#define RACE_AUR_MOD		15
#define RACE_MIN_AGE		16
#define RACE_MAX_AGE		17
#define RACE_MIN_HEIGHT		18
#define RACE_MAX_HEIGHT		19
#define RACE_FEM_HT_DIFF	20
#define RACE_NATIVE_TONGUE	21
#define RACE_SKILL_MODS		22
#define RACE_CREATED_BY		23
#define RACE_LAST_MODIFIED	24
#define RACE_MAX_HIT		25
#define RACE_MAX_MOVE		26
#define RACE_ARMOR		27


#define FRAME_FEATHER		0
#define FRAME_SCANT		1
#define FRAME_LIGHT		2
#define FRAME_MEDIUM		3
#define FRAME_HEAVY		4
#define FRAME_MASSIVE		5
#define FRAME_SUPER_MASSIVE	6

#define SIZE_UNDEFINED		0
#define SIZE_XXS		1	/* Smaller than PC sized mobs */
#define SIZE_XS			2
#define SIZE_S			3
#define SIZE_M			4
#define SIZE_L			5
#define SIZE_XL			6
#define SIZE_XXL		7	/* Larger than PC sized mobs */


#define MAX_REGISTRY        50

typedef struct registry_data REGISTRY_DATA;

class registry_data {
public:
	int int_value;
	std::string str_value;
	REGISTRY_DATA *next;

	registry_data() {
		int_value = 0;
		next = NULL;
	}

	~registry_data() {
	}
};


class encumberance_info {
public:
	int str_mult_wt;		/* if wt <= str * str_mult, then element applies */
	int delay;
	int move;
	int adjustment;		//used in fighting
	char *encumbered_status;

	//~encumberance_info() {
	//	free_mem(encumbered_status);
	//}
};

#define NUM_BUCKETS 1024
#define ENCUMBERANCE_ENTRIES	6

/* The FLAG_ bits are saved with the player character */

#define FLAG_KEEPER			( 1 << 0 )
#define FLAG_COMPACT		( 1 << 1 )	/* Player in compact mode */
#define FLAG_BRIEF			( 1 << 2 )	/* Player in brief mode */
#define FLAG_WIZINVIS		( 1 << 3 )
#define FLAG_SUBDUEE		( 1 << 4 )
#define FLAG_SUBDUER		( 1 << 5 )
#define FLAG_SUBDUING		( 1 << 6 )
#define FLAG_ANON			( 1 << 7 )
#define FLAG_COMPETE		( 1 << 8 )
#define FLAG_LEADER_1		( 1 << 9 )	/* Clan 1 leader */
#define FLAG_LEADER_2		( 1 << 10 )	/* Clan 2 leader */
#define FLAG_DEAD			( 1 << 11 )	/* Player has been killed */
#define FLAG_KILL			( 1 << 12 )	/* Player intends to kill */
#define FLAG_FLEE			( 1 << 13 )	/* Player wants to flee combat */
#define FLAG_BINDING		( 1 << 14 )	/* NPC is curently tending wounds */
#define FLAG_SEE_NAME		( 1 << 15 )	/* Show mortal name in says */
#define FLAG_AUTOFLEE		( 1 << 16 )	/* Flee automatically in combat */
#define FLAG_ENTERING		( 1 << 17 )
#define FLAG_LEAVING		( 1 << 18 )
#define FLAG_INHIBITTED		( 1 << 19 )	/* Mob event blocking on program */
#define FLAG_NOPROMPT		( 1 << 20 )	/* Make prompt disappear */
#define FLAG_RETIRED		( 1 << 21 )	// character is retired, not the same as dead
#define FLAG_TELEPATH 		( 1 << 22 )	/* Hears PC thoughts */
#define FLAG_PACIFIST		( 1 << 23 )	/* Character won't fight back */
#define FLAG_WIZNET 		( 1 << 24 )	/* Immortal wiznet toggle */
#define FLAG_HARNESS		( 1 << 25 )	// depreciated
#define FLAG_VARIABLE		( 1 << 26 )	/* Randomized mob prototype */
#define FLAG_ISADMIN		( 1 << 27 )	/* Is an admin's mortal PC */
#define FLAG_AVAILABLE		( 1 << 28 )	/* Available for petitions */
#define FLAG_GUEST			( 1 << 29 )	/* Guest login */

/* plr_flags */

#define NEWBIE_HINTS		( 1 << 0 )	/* Toggle the hint system on or off */
#define NEWBIE				( 1 << 1 )	/* Has not yet commenced */
#define NEW_PLAYER_TAG		( 1 << 2 )	/* Displays (new player) in their ldescs */
#define MENTOR				( 1 << 3 )	/* PC Mentor flag */
#define NOPETITION			( 1 << 4 )	/* No Petition */
#define NEWBCHAT			( 1 << 5 )	/* tuned into the newb chat channel */
#define START_ANGRENOST     ( 1 << 6 )	/* Human, chose to start in ANGRENOST */
#define MUTE_BEEPS			( 1 << 7 )	/* Doesn't receive NOTIFY beeps */
#define GROUP_CLOSED		( 1 << 8 )	/* Not accepting any other followers */
#define QUIET_SCAN			( 1 << 9 ) /* quick and quiet scan when entering rooms */
#define IS_CRAFTER          ( 1 << 10)  /* required to use cset */
#define REBOOT_ACCESS       ( 1 << 11) /* Allows non L5's to use reboot */

/* char_data.guardian_flags - controls notification of PC initiated attacks */
#define GUARDIAN_PC		( 1 << 0 )	/* 01 */
#define GUARDIAN_NPC_HUMANOIDS	( 1 << 1 )	/* 02 */
#define GUARDIAN_NPC_WILDLIFE 	( 1 << 2 )	/* 04 */
#define GUARDIAN_NPC_SHOPKEEPS	( 1 << 3 )	/* 08 */
#define GUARDIAN_NPC_SENTINELS	( 1 << 4 )	/* 16 */
#define GUARDIAN_NPC_KEYHOLDER	( 1 << 5 )	/* 32 */
#define GUARDIAN_NPC_ENFORCERS	( 1 << 6 )	/* 64 */

#define PICKS_ENTITLED	6		//number of skill to pick in chargen 


#define STATE_NAME			1	/* Enter name */
#define STATE_GENDER		2	/* Choose gender */
#define STATE_RACE			3	/* Choose race */
#define STATE_AGE			4	/* Input age */
#define STATE_ATTRIBUTES	5	/* Distribute attributes */
#define STATE_SDESC			6	/* Enter short desc */
#define STATE_LDESC			7	/* Enter long desc */
#define STATE_FDESC			8	/* Enter full desc */
#define STATE_KEYWORDS		9	/* Enter keywords */
#define STATE_FRAME			10	/* Choose frame */
#define STATE_SKILLS		11	/* Skill selection */
#define STATE_COMMENT		12	/* Creation comment */
#define STATE_ROLES			13	/* Hardcoded roles/advantages */
#define STATE_SPECIAL_ROLES	14	/* Admin-posted special roles */
#define STATE_PRIVACY		15	/* Flag app private? */
#define STATE_LOCATION		16	/* Humans choose start loc */
#define STATE_PROFESSION	17	/* Choosing profession */

#define TRIG_DONT_USE	0
#define TRIG_SAY		1
#define TRIG_ENTER		2
#define TRIG_EXIT		3
#define TRIG_HIT		4
#define TRIG_MOBACT		5
#define TRIG_ALARM		6
#define TRIG_HOUR		7
#define TRIG_DAY		8
#define TRIG_TEACH		9
#define TRIG_WHISPER	10
#define TRIG_PRISONER	11
#define TRIG_KNOCK		12

#define GAME_BASE_YEAR		2455		
	//#define GAME_SECONDS_BEGINNING  823152041	/* Subtr 18000 to ++gametime 12hr */
#define GAME_SECONDS_BEGINNING  822000000
#define GAME_SECONDS_PER_YEAR	31536000	//365 days per year
#define GAME_SECONDS_PER_MONTH	2592000		//30 days per month
#define GAME_SECONDS_PER_DAY	86400		//24 hour = day
#define GAME_SECONDS_PER_HOUR	3600		//60 seconds * 60 minutes = 1 hour


#define MODE_DONE_EDITING	(1 << 1)


#define HOME				"\033[H"
#define CLEAR				"\033[2J"

	//for approval building in mysql
#define BUILD_PENDING		0
#define BUILD_APPROVED		1
#define BUILD_DECLINED		2


#define C_LV1		( 1 << 0 )	/* Immortal level 1 */
#define C_LV2		( 1 << 1 )	/* Immortal level 2 */
#define C_LV3		( 1 << 2 )	/* Immortal level 3 */
#define C_LV4		( 1 << 3 )	/* Immortal level 4 */
#define C_LV5		( 1 << 4 )	/* Immortal level 5 */
#define	C_DEL		( 1 << 5 )	/* Will not break a delay */
#define C_SUB		( 1 << 6 )	/* Commands legal while subdued */
#define C_HID		( 1 << 7 )	/* Commands that keep character hidden */
#define C_DOA		( 1 << 8 )	/* Commands allowed when dead */
#define C_BLD		( 1 << 9 )	/* Commands allowed when blind */
#define C_WLK		( 1 << 10 )	/* Commands NOT allowed while moving */
#define C_XLS		( 1 << 11 )	/* Don't list command */
#define C_PAR		( 1 << 12 )	/* Things you CAN do paralyzed */
#define C_GDE		( 1 << 13 )	/* Guide-only command */
#define C_PBU		( 1 << 14)	/* Player_builder commands */
#define C_NLG		( 1 << 15 )	/* Command is not logged */
#define C_NWT		( 1 << 16 )	/* Command doesn't show up in SNOOP or WATCH */
#define C_IMP		( 1 << 17 )	/* IMPLEMENTOR ONLY */

class command_data {
public:
	char *command;
	void (*proc) (CHAR_DATA * ch, char *argument, int cmd);
	int min_position;
	int flags;

};

class social_data {
public:
	char *social_command;
	int hide;
	int min_victim_position;	/* Position of victim */

	/* No argument was supplied */

	char *char_no_arg;
	char *others_no_arg;

	/* An argument was there, and a victim was found */

	char *char_found;		/* if NULL, read no further, ignore args */
	char *others_found;
	char *vict_found;

	/* An argument was there, but no victim was found */

	char *not_found;

	/* The victim turned out to be the character */

	char *char_auto;
	char *others_auto;

	social_data() {
		social_command = NULL;
		hide = 0;
		min_victim_position = 0;
		char_no_arg = NULL;
		others_no_arg = NULL;
		char_found = NULL;
		others_found = NULL;
		vict_found = NULL;
		not_found = NULL;
		char_auto = NULL;
		others_auto = NULL;
	}
	
	~social_data() {
		free_mem(social_command);
		free_mem(char_no_arg);
		free_mem(others_no_arg);
		free_mem(char_found);
		free_mem(others_found);
		free_mem(vict_found);
		free_mem(not_found);
		free_mem(char_auto);
		free_mem(others_auto);
	}

};

/* data files used by the game system */

#define DFLT_DIR               	"lib"	/* default data directory     */
#define GREET_FILE	  	"text/greetings"
#define MAINTENANCE_FILE	"text/greetings.maintenance"
#define MENU1_FILE	  	"text/menu1"
#define ACCT_APP_FILE		"text/account_application"
#define ACCT_EMAIL_FILE		"text/account_email"
#define ACCT_POLICIES_FILE	"text/account_policies"
#define THANK_YOU_FILE		"text/thankyou"

/***** CHARGEN ******/
#define QSTAT_FILE	 	"text/chargen/stat_message"
#define PDESC_FILE		"text/chargen/new_desc"
#define NAME_FILE		"text/chargen/new_name"
#define PKEYWORDS_FILE	"text/chargen/new_keyword"
#define PLDESC_FILE		"text/chargen/new_ldesc"
#define PSDESC_FILE		"text/chargen/new_sdesc"
#define RACE_SELECT		"text/chargen/race_select"	/* Race choice question           */
#define AGE_SELECT		"text/chargen/age_select"	/* Age choice question        */
#define SEX_SELECT		"text/chargen/sex_select"	/* Sex choice question        */
#define SPECIAL_ROLE_SELECT	"text/chargen/special_role_select"	/* Explanation of special roles */
#define HEIGHT_FRAME	"text/chargen/height_frame"
#define COMMENT_HELP	"text/chargen/comment_help"
#define LOCATION		"text/chargen/location"
#define SKILL_SELECT	"text/chargen/skills_select"
#define PROFESSION_SELECT	"text/chargen/professions"
/********* end chargen *********/

#define PLAYER_FILE     "save/players"	/* the player database        */
#define SOCMESS_FILE    "text/actions"	/* messgs for social acts     */
#define WIZLIST_FILE    "text/wizlist"	/* for WIZLIST                */
#define SAVE_DIR		"save"
#define REGIONS			"../regions"
#define REGISTRY_FILE	REGIONS "/registry"
#define CRAFTS_FILE		REGIONS "/crafts"
#define BOARD_DIR		"boards"
#define JOURNAL_DIR		"player_journals"
#define BOARD_ARCHIVE	"archive"
#define PLAYER_BOARD_DIR "player_boards"
#define VIRTUAL_BOARD_DIR	"vboards"
#define TICKET_DIR		"tickets"
#define COLDLOAD_IDS	"../lib/coldload_ids"
#define DYNAMIC_REGISTRY REGIONS "/dynamic_registry"
#define STAYPUT_FILE	"stayput"
#define LUASCRIPT_DIR		"../lib/luatest"

	//useful for limiting builders
#define MAX_ZONE 99
#define ZONE_SIZE	1000
#define OBJECT_ZONE_SIZE 1000


#define MAX_MSGS_PER_BOARD		5000
#define MAX_LANG_PER_BOARD		  10

class board_data {
public:
	char *name;
	char *title;
	int level;
	int next_virtual;
	int msg_nums[MAX_MSGS_PER_BOARD];
	char *msg_titles[MAX_MSGS_PER_BOARD];
	int language[MAX_LANG_PER_BOARD];
	BOARD_DATA *next;

	board_data() {
		name = NULL;
		title = NULL;
		level = 0;
		next_virtual = 0;
		next = NULL;
	}

	~board_data() {
		free_mem(name);
		free_mem(title);
	}
};

#define MF_READ		( 1 << 0 )
#define MF_ANON		( 1 << 1 )
#define MF_PRIVATE	( 1 << 2 )
#define MF_URGENT	( 1 << 3 )
#define MF_DREAM	( 1 << 4 )
#define MF_REPLIED	( 1 << 5 )

class mudmail_data {
public:
	long flags;
	char *from;
	char *from_account;
	char *date;
	char *subject;
	char *message;
	MUDMAIL_DATA *next_message;
	char *target;

	mudmail_data() {
		flags = 0;
		from = NULL;
		from_account = NULL;
		date = NULL;
		subject = NULL;
		message = NULL;
		next_message = NULL;
		target = NULL;
	}

	~mudmail_data() {
		free_mem(from);
		free_mem(from_account);
		free_mem(date);
		free_mem(subject);
		free_mem(message);
		free_mem(target);
	}
};
#define FORMAT_SAY		72
#define FORMAT_DESC		65
#define FORMAT_STRING	79

#define TO_ROOM		( 1 << 0 )
#define TO_VICT		( 1 << 1 )
#define TO_NOTVICT	( 1 << 2 )
#define TO_CHAR		( 1 << 3 )
#define _ACT_FORMAT	( 1 << 4 )
#define TO_IMMS		( 1 << 5 )
#define _ACT_COMBAT	( 1 << 6 )
#define TO_GROUP	( 1 << 7 )
#define _ACT_SEARCH	( 1 << 8 )

#define MAX_FATIGUE_LEVEL	9
struct fatigue_data
{
	int percent;
	char name[AVG_STRING_LENGTH];
};

struct use_table_data
{
	int delay;
};



#define STATE_REJECTED		-1
#define STATE_APPLYING		0
#define STATE_SUBMITTED		1
#define STATE_APPROVED		2
#define STATE_SUSPENDED		3
#define STATE_DIED			4
#define STATE_RETIRED		5

class move_data {
public:
	int dir;				
	int flags;
	int desired_time;
	int portal;
	MOVE_DATA *next;
	char *travel_str;

	move_data() {
		dir = 0;
		flags = 0;
		desired_time = 0;
		portal = 0;
		next = NULL;
		travel_str = NULL;
	}
	
	~move_data() {
		free_mem(travel_str);
	}

};

class qe_data {
public:
	/* Quarter second events data structure */
	CHAR_DATA *ch;
	int dir;
	int speed_type;
	int flags;
	ROOM_DATA *from_room;
	int event_time;
	int arrive_time;
	int move_cost;
	QE_DATA *next;
	char *travel_str;
	ROOM_DATA *to_room;
	int portal;
	
	qe_data() {
		ch = NULL;
		dir = 0;
		speed_type = 0;
		flags = 0;
		from_room = NULL;
		event_time = 0;
		arrive_time = 0;
		move_cost = 0;
		next = NULL;
		travel_str = NULL;
		to_room = NULL;
		portal = 0;
	}

	~qe_data() {
		free_mem(travel_str);
	}
};

#define MF_WALK			( 1 << 0 )
#define MF_RUN			( 1 << 1 )
#define MF_SWIM			( 1 << 2 )
#define MF_PASSDOOR		( 1 << 3 )
#define MF_ARRIVAL		( 1 << 4 )
#define MF_TOEDGE		( 1 << 5 )
#define MF_TONEXT_EDGE	( 1 << 6 )
#define MF_SNEAK		( 1 << 7 )

#define SPEED_WALK		0
#define SPEED_CRAWL		1
#define SPEED_PACED		2
#define SPEED_JOG		3
#define SPEED_RUN		4
#define SPEED_SPRINT	5
#define SPEED_IMMORTAL	6
#define SPEED_SWIM		7


#define CRIME_KILL			1	/* 5 hours, see criminalize() */
#define CRIME_STEAL			2	/* 3 hours */
#define CRIME_PICKLOCK		3	/* 1 hour */
#define CRIME_BACKSTAB		4	/* 5 hours */
#define CRIME_SUBDUE		5	/* 1 hour */
#define CRIME_BRAWL         6   
#define CRIME_RESIST_ARREST 7
#define CRIME_FLEE          8

struct memory_t
{
	int dtype;
	int entry;
	int bytes;
	int time_allocated;
};

/* ========================== CRAFTS ============================================== */


/* These flags apply to objects within a phase */

#define SUBCRAFT_IN_ROOM		( 1 << 0 )
#define SUBCRAFT_GIVE			( 1 << 1 )	// Give to Crafter's hands on production
#define SUBCRAFT_HELD			( 1 << 2 )
#define SUBCRAFT_USED			( 1 << 3 )
#define SUBCRAFT_PRODUCED		( 1 << 4 )
#define SUBCRAFT_FINAL_PRODUCED ( 1 << 5 )	//final product that gets optional personalizing string

/* These flags apply to mobs within a phase */

#define SUBCRAFT_MOB_IN_ROOM		( 1 << 0 )
#define SUBCRAFT_MOB_USED			( 1 << 1 )
#define SUBCRAFT_MOB_PRODUCED		( 1 << 2 )
#define SUBCRAFT_MOB_FINAL_PRODUCED ( 1 << 3 )	//final mob that gets optional personalizing string
#define SUBCRAFT_MOB_OWNED			( 1 << 4 )	//mob produced and set as owned

/* Craft Mobile flags */

#define CRAFT_MOB_SETOWNER		( 1 << 0 )

/* Subcraft flags */

#define SCF_TARGET_OBJ			( 1 << 0 )	// Target object
#define SCF_TARGET_MOB			( 1 << 1 )	// targets a mobile
#define SCF_OBSCURE    			( 1 << 1 )	// Hide from Website/newsletter 

#define MAX_DEFAULT_ITEMS		20

class default_item_data {
public:
	int flags;				// See SUBCRAFT_ flags 
	int items[MAX_DEFAULT_ITEMS];	// Up to 20 default items 
	short item_counts;		// instances of items 
	int phase_num;		// Phase mentioned 
	int key_index;			// index for key items 
	char *color;

	default_item_data();
	default_item_data(const default_item_data &RHS);

	default_item_data& operator= (const default_item_data &RHS);

	~default_item_data();
};

#define MAX_DEFAULT_MOBS	20
class default_mob_data {
public:
	int flags;				// stayput, set_owner 
	int mobs[MAX_DEFAULT_MOBS];	// Up to 20 default mobs 
	short mob_counts;		// instances of mobss 
	int phase_num;		// Phase mentioned 
	int key_index;			// index for key items 
		
	default_mob_data();
	default_mob_data(const default_mob_data &RHS);
	
	default_mob_data& operator= (const default_mob_data &RHS);
	
	~default_mob_data();
};



class phase_data {
public:
	char *first;			// First person message 
	char *third;			// Third person message 
	int phase_seconds;		// Time in seconds of phase 
	int skill;				// Only used for skill checks - skill number
	int dice;				// dice v skill 
	int sides;				// sides v skill (diceDsides v skill) 
	int hit_cost;			// Hit cost of phase 
	int move_cost;			// Move cost of phase 
	int attribute;			// Used for attribute check (like skill) 
	char *group_mess;		// message to groups followers 

	phase_data();
	phase_data(const phase_data &RHS);

	phase_data& operator= (const phase_data &RHS);

	~phase_data();
};

#define MAX_PHASES_PER_SUBCRAFT	10
#define MAX_ITEMS_PER_SUBCRAFT	20 //SOI uses 150 objects
#define MAX_MOBS_PER_SUBCRAFT	20 

#define TERRAINSMAX		25
#define SEASONSMAX		7
#define OPENINGMAX		25
#define RACEMAX			25
#define WEATHERMAX		9 

class subcraft_head_data {
public:
	char *craft_name;
	char *subcraft_name;
	char *command;
	char *personal_string;	//personalization string to be added to FINAL product
	char *clans;
	PHASE_DATA *phases[MAX_PHASES_PER_SUBCRAFT];
	DEFAULT_ITEM_DATA *obj_items[MAX_ITEMS_PER_SUBCRAFT]; //all objects in the phases
	DEFAULT_MOB_DATA *mob_items[MAX_MOBS_PER_SUBCRAFT]; //all mobs in the phases
	int subcraft_flags;
	int terrains[TERRAINSMAX];
	int seasons[SEASONSMAX];
	int opening[OPENINGMAX];
	int race[RACEMAX];
	int weather[WEATHERMAX];
	int delay;
	int key_index;
	int key_first;			// which one of the items lists is the key
	int key_end;			// which production item list will be indexed for the key
	int followers;			// number of people following the caller if the craft 

	subcraft_head_data();
	subcraft_head_data(const subcraft_head_data &RHS);

	subcraft_head_data& operator= (const subcraft_head_data &RHS);

	~subcraft_head_data();
};
/* ===================== END CRAFTS =================================================== */




	//TODO: break this down into weather zones, and enforcement zones
class zone_data {
public:

	char *name;			/* name of this zone */
	char *lead;			/* Name of the project lead */ //L4 or above staff person
	int number;		/* zone number */
	
		//need these in enforcer zones
		//int jailer;
		//int jail_room_num;		//the room number of the jail room (not the cells)
		//ROOM_DATA *jail_room;
	
		//need this in weather zones
		//int weather_type;

	zone_data() {
		name = NULL;
		lead = NULL;
		number = 0;
			//flags = 0;
			//cmd = NULL;
			//jailer = 0;
			//jail_room_num = 0;
			//jail_room = NULL;
			//weather_type = 0;
	}

	
};

class text_data {
public:
	char *filename;
	char *name;
	char *text;
	TEXT_DATA *next;
	
	text_data() {
		filename = NULL;
		name = NULL;
		text = NULL;
		next = NULL;
	}
	
		~text_data() {
			free_mem(filename);
			free_mem(name);
			free_mem(text);
		}
};


struct sphere_info
{
	char* name;
	bool available;
	
};

	//for negoitaing

#define CRIT_FAIL				0
#define MOD_FAIL				1
#define MOD_SUCCESS				2
#define CRIT_SUCCESS				3

#endif //_rpie_structs_h
