//////////////////////////////////////////////////////////////////////////////
//
/// commands.c : Root Command Parser
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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "structs.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"
#include "decl.h"
#include "group.h"

DESCRIPTOR_DATA *last_descriptor;
char full_last_command[MAX_STRING_LENGTH]= { '\0' };
char last_command[MAX_STRING_LENGTH]= { '\0' };


extern const char *ignore[];
extern const char* dirs[];

const class command_data commands[] = {
	
	/* Mortal commands */
	
	{"\01craft", do_say, REST, C_XLS},
	{".", do_say, REST, C_SUB | C_DOA | C_BLD | C_PAR | C_DEL},
	{",", do_emote, REST, C_SUB | C_DOA | C_BLD | C_PAR | C_DEL},
	{":", do_emote, REST, C_SUB | C_DOA | C_BLD | C_PAR | C_DEL},
	{"\\", do_newbchat, DEAD, C_DOA | C_DEL | C_HID},
	{"accept", do_accept, REST, C_DEL | C_BLD | C_HID},
	{"as", do_as, DEAD, C_LV3},
	{"assist", do_assist, REST, C_SUB | C_DOA | C_BLD | C_PAR | C_DEL},
	{"barter", do_barter, SIT, 0},
	{"blindfold", do_blindfold, STAND, 0},
	{"buy", do_buy, SIT, 0},
	{"camp", do_camp, STAND, C_WLK | C_BLD},
	{"castout", do_castout, REST, C_BLD},
	{"close", do_close, SIT, 0},
	{"clockout", do_clockout, SIT, C_XLS},
	{"clockin", do_clockin, SIT, C_XLS},
	{"command", do_command, REST, C_BLD},
	{"commands", do_commands, DEAD,
		C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR | C_NLG},
	{"commence", do_commence, REST,
		C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR},
	{"compare", do_compare, SIT, 0},
	{"contents", do_contents, REST,
		C_SUB | C_DOA | C_PAR | C_NLG | C_HID},
	{"count", do_count, REST, C_NLG},
	{"crafts", do_crafts, DEAD,
		C_DEL | C_SUB | C_HID | C_DOA | C_BLD | C_PAR  | C_NLG},
	{"credits", do_credits, DEAD,
		C_DEL | C_SUB | C_HID | C_DOA | C_BLD | C_PAR },
	{"down", do_down, STAND, C_HID | C_DOA | C_BLD},
	{"dip", do_dip, SIT, C_WLK | C_DEL | C_HID | C_SUB | C_DOA},
	{"decline", do_decline, REST, C_DEL | C_BLD},
	{"dmote", do_dmote, SLEEP,
		C_SUB | C_DOA | C_HID | C_BLD | C_PAR  | C_DEL},
	{"drag", do_drag, STAND, C_WLK },
	{"dreams", do_dreams, SLEEP, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR},
	{"drink", do_drink, REST, C_BLD},
	{"drop", do_drop, REST, C_DOA | C_BLD},
	{"east", do_east, STAND, C_HID | C_DOA | C_BLD},
	{"eat", do_eat, REST, C_BLD},
	{"emote", do_emote, REST, C_SUB | C_DOA | C_BLD | C_PAR | C_DEL },
	{"empty", do_empty, REST, C_BLD},
	{"enter", do_enter, STAND, C_WLK | C_BLD},
	{"equipment", do_equipment, SLEEP,
		C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR  | C_NLG},
	{"erase", do_erase, REST, C_DEL | C_HID | C_SUB | C_DOA | C_BLD},
	{"evaluate", do_evaluate, SIT, 0},
	{"exits", do_exits, REST,
		C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR  | C_NLG},
	{"examine", do_examine, REST, 0},
	{"fill", do_fill, REST, 0},
	{"flee", do_flee, STAND, 0},
	{"flip", do_flip, REST, C_DOA},
	{"follow", do_follow, REST, C_BLD | C_PAR},
	{"furnishings", do_tables, REST,
		C_SUB | C_DOA  | C_PAR  | C_NLG},
	{"furniture", do_tables, REST,
		C_SUB | C_DOA  | C_PAR  | C_NLG},
	{"get", do_get, REST, C_BLD},
	{"give", do_give, REST, C_DOA},
	{"group", do_group, REST,
		C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR  | C_NLG},
	{"help", do_help, DEAD,
		C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_XLS | C_PAR },
	{"hide", do_hide, STAND, C_WLK | C_HID },
	{"hood", do_hood, REST, C_HID | C_BLD | C_DOA},
	{"inventory", do_equipment, SLEEP,
		C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR  | C_NLG},
	{"invite", do_invite, SIT, 0},
	{"journal", do_journal, DEAD, C_WLK | C_BLD | C_HID },
	{"jerase", do_jerase, DEAD, C_WLK | C_BLD | C_HID },
	{"jread", do_jread, DEAD, C_WLK | C_BLD | C_HID },
	{"jwrite", do_jwrite, DEAD, C_WLK | C_BLD | C_HID },
	{"junk", do_junk, REST },
	{"knock", do_knock, STAND, C_BLD},
	{"look", do_look, REST, C_DEL | C_HID | C_SUB | C_DOA | C_PAR},
	{"leave", do_leave, STAND, C_WLK | C_BLD},
	{"leaveprivate", do_leaveprivate, REST, C_HID},
	{"light", do_light, SIT, C_HID | C_DOA},
	{"list", do_list, SIT, C_BLD},
	{"lock", do_lock, SIT, 0},
	{"mark", do_mark, REST, 0},
	{"mend", do_mend, SIT, 0},
	{"mute", do_mute, DEAD,
		C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR  | C_NLG},
	{"north", do_north, STAND, C_HID | C_DOA | C_BLD},
	{"ne", do_northeast, STAND, C_HID | C_DOA | C_BLD},
	{"nw", do_northwest, STAND, C_HID | C_DOA | C_BLD},
	{"name", do_name, DEAD, C_BLD | C_WLK | C_HID },
	{"newbchat", do_newbchat, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR},
	{"nod", do_nod, REST, C_HID | C_SUB | C_DOA | C_BLD},
	{"northeast", do_northeast, STAND, C_HID | C_DOA | C_BLD},
	{"northwest", do_northwest, STAND, C_HID | C_DOA | C_BLD},
	{"news", do_news, DEAD,
		C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR },
	{"omote", do_omote, REST, C_SUB | C_DOA | C_BLD | C_PAR  | C_DEL},
	{"origins", do_origins, REST, 0}, /* grommit */
	{"ooc", do_ooc, DEAD, C_DEL | C_HID},
	{"open", do_open, SIT, C_BLD},
	{"outfit", do_outfit, REST, C_HID | C_SUB | C_DOA | C_PAR}, /* leader-command or level 2 */
	{"ownership", do_ownership, REST, 0},
	{"palm", do_palm, SIT, C_WLK | C_BLD | C_HID },
	{"payday", do_payday, REST, C_HID | C_SUB | C_DOA | C_PAR},
	{"payroll", do_payroll, SIT, C_BLD},
	{"petition", do_petition, DEAD,
		C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR },
	{"plan", do_plan, DEAD,
		C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR },
	{"pmote", do_pmote, SLEEP,
		C_SUB | C_DOA | C_HID | C_BLD | C_PAR  | C_DEL},
	{"point", do_point, STAND, C_WLK},
	{"pour", do_pour, REST, 0},
	{"preview", do_preview, SIT, C_BLD},
	{"promote", do_promote, REST, 0},
	{"put", do_put, REST, C_BLD},
	{"quit", do_quit, DEAD, C_HID | C_SUB | C_DOA | C_BLD | C_PAR},
	{"qscan", do_qscan, SIT, C_HID | C_WLK},
	{"read", do_read, REST, C_WLK | C_DEL | C_HID | C_SUB | C_DOA},
	{"receipts", do_receipts, SIT, C_BLD},
	{"remove", do_remove, STAND, 0},
	{"rest", do_rest, REST, C_WLK | C_HID | C_DOA | C_BLD | C_PAR},
	{"return", do_return, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR},
	{"roll", do_roll, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR},
	{"rollcall", do_rollcall, REST, C_BLD},
	{"south", do_south, STAND, C_HID | C_DOA | C_BLD},
	{"se", do_southeast, STAND, C_HID | C_DOA | C_BLD},
	{"sw", do_southwest, STAND, C_HID | C_DOA | C_BLD},
	{"say", do_say, REST, C_SUB | C_DOA | C_BLD | C_PAR | C_DEL},
	{"save", do_save, SLEEP,
		C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR  | C_NLG},
	{"score", do_score, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR  | C_NLG},
	{"scommand", do_scommand, STAND, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR  | C_NLG},
	{"scan", do_scan, STAND, C_WLK},
	{"scribe", do_select_script, DEAD,
		C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR },
	{"search", do_search, STAND, C_WLK },
	{"sell", do_sell, SIT, C_BLD},
	{"set", do_set, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR },
	{"shadow", do_shadow, STAND, C_WLK | C_SUB | C_HID | C_DEL | C_PAR},
	{"shout", do_shout, REST, C_BLD | C_PAR | C_DEL},
	{"sit", do_sit, REST, C_HID | C_DOA | C_BLD | C_PAR},
	{"sing", do_sing, REST, C_SUB | C_DOA | C_BLD | C_PAR | C_DEL},
	{"skills", do_skills, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR  | C_NLG},
	{"sleep", do_sleep, SLEEP, C_HID | C_BLD | C_PAR},
	{"southeast", do_southeast, STAND, C_HID | C_DOA | C_BLD},
	{"southwest", do_southwest, STAND, C_HID | C_DOA | C_BLD},
	{"sneak", do_sneak, STAND, C_WLK | C_HID },
	{"speak", do_speak, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR},
	{"stand", do_stand, SLEEP, C_HID | C_DOA | C_BLD},
	{"stop", do_stop, SIT, C_BLD | C_DEL | C_PAR },
	{"switch", do_switch, REST, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR},
	{"take", do_take, STAND, 0},
	{"talk", do_talk, REST, C_DEL | C_SUB | C_DOA | C_BLD | C_PAR},
	{"tally", do_tally, REST, C_HID | C_SUB | C_DOA | C_PAR}, /* leader-command or level 1 */
	{"tables", do_tables, REST, C_SUB | C_DOA  | C_PAR  | C_HID},
	{"tell", do_tell, REST, C_DEL | C_DOA | C_SUB | C_PAR},
	{"teach", do_teach, STAND, 0},
	{"tear", do_tear, REST, C_DOA},
	{"think", do_think, DEAD,
		C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR },
	{"time", do_time, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR },
	{"title", do_title, SIT, C_WLK | C_DEL | C_HID | C_SUB | C_DOA},
	{"toss", char__do_toss, SIT, C_SUB },
	{"track", do_track, STAND, C_WLK | C_DOA | C_BLD },
	{"travel", do_travel, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR },	/* act.comm.c */
	{"turn", do_flip, REST, C_DOA},
	{"typo", do_typo, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR },
	{"up", do_up, STAND, C_HID | C_DOA | C_BLD},
	{"unlock", do_unlock, SIT, 0},
	{"value", do_value, SIT, C_BLD},
	{"variant", do_variant, DEAD,
		C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR },
	{"vis", do_vis, REST, C_DEL | C_SUB | C_DOA | C_BLD | C_PAR | C_HID},
	{"voice", do_voice, DEAD,
		C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR },
	{"west", do_west, STAND, C_HID | C_DOA | C_BLD},
	{"wake", do_wake, SLEEP, C_HID | C_DOA | C_BLD},
	{"wanted", do_wanted, REST, C_BLD },
	{"wear", do_wear, REST, C_BLD},
	{"weather", do_weather, REST, C_DEL | C_HID | C_SUB | C_BLD | C_PAR },
	{"whisper", do_whisper, REST, C_SUB | C_DOA | C_BLD  | C_PAR | C_DEL},
	{"who", do_who, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR  | C_NLG},
	{"write", do_write, SIT, C_WLK | C_DEL | C_HID | C_SUB | C_DOA},
	{"yell", do_shout, REST, C_SUB | C_BLD | C_PAR | C_DEL},
	{"wizlist", do_wizlist, DEAD, C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR},
	
	/* Guide Commands Level 0 with Guide flag*/
	{"history", do_history, DEAD, C_GDE | C_DEL},
	{"mset", do_mset, DEAD, C_GDE},
	{"review", do_review, DEAD, C_GDE},
	{"show", do_show, DEAD, C_GDE},
	
	/* Player Builder  Lvl1: (basic commands, room and objects only) */
	
	{"goto", do_goto, DEAD, C_LV1},
	
	/* invis modified to be generally available, but then race limited */
	{"invis", do_invis, DEAD, C_LV1 | C_DEL | C_HID | C_SUB | C_DOA | C_BLD | C_PAR  | C_NLG | C_PBU},
	{"load", do_load, DEAD, C_LV1},
	{"map", do_map, DEAD, C_LV1 },	/* staff.c */
	{"olist", do_olist, DEAD, C_LV1},
	{"oset", do_oset, DEAD, C_LV1},
	{"plist", do_plist, DEAD, C_LV1},
	{"pset", do_pset, DEAD, C_LV1},
	{"rlist", do_rlist, DEAD, C_LV1},
	{"rset", do_rset, DEAD, C_LV1},
	{"stat", do_stat, DEAD, C_LV1},
	{"tags", do_tags, DEAD, C_LV1},
	{"immcommands", do_immcommands, DEAD, C_LV1},

	
		//
	{";", do_ichat, DEAD, C_DOA | C_DEL | C_HID},
	{"ichat", do_ichat, DEAD, C_DOA | C_DEL | C_HID},
	{"wiznet", do_ichat, DEAD, C_DOA | C_DEL | C_HID},
	
	
		//limited to implementors while coding is underway
	{"cset", do_cset, DEAD, C_IMP}, // lvl1 to allow crafter-only admins
	
	
	/*Advanced Builder Level 2:  (mobs and crafts) */
	/* General Staff Commands (Level 2+) but not Player_builders */
	
	{"at", do_at, DEAD, C_LV2},
	{"find", do_find, DEAD, C_LV2},
	{"gstat", do_gstat, DEAD, C_LV2},
	{"locate", do_locate, DEAD, C_LV2},
	{"mlist", do_mlist, DEAD, C_LV2},
	{"notes", do_notes, DEAD, C_LV2},
	{"purge", do_purge, DEAD, C_LV2},
	{"rend", do_rend, DEAD, C_LV2},	 
	{"restore", do_restore, DEAD, C_LV2},
	{"send", do_immtell, DEAD, C_LV2},
	{"shutdown", do_shutdown, DEAD, C_LV2},
	{"transfer", do_transfer, DEAD, C_LV2},	
	{"users", do_users, DEAD, C_LV2},
	{"vboards", do_vboards, DEAD, C_LV2},
	{"wearloc", do_wearloc, DEAD, C_LV2},
	{"where", do_where, DEAD, C_LV2},
	
	/* Basic RPA Level 3: (players) */
		//{"addcraft", do_addcraft, DEAD, C_LV3},
	{"becho", do_becho, DEAD, C_LV3},
	{"clan", do_clan, DEAD, C_LV3},
	{"echo", do_echo, DEAD, C_LV3},
	{"job", do_job, DEAD, C_LV3},
	{"last", do_last, DEAD, C_LV3},
	{"makeprivate", do_makeprivate, DEAD, C_LV3},
	{"openskill", do_openskill, DEAD, C_LV3},
	{"pecho", do_pecho, DEAD, C_LV3},
	{"register", do_register, DEAD, C_LV3},
		//{"remcraft", do_remcraft, DEAD, C_LV3},
	{"snoop", do_snoop, DEAD, C_LV3},
	{"subscribe", do_subscribe, DEAD, C_LV3},
	{"summon", do_summon, DEAD, C_LV3},
	{"unsubscribe", do_unsubscribe, DEAD, C_LV3},
	{"wclone", do_wclone, DEAD, C_LV3},
	{"zecho", do_zecho, DEAD, C_LV3},
	
	
	/* Advanced RPA Level 4 (advanced players) */
	{"alog", do_alog, DEAD, C_LV4},	/* Announcements */
	{"award", do_award, DEAD, C_LV4},
	{"ban", do_ban, DEAD, C_LV4},
	{"broadcast", do_broadcast, DEAD, C_LV4},
	{"deduct", do_deduct, DEAD, C_LV4},
	{"disband", do_disband, DEAD, C_LV4},
	{"email", do_email, DEAD, C_LV4},
	{"force", do_force, DEAD, C_LV4 | C_DEL | C_SUB | C_HID},
	{"givedream", do_givedream, DEAD, C_LV4},
		//{"hedit", do_hedit, DEAD, C_LV4},
	{"passwd", do_passwd, DEAD, C_LV4},
	{"plog", do_plog, DEAD, C_LV4},
	{"professions", do_professions, DEAD, C_LV4},
	{"recruit", do_recruit, DEAD, C_LV4},
	{"refresh", do_refresh, DEAD,  C_LV4},
	{"role", do_role, DEAD, C_LV4},
	{"saverooms", do_saverooms, DEAD, C_LV4},
	{"stayput", do_stayput, DEAD, C_LV4},
	{"writings", do_writings, DEAD, C_LV4},
	{"zset", do_zset, DEAD, C_LV4},
	
	/* HRPA Level 5 */
	{"*", do_fivenet, DEAD, C_LV5},
	{"affect", do_affect, DEAD, C_LV5},
	{"fivenet", do_fivenet, DEAD, C_LV5},
	{"gecho", do_gecho, DEAD, C_LV5},
	{"pfile", do_pfile, DEAD, C_LV5},
	{"roster", do_roster, DEAD, C_LV5},
	{"unban", do_unban, DEAD, C_LV5},
	{"watch", do_watch, DEAD, C_LV5},
	{"wizlock", do_wizlock, DEAD, C_LV5},
	{"wmotd", do_wmotd, DEAD, C_LV5}, // write the MOTD 
	
	/* IMP level */
	{"csv", do_csv, DEAD, C_IMP | C_XLS},  // send the user a particular chunk of data 
	{"day", do_day, DEAD, C_IMP | C_XLS},
	{"debug", do_debug, DEAD, C_IMP | C_XLS},
	{"hour", do_hour, DEAD, C_IMP},
	{"mysql", do_mysql, DEAD, C_IMP | C_XLS},
	{"coords", do_coords, DEAD, C_IMP | C_XLS},
	
	{"", NULL, 0, 0}
};

