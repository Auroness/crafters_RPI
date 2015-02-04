//////////////////////////////////////////////////////////////////////////////
//
/// commerce.c : shopkeeper commerce routines 
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
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include </usr/local/mysql/include/mysql.h>
#include <unistd.h>
#include <sys/stat.h>

#include "server.h"
#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "group.h"
#include "utility.h"


#define VNPC_COPPER_PURSE		200
#define MAX_INV_COUNT			32

extern rpie::server engine;
extern const char *month_name[12];

int
skill_roll (int ability)
{
	int r;
	int roll_result;
	
	if (ability < 5)
		ability = 5;
	
	r = number (1, MAX(100, ability));
	
	if (r > ability)
		roll_result = MOD_FAIL - ((r % 5) ? 0 : 1);
	else
		roll_result = MOD_SUCCESS + ((r % 5) ? 0 : 1);
	
	return roll_result;
}


// Unified function to calculate the sale price of a given object based on
// the shopkeeper and any negotiations by a CHAR_DATA, if specified.

// Sell argument is set to True when PC is selling (NPC is buying), and False when
// NPC is selling (PC is buying).

// Results are not rounded for individual list price, but they are rounded
// when an item is purchased or sold.

float
calculate_sale_price (OBJ_DATA * obj, CHAR_DATA * keeper, CHAR_DATA * ch,
					  int quantity, bool round_result, bool sell)
{
	OBJ_DATA *tobj;
	NEGOTIATION_DATA *neg;
	float val_in_coppers = 0;
	float markup = 1.0;
	float markup2 = 1.0;

	if (!keeper || !obj)
		return -1;

	// Calculate costs of item; use econ_markup() for sale to PC, econ_discount()
	// for purchase from NPC or for sale to PC of previously used item.

	// Calculate added cost of any drink in container.
	/**
	 capacity - od.value[0]
	 volume - od.value[1]
	 contents/liquid - od.value[2]
	 **/
	
	if (((obj->obj_flags.type_flag == ITEM_DRINKCON) || (obj->obj_flags.type_flag == ITEM_DRYCON))
		&& obj->o.od.value[1] > 0
		&& (tobj = vtoo (obj->o.od.value[2])))
	{
		if (!IS_SET (obj->obj_flags.extra_flags, ITEM_PC_SOLD) && !sell)
			markup2 = econ_markup (keeper, tobj);
		else
			markup2 = econ_discount (keeper, tobj);
		val_in_coppers += tobj->coppers * obj->o.od.value[1] * markup2;
	}

	// Calculate cost of main item.

	if (!IS_SET (obj->obj_flags.extra_flags, ITEM_PC_SOLD) && !sell)
		markup = econ_markup (keeper, obj);
	else //value for selling an item
		markup = econ_discount (keeper, obj);

	if (obj->obj_flags.set_cost > 0)
	{
		val_in_coppers = ((float)(obj->obj_flags.set_cost)/100.0) * quantity;
	}
	else if (obj->obj_flags.set_cost < 0)
	{
		val_in_coppers = 0.0f;
	}
	else
	{
		val_in_coppers += obj->coppers * markup * quantity;
	}

	/* if PC, check for possible negotiations and adjust cost */
	if (ch != NULL)
	{
		if (!sell)
		{			// PC buying item
			for (neg = keeper->mob->shop->negotiations; neg; neg = neg->next)
			{
				if (neg->ch_coldload_id == ch->coldload_id && neg->obj_vnum == obj->nVirtual)
					break;
			}
		}
		else
		{			// PC selling item
			for (neg = keeper->mob->shop->negotiations; neg; neg = neg->next)
			{
				if (neg->ch_coldload_id == ch->coldload_id && neg->obj_vnum == obj->nVirtual && !neg->true_if_buying)
					break;
			}
		}

		if (neg && neg->price_delta)
			val_in_coppers = val_in_coppers * (100.0 + neg->price_delta) / 100.0 ; //fixed formula

	}
	//ch != NULL

	if (round_result)
	{
		if (!sell)
			val_in_coppers = (int) (ceilf (val_in_coppers));
		else
			val_in_coppers = (int) (floorf (val_in_coppers));
		return val_in_coppers;
	}
	else
		return val_in_coppers;

}


void
refresh_colors (CHAR_DATA * keeper)
{
	ROOM_DATA *room;
	OBJ_DATA *tobj, *next_obj;
	int i = 0, j = 0, reload_objs[500];

	if (!IS_NPC (keeper) || !IS_SET (keeper->flags, FLAG_KEEPER)
		|| !keeper->mob->shop)
		return;

	if (!(room = vtor (keeper->mob->shop->store_vnum)))
		return;

	for (tobj = room->contents; tobj; tobj = next_obj)
	{
		next_obj = tobj->next_content;
		if (keeper_makes (keeper, tobj->nVirtual)
			&& IS_SET (tobj->obj_flags.extra_flags, ITEM_VARIABLE)
			&& !number (0, 1))
		{
			reload_objs[i] = tobj->nVirtual;
			i++;
			extract_obj (tobj);
		}
	}

	if (i)
	{
		for (j = 0; j < i; j++)
		{
			tobj = load_object (reload_objs[j]);
			if (tobj)
				obj_to_room (tobj, room->nVirtual);
		}
	}
}

/* return how much of purse was spent (returning all stops further calls) */
int
vnpc_customer (CHAR_DATA * keeper, int purse)
{
	ROOM_DATA *room;
	OBJ_DATA *tobj;
	int items_in_list = 0, target_item = 0, i = 0;
	int delivery_cost = 0;
	int val_in_coppers=0;

	if (!IS_NPC (keeper) || !IS_SET (keeper->flags, FLAG_KEEPER)
		|| !keeper->mob->shop)
		return purse;

	if (IS_NPC (keeper) && IS_SET (keeper->mob->action, ACT_NOVNPC))
		return purse;

	if (!(room = vtor (keeper->mob->shop->store_vnum)))
		return purse;

	if (!room->psave_loaded)
		load_save_room (room);

	/* search the storeroom for stuff, ignoring money */
	for (tobj = room->contents; tobj; tobj = tobj->next_content)
	{
		if (tobj->obj_flags.type_flag == ITEM_MONEY)
			continue;

		/* always activate roundup or you will get sales for 0cp*/
		val_in_coppers = (int)calculate_sale_price (tobj, keeper, NULL, 1, true, false);

		/* skip this if they can't afford it */
		if (val_in_coppers > purse)
			continue;

		if (tobj->count > 1)
			items_in_list += tobj->count;
		else
			items_in_list++;
	}

	/* if there is nothing to buy, skip */
	if (!items_in_list)
		return purse;

	/* pick a random item to buy */
	if (items_in_list == 1)
		target_item = 1;
	else
		target_item = number (1, items_in_list);

	/* search through the storeroom again */
	for (tobj = room->contents; tobj; tobj = tobj->next_content)
	{
		/* skip money objs */
		if (tobj->obj_flags.type_flag == ITEM_MONEY)
			continue;

		/* always activate roundup or you will get sales for 0cp*/
		val_in_coppers = (int)calculate_sale_price (tobj, keeper, NULL, 1, true, false);

		/* skip this if they can't afford it */
		if (val_in_coppers > purse)
			continue;

		/* plus one because i is indexed [0,...) but target_item is rolled [1,...) */
		if ((i + 1) == target_item)
		{
			break;
		}
		else if (tobj->count > 1)
		{
			if (i < target_item && target_item <= (i + tobj->count))
				break;
		}
		if (tobj->count > 1)
			i += tobj->count;
		else
			i++;
	}

	/* if there is nothing to buy, spend all money so the shop is not hit again */
	if (!tobj)
		return purse;

	/* use max to ensure cost is at least 1 */
	val_in_coppers = MAX(1,(int)calculate_sale_price (tobj, keeper, NULL, 1, true, false));

	/*  paranoid check to ensure it's affordable */
	if (val_in_coppers > purse)
	{
		send_to_gods("Grommit messed up and this vNPC has too much money!");
		return 0;
	}


	// Cost of ordering replacement item for merchant
	// Merchant will pay to order a replacement of what was just bought if they
	// can do so and the item is now out of stock
	delivery_cost = (int)calculate_sale_price (tobj, keeper, NULL, 1, true, true);

	int port_num = engine.get_port ();

	/* remove the item */
	target_item = tobj->nVirtual;
	obj_from_room (&tobj, 1);

	/* add the money */
	money_to_storeroom (keeper, val_in_coppers);

	/* log the receipt */
	mysql_safe_query
		("INSERT INTO receipts "
		"(time, shopkeep, transaction, who, customer, vnum, "
		"item, qty, cost, room, gametime, port) "
		"VALUES (NOW(),%d,'sold','%s','%s',%d,'%s',%d,%d,%d,'%d-%d-%d %d:00',%d)",
		keeper->mob->nVirtual,
		 "vNPC Customer",
		"an honest-looking person",
		 tobj->nVirtual,
		tobj->short_description,
		 1,
		 val_in_coppers,
		 keeper->in_room,
		time_info.year,
		 time_info.month + 1,
		 time_info.day + 1,
		time_info.hour,
		 port_num);

	/* now, if the item can be ordered from that shop and is now out of stock...*/
	if (keeper_makes (keeper, target_item) && !get_obj_in_list_num (target_item, room->contents))
	{
		/* order another if they can afford it */
		if (keeper_has_money (keeper, delivery_cost))
		{
			subtract_keeper_money (keeper, delivery_cost);
			OBJ_DATA *obj = load_object (target_item);
			if (obj)
			{
				/* set the cost to the same as before */
				obj->obj_flags.set_cost = tobj->obj_flags.set_cost;
				obj_to_room (obj, keeper->mob->shop->store_vnum);
				
				mysql_safe_query
				("INSERT INTO receipts "
				 "(time, shopkeep, transaction, who, customer, vnum, "
				 "item, qty, cost, room, gametime, port) "
				 "VALUES (NOW()+1,%d,'bought','%s','%s',%d,'%s',%d,%f,%d,'%d-%d-%d %d:00',%d)",
				 keeper->mob->nVirtual,
				 "vNPC Merchant",
				 "an honest-looking merchant",
				 tobj->nVirtual,
				 tobj->short_description,
				 1,
				 delivery_cost,
				 keeper->in_room,
				 time_info.year,
				 time_info.month + 1,
				 time_info.day + 1,
				 time_info.hour,
				 port_num);
			}
		}
	}
	/* item has been pulled, go destroy it */
	extract_obj (tobj);

	/* return the amount paid */
	return val_in_coppers;
}


#define NO_SUCH_ITEM1 "Don't seem to have that in my inventory. Would you like to buy something else?"
#define NO_SUCH_ITEM2 "I don't see that particular item. Perhaps you misplaced it?"
#define MISSING_CASH1 "A little too expensive for me now -- why don't you try back later?"
#define MISSING_CASH2 "You're a little short on coin, I see; come back when you can afford it."
#define DO_NOT_BUY    "I don't buy those sorts of things, I'm afraid."

/**************
 * "list <merchant keyword> <item keyword>" will list available items from specified merchant with the specified keyword
 * If there is only one argument, look for a merchant. If no merchant, list items 
 * "list <merchant keyword>" will list available items from specified merchant
 * "list <keyword>" will list all available items with the matching keyword
 * "list" with no argument will list everything
 **************
 */
