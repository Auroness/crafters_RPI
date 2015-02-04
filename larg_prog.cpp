//////////////////////////////////////////////////////////////////////////////
//
/// larg_prog.cpp - Lua programing interfaceand Functions
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


/* constants */
#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"
#include </usr/local/mysql/include/mysql.h>
#include </usr/local/include/mysql++/mysql++.h>
extern mysqlpp::Connection dbo;
extern MYSQL *database;
extern "C"
{
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}

	//lua_State		*luaVM;



/* call a function `f' defined in Lua */
double f (double x, double y)
{
	double z;
	char buf[MAX_STRING_LENGTH];
	lua_State *luaVM = lua_open();   /* opens Lua */
	luaL_openlibs(luaVM);

	
	send_to_imps("inside the function\n");
	
    sprintf(buf, "%s/lua_test_file", LUASCRIPT_DIR);
	if (luaL_loadfile(luaVM, buf))
	{
		send_to_imps( "can't find the file in the function\n");
		return(0);	
	}
	
	if (lua_pcall(luaVM, 0, 0, 0))
	{
		send_to_imps( "error in pcall\n");
		return(0);
	}
	
	/* push functions and arguments */
	lua_getglobal(luaVM, "frog");  /* function to be called */
	lua_pushnumber(luaVM, x);   /* push 1st argument */
	lua_pushnumber(luaVM, y);   /* push 2nd argument */
    
	/* do the call (2 arguments, 1 result) */
	if (lua_pcall(luaVM, 2, 1, 0) != 0)
	{
        send_to_imps("error in pcall\n");
		return 0;
	}
    
	/* retrieve result */
	if (!lua_isnumber(luaVM, -1))
	{
        send_to_imps("error in function\n");
		return 0;
	}
	z = lua_tonumber(luaVM, -1);
	lua_pop(luaVM, 1);  /* pop returned value */
	lua_close(luaVM);
	
	return z;
}


int lua_test(CHAR_DATA* ch)
{
	char buf[MAX_STRING_LENGTH];
	char* output;
	double result;
	lua_State *luaVM = lua_open();   /* opens Lua */
	luaL_openlibs(luaVM);
    FILE* fp;
	int width;
	int height;
	
	send_to_imps("inside lua_test\n");
	sprintf(buf, "%s/lua_test_file", LUASCRIPT_DIR);
	if (luaL_loadfile(luaVM, buf))
		{
			send_to_imps( "can't find the file\n");
			return(0);	
		}
		
	
	if (lua_pcall(luaVM, 0, 0, 0))
		{
			send_to_imps( "error in pcall\n");
			return(0);
		}
    	
	lua_getglobal(luaVM, "width");
	lua_getglobal(luaVM, "height");
	if (!lua_isnumber(luaVM, -2))
	{
        send_to_imps( "`width should be a number\n");
		return(0);	
	}
	if (!lua_isnumber(luaVM, -1))
	{
        send_to_imps("`height' should be a number\n");
		return(0);
	}
	
	width = (int)lua_tonumber(luaVM, -2);
	height = (int)lua_tonumber(luaVM, -1);
	
	sprintf(buf, "width is %d, and height is %d for an area of %d\n", width, height, width*height);
	send_to_imps(buf);
	
		//function call
	result = f(width, height);
	sprintf(buf, "result is: %f\n", result);
	
	send_to_imps(buf);
	
		//TRIGGER_DATA* trigger;
	
		
	larg_setup_room_triggers(ch->room);
	
		//output = duplicateString(buf);
		//trigger = larg_trigger_room_create(3, output, 2002, TS_ROOM);
	
	lua_close(luaVM);
	return 0;
	
}


