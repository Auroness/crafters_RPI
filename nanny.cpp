//////////////////////////////////////////////////////////////////////////////
//
/// nanny.cpp : Login Menu and Chargen Module 
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
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/telnet.h>
#include </usr/local/mysql/include/mysql.h>
#include <sstream>

#include "server.h"
#include "net_link.h"
#include "structs.h"
#include "account.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "utility.h"

	//TODO: evalaute functionality of chargen. Simplify it for IG use maybe?
extern rpie::server engine;
extern std::map<std::string, SKILL_DATA*> skill_data_map;


char echo_off_str[4] = { (char) IAC, (char) WILL, (char) TELOPT_ECHO, '\0' };
char echo_on_str[6] = { (char) IAC, (char) WONT, (char) TELOPT_ECHO, '\r', '\n', '\0' };

#define ECHO_OFF		SEND_TO_Q (echo_off_str, d);
#define ECHO_ON			SEND_TO_Q (echo_on_str, d);

#define MAX_PC_LIMIT		100

int new_accounts = 0;

/* Check for duplicate passwords
select aa.username, aa.user_password
from users aa, users bb
where aa.username != bb.username AND aa.user_password = bb.user_password
group by aa.username, aa.user_password
order by user_password, username;

Logins per day:
select account,ip,firsttime,lasttime, count,has_pwd,count/datediff(lasttime,firsttime) as lpd
from ip
order by  lpd desc
limit 60;
*/

const int pregame_furnishings[5] = {
	125,
	1460,
	1753,
	121,
	-1  //used as a tag for last item
};




bool
is_banned (DESCRIPTOR_DATA * d)
{
	SITE_INFO *site = NULL;

	if (banned_site && banned_site->name && d && d->strClientHostname
		&& d->strClientIpAddr)
	{

		for (site = banned_site; site; site = site->next)
		{

			if (!str_cmp (banned_site->name, "*"))
			{
				return true;
			}

			if (site->name[0] == '^')
			{

				if (strncmp
					(d->strClientHostname, site->name + 1,
					strlen (site->name) - 1) == 0)
				{
					return true;
				}

				if (strncmp
					(d->strClientIpAddr, site->name + 1,
					strlen (site->name) - 1) == 0)
				{
					return true;
				}

			}
			else
			{

				if (strstr (d->strClientHostname, site->name))
				{
					return true;
				}

				if (strstr (d->strClientIpAddr, site->name))
				{
					return true;
				}

			}

		}

	}
	return false;
}


/*                                                                          *
* function: display_unread_messages                                        *
*                                                                          *
* 09/20/2004 [JWW] - Log a Warning on Failed MySql query                   *
*                  - changes mysql_use_result to mysql_store_result        *
*                                                                          */
void
display_unread_messages (DESCRIPTOR_DATA * d)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int unread = 0;

	if (!d->acct || d->acct->name.empty ())
		return;

	mysql_safe_query
		("SELECT flags FROM hobbitmail WHERE account = '%s' ORDER BY timestamp DESC",
		d->acct->name.c_str ());

	if ((result = mysql_store_result (database)) == NULL)
	{
		sprintf (buf, "Warning: display_unread_messages(): %s",
			mysql_error (database));
		system_log (buf, true);
		return;
	}

	while ((row = mysql_fetch_row (result)) != NULL)
	{
		if (!IS_SET (atoi (row[0]), MF_READ))
			unread++;
	}

	mysql_free_result (result);
	result = NULL;

	if (!unread)
		return;

	sprintf (buf,
		"#6There %s %d unread Hobbit-Mail%s awaiting your attention!#0\n\n",
		unread > 1 ? "are" : "is", unread, unread > 1 ? "s" : "");
	SEND_TO_Q (buf, d);
}

void
display_main_menu (DESCRIPTOR_DATA * d)
{
	read_motd(d);

	SEND_TO_Q (get_text_buffer (NULL, text_list, "menu1"), d);
	display_unread_messages (d);
	SEND_TO_Q ("Your Selection: ", d);
	d->connected = CON_ACCOUNT_MENU;
}

void
chargen_save(DESCRIPTOR_DATA * d)
{
	d->character->pc->create_state = STATE_APPLYING;
	save_char (d->character, false);
	d->character->unload_pc();
	d->character = NULL;
	display_main_menu (d);
}

char *
encrypt_buf (const char *buf)
{

	return duplicateString (crypt (buf, "CR"));
}

int
check_password (const char *pass, const char *encrypted)
{
	char *p;
	int return_value;
	
	p = encrypt_buf (pass);
	
	return_value = (strcmp (p, encrypted) == 0);

	free_mem (p); 

	return return_value;
}

void
nanny_login_choice (DESCRIPTOR_DATA * d, char *argument)
{
	bool bIsBanned = false;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (argument, buf);

	while (*buf && buf[strlen (buf) - 1] == ' ')
		buf[strlen (buf) - 1] = '\0';

	*buf = toupper (*buf);

	bIsBanned = is_banned (d);

	if (bIsBanned && *buf != 'L')
	{
		sprintf (buf, "\nYour site is currently banned from connecting to %s.\n"
			"As a result of the large number of people we get from certain Internet\n"
			"domains, this may not be a result of anything you've done; if you feel that\n"
			"this is the case, please email our staff at %s and we\n"
			"will do our best to resolve this issue. Our apologies for the inconvenience.\n",
			MUD_NAME, STAFF_EMAIL);
		SEND_TO_Q (buf, d);
		sprintf (buf, "\nPlease press ENTER to disconnect from the server.\n");
		SEND_TO_Q (buf, d);
		d->connected = CON_PENDING_DISC;
		return;
	}

	if (*buf != 'L' && *buf != 'X' && *buf != 'C')
	{
		SEND_TO_Q ("That is not a valid option, friend.\n", d);
		SEND_TO_Q ("Your Selection: ", d);
		return;
	}


	else if (*buf == 'L')
	{
		SEND_TO_Q ("Your account name: ", d);
		d->connected = CON_ENTER_ACCT_NME;
		return;
	}

	else if (*buf == 'C')
	{
		SEND_TO_Q ("Your new account name: ", d);
		d->connected = CON_NEW_ACCT_NAME;
		return;
	}
	
	else if (*buf == 'X')
	{
		close_socket (d);
		return;
	}
}

void
nanny_create_guest (DESCRIPTOR_DATA * d, char *argument)
{
	create_guest_avatar (d, argument);
}

void
nanny_ask_password (DESCRIPTOR_DATA * d, char *argument)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	bool bIsBanned = false;

	
	if (!*argument)
	{
		close_socket (d);
		delete d->acct;
		return;
	}

	
	
	if (strstr (argument, " "))
	{
		SEND_TO_Q ("\nThe account name cannot contain whitespace.\n", d);
		if (!maintenance_lock)
			SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
		else
			SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
			d);
		SEND_TO_Q ("Your Selection: ", d);
		d->connected = CON_LOGIN;
		return;
	}

	if (!isalpha (*argument))
	{
		SEND_TO_Q
			("\nThe first character of the account name MUST be a letter.\n", d);
		if (!maintenance_lock)
			SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
		else
			SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
			d);
		SEND_TO_Q ("Your Selection: ", d);
		d->connected = CON_LOGIN;
		return;
	}

	sprintf (buf, "%s", argument);

	for (size_t i = 0; i < strlen (buf); i++)
	{
		if (!isalpha (buf[i]) && !isdigit (buf[i]))
		{
			SEND_TO_Q
				("\nIllegal characters in account name (letters/numbers only).\n",
				d);
			if (!maintenance_lock)
				SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
			else
				SEND_TO_Q (get_text_buffer
				(NULL, text_list, "greetings.maintenance"), d);
			SEND_TO_Q ("Your Selection: ", d);
			d->connected = CON_LOGIN;
			return;
		}
	}

	if (str_cmp (CAP (argument), "Anonymous"))
	{
		d->acct = new account (argument);
	}

	if (!str_cmp (argument, "Anonymous") || !d->acct->is_registered ())
	{
		SEND_TO_Q ("\nNo such account. If you wish to create a new account,\n"
			"please choose option 'C' from the main menu.\n", d);
		if (!maintenance_lock)
			SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
		else
			SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
			d);
		SEND_TO_Q ("Your Selection: ", d);
		d->connected = CON_LOGIN;
		return;
	}

	/*
	*  WE HAVE AN ACCOUNT CONNECTION
	*     1st timer: count = 1, has_pwd = 0, logins = 0, fails = 0
	*     otherwise: has_pwd = 0, count++
	*/
	int port_num = engine.get_port ();
	if (d->acct && d->acct->is_registered () && d->acct->name.length ())
	{
		mysql_safe_query
			("INSERT INTO ip "
			"  VALUES('%s','%s','%s',NOW(),NOW(),1,0,0,%d,0,0) "
			"  ON DUPLICATE KEY "
			"  UPDATE lasttime = NOW(), "
			"    count = count + 1, has_pwd = 0, host = '%s';",
			d->acct->name.c_str (),
			d->strClientHostname, 
			d->strClientIpAddr,
			port_num, 
			d->strClientHostname);
	}

	bIsBanned = is_banned (d);

	if (bIsBanned && !IS_SET (d->acct->flags, ACCOUNT_NOBAN))
	{
		sprintf (buf, "\nYour site is currently banned from connecting to %s.\n"
			"As a result of the large number of people we get from certain Internet\n"
			"domains, this may not be a result of anything you've done; if you feel that\n"
			"this is the case, please email our staff at %s and we\n"
			"will do our best to resolve this issue. Our apologies for the inconvenience.\n",
			MUD_NAME, STAFF_EMAIL);
		SEND_TO_Q (buf, d);
		sprintf (buf, "\nPlease press ENTER to disconnect from the server.\n");
		SEND_TO_Q (buf, d);
		d->connected = CON_PENDING_DISC;
		return;
	}

	if (str_cmp(d->acct->last_ip.c_str (), d->strClientHostname) != 0)
		SEND_TO_Q
		("\nPlease enter your password carefully - your access to the game server will be\n"
		"suspended for a one-hour period if you repeatedly fail your logins!\n\n",
		d);

	SEND_TO_Q ("Password: ", d);

	ECHO_OFF;

	d->connected = CON_PWDCHK;

}

/*                                                                          *
* function: nanny_check_password        < e.g.> Password: *********        *
*                                                                          *
*                                                                          */
void
nanny_check_password (DESCRIPTOR_DATA * d, char *argument)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char buf3[MAX_STRING_LENGTH]= { '\0' };
	char strAccountSharer[AVG_STRING_LENGTH] = "";
	MYSQL_RES *result;
	MYSQL_ROW row = NULL;
	int nFailedLogins = 0, nSharedIP = 0;

	int port_num = engine.get_port ();
	if (!*argument)
	{
		close_socket (d);
		return;
	}
		//SPAM prevention 
	if (strlen(argument) > 64)
	{
		close_socket (d);
		return;
	}
	
	if (!check_password (argument, d->acct->password.c_str ()))
	{

		/*
		*  WE HAVE A LOGIN FAILURE
		*     1st timer: count = 1, has_pwd = 0, logins = 0, fails = 1 (this should never happen)
		*     otherwise: has_pwd = 0, fails++ (count already incremented)
		*/
		mysql_safe_query
			("INSERT INTO ip "
			"  VALUES('%s','%s','%s',NOW(),NOW(),1,0,0,%d,0,1) "
			"  ON DUPLICATE KEY "
			"  UPDATE lasttime = NOW(),fails = fails + 1,"
			"     has_pwd = 0,host = '%s';",
			d->acct->name.c_str (),
			d->strClientHostname, 
			d->strClientIpAddr,
			port_num, 
			d->strClientHostname);


		if (str_cmp (d->acct->last_ip.c_str (), d->strClientHostname) != 0)
		{
			mysql_safe_query
				("INSERT INTO failed_logins VALUES ('%s', '%s', UNIX_TIMESTAMP())",
				d->acct->name.c_str (), d->strClientHostname);
			mysql_safe_query
				("SELECT * FROM failed_logins WHERE hostname = '%s' AND timestamp >= (UNIX_TIMESTAMP() - 60*60)", d->strClientHostname);
			if ((result = mysql_store_result (database)) != NULL)
			{
				nFailedLogins = mysql_num_rows (result);
				mysql_free_result (result);
				result = NULL;
			}
			else
			{
				sprintf (buf, "Warning: nanny_check_password(): %s",
					mysql_error (database));
				system_log (buf, true);
				close_socket (d);	/* something bad happened during login... disconnect them (JWW) */
				return;
			}

			if (nFailedLogins >= 3
				&& !IS_SET (d->acct->flags, ACCOUNT_NOBAN))
			{
				if (d->acct && is_admin (d->acct->name.c_str ()))
				{
					sprintf (buf, "Staff Security Notificiation [%s]",
						d->acct->name.c_str ());
					sprintf (buf2,
						"The following host, %s, has been temporarily banned from the MUD server for incorrectly logging into your game account three times within one hour.\n\nUse the BAN command in-game to see which player accounts, if any, are affected by the ban; this should help you track down the perpetrator.",
						d->strClientHostname);
					send_email (d->acct, STAFF_EMAIL,
						"Account Security <" STAFF_EMAIL ">", buf,
						buf2);
				}
				else if (d->acct)
				{
					sprintf (buf, "Account Security Notification");
					sprintf (buf2,
						"Hello,\n\nA user has just been temporarily sitebanned from the MUD server for three failed login attempts to your game account, %s, originating from a foreign IP and occurring within the past hour.\n\nIf this individual was not you, please notify the staff by replying directly to this message; someone may be attempting to access your account illegally. Otherwise, you will regain your access privileges automatically one hour from the time of this email notification.\n\nThank you for playing!\n\n\n\nWarmest Regards,\nThe Admin Team",
						d->acct->name.c_str ());
					sprintf (buf3, "Account Security <%s>", STAFF_EMAIL);
					send_email (d->acct, NULL, buf3, buf, buf2);
				}
				SEND_TO_Q
					("\n\nYour access to this server has been suspended for one hour due to repeated\n"
					"incorrect password attempts. Please email the staff if you have any questions.\n\n"
					"Additionally, the registered owner of this account has been notified via email.\n\n",
					d);
				ban_host (d->strClientHostname, "Password Security System", -2);
				d->connected = CON_PENDING_DISC;
				return;
			}
		}
		SEND_TO_Q
			("\n\nIncorrect password - have you forgotten it? Please hobbitmail Admin Person or PM him at http://yourweb.site/forums/",
			d);
		d->acct->password_attempt++;
		return;
	}

	ECHO_ON;

	d->color = d->acct->color;

	if (!strstr (d->strClientHostname, "localhost"))
	{
		std::string escaped_ip;
		std::string escaped_name;

		d->acct->get_last_ip_sql_safe (escaped_ip);
		d->acct->get_name_sql_safe (escaped_name);

		std::ostringstream ipshare_query_stream;

		ipshare_query_stream <<
			"SELECT name FROM users "
			"WHERE user_last_ip != '(null)' "
			"AND user_last_ip != '' "
			"AND user_last_ip = '" << escaped_ip << "' "
			"AND name != '" << escaped_name << "'";

		std::string ipshare_query_string = ipshare_query_stream.str ();
		mysql_safe_query ((char *)ipshare_query_string.c_str ());

		if ((result = mysql_store_result (database)) != NULL)
		{
			nSharedIP = mysql_num_rows (result);

			if (!IS_SET (d->acct->flags, ACCOUNT_IPSHARER) && nSharedIP > 0
				&& str_cmp (d->acct->name.c_str (), "Guest") != 0
				&& str_cmp (d->acct->last_ip.c_str (), "(null)") != 0
				&& d->acct->last_ip.find ("localhost") == std::string::npos)
			{
				strcpy (strAccountSharer,
					"  #1Possible IP sharing detected with:");
				while ((row = mysql_fetch_row (result)))
				{
					strcat (strAccountSharer, " ");
					strcat (strAccountSharer, row[0]);
				}
				strcat (strAccountSharer, "#0");
			}

			mysql_free_result (result);
			result = NULL;
		}
		else
		{
			sprintf (buf, "Warning: nanny_check_password(): %s",
				mysql_error (database));
			system_log (buf, true);
		}
	}

	sprintf (buf, "%s [%s] has logged in.%s\n", d->acct->name.c_str (),
		d->strClientHostname,
		(nSharedIP) ? strAccountSharer : "");
	send_to_gods (buf);
	system_log (buf, false);

	/*
	*  WE HAVE A LOGIN
	*     1st timer: count = 1, has_pwd = 1, logins = 1 (this should never happen)
	*     otherwise: has_pwd = 1, logins++ (count already incremented)
	*/

	mysql_safe_query
		("INSERT INTO ip "
		"  VALUES('%s','%s','%s',NOW(),NOW(),1,0,1,%d,1,0) "
		"  ON DUPLICATE KEY UPDATE lasttime = NOW(), "
		"    logins = logins + 1, has_pwd = 1,host = '%s';",
		d->acct->name.c_str (),
		d->strClientHostname,
		d->strClientIpAddr, port_num,
		d->strClientHostname);

	d->acct->update_last_ip (d->strClientHostname);
	d->acct->password_attempt = 0;

	display_main_menu (d);
}

void
nanny_new_account (DESCRIPTOR_DATA * d, char *argument)
{
	account *acct = NULL;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };

	sprintf (buf, "%s", argument);

	if (!*buf)
	{
		close_socket (d);
		return;
	}

	if (strlen (buf) > 36)
	{
		SEND_TO_Q
			("\nPlease choose an account name of less than 36 characters.\n", d);
		if (!maintenance_lock)
			SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
		else
			SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
			d);
		SEND_TO_Q ("Your Selection: ", d);
		d->connected = CON_LOGIN;
		return;
	}

	if (strstr (buf, " "))
	{
		SEND_TO_Q ("\nThe account name cannot contain whitespace.\n", d);
		if (!maintenance_lock)
			SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
		else
			SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
			d);
		SEND_TO_Q ("Your Selection: ", d);
		d->connected = CON_LOGIN;
		return;
	}

	if (!isalpha (*buf))
	{
		SEND_TO_Q
			("\nThe first character of the account name MUST be a letter.\n", d);
		if (!maintenance_lock)
			SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
		else
			SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
			d);
		SEND_TO_Q ("Your Selection: ", d);
		d->connected = CON_LOGIN;
		return;
	}

	for (size_t i = 0; i < strlen (buf); i++)
	{
		if (!isalpha (buf[i]) && !isdigit (buf[i]))
		{
			SEND_TO_Q
				("\nIllegal characters in account name (letters/numbers only). Please hit ENTER.\n",
				d);
			if (!maintenance_lock)
				SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
			else
				SEND_TO_Q (get_text_buffer
				(NULL, text_list, "greetings.maintenance"), d);
			SEND_TO_Q ("Your Selection: ", d);
			d->connected = CON_LOGIN;
			return;
		}

		if (i)
			buf[i] = tolower (buf[i]);
		else
			buf[i] = toupper (buf[i]);
	}

	acct = new account (CAP (argument));
	if (str_cmp (CAP (argument), "Anonymous")
		&& !acct->is_registered ())
	{
		sprintf (buf2, "\nApply for a login account named %s? [y/n]  ", buf);
		SEND_TO_Q (buf2, d);
		d->connected = CON_ACCT_POLICIES;
		d->stored = duplicateString (buf);
	}
	else
	{
		SEND_TO_Q ("\nThat account name has already been taken.\n", d);
		if (!maintenance_lock)
			SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
		else
			SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
			d);
		SEND_TO_Q ("Your Selection: ", d);
		d->connected = CON_LOGIN;
	}

	delete acct;
	return;
}

void
nanny_account_policies (DESCRIPTOR_DATA * d, char *argument)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };

	argument = one_argument (argument, buf);

	*buf = toupper (*buf);

	if (!*buf)
	{
		close_socket (d);
		return;
	}

	if (*buf == 'Y')
	{
		d->acct = new account;
		d->acct->set_name (d->stored);
		d->stored = duplicateString ("");
		d->acct->created_on = time (0);
		SEND_TO_Q (get_text_buffer (NULL, text_list, "account_policies"), d);
		SEND_TO_Q ("Do you agree? (y/n) ", d);
		d->connected = CON_ACCT_EMAIL;
		return;
	}
	else
	{
		if (!maintenance_lock)
			SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
		else
			SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
			d);
		SEND_TO_Q ("Your Selection: ", d);
		d->connected = CON_LOGIN;
		return;
	}
}