void
do_list (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char stock_buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char output[MAX_STRING_LENGTH]= { '\0' };
	int i;
	float val_in_coppers = 0;
	int header_said = 0;
	CHAR_DATA *keeper = NULL;
	CHAR_DATA *tch;
	ROOM_DATA *room;
	ROOM_DATA *store;
	OBJ_DATA *obj;
	NEGOTIATION_DATA *neg;

	room = ch->room;

	argument = one_argument (argument, buf);

	if (*buf)
	{
		keeper = get_char_room_vis (ch, buf);
		if (keeper && IS_NPC(keeper))
			argument = one_argument (argument, buf);
		else 
		{
			ch->send_to_char("There is no merchant here.\n");
			return;
		}
	}
	else
	{
		for (tch = room->people; tch; tch = tch->next_in_room)
			if (tch != ch && IS_SET (tch->flags, FLAG_KEEPER))
				break;

		keeper = tch;
	}

	if (!keeper)
	{
		ch->send_to_char("There is no merchant here.\n");
		return;
	}

	if ((ch->get_trust() < 2) && !can_see_mob(keeper, ch))
	{
		do_say (keeper, "Who's there?", 0);
		return;
	}

	if (keeper->get_position() <= SLEEP)
	{
		ch->act("$N is not conscious.", true, 0, keeper, TO_CHAR);
		return;
	}

	
	if (!keeper->mob->shop || !IS_SET (keeper->flags, FLAG_KEEPER))
	{
		ch->act("$N isn't a shopkeeper.", false, 0, keeper, TO_CHAR);
		return;
	}

	if (keeper->mob->shop->shop_vnum && keeper->mob->shop->shop_vnum != ch->in_room)
	{
		do_say (keeper, "I'm sorry.  Please catch me when I'm in my shop.", 0);
		return;
	}

		//TODO: remove store-rooms from NPC shops
	if (!(store = vtor (keeper->mob->shop->store_vnum)))
	{
		do_say (keeper, "I've lost my business.  You'll have to go elsewhere.",
			0);
		return;
	}

	if (!store->psave_loaded)
		load_save_room (store);

	if (!store->contents)
	{
		do_say (keeper, "I have nothing for sale at the moment.", 0);
		return;
	}

	i = 0;

	*output = '\0';

	for (obj = store->contents; obj; obj = obj->next_content)
	{

		i++;

		if (obj->obj_flags.type_flag == ITEM_MONEY
			&& keeper_uses_currency_type (keeper->mob->currency_type, obj))
		{
			i--;
			if (obj->next_content)
			{
				continue;
			}
			else
				break;
		}

		OBJ_DATA* contents;
		if (*buf
			&& (!isname (buf, obj->name))
			&& !(obj->obj_flags.type_flag == ITEM_BOOK && obj->book_title
				 && isname (buf, obj->book_title))
			&& !(obj->obj_flags.type_flag == ITEM_DRINKCON
				 && obj->o.drinkcon.volume
				 && (contents = vtoo (obj->o.drinkcon.liquid))
				 && isname (buf, contents->name))
			&& !(obj->obj_flags.type_flag == ITEM_DRYCON
				 && obj->o.drycon.volume
				 && (contents = vtoo (obj->o.drycon.contents))
				 && isname (buf, contents->name)))
			continue;

		if (!CAN_WEAR (obj, ITEM_TAKE))
			continue;

		/* Prevent players from buying back items they've sold, and prevent
		all others from buying a sold item for 15 minutes to prevent abuse */

		if ((obj->sold_by != ch->coldload_id
			&& (time (0) - obj->sold_at <= 60 * 15))
			|| (obj->sold_by == ch->coldload_id
			&& (time (0) - obj->sold_at <= 60 * 60)))
			continue;

		if (!header_said)
		{
			ch->act("$N describes $S inventory:", false, 0, keeper, TO_CHAR);
			sprintf (output + strlen (output),
				"\n   #      price        item\n");
			sprintf (output + strlen (output), "  ===     =====        ====\n");
			header_said = 1;
		}

		val_in_coppers =
			calculate_sale_price (obj, keeper, ch, 1, false, false);

		if (val_in_coppers == 0 && obj->obj_flags.set_cost == 0)
			val_in_coppers = 1;

		*stock_buf = '\0';

		if (!keeper_makes (keeper, obj->nVirtual))
			sprintf (stock_buf, "(%d in stock)", obj->count);
		
		char amt[AVG_STRING_LENGTH] = "";
		sprintf (amt,"%7.2f cp",val_in_coppers);
		sprintf (buf2, " #1%3d    %s%s  #2%-55.55s#0",
			i, ((val_in_coppers) ? amt : "   free   "),
			"   ", obj_short_desc (obj));
	

		for (neg = keeper->mob->shop->negotiations; neg; neg = neg->next)
		{
			if (neg->ch_coldload_id == ch->coldload_id &&
				neg->obj_vnum == obj->nVirtual)
				break;
		}

		if (IS_WEARABLE (obj) || neg)
		{
			if (strlen (obj_short_desc (obj)) > 40)
			{
				buf2[62] = '.';
				buf2[63] = '.';
				buf2[64] = '.';
			}
			buf2[65] = '\0';
			strcat (buf2, "#0");
		}
		else
		{
			if (strlen (obj_short_desc (obj)) > 52)
			{
				buf2[74] = '.';
				buf2[75] = '.';
				buf2[76] = '.';
			}
			buf2[77] = '\0';
			strcat (buf2, "#0");
		}

		if (IS_WEARABLE (obj))
		{
			if (obj->size)
				sprintf (buf2 + strlen (buf2), " (%s)", sizes_named[obj->size]);
			else if (keeper_makes (keeper, obj->nVirtual))
				sprintf (buf2 + strlen (buf2), " (all sizes)");
			else
				strcat (buf2, " (all sizes)");
		}

		if (IS_SET (obj->obj_flags.extra_flags, ITEM_PC_SOLD))
			strcat (buf2, " #6(used)#0");

		sprintf (buf2 + strlen (buf2), "%s", neg ? " (neg)" : "");

		strcat (buf2, "\n");

		if (strlen (output) + strlen (buf2) > MAX_STRING_LENGTH)
			break;

		sprintf (output + strlen (output), "%s", buf2);
	}

	if (!header_said)
	{
		if (*buf)
			ch->act("$N doesn't have any of those.", false, 0, keeper, TO_CHAR);
		else
			ch->act("Sadly, $N has nothing to sell.", false, 0, keeper, TO_CHAR);
	}
	else
		page_string (ch->desc, output);
}

void
do_preview (CHAR_DATA * ch, char *argument, int cmd)
{
	int i;
	OBJ_DATA *obj;
	CHAR_DATA *keeper = NULL;
	CHAR_DATA *tch;
	ROOM_DATA *room;
	ROOM_DATA *store;
	bool found = false;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };

	/* buy [keeper] [count] item [size | !] */

	/* cmd is 1 when this is a barter. */

	room = ch->room;

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char("Preview what?\n");
		return;
	}

	if ((keeper = get_char_room_vis (ch, buf)))
		argument = one_argument (argument, buf);

	else
	{
		for (tch = room->people; tch; tch = tch->next_in_room)
			if (tch != ch && IS_SET (tch->flags, FLAG_KEEPER))
				break;

		keeper = tch;

		if (!keeper)
		{
			ch->send_to_char("There is no shopkeeper here.\n");
			return;
		}

		if (!*buf)
		{
			ch->act("PREVIEW what from $N?", true, 0, keeper, TO_CHAR);
			return;
		}
	}

	if (!keeper)
	{
		ch->send_to_char("There is no merchant here.\n");
		return;
	}

	if (keeper->get_position() <= SLEEP)
	{
		ch->act("$N is not conscious.", true, 0, keeper, TO_CHAR);
		return;
	}


	if (!keeper->mob->shop)
	{

		if (keeper == ch)
			ch->send_to_char("You are not a shopkeeper.");
		else
			ch->act("$N is not a keeper.", false, 0, keeper, TO_CHAR);

		return;
	}

	if (keeper->mob->shop->shop_vnum && keeper->mob->shop->shop_vnum != ch->in_room)
	{
		do_say (keeper, "I'm sorry.  Please catch me when I'm in my shop.", 0);
		return;
	}

	if (!(store = vtor (keeper->mob->shop->store_vnum)))
	{
		do_say (keeper, "I've lost my business.  You'll have to go elsewhere.",
			0);
		return;
	}

	if (!store->psave_loaded)
		load_save_room (store);

	if (!store->contents)
	{
		do_say (keeper, "I have nothing for sale at the moment.", 0);
		return;
	}

	if (store->contents && just_a_number (buf))
	{

		obj = store->contents;

		while (obj && obj->obj_flags.type_flag == ITEM_MONEY
			&& keeper_uses_currency_type (keeper->mob->currency_type, obj))
			obj = obj->next_content;

		for (i = 1;; i++)
		{

			if (!obj)
				break;

			if (obj->obj_flags.type_flag == ITEM_MONEY
				&& keeper_uses_currency_type (keeper->mob->currency_type, obj))
			{
				i--;
				if (obj->next_content)
				{
					obj = obj->next_content;
					continue;
				}
				else
					break;
			}

			if (i == atoi (buf))
			{
				found = true;
				break;
			}

			obj = obj->next_content;
		}

		if (!obj || !found)
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, NO_SUCH_ITEM1);
			do_whisper (keeper, buf, 83);
			return;
		}
	}

	else if (!(obj = get_obj_in_list_vis_not_money (ch, buf,
		vtor (keeper->mob->shop->
		store_vnum)->
		contents)))
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, NO_SUCH_ITEM1);
		do_whisper (keeper, buf, 83);
		return;
	}

	ch->act("$N shows you $p.", false, obj, keeper, TO_CHAR);

	ch->send_to_char("\n");

	show_obj_to_char (obj, ch, 15, 2);

	/* grommit - show origins */
	char* origins = origins_list(ch,obj);
	ch->send_to_char("\n");
	ch->send_to_char(origins);
	free_mem(origins);

	//append the output of the evaluate command
	show_evaluate_information(ch, obj);
}

int
keeper_has_item (CHAR_DATA * keeper, int ovnum)
{
	OBJ_DATA *tobj;
	ROOM_DATA *room;

	if (!keeper || !(room = vtor (keeper->mob->shop->store_vnum)) || !ovnum)
		return 0;

	for (tobj = room->contents; tobj; tobj = tobj->next_content)
	{
		if (tobj->nVirtual == ovnum
			&& !IS_SET (vtoo (ovnum)->obj_flags.extra_flags, ITEM_VARIABLE))
			return 1;
	}

	return 0;
}

int
keeper_makes (CHAR_DATA * keeper, int ovnum)
{
	int i;

	if (!keeper || !keeper->mob->shop || !ovnum)
		return 0;

	for (i = 0; i < MAX_TRADES_IN; i++)
		if (keeper->mob->shop->delivery[i] == ovnum)
			return 1;

	return 0;
}

void
money_to_storeroom (CHAR_DATA * keeper, int amount)
{
	OBJ_DATA *obj, *next_obj;
	ROOM_DATA *store;
	int money = 0;

	if (!keeper->mob->shop)
		return;
	if (!keeper->mob->shop->store_vnum)
		return;
	if (!(store = vtor (keeper->mob->shop->store_vnum)))
		return;

	while (keeper_has_money (keeper, 1))
	{
		for (obj = store->contents; obj; obj = next_obj)
		{
			next_obj = obj->next_content;
			if (obj->obj_flags.type_flag == ITEM_MONEY)
			{
				money += ((int) obj->coppers) * obj->count;
				obj_from_room (&obj, 0);
				extract_obj (obj);
			}
		}
	}

	money += amount;

	give_change(money, keeper->mob->currency_type, store, NULL, NULL );
	
}

void
subtract_keeper_money (CHAR_DATA * keeper, int cost)
{
	OBJ_DATA *obj, *next_obj;
	ROOM_DATA *store;
	int money = 0;

	if (!keeper->mob->shop)
		return;
	if (!keeper->mob->shop->store_vnum)
		return;
	if (!(store = vtor (keeper->mob->shop->store_vnum)))
		return;

	while (keeper_has_money (keeper, 1))
	{
		for (obj = store->contents; obj; obj = next_obj)
		{
			next_obj = obj->next_content;
			if (obj->obj_flags.type_flag == ITEM_MONEY
				&& keeper_uses_currency_type (keeper->mob->currency_type, obj))
			{
				money += ((int) obj->coppers) * obj->count;
				obj_from_room (&obj, 0);
				extract_obj (obj);
			}
		}
	}

	money -= cost;


	give_change(money, keeper->mob->currency_type, store, NULL, NULL);
	
}

