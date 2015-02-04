//////////////////////////////////////////////////////////////////////////////
//
/// somatics.c : Short and Long Term Somatic Effects
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
#include "protos.h"
#include "utils.h"
#include "soma.h"

void affect_soma_type::soma_stat(CHAR_DATA* ch)
{
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char buf2[MAX_STRING_LENGTH]= { '\0' };

	switch (type)
	{
	case MUSCULAR_CRAMP:
		sprintf (buf2, "a muscle cramp");
		break;
	case MUSCULAR_TWITCHING:
		sprintf (buf2, "twitching");
		break;
	case MUSCULAR_TREMOR:
		sprintf (buf2, "tremors");
		break;
	case MUSCULAR_PARALYSIS:
		sprintf (buf2, "paralysis");
		break;
	case DIGESTIVE_ULCER:
		sprintf (buf2, "stomach ulcer");
		break;
	case DIGESTIVE_VOMITING:
		sprintf (buf2, "vomiting");
		break;
	case DIGESTIVE_BLEEDING:
		sprintf (buf2, "vomiting blood");
		break;
	case EYE_BLINDNESS:
		sprintf (buf2, "blindness");
		break;
	case EYE_BLURRED:
		sprintf (buf2, "blurred vision");
		break;
	case EYE_DOUBLE:
		sprintf (buf2, "double vision");
		break;
	case EYE_DILATION:
		sprintf (buf2, "dilated pupils");
		break;
	case EYE_CONTRACTION:
		sprintf (buf2, "contracted pupils");
		break;
	case EYE_LACRIMATION:
		sprintf (buf2, "lacrimation");
		break;
	case EYE_PTOSIS:
		sprintf (buf2, "ptosis");
		break;
	case EAR_TINNITUS:
		sprintf (buf2, "tinnitus");
		break;
	case EAR_DEAFNESS:
		sprintf (buf2, "deafness");
		break;
	case EAR_EQUILLIBRIUM:
		sprintf (buf2, "ear imbalance");
		break;
	case NOSE_ANOSMIA:
		sprintf (buf2, "anosmia");
		break;
	case NOSE_RHINITIS:
		sprintf (buf2, "rhinitis");
		break;
	case MOUTH_SALIVATION:
		sprintf (buf2, "salivation");
		break;
	case MOUTH_TOOTHACHE:
		sprintf (buf2, "toothache");
		break;
	case MOUTH_DRYNESS:
		sprintf (buf2, "dry mouth");
		break;
	case MOUTH_HALITOSIS:
		sprintf (buf2, "halitosis");
		break;
	case CHEST_DIFFICULTY:
		sprintf (buf2, "difficulty breathing");
		break;
	case CHEST_WHEEZING:
		sprintf (buf2, "wheezing");
		break;
	case CHEST_RAPIDBREATH:
		sprintf (buf2, "rapid breathing");
		break;
	case CHEST_SLOWBREATH:
		sprintf (buf2, "shallow breathing");
		break;
	case CHEST_FLUID:
		sprintf (buf2, "fluidous lungs");
		break;
	case CHEST_PALPITATIONS:
		sprintf (buf2, "heart palpitations");
		break;
	case CHEST_COUGHING:
		sprintf (buf2, "coughing fits");
		break;
	case CHEST_PNEUMONIA:
		sprintf (buf2, "pneumonia");
		break;
	case NERVES_PSYCHOSIS:
		sprintf (buf2, "psychosis");
		break;
	case NERVES_DELIRIUM:
		sprintf (buf2, "delerium ");
		break;
	case NERVES_COMA:
		sprintf (buf2, "a comatose state");
		break;
	case NERVES_CONVULSIONS:
		sprintf (buf2, "convulsions");
		break;
	case NERVES_HEADACHE:
		sprintf (buf2, "a headache");
		break;
	case NERVES_CONFUSION:
		sprintf (buf2, "confusion");
		break;
	case NERVES_PARETHESIAS:
		sprintf (buf2, "parethesias");
		break;
	case NERVES_ATAXIA:
		sprintf (buf2, "ataxia");
		break;
	case NERVES_EQUILLIBRIUM:
		sprintf (buf2, "nervous imbalance");
		break;
	case SKIN_CYANOSIS:
		sprintf (buf2, "cyanosis of the skin");
		break;
	case SKIN_DRYNESS:
		sprintf (buf2, "dryness of the skin");
		break;
	case SKIN_CORROSION:
		sprintf (buf2, "corrosion of the skin");
		break;
	case SKIN_JAUNDICE:
		sprintf (buf2, "jaundice of the skin");
		break;
	case SKIN_REDNESS:
		sprintf (buf2, "redness of the skin");
		break;
	case SKIN_RASH:
		sprintf (buf2, "a rash on the skin");
		break;
	case SKIN_HAIRLOSS:
		sprintf (buf2, "hairloss");
		break;
	case SKIN_EDEMA:
		sprintf (buf2, "edema of the skin");
		break;
	case SKIN_BURNS:
		sprintf (buf2, "burns on the skin");
		break;
	case SKIN_PALLOR:
		sprintf (buf2, "pallor of the skin");
		break;
	case SKIN_SWEATING:
		sprintf (buf2, "the sweats");
		break;
	case GENERAL_WEIGHTLOSS:
		sprintf (buf2, "weight loss");
		break;
	case GENERAL_LETHARGY:
		sprintf (buf2, "lethargy");
		break;
	case GENERAL_APPETITELOSS:
		sprintf (buf2, "appetite loss");
		break;
	case GENERAL_PRESSUREDROP:
		sprintf (buf2, "low blood pressure");
		break;
	case GENERAL_PRESSURERISE:
		sprintf (buf2, "high blood pressure");
		break;
	case GENERAL_FASTPULSE:
		sprintf (buf2, "a fast pulse");
		break;
	case GENERAL_SLOWPULSE:
		sprintf (buf2, "a slow pulse");
		break;
	case GENERAL_HYPERTHERMIA:
		sprintf (buf2, "hyperthermia");
		break;
	case GENERAL_HYPOTHERMIA:
		sprintf (buf2, "hypothermia");
		break;
	default:
		sprintf (buf2, "an unknown somatic effect");
		break;
	}

	ch->send_to_char("\n");
	if (ch->get_trust())
	{
		sprintf (buf,
			"#2%5d#0   Suffers from %s for %d more in-game hours.\n        Latency: %d hrs Power: %d to %d (%d @ %d min)\n        A: %d min, D: %d min, S: %d min, R: %d min\n",
				 type,
				 buf2,
				 duration,
				 latency,
				 max_power,
				 lvl_power,
				 atm_power,
				 minute,
				 attack,
				 decay,
				 sustain,
				 release);
	}
	else
	{
		sprintf (buf, "You suffer from %s.", buf2);
	}
	ch->send_to_char (buf);
}

