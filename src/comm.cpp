//////////////////////////////////////////////////////////////////////////////
//
/// comm.c : Central Game Loop
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


#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/resource.h>
#include </usr/local/include/mysql++/mysql++.h>

#include "server.h"

#include "structs.h"
#include "account.h"
#include "net_link.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"
#include "decl.h"
#include "room.h"
#include "group.h"

extern rpie::server engine;

extern int errno;		///< Global error number
extern int last_vnpc_sale; // timestamp of last time vnpc sale was called, stored in mysql
extern int getrlimit();

/* local globals */
int nMainSocket;
int knockout;			/* Cleanup dead pcs */
int run_mult = 10000;
rpie::server engine;

volatile sig_atomic_t crashed = 0;
bool debug_mode = true;		/* True to enable core dumps in crash recovery. */
bool mysql_logging = true;  //TODO REMEMBER TO CHANGE THIS to TRUE FOR THE REAL GAME

bool bIsCopyOver = false;
bool maintenance_lock = false;

int finished_booting = 0;
int guest_conns = 0;
long starttime, crashtime;
int count_guest_online = 0;
long bootstart;
bool pending_reboot = false;
int shutd = 0;			/* clean shutdown */
int tics = 0;			/* for extern checkpointing */

void check_reboot (void);

int run_the_game (int port_num);
int game_loop (int s);
int init_socket (int port_num);
int new_descriptor (int s);
int process_output (DESCRIPTOR_DATA * t);
int process_input (DESCRIPTOR_DATA * t);
void close_sockets (int s);
void close_socket (DESCRIPTOR_DATA * d);
struct timeval timediff (struct timeval *a, struct timeval *b);
void flush_queues (DESCRIPTOR_DATA * d);
void check_sitebans ();
void parse_name (DESCRIPTOR_DATA * desc, char *arg);
int valgrind = 0;
struct timeval time_now;


/* extern fcnts */

CHAR_DATA *make_char (char *name, DESCRIPTOR_DATA * desc);
	//void boot_db (void);
void zone_update (void);
void point_update (void);	/* In limits.c */
void mobile_activity (void);
void stop_fighting (CHAR_DATA * ch);
void show_string (DESCRIPTOR_DATA * d, char *input);

/* *********************************************************************
*  main game loop and related stuff				       *
********************************************************************* */

int
game_main (int argc, char *argv[])
{
	char buf[512];
	engine.load_config_files ();
	bootstart = time (0);

	if (chdir (DFLT_DIR) < 0)
	{
		std::string error_message =
			"The system call 'chdir' failed to switch to the directory '";
		error_message += DFLT_DIR;
		error_message += "' for the following reason";
		perror (error_message.c_str ());
		fprintf (stderr, "Did you run 'make install'?\n"
			"Does 'make install-libdir' fail?\n");
		exit (1);
	}

	int port_num = 0;
	if (argc < 2 || !isdigit (*argv[1]) || (port_num = strtol (argv[1],0,0)) <= 1024)
	{
		fprintf (stderr,
			"Please specify a port number above 1024.\n"
			"E.G: $ server 4500\n");
		exit (0);
	}

	if (argv[2] && *argv[2] == '-')
	{
		if (*(argv[2] + 1) == 'c')
		{
			bIsCopyOver = true;
			chdir ("lib");
			nMainSocket = atoi (argv[3]);
		}
		else if (*(argv[2] + 1) == 'v')
		{
			valgrind = true;
		}
	}
	engine.set_config ("server_port", std::string (argv[1]));

	init_mysql();
	init_mysqlplus();

	sprintf (buf, "Running game on port %d.", port_num);

	system_log (buf, false);

	srand (time (0));
	run_the_game (port_num);
	return (0);

}


/* Init sockets, run game, and cleanup sockets */
int
run_the_game (int port_num)
{
	FILE *fp;
	struct rlimit rlp;

	void signal_setup (void);

	if (!(fp = fopen ("booting", "r")))
	{
		fp = fopen ("booting", "w+");
		fclose (fp);
		system_log ("Crash loop check initiated.", false);
	}
	else
	{
		system_log ("Lockfile found during bootup - shutting down.", false);
		fprintf (stderr, "Lockfile found during bootup - shutting down.\n");
		fclose (fp);
		exit (-1);
	}

	system_log ("Signal trapping.", false);
	signal_setup ();

	if ((fp = fopen (".reboot", "r")))
	{
		fgets (BOOT, 26, fp);
		fclose (fp);
		unlink (".reboot");
	}

	system ("ulimit -c unlimited");

	system_log ("Initializing CPU cycle alarm.", false);
	init_alarm_handler ();

	if (!bIsCopyOver)
	{
		system_log ("Opening mother connection.", false);
		nMainSocket = init_socket (port_num);
	}

	try 
	{
		boot_db ();
	}
	catch (const std::bad_alloc& mallocErr) {
		std::cerr << "bad_alloc caught: " << mallocErr.what() << std::endl;
	}

	system_log ("Entering game loop.", false);
	starttime = time (0);

	getrlimit (RLIMIT_CORE, &rlp);
	rlp.rlim_cur = rlp.rlim_max;
	setrlimit (RLIMIT_CORE, &rlp);
	getrlimit (RLIMIT_CORE, &rlp);

	if ((fp = fopen ("last_crash", "r")))
	{
		crashtime = fread_number (fp);
		fclose (fp);
	}

		mysql_safe_query ("UPDATE server_statistics SET last_reboot = %d",
			(int) (time (0)));
	
	game_loop (nMainSocket);

	save_tracks ();
	save_stayput_mobiles ();
	save_player_rooms ();
	save_dwelling_rooms ();
	save_banned_sites ();
	
	mysql_safe_query ("DELETE FROM players_online WHERE port = %d", port_num);

	close_sockets (nMainSocket);	

	system_log ("Normal termination of game.", false);

	return 0;
}



/* Accept new connects, relay commands, and call 'heartbeat-functs' */