void
do_buy (CHAR_DATA * ch, char *argument, int cmd)
{
	int buy_count = 1;
	float keepers_cost = 0;
	float delivery_cost = 0;
	int iter;
	int regardless = 0;
	int wants_off_size = 0;
	int size = -1;
	int nobarter_flag;
	int discount;
	int keeper_success;
	int language_barrier = 0;
	int ch_success, flags = 0;
	int orig_count = 0;
	OBJ_DATA *obj;
	OBJ_DATA *tobj;
	CHAR_DATA *horse;
	CHAR_DATA *keeper = NULL;
	CHAR_DATA *tch;
	ROOM_DATA *room;
	ROOM_DATA *store;
	NEGOTIATION_DATA *neg = NULL;
	char name[MAX_STRING_LENGTH]= { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };

	/* buy [keeper] [count] item [size | !] */

	/* cmd is 1 when this is a barter. */

	/* cmd is 2 when confirming a purchase */

	room = ch->room;

	argument = one_argument (argument, buf);

	if (cmd != 2)
	{
		if (!*buf)
		{
			ch->send_to_char("Buy what?\n");
			return;
		}

		if ((keeper = get_char_room_vis (ch, buf))
			&& IS_SET (keeper->flags, FLAG_KEEPER))
			argument = one_argument (argument, buf);

		else
		{
			for (tch = room->people; tch; tch = tch->next_in_room)
				if (tch != ch && IS_SET (tch->flags, FLAG_KEEPER))
					break;

			keeper = tch;

			if (!*buf)
			{
				ch->act("Buy what from $N?", true, 0, keeper, TO_CHAR);
				return;
			}
		}

		argument = one_argument (argument, buf2);

		/* buf is a count if buf2 is an object */

		if (isdigit (*buf) && *buf2 &&
			(isdigit (*buf2)
			|| (keeper && keeper->mob->shop
			&& get_obj_in_list_vis_not_money (ch, buf2,
			vtor (keeper->mob->shop->
			store_vnum)->
			contents))))
		{
			buy_count = atoi (buf);
			if (buy_count > 50)
			{
				ch->send_to_char("You can only buy up to 50 items at a time.\n");
				return;
			}
			strcpy (buf, buf2);
			argument = one_argument (argument, buf2);
		}

		if (*buf2 == '!')
			regardless = 1;

		else if (*buf2)
		{

			size = index_lookup (sizes_named, buf2);

			if (size == -1)
				size = index_lookup (sizes, buf2);

			wants_off_size = 1;
		}
	}
	else
		keeper = ch->delay_ch;

	if (!keeper || keeper->room != ch->room)
	{
		ch->send_to_char("There is no merchant here.\n");
		return;
	}

	if (keeper == ch)
	{
		ch->send_to_char("You can't buy from yourself!\n");
		return;
	}

	if (keeper->get_position() <= SLEEP)
	{
		ch->act("$N is not conscious.", true, 0, keeper, TO_CHAR);
		return;
	}


	if ((ch->get_trust() < 2) && !can_see_mob(keeper, ch))
	{
		do_say (keeper, "Who's there?", 0);
		return;
	}

	if (!keeper->mob->shop)
	{

		if (keeper == ch)
			ch->send_to_char("You are not a shopkeeper.");
		else
			ch->act("$N is not a keeper.", false, 0, keeper, TO_CHAR);

		return;
	}

	if (!keeper->mob->shop || !IS_NPC (keeper))
	{
		ch->send_to_char("Are you sure they're a shopkeeper?\n");
		return;
	}

	if (keeper->mob->shop->shop_vnum && keeper->mob->shop->shop_vnum != ch->in_room)
	{
		do_say (keeper, "I'm sorry.  Please catch me when I'm in my shop.", 0);
		return;
	}

	if (!(store = vtor (keeper->mob->shop->store_vnum)))
	{
		do_say (keeper, "I've lost my business.  You'll have to go elsewhere.",
			0);
		return;
	}

	if (!store->psave_loaded)
		load_save_room (store);

	if (!store->contents)
	{
		do_say (keeper, "I have nothing for sale at the moment.", 0);
		return;
	}

	if (cmd != 2)
	{
		if (just_a_number (buf))
		{

			obj = store->contents;

			while (obj && obj->obj_flags.type_flag == ITEM_MONEY
				&& keeper_uses_currency_type (keeper->mob->currency_type,
				obj))
				obj = obj->next_content;

			for (iter = 1;; iter++)
			{

				if (!obj)
					break;

				if (obj->obj_flags.type_flag == ITEM_MONEY
					&& keeper_uses_currency_type (keeper->mob->currency_type,
					obj))
				{
					iter--;
					if (obj->next_content)
					{
						obj = obj->next_content;
						continue;
					}
					else
					{
						obj = NULL;
						break;
					}
				}

				/* Prevent players from buying back items they've sold, and prevent
				all others from buying a sold item for 15 minutes to prevent abuse */

				if ((obj->sold_by != ch->coldload_id
					&& (time (0) - obj->sold_at <= 60 * 15))
					|| (obj->sold_by == ch->coldload_id
					&& (time (0) - obj->sold_at <= 60 * 60)))
				{
					iter--;
					if (obj->next_content)
					{
						obj = obj->next_content;
						continue;
					}
					else
					{
						obj = NULL;
						break;
					}
				}

				if (iter == atoi (buf))
					break;

				obj = obj->next_content;
			}

			if (!obj)
			{
				ch->send_to_char("There are not that many items in the keeper's "
					"inventory.\n");
				return;
			}
		}

		else if (!(obj = get_obj_in_list_vis_not_money (ch, buf,
			vtor (keeper->mob->shop->
			store_vnum)->
			contents)))
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s %s", buf2, NO_SUCH_ITEM1);
			do_whisper (keeper, buf, 83);
			return;
		}

		if (IS_WEARABLE (obj) && wants_off_size)
		{
			if (obj->size && obj->size != size)
			{
				ch->act("$p isn't that size.", false, obj, 0, TO_CHAR);
				return;
			}
		}

		else if (IS_WEARABLE (obj) && obj->size &&
			obj->size != ch->get_size() && !regardless && obj->size != size)
		{
			ch->act("$p wouldn't fit you.", false, obj, 0, TO_CHAR);
			ch->act("(End the buy command with ! if you really want it.)",
				false, obj, 0, TO_CHAR);
			return;
		}

		if (!keeper_makes (keeper, obj->nVirtual) && buy_count > obj->count)
		{
			name_to_ident (ch, buf2);
			sprintf (buf, "%s I only have %d of that in stock at the moment.",
				buf2, obj->count);
			do_whisper (keeper, buf, 83);
			return;
		}
		else if (keeper_makes (keeper, obj->nVirtual) && buy_count > obj->count)
			obj->count = buy_count;

		if (buy_count < 1)
			buy_count = 1;
	}
	else
	{
		if (ch->delay_type != DEL_PURCHASE_ITEM || !ch->delay_obj)
		{
			ch->send_to_char("There is no purchase in progress, I'm afraid.\n");
			ch->delay_type = 0;
			ch->delay_info1 = 0;
			ch->delay_obj = NULL;
			ch->delay_ch = NULL;
			return;
		}

		if ((obj = ch->delay_obj) && obj->in_room != keeper->mob->shop->store_vnum)
		{
			ch->send_to_char("That item is no longer available for purchase.\n");
			ch->delay_type = 0;
			ch->delay_info1 = 0;
			ch->delay_obj = NULL;
			ch->delay_ch = NULL;
			return;
		}
		else if (!obj)
		{
			ch->send_to_char("That item is no longer available for purchase.\n");
			ch->delay_type = 0;
			ch->delay_info1 = 0;
			ch->delay_obj = NULL;
			ch->delay_ch = NULL;
			return;
		}

		buy_count = ch->delay_info1;

		if (buy_count < 1)
			buy_count = 1;

		ch->delay_type = 0;
		ch->delay_info1 = 0;
		ch->delay_obj = NULL;
		ch->delay_ch = NULL;
	}

	keepers_cost =
		calculate_sale_price (obj, keeper, ch, buy_count, true, false);

	if (IS_SET (ch->room->room_flags, LAWFUL) &&
		IS_SET (obj->obj_flags.extra_flags, ITEM_ILLEGAL))
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s Those are illegal.  I can't sell them.", buf2);
		do_whisper (keeper, buf, 83);
		return;
	}

	if (obj->obj_flags.type_flag == ITEM_NPC_OBJECT)
	{
		if (cmd != 2)
		{
			if (!*buf2)
			{
				ch->send_to_char
					("You'll need to specify a name for your new NPC, e.g. \"buy horse Shadowfax\".\n");
				return;
			}
		}
		else
		{
			sprintf (buf2, "%s", ch->delay_who);
			free_mem (ch->delay_who);
		}
		if (strlen (buf2) > 26)
		{
			ch->send_to_char
				("The NPC's name must be 26 letters or fewer in length.\n");
			return;
		}
		for (iter = 0; iter < strlen (buf2); iter++)
		{
			if (!isalpha (buf2[iter]))
			{
				ch->send_to_char("Invalid characters in the proposed NPC name.\n");
				return;
			}
		}
		sprintf (name, "%s", buf2);
	}

	if (cmd == 1)
	{				/* passed by barter command to do_buy */

		name_to_ident (ch, buf);

		if (keepers_cost < 20)
		{
			strcat (buf, " This isn't worth haggling over.");
			do_whisper (keeper, buf, 83);
			return;
		}

		if (IS_SET (obj->obj_flags.extra_flags, ITEM_PC_SOLD))
		{
			strcat (buf, " I won't haggle over a used piece of merchandse.");
			do_whisper (keeper, buf, 83);
			return;
		}

		nobarter_flag = index_lookup (econ_flags, "nobarter");

		if (nobarter_flag != -1 &&
			IS_SET (keeper->mob->shop->nobuy_flags, 1 << nobarter_flag))
		{
			strcat (buf, " I'm sorry, but I will not haggle.  My prices "
				"are fixed, take it or leave it.");
			do_whisper (keeper, buf, 83);
			return;
		}

		if (nobarter_flag != -1 && IS_SET (obj->econ_flags, 1 << nobarter_flag))
		{
			strcat (buf, " I'm sorry, but I will not haggle over the price "
				"of this item.");
			do_whisper (keeper, buf, 83);
			return;
		}

		/* Search for existing entry in keepers negotiations list */

		for (neg = keeper->mob->shop->negotiations; neg; neg = neg->next)
		{
			if (neg->ch_coldload_id == ch->coldload_id &&
				neg->obj_vnum == obj->nVirtual && neg->true_if_buying)
				break;
		}

		if (neg)
		{
			if (neg->price_delta > 0)
			{
				strcat (buf, " No, no, I cannot afford to lower the price on "
					"that again.");
				do_whisper (keeper, buf, 83);
				return;
			}

			strcat (buf, " You're persistent, aren't you?  I said no, and I "
				"meant no.");
			do_whisper (keeper, buf, 83);
			return;
		}

		/* keeper will be reluctant to sell to foreigners */
		language_barrier =
			(keeper->skill_map[ch->speaks] <
			15) ? (15 - keeper->skill_map[ch->speaks]) : (0);
		keeper_success =
			skill_roll (MIN
			(95, keeper->skill_map["Barter"] + language_barrier));
		ch_success =
			skill_roll (MAX (5, ch->skill_map["Barter"] - language_barrier));

		if (ch_success == CRIT_SUCCESS && keeper_success == MOD_SUCCESS)
			discount = 5;
		else if (ch_success == CRIT_SUCCESS && keeper_success == MOD_FAIL)
			discount = 10;
		else if (ch_success == CRIT_SUCCESS && keeper_success == CRIT_FAIL)
			discount = 15;

		else if (ch_success == MOD_SUCCESS && keeper_success == MOD_SUCCESS)
			discount = 0;
		else if (ch_success == MOD_SUCCESS && keeper_success == MOD_FAIL)
			discount = 5;
		else if (ch_success == MOD_SUCCESS && keeper_success == CRIT_FAIL)
			discount = 10;

		else if (ch_success == MOD_FAIL && keeper_success == MOD_SUCCESS)
			discount = 0;
		else if (ch_success == MOD_FAIL && keeper_success == MOD_FAIL)
			discount = 0;
		else if (ch_success == MOD_FAIL && keeper_success == CRIT_FAIL)
			discount = 5;

		else
			discount = 0;		/* A CF by ch */

		discount = -1 * discount; //changing to a lower price
		neg = new NEGOTIATION_DATA;
		neg->ch_coldload_id = ch->coldload_id;
		neg->obj_vnum = obj->nVirtual;
		neg->time_when_forgotten = time (NULL) + 6 * 60 * 60;	/* 6 hours */
		neg->price_delta = discount;
		neg->transactions = 0;
		neg->true_if_buying = 1;
		neg->next = keeper->mob->shop->negotiations;
		keeper->mob->shop->negotiations = neg;

		if (discount == 0)
		{
			strcat (buf, " The price is as stated.  Take it or leave it.");
			do_whisper (keeper, buf, 83);
			return;
		}

		else if (discount == 5)
			strcat (buf, " I like your face, you seem an honest and "
			"trustworthy sort.  You can have it for ");
		else if (discount == 10)
			strcat (buf, " It's just not my day, is it?  All right, you win, "
			"I'll sell at your price.  It's yours for ");
		else
			strcat (buf, " My word!  I need to learn to count!  At this rate, "
			"I'll be out of business in a week.  Here, here, take "
			"your ill-gotten gain and begone.  Take it away for ");

		keepers_cost = keepers_cost * (100 + discount) / 100;

		keepers_cost = (int) keepers_cost;

		sprintf (buf + strlen (buf), "%d copper%s", (int) keepers_cost,
			(int) keepers_cost > 1 ? "s" : "");
	
		strcat (buf, ".");

		do_whisper (keeper, buf, 83);

		return;
	}

	if ((keepers_cost > 0 && keepers_cost < 1)
		|| (keepers_cost == 0 && obj->obj_flags.set_cost == 0))
		keepers_cost = 1;


	keepers_cost = (int) keepers_cost;
	int markup_cost;
	if (obj->obj_flags.set_cost != 0)
	{
		markup_cost = obj->obj_flags.set_cost;
	}
	else
	{
		markup_cost = 0;
	}

	if (cmd != 2)
	{
		orig_count = obj->count;
		obj->count = buy_count;
		if (obj->in_room != keeper->mob->shop->store_vnum)
			obj->in_room = keeper->mob->shop->store_vnum;
		
		sprintf (buf,
			"You have opted to purchase #2%s#0, for a total of %d copper%s. To confirm, please use the ACCEPT command.",
			obj_short_desc (obj), (int) keepers_cost,
			(int) keepers_cost > 1 ? "s" : "");
		
		ch->act(buf, false,  0, 0, TO_CHAR | _ACT_FORMAT);
		ch->delay_type = DEL_PURCHASE_ITEM;
		ch->delay_obj = obj;
		ch->delay_info1 = buy_count;
		if (ch->delay_info1 < 1)
			ch->delay_info1 = 1;
		ch->delay_ch = keeper;
		obj->count = orig_count;
		if (obj->obj_flags.type_flag == ITEM_NPC_OBJECT)
			ch->delay_who = duplicateString (name);
		return;
	}

	if (!can_subtract_money (ch, (int) keepers_cost, keeper->mob->currency_type))
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, MISSING_CASH2);
		do_whisper (keeper, buf, 83);
		return;
	}

	if (obj->morphTime)
	{
		obj->clock = vtoo (obj->nVirtual)->clock;
		obj->morphTime = time (0) + obj->clock * 15 * 60;
	}

	tobj = obj;
	obj_from_room (&tobj, buy_count);
	int port_num = engine.get_port ();

	mysql_safe_query
		("INSERT INTO receipts "
		"(time, shopkeep, transaction, who, customer, vnum, "
		"item, qty, cost, room, gametime, port) "
		"VALUES (NOW(),%d,'sold','%s','%s',%d,'%s',%d,%f,%d,'%d-%d-%d %d:00',%d)",
		keeper->mob->nVirtual, ch->name,
		  ch->char_short(),
		tobj->nVirtual,
		 tobj->short_description,
		 tobj->count,
		 keepers_cost,
		keeper->in_room,
		 time_info.year,
		 time_info.month + 1,
		time_info.day + 1,
		 time_info.hour, port_num);

	if (keeper_makes (keeper, obj->nVirtual)
		&& !get_obj_in_list_num (obj->nVirtual,
		vtor (keeper->mob->shop->store_vnum)->contents))
	{
		tobj->count = buy_count;
		delivery_cost = calculate_sale_price (obj, keeper, NULL, 1, true, true);
		if (keeper_has_money (keeper, (int) delivery_cost))
		{
			OBJ_DATA *new_obj = load_object (obj->nVirtual);
			if (obj)
			{
				new_obj->obj_flags.set_cost = markup_cost;
				obj_to_room (new_obj, keeper->mob->shop->store_vnum);
				subtract_keeper_money (keeper, (int) delivery_cost);
				mysql_safe_query
				("INSERT INTO receipts "
				 "(time, shopkeep, transaction, who, customer, vnum, "
				 "item, qty, cost, room, gametime, port) "
				 "VALUES (NOW()+1,%d,'bought','%s','%s',%d,'%s',%d,%f,%d,'%d-%d-%d %d:00',%d)",
				 keeper->mob->nVirtual,
				 "vNPC Merchant",
				 "an honest-looking merchant",
				 obj->nVirtual,
				 obj->short_description,
				 1,
				 delivery_cost,
				 keeper->in_room,
				 time_info.year,
				 time_info.month + 1,
				 time_info.day + 1,
				 time_info.hour,
				 port_num);
			}
		}
	}

	ch->act("$n buys $p.", false, tobj, 0, TO_ROOM);
	ch->act("$N sells you $p.", false, tobj, keeper, TO_CHAR);
	keeper->act("You sell $N $p.", false, tobj, ch, TO_CHAR);

	subtract_money (ch, (int) keepers_cost, keeper->mob->currency_type);

	name_to_ident (ch, buf2);

	sprintf (buf, "%s A veritable steal at ", buf2);
	sprintf (buf + strlen (buf),
		"%d copper%s! Enjoy it, my friend.", (int) keepers_cost,
		(int) keepers_cost > 1 ? "s" : "");
	

	do_whisper (keeper, buf, 83);

	money_to_storeroom (keeper, (int) keepers_cost);


	if (obj->obj_flags.type_flag == ITEM_LIGHT)
	{
		tobj->o.od.value[1] = obj->o.od.value[1];
	}

	if (IS_WEARABLE (tobj))
	{
		if (size != -1)
			tobj->size = size;
		else if (regardless && tobj->size)
			;
		else
			tobj->size = ch->get_size();
	}

	if (tobj->obj_flags.type_flag == ITEM_CONTAINER 
		&& tobj->o.od.value[2] > 0
		&& vtoo (tobj->o.od.value[2])
		&& (vtoo (tobj->o.od.value[2]))->obj_flags.type_flag == ITEM_KEY)
	{
			//if the key exists, put two of them in the container
		if(obj = load_object (tobj->o.od.value[2]))
		{
			obj->o.od.value[1] = tobj->coldload_id;
			obj_to_obj (obj, tobj);
			
			obj = load_object (tobj->o.od.value[2]);
			obj->o.od.value[1] = tobj->coldload_id;
			obj_to_obj (obj, tobj);
		}
	}

	if (ch->right_hand && ch->left_hand)
	{
		sprintf (buf,
			"%s Your hands seem to be full, so I'll just set this down for you to pick up when you've a chance.",
			ch->name);
		ch->send_to_char("\n");
		do_whisper (keeper, buf, 83);
		one_argument (tobj->name, buf2);
		ch->send_to_char("\n");
		sprintf (buf, ": sets *%s down nearby, nodding to ~%s.", buf2,
			ch->name);
		command_interpreter (keeper, buf);
		obj_to_room (tobj, keeper->in_room);
	}
	else
		obj_to_char (tobj, ch);

	if (neg)
		if (!neg->transactions++)
			skill_use (ch, "Barter", 0);
}

