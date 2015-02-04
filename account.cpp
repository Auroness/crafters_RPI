/*----------------------------------------------------------------------------\
|  account.cpp : Informational Module  	            www.middle-earth.us      |
|  Copyright (C) 2009, Shadows of Isildur: Kite	                             |
|  Derived under license from DIKU GAMMA (0.0).                               |
\----------------------------------------------------------------------------*/


#include <ctype.h>
#include <stdlib.h>
#include </usr/local/include/mysql++/mysql++.h>

#include "protos.h"
#include "utils.h"
#include "structs.h"
#include "account.h"
#include "server.h"

extern mysqlpp::Connection dbo;

account::account (const char *acct_name)
{
	mysqlpp::Query query = dbo.query();
	char error_message[ERR_STRING_LENGTH] = "\0";
	
	initialize ();
	
	if (!acct_name || !*acct_name)
	{
		return;
	}
	
	if (str_cmp (acct_name, "Anonymous") == 0)
	{
		return;
	}
	
	query << "SELECT * FROM users WHERE user_name = '" << acct_name << "'";
	
	mysqlpp::StoreQueryResult res = query.store();
	if ( res.num_rows() > 0 ) 
	{
		mysqlpp::Row row = res[0];
		
		roleplay_points = strtol (row["roleplay_points"], 0, 10);
		set_password (row["user_password"]);
		set_last_ip (row["user_last_ip"]);
		created_on = strtol (row["user_regdate"], 0, 10);
		set_email (row["user_email"]);
		color = strtol (row["user_color"], 0, 10);
		flags = strtol (row["account_flags"], 0, 10);
		last_rpp = strtol (row["last_rpp"], 0, 10);
		id = strtol (row["user_id"], 0, 10);
		set_name (row["user_name"]);
	}
	else
	{
		sprintf (error_message, "Warning: account__load: %s", acct_name);
	}
	
	
	if (*error_message)
	{
		system_log (error_message, true);
	}
}

void
setup_new_account (account  *acct)
{
	char *encrypted;
	char *password;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char email[MAX_STRING_LENGTH]= { '\0' };
	
	password = duplicateString (generate_password (1, (char **) "8"));
	
	sprintf (buf, "Greetings,\n"
			 "\n"
			 "Thank you for your interest in our community! This\n"
			 "was sent to inform you that your login account has been\n"
			 "created on our server; please don't respond to this email,\n"
			 "as it is merely an automated system notification.\n"
			 "\n"
			 "MUD Account: %s\n"
			 "Password: %s\n"
			 "\n"
			 "Also, note that this username and account combination will allow\n"
			 "you to log into our web-based discussion forum, located at:\n\n"
			 "http://yourwesite/forums/\n"
			 "\n"
			 "Feel free to hop on in and join in our discussions!\n"
			 "\n"
			 "Best of luck, and again, welcome to %s.\n\n\n"
			 "                                 Warmest Regards,\n"
			 "                                 The Admin Team\n", acct->name.c_str (),
			 password, MUD_NAME);
	
	encrypted = encrypt_buf (password);
	acct->set_password (encrypted);
	free_mem (encrypted); // char* from crypt
	
	acct->newsletter = true;
	
	sprintf (email, "%s <%s>", MUD_NAME, MUD_EMAIL);
	sprintf (buf2, "Welcome to %s!", MUD_NAME);
	
	send_email (acct, NULL, email, buf2, buf);
	
	
	std::string escaped_name;
	std::string escaped_password;
	std::string escaped_email;
	std::string escaped_last_ip;
	
	acct->get_name_sql_safe (escaped_name);
	acct->get_password_sql_safe (escaped_password);
	acct->get_email_sql_safe (escaped_email);
	acct->get_last_ip_sql_safe (escaped_last_ip);
	
	mysql_set_server_option(database,MYSQL_OPTION_MULTI_STATEMENTS_ON);
		//changed for auroness version
	std::string insert_query =
	"INSERT INTO users "
	"(user_regdate, user_name, user_password, user_email, user_last_ip) "
	"VALUES(UNIX_TIMESTAMP(NOW()), "
	"'" + escaped_name + "', "
	"'" + escaped_password + "', "
	"'" + escaped_email + "', "
	"'" + escaped_last_ip + "');";
	
	mysql_safe_query ((char *)insert_query.c_str ());
	
	do
	{
		/* Process all results */
		MYSQL_RES *result;
		if (!(result= mysql_store_result (database)))
		{
			fprintf (stderr, "Got fatal error processing query: %s\n", mysql_error (database));
		}
		else
		{
			mysql_free_result (result);
		}
	}
	while (!mysql_next_result (database));
	
	mysql_set_server_option (database,MYSQL_OPTION_MULTI_STATEMENTS_OFF);
}

void
send_email (account * to_acct, const char *cc, char *from, char *subject,
			char *message)
{
	FILE *fp;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	
	if (!to_acct)
		return;
	
	if (to_acct->email.empty ())
		return;
	
	if (!strchr (to_acct->email.c_str (), '@'))
		return;
	
	if (!*subject)
		return;
	
	if (!*message)
		return;
	
	if (!*from)
		return;
	
	sprintf (buf, "%s -t", PATH_TO_SENDMAIL);
	
	fp = popen (buf, "w");
	if (!fp)
		return;
	fprintf (fp, "From: %s\n", from);
	fprintf (fp, "To: %s\n", to_acct->email.c_str ());
	if (cc != NULL && *cc)
		fprintf (fp, "Cc: %s\n", cc);
	fprintf (fp, "X-Sender: %s\n", MUD_EMAIL);
	fprintf (fp, "Mime-Version: 1.0\n");
	fprintf (fp, "Content-type: text/plain;charset=\"us-ascii\"\n");
	fprintf (fp, "Organization: %s\n", MUD_NAME);
	fprintf (fp, "Subject: %s\n", subject);
	fprintf (fp, "\n");
	fprintf (fp, "%s", message);
	
		pclose (fp);
	
}

/*! \class account account.h "inc/account.h"
 *  \brief This class contains player account information.
 *
 * Basic inforamtion for the player account is pulled from the database, and assigned to the class variables. 
 */