void
nanny_account_email (DESCRIPTOR_DATA * d, char *argument)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char referrer[AVG_STRING_LENGTH];

	argument = one_argument (argument, buf);

	*buf = toupper (*buf);

	if (!*buf)
	{
		close_socket (d);
		return;
	}
	
}

                                                                     
void
nanny_account_email_confirm (DESCRIPTOR_DATA * d, char *argument)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	MYSQL_RES *result;
	int nUserEmailMatches = 0;

	if (!*argument)
	{
		close_socket (d);
		return;
	}

	if (strstr (argument, " ") || strstr (argument, ";")
		|| strstr (argument, "\\") || strstr (argument, "("))
	{
		SEND_TO_Q ("\n#1Your input contains illegal characters.#0\n", d);
		SEND_TO_Q ("Your email address: ", d);
		d->connected = CON_EMAIL_CONFIRM;
		return;
	}

	if (!strstr (argument, "@") || !strstr (argument, "."))
	{
		SEND_TO_Q
			("\nYour listed email address must include an '@' symbol and a dot.\n\n",
			d);
		SEND_TO_Q ("Your email address: ", d);
		d->connected = CON_EMAIL_CONFIRM;
		return;
	}
//changed for auroness version
	mysql_safe_query
		("SELECT email FROM users WHERE user_email = '%s' AND user_name != '%s'",
		argument, d->acct->name.c_str ());
	if ((result = mysql_store_result (database)) != NULL)
	{
		nUserEmailMatches = mysql_num_rows (result);
		mysql_free_result (result);
		result = NULL;
	}
	else
	{
		sprintf (buf, "Warning: nanny_account_email_confirm(): %s",
			mysql_error (database));
		system_log (buf, true);
	}

	if (nUserEmailMatches > 0)
	{
		/// \todo LOG THESE

		SEND_TO_Q
			("\nWe're sorry, but that email address has already been registered\n"
			"under an existing game account. Please choose another.\n\n", d);
		SEND_TO_Q ("Your email address: ", d);
		d->connected = CON_EMAIL_CONFIRM;
		return;
	}

	sprintf (buf, "Is the address %s correct? [y/n] ", argument);
	SEND_TO_Q (buf, d);

	d->acct->set_email (argument);
	d->connected = CON_ACCOUNT_SETUP;
	mysql_free_result (result);

	return;
}

void
nanny_new_password (DESCRIPTOR_DATA * d, char *argument)
{
	if (!*argument || strlen (argument) < 6 || strlen (argument) > 25)
	{
		SEND_TO_Q ("\n\nPasswords should be 6 to 25 characters in length.\n\n",
			d);
		SEND_TO_Q ("Password: ", d);
		return;
	}

	d->stored = encrypt_buf (argument);

	SEND_TO_Q ("\nPlease retype your new password: ", d);

	d->connected = CON_PWDNCNF;
}

void
nanny_change_password (DESCRIPTOR_DATA * d, char *argument)
{
	nanny_new_password (d, argument);

	d->connected = CON_PWDNCNF;
}

void
nanny_conf_change_password (DESCRIPTOR_DATA * d, char *argument)
{

	if (!check_password (argument, d->stored))
	{
		SEND_TO_Q ("\n\nPasswords didn't match.\n\n", d);
		SEND_TO_Q ("Retype password: ", d);
		d->connected = CON_PWDNEW;
		return;
	}

	ECHO_ON;

	d->acct->update_password (d->stored);
	free_mem (d->stored); // char*

	SEND_TO_Q ("\n\n#2Account password successfully modified.#0\n\n", d);

	display_main_menu (d);

}

void
nanny_disconnect (DESCRIPTOR_DATA * d, char *argument)
{
	if (d->character)
	{
		(d->character)->extract_char();
		d->character = NULL;
	}

	close_socket (d);
}


void
nanny_account_setup (DESCRIPTOR_DATA * d, char *argument)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (!*argument)
	{
		close_socket (d);
		return;
	}

	argument[0] = toupper (argument[0]);

	if (*argument == 'Y')
	{
		SEND_TO_Q (get_text_buffer (NULL, text_list, "thankyou"), d);
		d->acct->set_last_ip (d->strClientHostname);
		setup_new_account (d->acct);
		sprintf (buf, "New account: %s [%s].\n", d->acct->name.c_str (),
			d->strClientHostname);
		send_to_gods (buf);
		sprintf (buf, "New account: %s [%s].", d->acct->name.c_str (),
			d->strClientHostname);
		system_log (buf, false);

		/*
		*  WE HAVE AN ACCOUNT CREATED
		*     1st timer: count = 1, is_new = 1, has_pwd = 0, logins = 0, fails = 0
		*     otherwise: not possible!
		*/
				
		
		mysql_safe_query
			("INSERT INTO ip VALUES('%s','%s','%s', NOW(), NOW(), 1, 1, 0, 4500, 0, 0)",
			(d->acct->name.c_str(),
			d->strClientHostname,
			d->strClientIpAddr));


		SEND_TO_Q ("Press ENTER to disconnect from the server.\n", d);
		d->connected = CON_PENDING_DISC;
		new_accounts++;
	
		mysql_safe_query
			("UPDATE newsletter_stats SET new_accounts=new_accounts+1");
 
		return;
	}
	else
	{
		if (!maintenance_lock)
			SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings"), d);
		else
			SEND_TO_Q (get_text_buffer (NULL, text_list, "greetings.maintenance"),
			d);
		SEND_TO_Q ("Your Selection: ", d);
		d->connected = CON_LOGIN;
		d->acct = NULL;
		return;
	}
}

void
nanny_name_confirm (DESCRIPTOR_DATA * d, char *argument)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (*argument != 'y' && *argument != 'Y')
	{
		SEND_TO_Q ("What name would you like? ", d);
		d->connected = CON_NEW_ACCT_NAME;
		return;
	}

	sprintf (buf, "New player %s [%s].", (d->character)->name,
		d->strClientHostname);
	system_log (buf, false);
	strcat (buf, "\n");
	send_to_gods (buf);

	SEND_TO_Q ("Select a password: ", d);
	ECHO_OFF;

	d->connected = CON_LOGIN;
}


	//Functions to allow players to retire characters without staff-input
void
post_retirement (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *tch;
	MYSQL_RES *result;
	MYSQL_ROW row;
	char date[AVG_STRING_LENGTH];
	time_t time_retire;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	time_retire = time (0);
	ctime_r (&time_retire, date);
	date[strlen (date) - 1] = '\0';

	if (!d->pending_message->message)
	{
		SEND_TO_Q ("\n#2Character retirement aborted.#0\n", d);
		display_main_menu (d);
		return;
	}
	mysql_safe_query
		("SELECT name,account"
		" FROM pfiles"
		" WHERE accountId = %d"
		" AND create_state = 2"
		" AND level = 0",
		d->acct->id);

	if ((result = mysql_store_result (database)) == NULL)
	{
		sprintf (buf, "Warning: post_retirement(): %s", mysql_error (database));
		system_log (buf, true);
		SEND_TO_Q ("\n#2An error occurred. Character retirement aborted.#0\n",
			d);
		display_main_menu (d);
		return;
	}

	while ((row = mysql_fetch_row (result)))
	{
		mysql_safe_query
			("UPDATE pfiles"
			" SET create_state=4"
			" WHERE name = '%s'", row[0]);

		add_message (1, row[0], -2, d->acct->name.c_str (), date, "Retired.", "",
			d->pending_message->message, 0);
		add_message (1, "Retirements", -5, row[1], date, row[0], "",
			d->pending_message->message, 0);
		if ((tch = load_pc (row[0])))
		{
			tch->unload_pc();
		}
	}
	mysql_free_result (result);
	result = NULL;

	SEND_TO_Q
		("\n#2Character retired successfully; you may create a new one now.#0\n",
		d);
	display_main_menu (d);
}

void
nanny_retire (DESCRIPTOR_DATA * d, char *argument)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	ECHO_ON;

	if (!*argument)
	{
		SEND_TO_Q ("\n#2Character retirement aborted.#0\n", d);
		display_main_menu (d);
		return;
	}

	if (!check_password (argument, d->acct->password.c_str ()))
	{
		SEND_TO_Q ("\n#2Incorrect password. Character retirement aborted.#0\n",
			d);
		display_main_menu (d);
		return;
	}
	mysql_safe_query
		("SELECT name,account"
		" FROM pfiles"
		" WHERE accountId = %d"
		" AND create_state = 2",
		d->acct->id);

	if ((result = mysql_store_result (database)) == NULL)
	{
		sprintf (buf, "Warning: nanny_retire(): %s", mysql_error (database));
		system_log (buf, true);
		SEND_TO_Q ("\n#2An error occurred. Character retirement aborted.#0\n",
			d);
		display_main_menu (d);
		return;
	}

	if (!mysql_num_rows (result))
	{
		if (result != NULL)
			mysql_free_result (result);
		SEND_TO_Q ("\n#2You do not currently have a character to retire.#0\n",
			d);
		display_main_menu (d);
		return;
	}

	while ((row = mysql_fetch_row (result)))
	{
		if (get_pc (row[0]))
		{
			mysql_free_result (result);
			SEND_TO_Q
				("\n#2You may not retire a character while s/he is logged in.#0\n",
				d);
			display_main_menu (d);
			return;
		}
	}
	mysql_free_result (result);
	result = NULL;

	SEND_TO_Q
		("\n#2Please document thoroughly the reasoning behind your character retirement\n"
		"below; when finished, terminate the editor with an '@' symbol.#0\n\n",
		d);

	free_mem(d->pending_message);
	d->pending_message = new MESSAGE_DATA;
	d->descStr = d->pending_message->message;
	d->max_str = MAX_STRING_LENGTH;

	d->proc = post_retirement;
}

void
nanny_terminate (DESCRIPTOR_DATA * d, char *argument)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	int id;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	ECHO_ON;

	if (!*argument)
	{
		SEND_TO_Q ("\n#2Account termination aborted.#0\n", d);
		display_main_menu (d);
		return;
	}

	if (!check_password (argument, d->acct->password.c_str ()))
	{
		SEND_TO_Q ("\n#2Incorrect password. Account termination aborted.#0\n",
			d);
		display_main_menu (d);
		return;
	}
	mysql_safe_query ("SELECT user_id FROM users WHERE user_name = '%s'",
		d->acct->name.c_str ());
	if ((result = mysql_store_result (database)) == NULL)
	{
		sprintf (buf, "Warning: nanny_terminate(): %s", mysql_error (database));
		system_log (buf, true);
		SEND_TO_Q ("\n#2An error occurred. Account termination aborted.#0\n",
			d);
		display_main_menu (d);
		return;
	}

	row = mysql_fetch_row (result);
	id = atoi (row[0]);
	mysql_free_result (result);
	result = NULL;

	mysql_safe_query ("SELECT name FROM pfiles WHERE accountId = %d", d->acct->id);
	if ((result = mysql_store_result (database)) == NULL)
	{
		sprintf (buf, "Warning: nanny_terminate(): %s", mysql_error (database));
		system_log (buf, true);
		SEND_TO_Q ("\n#2An error occurred. Account termination aborted.#0\n",
			d);
		display_main_menu (d);
		return;
	}

	while ((row = mysql_fetch_row (result)))
	{
		mysql_safe_query ("DELETE FROM player_notes WHERE name = '%s'", row[0]);
	}
	mysql_free_result (result);
	result = NULL;

	mysql_safe_query ("DELETE FROM forum_user_group WHERE user_id = %d", id);
	mysql_safe_query ("DELETE FROM hobbitmail WHERE account = '%s'",
		d->acct->name.c_str ());
	mysql_safe_query
		("DELETE FROM registered_characters WHERE account_name = '%s'",
		d->acct->name.c_str ());
		//changed for auroness version
	mysql_safe_query ("DELETE FROM user WHERE user_name = '%s'",
		d->acct->name.c_str ());
	mysql_safe_query ("DELETE FROM pfiles WHERE accountId = %d", d->acct->id);

	sprintf (buf, "Account %s has been terminated.", d->acct->name.c_str ());
	send_to_gods (buf);

	SEND_TO_Q ("\n#2Account termination successful. Goodbye.#0\n", d);
	sprintf (buf, "\nPlease press ENTER to disconnect from the server.\n");
	SEND_TO_Q (buf, d);
	d->connected = CON_PENDING_DISC;
	return;

}

void
nanny_change_email (DESCRIPTOR_DATA * d, char *argument)
{
	MYSQL_RES *result;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int nEmailMatches = 0;
	
	if (*argument)
	{
		
		if (strstr (argument, " ") || strstr (argument, ";")
			|| strstr (argument, "\\") || strstr (argument, "("))
		{
			SEND_TO_Q ("\n#1Your input contains illegal characters.#0\n", d);
			display_main_menu (d);
			return;
		}
		
		mysql_safe_query
		("SELECT user_email FROM user WHERE user_email = '%s' and user_name != '%s'",
		 argument, d->acct->name.c_str ());
		
		if ((result = mysql_store_result (database)) != NULL)
		{
			nEmailMatches = mysql_num_rows (result);
			mysql_free_result (result);
			result = NULL;
		}
		else
		{
			sprintf (buf, "Warning: nanny_change_email(): %s",
					 mysql_error (database));
			system_log (buf, true);
			SEND_TO_Q
			("\n#1An error occurred.#0\nYour email address was NOT updated in our server.\n",
			 d);
			display_main_menu (d);
			return;
		}
		
		if (nEmailMatches)
		{
				/// \todo LOG THESE
			
			SEND_TO_Q
			("\n#1We're sorry, but that email address has already been registered\n"
			 "under an existing game account. Please choose another.#0\n\n",
			 d);
			SEND_TO_Q ("Your email address was NOT updated in our server.\n",
					   d);
			display_main_menu (d);
			return;
		}
		
		sprintf (buf, "\nIs the address %s correct? [y/n] ", argument);
		SEND_TO_Q (buf, d);
		
		d->stored = duplicateString (argument);
		
		d->connected = CON_CHG_EMAIL_CNF;
		return;
	}
	else
	{
		SEND_TO_Q ("\nYour email address was NOT updated in our server.\n", d);
		display_main_menu (d);
		return;
	}
}

void
nanny_change_email_confirm (DESCRIPTOR_DATA * d, char *argument)
{
	argument[0] = toupper (argument[0]);
	
	if (*argument == 'Y')
	{
		SEND_TO_Q ("\nYour email address was successfully updated.\n", d);
		d->acct->update_email (d->stored);
		free_mem (d->stored); // char*
		d->stored = NULL;
		display_main_menu (d);
		return;
	}
	else
	{
		SEND_TO_Q ("\nYour email address was NOT updated in our server.\n", d);
		display_main_menu (d);
		return;
	}
}

/******** HOBBIT MAIL FUNCTIONS  *************/
/*                                                                          *
* function: display_hobbitmail_inbox                                       *
*                                                                          *
* 09/20/2004 [JWW] - Fixed an instance where mysql result was not freed    *
*                                                                          */
void
display_hobbitmail_inbox (DESCRIPTOR_DATA * d, account  *acct)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char from_buf[MAX_STRING_LENGTH]= { '\0' };
	char date_buf[MAX_STRING_LENGTH]= { '\0' };
	char re_buf[MAX_STRING_LENGTH]= { '\0' };
	char imm_buf[MAX_STRING_LENGTH]= { '\0' };
	int i, admin = 0;

	sprintf (buf, "                              #6Hobbit-Mail: Main Menu#0\n"
		"                              #6----------------------#0\n\n");

	if (!acct || acct->name.empty ())
	{
		sprintf (buf + strlen (buf),
			"                  #2There are currently no messages in your inbox.#0\n");
		SEND_TO_Q (buf, d);
		return;
	}

	mysql_safe_query
		("SELECT * FROM hobbitmail WHERE account = '%s' ORDER BY timestamp DESC",
		acct->name.c_str ());

	if ((result = mysql_store_result (database)) == NULL)
	{
		sprintf (buf + strlen (buf),
			"                  #2There are currently no messages in your inbox.#0\n");
		sprintf (buf + strlen (buf),
			"\nEnter message number to display, \"new\", or \"exit\": ");
		SEND_TO_Q (buf, d);
		sprintf (buf, "Warning: display_hobbitmail_inbox(): %s",
			mysql_error (database));
		system_log (buf, true);
		return;
	}

	if (mysql_num_rows (result) <= 0)
	{
		sprintf (buf + strlen (buf),
			"                  #2There are currently no messages in your inbox.#0\n");
		sprintf (buf + strlen (buf),
			"\nEnter message number to display, \"new\", or \"exit\": ");
		SEND_TO_Q (buf, d);
	}
	else
	{
		admin = is_admin (d->acct->name.c_str ());
		i = 0;
		while ((row = mysql_fetch_row (result)))
		{
			i++;
			if (i > 100)
			{
				sprintf (buf + strlen (buf),
					"\n...remaining messages truncated. Delete current messages to see them.\n");
				break;
			}
			if (d->acct && admin == true)
				sprintf (imm_buf, " [%s]", row[3]);
			else
				*imm_buf = '\0';

			*from_buf = '\0';
			*date_buf = '\0';
			*re_buf = '\0';

			sprintf (from_buf, "#2From:#0 %s%s", row[2], admin ? imm_buf : "");
			sprintf (date_buf, "#2Dated:#0 %s", row[4]);
			sprintf (re_buf, "     #2Regarding:#0 %s%s%s", row[5],
				!IS_SET (atoi (row[1]), MF_READ) ? " #6(unread)#0" : "",
				IS_SET (atoi (row[1]),
				MF_REPLIED) ? " #5(replied)#0" : "");

			sprintf (buf + strlen (buf), "#6%3d.#0 %-40s %s\n%s\n\n", i,
				from_buf, date_buf, re_buf);
		}
		sprintf (buf + strlen (buf),
			"Enter message number to display, \"new\", or \"exit\": ");
		page_string (d, buf);
	}

	mysql_free_result (result);
	result = NULL;


}

void
nanny_composing_message (DESCRIPTOR_DATA * d, char *argument)
{
	DESCRIPTOR_DATA *td;
	MUDMAIL_DATA *message;
	account *acct = NULL;
	time_t current_time;
	char date[AVG_STRING_LENGTH];
	
		//check for termination and send if so
	if ((strncmp(argument,"@",1)==0) && strlen(argument)==1)
	{
		current_time = time (0);
		ctime_r (&current_time, date);
		if (strlen (date) > 1)
			date[strlen (date) - 1] = '\0';
		
		message = new MUDMAIL_DATA;
		message->from = duplicateString (d->pending_message->poster);
		message->subject = duplicateString (d->pending_message->subject);
		message->message = duplicateString (d->descStr);
		message->from_account = duplicateString (d->acct->name.c_str ());
		message->date = duplicateString (date);
		message->flags = 0;
		message->target = duplicateString (d->pending_message->target);
		
		acct = new account (d->stored);
		
		save_hobbitmail_message (acct, message);
		
		free_mem (message->from);
		free_mem (message->subject);
		free_mem (message->message);
		free_mem (message->from_account);
		free_mem (message->date);
	free_mem (message); // MUDMAIL_DATA*
		
	free_mem(d->pending_message);
		d->pending_message = NULL;
		
		if (!acct->is_registered ())
		{
			SEND_TO_Q
			("#1Your message was not delivered; there was an error.#0\n\n", d);
		}
		else
		{
			SEND_TO_Q
			("#2Thanks! Your Hobbit-Mail has been delivered to the specified account.#0\n\n",
			 d);
			for (td = descriptor_list; td; td = td->next)
			{
				if (!td->acct || td->acct->name.empty () || !td->character
					|| td->connected != CON_PLYNG)
					continue;
				if (str_cmp (td->acct->name.c_str (), acct->name.c_str ()) == 0)
					SEND_TO_Q
					("#6\nA new Hobbit-Mail has arrived for your account!#0\n", td);
			}
		}
		
		delete acct;
		
		d->stored = duplicateString ("");
		
		display_hobbitmail_inbox (d, d->acct);
		
		d->connected = CON_MAIL_MENU;
	}
	else
	{
			//otherwise append the line and keep going.
		string_add(d, argument);
		
			//indicate that it was received by sending another prompt
		SEND_TO_Q("]\n",d);
	}
}

void
nanny_compose_message (DESCRIPTOR_DATA * d, char *argument)
{

	if (!*argument)
	{
		SEND_TO_Q ("\n", d);

		display_hobbitmail_inbox (d, d->acct);

		d->connected = CON_MAIL_MENU;
		return;
	}

	if (isdigit (*argument))
	{
		SEND_TO_Q ("\nRegarding? ", d);
		return;
	}

	d->pending_message->subject = duplicateString (argument);

	SEND_TO_Q
		("\n#2Enter message; terminate with an '@' when completed. Once finished,\n",
		d);
	SEND_TO_Q
		("hit ENTER again to send and return to the main Hobbit-Mail menu.#0\n\n",
		d);

	d->pending_message->message = NULL;
	d->descStr = d->pending_message->message;
	d->max_str = MAX_STRING_LENGTH;
	d->connected = CON_COMPOSING_MESSAGE;
}