int
keeper_uses_currency_type (int currency_type, OBJ_DATA * obj)
{
	if (currency_type == CURRENCY_TIRITH)
	{
		if (obj->nVirtual >= GONDOR_FIRST && obj->nVirtual <= GONDOR_LAST)
		return 1;
	}
	
	return 0;
}

int
trades_in (CHAR_DATA * keeper, OBJ_DATA * obj)
{
	OBJ_DATA *tobj;
	int i;
	bool block = true;
	std::list<std::string>::iterator materials_it;
	std::list<std::string>::iterator obj_it;
	
	if (!obj->coppers)
		return 0;

	if (obj->obj_flags.type_flag == 0)
		return 0;

	if (obj->obj_flags.type_flag == ITEM_MONEY
		&& keeper_uses_currency_type (keeper->mob->currency_type, obj))
		return 0;

	if (IS_SET (keeper->room->room_flags, LAWFUL)
		&& IS_SET (obj->obj_flags.extra_flags, ITEM_ILLEGAL))
		return 0;

	//since fluids are no-take, this check has to be here
	//casuing some side effects so I am setting it back to no-buy
	if (obj->obj_flags.type_flag == ITEM_FLUID)
		return 0;

	if (!IS_SET (obj->obj_flags.wear_flags, ITEM_TAKE))
		return 0;

	for (i = 0; i < MAX_TRADES_IN; i++)
	{
		if (obj->obj_flags.type_flag == keeper->mob->shop->trades_in[i])
			block = false;
	}

	if (block)
		return 0;

	if (!keeper->mob->shop->materials.empty()) //not in his material list, return 0
	{
		/** TODO finish up materials for shopkeepers here
		for ( materials_it = keeper->mob->shop->materials.begin(); materials_it != keeper->mob->shop->materials.end(); materials_it++ )
		{
			for (obj_it = obj->material.begin();
				 obj_it != obj->material.end();
				 obj_it++ )
			{
				if (obj_it == materials_it)
				{
					block = false;
				}
			}
			
		}
		**/
	}	
	if (block)
		return 0;
			

	if (keeper->mob->shop->buy_flags && !(obj->econ_flags & keeper->mob->shop->buy_flags))
		return 0;

	if (keeper->mob->shop->nobuy_flags
		&& (obj->econ_flags & keeper->mob->shop->nobuy_flags))
		return 0;

	// Check any contents inside the object for trades_in eligibility.
	/**
	 capacity - od.value[0]
	 volume - od.value[1]
	 contents/liquid - od.value[2]
	 **/
	if ((obj->obj_flags.type_flag == ITEM_DRINKCON || obj->obj_flags.type_flag == ITEM_DRYCON)
		&& obj->o.od.value[1] > 0)
	{
		if (tobj = vtoo (obj->o.od.value[2]))
		{
			if (!trades_in (keeper, tobj))
				return 0;
		}
		else
			return 0;
	}

	return 1;
}

int
keeper_has_money (CHAR_DATA * keeper, int cost)
{
	ROOM_DATA *store;
	OBJ_DATA *obj;
	int money = 0;

	if (!keeper->mob->shop)
		return 0;
	if (!keeper->mob->shop->store_vnum)
		return 0;
	if (!(store = vtor (keeper->mob->shop->store_vnum)))
		return 0;

	if (!store->psave_loaded)
		load_save_room (store);

	if (store->contents 
		&& (store->contents)->obj_flags.type_flag == ITEM_MONEY
		&& keeper_uses_currency_type (keeper->mob->currency_type,
		store->contents))
	{
		money = ((int) store->contents->coppers) * store->contents->count;
	}
	for (obj = store->contents; obj; obj = obj->next_content)
	{
		if (obj->next_content 
			&& (obj->next_content)->obj_flags.type_flag == ITEM_MONEY
			&& keeper_uses_currency_type (keeper->mob->currency_type,
			obj->next_content))
		{
			money += ((int) obj->next_content->coppers) * obj->next_content->count;
		}
	}

	if (money < cost)
		return 0;
	else
		return money;
}

