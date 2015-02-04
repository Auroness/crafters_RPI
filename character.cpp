//////////////////////////////////////////////////////////////////////////////
//
/// character.cpp : Character Module
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>
#include <string>
#include <sstream>
#include <sys/time.h>
#include "server.h"

#include "account.h"
#include "character.h"
#include "protos.h"
#include "group.h"
#include "structs.h"
#include "utils.h"
#include "utility.h"

extern rpie::server engine;
extern std::map<std::string, SKILL_DATA*> skill_data_map;
extern std::map<std::string, CLAN_DATA*> clan_data_map;
extern const char *dirs[];
extern const char *locations[];
extern const char *verbal_hunger[];
extern const char *verbal_thirst[];

CHAR_DATA * new_char (int pc_type)
{
	CHAR_DATA *ch;

	ch = new char_data();
	
	ch->char_clan_map.clear();
	ch->skill_map.clear();
	ch->body_coverage.clear();
	
	if (pc_type)
	{
		ch->pc = new pc_data();
	}
	else
	{
		ch->mob = new mob_data();
	}
	return ch;
}

void free_char (CHAR_DATA *&ch)
{
	delete ch;
	ch = NULL;
}

char_data::char_data ()
{
	this->in_room = 0;
	this->room = NULL;
	this->deleted = 0;
	this->coldload_id = 0;
	this->body_type = 0;
	this->flags = 0;
	this->move_points = 0;
	this->hit_points = 0;
	this->speaks = 0;
	this->alarm = 0;
	this->subdue = NULL;
	this->vehicle = NULL;
	this->str = 0;
	this->intel = 0;
	this->wil = 0;
	this->dex = 0;
	this->con = 0;
	this->aur = 0;
	this->agi = 0;
	this->luk = 0;
	this->tmp_str = 0;
	this->tmp_intel = 0;
	this->tmp_wil = 0;
	this->tmp_dex = 0;
	this->tmp_con = 0;
	this->tmp_aur = 0;
	this->tmp_agi = 0;
	this->tmp_luk = 0;
	
	this->hour_affects = NULL;
	this->equip = NULL;
	this->desc = NULL;
	this->next_in_room = NULL;
	this->next = NULL;
	this->next_fighting = NULL;
	this->next_assist = NULL;
	this->assist_pos = 0;
	this->following = NULL;
	this->pc = NULL;
	this->mob = NULL;
	this->moves = NULL;
	this->hit = 0;
	this->max_hit = 0;
	this->move = 0;
	this->max_move = 0;
	this->remembers = NULL;
	this->affected_by = 0;
	this->position = STAND;
	this->default_pos = STAND;
	this->action = 0;
	this->carry_weight = 0;
	this->carry_items = 0;
	this->delay_type = 0;
	this->delay = 0;
	this->delay_who = NULL;
	this->delay_who2 = NULL;
	this->delay_ch = NULL;
	this->delay_obj = NULL;
	this->delay_info1 = 0;
	this->delay_info2 = 0;
	this->delay_info3 = 0;
	this->delay_info4 = 0;
	this->intoxication = 0;
	this->hunger = 2400;
	this->thirst = 24;
	this->last_room = 0;
	this->keywords = NULL;
	this->name = NULL;
	this->short_descr = NULL;
	this->long_descr = NULL;
	this->pmote_str = NULL;
	this->voice_str = NULL;
	this->description = NULL;
	this->sex = 0;
	this->race = -1;	//race == 0 could be common human
	this->color = 0;
	this->speed = 0;
	this->age = 0;
	this->height = 0;
	this->frame = 0;
	this->time_str.birth = 0;
	this->time_str.logon = 0;
	this->time_str.played = 0;
	this->writes = 0;
	this->roundtime = 0;
	this->right_hand = NULL;
	this->left_hand = NULL;
	this->plr_flags = 0;
	this->from_dir = 0;
	this->sighted = NULL;
	this->balance = 0;
	this->travel_str = NULL;
	this->body_proto = 0;
	this->bmi = 0;
	this->size = 0;
	this->guardian_mode = 0;
	this->dmote_str = NULL;
	this->craft_index = 0;
	
	for (int i = 0; i < 100; i++) {
		this->enforcement[i] = 0;
	}
	this->petition_flags = 0;
	this->magic_sent = 0;
	
	init_body_loc(this);
	
	soma_affect.clear();
}


char_data::~char_data ()
{
	struct memory_data *mem;
	if (this->pc)
	{
		delete this->pc;
	}

	if (this->mob)
	{
		delete this->mob;
	}

	if (this->delay_who && !isdigit (*this->delay_who) && *this->delay_who)
	{
		free_mem (this->delay_who);
		this->delay_who = NULL;
	}

	if (this->delay_who2 && !isdigit (*this->delay_who2) && *this->delay_who2)
	{
		free_mem (this->delay_who2);
		this->delay_who2 = NULL;
	}

	if (this->voice_str && *this->voice_str)
	{
		free_mem (this->voice_str);
		this->voice_str = NULL;
	}

	if (this->travel_str && *this->travel_str)
	{
		free_mem (this->travel_str);
		this->travel_str = NULL;
	}

	if (this->pmote_str && *this->pmote_str)
	{
		free_mem (this->pmote_str);
		this->pmote_str = NULL;
	}

	if (this->body_coverage.size() > 0)
	{
		this->body_coverage.clear();
	}
	
	
	while (this->remembers)
	{
		mem = this->remembers;
		this->remembers = mem->next;
		free_mem (mem);
		mem = NULL;
	}


	while (this->hour_affects)
		affect_remove (this, this->hour_affects);

	clear_pmote();
	clear_dmote();

	body_coverage.clear();
	
	
		if (this->name && *this->name )
		{
			free_mem (this->name);
			this->name = NULL;
		}

		if (this->keywords && *this->keywords )
		{
			free_mem (this->keywords);
			this->keywords = NULL;
		}

		if (this->short_descr && *this->short_descr)
		{
			free_mem (this->short_descr);
			this->short_descr = NULL;
		}

		if (this->long_descr && *this->long_descr)
		{
			free_mem (this->long_descr);
			this->long_descr = NULL;
		}

		if (this->description && *this->description)
		{
			free_mem (this->description);
			this->description = NULL;
		}
	
}

pc_data::pc_data()
{
	this->nanny_state = 0;
	this->role = 0;
	this->admin_loaded = false;
	this->creation_comment = NULL;
	this->imm_enter = NULL;
	this->imm_leave = NULL;
	this->site_lie = NULL;
	this->account_name = NULL;
        this->account_id = 0;
	this->msg = NULL;
	this->dreams = NULL;
	this->dreamed = NULL;
	this->staff_notes = 0;
	this->mortal_mode = 0;
	this->create_state = 0;
	this->edit_obj = 0;
	this->edit_mob = 0;
	this->edit_player = NULL;
	this->edit_portal = 0;
	this->edit_craft = NULL;
	this->load_count = 0;
	this->start_str = 0;
	this->start_dex = 0;
	this->start_con = 0;
	this->start_wil = 0;
	this->start_aur = 0;
	this->start_intel = 0;
	this->start_agi = 0;
	this->level = 0;
	this->time_last_activity = 0;
	this->is_guide = 0;
	this->profession = -1;
	this->app_cost = 0;
	this->chargen_flags = 0;
	this->last_global_pc_msg = 0;
	this->writing_on = NULL;
	this->special_role = NULL;
	
	for (int i = 0; i < MAX_CRAFTS; i++)
	{
		this->crafts[i] = 0;
	}
	this->owner = NULL;
	this->last_logon = 0;
	this->last_logoff = 0;
	this->last_disconnect = 0;
	this->last_connect = 0;
	this->last_died = 0;
	this->plan = 0;
	this->goal = 0;
}

pc_data::~pc_data()
{
	DREAM_DATA *dream;
	ROLE_DATA *trole;

	while (this->dreams)
	{
		dream = this->dreams;

		this->dreams = this->dreams->next;

		if (dream->dream && *dream->dream)
		{
			free_mem (dream->dream);
			dream->dream = NULL;
		}

		free_mem (dream);
		dream = NULL;
	}
	while (this->dreamed)
	{
		dream = this->dreamed;
		this->dreamed = this->dreamed->next;
		if (dream->dream && *dream->dream)
		{
			free_mem (dream->dream);
			dream->dream = NULL;
		}
		free_mem (dream);
		dream = NULL;
	}

	if ((trole = this->special_role) != NULL)
	{
		if (trole->summary && *trole->summary)
			free_mem (trole->summary);
		if (trole->body && *trole->body)
			free_mem (trole->body);
		if (trole->date && *trole->date)
			free_mem (trole->date);
		if (trole->poster && *trole->poster)
			free_mem (trole->poster);
		free_mem (trole);
		this->special_role = NULL;
	}

	if (this->account_name && *this->account_name)
	{
		free_mem (this->account_name);
		this->account_name = NULL;
	}

	if (this->site_lie && *this->site_lie)
	{
		free_mem (this->site_lie);
		this->site_lie = NULL;
	}

	if (this->imm_leave && *this->imm_leave)
	{
		free_mem (this->imm_leave);
		this->imm_leave = NULL;
	}

	if (this->imm_enter && *this->imm_enter)
	{
		free_mem (this->imm_enter);
		this->imm_enter = NULL;
	}

	if (this->creation_comment && *this->creation_comment)
	{
		free_mem (this->creation_comment);
		this->creation_comment = NULL;
	}

	if (this->msg && *this->msg)
	{
		free_mem (this->msg);
		this->msg = NULL;
	}

	if (this->plan)
	{
		delete this->plan;
		this->plan = 0;
	}
	
	if (this->goal)
	{
		delete this->goal;
		this->goal = 0;
	}
	
	this->owner = NULL;
}

mob_data::mob_data()
{
	this->owner = NULL;
	this->damnodice = 0;
	this->damroll = 0;
	this->min_height = 0;
	this->max_height = 0;
	this->size = 0;
	this->action = 0;
	this->profession = 0;
	this->nVirtual = 0;
	this->zone = 0;
	this->spawnpoint = 0;
	this->merch_seven = 0;
	this->shop = 0;
	this->vehicle_type = 0;
	this->access_flags = 0;
	this->noaccess_flags = 0;
	this->currency_type = 0;
	this->jail = 0;
	this->cell_1 = 0;
	this->cell_2 = 0;
	this->cell_3 = 0;
	this->morph_type = 0;
	this->clock = 0;
	this->morph_time = 0;
	this->morphto = 0;

}

mob_data::~mob_data()
{
	if (this->owner && *this->owner)
	{
		free_mem(this->owner);
	}
}

// This function is only intended to be called by redefine_mobiles()
void char_data::partial_deep_copy (CHAR_DATA *proto)
{
	if (this->keywords)
	{
		free_mem(this->keywords);
	}
	this->keywords = duplicateString(proto->keywords);

	if (this->short_descr)
	{
		free_mem(this->short_descr);
	}
	this->short_descr = duplicateString(proto->short_descr);

	if (this->long_descr)
	{
		free_mem(this->long_descr);
	}
	this->long_descr = duplicateString(proto->long_descr);

	if (this->description)
	{
		free_mem(this->description);
	}
	this->description = duplicateString(proto->description);

	
	this->action = proto->action;
	this->mob->damnodice = proto->mob->damnodice;
	this->mob->damsizedice = proto->mob->damsizedice;
	this->position = proto->position;
	this->default_pos = proto->default_pos;

	this->str = proto->str;
	this->dex = proto->dex;
	this->intel = proto->intel;
	this->aur = proto->aur;
	this->con = proto->con;
	this->wil = proto->wil;
	this->agi = proto->agi;
	this->luk = proto->luk;

	this->flags = proto->flags;

	this->hit = proto->hit;
	this->max_hit = proto->max_hit;
	this->move = proto->move;
	this->max_move = proto->max_move;
	this->mob->damroll = proto->mob->damroll;
	this->sex = proto->sex;
	this->mob->zone = proto->mob->zone;
	this->mob->merch_seven = proto->mob->merch_seven;
	this->mob->vehicle_type = proto->mob->vehicle_type;
	this->race = proto->race;
	this->mob->access_flags = proto->mob->access_flags;
	this->mob->action = proto->mob->action;
	this->mob->profession = proto->mob->profession;
	this->speaks = proto->speaks;
	this->age = proto->age;
	this->speed = proto->speed;

	this->str = proto->str;
	this->dex = proto->dex;
	this->con = proto->con;
	this->wil = proto->wil;
	this->aur = proto->aur;
	this->intel = proto->intel;

	this->mob->currency_type = proto->mob->currency_type;
}

void char_data::deep_copy (CHAR_DATA *copy_from)
{
	// Lazy way of getting everything non-dynamic across. One advantage of this approach is that people who add members to char_data don't have to add them here unless they use dynamic memory.
	mob_data *tmob = this->mob;
	pc_data *tpc = this->pc;
	memcpy (this, copy_from, sizeof(CHAR_DATA));
	this->mob = tmob;
	this->pc = tpc;

	if (copy_from->delay_who)
	{
		this->delay_who = duplicateString(copy_from->delay_who);
	}

	if (copy_from->delay_who2)
	{
		this->delay_who2 = duplicateString(copy_from->delay_who2);
	}

	if (copy_from->keywords)
	{
		this->keywords = duplicateString(copy_from->keywords);
	}

	if (copy_from->name)
	{
		this->name = duplicateString(copy_from->name);
	}

	if (copy_from->short_descr)
	{
		this->short_descr = duplicateString(copy_from->short_descr);
	}

	if (copy_from->long_descr)
	{
		this->long_descr = duplicateString(copy_from->long_descr);
	}

	if (copy_from->pmote_str)
	{
		this->pmote_str = duplicateString(copy_from->pmote_str);
	}

	if (copy_from->voice_str)
	{
		this->voice_str = duplicateString(copy_from->voice_str);
	}

	if (copy_from->travel_str)
	{
		this->travel_str = duplicateString(copy_from->travel_str);
	}
	
	if (copy_from->description)
	{
		this->description = duplicateString(copy_from->description);
	}

	if (copy_from->travel_str)
	{
		this->travel_str = duplicateString(copy_from->travel_str);
	}

	if (copy_from->voice_str)
	{
		this->voice_str = duplicateString(copy_from->voice_str);
	}
	
	if (copy_from->dmote_str)
	{
		this->dmote_str = duplicateString(copy_from->dmote_str);
	}

	if (copy_from->body_coverage.size() > 0)
	{
		this->body_coverage = copy_from->body_coverage;
	}
	
	if (copy_from->pc && this->pc)
	{
		this->pc->deep_copy(copy_from->pc);
	}

	if (copy_from->mob && this->mob)
	{
		this->mob->deep_copy(copy_from->mob);
	}
	
	
}

std::pair<int, std::string> char_data::reportWhere(int RPP)
{
	
	bool underwater = (room->terrain_type == SECT_RIVER
					   || room->terrain_type == SECT_LAKE
					   || room->terrain_type == SECT_OCEAN
					   || room->terrain_type == SECT_REEF
					   || room->terrain_type == SECT_UNDERWATER);
	
	int characterNameColour = 0;
	
	if ((pc && pc->level > 1 && !fighting)) {
		characterNameColour = 5;
	}
	else if ((pc && pc->level == 1 && !fighting)) {
		characterNameColour = 9;
	}
	else if (fighting) {
		characterNameColour = 1;
	}
	else if (underwater) {
		characterNameColour = 4;
	}
	else if (IS_SET (flags, FLAG_ISADMIN)) {
		characterNameColour = 6;
	}
	else if (IS_SET (plr_flags, NEW_PLAYER_TAG)) {
		characterNameColour = 2;
	}
	else if (IS_GUIDE (this)) { //If the person is a guide
		characterNameColour = 3;
	}
	else {
		characterNameColour = 0;
	}
	
	char characterState = '_';
	
	switch (position) {
		case DEAD:
		case MORT:
			characterState = 'X';
			break;
		case UNCON:
			characterState = 'U';
			break;
		case SLEEP:
			characterState = 's';
			break;
		default:
			if (!IS_NPC (this) 
				&& !desc 
				&& !pc->admin_loaded)
			{
				characterState = 'L';
			}
			else if (IS_SET (action, PLR_QUIET) && !IS_NPC (this)) {
				characterState = 'e';
			}
			else if (get_affect (this, MAGIC_HIDDEN)) {
				characterState = 'h';
			}
			else if (desc && desc->idle) {
				characterState = 'i';
			}
	}
	
	std::stringstream reportStream;
	
	reportStream << "#" << characterNameColour;
	
	
	char buf[MAX_STRING_LENGTH] = { '\0' };
	if (IS_NPC(this) && (desc))
	{
		sprintf(buf, "%-10.10s-#5%-6.6s#0", name, desc->original->name);

	}
	else 
		sprintf(buf, "%-10.10s",name);
	
	
	int blankLineSize = 17;  
	if (buf)
	{
	    blankLineSize -= strlen(buf); 
	}
	
	if (blankLineSize < 1) 
	{
		std::string tempString = buf;  //never true on null string, as it would still be 15
		reportStream << tempString.substr(0,19);//accounts for the invisible characters
	}
	else 
	{
		reportStream << ((buf) ? buf : "(null)");
		for (size_t i = 0; i < blankLineSize; i++)
		{
			reportStream << " ";
		}
	}
	
	if (RPP < 10) {
		reportStream << "#3[ " << RPP << "]";
	}
	else {
		reportStream << "#3[" << RPP << "]";
	}
	
	reportStream << " #0" << characterState;
	
	
  // Right-aligned room number and room name
  reportStream << "#2[";
  reportStream.width(6);
  reportStream << std::right << room->nVirtual;
  reportStream << "]#6 " << room->name << "#0";
	
	return std::make_pair(room->nVirtual, reportStream.str());
}