void larg_setup_room_triggers(ROOM_DATA* room)
{
	/***
	mysqlpp::Query query = dbo.query();
	mysqlpp::Row row;
	TRIGGER_DATA *trig = NULL;
	int i;
	
	query << "SELECT * FROM room_trigger";
	
	mysqlpp::StoreQueryResult res = query.store();
	if ( res.num_rows() > 0 )
	{
		for (i = 0; i < res.num_rows(); i++)
		{
			row = res[i];
			
			trig = new TRIGGER_DATA;
			trig->trig_type = atoi(row["type"]);
			trig->script = strdup(row["script"]);
			trig->func = strdup(row["func"]);
			trig->base = atoi(row["base"]);
			trig->source = atoi(row["source"]);
			trig->trig_id = atoi(row["trig_id"]);
			
			room->triggers.push_front(trig);	
		}
	}
	 
	 ***/
	return;
}

/** Cyle through all rooms looking for scripts **/
/*********
void larg_trigger_time_select(void)
{
	
	CHAR_DATA 	*tch = NULL;
	OBJ_DATA  	*tobj = NULL;
	ROOM_DATA	*troom = NULL;
	std::map<int, ROOM_DATA*>::iterator room_iterator;
	
	char 		buf[80] = {'\0'};
	int 		rnum = 0;
	
		// go through the rooms 
	for (room_iterator = room_map.begin(); room_iterator != room_map.end(); room_iterator++)
	{
		troom = room_iterator->second;
		
		if (troom->people)
		{
			if (!troom->triggers.empty())
			{
				larg_room_trigger_time(troom, troom->people);
			}
			
			for ( tobj = troom->contents; tobj; tobj = tobj->next_content )
			{
				if (!tobj->triggers.empty())
				{
					larg_obj_trigger_time(tobj, troom->people);
				}
			}
			
			for ( tch = troom->people; tch; tch = tch->next_in_room )
			{
				if (!tch->triggers.empty())
				{
					larg_mob_trigger_time(tch, troom->people);
				}
			}
			
		}
		
	}
	
	return;
}
 
 *******/
/**********************************************************************/
/*********************** TIMMER TRIGGERS ******************************/
/**********************************************************************/
	/**
void larg_room_trigger_time(ROOM_DATA *targ, CHAR_DATA *ch_targ)
{
	std::map<int, TRIGGER_DATA *>::iterator trig_it;
	TRIGGER_DATA *tmp_trig;
	int result = 0;
	
	if (targ->triggers.empty())
		return;
	
	for (trig_it = targ->triggers.begin(); trig_it != targ->triggers.end(); trig_it++)
	{
		tmp_trig = trig_it->second;
		
		if (tmp_trig->type == TT_TIME)
		{
			result = larg_execute_script(tmp_trig->script,
										 tmp_trig->func,
										 TS_ROOM,
										 tmp_trig->me,
										 targ->nVirtual,
										 ch_targ,
										 NULL,
										 NULL,
										 NULL);
		}
	}
	
	return;
}
	 
	 
void larg_obj_trigger_time(OBJ_DATA *targ, CHAR_DATA *tch)
{
	OBJ_DATA		*tobj = NULL;
	std::map<int, TRIGGER_DATA *>::iterator trig_it;
	TRIGGER_DATA *tmp_trig;
	int result = 0;
	
	if (targ->triggers.empty())
		return;
	
	for (trig_it = targ->triggers.begin(); trig_it != targ->triggers.end(); trig_it++)
	{
		tmp_trig = trig_it->second;
		
		
		if (tmp_trig->type == TT_TIME)
		{
		
			tobj = get_obj_in_list_vis (tch, targ->name, tch->room->contents);
			if (tobj)
			{
				result = larg_execute_script(tmp_trig->script,
										tmp_trig->func,
										TS_ROOM,
										tmp_trig->me,
										tobj->in_room,  //room
										tch,
										NULL,
										NULL,
										NULL);
			}
		}
		
	}
	
	return;
}
 
	 
void larg_mob_trigger_time(CHAR_DATA *targ, CHAR_DATA *tch)
{
	CHAR_DATA		*tmob = NULL;
	std::map<int, TRIGGER_DATA *>::iterator trig_it;
	TRIGGER_DATA *tmp_trig;
	int result = 0;
	
	if (targ->triggers.empty())
		return;
	
	for (trig_it = targ->triggers.begin(); trig_it != targ->triggers.end(); trig_it++)
	{
		tmp_trig = trig_it->second;
		
		if (tmp_trig->type == TT_TIME)
		{
			result = larg_execute_script(tmp_trig->script,
										tmp_trig->func,
										TS_ROOM,
										tmp_trig->me,
										targ->in_room,  //room
										tch,
										NULL,
										NULL,
										NULL);
		}
		
	}
	
	return;
}
 *******/