void
keeper_money_to_char (CHAR_DATA * keeper, CHAR_DATA * ch, int money)
{
	OBJ_DATA *obj, *tobj, *cont_obj;
	int location;
	bool money_found = false;
	std::string amount;

	cont_obj = NULL;
	
	for (location = 0; location < MAX_WEAR; location++)
    {
		if (!(tobj = get_equip (ch, location)))
			continue;
			if (tobj->obj_flags.type_flag == ITEM_CONTAINER)
		{
				for (obj = tobj->contains; obj; obj = obj->next_content)
			{
				if (obj->obj_flags.type_flag == ITEM_MONEY)
					money_found = true;
			}
			
			cont_obj = tobj; //there is a container
			
			if (money_found) //and it has money
						break;
		}
    }

	if (!cont_obj) //no container with money was found, look again for containers
    {
		for (location = 0; location < MAX_WEAR; location++)
		{
			if (!(tobj = get_equip (ch, location)))
				continue;
			if (tobj->obj_flags.type_flag == ITEM_CONTAINER)
			{
				cont_obj = tobj; //this obj is a container
					break;
			}
		}
    }
	
	if (cont_obj)
		tobj = cont_obj;
	else
		tobj = NULL;

	give_change(money, keeper->mob->currency_type, ch->room, tobj, ch);
	
	return;
}
void
do_sell (CHAR_DATA * ch, char *argument, int cmd)
{
	int objs_in_storage;
	int sell_count = 1;
	int nobarter_flag;
	int language_barrier = 0;
	int keeper_success;
	int ch_success;
	int discount, same_obj = 0;
	float keepers_cost = 0;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	OBJ_DATA *obj;
	OBJ_DATA *tobj;
	CHAR_DATA *keeper = NULL;
	CHAR_DATA *tch;
	ROOM_DATA *room;
	NEGOTIATION_DATA *neg;

	argument = one_argument (argument, buf);

	room = ch->room;

	if (isdigit (*buf))
	{
		sell_count = atoi (buf);
		argument = one_argument (argument, buf);
		if (sell_count > MAX_INV_COUNT)
		{
			sprintf (buf2,
				"Sorry, but you can only sell up to %d items at a time.\n",
				MAX_INV_COUNT);
			ch->send_to_char(buf2);
			return;
		}
	}

	if (!*buf)
	{
		ch->send_to_char("Sell what?\n");
		return;
	}

	if ((keeper = get_char_room_vis (ch, buf)))
		argument = one_argument (argument, buf);

	else
	{
		for (tch = room->people; tch; tch = tch->next_in_room)
			if (tch != ch && IS_SET (tch->flags, FLAG_KEEPER))
				break;

		keeper = tch;

		if (!*buf)
		{
			ch->act("Sell what to $N?", true, 0, keeper, TO_CHAR);
			return;
		}
	}

	if (!keeper)
	{
		ch->send_to_char("There is no merchant here.\n");
		return;
	}

	if (keeper == ch)
	{
		ch->send_to_char("You can't sell to yourself!\n");
		return;
	}

	if (!keeper->mob->shop || !IS_NPC (keeper))
	{
		ch->act("$N does not seem to be a shopkeeper.", true, 0, keeper,
			TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (keeper->get_position() <= SLEEP)
	{
		ch->act("$N is not conscious.", true, 0, keeper, TO_CHAR);
		return;
	}

	if (!keeper->mob->shop ||
		(keeper->mob->shop->shop_vnum && keeper->mob->shop->shop_vnum != ch->in_room))
	{
		do_say (keeper, "I'm sorry.  Please catch me when I'm in my shop.", 0);
		return;
	}

	if (!(room = vtor (keeper->mob->shop->store_vnum)))
	{
		do_say (keeper, "I've lost my business.  You'll have to go elsewhere.",
			0);
		return;
	}

	if (IS_NPC (keeper) && IS_SET (keeper->mob->action, ACT_NOBUY))
	{
		do_say (keeper, "Sorry, but I don't deal in second-hand merchandise.",
			0);
		return;
	}

	if (!room->psave_loaded)
		load_save_room (room);

	if ((ch->get_trust() < 2) && !can_see_mob(keeper, ch))
	{
		do_say (keeper, "Who's there?", 0);
		return;
	}

	if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, NO_SUCH_ITEM2);
		do_whisper (keeper, buf, 83);
		return;
	}

	if (obj->count < sell_count)
		sell_count = obj->count;

	if (sell_count < 1)
		sell_count = 1;

	keepers_cost =
		calculate_sale_price (obj, keeper, ch, sell_count, true, true);

	if (!trades_in (keeper, obj))
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, DO_NOT_BUY);
		do_whisper (keeper, buf, 83);
		return;
	}

	if ((obj->obj_flags.type_flag == ITEM_LIGHT && !obj->o.od.value[1]))
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s No, I wouldn't even think of buying that.", buf2);
		do_whisper (keeper, buf, 83);
		return;
	}

	if (IS_SET (ch->room->room_flags, LAWFUL) &&
		IS_SET (obj->obj_flags.extra_flags, ITEM_ILLEGAL))
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s I can't buy that.  It's illegal to possess.", buf2);
		do_whisper (keeper, buf, 83);
		return;
	}

	if (vtor (keeper->mob->shop->store_vnum))
	{

		objs_in_storage = 0;

		for (tobj = vtor (keeper->mob->shop->store_vnum)->contents;
			tobj; tobj = tobj->next_content)
		{
			if (tobj->obj_flags.type_flag != ITEM_MONEY
				&& IS_SET (tobj->obj_flags.extra_flags, ITEM_PC_SOLD))
				objs_in_storage++;
			if (tobj->nVirtual == obj->nVirtual)
				same_obj += tobj->count;
		}

		if (((IS_SET (obj->obj_flags.extra_flags, ITEM_STACK)
			&& ((same_obj + sell_count) > MAX_INV_COUNT))
			|| (!IS_SET (obj->obj_flags.extra_flags, ITEM_STACK)
			&& ((same_obj + sell_count) > (MAX_INV_COUNT / 4))))
			&& (obj->obj_flags.type_flag != ITEM_TOSSABLE) && !(IS_SET (ch->room->room_flags, NOINVLIMIT)))
		{
			name_to_ident (ch, buf2);
			sprintf (buf,
				"%s I have quite enough of that for now, thank you; try back again later.",
				buf2);
			do_whisper (keeper, buf, 83);
			return;
		}

		  
		if (objs_in_storage > 125 && !(IS_SET (ch->room->room_flags, NOINVLIMIT)))
		{
			name_to_ident (ch, buf2);
			sprintf (buf,
				"%s I have too much stuff as it is.  Perhaps you'd like to purchase something instead?",
				buf2);
			do_whisper (keeper, buf, 83);
			return;
		}
	}

	if (cmd == 1)
	{				/* passed by barter command to do_sell */

		if (sell_count > 1)
		{
			ch->send_to_char("You can only barter for one item at a time.\n");
			return;
		}

		name_to_ident (ch, buf);

		if (keepers_cost < 20)
		{
			strcat (buf, " This isn't worth haggling over.");
			do_whisper (keeper, buf, 83);
			return;
		}

		nobarter_flag = index_lookup (econ_flags, "nobarter");

		if (nobarter_flag != -1 &&
			IS_SET (keeper->mob->shop->nobuy_flags, 1 << nobarter_flag))
		{
			strcat (buf, " I'm sorry, but I will not haggle.  My prices "
				"are fixed, take it or leave it.");
			do_whisper (keeper, buf, 83);
			return;
		}

		if (IS_SET (obj->obj_flags.extra_flags, ITEM_PC_SOLD))
		{
			strcat (buf, " I won't haggle over a used piece of merchandse.");
			do_whisper (keeper, buf, 83);
			return;
		}

		if (nobarter_flag != -1 && IS_SET (obj->econ_flags, 1 << nobarter_flag))
		{
			strcat (buf, " I'm sorry, but I will not haggle over the price "
				"of this item.");
			do_whisper (keeper, buf, 83);
			return;
		}

		/* Search for existing entry in keepers negotiations list */

		for (neg = keeper->mob->shop->negotiations; neg; neg = neg->next)
		{
			if (neg->ch_coldload_id == ch->coldload_id &&
				neg->obj_vnum == obj->nVirtual && !neg->true_if_buying)
				break;
		}

		if (neg)
		{
			if (neg->price_delta > 0)
			{
				strcat (buf, " No, no, I will not pay any higher a price.");
				do_whisper (keeper, buf, 83);
				return;
			}

			strcat (buf, " Listen, as much as I like you, I simply cannot "
				"offer you what you're asking.");
			do_whisper (keeper, buf, 83);
			return;
		}

		/* keeper will be reluctant to sell to foreigners */
		language_barrier =
			(keeper->skill_map[ch->speaks] <
			15) ? (15 - keeper->skill_map[ch->speaks]) : (0);
		keeper_success =
			skill_roll (MIN
			(95, keeper->skill_map["Barter"] + language_barrier));
		ch_success =
			skill_roll (MAX (5, ch->skill_map["Barter"] - language_barrier));

		if (ch_success == CRIT_SUCCESS && keeper_success == MOD_SUCCESS)
			discount = 5;
		else if (ch_success == CRIT_SUCCESS && keeper_success == MOD_FAIL)
			discount = 10;
		else if (ch_success == CRIT_SUCCESS && keeper_success == CRIT_FAIL)
			discount = 15;

		else if (ch_success == MOD_SUCCESS && keeper_success == MOD_SUCCESS)
			discount = 0;
		else if (ch_success == MOD_SUCCESS && keeper_success == MOD_FAIL)
			discount = 5;
		else if (ch_success == MOD_SUCCESS && keeper_success == CRIT_FAIL)
			discount = 10;

		else if (ch_success == MOD_FAIL && keeper_success == MOD_SUCCESS)
			discount = 0;
		else if (ch_success == MOD_FAIL && keeper_success == MOD_FAIL)
			discount = 0;
		else if (ch_success == MOD_FAIL && keeper_success == CRIT_FAIL)
			discount = 5;

		else
			discount = 0;		/* A CF by ch */

		neg = new NEGOTIATION_DATA;
		neg->ch_coldload_id = ch->coldload_id;
		neg->obj_vnum = obj->nVirtual;
		neg->time_when_forgotten = time (NULL) + 6 * 60 * 60;	/* 6 hours */
		neg->price_delta = discount;
		neg->transactions = 0;
		neg->true_if_buying = 0;
		neg->next = keeper->mob->shop->negotiations;
		keeper->mob->shop->negotiations = neg;

		if (discount == 0)
		{
			strcat (buf, " Sorry, but it's just not worth more than my "
				"initial offer.");
			do_whisper (keeper, buf, 83);
			return;
		}

		else if (discount == 5)
			strcat (buf, " I've been looking for these.  It's a pleasure doing "
			"business with you.  I'll pay you ");
		else if (discount == 10)
			strcat (buf, " Perhaps if I go back to bed now, I can salvage some "
			"small part of my self respect.  I'll pay you ");
		else
			strcat (buf, " It is a dark day.  I'll have to sell my home and "
			"business just to recoup what I've lost this day."
			"  I'll give you ");

		keepers_cost = keepers_cost * (100 + discount) / 100;

		sprintf (buf + strlen (buf), "%d copper%s", (int) keepers_cost,
			(int) keepers_cost > 1 ? "s" : "");

		strcat (buf, ".");

		do_whisper (keeper, buf, 83);

		return;
	} //end  if (cmd == 1) bartering

	keepers_cost = (int) keepers_cost;

	/* Look up negotiations for this ch/obj on keeper */

	for (neg = keeper->mob->shop->negotiations; neg; neg = neg->next)
	{
		if (neg->ch_coldload_id == ch->coldload_id &&
			neg->obj_vnum == obj->nVirtual && !neg->true_if_buying)
			break;
	}

	keepers_cost = (int) keepers_cost;

	if (keepers_cost < 1)
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s Bah, that isn't even worth my time!", buf2);
		do_whisper (keeper, buf, 83);
		return;
	}

	if (!keeper_has_money (keeper, (int) keepers_cost))
	{

		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, MISSING_CASH1);

		do_whisper (keeper, buf, 83);

		if ((IS_NPC (keeper) && !IS_SET (keeper->mob->action, ACT_PCOWNED)) ||
			!vtor (keeper->mob->shop->store_vnum))
			return; /// \todo Flagged as unreachable

		return;
	}

	ch->act("$n sells $p.", false, obj, 0, TO_ROOM);
	ch->act("You sell $p.", false, obj, 0, TO_CHAR);
	ch->send_to_char("\n");

	name_to_ident (ch, buf2);
	sprintf (buf, "%s Here's the amount we've agreed upon.", buf2);

	/* Pay customer */

	do_whisper (keeper, buf, 83);

	subtract_keeper_money (keeper, (int) keepers_cost);
	keeper_money_to_char (keeper, ch, (int) keepers_cost);

	obj_from_char (&obj, sell_count);
	int port_num = engine.get_port ();

	mysql_safe_query
		("INSERT INTO receipts "
		"(time, shopkeep, transaction, who, customer, vnum, "
		"item, qty, cost, room, gametime, port) "
		"VALUES (NOW(),%d,'bought','%s','%s',%d,'%s',%d,%f,%d,'%d-%d-%d %d:00',%d)",
		keeper->mob->nVirtual,
		 ch->name,
		  ch->char_short(),
		obj->nVirtual,
		 obj->short_description,
		 obj->count,
		 keepers_cost,
		keeper->in_room,
		 time_info.year,
		 time_info.month + 1,
		time_info.day + 1,
		 time_info.hour,
		 port_num);


	if (keeper_makes (keeper, obj->nVirtual))
		extract_obj (obj);
	else
	{
		obj->obj_flags.extra_flags |= ITEM_PC_SOLD;
		obj->sold_at = (int) time (0);
		obj->sold_by = ch->coldload_id;
		obj_to_room (obj, keeper->mob->shop->store_vnum);
	}

	if (neg)
		if (!neg->transactions++)
			skill_use (ch, "Barter", 0);
}

void
do_value (CHAR_DATA * ch, char *argument, int cmd)
{
	float keepers_cost = 0;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char buf3[MAX_STRING_LENGTH]= { '\0' };
	OBJ_DATA *obj;
	ROOM_DATA *room;
	CHAR_DATA *keeper = NULL;
	CHAR_DATA *tch;

	argument = one_argument (argument, buf);

	if (!*buf)
	{
		ch->send_to_char("Appraise what?\n");
		return;
	}

	room = ch->room;

	if ((keeper = get_char_room_vis (ch, buf)))
		argument = one_argument (argument, buf);

	else
	{
		for (tch = room->people; tch; tch = tch->next_in_room)
			if (tch != ch && IS_SET (tch->flags, FLAG_KEEPER))
				break;

		keeper = tch;

		if (!*buf)
		{
			ch->act("Have $N appraise what?", true, 0, keeper, TO_CHAR);
			return;
		}
	}

	if (!keeper)
	{
		ch->send_to_char("There is no merchant here.\n");
		return;
	}

	if (!keeper->mob->shop)
	{
		ch->act("$N does not seem to be a shopkeeper.", true, 0, keeper,
			TO_CHAR | _ACT_FORMAT);
		return;
	}

	if (keeper->get_position() <= SLEEP)
	{
		ch->act("$N is not conscious.", true, 0, keeper, TO_CHAR);
		return;
	}

	if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)))
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, NO_SUCH_ITEM2);
		do_whisper (keeper, buf, 83);
		return;
	}

	if (!trades_in (keeper, obj))
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s %s", buf2, DO_NOT_BUY);
		do_whisper (keeper, buf, 83);
		return;
	}

	if ((keepers_cost == 0 && obj->obj_flags.type_flag == ITEM_MONEY))
	{
		name_to_ident (ch, buf2);
		sprintf (buf, "%s No, I wouldn't even think of buying that.", buf2);
		do_whisper (keeper, buf, 83);
		return;
	}

	if ((ch->get_trust() < 2) && !can_see_mob(keeper, ch))
	{
		do_say (keeper, "Who's there?", 0);
		return;
	}

	keepers_cost =
		calculate_sale_price (obj, keeper, ch, obj->count, true, true);

	name_to_ident (ch, buf2);

	*buf3 = '\0';

	sprintf (buf3, "%d copper%s", (int) keepers_cost,
		(int) keepers_cost > 1 ? "s" : "");

	keepers_cost = (int) keepers_cost;

	if (!keepers_cost)
		sprintf (buf, "%s I'm afraid that isn't even worth my time...", buf2);
	else
		sprintf (buf, "%s I'd buy %s for... %s.", buf2,
		obj->count > 1 ? "those" : "that", buf3);

	do_whisper (keeper, buf, 83);
}