void pc_data::deep_copy (pc_data *copy_from)
{
	memcpy(this, copy_from, sizeof(pc_data));

	if (copy_from->creation_comment)
	{
		this->creation_comment = duplicateString(copy_from->creation_comment);
	}

	if (copy_from->imm_enter)
	{
		this->imm_enter = duplicateString(copy_from->imm_enter);
	}

	if (copy_from->imm_leave)
	{
		this->imm_leave = duplicateString(copy_from->imm_leave);
	}

	if (copy_from->site_lie)
	{
		this->site_lie = duplicateString(copy_from->site_lie);
	}

	if (copy_from->account_name)
	{
		this->account_name = duplicateString(copy_from->account_name);
	}

	if (copy_from->msg)
	{
		this->msg = duplicateString(copy_from->msg);
	}
	
	if (copy_from->plan)
	{
		this->plan = new std::string (*copy_from->plan);
	}
	
	if (copy_from->goal)
	{
		this->goal = new std::string (*copy_from->goal);
	}
}

void mob_data::deep_copy (mob_data *copy_from)
{
	memcpy(this, copy_from, sizeof(mob_data));

	if (copy_from->owner)
	{
		this->owner = duplicateString(copy_from->owner);
	}
}

bool char_data::isLevelFivePC()
{
	if (pc && pc->level == 5) {
		return true;
	}

	if (desc && desc->acct) {
		MYSQL_RES *result;
		MYSQL_ROW row;

		mysql_safe_query ("SELECT level FROM pfiles WHERE accountId = %d", desc->acct->id);
		result = mysql_store_result (database);

		if(!result || !mysql_num_rows(result)) {
			if( result != NULL) {
				mysql_free_result(result);
			}
			return false;
		}

		row = mysql_fetch_row (result);

		do {
			if(atoi(row[0]) == 5) {
				mysql_free_result(result);
				return true;
			}
			row = mysql_fetch_row (result);
		}while(row);
		mysql_free_result(result);
	}
	return false;
}

char * char_data::getLevelFiveName()
{
	if (pc && pc->level == 5)
	{
		return name;
	}

	if (desc && desc->acct)
	{
		MYSQL_RES *result;
		MYSQL_ROW row;

		mysql_safe_query ("SELECT level,name FROM pfiles WHERE accountId = %d", desc->acct->id);
		result = mysql_store_result (database);

		if (!result || !mysql_num_rows(result)) {
			if (result != NULL) {
				mysql_free_result( result );
			}
			return NULL;
		}

		row = mysql_fetch_row (result);

		do {
			if(atoi(row[0]) == 5) {
				return row[1];
			}
			row = mysql_fetch_row (result);
		}while(row);
	}
	return NULL;
}

bool char_data::hasMortalBody()
{
	if (!pc)
	{
		return true;
	}
	return (pc->level == 0 ? true : false);
}

void char_data::send_to_char(const char *message)
{
	DESCRIPTOR_DATA *d;
	
	if (!message)
		return;
	
	d = this->desc;
	
	if (!d && !pc)
		return;
	
	/* Check to see if real PC owner is still online */
	
	if (!d && pc && pc->owner)
		for (d = descriptor_list; d; d = d->next)
			if (d == pc->owner)
				break;
	
	if (!d)
		return;
	
	if (d->character && IS_SET (d->character->action, PLR_QUIET))
		return;
	
	write_to_q (message, &d->output);
}

void char_data::make_quiet()
{
	this->action |= PLR_QUIET;
}

void char_data::save_char_mysql()
{
	AFFECTED_TYPE *af;
	MYSQL_RES *result;
	int err_res;
	char buf[2*MAX_STRING_LENGTH]; //affects is accumulated here first
	char buflong[2*MAX_STRING_LENGTH];
	char tclan_name[MAX_STRING_LENGTH]= { '\0' };
	char tclan_rank[MAX_STRING_LENGTH]= { '\0' };
	std::string planstr;
	std::string goalstr;
	std::string dmotestr;
	std::string travelstr;
	std::string voicestr;
	int hooded = 0;
	char *skill_name;
	char *strtmp;
	std::string skill_buf;
	char buftmp[MAX_STRING_LENGTH] = {'\0'};
	std::map<std::string, int>::iterator skill_it;
	std::map<std::string, std::string>::iterator clan_it;
	
	std::string desc_temp;
	std::string sdesc_temp;
	std::string ldesc_temp;
	std::string clan_temp;
	std::string affects_temp;
	std::string role_summary;
	std::string role_body;
	std::string role_date;
	std::string role_poster;
	std::string full_query;
	std::string creation_temp;
	std::string immenter_temp;
	std::string immleave_temp;
	std::string msg_temp;
	int tskill;
	
	
	if (!pc || IS_SET (flags, FLAG_GUEST))
		return;
	
	/* adjust last_logoff to now for offline healing tracking in event of crash, but ONLY if PC is online. */
	if (desc)
		pc->last_logoff = time(0);
	
	
	
	skill_buf.assign("");
	planstr.assign("");
	goalstr.assign("");
	dmotestr.assign("");
	travelstr.assign("");
	clan_temp.assign("");
	full_query.assign("");
	affects_temp.assign("");  
	
		//escape strings as needed
	if (description)
	{
		mysql_real_escape_string (database, buflong, description, strlen(description));
		desc_temp = duplicateString(buflong);
	}
	
	if(short_descr)
	{
		mysql_real_escape_string (database, buflong, short_descr, strlen(short_descr));
		sdesc_temp = duplicateString(buflong);
	}
	
	if(long_descr)
	{
		mysql_real_escape_string (database, buflong, long_descr, strlen(long_descr));
		ldesc_temp = duplicateString(buflong);
	}
	
		//plan is a string
	if (pc->plan && !pc->plan->empty())
	{
		strtmp = (char *)pc->plan->c_str();
		mysql_real_escape_string (database, buflong, strtmp, strlen(strtmp));
		planstr.assign(buflong);
	}
	
		//goal is a string
	if (pc->goal && !pc->goal->empty())
	{
		strtmp = (char *)pc->goal->c_str();
		mysql_real_escape_string (database, buflong, strtmp, strlen(strtmp));
		goalstr.assign(buflong);
	}
	
	if (pc->creation_comment)
	{
		mysql_real_escape_string (database, buflong, pc->creation_comment, strlen(pc->creation_comment));
		creation_temp.assign(buflong);
	}
	
	if (pc->msg)
	{
		mysql_real_escape_string (database, buflong, pc->msg, strlen(pc->msg));
		msg_temp = duplicateString(buflong);
	}
	
	if (dmote_str)
	{
		mysql_real_escape_string (database, buflong, dmote_str, strlen(dmote_str));
		dmotestr.assign(buflong);
	}
	
	if (travel_str)
	{
		mysql_real_escape_string (database, buflong, travel_str, strlen(travel_str));
		travelstr.assign(buflong);
	}
	
	if (pc->imm_enter)
	{
		mysql_real_escape_string (database, buflong, pc->imm_enter, strlen(pc->imm_enter));
		immenter_temp.assign(buflong);
	}
	
	if (pc->imm_leave)
	{
		mysql_real_escape_string (database, buflong, pc->imm_leave, strlen(pc->imm_leave));
		immleave_temp.assign(buflong);
	}
	if (voice_str)
	{
		mysql_real_escape_string (database, buflong, voice_str, strlen(voice_str));
		voicestr.assign(buflong);
	}
	
	if (!speaks)
		speaks = "Westron";
	
	
		//ROLES
	if (pc->special_role)
	{
		mysql_real_escape_string (database, buflong, pc->special_role->summary, strlen(pc->special_role->summary));
		role_summary.assign(buflong);
		
		mysql_real_escape_string (database, buflong, pc->special_role->body, strlen(pc->special_role->body));
		role_body.assign(buflong);
		
		mysql_real_escape_string (database, buflong, pc->special_role->date, strlen(pc->special_role->date));
		role_date.assign(buflong);
		
		mysql_real_escape_string (database, buflong, pc->special_role->poster, strlen(pc->special_role->poster));
		role_poster.assign(buflong);
	}
	else 
	{
		role_summary.assign("~");
		role_body.assign("~");
		role_date.assign("~");
		role_poster.assign("~");
	}
	
	
	
		//first we accumulate some values to store in a buffers for later use
	
	for (skill_it = skill_map.begin(); skill_it != skill_map.end(); skill_it++)
	{
		if (skill_data_map.find(skill_it->first) != skill_data_map.end())
		{
			tskill = skill_it->second;
			if (tskill > 0)
			{
				skill_name = strdup(skill_it->first.c_str());
				
				skill_buf.append(skill_name);
				skill_buf.append(" ");
				sprintf(buftmp, "%d",tskill); 
				skill_buf.append(buftmp);
				skill_buf.append(" ");
			}
		}
	}
	
		//CLANS
	for (clan_it = char_clan_map.begin(); clan_it != char_clan_map.end(); clan_it++)
	{
		if (clan_data_map.find(clan_it->first)!=clan_data_map.end())
		{
			sprintf(tclan_name, "%s", clan_it->first.c_str());
			sprintf(tclan_rank, "%s", clan_it->second.c_str());
			
			clan_temp.append(tclan_name);
			clan_temp.append(" \"");
			clan_temp.append(tclan_rank);
			clan_temp.append("\"");
			clan_temp.append(" ");
		}
	}
	
		//Affects (clans, crafts, misc)
	
		//**** TODO: These probaly need re-written - restore when we get affects working properly ***/
		//only works with paydays for now
	*buf = '\0';
	for (af = hour_affects; af; af = af->next)
	{
		if (
			
			//skip 8000 - 18000
			(af->type < CRAFT_FIRST || af->type > CRAFT_LAST)
			
			//skip special cases
			&& af->type != MAGIC_CLAN_NOTIFY 
			&& af->type != MAGIC_NOTIFY 
			&& af->type != MAGIC_WATCH1 
			&& af->type != MAGIC_WATCH2 
			&& af->type != MAGIC_WATCH3 
			&& af->type != AFFECT_SHADOW 			
			)
			
			sprintf (buf + strlen (buf), "Affect	%d %d %d %d %d %d %ld\n",
					 af->type, af->a.spell.duration, af->a.spell.modifier,
					 af->a.spell.location, af->a.spell.bitvector,
					 af->a.spell.sn, af->a.spell.t);
		
		/** skipping crafts for now 
		 else if ( af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST  && af->a.craft &&
		 af->a.craft->subcraft && af->a.craft->subcraft->subcraft_name )
		 sprintf (buf + strlen (buf), "Subcraft     '%s'\n",
		 af->a.craft->subcraft->subcraft_name);
		 ***/
	}
	if (*buf)
		affects_temp.assign(buf);
	else 
		affects_temp.assign("");
	
	/************* above to be restored when affects are working properly ***/
	
	*buf = '\0';
	
	
	mysql_safe_query ("SELECT name FROM pfiles WHERE name = '%s'", name);
	result = mysql_store_result (database);
	
		//create a basic record if one doesn't exist
	if (mysql_num_rows (result) == 0)
	{
		sprintf(buftmp, "INSERT INTO pfiles (name, keywords, account, accountId) VALUES ('%s', '%s', '%s', %d)",
				name,
				keywords,
				pc->account_name,
				pc->account_id);
		mysql_safe_query(buftmp);
	}
	
		// Update PC record.
	if (result)
	{
		mysql_free_result (result);
			//nanny_state is a bitflag so foramt is %ld 
		sprintf(buftmp, "UPDATE pfiles SET keywords = '%s', account = '%s', accountId = %d, sdesc = '%s', ldesc = '%s', description = '%s', msg = '%s', create_comment = '%s', create_state = %d, nanny_state = %ld, role_check = %d, role_summary = '%s', role_body = '%s', role_date = '%s', role_poster = '%s', role_cost = %d, app_cost = %d, level = %d, sex = %d, race = %d, room = %d WHERE name = '%s'",
				keywords,
				pc->account_name,
				pc->account_id,
				sdesc_temp.c_str(),
				ldesc_temp.c_str(),
				desc_temp.c_str(),
				msg_temp.c_str(),
				creation_temp.c_str(),
				pc->create_state,
				pc->nanny_state,
				pc->role,
				role_summary.c_str(),
				role_body.c_str(),
				role_date.c_str(),
				role_poster.c_str(),
				pc->special_role ? pc->special_role->cost : 0,
				pc->app_cost,
				pc->level,
				sex,
				race,
				in_room,
				name);
		
		full_query.assign(buftmp);
		err_res = mysql_safe_query((char*)full_query.c_str());
		
		sprintf(buftmp, "UPDATE pfiles SET str = %d, intel = %d, wil = %d, con = %d, dex = %d, aur = %d, agi = %d, luk = %d, start_str = %d, start_intel = %d, start_wil = %d, start_con = %d, start_dex = %d, start_aur = %d, start_agi = %d, start_luk = %d, played = %d, birth = %d, time = %d, hit = %d, maxhit = %d, move = %d, maxmove = %d,  color = %d WHERE name = '%s'",
				str,
				intel,
				wil,
				con,
				dex,
				aur,
				agi,
				luk,
				pc->start_str,
				pc->start_intel,
				pc->start_wil,
				pc->start_con,
				pc->start_dex,
				pc->start_aur,
				pc->start_agi,
				pc->start_luk,
				(int) (time_str.played + time(0) - time_str.logon),
				(int) time_str.birth,
				(int) time(0),
				hit,
				max_hit,
				move,
				max_move,
				color,
				name);
		
		full_query.assign(buftmp);
		err_res = mysql_safe_query((char*)full_query.c_str());
		
		sprintf(buftmp, "UPDATE pfiles SET speaks = %d, flags = %d, plrflags = %d, speed = %d, coldload = %d, affectedby = %d, affects = '%s', age = %d, intoxication = %d, hunger = %d, thirst = %d, height = %d, frame = %d,  lastregen = %d, lastroom = %d, lastlogon = %d, lastlogoff = %d, lastdis = %d, lastconnect = %d, lastactivity = %d, lastdied = %d WHERE name = '%s'",
				lookup_skill_id(speaks),
				(int) flags,
				(int) plr_flags,
				speed,
				(int) coldload_id,
				(int) affected_by,
				affects_temp.c_str(),
				age,
				intoxication,
				hunger,
				thirst,
				height,
				frame,
				(int)lastregen,
				last_room,
				(int)pc->last_logon,
				(int) pc->last_logoff,    /* use current time as last logoff so if there is a crash, wounds heal properly not spam heal */
				(int) pc->last_disconnect,
				(int) pc->last_connect,
				(int) pc->time_last_activity,
				(int) pc->last_died,
				name);
		
		full_query.assign(buftmp);
		err_res = mysql_safe_query((char*)full_query.c_str());
		
		sprintf(buftmp, "UPDATE pfiles SET hooded = %d, immenter = '%s', immleave = '%s', sitelie = '%s', voicestr = '%s', clans = '%s', skills = '%s', writes = %d, profession = %d, travelstr = '%s', bmi = %d, guardian_mode = %d, plan = '%s', goal = '%s', role_id = %d,  dmotestr = '%s', petition_flags = %d  WHERE name = '%s'",
				hooded,
				immenter_temp.c_str(),
				immleave_temp.c_str(),
				pc->site_lie,
				voicestr.c_str(),
				clan_temp.c_str(),
				skill_buf.c_str(),
				writes ? lookup_skill_id(writes) : 0,
				pc->profession,
				travelstr.c_str(),
				bmi,
				guardian_mode,
				planstr.c_str(),
				goalstr.c_str(),
				pc->special_role ? pc->special_role->id : 0,
				dmotestr.c_str(),
				(int)petition_flags,
				name);
		full_query.assign(buftmp);
		err_res = mysql_safe_query((char*)full_query.c_str());
		
		save_dreams();
	}
	
}

