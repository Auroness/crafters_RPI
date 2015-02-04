//////////////////////////////////////////////////////////////////////////////
//
/// utility.c : Utility Module
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
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <functional>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>

#include "trigram.h"
#include "structs.h"
#include "net_link.h"
#include "account.h"
#include "utils.h"
#include "protos.h"
#include "decl.h"
#include "group.h"
#include "utility.h"

extern std::map<std::string, SKILL_DATA*> skill_data_map;


int all_are_set(int holder, int flags)
{
	if((holder & flags) == flags)
	{
		return flags;
	}
	else
	{
		return 0;
	}
}

int nil_are_set(int holder, int flags)
{
	if((holder & flags) == 0)
	{
		return flags;
	}
	else
	{
		return 0;
	}
}

int any_are_set(int holder, int flags)
{
	if((holder & flags))
	{
		return flags;
	}
	else
	{
		return 0;
	}
}

int check_flags(int holder,int true_flags, int false_flags)
{
	if(all_are_set(holder, true_flags) && nil_are_set(holder, false_flags))
	{
		return true_flags | false_flags;
	}
	else
	{
		return 0;
	}
}

bool IS_NPC (const CHAR_DATA *ch)
{
	bool is = false;
	
	if (!ch)
	{
		system_log ("IS_NPC error. Would have crashed. Hell yeah. Returning true.", true);
		send_to_room ("IS_NPC went Whoopsie!", 686);
		return true;
	}
	if (ch->mob && ch->mob->action)
		is = (IS_SET(ch->mob->action, ACT_ISNPC)); // For debugging purposes with GDB. It doesn't like macros.
	return is;
}

bool IS_NPC (CHAR_DATA *ch)
{
	bool is = false;
	
	if (!ch)
	{
		system_log ("IS_NPC error. Would have crashed. Hell yeah. Returning true.", true);
		return true;
	}
	if (ch->mob && ch->mob->action)
		is = IS_SET(ch->mob->action, ACT_ISNPC); // For debugging purposes with GDB. It doesn't like macros.

	return is;
}
int GCD(int a, int b)
{
	while( 1 )
	{
		a = a % b;
		if( a == 0 )
			return b;
		b = b % a;
		if( b == 0 )
			return a;
	}
}

double RoundDouble(double doValue, int nPrecision)
{
	static const double doBase = 10.0;
	double doComplete5, doComplete5i;

	doComplete5 = doValue * pow(doBase, (double) (nPrecision + 1));

	if(doValue < 0.0)
		doComplete5 -= 5.0;
	else
		doComplete5 += 5.0;

	doComplete5 /= doBase;
	modf(doComplete5, &doComplete5i);

	return doComplete5i / pow(doBase, (double) nPrecision);
}

/*
* Pick off one argument from a string and return the rest.
* Understands quotes.
*/
char *
one_argument (char *argument, char *arg_first)
{
	char cEnd;

	if (argument == NULL)
		return "";

	while (isspace (*argument))
		argument++;

	cEnd = ' ';

	if (*argument == '\'' || *argument == '"' || *argument == '`')
		cEnd = *argument++;

	while (*argument != '\0')
	{

		if (*argument == cEnd)
		{
			argument++;
			break;
		}

		if (cEnd == ' ')
			*arg_first = tolower (*argument);
		else
			*arg_first = *argument;

		arg_first++;
		argument++;
	}

	*arg_first = '\0';

	while (isspace (*argument))
		argument++;

	return argument;
}

std::string
one_argument (std::string& argument, std::string& arg_first)
{
	arg_first.erase ();
	if (argument.length () == 0)
	{
		return argument;
	}

	std::string::size_type x = 0;
	std::string::iterator i = argument.begin ();
	for (; i != argument.end (); ++i,++x)
	{
		if (!isspace (*i))
		{
			break;
		}
	}

	if (*i == '\'' || *i == '"' || *i == '`')
	{
		char token_separator = *i;
		++i;
		++x;

		for (; i != argument.end (); ++i,++x)
		{
			if (*i == token_separator)
			{
				++i;
				++x;
				break;
			}
			arg_first += *i;
		}
	}
	else
	{
		for (; i != argument.end (); ++i,++x)
		{
			if (isspace(*i))
			{
				++i;
				++x;
				break;
			}
			arg_first += tolower (*i);
		}
	}

	for (; i != argument.end (); ++i,++x)
	{
		if (!isspace (*i))
		{
			break;
		}
	}

	return argument.substr(x);
}

#ifdef NOVELL
int
strcasecmp (char *s1, char *s2)
{
	char *p, *q;

	p = s1;
	q = s2;

	if (strlen (p) != strlen (q))
		return (strlen (p) - strlen (q));

	for (; *p != '\0' && *q != '\0'; p++, q++)
	{
		if (tolower (*p) < tolower (*q))
			return (-1);
		if (tolower (*p) > tolower (*q))
			return (1);
	}

	return (0);
}
#endif

// is_overcast
// returns true if the room is blocked from sunlight (petrification, etc)
bool is_overcast (ROOM_DATA * room)
{
	bool result = false;
	int flags = room->room_flags;
	int clouds = weather_info[room->wzone].clouds;

	if ((room->terrain_type == SECT_FOREST)
		|| (room->terrain_type == SECT_CAVE)
		|| (room->terrain_type == SECT_PIT)
		|| (room->terrain_type == SECT_UNDERWATER)
		|| (flags & INDOORS)
		|| (flags & STIFLING_FOG)
		|| (flags & DARK) 
		|| (flags & TUNNEL)
		|| (clouds == OVERCAST)
		|| (clouds == HEAVY_CLOUDS))
	{
		result = true;
	}

	return result;
}
// is_sunlight_restricted
// returns true if the character is currently suffering due to the sun.
// TODO: move to char.h
bool
is_sunlight_restricted (CHAR_DATA * ch, ROOM_DATA * room)
{
	bool result = false;
	
	if (!room)
	{
		room = vtor(ch->in_room);
	}

	
	if ((global_sun_light > (SUN_TWILIGHT)) 
		&& get_affect(ch, AFF_SUNLIGHT_PEN))
	{
		if (!is_overcast (ch->room)
			|| (room && !is_overcast (room)))
		{
			ch->send_to_char ("The brilliant flame of Anor interferes "
				"with your attempt.\n");
			result = true;
		}
	}

	return result;
}


char *
generate_password (int argc, char **argv)
{
	int password_length;		/* how long should each password be */
	int n_passwords;		/* number of passwords to generate */
	int pwnum;			/* number generated so far */
	int c1, c2, c3;		/* array indices */
	long sumfreq;			/* total frequencies[c1][c2][*] */
	double pik;			/* raw random number in [0.1] from drand48() */
	long ranno;			/* random number in [0,sumfreq] */
	long sum;			/* running total of frequencies */
	char password[100];		/* buffer to develop a password */
	static char result[100];
	int nchar;			/* number of chars in password so far */
	struct timeval systime;	/* time reading for random seed */
	struct timezone tz;		/* unused arg to gettimeofday */

	password_length = 8;		/* Default value for password length */
	n_passwords = 10;		/* Default value for number of pws to generate */

	gettimeofday (&systime, &tz);	/* Read clock. */
	srand48 (systime.tv_usec);	/* Set random seed. */

	if (argc > 1)
	{				/* If args are given, convert to numbers. */
		n_passwords = atoi (&argv[1][0]);
		if (argc > 2)
		{
			password_length = atoi (&argv[2][0]);
		}
	}
	if (argc > 3 || password_length > 99 ||
		password_length < 0 || n_passwords < 0)
	{
		printf (" USAGE: gpw [npasswds] [pwlength]\n");
		abort ();
	}

	/* Pick a random starting point. */
	/* (This cheats a little; the statistics for three-letter
	combinations beginning a word are different from the stats
	for the general population.  For example, this code happily
	generates "mmitify" even though no word in my dictionary
	begins with mmi. So what.) */
	for (pwnum = 0; pwnum < n_passwords; pwnum++)
	{
		pik = drand48 ();		/* random number [0,1] */
		sumfreq = sigma;		/* sigma calculated by loadtris */
		ranno = (long)(pik * (double)sumfreq);	/* Weight by sum of frequencies. */
		sum = 0;
		for (c1 = 0; c1 < 26; c1++)
		{
			for (c2 = 0; c2 < 26; c2++)
			{
				for (c3 = 0; c3 < 26; c3++)
				{
					sum += tris[c1][c2][c3];
					if (sum > ranno)
					{		/* Pick first value */
						password[0] = 'a' + c1;
						password[1] = 'a' + c2;
						password[2] = 'a' + c3;
						c1 = c2 = c3 = 26;	/* Break all loops. */
					}		/* if sum */
				}		/* for c3 */
			}			/* for c2 */
		}			/* for c1 */

		/* Do a random walk. */
		nchar = 3;		/* We have three chars so far. */
		while (nchar < password_length)
		{
			password[nchar] = '\0';
			password[nchar + 1] = '\0';
			c1 = password[nchar - 2] - 'a';	/* Take the last 2 chars */
			c2 = password[nchar - 1] - 'a';	/* .. and find the next one. */
			sumfreq = 0;
			for (c3 = 0; c3 < 26; c3++)
				sumfreq += tris[c1][c2][c3];
			/* Note that sum < duos[c1][c2] because
			duos counts all digraphs, not just those
			in a trigraph. We want sum. */
			if (sumfreq == 0)
			{			/* If there is no possible extension.. */
				break;		/* Break while nchar loop & print what we have. */
			}
			/* Choose a continuation. */
			pik = drand48 ();
			ranno = (long)(pik * (double)sumfreq);	/* Weight by sum of frequencies for row. */
			sum = 0;
			for (c3 = 0; c3 < 26; c3++)
			{
				sum += tris[c1][c2][c3];
				if (sum > ranno)
				{
					password[nchar++] = 'a' + c3;
					c3 = 26;	/* Break the for c3 loop. */
				}
			}			/* for c3 */
		}			/* while nchar */
		printf ("%s\n", password);
	}
	sprintf (result, "%s", password);	/* for pwnum */
	return result;
}