void
do_barter (CHAR_DATA * ch, char *argument, int cmd)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (ch->skill_map["Barter"] < 1)
	{
		ch->send_to_char
			("You're not convincing enough to barter, unfortunately.\n");
		return;
	}
	argument = one_argument (argument, buf);

	if (is_abbrev (buf, "buy"))
		do_buy (ch, argument, 1);
	else if (is_abbrev (buf, "sell"))
		do_sell (ch, argument, 1);
	else
		ch->send_to_char("barter sell ITEM    or\n barter buy ITEM\n");
}

/*
TODO: this command needs some work. some old code was never finished and the new code is different
> receipts for [<day>] [<month>] [<year>]
> receipts to <person>
> receipts from <object>
-------
> receipts  [summary] [r <rvnum>]

*/
void
do_receipts (CHAR_DATA * ch, char *argument, int cmd)
{
	short last_day = -1, last_month = -1, last_year = -1, nArgs = 0, nSeekDay =
		0, nSeekMonth = 0, nSeekYear = 0;
	int nAmtSold = 0, nAmtBought = 0, nTAmtSold = 0, nTAmtBought = 0;
	long day = -1, month = -1, year = -1;
	char *ptrFor = NULL, *ptrFrom = NULL, *ptrTo = NULL;
	CHAR_DATA *keeper = NULL;
	MYSQL_RES *result;
	MYSQL_ROW row;
	char args[3][AVG_STRING_LENGTH / 3] = { "", "", "" };
	char buf[MAX_STRING_LENGTH] = "";
	bool sumchk = false;
	bool found_keeper = false;
	std::list<char_data*>::iterator tch_iterator;
	
	//int shopnum = 0;

	/****** begin future options test statments ***/
	if (argument && argument[0])
	{
			
		if (!strncmp (argument, "from ", 5))
		{
			ptrFrom = argument;
		}

		else if ((ptrFrom = strstr (argument, " from ")))
		{
			ptrFrom = ptrFrom + 6;
		}

		if (!strncmp (argument, "to ", 3))
		{
			ptrTo = argument;
		}
		else if ((ptrTo = strstr (argument, " to ")))
		{
			ptrTo = ptrTo + 4;
		}

		if (!strncmp (argument, "summary", 7))
		{
			sumchk = true;
		}
	}// if (argument && argument[0])



	if (IS_NPC (ch) && ch->mob->shop)
	{
		keeper = ch;
	}
	else
	{
		for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
		{
			keeper = *tch_iterator;
			if (keeper == ch)
				continue;
			if (IS_NPC (keeper) && keeper->mob->shop
				&& keeper->mob->shop->store_vnum == ch->in_room)
			{
				found_keeper = true;
				break;
			}
		}
	}
	if (!found_keeper)
	{
		ch->send_to_char("You do not see a book of receipts here.\n");
		return;
	}


	/* Detail */
	int port_num = engine.get_port ();
//for auroness version
	mysql_safe_query
		("SELECT time, shopkeep, transaction, who, customer, vnum, "
		"item, qty, cost, room, gametime, port, "
		"EXTRACT(YEAR FROM gametime) as year, "
		"EXTRACT(MONTH FROM gametime) as month, "
		"EXTRACT(DAY FROM gametime) as day "
		"FROM receipts "
		"WHERE shopkeep = '%d' AND port = '%d' "
		"ORDER BY time DESC;",
		keeper->mob->nVirtual, port_num);

	if ((result = mysql_store_result (database)) == NULL)
	{
		send_to_gods ((char *) mysql_error (database));
		ch->send_to_char("The book of receipts is unavailable at the moment.\n");
		return;
	}

	ch->send_to_char("Examining a book of receipts:\n");
	while ((row = mysql_fetch_row (result)) != NULL)
	{

		day = strtol (row[14], NULL, 10);
		month = strtol (row[13], NULL, 10) - 1;
		year = strtol (row[12], NULL, 10);
		if (day != last_day || month != last_month || year != last_year)
		{

			if (last_day > 0 && month != last_month)
			{
				
			sprintf (buf + strlen (buf),
				"\n    Total for #6%s %d#0: Sales #2%d cp#0, Purchases #2%d cp#0.\n\n",
				month_name[(int) last_month], (int) last_year,
				nAmtSold, nAmtBought);
				
				nTAmtBought += nAmtBought;
				nTAmtSold += nAmtSold;
				nAmtBought = 0;
				nAmtSold = 0;
			}

			if (!sumchk)
			{
				sprintf (buf + strlen (buf),
					"\nOn #6%d %s %d#0:\n\n",
					(int) day, month_name[(int) month], (int) year);
			}

			last_day = day;
			last_month = month;
			last_year = year;

		}

		if (strcmp (row[2], "sold") == 0)
		{
			nAmtSold += strtol (row[8], NULL, 10);
		}
		else if (strcmp (row[2], "bought") == 0)
		{
			nAmtBought += strtol (row[8], NULL, 10);
		}

		row[2][0] = toupper (row[2][0]);
		if (!sumchk)
		{
		sprintf (buf + strlen (buf),
			"%s #2%s#0 of #2%s#0 %s #5%s#0 for #2%s cp#0.\n",
			row[2], row[7], row[6], (row[2][0] == 's') ? "to" : "from",
			(IS_NPC (ch) || (!ch->get_trust())) ? row[4] : row[3], row[8]);
			
		}

		if (strlen (buf) > MAX_STRING_LENGTH - 512)
		{
			strcat (buf,
				"\n#1There were more sales than could be displayed.#0\n\n");
			break;
		}
	}

	mysql_free_result (result);

	if (last_day > 0)
	{
		if ((nTAmtBought + nTAmtSold == 0) && (nAmtSold + nAmtBought > 0))
		{
		sprintf (buf + strlen (buf),
			"\n    Total for #6%s %d#0: Sales #2%d cp#0, Purchases #2%d cp#0.\n\n"
			"    Current coin on hand: #2%d cp#0.\n",
			month_name[(int) last_month], (int) last_year,
			nAmtSold, nAmtBought, keeper_has_money (keeper, 0));
		}
		else
		{
		sprintf (buf + strlen (buf),
			"\n    Total for #6%s %d#0: Sales #2%d cp#0, Purchases #2%d cp#0.\n\n"
			"    Total for period:    Sales #2%d cp#0, Purchases #2%d cp#0.\n"
			"    Current coin on hand:  #2%d cp#0.\n",
			month_name[(int) last_month], (int) last_year,
			nAmtSold, nAmtBought, nTAmtSold, nTAmtBought,
			keeper_has_money (keeper, 0));
			
		}
		page_string (ch->desc, buf);
	}
}

int
get_uniq_ticket (void)
{
	int tn = 1;
	int i;
	FILE *fp_ls;
	char buf[MAX_STRING_LENGTH]= { '\0' };

	if (!(fp_ls = popen ("ls tickets", "r")))
	{
		system_log ("The ticket system is broken, get_uniq_ticket()", true);
		return -1;
	}

	/* The TICKET_DIR should be filled with files that have seven
	digit names (zero padded on the left).
	*/

	while (!feof (fp_ls))
	{

		if (!fgets (buf, 80, fp_ls))
			break;

		for (i = 0; i < 7; i++)
			if (!isdigit (buf[i]))
				continue;

		if (tn != atoi (buf))
			break;

		tn = atoi (buf) + 1;
	}

	pclose (fp_ls);

	return tn;
}



int
can_subtract_money (CHAR_DATA * ch, int coppers_to_subtract,
					int currency_type)
{
	OBJ_DATA *obj, *tobj;
	int money = 0, location;

	if ((obj = ch->right_hand))
	{
		if (obj->obj_flags.type_flag == ITEM_MONEY)
		{
		if (obj->nVirtual >= GONDOR_FIRST && obj->nVirtual <= GONDOR_LAST)
			money += ((int) ch->right_hand->coppers) * ch->right_hand->count;
			
		}
		else if (obj->obj_flags.type_flag == ITEM_CONTAINER)
		{
			for (tobj = obj->contains; tobj; tobj = tobj->next_content)
			{
				if (tobj->obj_flags.type_flag == ITEM_MONEY)
				{
					if (tobj->nVirtual >= GONDOR_FIRST && tobj->nVirtual <= GONDOR_LAST)
							money += ((int) tobj->coppers) * tobj->count;
				}
			}
		}
	}

	if ((obj = ch->left_hand))
	{
		if (obj->obj_flags.type_flag == ITEM_MONEY)
		{
			if (obj->nVirtual >= GONDOR_FIRST && obj->nVirtual <= GONDOR_LAST)
				money += ((int) ch->left_hand->coppers) * ch->left_hand->count;
			
		}
		else if (obj->obj_flags.type_flag == ITEM_CONTAINER)
		{
			for (tobj = obj->contains; tobj; tobj = tobj->next_content)
			{
				if (tobj->obj_flags.type_flag == ITEM_MONEY)
				{
				if (tobj->nVirtual >= GONDOR_FIRST && tobj->nVirtual <= GONDOR_LAST)
					money += ((int) tobj->coppers) * tobj->count;
					
				}
			}
		}
	}

	for (location = 0; location < MAX_WEAR; location++)
	{
		if (!(obj = get_equip (ch, location)))
			continue;
		if (obj->obj_flags.type_flag != ITEM_CONTAINER)
			continue;
		if (IS_SET (obj->o.container.flags, CONT_CLOSED))
			continue;
		for (tobj = obj->contains; tobj; tobj = tobj->next_content)
		{
			if (tobj->obj_flags.type_flag == ITEM_MONEY)
			{
				if (tobj->nVirtual >= GONDOR_FIRST && tobj->nVirtual <= GONDOR_LAST)
						money += ((int) tobj->coppers) * tobj->count;
			}
		}
	}

	if (money < coppers_to_subtract)
		return 0;

	return 1;
}
// If you give the subtract_money function a negative coppers_to_subtract, it will supress all output to the player
void
subtract_money (CHAR_DATA * ch, int coppers_to_subtract, int currency_type)
{
	OBJ_DATA *tobj, *obj, *next_obj;
	int money = 0, location;
	bool container = false;
	bool SupressOutput = false; // Japheth's addition

	if (coppers_to_subtract < 0)
	{
		coppers_to_subtract *= -1;
		SupressOutput = true;
	}

	for (location = 0; location < MAX_WEAR; location++)
	{
		if (!(obj = get_equip (ch, location)))
			continue;
		if (obj->obj_flags.type_flag != ITEM_CONTAINER)
			continue;
		if (IS_SET (obj->o.container.flags, CONT_CLOSED))
			continue;
		for (tobj = obj->contains; tobj; tobj = next_obj)
		{
			next_obj = tobj->next_content;
			if (tobj->obj_flags.type_flag == ITEM_MONEY)
			{
				if (game_currency(currency_type, tobj->nVirtual))
				{
					money += ((int) tobj->coppers) * tobj->count;
					obj_from_obj (&tobj, 0, 0);
					extract_obj (tobj);
					ch->delay_obj = obj;
					container = true;
				}
			}
		}
	}

	if ((obj = ch->right_hand))
	{
		if (obj->obj_flags.type_flag == ITEM_MONEY)
		{
			if (game_currency(currency_type, obj->nVirtual))
			{
				money += ((int) ch->right_hand->coppers) * ch->right_hand->count;
				extract_obj (ch->right_hand);
				ch->right_hand = NULL;
			}
		}
		else if (obj->obj_flags.type_flag == ITEM_CONTAINER)
		{
			for (tobj = obj->contains; tobj; tobj = next_obj)
			{
				next_obj = tobj->next_content;
				if (tobj->obj_flags.type_flag == ITEM_MONEY)
				{
					if (game_currency(currency_type, tobj->nVirtual))
					{
						money += ((int)tobj->coppers * tobj->count);
						obj_from_obj (&tobj, 0, 0);
						extract_obj (tobj);
						ch->delay_obj = obj;
						container = true;
					}
				}
			}
		}
	}

	if ((obj = ch->left_hand))
	{
		if (obj->obj_flags.type_flag == ITEM_MONEY)
		{
			if (game_currency(currency_type, obj->nVirtual))
			{
				money += ((int)ch->left_hand->coppers * ch->left_hand->count);
				extract_obj (ch->left_hand);
				ch->left_hand = NULL;
			}
		}
		else if (obj->obj_flags.type_flag == ITEM_CONTAINER)
		{
			for (tobj = obj->contains; tobj; tobj = next_obj)
			{
				next_obj = tobj->next_content;
				if (tobj->obj_flags.type_flag == ITEM_MONEY)
				{
					if (game_currency(currency_type, tobj->nVirtual))
					{
						money += ((int) tobj->coppers * tobj->count);
						obj_from_obj (&tobj, 0, 0);
						extract_obj (tobj);
						ch->delay_obj = obj;
						container = true;
					}
				}
			}
		}
	}

	money -= coppers_to_subtract;

	if (money <= 0) // Serious bugfix - Japheth 10th May 2007
	{
		return;
	}

	obj = ch->delay_obj;
	ch->delay_obj = NULL;
 
	if (!SupressOutput)
	{
		if (container)
			ch->send_to_char
			("\nRifling through your belongings, you retrieve your coin.\n");
		else
			ch->send_to_char("\nYou offer up the specified amount.\n");
	}

	give_change(money, currency_type, ch->room, obj, ch);

	return;
}