int
game_loop (int s)
{
	fd_set readfds;
	fd_set writefds;
	fd_set exceptfds;
	fd_set visedit_fds;
	fd_set tmp_read_fds;
	fd_set tmp_write_fds;
	char comm[MAX_STRING_LENGTH] = "";
	DESCRIPTOR_DATA *point, *next_point, *next_to_process;
	DESCRIPTOR_DATA *d;
	CHAR_DATA *tch;
	int pulse = 0, purse = 0;
	int mask;
	int i;
	struct timeval null_time;
	struct timeval pulse_time;
	struct timeval current_time;
	struct rlimit limit;
	std::string prompt;
	std::list<char_data*>::iterator tch_iterator;
	extern QE_DATA *quarter_event_list;


	timerclear (&null_time);	/* a define in sys/time.h */
	FD_ZERO (&readfds);
	FD_ZERO (&writefds);
	FD_ZERO (&exceptfds);
	FD_ZERO (&visedit_fds);
	FD_ZERO (&tmp_read_fds);
	FD_ZERO (&tmp_write_fds);

	maxdesc = s;

	if (bIsCopyOver)
	{
		copyover_recovery ();
		bIsCopyOver = false;
		if (s <= 0)
		{
			system_log
				("Mother descriptor not found after copyover. Shutting down.",
				true);
			shutd = 1;
		}
	}

	check_maintenance ();


#ifndef MACOSX

	getrlimit (RLIMIT_NOFILE, &limit);	/* Determine max # descriptors */

#else

	limit.rlim_cur = limit.rlim_max = (rlim_t) 1024;

#endif
 
	
	avail_descs = limit.rlim_max - 2;
	
	mask = sigmask (SIGINT) |
		sigmask (SIGPIPE) | sigmask (SIGALRM) | sigmask (SIGTERM) |
		sigmask (SIGURG) | sigmask (SIGXCPU) | sigmask (SIGHUP);

	gettimeofday (&time_now, NULL);

	finished_booting = 1;

	unlink ("booting");
	unlink ("recovery");
	
	while (!shutd)
	{

		FD_ZERO (&readfds);
		FD_ZERO (&writefds);
		FD_ZERO (&exceptfds);
		FD_ZERO (&visedit_fds);
		FD_SET (s, &readfds);

		for (d = descriptor_list; d; d = d->next)
		{
			
			FD_SET (d->hSocketFD, &readfds);
			FD_SET (d->hSocketFD, &exceptfds);
			FD_SET (d->hSocketFD, &writefds);
			if (!IS_SET (d->edit_mode, MODE_DONE_EDITING))
				FD_SET (d->hSocketFD, &visedit_fds);
		}
		gettimeofday (&current_time, NULL);

		pulse_time = timediff (&current_time, &time_now);

		/* Compensate if we're not getting enough CPU */

		if (pulse_time.tv_sec || pulse_time.tv_usec > 25 * run_mult)
		{
			if (pulse_time.tv_sec > 2)
			{
				// was sprintf(comm,...)
				printf ("Insufficient CPU! %ld:%ld sec between slices",
					(long) pulse_time.tv_sec, (long) pulse_time.tv_usec);
				/* system_log (comm, true); */
			}
			pulse_time.tv_sec = 0;
			pulse_time.tv_usec = 1;
			time_now = current_time;
		}
		else
			pulse_time.tv_usec = 25 * run_mult - pulse_time.tv_usec;

		sigprocmask (SIG_SETMASK, (sigset_t *) & mask, 0);

		if (select (maxdesc + 1,
			&readfds, &writefds, &exceptfds, &null_time) < 0)
		{
			perror ("Select poll");
			return -1;
		}

		if (select (0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0,
			&pulse_time) < 0)
		{
			perror ("Select sleep");
			exit (1);
		}

		/* time_now is set to the exact time we expected our time slice.
		It may be slightly off.  However, it allows us to maintain a
		perfect 1/4 second pulse on average. */

		time_now.tv_usec += 25 * run_mult;
		if (time_now.tv_usec > 100 * run_mult)
		{
			time_now.tv_usec -= 100 * run_mult;
			time_now.tv_sec++;
		}

		sigprocmask (SIG_SETMASK, (sigset_t *) 0, 0);

		engine.set_abort_threshold_post_booting ();


		/* Drop connections */
		descriptor__drop_connections (&readfds, &writefds, &exceptfds);

		if (knockout)
		{
			cleanup_the_dead (0);
			knockout = 0;
		}

		/* New connections */

		if (FD_ISSET (s, &readfds))
			if (new_descriptor (s) < 0)
				perror ("New connection");

		for (point = descriptor_list; point; point = next_point)
		{
			next_point = point->next;
			if (FD_ISSET (point->hSocketFD, &readfds))
				if (process_input (point) < 0)
				{
					FD_CLR (point->hSocketFD, &readfds);
					FD_CLR (point->hSocketFD, &exceptfds);
					FD_CLR (point->hSocketFD, &writefds);
					close_socket (point);
				}
		}

		if (knockout)
		{
			cleanup_the_dead (0);
			knockout = 0;
		}

		/* process_commands; */

		for (point = descriptor_list; point; point = next_to_process)
		{
			next_to_process = point->next;

			if ((--(point->wait) <= 0) && get_from_q (&point->input, comm))
			{
				point->wait = 1;

				/* reset idle time */

				point->time_last_activity = mud_time;

				if (point->character && !point->character->deleted
					&& point->character->pc != NULL)
					point->character->pc->time_last_activity = mud_time;

				if (point->original && !point->original->deleted
					&& point->original->pc != NULL)
					point->original->pc->time_last_activity = time (0);

				point->prompt_mode = 1;

				if (point->proc != NULL)
					string_add (point, comm);
				else if (point->showstr_point)
					show_string (point, comm);
				else if (!point->connected)
				{
					if (point->showstr_point)
						show_string (point, comm);
					else if (point->character)
						command_interpreter (point->character, comm);
				}
				else
					nanny (point, comm);
			}
		}

		if (knockout)
		{
			cleanup_the_dead (0);
			knockout = 0;
		}

		for (point = descriptor_list; point; point = next_point)
		{
			next_point = point->next;

			if (FD_ISSET (point->hSocketFD, &writefds) && point->output.head)
			{
				if (process_output (point) < 0)
				{
					FD_CLR (point->hSocketFD, &readfds);
					FD_CLR (point->hSocketFD, &exceptfds);
					FD_CLR (point->hSocketFD, &writefds);
					close_socket (point);
				}
				else
					point->prompt_mode = 1;
			}
		}

		for (point = descriptor_list; point; point = next_point)
		{

			next_point = point->next;

			if (point->prompt_mode)
			{

				if (point->proc != NULL)
					write_to_descriptor (point,
					point->connected ==
					CON_CREATION ? "> " : "] ");

				else if (point->showstr_point)
					write_to_descriptor (point,
					"*** Press return to continue - 'q' to quit *** ");

				else if (!point->connected)
				{
					prompt.assign("");
					if (point->character->flags & FLAG_NOPROMPT)
					{
						if (point->character->flags & FLAG_WIZINVIS)
							prompt +="##";
					}
					else
					{
						prompt += '<';

						if (IS_NPC (point->character))
						{
							prompt += '=';
						}

						if (point->character->flags & FLAG_WIZINVIS)
						{
							prompt += "##";
						}

						if (point->character->flags & FLAG_ANON)
						{
							prompt += "##";
						}

						prompt += (point->character)->fatigue_bar();


					}
					prompt += "> ";
					write_to_descriptor (point, prompt.c_str ());
				}

				point->prompt_mode = 0;
			}
		}

		/* handle heartbeat stuff */
		/* Note: pulse now changes every 1/4 sec  */

		pulse++;

		//every RL second
		if (!(pulse % SECOND_PULSE)) 
		{
			second_affect_update ();
			if (pending_reboot)
				check_reboot ();
		}

		if (knockout)
		{
			cleanup_the_dead (0);
			knockout = 0;
		}

		
		//every RL second with 3/4 sec offset
		if (!((pulse + 3) % PULSE_DELAY)) 
			update_delays ();

		if (knockout)
		{
			cleanup_the_dead (0);
			knockout = 0;
		}
		
			// every RL second with 1/4 sec offset
			// pulse_smart_mobs = second_pulse
		if (!((pulse + 1) % PULSE_SMART_MOBS))
			mobile_routines (pulse);

		if (knockout)
		{
			cleanup_the_dead (0);
			knockout = 0;
		}

		// every 16 RL second with 2/16 sec offset
		if (!((pulse + 2) % UPDATE_PULSE)) 
			point_update ();

		if (knockout)
		{
			cleanup_the_dead (0);
			knockout = 0;
		}

		// every 10 RL second
		if (!(pulse % (10 * SECOND_PULSE))) 
			ten_second_update ();

		if (knockout)
		{
			cleanup_the_dead (0);
			knockout = 0;
		}

		// every 10 RL second with 5/4 sec offset
		if (!((pulse + 5) % (10 * SECOND_PULSE))) 
		{
			check_maintenance ();
		}

			//every 25 RL seconds = 1 IG minute - verified
		if (!(pulse % (SECOND_PULSE * 25)))
		{
			time_info.minute ++;
		}
		
		//every RL minute
		if (!(pulse % (SECOND_PULSE * 60))) 
		{
			check_idlers ();
			check_linkdead ();
			check_sitebans ();
			process_reviews ();
		}

		if (knockout)
		{
			cleanup_the_dead (0);
			knockout = 0;
		}

		
                
		// every RL minute with 5/4 sec offset
		//PC are saved
		if (!((pulse + 5) % (SECOND_PULSE * 60)))
		{
			autosave ();
		}
			// every 2 RL minute - verified at 2 minutes RL
		if (!(pulse % (120 * SECOND_PULSE)))
		{
			newbie_hints ();
		}
		
		// every 2 RL minute with 5/4 sec offset
		if (!((pulse + 5) % (PULSE_AUTOSAVE * 2)))
		{
			save_stayput_mobiles ();
		}
		
                // every 3 RL minute with 5/4 sec offset
		if (!((pulse + 5) % (PULSE_AUTOSAVE * 3)))
		{
			save_player_rooms ();
			save_dwelling_rooms();
			
		}
		
		// every 14 RL minute with 5/4 sec offset
		if (!((pulse + 5) % (PULSE_AUTOSAVE * 14)))
		{
			save_tracks ();
		}

		// every 14 RL minute with 5/4 sec offset
		if (!((pulse + 5) % (PULSE_AUTOSAVE * 14)))
		{
			save_banned_sites ();
		}
		
				
		// Every 30 RL minutes; make sure we keep the mysqlpp connection alive
		if ( !(pulse % (PULSES_PER_SEC * 30 * 60)) )
		{
			ping_mysqlplus();
		}
		

		// every 25 RL minute = every 1 IG hour
		if (!(pulse % (SECOND_PULSE * 60 * 25)))
		{
			
			int sales_time = time(0) - last_vnpc_sale;
			int sale_pulses = sales_time / (6 * 60 * 60 * 30); /* six hour pulse (1 game day) * 30 = 1 game month */
			
			if (sale_pulses > 0)
			{ /* do all the below */
				
				for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
				{
					tch = *tch_iterator;
					if (tch->deleted)
						continue;
					if (!IS_NPC (tch) || !IS_SET (tch->flags, FLAG_KEEPER))
						continue;
					
						//loop sale pulse amount of time
						// all these adjusted to monthly totals by multiplying by 30
					for (int i=0; i<sale_pulses; i++)
					{
						if (IS_SET(tch->room->room_flags, WEALTHY))
							purse = number (1200, 2400);
						else if (IS_SET(tch->room->room_flags, SCUM))
							purse = number (300, 750);
						else if (IS_SET(tch->room->room_flags, POOR))
							purse = number (300, 1500);
						else
							purse = number (600, 1800);
						while (purse > 0)
							purse -= vnpc_customer (tch, purse);
						refresh_colors (tch);
					}
				} // most recent iterator ^
				last_vnpc_sale = time(0);
				save_vnpc_timestamp(); // save timestamp to db;
			} // has more than one pulse to do
			
		} // sales pulse

		
		
		// every RL minute about 2.4 IG minutes
		if (time (0) >= next_minute_update)
		{
			rl_minute_affect_update ();
		}
		if (knockout)
		{
			cleanup_the_dead (0);
			knockout = 0;
		}

			// every IG hour (or 25 RL min)
		if (time (0) >= next_hour_update)
		{
			hourly_update ();
			update_room_tracks ();
			weather_and_time (1);
			hour_affect_update ();
			room_update ();
		}

		if (knockout)
		{
			cleanup_the_dead (0);
			knockout = 0;
		}

		// every 8 RL seconds with 2/8 second offset
		if (!((pulse + 2) % PULSE_VIOLENCE))
		{
			for (i = 0; i <= MAX_WEATHER_AREAS; i++)
			{
				if (weather_info[i].lightning)
					if (number (90, 150) < weather_info[i].clouds)
						send_outside ("A fork of lightning flashes in the sky.\n");
			}
		}

		// every 10 RL seconds with 1/4 second offset
		if (!((pulse + 1) % (10 * SECOND_PULSE)))
		{
			cleanup_the_dead (0);
		}

		// every 6 RL hours - total reset of pulse clock
		if (pulse > 86400)
		{
			pulse = 0;
		}

			//every pulse
		if (quarter_event_list)
			process_quarter_events ();

		tics++;			/* tics since last checkpoint signal */

	}

	return 0;
}

