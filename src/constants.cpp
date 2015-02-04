
//////////////////////////////////////////////////////////////////////////////
//
/// constants.cpp - Program Constant Values
//
// TODO: replace with init or text files?
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

const char* obj_dam_type[20] = {
	"none",
	"stab",
	"pierce",
	"chop",
	"blunt",
	"slash",	
	"freeze",			
	"burn",
	"tooth",
	"claw",
	"fist",		
	"blood",			
	"water",
	"lightning",
	"soiled",
	"permanent",	
	"repair",
	"stun",
	"natural",
	"\n"
};

const char* damage_severity[10] = {
	"barely noticeable",
	"miniscule",
	"small",
	"minor",
	"moderate",
	"large",
	"deep",
	"massive",
	"terrible",
	"\n"
};

const char* relative_dirs[11] = {
	"northern",
	"eastern",
	"southern",
	"western",
	"northeastern",
	"southeastern",
	"southwestern",
	"northwestern",
	"above",
	"below",
	"\n"
};

const char* dirs[11] = {
	"north",	//0
	"east",		//1
	"south",
	"west",		//3
	"northeast",
	"southeast",	//5
	"southwest",
	"northwest",	//7
	"up",
	"down",			//9
	"\n"
};



const char *verbose_dirs[11] = {
	"the north",
	"the east",
	"the south",
	"the west",
	"the northeast",
	"the southeast",
	"the southwest",
	"the northwest",
	"above",
	"below",
	"\n"
};

const char *extended_dirs[11] = {
	"to the north",
	"to the east",
	"to the south",
	"to the west",
	"to the northeast",
	"to the southeast",
	"to the southwest",
	"to the northwest",
	"up",
	"down",
	"\n"
};


const char *opposite_dirs[11] = {
	"the south",
	"the west",
	"the north",
	"the east",
	"the southwest",
	"the northwest",
	"the northeast",
	"the southeast",
	"below",
	"above",
	"\n"
};

const char *extended_opposite_dirs[11] = {
	"to the south",
	"to the west",
	"to the north",
	"to the east",
	"to the southwest",
	"to the northwest",
	"to the northeast",
	"to the southeast",
	"down",
	"up",
	"\n"
};

const char *exit_dirs[11] = {
	"North",
	"East ",
	"South",
	"West ",
	"Northeast",
	"Southeast",
	"Southwest",
	"Northwest",
	"Up   ",
	"Down ",
	"\n"
};


const char *seasons[5] = {
	"Spring",
	"Summer",
	"Autumn",
	"Winter",
	"\n"
};

const char *affected_bits[15] = {
	"Undefined",
	"Invisible",
	"Infravision",
	"Leader-command",
	"Sanctuary",
	"Group",
	"AScan",
	"ASneak",
	"AHide",
	"Follow",
	"Hooded",
	"Sunlight_Penalty",
	"Sunlight_Petrify",
	"Breathe_water",
	"\n"
};


const char *action_bits[18] = {
	"Memory",			//will remember who he is fighting
	"NoCommand",		//will not accept PC commands
	"IsNPC",
	"NoVNPC",			//no vnpc sales
	"Aggro",			//aggressively attacks
	"Wimpy",			//runs from combat
	"Pursue",			//chases after attackers and victims
	"NoOrder",			//cannot order from this merchant
	"NoBuy",			//merchant will not buy from PC
	"Stealthy",			//auto-sneaks and hides
	"PCOwned",			//can be owned and controlled by PC
	"Stayput",			// Mob saves and reloads after boot 
	"Passive",			// Mob won't assist clan brother in combat 
	"NoBind",			//will not bind it's own wounds
	"NoBleed",			//doesn't bleed
	"Flying",			//can fly
	"\n"
};

const char *profession_bits[13] = {
	"Sentinel",
	"Repair",
	"Soldier",
	"Enforcer",
	"Vehicle",
	"Criminal",
	"Pet",
	"Wildlife",			/* Mob won't attack other wildlife */
	"Jailer",
	"Physician",
	"Prey",
	"\n"
};


const char *position_types[10] = {
	"Dead",
	"Mortally wounded",
	"Unconscious",
	"Stunned",
	"Sleeping",
	"Resting",
	"Sitting",
	"Fighting",
	"Standing",
	"\n"
};

const char *connected_types[30] = {
	"Playing",
	"Entering Name",
	"Confirming Name",
	"Entering Password",
	"Entering New Password",
	"Confirming New password",
	"Choosing Gender",
	"Reading Message of the Day",
	"Main Menu",
	"Changing Password",
	"Confirming Changed Password",
	"Rolling Attributes",
	"Selecting Race",
	"Decoy Screen",
	"Creation Menu",
	"Selecting Attributes",
	"New Player Menu",
	"Documents Menu",
	"Selecting Documentation",
	"Reading Documentation",
	"Picking Skills",
	"New Player",
	"Age Select",
	"Height-Frame Select",
	"New Char Intro Msg",
	"New Char Intro Wait",
	"Creation Comment",
	"Read Reject Message",
	"Web Connection",
	"\n"
};

const char *sex_types[4] = {
	"Sexless",
	"Male",
	"Female",
	"\n"
};

const char *weather_room[23] = {
	"foggy",
	"cloudy",
	"rainy",
	"stormy",
	"snowy",
	"blizzard",
	"night",
	"night-foggy",
	"night-rainy",
	"night-stormy",
	"night-snowy",
	"night-blizzard",
	"night-cloudy",
	"spring",
	"summer",
	"autumn",
	"winter",
	"night-spring",
	"night-summer",
	"night-autumn",
	"night-winter",
	"normal",
	"\n"
};