void
nanny_compose_subject (DESCRIPTOR_DATA * d, char *argument)
{
	if (!*argument)
	{

		SEND_TO_Q ("\n", d);

		display_hobbitmail_inbox (d, d->acct);

		d->connected = CON_MAIL_MENU;
		return;
	}

	if (isdigit (*argument))
	{
		SEND_TO_Q ("\nAs whom do you wish to send the message? ", d);
		return;
	}

	d->pending_message->poster = duplicateString (argument);
	d->connected = CON_COMPOSE_MESSAGE;
	SEND_TO_Q ("\nRegarding? ", d);
	return;
}

/*                                                                          *
* function: nanny_compose_mail_to                                          *
*                                                                          *
* 09/20/2004 [JWW] - Fixed an instance where mysql result was not freed    *
*                  - handled case of result = NULL                         *
*                                                                          */
void
nanny_compose_mail_to (DESCRIPTOR_DATA * d, char *argument)
{
	CHAR_DATA *tch;
	account *acct;
	MYSQL_RES *result;
	MYSQL_ROW row;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (!*argument)
	{
		SEND_TO_Q ("\n", d);

		display_hobbitmail_inbox (d, d->acct);

		d->connected = CON_MAIL_MENU;
		return;
	}

	if (!(tch = load_pc (argument)))
	{
		mysql_safe_query
			("SELECT name"
			" FROM pfiles"
			" WHERE keywords"
			" LIKE '%%%s%%' LIMIT 1", argument);
		if ((result = mysql_store_result (database)) != NULL)
		{
			if (mysql_num_rows (result) > 0)
			{
				row = mysql_fetch_row (result);
				tch = load_pc (row[0]);
			}
			mysql_free_result (result);
			result = NULL;
		}
		else
		{
			sprintf (buf, "Warning: nanny_compose_mail_to(): %s",
				mysql_error (database));
			system_log (buf, true);
		}

		if (!tch)
		{
			SEND_TO_Q ("#1\nI am sorry, but that PC could not be found.#0\n",
				d);
			SEND_TO_Q ("\nTo which PC's player do you wish to send a message? ",
				d);
			return;
		}

	}

	acct = new account (tch->pc->account_name);

	if (!acct->is_registered ())
	{
		delete acct;
		SEND_TO_Q
			("#1\nThere seems to be a problem with that PC's account.#0\n", d);
		SEND_TO_Q ("\nTo which PC's player do you wish to send a message? ", d);
		tch->unload_pc();
		return;
	}

	mysql_safe_query ("SELECT COUNT(*) FROM hobbitmail WHERE account = '%s'",
		acct->name.c_str ());

	if ((result = mysql_store_result (database)) != NULL)
	{
		row = mysql_fetch_row (result);
		if (atoi (row[0]) >= (tch->get_trust() ? 500 : 100 )) /* limit 500 not 100 for admin */
		{
			SEND_TO_Q
				("#1\nSorry, but that person's mailbox is currently full.#0\n",
				d);
			SEND_TO_Q ("\nTo which PC's player do you wish to send a message? ",
				d);
			mysql_free_result (result);
			result = NULL;
			delete acct;
			return;
		}
		mysql_free_result (result);
		result = NULL;
	}
	else
	{
		sprintf (buf, "Warning: nanny_compose_mail_to(): %s",
			mysql_error (database));
		system_log (buf, true);
	}

	if (str_cmp (acct->name.c_str (), "Guest") == 0)
	{
		SEND_TO_Q
			("#1\nSorry, but Hobbit-Mail cannot be sent to the guest account.#0\n",
			d);
		SEND_TO_Q ("\nTo which PC's player do you wish to send a message? ", d);
		tch->unload_pc();
		delete acct;
		return;
	}

	d->pending_message = new MESSAGE_DATA;
	d->pending_message->target = duplicateString (argument);
	d->stored = duplicateString (acct->name.c_str ());
	delete acct;
	tch->unload_pc();


	SEND_TO_Q ("\nAs whom do you wish to send the message? ", d);
	d->connected = CON_COMPOSE_SUBJECT;
}

/*                                                                          *
* function: nanny_mail_menu                                                *
*                                                                          *
*                                                                          */
void
nanny_mail_menu (DESCRIPTOR_DATA * d, char *argument)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	int i = 1;
	char imm_buf[MAX_STRING_LENGTH]= { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (!argument || !*argument
		|| (!isdigit (*argument)
		&& strn_cmp (argument, "new", strlen (argument))
		&& strn_cmp (argument, "exit", strlen (argument))))
	{
		SEND_TO_Q ("Enter message number to display, \"new,\" or \"exit\": ",
			d);
		return;
	}

	if (!strn_cmp (argument, "exit", strlen (argument)))
	{
		display_main_menu (d);
		return;
	}

	if (!strn_cmp (argument, "new", strlen (argument)))
	{
		d->connected = CON_COMPOSE_MAIL_TO;
		SEND_TO_Q ("\nTo which PC's player do you wish to send a message? ", d);
		return;
	}

	if (isdigit (*argument))
	{
		std::string escaped_name;
		d->acct->get_name_sql_safe (escaped_name);

		std::ostringstream message_query_stream;

		message_query_stream <<
			"SELECT account,flags,from_line,from_account,"
			"sent_date, subject,message, timestamp, id,"
			"DATE_FORMAT(FROM_UNIXTIME(timestamp + "
			"),\"%a %b %d %T %Y\") AS sent_date,to_line "
			" FROM hobbitmail WHERE account = '"
			<< escaped_name <<
			"' ORDER BY timestamp DESC";

		std::string message_query_string = message_query_stream.str ();
		mysql_safe_query ((char *)message_query_string.c_str ());

		if ((result = mysql_store_result (database)) == NULL)
		{
			sprintf (buf, "Warning: nanny_mail_menu(): %s",
				mysql_error (database));
			system_log (buf, true);
			SEND_TO_Q
				("\n#1I am sorry, but that message could not be found.#0\n", d);
			SEND_TO_Q
				("\nEnter message number to display, \"new\", or \"exit\": ", d);
			return;
		}

		if (!mysql_num_rows (result))
		{
			if (result != NULL)
				mysql_free_result (result);
			SEND_TO_Q
				("\n#1I am sorry, but that message could not be found.#0\n", d);
			SEND_TO_Q
				("\nEnter message number to display, \"new\", or \"exit\": ", d);
			return;
		}

		while ((row = mysql_fetch_row (result)))
		{
			if (i == atoi (argument))
				break;
			if (i >= 100)
			{
				row = NULL;
				break;
			}
			i++;
		}

		if (!row)
		{
			mysql_free_result (result);
			result = NULL;
			SEND_TO_Q
				("\n#1I am sorry, but that message could not be found.#0\n", d);
			SEND_TO_Q
				("\nEnter message number to display, \"new\", or \"exit\": ", d);
			return;
		}

		*imm_buf = '\0';

		if (is_admin (d->acct->name.c_str ()))
			sprintf (imm_buf, " [%s]", row[3]);

		sprintf (buf, "\n#2From:#0      %s%s\n"
			"%s%s%s"
			"#2Dated:#0     %s\n"
			"#2Regarding:#0 %s\n"
			"\n"
			"%s\n",
			row[2],
			(is_admin (d->acct->name.c_str ()) ? imm_buf : ""),
			(row[10] ? "#2To:#0        " : ""),
			(row[10] ? row[10] : ""),
			(row[10] ? "\n" : ""), row[9], row[5], row[6]);

		sprintf (buf + strlen (buf),
			"Enter \"delete,\" \"reply,\" or \"exit\": ");

		set_hobbitmail_flags (atoi (row[8]), MF_READ);

		SEND_TO_Q (buf, d);

		d->stored = (char *) atoi (row[8]);

		d->connected = CON_READ_MESSAGE;

		mysql_free_result (result);
		result = NULL;

	}
}

/*                                                                          *
* function: nanny_read_message                                             *
*                                                                          *
*                                                                          */
void
nanny_read_message (DESCRIPTOR_DATA * d, char *argument)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (!*argument || isdigit (*argument))
	{
		SEND_TO_Q ("Enter \"delete\", \"reply\", or \"exit\": ", d);
		d->connected = CON_READ_MESSAGE;
		return;
	}

	if (strn_cmp (argument, "delete", strlen (argument))
		&& strn_cmp (argument, "reply", strlen (argument))
		&& strn_cmp (argument, "exit", strlen (argument)))
	{
		SEND_TO_Q ("Enter \"delete,\" \"reply,\" or \"exit\": ", d);
		d->connected = CON_READ_MESSAGE;
		return;
	}

	if (!strn_cmp (argument, "reply", strlen (argument)))
	{
		set_hobbitmail_flags ((long int) d->stored, MF_READ | MF_REPLIED);
		mysql_safe_query
			("SELECT from_account, from_line FROM hobbitmail WHERE account = '%s' AND id = %d",
			d->acct->name.c_str (), (long int) d->stored);
		if ((result = mysql_store_result (database)) == NULL)
		{
			sprintf (buf, "Warning: nanny_read_message(): %s",
				mysql_error (database));
			system_log (buf, true);
			SEND_TO_Q ("Enter \"delete\", \"reply\", or \"exit\": ", d);
			d->connected = CON_READ_MESSAGE;
			return;
		}
		row = mysql_fetch_row (result);
		d->stored = duplicateString (row[0]);

		d->pending_message = new MESSAGE_DATA;
		d->pending_message->target = duplicateString (row[1]);
		mysql_free_result (result);
		result = NULL;
		d->connected = CON_COMPOSE_SUBJECT;
		SEND_TO_Q ("\nAs whom do you wish to send the reply? ", d);
		return;
	}

	if (!strn_cmp (argument, "delete", strlen (argument)))
	{
		mysql_safe_query
			("DELETE FROM hobbitmail WHERE account = '%s' AND id = %d",
			d->acct->name.c_str (), (long int) d->stored);
		sprintf (buf,
			"\n#1The specified Hobbit-Mail has been deleted from your account.#0\n\n");
		SEND_TO_Q (buf, d);

		display_hobbitmail_inbox (d, d->acct);

		d->connected = CON_MAIL_MENU;
		return;
	}

	if (!strn_cmp (argument, "exit", strlen (argument)))
	{

		SEND_TO_Q ("\n", d);

		display_hobbitmail_inbox (d, d->acct);

		d->connected = CON_MAIL_MENU;
		return;
	}
}


/********** CHARACTER GENERATION **************/


//TODO: may need to change numbers on menu - change lib/text/menu1 to match
void
nanny_connect_select (DESCRIPTOR_DATA * d, char *argument)
{
	MYSQL_ROW row = NULL;
	MYSQL_RES *result = NULL;
	DESCRIPTOR_DATA *td;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char state[MAX_STRING_LENGTH]= { '\0' };
	int i = 0, nCount = 0;
	char c;
	int argn = 0;

	if (d->acct->color && !d->color)
		d->color = 1;

	if (!*argument)
	{
		display_main_menu (d);
		return;
	}

	while (isspace ((c = tolower (*argument))))
		argument++;

	argn = strtol (argument,0,10);

		//enter the game with a live character
	if (c == 'e' || argn == 1)
	{
		mysql_safe_query ("SELECT name,create_state"
						  " FROM pfiles"
						  " WHERE accountId = %d"
						  " AND create_state = %d",
						  d->acct->id,
						  STATE_SUSPENDED);
		
		if ((result = mysql_store_result (database)))
		{
			
			nCount = mysql_num_rows (result);
			mysql_free_result (result);
			
			if (nCount > 0)
			{
				
				SEND_TO_Q
				("You may not enter the game while you have suspended characters in your account.\n",
				 d);
				display_main_menu (d);
				return;
				
			}
		}
		
		
		mysql_safe_query ("SELECT name, create_state FROM pfiles WHERE accountId = %d ORDER BY create_state ASC", d->acct->id);
		
		result = mysql_store_result (database);
		
		if (!result || !mysql_num_rows (result))
		{
			SEND_TO_Q
			("There are currently no PCs registered under this account.\n",
			 d);
			display_main_menu (d);
			if (result)
				mysql_free_result (result);
			return;
		}
		
		if (mysql_num_rows (result) > 1)
		{
			SEND_TO_Q ("\nWhich character would you like to log in?\n\n", d);
			i = 1;
			
				//see character application status
			while ((row = mysql_fetch_row (result)))
			{
				
				if (atoi (row[1]) < 1)
					sprintf (state, "#3(Pending)#0");
				else if (atoi (row[1]) == 1)
					sprintf (state, "#6(Submitted)#0");
				else if (atoi (row[1]) == 2)
					sprintf (state, "#2(Active)#0");
				else if (atoi (row[1]) == 3)
					sprintf (state, "#5(Suspended)#0");
				else if (atoi (row[1]) == 4)
					sprintf (state, "#1(Deceased)#0");
				else
					sprintf (state, "#1(Retired)#0");
				sprintf (buf, "%2d. %-20s %s\n", i, row[0], state);
				SEND_TO_Q (buf, d);
				i++;
			}
			SEND_TO_Q ("\nYour Selection: ", d);
			d->connected = CON_CHOOSE_PC;
			if (result)
				mysql_free_result (result);
			return;
		}
		else
		{
			if (result)
				mysql_free_result (result);
			nanny_choose_pc (d, "1");
			return;
		}
	}
	
		//guest access
	else if (c == 'g' || argn == 2)
	{
		if (IS_SET (d->acct->flags, ACCOUNT_NOGUEST))
		{
			SEND_TO_Q
			("\n#1Your guest login privileges have been revoked by an admin.\n\n#0Your Selection: ",
			 d);
			return;
		}
		
		for (td = descriptor_list; td; td = td->next)
		{
			if (!td->character || !td->acct || td->acct->name.empty ())
				continue;
			if (IS_NPC (td->character))
				continue;
			if (td->connected != CON_PLYNG)
				continue;
			if ((td->character)->get_trust())
				continue;
			if (!str_cmp (td->acct->name.c_str (), d->acct->name.c_str ()))
			{
				SEND_TO_Q
				("\n#1Sorry, but it is against policy to have two characters from the\n"
				 "same account logged in at the same time.#0\n\nYour Selection: ",
				 d);
				return;
			}
		}
		
		SEND_TO_Q ("\n#2A guest login is being created for you...#0\n\n", d);
		SEND_TO_Q
		("While visiting as a guest, you will be held responsible for\n"
		 "following our policies. Guest logins are provided for new\n"
		 "players to experience the game in a limited capacity while\n"
		 "waiting for an application, or for researching GameWprld\n"
		 "using our in-game material. We hope you enjoy your stay!\n\n"
		 "#1Under NO CIRCUMSTANCES should these logins be used to harass\n"
		 "immortals regarding pending character applications! We frown\n"
		 "very highly on this sort of abuse of our guest login system.#0\n\n"
		 "Please be aware that all guest IPs and commands are logged.\n\n",
		 d);
		
		SEND_TO_Q ("#2When ready, please press ENTER to be incarnated.#0\n", d);
		d->connected = CON_CREATE_GUEST;
		return;
	}
	
		//char-generation
	else if (c == 'c' || argn == 3)
	{
		if (str_cmp ("Unknown", d->acct->name.c_str ()) == 0)
		{
			SEND_TO_Q
			("\n#1Sorry, but that isn't a valid option.#0\n\nYour Selection: ",
			 d);
			return;
		}
		mysql_safe_query ("SELECT name,create_state "
						  "FROM pfiles "
						  "WHERE account = '%s'"
						  " AND create_state = %d",
						  d->acct->name.c_str (), STATE_SUSPENDED);
		
		if ((result = mysql_store_result (database)))
		{
			
			nCount = mysql_num_rows (result);
			mysql_free_result (result);
			
			if (nCount > 0)
			{
				
				SEND_TO_Q
				("You may not create new characters while you are suspended.\n",
				 d);
				display_main_menu (d);
				return;
				
			}
		}
		
		d->character = new_char (1);
		d->character->race = -1;
		d->character->desc = d;
		SEND_TO_Q (get_text_buffer (NULL, text_list, "new_name"), d);
		SEND_TO_Q ("\nWhat would you like to name your new character? ", d);
		d->connected = CON_NAME_CONFIRM;
		return;
	}
	
		//CHARGEN delete pending applciations
	else if (c == 'd' || argn == 4)
	{
		if (strcasecmp ("Unknown", d->acct->name.c_str ()) == 0)
		{
			SEND_TO_Q
			("\n#1Sorry, but that isn't a valid option.#0\n\nYour Selection: ",
			 d);
			return;
		}
		
		mysql_safe_query
		("SELECT name,create_state"
		 " FROM pfiles"
		 " WHERE account = '%s'"
		 " AND create_state <= 1",
		 d->acct->name.c_str ());
		result = mysql_store_result (database);
		
		if (!result || !mysql_num_rows (result))
		{
			SEND_TO_Q
			("There are currently no pending PCs on this account to delete.\n",
			 d);
			if (result)
				mysql_free_result (result);
			display_main_menu (d);
			return;
		}
		
		SEND_TO_Q ("\nWhich pending character would you like to delete?\n"
				   "Enter 0 to return to main menu without deletion.\n\n",
				   d);
		
		i = 1;
		
		while ((row = mysql_fetch_row (result)))
		{
			sprintf (buf, "%d. %s\n", i, row[0]);
			SEND_TO_Q (buf, d);
			i++;
		}
		
		SEND_TO_Q ("\nYour Selection: ", d);
		d->connected = CON_DELETE_PC;
		
		if (result)
			mysql_free_result (result);
		
		return;
	}
	
		//change email address - 5
	/****** removed because email changes take place on forum
	 
	 else if (c == 'c' || argn == 5)
	 {
	 if (strcasecmp ("Unknown", d->acct->name.c_str()) == 0)
	 {
	 SEND_TO_Q
	 ("\n#1Sorry, but that isn't a valid option.#0\n\nYour Selection: ",
	 d);
	 return;
	 }
	 SEND_TO_Q ("\nYour registered email address: ", d);
	 SEND_TO_Q (d->acct->email.c_str (), d);
	 SEND_TO_Q ("\n", d);
	 SEND_TO_Q ("\nEnter the desired email address: ", d);
	 d->connected = CON_CHG_EMAIL;
	 return;
	 }
	 ** end of this section being removed in favor of forum changes
	 ******************/
	
		//new password - 6
	/****** removed because changes take place on forum

	else if (c == 'm' || argn == 6)
	{
		if (str_cmp ("Unknown", d->acct->name.c_str ())==0)
		{
			SEND_TO_Q
			("\n#1Sorry, but that isn't a valid option.#0\n\nYour Selection: ",
			 d);
			return;
		}
		SEND_TO_Q ("Enter a new password: ", d);
		ECHO_OFF;
		d->connected = CON_PWDNEW;
		return;
	}
	 *************/
	
		//ansi colors
	else if (c == 'a' || argn == 7)
	{
		if (strcasecmp ("Unknown", d->acct->name.c_str ()) == 0)
		{
			SEND_TO_Q ("\n#1Sorry, but that isn't a valid option.#0\n\n"
					   "Your Selection: ", d);
			return;
		}
		
		if (d->acct->toggle_color_flag())
		{
			SEND_TO_Q ("ANSI color #2enabled#0.\n", d);
			SEND_TO_Q ("#1NOTE: For best results, "
					   "a default of #0white#1 or #0whitish-gray#1 text "
					   "is recommended.\n", d);
			d->color = 1;
		}
		else
		{
			d->color = 0;
			SEND_TO_Q ("ANSI color disabled.\n", d);
		}
		display_main_menu (d);
		return;
	}
	
		//newsletter toggle - 8
	/******** removed because newsletter subscription is now handled in the forums
	 
	 //newsletter toggle
	 else if (c == 'n' || argn == 8)
	 {
	 if (strcasecmp ("Unknown", d->acct->name.c_str ()) == 0)
	 {
	 SEND_TO_Q ("\n#1Sorry, but that isn't a valid option.#0\n\n"
	 "Your Selection: ", d);
	 return;
	 }
	 if (d->acct->toggle_newsletter_flag ())
	 {
	 SEND_TO_Q ("\n#2You will now receive our weekly newsletter "
	 "via email.#0\n", d);
	 }
	 else
	 {
	 SEND_TO_Q ("\n#1You will no longer receive our weekly newsletter "
	 "via email.#0\n", d);
	 }
	 display_main_menu (d);
	 return;
	 }
	 /**********/
	
		//hobbit mail
	else if (c == 'h' || argn == 9)
	{
		if (strcasecmp ("Unknown", d->acct->name.c_str ()) == 0)
		{
			SEND_TO_Q
			("\n#1Sorry, but that isn't a valid option.#0\n\nYour Selection: ",
			 d);
			return;
		}
		
		display_hobbitmail_inbox (d, d->acct);
		
		d->connected = CON_MAIL_MENU;
		return;
	}
	
		//logging out
	else if (c == 'l' || argn == 10)
	{
		sprintf (buf, "%s [%s] has logged out.\n", d->acct->name.c_str (),
			d->strClientHostname);
		send_to_gods (buf);
		sprintf (buf, "%s [%s] has logged out.", d->acct->name.c_str (),
			d->strClientHostname);
		system_log (buf, false);
		close_socket (d);
		return;
	}

	else
	{
		SEND_TO_Q
			("\n#1Sorry, but that isn't a valid option1.#0\n\nYour Selection: ",
			d);
	}
} 