void
handle_sec_input (int clntSocket)
{
	FILE *fp;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char file[MAX_STRING_LENGTH]= { '\0' };
	int recvMsgSize;

	sprintf (file, "rpinet");
	*buf = '\0';

	if ((recvMsgSize =
		recvfrom (clntSocket, buf, MAX_STRING_LENGTH, 0, NULL, 0)) < 0)
		return;

	buf[recvMsgSize - 1] = '\0';

	if (!(fp = fopen (file, "w+")))
		return;
	fprintf (fp, "%s", buf);
	fclose (fp);

	close (clntSocket);
}


void
newbie_hints (void)
{
	DESCRIPTOR_DATA *d;
	NEWBIE_HINT *hint;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char *p;
	int i, limit, hintnum;
	extern NEWBIE_HINT *hint_list;

	for (hint = hint_list, limit = 1; hint; hint = hint->next, limit++)
		;
	limit--;

	for (d = descriptor_list; d; d = d->next)
	{
		if (!d->character)
			continue;
		if (IS_NPC (d->character))
			continue;
		if (!IS_SET (d->character->plr_flags, NEWBIE_HINTS))
			continue;
		hintnum = number (1, limit);
		for (hint = hint_list, i = 1; hint; hint = hint->next, i++)
		{
			if (i != hintnum)
				continue;
			reformat_string (hint->hint, &p);
			sprintf (buf, "\n#6%s#0", p);
			d->character->send_to_char (buf);
			free_mem (p);
			break;
		}
	}
}

void
unban_site (SITE_INFO * site)
{
	SITE_INFO *tmp_site = NULL;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (!site)
		return;
	if (!banned_site) //global value
		return;

	if (banned_site == site)
	{
		sprintf (buf, "The siteban has been lifted on %s.\n",
			banned_site->name);
		send_to_gods (buf);
		banned_site = site->next;
	}
	else
	{
		for (tmp_site = banned_site; tmp_site; tmp_site = tmp_site->next)
		{
			if (tmp_site->next == site)
			{
				sprintf (buf, "The siteban has been lifted on %s.\n",
					tmp_site->next->name);
				send_to_gods (buf);
				system_log (buf, false);
				tmp_site->next = site->next;
				continue;
			}
		}

		free_mem (site->name);
		free_mem (site->banned_by);
		free_mem (site);
		site = NULL;

	}


	save_banned_sites ();
}

void
check_sitebans ()
{
	SITE_INFO *site = NULL;
	SITE_INFO *next_site = NULL;


	if (!banned_site)
		return;

	if (banned_site)
	{
		if (banned_site->banned_until != -1
			&& time (0) >= banned_site->banned_until)
		{
			unban_site (banned_site);
		}
	}

	for (site = banned_site; site; site = next_site)
	{
		next_site = site->next;
		if (site->next)
		{
			if (site->next->banned_until == -1)
				continue;

			if (site->next->banned_until != -1
				&& time (0) >= site->next->banned_until)
			{
				if (site->next->next) //is there another site to look at
				{
					unban_site (site->next);
					continue;
				}
				else
				{
					unban_site (site->next); //last site
					return;
				}
			}
		}
	}
}

void
check_maintenance (void)
{
	FILE *fp = 0;
	
	if ((fp = fopen (".mass_emailing", "r")))
	{
		send_to_gods ("...mass-emailing completed.\n");
		fclose (fp);
		unlink (".mass_emailing");
	}
	
	
	if ((fp = fopen ("maintenance_lock", "r")))
	{
		if (!maintenance_lock)
			send_to_all ("#2The server is now locked down for maintenance.#0\n");
		maintenance_lock = true;
		fclose (fp);
	}
	else
	{
		if (maintenance_lock)
			send_to_all ("#2The server is now open for play.#0\n");
		maintenance_lock = false;
	}
	
}


void
send_to_not_char (const char *message, const CHAR_DATA *ch)

