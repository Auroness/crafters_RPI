//////////////////////////////////////////////////////////////////////////////
//
/// weather.cpp - Weather Module
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

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"

extern const char *wind_directions[];

Weather weather_info[MAX_WEATHER_AREAS]; //(Tirith, Angrenost)

int moon_light[MAX_WEATHER_ZONES];
float global_sun_light = 0.0;
int desc_weather[MAX_WEATHER_ZONES];
AFFECTED_TYPE *world_affects = NULL;
int weather_zone_table[MAX_WEATHER_ZONES]; //(temperate, artic, desert)
struct moon_data global_moon_values;

const int sunrise[] = { 7, 7, 6, 6, 5, 5, 4, 5, 5, 6, 6, 7 };
const int sunset[] = { 17, 18, 18, 19, 19, 20, 20, 20, 19, 19, 18, 18 };

const int seasonal_temp[7][12] = {
		//{40, 47, 55, 63, 71, 79, 85, 79, 71, 63, 55, 47},	/* Temperate baseline */
	{38, 41, 48, 58, 66, 75, 79, 77, 71, 60, 50, 41}, // Richmond Virginia - Angrenost

	{22, 29, 37, 45, 53, 61, 67, 61, 53, 45, 37, 29},	/* Cool */
	{15, 22, 28, 38, 46, 50, 55, 50, 46, 38, 28, 22},	/* Cold */
	{7, 14, 18, 22, 25, 27, 30, 27, 25, 22, 18, 14},	/* Arctic */

	{55, 57, 60, 65, 73, 81, 89, 81, 73, 65, 60, 53},	/* Hot */
	{60, 62, 65, 70, 78, 86, 94, 86, 78, 70, 65, 58},	/* Hot */
	{75, 77, 80, 85, 93, 101, 109, 101, 93, 85, 80, 73}	/* Desert */
};



void
initialize_weather_zones (void)
{
	int index;
	
		//holds the type of weather (temperate, artic, desert)
	for (index = 0; index < MAX_WEATHER_ZONES; index ++)
	{
	weather_zone_table[index] = index;
	}
		
		//weather conditions by wzone (Angernost, Minas Tirith)
	for (index = 0; index < MAX_WEATHER_AREAS; index ++)
	{
		desc_weather[index] = WR_NORMAL;
	}
}