void
nanny_reading_wait (DESCRIPTOR_DATA * d, char *argument)
{
	display_main_menu (d);
	SEND_TO_Q ("Your Selection: ", d);
	return;
}

void
nanny_delete_pc (DESCRIPTOR_DATA * d, char *argument)
{
	MYSQL_ROW row;
	MYSQL_RES *result;
	char name[MAX_STRING_LENGTH]= { '\0' };
	int i = 0, j = 0;

	if (!*argument)
	{
		display_main_menu (d);
		return;
	}

	if (!isdigit (*argument))
	{
		display_main_menu (d);
		return;
	}
	mysql_safe_query
		("SELECT name,create_state"
		" FROM pfiles"
		" WHERE accountId = %d"
		" AND create_state <= 1",
		d->acct->id);
	result = mysql_store_result (database);

	if (!result)
	{
		display_main_menu (d);
		return;
	}

	j = mysql_num_rows (result);

	if (atoi (argument) < 1 || atoi (argument) > j)
	{
		SEND_TO_Q ("Character deletion has been aborted.\n\n", d);
		
		d->connected = CON_ACCOUNT_MENU;
		if (result)
			mysql_free_result (result);
		display_main_menu (d);
		return;
	}

	i = 1;

	while ((row = mysql_fetch_row (result)))
	{
		if (atoi (argument) == i)
		{
			if (!(d->character = load_pc (row[0])))
			{
				SEND_TO_Q
					("\n#1That character could not be loaded. The playerfile may have become\n"
					"corrupt; please email the staff list about this.#0\n\n", d);
				SEND_TO_Q ("Your Selection: ", d);
				d->connected = CON_DELETE_PC;
				if (result)
					mysql_free_result (result);
				return;
			}
			else
				break;
		}
		i++;
	}

	sprintf (name, "%s", d->character->name);
	d->character->unload_pc();
	d->character = NULL;

	mysql_safe_query ("DELETE FROM pfiles WHERE name = '%s'", name);
	mysql_safe_query ("DELETE FROM player_journals WHERE name = '%s'", name);
	mysql_safe_query ("DELETE FROM player_notes WHERE name = '%s'", name);

	SEND_TO_Q ("\n#1This pending character has been successfully deleted.#0\n",
		d);
	display_main_menu (d);
	d->connected = CON_ACCOUNT_MENU;

	return;

}

int
available_roles (int points)
{
	ROLE_DATA *role;
	
	for (role = role_list; role; role = role->next)
	{
		if (role->cost <= points)
			return 1;
	}
	
	return 0;
}


void
determine_nanny_state(DESCRIPTOR_DATA * d)
{
	if (d->character->sex == 0)
		d->character->pc->nanny_state = STATE_GENDER;
	
	else if (d->character->race < 0)
		d->character->pc->nanny_state = STATE_RACE;
	
	else if (!d->character->pc->special_role || d->character->pc->special_role->id == 0)
		d->character->pc->nanny_state = STATE_SPECIAL_ROLES;
	
	else if (d->character->age == 0)
		d->character->pc->nanny_state = STATE_AGE;
	
	else if (d->character->str == 0)
		d->character->pc->nanny_state = STATE_ATTRIBUTES;
	
	else if (d->character->height == 0)
		d->character->pc->nanny_state = STATE_FRAME;
	
	else if (!str_cmp(d->character->short_descr, ""))
		d->character->pc->nanny_state = STATE_SDESC;
	
	else if (!str_cmp(d->character->long_descr, ""))
		d->character->pc->nanny_state = STATE_LDESC;
	
	else if (!str_cmp(d->character->keywords, ""))
		d->character->pc->nanny_state = STATE_KEYWORDS;
	
	else if (!str_cmp(d->character->description, ""))
		d->character->pc->nanny_state = STATE_FDESC;
	
	else if (d->character->pc->profession < 0)
		d->character->pc->nanny_state = STATE_PROFESSION;
	
	else if (d->character->skill_map.size() < 6)
		d->character->pc->nanny_state = STATE_SKILLS;
	
	else if (!d->character->pc->creation_comment)
		d->character->pc->nanny_state = STATE_COMMENT;
	
	else 
		d->character->pc->nanny_state = 0;
	
	return;
	
}
					  



/*                                                                          *
* function: nanny_choose_pc                                                *
*                                                                          *
*   selects character either from chargen or active character              */
void
nanny_choose_pc (DESCRIPTOR_DATA * d, char *argument)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	DESCRIPTOR_DATA *td;
	AFFECTED_TYPE *af;
	CHAR_DATA *ch;
	ROOM_DATA *troom;
	OBJ_DATA *obj;
	time_t current_time;
	char date[AVG_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int i = 0, j = 0, online, guest, to_room = 0;
	extern int count_guest_online;

	if (!*argument)
	{
		display_main_menu (d);
		return;
	}

	if (!isdigit (*argument))
	{
		display_main_menu (d);
		return;
	}

	if (atoi (argument) == 0)
	{
		display_main_menu (d);
		return;
	}
	mysql_safe_query ("SELECT name, create_state FROM pfiles WHERE accountId = %d ORDER BY create_state ASC", d->acct->id);
	result = mysql_store_result (database);

	if (result && mysql_num_rows (result) > 1)
	{
		j = mysql_num_rows (result);

		if (atoi (argument) > j || atoi (argument) < 1)
		{
			SEND_TO_Q ("\nThat isn't a valid PC. Please pick again.\n\n", d);
			SEND_TO_Q ("Your Selection: ", d);
			d->connected = CON_CHOOSE_PC;
			if (result)
				mysql_free_result (result);
			return;
		}

		i = 1;

		while ((row = mysql_fetch_row (result)))
		{
			if (atoi (argument) == i)
			{
				if (!(d->character = load_pc (row[0])))
				{
					SEND_TO_Q
						("\nThat character could not be loaded. Please pick again.\n\n",
						d);
					SEND_TO_Q ("Your Selection: ", d);
					d->connected = CON_CHOOSE_PC;
					if (result)
						mysql_free_result (result);
					return;
				}
				break;
			}
			i++;
		}
	}
	else
	{
		if (!result || !(row = mysql_fetch_row (result))
			|| !(d->character = load_pc (row[0])))
		{
			SEND_TO_Q
				("\nUh oh - your PC could not be loaded from the database!\n\n",
				d);
			display_main_menu (d);
			if (result)
				mysql_free_result (result);
			return;
		}
	}

	if (result)
		mysql_free_result (result);

	if (!strstr (d->strClientHostname, "localhost"))
	{
		for (td = descriptor_list; td; td = td->next)
		{
			if (!td->character || !td->acct || td->acct->name.empty ())
				continue;
			if (IS_NPC (td->character))
				continue;
			if (td->connected != CON_PLYNG)
				continue;
			if (IS_SET (td->acct->flags, ACCOUNT_IPSHARER) &&
				strcmp (td->acct->name.c_str(), d->acct->name.c_str()))
				continue;
			if ((!str_cmp (td->strClientHostname, d->strClientHostname)
				|| !str_cmp (td->acct->name.c_str (), d->acct->name.c_str ()))
				&& str_cmp (td->character->name, d->character->name))
			{
				SEND_TO_Q
					("\n#1Sorry, but it is against policy to have two characters from the\n"
					"same account or IP address logged in at the same time.#0\n",
					d);
				display_main_menu (d);
				return;
			}
		}
	}

	if (d->character->pc->create_state == STATE_REJECTED)
		d->character->pc->create_state = STATE_APPLYING;

	if (d->character->pc->create_state == STATE_APPLYING)
	{
		d->character->desc = d;
		determine_nanny_state(d);
		d->connected = CON_CREATION;
		create_menu_options (d);
		return;
	}

	else if (d->character->pc->create_state == STATE_SUBMITTED)
	{
		SEND_TO_Q
			("\n#6A review of your character application is pending. Please be patient.#0\n",
			d);
		display_main_menu (d);
		d->character->unload_pc();
		d->character = NULL;
		return;
	}

	else if (d->character->pc->create_state == STATE_DIED
			 || d->character->pc->create_state == STATE_RETIRED)
	{
		SEND_TO_Q ("\n#1The selected character is not available because they are retired or dead.#0\n", d);
		display_main_menu (d);
		d->character->unload_pc();
		d->character = NULL;
		return;
	}

	else if (d->character->pc->create_state == STATE_SUSPENDED)
	{
		SEND_TO_Q
			("\n#1This character has been suspended by an administrator.#0\n", d);
		display_main_menu (d);
		d->character->unload_pc();
		d->character = NULL;
		return;
	}

	
		
	if (!is_admin (d->acct->name.c_str ()))
		{
			if (!IS_SET (d->acct->flags, ACCOUNT_BETA))
			{
				SEND_TO_Q
				("\n#1You are not a member of the Closed Beta Players.#0 Feel free to log into the Guest Lounge instead.\n",
				 d);
				display_main_menu (d);
				d->character->unload_pc();
				d->character = NULL;
				return;
			}
		}
	
	
	if (d->acct->color)
		d->character->color = 1;
	else
		d->character->color = 0;


	if (d->character->pc->owner)
	{

		for (td = descriptor_list; td; td = td->next)
			if (td == d->character->pc->owner)
				break;

		if (!td)
		{
			d->character->pc->owner = NULL;
			system_log
				("AVOIDED CRASH BUG:  Entering game with owner set wrong.", true);
		}
	}

	/* Character is re-estabilishing a connection while connected */

	if (d->character->desc && d->character->desc != d)
	{
		SEND_TO_Q ("\n#6Your character was not gracefully removed from the world. Disconnecting.\n", d);
		if((d->character)->name)
			sprintf (buf, "%s already online, disconnecting old connection.",(d->character)->name);
		else
			sprintf(buf, "An unknown entity was online already, disconnecting old connection. ERR:VM001");
		system_log (buf, false);
		close_socket (d->character->desc);
	}

	for (td = descriptor_list; td; td = td->next)
	{

		if (td == d)
			continue;

		if (td->original == d->character)
		{
			do_return (td->character, "", 0);
		}
		if (td->character == d->character)
		{
			close_socket (td);
			break;
		}

	}

	d->character->pc->owner = d;

	d->character->pc->last_connect = time (0);

	if ((ch = get_pc ((d->character)->name)) && !ch->pc->admin_loaded)
	{
		ch->action &= ~PLR_QUIET;
		d->connected = CON_PLYNG;

		if (IS_SET (ch->flags, FLAG_FLEE))
		{
			ch->send_to_char("You stop trying to flee.\n");
			ch->flags &= ~FLAG_FLEE;
		}

		ch->desc = d;
		d->time_last_activity = mud_time;
		ch->pc->time_last_activity = mud_time;

		ch->unload_pc();		/* Reconnected, we have an extra load count */

		if (d->character->room->nVirtual == LINKDEATH_HOLDING_ROOM)
		{
			(d->character)->char_from_room();
			(d->character)->char_to_room (d->character->last_room);
		}

		ch->act("$n has reconnected.", true, 0, 0, TO_ROOM);
		sprintf (buf, "%s has reconnected.", ch->name);
		system_log (buf, false);
		strcat (buf, "\n");
		send_to_gods (buf);
		do_look (d->character, "", 0);
		d->prompt_mode = 1;
		d->connected = CON_PLYNG;

			//level 1 builders don't see thier notes
		if (d->character->pc->level > 1)
			d->character->show_unread_messages();

		return;
	}


	d->character->pc_to_game();
	d->prompt_mode = 1;
	d->connected = CON_PLYNG;
	d->character->desc = d;

	d->character->flags &= ~(FLAG_FLEE | FLAG_ENTERING | FLAG_LEAVING | FLAG_SUBDUER | FLAG_SUBDUING | FLAG_SUBDUEE);
	
	if ((d->character)->get_trust())
	{
		d->character->flags &= ~FLAG_AVAILABLE;
		d->character->flags |= FLAG_WIZINVIS;
	}

	d->time_last_activity = mud_time;
	d->character->pc->time_last_activity = mud_time;

	if (d->character->pc->create_state == STATE_DIED)
	{

		(d->character)->set_position(STAND);

		(d->character)->char_to_room(VNUM_DEFAULT_LOAD_ROOM);
		d->character->flags |= FLAG_DEAD;
		sprintf (buf, "#1Dead Character:#0 %s has entered the game.\n",
			(d->character)->name);
		send_to_gods (buf);
		do_look (d->character, "", 1);

		return;
	}

	if (d->character->pc->create_state == STATE_RETIRED)
	{
		
		(d->character)->set_position(STAND);
		
		(d->character)->char_to_room(VNUM_DEFAULT_LOAD_ROOM);
		d->character->flags |= FLAG_RETIRED;
		sprintf (buf, "#1Retired Character:#0 %s has entered the game!?\n",
				 (d->character)->name);
		send_to_gods (buf);
		do_look (d->character, "", 1);
		
		return;
	}
	
	//new character enters the game for the first time
	if ((!d->character->in_room || d->character->in_room == NOWHERE) 
		&& !d->character->right_hand 
		&& !d->character->left_hand
		&& !d->character->equip)
	{
		d->character->description = duplicateString((reformat_desc((char*)d->character->description).c_str()));
	}

		//loads gear for newbies and returning players
		//then saves it instantly
	load_char_objs (d->character, (d->character)->name);
	save_char_objs (d->character, d->character->name);

	if ((!d->character->in_room || d->character->in_room == NOWHERE)
		|| !vtor (d->character->in_room))
	{
		if (IS_SET (d->character->plr_flags, NEWBIE)
			&& vtor (PREGAME_ROOM_PROTOTYPE))
		{
			to_room =
				clone_contiguous_rblock (vtor (PREGAME_ROOM_PROTOTYPE), -1);
			if ((troom = vtor (to_room)))
			{
				for (i = 0; pregame_furnishings[i] != -1; i++)
				{
					if ((obj = load_object (pregame_furnishings[i])))
						obj_to_room (obj, troom->nVirtual);
				}
				(d->character)->char_to_room(troom->nVirtual);
			}
			else
				(d->character)->char_to_room(OOC_LOUNGE);
		}//end newbie
		else
			(d->character)->char_to_room(OOC_LOUNGE);
	}
	else
	{
		(d->character)->char_to_room (d->character->in_room);
	}

	(d->character)->act("$n enters the area.", true, 0, 0, TO_ROOM);

	sprintf (buf, "%s last entered the game %s\n",
		(d->character)->name,
		d->character->pc->last_logon ?
		(char *) ctime (&d->character->pc->last_logon) : "never ");
	buf[strlen (buf) - 2] = '.';	/* gets rid of nl created by ctime */
	send_to_gods (buf);

	d->character->flags &= ~FLAG_BINDING;

	sprintf (buf, "save/player/%c/%s.a",
		tolower (*(d->character)->name), CAP ((d->character)->name));

	load_saved_mobiles (d->character, buf);
	*buf = '\0';
	
	if (d->character->pc->last_logoff)
	{
		offline_stamina (d->character, d->character->pc->last_logoff);
		offline_skill_train(d->character, d->character->pc->last_logoff);
	}

		
	if (d->character->room->nVirtual == LINKDEATH_HOLDING_ROOM)
	{
		(d->character)->char_from_room();
		(d->character)->char_to_room (d->character->last_room);
		d->character->last_room = 0;
		(d->character)->act ("$n enters the area.", true, 0, 0, TO_ROOM);
	}

	d->character->send_to_char("\n");

	//special message for characters who took a role
	if (d->character->pc->role)
	{
		outfit_new_char (d->character, d->character->pc->special_role);
		d->pending_message = new MESSAGE_DATA;
		d->pending_message->poster = duplicateString ((d->character)->name);
		d->pending_message->subject = duplicateString ("Special Role Selected in Chargen.");
		sprintf (buf,
			"Role Name: %s\n" "Role Cost: %d points\n" "Posted By: %s\n"
			"Posted On: %s\n" "\n" "%s\n",
			d->character->pc->special_role->summary,
			d->character->pc->special_role->cost,
			d->character->pc->special_role->poster,
			d->character->pc->special_role->date,
			d->character->pc->special_role->body);
		d->pending_message->message = duplicateString (buf);
		add_message_to_mysql_player_notes (d->character->name,
			d->character->name,
			d->pending_message);
	}

	do_look (d->character, "", 1);

	//normal start without role
	if (!str_cmp (d->character->room->name, PREGAME_ROOM_NAME))
	{
		d->character->send_to_char("\n");
		(d->character)->act
		("Welcome to Your Game World! If you are a new player and would like to speak one-on-one with an experienced player guide before beginning play, please type #6ASSIST REQUEST#0, and then #6HELP ASSIST#0. Otherwise, to enter Game World  itself and begin play, simply type #6COMMENCE#0.", false, 0, 0, TO_CHAR | _ACT_FORMAT);
	}
	

	if ((d->character->race >= 0 && d->character->race <= 29)
		&& (d->character->max_hit !=
		50 + (d->character->con * CONSTITUTION_MULTIPLIER)) + (MIN(d->character->aur,25) * 4))
	{
		d->character->max_hit = 50 + (d->character->con * CONSTITUTION_MULTIPLIER) + (MIN(d->character->aur,25) * 4);	// All humanoids are roughly the same,
		d->character->hit = d->character->max_hit;	// in terms of wound-endurance.
	}

	if (d->character->race == 28)//trolls - NPC
	{
		if (d->character->race == 28)
			d->character->max_hit =
			200 + (d->character->con * CONSTITUTION_MULTIPLIER) + (MIN(d->character->aur,25) * 4);
		d->character->hit = d->character->max_hit;
		d->character->armor = 2;
	}

	if (d->character->race == 86)//Olog-hai - PC
	{
		d->character->max_hit =
			200 + (d->character->con * CONSTITUTION_MULTIPLIER) + (MIN(d->character->aur, 25) * 4);
		d->character->hit = d->character->max_hit;
		d->character->armor = 3;
	}

	if (d->character->pc->level > 1)
		d->character->show_unread_messages();

	if (d->character->pc->creation_comment
		&& strlen (d->character->pc->creation_comment) > 2
		&& !d->character->pc->level)
	{

		current_time = time (0);
		ctime_r (&current_time,date);
		if (strlen (date) > 1)
			date[strlen (date) - 1] = '\0';

		d->pending_message = new MESSAGE_DATA;
		d->pending_message->poster = duplicateString ((d->character)->name);
		d->pending_message->subject = duplicateString ("Background Information.");
		d->pending_message->message =
			duplicateString (d->character->pc->creation_comment);
		d->pending_message->date = duplicateString (date);
		add_message_to_mysql_player_notes (d->character->name,
			d->character->name,
			d->pending_message);

		d->pending_message = new MESSAGE_DATA;
		d->pending_message->poster = duplicateString ((d->character)->name);
		d->pending_message->subject = duplicateString ("My Background.");
		d->pending_message->message =
			duplicateString (d->character->pc->creation_comment);
		post_to_mysql_journal (d);
		
		d->pending_message = new MESSAGE_DATA;
		d->pending_message->poster = duplicateString ((d->character)->name);
		d->pending_message->subject = duplicateString ("Reviewer Comments.");
		d->pending_message->message =
		duplicateString (d->character->pc->msg);
		post_to_mysql_journal (d);
		

		d->character->pc->creation_comment = NULL;
	}

	online = 0;
	guest = 0;

	for (td = descriptor_list; td; td = td->next)
	{

		if (!td->character)
			continue;

		if (!td->character->pc)
			continue;

		if (td->character->pc->level)
			continue;

		if (td->character->pc->create_state != 2)
			continue;

		if (td->connected)
			continue;

		if (IS_SET (td->character->flags, FLAG_GUEST))
			guest++;

		if ((!(td->character)->get_trust())
			&& !IS_SET (td->character->flags, FLAG_GUEST))
			online++;
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

	return;

}


void
spitstat (CHAR_DATA * ch, DESCRIPTOR_DATA * recipient)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	bool missing_info = false;
	bool pagebuf = false;
	int req = 0;

	sprintf (buf2, "%d inches", ch->height);

	sprintf (buf, "\nName: %s", ch->name);
	pad_buffer (buf, 25);
	sprintf (buf + strlen (buf), "Account: %s", ch->pc->account_name);
	pad_buffer (buf, 50);
	sprintf (buf + strlen (buf), "Race: %s", lookup_race_variable (ch->race, RACE_NAME));
	SEND_TO_Q (buf, recipient);
	*buf = '\0';

	sprintf (buf + strlen (buf), "\nGender: %s", sex_types[ch->sex]);
	pad_buffer (buf, 25);
	sprintf (buf + strlen (buf), "Age: %d", ch->age);
	pad_buffer (buf, 50);
	sprintf (buf + strlen (buf), "Build: %s, %s\n",
		ch->height == 1 ? "short" : ch->height ==
		2 ? "average" : ch->height == 3 ? "tall" : buf2,
		frames[ch->frame]);
	SEND_TO_Q (buf, recipient);
	*buf = '\0';

	if (!ch->keywords || !*ch->keywords || !str_cmp (ch->keywords, "(null)"))
	{
		sprintf (buf + strlen (buf), "\n#1Keywords:#0 %s\n\n", ch->keywords);
		missing_info = true;
	}
	else
		sprintf (buf + strlen (buf), "\nKeywords: %s\n\n", ch->keywords);
	if (!ch->short_descr || !*ch->short_descr
		|| !str_cmp (ch->short_descr, "(null)"))
	{
		sprintf (buf + strlen (buf), "#1Short Description:#0 %s\n", ch->short_descr);
		missing_info = true;
	}
	else
		sprintf (buf + strlen (buf), "Short Description: %s\n", ch->short_descr);
	if (!ch->long_descr || !*ch->long_descr
		|| !str_cmp (ch->long_descr, "(null)"))
	{
		sprintf (buf + strlen (buf), "#1Long Description:#0  %s\n\n", ch->long_descr);
		missing_info = true;
	}
	else
		sprintf (buf + strlen (buf), "Long Description:  %s\n\n", ch->long_descr);

	SEND_TO_Q (buf, recipient);

	if (!ch->description || !*ch->description
		|| !str_cmp (ch->description, "(null)"))
	{
		SEND_TO_Q ("#1Physical Description:#0\n", recipient);
		missing_info = true;
	}
	else
		SEND_TO_Q ("Physical Description:\n", recipient);

	if (ch->description)
	{
		SEND_TO_Q (ch->description, recipient);
	}

	*buf = '\0';

	if (!ch->pc) {
		ch->pc = new PC_DATA;
	}

		
	if (ch->pc->special_role 
		&& (!(recipient->character)->get_trust()))
		sprintf (buf + strlen (buf), "\nSelected Role:\n%s", ch->pc->special_role->body);
	else if (ch->pc->special_role && (recipient->character)->get_trust())
		sprintf (buf + strlen (buf), "Selected Role: %s\n", ch->pc->special_role->summary);

	if ((recipient->character)->get_trust())
	{
		if (ch->pc->app_cost)
			sprintf (buf + strlen (buf), "Application Cost: %d RP Points\n",
			ch->pc->app_cost);
		if (ch->pc->special_role)
			req =
			MAX (ch->pc->special_role->cost,
			atoi (lookup_race_variable (ch->race, RACE_RPP_COST)));
		else
			req = atoi (lookup_race_variable (ch->race, RACE_RPP_COST));
		if (req)
			sprintf (buf + strlen (buf), "Application Tier: %d RP Points\n", req);
	}

	SEND_TO_Q (buf, recipient);

	*buf = '\0';

	sprintf (buf + strlen (buf), "\nChosen Profession: %s\n",
		get_profession_name (ch->pc->profession));

	if (!ch->pc->creation_comment || !*ch->pc->creation_comment
		|| (ch->pc->creation_comment
		&& !str_cmp (ch->pc->creation_comment, "(null)")))
	{
		sprintf (buf + strlen (buf), "\n#1Background Comment:#0\n%s",
			ch->pc->creation_comment);
		missing_info = true;
	}
	else
	{
		pagebuf = true;
		//sprintf (buf + strlen (buf), "\nBackground Comment:\n%s", ch->pc->creation_comment);
	}

	if (missing_info 
		&& (!(recipient->character)->get_trust()))
		sprintf (buf + strlen (buf),
		"\n#1Items marked in red MUST be completed before you submit your application.#0\n");
	else if (missing_info 
			 && (recipient->character)->get_trust())
		sprintf (buf + strlen (buf),
		"\n#1Items marked in red were not completed by the applicant.#0\n");
	if (*buf)
		SEND_TO_Q (buf, recipient);
	if (pagebuf)
		page_string(recipient,ch->pc->creation_comment);

}
void
chargen_desc (DESCRIPTOR_DATA * d, char* argument)
{
	CHAR_DATA *ch;
	
	ch = d->character;
	
	if ((strncmp(argument,"@",1)==0) && strlen(argument)==1)
	{
		if (!d->descStr)
			ch->description = strdup("nothing");
		else 
			ch->description = strdup(d->descStr);
		
		ch->description = duplicateString((reformat_desc((char*)ch->description).c_str()));
		
		
		if (d->character->pc->nanny_state)
			d->character->pc->nanny_state = STATE_PROFESSION;
		
		d->connected = CON_CREATION;
		save_char(d->character, false);
	}
	else
	{
		d->connected = CON_COMPOSING_FDESC;
		ch->pc->nanny_state = STATE_FDESC;
			//otherwise append the line and keep going.
		string_add(d, argument);
		
			//indicate that it was received by sending another prompt
		SEND_TO_Q("> ",d);
	}
	
	return;
}