{

	DESCRIPTOR_DATA		*d;
	CHAR_DATA 		*tch;

	if ( !message )
		return;

	if ( !ch )
		return;

	for (tch = ch->room->people; tch; tch = tch->next_in_room)
	{

		d = tch->desc;
		if ( !d && IS_NPC (tch) )
			continue;

		/* Check to see if real PC owner is still online */

		if ( !d && tch->pc && tch->pc->owner )
			for ( d = descriptor_list; d; d = d->next )
				if ( d == tch->pc->owner )
					break;

		if ( !d )
			continue;

		if ( d->character && IS_SET (d->character->action, PLR_QUIET) )
			continue;

		if ( tch == ch)
			continue;

		write_to_q (message, &d->output);
	}

}

void
send_to_char (const char *message, const CHAR_DATA * ch)
{
	DESCRIPTOR_DATA *d;

	if (!message)
		return;

	if (!ch)
		return;

	d = ch->desc;

	if (!d && IS_NPC (ch))
		return;

	/* Check to see if real PC owner is still online */

	if (!d && ch->pc && ch->pc->owner)
		for (d = descriptor_list; d; d = d->next)
			if (d == ch->pc->owner)
				break;

	if (!d)
		return;

	if (d->character && IS_SET (d->character->action, PLR_QUIET))
		return;

	write_to_q (message, &d->output);
}

void
send_to_all_unf (char *messg)
{
	DESCRIPTOR_DATA *i;

	if (messg)
		for (i = descriptor_list; i; i = i->next)
			if (!i->connected)
				write_to_q (messg, &i->output);
}

void
send_to_all (char *messg)
{
	DESCRIPTOR_DATA *i;
	char *formatted;

	if (!messg || !*messg)
		return;

	reformat_string (messg, &formatted);

	if (messg)
		for (i = descriptor_list; i; i = i->next)
			if (!i->connected)
				write_to_q (formatted, &i->output);

	free_mem (formatted);
}

void
send_to_guides (char *message)
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char *formatted;

	if (!message || !*message)
		return;

	sprintf (buf, "\n#6[Guide]#0 %s", message);

	reformat_string (buf, &formatted);

	for (d = descriptor_list; d; d = d->next)
		if (!d->connected &&
			IS_GUIDE (d->character) && !IS_SET (d->character->action, PLR_QUIET))
			write_to_q (formatted, &d->output);
	free_mem (formatted);
}

void
send_to_gods (const char *message)
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char *formatted;

	if (!message || !*message)
		return;

	sprintf (buf, "\n#2[System Message]#0 %s", message);

	reformat_string (buf, &formatted);

	for (d = descriptor_list; d; d = d->next)
		if (!d->connected
			&& (d->character)->get_trust() 
			&& !IS_SET (d->character->action, PLR_QUIET))
			write_to_q (formatted, &d->output);
	free_mem (formatted);
}

void
send_to_imps (char *message)
{
	FILE *fimp_message;
	int message_time = 0;
	
	if (!message || !*message)
		return;
	
	if ((fimp_message = fopen ("implementor_messages", "a")) == NULL)
		return;

	message_time = time(0);
	
	fprintf (fimp_message, "%d -- %s\n", message_time, message);

	fclose(fimp_message);
	return;
}

void
send_to_guardians (char *message, unsigned short int flag)
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char *formatted;

	if (!message || !*message)
		return;

	sprintf (buf, "\n%s", message);

	reformat_string (buf, &formatted);

	system_log (message, false);
	for (d = descriptor_list; d; d = d->next)
	{

		if (!d->connected 
			&& (d->character)->get_trust()
			&& !IS_SET (d->character->action, PLR_QUIET)
			&& (flag & d->character->guardian_mode))
		{
			write_to_q (formatted, &d->output);
		}

	}
	free_mem (formatted);
}

void
send_outside (char *message)
{
	DESCRIPTOR_DATA *d;
	char *formatted;

	if (!message || !*message)
		return;

	reformat_string (message, &formatted);

	for (d = descriptor_list; d; d = d->next)
	{

		if (d->connected || !d->character || !d->character->room)
			continue;

		if (IS_OUTSIDE (d->character) &&
			!IS_SET (d->character->action, PLR_QUIET) && d->character->is_awake())
			write_to_q (formatted, &d->output);
	}
	free_mem (formatted);
}

void
send_outside_zone (char *message, int zone)
{
	DESCRIPTOR_DATA *d;
	char *formatted;

	if (!message || !*message)
		return;

	if (zone == 1)
	{
		send_outside_zone (message, 3);
		send_outside_zone (message, 8);
		send_outside_zone (message, 11);
	}

	if (zone == 10)
	{
		send_outside_zone (message, 70);
		send_outside_zone (message, 71);
		send_outside_zone (message, 72);
		send_outside_zone (message, 73);
		send_outside_zone (message, 74);
		send_outside_zone (message, 75);
		send_outside_zone (message, 76);
		send_outside_zone (message, 77);
		send_outside_zone (message, 78);
		send_outside_zone (message, 79);
	}

	if (zone == 64)
	{
		send_outside_zone (message, 65);
		send_outside_zone (message, 66);
	}

	if (zone == 2)
	{
		send_outside_zone (message, 15);
	}

	reformat_string (message, &formatted);

	for (d = descriptor_list; d; d = d->next)
	{

		if (d->connected || !d->character || !d->character->room)
			continue;

		if (d->character->room->zone != zone)
			continue;

		if (IS_OUTSIDE (d->character) &&
			!IS_SET (d->character->action, PLR_QUIET) && d->character->is_awake())
			write_to_q (formatted, &d->output);
	}

	free_mem (formatted);
}

void
send_to_room (char *message, int room_num)
{
	CHAR_DATA *tch;
	ROOM_DATA *room;
	char *formatted;

	if (!message || !*message)
		return;

	if (!(room = vtor (room_num)))
		return;

	reformat_string (message, &formatted);

	for (tch = room->people; tch; tch = tch->next_in_room)
		if (tch->desc && !IS_SET (tch->action, PLR_QUIET))
			write_to_q (formatted, &tch->desc->output);

	free_mem (formatted);
}

void
send_to_room_unf (char *message, int room_num)
{
	CHAR_DATA *tch;
	ROOM_DATA *room;

	if (!message || !*message)
		return;

	if (!(room = vtor (room_num)))
		return;

	for (tch = room->people; tch; tch = tch->next_in_room)
		if (tch->desc && !IS_SET (tch->action, PLR_QUIET))
			write_to_q (message, &tch->desc->output);
}

/* higher-level communication */


extern int bytes_allocated;
extern int first_free;
extern int mud_memory;

