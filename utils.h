//////////////////////////////////////////////////////////////////////////////
//
/// utils.h - Inline Utility Macros
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


#ifndef _rpie_utils_h_
#define _rpie_utils_h_

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r')
#define CAP(st)  (*(st) = toupper(*(st)), st)
#define LOW(st)  (*(st) = tolower(*(st)), st)

#define IS_SET(flag,bit)  ((flag) & (bit))
#define IS_AFFECTED(ch,skill) ( IS_SET((ch)->affected_by, (skill)) )


#define SKIL_LEV(val) val >= 70 ? "Master   " : val >= 50 ? "Adroit   " : val >= 30 ?  "Familiar " : val >= 10 ? "Novice   " : "Exposed  "

#define TOGGLE_BIT(var,bit)  ((var) = (var) ^ (bit) )
#define TOGGLE(flag, bit) { if ( IS_SET (flag, bit) ) \
	flag &= ~bit; \
	else \
	flag |= bit; \
	}



#define HSHR(ch) (!IS_SET (ch->affected_by, AFF_HOODED) ? ((ch)->sex ?	\
	(((ch)->sex == 1) ? "his" : "her") : "its") : "its")

#define HSSH(ch) (!IS_SET (ch->affected_by, AFF_HOODED) ? ((ch)->sex ?	\
	(((ch)->sex == 1) ? "he" : "she") : "it") : "it")

#define HMHR(ch) (!IS_SET (ch->affected_by, AFF_HOODED) ? ((ch)->sex ?	\
	(((ch)->sex == 1) ? "him" : "her") : "it") : "it")
#define ANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")

#define SANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")


#define IS_GUIDE(ch)	(!IS_NPC(ch) ? ((!ch->get_trust()) && !IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ? ch->pc->is_guide : 0) : 0)




#define WAIT_STATE(ch, cycle)  (((ch)->desc) ? (ch)->desc->wait = (cycle) : 0)
#define GET_FLAG(ch,flag) (IS_SET ((ch)->flags, flag))

/* Object And Carry related macros */

#define IS_OBJ_VIS(sub, obj)										\
	( (( !IS_SET((obj)->obj_flags.extra_flags, ITEM_INVISIBLE) || 	\
	get_affect (sub, MAGIC_AFFECT_SEE_INVISIBLE) ) &&					\
	!sub->is_blind())                                           \
	|| obj->location == WEAR_BLINDFOLD                         \
	|| !(sub->get_trust()))



#define CAN_WEAR(obj, part) (IS_SET((obj)->obj_flags.wear_flags,part))
#define IS_WEARABLE(obj) ((obj)->obj_flags.wear_flags &            \
	(ITEM_WEAR_BODY | ITEM_WEAR_LEGS | ITEM_WEAR_ARMS))


#define CAN_CARRY_OBJ(ch,obj)  \
	(((ch->carrying() + obj->obj_flags.weight) <= ch->can_carry_weight()))

#define CAN_GET_OBJ(ch, obj)   \
	(CAN_WEAR((obj), ITEM_TAKE) && CAN_CARRY_OBJ((ch),(obj)) &&          \
	can_see_obj((ch),(obj)))

#define IS_OBJ_STAT(obj,stat) (IS_SET((obj)->obj_flags.extra_flags,stat))
#define IS_MERCHANT(mob) (IS_SET((mob)->flags,FLAG_KEEPER))



/* char name/short_desc(for mobs) or someone?  */

#define PERS(ch, vict)  (can_see_mob((vict), (ch)) ? \
	(ch)->char_short() : "someone")

#define OBJS(obj, vict) (can_see_obj((vict), (obj)) ? \
	obj_short_desc (obj) : "something")

#define OBJN(obj, vict) (can_see_obj((vict), (obj)) ? \
	fname((obj)->name) : "something")

#define IS_OUTSIDE(ch) (!IS_SET((ch)->room->room_flags,INDOORS) && \
	ch->room->terrain_type != SECT_CAVE && \
	ch->room->terrain_type != SECT_UNDERWATER)

#define CAN_GO(ch, door) (is_exit(ch,door)  &&  (is_exit(ch,door)->to_room != NOWHERE) \
&& !IS_SET(is_exit(ch, door)->port_flags, EX_CLOSED))

#define CAN_FLEE_SOMEWHERE(ch) (CAN_GO(ch, NORTH) || CAN_GO(ch, SOUTH) || \
	CAN_GO(ch, EAST) || CAN_GO(ch, WEST))





#define IS_TABLE(obj) (obj->obj_flags.type_flag == ITEM_CONTAINER && \
	IS_SET (obj->obj_flags.extra_flags, ITEM_TABLE))


#define SEND_TO_Q(messg, desc)  write_to_q ((messg), (desc) ? &(desc)->output : NULL)

#ifdef NOVELL
#define sigmask(m) ((unsigned long) 1 << ((m) - 1 ))
#endif

#endif // _rpie_utils_h_