/* creates a random number in interval [from;to] */
int
number (int from, int to)
{
	if (to == from)
		return from;

	if (to > from)
		return ((rand () % (to - from + 1)) + from);
	else
		return ((rand () % (from - to + 1)) + to);
}

/* simulates dice roll */
unsigned int
dice (unsigned int number, unsigned int size)
{
	unsigned int r;
	unsigned int sum = 0;

	if (size <= 0)
		size = 1;

	for (r = 1; r <= number; r++)
		sum += (rand () % size) + 1;

	return sum;
}



/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different, end of both, or n reached     */
int
strn_cmp (const char *arg1, const char *arg2, int n)
{
	int chk, i;

	for (i = 0; (*(arg1 + i) || *(arg2 + i)) && (n > 0); i++, n--)
		if ((chk = (tolower (*(arg1 + i)) - tolower (*(arg2 + i)))))
		{
			if (chk < 0)
				return (-1);
			else
				return (1);
		}

		return (0);
}

	//examines the bits and 
	//returns a the matchng values from a const char array
char*
sprintbit (long vektor, const char *names[])
{
	std::string out_buf;
	int i;

	out_buf.clear();
	
	for (i = 0; i <= 31; i++)
	{
		if (IS_SET (vektor, 1 << i))
		{
			out_buf.append(" ");
			out_buf.append(names[i]);
		}
		

	}
	
	return ((char*)out_buf.c_str());
	
}


	//returns the indexed value from a const char array
	//side effect/not a proper return
void
sprinttype (int type, const char *names[], char *result)
{
	int nr;

	for (nr = 0; (*names[nr] != '\n'); nr++);
	if (type < nr && names[type])
		strcpy (result, names[type]);
	else
		strcpy (result, "UNDEFINED");
}


/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data
	real_time_passed (time_t t2, time_t t1)
{
	long secs;
	struct time_info_data now;

	secs = (long) (t2 - t1);

	now.minute = (secs / 60) % 60;
	secs -= 60 * now.minute;

	now.hour = (secs / SECS_PER_REAL_HOUR) % 24;	/* 0..23 hours */
	secs -= SECS_PER_REAL_HOUR * now.hour;

	now.day = (secs / SECS_PER_REAL_DAY);	/* 0..34 days  */
	secs -= SECS_PER_REAL_DAY * now.day;

	now.month = -1;
	now.year = -1;

	return now;
}

/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data
	mud_time_passed (time_t t2, time_t t1)
{
	time_t secs;
	struct time_info_data now = {0, 0, 0, GAME_BASE_YEAR, 0, 0, 0, 0};

	secs = (t2 - t1) * IG_HOUR_PER_RL_HOUR;

	now.year += secs / GAME_SECONDS_PER_YEAR;
	secs = secs % GAME_SECONDS_PER_YEAR;

	now.month += secs / GAME_SECONDS_PER_MONTH;
	secs = secs % GAME_SECONDS_PER_MONTH;

	now.day += secs / GAME_SECONDS_PER_DAY;
	secs = secs % GAME_SECONDS_PER_DAY;

	now.hour += secs / GAME_SECONDS_PER_HOUR;
	
	now.minute = secs % 60;

	now.accum_days = (now.month * 30) + now.day;
	
	return now;
}




struct time_info_data
age (CHAR_DATA * ch)
{
	struct time_info_data player_age;

	if (ch->time_str.birth > 0)
	{
	player_age = mud_time_passed (time (0), ch->time_str.birth);

	player_age.year += ch->age - GAME_BASE_YEAR;	
	return player_age;
	}
	else 
	{
		player_age.month = 0;
		player_age.day = 0;
		player_age.hour = 0;
		player_age.year = ch->age;
		
		return player_age;
	
	}
}

int
parse_argument (const char *commands_list[], char *string)
{
	int len;
	int i;

	if (!*string)
		return -1;

	len = strlen (string);

	for (i = 0; *commands_list[i] != '\n'; i++)
	{
		if (strn_cmp (commands_list[i], string, len) == 0)
			return i;
	}

	return -1;
}

void
arg_splitter (int argc, char *fmt, ...)
{
	va_list arg_ptr;
	char *p;
	char *sval;

	va_start (arg_ptr, fmt);	/* arg_ptr points to 1st unnamed arg */
	for (p = fmt; *p;)
	{
		for (; isspace (*p); p++);

		if (*p && argc)
		{
			sval = va_arg (arg_ptr, char *);
			if (*p == '\'' || *p == '"') {
				int term = (int)*p;
				++p; // skip first quote
				for (; (*p) && ((int)(*p) != term) && (*sval = *p); p++, sval++);
				if  (*p) {
					++p; // skip last quote
				}
			}
			else {
				for (; !isspace (*p) && (*sval = *p); p++, sval++);
			}
			if (argc == 1 || !*p)
				for (; (*sval = *p); p++, sval++);
			else
				*sval = '\0';
			argc--;
		}
	}

	while (argc)
	{
		sval = va_arg (arg_ptr, char *);
		*sval = '\0';
		argc--;
	}

	va_end (arg_ptr);		/* clean up when done */
}

#define TOKEN_NUMBER	1
#define TOKEN_INT		3
#define TOKEN_DEX		4
#define TOKEN_CON		5
#define TOKEN_WIL		6
#define TOKEN_STR		7
#define TOKEN_AUR		8
#define TOKEN_DIV		10
#define TOKEN_SUB		11
#define TOKEN_PLUS		12
#define TOKEN_MULT		13
#define TOKEN_OPEN_PAR	14
#define TOKEN_CLOSE_PAR	15
#define TOKEN_AGI		16
#define TOKEN_LUK		17

int
get_eval_token (char **p, int *token_type, int *value)
{
	char tmp[MAX_INPUT_LENGTH];
	int index = 0;

	*value = 0;
	*token_type = 0;

	while (isspace (**p))
		(*p)++;

	/* Number */

	if (isdigit (**p))
	{
		while (isdigit (**p))
		{
			*value = *value * 10 + (**p - '0');
			tmp[index++] = **p;
			(*p)++;
		}
		tmp[index] = 0;
		*token_type = TOKEN_NUMBER;
		return 1;
	}

	/* Special: ()/+-* */

	if (**p == '+')
		*token_type = TOKEN_PLUS;
	else if (**p == '/')
		*token_type = TOKEN_DIV;
	else if (**p == '*')
		*token_type = TOKEN_MULT;
	else if (**p == '-')
		*token_type = TOKEN_SUB;
	else if (**p == '(')
		*token_type = TOKEN_OPEN_PAR;
	else if (**p == ')')
		*token_type = TOKEN_CLOSE_PAR;

	if (*token_type)
	{
		(*p)++;
		return 1;
	}

	/* attribute */

	if (!strncmp (*p, "int", 3))
		*token_type = TOKEN_INT;
	else if (!strncmp (*p, "dex", 3))
		*token_type = TOKEN_DEX;
	else if (!strncmp (*p, "str", 3))
		*token_type = TOKEN_STR;
	else if (!strncmp (*p, "aur", 3))
		*token_type = TOKEN_AUR;
	else if (!strncmp (*p, "con", 3))
		*token_type = TOKEN_CON;
	else if (!strncmp (*p, "wil", 3))
		*token_type = TOKEN_WIL;
	else if (!strncmp (*p, "agi", 3))
		*token_type = TOKEN_AGI;
	else if (!strncmp (*p, "luk", 3))
		*token_type = TOKEN_LUK;

	if (*token_type)
	{
		*p += 3;
		return 1;
	}

	return 0;
}