void
weather_create (void)
{
	int last_temp = 0, last_clouds = 0, last_state = 0;
	int roll = 0, chance_of_rain = 0, last_fog = 0, i = 0;
	char buf[MAX_STRING_LENGTH]= { '\0' };
	char storm[MAX_STRING_LENGTH]= { '\0' };
	char wind[AVG_STRING_LENGTH] = { '\0' };

		//0 is WEATHER_ANGRENOST or WEATHER_TIRITH
	for (i = 0; i < MAX_WEATHER_AREAS; i++)
	{
		chance_of_rain = 0;

		last_temp = weather_info[i].temperature;
		last_clouds = weather_info[i].clouds;
		last_state = weather_info[i].state;
		last_fog = weather_info[i].fog;

		weather_info[i].temperature =
			seasonal_temp[weather_zone_table[i]][time_info.month];

		if (time_info.hour == sunrise[time_info.month])
			weather_info[i].trend =
			(weather_info[i].trend * 2 + number (0, 15)) / 3;

		weather_info[i].temperature += weather_info[i].trend;

		/*** If there is a wind there is a chance it will change direction ***/
		/*
		I do not claim to be knowledgeable in meteorology so this
		code is going to be far from scientific. I am working on
		the following principle, however:

		The wind is more likely to change direction a little than
		to completely turn around and blow in the opposite direction.
		That assumption made, whatever the current direction of the wind,
		the chance of it moving to another direction gets lower as the
		directions get further from the current one, with the least
		likely being the absolute opposite direction.

		- Valarauka
		*/
		if ((weather_info[i].wind_speed < STORMY)
			&& (weather_info[i].wind_speed > CALM))
		{
			if (weather_info[i].wind_dir == WEST_WIND) //currently westerly
			{
				/* Check for changes: most likely -> least likely */

				/* Southwest and Northwest are most likely */
				if (!number (0, 5))
				{
					send_outside_zone
						("The westerly winds veer a little, coming more now from the southwest.\n",
						i);
					weather_info[i].wind_dir = SOUTHWEST_WIND;
				}
				else if (!number (0, 5))
				{
					send_outside_zone
						("The westerly winds veer a little, coming more now from the northwest.\n",
						i);
					weather_info[i].wind_dir = NORTHWEST_WIND;
				}

				/* Next most likely are North and South */
				else if (!number (0, 10))
				{
					send_outside_zone
						("The westerly wind turns and begins to blow more from the north.\n",
						i);
					weather_info[i].wind_dir = NORTH_WIND;
				}
				else if (!number (0, 10))
				{
					send_outside_zone
						("The westerly wind turns and begins to blow more from the south.\n",
						i);
					weather_info[i].wind_dir = SOUTH_WIND;
				}

				/* Next most likely are Northeast and Southeast */
				else if (!number (0, 20))
				{
					send_outside_zone
						("The westerly wind turns dramatically, moving almost back on itself to blow from the southeast.\n",
						i);
					weather_info[i].wind_dir = SOUTHEAST_WIND;
				}
				else if (!number (0, 20))
				{
					send_outside_zone
						("The westerly wind turns dramatically, moving almost back on itself to blow from the northeast.\n",
						i);
					weather_info[i].wind_dir = NORTHEAST_WIND;
				}

				/* Least likely is an Easterly wind*/
				else if (!number (0, 25))
				{
					send_outside_zone
						("The westerly wind suddenly reverses, blowing back on itself now from the east.\n",
						i);
					weather_info[i].wind_dir = EAST_WIND;
				}

				/* If none of these happen, wind remains in the current direction for now */

			}
			else if (weather_info[i].wind_dir == SOUTHWEST_WIND) //currently southwesterly
			{
				/* Check for changes: most likely -> least likely */

				/* South and west are most likely */
				if (!number (0, 5))
				{
					send_outside_zone
						("The south westerly winds veer a little, coming more now from the south.\n",
						i);
					weather_info[i].wind_dir = SOUTH_WIND;
				}
				else if (!number (0, 5))
				{
					send_outside_zone
						("The south westerly winds veer a little, coming more now from the west.\n",
						i);
					weather_info[i].wind_dir = WEST_WIND;
				}

				/* Next most likely are Northwest and Southeast */
				else if (!number (0, 10))
				{
					send_outside_zone
						("The south westerly wind turns and begins to blow more from the north west.\n",
						i);
					weather_info[i].wind_dir = NORTHWEST_WIND;
				}
				else if (!number (0, 10))
				{
					send_outside_zone
						("The south westerly wind turns and begins to blow more from the south east.\n",
						i);
					weather_info[i].wind_dir = SOUTHEAST_WIND;
				}

				/* Next most likely are North and East */
				else if (!number (0, 20))
				{
					send_outside_zone
						("The south westerly wind turns dramatically, moving almost back on itself to blow from the north.\n",
						i);
					weather_info[i].wind_dir = NORTH_WIND;
				}
				else if (!number (0, 20))
				{
					send_outside_zone
						("The south westerly wind turns dramatically, moving almost back on itself to blow from the east.\n",
						i);
					weather_info[i].wind_dir = EAST_WIND;
				}

				/* Least likely is a northeasterly wind*/
				else if (!number (0, 25))
				{
					send_outside_zone
						("The south westerly wind suddenly reverses, blowing back on itself now from the north east.\n",
						i);
					weather_info[i].wind_dir = NORTHEAST_WIND;
				}

				/* If none of these happen, wind remains in the current direction for now */
			}
			else if (weather_info[i].wind_dir == SOUTH_WIND) //currently southerly
			{
				/* Check for changes: most likely -> least likely */

				/* Southeast and Southwest are most likely */
				if (!number (0, 5))
				{
					send_outside_zone
						("The southerly winds veer a little, coming more now from the south east.\n",
						i);
					weather_info[i].wind_dir = SOUTHEAST_WIND;
				}
				else if (!number (0, 5))
				{
					send_outside_zone
						("The southerly winds veer a little, coming more now from the south west.\n",
						i);
					weather_info[i].wind_dir = SOUTHWEST_WIND;
				}

				/* Next most likely are west and east */
				else if (!number (0, 10))
				{
					send_outside_zone
						("The southerly wind turns and begins to blow more from the west.\n",
						i);
					weather_info[i].wind_dir = WEST_WIND;
				}
				else if (!number (0, 10))
				{
					send_outside_zone
						("The southerly wind turns and begins to blow more from the east.\n",
						i);
					weather_info[i].wind_dir = EAST_WIND;
				}

				/* Next most likely are Northwest and Northeast */
				else if (!number (0, 20))
				{
					send_outside_zone
						("The southerly wind turns dramatically, moving almost back on itself to blow from the north west.\n",
						i);
					weather_info[i].wind_dir = NORTHWEST_WIND;
				}
				else if (!number (0, 20))
				{
					send_outside_zone
						("The southerly wind turns dramatically, moving almost back on itself to blow from the north east.\n",
						i);
					weather_info[i].wind_dir = NORTHEAST_WIND;
				}

				/* Least likely is a northerly wind*/
				else if (!number (0, 25))
				{
					send_outside_zone
						("The southerly wind suddenly reverses, blowing back on itself now from the north.\n",
						i);
					weather_info[i].wind_dir = NORTH_WIND;
				}

				/* If none of these happen, wind remains in the current direction for now */
			}
			else if (weather_info[i].wind_dir == SOUTHEAST_WIND) //currently southeasterly
			{
				/* Check for changes: most likely -> least likely */

				/* East and South are most likely */
				if (!number (0, 5))
				{
					send_outside_zone
						("The south easterly winds veer a little, coming more now from the east.\n",
						i);
					weather_info[i].wind_dir = EAST_WIND;
				}
				else if (!number (0, 5))
				{
					send_outside_zone
						("The south easterly winds veer a little, coming more now from the south.\n",
						i);
					weather_info[i].wind_dir = SOUTH_WIND;
				}

				/* Next most likely are southwest and northeast */
				else if (!number (0, 10))
				{
					send_outside_zone
						("The south easterly wind turns and begins to blow more from the south west.\n",
						i);
					weather_info[i].wind_dir = SOUTHWEST_WIND;
				}
				else if (!number (0, 10))
				{
					send_outside_zone
						("The south easterly wind turns and begins to blow more from the north east.\n",
						i);
					weather_info[i].wind_dir = NORTHEAST_WIND;
				}

				/* Next most likely are West and North */
				else if (!number (0, 20))
				{
					send_outside_zone
						("The south easterly wind turns dramatically, moving almost back on itself to blow from the west.\n",
						i);
					weather_info[i].wind_dir = WEST_WIND;
				}
				else if (!number (0, 20))
				{
					send_outside_zone
						("The south easterly wind turns dramatically, moving almost back on itself to blow from the north.\n",
						i);
					weather_info[i].wind_dir = NORTH_WIND;
				}

				/* Least likely is a northwesterly wind*/
				else if (!number (0, 25))
				{
					send_outside_zone
						("The westerly wind suddenly reverses, blowing back on itself now from the north west.\n",
						i);
					weather_info[i].wind_dir = NORTHWEST_WIND;
				}

				/* If none of these happen, wind remains in the current direction for now */
			}
			else if (weather_info[i].wind_dir == EAST_WIND) //currently easterly
			{
				/* Check for changes: most likely -> least likely */

				/* Northeast and Southeast are most likely */
				if (!number (0, 5))
				{
					send_outside_zone
						("The easterly winds veer a little, coming more now from the north east.\n",
						i);
					weather_info[i].wind_dir = NORTHEAST_WIND;
				}
				else if (!number (0, 5))
				{
					send_outside_zone
						("The easterly winds veer a little, coming more now from the south east.\n",
						i);
					weather_info[i].wind_dir = SOUTHEAST_WIND;
				}

				/* Next most likely are north and south */
				else if (!number (0, 10))
				{
					send_outside_zone
						("The easterly wind turns and begins to blow more from the north.\n",
						i);
					weather_info[i].wind_dir = NORTH_WIND;
				}
				else if (!number (0, 10))
				{
					send_outside_zone
						("The easterly wind turns and begins to blow more from the south.\n",
						i);
					weather_info[i].wind_dir = SOUTH_WIND;
				}

				/* Next most likely are Northwest and Southwest */
				else if (!number (0, 20))
				{
					send_outside_zone
						("The easterly wind turns dramatically, moving almost back on itself to blow from the north west.\n",
						i);
					weather_info[i].wind_dir = NORTHWEST_WIND;
				}
				else if (!number (0, 20))
				{
					send_outside_zone
						("The easterly wind turns dramatically, moving almost back on itself to blow from the south west.\n",
						i);
					weather_info[i].wind_dir = SOUTHWEST_WIND;
				}

				/* Least likely is a westerly wind*/
				else if (!number (0, 25))
				{
					send_outside_zone
						("The easterly wind suddenly reverses, blowing back on itself now from the west.\n",
						i);
					weather_info[i].wind_dir = WEST_WIND;
				}

				/* If none of these happen, wind remains in the current direction for now */
			}
			else if (weather_info[i].wind_dir == NORTHEAST_WIND) //currently northeasterly
			{
				/* Check for changes: most likely -> least likely */

				/* North and East are most likely */
				if (!number (0, 5))
				{
					send_outside_zone
						("The north easterly winds veer a little, coming more now from the north.\n",
						i);
					weather_info[i].wind_dir = NORTH_WIND;
				}
				else if (!number (0, 5))
				{
					send_outside_zone
						("The north easterly winds veer a little, coming more now from the east.\n",
						i);
					weather_info[i].wind_dir = EAST_WIND;
				}

				/* Next most likely are northwest and southeast */
				else if (!number (0, 10))
				{
					send_outside_zone
						("The north easterly wind turns and begins to blow more from the north west.\n",
						i);
					weather_info[i].wind_dir = NORTHWEST_WIND;
				}
				else if (!number (0, 10))
				{
					send_outside_zone
						("The north easterly wind turns and begins to blow more from the south east.\n",
						i);
					weather_info[i].wind_dir = SOUTHEAST_WIND;
				}

				/* Next most likely are West and South */
				else if (!number (0, 20))
				{
					send_outside_zone
						("The north easterly wind turns dramatically, moving almost back on itself to blow from the west.\n",
						i);
					weather_info[i].wind_dir = WEST_WIND;
				}
				else if (!number (0, 20))
				{
					send_outside_zone
						("The north easterly wind turns dramatically, moving almost back on itself to blow from the south.\n",
						i);
					weather_info[i].wind_dir = SOUTH_WIND;
				}

				/* Least likely is a southwesterly wind*/
				else if (!number (0, 25))
				{
					send_outside_zone
						("The north easterly wind suddenly reverses, blowing back on itself now from the south west.\n",
						i);
					weather_info[i].wind_dir = SOUTHWEST_WIND;
				}

				/* If none of these happen, wind remains in the current direction for now */
			}
			else if (weather_info[i].wind_dir == NORTH_WIND) //currently northerly
			{
				/* Check for changes: most likely -> least likely */

				/* Northeast and Northwest are most likely */
				if (!number (0, 5))
				{
					send_outside_zone
						("The northerly winds veer a little, coming more now from the north east.\n",
						i);
					weather_info[i].wind_dir = NORTHEAST_WIND;
				}
				else if (!number (0, 5))
				{
					send_outside_zone
						("The northerly winds veer a little, coming more now from the north north west.\n",
						i);
					weather_info[i].wind_dir = NORTHWEST_WIND;
				}

				/* Next most likely are west and east */
				else if (!number (0, 10))
				{
					send_outside_zone
						("The northerly wind turns and begins to blow more from the west.\n",
						i);
					weather_info[i].wind_dir = WEST_WIND;
				}
				else if (!number (0, 10))
				{
					send_outside_zone
						("The northerly wind turns and begins to blow more from the east.\n",
						i);
					weather_info[i].wind_dir = EAST_WIND;
				}

				/* Next most likely are Southeast and Southwest */
				else if (!number (0, 20))
				{
					send_outside_zone
						("The northerly wind turns dramatically, moving almost back on itself to blow from the south east.\n",
						i);
					weather_info[i].wind_dir = SOUTHEAST_WIND;
				}
				else if (!number (0, 20))
				{
					send_outside_zone
						("The northerly wind turns dramatically, moving almost back on itself to blow from the south west.\n",
						i);
					weather_info[i].wind_dir = SOUTHWEST_WIND;
				}

				/* Least likely is a southerly wind*/
				else if (!number (0, 25))
				{
					send_outside_zone
						("The northerly wind suddenly reverses, blowing back on itself now from the south.\n",
						i);
					weather_info[i].wind_dir = SOUTH_WIND;
				}

				/* If none of these happen, wind remains in the current direction for now */
			}
			else if (weather_info[i].wind_dir == NORTHWEST_WIND) //currently northwesterly
			{
				/* Check for changes: most likely -> least likely */

				/* North and West are most likely */
				if (!number (0, 5))
				{
					send_outside_zone
						("The north westerly winds veer a little, coming more now from the north.\n",
						i);
					weather_info[i].wind_dir = NORTH_WIND;
				}
				else if (!number (0, 5))
				{
					send_outside_zone
						("The north westerly winds veer a little, coming more now from the west.\n",
						i);
					weather_info[i].wind_dir = WEST_WIND;
				}

				/* Next most likely are southwest and northeast */
				else if (!number (0, 10))
				{
					send_outside_zone
						("The north westerly wind turns and begins to blow more from the southwest.\n",
						i);
					weather_info[i].wind_dir = SOUTHWEST_WIND;
				}
				else if (!number (0, 10))
				{
					send_outside_zone
						("The north westerly wind turns and begins to blow more from the northeast.\n",
						i);
					weather_info[i].wind_dir = NORTHEAST_WIND;
				}

				/* Next most likely are east and South */
				else if (!number (0, 20))
				{
					send_outside_zone
						("The north westerly wind turns dramatically, moving almost back on itself to blow from the east.\n",
						i);
					weather_info[i].wind_dir = EAST_WIND;
				}
				else if (!number (0, 20))
				{
					send_outside_zone
						("The north westerly wind turns dramatically, moving almost back on itself to blow from the south.\n",
						i);
					weather_info[i].wind_dir = SOUTH_WIND;
				}

				/* Least likely is a southeasterly wind*/
				else if (!number (0, 25))
				{
					send_outside_zone
						("The north westerly wind suddenly reverses, blowing back on itself now from the southeast.\n",
						i);
					weather_info[i].wind_dir = SOUTHEAST_WIND;
				}

				/* If none of these happen, wind remains in the current direction for now */
			}
		}
		/*** End wind direction code ***/

			//clouds based on season, wind and pre-existing clouds
		if (!number (0, 15) && weather_info[i].wind_speed)
		{
			roll = number (0, 99);
			roll += weather_info[i].temperature / 3;

			if ((time_info.season == SPRING) || (time_info.season == AUTUMN))
			{
				if (roll < 20)
				{
					if (last_clouds == OVERCAST)
						weather_info[i].clouds = HEAVY_CLOUDS;
					else
						weather_info[i].clouds = CLEAR_SKY;
				}
				else if (roll < 45)
					weather_info[i].clouds = LIGHT_CLOUDS;
				else if (roll < 80)
					weather_info[i].clouds = HEAVY_CLOUDS;
				else
				{
					if (last_clouds == CLEAR_SKY)
						weather_info[i].clouds = LIGHT_CLOUDS;
					else
						weather_info[i].clouds = OVERCAST;
				}
			}
			else if (time_info.season == SUMMER)
			{
				if (roll < 50)
				{
					if (last_clouds == OVERCAST)
						weather_info[i].clouds = HEAVY_CLOUDS;
					else
						weather_info[i].clouds = CLEAR_SKY;
				}
				else if (roll < 80)
					weather_info[i].clouds = LIGHT_CLOUDS;
				else if (roll < 90)
					weather_info[i].clouds = HEAVY_CLOUDS;
				else
				{
					if (last_clouds == CLEAR_SKY)
						weather_info[i].clouds = LIGHT_CLOUDS;
					else
						weather_info[i].clouds = OVERCAST;
				}
			}
			else
			{
				if (roll < 10)
				{
					if (last_clouds == OVERCAST)
						weather_info[i].clouds = HEAVY_CLOUDS;
					else
						weather_info[i].clouds = CLEAR_SKY;
				}
				else if (roll < 25)
					weather_info[i].clouds = LIGHT_CLOUDS;
				else if (roll < 75)
					weather_info[i].clouds = HEAVY_CLOUDS;
				else
				{
					if (last_clouds == CLEAR_SKY)
						weather_info[i].clouds = LIGHT_CLOUDS;
					else
						weather_info[i].clouds = OVERCAST;
				}
			}
		}

		sprintf(wind, "%s", wind_directions[weather_info[i].wind_dir]);
		
			//cloud changes
		if (weather_info[i].fog < THICK_FOG)
		{
			if ((weather_info[i].clouds == CLEAR_SKY)
				&& (weather_info[i].clouds != last_clouds))
			{
				sprintf (buf,
					"The clouds are born away upon the prevailing %s winds and clear skies open up above.\n",
					wind);
				send_outside_zone (buf, i);

			}

			if ((weather_info[i].clouds == LIGHT_CLOUDS)
				&& (weather_info[i].clouds != last_clouds))
			{

				if (last_clouds > weather_info[i].clouds)
				{
					sprintf (buf,
						"The cloud cover begins to clear, carried away upon the prevailing %s winds.\n",
						wind);
					send_outside_zone (buf, i);
				}
				if (last_clouds < weather_info[i].clouds)
				{
					sprintf (buf,
						"Wisplike clouds drift in overhead, carried upon the prevailing %s winds.\n",
						wind);
					send_outside_zone (buf, i);
				}  
			}

			if ((weather_info[i].clouds == HEAVY_CLOUDS)
				&& (weather_info[i].clouds != last_clouds))
			{
				if (last_clouds < weather_info[i].clouds)
				{
					sprintf (buf,
						"A host of clouds marches overhead upon the prevailing %s winds.\n",
						wind);
					send_outside_zone (buf, i);
				}
				if (last_clouds > weather_info[i].clouds)
				{
					sprintf (buf,
						"Small patches of sky open up as the storm clouds drift away on the prevailing %s winds.\n",
						wind);
					send_outside_zone (buf, i);
				}
			}

			if ((weather_info[i].clouds == OVERCAST)
				&& (weather_info[i].clouds != last_clouds))
			{
				if (global_sun_light > SUN_TWILIGHT)
				{
					send_outside_zone
					("The prevailing winds bring a blanket of thick storm clouds to obscure Anor.\n",
					i);
				}
				else
					send_outside_zone
					("The prevailing winds bring a blanket of thick storm clouds into the sky.\n",
					i);
			}
		}

			//what is the chance of rain, based on cloud cover and season
		if (weather_info[i].clouds != last_clouds)
		{			/*   Is the new front a rain front?   */

			if (time_info.season == SPRING)	/*   Spring rains   */
				chance_of_rain = 20;

			if (weather_info[i].clouds == CLEAR_SKY)	/*   More clouds = Higher chance of rain   */
				chance_of_rain = 0;
			else
				chance_of_rain += (weather_info[i].clouds * 15);

			if (weather_zone_table[i] == WEATHER_DESERT)
				chance_of_rain = 0;

			if (number (0, 99) < chance_of_rain)
			{
				weather_info[i].state = CHANCE_RAIN;
			}
			else if ((last_state > CHANCE_RAIN) && (last_state < LIGHT_SNOW))
			{
				weather_info[i].state = NO_RAIN;
				send_outside_zone ("The rain passes.\n", i);
			}
			else if (last_state > HEAVY_RAIN)
			{
				weather_info[i].state = NO_RAIN;
				send_outside ("It stops snowing.\n");
			}
		}

		/*   Lightning is more common the closer you get to midsummer. I wanted it to be more common with higher temperatures, but we haven't determined temp and we need to know now.   */


		if ((weather_info[i].clouds > LIGHT_CLOUDS)
			&& (weather_info[i].state > NO_RAIN))
		{
			if (number (35, 350) <
				seasonal_temp[weather_zone_table[i]][time_info.month])
			{
				if (number (1, 10) && number (1, 3) < weather_info[i].clouds)
				{
					weather_info[i].lightning = 1;
					send_outside_zone
						("Lightning flashes across the heavens.\n", i);
				}
				else
					weather_info[i].lightning = 0;
			}
		}

			//wind changes direction and strength semi-randomly
		if (!number (0, 4))
		{
			roll = number (-1, 1);
			switch (roll)
			{
			case -1:
				if (weather_info[i].wind_speed == CALM)
					break;
				if (weather_info[i].wind_speed == BREEZE && number (0, 1))
					break;
				weather_info[i].wind_speed -= 1;
				if (weather_info[i].wind_speed == CALM)
					send_outside_zone ("The winds die and the air stills.\n",
					i);
				if (weather_info[i].wind_speed == BREEZE)
					send_outside_zone ("The wind dies down to a mild breeze.\n",
					i);
				if (weather_info[i].wind_speed == WINDY)
					send_outside_zone
					("The gale winds die down to a steady current.\n", i);
				if (weather_info[i].wind_speed == GALE)
					send_outside_zone
					("The stormy winds slow to a steady gale.\n", i);
				break;

			case 1:
				sprintf (storm, "wind storm");
				if (weather_info[i].state > CHANCE_RAIN)
					sprintf (storm, "rain storm");
				if (weather_info[i].lightning)
					sprintf (storm, "thunder storm");
				if (weather_info[i].state > HEAVY_RAIN)
					sprintf (storm, "blizzard");
				if (weather_info[i].wind_speed == STORMY)
				{
					send_outside_zone
						("The storm winds slow, leaving a steady gale in their wake.\n",
						i);
					weather_info[i].wind_speed -= 1;
					break;
				}
				if (weather_info[i].wind_speed == CALM)
					send_outside_zone ("A capricious breeze picks up.\n", i);
				if (weather_info[i].wind_speed == BREEZE)
				{
					if (number (0, 1))
						break;
					send_outside_zone
						("The breeze strengthens into a steady wind.\n", i);
				}
				if (weather_info[i].wind_speed == WINDY)
				{
					if (!number (0, 3))
						break;
					if (weather_info[i].state < LIGHT_RAIN)
						send_outside_zone
						("The winds grow fierce, building into a strong gale.\n",
						i);
					else
					{
						if (weather_info[i].state > HEAVY_RAIN)
							sprintf (storm, "snow storm");
						sprintf (buf,
							"The winds grow fierce, building into a mild %s.\n",
							storm);
						send_outside_zone (buf, i);
					}
				}
				if (weather_info[i].wind_speed == GALE)
				{
					if (!number (0, 5))
						break;
					sprintf (buf,
						"The winds begin to rage, and a fierce %s is born.\n",
						storm);
					send_outside_zone (buf, i);
				}
				weather_info[i].wind_speed += 1;
				break;
			}
		}

			//temperature adjusted by wind speed and direction
		/*   Wind Chill - This is FAR from scientific, but I didnt want winds to totally take over temperatures. - Koldryn  */
		if (weather_info[i].wind_dir == NORTH_WIND)
		{
			weather_info[i].temperature -= weather_info[i].wind_speed * 2;
			roll = 0 - weather_info[i].wind_speed * 2;
		}

		if (weather_info[i].wind_dir == WEST_WIND)
		{
			weather_info[i].temperature += (5 - weather_info[i].wind_speed * 2);
			roll = 0 + (5 - weather_info[i].wind_speed * 2);
		}

			//temperature based on time of day 
			//roll is peak heat time of the day
		/*   Angle of Sunlight   */
		if (global_sun_light > SUN_TWILIGHT)
		{
			roll = ((sunrise[time_info.month] + sunset[time_info.month]) / 2);
			
			if (time_info.hour > roll)
				roll =
				(sunset[time_info.month] - time_info.hour) * 100 / 
				(sunset[time_info.month] - roll) * 15 / 100;
			else if (time_info.hour == roll)
				roll = 15;
			else if (time_info.hour < roll)
				roll =
				(time_info.hour - sunrise[time_info.month]) * 100 /
				(roll - sunrise[time_info.month]) * 15 / 100;
			

			weather_info[i].temperature += roll;
		}

		/*   Cloud Chill, which applies only in the daytime - This is not scientific.   */
			//nighttime cool down of temperature
		if (global_sun_light > SUN_TWILIGHT)
		{
			if (weather_info[i].clouds == LIGHT_CLOUDS)
				weather_info[i].temperature -= ((roll * 3) / 10);
			if (weather_info[i].clouds == HEAVY_CLOUDS)
				weather_info[i].temperature -= ((roll * 6) / 10);
			if (weather_info[i].clouds == OVERCAST)
				weather_info[i].temperature -= ((roll * 9) / 10);
			weather_info[i].temperature = ((weather_info[i].temperature + last_temp * 2) / 3);	/*   Limits Drastic Immediate Changes   */
		}
		else
		{
			if (weather_zone_table[i] == WEATHER_DESERT)
				weather_info[i].temperature -= 50;
			else if ((time_info.season == SPRING) || (time_info.season == AUTUMN))	/*   Gradual Nighttime Cooling   */
				weather_info[i].temperature -= 10;
			else if (time_info.season == SUMMER)
				weather_info[i].temperature -= 15;
			else
				weather_info[i].temperature -= 5;
			roll = 0;
			if (time_info.hour != sunset[time_info.month])
				roll = 5;
			weather_info[i].temperature =
				((weather_info[i].temperature + (last_temp + roll) * 4) / 5);
			weather_info[i].temperature -= 5;	/*   Immediate Nighttime Chill   */
		}

		/*   Will it rain?   */
		if (weather_info[i].state == CHANCE_RAIN)
		{
			chance_of_rain = (weather_info[i].clouds * 15);
			if (time_info.season == SUMMER)
				chance_of_rain -= 20;
			if (time_info.season == AUTUMN)
				chance_of_rain -= 10;
			if (time_info.season == SPRING)
				chance_of_rain += 10;
			chance_of_rain = MAX (1, chance_of_rain);
			if (weather_info[i].lightning && number (0, 39))	/*   Its very rare for lightning not to cause rain   */
				chance_of_rain = 100;

			if (number (0, 99) < chance_of_rain)
				weather_info[i].state = weather_info[i].clouds + 1;
		}

		/*   If its going to rain, how hard?   */
			//will snow if temp is low enough
		if (weather_info[i].state > CHANCE_RAIN)
		{
			if (weather_info[i].state > HEAVY_RAIN)
				weather_info[i].state -= 3;
			roll = number (0, 99);
			if (weather_info[i].clouds == LIGHT_CLOUDS)
			{
				if (roll < 40)
					weather_info[i].state = CHANCE_RAIN;
				else if (roll < 85)
					weather_info[i].state = LIGHT_RAIN;
				else
					weather_info[i].state = STEADY_RAIN;
			}
			if (weather_info[i].clouds == HEAVY_CLOUDS)
			{
				if (roll < 30)
					weather_info[i].state = CHANCE_RAIN;
				else if (roll < 60)
					weather_info[i].state = LIGHT_RAIN;
				else if (roll < 90)
					weather_info[i].state = STEADY_RAIN;
				else
					weather_info[i].state = HEAVY_RAIN;
			}
			if (weather_info[i].clouds == OVERCAST)
			{
				if (roll < 20)
					weather_info[i].state = CHANCE_RAIN;
				else if (roll < 50)
					weather_info[i].state = LIGHT_RAIN;
				else if (roll < 80)
					weather_info[i].state = STEADY_RAIN;
				else
					weather_info[i].state = HEAVY_RAIN;
			}

			/*   Is it rain or snow?   */
			if ((weather_info[i].temperature < 32) 
				&& (weather_info[i].state > CHANCE_RAIN))	
			{
				if ((weather_info[i].state += 3) == HEAVY_SNOW
					&& (weather_info[i].temperature < 20))
					weather_info[i].state--;
			}

			/*   Lightning should never allow existing rain to stop   */
			if (weather_info[i].lightning 
				&& (weather_info[i].state == CHANCE_RAIN))
				weather_info[i].state = weather_info[i].clouds + 1;
		}

		/*   If the preception has changed, display a message.   */
		if ((weather_info[i].state != last_state)
			&& (weather_info[i].state != NO_RAIN))
		{
			if ((weather_info[i].state == CHANCE_RAIN)
				&& (last_state > CHANCE_RAIN) && (last_state < LIGHT_SNOW))
				send_outside_zone ("The rain fades and stops.\n", i);
			if ((weather_info[i].state == CHANCE_RAIN)
				&& (last_state > HEAVY_RAIN))
				send_outside_zone ("It stops snowing.\n", i);
			if (weather_info[i].state == LIGHT_RAIN)
				send_outside_zone
				("A light sprinkling of rain falls from the sky.\n", i);
			if (weather_info[i].state == STEADY_RAIN)
				send_outside_zone ("A steady rain falls from the sky.\n", i);
			if (weather_info[i].state == HEAVY_RAIN)
				send_outside_zone ("Pouring rain showers down upon the land.\n",
				i);
			if (weather_info[i].state == LIGHT_SNOW)
				send_outside_zone ("A light snow falls lazily from the sky.\n",
				i);
			if (weather_info[i].state == STEADY_SNOW)
				send_outside_zone ("Snow falls steadily from the sky.\n", i);
			if (weather_info[i].state == HEAVY_SNOW)
				send_outside_zone ("Blinding snows fall from the sky.\n", i);
		}

			//will fog clear based on wind, time of day and cloud cover
		if (weather_info[i].fog)
		{
			if (weather_info[i].wind_speed == WINDY)
				weather_info[i].fog -= 1;
			if (weather_info[i].wind_speed > WINDY)
			{
				weather_info[i].fog = NO_FOG;
				send_outside_zone
					("The fog churns in the wind and is swept away.\n", i);
			}
			if (time_info.hour > sunrise[time_info.month]
			&& weather_info[i].clouds == CLEAR_SKY)
				weather_info[i].fog -= 1;
			if (time_info.hour == 9)
				weather_info[i].fog -= 1;
			if (time_info.hour == 12)
				weather_info[i].fog -= 1;
			if (time_info.hour > sunrise[time_info.month]
			&& weather_info[i].clouds > CLEAR_SKY)
			{
				roll = number (1, 4);
				if (roll > weather_info[i].clouds)
					weather_info[i].fog -= 1;
			}
			if (weather_info[i].fog < NO_FOG)
				weather_info[i].fog = NO_FOG;
			if (weather_info[i].fog == THIN_FOG && last_fog == THICK_FOG)
				send_outside_zone ("The fog thins a little.\n", i);
			if (weather_info[i].fog == NO_FOG)
				send_outside_zone ("The fog lifts.\n", i);
		}

			//will fog build right before dawn?
		/*  If its after midnight, before dawn, within 3 hours of dawn, there is no fog, and there is no artificial sunlight....   */
		if ((sunrise[time_info.month] < (time_info.hour + 4))
			&& (global_sun_light == 0) 
			&& (time_info.hour < sunrise[time_info.month])
			&& (weather_info[i].fog == NO_FOG))
		{
			switch (time_info.season)
			{
			case WINTER:
				chance_of_rain = 25;
				break;		/*   Chance of Fog   */
			case AUTUMN:
				chance_of_rain = 15;
				break;
			case SPRING:
				chance_of_rain = 8;
				break;
			case SUMMER:
				chance_of_rain = 5;
				break;
			}
			roll = number (1, 100);
			if (weather_info[i].wind_speed == BREEZE && roll < chance_of_rain)
			{
				weather_info[i].fog = THIN_FOG;
				send_outside_zone ("A thin fog wafts in on the breeze.\n", i);
			}
			if (weather_info[i].wind_speed == CALM && roll < chance_of_rain)
			{
				weather_info[i].fog = THICK_FOG;
				send_outside_zone
					("A thick fog begins to condense in the still air.\n", i);
			}
		}

		weather_setup(i);
		
		global_sun_light = global_sun_light * (0.25 * weather_info[i].clouds);
		global_sun_light = global_sun_light * (0.33 * weather_info[i].fog);
		
		if (global_sun_light < SUN_DARK)
			global_sun_light = SUN_DARK;
		
		moon_state(i);
		

	}
}