const char *room_sizes[6] = {
	"Detail",
	"Exploration",
	"Valley",
	"Storage",
	"Other",
	"\n"
};

const char *frames[12] = {
	"fragile",
	"scant",
	"light",
	"medium",
	"heavy",
	"massive",
	"monstrous",
	"mammoth",
	"gigantic",
	"gargantuan",
	"colossal",
	"\n"
};


const char *wear_bits[25] = {
	"Take",
	"Finger",
	"Neck",
	"Body",
	"Head",
	"Legs",
	"Feet",
	"Hands",
	"Arms",
	"About",
	"Waist",
	"Wrist",
	"Belt",
	"Back",
	"Blindfold",
	"Throat",
	"Ears",
	"Shoulder",
	"Ankle",
	"Hair",
	"Face",
	"Armband",
	"\n"
};

const char *extra_bits[27] = {
	"Destroyed",
	"Invisible",
	"Magic",
	"Nodrop",
	"Get-affect",
	"Drop-affect",
	"Multi-affect",
	"Wear-affect",
	"Held-affect",
	"Hit-affect",
	"Ok",
	"item-leader",
	"item-member",
	"item-omni",
	"Illegal",
	"Restricted",
	"Mask",
	"Table",
	"Stack",
	"Variable",
	"Timer",
	"PC-Sold",
	"Thrown",
	"IsVNPC",
	"No-destroy",
	"\n"
};

const char *room_bits[25] = {
	"Dark",
	"Light",
	"Ruins",
	"NoMob",
	"NoMerchant",
	"NoHide",
	"Indoors",
	"Lawful",
	"Tunnel",
	"Unused",	//not used
	"SafeQuit",
	"Storage",
	"NoInvLim",
	"Fall",
	"Climb",
	"Vehicle",
	"Fog",
	"Road",
	"Wealthy",
	"Poor",
	"Scum",
	"Temporary",
	"OOC",
	"\n"
};

const char *terrain_types[21] = {
	"Urban",		//includes house interiors
	"Street",		//streets with house, gutters etc
	"Road",
	"Trail",
	"Field",
	"Woods",
	"Forest",
	"Hills",
	"Mountains",
	"Swamp",
	"Dock",
	"Cave",
	"Pasture",
	"Heath",
	"Pit",
	"Lake",
	"River",
	"Ocean",
	"Reef",
	"Underwater",
	"\n"
};

int movement_loss[20] = {
	1,				/* Urban */
	1,				/* Street */
	2,				/* Road */
	3,				/* Trail */
	4,				/* Field */
	5,				/* Woods */
	6,				/* Forest */
	7,				/* Hills */
	8,				/* Mountain */
	8,				/* Swamp */
	1,				/* Dock */
	4,				/* Cave */
	4,				/* Pasture */
	5,				/* Heath */
	3,				/* Pit */
	5,				/* Lake */
	15,				/* River */
	9,				/* Ocean */
	7,				/* Reef */
	10				/* Underwater */
};
const char *item_types[67] = {
	"Undefined",
	"Light",
	"Dry_container",
	"Treasure",
	"Potion",
	"Worn",
	"Other",
	"Trash",
	"Trap",
	"Container",
	"Note",
	"Liquid_container",
	"Key",
	"Food",
	"Money",
	"Ore",
	"Board",
	"Fountain",
	"Grain",
	"Pottery",
	"Plant",
	"Component",
	"Herb",
	"Salve",
	"Poison",
	"Lockpick",
	"Wind_inst",
	"Percu_inst",
	"String_inst",
	"Fur",
	"Woodcraft",
	"Spice",
	"Tool",
	"Usury_note",
	"Ticket",
	"Dye",
	"Cloth",
	"Ingot",
	"Timber",
	"Fluid",
	"Fuel",
	"Remedy",
	"Parchment",
	"Book",
	"Writing_inst",
	"Ink",
	"Keyring",
	"NPC_Object",
	"Repair",
	"Tossable",
	"MerchTicket",
	"RoomRental",
	"\n"
};

const char *sizes[9] = {
	"Sizeless",
	"XXS",
	"XS",
	"S",
	"M",
	"L",
	"XL",
	"XXL",
	"\n"
};

const char *sizes_named[9] = {
	"\01",			/* Binary 1 (^A) should be hard to enter for players */
	"XX-Small",  //1
	"X-Small",   //2
	"Small",     //3
	"Medium",    //4
	"Large",     //5
	"X-Large",   //6
	"XX-Large",  //7
	"\n"
};


const char *econ_flags[18] = {
	"magical",				
	"rare",					
	"valuable",				
	"foreign",				
	"junk",					
	"illegal",				
	"wild",					
	"poor",					
	"fine",					
	"practice",				
	"used",					
	"elvish",				
	"admin",				
	"fellowship",			
	"cooked",				
	"meat",					
	"generic",				
	"\n"
};

const char *month_name[12] = {
	"Morning_Star",
	"Suns_Dawn",
	"First_Seed",
	"Rains_Hand",
	"Second_Seed",
	"Mid_Year",
	"Suns_Height",
	"Last_Seed",
	"Hearth_Fire",
	"Frost_Fall",
	"Suns_Dusk",
	"Evening_Star"
};



const char *variable_races[17] = {
	"Breton",		//common men?
	"Imperial",		//Dunedain
	"Nords",		//common men?
	"Reachmen",		//primitive type
	"Redguard",		//Haradan
	"Altmer",		//high elves
	"Bosmer",		//hobbits
	"Dunmer",		//dark elf
	"Orc",			//big mean
	"Falmer",		//snaga
	"Argonian",		//Frogs?
	"Khajiit",		//Cat?
	"Horse",
	"Warhorse",
	"Bird",
	"Cat",
	"\n"
};

