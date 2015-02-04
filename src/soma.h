//////////////////////////////////////////////////////////////////////////////
//
/// soma.h - Somatics structures
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

#define MUSCULAR_CRAMP             MAGIC_FIRST_SOMA + 1	/* pain echo */
#define MUSCULAR_TWITCHING         MAGIC_FIRST_SOMA + 2
#define MUSCULAR_TREMOR            MAGIC_FIRST_SOMA + 3
#define MUSCULAR_PARALYSIS         MAGIC_FIRST_SOMA + 4
#define DIGESTIVE_ULCER            MAGIC_FIRST_SOMA + 5	/* pain echo */
#define DIGESTIVE_VOMITING         MAGIC_FIRST_SOMA + 6
#define DIGESTIVE_BLEEDING         MAGIC_FIRST_SOMA + 7	/* vomiting blood */
#define EYE_BLINDNESS              MAGIC_FIRST_SOMA + 8
#define EYE_BLURRED                MAGIC_FIRST_SOMA + 9
#define EYE_DOUBLE                 MAGIC_FIRST_SOMA + 10
#define EYE_DILATION               MAGIC_FIRST_SOMA + 11	/* light sensitivity */
#define EYE_CONTRACTION            MAGIC_FIRST_SOMA + 12	/* tunnel vision? */
#define EYE_LACRIMATION            MAGIC_FIRST_SOMA + 13	/* watery eyes */
#define EYE_PTOSIS                 MAGIC_FIRST_SOMA + 14	/* drooping lids */
#define EAR_TINNITUS               MAGIC_FIRST_SOMA + 15	/* noise echo */
#define EAR_DEAFNESS               MAGIC_FIRST_SOMA + 16
#define EAR_EQUILLIBRIUM           MAGIC_FIRST_SOMA + 17	/* dizziness */
#define NOSE_ANOSMIA               MAGIC_FIRST_SOMA + 18	/* ignore aroma effects */
#define NOSE_RHINITIS              MAGIC_FIRST_SOMA + 19	/* itchy/runny nose */
#define MOUTH_SALIVATION           MAGIC_FIRST_SOMA + 20	/* heh */
#define MOUTH_TOOTHACHE            MAGIC_FIRST_SOMA + 21	/* pain echo / looseness */
#define MOUTH_DRYNESS              MAGIC_FIRST_SOMA + 22	/* echo */
#define MOUTH_HALITOSIS            MAGIC_FIRST_SOMA + 23
#define CHEST_DIFFICULTY           MAGIC_FIRST_SOMA + 24	/* pain echo */
#define CHEST_WHEEZING             MAGIC_FIRST_SOMA + 25
#define CHEST_RAPIDBREATH          MAGIC_FIRST_SOMA + 26
#define CHEST_SLOWBREATH           MAGIC_FIRST_SOMA + 27
#define CHEST_FLUID                MAGIC_FIRST_SOMA + 28
#define CHEST_PALPITATIONS         MAGIC_FIRST_SOMA + 29
#define CHEST_COUGHING             MAGIC_FIRST_SOMA + 30
#define CHEST_PNEUMONIA            MAGIC_FIRST_SOMA + 31
#define NERVES_PSYCHOSIS           MAGIC_FIRST_SOMA + 32
#define NERVES_DELIRIUM            MAGIC_FIRST_SOMA + 33
#define NERVES_COMA                MAGIC_FIRST_SOMA + 34	/* depression & drowsiness too */
#define NERVES_CONVULSIONS         MAGIC_FIRST_SOMA + 35
#define NERVES_HEADACHE            MAGIC_FIRST_SOMA + 36
#define NERVES_CONFUSION           MAGIC_FIRST_SOMA + 37	/* misdirection? ;) */
#define NERVES_PARETHESIAS         MAGIC_FIRST_SOMA + 38	/* am i on fire? ;) */
#define NERVES_ATAXIA              MAGIC_FIRST_SOMA + 39	/* --dex */
#define NERVES_EQUILLIBRIUM        MAGIC_FIRST_SOMA + 40
#define SKIN_CYANOSIS              MAGIC_FIRST_SOMA + 41
#define SKIN_DRYNESS               MAGIC_FIRST_SOMA + 42
#define SKIN_CORROSION             MAGIC_FIRST_SOMA + 43
#define SKIN_JAUNDICE              MAGIC_FIRST_SOMA + 44
#define SKIN_REDNESS               MAGIC_FIRST_SOMA + 45
#define SKIN_RASH                  MAGIC_FIRST_SOMA + 46
#define SKIN_HAIRLOSS              MAGIC_FIRST_SOMA + 47
#define SKIN_EDEMA                 MAGIC_FIRST_SOMA + 48
#define SKIN_BURNS                 MAGIC_FIRST_SOMA + 49
#define SKIN_PALLOR                MAGIC_FIRST_SOMA + 50
#define SKIN_SWEATING              MAGIC_FIRST_SOMA + 51
#define GENERAL_WEIGHTLOSS         MAGIC_FIRST_SOMA + 52
#define GENERAL_LETHARGY           MAGIC_FIRST_SOMA + 53
#define GENERAL_APPETITELOSS       MAGIC_FIRST_SOMA + 54
#define GENERAL_PRESSUREDROP       MAGIC_FIRST_SOMA + 55
#define GENERAL_PRESSURERISE       MAGIC_FIRST_SOMA + 56
#define GENERAL_FASTPULSE          MAGIC_FIRST_SOMA + 57
#define GENERAL_SLOWPULSE          MAGIC_FIRST_SOMA + 58
#define GENERAL_HYPERTHERMIA       MAGIC_FIRST_SOMA + 59
#define GENERAL_HYPOTHERMIA        MAGIC_FIRST_SOMA + 60
#define BREAK_ARM_L			    MAGIC_FIRST_SOMA + 61
#define BREAK_ARM_R			    MAGIC_FIRST_SOMA + 62
#define BREAK_LEG_L			    MAGIC_FIRST_SOMA + 63
#define BREAK_LEG_R			    MAGIC_FIRST_SOMA + 64
#define BREAK_HAND_R				MAGIC_FIRST_SOMA + 65
#define BREAK_HAND_L				MAGIC_FIRST_SOMA + 66
#define BREAK_FOOT_R				MAGIC_FIRST_SOMA + 67
#define BREAK_FOOT_L				MAGIC_FIRST_SOMA + 68
#define BREAK_COLLARBONE			MAGIC_FIRST_SOMA + 69
#define BREAK_RIB					MAGIC_FIRST_SOMA + 70
#define BREAK_HIP_L				MAGIC_FIRST_SOMA + 71
#define BREAK_HIP_R				MAGIC_FIRST_SOMA + 72
#define BREAK_SKULL				MAGIC_FIRST_SOMA + 73
#define BREAK_SHOULDER_L			MAGIC_FIRST_SOMA + 74
#define BREAK_SHOULDER_R			MAGIC_FIRST_SOMA + 75
#define BREAK_NECK					MAGIC_FIRST_SOMA + 76
#define BREAK_BACK					MAGIC_FIRST_SOMA + 77
#define BREAK_JAW					MAGIC_FIRST_SOMA + 78
#define BREAK_NOSE					MAGIC_FIRST_SOMA + 79
#define BREAK_EYE_L				MAGIC_FIRST_SOMA + 80
#define BREAK_EYE_R				MAGIC_FIRST_SOMA + 81
#define SPRAIN_ARM_L			    MAGIC_FIRST_SOMA + 82
#define SPRAIN_ARM_R			    MAGIC_FIRST_SOMA + 83
#define SPRAIN_LEG_L			    MAGIC_FIRST_SOMA + 84
#define SPRAIN_LEG_R			    MAGIC_FIRST_SOMA + 85
#define SPRAIN_HAND_R				MAGIC_FIRST_SOMA + 86
#define SPRAIN_HAND_L				MAGIC_FIRST_SOMA + 87
#define SPRAIN_FOOT_R				MAGIC_FIRST_SOMA + 88
#define SPRAIN_FOOT_L				MAGIC_FIRST_SOMA + 89
#define SPRAIN_COLLARBONE			MAGIC_FIRST_SOMA + 90
#define SPRAIN_RIB					MAGIC_FIRST_SOMA + 91
#define SPRAIN_HIP_L				MAGIC_FIRST_SOMA + 92
#define SPRAIN_HIP_R				MAGIC_FIRST_SOMA + 93
#define SPRAIN_SHOULDER_L			MAGIC_FIRST_SOMA + 94
#define SPRAIN_SHOULDER_R			MAGIC_FIRST_SOMA + 95
#define SPRAIN_NECK				MAGIC_FIRST_SOMA + 96
#define SPRAIN_BACK				MAGIC_FIRST_SOMA + 97
#define SPRAIN_JAW					MAGIC_FIRST_SOMA + 98
#define BLACK_EYE_L				MAGIC_FIRST_SOMA + 99
#define BLACK_EYE_R				MAGIC_FIRST_SOMA + 100
#define SEVERED_ARM_L			    MAGIC_FIRST_SOMA + 101
#define SEVERED_ARM_R			    MAGIC_FIRST_SOMA + 102
#define SEVERED_LEG_L			    MAGIC_FIRST_SOMA + 103
#define SEVERED_LEG_R			    MAGIC_FIRST_SOMA + 104
#define SEVERED_HAND_R				MAGIC_FIRST_SOMA + 105
#define SEVERED_HAND_L				MAGIC_FIRST_SOMA + 106
#define SEVERED_FOOT_R				MAGIC_FIRST_SOMA + 107
#define SEVERED_FOOT_L				MAGIC_FIRST_SOMA + 108

/*
 SOMATIC RESPONSES by Sighentist
 
 Basically the idea here, is that the magnitude of the effect is
 enveloped over time. So after a delay, the effect begins to grow
 in strength until it reaches a peak. After cresting the peak the
 strength weakens to a fraction of the peak strength. It remains
 fixed there until the effect begins to wear off completely.
 
 Notes:
 If latency == duration, the affected is a carrier.
 
 */


class affect_soma_type 
{
	public:
	int type;				// soma type
	int duration;			//* rl minutes 
	int latency;			//* rl minutes of delay 
	
	int minute;				//* timer  
	int max_power;			//* type-dependant value 
	int lvl_power;			//* fraction of max_power
	int atm_power;			//* the current power 
	
	int attack;				//* RL minutes for amp up to max_power 
	int decay;				//* RL minutes for decay down to lvl_power
	int sustain;			//* RL minutes for lvl_power to drop 
	uint release;			//* RL minutes until end of effect 
	
	void soma_stat(CHAR_DATA* ch);
	void soma_ten_second_affect(CHAR_DATA* ch);
	void soma_rl_minute_affect(CHAR_DATA* ch);
	int lookup_soma (char *argument);
};