int
is_leap_year (int year)
{
	if (year % 4 == 0)
	{
		if (year % 100 == 0)
		{
			if (year % 400 == 0)
				return 1;
			else
				return 0;
		}
		return 1;
	}

	return 0;
}

	//increments game by 1 hour and then recalulates date and weather

void
weather_and_time (int mode)
{
	bool new_day = false;

	next_hour_update = time(0) + RL_SEC_PER_IG_HOUR;
	time_info.hour++;

	time_info.minute = 0;
	
	if (time_info.hour >= 24)
	{
		time_info.day++;
		time_info.accum_days++;
		time_info.hour = 0;
		new_day = true;
	}

	if (time_info.day >= 30 && new_day)
	{
		if (!time_info.holiday)
			time_info.month++;
		if (time_info.month >= 12)
		{
			time_info.month = 0;
		}
		if (time_info.month == 0 || time_info.month == 1
			|| time_info.month == 11)
			time_info.season = WINTER;
		else if (time_info.month >= 2 && time_info.month <= 4)
			time_info.season = SPRING;
		else if (time_info.month > 4 && time_info.month <= 7)
			time_info.season = SUMMER;
		else if (time_info.month > 7 && time_info.month <= 10)
			time_info.season = AUTUMN;
		
		if (time_info.holiday == HOLIDAY_METTARE)
		{
			time_info.holiday = HOLIDAY_YESTARE;
			time_info.year++;
		}
		else if (time_info.holiday == HOLIDAY_LOENDE
				 && is_leap_year (time_info.year))
			time_info.holiday = HOLIDAY_ENDERI;
		else if (time_info.holiday == HOLIDAY_ENDERI)
		{
			time_info.holiday = 0;
			time_info.day = 0;
		}
		else if (time_info.month == 0)
		{
			if (time_info.holiday != HOLIDAY_YESTARE)
				time_info.holiday = HOLIDAY_METTARE;
			else
			{
				time_info.holiday = 0;
				time_info.day = 0;
			}
		}
		else if (time_info.month == 3)
		{
			if (time_info.holiday != HOLIDAY_TUILERE)
				time_info.holiday = HOLIDAY_TUILERE;
			else
			{
				time_info.holiday = 0;
				time_info.day = 0;
			}
		}
		else if (time_info.month == 6)
		{
			if (time_info.holiday != HOLIDAY_LOENDE)
				time_info.holiday = HOLIDAY_LOENDE;
			else
			{
				time_info.holiday = 0;
				time_info.day = 0;
			}
		}
		else if (time_info.month == 9)
		{
			if (time_info.holiday != HOLIDAY_YAVIERE)
				time_info.holiday = HOLIDAY_YAVIERE;
			else
			{
				time_info.holiday = 0;
				time_info.day = 0;
			}
		}
		else
			time_info.day = 0;
	}

	if (time_info.month >= 12)
	{
		time_info.year++;
		time_info.month = 0;
		time_info.accum_days = 0;
	}

	weather_create();
	sunrise_sunset();
}

