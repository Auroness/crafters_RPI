//////////////////////////////////////////////////////////////////////////////
//
/// protos.h - Function Prototypes
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

#ifndef _rpie_protos_h_
#define _rpie_protos_h_

#include <string>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <sys/time.h>
#include </usr/local/mysql/include/mysql.h>
#include </usr/local/include/mysql++/mysql++.h>

#include <list>
#include <map>
#include <set>
#include "clan.h"
#include "net_link.h"
#include "portal.h"

#ifndef uint16
typedef unsigned short uint16;
#endif

#define MAKE_STRING(msg) (((std::ostringstream&) (std::ostringstream() << std::boolalpha << msg)).str())  
#define STR_MATCH 0		// useful to make strcmp more readable



//TODO: Portability: Change all calls to strcasecmp, 
//      And check that it is installed else use str_cmp
#define str_cmp strcasecmp	// q.v. utility. c

typedef void *malloc_t;

/* EXTERNAL DECLARATIONS */
extern int movement_loss[];
extern std::map<int, CHAR_DATA*> proto_mob_map;
extern std::map<int, OBJ_DATA*> proto_obj_map;
extern std::list<SUBCRAFT_HEAD_DATA*> crafts_list;
extern std::list<zone_data*> zone_table;
extern struct timeval time_now;
extern SITE_INFO *banned_site;
extern const int sunrise[];	// weather.c
extern const int sunset[];	// weather.c
extern const int seasonal_temp[7][12];	// weather.c
extern bool pending_reboot;
extern const struct encumberance_info enc_tab[];
extern long mud_time;
extern const int restricted_skills[];

extern float global_sun_light;
extern int moon_light[];
extern int desc_weather[];
extern int OBJECT_MAX_ZONE;

extern int count_max_online;
extern int guest_conns;
extern int new_accounts;
extern char max_online_date[AVG_STRING_LENGTH];
extern int num_texts;
extern TEXT_DATA *text_list;
extern long starttime;
extern char fatbuf[];
extern char dambuf[];
extern const char *variable_races[];	// create_mobile.c
extern const char *standard_object_colors[];	// commerce.c
extern const char *fine_object_colors[];	// commerce.c
extern const char *drab_object_colors[];	// commerce.c
extern const char *gem_colors[];	// commerce.c
extern const char *fine_gem_colors[];	// commerce.c
extern const char *sizes[];	// commerce.c
extern const char *sizes_named[];	// commmerce.c
extern const char *frames[];
extern const char *rs_name[];	// fight.c
extern const char *attack_names[];	// fight.c
extern const char *econ_flags[];	// commerce.c
extern const char *item_default_flags[];
extern const char *craft_mobile_flags[];
extern const char *subcraft_flags[];
extern const struct fatigue_data fatigue[];
extern const struct command_data commands[];
	//extern SUBCRAFT_HEAD_DATA *crafts;
extern REGISTRY_DATA *registry[];
extern CHAR_DATA *assist_queue;
extern std::list<char_data*> character_list;
extern ROLE_DATA *role_list;
extern std::list<obj_data*> object_list;
extern struct use_table_data use_table[];	// handler.c
extern const char *item_types[];
extern const char *verbal_time[];
extern const char *fullness[];
extern const char *weather_room[];
extern const char *room_sizes[];
extern struct msg_data *msg_list;
extern int season_time;
extern int port;
extern int times_do_hour_called;
extern int next_mob_coldload_id;
extern int next_pc_coldload_id;
extern int next_obj_coldload_id;
extern bool maintenance_lock;

extern struct room_exit_data *dir_options[];
extern MYSQL *database;
extern MYSQL mysql;
extern bool mysql_logging;
extern const char *position_types[];
extern const char *sex_types[];
extern const char *action_bits[];
extern const char *profession_bits[];
extern const char *affected_bits[];
extern const char *room_bits[];
extern const char *terrain_types[];	// olc.c
extern const char *seasons[];
extern const char *wear_bits[];
extern const char *extra_bits[];
extern int shutd;
extern char BOOT[];
extern int advance_hour_now;
extern struct time_info_data time_info;
extern time_t next_hour_update;
extern time_t next_minute_update;
extern int mp_dirty;
extern int maxdesc;
extern const int SPHERE_COUNT;
extern SPHERE_INFO spheres[];

struct time_info_data mud_time_passed (time_t t2, time_t t1);