void char_data::save_dreams()
{
	DREAM_DATA *dream;
	
	if (!name)
		return;
	
	mysql_safe_query ("DELETE FROM dreams WHERE name = '%s'", name);
	
	if (pc->dreams)
	{
		for (dream = pc->dreams; dream; dream = dream->next)
		{
			mysql_safe_query ("INSERT INTO dreams VALUES('%s', %d, '%s')",
							  name, 0, dream->dream);
		}
	}
	
	if (pc->dreamed)
	{
		for (dream = pc->dreamed; dream; dream = dream->next)
		{
			mysql_safe_query ("INSERT INTO dreams VALUES('%s', %d, '%s')",
							  name, 1, dream->dream);
		}
	}
}

bool char_data::is_hooded()
{
	OBJ_DATA *obj;
	
	if ((obj = get_equip (this, WEAR_NECK_1))
		&& IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)
		&& IS_SET (affected_by, AFF_HOODED))
		return true;
	
	if ((obj = get_equip (this, WEAR_NECK_2))
		&& IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)
		&& IS_SET (affected_by, AFF_HOODED))
		return true;
	
	if ((obj = get_equip (this, WEAR_ABOUT))
		&& IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)
		&& IS_SET (affected_by, AFF_HOODED))
		return true;
	
	if (((obj = get_equip (this, WEAR_HEAD))
		 && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK))
		|| ((obj = get_equip (this, WEAR_FACE))
			&& IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)))
	{
		
		if (obj->obj_flags.type_flag == ITEM_WORN)
			return true;
	}
	
	return false;
}

char * char_data::char_names()
{
	OBJ_DATA *obj;
	
	if ((obj = get_equip (this, WEAR_NECK_1)) &&
		IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
		IS_SET (affected_by, AFF_HOODED))
		return obj->desc_keys;
	
	if ((obj = get_equip (this, WEAR_NECK_2)) &&
		IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
		IS_SET (affected_by, AFF_HOODED))
		return obj->desc_keys;
	
	if ((obj = get_equip (this, WEAR_ABOUT)) &&
		IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
		IS_SET (affected_by, AFF_HOODED))
		return obj->desc_keys;
	
	if (((obj = get_equip (this, WEAR_HEAD))
		 && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK))
		|| ((obj = get_equip (this, WEAR_FACE))
			&& IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)))
	{
		
		if (obj->obj_flags.type_flag == ITEM_WORN)
		{
			if (!obj->desc_keys)
				return "obscured";
			
			return obj->desc_keys;
		}
		
	}
	
	return keywords;
}

char * char_data::height_phrase()
{
	static char phrase[MAX_STRING_LENGTH];
	const char *frame_built[8] = {
		"fragily-built",
		"scantly-built",
		"lightly-built",
		"typically-built",
		"heavily-built",
		"massively-built",
		"gigantically-built",
		"\n"
	};
	
	sprintf (phrase, "gigantic");
	
	if (height < 96)
		sprintf (phrase, "towering");
	if (height < 78)
		sprintf (phrase, "very tall");
	if (height < 75)
		sprintf (phrase, "tall");
	if (height < 71)
		sprintf (phrase, "%s", frame_built[frame]);
	if (height < 60)
		sprintf (phrase, "short");
	if (height < 48)
		sprintf (phrase, "very short");
	if (height < 36)
		sprintf (phrase, "extremely short");
	if (height < 24)
		sprintf (phrase, "tiny");
	
	return phrase;
}

char * char_data::char_short()
{
		// A or An needs to be based on the phrase, not the height

	OBJ_DATA *obj = NULL;
	std::stringstream buffer;
	char phrase[MAX_STRING_LENGTH] = { '\0' };
	
	if (!short_descr && !pc && !mob)
	{
		return NULL;
	}
	
	sprintf (phrase, "%s", height_phrase());
	
	if ((obj = get_equip (this, WEAR_NECK_1)) &&
		IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
		IS_SET (affected_by, AFF_HOODED))
	{
		buffer.sync();
		buffer << "a" << (is_vowel (phrase[0]) ? "n" : "");
		buffer << " " << phrase << ", " << obj->desc_keys << " person";
		return ((char*)buffer.str().c_str());
	}
	
	if ((obj = get_equip (this, WEAR_NECK_2)) &&
		IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
		IS_SET (affected_by, AFF_HOODED))
	{
		buffer.sync();
		buffer << "a" << (is_vowel (phrase[0]) ? "n" : "");
		buffer << " " << phrase << ", " << obj->desc_keys << " person";
		return ((char*)buffer.str().c_str());
	}
	
	if ((obj = get_equip (this, WEAR_ABOUT)) &&
		IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
		IS_SET (affected_by, AFF_HOODED))
	{
		buffer.sync();
		buffer << "a" << (is_vowel (phrase[0]) ? "n" : "");
		buffer << " " << phrase << ", " << obj->desc_keys << " person";
		return ((char*)buffer.str().c_str());
	}
	
	if (((obj = get_equip (this, WEAR_HEAD))
		 && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK))
		|| ((obj = get_equip (this, WEAR_FACE))
			&& IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)))
	{
		
		buffer.sync();
		buffer << "the " << phrase << ", " << obj->desc_keys << " person";
		return ((char*)buffer.str().c_str());
	}
	
	if ((obj = get_equip (this, WEAR_FACE))
		&& IS_SET (obj->obj_flags.extra_flags, ITEM_MASK))
	{
		buffer.sync();
		buffer << "the " << phrase << ", " << obj->desc_keys << " person";
		return ((char*)buffer.str().c_str());
	}
	
	if (!short_descr)
		return "a strangely amorphous person";
	return short_descr;
}

char * char_data::char_long()
{
	OBJ_DATA *obj = NULL;
	std::stringstream buffer;
	char* buf;
	char* phrase;
	char* desc_key;
	bool flag = false;
	
	sprintf (phrase, "%s", height_phrase ());
	
	
	if ((obj = get_equip (this, WEAR_NECK_1))
		&& IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)
		&& IS_SET (affected_by, AFF_HOODED))
	{
		flag = true;
		sprintf(desc_key, "%s", obj->desc_keys);
	}
	
	else if ((obj = get_equip (this, WEAR_NECK_1))
			 && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)
			 && IS_SET (affected_by, AFF_HOODED))
	{
		flag = true;
		sprintf(desc_key, "%s", obj->desc_keys);
		
	}
	
	else if ((obj = get_equip (this, WEAR_ABOUT))
			 && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)
			 && IS_SET (affected_by, AFF_HOODED))
	{
		flag = true;
		sprintf(desc_key, "%s", obj->desc_keys);
		
	}
	
	else if (((obj = get_equip (this, WEAR_HEAD))
			  && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK))
			 || ((obj = get_equip (this, WEAR_FACE))
				 && IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)))
	{
		
		flag = true;
		sprintf(desc_key, "%s", obj->desc_keys);
		
	}
	
	if (flag == true)
	{		
		if (pmote_str)
		{
			buffer.sync();
			if (*(pmote_str) == '\'')
				buffer << "A " << phrase << ", " << desc_key << " person" << pmote_str;
			else 
				buffer << "A " << phrase << ", " << desc_key << " person " << pmote_str;
			return ((char*)buffer.str().c_str());
		}
		else
		{
			buffer.sync();
			buffer << "A " << phrase << ", " << desc_key << " person is here.";
			return ((char*)buffer.str().c_str());
		}
		
	}
	
	else if (pmote_str)
	{
		buffer.sync();
		if (*(pmote_str) == '\'')
			buffer << short_descr << pmote_str;
		else 
			buffer << short_descr << " " << pmote_str;
		
		
		sprintf(buf, "%s", buffer.str().c_str());
		buf[0] = (('a' <= buf[0]) && (buf[0] <= 'z'))
		? (char) (buf[0] - 32) : buf[0];
		buf[0] = toupper (buf[0]);
		return buf;
	}
	else
		return long_descr;
}

void char_data::break_delay()
{
	switch (delay_type)
	{
			
		case DEL_TRACK:
			delay = 0;
			send_to_char ("You discontinue your search for tracks.\n");
			break;
			
		case DEL_MEND_OBJECT:
			delay = 0;
			send_to_char ("You cease your repairs.\n");
			break;
			
		case DEL_PURCHASE_ITEM:
		case DEL_ORDER_ITEM:
			delay = 0;
			delay_type = 0;
			delay_info1 = 0;
			delay_info2 = 0;
			delay_ch = NULL;
			send_to_char ("You decide against making the purchase.\n");
			break;
			
			
		case DEL_OOC:
			delay = 0;
			send_to_char ("You decide against going to the OOC lounge.\n");
			break;
			
		case DEL_APP_APPROVE:
			delay = 0;
			send_to_char ("You pass on this application.\n");
			if (pc->msg)
			{
				send_to_char ("Your prepared message was deleted.\n");
				free_mem (pc->msg);
				pc->msg = NULL;
			}
			break;
			
			
		case DEL_COUNT_COIN:
			stop_counting ();
			break;
			
		case DEL_COMBINE:
			delay_who = 0;
			delay = 0;
			break;
			
		case DEL_GET_ALL:
			get_break_delay();
			break;
			
		case DEL_SEARCH:
			delay = 0;
			act ("You stop searching.", false, 0, 0, TO_CHAR);
			act ("$n stops searching.", true, 0, 0, TO_ROOM);
			break;
			
		case DEL_PICK:
			delay = 0;
			act ("You stop trying to pick the lock.", false, 0, 0, TO_CHAR);
			break;
			
		case DEL_ALERT:
			delay = 0;
			act ("You forget about responding to the alert.", false, 0, 0,
				 TO_CHAR);
			break;
			
		
		case DEL_CAMP1:
		case DEL_CAMP2:
		case DEL_CAMP3:
		case DEL_CAMP4:
			delay = 0;
			send_to_char ("You stop building your camp.\n");
			act ("$n stops building $s camp.", true, 0, 0, TO_ROOM);
			break;
			
		case DEL_TAKE:
			delay = 0;
			send_to_char ("You stop trying to remove the object.\n");
			act ("$n stops trying to get the object from the body.",
				 false, 0, 0, TO_ROOM);
			break;
			
		case DEL_PUTCHAR:
			delay = 0;
			send_to_char ("You stop doing what you're doing.\n");
			act ("$n 'stops doing what $e's doing.", false, 0, 0, TO_ROOM);
			break;
			
		case DEL_STARE:
			delay = 0;
			break;
			
		case DEL_HIDE:
			delay = 0;
			send_to_char ("You stop trying to hide.\n");
			break;
			
		case DEL_SCAN:
			delay = 0;
			delay_info1 = 0;
			delay_info2 = 0;
			delay_info3 = 0;
			delay_info4 = 0;
			break;
		case DEL_SCAN_ALL:
			delay = 0;
			delay_info1 = 0;
			delay_info2 = 0;
			delay_info3 = 0;
			delay_info4 = 0;
			break;
		case DEL_QUICK_SCAN:
			delay = 0;
			delay_info1 = 0;
			delay_info2 = 0;
			delay_info3 = 0;
			delay_info4 = 0;
			break;
			
		case DEL_HIDE_OBJ:
			delay = 0;
			break;
			
		case DEL_PICK_OBJ:
			delay = 0;
			break;
			
	}
	
	delay_type = 0;
}

void char_data::get_break_delay()
{
	if (delay_info1 == CONTAINER_LOC_ROOM)
		send_to_char ("You stop picking things up.\n");
	else
		send_to_char ("You stop removing things.\n");
	
	delay = 0;
}

void char_data::delayed_mend ()
{
	OBJECT_DAMAGE *damage;
	OBJ_DATA *obj;
	OBJ_DATA *kit;
	int done_repair;
	int num_needed = 0;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	std::vector<object_damage *>::iterator odam_iterator;
	
	
		//sanity checks in case something happened after 'mend' and before the delay kicked in
	/* Are we holding what we wanted? */
	if (!(obj = get_obj_in_dark (this, delay_who, right_hand))
		&& !(obj = get_obj_in_dark (this, delay_who, left_hand)))
	{
		send_to_char ("You're not holding anything like that.\n");
		delay = 0;
		delay_type = 0;
		delay_who = 0;
		delay_info1 = 0;
		return;
	}
	
	/* Get the kit from our other hand */
	kit = ((obj == right_hand) ? left_hand : right_hand);
	if (!kit || kit->obj_flags.type_flag != ITEM_REPAIR_KIT)
	{
		act("You're not holding anything that you can mend or clean $p with.",
				false,  obj, 0, TO_CHAR | _ACT_FORMAT);
		delay = 0;
		delay_type = 0;
		delay_who = 0;
		delay_info1 = 0;
		return;
	}
	
	if (kit->o.od.value[0] == 0)
	{
		send_to_char ("That repair kit no longer contains any useful materials.\n");
		delay = 0;
		delay_type = 0;
		delay_who = 0;
		delay_info1 = 0;
		return;
	}
	
	if (kit->o.od.value[3] > 0 && kit->o.od.value[2] > skill_map[lookup_skill_name(kit->o.od.value[3])])
	{
		send_to_char
		("You do not have the skill required to use this repair kit.\n");
		delay = 0;
		delay_type = 0;
		delay_who = 0;
		delay_info1 = 0;
		return;
	}
	
	
	/**
	 A skill_use check is made to give the PC a chance to increase thier skill, although the result has no bearing on the code below.
	 **/
		//the actuall skill that they are using
	if (kit->o.od.value[3] > 0)
	{
		skill_use (this, lookup_skill_name(kit->o.od.value[3]), 0);
	}
	
	int targ_cnt = delay_info1;
	int index = 1;
	
	if (targ_cnt == 0) //repairs first damage it finds as part of repairing all damage
	{	
		for (odam_iterator = obj->damage.begin(); odam_iterator != obj->damage.end(); odam_iterator++)
		{
			damage = *odam_iterator;
			if (!damage)//damage_from_obj removes damage, so we must check here too
				break;
			
			if (skill_mend(this, kit, damage))
			{
				if ((kit->o.od.value[4] < damage->severity))
				{
					send_to_char
					("You cannot repair damage this severe with this kit, so you stop.\n");
					delay = 0;
					delay_type = 0;
					delay_who = 0;
					delay_info1 = 0;
					return;
				}
				
					//amount of kit uses depends on points of damage	
				num_needed = (int)(damage->impact/5) + 1;
				
				if ((kit->o.od.value[0] > 0) && (num_needed > kit->o.od.value[0]))
				{
					send_to_char
					("You do not have enough in your kit to repair this damage, so you stop.\n");
					delay = 0;
					delay_type = 0;
					delay_who = 0;
					delay_info1 = 0;
					return;
				}
				else
				{
					kit->o.od.value[0] -= num_needed;
					if (kit->o.od.value[0] < -1)
						kit->o.od.value[0] = -1;
				}
				
				damage = damage_from_obj(obj, damage);
				
				if (damage->impact>0)
				{
					sprintf (buf,
							 "You repaired the damage carefully, making it look better.");
					sprintf (buf2,
							 "$n repaired the damage carefully, making it look better.");
				}
				else
				{
					sprintf (buf,
							 "You cleaned the damage carefully, making it look better.");
					sprintf (buf2,
							 "$n cleaned the damage carefully, making it look better.");
				}
				act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
				act(buf2, false,  0, 0, TO_ROOM | _ACT_FORMAT);
				done_repair = true;
				break;
			}
			else
			{
				send_to_char
				("You do not have the skill required to repair this damage, so you stop.\n");
				delay = 0;
				delay_type = 0;
				delay_who = 0;
				delay_info1 = 0;
				return;
			}
		}
		
	}
	else if (targ_cnt > 0)//repairs only the specified damage
	{
		index = 1;
		for (odam_iterator = obj->damage.begin(); odam_iterator != obj->damage.end(); odam_iterator++)
		{
			damage = *odam_iterator;
			if (!damage)//damage_from_obj removes damage, so we must check here too
				break;
			
			if(index == targ_cnt)
			{
				if (skill_mend(this, kit, damage))
				{
					if ((kit->o.od.value[4] < damage->severity))
					{
						send_to_char
						("You cannot repair damage this severe with this kit, so you stop.\n");
						delay = 0;
						delay_type = 0;
						delay_who = 0;
						delay_info1 = 0;
						return;
					}
					
						//amount of kit uses depends on points of damage	
					num_needed = (int)(damage->impact/5) + 1;
					
					if ((kit->o.od.value[0] > 0) && (num_needed > kit->o.od.value[0]))
					{
						send_to_char
						("You do not have enough in your kit to repair this damage, so you stop.\n");
						delay = 0;
						delay_type = 0;
						delay_who = 0;
						delay_info1 = 0;
						return;
					}
					else
					{
						kit->o.od.value[0] -= num_needed;
						if (kit->o.od.value[0] < -1)
							kit->o.od.value[0] = -1;
					}
					
					damage = damage_from_obj(obj, damage);
					delay_info1 = -1;
					
					if (damage->impact>0)
					{
						sprintf (buf,
								 "You repaired the damage carefully, making it look better.");
						sprintf (buf2,
								 "$n repaired the damage carefully, making it look better.");
					}
					else
					{
						sprintf (buf,
								 "You cleaned the damage carefully, making it look better.");
						sprintf (buf2,
								 "$n cleaned the damage carefully, making it look better.");
					}
					act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
					act(buf2, false,  0, 0, TO_ROOM | _ACT_FORMAT);
					done_repair = true;
					delay = 0;
					delay_type = 0;
					delay_who = 0;
					delay_info1 = 0;
					return;
				}
				else
				{
					send_to_char
					("You do not have the skill required to repair this damage, so you stop.\n");
					delay = 0;
					delay_type = 0;
					delay_who = 0;
					delay_info1 = 0;
					return;
				}
				
			}
			index ++;
		}
		
	}
	
	if (kit->o.od.value[0] == 0)
	{
		send_to_char
		("Having used the last of the materials in your kit, you quickly discard it.\n");
		if (kit->count > 1)
		{
			kit->o.od.value[0] = vtoo (kit->nVirtual)->o.od.value[0];
			kit->count -= 1;
		}
		else
		{
			extract_obj (kit);
			
		}
		delay = 0;
		delay_type = 0;
		delay_who = 0;
		delay_info1 = 0;
		return;
	}
		//go on to repair more if needed
	begin_repair(this, obj, kit, targ_cnt);
}

