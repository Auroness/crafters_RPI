//////////////////////////////////////////////////////////////////////////////
//
/// character.h - Character Class Structures and Functions
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

#ifndef _rpie_character_h
#define _rpie_character_h

#include <map>
#include <string>
#include <sys/time.h>
#include "clan.h"

class clans {
public:
	int clan_id;
	char *clan_name;
	char *clan_rank;
	
};

class pc_data {
public:
	bitflag nanny_state;		//used in chargen to track progress
	bool role;				//do they need to be outfitted? 
	
	bool admin_loaded;
	
	char *creation_comment;
	char *imm_enter;		///< Immortals only
	char *imm_leave;
	char *site_lie;		/* Lie about connecting site */
	char *account_name;
    int account_id;
	char *msg;			/* SUBMIT/APPROVAL system */
	
	
	DREAM_DATA *dreams;
	DREAM_DATA *dreamed;
	
		// Values for staff functions
	int staff_notes;
	int mortal_mode;		/* Immortals can act mortal */
	int create_state;		/* Approval system */
	int edit_portal;	//portal ident
	int edit_obj;		//obj vnum
	int edit_mob;		//mob vnum
	CHAR_DATA *edit_player;		
	SUBCRAFT_HEAD_DATA *edit_craft;	
	int load_count;		/* See load_pc */
	
		// Stats
	int start_str;
	int start_dex;
	int start_con;
	int start_wil;
	int start_aur;
	int start_intel;
	int start_agi;
	int start_luk;
	
		// Miscellaneous variables
	int level;
	int time_last_activity;
	int is_guide;
	int profession;
	int app_cost;
	int chargen_flags;
	int last_global_pc_msg;
	
	bool RPFlag;
	std::string *plan;
	std::string *goal;
	
	OBJ_DATA *writing_on;
	
	ROLE_DATA *special_role;
	
	int crafts[MAX_CRAFTS];		//only PC can do crafts
	struct descriptor_data *owner;
	
	time_t last_logon;
	time_t last_logoff;
	time_t last_disconnect;
	time_t last_connect;
	time_t last_died;
	
	
	pc_data();
	~pc_data();
	
	void deep_copy (pc_data *copy_from);
	
};

class mob_data {
public:
	char *owner;

	// Attributes
	int damnodice;
	int damsizedice;
	int damroll;
	int min_height;		// min height for proto define
	int max_height;		// min height for proto define
	int size;			// race size for proto define

	bitflag profession;	//keeper, guard, wildlfife
	bitflag action;		//aggressive, wimpy, stealthy
	
	// Miscellaneous values
	int nVirtual;
	int zone;
	int spawnpoint;
	int merch_seven;		// set for 7 economic rules
	int vehicle_type;		// Mobile value: boat, etc 
	int access_flags;		//Flags; mob room access 
	int noaccess_flags;		//Flags; mob room noaccess 
	int currency_type;
	int jail;			//TODO: move to clan struct
	int cell_1, cell_2, cell_3;	//TODO: move to clan struct
	int circle; //depreciated value, but needed for legacy code

	SHOP_DATA *shop; 
	

		// Morph/Craft Values
	int clock;
	int morph_type;
	int morph_time;
	int morphto;
	
	mob_data();
	~mob_data();

	void deep_copy (mob_data *copy_from);
};

class char_data {

public:
	AFFECTED_TYPE *hour_affects;

	ATTACKER_DATA *attackers;

	bitflag action;		//for PLR_QUIET and PLR_STOP
	bitflag plr_flags;
	bitflag petition_flags; /* grommit - used to subscribe to zones for sphere-based petitions*/
	bitflag flags;				// FLAG_ stuff 

	bool enforcement [100]; // keeps track of zones this char is enforcer in
	bool bleeding_prompt;
	
	char *delay_who;
	char *delay_who2;
	char *keywords;		//keywords
	char *name;
	char *short_descr;
	char *long_descr;
	char *pmote_str;
	char *voice_str;
	char *description;
	char *travel_str;
	char *dmote_str;

	CHAR_DATA *subdue;		// Subduer or subduee 
	CHAR_DATA *vehicle;		// Char that is the vehicle
	CHAR_DATA *next_in_room;
	CHAR_DATA *next;
	CHAR_DATA *next_fighting;
	CHAR_DATA *next_assist;
	CHAR_DATA *following;
	CHAR_DATA *fighting;
	CHAR_DATA *delay_ch;
	


	// Attributes
	int str;
	int intel;
	int wil;
	int dex;
	int con;
	int aur;
	int agi;
	int luk;
	int tmp_str;
	int tmp_intel;
	int tmp_wil;
	int tmp_dex;
	int tmp_con;
	int tmp_aur;
	int tmp_agi;
	int tmp_luk;