void
do_gstat (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int uc = 0;
	int con = 0;
	int days;
	int hours;
	int minutes;
	int crashdays, crashminutes, crashhours;
	long uptime, ctime;
	char *tmstr;
	DESCRIPTOR_DATA *e;

	for (e = descriptor_list; e; e = e->next)
		if (!e->connected || e->acct)
			uc++;
		else
			con++;

	tmstr = (char *) asctime (localtime (&starttime));
	*(tmstr + strlen (tmstr) - 1) = '\0';

	uptime = time (0) - starttime;
	ctime = time (0) - crashtime;
	days = uptime / 86400;
	hours = (uptime / 3600) % 24;
	minutes = (uptime / 60) % 60;
	crashdays = ctime / 86400;
	crashhours = (ctime / 3600) % 24;
	crashminutes = (ctime / 60) % 60;

	ch->send_to_char ("\n#6Current Game Statistics#0\n");
	ch->send_to_char ("#6-----------------------#0\n");

	// Collects the latest Subversion Revision
#ifdef SVN_REV
	sprintf (buf, "#2Subversion Revision:            #0%s\n", SVN_REV);
	ch->send_to_char (buf);
	std::string svn_url = "$HeadURL$";
	svn_url.erase (0,38);
	svn_url.erase (svn_url.rfind ("/pp/src"));
	sprintf (buf, "#2Subversion URL:                 #0%s\n", svn_url.c_str ());
	ch->send_to_char (buf);
#endif
	sprintf (buf, "#2Connected Descriptors:          #0%d\n", uc);
	ch->send_to_char (buf);
	sprintf (buf, "#2Connections Since Boot:         #0%d\n",
		nTotalConnections);
	ch->send_to_char (buf);
	sprintf (buf, "#2Guest Logins Since Boot:        #0%d\n", guest_conns);
	ch->send_to_char (buf);
	sprintf (buf, "#2New Accounts Since Boot:        #0%d\n", new_accounts);
	ch->send_to_char (buf);
	sprintf (buf, "#2Mother Descriptor:              #0%d\n", nMainSocket);
	ch->send_to_char (buf);
	sprintf (buf, "#2Descriptors Pending:            #0%d\n", con);
	ch->send_to_char (buf);
	sprintf (buf, "#2Free Descriptors:               #0%d\n",
		avail_descs - maxdesc);
	ch->send_to_char (buf);
	sprintf (buf, "#2Shell Process ID:               #0%d\n", getpid ());
	ch->send_to_char (buf);
	sprintf (buf, "#2MySQL Database Host:            #0%s\n",
		mysql_get_host_info (database));
	ch->send_to_char (buf);
	sprintf (buf, "#2MySQL Server Version:           #0%s\n",
		mysql_get_server_info (database));
	ch->send_to_char (buf);

	sprintf (buf, "#2Running on Port:                #0%d\n",
		engine.get_port ());
	ch->send_to_char (buf);
	sprintf (buf, "#2Last Reboot By:                 #0%s",
		((BOOT[0]) ? BOOT : "Startup Script\n"));
	ch->send_to_char (buf);
	sprintf (buf, "#2Last Reboot Time:               #0%s\n", tmstr);
	ch->send_to_char (buf);
	sprintf (buf, "#2Time Spent on Last Boot:        #0%ld seconds\n",
		starttime - bootstart);
	ch->send_to_char (buf);
	sprintf (buf,
		"#2Current Uptime:                 #0%d day%s %d hour%s %d minute%s\n",
		days, ((days == 1) ? "" : "s"), hours, ((hours == 1) ? "" : "s"),
		minutes, ((minutes == 1) ? "" : "s"));
	ch->send_to_char (buf);
	if (crashtime)
	{
		sprintf (buf,
			"#2Last Crashed:                   #0%d day%s %d hour%s %d minute%s ago\n",
			crashdays, ((crashdays == 1) ? "" : "s"), crashhours,
			((crashhours == 1) ? "" : "s"), crashminutes,
			((crashminutes == 1) ? "" : "s"));
		ch->send_to_char (buf);
	}
	sprintf (buf, "#2Last PC Coldload:               #0%d\n",
		next_pc_coldload_id);
	ch->send_to_char (buf);
	sprintf (buf, "#2Last Mobile Coldload:           #0%d\n",
		next_mob_coldload_id);
	ch->send_to_char (buf);
	sprintf (buf, "#2Last Object Coldload:           #0%d\n",
			 next_obj_coldload_id);
	ch->send_to_char (buf);
	sprintf (buf, "#2Binary build date:              #0%s\n",__DATE__);
	ch->send_to_char (buf);
	ch->send_to_char ("#6-----------------------#0\n");
}


// Translates substrings in the form #n to ANSI Color
// For Reference: #define ANSI_AUTO_DETECT	"\x1B[6n"
char *
colorize (const char *source, char *target, struct descriptor_data *d)
{
	const char * colors [] = {
		"\x1B[0m",		// #0 color reset
		"\x1B[31m",		// #1 red
		"\x1B[32m",		// #2 green
		"\x1B[33m",		// #3 yellow
		"\x1B[34m",		// #4 blue
		"\x1B[35m",		// #5 magenta
		"\x1B[36m",		// #6 cyan
		"\x1B[37m",		// #7 white
		"\x1B[0m",		// #8 color reset
		"\x1B[1;31m",	// #9 bold, red
		"\x1B[1;32m",	// #A bold, green
		"\x1B[1;33m",	// #B bold, yellow
		"\x1B[1;34m",	// #C bold, blue
		"\x1B[1;35m",	// #D bold, magenta
		"\x1B[1;36m",	// #E bold, cyan
		"\x1B[1;37m"	// #F bold, white
	};

	char *retval = target;
	bool is_color_link = (d && ((d->character && d->character->color)
		|| (d->color)));

	*target = '\0';
	while (*source != '\0')
	{
		if (*source == '#')
		{
			++source;
			int escaped = *source;

			if (isxdigit(escaped))
			{
				// Reduce escaped hex code to its integer equivalent
				if (escaped <= '9')
				{
					escaped -= '0';
				}
				else
				{
					if (escaped <= 'F')
					{
						escaped -= 'A';
					}
					else
					{
						escaped -= 'a';
					}
					escaped += 10;
				}

				if (is_color_link)
				{
					strcpy (target, colors [escaped]);
					target = &target[strlen (target)];
				}

				source++;
			}
			else if (escaped == '#') // print #
			{
				*target++ = *source++;
			}
			else // no substitution
			{
				*target++ = '#';
				*target++ = *source++;
			}
		}
		else
			*target++ = *source++;
	}

	*target = '\0';
	return retval;
}

void
sigusr1 (int signo)
{
	system_log ("SIGUSR1 received: running cleanup_the_dead().", false);
	cleanup_the_dead (0);
}

void
signal_setup (void)
{
	signal (SIGUSR2, shutdown_request);
	signal (SIGUSR1, sigusr1);
	signal (SIGPIPE, SIG_IGN);
	signal (SIGALRM, logsig);
	signal (SIGSEGV, sigsegv);
	signal (SIGCHLD, sigchld);

}

void
reset_itimer ()
{
	struct itimerval itimer;
	itimer.it_interval.tv_usec = 0;	/* miliseconds */
	itimer.it_interval.tv_sec = rpie::server::ALARM_FREQUENCY;
	itimer.it_value.tv_usec = 0;
	itimer.it_value.tv_sec = rpie::server::ALARM_FREQUENCY;

	if (setitimer (ITIMER_VIRTUAL, &itimer, NULL) < 0)
	{
		perror ("reset_itimer:setitimer");
		abort ();
	}
}

const char *szFrozenMessage =
"Alarm_handler: Not checkpointed recently, aborting!\n";

void
alarm_handler (int signo)
{
	if (engine.loop_detect ())
	{
		send_to_gods
			("Infinite loop detected - attempting recovery via reboot...");
		system_log ("Loop detected - attempting copyover to recover.", true);
		shutdown_request (SIGUSR2);
	}
}

void
init_alarm_handler ()
{
	struct sigaction sa;

	sa.sa_handler = alarm_handler;
	sa.sa_flags = SA_RESTART;	/* Restart interrupted system calls */
	sigemptyset (&sa.sa_mask);

	if (sigaction (SIGVTALRM, &sa, NULL) < 0)
	{
		perror ("init_alarm_handler:sigaction");
		abort ();
	}
	engine.set_abort_threshold_pre_booting ();
	reset_itimer ();		/* start timer */
}

void
checkpointing (int signo)
{
	extern int tics;

	system_log ("Checkpointing...", false);

	if (signo == SIGVTALRM)
		signal (SIGVTALRM, checkpointing);	/* Guess we have to rearm */

	if (!tics)
	{
		system_log ("Checkpoint shutdown - tics not updated!", true);
		sigsegv (SIGSEGV);
	}
	else
		tics = 0;
}