void
comment_creation (DESCRIPTOR_DATA * d, char* argument)
{
	CHAR_DATA *ch;
	
	ch = d->character;
	
	if ((strncmp(argument,"@",1)==0) && strlen(argument)==1)
	{
		if (!d->descStr)
			ch->pc->creation_comment = strdup("no comment");
		else 
			ch->pc->creation_comment = strdup(d->descStr);
		
		d->connected = CON_CREATION;
		ch->pc->nanny_state = 0;
		save_char(d->character, false);
	}
	else
	{
		d->connected = CON_COMPOSING_COMMENT;
		ch->pc->nanny_state = STATE_COMMENT;
			//otherwise append the line and keep going.
		string_add(d, argument);
		
			//indicate that it was received by sending another prompt
		SEND_TO_Q("> ",d);
	}
	
	return;
}

void
create_menu_options (DESCRIPTOR_DATA * d)
{
	CHAR_DATA *ch = d->character;
	
	if (!ch->name)
	{
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "NAME");
		return;
	}

	if (ch->pc->nanny_state == STATE_GENDER)
	{
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "SEX");
		return;
	}

	else if (ch->pc->nanny_state == STATE_RACE)
	{
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "RACE");
		return;
	}
	
	else if (ch->pc->nanny_state == STATE_AGE)
	{
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "AGE");
		return;
	}
	
	else if (ch->pc->nanny_state == STATE_ATTRIBUTES)
	{
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "ATTRIBUTES");
		return;
	}
	
	else if (ch->pc->nanny_state == STATE_FRAME)
	{
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "HEIGHT");
		return;
	}
	
	else if (ch->pc->nanny_state == STATE_SDESC)
	{
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "SHORT");
		return;
	}
	
	else if (ch->pc->nanny_state == STATE_LDESC)
	{
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "LONG");
		return;
	}
	
	else if (ch->pc->nanny_state == STATE_KEYWORDS)
	{
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "KEYWORDS");
		return;
	}
	
	else if (ch->pc->nanny_state == STATE_FDESC)
	{
		
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "DESCRIPTION");
		return;
	}
	
	else if (ch->pc->nanny_state == STATE_SPECIAL_ROLES)
	{
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "CLASSIFIEDS");
		return;
	}

	else if (ch->pc->nanny_state == STATE_SKILLS)
	{
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "SKILLS");
		return;
	}
	
		//not used currently
	else if (ch->pc->nanny_state == STATE_LOCATION)
	{
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "LOCATION");
		return;
	}
		//not used for closed beta
	else if (ch->pc->nanny_state == STATE_PROFESSION)
	{
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "PROFESSION");
		return;
	}

	else if (ch->pc->nanny_state == STATE_COMMENT)
	{
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "COMMENT");
		return;
	}
	
	if (ch->pc->create_state == STATE_SUBMITTED)
	{
		SEND_TO_Q ("\nCommands:  QUIT, CHECK\n\n> ", d);
		d->connected = CON_CREATION;
		return;
	}

	spitstat (ch, d);

	save_char (ch, false);

	SEND_TO_Q
		("\nCommands:  Age, Attributes, Classifieds, Comment, Description, Frame, Height,\n"
		"           Keywords, Location, Long, Profession, Quit, Race, Sex, Short, and Skills.\n",
		d);

	if (d->character->pc->msg 
		&& *d->character->pc->msg != '~'
		&& strlen (d->character->pc->msg) > 3)
	{
		SEND_TO_Q
			("\n#6Note:      Your application has been processed; type REVIEW to read the response.#0\n",
			d);
	}

	SEND_TO_Q
		("\n#2Note:      When ready, please use SUBMIT to finalize your application.\n#0",
		d);

	SEND_TO_Q ("\n> ", d);

	d->connected = CON_CREATION;
}

//Aura is a calculated stat, so they cannot assign values to it
//seven stats are STR, DEX, CON, WIL, INT, LUK & AGI
void
attribute_priorities (DESCRIPTOR_DATA * d, char *arg)
{
	int bonus;
	int attr;
	int i;
	int curr_attr;
	CHAR_DATA *ch = d->character;
	int attr_starters[7] = { 17, 15, 13, 11, 10, 9, 7 };
	int attr_priorities[7] = { -1, -1, -1, -1, -1, -1, -1 };
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char msg[MAX_STRING_LENGTH]= { '\0' };
	extern const char *attrs[9];
	
	
	if (!*arg)
	{
		SEND_TO_Q ("\n#2Please enter all attributes in descending "
				   "priority.\nExample:  STR DEX"
				   " CON WIL INT AGI LUK#0\n", d);
		d->connected = CON_CREATION;
		return;
	}

	
	for (i = 0; i < NUM_ATTRIBUTES-1; i++)
	{
		arg = one_argument (arg, buf);
		
		attr = index_lookup (attrs, buf);

		if (strn_cmp(buf, "save",4) && strn_cmp(buf, "quit",4))
		{		
			if (attr == -1)
			{
				sprintf (msg, "\n#1'%s' is not recognized as an attribute.\n#0",
						 buf);
				d->connected = CON_CREATION;
				SEND_TO_Q (msg, d);
				return;
			}
			
			if (attr_priorities[attr] != -1)
			{
				sprintf (msg, "The attribute '%s' is duplicated in your list.\n",
						 buf);
				SEND_TO_Q (msg, d);
				return;
			}
		}
		else 
			return;

		attr_priorities[attr] = i;
	}

	/* Add in bonus 8 attribute points randomly */

	bonus = 2;
	
	while (bonus)
	{
		attr = number (0, NUM_ATTRIBUTES-1);

		curr_attr = attr_priorities[attr];
		
		if (attr_starters[curr_attr] < 20)
		{ 
			if (attr_starters[curr_attr - 1] >= attr_starters[curr_attr])
			{
			attr_starters[curr_attr]++;
			bonus--;
			}
		}
	}

	/* Assign actual numbers */

	ch->str = attr_starters[attr_priorities[0]];
	ch->dex = attr_starters[attr_priorities[1]];
	ch->con = attr_starters[attr_priorities[2]];
	ch->wil = attr_starters[attr_priorities[3]];
	ch->intel = attr_starters[attr_priorities[4]];
	ch->agi = attr_starters[attr_priorities[5]];
	ch->luk = attr_starters[attr_priorities[6]];

	ch->pc->start_str = ch->str;
	ch->pc->start_dex = ch->dex;
	ch->pc->start_con = ch->con;
	ch->pc->start_wil = ch->wil;
	ch->pc->start_intel = ch->intel;
	ch->pc->start_agi = ch->agi;
	ch->pc->start_luk = ch->luk;

	ch->tmp_str = ch->str;
	ch->tmp_con = ch->con;
	ch->tmp_intel = ch->intel;
	ch->tmp_wil = ch->wil;
	ch->tmp_aur = ch->aur;
	ch->tmp_dex = ch->dex;
	ch->tmp_agi = ch->agi;
	ch->tmp_luk = ch->luk;
	
		//Aura depends on Race - For now, everyone is human
	ch->aur = number (1,6);
	ch->pc->start_aur = ch->aur;
	
	/* Reset skills */
	d->character->skill_map.clear();
	save_char(d->character, false);
		
}

void
sex_selection (DESCRIPTOR_DATA * d, char *arg)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	CHAR_DATA *ch = d->character;

	arg = one_argument (arg, buf);

	if (toupper (*buf) == 'M' || !str_cmp (buf, "male"))
	{
		ch->sex = SEX_MALE;
		save_char(d->character, false);	
	}
	else if (toupper (*buf) == 'F' || !str_cmp (buf, "female"))
	{
		ch->sex = SEX_FEMALE;
		save_char(d->character, false);
	}
	else if (strn_cmp(buf, "save",4) && strn_cmp(buf, "quit",4))
		SEND_TO_Q ("Please choose MALE or FEMALE.\n", d);
}

void
race_selection_screen (DESCRIPTOR_DATA * d)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	int i = 0, j = 0;
	char temp_buf[MAX_STRING_LENGTH] = {'\0'};

	SEND_TO_Q ("\n", d);
	SEND_TO_Q (get_text_buffer (NULL, text_list, "race_select"), d);

	mysql_safe_query
		("SELECT * FROM races WHERE pc_race = true ORDER BY name ASC");
	
	result = mysql_store_result (database);
	if (!result || !mysql_num_rows (result))
	{
		if (result != NULL)
			mysql_free_result (result);
		SEND_TO_Q ("No races defined in database - bypassing...\n", d);
		d->character->race = RACE_DEFAULT;
		d->connected = CON_CREATION;
		d->character->pc->nanny_state = STATE_AGE;
		create_menu_options (d);
		return;
	}

	i = 0;
	j = 0;

	while ((row = mysql_fetch_row (result)))
	{
		i++;
		j++;
		sprintf (temp_buf + strlen (temp_buf), "%2d.  #2%-20.20s#0", i,
			d->acct->get_rpp () >=
			atoi (row[RACE_RPP_COST]) ? row[RACE_NAME] : "");
		if (!(j % 3))
			sprintf (temp_buf + strlen (temp_buf), "\n");
	}

	if ((j % 3))
		sprintf (temp_buf + strlen (temp_buf), "\n");

	strcat (temp_buf, "\n#2Your Desired Race:#0 ");
	page_string (d, temp_buf);
	d->character->race = -1;
	d->connected = CON_RACE_SELECT;
	mysql_free_result (result);
}

void
nanny_race_confirm (DESCRIPTOR_DATA * d, char *arg)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int start_loc = 0;

	if (!*arg)
	{
		sprintf (buf, "\nDo you still wish to play a %s? [y/n] ",
			lookup_race_variable (d->character->race, RACE_NAME));
		SEND_TO_Q (buf, d);
		return;
	}

	arg[0] = toupper (arg[0]);

	if (*arg == 'Y')
	{
		d->character->skill_map.clear();
		

		d->connected = CON_CREATION;

		start_loc = num_starting_locs (d->character->race);

		if (start_loc > 1)
			d->character->pc->nanny_state = STATE_LOCATION;
		else
			d->character->pc->nanny_state = STATE_AGE;

		create_menu_options (d);

		if (atoi (lookup_race_variable (d->character->race, RACE_RPP_COST)) > 0)
			d->character->pc->app_cost += (int)(atof (lookup_race_variable (d->character->race, RACE_RPP_COST)) / 2.0 + 0.5);
		if (d->character->pc->app_cost == 1)
			d->character->pc->app_cost = 0;

		if (atoi (lookup_race_variable (d->character->race, RACE_MIN_AGE)) >
			d->character->age)
			d->character->age =
			atoi (lookup_race_variable (d->character->race, RACE_MIN_AGE));
		if (atoi (lookup_race_variable (d->character->race, RACE_MAX_AGE)) <
			d->character->age)
			d->character->age =
			atoi (lookup_race_variable (d->character->race, RACE_MAX_AGE));

		save_char(d->character, false);
		return;
	}
	else
	{
		race_selection_screen (d);
		return;
	}
}


void
nanny_char_name_confirm (DESCRIPTOR_DATA * d, char *arg)
{
	CHAR_DATA *tch;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (!*arg)
	{
		SEND_TO_Q ("\n#2Character generation aborted.#0\n", d);
		d->character->unload_pc();
		d->character = 0;
		display_main_menu (d);
		return;
	}

	if (!str_cmp(arg, "quit"))
	{
		SEND_TO_Q ("\n#2Character generation aborted.#0\n", d);
		d->character->unload_pc();
		d->character = 0;
		display_main_menu (d);
		return;
	}
	
	if (!strncasecmp (d->acct->name.c_str (), arg, d->acct->name.length ())
		|| !strncasecmp (arg, d->acct->name.c_str (), strlen (arg)))
	{
		if (arg[strlen (arg) - 1] != '!')
		{
			SEND_TO_Q
				("\n#1Your character name may not be similar to your account name.#0\n\n#1Please press ENTER to continue.#0\n",
				d);
			d->connected = CON_PLAYER_NEW;
			return;
		}
		else
		{
			arg[strlen (arg) - 1] = 0;
		}
	}

	if ((tch = load_pc (arg)))
	{
		SEND_TO_Q
			("\n#1That character name is already in use. Please press ENTER to continue.#0\n",
			d);
		d->connected = CON_PLAYER_NEW;
		tch->unload_pc();
		return;
	}

	if (strlen (arg) > MAX_NAME_LENGTH)
	{
		sprintf(buf, "Please choose a character name of less than %d characters.\n", MAX_NAME_LENGTH);
		
		SEND_TO_Q(buf, d);
		d->connected = CON_PLAYER_NEW;
		return;
	}

	if (strlen (arg) < 4)
	{
		sprintf(buf, "Please choose a longer character name with at least 4 letters.\n", MAX_NAME_LENGTH);
		
		SEND_TO_Q(buf, d);
		d->connected = CON_PLAYER_NEW;
		return;
	}
	
	for (size_t i = 0; i < strlen (arg); i++)
	{
		if (!isalpha (arg[i]))
		{
			SEND_TO_Q
				("'Illegal characters in character name (letters only, please).\n",
				d);
			d->connected = CON_PLAYER_NEW;
			return;
		}

		if (i)
			arg[i] = tolower (arg[i]);
		else
			arg[i] = toupper (arg[i]);
	}

	if (!*arg)
	{
		SEND_TO_Q ("What would you like to name your new character? ", d);
		d->connected = CON_PLAYER_NEW;
		return;
	}

	SEND_TO_Q ("\n#2Your character has been named. Press ENTER to continue.#0",
		d);

	arg[0] = toupper (arg[0]);
	d->character = new_char (1);

	d->character->name = duplicateString (arg);
	d->character->pc->create_state = STATE_APPLYING;
	d->character->race = -1;
	d->character->pc->account_name = duplicateString (d->acct->name.c_str());
    d->character->pc->account_id = d->acct->id;

	d->character->desc = d;

	d->character->short_descr = NULL;
	d->character->long_descr = NULL;
	d->character->description = NULL;

	d->character->time_str.birth = time (0);
	d->character->time_str.played = 0;
	d->character->time_str.logon = time (0);

	d->character->armor = 0;
	d->character->speaks = "Westron";
	
	d->character->affected_by = 0;

	d->character->intoxication = 0;
	d->character->thirst = 24;
	d->character->hunger = 2400;

	d->character->pc->load_count = 1;
	
	d->character->pc->nanny_state = STATE_GENDER;
	d->connected = CON_RACE;
	
	save_char (d->character, false);
}