void affect_soma_type::soma_ten_second_affect(CHAR_DATA* ch)
{
	int save = 0, stat = 0;
	const char *locat = NULL;
	char buf2[MAX_STRING_LENGTH]= { '\0' };
	char buf[MAX_STRING_LENGTH]= { '\0' };

	stat = ch->tmp_con;
	
		//if they make a random check against atm_power
		//or a random check agaisnt their con
		//they show no affect for this 10-second check
	if ((number (1, 1000) > atm_power)
		|| (number (1, (stat > 20) ? stat : 20) <= stat))
		return;

	switch (type)
	{
		/*    case MUSCULAR_CRAMP:       sprintf(buf2,"a muscle cramp"); break;
		case MUSCULAR_TWITCHING:      sprintf(buf2,"twitching"); break;
		case MUSCULAR_TREMOR: sprintf(buf2,"tremors"); break;
		case MUSCULAR_PARALYSIS:      sprintf(buf2,"paralysis"); break;
		case DIGESTIVE_ULCER: sprintf(buf2,"stomach ulcer"); break; 
		case DIGESTIVE_VOMITING:      sprintf(buf2,"vomiting"); break;
		case DIGESTIVE_BLEEDING:      sprintf(buf2,"vomiting blood"); break; 
		case EYE_BLINDNESS:   sprintf(buf2,"blindness"); break;
		case EYE_BLURRED:             sprintf(buf2,"blurred vision"); break;
		case EYE_DOUBLE:              sprintf(buf2,"double vision"); break;
		case EYE_DILATION:            sprintf(buf2,"dilated pupils"); break; 
		case EYE_CONTRACTION: sprintf(buf2,"contracted pupils"); break; 
		case EYE_LACRIMATION: sprintf(buf2,"lacrimation"); break;
		case EYE_PTOSIS:              sprintf(buf2,"ptosis"); break;
		case EAR_TINNITUS:            sprintf(buf2,"tinnitus"); break;
		case EAR_DEAFNESS:            sprintf(buf2,"deafness"); break;
		case EAR_EQUILLIBRIUM:        sprintf(buf2,"ear imbalance"); break;
		case NOSE_ANOSMIA:            sprintf(buf2,"anosmia"); break;
		case NOSE_RHINITIS:   sprintf(buf2,"rhinitis"); break;
		case MOUTH_SALIVATION:        sprintf(buf2,"salivation"); break;
		case MOUTH_TOOTHACHE: sprintf(buf2,"toothache"); break;
		case MOUTH_DRYNESS:   sprintf(buf2,"dry mouth"); break;
		case MOUTH_HALITOSIS: sprintf(buf2,"halitosis"); break;
		case CHEST_DIFFICULTY:        sprintf(buf2,"difficulty breathing"); break;
		case CHEST_RAPIDBREATH:       sprintf(buf2,"rapid breathing"); break;
		case CHEST_SLOWBREATH:        sprintf(buf2,"shallow breathing"); break;
		case CHEST_FLUID:             sprintf(buf2,"fluidous lungs"); break;
		case CHEST_PALPITATIONS:      sprintf(buf2,"heart palpitations"); break; */

	case CHEST_COUGHING:
				//save vs willpower to hold in cough
		stat = ch->tmp_wil;
		save = number (1, (stat > 20) ? stat : 20);

		if (get_affect (ch, MAGIC_HIDDEN) && ch->would_reveal())
		{
			if (save > stat)
			{
				remove_affect_type (ch, MAGIC_HIDDEN);
				ch->act("$n reveals $mself with an audible cough.", true, 0, 0,
					TO_ROOM);
			}
			else if (save > (stat / 2))
			{
				ch->act("You hear a muffled sound from somewhere nearby.", true,
					0, 0, TO_ROOM);
			}
		}
		else if ((save <= stat) && (save > (stat / 2)))
		{
			ch->act("$n tries to stifle a cough.", true, 0, 0, TO_ROOM);
		}

		if (save > stat)
		{
			ch->act("You cough audibly.", true, 0, 0, TO_CHAR);
		}
		else
		{
			ch->act("You try to stifle a cough silently.", true, 0, 0,
				TO_CHAR);
		}
		break;

	case CHEST_WHEEZING:

		stat = ch->tmp_wil;
		save = number (1, (stat > 20) ? stat : 20);

		if (get_affect (ch, MAGIC_HIDDEN) && ch->would_reveal())
		{
			if (save > stat)
			{
				remove_affect_type (ch, MAGIC_HIDDEN);
				ch->act("$n reveals $mself with an audible wheeze.", true, 0, 0,
					TO_ROOM);
			}
			else if (save > (stat / 2))
			{
				ch->act("You hear a muffled sound from somewhere nearby.", true,
					0, 0, TO_ROOM);
			}
		}
		else if ((save <= stat) && (save > (stat / 2)))
		{
			ch->act("$n tries to stifle their wheezing.", true, 0, 0, TO_ROOM);
		}

		if (save > stat)
		{
			ch->act("You wheeze audibly.", true, 0, 0, TO_CHAR);
		}
		else
		{
			ch->act("You try to stifle your wheezing.", true, 0, 0,
				TO_CHAR);
		}
		break;


	case NERVES_HEADACHE:

		stat = ch->tmp_wil;
		save = number (1, (stat > 20) ? stat : 20);

		if (save > stat)
		{
			ch->act("Your head pounds with a headache.", true, 0, 0, TO_CHAR);
		}
		else
		{
			ch->act("You manage to ignore the pounding in your head.", true, 0, 0,
				TO_CHAR);
		}
		break;

	
	


		/*  case CHEST_PNEUMONIA:      sprintf(buf2,"pneumonia"); break;
		case NERVES_PSYCHOSIS:      sprintf(buf2,"psychosis"); break;
		case NERVES_DELIRIUM:       sprintf(buf2,"delerium "); break;
		case NERVES_COMA:           sprintf(buf2,"a comatose state"); break;
		case NERVES_CONVULSIONS:    sprintf(buf2,"convulsions"); break;
		case NERVES_CONFUSION:      sprintf(buf2,"confusion"); break;
		case NERVES_PARETHESIAS:    sprintf(buf2,"parethesias"); break;
		case NERVES_ATAXIA: sprintf(buf2,"ataxia"); break;
		case NERVES_EQUILLIBRIUM:   sprintf(buf2,"nervous imbalance"); break;
		case SKIN_CYANOSIS: sprintf(buf2,"cyanosis of the skin"); break;
		case SKIN_DRYNESS:          sprintf(buf2,"dryness of the skin"); break;
		case SKIN_CORROSION:        sprintf(buf2,"corrosion of the skin"); break;
		case SKIN_JAUNDICE: sprintf(buf2,"jaundice of the skin"); break;
		case SKIN_REDNESS:          sprintf(buf2,"redness of the skin"); break;
		case SKIN_RASH:             sprintf(buf2,"a rash on the skin"); break;
		case SKIN_HAIRLOSS: sprintf(buf2,"hairloss"); break;
		case SKIN_EDEMA:            sprintf(buf2,"edema of the skin"); break;
		case SKIN_BURNS:            sprintf(buf2,"burns on the skin"); break;
		case SKIN_PALLOR:           sprintf(buf2,"pallor of the skin"); break;
		case SKIN_SWEATING: sprintf(buf2,"the sweats"); break;
		case GENERAL_WEIGHTLOSS:    sprintf(buf2,"weight loss"); break;
		case GENERAL_LETHARGY:      sprintf(buf2,"lethargy"); break;
		case GENERAL_APPETITELOSS:  sprintf(buf2,"appetite loss"); break;
		case GENERAL_PRESSUREDROP:  sprintf(buf2,"low blood pressure"); break;
		case GENERAL_PRESSURERISE:  sprintf(buf2,"high blood pressure"); break;
		case GENERAL_FASTPULSE:     sprintf(buf2,"a fast pulse"); break;
		case GENERAL_SLOWPULSE:     sprintf(buf2,"a slow pulse"); break;
		case GENERAL_HYPERTHERMIA:  sprintf(buf2,"hyperthermia"); break;
		 
		 /**/
		case GENERAL_HYPOTHERMIA:   sprintf(buf2,"hypothermia"); break;
		
	default:
		break;
	}
}