void char_data::delayed_track()
{
	TRACK_DATA *track;
	bool found = false;
	int needed;
	char *p;
	char buf[MAX_STRING_LENGTH];
	char output[MAX_STRING_LENGTH];
	
	skill_use (this, "Tracking",0); //chance to learn automatically
	*output = '\0';
	
	for (track = room->tracks; track; track = track->next)
	{
		*buf = '\0';
		needed = skill_map["Tracking"];
		needed -= track->hours_passed / 4;
		if (IS_SET (track->flags, BLOODY_TRACK))
			needed += number (10, 25);
		needed += number (5, 10);
		needed = MAX (needed, 5);
		if (number (1, 100) > needed)
			continue;
		if (!found)
			send_to_char ("\n");
		found = true;
		if (needed < 30)
		{
			if (track->from_dir != track->to_dir)
				sprintf (buf + strlen (buf),
						 "#2The %stracks of a %s#0 are here, leading from %s to %s.",
						 IS_SET (track->flags,
								 BLOODY_TRACK) ? "#1blood-pooled#0 " : "",
						 crude_name (track->race), dirs[track->from_dir],
						 dirs[track->to_dir]);
			else
				sprintf (buf + strlen (buf),
						 "#2The %stracks of a %s#0 are here, coming from the %s and then doubling back.",
						 IS_SET (track->flags,
								 BLOODY_TRACK) ? "#1blood-pooled#0 " : "",
						 crude_name (track->race), dirs[track->from_dir]);
		}
		else if (needed < 50)
		{
			if (track->from_dir != track->to_dir)
				sprintf (buf + strlen (buf),
						 "#2The %stracks of a %s#0 were laid here %s, leading from %s to %s.",
						 IS_SET (track->flags,
								 BLOODY_TRACK) ? "#1blood-pooled#0 " : "",
						 crude_name (track->race),
						 track_age (track->hours_passed),
						 dirs[track->from_dir], dirs[track->to_dir]);
			else
				sprintf (buf + strlen (buf),
						 "#2The %stracks of a %s#0 were laid here %s, coming from the %s and then doubling back.",
						 IS_SET (track->flags,
								 BLOODY_TRACK) ? "#1blood-pooled#0 " : "",
						 crude_name (track->race),
						 track_age (track->hours_passed),
						 dirs[track->from_dir]);
		}
		else if (needed < 90)
		{
			if (track->from_dir != track->to_dir)
				sprintf (buf + strlen (buf),
						 "#2A set of %s tracks#0%s were laid here %s at %s, leading from %s to %s.",
						 specific_name (track->race), IS_SET (track->flags,
															  BLOODY_TRACK) ?
						 ", #1pooled with blood#0, " : "",
						 track_age (track->hours_passed),
						 speed_adj (track->speed), dirs[track->from_dir],
						 dirs[track->to_dir]);
			else
				sprintf (buf + strlen (buf),
						 "#2A set of %s tracks#0%s were laid here %s at %s, coming from the %s and then doubling back.",
						 specific_name (track->race), IS_SET (track->flags,
															  BLOODY_TRACK) ?
						 ", #1pooled with blood#0, " : "",
						 track_age (track->hours_passed),
						 speed_adj (track->speed), dirs[track->from_dir]);
		}
		else
		{
			if (track->from_dir != track->to_dir)
				sprintf (buf + strlen (buf),
						 "#2A set of %s tracks#0%s were laid here %s at %s, leading from %s to %s.",
						 lookup_race_variable (track->race, RACE_NAME),
						 IS_SET (track->flags,
								 BLOODY_TRACK) ? ", #1pooled with blood#0, " : "",
						 track_age (track->hours_passed),
						 speed_adj (track->speed), dirs[track->from_dir],
						 dirs[track->to_dir]);
			else
				sprintf (buf + strlen (buf),
						 "#2A set of %s tracks#0%s were laid here %s at %s, coming from the %s and then doubling back.",
						 lookup_race_variable (track->race, RACE_NAME),
						 IS_SET (track->flags,
								 BLOODY_TRACK) ? ", #1pooled with blood#0, " : "",
						 track_age (track->hours_passed),
						 speed_adj (track->speed), dirs[track->from_dir]);
		}
		*buf = toupper (*buf);
		sprintf (output + strlen (output), "%s ", buf);
	}
	
	if (!found)
	{
		send_to_char ("You were unable to locate any tracks in the area.\n");
		return;
	}
	else
	{
		reformat_string (output, &p);
		page_string (desc, p);
		free_mem (p); //char*
	}
}

void char_data::delayed_alert()
{
	int dir = 0;
	int save_speed = 0;
	int current_room = 0;
	ROOM_DATA *to_room = NULL;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	
	dir = track (this, delay_info1);
	
	if (dir == -1)
	{
		send_to_char ("You can't figure out where the whistle came from.\n");
		delay = 0;
		delay_type = 0;
		delay_info1 = 0;
		delay_info2 = 0;
		return;
	}
	
	current_room = in_room;
	
	if (this->is_subduee())
		return;
	
	if (!is_exit(this, dir))
		return;
	
	if (!(to_room = vtor (is_exit(this, dir)->to_room)))
		return;
	
	if (IS_SET (to_room->room_flags, NO_MOB))
		return;
	
	if (IS_MERCHANT (this) && IS_SET (to_room->room_flags, NO_MERCHANT))
		return;
	
	if (mob && mob->access_flags &&
		!(mob->access_flags & to_room->room_flags))
		return;
	
	save_speed = speed;
	speed = SPEED_RUN;
	
	sprintf (buf, "%s", dirs[dir]);
	
	command_interpreter (this, buf);
	
	speed = save_speed;
	
	delay_info2--;
	
	if (current_room == in_room || delay_info2 <= 0)
	{
		send_to_char ("You can't locate the whistle.\n");
		delay = 0;
		delay_type = 0;
		delay_info1 = 0;
		delay_info2 = 0;
		delay_ch = 0;
		return;
	}
	
	if (in_room != delay_info1)
		delay = 8;
	else
	{
		delay = 0;
		delay_type = 0;
		delay_info1 = 0;
		delay_info2 = 0;
	}
}

void char_data::delayed_camp1()
{
	send_to_char("Finding a safe location behind a tree, you begin constructing\n"
					 "a frame out of fallen branches, found nearby.\n");
	act("$n starts constructing a frame using stripped\ntree branches.", false,
			0, 0, TO_ROOM);
	
	delay_type = DEL_CAMP2;
	delay = 30;
}

void char_data::delayed_camp2()
{
	send_to_char("You begin sewing leafy branches to the frame with long pieces\n"
					 "of saw grass.\n");
	
	act("$n begins sewing leafy branches with grass\nto the lean-to's frame.",
			false,  0, 0, TO_ROOM);
	
	delay_type = DEL_CAMP3;
	delay = 30;
}

void char_data::delayed_camp3()
{
	ROOM_DATA *room = room;
	
	send_to_char("Finally finished, you enter your lean-to, carefully adjusting\n"
					 "its covering of leaves and bark along the way.\n");
	
	act("Finished, $n enters $s lean-to.", true, 0, 0, TO_ROOM);
	
	room->room_flags |= SAFE_Q;
	do_quit (this, "", 1);
	room->room_flags &= ~SAFE_Q;
		/// \todo Can we camp without modifying the SAFE_Q room flag?
}

void char_data::delayed_camp4()
{
	do_quit (this, "", 0);
}


void char_data::delayed_hide()
{
	int mod = 0;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	OBJ_DATA *obj = NULL;
	
	delay_type = 0;
	
	switch (room->terrain_type)
	{
		case SECT_URBAN:
			mod = 0;
			break;
		case SECT_ROAD:
			mod = 10;
			break;
		case SECT_TRAIL:
			mod = 10;
			break;
		case SECT_CAVE:
			mod = -20;
			break;		
		case SECT_FIELD:
			mod = 20;
			break;
		case SECT_WOODS:
			mod = -10;
			break;
		case SECT_FOREST:
			mod = -20;
			break;
		case SECT_HILLS:
			mod = -20;
			break;
		case SECT_MOUNTAIN:
			mod = -20;
			break;
		case SECT_SWAMP:
			mod = -10;
			break;
		case SECT_PASTURE:
			mod = 20;
			break;
		case SECT_HEATH:
			mod = 20;
			break;
	}
	
	if (right_hand && right_hand->obj_flags.type_flag == ITEM_LIGHT)
		obj = right_hand;
	
	if (obj)
	{
		if (obj->o.light.hours > 0 && obj->o.light.on)
		{
			
			act("You put out $p so you won't be detected.",
					false,  obj, 0, TO_CHAR | _ACT_FORMAT);
			act("$n put out $p.", false,  obj, 0, TO_ROOM | _ACT_FORMAT);
			
			obj->o.light.on = 0;
		}
		obj = NULL;
	}
	
	if (left_hand && left_hand->obj_flags.type_flag == ITEM_LIGHT)
		obj = left_hand;
	
	if (obj)
	{
		if (obj->o.light.hours > 0 && obj->o.light.on)
		{
			
			act("You put out $p so you won't be detected.",
					false,  obj, 0, TO_CHAR | _ACT_FORMAT);
			act("$n put out $p.", false,  obj, 0, TO_ROOM | _ACT_FORMAT);
			
			obj->o.light.on = 0;
		}
		obj = NULL;
	}
	
	for (obj = equip; obj; obj = obj->next_content)
	{
		if (obj->obj_flags.type_flag != ITEM_LIGHT)
			continue;
		
		if (obj->o.light.hours > 0 && obj->o.light.on)
		{
			
			act("You put out $p so you won't be detected.",
					false,  obj, 0, TO_CHAR | _ACT_FORMAT);
			act("$n put out $p.", false,  obj, 0, TO_ROOM | _ACT_FORMAT);
			
			obj->o.light.on = 0;
		}
	}
	
	
	if (skill_use (this, "Hide", mod))
	{
		send_to_char ("You settle down in what looks like a good spot.\n");
		if (!get_affect(this,MAGIC_HIDDEN))
		{
			magic_add_affect (this, MAGIC_HIDDEN, -1, 0, 0, 0, 0);
		}
		sprintf (buf, "[%s hides]", name);
		act(buf, true,  0, 0, TO_NOTVICT | TO_IMMS);
	}
	else
	{
		send_to_char ("You struggle to find a suitable place to hide.\n");
	}
	
	room_light (room);
}

void char_data::delayed_hide_obj()
{
	OBJ_DATA *obj;
	AFFECTED_TYPE *af;
	
	obj = (OBJ_DATA *) delay_info1;
	
	if (obj != right_hand && obj != left_hand)
	{
		send_to_char ("You don't have whatever you were hiding anymore.\n");
		return;
	}
	
	obj_from_char (&obj, 0);
	
	remove_obj_affect (obj, MAGIC_HIDDEN);	/* Probably doesn't exist */
	
	af = new AFFECTED_TYPE;
	
	af->type = MAGIC_HIDDEN;
	af->a.hidden.duration = -1;
	af->a.hidden.hidden_value = skill_map["Hide"];
	af->a.hidden.coldload_id = coldload_id;
	
	act("You hide $p.", false,  obj, 0, TO_CHAR);
	
	af->next = obj->xaffected;
	obj->xaffected = af;
	obj_to_room (obj, in_room);
	
	act("$n hides $p.", false,  obj, 0, TO_ROOM);
}

void char_data::delayed_quick_scan()
{
	
	delay_info4 = QUICK_SCAN_DIST;
	delayed_scan_all();
	return;
}

void char_data::delayed_scan()
{
	int dir = 0;
	int blank = 0;
	int seen = 0;
	int penalty = 0;
	int skil_pen_level = 0;
	float dist = 0;
	ROOM_EXIT_DATA *exit = NULL;
	ROOM_DATA *curr_room = NULL;
	ROOM_DATA *next_room = NULL;
	ROOM_DATA *temp_room = NULL;
	char outbuf[MAX_STRING_LENGTH] = { '\0' };
	float next_room_dist;
	float curr_dist;
	bool bSaw = false;
	
	
	dir = delay_info1;
		//if we have a limit, use it, otherwise the limit is MAX_SCAN_DIST yards
	dist = delay_info4;
	if (dist == 0)
		dist = MAX_SCAN_DIST;
	
	curr_room = room;
	
	if (skill_map["Scan"] <= 0)
	{
		send_to_char("You can see nothing there.\n");
		return;
	}
	
		//find our first room to scan
	exit = is_exit(this, dir);
	
		//if no obvious exit, look for a diagonal exit
	if (!exit)
	{
		temp_room = get_diagonal_move_room (this, dir);
		
		if (!temp_room)
		{
			send_to_char("There's nothing to scan that way.\n");
			return;
		}
		
		if (room_distance(curr_room, temp_room) < dist)
		{
			exit = new room_exit_data;
			exit->to_room = temp_room->nVirtual;
		}
		else 
		{
			send_to_char("You cannot make out any details.\n");
			return;
		}
	}
	
	next_room = vtor (exit->to_room);
	if (!next_room)
	{
		send_to_char("There's nothing to scan that way.\n");
		return;
	}
	
	next_room_dist = room_distance(curr_room, next_room);
	curr_dist = next_room_dist;
	
	while (next_room_dist < dist)
	{		
			//we have a room, but can we 'see' it?
		if (IS_SET (exit->port_flags, EX_ISDOOR) &&
			IS_SET (exit->port_flags, EX_CLOSED) &&
			!IS_SET (exit->port_flags, EX_ISGATE))
		{
			if (is_dark (room))
				send_to_char("You see nothing.\n");
			else
				send_to_char("Your view is blocked.\n");
			
			break;
		}
		
			//is it close enough to see anything?
		if (next_room_dist > dist)
		{
			send_to_char("It is too far to make out any details.\n");
			break;
		}
		
			//modifiers for weather, distance, darkness will affect 'difficulty' of scanning
		seen = 0;
		blank = 1;
		penalty += scan_weather_penalty(room);
		penalty += scan_distance_penalty(next_room_dist);
		penalty += scan_light_penalty(room, next_room);
		
		
			//give PC a chance to increase skill at this set of penalties
			//actually effects are detailed later
		skill_use(this, "Scan", penalty);
		
		if (!next_room->psave_loaded)
			load_save_room (next_room);
		
		skil_pen_level = skill_level (this, "Scan", penalty);
		
		
		bSaw = char__room_scan(this, next_room, dirs[dir], skil_pen_level, bSaw, curr_dist);
		
			//find the next room
		curr_room = next_room;
		exit = next_room->dir_option[dir];
		
			//we are looking straight through, so no diagonals
		if (!exit)
		{
			break;
		}
		
		next_room = vtor (exit->to_room);
		if (!next_room)
		{
			send_to_char("There's nothing to scan that way.\n");
			break;
		}
		
		curr_dist = curr_dist + room_distance(curr_room, next_room);
		if (curr_dist > dist)
		{
			sprintf(outbuf, "You cannot make out any further details to the %s.\n\n", dirs[dir]);
			send_to_char(outbuf);
			delay = 0;
			delay_type = 0;
			delay_info1 = 0;
			delay_info2 = 0;
			delay_info4 = 0;
			break;
		}
	}
	
	sprintf(outbuf, "Your scanning to the %s is complete.\n\n", dirs[dir]);
	send_to_char(outbuf);
	return;
	
}