void
nanny_special_role_selection (DESCRIPTOR_DATA * d, char *arg)
{
	ROLE_DATA *role;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int i, j;

	
	if (!*arg || !isdigit (*arg) || atoi (arg) < 0)
	{
		SEND_TO_Q
			("#2\nPlease select the number of one of the listed entries, or 0 to pass.#0\n\n",
			d);
		SEND_TO_Q ("Your Desired Role: ", d);
		return;
	}

		
	if (atoi (arg) == 0)
	{
		if (d->character->pc->nanny_state)
			d->character->pc->nanny_state = STATE_RACE;
		d->connected = CON_CREATION;
		if (d->character->pc->special_role)
		{
			d->character->pc->special_role = NULL;
		}
		create_menu_options (d);
		return;
	}

	i = atoi (arg);

	for (j = 1, role = role_list; role; role = role->next, j++)
	{
		if (role->cost > (d->acct->get_rpp ()))
		{
			j--;
			continue;
		}
		if (j == i)
			break;
	}

	if (!role)
	{
		SEND_TO_Q
			("#2\nPlease select the number of one of the listed entries, or 0 to pass.#0\n\n",
			d);
		SEND_TO_Q ("Your Desired Role: ", d);
		return;
	}

	d->character->pc->special_role = new ROLE_DATA;
	d->character->pc->special_role->next = NULL;
	d->character->pc->special_role->summary = duplicateString (role->summary);
	d->character->pc->special_role->body = duplicateString (role->body);
	d->character->pc->special_role->poster = duplicateString (role->poster);
	d->character->pc->special_role->date = duplicateString (role->date);
	d->character->pc->special_role->cost = role->cost;
	d->character->pc->special_role->id = role->id;

	SEND_TO_Q ("\n", d);
	sprintf (buf, "#2Role Contact:#0  %s\n", role->poster);
	sprintf (buf + strlen (buf), "#2Posted On:#0     %s\n\n", role->date);
	SEND_TO_Q (buf, d);
	SEND_TO_Q (role->body, d);
	SEND_TO_Q ("\n", d);
	SEND_TO_Q ("Do you still wish to choose this role? [y/n]", d);
	d->connected = CON_SPECIAL_ROLE_CONFIRM;
	return;
}

void
nanny_special_role_confirm (DESCRIPTOR_DATA * d, char *arg)
{

	if (!*arg)
	{
		SEND_TO_Q ("Do you still wish to choose this role? [y/n]", d);
		return;
	}

	
	
	arg[0] = toupper (arg[0]);

	if (*arg == 'Y')
	{
		d->character->pc->role = 1;  //they have taken a role
		
		if (d->character->pc->nanny_state)
			d->character->pc->nanny_state = STATE_RACE;
		d->connected = CON_CREATION;
		save_char(d->character, false);
		return;
	}
	else
	{
		d->connected = CON_SPECIAL_ROLE_SELECT;
		return;
	}
}

void
race_selection (DESCRIPTOR_DATA * d, char *arg)
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	int i = 0, abilities = 0;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char race_name[MAX_STRING_LENGTH]= { '\0' };
	char *p;

	mysql_safe_query
		("SELECT * FROM races WHERE pc_race = true ORDER BY name ASC");
	result = mysql_store_result (database);

	*race_name = '\0';

	d->character->plr_flags &= ~(START_ANGRENOST);

	while ((row = mysql_fetch_row (result)))
	{
		i++;
		if (i == atoi (arg))
		{
			sprintf (race_name, "%s", row[RACE_NAME]);
			break;
		}
	}

	if (!*race_name)
	{
		SEND_TO_Q ("Please select the number of one of the above races.\n", d);
		SEND_TO_Q ("\n#2Your Desired Race:#0 ", d);
		return;
	}

	if (atoi (row[RACE_RPP_COST]) > d->acct->get_rpp ())
	{
		SEND_TO_Q ("That race is currently unavailable to you.\n", d);
		SEND_TO_Q ("\n#2Your Desired Race:#0 ", d);
		return;
	}

	d->character->race = atoi (row[RACE_ID]);

	sprintf (buf, "%s", row[RACE_DESC]);
	if (buf && strlen (buf) > 1)
	{
		reformat_string (buf, &p);
		sprintf (buf2, "\n#2%s:#0\n\n%s", row[RACE_NAME], p);
		free_mem (p); //char*
	}
	else
	{
		sprintf (buf2, "\n#2%s:#0\n\nNo description provided.\n",
			row[RACE_NAME]);
	}

	if ((abilities = atoi (row[RACE_AFFECTS])) > 0)
	{
		sprintf (buf2 + strlen (buf2), "\n#2Innate Abilities:#0");
		if (IS_SET (abilities, INNATE_INFRA))
			sprintf (buf2 + strlen (buf2), " Infravision");
		if (IS_SET (abilities, INNATE_FLYING))
			sprintf (buf2 + strlen (buf2), " Flight");
		if (IS_SET (abilities, INNATE_WAT_BREATH))
			sprintf (buf2 + strlen (buf2), " WaterBreathing");
		if (IS_SET (abilities, INNATE_NOBLEED))
			sprintf (buf2 + strlen (buf2), " NoBleed");
		if (IS_SET (abilities, INNATE_SUN_PEN))
			sprintf (buf2 + strlen (buf2), " DaylightPenalty");
		if (IS_SET (abilities, INNATE_SUN_PET))
			sprintf (buf2 + strlen (buf2), " DaylightPetrification");
		sprintf (buf2 + strlen (buf2), "\n");
	}

	*buf = '\0';

	if (atoi (row[RACE_STR_MOD]) != 0)
	{
		if (atoi (row[RACE_STR_MOD]) > 0)
			sprintf (buf + strlen (buf), " +Str");
		else
			sprintf (buf + strlen (buf), " -Str");
	}

	if (atoi (row[RACE_CON_MOD]) != 0)
	{
		if (atoi (row[RACE_CON_MOD]) > 0)
			sprintf (buf + strlen (buf), " +Con");
		else
			sprintf (buf + strlen (buf), " -Con");
	}

	if (atoi (row[RACE_DEX_MOD]) != 0)
	{
		if (atoi (row[RACE_DEX_MOD]) > 0)
			sprintf (buf + strlen (buf), " +Dex");
		else
			sprintf (buf + strlen (buf), " -Dex");
	}

	if (atoi (row[RACE_AGI_MOD]) != 0)
	{
		if (atoi (row[RACE_AGI_MOD]) > 0)
			sprintf (buf + strlen (buf), " +Agi");
		else
			sprintf (buf + strlen (buf), " -Agi");
	}

	if (atoi (row[RACE_INT_MOD]) != 0)
	{
		if (atoi (row[RACE_INT_MOD]) > 0)
			sprintf (buf + strlen (buf), " +Int");
		else
			sprintf (buf + strlen (buf), " -Int");
	}

	if (atoi (row[RACE_WIL_MOD]) != 0)
	{
		if (atoi (row[RACE_WIL_MOD]) > 0)
			sprintf (buf + strlen (buf), " +Wil");
		else
			sprintf (buf + strlen (buf), " -Wil");
	}

	if (atoi (row[RACE_AUR_MOD]) != 0)
	{
		if (atoi (row[RACE_AUR_MOD]) > 0)
			sprintf (buf + strlen (buf), " +Aur");
		else
			sprintf (buf + strlen (buf), " -Aur");
	}

	if (*buf)
		sprintf (buf2 + strlen (buf2), "\n#2Attribute Modifiers:#0%s\n", buf);

	sprintf (buf2 + strlen (buf2), "\n#2Do you still wish to play a %s? [y/n]#0 ",
		race_name);

	page_string (d, buf2);

	d->connected = CON_RACE_CONFIRM;

	mysql_free_result (result);
}

void
age_selection (DESCRIPTOR_DATA * d, char *arg)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	CHAR_DATA *ch = d->character;
	int lower = 0, upper = 0;

	if (!strn_cmp(arg, "save",4) || !strn_cmp(arg, "quit",4))
	{
		chargen_save(d);
		return;
	}	
	
	d->character->age = 0;

	arg = one_argument (arg, buf);
	
	
	
	d->character->pc->nanny_state = STATE_AGE;


	if (lookup_race_variable (d->character->race, RACE_MIN_AGE))
		lower = atoi (lookup_race_variable (d->character->race, RACE_MIN_AGE));
	else
		lower = 15;

	if (lookup_race_variable (d->character->race, RACE_MAX_AGE))
		upper = atoi (lookup_race_variable (d->character->race, RACE_MAX_AGE));
	else
		upper = 55;

	if (atoi (buf) < lower || atoi (buf) > upper)
	{
		SEND_TO_Q ("\n#1Please select an age within the specified range.#0\n",
			d);
		d->connected = CON_AGE;
		return;
	}

	ch->age = atoi (buf);
	d->connected = CON_AGE;
	save_char(d->character, false);
}



void
location_selection (DESCRIPTOR_DATA * d, char *argument)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };

	
	argument = one_argument (argument, buf);

	if (!*buf || !isdigit (*buf) || (atoi (buf) < 1 || atoi (buf) > num_starting_locs(d->character->race)))
	{
		SEND_TO_Q ("#2Please select a number from the list above.#0\n", d);
		return;
	}

	int flags = atoi (lookup_race_variable (d->character->race, RACE_START_LOC));
	int i = 1;
	if (IS_SET (flags, RACE_HOME_ANGRENOST))
	{
		if (i++ == atoi(buf))
			d->character->plr_flags |= START_ANGRENOST;
	}
	
	if (d->character->pc->nanny_state)
		d->character->pc->nanny_state = STATE_AGE;

	d->connected = CON_CREATION;
}


void
sdesc_selection (DESCRIPTOR_DATA * d, char *argument)
{
	
	
	
	if (!*argument)
	{
		SEND_TO_Q ("#2Enter a short description, e.g. 'a towering, raven-haired man'.#0\n", d);
		d->character->short_descr = NULL;
		return;
	}
	
	if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
		return;
	
	d->character->short_descr = strdup(argument);
	
	if (d->character->pc->nanny_state)
		d->character->pc->nanny_state = STATE_LDESC;
	
	d->connected = CON_CREATION;
	save_char(d->character, false);
}

void
ldesc_selection (DESCRIPTOR_DATA * d, char *argument)
{
	
	if (!*argument)
	{
		SEND_TO_Q ("#2Enter a long description, e.g. 'A towering, raven-haired man stands here.'.#0\n", d);
		d->character->long_descr = NULL;
		return;
	}
	
	if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
		return;
	
	d->character->long_descr = strdup(argument);
	
	if (d->character->pc->nanny_state)
		d->character->pc->nanny_state = STATE_KEYWORDS;
	
	d->connected = CON_CREATION;
	save_char(d->character, false);
}

void
keyword_selection (DESCRIPTOR_DATA * d, char *argument)
{
	
	if (!*argument)
	{
		SEND_TO_Q ("#2Enter your keywords, e.g. 'towering raven man'.#0\n", d);
		d->character->keywords = NULL;
		return;
	}
	
	if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
		return;
	
	d->character->keywords = strdup(argument);
	
	if (d->character->pc->nanny_state)
		d->character->pc->nanny_state = STATE_FDESC;
	
	d->connected = CON_CREATION;
	save_char(d->character, false);
}


void
height_frame_selection (DESCRIPTOR_DATA * d, char *argument)
{
	int ind;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	
	
	argument = one_argument (argument, buf);

	if (!*buf)
	{
		SEND_TO_Q ("#2Enter a frame and height, e.g. 'tall light'.#0\n", d);
		d->character->height = 0;
		return;
	}

	if (strn_cmp(buf, "save",4) && strn_cmp(buf, "quit",4))
	{
		if (!str_cmp (buf, "short"))
			d->character->height = 1;
		else if (!str_cmp (buf, "average"))
			d->character->height = 2;
		else if (!str_cmp (buf, "tall"))
			d->character->height = 3;
		else
		{
			SEND_TO_Q ("#2\nEnter 'height frame', e.g. 'tall light'.#0\n", d);
			d->character->height = 0;
			return;
		}
	}
	else 
		return;
	

	
	argument = one_argument (argument, buf);

	if (!*buf)
	{
		SEND_TO_Q ("#2\nEnter a frame after the height, e.g. 'tall light'.#0\n",
			d);
		d->character->height = 0;
		return;
	}

	if (strn_cmp(buf, "save",4) && strn_cmp(buf, "quit",4))
	{
		ind = index_lookup (frames, buf);
		
		if (ind < 1 || ind >= FRAME_SUPER_MASSIVE)
		{
			SEND_TO_Q
			("Valid frames are scant, light, medium, heavy and massive.\n", d);
			d->character->height = 0;
			return;
		}
	}
	else
		return;
	
	d->character->frame = ind;

	if (d->character->pc->nanny_state)
		d->character->pc->nanny_state = STATE_SDESC;

	d->connected = CON_SDESC;
	save_char(d->character, false);
}



void
profession_display (DESCRIPTOR_DATA * d)
{
	int i;
	CHAR_DATA *ch;
	MYSQL_RES *result;
	MYSQL_ROW row;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	int nProfessions = 0;

	ch = d->character;

	
	SEND_TO_Q ("\n", d);
	SEND_TO_Q (get_text_buffer (NULL, text_list, "professions"), d);

	*buf = '\0';

	mysql_safe_query ("SELECT * FROM professions ORDER BY name ASC");
	if ((result = mysql_store_result (database)) == NULL)
	{
		sprintf (buf, "Warning: profession_display(): %s",
			mysql_error (database));
		system_log (buf, true);
	}
	else
	{
		nProfessions = mysql_num_rows (result);
	}

	if (nProfessions > 0)
	{

		i = 0;
		while ((row = mysql_fetch_row (result)))
		{
			i++;
			if (i < 10)
				sprintf (buf + strlen (buf), "%d.  #2%-18s#0    ", i,
				 row[0]);
			else
				sprintf (buf + strlen (buf), "%d. #2%-18s#0    ", i,
				 row[0]);
			if (!(i % 3))
				sprintf (buf + strlen (buf), "\n");
		}
		if ((i % 3))
			sprintf (buf + strlen (buf), "\n");

		sprintf (buf + strlen (buf), "\nYour Desired Profession: ");
		SEND_TO_Q (buf, d);
		d->connected = CON_PROFESSION;

	}
	else
	{
		SEND_TO_Q
			("There are no professions currently defined. Continuing...\n\n", d);
		d->connected = CON_CREATION;
		if (d->character->pc->nanny_state)
			d->character->pc->nanny_state = STATE_SKILLS;
		create_menu_options (d);
	}

	if (result)
	{
		mysql_free_result (result);
		result = NULL;
	}
	return;
}


void
profession_selection (DESCRIPTOR_DATA * d, char *argument)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	MYSQL_RES *result;
	MYSQL_ROW row;
	int i = 0, max = 0;

	if (!*argument)
	{
		profession_display (d);
		return;
	}

	
	
	d->character->pc->profession = -1;

	mysql_safe_query ("SELECT count(*) FROM professions");
	if ((result = mysql_store_result (database)) != NULL)
	{
		row = mysql_fetch_row (result);
		max = atoi (row[0]);
		mysql_free_result (result);
		result = NULL;
	}
	else
	{
		sprintf (buf, "Warning: profession_selection(): %s",
			mysql_error (database));
		system_log (buf, true);
		sprintf (buf,
			"\nPlease select a number from 1 to %d.\n\nYour Desired Profession: ",
			max);
		SEND_TO_Q (buf, d);
		return;
	}

	if (!isdigit (*argument) || atoi (argument) < 1 || atoi (argument) > max)
	{
		sprintf (buf,
			"\nPlease select a number from 1 to %d.\n\nYour Desired Profession: ",
			max);
		SEND_TO_Q (buf, d);
		return;
	}

	mysql_safe_query ("SELECT * FROM professions ORDER BY name ASC");
	if ((result = mysql_store_result (database)) == NULL)
	{
		sprintf (buf, "Warning: profession_selection(): %s",
			mysql_error (database));
		system_log (buf, true);
		sprintf (buf, "\nAn error occurred.\n\nYour Desired Profession: ");
		SEND_TO_Q (buf, d);
		return;
	}

	i = 0;

	while ((row = mysql_fetch_row (result)))
	{
		i++;
		if (i == atoi (argument))
		{
			d->character->pc->profession = atoi (row[2]);
			
			d->character->skill_map.clear();
			
			add_profession_skills (d->character, row[1]);
			break;
		}
	}

	mysql_free_result (result);
	result = NULL;

	d->connected = CON_CREATION;

	if (d->character->pc->nanny_state)
		d->character->pc->nanny_state = STATE_SKILLS;

	create_menu_options (d);

	save_char (d->character, false);

	return;
}


	
void
skill_selection (DESCRIPTOR_DATA * d, char *argument)
{
	std::string output;
	char *skill_name;
	char *count_skill_name;
	int picks_left = 0;
	CHAR_DATA *ch;
	SKILL_DATA* tskill;
	std::map<std::string, SKILL_DATA*>::iterator skill_map_it;
	std::map<std::string, int>::iterator found_it;

	
	if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
		return;
	
	
	ch = d->character;
	
	picks_left = PICKS_ENTITLED;

	
	if (!*argument)
	{
		skill_display (d);
		return;
	}

		//counts number of skill picks left
	for (skill_map_it = skill_data_map.begin(); skill_map_it != skill_data_map.end(); skill_map_it++)
	{
		tskill = skill_map_it->second;
		count_skill_name = strdup(tskill->skill_name.c_str());
		
			//find innate skills
		if(tskill->innate == 1)
		{
				//does character have skill?
			found_it = ch->skill_map.find(count_skill_name);
			if (found_it != ch->skill_map.end())
			{
				if (ch->skill_map[count_skill_name] > 0)
					picks_left--;
			}
		}
	}
	
	if (!str_cmp (argument, "done"))
	{
		if (picks_left)
		{
			skill_display (d);
			return;
		}
		
		
		d->connected = CON_CREATION;

		if (d->character->pc->nanny_state)
			d->character->pc->nanny_state = STATE_COMMENT;
		save_char (ch, false);
		create_menu_options (d);
		return;
	}

	*argument = toupper(*argument);
	skill_name = strdup(argument);
	skill_map_it = skill_data_map.find(argument);
	
	if (skill_map_it == skill_data_map.end())
	{
		SEND_TO_Q
			("\n#2Unknown skill.  Please pick one from the list. Skill pick is case sensitive.#0\n\nPress ENTER to continue...\n",
			d);
		d->connected = CON_SKILLS;
		return;
	}

	found_it = ch->skill_map.find(skill_name);
		//unpick a skill
	if (found_it != ch->skill_map.end())
	{
		ch->skill_map.erase(skill_name);
		save_char (ch, false);
		picks_left++;
	}
	
	else //picking the skill
	{
		open_skill (ch, lookup_skill_id(skill_name));
		save_char (ch, false);
		picks_left--;
	}
	
	if (picks_left < 0) 
	{
		SEND_TO_Q
			("\n#2You've picked too many.  Remove another skill by typing its name.#0\n\nPress ENTER to continue...\n",
			 d);
			d->connected = CON_SKILLS;
			return;
	}
	

	d->connected = CON_SKILLS;
	skill_display (d);
}


	//Innate skills only at this point
void
skill_display (DESCRIPTOR_DATA * d)
{
	int col = 0;
	int picks;
	CHAR_DATA *ch;
	std::string output;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char *skill_name;
	std::map<std::string, SKILL_DATA*>::iterator skill_map_it;
	std::map<std::string, int>::iterator found_skill;
	SKILL_DATA* tskill;
	
	ch = d->character;

	SEND_TO_Q ("\n", d);
	SEND_TO_Q (get_text_buffer (NULL, text_list, "skills_select"), d);

	picks = PICKS_ENTITLED;

	output.clear();
	
		//displays available skills
	for (skill_map_it = skill_data_map.begin(); skill_map_it != skill_data_map.end(); skill_map_it++)
	{
		tskill = skill_map_it->second;
		skill_name = strdup(tskill->skill_name.c_str());
			//does character have skill?
		found_skill = ch->skill_map.find(skill_name);
		
			//find inanate skills
		if(tskill->innate == 1)
		{
			if (found_skill != ch->skill_map.end())
			{
				if (ch->skill_map[skill_name] > 0)
				{
					sprintf (buf, "    #2Picked:#0  %-16s", skill_name);
					picks--;
				}
			}
			else
				sprintf (buf, "    Skill:  %-17s", skill_name);
			
			
			
			output.append(buf);
			if (col++ % 2)
				output.append("\n");
		}
	}
	
	SEND_TO_Q (output.c_str(), d);

	if (picks > 1)
		sprintf (buf, "\n\n%d picks remaining> ", picks);
	else if (picks == 1)
		sprintf (buf, "\n\n1 pick remaining> ");
	else
		sprintf (buf, "\n\nEnter DONE to finish or skill name> ");

	SEND_TO_Q (buf, d);

	d->connected = CON_SKILLS;
	 
}