int
weather_object_exists(OBJ_DATA * list, int vnum)
{
	for (; list; list = list->next_content)
		if (list->nVirtual == vnum)
			return 1;

	return 0;
}


/*
load_weather_obj()

This function is called every time do_look is called for a room
and checks the weather in room being looked at to see if the appropriate
objects for the current weather are present and if not loads them as required.

The objects

*/
void
load_weather_obj(ROOM_DATA *troom)
{
	OBJ_DATA *obj = NULL;

	/* If it is raining */
	if((weather_info[troom->zone].state > CHANCE_RAIN)
		&&(weather_info[troom->zone].state < LIGHT_SNOW))
	{
		//if there is snow in the room replace it with slush
		if((weather_object_exists(troom->contents, weather_objects[0]))||
			(weather_object_exists(troom->contents, weather_objects[1])))
		{
			//remove snow from room
			if((obj = get_obj_in_list_num (weather_objects[0], troom->contents)))
				extract_obj(obj);
			if((obj = get_obj_in_list_num (weather_objects[1], troom->contents)))
				extract_obj(obj);

			//load slush in room
			obj = load_object(weather_objects[2]);
			if (obj = load_object(weather_objects[2]))
				obj_to_room (obj, troom->nVirtual);
		}
		else
		{
			//if there is ash, replace it with muddy ash
			if((weather_object_exists(troom->contents, weather_objects[5]))||
				(weather_object_exists(troom->contents, weather_objects[6])))
			{
				//remove ash
				if((obj = get_obj_in_list_num (weather_objects[5], troom->contents)))
					extract_obj(obj);
				if((obj = get_obj_in_list_num (weather_objects[6], troom->contents)))
					extract_obj(obj);

				//if not already muddy ash in the room load some
				if(!(obj = get_obj_in_list_num (weather_objects[0], troom->contents)))
				{
					if (obj = load_object(weather_objects[9]))
						obj_to_room (obj, troom->nVirtual);
				}
			}
			else
			{
				//if there are old puddles replace them with new ones
				if(weather_object_exists(troom->contents, weather_objects[4]))
				{
					//remove old puddles from room
					if((obj = get_obj_in_list_num (weather_objects[4], troom->contents)))
						extract_obj(obj);

					//load fresh puddles in room
					if(obj = load_object(weather_objects[3]))
						obj_to_room (obj, troom->nVirtual);
				}
				else
				{
					//if there are no puddles or slush or muddy ash load puddles
					if((!weather_object_exists(troom->contents, weather_objects[3]))&&
						(!weather_object_exists(troom->contents, weather_objects[2]))&&
						(!weather_object_exists(troom->contents, weather_objects[9])))
					{
						//load fresh puddles in room
						if(obj = load_object(weather_objects[3]))
							obj_to_room (obj, troom->nVirtual);
					}
				}
			}
		}
	}

	/* If it is snowing */
	else if(weather_info[troom->zone].state > HEAVY_RAIN)
	{
		//if there is rain in the room replace it with slush
		if((weather_object_exists(troom->contents, weather_objects[3]))||
			(weather_object_exists(troom->contents, weather_objects[4])))
		{
			//remove rain from room
			if((obj = get_obj_in_list_num (weather_objects[3], troom->contents)))
				extract_obj(obj);
			if((obj = get_obj_in_list_num (weather_objects[4], troom->contents)))
				extract_obj(obj);

			//load slush in room
			if(obj = load_object(weather_objects[2]))
				obj_to_room (obj, troom->nVirtual);
		}
		else
		{
			//if there is ash, replace it with muddy ash
			if((weather_object_exists(troom->contents, weather_objects[5]))||
				(weather_object_exists(troom->contents, weather_objects[6])))
			{
				//remove ash
				if((obj = get_obj_in_list_num (weather_objects[5], troom->contents)))
					extract_obj(obj);
				if((obj = get_obj_in_list_num (weather_objects[6], troom->contents)))
					extract_obj(obj);

				//if not already muddy ash in the room load some
				if(!(obj = get_obj_in_list_num (weather_objects[9], troom->contents)))
				{
					if(obj = load_object(weather_objects[9]))
						obj_to_room (obj, troom->nVirtual);
				}
			}
			else
			{
				//if there is old snow replace it with new snow
				if(weather_object_exists(troom->contents, weather_objects[1]))
				{
					//remove old snow from room
					if((obj = get_obj_in_list_num (weather_objects[1], troom->contents)))
						extract_obj(obj);

					//load fresh snow in room
					if(obj = load_object(weather_objects[0]))
						obj_to_room (obj, troom->nVirtual);
				}
				else
				{
					//if there is no snow or slush or muddy ash load fresh snow
					if((!weather_object_exists(troom->contents, weather_objects[2]))&&
						(!weather_object_exists(troom->contents, weather_objects[0]))&&
						(!weather_object_exists(troom->contents, weather_objects[9])))
					{
						//load fresh snow in room
						if(obj = load_object(weather_objects[0]))
							obj_to_room (obj, troom->nVirtual);
					}
				}
			}
		}
	}

	/* If volcanic smoke is set */
	if(weather_info[troom->zone].special_effect == VOLCANIC_SMOKE)
	{
		//if there is snow or rain or slush, replace it with muddy ash
		if((weather_object_exists(troom->contents, weather_objects[0]))||
			(weather_object_exists(troom->contents, weather_objects[1]))||
			(weather_object_exists(troom->contents, weather_objects[2]))||
			(weather_object_exists(troom->contents, weather_objects[3]))||
			(weather_object_exists(troom->contents, weather_objects[4])))
		{
			//remove snow/rain/slush
			if((obj = get_obj_in_list_num (weather_objects[0], troom->contents)))
				extract_obj(obj);
			if((obj = get_obj_in_list_num (weather_objects[1], troom->contents)))
				extract_obj(obj);
			if((obj = get_obj_in_list_num (weather_objects[2], troom->contents)))
				extract_obj(obj);
			if((obj = get_obj_in_list_num (weather_objects[3], troom->contents)))
				extract_obj(obj);
			if((obj = get_obj_in_list_num (weather_objects[4], troom->contents)))
				extract_obj(obj);

			//if not already muddy ash in the room load some
			if(!(obj = get_obj_in_list_num (weather_objects[9], troom->contents)))
			{
				if(obj = load_object(weather_objects[9]))
					obj_to_room (obj, troom->nVirtual);
			}
		}
		else
		{
			//if there is old ash replace it with new ash
			if(weather_object_exists(troom->contents, weather_objects[6]))
			{
				//remove old ash from room
				if((obj = get_obj_in_list_num (weather_objects[6], troom->contents)))
					extract_obj(obj);

				//load fresh ash in room
				if(obj = load_object(weather_objects[5]))
					obj_to_room (obj, troom->nVirtual);
			}
			else
			{
				//if there is no ash or muddy ash load it
				if((!weather_object_exists(troom->contents, weather_objects[5]))&&
					(!weather_object_exists(troom->contents, weather_objects[9])))
				{
					//load fresh ash in room
					if(obj = load_object(weather_objects[5]))
						obj_to_room (obj, troom->nVirtual);
				}
			}
		}
	}

	/* If foul stench is set */
	if(weather_info[troom->zone].special_effect == FOUL_STENCH)
	{
		//if there is old stench replace it with new stench
		if(weather_object_exists(troom->contents, weather_objects[8]))
		{
			//remove old stench from room
			if((obj = get_obj_in_list_num (weather_objects[8], troom->contents)))
				extract_obj(obj);

			//load fresh stench in room
			if(obj = load_object(weather_objects[7]))
				obj_to_room (obj, troom->nVirtual);
		}
		else
		{
			//if there is no stench load it
			if((!weather_object_exists(troom->contents, weather_objects[7])))
			{
				//load fresh stench in room
				if(obj = load_object(weather_objects[7]))
					obj_to_room (obj, troom->nVirtual);
			}
		}
	}
}

	//sets weather by zone
	//zvalue will be WEATHER_TEMPERATE, WEATHER_ARCTIC, or similar
	//we only really need the one for Angrenost 