	// Body values
	int balance;
	int bmi;
	int body_proto;
	int body_type;
	int height;
	int frame;	// Determines hit locations 
	int size;

	// Physical values
	int hunger;
	int thirst;
	int sex;
	int age;
	int race;
	int move_points;		// Not saved; move remainder 
	int hit_points;			// Not saved; hit remainder 
	int hit;
	int max_hit;
	int move;
	int max_move;
	int armor;
	int carry_weight;
	int carry_items;
	int lastregen;	
	int speed;
	
		// Clan variables
	std::map<std::string, std::string> char_clan_map; //clan name and rank name 
	
	// Skills variables
	char *speaks;				// Currently spoken language 
	char *writes;				// currently writes in
	int psionic_talents;
	std::map<std::string, int> skill_map; //maps skill name to skill value
	
	int craft_index; //remembers which item was used in craft key-pair

	// Miscellaneous variables
	int deleted;
	int in_room;
	int coldload_id;		// Uniq id of mob
	int alarm;				// Not saved. 
	int position;
	int default_pos;
	int delay_type;
	int delay;
	long delay_info1;
	long delay_info2;
	long delay_info3;
	long delay_info4;		//used by scan to track distance
	int intoxication;
	int last_room;
	int color;
	int assist_pos;
	int roundtime;			//seconds left in this round
	int from_dir;
	int magic_sent;			//used instead of an affect
	long affected_by;
	

	MOB_DATA *mob;
	PC_DATA *pc;
	MOVE_DATA *moves;

	OBJ_DATA *equip;		//list of things that are equipped
	OBJ_DATA *delay_obj;	//speciality hold data
	OBJ_DATA *right_hand;	//carried in the right hand
	OBJ_DATA *left_hand;	//carried in the left hand

	std::vector<BODY_LOC_INFO*> body_coverage;

	std::set<SOMA_TYPE*> soma_affect;
	
	ROOM_DATA *room;

	SIGHTED_DATA *sighted;

	
	struct descriptor_data *desc;
	struct memory_data *remembers;
	struct time_data_str time_str;

	THREAT_DATA *threats;

	unsigned short int guardian_mode;	//staff who are animating still get messages


	//******************* Methods for char_data
	
	void clear_char();
	char_data();
	~char_data();

	void deep_copy (CHAR_DATA *copy_from);
	void partial_deep_copy (CHAR_DATA *copy_from);
		
	std::pair<int, std::string> reportWhere(int RPP);
		
	bool isLevelFivePC();
	char * getLevelFiveName();
	bool hasMortalBody();
	void send_to_char(const char *message);
	void make_quiet();
	void save_char_mysql();
	void save_dreams();
	bool is_hooded();
	char * char_names();
	char * char_short();
	char * char_long();
	char * height_phrase ();
	void break_delay();
	void get_break_delay();
	void delayed_mend();
	void delayed_track();
	void delayed_alert();
	void delayed_camp1();
	void delayed_camp2();
	void delayed_camp3();
	void delayed_camp4();
	void delayed_count_coin();
	void delayed_hide();
	void delayed_hide_obj();
	void delayed_quick_scan();
	void delayed_scan();
	void delayed_scan_all();
	void delayed_pick();
	void delayed_pick_obj();
	void delayed_search();
	void delayed_study();
	void delayed_take();
	void delayed_get();	
	void delayed_remove();
	void delayed_putchar();
	
	void stop_counting();
	void act (const char *action_message, int hide_invisible, OBJ_DATA * obj, void *vict_obj, int type);
	
	void assign_hit_points();
	void apply_race_affects();
	void equip_newbie();
	void setup_new_character();
	int move_gain();
	void char_from_room();
	void char_to_room(int room);
	void extract_char();
	char * fatigue_bar();
	void clear_travel();	
	void clear_dmote();
	void clear_pmote();
	void clear_voice();
	void hunger_thirst_process();
	
	int get_trust();
	int get_position();
	void set_position(int value);
	char * get_name();
	int carrying();
	bool is_encumbered();
	bool is_awake();
	int can_carry_weight();
	bool is_subduee();
	bool is_subduer();
	int get_size();
	int get_weight();
	void clear_moves();
	int clear_current_move();
	void stop_followers();
	bool is_blind();
	bool forced_wakeup();
	bool would_reveal();
	bool can_move();
	void unload_pc();
	void pc_to_game();
	int get_queue_position();
	int get_native_tongue();
	CHAR_DATA *being_dragged();
	void clear_watch();
	void show_unread_messages();
	CHAR_DATA *is_switched();
	void add_soma(SOMA_TYPE* soma);
};
#endif