const char *fill[8] = {
	"in",
	"from",
	"with",
	"the",
	"on",
	"at",
	"to",
	"\n"
};

int
search_block (char *arg, const char **list, bool exact)
{
	register int i = 0, l;

	/* Make into lower case, and get length of string */
	for (l = 0; *(arg + l); l++)
		*(arg + l) = tolower (*(arg + l));

	if (exact)
	{
		for (i = 0; **(list + i) != '\n'; i++)
			if (!strcmp (arg, *(list + i)))
				return (i);
	}
	else
	{
		if (!l)
			l = 1;			/* Avoid "" to match the first available string */
		for (i = 0; **(list + i) != '\n'; i++)
			if (!strncmp (arg, *(list + i), l))
				return (i);
	}

	return (-1);
}

void
show_to_watchers (CHAR_DATA * ch, char *command)
{
	CHAR_DATA *tch;
	AFFECTED_TYPE *af;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if ((af = get_affect (ch, MAGIC_WATCH1)))
	{
		if (!is_he_somewhere ((CHAR_DATA *) af->a.spell.t))
			affect_remove (ch, af);
		else
		{
			tch = (CHAR_DATA *) af->a.spell.t;
			sprintf (buf, "%s:  %s\n", ch->name, command);
			tch->send_to_char (buf);
		}
	}

	if ((af = get_affect (ch, MAGIC_WATCH2)))
	{
		if (!is_he_somewhere ((CHAR_DATA *) af->a.spell.t))
			affect_remove (ch, af);
		else
		{
			tch = (CHAR_DATA *) af->a.spell.t;
			sprintf (buf, "%s:  %s\n", ch->name, command);
			tch->send_to_char (buf);
		}
	}

	if ((af = get_affect (ch, MAGIC_WATCH3)))
	{
		if (!is_he_somewhere ((CHAR_DATA *) af->a.spell.t))
			affect_remove (ch, af);
		else
		{
			tch = (CHAR_DATA *) af->a.spell.t;
			sprintf (buf, "%s:  %s\n", ch->name, command);
			tch->send_to_char (buf);
		}
	}
}