void
create_menu_actions (DESCRIPTOR_DATA * d, char *arg)
{
	FILE *fp;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char temp_buf[MAX_STRING_LENGTH]= { '\0' };
	char key[MAX_STRING_LENGTH]= { '\0' };
	int i, block = 0;
	int start_loc = 0, req = 0;
	MYSQL_RES *result;
	CHAR_DATA *ch = d->character;
	CHAR_DATA *tch;
	ROLE_DATA *role;
	std::list<char_data*>::iterator tch_iterator;

	if (ch->pc->create_state == STATE_APPROVED)
	{
		create_menu_options (d);
		return;
	}

	arg = one_argument (arg, key);

	if (!*key)
	{
		SEND_TO_Q ("> ", d);
		return;
	}
	
 // A tiny little bit of cleanup on the keywords.  Make sure that the player's name is in the keyword list 

	if (ch->keywords && !isname (ch->name, ch->keywords))
	{
		sprintf (buf, "%s %s", ch->name, ch->keywords);
		ch->keywords = duplicateString (buf);
	}

	
	if (!str_cmp (key, "quit"))
	{
		d->character->unload_pc();
		d->character = NULL;
		display_main_menu (d);
		return;
	}

	
	else if (!str_cmp (key, "check"))
	{

		if (ch->pc->create_state == STATE_SUBMITTED)
		{
			SEND_TO_Q ("No response, as of yet. Please try back later.\n", d);
			SEND_TO_Q ("> ", d);
		}

		else if (ch->pc->create_state == STATE_APPLYING)
		{
			create_menu_options (d);
			return;
		}

		else if (ch->pc->create_state == STATE_REJECTED)
		{
			create_menu_options (d);
			d->connected = CON_CREATION;
			d->character->pc->create_state = STATE_APPLYING;
			return;
		}

		return;
	}
	
	else if (ch->pc->create_state == STATE_SUBMITTED)
	{
		SEND_TO_Q
		("Only the QUIT and CHECK commands function, now. Please hit ENTER.\n",
		 d);
		return;
	}
		//app has been declined and they want to see the message
	else if (!str_cmp (key, "review"))
	{
		if (d->character->pc->msg 
			&& *d->character->pc->msg != '~'
			&& strlen (d->character->pc->msg) > 3
			&& d->connected == CON_CREATION)
		{
			sprintf (buf, "%s\n", d->character->pc->msg);
			SEND_TO_Q (buf, d);
			SEND_TO_Q ("> ", d);
			return;
		}
		else
			SEND_TO_Q
			("\nNo response has been filed to your application at this point.\n\n> ",
			 d);
		return;
	}
	
	else if (!str_cmp (key, "name"))
	{
		SEND_TO_Q ("What would you like to name your character? ", d);
		d->connected = CON_NAME_CONFIRM;
		return;
	}

	else if (!str_cmp (key, "sex"))
	{
		strcpy (temp_buf, get_text_buffer (NULL, text_list, "sex_select"));
		strcat (temp_buf, "#2Your Character's Gender:#0 ");
		page_string (d, temp_buf);
		d->connected = CON_SEX;
		return;
	}
	
	else if (!str_cmp (key, "race"))
	{
		race_selection_screen (d);
		return;
	}

	else if (!str_cmp (key, "age"))
	{
		sprintf (temp_buf, "\n%s",
				 get_text_buffer (NULL, text_list, "age_select"));
		
		if (lookup_race_variable (d->character->race, RACE_MIN_AGE)
			&& lookup_race_variable (d->character->race, RACE_MAX_AGE))
			sprintf (temp_buf + strlen (temp_buf), "\n#2Your Desired Age (%d to %d):#0 ",
					 atoi (lookup_race_variable
						   (d->character->race, RACE_MIN_AGE)),
					 atoi (lookup_race_variable
						   (d->character->race, RACE_MAX_AGE)));
		else
			sprintf (temp_buf + strlen (temp_buf), "\n#2Your Desired Age (15 to 60):#0 "),
			SEND_TO_Q ("\n", d);
		SEND_TO_Q (temp_buf, d);
		d->connected = CON_AGE;
		return;
	}
	
	else if (!str_cmp (key, "attributes"))
	{
		SEND_TO_Q (get_text_buffer (NULL, text_list, "stat_message"), d);
		SEND_TO_Q ("\n#2Desired Attribute Order: #0", d);
		d->connected = CON_ATTRIBUTES;
		return;
	}

	else if (!str_cmp (key, "height") || !str_cmp (key, "frame"))
	{
		SEND_TO_Q ("\n", d);
		strcpy (temp_buf, get_text_buffer (NULL, text_list, "height_frame"));
		strcat (temp_buf, "\n#2Choose height and frame:#0 ");
		page_string (d, temp_buf);
		d->connected = CON_HEIGHT_WEIGHT;
		return;
	}
	
	else if (!str_cmp (key, "short"))
	{
		SEND_TO_Q (get_text_buffer (NULL, text_list, "new_sdesc"), d);
		SEND_TO_Q ("\n", d);
		
		if (ch->short_descr && str_cmp (ch->short_descr, ""))
		{
			sprintf (buf, "\nReplacing:  %s\n", ch->short_descr);
			SEND_TO_Q (buf, d);
		}
		
		SEND_TO_Q ("\n#2Short description:#0 ", d);
		ch->short_descr = NULL;
		
		
		d->connected = CON_SDESC;
		ch->desc->max_str = STR_ONE_LINE;
		return;
	}
	
	else if (!str_cmp (key, "long"))
	{
		SEND_TO_Q ("\n", d);
		SEND_TO_Q (get_text_buffer (NULL, text_list, "new_ldesc"), d);
		
		if (ch->long_descr && str_cmp (ch->long_descr, ""))
		{
			sprintf (buf, "\nReplacing:  %s\n", ch->long_descr);
			SEND_TO_Q (buf, d);
		}
		SEND_TO_Q ("\n#2Long description:#0 ", d);
		
		ch->long_descr = NULL;
		d->connected = CON_LDESC;
		ch->desc->max_str = STR_ONE_LINE;
		
		return;
	}
	
	else if (!str_cmp (key, "keywords"))
	{
		SEND_TO_Q ("\n", d);
		SEND_TO_Q (get_text_buffer (NULL, text_list, "new_keyword"), d);
		
		
		if (ch->keywords && str_cmp (ch->keywords, ""))
		{
			sprintf (buf, "%s (null)", ch->name);
			if (str_cmp (ch->keywords, buf))
			{
				sprintf (buf, "\nReplacing:  %s\n", ch->keywords);
				SEND_TO_Q (buf, d);
			}
			ch->keywords = NULL;
		}
					
		SEND_TO_Q ("\n#2Keywords:#0 ", d);

		d->connected = CON_KEYWORDS;
		ch->desc->max_str = STR_ONE_LINE;
		
		return;
	}


	else if (!str_cmp (key, "description"))
	{		
		if (d->connected != CON_COMPOSING_FDESC)
		{
			SEND_TO_Q (get_text_buffer (NULL, text_list, "new_desc"), d);
			
			if (ch->description && str_cmp (ch->description, ""))
			{
				sprintf (buf, "\nReplacing:\n\n%s\n", ch->description);
				SEND_TO_Q (buf, d);
			}
			ch->description = NULL;
			
			
			SEND_TO_Q ("\n> ", d);
			d->descStr = NULL;
			d->max_str = MAX_STRING_LENGTH;
			d->connected = CON_COMPOSING_FDESC;
			ch->pc->nanny_state = 0;
		}
		return;
	}

	else if (!str_cmp (key, "skills"))
	{
		if (d->character->race < 0)
		{
			SEND_TO_Q ("You'll need to choose a race first.\n", d);
			create_menu_actions (d, "RACE");
			return;
		}
					
		skill_display (d);
		return;
	}
	
	else if (!str_cmp (key, "classifieds"))
	{
		if (!available_roles (d->acct->get_rpp ()))
		{
			SEND_TO_Q
				("\n#2Sorry, but there are currently no posted roles available to you.#0\n\n> ",
				d);
			return;
		}
		SEND_TO_Q ("\n", d);
		SEND_TO_Q (get_text_buffer (NULL, text_list, "special_role_select"), d);
		if (d->character->pc->special_role)
		{
			d->character->pc->special_role = NULL;
		}
		*temp_buf = '\0';
		for (role = role_list, i = 1; role; role = role->next, i++)
		{
			if (role->cost > (d->acct->get_rpp ()))
			{
				i--;
				continue;
			}
			if (i < 10)
				sprintf (temp_buf + strlen (temp_buf), "%d.  #2%s#0\n", i,
				role->summary);
			else
				sprintf (temp_buf + strlen (temp_buf), "%d. #2%s#0\n", i,
				role->summary);
		}
		strcat (temp_buf, "\nYour Desired Role: ");
		page_string (d, temp_buf);
		d->connected = CON_SPECIAL_ROLE_SELECT;
		return;
	}

	
	else if (!str_cmp (key, "location"))
	{
		start_loc = num_starting_locs (d->character->race);
		if (start_loc < 2)
		{
			SEND_TO_Q ("This option is not available to your PC's race.\n\n>",
				d);
			return;
		}
		strcpy (temp_buf, get_text_buffer (NULL, text_list, "location"));
		int flags = atoi (lookup_race_variable (d->character->race, RACE_START_LOC));
		i = 1;
		std::string location_string;
		if (IS_SET (flags, RACE_HOME_ANGRENOST))
		{
			location_string = MAKE_STRING("   #2") + MAKE_STRING(i++) + MAKE_STRING("#0: The Tower of Angrenost in Gondor\n");
			strcat (temp_buf, location_string.c_str());
		}
		
		strcat (temp_buf, "\n#2Choose starting location:#0 ");
		page_string (d, temp_buf);
		d->connected = CON_LOCATION;
		return;
	}

	else if (!str_cmp (key, "profession"))
	{
		profession_display (d);
		return;
	}

	else if (!str_cmp (key, "comment"))
	{
		if (d->connected != CON_COMPOSING_COMMENT)
		{
			
			SEND_TO_Q (get_text_buffer (NULL, text_list, "comment_help"), d);
			
			if (ch->pc->creation_comment 
				&& (str_cmp(ch->pc->creation_comment, "(null)") 
				 || str_cmp(ch->pc->creation_comment, "")))
			{
				sprintf (buf, "Replacing:\n\n%s\n", ch->pc->creation_comment);
				SEND_TO_Q (buf, d);
			}
			
			ch->pc->creation_comment = NULL;
			
			SEND_TO_Q ("> ", d);
			d->descStr = NULL;
			d->max_str = MAX_STRING_LENGTH;
			d->character->pc->nanny_state = 0;
			d->connected = CON_COMPOSING_COMMENT;
		}
		return;
	}

	else if (!str_cmp (key, "submit"))
	{
		if (!ch->sex)
		{
			SEND_TO_Q
				("\n#2You need to select a gender. Please type SEX to do so.#0\n\n",
				d);
			SEND_TO_Q ("> ", d);
			return;
		}

		if (!ch->height)
		{
			SEND_TO_Q
				("\n#2You need to select a build. Please type HEIGHT to do so.#0\n\n",
				d);
			SEND_TO_Q ("> ", d);
			return;
		}

		if (ch->keywords && !*ch->keywords)
		{
			SEND_TO_Q
				("\n#2You need to supply a keyword list. Please type KEYWORDS to do so.#0\n\n",
				d);
			SEND_TO_Q ("> ", d);
			return;
		}

		if (ch->long_descr && !*ch->long_descr)
		{
			SEND_TO_Q
				("\n#2You need to supply a long description. Please type LONG to do so.#0\n\n",
				d);
			SEND_TO_Q ("> ", d);
			return;
		}

		if (ch->long_descr && !*ch->short_descr)
		{
			SEND_TO_Q
				("\n#2You need to supply a short description. Please type 'SHORT' to do so.#0\n\n",
				d);
			SEND_TO_Q ("> ", d);
			return;
		}

		if (ch->long_descr && !*ch->description)
		{
			SEND_TO_Q
				("\n#2You need to supply a description. Please type DESCRIPTION to do so.#0\n\n",
				d);
			SEND_TO_Q ("> ", d);
			return;
		}

		if (ch->pc && ch->pc->creation_comment && !*ch->pc->creation_comment)
		{
			SEND_TO_Q
				("\n#2You need to supply a background. Please type COMMENT to do so.#0\n\n",
				d);
			SEND_TO_Q ("> ", d);
			return;
		}
		mysql_safe_query
			("SELECT name,create_state"
			" FROM pfiles"
			" WHERE accountId = %d"
			" AND (create_state >= 1 AND create_state < 4)",
			d->acct->id);
		result = mysql_store_result (database);

		if (result && mysql_num_rows (result) >= 1)
			block++;

		if (result)
			mysql_free_result (result);

		if ((fp = fopen ("application_lock", "r")))
		{
			SEND_TO_Q
				("\n#2Sorry, but new character submissions are not being accepted\n"
				"at this time. Please try back later; thank you for your interest.#0\n",
				d);
			fclose (fp);
			display_main_menu (d);
			return;
		}

		if (block && !is_admin (d->acct->name.c_str ()))
		{
			SEND_TO_Q
				("\n#2Sorry, but you may only have one character submitted and/or in the\n"
				"game at any one time; you'll have to wait until your current character\n"
				"dies or is retired to submit an application for a new one.#0\n",
				d);
			save_char (d->character, false);
			display_main_menu (d);
			return;
		}

		req = 0;

		if (lookup_race_variable (ch->race, RACE_RPP_COST))
			req = atoi (lookup_race_variable (ch->race, RACE_RPP_COST));
		if (ch->pc->special_role)
			req = MAX (ch->pc->special_role->cost, req);

		if (d->acct->get_rpp () < req)
		{
			SEND_TO_Q
				("\n#2Sorry, but you do not seem to have the roleplay points required to\n"
				"submit the application for this particular character.#0\n", d);
			display_main_menu (d);
			return;
		}

		*ch->name = toupper (*ch->name);
		ch->pc->create_state = STATE_SUBMITTED;

		int	native_tongue = ch->get_native_tongue();
		if (native_tongue)
			ch->speaks = lookup_skill_name(native_tongue);

		if (num_starting_locs (ch->race) <= 1
			&& lookup_race_variable (ch->race, RACE_START_LOC))
		{
			int flags = strtol (lookup_race_variable (ch->race, RACE_START_LOC), NULL, 10);
			if (IS_SET (flags, RACE_HOME_ANGRENOST))
				ch->plr_flags |= START_ANGRENOST;
			
		}


		ch->in_room = NOWHERE;
		ch->time_str.played = 0;

		if (ch->long_descr)
		{
			if (ch->long_descr[strlen (ch->long_descr) - 1] != '.')
				strcat (ch->long_descr, ".");
			if (!isupper (*ch->long_descr))
				*ch->long_descr = toupper (*ch->long_descr);
			sprintf (buf, "%s", ch->long_descr);
			ch->long_descr = tilde_eliminator (buf);
		}

		if (ch->short_descr)
		{
			if (isupper (*ch->short_descr))
				*ch->short_descr = tolower (*ch->short_descr);
			if (ch->short_descr[strlen (ch->short_descr) - 1] == '.')
				ch->short_descr[strlen (ch->short_descr) - 1] = '\0';
			sprintf (buf, "%s", ch->short_descr);
			ch->short_descr = tilde_eliminator (buf);
		}

		if (ch->description)
		{
			sprintf (buf, "%s", ch->description);
			ch->description = tilde_eliminator (buf);
		}

		if (ch->keywords)
		{
			sprintf (buf, "%s", ch->keywords);
			ch->keywords = tilde_eliminator (buf);
		}

		if (ch->pc->creation_comment)
		{
			sprintf (buf, "%s", ch->pc->creation_comment);
			ch->pc->creation_comment = tilde_eliminator (buf);
		}

		for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
		{
			tch = *tch_iterator;

			if (!tch)
				continue;

			if (tch->deleted)
				continue;

			if (IS_NPC (tch))
				continue;

			if (tch->get_trust() > 3
				|| (is_newbie (ch) && tch->pc->is_guide))
			{
				sprintf (buf,
					"%s has submitted an application for a new character: %s.\n",
					ch->pc->account_name, ch->name);
				tch->send_to_char(buf);
			}
		}

		if (ch->pc->app_cost < 0)
			ch->pc->app_cost = 0;

		d->character->time_str.birth = time (0);
		d->character->time_str.played = 0;
		d->character->time_str.logon = time (0);
		save_char (d->character, false);
		
		SEND_TO_Q
			("\nThank you.  Your character application has been submitted.\n"
			"An administrator may be reviewing your application soon.\n", d);
		SEND_TO_Q
			("You will receive an email at the address registered for this\n"
			"account when the application has been reviewed, along with any\n"
			"comments the reviewing administrator wished to make.\n" "\n"
			"Character review generally takes anywhere from 24-48 hours,\n"
			"depending on the workload of our roleplay admins. We thank you\n"
			"in advance for your patience, and for your interest in Game\n"
			"World! We'll see you soon, in Game World.\n", d);
		
		display_main_menu (d);
		save_char (d->character, false);
		d->character->unload_pc();
		d->character = NULL;
		return;
	}

	else
		SEND_TO_Q ("Unknown keyword!\n", d);

	create_menu_options (d);
}
 
void
nanny (DESCRIPTOR_DATA * d, char *argument)
{
	
	
	switch (d->connected)
	{
		case CON_LOGIN:
			nanny_login_choice (d, argument);
			break;
		case CON_NEW_ACCT_NAME:
			nanny_new_account (d, argument);
			break;
		case CON_ACCT_POLICIES:
			nanny_account_policies (d, argument);
			break;
		case CON_ACCT_EMAIL:
			nanny_account_email (d, argument);
			break;
		case CON_EMAIL_CONFIRM:
			nanny_account_email_confirm (d, argument);
			break;
		case CON_ACCOUNT_SETUP:
			nanny_account_setup (d, argument);
			break;
		case CON_ENTER_ACCT_NME:
			nanny_ask_password (d, argument);
			break;
		case CON_PWDCHK:
			if (strlen(argument) <= 64)
				nanny_check_password (d, argument);
			break;
		case CON_PENDING_DISC:
			nanny_disconnect (d, argument);
			break;
		case CON_ACCOUNT_MENU:
			nanny_connect_select (d, argument);
			break;
		case CON_PWDNEW:
				//      nanny_change_password (d, argument);
				//      break;
		case CON_PWDGET:
			nanny_new_password (d, argument);
			break;
		case CON_PWDNCNF:
			nanny_conf_change_password (d, argument);
			break;
			/***email changes are through the user control panel in the forum now
			 case CON_CHG_EMAIL:
			 nanny_change_email (d, argument);
			 break;
			 case CON_CHG_EMAIL_CNF:
			 nanny_change_email_confirm (d, argument);
			 break;
			 ****************/
			
			
		case CON_READING_WAIT:
			nanny_reading_wait (d, argument);
			break;
			
		case CON_TERMINATE_CONFIRM:
			nanny_terminate (d, argument);
			break;
		case CON_RETIRE:
			nanny_retire (d, argument);
			break;
			
		case CON_CREATE_GUEST:
			nanny_create_guest (d, argument);
			break;
		case CON_MAIL_MENU:
			nanny_mail_menu (d, argument);
			break;
		case CON_COMPOSE_MAIL_TO:
			nanny_compose_mail_to (d, argument);
			break;
		case CON_COMPOSE_SUBJECT:
			nanny_compose_subject (d, argument);
			break;
		case CON_COMPOSE_MESSAGE:
			nanny_compose_message (d, argument);
			break;
		case CON_COMPOSING_MESSAGE:
			nanny_composing_message (d, argument);
			break;
		case CON_READ_MESSAGE:
			nanny_read_message (d, argument);
			break;
			
			/************* CHARGEN ************/
		case CON_DELETE_PC:
			nanny_delete_pc (d, argument);
			break;
			
		case CON_CHOOSE_PC:
			nanny_choose_pc (d, argument);
			break;
			
		case CON_PLAYER_NEW:
			d->connected = CON_CREATION;
			create_menu_options (d);
			break;
			
		case CON_NAME_CONFIRM:
			nanny_char_name_confirm (d, argument);
			break;
			
		case CON_SEX:
			sex_selection (d, argument);
			if (d->character->pc->nanny_state && d->character->sex)
			{
				if (!available_roles (d->acct->get_rpp ()))
					d->character->pc->nanny_state = STATE_RACE;
				else
					d->character->pc->nanny_state = STATE_SPECIAL_ROLES;
				d->connected = CON_CREATION;
			}
			
			if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
			{
				chargen_save(d);
				break;
			}
			create_menu_options (d);
			break;
			
		case CON_RACE:
			d->connected = CON_CREATION;
			if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
			{
				chargen_save(d);
				break;
			}
			create_menu_options (d);
			break;
			
		case CON_RACE_SELECT:
			race_selection (d, argument);
			if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
			{
				chargen_save(d);
				break;
			}
			break;
			
		case CON_RACE_CONFIRM:
			nanny_race_confirm (d, argument);
			break;
			
			
		case CON_LOCATION:
			location_selection (d, argument);
			if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
			{
				chargen_save(d);
				break;
			}
			create_menu_options (d);
			break;
			
		case CON_SPECIAL_ROLE_SELECT:
			nanny_special_role_selection (d, argument);
			if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
			{
				chargen_save(d);
				break;
			}
			
			break;
			
		case CON_SPECIAL_ROLE_CONFIRM:
			nanny_special_role_confirm (d, argument);
			if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
			{
				chargen_save(d);
				break;
			}
			create_menu_options (d);
			break;
			
		case CON_AGE:
			age_selection (d, argument);
			if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
			{
				break;
			}
			
			if (age(d->character).year)
			{
				d->character->pc->nanny_state = STATE_ATTRIBUTES;
				d->connected = CON_CREATION;
			}
			else
				d->connected = CON_AGE;
			
			create_menu_options (d);
			break;
			
			
		case CON_ATTRIBUTES:
			attribute_priorities (d, argument);
			if (d->character->pc->nanny_state && d->character->str)
			{
				d->character->pc->nanny_state = STATE_FRAME;
			}
			d->connected = CON_CREATION;
			
			if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
			{
				chargen_save(d);
				break;
			}
			create_menu_options (d);
			break;
			
		case CON_HEIGHT_WEIGHT:
			height_frame_selection (d, argument);
			if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
			{
				chargen_save(d);
				break;
			}
			create_menu_options (d);
			break;
			
			
		case CON_SDESC:
			sdesc_selection(d, argument);
			if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
			{
				chargen_save(d);
				break;
			}
			create_menu_options (d);
			break;
			
		case CON_LDESC:
			ldesc_selection(d, argument);
			if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
			{
				chargen_save(d);
				break;
			}
			create_menu_options (d);
			break;
			
		case CON_KEYWORDS:
			keyword_selection(d, argument);
			if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
			{
				chargen_save(d);
				break;
			}
			create_menu_options (d);
			break;
			
		case CON_FDESC:
			chargen_desc(d, argument);
			if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
			{
				chargen_save(d);
				break;
			}
			create_menu_options (d);
			break;
			
		case CON_COMPOSING_FDESC:
			if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
			{
				chargen_save(d);
				break;
			}
			chargen_desc(d, argument);
			create_menu_options (d);
			break;
			
		case CON_PROFESSION:
			profession_selection (d, argument);
			if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
			{
				chargen_save(d);
				break;
			}
			break;
			
		case CON_SKILLS:
			skill_selection (d, argument);
			if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
			{
				chargen_save(d);
				break;
			}
			break;
			
		case CON_COMMENT:
			comment_creation(d, argument);
			if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
			{
				chargen_save(d);
				break;
			}
			create_menu_options (d);
			break;
			
		case CON_COMPOSING_COMMENT:
			if (!strn_cmp(argument, "save",4) || !strn_cmp(argument, "quit",4))
			{
				chargen_save(d);
				break;
			}
			comment_creation(d, argument);
			create_menu_options (d);
			break;
			
		case CON_CREATION:
			
			create_menu_actions (d, argument);
			break;
			
			
		default:
			break;
			
	}
}