/**********************************************************************/
/*************************** CREATE TRIGGERS **************************/
/**********************************************************************/
/******/
/****************
 
TRIGGER_DATA *larg_trigger_obj_create(int type, char *script, int ovnum, int source)
{

	TRIGGER_DATA 	*t;
	char 			*ptr;

		//CREATE(t, TRIGGER_DATA, 1);
	t->type   = type;
	t->me     = ovnum;
	t->source = source;

	if ((ptr = strchr(script, ':')) != NULL)
	{
		*ptr = 0;
		t->script = duplicateString(script);
		t->func   = duplicateString(ptr + 1);
	}
	else
	{
		t->script = duplicateString(script);
	}

	return t;
}

TRIGGER_DATA *larg_trigger_mob_create(int type, char *script, int mvnum, 																		int source)
{

	TRIGGER_DATA 	*t;
	char 			*ptr;

		//CREATE(t, TRIGGER_DATA, 1);
	t->type   = type;
	t->me     = mvnum;
	t->source = source;

	if ((ptr = strchr(script, ':')) != NULL)
	{
		*ptr = 0;
		t->script = duplicateString(script);
		t->func   = duplicateString(ptr + 1);
	}
	else
	{
		t->script = duplicateString(script);
	}

	return t;
}
**********/

/**********************************************************************/
/*
 * FUNCTION:  execute_script
 * ARGUMENTS:  char *script, char *func, int source, int me, int room,  OBJ_DATA *              obj,  CHAR_DATA *ch,  char *txt
  DESCRIPTION: Load a Lua script, and eventually call a function in it.
 * RETURNS:     -1 if something went wrong
 *               0 if script was executed
 */
/**********
int larg_execute_script(char *script, char *func, int source, int me, int room, CHAR_DATA *targ_ch,  OBJ_DATA *obj,  CHAR_DATA *ch,  char *txt)
{
	
	char buf[MAX_STRING_LENGTH] = {'\0'};
	int err = 0;
	char *mess = NULL;
	char *command = NULL;
	int mess_room_num = 0;
	int room_sect = 0;
	
	snprintf(buf, MAX_STRING_LENGTH, "%s/%s", SCRIPT_DIR, script);
	
	err = luaL_dofile(luaVM, buf);

	if (err == 1){
		send_to_gods("Error in execute_script");
		return (-1);
	}
		
	
	lua_getglobal(luaVM, func);
	
	lua_newtable(luaVM);        /* We will pass a table */

/* To put values into the table, we first push the index, then the
 * value, and then call lua_rawset() with the index of the table in the
 * stack. Let's see why it's -3: In Lua, the value -1 always refers to
 * the top of the stack. When you create the table with lua_newtable(),
 * the table gets pushed into the top of the stack. When you push the
 * index and then the cell value, the stack looks like:
 *
 * <- [stack bottom] -- table, index, value [top]
 *
 * So the -1 will refer to the cell value, thus -3 is used to refer to
 * the table itself. Note that lua_rawset() pops the two last elements
 * of the stack, so that after it has been called, the table is at the
 * top of the stack.
*/
/*******
lua_pushnumber(luaVM, 1);   // Push the table index 
lua_pushnumber(luaVM, time_info.hour); // Push the cell value 
lua_rawset(luaVM, -3);      // Stores the pair in the table

lua_pushnumber(luaVM, 2);   //* Push the table index 
lua_pushnumber(luaVM, time_info.day); //* Push the cell value 
lua_rawset(luaVM, -3);      //* Stores the pair in the table
	
	room_sect = vtor(room)->terrain_type;
	
lua_pushnumber(luaVM, 3);   //* Push the table index 
lua_pushnumber(luaVM, room_sect); //* Push the cell value 
lua_rawset(luaVM, -3);      //* Stores the pair in the table
*********/
/*
 * A table must be terminated by a cell which is indexed by the literal
 * "n" and contains the total number of elements in the table.
 */