void
command_interpreter (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char *command_args, *p, *social_args;
	int cmd_level = 0;
	int i = 0, echo = 1;
	AFFECTED_TYPE *craft_affect = NULL;
	AFFECTED_TYPE *af;
	extern int second_affect_active;

	if (!ch)
		return;

	*buf = '\0';

	p = argument;

	while (*p == ' ')
		p++;

	if (strchr (p, '%'))
	{
		ch->send_to_char ("Input with the '%' character is not permitted.\n");
		return;
	}

	if (strchr (p, '#') && (!ch->get_trust()) && strncmp (p, "ge", 2) != 0
		&& strncmp (p, "buy", 3) != 0)
	{
		ch->send_to_char ("Input with the '#' character is not permitted.\n");
		return;
	}

	if ((!ch->get_trust()) && strchr (p, '$'))
	{
		ch->send_to_char ("Input with the '$' character is not permitted.\n");
		return;
	}


	/* this is where it crashes on the hour - Grommit */

	if (!ch->room )
	{
		std::ostringstream stream;
		stream << "Error in command_interpreter:commands.cpp. Command \"" << 
			argument << "\" called by \"" << ch->name << "\" with null room. Previously in "
			<< (ch->last_room) << " entering null room from the " << (dirs[ch->from_dir]) << ".";

		system_log(stream.str().c_str(),true);
		return;
	}

	/* end grommit diagnostics to avoid segfaulting on the below for loop */
	
	for (CHAR_DATA *temp_char = ch->room->people; temp_char; temp_char = temp_char->next_in_room)
	{
		
		if (temp_char == ch)
			continue;

		if (!IS_NPC(temp_char))
			continue;

	}

	
	if (ch->desc)
	{
		last_descriptor = ch->desc;
		sprintf (full_last_command, "Last Command Issued, by %s [%d]: %s",
			ch->name, ch->in_room, argument);
		sprintf (last_command, "%s", argument);
	}

	social_args = argument;

	command_args = one_argument (argument, buf);

	if (!*buf)
		return;

	while (*command_args == ' ')
		command_args++;


	for (i = 1; *commands[i].command; i++)
		if (is_abbrev (buf, commands[i].command))
			break;

	if ((craft_affect = is_craft_command (ch, argument)))
		i = 0;

	if (IS_SET (commands[i].flags, C_IMP)) {
		cmd_level = 6;
	}
	else if (IS_SET (commands[i].flags, C_LV5)) {
		cmd_level = 5;
	}
	else if (IS_SET (commands[i].flags, C_LV4)) {
		cmd_level = 4;
	}
	else if (IS_SET (commands[i].flags, C_LV3)) {
		cmd_level = 3;
	}
	else if (IS_SET (commands[i].flags, C_LV2)) {
		cmd_level = 2;
	}
	else if (IS_SET (commands[i].flags, C_LV1)) {
		cmd_level = 1;
	}
	else if (IS_SET (commands[i].flags, C_PBU)) {
			cmd_level = 0;
	}
	else 
		cmd_level = 0;

	if (IS_SET (commands[i].flags, C_GDE)
		&& (IS_NPC (ch) || (!ch->pc->is_guide && !ch->pc->level)))
	{
		ch->send_to_char ("Eh?\n");
		return;
	}
	
	if (IS_SET (commands[i].flags, C_PBU)
		&& (IS_NPC (ch)))
	{
		ch->send_to_char ("Eh?\n");
		return;
	}
	/* 
	Need to pass the CHAR_DATA pointer for the person who made the command and modify
	the following line to test the commanding char's trust against the trust level for
	the command.  - Methuselah
	*/

	if ((!*commands[i].command) 
		|| (cmd_level > ch->get_trust()))
	{
		if (!social (ch, argument))
		{
			echo = number (1, 9);
			if (echo == 1)
				ch->send_to_char ("Eh?\n");
			else if (echo == 2)
				ch->send_to_char ("Huh?\n");
			else if (echo == 3)
				ch->send_to_char ("I'm afraid that just isn't possible...\n");
			else if (echo == 4)
				ch->send_to_char ("I don't recognize that command.\n");
			else if (echo == 5)
				ch->send_to_char ("What?\n");
			else if (echo == 6)
				ch->send_to_char
				("Perhaps you should try typing it a different way?\n");
			else if (echo == 7)
				ch->send_to_char
				("Try checking your typing - I don't recognize it.\n");
			else if (echo == 8)
				ch->send_to_char
				("That isn't a recognized command, craft, or social.\n");
			else
				ch->send_to_char ("Hmm?\n");
		}
		else
		{
			if (!IS_SET (commands[i].flags, C_NWT))
				show_to_watchers (ch, argument);
		}
		return;
	}

	

	if (ch->roundtime)
	{
		sprintf (buf, "You'll need to wait another %d seconds.\n",
			ch->roundtime);
		ch->send_to_char (buf);
		return;
	}

	if (IS_SET (commands[i].flags, C_WLK) &&
		(ch->moves || GET_FLAG (ch, FLAG_LEAVING)
		|| GET_FLAG (ch, FLAG_ENTERING)))
	{
		ch->send_to_char ("Stop traveling first.\n");
		return;
	}

	if (commands[i].min_position > ch->get_position())
	{
		switch (ch->get_position())
		{
		case DEAD:
			if ((!ch->get_trust()))
			{
				ch->send_to_char ("You are dead.  You can't do that.\n");
				return;
			}
		case UNCON:
		case MORT:
			ch->send_to_char ("You're seriously wounded and unconscious.\n");
			return;


		case SLEEP:
			ch->send_to_char ("You can't do that while sleeping.\n");
			return;

		case REST:
			ch->send_to_char ("You can't do that while lying down.\n");
			return;

		case SIT:
			ch->send_to_char ("You can't do that while sitting.\n");
			return;

		}

		return;
	}

	if (!IS_NPC (ch) 
		&& (ch->pc->create_state == STATE_DIED 
			||ch->pc->create_state == STATE_RETIRED)
		&& !IS_SET (commands[i].flags, C_DOA))
	{
		ch->send_to_char ("You can't do that when you're retired or dead.\n");
		return;
	}

	if (!IS_SET (commands[i].flags, C_BLD) && ch->is_blind())
	{
		if (get_equip (ch, WEAR_BLINDFOLD))
			ch->send_to_char ("You can't do that while blindfolded.\n");
		else
			ch->send_to_char ("You can't do that while blind.\n");
		return;
	}

	if ((af = get_affect (ch, MAGIC_AFFECT_PARALYSIS)) &&
		!IS_SET (commands[i].flags, C_PAR) && (!ch->get_trust()))
	{
		ch->send_to_char ("You can't move.\n");
		return;
	}

	if (ch->is_subduee() && !IS_SET (commands[i].flags, C_SUB) && !cmd_level)
	{
		ch->act("$N won't let you.", false, 0, ch->subdue, TO_CHAR);
		return;
	}

	/* Most commands break delays */

	if (ch->delay && !IS_SET (commands[i].flags, C_DEL))
		ch->break_delay();

	/* Send this command to the log */
	if (!second_affect_active && (!IS_NPC (ch) || ch->desc))
	{
		if (IS_SET (commands[i].flags, C_NLG))
			;
		else if (i > 0)
		{			/* Log craft commands separately. */
			if (!str_cmp (commands[i].command, "."))
				player_log (ch, "say", command_args);
			else if (!str_cmp (commands[i].command, ","))
				player_log (ch, "emote", command_args);
			else if (!str_cmp (commands[i].command, ":"))
				player_log (ch, "emote", command_args);
			else if (!str_cmp (commands[i].command, ";"))
				player_log (ch, "wiznet", command_args);
			else
				player_log (ch, commands[i].command, command_args);
		}
	}

	if ((!ch->get_trust()) 
		&& get_affect (ch, MAGIC_HIDDEN) 
		&& !IS_SET (commands[i].flags, C_HID) 
		&& skill_level (ch, "Sneak", 0) < number (1, MAX(100, skill_level(ch, "Sneak", 0)))
		&& ch->would_reveal())
	{
		remove_affect_type (ch, MAGIC_HIDDEN);
		ch->act("$n reveals $mself.", true, 0, 0, TO_ROOM | _ACT_FORMAT);
		ch->act("Your actions have compromised your concealment.", true, 0, 0,
			TO_CHAR);
	}

	/* Execute command */

	if (!IS_SET (commands[i].flags, C_NWT))
		show_to_watchers (ch, social_args);

	if (!i)			/* craft_command */
		craft_command (ch, command_args, craft_affect);
	else

		(*commands[i].proc) (ch, command_args, 0);

	last_descriptor = NULL;
}

	//strips the 'fill' words from an argument. 'at', 'to', 'from' etc.