int
eval_att_eq (CHAR_DATA * ch, char **equation)
{
	int token_type;
	int token_value;
	int left_side;
	int right_side;
	int sign_flag;
	int operation;

	left_side = 0;
	operation = TOKEN_PLUS;

	while (**equation)
	{

		sign_flag = -1;

		do
		{
			sign_flag = -sign_flag;

			if (!get_eval_token (equation, &token_type, &token_value))
				return -12370;

		}
		while (token_type == TOKEN_SUB);

		if (token_type == TOKEN_OPEN_PAR)
			right_side = eval_att_eq (ch, equation);
		else if (token_type == TOKEN_NUMBER)
			right_side = token_value;

		else if (token_type == TOKEN_INT)
			right_side = ch->tmp_intel;
		else if (token_type == TOKEN_DEX)
			right_side = ch->tmp_dex;
		else if (token_type == TOKEN_STR)
			right_side = ch->tmp_str;
		else if (token_type == TOKEN_AUR)
			right_side = ch->tmp_aur;
		else if (token_type == TOKEN_CON)
			right_side = ch->tmp_con;
		else if (token_type == TOKEN_WIL)
			right_side = ch->tmp_wil;
		else if (token_type == TOKEN_AGI)
			right_side = ch->tmp_agi;
		else if (token_type == TOKEN_LUK)
			right_side = ch->tmp_luk;

		else
			return -12350;

		right_side = right_side * sign_flag;

		switch (operation)
		{
		case TOKEN_PLUS:
			left_side = left_side + right_side;
			break;

		case TOKEN_SUB:
			left_side = left_side - right_side;
			break;

		case TOKEN_DIV:
			if (!right_side)
				return -12360;
			left_side = left_side / right_side;
			break;

		case TOKEN_MULT:
			left_side = left_side * right_side;
			break;
		}

		if (!get_eval_token (equation, &operation, &token_value) ||
			operation == TOKEN_CLOSE_PAR)
			return left_side;

		if (operation != TOKEN_PLUS && operation != TOKEN_SUB &&
			operation != TOKEN_MULT && operation != TOKEN_DIV)
		{
			return -12340;
		}
	}

	return left_side;
}

int
calc_lookup (CHAR_DATA * ch, int reg_index, int reg_entry)
{
	char *p;
	int calced_value;

	if (!(p = (char*)lookup_string (reg_entry, reg_index).c_str()))
    {
		return 100;
    }
	
	if (reg_index == REG_CAP)
	{
		calced_value = eval_att_eq (ch, &p);
		calced_value = MIN (calced_value, 100);
	}
	else
		calced_value = eval_att_eq (ch, &p);

	if (calced_value < -1)
	{
		return 100;
	}

	return calced_value;
}


#ifdef MEMORY_CHECK
#define MAX_ALLOC	70000

MEMORY_T *alloc_ptrs[MAX_ALLOC];
#endif

int bytes_allocated = 0;
int first_free = 0;
int mud_memory = 0;

char* duplicateString(const char *source) {
	if (source == NULL) {
		return NULL;
	}
	else if (strcmp(source, "") == STR_MATCH) {
		static char *emptyString = "";
		return emptyString;
	}
	std::string tempString = source;
	char *output = new char[tempString.length() + 1];
	
	strncpy(output, tempString.c_str(), tempString.length() + 1);
	return output;
}
	
	// Frees the target then duplicates a string into it - Case
void replaceString(char *&destination, const char *source) {
	free_mem(destination);
	destination = duplicateString(source);
}

int free_mem (char *&ptr) {
	if (ptr != NULL && (strcmp(ptr, "") != STR_MATCH)) {
		delete [] ptr;
		ptr = (char *)NULL;
		return 1;
	}
	else {
		return 0;
	}
}

int free_mem (void *ptr) {
	delete ptr;
	ptr = NULL;
	return 1;
}

int free_mem_array (void *ptr) {
	delete [] ptr;
	ptr = NULL;
	return 1;
}

void
add_char (char *buf, char c)
{
	buf[strlen (buf) + 1] = '\0';
	buf[strlen (buf)] = c;
}

int
is_obj_here (CHAR_DATA * ch, OBJ_DATA * obj, int check)
{
	OBJ_DATA *tobj;

	if (ch->deleted || !ch->room)
		return 0;

	for (tobj = ch->room->contents; tobj; tobj = tobj->next_content)
		if (!tobj->deleted && tobj == obj)
			break;

	if (!tobj || (check && !can_see_obj(ch, tobj)))
		return 0;

	return 1;
}

int
is_he_there (CHAR_DATA * ch, ROOM_DATA * room)
{
	CHAR_DATA *tch = NULL;

	/* he could be dead, or have left the game...who knows.   We
	cannot dereference "he" cause we are uncertain of the pointer
	*/

	for (tch = room->people; tch; tch = tch->next_in_room)
		if (tch == ch && !tch->deleted)
			return 1;

	if (!tch)
		return 0;

	return 1;			/* "he" is valid, and in ch's room */
}

int
is_he_here (CHAR_DATA * ch, CHAR_DATA * he, int check)
{
	CHAR_DATA *tch = NULL;

	/* "check" means make sure we can see him.

	He could be dead, or have left the game...who knows.   We
	cannot dereference "he" cause we are uncertain of the pointer
	*/

	if (ch == NULL)
		return 0;

	if (he == NULL)
		return 0;

	if (ch->room)
	{
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
			if (tch == he && !tch->deleted)
				break;
	}

	if (!tch || (check && !can_see_mob(ch, tch)))
		return 0;

	return 1;			/* "he" is valid, and in ch's room */
}

int
is_he_somewhere (CHAR_DATA * he)
{
	CHAR_DATA *tch;
	std::list<char_data*>::iterator tch_iterator;
	if (!he)
		return 0;

	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		tch = *tch_iterator;
		if (!tch->deleted && tch == he)
			return 1;
	}

	return 0;
}

int
is_obj_in_list (OBJ_DATA * obj, OBJ_DATA * list)
{
	OBJ_DATA *tobj;
	int count = 0;

	for (tobj = list; tobj; tobj = tobj->next_content)
	{
		count++;
		if (tobj == obj)
			return count;
	}

	return 0;
}

void
name_to_ident (CHAR_DATA * ch, char *ident)
{
	int i = 1;
	CHAR_DATA *tch;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char *temp_buf;
	
	*buf = '\0';
	*buf2 = '\0';

	if (!ch || !ident)
		return;

	if (!IS_NPC (ch) && !ch->is_hooded())
	{
		sprintf (ident, "%s", ch->name);
		return;
	}

	temp_buf = new char[MAX_STRING_LENGTH];
	sprintf(temp_buf, "%s", ch->char_names());
	one_argument (temp_buf, buf);

	for (tch = ch->room->people; ch != tch; tch = tch->next_in_room)
	{
		sprintf(temp_buf, "%s", tch->char_names());
		one_argument (temp_buf, buf2);
		if (can_see_mob(ch, tch) && !strcmp (buf, buf2))
			i++;
	}
	delete [] temp_buf;

	if (i == 1)
		sprintf (ident, "%s", buf);
	else
		sprintf (ident, "%d.%s", i, buf);
}



CHAR_DATA *
get_pc (char *buf)
{
	CHAR_DATA *ch;
	std::list<char_data*>::iterator tch_iterator;
	if (!buf || !*buf)
		return NULL;

	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		ch = *tch_iterator;

		if (ch->deleted || IS_NPC (ch) || !ch->name)
			continue;

		if (!str_cmp (ch->name, buf))
			return ch;
	}

	return NULL;
}

CHAR_DATA *
get_pc_dead (const char *buf)
{
	CHAR_DATA *ch;
	std::list<char_data*>::iterator tch_iterator;
	if (!*buf)
		return NULL;

	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		ch = *tch_iterator;
		if (ch->deleted)
			continue;
		if (!ch->name)
			continue;
		if (!IS_NPC (ch) && !str_cmp (ch->name, buf))
			return ch;
	}

	return NULL;
}

CHAR_DATA *
load_pc (char *buf)
{
	CHAR_DATA *ch = NULL;
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	std::list<char_data*>::iterator tch_iterator;

	if (!buf || !*buf)
		return NULL;

	if ((ch = get_pc_dead (buf)))
	{
		ch->pc->load_count++;
		sprintf (buf2, "(online) Loading %s, count %d", buf,
			ch->pc->load_count);
		system_log (buf2, false);
		return ch;
	}
	
	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		ch = *tch_iterator;

		if (ch->deleted)
			continue;

		if (!ch->pc)
			continue;

		if (!ch->name)
			continue;

		if (!str_cmp (ch->name, buf))
		{
			ch->pc->load_count++;
			sprintf (buf2, "(loaded) char list %s, count %d",
				buf, ch->pc->load_count);
			system_log (buf2, false);
			return ch;
		}
	}

	if (!(ch = load_char_mysqlpp (buf)))
	{
		return NULL;
	}

	ch->pc->load_count = 1;
	
	sprintf (buf2, "(loading) first %s, count %d", buf, ch->pc->load_count);
	system_log (buf2, false);
	
	return ch;
}



	//returns 1 if it is dark
	//returns 0 it is is lit