void
weather_setup(int zvalue)
{
	
		//set up overall weather conditons by the area
	desc_weather[zvalue] = WR_NORMAL;
	
		//light clouds, heavy clouds or overcast
	if (weather_info[zvalue].clouds > CLEAR_SKY)
		desc_weather[zvalue] = WR_CLOUDY;
	
	if (weather_info[zvalue].fog)
		desc_weather[zvalue] = WR_FOGGY;
	
		//light, steady, heavy rain, light, steady, heavy snow
	if (weather_info[zvalue].state > CHANCE_RAIN)
	{
		desc_weather[zvalue] = WR_RAINY;
		if (weather_info[zvalue].wind_speed == STORMY)
			desc_weather[zvalue] = WR_STORMY;
	}
	
		//light, steady, heavy snow
	if (weather_info[zvalue].state > HEAVY_RAIN)
	{
		desc_weather[zvalue] = WR_SNOWY;
		if (weather_info[zvalue].wind_speed == STORMY)
			desc_weather[zvalue] = WR_BLIZARD;
	}
	
	if (global_sun_light < SUN_TWILIGHT)
	{
		switch (desc_weather[zvalue])
		{
			case WR_NORMAL:
				desc_weather[zvalue] = WR_NIGHT;
				break;
			case WR_FOGGY:
				desc_weather[zvalue] = WR_NIGHT_FOGGY;
				break;
			case WR_CLOUDY:
				desc_weather[zvalue] = WR_NIGHT_CLOUDY;
				break;
			case WR_RAINY:
				desc_weather[zvalue] = WR_NIGHT_RAINY;
				break;
			case WR_STORMY:
				desc_weather[zvalue] = WR_NIGHT_STORMY;
				break;	
			case WR_SNOWY:
				desc_weather[zvalue] = WR_NIGHT_SNOWY;
				break;
			case WR_BLIZARD:
				desc_weather[zvalue] = WR_NIGHT_BLIZARD;
				break;
			
		}
	}
	
}