void
argument_interpreter (char *argument, char *first_arg, char *second_arg)
{
	int look_at, found, begin;

	found = begin = 0;

	do
	{
		/* Find first non blank */
		for (; *(argument + begin) == ' '; begin++);

		/* Find length of first word */
		for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)

			/* Make all letters lower case,
			AND copy them to first_arg */
			*(first_arg + look_at) = tolower (*(argument + begin + look_at));

		*(first_arg + look_at) = '\0';
		begin += look_at;

	}
	while (fill_word (first_arg));

	do
	{
		/* Find first non blank */
		for (; *(argument + begin) == ' '; begin++);

		/* Find length of first word */
		for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)

			/* Make all letters lower case,
			AND copy them to second_arg */
			*(second_arg + look_at) = tolower (*(argument + begin + look_at));

		*(second_arg + look_at) = '\0';
		begin += look_at;

	}
	while (fill_word (second_arg));
}

	//tests if each character in a string is a digit
int
just_a_number (char *buf)
{
	unsigned int i;
	
	if (!*buf)
		return 0;
	
	for (i = 0; i < strlen (buf); i++)
		if (!isdigit (buf[i]))
			return 0;
	
	return 1;
}

	//test if the string contains only digits and/or a single decimal point
int 
is_real_number (std::string str)
{
	if (str.empty())
		return 0;

	bool decimal_found = false;
	for (unsigned int i = 0; i < str.length(); i++)
	{
		if (!isdigit(str[i]))
		{
			if (i == 0 && str[i] == '-')
				continue;

			if (!decimal_found && str[i] == '.')
			{
				decimal_found = true;
				continue;
			}

			return 0;
		}
	}
	return 1;
}