void affect_soma_type::soma_rl_minute_affect(CHAR_DATA* ch)
{
	minute++;
	
	switch (type)
	{
		/*    case MUSCULAR_CRAMP:       sprintf(buf2,"a muscle cramp"); break;
		case MUSCULAR_TWITCHING:      sprintf(buf2,"twitching"); break;
		case MUSCULAR_TREMOR: sprintf(buf2,"tremors"); break;
		case MUSCULAR_PARALYSIS:      sprintf(buf2,"paralysis"); break;
		case DIGESTIVE_ULCER: sprintf(buf2,"stomach ulcer"); break; 
		case DIGESTIVE_VOMITING:      sprintf(buf2,"vomiting"); break;
		case DIGESTIVE_BLEEDING:      sprintf(buf2,"vomiting blood"); break; 
		case EYE_BLINDNESS:   sprintf(buf2,"blindness"); break;
		case EYE_BLURRED:             sprintf(buf2,"blurred vision"); break;
		case EYE_DOUBLE:              sprintf(buf2,"double vision"); break;
		case EYE_DILATION:            sprintf(buf2,"dilated pupils"); break; 
		case EYE_CONTRACTION: sprintf(buf2,"contracted pupils"); break; 
		case EYE_LACRIMATION: sprintf(buf2,"lacrimation"); break;
		case EYE_PTOSIS:              sprintf(buf2,"ptosis"); break;
		case EAR_TINNITUS:            sprintf(buf2,"tinnitus"); break;
		case EAR_DEAFNESS:            sprintf(buf2,"deafness"); break;
		case EAR_EQUILLIBRIUM:        sprintf(buf2,"ear imbalance"); break;
		case NOSE_ANOSMIA:            sprintf(buf2,"anosmia"); break;
		case NOSE_RHINITIS:   sprintf(buf2,"rhinitis"); break;
		case MOUTH_SALIVATION:        sprintf(buf2,"salivation"); break;
		case MOUTH_TOOTHACHE: sprintf(buf2,"toothache"); break;
		case MOUTH_DRYNESS:   sprintf(buf2,"dry mouth"); break;
		case MOUTH_HALITOSIS: sprintf(buf2,"halitosis"); break;
		case CHEST_DIFFICULTY:        sprintf(buf2,"difficulty breathing"); break;
		case CHEST_WHEEZING:  sprintf(buf2,"wheezing"); break;
		case CHEST_RAPIDBREATH:       sprintf(buf2,"rapid breathing"); break;
		case CHEST_SLOWBREATH:        sprintf(buf2,"shallow breathing"); break;
		case CHEST_FLUID:             sprintf(buf2,"fluidous lungs"); break;
		case CHEST_PALPITATIONS:      sprintf(buf2,"heart palpitations"); break; */

	case MUSCULAR_CRAMP:
	case MUSCULAR_TWITCHING:
	case CHEST_COUGHING:
	case CHEST_WHEEZING:
	case NERVES_HEADACHE:

		if (minute <= attack)
		{
			atm_power = (max_power * minute) / attack;
		}
		else if (minute <= decay)
		{
			atm_power =
				max_power -
				(((max_power - lvl_power) *
				(minute - attack)) /
				(decay - attack));
		}

		else if (minute <= sustain)
		{
			atm_power = lvl_power;
		}
		else if (minute <= release)
		{
			atm_power =
				lvl_power -
				(((lvl_power) *
				(minute - sustain)) / 
				(release - sustain));
		}

		else
		{
			delete(this);
		}
		break;

		/*
		case CHEST_PNEUMONIA: sprintf(buf2,"pneumonia"); break;
		case NERVES_PSYCHOSIS:        sprintf(buf2,"psychosis"); break;
		case NERVES_DELIRIUM: sprintf(buf2,"delerium "); break;
		case NERVES_COMA:             sprintf(buf2,"a comatose state"); break;
		case NERVES_CONVULSIONS:      sprintf(buf2,"convulsions"); break;
		case NERVES_HEADACHE: sprintf(buf2,"headache"); break;
		case NERVES_CONFUSION:        sprintf(buf2,"confusion"); break;
		case NERVES_PARETHESIAS:      sprintf(buf2,"parethesias"); break;
		case NERVES_ATAXIA:   sprintf(buf2,"ataxia"); break;
		case NERVES_EQUILLIBRIUM:     sprintf(buf2,"nervous imbalance"); break;
		case SKIN_CYANOSIS:   sprintf(buf2,"cyanosis of the skin"); break;
		case SKIN_DRYNESS:            sprintf(buf2,"dryness of the skin"); break;
		case SKIN_CORROSION:  sprintf(buf2,"corrosion of the skin"); break;
		case SKIN_JAUNDICE:   sprintf(buf2,"jaundice of the skin"); break;
		case SKIN_REDNESS:            sprintf(buf2,"redness of the skin"); break;
		case SKIN_RASH:               sprintf(buf2,"a rash on the skin"); break;
		case SKIN_HAIRLOSS:   sprintf(buf2,"hairloss"); break;
		case SKIN_EDEMA:              sprintf(buf2,"edema of the skin"); break;
		case SKIN_BURNS:              sprintf(buf2,"burns on the skin"); break;
		case SKIN_PALLOR:             sprintf(buf2,"pallor of the skin"); break;
		case SKIN_SWEATING:   sprintf(buf2,"the sweats"); break;
		case GENERAL_WEIGHTLOSS:      sprintf(buf2,"weight loss"); break;
		case GENERAL_LETHARGY:        sprintf(buf2,"lethargy"); break;
		case GENERAL_APPETITELOSS:    sprintf(buf2,"appetite loss"); break;
		case GENERAL_PRESSUREDROP:    sprintf(buf2,"low blood pressure"); break;
		case GENERAL_PRESSURERISE:    sprintf(buf2,"high blood pressure"); break;
		case GENERAL_FASTPULSE:       sprintf(buf2,"a fast pulse"); break;
		case GENERAL_SLOWPULSE:       sprintf(buf2,"a slow pulse"); break;
		case GENERAL_HYPERTHERMIA:    sprintf(buf2,"hyperthermia"); break;
		case GENERAL_HYPOTHERMIA:     sprintf(buf2,"hypothermia"); break;
		*/
	default:
		break;
	}
}


int affect_soma_type::lookup_soma (char *argument)
{
	if (!argument)
		return (-1);

	if (!strcmp(argument, "cramps"))
		return (MUSCULAR_CRAMP);

	else if (!strcmp(argument,"twitching")) 
		return (MUSCULAR_TWITCHING);

	else if (!strcmp(argument, "cough"))
		return (CHEST_COUGHING);

	else if (!strcmp(argument, "wheeze"))
		return (CHEST_WHEEZING);

	else if (!strcmp(argument, "headache"))
		return (NERVES_HEADACHE);

	else
		return (-1);

}