void
save_door_state (void)
{
	ROOM_DATA *room;
	std::map<int, ROOM_DATA*>::iterator room_iterator;
	int i;

	for (room_iterator = room_map.begin(); room_iterator != room_map.end(); room_iterator++)
	{
		room = room_iterator->second;
		
		for (i = 0; i <= LAST_DIR; i++)
		{
			if (!room->dir_option[i])
				continue;
			if (!IS_SET (room->dir_option[i]->port_flags, EX_ISDOOR)
				&& !IS_SET (room->dir_option[i]->port_flags, EX_ISGATE))
				continue;

			/**
			0 - unlocked_and_open,
			1 - unlocked_and_closed,
			2 - locked_and_closed
			3 - gate_unlocked_and_open,
			4 - gate_unlocked_and_closed,
			5 - gate_locked_and_closed
			**/

			if (IS_SET (room->dir_option[i]->port_flags, EX_CLOSED)
				&& IS_SET (room->dir_option[i]->port_flags, EX_LOCKED)
				&& IS_SET (room->dir_option[i]->port_flags, EX_ISGATE))
				mysql_safe_query ("INSERT INTO copyover_doors VALUES (%d, %d, 5)",
				room->nVirtual, i);

			else if (IS_SET (room->dir_option[i]->port_flags, EX_CLOSED)
				&& IS_SET (room->dir_option[i]->port_flags, EX_ISGATE))
				mysql_safe_query ("INSERT INTO copyover_doors VALUES (%d, %d, 4)",
				room->nVirtual, i);

			else if (IS_SET (room->dir_option[i]->port_flags, EX_ISGATE))
				mysql_safe_query ("INSERT INTO copyover_doors VALUES (%d, %d, 3)",
				room->nVirtual, i);

			else if (IS_SET (room->dir_option[i]->port_flags, EX_CLOSED) && IS_SET (room->dir_option[i]->port_flags, EX_LOCKED))
				mysql_safe_query ("INSERT INTO copyover_doors VALUES (%d, %d, 2)",
				room->nVirtual, i);
			else if (IS_SET (room->dir_option[i]->port_flags, EX_CLOSED))
				mysql_safe_query ("INSERT INTO copyover_doors VALUES (%d, %d, 1)",
				room->nVirtual, i);
			else
				mysql_safe_query ("INSERT INTO copyover_doors VALUES (%d, %d, 0)",
				room->nVirtual, i);
		}
	}
}

void
prepare_copyover (int cmd)
{
	DESCRIPTOR_DATA *d, *d_next;
	AFFECTED_TYPE *af;
	FILE *fp;
	OBJ_DATA *obj;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	bool obj_recorded = false;
	int count;

	system_log ("Entering prepare_copyover() to set up the reboot.", false);

	fp = fopen (".copyover_data", "w");

	mysql_safe_query ("DELETE FROM players_online");

	// Prepare the soft reboot file for use after server bootup.

	for (d = descriptor_list; d; d = d_next)
	{
		d_next = d->next;

		if (!d->acct || !d->character || d->connected != CON_PLYNG
			|| (d->character && d->character->in_room == NOWHERE))
		{
			if (d->character)
			{
				if (d->connected == CON_PLYNG)
					d->character->unload_pc();
				d->character = NULL;
			}
			write_to_descriptor (d,
				"\n\n#6We are rebooting the server - come back in a moment!#0\n\n");
			close_socket (d);
			continue;
		}
		if (d->character && d->original)
		{
			mysql_safe_query ("INSERT INTO copyover_animations VALUES (%d, %d)",
				d->original->coldload_id,
				d->character->coldload_id);
			do_return (d->character, "", 0);
		}
		if (d->character)
		{
			if (cmd == 1)
				write_to_descriptor (d,
				"\n#1The server is recovering from a crash. Please wait...#0\n\n");
			else
				write_to_descriptor (d,
				"\n#2The server is rebooting. Please wait...#0\n\n");
		}
		if (d->character 
			&& d->connected == CON_PLYNG 
			&& d->character->room
			&& d->character->in_room != NOWHERE)
		{
			
			fprintf (fp, "%d %s %s %s %d", d->hSocketFD, d->strClientHostname,
					 d->acct->name.c_str (), d->character->name,
					 (d->character)->get_position());
			
			count = 0;
			if ((d->character)->get_position() == SIT
				&& (af = get_affect (d->character, MAGIC_SIT_TABLE)))
			{
				if (af->a.table.obj
					&& af->a.table.obj->in_room == d->character->in_room)
				{
					for (obj = d->character->room->contents; obj;
						 obj = obj->next_content)
					{
						if (str_cmp (af->a.table.obj->name, obj->name))
							continue;
						else
						{
							count++;
							if (obj == af->a.table.obj)
							{
								fprintf (fp, " %d%s~\n", count,
										 af->a.table.obj->name);
								obj_recorded = true;
								break;
							}
						}
					}
				}
				remove_affect_type (d->character, MAGIC_SIT_TABLE);
			}
			if (!obj_recorded)
				fprintf (fp, " 0none~\n");
			
			save_char (d->character, true);
		}
		obj_recorded = false;
	}

	fprintf (fp, "-1\n");
	fclose (fp);

	save_player_rooms ();
	save_dwelling_rooms ();
	save_stayput_mobiles ();
	save_tracks ();
	save_banned_sites ();
	save_door_state ();
	

	sprintf (buf, "%d", engine.get_port ());
	sprintf (buf2, "%d", nMainSocket);
	chdir ("..");
	execl ("bin/server", "bin/server", buf, "-c", buf2, (char *) NULL);
	system_log ("execl() failed!", true);
	chdir ("lib");

	send_to_gods
		("Reboot execl() failed. Aborting and resuming normal operation... please do not attempt again.");
}