int
fill_word (char *argument)
{
	return (search_block (argument, fill, true) >= 0);
}

/* determine if a given string is an abbreviation of another */
int
is_abbrev (const char *arg1, const char *arg2)
{
	if (!*arg1)
		return (0);

	for (; *arg1; arg1++, arg2++)
		if (tolower (*arg1) != tolower (*arg2))
			return (0);

	return (1);
}

/* case-sensitive; determine if a given string is an abbreviation of another */
int
is_abbrevc (const char *arg1, const char *arg2)
{
	if (!*arg1)
		return (0);

	for (; *arg1; arg1++, arg2++)
		if (*arg1 != *arg2)
			return (0);

	return (1);
}

/* return first 'word' plus trailing substring of input string */
void
half_chop (char *string, char *arg1, char *arg2)
{
	for (; isspace (*string); string++);

	for (; !isspace (*arg1 = *string) && *string; string++, arg1++);

	*arg1 = '\0';

	for (; isspace (*string); string++);

	for (; (*arg2 = *string); string++, arg2++);
}


/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */

SOCIAL_DATA *social_messages = NULL;
static int list_top = -1;

char *
fread_action (FILE * fl)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };

	fgets (buf, MAX_STRING_LENGTH, fl);
	if (feof (fl))
	{
		system_log ("Fread_action() - unexpected EOF!", true);
		abort ();
	}

	if (*buf == '#')
		return 0;

	buf[strlen (buf) - 1] = '\0';

	return duplicateString (buf);
}