void
sunrise_sunset(void)
{
	int high_sun;
	int hour_spread;
	
	high_sun = ((sunrise[time_info.month] + sunset[time_info.month]) / 2);
	
	if (time_info.hour == sunrise[time_info.month] - 1)
	{
		send_outside ("A glow illuminates the eastern horizon.\n");
		global_sun_light = SUN_TWILIGHT;
		return;
	}
	
	if (time_info.hour == sunrise[time_info.month])
	{
		send_outside
		("Anor's fiery exterior slowly lifts itself up over the eastern horizon beneath Arien's unwavering guidance.\n");
		global_sun_light = SUN_RISE_SET;
		return;
	}
	
	else if ((time_info.hour > (sunrise[time_info.month]))
		&& (time_info.hour < high_sun))
	{
		hour_spread = high_sun - time_info.hour;
		
			//allow sunlight to go from SUN_NOON to SUN_RISE_SET
			//base of 6 hours for the change from 40 to 13000
		global_sun_light = SUN_NOON - (2160 * hour_spread);
		return;
	}
	
	else if ((time_info.hour == high_sun))
	{
		send_outside
		("Anor's brillant blaze stands high above the lands.\n");
				
			//allow sunlight to go from SUN_NOON to SUN_RISE_SET
			//base of 6 hours for the change from 40 to 13000
		global_sun_light = SUN_NOON;
		return;
	}
	
	else if ((time_info.hour > high_sun)
			 && (time_info.hour < (sunset[time_info.month] - 1)))
	{
		hour_spread = time_info.hour - high_sun;
		
			//allow sunlight to go from SUN_NOON to SUN_RISE_SET
			//base of 6 hours for the change from 40 to 13000
		global_sun_light = SUN_NOON - (2160 * hour_spread);
		return;
	}
	
	else if (time_info.hour == sunset[time_info.month] - 1)
	{
		send_outside
		("Anor begins dipping below the western horizon, guided to its respite by Arien.\n");
		global_sun_light = SUN_RISE_SET;
		return;
	}
	
	else if (time_info.hour == sunset[time_info.month])
	{
		send_outside
		("Anor sets in a fiery cascade of brilliant color upon the western horizon.\n");
		global_sun_light = SUN_TWILIGHT;
		return;
	}
	else if (time_info.hour > sunset[time_info.month])
	{
		global_sun_light = SUN_DARK;
		return;
	}
}

	//we only need to calcualte moon and weather effects for WEATHER_ANGRENOST area,
	//but we leave in the zvalue for future expansions. 
	//A simplistic version, but Middle-earth is much closer to perfection than reality.
	//TODO: Calculating the moon's phase happens here every game hour - should move to hour_update call instead