void char_data::delayed_scan_all()
{
	int dir = 0;
	
	ROOM_EXIT_DATA *exit = NULL;
	
	
	if (skill_map["Scan"] <= 0)
	{
		send_to_char("You can see nothing there.\n");
		return;
	}
	
	for (dir = 0; dir <= LAST_DIR; dir++)
	{ 
		exit = is_exit(this, dir);
		
		if (!exit)
			continue;
		else
		{
			delay_info1 = dir;
			delayed_scan();	
		}
		
	}
	send_to_char("Your scanning is completed.\n");
	delay = 0;
	delay_type = 0;
	delay_info1 = 0;
	delay_info2 = 0;
	delay_info4 = 0;
	return;
	
}

void char_data::delayed_pick_obj()
{
	int roll;
	OBJ_DATA *tobj;
	OBJ_DATA *locked_obj = NULL;
	OBJ_DATA *obj = NULL;
	
	locked_obj = (OBJ_DATA *) delay_info1;
	
	if (!is_obj_here (this, locked_obj, 1))
	{
		
		if (right_hand && right_hand == locked_obj)
			obj = right_hand;
		
		if (left_hand && left_hand == locked_obj)
			obj = left_hand;
		
		if (obj == locked_obj)
		{
			send_to_char ("You stop picking.\n");
			return;
		}
	}
	
	if (!IS_SET (locked_obj->o.container.flags, CONT_CLOSED) ||
		!IS_SET (locked_obj->o.container.flags, CONT_LOCKED))
	{
		send_to_char ("You stop picking.\n");
		return;
	}
	
	if (locked_obj->obj_flags.type_flag == ITEM_CONTAINER)
	{
		if (skill_map["Pick"] > locked_obj->o.container.pick_penalty)
			skill_use (this, "Pick", locked_obj->o.container.pick_penalty);
		
		if ((roll = number (1,100)) 
			> (skill_map["Pick"] - locked_obj->o.container.pick_penalty))
		{
			if (!(roll % 5) && (tobj = get_carried_item (this, ITEM_LOCKPICK)))
			{
				act("You fail miserably, snapping your pick in the process!",
						false, 0, 0, TO_CHAR);
				act("$n mumbles as $s lockpick snaps.", true, 0, 0,
						TO_ROOM | _ACT_FORMAT);
				extract_obj (tobj);
				return;
			}
			act("You failed.", false, 0, 0, TO_CHAR);
			act("$n fails to pick $p.", true, locked_obj, 0, TO_ROOM);
			return;
		}
	}
	
	
	act("You have successfully picked the lock.", true, 0, 0, TO_CHAR);
	act("$n has picked the lock of $p.", true, locked_obj, 0,
			TO_ROOM | _ACT_FORMAT);
	
	if (locked_obj->obj_flags.type_flag == ITEM_CONTAINER)
		locked_obj->o.container.flags &= ~CONT_LOCKED;
	else
		locked_obj->o.od.value[2] &= ~CONT_LOCKED;
}


void char_data::delayed_search()
{
	int dir = 0;
	int skill_tried = 0;
	int somebody_found = 0;
	int search_quality = 0;
	CHAR_DATA *tch = NULL;
	OBJ_DATA *obj = NULL;
	AFFECTED_TYPE *af = NULL;
	
	dir = delay_info1;
	
	if (dir == -1)
	{
		
		for (tch = room->people; tch; tch = room->people->next_in_room)
		{
			
				// You will never find yourself playing SOI
			if (tch == this)
				continue;
			
				// Don't reveal people you can already see
			if (!get_affect (tch, MAGIC_HIDDEN))
				continue;
			
				// Don't reveal group members
			if (are_grouped (this, tch))
				continue;
			
				// Don't reveal Admins
			if (GET_FLAG (tch, FLAG_WIZINVIS))
				continue;
			
				// Don't reveal when night blind
			if (is_dark (room)
				&& !get_affect (this, MAGIC_AFFECT_INFRAVISION)
				&& !IS_SET (affected_by, AFF_INFRAVIS))
				continue;
			
				// Don't reveal elves?
			if ((room->terrain_type == SECT_WOODS ||
				 room->terrain_type == SECT_FOREST ||
				 room->terrain_type == SECT_HILLS) &&
				get_affect (tch, MAGIC_AFFECT_CONCEALMENT))
				continue;
			
				// Don't reveal the invisible
			if (get_affect (tch, MAGIC_AFFECT_INVISIBILITY) &&
				!get_affect (this, MAGIC_AFFECT_SEE_INVISIBLE))
				continue;
			
				//
			if (!skill_tried)
			{
				skill_use (this, "Search", 0);
				skill_tried = 1;
			}
			
			search_quality = skill_level (this, "Search", 0) -
			skill_level (this, "Hide", 0) / 5;
			
			if (search_quality <= 0)
				continue;
			
			if (search_quality < number (0, 100))
			{
				if (!number (0, 10))
					act("$n looks right at you, but still doesn't see you.",
							true, 0, tch, TO_VICT);
				continue;
			}
			
			tch->act("You are exposed by $N!", false, 0, this, TO_CHAR);
			act("You expose $N!", false, 0, tch, TO_CHAR);
			act("$n exposes $N.", false, 0, tch, TO_NOTVICT);
			
			remove_affect_type (tch, MAGIC_HIDDEN);
			
			somebody_found = 1;
		}
		
		if (!somebody_found)
		{
			send_to_char("You didn't find anybody hiding.\n");
			act("$n finishes searching.", true, 0, 0, TO_ROOM);
		}
		
		for (obj = room->contents; obj; obj = obj->next_content)
		{
			
			if (!(af = get_obj_affect (obj, MAGIC_HIDDEN)))
				continue;
			
			if (is_dark (room)
				&& !get_affect (this, MAGIC_AFFECT_INFRAVISION)
				&& !IS_SET (affected_by, AFF_INFRAVIS))
				continue;
			
			if (af->a.hidden.coldload_id == coldload_id)
				continue;
			
			if (!skill_tried)
			{
				skill_use (this, "Search", 0);
				skill_tried = 1;
			}
			
			if (skill_map["Search"] <= number (1, 100))
				continue;
			
			if (af->a.hidden.hidden_value
				&& number (1, skill_level (this, "Search", 0)) < number (1,
																	   af->
																	   a.
																	   hidden.
																	   hidden_value))
				continue;
			
			remove_obj_affect (obj, MAGIC_HIDDEN);
			
			act("You reveal $p.", false, obj, 0, TO_CHAR);
			act("$n reveals $p.", false, obj, 0, TO_ROOM);
		}
		
		return;
	}
	
	if (!room->secrets[dir]
		|| (skill_map["Search"] < room->secrets[dir]->diff)
		|| (!skill_use (this, "Search", room->secrets[dir]->diff)))
	{
		send_to_char("You didn't find anything in that direction.\n");
		return;
	}
	else 
	{
		send_to_char(room->secrets[dir]->stext);
		
	}
}

void char_data::delayed_take()
{
	OBJ_DATA *obj;
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	
	delay = 0;
	victim = delay_ch;
	obj = (OBJ_DATA *) delay_info1;
	
	if (!is_he_here (this, victim, true))
	{
		send_to_char ("Your victim left before you could finish taking the "
						  "object.\n");
		return;
	}
	
	if (get_equip (victim, delay_info2) != obj)
	{
		send_to_char ("The thing you were after is gone now.");
		return;
	}
	
	if (victim->get_position() == SLEEP && !get_trust())
	{
		
		victim->forced_wakeup();
		
		if (victim->get_position() != SLEEP)
		{
			
			act("$N awakens as struggle with $M.",
					true,  0, victim, TO_CHAR);
			act("$n awakens $N as $e struggles with $M.",
					false,  0, victim, TO_NOTVICT);
			
			if (!get_affect (victim, MAGIC_AFFECT_PARALYSIS))
				return;
		}
	}
	
	if (!(get_trust() ||
		  victim->get_position() <= SLEEP ||
		  get_affect (victim, MAGIC_AFFECT_PARALYSIS) ||
		  victim->is_subduee()))
	{
		act("$N prevents you from taking $p.", true,  obj, victim, TO_CHAR);
		victim->act("$N unsuccessfully tries to take $p from you.",
					true,  obj, this, TO_CHAR);
		victim->act("$n prevents $N from taking $p.",
					false,  obj, this, TO_NOTVICT);
		return;
	}
	
	sprintf (buf, "$n removes and takes $p from $N's %s.",
			 locations[obj->location]);
	act(buf, false,  obj, victim, TO_NOTVICT | _ACT_FORMAT);
	
	sprintf (buf, "$N removes and takes $p from your %s.",
			 locations[obj->location]);
	victim->act(buf, true, obj, this, TO_CHAR | _ACT_FORMAT);
	
	sprintf (buf, "You remove and take $p from $N's %s.",
			 locations[obj->location]);
	act(buf, true,  obj, victim, TO_CHAR | _ACT_FORMAT);
	
	unequip_char (victim, obj->location);
	obj_to_char (obj, this);
	
	if (obj->activation && IS_SET (obj->obj_flags.extra_flags, ITEM_GET_AFFECT))
		obj_activate(this, obj);
}

void char_data::delayed_get()
{
	OBJ_DATA *container;
	OBJ_DATA *tobj;
	OBJ_DATA *obj;
	OBJ_DATA *first_obj;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int item_num = 0;
	int container_num = 0;
	int error;
	char *locs[5] = { "", "room", "inventory", "worn", "" };
	
		//delay_who is container
	if (delay_who)
	{
		/* Makes sure that this container is in the room */
		
		container = (OBJ_DATA *) delay_who;
		
		if (delay_info1 == CONTAINER_LOC_ROOM)
			tobj = room->contents;
		else if (delay_info1 == CONTAINER_LOC_INVENTORY)
			tobj = right_hand;
		else if (delay_info1 == CONTAINER_LOC_WORN)
			tobj = equip;
		else
			tobj = NULL;
		
		
		for (; tobj; tobj = tobj->next_content)
		{
				//how many items are in the container
			container_num++;
			
				//if the object at container_loc is the container, we break out
			if (tobj == container)
				break;
		}
		
		if (!tobj)
		{
			send_to_char ("You can't get anything else.\n");
			return;
		}
		
		if (container->obj_flags.type_flag == ITEM_DRYCON)
			first_obj = vtoo(container->o.drycon.contents);	
		else
			first_obj = container->contains;
	}
	else
		first_obj = room->contents;
	
	if (container->obj_flags.type_flag == ITEM_DRYCON)
	{
		if (!IS_OBJ_VIS (this, container))
		{
			send_to_char ("You don't see that container.\n");
			return;
		}
		if (container->o.drycon.volume)
		{
			sprintf (buf, "%d %s %s",
					 container->o.drycon.volume, fname(first_obj->name), container->name);
		}
		else
			printf ("couldn't find it\n");
		
		do_get (this, buf, 0);
		
		if (first_obj->carried_by != this)
			printf ("Oh boy...couldn't pick up %d\n", first_obj->nVirtual);
		else
			delay = 0;
		
		return;
	}
	else 
	{
		
		for (obj = first_obj; obj; obj = obj->next_content)
		{
			
			if (!IS_OBJ_VIS (this, obj))
				continue;
			
			item_num++;
			
			if (can_obj_to_inv (obj, this, &error, 0))
			{
				
				if (container_num)
					sprintf (buf, "#%d %s #%d",
							 item_num, locs[delay_info1], container_num);
				else
					sprintf (buf, "#%d", item_num);
				
				do_get (this, buf, 0);
				
				if (obj->carried_by != this)
					printf ("Oh boy...couldn't pick up %d\n", obj->nVirtual);
				else
					delay = 4;
				
				return;
			}
		}
		
		send_to_char ("...and that's about all you can get.\n");
	}
	
	delay = 0;
}

void char_data::delayed_remove()
{
	OBJ_DATA *obj, *eq;
	
	obj = (OBJ_DATA *) delay_who;
	
	if (!obj)
	{
		delay_type = 0;
		delay_who = 0;
		delay = 0;
		return;
	}
	
	if (obj->location == WEAR_WAIST)
	{
		if ((eq = get_equip (this, WEAR_BELT_1)))
		{
			act("$p falls free.", true,  eq, 0, TO_CHAR);
			act("$n drops $p.", true,  eq, 0, TO_ROOM);
			obj_to_room (unequip_char (this, WEAR_BELT_1), in_room);
		}
		
		if ((eq = get_equip (this, WEAR_BELT_2)))
		{
			act("$p falls free.", true,  eq, 0, TO_CHAR);
			act("$n drops $p.", true,  eq, 0, TO_ROOM);
			obj_to_room (unequip_char (this, WEAR_BELT_2), in_room);
		}
	}
	
	
	if (IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
		IS_SET (affected_by, AFF_HOODED))
		do_hood (this, "", 0);
	
	if (obj->location == WEAR_LIGHT && obj->obj_flags.type_flag == ITEM_LIGHT)
		light (this, obj, false, true);
	
	if (obj->location == WEAR_PRIM || obj->location == WEAR_SEC ||
		obj->location == WEAR_BOTH)
		unequip_char (this, obj->location);
	else
		obj_to_char (unequip_char (this, obj->location), this);
	
	act("You stop using $p.", false,  obj, 0, TO_CHAR);
	act("$n stops using $p.", true,  obj, 0, TO_ROOM);
	
	delay = 0;
	delay_type = 0;
	delay_who = 0;
}


void char_data::stop_counting()
{
	send_to_char("You bore of counting coins.\n");
	
	delay = 0;
	
	delay_obj = NULL;
}

void char_data::delayed_putchar()
{
	int location;
	OBJ_DATA *obj;
	OBJ_DATA *tobj;
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	
	delay = 0;
	victim = delay_ch;
	obj = (OBJ_DATA *) delay_info1;
	location = delay_info2;
	
	if (!is_he_here (this, victim, true))
	{
		send_to_char ("Your victim left before you could finish dressing "
						  "them.\n");
		return;
	}
	
	if (!is_obj_in_list (obj, right_hand) &&
		!is_obj_in_list (obj, left_hand))
	{
		act("You no longer have the thing you were putting on $N.",
				false,  0, victim, TO_CHAR);
		act("$n stops putting something on $N.", true,  0, victim, TO_ROOM);
		act("$n stops putting something on you.",
				true,  0, victim, TO_VICT);
		return;
	}
	
	
	tobj = get_equip (victim, delay_info2);
	
	if (tobj && tobj != obj)
	{
		act("You discover that $N is already wearing $p.",
				true,  tobj, victim, TO_CHAR);
		act("$n stops dressing $N.", false,  0, victim, TO_NOTVICT);
		return;
	}
	
	if (victim->get_position() == SLEEP && !get_trust())
	{
		
		victim->forced_wakeup();
		
		if (victim->get_position() != SLEEP)
		{
			
			act("$N awakens as struggle with $M.",
					true,  0, victim, TO_CHAR);
			act("$n awakens $N as $e struggles with $M.",
					false,  0, victim, TO_NOTVICT);
			
			if (!get_affect (victim, MAGIC_AFFECT_PARALYSIS))
				return;
		}
	}
	
	if (!(get_trust() ||
		  victim->get_position() <= SLEEP ||
		  get_affect (victim, MAGIC_AFFECT_PARALYSIS)))
	{
		act("$N prevents you from taking $p.", true,  obj, victim, TO_CHAR);
		victim->act("$N unsuccessfully tries to take $p from you.",
					true, obj, this, TO_CHAR);
		victim->act("$n prevents $N from taking $p.",
					false, obj, this, TO_NOTVICT);
		return;
	}
	
	obj_from_char (&obj, 0);
	equip_char (victim, obj, location);
	
	strcpy (buf2, locations[location]);
	*buf2 = tolower (*buf2);
	
	sprintf (buf, "$n puts $p on $N's %s.", buf2);
	act(buf, false,  obj, victim, TO_NOTVICT | _ACT_FORMAT);
	
	sprintf (buf, "$N puts $p on your %s.", buf2);
	victim->act(buf, true, obj, this, TO_CHAR | _ACT_FORMAT);
	
	sprintf (buf, "You put $p on $N's %s.", buf2);
	act(buf, true,  obj, victim, TO_CHAR | _ACT_FORMAT);
	
	if (obj->activation &&
		IS_SET (obj->obj_flags.extra_flags, ITEM_WEAR_AFFECT))
		obj_activate (this, obj);
}