void
boot_social_messages (void)
{
	FILE *fl;
	char *social_command;
	int hide;
	int min_pos;

	if (!(fl = fopen (SOCMESS_FILE, "r")))
	{
		perror ("boot_social_messages");
		abort ();
	}

#define MAX_SOCIALS		200

	social_messages = new SOCIAL_DATA[MAX_SOCIALS];

	for (list_top = 0;; list_top++)
	{

		if (!(social_command = fread_action (fl)))
			break;

		fscanf (fl, " %d ", &hide);
		fscanf (fl, " %d \n", &min_pos);

		if (list_top >= MAX_SOCIALS - 1)
		{
			break;
		}

		social_messages[list_top].social_command = social_command;
		social_messages[list_top].hide = hide;
		social_messages[list_top].min_victim_position = min_pos;
		social_messages[list_top].char_no_arg = fread_action (fl);
		social_messages[list_top].others_no_arg = fread_action (fl);
		social_messages[list_top].char_found = fread_action (fl);

		/* if no char_found, the rest is to be ignored */

		if (!social_messages[list_top].char_found)
			continue;

		social_messages[list_top].others_found = fread_action (fl);
		social_messages[list_top].vict_found = fread_action (fl);
		social_messages[list_top].not_found = fread_action (fl);
		social_messages[list_top].char_auto = fread_action (fl);
		social_messages[list_top].others_auto = fread_action (fl);
	}

	fclose (fl);
}