/* COMMAND PROTOTYPES */
void do_accept (CHAR_DATA * ch, char *argument, int cmd);	
void do_accuse (CHAR_DATA * ch, char *argument, int cmd);	
void do_addcraft (CHAR_DATA * ch, char *argument, int cmd);	
void do_affect (CHAR_DATA * ch, char *argument, int cmd);
void do_aide (CHAR_DATA *ch, char * arg, int cmd);
void do_alert (CHAR_DATA * ch, char *argument, int cmd);
void do_alog (CHAR_DATA * ch, char *argument, int cmd);
void do_as (CHAR_DATA * ch, char *argument, int cmd);
void do_ask (CHAR_DATA * ch, char *argument, int cmd);
void do_assist (CHAR_DATA * ch, char *argument, int cmd);
void do_at (CHAR_DATA * ch, char *argument, int cmd);
void do_award (CHAR_DATA * ch, char *argument, int cmd);
void do_ban (CHAR_DATA * ch, char *argument, int cmd);
void do_barter (CHAR_DATA * ch, char *argument, int cmd);	
void do_becho (CHAR_DATA * ch, char *argument, int cmd);
void do_behead (CHAR_DATA * ch, char *argument, int cmd);
void do_blindfold (CHAR_DATA * ch, char *argument, int cmd);	
void do_blog (CHAR_DATA * ch, char *argument, int cmd);	
void do_brief (CHAR_DATA * ch, char *argument, int cmd);
void do_broadcast (CHAR_DATA * ch, char *argument, int cmd);
void do_buy (CHAR_DATA * ch, char *argument, int cmd);	
void do_camp (CHAR_DATA * ch, char *argument, int cmd);
void do_castout (CHAR_DATA * ch, char *argument, int cmd);
void do_choke (CHAR_DATA * ch, char *argument, int cmd);
void do_clockin (CHAR_DATA * ch, char *argument, int cmd);
void do_clockout (CHAR_DATA * ch, char *argument, int cmd);
void do_clog (CHAR_DATA * ch, char *argument, int cmd);
void do_close (CHAR_DATA * ch, char *argument, int cmd);
void do_command (CHAR_DATA * ch, char *argument, int cmd);
void do_commands (CHAR_DATA * ch, char *argument, int cmd);
void do_commence (CHAR_DATA * ch, char *argument, int cmd);
void do_compact (CHAR_DATA * ch, char *argument, int cmd);
void do_compare (CHAR_DATA * ch, char *argument, int cmd);
void do_contents (CHAR_DATA * ch, char *argument, int cmd);
void do_coords(CHAR_DATA * ch, char *argument, int cmd);
void do_count (CHAR_DATA * ch, char *argument, int cmd);
void do_crafts (CHAR_DATA * ch, char *argument, int cmd);
void do_craftspc (CHAR_DATA * ch, char *argument, int cmd);
void do_credits (CHAR_DATA * ch, char *argument, int cmd);
void do_cset (CHAR_DATA * ch, char *argument, int cmd);
void do_csv (CHAR_DATA* ch, char *argument, int cmd); 
void do_day (CHAR_DATA * ch, char *argument, int cmd);
void do_debug (CHAR_DATA * ch, char *argument, int cmd);
void do_decline (CHAR_DATA * ch, char *argument, int cmd);
void do_deduct (CHAR_DATA * ch, char *argument, int cmd);
void do_dip (CHAR_DATA * ch, char *argument, int cmd);
void do_disband (CHAR_DATA * ch, char *argument, int cmd);
void do_dismantle (CHAR_DATA * ch, char *argument, int cmd);
void do_dmote (CHAR_DATA * ch, char *argument, int cmd);
void do_document (CHAR_DATA * ch, char *argument, int cmd);
void do_down (CHAR_DATA * ch, char *argument, int cmd);
void do_drag (CHAR_DATA * ch, char *argument, int cmd);
void do_dreams (CHAR_DATA * ch, char *argument, int cmd);
void do_drink (CHAR_DATA * ch, char *argument, int cmd);	
void do_drop (CHAR_DATA * ch, char *argument, int cmd);	
void do_east (CHAR_DATA * ch, char *argument, int cmd);
void do_eat (CHAR_DATA * ch, char *argument, int cmd);	
void do_echo (CHAR_DATA * ch, char *argument, int cmd);
void do_email (CHAR_DATA * ch, char *argument, int cmd);
void do_emote (CHAR_DATA * ch, char *argument, int cmd);
void do_empty (CHAR_DATA * ch, char *argument, int cmd);	
void do_enter (CHAR_DATA * ch, char *argument, int cmd);
void do_equipment (CHAR_DATA * ch, char *argument, int cmd);
void do_erase (CHAR_DATA * ch, char *argument, int cmd);
void do_escape (CHAR_DATA * ch, char *argument, int cmd);
void do_evaluate (CHAR_DATA *ch, char *argument, int cmd);
void do_examine (CHAR_DATA * ch, char *argument, int cmd);
void do_exits (CHAR_DATA * ch, char *argument, int cmd);
void do_fill (CHAR_DATA * ch, char *argument, int cmd);	
void do_find (CHAR_DATA * ch, char *argument, int cmd);
void do_fire (CHAR_DATA * ch, char *argument, int cmd);
void do_fivenet (CHAR_DATA * ch, char *argument, int cmd);
void do_flee (CHAR_DATA * ch, char *argument, int cmd);
void do_flip (CHAR_DATA * ch, char *argument, int cmd);
void do_force (CHAR_DATA * ch, char *argument, int cmd);
void do_gecho (CHAR_DATA * ch, char *argument, int cmd);
void do_get (CHAR_DATA * ch, char *argument, int cmd);	
void do_give (CHAR_DATA * ch, char *argument, int cmd);	
void do_givedream (CHAR_DATA * ch, char *argument, int cmd);
void do_goto (CHAR_DATA * ch, char *argument, int cmd);
void do_gstat (CHAR_DATA * ch, char *argument, int cmd);
void do_help (CHAR_DATA * ch, char *argument, int cmd);
void do_hide (CHAR_DATA * ch, char *argument, int cmd);
void do_history (CHAR_DATA * ch, char *argument, int cmd);
void do_hood (CHAR_DATA * ch, char *argument, int cmd);
void do_hour (CHAR_DATA * ch, char *argument, int cmd);
void do_ic (CHAR_DATA * ch, char *argument, int cmd);
void do_ichat (CHAR_DATA * ch, char *argument, int cmd);
void do_immcommands (CHAR_DATA * ch, char *argument, int cmd);
void do_immtell (CHAR_DATA * ch, char *argument, int cmd);
void do_inventory (CHAR_DATA * ch, char *argument, int cmd);
void do_invis (CHAR_DATA * ch, char *argument, int cmd);
void do_invite (CHAR_DATA * ch, char *argument, int cmd);
void do_jerase (CHAR_DATA * ch, char *argument, int cmd);
void do_job (CHAR_DATA * ch, char *argument, int cmd);
void do_journal (CHAR_DATA * ch, char *argument, int cmd);
void do_jread (CHAR_DATA * ch, char *argument, int cmd);
void do_junk (CHAR_DATA * ch, char *argument, int cmd);
void do_jwrite (CHAR_DATA * ch, char *argument, int cmd);
void do_knock (CHAR_DATA * ch, char *argument, int cmd);
void do_last (CHAR_DATA * ch, char *argument, int cmd);
void do_leave (CHAR_DATA * ch, char *argument, int cmd);
void do_leaveprivate(CHAR_DATA* ch, char* argument, int cmd);
void do_light (CHAR_DATA * ch, char *argument, int cmd);	
void do_list (CHAR_DATA * ch, char *argument, int cmd);	
void do_load (CHAR_DATA * ch, char *argument, int cmd);
void do_locate (CHAR_DATA * ch, char *argument, int cmd);
void do_lock (CHAR_DATA * ch, char *argument, int cmd);
void do_look (CHAR_DATA * ch, char *argument, int cmd);
void do_makeprivate (CHAR_DATA *ch, char *argument, int cmd);
void do_map (CHAR_DATA * ch, char *argument, int cmd);
void do_mark (CHAR_DATA* ch, char *argument, int cmd); 
void do_materials (CHAR_DATA * ch, char *argument, int cmd);
void do_mcopy (CHAR_DATA * ch, char *argument, int cmd);
void do_mend (CHAR_DATA * ch, char *argument, int cmd);	
void do_mlist (CHAR_DATA *ch, char *argument, int cmd);  
void do_move (CHAR_DATA * ch, char *argument, int cmd);
void do_mset (CHAR_DATA * ch, char *argument, int cmd);
void do_mute (CHAR_DATA * ch, char *argument, int cmd);
void do_mysql (CHAR_DATA * ch, char *argument, int cmd);
void do_name (CHAR_DATA *ch, char *argument, int cmd);
void do_newbchat (CHAR_DATA * ch, char *argument, int cmd);
void do_news (CHAR_DATA * ch, char *argument, int cmd);
void do_nod (CHAR_DATA * ch, char *argument, int cmd);
void do_nokill (CHAR_DATA * ch, char *argument, int cmd);
void do_north (CHAR_DATA * ch, char *argument, int cmd);
void do_northeast (CHAR_DATA * ch, char *argument, int cmd);
void do_northwest (CHAR_DATA * ch, char *argument, int cmd);
void do_notes (CHAR_DATA * ch, char *argument, int cmd);
void do_olist (CHAR_DATA * ch, char *argument, int cmd);
void do_omote (CHAR_DATA * ch, char *argument, int cmd);
void do_ooc (CHAR_DATA * ch, char *argument, int cmd);
void do_open (CHAR_DATA * ch, char *argument, int cmd);
void do_openskill (CHAR_DATA * ch, char *argument, int cmd);
void do_origins (CHAR_DATA * ch, char *argument, int cmd);
void do_oset (CHAR_DATA * ch, char *argument, int cmd);
void do_outfit (CHAR_DATA * ch, char *argument, int cmd);
void do_ownership (CHAR_DATA *ch, char *argument, int cmd);
void do_palm (CHAR_DATA * ch, char *argument, int cmd);
void do_pardon (CHAR_DATA * ch, char *argument, int cmd);
void do_passwd (CHAR_DATA * ch, char *argument, int cmd);
void do_payday (CHAR_DATA * ch, char *argument, int cmd);
void do_payroll (CHAR_DATA * ch, char *argument, int cmd);
void do_pecho (CHAR_DATA * ch, char *argument, int cmd);
void do_petition (CHAR_DATA * ch, char *argument, int cmd);
void do_pfile (CHAR_DATA * ch, char *argument, int cmd);
void do_pick (CHAR_DATA * ch, char *argument, int cmd);
void do_plan (CHAR_DATA * ch, char *argument, int cmd);
void do_plist (CHAR_DATA * ch, char *argument, int cmd);
void do_plog (CHAR_DATA * ch, char *argument, int cmd);
void do_pmote (CHAR_DATA * ch, char *argument, int cmd);
void do_point (CHAR_DATA * ch, char *argument, int cmd);
void do_pour (CHAR_DATA * ch, char *argument, int cmd);	
void do_preview (CHAR_DATA * ch, char *argument, int cmd);
void do_professions (CHAR_DATA * ch, char *argument, int cmd);
void do_promote (CHAR_DATA * ch, char *argument, int cmd);
void do_purge (CHAR_DATA * ch, char *argument, int cmd);
void do_put (CHAR_DATA * ch, char *argument, int cmd);	
void do_qscan (CHAR_DATA * ch, char *argument, int cmd);
void do_quit (CHAR_DATA * ch, char *argument, int cmd);
void do_read (CHAR_DATA * ch, char *argument, int cmd);
void do_receipts (CHAR_DATA * ch, char *argument, int cmd);	
void do_recruit (CHAR_DATA * ch, char *argument, int cmd);
void do_refresh (CHAR_DATA * ch, char *argument, int cmd);
void do_register (CHAR_DATA * ch, char *argument, int cmd);
void do_release (CHAR_DATA * ch, char *argument, int cmd);
void do_remcraft (CHAR_DATA * ch, char *argument, int cmd);
void do_remove (CHAR_DATA * ch, char *argument, int cmd);
void do_rend (CHAR_DATA * ch, char *argument, int cmd);
void do_report (CHAR_DATA * ch, char *argument, int cmd);
void do_rest (CHAR_DATA * ch, char *argument, int cmd);
void do_restore (CHAR_DATA * ch, char *argument, int cmd);
void do_retreat (CHAR_DATA * ch, char *argument, int cmd);
void do_return (CHAR_DATA * ch, char *argument, int cmd);
void do_review (CHAR_DATA * ch, char *argument, int cmd);
void do_rlist (CHAR_DATA * ch, char *argument, int cmd);
void do_role (CHAR_DATA * ch, char *argument, int cmd);
void do_roll (CHAR_DATA * ch, char *argument, int cmd);
void do_roster (CHAR_DATA * ch, char *argument, int cmd);
void do_rset (CHAR_DATA * ch, char *argument, int cmd);
void do_save (CHAR_DATA * ch, char *argument, int cmd);
void do_saverooms (CHAR_DATA * ch, char *argument, int cmd);
void do_say (CHAR_DATA * ch, char *argument, int cmd);
void do_scan (CHAR_DATA * ch, char *argument, int cmd);
void do_scommand (CHAR_DATA *ch, char *argument, int cmd);
void do_score (CHAR_DATA * ch, char *argument, int cmd);
void do_scribe (CHAR_DATA * ch, char *argument, int cmd);
void do_search (CHAR_DATA * ch, char *argument, int cmd);
void do_select_script (CHAR_DATA * ch, char *argument, int cmd);
void do_sell (CHAR_DATA * ch, char *argument, int cmd);	
void do_send (CHAR_DATA * ch, char *argument, int cmd);
void do_sense (CHAR_DATA * ch, char *argument, int cmd);
void do_set (CHAR_DATA * ch, char *argument, int cmd);
void do_shadow (CHAR_DATA * ch, char *argument, int cmd);
void do_shout (CHAR_DATA * ch, char *argument, int cmd);
void do_show (CHAR_DATA * ch, char *argument, int cmd);
void do_shutdown (CHAR_DATA * ch, char *argument, int cmd);
void do_sing (CHAR_DATA * ch, char *argument, int cmd);
void do_sit (CHAR_DATA * ch, char *argument, int cmd);
void do_skills (CHAR_DATA * ch, char *argument, int cmd);
void do_sleep (CHAR_DATA * ch, char *argument, int cmd);
void do_sneak (CHAR_DATA * ch, char *argument, int cmd);
void do_snoop (CHAR_DATA * ch, char *argument, int cmd);
void do_south (CHAR_DATA * ch, char *argument, int cmd);
void do_southeast (CHAR_DATA * ch, char *argument, int cmd);
void do_southwest (CHAR_DATA * ch, char *argument, int cmd);
void do_speak (CHAR_DATA * ch, char *argument, int cmd);
void do_stand (CHAR_DATA * ch, char *argument, int cmd);
void do_stat (CHAR_DATA * ch, char *argument, int cmd);
void do_stayput (CHAR_DATA * ch, char *argument, int cmd);
void do_steal (CHAR_DATA * ch, char *argument, int cmd);
void do_stop (CHAR_DATA * ch, char *argument, int cmd);
void do_strike (CHAR_DATA * ch, char *argument, int cmd);
void do_study (CHAR_DATA * ch, char *argument, int cmd);
void do_subscribe (CHAR_DATA * ch, char *argument, int cmd);
void do_subdue (CHAR_DATA * ch, char *argument, int cmd);
void do_summon (CHAR_DATA * ch, char *argument, int cmd);
void do_surrender (CHAR_DATA * ch, char *argument, int cmd);
void do_swap (CHAR_DATA * ch, char *argument, int cmd);
void do_switch (CHAR_DATA * ch, char *argument, int cmd);
void do_tables (CHAR_DATA * ch, char *argument, int cmd);
void do_tags (CHAR_DATA * ch, char *argument, int cmd);
void do_take (CHAR_DATA * ch, char *argument, int cmd);
void do_talk (CHAR_DATA * ch, char *argument, int cmd);
void do_tally (CHAR_DATA * ch, char *argument, int cmd);	
void do_teach (CHAR_DATA * ch, char *argument, int cmd);
void do_tear (CHAR_DATA * ch, char *argument, int cmd);
void do_tell (CHAR_DATA * ch, char *argument, int cmd);
void do_think (CHAR_DATA * ch, char *argument, int cmd);
void do_time (CHAR_DATA * ch, char *argument, int cmd);
void do_title (CHAR_DATA * ch, char *argument, int cmd);
void do_track (CHAR_DATA * ch, char *argument, int cmd);
void do_transfer (CHAR_DATA * ch, char *argument, int cmd);
void do_travel (CHAR_DATA * ch, char *argument, int cmd);	
void do_typo (CHAR_DATA * ch, char *argument, int cmd);
void do_unban (CHAR_DATA * ch, char *argument, int cmd);
void do_unlock (CHAR_DATA * ch, char *argument, int cmd);
void do_unsubscribe (CHAR_DATA * ch, char *argument, int cmd);
void do_up (CHAR_DATA * ch, char *argument, int cmd);
void do_users (CHAR_DATA * ch, char *argument, int cmd);
void do_value (CHAR_DATA * ch, char *argument, int cmd);
void do_variant (CHAR_DATA * ch, char *argument, int cmd);
void do_vboards (CHAR_DATA * ch, char *argument, int cmd);
void do_vis (CHAR_DATA * ch, char *argument, int cmd);
void do_voice (CHAR_DATA * ch, char *argument, int cmd);
void do_wake (CHAR_DATA * ch, char *argument, int cmd);
void do_wanted (CHAR_DATA * ch, char *argument, int cmd);
void do_watch (CHAR_DATA * ch, char *arguemnt, int cmd);
void do_wclone (CHAR_DATA * ch, char *argument, int cmd);
void do_wear (CHAR_DATA * ch, char *argument, int cmd);	
void do_wearloc (CHAR_DATA * ch, char *argument, int cmd);
void do_weather (CHAR_DATA * ch, char *argument, int cmd);
void do_west (CHAR_DATA * ch, char *argument, int cmd);
void do_where (CHAR_DATA * ch, char *argument, int cmd);
void do_whisper (CHAR_DATA * ch, char *argument, int cmd);
void do_who (CHAR_DATA * ch, char *argument, int cmd);
void do_wizlist (CHAR_DATA * ch, char *argument, int cmd);
void do_wizlock (CHAR_DATA * ch, char *argument, int cmd);
void do_wlog (CHAR_DATA * ch, char *argument, int cmd);
void do_wmotd(CHAR_DATA * ch, char *argument, int cmd);
void do_would (CHAR_DATA * ch, char *argument, int cmd);
void do_write (CHAR_DATA * ch, char *argument, int cmd);
void do_write_book (CHAR_DATA * ch, char *argument, int cmd);
void do_writings (CHAR_DATA * ch, char *argument, int cmd);
void do_zecho (CHAR_DATA * ch, char *argument, int cmd);
void do_zset (CHAR_DATA * ch, char *argument, int cmd);