void char_data::delayed_count_coin()
{
	
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[MAX_STRING_LENGTH] = { '\0' };
	OBJ_DATA* hand_obj_ary[2] = { right_hand, left_hand };
	char* hand_str_ary[2] = { "right hand", "left hand" };
	OBJ_DATA *obj = NULL, *tobj = NULL;
	int location = 0, money = 0; 
	std::string amount;
	
	for( int i = 0; i < 2; i++ )
	{
		if( hand_obj_ary[i] )
		{
			if((hand_obj_ary[i]->obj_flags.type_flag == ITEM_MONEY)
			   && (game_currency(CURRENCY_TIRITH, hand_obj_ary[i]->nVirtual)))
			{
				money += hand_obj_ary[i]->coppers * hand_obj_ary[i]->count;
				sprintf (buf2 + strlen (buf2),
						 "   #2%d %s#0 (#2%s#0): %d coppers\n",
						 hand_obj_ary[i]->count,
						 coin_sdesc(hand_obj_ary[i]),
						 hand_str_ary[i],
						 (int)(hand_obj_ary[i]->coppers * hand_obj_ary[i]->count));
			}
			if((hand_obj_ary[i]->obj_flags.type_flag == ITEM_CONTAINER)
			   && (game_currency(CURRENCY_TIRITH, hand_obj_ary[i]->nVirtual)))
			{
				if (IS_SET (hand_obj_ary[i]->o.od.value[1], CONT_CLOSED))
				{
					std::stringstream ss;
					ss << "#2" <<  (char)toupper(hand_obj_ary[i]->short_description[0]) << (char*)&(hand_obj_ary[i]->short_description[1]) << "#0" << " is closed.\n";
					send_to_char(ss.str().c_str());
				}
				else
				{
					
					for( tobj = hand_obj_ary[i]->contains; tobj; tobj = tobj->next_content )
					{
						if((tobj->obj_flags.type_flag == ITEM_MONEY)
						   && (game_currency(CURRENCY_TIRITH, tobj->nVirtual)))
						{
							money += (int) tobj->coppers * tobj->count;
							sprintf (buf2 + strlen (buf2),
									 "   #2%d %s#0 (#2%s#0): %d coppers\n",
									 tobj->count,
									 coin_sdesc (tobj),
									 obj_short_desc (tobj->in_obj),
									 (int)(tobj->coppers * tobj->count));
							
						}
					}
				} // is closed
			} // is container
		} // item exists in this hand
	} // iterate both hands
	
	for( location = 0; location < MAX_WEAR; location++ )
	{
		if((obj = get_equip(this, location)))
		{
			if( obj->obj_flags.type_flag == ITEM_CONTAINER )
			{
				if (IS_SET (obj->o.od.value[1], CONT_CLOSED))
				{
					std::stringstream ss;
					ss << "#2" <<  (char)toupper(obj->short_description[0]) << (char*)&(obj->short_description[1]) << "#0" << " is closed.\n";
					send_to_char(ss.str().c_str());
				}
				else
				{
					for( tobj = obj->contains; tobj; tobj = tobj->next_content )
					{
						if((tobj->obj_flags.type_flag == ITEM_MONEY)
						   && (game_currency(CURRENCY_TIRITH, tobj->nVirtual)))
						{
							money += (int) tobj->coppers * tobj->count;
							sprintf (buf2 + strlen (buf2),
									 "   #2%d %s#0 (#2%s#0): %d coppers\n",
									 tobj->count,
									 coin_sdesc (tobj),
									 obj_short_desc (tobj->in_obj),
									 (int)(tobj->coppers * tobj->count));
						}
					}
				}
			} // container
		} // object exists at location
	}// for loop through locations
	
	
	
	if (money)
		sprintf( buf, "By your count, you have %d coppers' worth in coin:", money);
	else
		send_to_char("You don't seem to have any coin.\n");
	
	act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
	send_to_char("\n");
	send_to_char(buf2);
}
	
void char_data::act(const char *action_message, int hide_invisible, OBJ_DATA * obj, void *vict_obj, int type)
{
		
		/// \brief  Show an action relative to the actors and watchers.
		///
		/// \param[in]  action_message  A specially formatted action string.
		/// \param[in]  hide_invisible  When set do not show the action to those who cannot see the actor
		/// \param[in]  obj             A target object or character.
		/// \param[in]  vict_obj        An additional target object or character
		/// \param[in]  type            Bitvector defines who is to see this action
		
	
		/// \par Local Variables:
	
	
		/// \li \e  strp  - This is an index into the \c action_message parameter.
		/// \li \e  point  - This is an index into the \c buf local.
		/// \li \e  i  - text to be inserted in place of format tags.
		/// \li \e  p  - A pointer into the \c buf local during the reformat phase.
		/// \li \e  buf  - The output buffer.
		/// \li \e  immbuf1  - A buffer holding the actor's real name.
		/// \li \e  immbuf2  - A buffer holding the victim's real name.
		/// \li \e  chsex  - A temporary backup of a PC's sex if they are hooded.
		/// \li \e  color  - A reference to the ANSI color to use for output part.
		/// \li \e  do_cap  - Determines proper capitalization (start of line, e.g).
		/// \li \e  to  - A pointer to the recipient of the action message.
		/// \li \e  tch  - A pointer to an included secondary actor.
	
	const char *strp;
	char *point;
	const char *i = '\0';
	char *p;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char immbuf1[MAX_STRING_LENGTH]= { '\0' };
	char immbuf2[MAX_STRING_LENGTH]= { '\0' };
	int chsex;
	int color = 0;
	int do_cap = 0;
	CHAR_DATA *to;
	CHAR_DATA *tch;
	
	
		/// \par Implementation:
	
		/// <ul>
	
		/// <li>
		///       First we test the parameter integrity as a precaution. If we have
		///       bad parameters we return immediately.
		/// </li>
	
	if (!action_message || !*action_message)
	{
		return;
	}
	
		
		/// <li>
		///       Examine \c type to determine who receives this action message.
		/// </li>
	
	if (IS_SET (type, TO_VICT))
	{
		tch = (CHAR_DATA *) vict_obj;
		if (pc && !desc)
		{
			return;
		}
		to = tch;
	}
	else if (IS_SET (type, TO_CHAR))
	{
		if (pc && !desc)
		{
			return;
		}
		to = this;
	}
	else if (!room)
	{
		return;
	}
	else
	{
		to = room->people;
	}
	
		/// <li>
		///       Iterate through persons in the recipient's room sending the
		///       action message where appropriate.
		/// <ul>
	
	for (; to; to = to->next_in_room)
	{
		
		
		
			/// <li>
			///       We do not translate and send the message:
			/// <ul>
			/// <li>    \e IF the recipient is not the actor </li>
			/// <li>    \e AND the actor is immortal </li>
			/// <li>    \e AND the recipient is mortal </li>
			/// <li>    \e AND the actor is invisible. </li>
			/// </ul>
			/// </li>
		
		if (this != to 
			&& get_trust() 
			&& to->get_trust()
			&& GET_FLAG (this, FLAG_WIZINVIS))
		{
			continue;
		}
		
		
			/// <li>
			///       We translate and send the message:
			/// <ul>
			/// <li>    \e IF the recipient is connected (i.e. a link-live PC or an
			///            animated NPC) </li>
			/// <li>    \e AND the recipient is not the actor (\e OR this message
			///            type is to the actor) </li>
			/// <li>    \e AND the recipient is in the actor's group (\e OR this
			///            message type is not group-only) </li>
			/// <li>    \e AND the recipient can see the actor (\e OR we do not want
			///            to hide invisible actors </li>
			/// <li>    \e AND the recipient is awake </li>
			/// <li>    \e AND the type is not immortal-only (\e OR the recipient is
			///            on staff and the actor is not (?)) </li>
			/// <li>    \e AND \e NOT a non-victim message \e IF the recipient is
			///            the same as the victim. </li>
			/// </ul>
			/// </li>
		
		if (to->desc
			&& (to != this || IS_SET (type, TO_CHAR))
			&& (are_grouped (to, this) 
				|| !IS_SET (type, TO_GROUP))
			&& (can_see_mob(to, this)
				|| !hide_invisible)
			&& to->is_awake()
			&& (!IS_SET (type, TO_IMMS)
				|| (to->get_trust())
				&& !(get_trust()))
			&& !(IS_SET (type, TO_NOTVICT) 
				 && to == (CHAR_DATA *) vict_obj))
		{
			
			
				/// <li>  Iterate through the action_message and output buffer
				///       character-at-a-time.
				/// <ul>
			
			for (strp = action_message, point = buf;;)
			{
				
					/// <li>
					///       We parse the next character as a format identifier
					///       \e IF this character is \c $. \e ELSE we copy this
					///       character from the message to the buffer (stopping
					///       when we reach the end of the string.
					/// </li>
				
					/// <li>  Format translations:
					/// <ul>
				
				if (*strp == '$')
				{
					
					switch (*(++strp))
					{
							
								/// <li>
								///       \c $n - insert the short description of actor
								///               (showing "someone" if the recipient
								///               cannot see, and storing the actor's
								///               name for immortals.
								/// </li>
							
						case 'n':
							i = PERS (this, to);
							if (to->get_trust() && is_hooded())
							{
								strcpy (immbuf1, i);
								sprintf (immbuf1, "%s (%s)", i, name);
								i = immbuf1;
							}
							color = 5;
							break;
							
							
								/// <li>
								///       \c $N - insert the short description of the
								///               victim (showing "someone" if the
								///               recipient cannot see, and storing the
								///               victim's name for immortals to see.
								/// </li>
							
						case 'N':
							tch = (CHAR_DATA *) vict_obj;
							i = PERS (tch, to);
							if (to->get_trust() && tch->is_hooded())
							{
								strcpy (immbuf2, i);
								sprintf (immbuf2, "%s (%s)", i, tch->name);
								i = immbuf2;
							}
							color = 5;
							break;
							
							
								/// <li>
								///       \c $3 - insert the short description of third
								///               PC (?).
								/// </li>
							
						case '3':
							i = PERS ((CHAR_DATA *) obj, to), color = 5;
							break;
							
							
								/// <li>
								///       \c $m - actor as "him", "her", or "it" (the
								///               latter in the case of neuter or hooded
								///               actors).
								/// </li>
							
						case 'm':
							if (is_hooded())
							{
								chsex = sex;
								sex = 0;
								i = HMHR (this);
								sex = chsex;
							}
							else
								i = HMHR (this);
							color = 0;
							break;
							
							
								/// <li>
								///       \c $M - victim as "him", "her", or "it" (the
								///               latter in the case of neuter or hooded
								///               victims).
								/// </li>
							
						case 'M':
							tch = (CHAR_DATA *) vict_obj;
							if (tch->is_hooded())
							{
								chsex = tch->sex;
								tch->sex = 0;
								i = HMHR (tch);
								tch->sex = chsex;
							}
							else
								i = HMHR (tch);
							color = 0;
							break;
							
							
								/// <li>
								///       \c $s - actor as "his", "her", or "its" (the
								///               latter in the case of neuter or hooded
								///               actors).
								/// </li>
							
						case 's':
							if (is_hooded())
							{
								chsex = sex;
								sex = 0;
								i = HSHR (this);
								sex = chsex;
							}
							else
								i = HSHR (this);
							color = 0;
							break;
							
							
								/// <li>
								///       \c $S - victim as "his", "her", or "its" (the
								///               latter in the case of neuter or hooded
								///               victims).
								/// </li>
							
						case 'S':
							tch = (CHAR_DATA *) vict_obj;
							if (tch->is_hooded())
							{
								chsex = tch->sex;
								tch->sex = 0;
								i = HSHR (tch);
								tch->sex = chsex;
							}
							else
								i = HSHR (tch);
							color = 0;
							break;
							
							
								/// <li>
								///       \c $e - actor as "he", "she", or "it" (the
								///               latter in the case of neuter or hooded
								///               actors).
								/// </li>
							
						case 'e':
							if (is_hooded())
							{
								chsex = sex;
								sex = 0;
								i = HSSH (this);
								sex = chsex;
							}
							else
								i = HSSH (this);
							color = 0;
							break;
							
							
								/// <li>
								///       \c $E - victim as "he", "she", or "it" (the
								///               latter in the case of neuter or hooded
								///               victims).
								/// </li>
							
						case 'E':
							tch = (CHAR_DATA *) vict_obj;
							if (tch->is_hooded())
							{
								chsex = tch->sex;
								tch->sex = 0;
								i = HSSH (tch);
								tch->sex = chsex;
							}
							else
								i = HSSH (tch);
							color = 0;
							break;
							
							
								/// <li>
								///       \c $o - object name or "something" if hidden
								///               from the recipient
								/// </li>
							
						case 'o':
							i = OBJN (obj, to);
							color = 2;
							break;
							
							
								/// <li>
								///       \c $O - victim object name or "something" if
								///         hidden from the recipient
								/// </li>
							
						case 'O':
							i = OBJN ((OBJ_DATA *) vict_obj, to);
							color = 2;
							break;
							
							
								/// <li>
								///       \c $p - object short description or "something"
								///               if hidden from the recipient
								/// </li>
							
						case 'p':
							i = OBJS (obj, to);
							color = 2;
							break;
							
							
								/// <li>
								///       \c $P - victim object short description or
								///              "something" if hidden from the recipient
								/// </li>
							
						case 'P':
							i = OBJS ((OBJ_DATA *) vict_obj, to);
							color = 2;
							break;
							
							
								/// <li>
								///       \c $a - "a" or "an" object (based on name)
								/// </li>
							
						case 'a':
							i = SANA (obj);
							break;
							
							
								/// <li>
								///       \c $A - "a" or "an" victim object (based on
								///               name)
								/// </li>
							
						case 'A':
							i = SANA ((OBJ_DATA *) vict_obj);
							break;
							
							
								/// <li>
								///       \c $T - Cast parameter \c vict_obj as a string
								///               and set to local \c i (?).
								/// </li>
							
						case 'T':
							i = (char *) vict_obj;
							break;
							
							
								/// <li>
								///       \c $F - Cast parameter \c vict_obj as a string
								///               of names and set to local \c i to the
								///               first of them (?).
								/// </li>
							
						case 'F':
							i = fname ((char *) vict_obj);
							break;
							
							
								/// <li>
								///       \c $$  - Insert a dollar-sign
								/// </li>
							
						case '$':
							i = "$";
							break;
						default:
							break;
							
							
								/// </ul></li>
					}
					
					
						/// <li>  Do not copy local \c i to \c buf \e IF there is
						///       nothing to insert (\e OR we are at the end of the
						///       buffer), \e ELSE Capitalize, color, and copy.</li>
					
					if (!i || !point)
					{
						;
					}
					else
					{
						if (point == buf)
							do_cap = 1;
						else
							do_cap = 2;
						
						if (color != 0)
						{
							*point++ = '#';
							*point++ = '0' + color;
						}
						
						if (*i)
						{
							
							*point = *(i++);
							
							if (do_cap == 1)
								point[0] = toupper (point[0]);
							else if (do_cap == 2)
								point[0] = tolower (point[0]);
							
							point++;
							do_cap = 0;
						}
						while (((*point) = (*(i++))))
							++point;
						
						if (color != 0)
						{
							*point++ = '#';
							*point++ = '0';
						}
					}
					++strp;
				}
				else if (!(*(point++) = *(strp++)))
				{
					break;
				}
				
					/// </li></ul>
				
			}
			
			
				/// </li>
				/// <li>  Cap the end of the output buffer with a newline & null.
				/// </li>
			
			*(--point) = '\r';
			*(++point) = '\n';
			*(++point) = '\0';
			
			
				/// <li>  We do not send the buffer:
				/// <ul>
				/// <li>  \e IF this is a search/scan message </li>
				/// <li>  \e AND the recipient is grouped or admin/li>
				/// </ul>
			if (IS_SET (type, _ACT_SEARCH)
				&& (are_grouped (this, to)
					|| to->get_trust()))
			{
				continue;
			}
			
			
				/// <li>  Reformat the buffer \e IF the \c type parameter is set
				///       accordingly </li>
				/// <li>  Send the buffer to the recipient </li>
			if (IS_SET (type, _ACT_FORMAT))
			{
				reformat_string (buf, &p);
				if (*p != '\r' && *p != '\n' && IS_SET (type, TO_VICT)
					&& to == vict_obj)
					to->send_to_char ("\n");
				to->send_to_char (p);
				free_mem (p);
			}
			else
				to->send_to_char (buf);
		}
		
		
			/// <li>  \e IF the action \c type is to a single recipient, we can
			///       stop iterating. \e ELSE process the next recipient.
		
		if (IS_SET (type, TO_VICT) || IS_SET (type, TO_CHAR))
			return;
		
	}
		/// </ul>
		/// <li>  Return to the caller after the message has been sent to any
		///       recipients </li>
		/// </ul>
	
		// TODO LIST
		/// \todo  Consider enumerating the type of the \c type parameter.
		/// \todo  Consider swapping out \c MAX_STRING_LENGTH for something more
		///        reasonable.
		/// \todo  Ensure \c immbuf1 and \c immbuf2 are redundant and merge them.
		/// \todo  Consider using an #ifndef block and a command-line option to decide
		///        if we want strong argument checking compiled into the server, and
		///        available as a command-line option.
		/// \todo  Evaluate critcalness of bad parameters and handle with a warning
		///        or error as appropriate.
		/// \todo  Remove "sex change" sequences when getting the pronoun of a hooded
		///        actor (ideally the \c ch parameter would be a const).
		/// \todo  Consider making \c color an enumerated type.
		/// \todo  Detect multi-sentence actions and utilize \c do_cap appropriately.
		/// \todo  Consider giving immortals the option to see names only (no sdesc).
}