void
moon_state(int zvalue)
{
	int moon_age;
	int moon_phase;
	int moonrise, moonset;
	
		//moon_phase = 0, moon_rise = 6pm - full moon
		//moon_phase = 15, moon_rise = 6 am - new moon
	
	moon_phase = time_info.accum_days % 30;
	moon_age = moon_phase * 24 / 30;
	moonrise = (24 + (moon_age - 6)) % 24;
	moonset = (24 + (moon_age - 17)) % 24;
	
	global_moon_values.phase = moon_phase;
	global_moon_values.age = moon_age;
	global_moon_values.moonrise = moonrise;
	global_moon_values.moonset = moonset;
	
	
	global_moon_values.light = NO_MOON;
	
	if (moonset < moonrise)
	{
		
		if (((time_info.hour > moonrise) && (time_info.hour < 24))
			|| ((time_info.hour < 24) && (time_info.hour < moonrise)) )
		{
			if (moon_phase == 0)
				global_moon_values.light = MOON_FULL;
			else if ((moon_phase >= 0) && (moon_phase < 5))
				global_moon_values.light = MOON_GIBOUS;
			else if ((moon_phase >= 5) && (moon_phase < 10))
				global_moon_values.light = MOON_CRESCENT;
			else if ((moon_phase >= 10) && (moon_phase < 15))
				global_moon_values.light = NO_MOON;	
			else if ((moon_phase >= 15) && (moon_phase < 20))
				global_moon_values.light = MOON_CRESCENT;
			else if ((moon_phase >= 20) && (moon_phase < 25))
				global_moon_values.light = MOON_GIBOUS;
			else if ((moon_phase >= 25) && (moon_phase <= 30))
				global_moon_values.light = MOON_FULL;	
			
		}
		
	}
	
	else 
		{
			if ((time_info.hour > moonrise) && (time_info.hour < moonrise))
			{
				if (moon_phase == 0)
					global_moon_values.light = MOON_FULL;
				else if ((moon_phase >= 0) && (moon_phase < 5))
					global_moon_values.light = MOON_GIBOUS;
				else if ((moon_phase >= 5) && (moon_phase < 10))
					global_moon_values.light = MOON_CRESCENT;
				else if ((moon_phase >= 10) && (moon_phase < 15))
					global_moon_values.light = NO_MOON;	
				else if ((moon_phase >= 15) && (moon_phase < 20))
					global_moon_values.light = MOON_CRESCENT;
				else if ((moon_phase >= 20) && (moon_phase < 25))
					global_moon_values.light = MOON_GIBOUS;
				else if ((moon_phase >= 25) && (moon_phase <= 30))
					global_moon_values.light = MOON_FULL;	
				
			}
			
		}
	

	
		//moon rises and we see it depending on cloud cover
	if (weather_info[zvalue].clouds < HEAVY_CLOUDS
		&& weather_info[zvalue].fog < THICK_FOG
		&& weather_info[zvalue].state < HEAVY_SNOW)
	{
		if ((time_info.hour < (moonset - 1)) && (time_info.hour > (moonrise + 5)))
			send_outside_zone ("Ithil hangs low in the sky.\n", zvalue);
		
		else if ((time_info.hour < (moonset - 5)) && (time_info.hour > (moonrise + 1)))
			send_outside_zone ("Ithil sits high overhead.\n", zvalue);
		
		else if (time_info.hour == moonset)
			send_outside_zone
			("Ithil slowly sinks from the sky, guided by Tilion to its rest.\n",
			 zvalue);
		
		else if (time_info.hour == moonrise)
			send_outside_zone
			("Ithil rises with stately grace into the sky.\n", zvalue);
		
		moon_light[zvalue] = global_moon_values.light - weather_info[zvalue].clouds;
		moon_light[zvalue] = global_moon_values.light - weather_info[zvalue].fog;
	}
	else
	{
		moon_light[zvalue] = global_moon_values.light;
	}
	
	
}