void
econ_markup_discount (CHAR_DATA * keeper, OBJ_DATA * obj, float *markup,
					  float *discount)
{
	
	if (obj->econ_flags & keeper->mob->shop->econ_flags1 &&
		keeper->mob->shop->econ_markup1 > 0)
	{
		*markup = keeper->mob->shop->econ_markup1;
		*discount = keeper->mob->shop->econ_discount1;
	}

	else if (obj->econ_flags & keeper->mob->shop->econ_flags2 &&
		keeper->mob->shop->econ_markup2 > 0)
	{
		*markup = keeper->mob->shop->econ_markup2;
		*discount = keeper->mob->shop->econ_discount2;
	}

	else if (obj->econ_flags & keeper->mob->shop->econ_flags3 &&
		keeper->mob->shop->econ_markup3 > 0)
	{
		*markup = keeper->mob->shop->econ_markup3;
		*discount = keeper->mob->shop->econ_discount3;
	}

	else if (obj->econ_flags & keeper->mob->shop->econ_flags4 &&
		keeper->mob->shop->econ_markup4 > 0)
	{
		*markup = keeper->mob->shop->econ_markup4;
		*discount = keeper->mob->shop->econ_discount4;
	}

	else if (obj->econ_flags & keeper->mob->shop->econ_flags5 &&
		keeper->mob->shop->econ_markup5 > 0)
	{
		*markup = keeper->mob->shop->econ_markup5;
		*discount = keeper->mob->shop->econ_discount5;
	}

	else if (obj->econ_flags & keeper->mob->shop->econ_flags6 &&
		keeper->mob->shop->econ_markup6 > 0)
	{
		*markup = keeper->mob->shop->econ_markup6;
		*discount = keeper->mob->shop->econ_discount6;
	}

	else if (obj->econ_flags & keeper->mob->shop->econ_flags7 &&
		keeper->mob->shop->econ_markup7 > 0)
	{
		*markup = keeper->mob->shop->econ_markup7;
		*discount = keeper->mob->shop->econ_discount7;
	}

	else
	{
		*markup = keeper->mob->shop->markup;
		*discount = keeper->mob->shop->discount;
	}
}
	//returns the discounted value for this object in the shop
float
econ_discount (CHAR_DATA * keeper, OBJ_DATA * obj)
{
	float markup;
	float discount;

	econ_markup_discount (keeper, obj, &markup, &discount);

	return discount;
}

//returns the marked-up value for this object in the shop
float
econ_markup (CHAR_DATA * keeper, OBJ_DATA * obj)
{
	float markup;
	float discount;

	econ_markup_discount (keeper, obj, &markup, &discount);

	return markup;
}

void
leader_tally(OBJ_DATA * obj, char *buffer, int depth)
{
	int count = 0;
	char format[AVG_STRING_LENGTH] = "";
	OBJ_DATA *tobj = NULL;
	
	if (!obj || (strlen (buffer) > MAX_STRING_LENGTH - 256))
		return;
	
	count = (obj->count) ? obj->count : 1;
	sprintf (format, " %%%dc#2%%s#0", 2 * depth);
	sprintf (buffer + strlen (buffer), format, ' ',
			 obj->short_description);
	
	if (count > 1)
	{
		sprintf (buffer + strlen (buffer), " (x%d)\n", count);
	}
	else
	{
		strcat (buffer, "\n");
	}
	
	for (tobj = obj->contains; tobj; tobj = tobj->next_content)
	{
		leader_tally (tobj, buffer, depth + 1);
	}
	return;
	
}

float
tally (OBJ_DATA * obj, char *buffer, int depth)
{
	int count = 0;
	float cost = 0.0, subtotal = 0.0;
	char format[AVG_STRING_LENGTH] = "";
	OBJ_DATA *tobj = NULL;

	if (!obj || (strlen (buffer) > MAX_STRING_LENGTH - 256))
		return 0.00;
	
		

	count = (obj->count) ? obj->count : 1;
	cost = obj->coppers * count;
	sprintf (format, "%% 10.02f cp - %%%dc#2%%s#0", 2 * depth);
	sprintf (buffer + strlen (buffer), format, cost, ' ',
		obj->short_description);
	if (count > 1)
	{
		sprintf (buffer + strlen (buffer), " (x%d)\n", count);
	}
	else
	{
		strcat (buffer, "\n");
	}
	subtotal += cost;

	for (tobj = obj->contains; tobj; tobj = tobj->next_content)
	{
		subtotal += tally (tobj, buffer, depth + 1);
	}
	return subtotal;
}

void
do_tally (CHAR_DATA * ch, char *argument, int cmd)
{
	CHAR_DATA *tch = NULL, *tch2 = NULL;
	OBJ_DATA *obj = NULL;
	int location = 0;
	float subtotal = 0.0, total = 0.0;
	bool bTallyAll = true;
	bool leaderTally = false;
	char arg1[AVG_STRING_LENGTH] = "";
	char buffer[MAX_STRING_LENGTH] = "";

	
	if (!IS_SET(ch->affected_by, AFF_LEADER_COMMAND)
		&& (ch->get_trust() < 2))
	{
		ch->send_to_char("You do not have approval for leadership commands");
		return;
	}

	if (IS_SET(ch->affected_by, AFF_LEADER_COMMAND))
		{
			bTallyAll = false;
			leaderTally = true;
		}

		
	if (ch->get_trust())
	{
		bTallyAll = true;
		leaderTally = false;
		
		if (argument && *argument)
		{
			argument = one_argument (argument, arg1);
			bTallyAll = false;
			if (!(tch2 = get_char_room_vis (ch, arg1)))
			{
				ch->send_to_char("You do not see them here.\n");
				return;
			}
		}
	}
	
	if (bTallyAll && !leaderTally)
	{
		strcpy (buffer, "\n#6Tally in Room:#0\n");
		for (obj = ch->room->contents; obj; obj = obj->next_content)
		{
			subtotal += tally (obj, buffer, 1);
		}
		sprintf (buffer + strlen (buffer),
			"----------------------------------------------\n"
			"% 10.02f cp - Subtotal\n", subtotal);
		total += subtotal;
	}

	if (!leaderTally)
	{
		for (tch = ch->room->people; tch; tch = tch->next_in_room)
		{
			
			if ((!bTallyAll && tch != tch2) || (bTallyAll && tch == ch))
			{
				continue;
			}
			
			
			subtotal = 0.0;
			sprintf (buffer + strlen (buffer),
					 "\n#6Tally of #5%s#0 (%s):#0\n", tch->short_descr, tch->name);
			
			if ((obj = tch->right_hand))
			{
				subtotal += tally (obj, buffer, 1);
			}
			
			if ((obj = tch->left_hand))
			{
				subtotal += tally (obj, buffer, 1);
			}
			
			for (location = 0; location < MAX_WEAR; location++)
			{
				if (!(obj = get_equip (tch, location)))
					continue;
				subtotal += tally (obj, buffer, 1);
			}
			
			sprintf (buffer + strlen (buffer),
					 "----------------------------------------------\n"
					 "% 10.02f cp - %sotal\n",
					 subtotal, (bTallyAll) ? "Subt" : "T");
			
			total += subtotal;
		}
	}
	
	if (bTallyAll && !leaderTally)
	{

		sprintf (buffer + strlen (buffer),
			"==============================================\n"
			"% 10.02f cp - Total\n", total);

	}

	if (!bTallyAll && leaderTally)
	{
		strcpy (buffer, "\n#6Tally in Room:#0\n");
		for (obj = ch->room->contents; obj; obj = obj->next_content)
		{
			leader_tally (obj, buffer, 1);
		}
	}
	page_string (ch->desc, buffer);
}

#include <memory>

void
do_mark (CHAR_DATA* ch, char *argument, int cmd)
{
	bool multiplication = false, subtraction = false, addition = false;

	// First - Assert we have a valid usage case
	if (!IS_NPC(ch) || !ch->mob->shop)
	{
		const char* message =
			"#6OOC - Only the shopkeeper mob may use this command.#0\n"
			"#6      e.g: command <clanmember> mark <item> <value>#0\n";
		ch->send_to_char(message);
		return;
	}

	// Make certain we have a storage room
	ROOM_DATA* store = 0;
	if (!(store = vtor (ch->mob->shop->store_vnum)))
	{
		do_ooc (ch, "I seem to have lost my store room.", 0);
		return;
	}

	// Load the storage if we need to
	if (!store->psave_loaded)
	{
		load_save_room (store);
	}

	// Check if we have an inventory to work with
	if (!store->contents)
	{
		do_say (ch, "I have nothing for sale at the moment.", 0);
		return;
	}


	////////////////////////////////////////////////////////////////////////////
	// Usages: mark <what to change> <new value>
	// what to change: number, keyword, nth.keyword, all.keyword, all
	// new value: number, free, reset
	////////////////////////////////////////////////////////////////////////////
	char* ptr = 0;

	enum {
		mark_none = 0,
		mark_by_list_number,
		mark_by_keyword,
		mark_nth_by_keyword,
		mark_all_by_keyword,
		mark_all
	}
	mark_mode = mark_none;

	// first argument either a number or keyword

	if (!argument || !*argument)
	{
		do_say (ch, "What did you want to change the price of?", 0);
		return;
	}

	int list_number = 0;
	std::string keyword;
	char *item_string = new char[strlen (argument)];
	argument = one_argument (argument, item_string);
	if (strncasecmp ("all", item_string, 3) == 0)
	{
		if (strlen (item_string) == 3)
		{
			mark_mode = mark_all;
		}
		else if (item_string[3] == '.')
		{
			mark_mode = mark_all_by_keyword;
			keyword = item_string + 4;
		}
		else
		{
			mark_mode = mark_by_keyword;
			keyword = item_string;
		}
	}
	else if ((list_number = strtol (item_string, &ptr, 10)))
	{
		if (ptr && *ptr)
		{
			if (*ptr == '.')
			{
				mark_mode = mark_nth_by_keyword;
				keyword = ptr + 1;
			}
			else
			{
				mark_mode = mark_by_keyword;
				keyword = item_string;
			}
		}
		else
		{
			mark_mode = mark_by_list_number;
		}
	}
	else
	{
		mark_mode = mark_by_keyword;
		keyword = item_string;
	}
	delete [] item_string;

	// second argument must be a decimal, "free", or "reset"
	if (!argument || !*argument)
	{
		do_say (ch, "What price did you want to set?", 0);
		return;
	}

	float new_value;
	char* value_string = new char[strlen (argument)];
	argument = one_argument (argument, value_string);
	if (strcasecmp ("free", value_string) == 0)
	{
		new_value = -1.0f;
	}
	else if (strcasecmp ("reset", value_string) == 0)
	{
		new_value = 0.0f;
	}
	else if (value_string[0] == '+' || value_string[0] == '-')
	{
		if (value_string[0] == '+')
			addition = true;
		else
			subtraction = true;
		value_string[0] = '0';
		if (!(new_value = strtof (value_string, &ptr)) && (ptr >= value_string))
		{
			char *errmsg = "No number after +/- mark.";
			delete [] value_string;
			do_ooc (ch, errmsg, 0);
			return;
		}

	}
	else if (value_string[0] == '*')
	{
		value_string[0] = '0';
		if (!(new_value = strtof (value_string, &ptr)) && (ptr >= value_string))
		{
			char *errmsg = "No number after * mark.";
			delete [] value_string;
			do_ooc (ch, errmsg, 0);
			return;
		}
		multiplication = true;
	}
	else if (!(new_value = strtof (value_string, &ptr))
		&& (ptr >= value_string))
	{
		char* errmsg =
			"The 'new value' parameter must be either a decimal number, "
			"'free', or 'reset'.";
		delete [] value_string;
		do_ooc (ch, errmsg, 0);
		return;
	}
	else if (new_value <= 0.0f)
	{
		new_value = -1.0f;
	}
	delete [] value_string;
	// else: new_value is a valid decimal.


	if (mark_mode == mark_by_keyword)
	{
		mark_mode = mark_nth_by_keyword;
		list_number = 1;
	}

	bool found = false;
	int i = 1;
	int count = 0;
	int currency = ch->mob->currency_type;
	OBJ_DATA* drink = 0;
	OBJ_DATA* dry_cont = 0;
	OBJ_DATA* obj = 0;
	for (obj = store->contents; (obj && !found); obj = obj->next_content)
	{
		// skip money
		if (obj->obj_flags.type_flag == ITEM_MONEY
			&& keeper_uses_currency_type (currency, obj))
		{
			continue;
		}

		if(
			// mark all
			(mark_mode == mark_all)

			// mark_by_list_number
			|| (mark_mode == mark_by_list_number
				&& (list_number == i++)
				&& (found = true))

			// mark_(nth/all)_by_keyword
			|| ((isname (keyword.c_str (), obj->name)
				 || (obj->obj_flags.type_flag == ITEM_BOOK
					 && obj->book_title
					 && isname (keyword.c_str (), obj->book_title))
				 || (obj->obj_flags.type_flag == ITEM_DRINKCON
					 && obj->o.drinkcon.volume
					 && (drink = vtoo (obj->o.drinkcon.liquid))
					 && isname (keyword.c_str (), drink->name))
				 || (obj->obj_flags.type_flag == ITEM_DRYCON
					 && obj->o.drycon.volume
					 && (dry_cont = vtoo (obj->o.drycon.contents))
					 && isname (keyword.c_str (), dry_cont->name)))
				&& (mark_mode == mark_all_by_keyword
					|| (mark_mode == mark_nth_by_keyword
						&& (list_number == i++)
						&& (found = true)))))

		{
			if (addition)
			{
				if (!obj->obj_flags.set_cost)
					obj->obj_flags.set_cost = (int) ((obj->coppers * 100) + (100 * new_value));
				else
					obj->obj_flags.set_cost = obj->obj_flags.set_cost + (int) (100 * new_value);
			}
			if (subtraction)
			{
				if (!obj->obj_flags.set_cost)
					obj->obj_flags.set_cost = (int) ((obj->coppers * 100) - (100 * new_value));
				else
					obj->obj_flags.set_cost = obj->obj_flags.set_cost - (int) (100 * new_value);
			}
			if (multiplication)
			{
				if (!obj->obj_flags.set_cost)
					obj->obj_flags.set_cost = (int) (obj->coppers * 100 * new_value);
				else
					obj->obj_flags.set_cost = (int) (obj->obj_flags.set_cost * new_value);
			}
			if (!multiplication && !addition && !subtraction)
				obj->obj_flags.set_cost = (int)(100.0 * new_value);
			++count;
		}

	}
	if (count)
	{
		do_say (ch, "Our inventory is updated.", 0);
	}
	else
	{
		do_say (ch, "There doesn't seem to be anything like that "
			"in our inventory.", 0);
	}
}