int
is_dark (ROOM_DATA * room)
{
	
	if (!room)
		return(1);

	if (is_room_affected (room->affects, MAGIC_ROOM_DARK))
		return(1);

	if (IS_SET (room->room_flags, DARK))
		return(1);

	if (IS_SET (room->room_flags, LIGHT))
		return(0);
	
	if (is_room_affected (room->affects, MAGIC_ROOM_LIGHT))
		return(0);
	
	
	if (room->light < MOON_CRESCENT)
		return (1);
	
	
	return(0);
}





DESCRIPTOR_DATA *
is_pc_attached (char *buf)
{
	DESCRIPTOR_DATA *d;

	for (d = descriptor_list; d; d = d->next)
	{

		if (!d->character)
			continue;

		if (!str_cmp (d->character->name, buf))
			return d;
	}

	return NULL;
}
/********************************************
Used:

61: XXS XXS XXS XXS XXS XXS XXS XXS XXS XXS
68:  XS  XS  XS  XS  XS  XS  XS  XS  XS  XS
75:  XS  XS  XS  XS  XS  XS  XS  XS  XS  XS
82:  XS  XS  XS  XS  XS  XS  XS  XS  XS  XS
89:  XS  XS  XS  XS  XS  XS  XS  XS  XS  XS
96:  XS  XS  XS  XS  XS  XS  XS  XS   S   S
103:  XS  XS  XS  XS  XS  XS   S   S   S   S
110:  XS  XS  XS  XS   S   S   S   S   S   S
117:  XS  XS  XS   S   S   S   S   S   S   S
124:  XS   S   S   S   S   S   S   S   S   S
131:   S   S   S   S   S   S   S   S   M   M
138:   S   S   S   S   S   S   M   M   M   M
145:   S   S   S   S   M   M   M   M   M   M
152:   S   S   S   M   M   M   M   M   M   M
159:   S   M   M   M   M   M   M   M   M   M
166:   M   M   M   M   M   M   M   M   M   L
173:   M   M   M   M   M   M   M   M   L   L
180:   M   M   M   M   M   M   L   L   L   L
187:   M   M   M   M   L   L   L   L   L   L
194:   M   M   L   L   L   L   L   L   L   L
201:   M   L   L   L   L   L   L   L   L   L
208:   L   L   L   L   L   L   L   L   L  XL
215:   L   L   L   L   L   L   L  XL  XL  XL
222:   L   L   L   L   L  XL  XL  XL  XL  XL
229:   L   L   L   L  XL  XL  XL  XL  XL  XL
236: XXL XXL XXL XXL XXL XXL XXL XXL XXL XXL
40  44  48  52  56  60  64  68  72  76

*/







OBJ_DATA *
is_at_table (CHAR_DATA * ch, OBJ_DATA * table)
{
	AFFECTED_TYPE *af;
	OBJ_DATA *obj;

	if (!(af = get_affect (ch, MAGIC_SIT_TABLE)))
		return NULL;

	if (table && table == af->a.table.obj)
		return table;

	if (table)
		return NULL;

	for (obj = ch->room->contents; obj; obj = obj->next_content)
		if (obj == af->a.table.obj)
			return obj;

	return NULL;
}



	//can the character see the target character?
int
could_see (CHAR_DATA * ch, CHAR_DATA * target)
{
	int temp_room;
	int seen;
	int target_room_light;

	/* Determine if ch could see target if ch were in target's room */

	if (ch->room == target->room)
		return can_see_mob(ch, target);

	temp_room = ch->in_room;

	ch->char_from_room();

	target_room_light = target->room->light;

	ch->char_to_room(target->in_room);

	/* Adding the character to the room may also bring his light.
	We don't want that. */

	target->room->light = target_room_light;

	seen = can_see_mob(ch, target);

	ch->char_from_room();
	ch->char_to_room(temp_room);

	return seen;
}

	//can the character see the object that is located in an ajoining room.
int
could_see_obj (CHAR_DATA * ch, OBJ_DATA * obj)
{
	int temp_room;
	int seen;
	int target_room_light;
	ROOM_DATA *troom;

	/* Determine if ch could see target if ch were in target's room */

	if (ch->in_room == obj->in_room)
		return can_see_obj(ch, obj);

	temp_room = ch->in_room;

	ch->char_from_room();

	troom = vtor (obj->in_room);
	target_room_light = troom->light;

	ch->char_to_room(obj->in_room);

	/* Adding the character to the room may also bring his light.
	We don't want that. */

	troom->light = target_room_light;

	seen = can_see_obj(ch, obj);

	ch->char_from_room();
	ch->char_to_room(temp_room);

	return seen;
}

