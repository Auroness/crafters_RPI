/////////////////////////////////////////////////////////////////////////////
//
/// net_link.h - MUD Connection Class
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


#ifndef _rpie_net_link_h_
#define _rpie_net_link_h_

#include <string>
#include <sys/types.h>
#include <sys/time.h>
#include "constants.h"

class account;

/* Descriptor related constants for limits.c */
#define DESCRIPTOR_DISCONNECT_SECS	(5 * 60)
#define PLAYER_IDLE_SECS		(15 * 60)
#define PLAYER_DISCONNECT_SECS		(20 * 60)


typedef struct descriptor_data DESCRIPTOR_DATA;
typedef struct line_data LINE_DATA;
typedef struct message_data MESSAGE_DATA;


typedef void CALL_PROC (DESCRIPTOR_DATA * d);

extern int free_mem(char *&ptr);
extern int free_mem(void *ptr);



/* modes of connectedness */
enum connection_state
{
	CON_SOFT_REBOOT = -15,
	CON_LINKDEAD = -1,
	CON_PLYNG = 0,
	CON_LOGIN,
	CON_NEW_ACCT_NAME,
	CON_ACCT_EMAIL,
	CON_ENTER_ACCT_NME,
	CON_EMAIL_CONFIRM,
	CON_ACCOUNT_SETUP,
	CON_PENDING_DISC,
	CON_PWDCHK,
	CON_ACCOUNT_MENU,
	CON_WRITING_EMAIL,
	CON_CHG_EMAIL,
	CON_CHG_EMAIL_CNF,
	CON_CHOOSE_PC,
	CON_READING_WAIT,
	CON_RACE_CONFIRM,
	CON_NAME_CONFIRM,
	CON_SEX,
	CON_RMOTD,
	CON_SLCT1,
	CON_PWDNEW,
	CON_PWDNCNF,
	CON_QSTATS,
	CON_RACE,
	CON_DECOY,
	CON_CREATION,
	CON_ATTRIBUTES,
	CON_DOCUMENTS,
	CON_WAIT_MSG,
	CON_DOC_WAIT,
	CON_SKILLS,
	CON_PLAYER_NEW,
	CON_AGE,
	CON_HEIGHT_WEIGHT,
	CON_INTRO_MSG,
	CON_INTRO_WAIT,
	CON_COMMENT,
	CON_READ_REJECT,
	CON_WEB_CONNECTION,
	CON_RACE_SELECT,
	CON_DELETE_PC,
	CON_TERMINATE_CONFIRM,
	CON_PWDGET,
	CON_PWDCNF,
	CON_CREATE_GUEST,
	CON_GUEST_ARRIVE,
	CON_ROLE_CONFIRM,
	CON_SPECIAL_ROLE_SELECT,
	CON_SPECIAL_ROLE_CONFIRM,
	CON_MAIL_MENU,
	CON_COMPOSE_SUBJECT,
	CON_COMPOSE_MAIL_TO,
	CON_COMPOSING_MESSAGE,
	CON_COMPOSE_MESSAGE,
	CON_READ_MESSAGE,
	CON_PRIV_CONFIRM,
	CON_LOCATION,
	CON_RETIRE,
	CON_ACCT_POLICIES,
	CON_PROFESSION,
	CON_SDESC,
	CON_LDESC,
	CON_KEYWORDS,
	CON_FDESC,
	CON_COMPOSING_FDESC,
	CON_COMPOSING_COMMENT
};
typedef enum connection_state CONNECTION_STATE;

struct txt_block
{
	char *text;
	struct txt_block *next;
	
	txt_block()
	{
		text = NULL;
		next = NULL;
	}
};

struct txt_q
{
	struct txt_block *head;
	struct txt_block *tail;
};

struct snoop_data
{
	struct char_data *snooping;
	struct char_data *snoop_by;
};


struct line_data
{
	char *line[MAX_STRING_LENGTH];
};