/*****
lua_pushliteral(luaVM, "n");  //* Pushes the literal 
lua_pushnumber(luaVM, 4);     //* Pushes the total number of cells 
lua_rawset(luaVM, -3);        //* Stores the pair in the table 


	//* Send data to be used by Lua scripts 
	
	lua_pushnumber (luaVM, source);
	lua_pushnumber (luaVM, me);
	lua_pushnumber (luaVM, room);
	********/

/***************************************************************
 call script and get results back 
 sent table, source, me value and room number
 returns the command string to be used by 
 command_interpreter (CHAR_DATA *ch, char *argument)
 Could use a case statment here or call selection function
****************************************************************/

/*******
	lua_call(luaVM, 4, 2);
	
	command = (char *)lua_tostring(luaVM, -2);
	mess = (char *)lua_tostring(luaVM, -1);
	
	if (!strcmp(command, "echo"))
	{
		send_to_room(mess, room);
	}
	
	else if (!strcmp(command, "loadmob"))
	{
		load_mobile(atoi(mess));
	}

	return (0);

	
}
********/

/**********************************************************************/
/*************************** SETUP TRIGGERS **************************/
/**********************************************************************/

/******
 void larg_setup_obj_triggers(FILE *fl, OBJ_DATA *obj)
{
	int 			trig_type;
	char			*trig_script = NULL;
	char			*temp_arg = NULL;
	TRIGGER_DATA	*trig = NULL;
	int size_val = 0;

	temp_arg = fread_word(fl);  //gets rid of the R
	trig_type = fread_number(fl);
	trig_script = fread_string(fl);
	
	
	trig = larg_trigger_obj_create(trig_type, trig_script, obj->nVirtual, TS_OBJECT);

	size_val = obj->triggers.size();
	trig->trig_id = size_val+1;
	obj->triggers[trig->trig_id] = trig;
	
	return;
}

 void larg_setup_mob_triggers(FILE *fl, CHAR_DATA *tch)
{
	int 			trig_type;
	char			*trig_script = NULL;
	char			*temp_arg = NULL;
	TRIGGER_DATA	*trig = NULL;
	int size_val = 0;
	
	temp_arg = fread_word(fl);  //gets rid of the R
	trig_type = fread_number(fl);
	trig_script = fread_string(fl);
	
	trig = larg_trigger_mob_create(trig_type, trig_script, tch->mob->nVirtual, TS_MOBILE);
	
	size_val = tch->triggers.size();
	trig->trig_id = size_val+1;
	tch->triggers[trig->trig_id] = trig;
	
	return;
}
**********/
/**********************************************************************/
/*************************** ROOM UTILITIES **************************/
/**********************************************************************/
/*****
void rscript_add (CHAR_DATA *ch, char *argument)
{
	ROOM_DATA 	*temp_room = NULL;
	
	temp_room = vtor (ch->in_room);
	larg_room_script_add (ch, argument, temp_room);
	return;
}

void rscript_del (CHAR_DATA *ch, char *argument)
{
	ROOM_DATA 	*temp_room = NULL;
	
	temp_room = vtor (ch->in_room);
	larg_room_script_delete (ch, argument, temp_room);
	return;
}

 void rscript_list (CHAR_DATA *ch, char *argument)
{
	ROOM_DATA 	*temp_room = NULL;
		
	temp_room = vtor (ch->in_room);
	larg_room_script_list (ch, argument, temp_room);
}


 void larg_room_script_list(CHAR_DATA *ch, char *argument, ROOM_DATA *room)
{
	char		buf	[MAX_STRING_LENGTH] = { '\0' };
	std::map<int, TRIGGER_DATA *>::iterator trig_it;
	TRIGGER_DATA *tmp_trig;
	
	sprintf (buf, "This room has the following scripts:\n\n Index  Type     Script     Function\n");
	ch->send_to_char (buf);
	
	for (trig_it = room->triggers.begin(); trig_it != room->triggers.end(); trig_it++)
	{
		tmp_trig = trig_it->second;
		
		snprintf (buf, MAX_STRING_LENGTH,  "%d  %d     %s     %s\n", tmp_trig->trig_id, tmp_trig->type, tmp_trig->script, tmp_trig->func);
		ch->send_to_char (buf);
	}
	return;	
}

 void larg_room_script_add (CHAR_DATA *ch, char *argument, ROOM_DATA *room)
{
	char		buf	[MAX_STRING_LENGTH] = { '\0' };
	TRIGGER_DATA		*temp_trig = NULL;
	int			sct_type = 0;
	char		*sct_script = NULL;
	char		*sct_func = NULL;
	int size_val = 0;

		if ( !*argument )
		{
			ch->send_to_char("Correct format is - script-add <Type> <Script_name>  <Function_name>.\nTypes are 1-Timed, 2-Enter, 3-Leave, 4-Tell, 5-Ask, 6-Say, 7-Give\nType 1 is the only one implemented at this time\n");
			return;
		}
		else
		{	
			temp_trig = new TRIGGER_DATA;
			argument = one_argument (argument, buf);
			if (!isdigit (*buf))
			{
				ch->send_to_char ("Expected type 1..7\n");
				return;
			}
			temp_trig->type = atoi(buf);
			
			argument = one_argument (argument, buf);
			if ( !(*buf)) 
			{
				ch->send_to_char ("Expected Name of script\n");
				return;
			}
			temp_trig->script = duplicateString(buf);
			
			argument = one_argument (argument, buf);
			if ( !(*buf))
			{
				ch->send_to_char ("Expected Name of function\n");
				return;
			}
			temp_trig->func = duplicateString(buf);
			

			temp_trig->source = 0; //0-room, 1-obj, 2-mobile
			temp_trig->me = ch->in_room;
			
			size_val = room->triggers.size();
			temp_trig->trig_id = size_val+1;
			room->triggers[temp_trig->trig_id] = temp_trig;
		}
		

	return;
}

 void larg_room_script_delete (CHAR_DATA *ch, char *argument, ROOM_DATA *room)
{
	char		buf	[MAX_STRING_LENGTH] = { '\0' };
	int			trig_type = 0;
	int			trig_source = 0;
	int			trig_me = 0;
	char		*trig_script = NULL;
	char		*trig_func = NULL;
	TRIGGER_DATA *ttrig = NULL;
	int in_val;
	
		if ( !*argument )
		{
			ch->send_to_char("Correct format is - script-del <index_number>.\n");
			return;
		}
		else
		{	
			argument = one_argument (argument, buf);
			if (*buf && !isdigit (*buf))
			{
				ch->send_to_char ("Expected a number\n");
				return;
			}
			
			in_val = atoi(buf);
		}

	room->triggers.erase(in_val);


	return;
}

/**********************************************************************/
/*************************** MOB UTILITIES ***************************/
/** See mset for add, del, and list utilities                      ***/
/*********************************************************************/

/**********************************************************************/
/*************************** OBJ UTILITIES ***************************/
/** See oset for add, del, and list utilities                      ***/
/*********************************************************************/