char *
obj_short_desc (OBJ_DATA * obj)
{
	int bite_num;
	int total_bites;
	OBJ_DATA *tobj;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	static char description[MAX_STRING_LENGTH]= { '\0' };
	static char coins[MAX_STRING_LENGTH]= { '\0' };
	static char food[MAX_STRING_LENGTH]= { '\0' };
	static char drinkcon[MAX_STRING_LENGTH]= { '\0' };
	static char drycon[MAX_STRING_LENGTH]= { '\0' };
	char *argument;

	if (!obj)
		return NULL;

	*buf = '\0';

	if (obj->obj_flags.type_flag == ITEM_MONEY)
	{

		if (obj->count == 1)
			sprintf (coins, "one");

		else if (obj->count == 2)
			sprintf (coins, "two");

		else if (obj->count == 3)
			sprintf (coins, "three");

		else if (obj->count == 4)
			sprintf (coins, "four");

		else if (obj->count > 2501)	/* more than 2500 coins */
			sprintf (coins, "an enormous pile of");

		else if (obj->count > 1001)	/* 1001 - 2500 coins */
			sprintf (coins, "a huge pile of");

		else if (obj->count > 101)	/* 101 - 1000 coins */
			sprintf (coins, "a big pile of");

		else if (obj->count > 51)	/* 51 - 100 coins */
			sprintf (coins, "a pile of");

		else if (obj->count > 21)	/* 21 - 50 coins */
			sprintf (coins, "a small pile of");

		else			/* 5 - 20 coins */
			sprintf (coins, "a handful of");

		if (obj->nVirtual == MORGUL_1)
		{
			if (obj->count == 1)
				strcat (coins, " crudely-hewn dark granite token");
			else
				strcat (coins, " crudely-hewn dark granite tokens");
		}

		else if (obj->nVirtual == MORGUL_5)
		{
			if (obj->count == 1)
				strcat (coins, " razor-sharp flake of obsidian");
			else
				strcat (coins, " razor-sharp flakes of obsidian");
		}

		else if (obj->nVirtual == MORGUL_50)
		{
			if (obj->count == 1)
				strcat (coins, " rectangular token of dusky brass");
			else
				strcat (coins, " rectangular tokens of dusky brass");
		}

		else if (obj->nVirtual == MORGUL_200)
		{
			if (obj->count == 1)
				strcat (coins, " weighty pentagonal bronze coin");
			else
				strcat (coins, " weighty pentagonal bronze coins");
		}

		else if (obj->nVirtual == MORGUL_K)
		{
			if (obj->count == 1)
				strcat (coins, " hexagonal token of blackened steel");
			else
				strcat (coins, " hexagonal tokens of blackened steel");
		}

		else if (obj->nVirtual == MORGUL_10K)
		{
			if (obj->count == 1)
				strcat (coins, " octagonal coin of smoky silver");
			else
				strcat (coins, " octagonal coins of smoky silver");
		}

		else if (obj->nVirtual == GONDOR_1)
		{
			if (obj->count == 1)
				strcat (coins, " semicircular copper coin");
			else
				strcat (coins, " semicircular copper coins");
		}

		else if (obj->nVirtual == GONDOR_5)
		{
			if (obj->count == 1)
				strcat (coins, " large, rounded bronze coin");
			else
				strcat (coins, " large, rounded bronze coins");
		}
		
		else if (obj->nVirtual == GONDOR_50)
		{
			if (obj->count == 1)
				strcat (coins, " thin, ridged silver coin");
			else
				strcat (coins, " thin, ridged silver coins");
		}
		
		else if (obj->nVirtual == GONDOR_200)
		{
			if (obj->count == 1)
				strcat (coins, " heavy, oblong silver coin");
			else
				strcat (coins, " heavy, oblong silver coins");
		}
		
		else if (obj->nVirtual == GONDOR_K)
		{
			if (obj->count == 1)
				strcat (coins, " thick, hexagonal gold coin");
			else
				strcat (coins, " thick, hexagonal gold coins");
		}
		
		else if (obj->nVirtual == GONDOR_10K)
		{
			if (obj->count == 1)
				strcat (coins, " thin, slightly fluted gold coin");
			else
				strcat (coins, " thin, slightly fluted gold coins");
		}

		else if (obj->nVirtual == NORTH_1)
		{
			if (obj->count == 1)
				strcat (coins, " small, rounded bronze coin");
			else
				strcat (coins, " small, rounded bronze coins");
		}
		else if (obj->nVirtual == NORTH_4)
		{
			if (obj->count == 1)
				strcat (coins, " heavy, stamped bronze coin");
			else
				strcat (coins, " heavy, stamped bronze coins");
		}
		else if (obj->nVirtual == NORTH_24)
		{
			if (obj->count == 1)
				strcat (coins, " hexagonal, stamped silver coin");
			else
				strcat (coins, " hexagonal, stamped silver coins");
		}
		else if (obj->nVirtual == NORTH_960)
		{
			if (obj->count == 1)
				strcat (coins, " large, ornately detailed silver coin");
			else
				strcat (coins, " large, ornately detailed silver coins");
		}
		else if (obj->nVirtual == EDEN_1)
		{
			if (obj->count == 1)
				strcat (coins, " small iron disc");
			else
				strcat (coins, " small iron discs");
		}
		else if (obj->nVirtual == EDEN_5)
		{
			if (obj->count == 1)
				strcat (coins, " dull copper coin");
			else
				strcat (coins, " dull copper coins");
		}
		else if (obj->nVirtual == EDEN_50)
		{
			if (obj->count == 1)
				strcat (coins, " small, circular silver coin");
			else
				strcat (coins, " small, circular silver coins");
		}
		else if (obj->nVirtual == EDEN_200)
		{
			if (obj->count == 1)
				strcat (coins, " brazen silver coin");
			else
				strcat (coins, " brazen silver coins");
		}
		else if (obj->nVirtual == EDEN_K)
		{
			if (obj->count == 1)
				strcat (coins, " ornate silver coin");
			else
				strcat (coins, " ornate silver coins");
		}
		else if (obj->nVirtual == EDEN_10K)
		{
			if (obj->count == 1)
				strcat (coins, " heavy, gleaming, gold coin");
			else
				strcat (coins, " heavy, gleaming, gold coins");
		}
		else if (obj->nVirtual == HARAD_1)
		{
			if (obj->count > 1)
				strcat (coins, " small copper coins");
			else
				strcat (coins, " small copper coin");
		}
		else if (obj->nVirtual == HARAD_5)
		{
			if (obj->count > 1)
				strcat (coins, " large bronze coins");
			else
				strcat (coins, " large bronze coin");
		}
		else if (obj->nVirtual == HARAD_50)
		{
			if (obj->count > 1)
				strcat (coins, " thin silver coins");
			else
				strcat (coins, " thin silver coin");
		}
		else if (obj->nVirtual == HARAD_200)
		{
			if (obj->count > 1)
				strcat (coins, " heavy silver coins");
			else
				strcat (coins, " heavy silver coin");
		}
		else if (obj->nVirtual == HARAD_K)
		{
			if (obj->count > 1)
				strcat (coins, " small gold coins");
			else
				strcat (coins, " small gold coin");
		}
		else if (obj->nVirtual == HARAD_10K)
		{
			if (obj->count > 1)
				strcat (coins, " shiny, thick round gold coins");
			else
				strcat (coins, " shiny, thick round gold coin");
		}
		return coins;
	}

	else if (obj->obj_flags.type_flag == ITEM_DRINKCON)
	{

		if (!obj->o.drinkcon.volume)
			return obj->short_description;

		if (!(tobj = vtoo (obj->o.drinkcon.liquid)))
		{
			sprintf (drinkcon, "%s filled with an unknown liquid",
				obj->short_description);
			return drinkcon;
		}

		sprintf (drinkcon, "%s filled with %s",
			obj->short_description, tobj->short_description);
		return drinkcon;
	}

	else if (obj->obj_flags.type_flag == ITEM_DRYCON)
	{
		
		if (!obj->o.drycon.volume)
			return obj->short_description;
		
		if (!(tobj = vtoo (obj->o.drycon.contents)))
		{
			sprintf (drycon, "%s filled with an unknown contents",
					 obj->short_description);
			return drycon;
		}
		
		sprintf (drycon, "%s filled with %s",
				 obj->short_description, tobj->short_description);
		return drycon;
	}
	
	else if (obj->obj_flags.type_flag == ITEM_FOOD && obj->count <= 1)
	{

		bite_num = obj->o.food.bites;

		total_bites = vtoo (obj->nVirtual)->o.food.bites;

		if (bite_num > total_bites)
			total_bites = bite_num;

		if (!bite_num || bite_num == total_bites)
			return obj->short_description;

		total_bites = MAX (1, total_bites);

		switch (((bite_num - 1) * 7) / total_bites)
		{
		case 0:
			sprintf (food, "scraps of %s", obj->short_description);
			break;

		case 1:
			sprintf (food, "a small amount of %s", obj->short_description);
			break;

		case 2:
			sprintf (food, "less than half of %s", obj->short_description);
			break;

		case 3:
			sprintf (food, "half of %s", obj->short_description);
			break;

		case 4:
			sprintf (food, "more than half of %s", obj->short_description);
			break;

		case 5:
			sprintf (food, "%s that was bitten", obj->short_description);
			break;

		case 6:
			sprintf (food, "%s with a bite taken out", obj->short_description);
			break;
		}

		return food;
	}

	
	if (obj->count <= 1)
		return obj->short_description;

	argument = one_argument (obj->short_description, buf);

	if (!str_cmp (buf, "a") || !str_cmp (buf, "an") || !str_cmp (buf, "the"))
		sprintf (buf, "%d %s%s", obj->count, argument,
		(argument[strlen (argument) - 1] != 's') ? "s" : "");
	else
		sprintf (buf, "%s (x%d)", obj->short_description, obj->count);

	if (strlen (buf) > 158)
	{
		memcpy (description, buf, 158);
		description[159] = '\0';
	}
	else
		strcpy (description, buf);

	return description;
}

char *
obj_desc (OBJ_DATA * obj, int level_see)
{
	int bite_num;
	int total_bites;
	static char buf[MAX_STRING_LENGTH]= { '\0' };
	static char description[160];

	if (obj->obj_flags.type_flag == ITEM_MONEY)
	{
		if (level_see == 2)
		{
		if (obj->count > 1 && obj->count < 5)
			sprintf (description, "There are %s here.", obj_short_desc (obj));
		else
			sprintf (description, "There is %s here.", obj_short_desc (obj));

		return description;
		}
		else 
		{
			sprintf (description, "Something small is here.");
						
			return description;
		}	
	}

	if (obj->obj_flags.type_flag == ITEM_FOOD)
	{
		bite_num = obj->o.food.bites;
		total_bites = vtoo (obj->nVirtual)->o.food.bites;

		if (!bite_num || bite_num >= total_bites)
			sprintf (description, "%s", obj->description);

		else
			switch (((bite_num - 1) * 7) / total_bites)
		{
			case 0:
				if (level_see > 1)
					sprintf (description, "Scraps of %s have been left here.",
					obj->short_description);
				else 
					sprintf (description, "Scraps of something have been left here.");

				break;

			case 1:
				if (level_see > 1)
					sprintf (description, "A small amount of %s has been left "
					"here.", obj->short_description);
				else 
					sprintf (description, "A small amount of something have been left here.");
				break;

			case 2:
				if (level_see > 1)
					sprintf (description, "%s is here, more than half eaten.",
					obj->short_description);
				else 
					sprintf (description, "Something is here, more than half eaten.");

				break;

			case 3:
				if (level_see > 1)
					sprintf (description, "%s is here, half eaten.",
					obj->short_description);
				else
					sprintf (description, "Something is here, half eaten.",
							 obj->short_description);
			
				break;

			case 4:
				if (level_see > 1)
					sprintf (description, "%s is here, partially eaten.",
					obj->short_description);
				else
					sprintf (description, "Something is here, partially eaten.");
				break;

			case 5:
				if (level_see > 1)
					sprintf (description, "%s with a couple of bites taken out. "
					"is here.", obj->short_description);
				else
					sprintf (description, "Something is here, with a couple of bites taken out.");
				
				break;

			case 6:
				if (level_see > 1)
					sprintf (description, "%s with a bite taken out is here.",
					obj->short_description);
				else
					sprintf (description, "Something with a bite taken out is here.");
				break;
		}

		*description = toupper (*description);
	}
	else if (obj->obj_flags.type_flag == ITEM_LIGHT 
			 && obj->o.light.hours 
			 && obj->o.light.on)
		sprintf (description, "%s #1(lit)#0", obj->description);
	
	else if (level_see == 1)
		sprintf (description, "Something is here.");
	else 
		sprintf (description, "%s", obj->description);

	if (obj->count <= 1)	
		return description;


	sprintf (buf, "%s #2(x%d)#0", description, obj->count);

	return buf;
}