int
social (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	SOCIAL_DATA *action;
	CHAR_DATA *victim;
	int i;

	argument = one_argument (argument, buf);

	for (i = 0; i < list_top; i++)
	{
		if (is_abbrev (buf, social_messages[i].social_command))
			break;
	}

	if (i == list_top)
		return 0;

	action = &social_messages[i];

	if (action->char_found)
		one_argument (argument, buf);
	else
		*buf = '\0';

	if (!*buf)
	{
		ch->send_to_char (action->char_no_arg);
		ch->send_to_char ("\n");
		if (action->others_no_arg)
			ch->act(action->others_no_arg, action->hide, 0, 0, TO_ROOM);

		return 1;
	}

	if (!(victim = get_char_room_vis (ch, buf)))
	{
		ch->send_to_char (action->not_found);
		ch->send_to_char ("\n");
	}

	else if (victim == ch)
	{
		ch->send_to_char (action->char_auto);
		ch->send_to_char ("\n");
		if (action->others_auto)
			ch->act(action->others_auto, action->hide, 0, 0, TO_ROOM);
	}

	else if (victim->get_position() < action->min_victim_position)
		ch->act("$N is not in a proper position for that.",
		false, 0, victim, TO_CHAR);

	else
	{
		if (action->char_found)
			ch->act(action->char_found, false, 0, victim, TO_CHAR);

		if (action->others_found)
			ch->act(action->others_found, action->hide, 0, victim, TO_NOTVICT);

		if (action->vict_found)
			ch->act(action->vict_found, action->hide, 0, victim, TO_VICT);
	}

	return 1;
}