void char_data::assign_hit_points()
{
	if (race == 28) //trolls - NPC
	{
		max_hit = 200 + tmp_con * CONSTITUTION_MULTIPLIER + (MIN(tmp_aur,25) * 4);
	}
	else if (race == 86) {//olog-hai - PC
		max_hit = 200 + tmp_con * CONSTITUTION_MULTIPLIER + (MIN(tmp_aur,25) * 4);
	}
	else
	{
		max_hit = 50 + tmp_con * CONSTITUTION_MULTIPLIER + (MIN(tmp_aur,25) * 4);
	}
	
	
	return;
	
}

void char_data::apply_race_affects()
{
	int affects = 0;
	
	if (race < 0)
		return;
	
	if (lookup_race_variable (race, RACE_AFFECTS))
	{
		affects = atoi (lookup_race_variable (race, RACE_AFFECTS));
		if (IS_SET (affects, INNATE_INFRA))
			affected_by |= AFF_INFRAVIS;
		else
			affected_by &= ~AFF_INFRAVIS;
		
		if (IS_SET (affects, INNATE_WAT_BREATH))
			affected_by |= AFF_BREATHE_WATER;
		else
			affected_by &= ~AFF_BREATHE_WATER;
		
		if (IS_SET (affects, INNATE_SUN_PEN))
			affected_by |= AFF_SUNLIGHT_PEN;
		else
			affected_by &= ~AFF_SUNLIGHT_PEN;
		
		if (IS_SET (affects, INNATE_SUN_PET))
			affected_by |= AFF_SUNLIGHT_PET;
		else
			affected_by &= ~AFF_SUNLIGHT_PET;
		
		if (!pc)
		{
			if (IS_SET (affects, INNATE_FLYING))
				mob->action |= ACT_FLYING;
			else
				mob->action &= ~ACT_FLYING;
			
			if (IS_SET (affects, INNATE_NOBLEED))
				mob->action |= ACT_NOBLEED;
			else
				mob->action &= ~ACT_NOBLEED;
		}
	}
}

void char_data::equip_newbie()
{
	OBJ_DATA *tobj = NULL;
	OBJ_DATA *obj = NULL;
	
	int rand_num = 0;
	
	for (tobj = equip; tobj; tobj = tobj->next_content)
	{
		if (tobj == equip)
			equip = equip->next_content;
		else
			equip->next_content = tobj->next_content;
	}
	
	
		// TODO: THIS IS WHERE WE GIVE NEWBS THEIR STARING GEAR
	
		//Hobbit Party Pavillion for GUESTS only
	if (IS_SET (flags, FLAG_GUEST))
	{
		if (sex == 1)
		{
			if ((obj = load_object (2007))) //tunic
				equip_char (this, obj, WEAR_BODY);
			if ((obj = load_object (2009))) //leggings
				equip_char (this, obj, WEAR_LEGS);
		}
		else 
		{
			if ((obj = load_object (2055))) //dress
				equip_char (this, obj, WEAR_BODY);
		}
		
		if ((obj = load_object (97422))) //cloak
			equip_char (this, obj, WEAR_ABOUT);
		if ((obj = load_object (11070))) //belt
			equip_char (this, obj, WEAR_WAIST);
		
		if ((obj = load_object (90096))) //satchel over the shoulder
		{
			equip_char (this, obj, WEAR_SHOULDER_L);
			tobj = obj;
		}
		
			// backpack gear...bread knife etc 
		if (tobj)
		{
			if ((obj = load_object (1281))) //bread
				obj_to_obj (obj, tobj);
			
			rand_num = number (0,2);
			if (rand_num == 0)
			{
				if ((obj = load_object (91479))) //cheese
					obj_to_obj (obj, tobj);
			}
			else if (rand_num == 1)
			{
				if ((obj = load_object (91420))) //cheese
					obj_to_obj (obj, tobj);
			}
			else 
			{
				if ((obj = load_object (1739))) //cheese
					obj_to_obj (obj, tobj);
				
			}
			
			rand_num = number (0,2);
			if (rand_num == 0)
			{
				if ((obj = load_object (90887))) //cookie
					obj_to_obj (obj, tobj);
				if ((obj = load_object (97496))) //cookie
					obj_to_obj (obj, tobj);
				if ((obj = load_object (97496))) //cookie
					obj_to_obj (obj, tobj);
				
			}
			else if (rand_num == 1)
			{
				if ((obj = load_object (91169))) //apple tart
					obj_to_obj (obj, tobj);
			}
			else 
			{
				if ((obj = load_object (1524))) //fruit tart
					obj_to_obj (obj, tobj);
				
			}
			
			if ((obj = load_object (1266))) //apple
				obj_to_obj (obj, tobj);
			
			if ((obj = load_object (1560))) //gondorian waterskin
			{
				obj->o.od.value[1] = 7;
				obj_to_obj (obj, tobj);
			}
			
			if ((obj = load_object (1015))) //knife
				obj_to_obj (obj, tobj);
			if ((obj = load_object (97650))) //torch
				obj_to_obj (obj, tobj);
			
			
		} //end Hobbit party Pavillion Guests
	}
	
	
	right_hand = NULL;
	left_hand = NULL;
	
	save_char (this, true);
}

void char_data::setup_new_character()
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char *count_skill_name;
	std::map<std::string, SKILL_DATA*>::iterator skill_map_it;
	std::map<std::string, int>::iterator found_it;
	SKILL_DATA* tskill;
	
	/*      Add in auto-selected skills here, as well as psi-check          */
	speed = SPEED_WALK;
	
	pc->create_state = STATE_APPROVED;
	
	str = pc->start_str;
	dex = pc->start_dex;
	intel = pc->start_intel;
	wil = pc->start_wil;
	aur = pc->start_aur;
	con = pc->start_con;
	agi = pc->start_agi;
	luk = pc->start_luk;
	
	
	tmp_str = str;
	tmp_con = con;
	tmp_intel = intel;
	tmp_wil = wil;
	tmp_aur = aur;
	tmp_dex = dex;
	tmp_agi = agi;
	tmp_luk = luk;
	
	max_hit = 50 + (CONSTITUTION_MULTIPLIER * tmp_con) + (MIN(tmp_aur,25) * 4);
	
	max_move = calc_lookup (this, REG_MISC, MISC_MAX_MOVE);
	
	hit = max_hit;
	move = max_move;
	
		//add in all inate skills at expose level
	for (skill_map_it = skill_data_map.begin(); skill_map_it != skill_data_map.end(); skill_map_it++)
	{
		tskill = skill_map_it->second;
		count_skill_name = strdup(tskill->skill_name.c_str());
		
			//find innate skills
		if(tskill->innate == 1)
		{
				//if they don't have the skill, expose them to it
			found_it = skill_map.find(count_skill_name);
			if (found_it == skill_map.end())
				expose_skill(this, lookup_skill_id(count_skill_name));
			
		}
	}
	
	
	int nat_tongue = get_native_tongue(); // this is race0 aware 
	if (nat_tongue)
	{
		skill_map[lookup_skill_name(nat_tongue)] = calc_lookup (this, REG_CAP, nat_tongue);
	}
	
		// Define all race-specific characteristics
	
	starting_skill_boost (this, "Search");
	starting_skill_boost (this, "Listen");
	starting_skill_boost (this, "Scan");
	starting_skill_boost (this, "Dodge");
	starting_skill_boost (this, "Parry");
	starting_skill_boost (this, "Block");
	starting_skill_boost (this, "Brawling");
	starting_skill_boost (this, "Climb");
	
	
	pc->nanny_state = 0;
	
	if (race == 15 || race == 16 || race == 17 ||
		race == 18 || race == 19 || race == 23 ||
		race == 27 || race == 28 || race == 86 || 
		race == 93 || race == 119 || race == 120 
		|| race == 121)
	{      
		if (!IS_SET (affected_by, AFF_INFRAVIS))
			affected_by |= AFF_INFRAVIS;
	}
	
	
	description = duplicateString((reformat_desc((char*)description).c_str()));
	
	plr_flags |= (NEWBIE | NEWBIE_HINTS | NEW_PLAYER_TAG);
	
	time_str.played = 0;
	
	sprintf (buf, "save/objs/%c/%s", tolower (*name), name);
	unlink (buf);
	
	in_room = NOWHERE;
}

int char_data::move_gain()
{
	float gain;
	float moves_gained;
	float con_stat;
	float time_stat;
	float condition_stat = 0;
	
	/* Move_rate is 100 moves per 5 mins  ( * 10000 for granularity ) */
		// with an adjsutment for CON comapred to average value of 10
		//CON of less than 10 must SIT/REST/SLEEP to re-gain stamina
	con_stat = (tmp_con/10.0) * 100.0;
	time_stat = (5.0 * SECOND_PULSE * 60.0);
	gain = 10000 * (con_stat)/time_stat;
	
	switch (position)
	{
		case SLEEP:
			gain = gain * 8.0;
			break;
		case REST:
			gain = gain * 4.0;
			break;
		case SIT:
			gain = gain * 2.0;
			break;
	}
	
	
	if (!hunger || !thirst)
		gain = gain * 4.0;
	
	if (move_points < 0)
		move_points = 0;
	
		//move point change, not actual move points remaining
	move_points += gain;
	
	moves_gained = move_points / 1000.0;
	
	
	move_points -= moves_gained * 1000.0;
	
	if (IS_SET (room->room_flags, OOC) && !IS_SET (flags, FLAG_GUEST))
		moves_gained = 0;
	
	return (int)moves_gained;
}

void char_data::char_from_room()
{
	CHAR_DATA *i;
	ROOM_DATA *troom;
	AFFECTED_TYPE *af;
	
	if (in_room == NOWHERE)
	{
		system_log
		("NOWHERE extracting char from room (handler.c, char_from_room)",
		 true);
		room = NULL;
			//abort ();
	}
	
	if (room == NULL)
		return;
	
	troom = room;
	
	
	if (this == troom->people)	/* head of list */
		troom->people = next_in_room;
	
	else
	{				/* locate the previous element */
		
		for (i = troom->people; i; i = i->next_in_room)
		{
			if (i->next_in_room == this)
			{
				i->next_in_room = next_in_room;
				break;
			}
		}
		
	}
	
	in_room = NOWHERE;
	room = NULL;
	next_in_room = NULL;
	
	if ((af = get_affect (this, MAGIC_SIT_TABLE)))
		remove_affect_type (this, MAGIC_SIT_TABLE);
	
	if (troom)
	{
		room_light(troom);
		if (!get_trust()
			&& troom->nVirtual >= 100000
			&& IS_SET (troom->room_flags, TEMPORARY))
		{
			if (!troom->people)
				room_delete (troom);
		}
	}
}

void char_data::char_to_room(int room_num)
{
	ROOM_DATA *troom = NULL;
	std::map<int, ROOM_DATA*>::iterator room_iterator;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int curr = 0;
	
	troom = vtor(room_num);
	
	if ( !troom )
	{
		sprintf (buf, "Room %d doesn't exist in char_to_room()! (%s)", room_num,
				 name);
		system_log (buf, true);
		
		troom = vtor(VNUM_DEFAULT_LOAD_ROOM);
		if (!troom)
		{
			sprintf (buf, "VNUM_DEFAULT_LOAD_ROOM doesn't exist in char_to_room! (%s)", name);
			system_log (buf, true);
			room_iterator = room_map.begin();
			troom = room_iterator->second;
		}
	}
	
	
	if (troom->people 
		&& (troom->people != this))
		next_in_room = troom->people;
	else
		next_in_room = NULL;
	
	troom->people = this;
	
	in_room = troom->nVirtual;
	room = troom;
	
	
	
	if (!troom->psave_loaded)
		load_save_room(troom);
	
	room_light(room);
}

char * char_data::fatigue_bar()
{
	static char buf[25] = { '\0' };
	float calc = 0;
	float move = 0;
	
	if (move >= max_move)
		sprintf (buf, "#1||#3||#2||#0");
	
	if ((move = move) >= (calc = max_move * .6667)
		&& move < max_move)
		sprintf (buf, "#1||#3||#2|#0 ");
	
	if ((move = move) >= (calc = max_move * .5)
		&& (move = move) < (calc = max_move * .6667))
		sprintf (buf, "#1||#3||#0  ");
	
	if ((move = move) >= (calc = max_move * .3333)
		&& (move = move) < (calc = max_move * .5))
		sprintf (buf, "#1||#3|#0   ");
	
	if ((move = move) >= (calc = max_move * .1667)
		&& (move = move) < (calc = max_move * .3333))
		sprintf (buf, "#1||#0    ");
	
	if ((move = move) >= (calc = max_move * .0001)
		&& (move = move) < (calc = max_move * .1667))
		sprintf (buf, "#1|#0     ");
	
	if (move == 0)
		sprintf (buf, "       ");
	
	
	return buf;
}

void char_data::clear_travel()
{
	if (travel_str)
	{
		free_mem (travel_str); // char*
		travel_str = NULL;
	}
}

void char_data::clear_dmote()
{
	if (dmote_str)
	{
		free_mem (dmote_str); // char*
		dmote_str = NULL;
	}
}

void char_data::clear_pmote()
{
	if (pmote_str)
	{
		free_mem (pmote_str); // char*
		pmote_str = NULL;
	}
}


void char_data::clear_voice()
{
	if (voice_str)
	{
		free_mem (voice_str); // char*
		voice_str = NULL;
	}
}

void char_data::extract_char()
{
	CHAR_DATA *k;
	CHAR_DATA *next_char;
	CHAR_DATA *tch;
	ROOM_DATA *troom;
	OBJ_DATA *tobj;
	DESCRIPTOR_DATA *td;
	AFFECTED_TYPE *af;
	int was_in;
	char buf[MAX_STRING_LENGTH] = {'\0'};
	std::list<char_data*>::iterator tch_iterator;
	
	extern struct timeval time_now;
	
	if (!pc)
	{
		if (IS_SET (mob->action, ACT_STAYPUT))
		{			
			sprintf(buf, "DELETE FROM stayput_mobs WHERE coldload_id = %d",
					coldload_id);
			mysql_safe_query(buf);
			
			sprintf (buf, "save/mobiles/%d", coldload_id);
			unlink (buf);
		}
	}
	
	if (mob 
		&& !pc
		&& IS_SET (mob->profession, PROF_VEHICLE)
		&& (troom = vtor (mob->nVirtual)))
	{
		
		/* By mistake, the vehicle might be in the same room as its
		 entrance room.  In that case, n/pcs and mobs don't need to
		 be moved.
		 */
		
		if (troom == room)
		{
			act("$n is destroyed and some people fall out.",
					false, 0, 0, TO_ROOM);
			for (tch = troom->people; tch; tch = tch->next)
				tch->vehicle = NULL;
		}
		
		else
		{
			
			while (troom->people)
			{
				tch = troom->people;
				tch->char_from_room();
				tch->char_to_room(in_room);
				tch->vehicle = NULL;
				act("$N falls out of $n.", false, 0, tch, TO_NOTVICT);
				tch->act("You fall out of $N as it is destroyed!",
						 false, 0, this, TO_CHAR);
			}
			
			while (troom->contents)
			{
				tobj = troom->contents;
				obj_from_room (&tobj, 0);
				obj_to_room (tobj, in_room);
			}
		}
	}
	
	if (desc && desc->original)
		do_return (this, NULL, 0);
	
	if (pc && room)
		save_attached_mobiles (this, 1);
	
	if (pc)
		clear_watch();
	
	
	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		k = *tch_iterator;
		if (k->deleted)
			continue;
		
		if (k->subdue == this)
			k->subdue = NULL;
		
	}
	
	if (pc && pc->edit_player)
		pc->edit_player->unload_pc();
	
	if (pc)
		save_char (this, true);
	
	if (pc && !desc)
	{
		for (td = descriptor_list; td; td = td->next)
			if (td->original == this)
				do_return (td->character, NULL, 0);
	}
	
	if (in_room == NOWHERE)
	{
		system_log ("NOWHERE extracting char. (handler.c, extract_char)", true);
			//abort ();
	}
	
	stop_followers();
	
	
	if (desc)
	{
		if (desc->snoop.snooping && desc->snoop.snooping->desc)
			desc->snoop.snooping->desc->snoop.snoop_by = 0;
		
		if (desc->snoop.snoop_by && desc->snoop.snoop_by->desc)
		{
			desc->snoop.snoop_by->send_to_char("Your victim is no longer among us.\n");
			desc->snoop.snoop_by->desc->snoop.snooping = 0;
		}
		
		desc->snoop.snooping = desc->snoop.snoop_by = 0;
	}
	
	
	
	if (get_queue_position() != -1)
		update_assist_queue(this, true);
	
	/* Must remove from room before removing the equipment! */
	
	was_in = in_room;
	char_from_room();
	in_room = was_in;
	
	if (right_hand)
		extract_obj (right_hand);
	
	if (left_hand)
		extract_obj (left_hand);
	
	while (equip)
		extract_obj (equip);
	
	deleted = 1;
	
	if (!pc)
	{
		while (hour_affects)
			affect_remove (this, hour_affects);
	}
	
	if (desc != NULL)
		desc->character = NULL;
	
	
	character_list.remove(this);
	
	
	if (desc != NULL && !desc->acct
		&& !IS_SET (flags, FLAG_GUEST))
	{
		td = desc;
		if (!maintenance_lock)
			SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), td);
		else
			SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
					   td);
		SEND_TO_Q ("Your Choice: ", td);
		td->connected = CON_LOGIN;
		desc = NULL;
		td->character = NULL;
		td->original = NULL;
		td->prompt_mode = 0;
		td->login_time = time_now;
		td->time_last_activity = mud_time;
	}
}