int
odds_sqrt (int percent)
{
	/* I had a bit of trouble getting sqrt to link in with the mud on
	Novell's Unix (UNIX_SV), so I came up with this table.  The index
	is the a percent, and the table lookup is the square root of the
	percent (x100).  The table goes from 0 to 119 */

	const int sqrt_tab[] = {
		00, 10, 14, 17, 20, 22, 24, 26, 28, 29, 31, 33, 34, 36, 37, 38,
		40, 41, 42, 43, 44, 45, 46, 47, 48, 50, 50, 51, 52, 53, 54,
		55, 56, 57, 58, 59, 59, 60, 61, 62, 63, 64, 64, 65, 66, 67,
		67, 68, 69, 69, 70, 71, 72, 72, 73, 74, 74, 75, 76, 76, 77,
		78, 78, 79, 80, 80, 81, 81, 82, 83, 83, 84, 84, 85, 86, 86,
		87, 87, 88, 88, 89, 90, 90, 91, 91, 92, 92, 93, 93, 94, 94,
		95, 95, 96, 96, 97, 97, 98, 98, 99, 100, 100, 100, 101, 101, 102,
		102, 103, 103, 104, 104, 105, 105, 106, 106, 107, 107, 108, 108, 109
	};

	if (percent < 0)
		return 0;

	if (percent >= 120)
		return percent;

	return sqrt_tab[percent];
}

	//test if the room is lit. Checks for carried light sources as well as sources in the room or on the tables. Called after actions that may have extinguished light sources
	//will also includes environemntal light - sun, moon, snow reflections
void
room_light (ROOM_DATA * room)
{
	float light = NO_MOON;
	CHAR_DATA *tch;
	OBJ_DATA *obj;
	OBJ_DATA *table_obj;

	if (!room)
		return;

	if (room->people != NULL)
	{
		for (tch = room->people; tch; tch = tch->next_in_room) // check to see if anyone is holding a light
		{

			if (tch->deleted)
				continue;

			if (tch->right_hand 
				&& tch->right_hand->obj_flags.type_flag == ITEM_LIGHT
				&& tch->right_hand->o.light.hours
				&& tch->right_hand->o.light.on)
				light += tch->right_hand->o.light.bright;

			if (tch->left_hand 
				&& tch->left_hand->obj_flags.type_flag == ITEM_LIGHT
				&& tch->left_hand->o.light.hours 
				&& tch->left_hand->o.light.on)
				light += tch->left_hand->o.light.bright;

			if (tch->equip != NULL)
			{
				for (obj = tch->equip; obj; obj = obj->next_content)
				{
					if (obj->obj_flags.type_flag == ITEM_LIGHT
						&& obj->o.light.hours 
						&& obj->o.light.on)
						light += obj->o.light.bright;
				}
			}
		}
	}

	if (room->contents != NULL)
	{
		for (obj = room->contents; obj; obj = obj->next_content) 
				// look to see if there are any lights in the room
			
		{
			if (obj->next_content && obj->next_content == obj)
				obj->next_content = NULL;
			if (obj->obj_flags.type_flag == ITEM_LIGHT &&
				obj->o.light.hours && obj->o.light.on)
			{
				light+=obj->o.light.bright;
			}
			if (IS_TABLE(obj))
			{
				for (table_obj = obj->contains; table_obj; table_obj = table_obj->next_content)
				{
					if (table_obj->obj_flags.type_flag == ITEM_LIGHT && table_obj->o.light.hours && table_obj->o.light.on)
					{
						light+=table_obj->o.light.bright;
					}
				}
			}
		}
	}

		//drop in intensty of objects that generate light, by room size in square-yards
	if (room->room_size == ROOM_SIZE_STORAGE) //storage room
		light = (light/1.0);
	else if (room->room_size == ROOM_SIZE_DETAIL) 
		light = (light/30.0);
	else if (room->room_size == ROOM_SIZE_EXPLORE) 
		light = (light/484.0);
	else  if (room->room_size == ROOM_SIZE_VALLEY)
		light = (light/12100.0);
	
	if (IS_SET (room->room_flags, LIGHT))
		light = light + DEFAULT_LIGHT;
	
	if (!IS_SET (room->room_flags, INDOORS))
		room->light = light + global_sun_light + moon_light[room->zone];
	else 
		room->light = light;
	
}



	//assuming object and character are in the same room, this test if the character can actually 'see' the obejct.
	//returns 0 if they cannot see it
	//returns 1 if they can see it partially
	//returns 2 if they can see it fully
	//affected by light level in the room
int
can_see_obj (CHAR_DATA * ch, OBJ_DATA * obj)
{
	AFFECTED_TYPE *af;

	if (!ch || !obj)
		return 0;

	if (ch->get_trust())
		return 2;

	if (IS_SET (obj->obj_flags.extra_flags, ITEM_VNPC))
		return 0;

	if (ch->is_blind())
		return 0;

	if (is_dark (ch->room)
		&& !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
		&& !IS_SET (ch->affected_by, AFF_INFRAVIS)
		&& !(obj->obj_flags.type_flag == ITEM_LIGHT && obj->o.light.on))
		return 0;

	if ((af = get_obj_affect (obj, MAGIC_HIDDEN)))
		if (af->a.hidden.coldload_id != ch->coldload_id)
			return 0;

	if (IS_SET (obj->obj_flags.extra_flags, ITEM_INVISIBLE) &&
		!get_affect (ch, MAGIC_AFFECT_SEE_INVISIBLE))
		return 0;

	if (ch->room->light > FAINT_LIGHT)
		return 2;
	else if ((ch->room->light <= FAINT_LIGHT) && (ch->room->light > NO_MOON))
		return 1;
	else if (ch->room->light <= NO_MOON)
		return 0;

	return 2;
}

	//assuming mob and character are in the same room, this test if the character can actually 'see' the mob.
	//returns 0 if they cannot see it
	//returns 1 if they can see it partially
	//returns 2 if they can see it fully
	//affected by light level in the room

int
can_see_mob (CHAR_DATA * ch, CHAR_DATA * tmob)
{

	if (!ch || !tmob)
		return 0;
	
	if (ch->get_trust())
		return 2;
	
	if (ch->is_blind())
		return 0;
	
	if (IS_SET (tmob->flags, FLAG_WIZINVIS))
		return 0;
		
		//is the target or his group carrying lights
	if (is_with_group(ch) && (group_light_level(ch) > 4))
		return 2;
	
		//is the room really dark and the mob is not carrying a light?
	if (is_dark (ch->room)
		&& !get_affect (ch, MAGIC_AFFECT_INFRAVISION)
		&& !IS_SET (ch->affected_by, AFF_INFRAVIS)
		&& (char_light_carry(ch) < -3.0))
		return 0;
	
		//is he hidden
	if (get_affect(tmob, MAGIC_HIDDEN))
			return 0;
	
	
	if (ch->room->light > FAINT_LIGHT)
		return 2;
	else if ((ch->room->light <= FAINT_LIGHT) && (ch->room->light > NO_MOON))
		return 1;
	else if (ch->room->light <= NO_MOON)
		return 0;
	
		//TODO: Orc adjsutments for ability to see in the dark
	return 2;
}


TEXT_DATA *
add_text (TEXT_DATA ** list, char *filename, char *document_name)
{
	TEXT_DATA *text;
	char *doc;
	
	doc = file_to_string (filename);
	
	text = new TEXT_DATA;
	
	if (list == NULL)
		text->next = NULL;
	else
		text->next = *list;
	
	*list = text;
	
	text->filename = duplicateString (filename);
	text->name = duplicateString (document_name);
	text->text = doc;
	
	return text;
}

int
get_next_coldload_id (int for_a_pc)
{
	CHAR_DATA *tch;
	static int coldloads_read = 0;
	int return_coldload_id;
	FILE *fp = NULL;

	if (!coldloads_read)
	{
		if (!(fp = fopen (COLDLOAD_IDS, "r")))
			system_log ("NEW COLDLOAD ID FILE BEING CREATED.", true);
		else
		{
			fscanf (fp, "%d %d %d\n",
				&next_pc_coldload_id,
				&next_mob_coldload_id, &next_obj_coldload_id);
			fclose (fp);
		}

		if (next_mob_coldload_id > 100000)
			next_mob_coldload_id = 0;
		if (next_obj_coldload_id > 100000)
			next_obj_coldload_id = 0;

		next_mob_coldload_id += 100;	/* On boot, inc 100 in case */
		/* the mud crashed last time, */
		/* so we don't double count. */
		next_obj_coldload_id += 100;

		coldloads_read = 1;
	}

	if (for_a_pc == 1)
	{
		while ((tch =
			get_char_id ((return_coldload_id = ++next_pc_coldload_id))))
			;
	}
	else if (for_a_pc == 2)
	{
		return_coldload_id = ++next_obj_coldload_id;
	}
	else
	{
		while ((tch =
			get_char_id ((return_coldload_id = ++next_mob_coldload_id))))
			;
	}

	if (for_a_pc == 2 || !for_a_pc)
		return return_coldload_id;

	if (!(fp = fopen (COLDLOAD_IDS ".new", "w")))
	{
		system_log ("COLDLOAD ID FILE COULD NOT BE CREATED!!!", true);
		perror ("coldload");
		return return_coldload_id;
	}

	fprintf (fp, "%d %d %d\n", next_pc_coldload_id, next_mob_coldload_id,
		next_obj_coldload_id);

	fclose (fp);

	system ("mv " COLDLOAD_IDS ".new " COLDLOAD_IDS);

	return return_coldload_id;
}