void read_motd(DESCRIPTOR_DATA * d)
{
	std::string msg_line;
	std::string output;

	std::ifstream fin( "MOTD" );

	if( !fin )
	{
		system_log ("The MOTD could not be found", true);
		return;
	}

	output.assign("\n\n");

	while( getline(fin, msg_line) )
	{
		output.append(msg_line);
	}

	fin.close();

	if (!output.empty())
	{
		output.append("\n");
		SEND_TO_Q (output.c_str(), d);
	}

	return;
}


void
email_acceptance (DESCRIPTOR_DATA * d)
{
	account *acct = NULL;
	CHAR_DATA *tch = NULL;
	time_t current_time;
	char *date;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char email[MAX_STRING_LENGTH]= { '\0' };
	
	current_time = time (0);
	date = (char *) asctime (localtime (&current_time));
	
	if (strlen (date) > 1)
		date[strlen (date) - 1] = '\0';
	
	tch = NULL;
	tch = load_pc (d->character->delay_who2);
	if (!tch)
		return;
	
	acct = new account (tch->pc->account_name);
	if (!acct->is_registered ()) 
	{
		delete acct;
		return;
	}
	
	if (!*d->descStr)
	{
		d->character->send_to_char("No email sent!\n");
		delete acct;
		if (tch)
			tch->unload_pc();
		return;
	}
	else if (!tch || !acct->is_registered ())
	{
		d->character->send_to_char
		("\nEmail response aborted; there was a problem loading this PC.\n");
		delete acct;
		if (tch)
			tch->unload_pc();
		return;
	}
	else
	{
		if (d->character->pc && (!(d->character)->get_trust())
			&& d->character->pc->is_guide)
		{
			sprintf (buf, "Greetings,\n"
					 "\n"
					 "   Thank you for your interest in %s! This is an automated\n"
					 "system notification sent to inform you that your application for a character\n"
					 "named %s has been ACCEPTED by a Guide, and that you may enter\n"
					 "Game World at your earliest convenience. We'll see you there!\n"
					 "\n"
					 "%s left the following comments regarding your application:\n"
					 "\n%s", MUD_NAME, tch->name, d->character->pc->account_name,
					 d->descStr);
			sprintf (email, "%s Player Guide <%s>", MUD_NAME,
					 d->acct->email.c_str ());
			send_email (acct, NULL, email, "Your Character Application",
						buf);
		}
		else
		{
			sprintf (buf, "Greetings,\n"
					 "\n"
					 "   Thank you for your interest in %s! This is an automated\n"
					 "system notification sent to inform you that your application for a character\n"
					 "named %s has been ACCEPTED by the reviewer, and that you may enter\n"
					 "Game World at your earliest convenience. We'll see you there!\n"
					 "\n"
					 "%s left the following comments regarding your application:\n"
					 "\n%s", MUD_NAME, tch->name, d->character->name,
					 d->descStr);
			sprintf (email, "%s <%s>", MUD_NAME, APP_EMAIL);
			send_email (acct, NULL, email, "Your Character Application",
						buf);
		}
		
		d->pending_message = new MESSAGE_DATA;
		d->pending_message->nVirtual = 0;
		d->pending_message->info = duplicateString ("");
		sprintf (buf, "#2Accepted:#0 %s: %s", tch->pc->account_name, tch->name);
		d->pending_message->subject = duplicateString (buf);
		d->pending_message->message = duplicateString (d->descStr);
		
		add_message (1, "Applications",
					 -5,
					 d->acct->name.c_str (),
					 date,
					 d->pending_message->subject,
					 d->pending_message->info,
					 d->pending_message->message, d->pending_message->flags);
		
		add_message (1, tch->name,
					 -2,
					 d->acct->name.c_str (),
					 date,
					 "Application Acceptance",
					 d->pending_message->info,
					 d->pending_message->message, d->pending_message->flags);
		
		sprintf (buf,
				 "\n#6Your application was accepted on its most recent review.\n\n%s left the following comment(s):#0\n"
				 "\n%s", d->acct->name.c_str (), d->pending_message->message);
		if (buf[strlen (buf) - 1] != '\n')
			strcat (buf, "\n");
		
		tch->pc->msg = duplicateString (buf);
		
		free_mem(d->pending_message);
		d->pending_message = NULL;
		if (d->descStr)
		{
			free_mem (d->descStr);
			d->character->pc->msg = NULL;
		}
		d->pending_message = NULL;
		
		mysql_safe_query
		("UPDATE newsletter_stats SET accepted_apps=accepted_apps+1");
		mysql_safe_query ("UPDATE professions SET picked=picked+1 WHERE id=%d",
						  tch->pc->profession);
		
		if (tch->pc->special_role)
		{
			sprintf (email, "%s Player <%s>", MUD_NAME, acct->email.c_str ());
			delete acct;
			acct = new account (tch->pc->special_role->poster);
			if (acct)
			{
				sprintf (buf, "Hello,\n\n"
						 "This email is being sent to inform you that a special role you placed\n"
						 "in chargen has been applied for and accepted. The details are attached.\n"
						 "\n"
						 "Please contact this player at your earliest convenience to arrange a time\n"
						 "to set up and integrate their new character. To do so, simply click REPLY;\n"
						 "their email has been listed in this message as the FROM address.\n\n"
						 "Character Name: %s\n"
						 "Role Description: %s\n"
						 "Role Post Date: %s\n"
						 "\n"
						 "%s\n\n", tch->name, tch->pc->special_role->summary,
						 tch->pc->special_role->date,
						 tch->pc->special_role->body);
				sprintf (buf2, "New PC: %s", tch->pc->special_role->summary);
				send_email (acct, APP_EMAIL, email, buf2, buf);
			}
		}
	}
	
	delete acct;
	d->character->delay_who = duplicateString (tch->name);
	if (d->character->delay_who2)
	{
		free_mem (d->character->delay_who2);
		d->character->delay_who2 = NULL;
	}
	d->character->delay_ch = NULL;
	save_char (tch, false);
	tch->unload_pc();
}

void
email_rejection (DESCRIPTOR_DATA * d)
{
	account *acct = NULL;
	CHAR_DATA *tch = NULL;
	time_t current_time;
	char *date;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char email[MAX_STRING_LENGTH]= { '\0' };
	
	current_time = time (0);
	date = (char *) asctime (localtime (&current_time));
	if (strlen (date) > 1)
		date[strlen (date) - 1] = '\0';
	
	tch = NULL;
	tch = load_pc (d->character->delay_who2);
	if (!tch)
		return;
	
	acct = new account (tch->pc->account_name);
	if (!acct->is_registered ())
	{
		delete acct;
		return;
	}
	
	if (!*d->descStr)
	{
		delete acct;
		d->character->send_to_char("No email sent!\n");
		if (tch)
			tch->unload_pc();
		return;
	}
	else if (!tch || !acct)
	{
		delete acct;
		d->character->send_to_char
		("\nEmail response aborted; there was a problem loading this PC.\n");
		if (tch)
			tch->unload_pc();
		return;
	}
	else
	{
		if (d->character->pc && (!(d->character)->get_trust())
			&& d->character->pc->is_guide)
		{
			sprintf (buf, "Greetings,\n"
					 "\n"
					 "   Thank you for your interest in %s! This is an\n"
					 "automated system notification to inform you that your application for\n"
					 "a character named %s was deemed inappropriate by a Guide, and\n"
					 "therefore was declined. However, don't despair! This is a relatively\n"
					 "common occurrence, and nothing to worry about. Your application has\n"
					 "been saved on our server, and you may make the necessary changes simply\n"
					 "by entering the game  as that character. You will be dropped back\n"
					 "into the character generation engine, where you may make corrections.\n"
					 "\n"
					 "   If you have any questions regarding the comments below, clicking\n"
					 "REPLY to this email will allow you to get in touch with the Guide\n"
					 "who reviewed your application. Please be civil if you do choose to\n"
					 "contact them - remember, they volunteer their free time to help you!\n\n"
					 "%s left the following comments regarding your application:\n"
					 "\n%s", MUD_NAME, tch->name, d->character->pc->account_name,
					 d->descStr);
			sprintf (email, "%s Player Guide <%s>", MUD_NAME,
					 d->acct->email.c_str ());
			send_email (acct, NULL, email, "Your Character Application",
						buf);
		}
		else
		{
			sprintf (buf, "Greetings,\n"
					 "\n"
					 "   Thank you for your interest in %s! This is an\n"
					 "automated system notification to inform you that your application for\n"
					 "a character named %s was deemed inappropriate by an administrator, and\n"
					 "therefore was declined. However, don't despair! This is a relatively\n"
					 "common occurrence, and nothing to worry about. Your application has\n"
					 "been saved on our server, and you may make the necessary changes simply\n"
					 "by 'Entering Game World' as that character. You will be dropped back\n"
					 "into the character generation engine, where you may make corrections.\n"
					 "\n"
					 "%s left the following comments regarding your application:\n"
					 "\n%s", MUD_NAME, tch->name, d->character->name,
					 d->descStr);
			sprintf (email, "%s <%s>", MUD_NAME, APP_EMAIL);
			send_email (acct, NULL, email, "Your Character Application",
						buf);
		}
		
		d->pending_message = new MESSAGE_DATA;
		d->pending_message->poster = duplicateString ("player_applications");
		d->pending_message->nVirtual = 0;
		d->pending_message->info = duplicateString ("");
		sprintf (buf, "#1Declined:#0 %s: %s", tch->pc->account_name, tch->name);
		d->pending_message->subject = duplicateString (buf);
		d->pending_message->message = duplicateString (d->descStr);
		
		add_message (1, "Applications",
					 -5,
					 d->acct->name.c_str (),
					 date,
					 d->pending_message->subject,
					 d->pending_message->info,
					 d->pending_message->message, d->pending_message->flags);
		
		free_mem(d->pending_message);
		d->pending_message = NULL;
				
		sprintf (buf,
				 "\n#6Unfortunately, your application was declined on its most recent review.\n\n%s left the following comment(s) explaining why:#0\n"
				 "\n%s", d->acct->name.c_str (), d->descStr);
		if (buf[strlen (buf) - 1] != '\n')
			strcat (buf, "\n");
		
		tch->pc->msg = duplicateString (buf);
		
		mysql_safe_query
		("UPDATE newsletter_stats SET declined_apps=declined_apps+1");
	}
	

	delete acct;
	
	d->character->delay_who = duplicateString (tch->name);
	if (d->character->delay_who2)
	{
		free_mem (d->character->delay_who2);
		d->character->delay_who2 = NULL;
	}
	d->character->delay_ch = NULL;
	
	if (d->descStr)
	{
		free_mem (d->descStr);
		d->character->pc->msg = NULL;
	}
	d->pending_message = NULL;
	
	save_char (tch, false);
	tch->unload_pc();
}

void
check_psionic_talents (CHAR_DATA * ch)
{
	int chance = 0, roll = 0;
	int cur_talents = 0, i = 0, j = 1;
	int talents[8] = { 
		lookup_skill_id("Clairvoyance"),
		lookup_skill_id("Dager-sense"),
		lookup_skill_id("Empathic-heal"),
		lookup_skill_id("Hex"),
		lookup_skill_id("Mental-bolt"),
		lookup_skill_id("Prescience"),
		lookup_skill_id("Sensitivity"),
		lookup_skill_id("Telepathy")
	};
	bool check = true, again = true, awarded = false, block = false;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char *date;
	time_t time_check;
	
	if (is_newbie (ch) || ch->aur < 16)
	{
		return;
	}
	
	if (ch->pc && ch->pc->account_name)
	{
		if (!account::is_psionic_capable (ch->pc->account_name))
		{
			block = true;
		}
		
			// Psi tends to make ImmPCs foreground rather than background
		if (GET_FLAG (ch, FLAG_ISADMIN))
		{
			block = true;
		}
	}
	else
	{
		return;
	}
	
	
	if (block)
	{
		return;
	}
	
	if (ch->aur == 16)
		chance = 5;
	else if (ch->aur == 17)
		chance = 10;
	else if (ch->aur == 18)
		chance = 20;
	else if (ch->aur == 19)
		chance = 30;
	else if (ch->aur == 20)
		chance = 45;
	else if (ch->aur == 21)
		chance = 50;
	else if (ch->aur == 22)
		chance = 60;
	else if (ch->aur == 23)
		chance = 70;
	else if (ch->aur == 24)
		chance = 80;
	else
		chance = 95;
	
	chance += number (1, 10);
	chance = MIN (chance, 95);
	
	for (i = 0; i <= 7; i++)
		if (ch->skill_map[lookup_skill_name(talents[i])])
			cur_talents++;
	
	while (check && cur_talents <= 4)
	{
		if (number (1, 100) <= chance)
		{
			again = true;
			while (again)
			{
				roll = talents[number (0, 7)];
				if (ch->skill_map[lookup_skill_name(roll)])
				{
					ch->skill_map[lookup_skill_name(roll)] = 1;
					cur_talents++;
					again = false;
					awarded = true;
				}
				chance /= 2;
			}
			if (cur_talents >= 4)
				check = false;
		}
		else
			check = false;
	}
	
	if (!awarded)
		return;
	
	sprintf (buf,
			 "This character rolled positive for the following talents:\n\n");
	
	for (i = 0; i <= 7; i++)
		if (ch->skill_map[lookup_skill_name(talents[i])])
			sprintf (buf + strlen (buf), "   %d. %s\n", j++, lookup_skill_name(talents[i]));
	
	time_check = time (0);
	date = (char *) asctime (localtime (&time_check));
	date[strlen (date) - 1] = '\0';
	
	add_message (1, "Psi_talents", -5, "Server", date, ch->name, "", buf, 0);
	add_message (1, ch->name, -2, "Server", date, "Psionic Talents.", "", buf,
				 0);
	
	free_mem (date);
}

void
starting_skill_boost (CHAR_DATA * ch, char *skill_name)
{
		 
	if (ch->skill_map.find(skill_name) != ch->skill_map.end())
		ch->skill_map[skill_name] += ch->tmp_aur + number (10, 20);
			
	else
	{
		open_skill(ch, lookup_skill_id(skill_name));
		ch->skill_map[skill_name] += ch->tmp_aur + number (10, 20);
	}
	
}




	//cmd = 345 is accepting the applicatation
	//cmd = 0 is declining application
void
answer_application (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch, *tmp_ch;
	char buf[MAX_INPUT_LENGTH];
	char name[MAX_INPUT_LENGTH];
	long int time_elapsed = 0;
	std::list<char_data*>::iterator tch_iterator;
	
	argument = one_argument (argument, buf);
	
	if (!ch->delay_who || !*ch->delay_who)
	{
		ch->send_to_char("Please REVIEW an application first.\n");
		return;
	}
	
	
		//will be used to hold the response to the acceptance
	if (ch->pc->msg)
	{
		free_mem (ch->pc->msg);
		ch->pc->msg = NULL;
	}
	
	sprintf (name, "%s", ch->delay_who);
	
	free_mem (ch->delay_who);
	ch->delay_who = NULL;
	ch->delay = 0;
	
	if (!(tch = load_pc (name)))
	{
		ch->send_to_char("Couldn't find PC...maybe a pfile corruption?\n");
		return;
	}
	
	while (tch->pc->load_count > 1)
		tch->unload_pc();
	
	for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
	{
		tmp_ch = *tch_iterator;
		if (tmp_ch->deleted)
			continue;
		if (tmp_ch->pc && tmp_ch->pc->edit_player
			&& !str_cmp (tmp_ch->pc->edit_player->name, tch->name))
		{
			tmp_ch->send_to_char
			("The PC in your editor has been closed for an application response.\n");
			tmp_ch->pc->edit_player = NULL;
		}
	}
	
	if (tch->pc->create_state != 1)
	{
		sprintf (buf,
				 "It appears that this application has already been reviewed and responded to.\n");
		ch->send_to_char(buf);
		tch->unload_pc();
		return;
	}
	
	ch->delay_who = duplicateString (tch->pc->account_name);
	ch->delay_who2 = duplicateString (tch->name);
	
		//declining
	if (cmd != 345)
	{
		ch->send_to_char
		("\n#2Please enter the changes necessary for this application to be approved.\n#0");
		ch->send_to_char
		("#2This will be sent via email to the player; terminate with an '@' when finished.#0\n");
		ch->make_quiet();
		
		ch->desc->descStr = duplicateString(ch->pc->msg);
		ch->desc->max_str = MAX_STRING_LENGTH;
		ch->desc->proc = email_rejection;
	}
	
	else if (cmd == 345)
	{
		ch->send_to_char("\n#2Please enter any comments or advice you have regarding this approved character.\n#0");
		ch->send_to_char("#2This will be sent via email to the player; terminate with an '@' when finished.#0\n");
		ch->make_quiet();
		ch->desc->descStr = duplicateString(ch->pc->msg);
		ch->desc->max_str = MAX_STRING_LENGTH;
		ch->desc->proc = email_acceptance;
	}
	
	ch->pc->msg = NULL;
	
	if (tch->pc->create_state > STATE_SUBMITTED)
	{
		sprintf (buf, "PC is currently in create state %d.\n",
				 tch->pc->create_state);
		ch->send_to_char(buf);
		tch->unload_pc();
	}
	
		// Acceptance; process the new PC for entry into the game
	
	if (cmd == 345)
		tch->setup_new_character();
	else if (cmd != 345)
		tch->pc->create_state = STATE_REJECTED;
	
	time_elapsed = time (0) - tch->time_str.birth;
	
	mysql_safe_query ("INSERT INTO application_wait_times (wait_time) "
					  "VALUES (%d)",
					  (int) time_elapsed);
	mysql_safe_query ("DELETE FROM reviews_in_progress WHERE char_name = '%s'",
					  tch->name);
	
	tch->unload_pc();
}

void
remove_guest_skills (CHAR_DATA * ch)
{
	if (!IS_SET (ch->flags, FLAG_GUEST))
		return;
	
	ch->skill_map.clear();
	
}