void
copyover_recovery (void)
{
	DESCRIPTOR_DATA *d, *td;
	FILE *fp;
	OBJ_DATA *obj;
	AFFECTED_TYPE *af;
	char name[MAX_STRING_LENGTH]= { '\0' };
	char table_name[MAX_STRING_LENGTH]= { '\0' };
	char host[MAX_STRING_LENGTH]= { '\0' };
	char account_name[MAX_STRING_LENGTH]= { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int desc, online, guest;
	time_t current_time;
	int i = 0, conn, pos = 0, count = 0;
	
	system_log ("Soft reboot recovery sequence initiated.", false);
	
	if (!(fp = fopen (".copyover_data", "r")))
	{
		system_log
		("Copyover file not found! Aborting copyover and shutting down.",
		 true);
		abort ();
	}
	
	descriptor_list = NULL;
	
	for (;;)
	{
		conn = 0;
		
		fscanf (fp, "%d %s %s %s %d %d", &desc, host, account_name, name, &pos,
				&count);
		
		if (desc == -1)
		{
			break;
		}
		
		d = new descriptor_data;
		init_descriptor (d, desc);
		
		/*		descriptor_list = d; */
		
		if (!descriptor_list)
			descriptor_list = d;
		else
			for (td = descriptor_list; td; td = td->next)
			{
				if (!td->next)
				{
					d->next = NULL;
					td->next = d;
					break;
				}
			}
		
		if (d->hSocketFD > maxdesc)
			maxdesc = d->hSocketFD;
		
		d->strClientHostname = duplicateString (host);
		
		d->acct = new account (account_name);
		
		if (!strstr (name, "Guest"))
			d->character = load_pc (name);
		else
			create_guest_avatar (d, "recreate");
		
		if (!d->character)
			continue;
		
		d->character->desc = d;
		d->prompt_mode = 1;
		d->connected = CON_SOFT_REBOOT;
		
		d->character->pc->owner = d;
		d->character->pc->last_connect = time (0);
		
		if (!IS_SET (d->character->flags, FLAG_GUEST) && d->acct
			&& d->acct->color)
			d->character->color++;
		
		if (IS_SET (d->character->flags, FLAG_GUEST))
			d->character->color++;
		
		if (d->character->race == 86)//olog-hai - PC
		{
			d->character->armor = 3;
		}
		
		if (d->character->race == 28) //trolls - NPC
		{
			d->character->armor = 2;
		}
		
		d->character->flags &= ~(FLAG_ENTERING | FLAG_LEAVING);
		
		d->character->pc_to_game();
		
		if (!IS_SET (d->character->flags, FLAG_GUEST))
		{
			load_char_objs (d->character, (d->character)->name);
			
			if (!d->character->in_room || d->character->in_room == NOWHERE)
				(d->character)->char_to_room (OOC_LOUNGE);
			else
				(d->character)->char_to_room (d->character->in_room);
				
			
			
			sprintf (buf, "save/player/%c/%s.a",
					 tolower (*(d->character)->name),
					 CAP ((d->character)->name));
			if (!IS_SET (d->character->flags, FLAG_GUEST))
				load_saved_mobiles (d->character, buf);
		}
		
		online = 0;
		guest = 0;
		
		for (td = descriptor_list; td; td = td->next)
		{
			if (!td->character)
				continue;
			if (td->character->pc->level)
				continue;
			if (td->character->pc->create_state != 2)
				continue;
			if (td->connected)
				continue;
			if (IS_SET (td->character->flags, FLAG_GUEST))
			{
				guest++;
			}
			if ((!(td->character)->get_trust())
				&& !IS_SET (td->character->flags, FLAG_GUEST))
			{
				online++;
			}
		}
		
		if (online >= count_max_online)
		{
			count_max_online = online;
			current_time = time (0);
			ctime_r (&current_time, max_online_date);
			max_online_date[strlen (max_online_date) - 1] = '\0';
		}
		
		if (guest >= count_guest_online)
		{
			count_guest_online = guest;
		}
		
		d->character->pc->last_logon = time (0);
		
		sprintf (table_name, "%s", fread_string (fp));
		
		if (str_cmp (table_name, "none"))
		{
			i = 0;
			for (obj = d->character->room->contents; obj;
				 obj = obj->next_content)
			{
				if (!str_cmp (table_name, obj->name))
				{
					i++;
					if (i == count)
					{
						if ((af = get_affect (d->character, MAGIC_SIT_TABLE)))
						{
							affect_remove (d->character, af);
						}
						
						table_add_affect (d->character, obj, MAGIC_SIT_TABLE);
						/**
						magic_add_affect (d->character, MAGIC_SIT_TABLE, -1, 0,
										  0, 0, 0);
						af = get_affect (d->character, MAGIC_SIT_TABLE);
						af->a.table.obj = obj;
						 **/
						break;
					}
				}
			}
		}
		
		(d->character)->set_position(pos);
		
		d->connected = CON_PLYNG;
		
	}
	
	fclose (fp);
	
	/**********
	mysql_safe_query ("SELECT room, direction, state "
					  "FROM copyover_doors");
	if ((result = mysql_store_result (database)))
	{
		if (mysql_num_rows (result))
		{
			while ((row = mysql_fetch_row (result)))
			{
				int room_number = strtol(row[0],0,10);
				int door_dir = strtol(row[1],0,10);
				exit_state door_state = (exit_state) strtol(row[2],0,10);
				
				set_door_state (room_number, door_dir, door_state);
			}
		}
		mysql_free_result (result);
	}
	else // result = 0
	{
		std::ostringstream error_message;
		error_message << "Error: " << std::endl
		<< __FILE__ << ':'
		<< __func__ << '('
		<< __LINE__ << "):" << std::endl
		<< mysql_error (database);
		send_to_gods ((error_message.str ()).c_str());
	}
	
	mysql_safe_query ("DELETE FROM copyover_doors");
	***********/
	
}

void
check_reboot (void)
{
	DESCRIPTOR_DATA *d;
	FILE *fp;
	AFFECTED_TYPE *af;

	bool block = false;

	for (d = descriptor_list; d; d = d->next)
	{
		if (!d->character)
			continue;
		if (!d->character->pc)
			continue;
		if (d->connected != CON_PLYNG && d->character->pc->nanny_state)
			block = true;
		if (d->character->pc->create_state == STATE_APPLYING)
			block = true;
		if (IS_SET (d->character->action, PLR_QUIET))
			block = true;
		

		/* stop if crafting */
		for (af = d->character->hour_affects; af; af = af->next)
		{
			if (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST
				&& af->a.craft->timer)
			{
				block = true;
			}
		}

		if (d->character->subdue && IS_NPC (d->character->subdue))
			block = true;
	}

	if (!block)
	{
		if (!(fp = fopen (".reboot", "w")))
		{
			system_log ("Error creating reboot file.", true);
			return;
		}
		fprintf (fp, "Reboot Queue\n");
		fclose (fp);
		unlink ("booting");
		prepare_copyover (0);
	}
}

void
shutdown_request (int signo)
{
	FILE *fp;

	system_log ("Received USR2 - reboot request.", false);
	send_to_gods ("#1Received USR2 - reboot request.#0\n");

	if (!finished_booting || (fp = fopen ("booting", "r")))
	{
		/* Rebooting already. */
		system_log ("Signal ignored - already in the middle of a reboot.",
			true);
		return;
	}

	if (!(fp = fopen (".reboot", "w")))
	{
		system_log ("Error opening reboot file - aborting!", true);
		abort ();
	}
	fprintf (fp, "System Reboot Signal\n");
	fclose (fp);

	prepare_copyover (0);
}

void
logsig (int signo)
{
	system_log ("Signal received. Ignoring.", true);
}

void
sigchld (int signo)
{
	pid_t pid;
	int stat;

	while ((pid = waitpid (-1, &stat, WNOHANG)) > 0);
	signal (SIGCHLD, sigchld);

	return;
}

bool
in_crash_loop (void)
{
	bool bInCrashLoop = false;
	FILE *fp;

	fp = fopen ("recovery", "r");
	if (fp)
	{
		system_log ("Crash loop detected. Shutting down and dumping core...",
			true);
		bInCrashLoop = true;
		fclose (fp);
	}
	return bInCrashLoop;
}

bool
set_reboot_file (void)
{
	bool bSetRebootFile = false;
	FILE *fp;

	fp = fopen (".reboot", "w");
	if (fp)
	{
		fprintf (fp, "Crash Recovery Script\n");
		bSetRebootFile = true;
		fclose (fp);
	}
	else
	{
		system_log ("Error opening reboot file in sigsegv()!", true);
	}
	return bSetRebootFile;
}

#define GDB_PATH "/usr/bin/gdb"
#define PERL_PATH "/usr/bin/perl"
#define PERL_GDB_FILTER "$x=0;while(<STDIN>){if($_=~/^#4/){$x=1;print \"---------------------------\\n\"} print \"$_\" if $x;if($_=~/in main .argc/){$x=0;}}"
void
gdbdump (char *strGdbCommandFile, char *strFilterScript)
{
	int nServerPid, nGdbPid, nFilterPid;
	int fdTmpStdin, fdTmpStdout, fdTmpStderr;
	int fdFilterOutput;
	int fdGdbToFilterPipe[2];
	char strServerPid[10];
	char strServerPath[AVG_STRING_LENGTH * 2] = "";
	char strOutfilePath[AVG_STRING_LENGTH * 2] = "";

	/* Get the pid of this server. */
	nServerPid = getpid ();
	sprintf (strServerPid, "%d", nServerPid);
	sprintf (strOutfilePath, "%s/crashes/gdb.%d",
		(engine.get_base_path ("test")).c_str (), nServerPid);
	sprintf (strServerPath, "%s/bin/server",
		(engine.get_base_path ()).c_str ());

	/* backup fds and redirect i/o for gdb */
	fdTmpStdin = dup (0);
	fdTmpStdout = dup (1);
	fdTmpStderr = dup (2);
	if (pipe (fdGdbToFilterPipe) == -1)
	{
		send_to_gods ("gdbdumb: pipe failed.");
		return;
	}
	dup2 (fdTmpStdin, 0);
	dup2 (fdGdbToFilterPipe[1], 1);
	dup2 (fdTmpStderr, 2);

	/* Create new process for "gdb" */
	nGdbPid = fork ();
	if (nGdbPid == -1)
	{
		system_log ("gdbdump: gdb fork failed.", true);
		return;
	}

	if (nGdbPid == 0)
	{

		/* childe - close unused fds and exec gdb */
		close (fdGdbToFilterPipe[0]);
		close (fdGdbToFilterPipe[1]);
		close (fdTmpStdin);
		close (fdTmpStdout);
		close (fdTmpStderr);

		execlp (GDB_PATH, "gdb", "-silent", "-x", strGdbCommandFile,
			strServerPath, strServerPid, NULL);
		system_log ("gdbdump: exec gdb", true);
		exit (2);
	}

	/* backup fds and redirect i/o for perl */
	dup2 (fdGdbToFilterPipe[0], 0);
	fdFilterOutput = creat (strOutfilePath, 0666);
	if (fdFilterOutput < 0)
	{
		system_log ("gdbdump: creat strOutfilePath failed", true);
		return;
	}
	dup2 (fdFilterOutput, 1);
	close (fdFilterOutput);
	dup2 (fdTmpStderr, 2);

	nFilterPid = fork ();
	if (nFilterPid == -1)
	{
		system_log ("gdbdump: perl fork failed", true);
	}

	if (nFilterPid == 0)
	{

		/* childe - close unused fds and exec perl filter */
		close (fdGdbToFilterPipe[0]);
		close (fdGdbToFilterPipe[1]);
		close (fdTmpStdin);
		close (fdTmpStdout);
		close (fdTmpStderr);

		execlp (PERL_PATH, "gdb", "-e", strFilterScript, NULL);
		system_log ("gdbdump: exec filter", true);
		exit (2);

	}

	/* clean-up: restore and close fds */
	dup2 (fdTmpStdin, 0);
	dup2 (fdTmpStdout, 1);
	dup2 (fdTmpStderr, 2);
	close (fdGdbToFilterPipe[0]);
	close (fdGdbToFilterPipe[1]);
	close (fdTmpStdin);
	close (fdTmpStdout);
	close (fdTmpStderr);

	/* wait on filter command */
	waitpid (nFilterPid, 0, WNOHANG);
}

void
sigsegv (int signo)
{
	int nLastCrash = 0, nServerPid = 0;
	char *p;
	FILE *fp;
	/*FILE          *fdCrashInfo;
	char         strCrashInfo [AVG_STRING_LENGTH];
	char         strCrashInfoPath [AVG_STRING_LENGTH * 2]; */
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };

	extern char last_command[];
	int port_num = engine.get_port ();

	nLastCrash = (int) time (0);
	system_log ("Game has crashed!", true);

	fp = fopen ("last_crash", "w+");
	if (fp)
	{
		fprintf (fp, "%d\n", nLastCrash);
		fclose (fp);
	}

	nServerPid = getpid ();

	if (!in_crash_loop ()
		&& set_reboot_file ()
		&& last_descriptor != NULL
		&& last_descriptor->connected == CON_PLYNG
		&& last_descriptor->character != NULL)
	{

		sprintf (buf,
			"Something in the last command you entered, "
			"'#6%s#0', has crashed the MUD. Although it may "
			"have been due to a random memory corruption, please "
			"try to use this command as sparingly as possible "
			"after the recovery, just in case. A core dump has "
			"been generated to assist debugging, and a coder "
			"will look into this issue as soon as possible. "
			"Thank you for your patience.", last_command);

		reformat_string (buf, &p);

		write_to_descriptor (last_descriptor, "\n#6--- ATTENTION ---#0\n");
		write_to_descriptor (last_descriptor, p);
		write_to_descriptor (last_descriptor, "#6-----------------#0\n");

		sprintf (buf,
			"%s [%d]: %s\n\nType \'debug %d\' on the shell to retrieve full stack dump.\n",
			last_descriptor->character->name,
			last_descriptor->character->in_room, last_command, nServerPid);

		sprintf (buf2, "Server Crash [%d: pid %d]", port_num, nServerPid);

		add_message (1, last_descriptor->character->name, -2,
			"Server", NULL, buf2, "", buf, 0);

		add_message (1, "Crashes", -5,
			last_descriptor->character->name, NULL, buf2, "", buf, 0);
	}

	// gdbdump(PATH_TO_TP "/src/gdbdump.ini", PERL_GDB_FILTER);
	sprintf (buf, "/bin/cp %s/bin/server %s/crashes/server-%d.%d",
		(engine.get_base_path ()).c_str (),
		(engine.get_base_path ("test")).c_str (),
		port_num, nServerPid);

	system (buf);
	abort ();
}