int
get_bite_value (OBJ_DATA * obj)
{
	int bite_num;
	int total_bites;
	int bite_value;

	/* This little routine calculates how much food value is in one
	bite.  The initial bites may yield more food than later bites.
	This depends on how even the ratio of bites to food value is.
	*/

	bite_num = obj->o.food.bites;
	total_bites = vtoo (obj->nVirtual)->o.food.bites;

	if (total_bites <= 1)
		return obj->o.food.food_value;

	bite_value = obj->o.food.food_value / total_bites;

	if (bite_value * total_bites + bite_num <= obj->o.food.food_value)
		bite_value++;

	fflush (stdout);
	return bite_value;
}

int
is_name_in_list (char *name, char *list)
{
	char *argument;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (list, buf);

	while (*buf)
	{

		if (!str_cmp (name, buf))
			return 1;

		argument = one_argument (argument, buf);
	}

	return 0;
}

char *
vnum_to_name (int vnum)
{
	OBJ_DATA *obj;
	static char buf[MAX_STRING_LENGTH]= { '\0' };

	if (!(obj = vtoo (vnum)))
		return "an unknown fuel.";

	*buf = '\0';

	one_argument (obj->name, buf);

	if (buf[strlen (buf) - 1] == ',')
		buf[strlen (buf) - 1] = '\0';

	return buf;
}

int
obj_mass (OBJ_DATA * obj)
{
	int mass = 0;
	OBJ_DATA *contents;
	
	mass = obj->obj_flags.weight;
	
	if (obj->count)
		mass = mass * obj->count;
	
	
	/**
	 capacity - od.value[0]
	 volume - od.value[1]
	 contents/liquid - od.value[2]
	 **/
	if (obj->obj_flags.type_flag == ITEM_DRINKCON ||
		obj->obj_flags.type_flag == ITEM_DRYCON ||
		obj->obj_flags.type_flag == ITEM_FOUNTAIN ||
		obj->obj_flags.type_flag == ITEM_LIGHT)
	{
		if (obj->o.od.value[1] == -1)
			mass += 1000000;
		else
		{
				// we need to figure in the weight of the liquid/dry the item contains
			
				//contents or liquid vnum
			contents = vtoo (obj->o.od.value[2]);
			
				//increase mass by contents volume * weight
			if (contents)
				mass += obj->o.od.value[1] * contents->obj_flags.weight;
			
		}
	}
	
	else
	{
		mass += obj->contained_wt;
	}
	
	if (mass == 0)
		mass = 1;
	
	return mass;
}



/*------------------------------------------------------------------------\
|  is_vowel(char)                                                          |
|                                                                         |
|  Returns true if the character is a vowel. Useful if you need to know   |
|  if an indefinite article should be 'a' or 'an'.                        |
\------------------------------------------------------------------------*/
bool
is_vowel (char c)
{
	switch (c)
	{
	case 'a':
	case 'e':
	case 'i':
	case 'o':
	case 'u':
		return true;
	default:
		return false;
	}
}


/*---------------------------------------------------------------------\
| swap_xmote_target (CHAR_DATA * ch, char *argument int *is_imote)    |
|                                                                      |
| swaps *target @ or ~target with short_desc                           |
| cmd = 1 (emote call)                                                 |
| cmd = 2 (pmote call)                                                 |
| cmd = 3 (travel string)                                              |
\---------------------------------------------------------------------*/

char *
swap_xmote_target (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH] = { '\0' };
	char key[MAX_STRING_LENGTH] = { '\0' };
	char copy[MAX_STRING_LENGTH] = { '\0' };
	bool tochar = false;
	OBJ_DATA *obj = NULL;
	int key_e = 0;
	char *p = '\0';
	char *temp = NULL;
	bool is_imote = false;

	p = copy;
	temp = argument;

	while (*argument)
	{
		if(cmd==2 && IS_NPC(ch))
			return NULL;

		if (*argument == '@' )
		{
			if (cmd == 2 ) // don't allow @ to be used in pmote
			{
				ch->send_to_char ("You may not refer to yourself in a pmote.");
				return NULL;
			}
			is_imote = true;
			sprintf (p, "#5%s#0",  ch->char_short());
			p += strlen (p);
			argument++;
		}

		else if (*argument == '*')
		{

			argument++;
			while (isdigit (*argument))
			{
				key[key_e++] = *(argument++);
			}
			if (*argument == '.')
			{
				key[key_e++] = *(argument++);
			}
			while (isalpha (*argument) || *argument == '-')
			{
				key[key_e++] = *(argument++);
			}
			key[key_e] = '\0';
			key_e = 0;

			if (!get_obj_in_list_vis (ch, key, ch->room->contents) &&
				!get_obj_in_list_vis (ch, key, ch->right_hand) &&
				!get_obj_in_list_vis (ch, key, ch->left_hand) &&
				!get_obj_in_list_vis (ch, key, ch->equip))
			{
				sprintf (buf, "I don't see %s here.\n", key);
				ch->send_to_char (buf);
				return NULL;
			}
			obj = get_obj_in_list_vis (ch, key, ch->right_hand);

			if (!obj)
				obj = get_obj_in_list_vis (ch, key, ch->left_hand);
			if (!obj)
				obj = get_obj_in_list_vis (ch, key, ch->room->contents);
			if (!obj)
				obj = get_obj_in_list_vis (ch, key, ch->equip);
			sprintf (p, "#2%s#0", obj_short_desc (obj));
			p += strlen (p);

		}
		else if (*argument == '~')
		{

			argument++;
			while (isdigit (*argument))
			{
				key[key_e++] = *(argument++);
			}
			if (*argument == '.')
			{
				key[key_e++] = *(argument++);
			}
			while (isalpha (*argument) || *argument == '-')
			{
				key[key_e++] = *(argument++);
			}
			key[key_e] = '\0';
			key_e = 0;

			if (!get_char_room_vis (ch, key))
			{
				sprintf (buf, "Who is %s?\n", key);
				ch->send_to_char (buf);
				return NULL;
			}
			if (get_char_room_vis (ch, key) == ch)
			{
				ch->send_to_char
					("You shouldn't refer to yourself using the token system.\n");
				return NULL;
			}
			sprintf (p, "#5%s#0", (get_char_room_vis (ch, key))->char_short());
			p += strlen (p);
			tochar = true;
		}
		else
			*(p++) = *(argument++);
	}

	*p = '\0';
	if (cmd == 1)
	{
		if (copy[0] == '\'')
		{
			if (!is_imote)
			{
				sprintf (buf, "#5%s#0%s",  ch->char_short(), copy);
				buf[2] = toupper (buf[2]);
			}
			else
			{
				sprintf (buf, "%s", copy);
				if (buf[0] == '#')
				{
					buf[2] = toupper (buf[2]);
				}
				else
				{
					buf[0] = toupper (buf[0]);
				}
			}
		}
		else
		{
			if (!is_imote)
			{
				sprintf (buf, "#5%s#0 %s",  ch->char_short(), copy);
				buf[2] = toupper (buf[2]);
			}
			else
			{
				sprintf (buf, "%s", copy);
				if (buf[0] == '#')
				{
					buf[2] = toupper (buf[2]);
				}
				else
				{
					buf[0] = toupper (buf[0]);
				}
			}
		}
	}
	else 
	{
		sprintf (buf, "#0%s", copy);// need to add #0 here to reset colors depending on call
	}

	if (buf[strlen (buf) - 1] != '.' && buf[strlen (buf) - 1] != '!'
		&& buf[strlen (buf) - 1] != '?')
		strcat (buf, ".");

	//argument = temp;
	sprintf (argument, "%s", buf);

	return (argument);
}

bool ci_equal_to::compare_equal::operator() (const unsigned char& c1, const unsigned char& c2) const
{ return tolower (c1) == tolower (c2); }

bool ci_equal_to::operator() (const std::string & s1, const std::string & s2) const
{

	std::pair <std::string::const_iterator,
		std::string::const_iterator> result =
		std::mismatch (s1.begin (), s1.end (),   // source range
		s2.begin (),              // comparison start
		compare_equal ());  // comparison

	// match if both at end
	return result.first == s1.end () &&
		result.second == s2.end ();

}

// compare strings for equality using the binary function above
// returns true is s1 == s2
bool ciStringEqual (const std::string & s1, const std::string & s2)
{
	return ci_equal_to () (s1, s2);
}  // end of ciStringEqual


	//mode = 1 is for all normal exits, doors, gates and openings
	//mode = 2 is for all other portals
ROOM_EXIT_DATA *
is_exit_normal(CHAR_DATA* ch, int dir, int mode)
{
	std::multimap<int,ROOM_EXIT_DATA*>::iterator iter;
	if (mode == 1)
	{
		iter = ch->room->exitmap.find(dir);
		
		if (iter != ch->room->exitmap.end())
		{
			if (vtor(iter->second->to_room) != 0)
			{
				return (iter->second);
			}
		}
	}
	return NULL;
	
}


ROOM_EXIT_DATA *
is_exit(CHAR_DATA* ch, int dir)
{
	std::multimap<int,ROOM_EXIT_DATA*>::iterator iter;
	
	iter = ch->room->exitmap.find(dir);
	
	if (iter != ch->room->exitmap.end())
	{
		if (vtor(iter->second->to_room) != 0)
		{
			return (iter->second);
		}
	}
	
	return NULL;
}