struct msg_type
{
	char *attacker_msg;
	char *victim_msg;
	char *room_msg;
};

struct message_type
{
	struct msg_type die_msg;
	struct msg_type miss_msg;
	struct msg_type hit_msg;
	struct msg_type sanctuary_msg;
	struct msg_type god_msg;
	struct message_type *next;
};

struct message_list
{
	int a_type;
	int number_of_attacks;
	struct message_type *msg;
};

class message_data {
public:
	int nVirtual;
	long nTimestamp;
	long flags;
	char *poster;
	char *date;
	char *subject;
	char *info;
	char *message;
	char *icdate;
	char *target;
	
	message_data() {
		nVirtual = 0;
		nTimestamp = 0;
		flags = 0;
		poster = NULL;
		date = NULL;
		subject = NULL;
		info = NULL;
		message = NULL;
		icdate = NULL;
		target = NULL;
	}
	
	~message_data() {
		free_mem(poster);
		free_mem(date);
		free_mem(subject);
		free_mem(info);
		free_mem(message);
		free_mem(icdate);
		free_mem(target);
	}
};

/*
 specifically this is a socket data class
 totally bloated with data bits that belong
 elsewhere.
 */
struct descriptor_data
{
	int hSocketFD;		/* file descriptor for socket (comm.c) */
	
	char *strClientHostname;	/* (act.informative.c, comm.c, guest.c, nanny.c, staff.c) */
	char *strClientIpAddr;	/* (comm.c, nanny.c)  */
	account *acct;		/* login account */
	
	CONNECTION_STATE connected;	/* mode of 'connectedness'    */
	int wait;			/* wait for how many loops    */
	char *showstr_head;		/* for paging through texts     */
	char *showstr_point;		/*       -                    */
	char *header;
	char *descStr;			/* for the modify-str system  */
	int edit_type;		/* Type of document being edited */
	int edit_index;		/* Type relative index for edit */
	char *edit_string;		/* String being edited by edit command */
	int edit_length;		/* Length of piece being edited */
	int edit_line_first;		/* Starting line edited in document */
	unsigned int max_str;		/* -                          */
	int prompt_mode;		/* control of prompt-printing */
	char buf[MAX_STRING_LENGTH];	/* buffer for raw input       */
	char last_input[MAX_INPUT_LENGTH];	/* the last input  > allows '!' to do last command       */
	struct txt_q output;		/* q of strings to send       */
	struct txt_q input;		/* q of unprocessed input     */
	struct char_data *character;	/* linked to char             */
	struct char_data *original;	/* original char              */
	struct snoop_data snoop;	/* to snoop people.              */
	struct descriptor_data *next;	/* link to next descriptor    */
	CALL_PROC *proc;		/* Procedure to call after input */
	LINE_DATA *lines;
	size_t col;
	int line;
	int tos_line;			/* top of screen line 1 */
	int max_lines;
	int max_columns;
	int edit_mode;
	struct timeval login_time;
	message_data *pending_message;
	int time_last_activity;
	int idle;
	char *stored;			/* miscellaneous storage */
	int color;			/* ANSI color */
	int resolved;
};

/* Function Prototypes */
void init_descriptor (DESCRIPTOR_DATA * newd, int desc);
void free_descriptor (DESCRIPTOR_DATA * d);

void descriptor__drop_connections (fd_set * readfds, fd_set * writefds,
								   fd_set * exceptfds);

/* Private Interface */
void nonblock (int s);


int write_to_descriptor (DESCRIPTOR_DATA * d, const char *txt);
int get_from_q (struct txt_q *queue, char *dest);
bool get_from_q (struct txt_q *queue, std::string& dest);
void write_to_q (const char *txt, struct txt_q *queue);


/* Public Variables */
extern struct descriptor_data *descriptor_list;
extern struct descriptor_data *last_descriptor;
extern int avail_descs;
extern int nTotalConnections;
#endif // _rpie_net_link_h_