void
do_commands (CHAR_DATA * ch, char *argument, int cmd)
{
	int col_no = 0;
	int cmd_no;
	int cmd_level;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	*buf = '\0';

	for (cmd_no = 0; *commands[cmd_no].command; cmd_no++)
	{

		cmd_level = 0;

		if (IS_SET (commands[cmd_no].flags, C_IMP))
			cmd_level = 6;
		if (IS_SET (commands[cmd_no].flags, C_LV5))
			cmd_level = 5;
		else if (IS_SET (commands[cmd_no].flags, C_LV4))
			cmd_level = 4;
		else if (IS_SET (commands[cmd_no].flags, C_LV3))
			cmd_level = 3;
		else if (IS_SET (commands[cmd_no].flags, C_LV2))
			cmd_level = 2;
		else if (IS_SET (commands[cmd_no].flags, C_LV1))
			cmd_level = 1;
		
		if (cmd_level)
			continue;

		if (cmd_level > ch->get_trust())
			continue;

		if (IS_SET (commands[cmd_no].flags, C_XLS))
			continue;

		sprintf (buf + strlen (buf), "%-9.9s ", commands[cmd_no].command);

		if (++col_no >= 7)
		{
			strcat (buf, "\n");
			ch->send_to_char (buf);
			*buf = '\0';
			col_no = 0;
		}
	}

	if (*buf)
	{
		strcat (buf, "\n");
		ch->send_to_char (buf);
	}
}

void
do_immcommands (CHAR_DATA * ch, char *argument, int cmd)
{
	int col_no = 0;
	int cmd_no;
	int cmd_level;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	*buf = '\0';

	for (cmd_no = 0; *commands[cmd_no].command; cmd_no++)
	{

		cmd_level = 0;

		if (IS_SET (commands[cmd_no].flags, C_LV5))
			cmd_level = 5;
		else if (IS_SET (commands[cmd_no].flags, C_LV4))
			cmd_level = 4;
		else if (IS_SET (commands[cmd_no].flags, C_LV3))
			cmd_level = 3;
		else if (IS_SET (commands[cmd_no].flags, C_LV2))
			cmd_level = 2;
		else if (IS_SET (commands[cmd_no].flags, C_LV1))
			cmd_level = 1;

		if (!cmd_level)
			continue;

		if (cmd_level > ch->get_trust())
			continue;

		sprintf (buf + strlen (buf), "%-11.11s ", commands[cmd_no].command);

		if (++col_no >= 6)
		{
			strcat (buf, "\n");
			ch->send_to_char (buf);
			*buf = '\0';
			col_no = 0;
		}
	}

	if (*buf)
	{
		strcat (buf, "\n");
		ch->send_to_char (buf);
	}
}