/* Add user input to the 'current' string (as defined by d->str) */
void
string_add (DESCRIPTOR_DATA * d, char *str)
{
	char *scan = NULL;
	int terminator = 0;
	char end_char;
	char *p;
	CALL_PROC *proc;
	char last_char = '\n';

	end_char = d->max_str == STR_ONE_LINE ? -1 : '@';

	/* Get rid of the \r's that may be included in the user input */

	/* determine if this is the terminal string, and truncate if so */

	for (scan = str; *scan; last_char = *scan++)
	{
		if ((terminator = (*scan == end_char && last_char == '\n')))
		{
			*scan = '\0';
			break;
		}

	}

	if (d->max_str == STR_ONE_LINE)
		terminator = 1;

	if (!d->descStr)
	{
		if (strlen (str) >= d->max_str)
		{
			d->character->send_to_char ("String too long - Truncated.\n");
			*(str + d->max_str - 1) = '\0';
			terminator = 1;
		}
		d->descStr = new char[strlen (str) + 3];
		strcpy (d->descStr, str);
	}

	else
	{
		if (strlen (str) + strlen (d->descStr) >= d->max_str)
		{
			d->character->send_to_char ("String too long. Last line skipped.\n");
			terminator = 1;
		}
		else
		{
			p = new char[strlen(d->descStr) + strlen(str) + 3];
			strcpy (p, d->descStr);
			strcat (p, str);

			free_mem(d->descStr);
			d->descStr = p;
		}
		if (d->pending_message != NULL)
		{
			d->pending_message->message = duplicateString(d->descStr);
		}
	}

	if (terminator)
	{
		if (d->character)
			d->character->action &= ~PLR_QUIET;

		if (d->proc)
		{
			proc = d->proc;
			d->proc = NULL;
			(proc) (d);
		}
		else if (d->connected == CON_CREATION 
				 || d->connected == CON_COMMENT
				 || d->connected == CON_FDESC)
			create_menu_options (d);

		free_mem(d->descStr);
		d->descStr=NULL;
	}

	else
		strcat (d->descStr, "\n");
}


void
page_string (DESCRIPTOR_DATA * d, const char *str)
{
	if (!d)
		return;

	if (d->showstr_head && *d->showstr_head)
		free_mem (d->showstr_head);

	d->showstr_head = duplicateString (str);
	d->showstr_point = d->showstr_head;

	show_string (d, "");
}

void
show_string (DESCRIPTOR_DATA * d, char *input)
{
	char buffer[MAX_STRING_LENGTH]= { '\0' };
	char buf[MAX_INPUT_LENGTH];
	char *scan;
	char *chk;
	int lines = 0;

	one_argument (input, buf);

	if (*buf)
	{

		if (d->showstr_head)
		{
			free_mem (d->showstr_head);
			d->showstr_head = NULL;
		}

		if (d->header)
			free_mem (d->header);

		d->header = NULL;
		d->showstr_point = NULL;

		return;
	}

	/* show a chunk */

	if (d->header)
	{
		SEND_TO_Q (d->header, d);
		lines++;
	}

	for (scan = buffer;; scan++, d->showstr_point++)
	{

		*scan = *d->showstr_point;

		if (*scan == '\n')
			lines++;

		else if (!*scan || lines >= 22)
		{
			*scan = '\0';

			/* see if this is the end (or near the end) of the string */

			for (chk = d->showstr_point; isspace (*chk); chk++)
				;

			if (!*chk)
			{

				if (d->showstr_head)
				{
					free_mem (d->showstr_head);
					d->showstr_head = NULL;
				}

				d->showstr_point = NULL;

				if (d->header)
					free_mem (d->header);

				d->header = NULL;
			}
			else
				SEND_TO_Q ("\n", d);

			SEND_TO_Q (buffer, d);

			return;
		}
	}
}