void char__do_bind (CHAR_DATA * ch, char *argument, int cmd);	
void char__do_toss (CHAR_DATA * ch, char *argument, int cmd);
void move (CHAR_DATA * ch, char *argument, int dir, int speed, int portal);

/*************************
 //  Clan methods
 */

void clan_rem_obj (OBJ_DATA *obj, OBJ_CLAN_DATA * targ);
void update_enforcement_array (CHAR_DATA * ch);
int is_area_enforcer (CHAR_DATA * ch);
void load_clan_ranks_mysql (CLAN_DATA *tclan);
int lookup_clan_id (char *clan_name);

/*************************
//  Communication methods
*/
int decipher_speaking (CHAR_DATA * ch, char *skillname, int skill);
const char *accent_desc (CHAR_DATA * ch, int skill);
void personalize_emote (CHAR_DATA *ch, char *emote);
bool evaluate_emote_string (CHAR_DATA *ch, std::string * first_person, std::string third_person, std::string argument);
void reformat_say_string (char *source, char **target, CHAR_DATA * to);
void reformat_string (char *source, char **target);
std::string reformat_desc (char *source);
char *tilde_eliminator (char *string);

void send_to_gods (const char *messg);
void send_to_guides (char *message);
void send_to_imps (char *messg);
void send_to_guardians (char *messg, unsigned short int flag);
void send_outside (char *message);
void send_outside_zone (char *message, int zone);
void send_to_all (char *message);
void send_to_all_unf (char *message);
void send_to_not_char (const char *message, const CHAR_DATA *ch);
void send_to_room (char *message, int room_num);
void send_to_room_unf (char *message, int room_num);