ROOM_EXIT_DATA *
exit_from_portal(int port_ident, int to_room)
{
	ROOM_PORTAL_DATA *tport;
	ROOM_EXIT_DATA * texit;
	int from_side;
	int dir;
	
	tport = vtop(port_ident);
	if (!tport)
		return NULL;
	
	if (tport->room_2 == to_room)
	{
		from_side = 2;
		dir = tport->dir_2;
	}
	else if (tport->room_1 == to_room)
	{
		from_side = 1;
		dir = tport->dir_1;
	}
	
	texit = new room_exit_data;
	
	if (from_side == 1)
	{
		texit->sdesc = duplicateString(tport->sdesc_1);
		texit->ldesc = duplicateString(tport->ldesc_1);
		texit->fdesc = duplicateString(tport->fdesc_1);
		texit->keyword = duplicateString(tport->keywords_1);
		texit->portal = tport->ident;
		texit->direction = dir;
		texit->type = tport->type;
		texit->port_flags = tport->port_flags;
		texit->key = tport->key_num_1;
		texit->pick_penalty = tport->pick_key_pen_1;
		texit->to_room = to_room;
		texit->slope = tport->slope;
		texit->quality = tport->quality;
		texit->difficulty = tport->difficulty;
		texit->skill = tport->skill;
		return (texit);
				
	}
	else if (from_side == 2)
	{		
		texit->sdesc = duplicateString(tport->sdesc_2);
		texit->ldesc = duplicateString(tport->ldesc_2);
		texit->fdesc = duplicateString(tport->fdesc_2);
		texit->keyword = duplicateString(tport->keywords_2);
		texit->portal = tport->ident;
		texit->direction = dir;
		texit->type = tport->type;
		texit->port_flags = tport->port_flags;
		texit->key = tport->key_num_2;
		texit->pick_penalty = tport->pick_key_pen_2;
		texit->to_room = to_room;
			//slope is opposite what it is for side 1
		texit->slope = -tport->slope; 
		texit->quality = tport->quality;
		texit->difficulty = tport->difficulty;
		texit->skill = tport->skill;
		return (texit);

	}

	
	return NULL;
}

bool
can_go_exit(CHAR_DATA * ch, int door)
{
	if (ch->room)
		if ((ch)->room->dir_option[door])
			if((ch)->room->dir_option[door]->to_room != NOWHERE)
				if(!IS_SET((ch)->room->dir_option[door]->port_flags, EX_CLOSED))
					return (true);
				
	return (false);	
}

	//returns the distance between two adjacent rooms
float
room_distance (ROOM_DATA *start, ROOM_DATA *targ)
{
	int rsize_1;
	int rsize_2;
	float range_1;
	float range_2;
	float distance;
	
	rsize_1 = start->room_size;
	rsize_2 = targ->room_size;
	
			//get the size of the rooms
	switch (rsize_1)
	{
		case ROOM_SIZE_DETAIL:
			range_1 = DETAIL_SIZE;
			break;
		case ROOM_SIZE_EXPLORE:
			range_1 = EXPLORE_SIZE;
			break;
		case ROOM_SIZE_VALLEY:
			range_1 = VALLEY_SIZE;
			break;	
		case ROOM_SIZE_STORAGE:
			range_1 = STORAGE_SIZE;
			break;	
		default:
			range_1 = DETAIL_SIZE;
			break;
	}
	
	switch (rsize_2)
	{
		case ROOM_SIZE_DETAIL:
			range_2 = DETAIL_SIZE;
			break;
		case ROOM_SIZE_EXPLORE:
			range_2 = EXPLORE_SIZE;
			break;
		case ROOM_SIZE_VALLEY:
			range_2 = VALLEY_SIZE;
			break;
		case ROOM_SIZE_STORAGE:
			range_2 = STORAGE_SIZE;
			break;
		default:
			range_2 = DETAIL_SIZE;
			break;
	}
	
	distance = (range_1 + range_2) / 2.0;
	return (distance);
	
}

	//level of light carried by character
float
char_light_carry(CHAR_DATA* ch)
{ 
	float light_level = 0;
	OBJ_DATA* obj;
	
	if (ch->right_hand && ch->right_hand->obj_flags.type_flag == ITEM_LIGHT
		&& ch->right_hand->o.light.hours
		&& ch->right_hand->o.light.on)
		light_level += ch->right_hand->o.light.bright;
	
	if (ch->left_hand && ch->left_hand->obj_flags.type_flag == ITEM_LIGHT
		&& ch->left_hand->o.light.hours && ch->left_hand->o.light.on)
		light_level += ch->left_hand->o.light.bright;
	
	if (ch->equip != NULL)
	{
		for (obj = ch->equip; obj; obj = obj->next_content)
		{
			if (obj->obj_flags.type_flag == ITEM_LIGHT &&
				obj->o.light.hours && obj->o.light.on)
				light_level += obj->o.light.bright;
		}
	}
	
	return (log10(light_level));
}

char *
convert_dir(char *dir)
{
	if (!str_cmp(dir, "NW") || !str_cmp(dir, "nw"))
		dir = strdup("northwest");
	
	else if (!str_cmp(dir, "NE") || !str_cmp(dir, "ne"))
		dir = strdup("northeast");
	
	else if (!str_cmp(dir, "SE") || !str_cmp(dir, "se"))
		dir = strdup("southeast");
	
	else if (!str_cmp(dir, "SW") || !str_cmp(dir, "sw"))
		dir = strdup("southwest");

	return (dir);
	
	
}

bool
is_orc(CHAR_DATA* ch)
{

	bool flag = false;
	if ((ch->race == lookup_race_id("Orc"))
		|| (ch->race == lookup_race_id("Half-Orc"))
		|| (ch->race == lookup_race_id("Half-Troll"))
		|| (ch->race == lookup_race_id("Troll"))
		|| (ch->race == lookup_race_id("Olog-Hai"))
		|| (ch->race == lookup_race_id("Half Orc"))
		|| (ch->race == lookup_race_id("Mountain Orc"))
		|| (ch->race == lookup_race_id("Mirkwood Orc"))
		|| (ch->race == lookup_race_id("Mordorian Orc")))
		flag = true;

	return flag;
}


	//scans the next room only
	//to be used with crossing or window type portals
void
scan_portal_key(CHAR_DATA* ch, char* key)
{
	int port_id;
	int to_room;
	int dir;
	ROOM_PORTAL_DATA* tport;
	int penalty = 0;
	int skil_pen_level = 0;
	ROOM_DATA *curr_room = NULL;
	ROOM_DATA *next_room = NULL;
	float next_room_dist;
	float curr_dist;
	bool seen = false;
	extern const char* dirs[];
	
	curr_room = ch->room;
	
	for (dir = 0; dir <= LAST_DIR; dir++)
	{
		port_id = keyword_to_portal(ch, dir, key);
		if (port_id > 1)
			break;
	}
	
	if (port_id > 1)
	{
		tport = vtop(port_id);
	
		if (!(tport->type == PORTAL_CROSSING)
			&& !(tport->type == PORTAL_WINDOW))
		{
			ch->send_to_char ("You can't scan that way.\n");
			return;
		}
			
			
		if (tport->room_2 == curr_room->nVirtual)
		{
			to_room = tport->room_1;
			dir = tport->dir_2;
		}
		else if (tport->room_1 == curr_room->nVirtual)
		{
			to_room = tport->room_2;
			dir = tport->dir_1;
		}
		
		next_room = vtor(to_room);
		if (!next_room)
			return;
		
		
		next_room_dist = room_distance(curr_room, next_room);
		curr_dist = next_room_dist;
		
		penalty += scan_weather_penalty(ch->room);
		penalty += scan_distance_penalty(next_room_dist);
		penalty += scan_light_penalty(ch->room, next_room);	
	 
			//give PC a chance to increase skill at this set of penalties
			//actually effects are detailed later
		skill_use(ch, "Scan", penalty);
		
		if (!next_room->psave_loaded)
			load_save_room (next_room);
		
		skil_pen_level = skill_level (ch, "Scan", penalty);
		ch->delay_info1 = dir;
		
		seen = char__room_scan (ch, next_room, dirs[dir], skil_pen_level, false, curr_dist);
	
		if (!seen)
		{
			ch->send_to_char ("You can't see anything on the other side.\n");
			return;
		}
	}
	else 
	{
	ch->send_to_char ("You can't scan that way.\n");
	return;
	}
	
}

char* get_line (char **buf, char *ret_buf)	/* Used by aliases too */
{
	char *p;
	
	if (!*buf)
		return NULL;
	
	while (**buf == ' ' || **buf == '\n' || **buf == '\t' || **buf == '\r')
		(*buf)++;
	
	p = *buf;
	
	while (**buf && **buf != '\n')
    {
		*ret_buf++ = **buf;
		(*buf)++;
    }
	
	*ret_buf = '\0';
	
	return *p ? p : NULL;
}