void char_data::hunger_thirst_process()
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char update_buf[MAX_STRING_LENGTH] = { '\0' };
	
	if (!pc)
		return;
	
	if (get_trust())
		return;
	
	*update_buf = '\0';
	
	if (get_comestible_range_food(hunger) !=
		get_comestible_range_food(hunger - 1))
	{
		sprintf (update_buf + strlen (update_buf), "%s",
				 verbal_hunger[get_comestible_range_food(hunger - 1)]);
	}
	
	if (get_comestible_range (thirst) !=
		get_comestible_range (thirst - 1))
	{
		if (*update_buf)
			sprintf (update_buf + strlen (update_buf), ", and ");
		sprintf (update_buf + strlen (update_buf), "%s",
				 verbal_thirst[get_comestible_range (thirst - 1)]);
	}
	
	if (*update_buf)
	{
		sprintf (buf, "#6You are %s.#0\n", update_buf);
		send_to_char(buf);
	}
	
		//checked once an hour during the day, so you lose 1/24 each check
	if (hunger > 0)
		hunger = hunger - (int)(MAX_CALORIE/24);
	
	if (thirst > 0)
		thirst = thirst - (int)(MAX_WATER/24);
	
	if (thirst < -1)
		thirst = 1;
	
	if (hunger < -1)
		hunger = 1;
}

int char_data::get_trust()
{
	CHAR_DATA *tch = NULL;
	
	if (!desc || (flags & FLAG_GUEST))
	{
		return 0;
	}
	
	tch = desc->original != NULL ? desc->original : desc->character;
	
	if (!tch || !tch->pc || tch->pc->mortal_mode)
	{
		return 0;
	}
	
	
	return pc->level;
}

int char_data::get_position()
{
	return position;
}

void char_data::set_position(int value)
{
	position = value;
}

char * char_data::get_name()
{
	return (name);
}

int char_data::carrying()
{
	int mass = 0;
	OBJ_DATA *obj;
	
	if (right_hand)
		mass += obj_mass (right_hand);
	
	if (left_hand)
		mass += obj_mass (left_hand);
	
	for (obj = equip; obj; obj = obj->next_content)
		mass += obj_mass (obj);
	
	return mass;
}

bool char_data::is_encumbered()
{
	if ((tmp_str * enc_tab [1].str_mult_wt) < carrying())
		return true;
	else 
		return false;
}
	
bool char_data::is_awake()
{
	if (get_position() > SLEEP)
		return true;
	else 
		return false;
}

int char_data::can_carry_weight()
{
	
	return (tmp_str * 2500);
}

	//subduer and subduee - same variable, but different flags
bool char_data::is_subduee()
{
	if (is_he_here (this, subdue, 0)
		&& GET_FLAG (this, FLAG_SUBDUEE))
		return true;
	else 
		return false;
	
}

bool char_data::is_subduer()
{
	if (is_he_here (this, subdue, 0)
		&& GET_FLAG (this, FLAG_SUBDUER))
		return true;
	else 
		return false;
	
}

int char_data::get_size()
{
	int size = -1;
	int weight;
	
	weight = this->get_weight() / 100;
	
	if (weight < 63)
		size = 1;			/* XXS */
	else if (weight > 230)
		size = 7;
	else if ((height - 41) + weight < 125)
		size = 2;			/* XS  */
	else if ((height - 41) + weight < 160)
		size = 3;			/* S   */
	else if ((height - 41) + weight < 201)
		size = 4;			/* M   */
	else if ((height - 41) + weight < 241)
		size = 5;			/* L   */
	else if ((height - 41) + weight < 280)
		size = 6;			/* XL  */
	else
		size = 7;
	
	return size;
}

int char_data::get_weight()
{
	int tmp_weight = 0;
	
	if (frame && !bmi)
	{
		if (frame == 1)
			bmi = number (19, 20);
		else if (frame == 2)
			bmi = number (20, 21);
		else if (frame == 3)
			bmi = number (22, 25);
		else if (frame == 4)
			bmi = number (26, 29);
		else if (frame == 5)
			bmi = number (29, 32);
		else
			bmi = number (22, 26);
		
	}
	else
	{				// Let's assume medium BMI unless otherwise
		bmi = number (22, 26);
	}
	
		// formula: bmi = (weight/2.2046) / (height/39.3696)squared
	tmp_weight = (height * height * bmi) / 704;
	
		// weight in pounds based on BMI and height
	
	return (100*tmp_weight);
}

void char_data::clear_moves()
{
	MOVE_DATA *tmp_move = NULL;
	AFFECTED_TYPE *af;
	
	while (moves)
	{
		tmp_move = moves;
		moves = tmp_move->next;
		if (tmp_move->travel_str)
			free_mem (tmp_move->travel_str);
		free_mem (tmp_move); // MOVE_DATA*
	}
	
	
	flags &= ~(FLAG_ENTERING | FLAG_LEAVING);
	
	if ((af = get_affect (this, MAGIC_DRAGGER)))
		affect_remove (this, af);
}

int char_data::clear_current_move()
{
	CHAR_DATA *tch;
	QE_DATA *qe;
	QE_DATA *tqe;
	
	if (GET_FLAG (this, FLAG_ENTERING))
		return 0;
	
	for (qe = quarter_event_list; qe; qe = qe->next)
		if (qe->ch == this)
			break;
	
	if (!qe)
		return 0;
	
	if (qe == quarter_event_list)
		quarter_event_list = qe->next;
	else
	{
		
		for (tqe = quarter_event_list; tqe->next != qe; tqe = tqe->next)
			;
		
		tqe->next = qe->next;
	}
	
	flags &= ~FLAG_LEAVING;
	if (qe->travel_str)
		free_mem (qe->travel_str);
	free_mem (qe); // QE_DATA*
	
	for (tch = this->room->people; tch; tch = tch->next_in_room)
	{
		
		if (tch == this || tch->deleted)
			continue;
		
		if (tch->following == this && can_see_mob(tch, this))
		{
			tch->clear_moves();
			tch->clear_current_move();
		}
	}
	
	return 1;
}

bool char_data::is_blind()
{
	if (get_trust())
		return false;
	
	if (get_equip (this, WEAR_BLINDFOLD))
		return true;
	
	return false;
}

bool char_data::forced_wakeup()
{
	AFFECTED_TYPE *af;
	
	if (get_position() != SLEEP)
		return false;
	
	if (get_affect (this, MAGIC_AFFECT_SLEEP))
		return false;
	
	this->send_to_char ("Your sleep is disturbed.\n");
	
	if ((af = get_affect (this, MAGIC_SIT_TABLE)) &&
		is_obj_in_list (af->a.table.obj, room->contents))
		set_position(SIT);
	else
		set_position(REST);
	
	return true;
}

bool char_data::would_reveal()
{
	AFFECTED_TYPE *was_hidden;
	CHAR_DATA *tch;
	
	/* Remove the hide affect to test can_see_mob.  We don't reveal the
	 PC if s/he couldn't be seen anyway. */
	
	if ((was_hidden = get_affect (this, MAGIC_HIDDEN)))
		affect_remove (this, was_hidden);
	
	for (tch = room->people; tch; tch = tch->next_in_room)
	{
		
		if (tch == this)
			continue;
		
		if (!tch->is_awake())
			continue;
		
		if (tch->get_trust())	/* Imms don't count */
			continue;
		
		if (!can_see_mob(tch, this))
			continue;
		
		if (are_grouped (tch, this))
			continue;
		
		if (was_hidden)
			magic_add_affect (this, MAGIC_HIDDEN, -1, 0, 0, 0, 0);
		
		return true;
	}
	
	if (was_hidden)
		magic_add_affect (this, MAGIC_HIDDEN, -1, 0, 0, 0, 0);
	
	return false;
}

bool char_data::can_move()
{
	AFFECTED_TYPE *af;
	
	if ((af = get_affect (this, MAGIC_AFFECT_PARALYSIS))
		|| is_subduee() 		
		|| get_position() == UNCON)
		return false;
	
	return true;
}

void char_data::unload_pc()
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	
	if (!name || !*name)
	{
		delete (this);
		return;
	}
	
	if (islower (*name))
		*name = toupper(*name);
	
	if (pc)
		sprintf (buf, "(unload) Unloading %s (lc = %d)", name,
				 pc->load_count);
	else
		sprintf (buf, "Unloading char: %s\n", name);
	
	system_log (buf, false);
	
	if (!pc->load_count)
	{
		system_log ("Uh oh, PC was not loaded!", true);
		system_log (name, true);
		return;
	}
	
	pc->load_count--;
	
	if (!deleted)
	{
		sprintf (buf, "Saving character %s", name);
		system_log (buf, false);
		save_char (this, false);
	}
	
	if (pc->load_count)
	{
		return;
	}
	
	character_list.remove(this);
	delete(this);
	return;
}

void char_data::pc_to_game()
{
	char* skill_name;
	std::map<std::string, SKILL_DATA*>::iterator skill_it;
	SKILL_DATA* tskill;
	
		//are they already in character_list?
	if (get_pc (name))
		return;
	
	character_list.push_front(this);
	
	if (!writes)
	{
		for (skill_it = skill_data_map.begin(); skill_it != skill_data_map.end(); skill_it++)
		{
			tskill = skill_it->second;
			skill_name = strdup(tskill->skill_name.c_str());
			if ((tskill->written == 1)
				&& (skill_map[skill_name] > 0))
				writes = strdup(skill_name);
		}
		
	}
}



int char_data::get_queue_position()
{
	CHAR_DATA *tch;
	int i = 0;
	bool found = false;
	
	for (tch = assist_queue; tch; tch = tch->next_assist)
	{
		i++;
		if (tch == this)
		{
			found = true;
			break;
		}
	}
	
	if (!found)
		return -1;
	else
		return i;
}

int char_data::get_native_tongue()
{
	char* native_tongue = 0;
	
		// retrieve the default racial tongue 
	native_tongue = lookup_race_variable (race, RACE_NATIVE_TONGUE);
	
		// unless they are human, which have different natives based on location 
	if (race == 0)
	{
		if (IS_SET (plr_flags, START_ANGRENOST))
			return lookup_skill_id("Westron");
		
		
			// Error - human is not starting anywhere. Set to Westron 
		fprintf(stderr,"Error - Common Human %s not starting anywhere\n",name);
		return lookup_skill_id("Westron");
	}
	
		// error with race 
	if (atoi(native_tongue)==0)
	{
		fprintf(stderr,"Error - race %d has null starting tongue\n",race);
		return 0;
	}
	
		// otherwise return the racial native 
	return atoi(native_tongue);
}


CHAR_DATA * char_data::being_dragged()
{
	CHAR_DATA *tch;
	AFFECTED_TYPE *af;
	
	for (tch = room->people; tch; tch = tch->next_in_room)
	{
		
		if (tch->deleted || tch == this)
			continue;
		
		if ((af = get_affect (tch, MAGIC_DRAGGER)) && af->a.spell.t == (long int) this)
			return tch;
	}
	
	return NULL;
}

void char_data::clear_watch()
{
	CHAR_DATA *tch;
	AFFECTED_TYPE *af;
	std::list<char_data*>::iterator tch_iterator;
	
	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		tch = *tch_iterator;
		
		if (tch->deleted || IS_NPC (tch))
			continue;
		
		if ((af = get_affect (tch, MAGIC_WATCH1)) && af->a.spell.t == (long int) this)
			affect_remove (tch, af);
		
		if ((af = get_affect (tch, MAGIC_WATCH2)) && af->a.spell.t == (long int) this)
			affect_remove (tch, af);
		
		if ((af = get_affect (tch, MAGIC_WATCH3)) && af->a.spell.t == (long int) this)
			affect_remove (tch, af);
	}
}

void char_data::show_unread_messages()
{
	int header = 1;
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char buf2[AVG_STRING_LENGTH] = "";
	char query[MAX_STRING_LENGTH] = { '\0' };
	char temp_buf[MAX_STRING_LENGTH];
	
	MYSQL_RES *result = NULL;
	MYSQL_ROW row = NULL;
	
	mysql_safe_query
	("SELECT * FROM player_notes WHERE name = '%s' AND flags = 0 ORDER BY post_number DESC",
	 name);
	result = mysql_store_result (database);
	
	*temp_buf = '\0';
	
	while ((row = mysql_fetch_row (result)))
	{
		if (header)
		{
			sprintf (temp_buf, "\nUnread messages on your private board:\n\n");
			header = false;
		}
		if (strlen (row[2]) > 44)
		{
			sprintf (query, "%s", row[2]);
			query[41] = '.';
			query[42] = '.';
			query[43] = '.';
			query[44] = '\0';
		}
		else
			sprintf (query, "%s", row[2]);
		sprintf (temp_buf + strlen (temp_buf), " #6%3d#0 - %16s %-10.10s: %s\n",
				 atoi (row[1]), row[4], row[3], query);
	}
	
	if (!header)			/* Meaning, we have something to print */
		page_string (desc, temp_buf);
	
	mysql_free_result (result);
	result = NULL;
	
	mysql_safe_query
	("SELECT board_name, MIN(post_number) post_from, MAX(post_number) post_to, author, subject FROM virtual_boards WHERE timestamp >= %d GROUP BY board_name ORDER BY board_name ASC",
	 (int) pc->last_logon);
	result = mysql_store_result (database);
	header = true;
	
	while ((row = mysql_fetch_row (result)))
	{
		if (header)
		{
			sprintf (temp_buf, "\nWelcome back! Since you last logged in:\n\n");
			send_to_char(temp_buf);
			header = false;
		}
		if (strcmp (row[1], row[2]) == 0)
		{
			if (strcmp (row[0], "Applications") == 0)
			{
				if (strstr (row[4], "Accepted"))
				{
					sprintf (buf2, "#6%s", row[4] + 14);
				}
				else
				{
					sprintf (buf2, "#1%s", row[4] + 14);
				}
			}
			else if (strcmp (row[0], "Crashes") == 0)
			{
				sprintf (buf2, "#6%s", row[3]);
			}
			else if (strcmp (row[0], "Helpfiles") == 0)
			{
				sprintf (buf2, "#6%s", row[4] + 15);
			}
			else if (strcmp (row[0], "Petitions") == 0)
			{
				sprintf (buf2, "#6%s", row[3]);
			}
			else if (strcmp (row[0], "Submissions") == 0)
			{
				row[4][strlen (row[4] - 2)] = '\0';
				sprintf (buf2, "#6%s", row[4] + 2);
			}
			else
			{
				sprintf (buf2, "#6%s", row[4]);
			}
			if (strlen (buf2) > 28)
			{
				strcpy (buf2 + 25, "...");
			}
			sprintf (buf, "   - '#2%s#0' was posted to (msg %s: %s#0).\n",
					 row[0], row[1], buf2);
		}
		else
		{
			sprintf (buf, "   - '#2%s#0' was posted to (msgs %s to %s).\n",
					 row[0], row[1], row[2]);
		}
		send_to_char(buf);
	}
	
	mysql_free_result (result);
}

void char_data::stop_followers()
{
	CHAR_DATA *tch;
	std::list<CHAR_DATA*>::iterator tch_iterator;
	
	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		tch = *tch_iterator;
		if (tch->deleted)
			continue;
		
		if (tch->following == this)
		{
			tch->following = 0;			
		}
	}
}

CHAR_DATA * char_data::is_switched()
{
	CHAR_DATA *tch;
	std::list<CHAR_DATA*>::iterator tch_iterator;
	
		//for (tch = character_list; tch; tch = tch->next)
	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		tch = *tch_iterator;
		if (tch->deleted)
			continue;
		if (!tch->desc)
			continue;
		if (tch->desc->original && tch->desc->original == this)
			return tch;
	}
	
	return NULL;
}

void char_data::add_soma(SOMA_TYPE* soma)
{

}