void
do_payroll (CHAR_DATA * ch, char *argument, int cmd)
{
	short last_day = -1, last_month = -1, last_year = -1;
	int payrollAmt = 0, payrollTAmt = 0;
	long day = -1, month = -1, year = -1;
	CHAR_DATA *keeper = NULL;
	MYSQL_RES *result;
	MYSQL_ROW row;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	bool found_keeper;
	std::list<char_data*>::iterator tch_iterator;
	


	if (IS_NPC (ch) && ch->mob->shop)
	{
		keeper = ch;
	}
	else
	{
		found_keeper = false;
		for (tch_iterator = character_list.begin(); tch_iterator != character_list.end(); tch_iterator++)
		{
			keeper = *tch_iterator;
			if (IS_NPC (keeper) &&
				keeper->mob->shop &&
				keeper->mob->shop->store_vnum == ch->in_room)
				found_keeper = true;
			break;
		}
	}

	if (!found_keeper)
	{
		ch->send_to_char("You do not see a payroll ledger here.\n");
		return;
	}

	/* Detail */
	mysql_safe_query
		("SELECT time, shopkeep, customer, "
		"amount, room, gametime, port, "
		"EXTRACT(YEAR FROM gametime) as year, "
		"EXTRACT(MONTH FROM gametime) as month, "
		"EXTRACT(DAY FROM gametime) as day "
		"FROM payroll "
		"WHERE shopkeep = '%d'"
		"ORDER BY time DESC;",
		keeper->mob->nVirtual);

	if ((result = mysql_store_result (database)) == NULL)
	{
		send_to_gods ((char *) mysql_error (database));
		ch->send_to_char("The payroll ledgers are unavailable at the moment.\n");
		return;
	}

	ch->send_to_char("Examining a payroll ledger:\n");

	sprintf(buf, "---------------------------\n");
	while ((row = mysql_fetch_row (result)) != NULL)
	{

		day = strtol (row[9], NULL, 10);
		month = strtol (row[8], NULL, 10) - 1;
		year = strtol (row[7], NULL, 10);
		if (day != last_day || month != last_month || year != last_year)
		{
			if (last_day > 0 && month != last_month)
			{
				sprintf (buf + strlen (buf),
					"\n    Total for #6%s %d#0: #2%d cp#0.\n\n",
					month_name[(int) last_month],
					(int) last_year,
					payrollAmt);

				payrollTAmt += payrollAmt;
				payrollAmt = 0;
			}

			sprintf (buf + strlen (buf),
				"\n On #6%d %s %d#0:\n\n",
				(int) day,
				month_name[(int) month],
				(int) year);

			last_day = day;
			last_month = month;
			last_year = year;
		}

		payrollAmt += strtol (row[3], NULL, 10);

		sprintf (buf + strlen (buf), " #5%s#0 was paid %ld coppers.\n", row[2], strtol (row[3], NULL, 10));

		if (strlen (buf) > MAX_STRING_LENGTH - 512)
		{
			strcat (buf, "\n #1There were more paychecks than could be displayed.#0\n\n");
			break;
		}
	} //end while

	mysql_free_result (result);

	if (last_day > 0)
	{
		if (payrollTAmt == 0 &&
			payrollAmt > 0)
		{
			sprintf (buf + strlen (buf),
				"\n    Total for #6%s %d#0: Payroll #2%d cp#0.\n\n"
				"    Current coin on hand: #2%d cp#0.\n",
				month_name[(int) last_month],
				(int) last_year,
				payrollAmt,
				keeper_has_money (keeper, 0));
		}
		else
		{
			sprintf (buf + strlen (buf),
				"\n    Total for #6%s %d#0: Payroll #2%d cp#0.\n\n"
				"    Total for period:      Payroll #2%d cp#0.\n"
				"    Current coin on hand:  #2%d cp#0.\n",
				month_name[(int) last_month],
				(int) last_year,
				payrollAmt,
				payrollTAmt,
				keeper_has_money (keeper, 0));
		}
		page_string (ch->desc, buf);
	}
}

/***
 * money - is the amount we start with
 * store -  the storeroom used by the merchant
 * tobj - a container object carried by character
 * ch - the character getting change
 *
 * tobj and ch are NULL if we are working with the shopkeepers coins
 * coins come from or go into his storeroom
 *
 * store is keepers storeroom if his money, 
 * or the room the character is in, for charcter receiving money
 *
 * returns true if change is given to the pc or his object
 * amount of change given is echoed to player 
 * (unless money was sent in as negative, to set up echo_suppression)
 *
 * return value not used by shopkeepers
*********/
void
give_change(int money, int currency_type, ROOM_DATA *store, OBJ_DATA *tobj, CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	OBJ_DATA *obj;
	
	if (!load_object(GONDOR_10K)
		&& !load_object(GONDOR_K)
		&& !load_object(GONDOR_200)
		&& !load_object(GONDOR_50)
		&& !load_object(GONDOR_5)
		&& !load_object(GONDOR_10K)
		&& !load_object(GONDOR_1))
	{
		ch->send_to_char("A coin needed for change does not exist. Please inform staff.\n");
		return;
	}
	if (tobj)
		sprintf (buf, "You are given the following coin as change, which you tuck"
				 " away in #2%s#0:", obj_short_desc (tobj));
	else
		sprintf (buf, "You are given the following coin as change:");
	
	// gold hundredpieces (10K)
	if (money / 10000)
	{				
		obj = load_object (GONDOR_10K);
		obj->count = money / 10000;
		sprintf (buf + strlen (buf),
				 "   #2%d thin, slightly fluted gold coin%s#0\n",
				 obj->count, obj->count > 1 ? "s" : "");
		
		money %= 10000;
		
			//goes in a pouch, to the charcter, or to the room
		if (tobj && tobj->obj_flags.type_flag == ITEM_CONTAINER)
		{
			obj_to_obj (obj, tobj);
		}
		else if (ch)
		{
			obj_to_char (obj, ch);
		}
		else
		{
			obj_to_room (obj, store->nVirtual);
		}

	}
	
		// Gold crowns (K)

	if (money / 1000)
	{				
		obj = load_object (GONDOR_K);
		obj->count = money / 1000;
		sprintf (buf + strlen (buf),
				 "   #2%d thick, hexagonal gold coin%s#0\n", obj->count,
				 obj->count > 1 ? "s" : "");
		
		money %= 1000;
		
			//goes in a pouch, to the charcter, or to the room
		if (tobj && tobj->obj_flags.type_flag == ITEM_CONTAINER)
		{
			obj_to_obj (obj, tobj);
		}
		else if (ch)
		{
			obj_to_char (obj, ch);
		}
		else
		{
			obj_to_room (obj, store->nVirtual);
		}
		
	}
	
	// Silver trees	(200)
	if (money / 200)
	{				
		
	obj = load_object (GONDOR_200);
	obj->count = money / 200;
	sprintf (buf + strlen (buf),
			 "   #2%d heavy, oblong silver coin%s#0\n", obj->count,
			 obj->count > 1 ? "s" : "");
		
		money %= 200;
		
			//goes in a pouch, to the charcter, or to the room
		if (tobj && tobj->obj_flags.type_flag == ITEM_CONTAINER)
		{
			obj_to_obj (obj, tobj);
		}
		else if (ch)
		{
			obj_to_char (obj, ch);
		}
		else
		{
			obj_to_room (obj, store->nVirtual);
		}
		
	}
	
	// Silver royals (50)
	if (money / 50)
	{				
	obj = load_object (GONDOR_50);
	obj->count = money / 50;
	sprintf (buf + strlen (buf), "   #2%d thin, ridged silver coin%s#0\n",
			 obj->count, obj->count > 1 ? "s" : "");

		
		money %= 50;
		
			//goes in a pouch, to the charcter, or to the room
		if (tobj && tobj->obj_flags.type_flag == ITEM_CONTAINER)
		{
			obj_to_obj (obj, tobj);
		}
		else if (ch)
		{
			obj_to_char (obj, ch);
		}
		else
		{
			obj_to_room (obj, store->nVirtual);
		}
		
	}
	
	// Bronze coppers (5)
	if (money / 5)
	{				
		
		obj = load_object (GONDOR_5);
		obj->count = money / 5;
		sprintf (buf + strlen (buf),
				 "   #2%d large, rounded bronze coin%s#0\n", obj->count,
				 obj->count > 1 ? "s" : "");
		
		money %= 5;

			//goes in a pouch, to the charcter, or to the room
		if (tobj && tobj->obj_flags.type_flag == ITEM_CONTAINER)
		{
			obj_to_obj (obj, tobj);
		}
		else if (ch)
		{
			obj_to_char (obj, ch);
		}
		else
		{
			obj_to_room (obj, store->nVirtual);
		}
		
	}
	
	if (money)
	{				// Copper bit.
		
		obj = load_object (GONDOR_1);
		obj->count = money;
		sprintf (buf + strlen (buf), "   #2%d semicircular copper coin%s#0\n",
				 obj->count, obj->count > 1 ? "s" : "");
		
			//goes in a pouch, to the charcter, or to the room
		if (tobj && tobj->obj_flags.type_flag == ITEM_CONTAINER)
		{
			obj_to_obj (obj, tobj);
		}
		else if (ch)
		{
			obj_to_char (obj, ch);
		}
		else
		{
			obj_to_room (obj, store->nVirtual);
		}
	}
	
	if (ch)
		ch->send_to_char(buf);
	
	return;
}

bool
game_currency(int currency_type, int tobjVnum)
{
	if (currency_type == CURRENCY_TIRITH
			&& (tobjVnum >= GONDOR_FIRST 
			&& tobjVnum <= GONDOR_LAST))
		return (true);
	else 
		return (false);

		
}