void post_dream (struct descriptor_data *d);
void post_track_response (struct descriptor_data *d);
void post_to_mysql_virtual_board (struct descriptor_data *d);
void post_to_mysql_player_board (struct descriptor_data *d);
void post_to_mysql_journal (struct descriptor_data *d);
void post_to_mysql_board (struct descriptor_data *d);
/*************************
 // Craft Related methods
 */

void list_all_crafts (CHAR_DATA * ch);
void display_craft (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft);
void display_spec_craft (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA *craft); 
char *origins_list(CHAR_DATA * ch, OBJ_DATA * obj);
int craft_uses (SUBCRAFT_HEAD_DATA * craft, int vnum);
int craft_produces (SUBCRAFT_HEAD_DATA * craft, int vnum);
void missing_item_msg (CHAR_DATA * ch, DEFAULT_ITEM_DATA * item, char *header);
void missing_mob_msg (CHAR_DATA * ch, DEFAULT_MOB_DATA * item, char *header);
void craft_clan (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_delay (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_delete (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_key (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_key_product (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_opening (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_race (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_seasons (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_terrains (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_setup (CHAR_DATA * ch, char *argument, char *subcmd);
void craft_weather (CHAR_DATA * ch, char *argument, char *subcmd);
OBJ_DATA * get_key_end_obj (CHAR_DATA * ch, DEFAULT_ITEM_DATA * item, PHASE_DATA * phase, int index);
OBJ_DATA * get_key_start_obj (CHAR_DATA * ch, DEFAULT_ITEM_DATA * item, PHASE_DATA * phase, int index);
void craft_group (CHAR_DATA * ch, char *argument, char *subcmd);
void craftstat (CHAR_DATA * ch, char *argument);
void spec_craftstat (CHAR_DATA * ch, char *argument);
void update_crafts (CHAR_DATA * ch);
void craft_prepare_message (CHAR_DATA * ch, char *message, CHAR_DATA * n,
							CHAR_DATA * N, CHAR_DATA * T, char *phase_msg,
							OBJ_DATA ** obj_list, CHAR_DATA ** mob_list);
void craft_command (CHAR_DATA * ch, char *command_args,
					AFFECTED_TYPE * craft_affect);
AFFECTED_TYPE *is_craft_command (CHAR_DATA * ch, char *argument);
void activate_phase (CHAR_DATA * ch, AFFECTED_TYPE * af);
CHAR_DATA *get_item_mob (CHAR_DATA * ch, DEFAULT_MOB_DATA * mob, int phasenum);
OBJ_DATA *get_item_obj (CHAR_DATA * ch, DEFAULT_ITEM_DATA * item, int phasenum);
CHAR_DATA * mob_list_vnum (CHAR_DATA * ch, CHAR_DATA * mlist, int vnum);
void craft_new(CHAR_DATA * ch, char *argument);
void craft_save(CHAR_DATA * ch);
void craft_phases (CHAR_DATA * ch, char *argument, char *subcmd);
/************************
// Informational methods
*/
void post_motd (DESCRIPTOR_DATA * d);
void read_motd(DESCRIPTOR_DATA * d);
std::string gatheringPlace(int room_num, std::string name);
std::string gatheringPlaceCore(int room_num, std::string name, bool colorise);
int get_stat_range (int score);
void combine_money_inv (OBJ_DATA * source, CHAR_DATA * ch);
void combine_money_obj (OBJ_DATA * source, OBJ_DATA * container,
						CHAR_DATA * ch);

char *read_ticket (CHAR_DATA * ch, int tick_num, int cmd);
void search_ticket (CHAR_DATA * ch, char * chkvalue, int searchtype);
void delete_ticket (CHAR_DATA * ch, int tick_num);


void look_direction(CHAR_DATA *ch, char *arg1);
void look_gl_window(CHAR_DATA *ch);
void look_in_container (CHAR_DATA *ch, char *argument);
void look_room(CHAR_DATA *ch, int cmd);
void look_room_weather (CHAR_DATA *ch, int cmd);

/*****************
 // Mobile methods
 */
	//shopkeepers-Mobile
int keeper_makes (CHAR_DATA * keeper, int ovnum);	
void money_to_storeroom (CHAR_DATA * keeper, int amount);	
void subtract_keeper_money (CHAR_DATA * keeper, int cost);
int keeper_uses_currency_type (int currency_type, OBJ_DATA * obj);	
int keeper_has_money (CHAR_DATA * keeper, int cost);	
int trades_in (CHAR_DATA * keeper, OBJ_DATA * obj);
int keeper_has_item (CHAR_DATA * keeper, int ovnum);
float calculate_sale_price (OBJ_DATA * obj, CHAR_DATA * keeper,
							CHAR_DATA * ch, int quantity, bool round_result,
							bool sell);
void give_change(int money, int currency_type, ROOM_DATA *store, OBJ_DATA *tobj, CHAR_DATA *ch);

OBJ_DATA *get_obj_in_list_vis_not_money (CHAR_DATA * ch, char *name,
										 OBJ_DATA * list);

	//misc-Mobile
void list_char_to_char (CHAR_DATA * list, CHAR_DATA * ch);
int craft_mob_produces (SUBCRAFT_HEAD_DATA * craft, int vnum);
int craft_mob_uses (SUBCRAFT_HEAD_DATA * craft, int vnum);
void morph_mob (CHAR_DATA * ch);
int mob_weather_reaction (CHAR_DATA * ch);
int calculate_race_height (CHAR_DATA * tch);
int calculate_size_height (CHAR_DATA * tch);
void make_height (CHAR_DATA * mob);
void make_frame (CHAR_DATA * mob);

CHAR_DATA *vtom (int nVirtual);
void randomize_mobile (CHAR_DATA * mob);
CHAR_DATA *load_mobile (int nVirtual);
void post_mprog (struct descriptor_data *d);
void table_add_affect (CHAR_DATA * ch, OBJ_DATA * obj, int type);
void import_mobs(void);
int translate_mob_flag(int tflag);
int can_see_mob(CHAR_DATA * ch, CHAR_DATA * tmob);
void save_mysql_stayput_mob (CHAR_DATA * tmob); 
CHAR_DATA *load_stayput_mobile_mysql (int mob_num, int coldload);
/************************
 // Mysql related methods
 */

void mysql_secure_query (MYSQL * conn, char *query, int length);
void retrieve_mysql_board_listing (CHAR_DATA * ch, char *board_name);
void display_mysql_board_message (CHAR_DATA * ch, char *board_name,
								  int msg_num, bool bHideHeader);
MYSQL_RES *mysql_player_search (int search_type, char *string, int timeframe);
int mysql_safe_query (char *fmt, ...);
CHAR_DATA *load_char_mysql (char *name);
CHAR_DATA *load_char_mysqlpp (char *name);
void save_mysql_mob (CHAR_DATA *builder, CHAR_DATA * tmob, int approved); 
void load_mobile_mysql (int mob_num);
void save_mysql_object (CHAR_DATA *builder, OBJ_DATA * tobj, int edit_approval);
void load_object_mysql (int obj_num);
void save_mysql_room (CHAR_DATA *builder, ROOM_DATA * troom, int edit_approval);
void load_room_mysql (int room_num);
void load_portals_mysql (int port_num);
void save_mysql_portal (CHAR_DATA *builder, ROOM_PORTAL_DATA * tport, int edit_approval);
void save_mysql_crafts(CHAR_DATA *builder, SUBCRAFT_HEAD_DATA *craft, int edit_approval);
void save_mysql_phase(SUBCRAFT_HEAD_DATA *craft, PHASE_DATA *phase, int phase_num);
void save_mysql_craft_element(SUBCRAFT_HEAD_DATA *craft, DEFAULT_ITEM_DATA *obj_item, DEFAULT_MOB_DATA *mob_item, char* elem_type, int elem_numb);
void load_subcraft_mysql (int craft_id);
void load_phases_mysql(int craft_id, SUBCRAFT_HEAD_DATA *craft);
void load_elements_mysql(int craft_id, SUBCRAFT_HEAD_DATA *craft);
void load_materials(void);
void load_variant_values(void);
void builder_log (CHAR_DATA * ch, char *command, char *str);
void init_body_loc(CHAR_DATA *ch);
void skill_record (CHAR_DATA* ch, char* skill_name, int timer_length);
/*****************
// Object methods
*/
	//Currency and Commerce-Object

void currency_print_line( const OBJ_DATA* obj, int* money, char* buffer, const char* wear_loc );
char* coin_sdesc(const OBJ_DATA * coin);
float tally (OBJ_DATA * obj, char *buffer, int depth); 
int can_subtract_money (CHAR_DATA * ch, int coppers_to_subtract, int currency_type);
void subtract_money (CHAR_DATA * ch, int coppers_to_subtract, int currency_type);
void keeper_money_to_char (CHAR_DATA * keeper, CHAR_DATA * ch, int money);

float econ_markup (CHAR_DATA * keeper, OBJ_DATA * obj);
float econ_discount (CHAR_DATA * keeper, OBJ_DATA * obj);

	//Repair-Object
void begin_repair (CHAR_DATA * ch, OBJ_DATA *obj, OBJ_DATA *kit, int mode);
void npc_repair (CHAR_DATA * ch, CHAR_DATA * mob, OBJ_DATA *obj, char *argument);

int skill_mend(CHAR_DATA * ch, OBJ_DATA *kit, OBJECT_DAMAGE * damage);

	//Damage-Object
void object__drench (CHAR_DATA * ch, OBJ_DATA * _obj, bool isChEquip);	
char *object__examine_damage (OBJ_DATA * thisPtr, CHAR_DATA * ch);	
OBJECT_DAMAGE *object__add_damage (OBJ_DATA * thisPtr, int source, unsigned int impact, CHAR_DATA * ch, int flag);	
OBJECT_DAMAGE * damage_from_obj (OBJ_DATA * obj, OBJECT_DAMAGE * damage);
OBJECT_DAMAGE *object_damage__new ();
OBJECT_DAMAGE *object_damage__new_init (int source, ushort impact, int mtype, int hardness,  int maxhit);
char *object_damage__get_sdesc (OBJECT_DAMAGE * thisPtr);
int object_damage__write_to_file (OBJECT_DAMAGE * thisPtr, FILE * fp);
OBJECT_DAMAGE *object_damage__read_from_file (FILE * fp);
int consolidate_damage(OBJ_DATA* thisobj, CHAR_DATA* ch);

	//Building objects

void obj_type (CHAR_DATA* ch, char*argument);
void obj_wear (CHAR_DATA* ch, char* argument);
void obj_timer (CHAR_DATA* ch, char* argument);
void obj_extra (CHAR_DATA* ch, char* argument);
void obj_affect (CHAR_DATA* ch, char* argument);
void obj_delete (CHAR_DATA* ch, char* argument);
void obj_material (CHAR_DATA* ch, char* argument);
void obj_wtype (CHAR_DATA* ch, char* argument);

	//Misc-Object
void refresh_colors (CHAR_DATA * keeper);
int redefine_objects (OBJ_DATA * proto);
void clear_omote (OBJ_DATA * obj);	
OBJ_DATA *get_obj_in_list (char *name, OBJ_DATA * list);
OBJ_DATA *get_obj_in_list_num (int num, OBJ_DATA * list);
OBJ_DATA *get_obj (char *name);
OBJ_DATA *get_obj_in_list_vis (CHAR_DATA * ch, const char *name, OBJ_DATA * list);
OBJ_DATA *get_obj_vis (CHAR_DATA * ch, char *name);
OBJ_DATA *get_object_in_equip_vis (CHAR_DATA * ch, char *arg,
								   OBJ_DATA * equipment[], int *j);

OBJ_DATA *load_object (int nVirtual);
OBJ_DATA *load_object_full (int vnum, bool newWritingID, int l_count);


void read_obj_suppliment (struct char_data *ch, FILE * fp);
void write_obj_suppliment (struct char_data *ch, FILE * fp);
void obj_to_room (OBJ_DATA * obj, int room);
void obj_from_room (OBJ_DATA ** obj, int count);
void extract_obj (OBJ_DATA * obj);
void obj_to_obj (OBJ_DATA * obj, OBJ_DATA * container);
void obj_from_obj (OBJ_DATA ** obj, int count, OBJ_DATA * container);
int can_see_obj (CHAR_DATA * ch, OBJ_DATA * obj);
int can_obj_to_container (OBJ_DATA * obj, OBJ_DATA * container, char **msg, int count);	
int can_obj_to_inv (OBJ_DATA * obj, CHAR_DATA * ch, int *error, int count);	

void show_evaluate_information (CHAR_DATA *ch, OBJ_DATA * obj);
int find_material(char* argument);
int get_material_hardness(OBJ_DATA* tobj);
char *vnum_to_name (int vnum);
int is_name_in_list (char *name, char *list);
int get_bite_value (OBJ_DATA * obj);
void morph_obj (OBJ_DATA * obj);
OBJ_DATA *vtoo (int nVirtual);
char *obj_short_desc (OBJ_DATA * obj);
char *obj_desc (OBJ_DATA * obj, int level_see);
OBJ_DATA *load_colored_object (int nVirtual, char *string);
bool game_currency(int currency_type, int tobjVnum);
int translate_obj_flag(int tflag);
std::string translate_material(int value);


/*************************
// Player Related methods
*/
	//flags-Player
int all_are_set( int holder, int flags );
int nil_are_set( int holder, int flags );
int any_are_set( int holder, int flags );
int check_flags( int holder, int true_flags, int false_flags );

	//chargen-Player
void answer_application (CHAR_DATA * ch, char *argument, int cmd);
int is_being_reviewed (const char *name, const char *account);
bool is_newbie_acct (const char* account_name);
void starting_skill_boost (CHAR_DATA * ch, char* skill_name);

void process_queued_reviews (void);
void process_reviews (void);
struct time_info_data age (CHAR_DATA * ch);
void outfit_new_char (CHAR_DATA *ch, ROLE_DATA *role);
void newbie_hints (void);
void create_menu_options (struct descriptor_data *d);
void create_menu_actions (struct descriptor_data *d, char *arg);
void attribute_priorities (struct descriptor_data *d, char *arg);
void sex_selection (struct descriptor_data *d, char *arg);
void race_selection (struct descriptor_data *d, char *arg);
void skill_selection (struct descriptor_data *d, char *argument);
void skill_display (struct descriptor_data *d);
int is_restricted_skill (CHAR_DATA * ch, int skill);

	//Inventory-Player
OBJ_DATA *get_equip (CHAR_DATA * ch, int location);
int load_char_objs (struct char_data *ch, char *name);
int can_handle (OBJ_DATA * obj, CHAR_DATA * ch);
void obj_to_char (OBJ_DATA * obj, CHAR_DATA * ch);
void obj_from_char (OBJ_DATA ** obj, int count);
void equip_char (CHAR_DATA * ch, OBJ_DATA * obj, int pos);
OBJ_DATA *unequip_char (CHAR_DATA * ch, int pos);
OBJ_DATA *get_carried_item (CHAR_DATA * ch, int item_type);
int obj_mass (OBJ_DATA * obj);



	//Delays - Players
void update_delays (void);


	//Movement - players(IC and OOC)

void reload_char(void);
ROOM_DATA * get_diagonal_move_room (CHAR_DATA * ch, int dir);
bool point_diagonal (CHAR_DATA * ch, char *argument);
void movement_wear(CHAR_DATA* ch, int needed_movement);
void fallen_result(CHAR_DATA* ch, ROOM_PORTAL_DATA* tport);
void initiate_move (CHAR_DATA * ch);

	//misc-Player
void char__switch_item (CHAR_DATA * ch, char *argument, int cmd);	
	
int get_comestible_range (int num);
int get_comestible_range_food(int num);
void show_obj_to_char (OBJ_DATA * obj, CHAR_DATA * ch, int mode, int level_see);
int could_see_obj (CHAR_DATA * ch, OBJ_DATA * obj);
int has_craft (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft);


int is_yours (const char *name, const char *account);
int is_admin (const char *username);
int is_guide (const char *username);
int is_newbie (const CHAR_DATA* ch);
AFFECTED_TYPE *is_crafting (CHAR_DATA * ch);

int skill_level (CHAR_DATA * ch, char * skill, int diff_mod);
int skill_use (CHAR_DATA * ch, char * skill, int diff_mod);

void scribe (int new_message, int nVirtual, char *author, char *date,
			 char *message, long flags);

void offline_stamina (CHAR_DATA * ch, int since);
void offline_skill_train (CHAR_DATA * ch, int since);

/***************
// Room and Portal methods
*/
void light (CHAR_DATA * ch, OBJ_DATA * obj, int on, int on_off_msg);
int room_avail(ROOM_DATA *troom, OBJ_DATA *tobj, CHAR_DATA *tch);
int force_enter (CHAR_DATA *tch, ROOM_DATA *troom);
void update_room_tracks (void);
void room_update (void);
void add_room_affect (AFFECTED_TYPE ** af, int type, int duration, int intensity);
ROOM_DATA * vtor(int roomVirtual);
void room_new (CHAR_DATA * ch, char *argument);
void room_weather_desc(CHAR_DATA * ch, char *argument);
void room_alas_desc(CHAR_DATA * ch, char *argument);
int room_delete (ROOM_DATA * room);
void room_location (CHAR_DATA * ch, char *argument);
void room_size (CHAR_DATA * ch, char *argument);
void room_name (CHAR_DATA * ch, char *argument);
void room_desc (CHAR_DATA * ch, char *argument);
void room_bzone (CHAR_DATA * ch, char *argument);
void room_redesc (CHAR_DATA * ch, char *argument);
void room_wzone (CHAR_DATA * ch, char *argument);
void room_capacity (CHAR_DATA * ch, char *argument);
int test_overlap (ROOM_DATA *troom, CHAR_DATA *ch);
float room_distance (ROOM_DATA *start, ROOM_DATA *targ);

	//Room - Portal methods
void room_portal (CHAR_DATA * ch, char *argument);
void do_pset (CHAR_DATA * ch, char *argument, int cmd);
ROOM_PORTAL_DATA * vtop(int ident);
std::string plist_show (ROOM_PORTAL_DATA * tportal);
void portal_type (CHAR_DATA * ch, char *argument);
void pstat (CHAR_DATA * ch, int port_num);
int room_place_portal(CHAR_DATA * ch, ROOM_PORTAL_DATA *portal);
bool is_portal_placed(ROOM_PORTAL_DATA *tport, ROOM_DATA *troom);
void portal_new(CHAR_DATA * ch, char *argument);
void portal_room (CHAR_DATA * ch, char *argument);
void portal_flag (CHAR_DATA * ch, char *argument);
void portal_material (CHAR_DATA * ch, char *argument);
void portal_belong(ROOM_DATA *troom);
void portals_to_dir_options(ROOM_DATA *troom);
void portal_sdesc(CHAR_DATA * ch, char *arg);
void portal_ldesc(CHAR_DATA * ch, char *arg);
void portal_fdesc(CHAR_DATA * ch, char *arg);
void portal_keywords(CHAR_DATA * ch, char *arg);
void portal_skill(CHAR_DATA * ch, char *arg);
void portal_save(CHAR_DATA * ch);
void portal_slope (CHAR_DATA * ch, char *argument);
void room_remove_portal(CHAR_DATA * ch, char *arg);
void post_portal_fdesc (DESCRIPTOR_DATA * d);
ROOM_EXIT_DATA * is_exit(CHAR_DATA* ch, int dir);
ROOM_EXIT_DATA * is_exit_normal(CHAR_DATA* ch, int dir, int mode);
bool can_go_exit(CHAR_DATA * ch, int door);
ROOM_DATA * get_portal_room(CHAR_DATA *ch, int dir);
ROOM_DATA * room_from_port_dir(int port_num, int tdir);
int port_side_room (ROOM_DATA * troom, ROOM_PORTAL_DATA * tport);
int keyword_to_portal(CHAR_DATA *ch, int dir, char* keyword);
int past_barrier(CHAR_DATA * ch, ROOM_PORTAL_DATA * tport, bool sneak_check);
int past_crossing(CHAR_DATA * ch, ROOM_PORTAL_DATA * tport, bool sneak_check);
void past_transport(CHAR_DATA * ch, ROOM_PORTAL_DATA * tport, bool sneak_check);
bool test_location_rooms(CHAR_DATA * ch, ROOM_PORTAL_DATA *portal);
void room_link (CHAR_DATA * ch, char *argument);
void portal_fail (CHAR_DATA * ch, char *argument);
ROOM_EXIT_DATA * exit_from_portal(int port_ident, int to_room);
void look_portal_key(CHAR_DATA* tch, char* arg);
void scan_portal_key(CHAR_DATA* ch, char* key);
/*****************
// System methods
*/
	//Misc - System
void copyover_recovery (void);
void prepare_copyover (int cmd);
void save_vnpc_timestamp();
void load_vnpc_timestamp();
int vnpc_customer (CHAR_DATA * keeper, int purse);	
int get_user_seconds ();
int strn_cmp (const char *arg1, const char *arg2, int n);
int free_mem (char *&ptr);
int free_mem (void *ptr);
int free_mem_array (void *ptr);
void reset_itimer ();
void alarm_handler ();
void init_alarm_handler ();
void close_socket (struct descriptor_data *d);
void shutdown_request (int signo); 
void logsig (int signo); 
void sigsegv (int signo);
void sigchld (int signo);

	//Initilization and loading-System
void boot_mobiles (void);
void boot_objects (void);
void boot_crafts (void);
void boot_db (void);
void boot_zones(void);
void boot_zones_raffects (void);
void boot_rooms ();
void boot_portals ();
void update_crafts_database (void);
void init_stayput_mobiles ();
void load_race_table (void);
void load_writing (OBJ_DATA * obj);
void load_all_writing (void);
void load_rooms (void);
void reload_roles (void);
void setup_registry (void);
void load_skills_mysql (void);
void load_clans_mysql (void);
void init_mysql (void);
void init_mysqlplus (void);
void ping_mysqlplus (void);
void initialize_materials (void);
void initialize_location_map (void);
void refresh_db_connection (void);
void reload_sitebans (void);
char *get_text_buffer (CHAR_DATA * ch, TEXT_DATA * list, char *text_name);
void free_registry (void);
void save_tracks ();
void load_tracks ();
void load_online_stats ();
CHAR_DATA *load_saved_mobiles (CHAR_DATA * ch, char *name);
CHAR_DATA *load_a_saved_mobile (int nVirtual, int coldload, FILE * fp);
void initialize_flag_values(void);

	//account-System
int game_main (int argc, char *argv[]);
void create_menu_options (struct descriptor_data *d);
void check_maintenance ();
void unban_site (SITE_INFO * site);
void disconnect_banned_hosts ();
void ban_host (char *host, char *banned_by, int length);
std::string resolved_host (char *ip);
char *reference_ip (char *guest_name, char *host);

	//saving-System
int save_rooms (CHAR_DATA * ch, int zone);
void save_writing (OBJ_DATA * obj);
void save_banned_sites ();
void save_roles ();
void save_stayput_mobiles ();
int save_char (CHAR_DATA * ch, int save_objs);
int save_room_affects (int zone);
void fwrite_room_affects (ROOM_DATA * troom, FILE * fp);
void autosave (void);
void save_mobile (CHAR_DATA * mob, FILE * fp, char *save_reason, int extract);
void save_attached_mobiles (CHAR_DATA * ch, int extract);
int fwrite_a_obj (OBJ_DATA * obj, FILE * fp);
/******************
 // Weather & Date methods
 */
void initialize_weather_zones (void);
void load_weather_obj(ROOM_DATA *troom);
int weather_object_exists(OBJ_DATA * list, int vnum);
void weather_and_time (int mode);
void weather_setup(int zvalue);
void sunrise_sunset(void);
void moon_state(int zvalue);
char* time_phrase(int high_sun);

/******************
// Utility methods
*/
float lookup_max_gain(int skill_num);
int just_a_number (char *buf);
int is_real_number (std::string str);
bool is_vowel (char c);
int isname (const char *str, char *namelist);
int isnamec (char *str, char *namelist);
char *fname (char *namelist);
bool IS_NPC (const CHAR_DATA *ch);
bool IS_NPC (CHAR_DATA *ch);

void sprinttype (int type, const char *names[], char *result);
char* sprintbit (long vektor, const char *names[]);
char* get_line (char **buf, char *ret_buf);
int get_token (char **s, char *token);
int get_next_coldload_id (int for_a_pc);

int is_he_here (CHAR_DATA * ch, CHAR_DATA * he, int check);
int is_he_somewhere (CHAR_DATA * he);
CHAR_DATA *get_char_room_vis (CHAR_DATA * ch, const char *name);
CHAR_DATA *get_char_room_vis2 (CHAR_DATA * ch, int vnum, char *name);
CHAR_DATA *get_char_vis (CHAR_DATA * ch, const char *name);
CHAR_DATA *get_mob_vnum (int nVirtual);
CHAR_DATA *get_char_id (int coldload_id);
OBJ_DATA *get_obj_id (int coldload_id);
OBJ_DATA *get_obj_in_list_id (int coldload_id, OBJ_DATA * list);
CHAR_DATA *get_char_room (char *name, int room);
CHAR_DATA *get_char (char *name);
CHAR_DATA *get_char_nomask (char *name);
CHAR_DATA *get_char_nomask_nonpc (char *name);
CHAR_DATA *get_pc (char *buf);
CHAR_DATA *load_pc (char *buf);

char * room_get_description (ROOM_DATA * room);

int scan_light_penalty(ROOM_DATA* current_room, ROOM_DATA* target_room);
int scan_distance_penalty (int dist);
int scan_weather_penalty(ROOM_DATA* room);
bool char__room_scan (CHAR_DATA * ch, ROOM_DATA * next_room, const char *ptrStrDir, int nSkillLevel, bool bSaw, float curr_dist);

float char_light_carry(CHAR_DATA* ch);
char * convert_dir(char *dir);

int number (int from, int to);
unsigned int dice (unsigned int number, unsigned int size);
void pad_buffer (char *buf, int pad_stop);
int odds_sqrt (int percent);

char *strgdup (char *source);
void arg_splitter (int argc, char *fmt, ...);
int parse_argument (const char *commands[], char *string);
char *get_line (char **buf, char *ret_buf);
char *file_to_string (char *name);
char *read_a_line (FILE * fp);

int GCD(int, int);
double RoundDouble(double, int);
int fread_number (FILE * fp);


void replaceString(char *&destination, const char *source);
char* duplicateString(const char *source);


/******************
 // Lua methods
 */

int lua_test(CHAR_DATA* ch);
void larg_setup_room_triggers(ROOM_DATA* room);
/********************
 // TODO - These methods work, but need updating
 */

AFFECTED_TYPE *is_room_affected (AFFECTED_TYPE * af, int type);
int apply_affect (CHAR_DATA * ch, int sn, int duration, int power);
void remove_affect_type (CHAR_DATA * ch, int type);
void affect_modify (CHAR_DATA * ch, int type, int loc, int mod, int bitv,
					int add, int sn);
void affect_to_char (CHAR_DATA * ch, struct affected_type *af);
void affect_remove (CHAR_DATA * ch, struct affected_type *af);
void affect_join (CHAR_DATA * ch, struct affected_type *af, bool avg_dur,
				  bool avg_mod);


AFFECTED_TYPE *get_obj_affect (OBJ_DATA * obj, int type);
AFFECTED_TYPE *get_obj_affect_location (OBJ_DATA * obj, int location);
void remove_obj_affect (OBJ_DATA * obj, int type);
void remove_obj_affect_location (OBJ_DATA * obj, int location);
void affect_to_obj (OBJ_DATA * obj, AFFECTED_TYPE * af);
int magic_add_affect (CHAR_DATA * ch, int type, int duration,
					  int modifier, int location, int bitvector, int sn);
void magic_affect (CHAR_DATA * ch, int magic);
int magic_add_obj_affect (OBJ_DATA * obj, int type, int duration,
						  int modifier, int location, int bitvector, int sn);

void magic_add_delayed_affect (CHAR_DATA * victim, int sn, int delay,
							   int duration, int power);

void rl_minute_delayed_affects (void);
char *type_to_spell_name (int type);
/********************
 // Unsorted methods
 */

char * crude_name (int race);
char * track_age (int hours_passed);
int obj_activate (CHAR_DATA * ch, OBJ_DATA * obj);
char * specific_name (int race);
char * speed_adj (int speed);
int has_been_sighted (CHAR_DATA * ch, CHAR_DATA * target);

int generic_find (char *arg, int bitvector, CHAR_DATA * ch,
				  CHAR_DATA ** tar_ch, OBJ_DATA ** tar_obj);
char *swap_xmote_target (CHAR_DATA * ch, char *argument, int cmd);
void insert_string_variables (OBJ_DATA * new_obj, char *string);
int is_direction (char *argument);
void update_char_objects (CHAR_DATA * ch);
void raw_kill (CHAR_DATA * ch);
void gain_condition (CHAR_DATA * ch, int condition, int value);
void npc_evasion (CHAR_DATA * ch, int dir);
void refresh_map (void);
void page_string (struct descriptor_data *d, const char *str);

int is_abbrev (const char *arg1, const char *arg2);
int is_abbrevc (const char *arg1, const char *arg2);
void free_obj (OBJ_DATA * obj);
int search_block (char *arg, const char **list, bool exact);
bool get_obj_in_equip_num (CHAR_DATA * ch, long vnum);
void spitstat (CHAR_DATA * ch, struct descriptor_data *recipient);
void save_char_objs (CHAR_DATA * ch, char *name);
void criminalize (CHAR_DATA * ch, CHAR_DATA * vict, int zone,
				  int penalty_time);
int index_lookup (const char* const* index, const char* lookup);
void archive_log (int log_type);
int lookup_value (std::string str_value, int reg_index);
AFFECTED_TYPE *get_affect (const CHAR_DATA * ch, int spell);
std::string lookup_string (int value, int reg_index);
void set_hobbitmail_flags (int id, int flags);
void string_add (DESCRIPTOR_DATA * d, char *str);
struct time_info_data real_time_passed (time_t t2, time_t t1);
int calc_lookup (CHAR_DATA * ch, int index, int entry);
void mobile_routines (int pulse);
int strike (CHAR_DATA * src, CHAR_DATA * tar, int attack_num);
void show_char_to_char (CHAR_DATA * i, CHAR_DATA * ch, int mode, int llevel);

void notify_guardians (CHAR_DATA * ch, CHAR_DATA * tch, int cmd);
int is_in_cell (CHAR_DATA * ch, int zone);
void create_guest_avatar (struct descriptor_data *d, char *argument);

char *encrypt_buf (const char *buf);
void free_char (CHAR_DATA *&ch);
OBJ_DATA *find_dwelling_obj (int dwelling_room);
void cleanup_the_dead (int mode);

int save_dwelling_rooms ();
void load_dwelling_rooms ();
char *time_string (CHAR_DATA * ch);
int num_starting_locs (int race);
void insert_newsletter_into_website (int timestamp, char *newsletter);
int unused_writing_id (void);
void process_reviews (void);
CHAR_DATA *new_char (int pc_type);
OBJ_DATA *new_object ();
int attempt_disarm (CHAR_DATA * ch, CHAR_DATA * victim);
void forget (CHAR_DATA * ch, CHAR_DATA * foe);
void* alloc (int bytes);
int redefine_mobiles (CHAR_DATA * proto);
char *colorize (const char *source, char *target, struct descriptor_data *d);
void hourly_update (void);
void traslate_it (int num);
OBJ_DATA *get_obj_in_dark (CHAR_DATA * ch, char *name, OBJ_DATA * list);
char getall (char *name, char *newname);
void define_variable (CHAR_DATA * mob, MOBPROG_DATA * program,
					  char *argument);

int mp_eval_eq (CHAR_DATA * mob, char **equation);
FILE *open_and_rename (CHAR_DATA * ch, char *name, int zone);
int getCharNum(CHAR_DATA * ch, bool isMob);

void stock_new_deliveries ();

int lookup_skill_id (std::string name); //new style with map
char * lookup_skill_name (int skill_num);

int is_in_room (CHAR_DATA * ch, CHAR_DATA * target);
void command_interpreter (CHAR_DATA * ch, char *argument);
void argument_interpreter (char *argument, char *first_arg, char *second_arg);
int fill_word (char *argument);
void half_chop (char *string, char *arg1, char *arg2);
void nanny (struct descriptor_data *d, char *arg);
int social (CHAR_DATA * ch, char *argument);
void checkpointing (int signo); 
int create_entry (char *name);
void zone_update (void);
void clear_object (OBJ_DATA * obj);
char *read_string (char *string);
char *fread_string (FILE * fl);
char *fread_word (FILE * fl);
void load_boards (void);
int add_registry (int reg_index, int value, const char *string);
void write_board_list (void);
BOARD_DATA *board_lookup (const char *name);
void add_message (int new_message, char *name, int nVirtual, const char *poster,
				  char *date, char *subject, char *info, char *message,
				  long flags);
void update_assist_queue (CHAR_DATA * ch, bool remove);

void add_char (char *buf, char c);
void add_board (int level, char *name, char *title);
void mark_as_read (CHAR_DATA * ch, int number);
struct message_data *load_mysql_message (char *msg_name, int board_type,
	int msg_number);
int erase_mysql_board_post (CHAR_DATA * ch, char *name, int board_type,
							char *argument);
int get_mysql_board_listing (CHAR_DATA * ch, int board_type, char *name);
void add_message_to_mysql_vboard (const char *name, const char *poster,
struct message_data *message);
void add_message_to_mysql_player_notes (const char *name, const char *poster,
struct message_data *message);
char *lookup_race_variable (int id, int which_var);
int lookup_race_id (const char *name);
TEXT_DATA *add_text (TEXT_DATA ** list, char *filename, char *document_name);
CHAR_DATA *try_load_char (char *name);

void post_message (struct descriptor_data *d);
void nanny_choose_pc (struct descriptor_data *d, char *argument);
void sense_activity (CHAR_DATA * user, int talent);
void check_idlers ();
void check_linkdead ();
int track (CHAR_DATA * ch, int to_room);
void name_to_ident (CHAR_DATA * ch, char *buf);

void add_second_affect (int type, int seconds, CHAR_DATA * ch,
						OBJ_DATA * obj, const char *info, int info2);
void second_affect_update (void);
void hour_affect_update (void);
int find_door (CHAR_DATA * ch, char *type, char *dir);
char *get_profession_name (int prof_id);
void add_profession_skills (CHAR_DATA * ch, char *skill_list);
int has_required_crafting_skills (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft);
int is_opening_craft (CHAR_DATA * ch, SUBCRAFT_HEAD_DATA * craft);
OBJ_DATA *has_key (CHAR_DATA * ch, OBJ_DATA * obj, int key);

void refresh_zone (void);
int is_higher_rank (CHAR_DATA * src, CHAR_DATA * tar);
void invite_accept (CHAR_DATA * ch, char *argument);
int flee_attempt (CHAR_DATA * ch, int dir_flag);
SECOND_AFFECT *get_second_affect (CHAR_DATA * ch, int type, OBJ_DATA * obj);
void clear_player_from_second_affects (CHAR_DATA *ch);
void remove_second_affect (SECOND_AFFECT * sa);

void open_skill (CHAR_DATA * ch, int skill);
void expose_skill (CHAR_DATA * ch, int skill);



void add_memory (CHAR_DATA * add, CHAR_DATA * mob);

int is_obj_here (CHAR_DATA * ch, OBJ_DATA * obj, int check);





struct message_data *load_message (char *msg_name, int pc_message,
	int msg_number);

void process_quarter_events (void);

int skill_roll (int ability);

int eval_att_eq (CHAR_DATA * ch, char **equation);
void knock_out (CHAR_DATA * ch, int seconds);


struct descriptor_data *is_pc_attached (char *buf);
void rl_minute_affect_update (void);
void export_who_list(void);

int is_obj_in_list (OBJ_DATA * obj, OBJ_DATA * list);


void job_add_affect (CHAR_DATA * ch, int type, int days, int pay_date,
					 int cash, int count, int object_vnum, int employer);
void remove_object_affect (OBJ_DATA * item, AFFECTED_TYPE * af);
AFFECTED_TYPE *get_obj_affect_type (OBJ_DATA * obj, int type);

int decipher_speakign (CHAR_DATA * ch, int skillnum, int skill);
OBJ_DATA *is_at_table (CHAR_DATA * ch, OBJ_DATA * table);
int whisper_it (CHAR_DATA * ch, char * skillname, char *source, char *target);


void shadowers_shadow (CHAR_DATA * ch, int to_room, int move_dir);
int could_see (CHAR_DATA * ch, CHAR_DATA * target);
void ten_second_update (void);
int check_password (const char *pass, const char *encrypted);
char *generate_password (int argc, char **argv);

int enforcer (CHAR_DATA * ch, CHAR_DATA * crim, int will_act, int witness);
void mob_jailer_func (CHAR_DATA * ch);
void add_criminal_time (CHAR_DATA * ch, int zone, int penalty_time);
void system_log (const char *str, bool error);
void player_log (CHAR_DATA * ch, char *command, char *str);



#endif // _rpie_protos_h_
