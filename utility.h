//////////////////////////////////////////////////////////////////////////////
//
/// utility.h - Inline Utility Functions
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


#ifndef _rpie_utility_h
#define _rpie_utility_h

#include <string>
#include <strings.h>
#include <sstream>
#include <vector>
#include <stdexcept>


//////////////////////////////////////////////////////////////////////////////
// set_flags ()
//////////////////////////////////////////////////////////////////////////////
//
/// \brief  Mnumonic function for setting bits to 1
/// 
/// \param[in]  __stored_flags  Existing flags to receive new flags
/// \param[in]  __new_flags     New flags to assign
/// \param[out]                 Value of updated flags 
//
/// \par Note on Usage:
//
/// Use of the bitwise OR assignment operator "|=" is preferred. This 
/// function template therefore is vestigial, acting for the most part
/// as a reminder to use the builtin C/C++ operator. This function 
/// replaces the SET_BIT macro under old implementations.
//
/// \par Examples:
//
/// stored_flags |= new_flag
/// stored_flags |= (new_flag | other_new_flag)
//
//////////////////////////////////////////////////////////////////////////////
template <typename _bit_flags>
inline _bit_flags
set_flags (_bit_flags& __stored_flags, _bit_flags __new_flags)
{
	return (__stored_flags |= __new_flags);
}

//////////////////////////////////////////////////////////////////////////////
// clear_flags ()
//////////////////////////////////////////////////////////////////////////////
//
/// \brief  Mnumonic function for clearing bits to 0.
/// 
/// \param[in]  __stored_flags  Existing flags to receive new flags
/// \param[in]  __old_flags     Old flags to clear
/// \param[out]                 Value of updated flags 
//
/// \par Note on Usage:
//
/// Use of the bitwise AND assignment operator "&=" of the old_flags
/// complement "~" is preferred. This function template therefore is 
/// vestigial, acting for the most part as a reminder to use the builtin 
/// C/C++ operations. This function replaces the REMOVE_BIT macro under the 
/// old implementation.
//
/// \par Example:
//
/// stored_flags &= ~old_flags
/// stored_flags &= ~(old_flags | other_flag_to_clear)
//
//////////////////////////////////////////////////////////////////////////////
template <typename _bit_flags>
inline _bit_flags
clear_flags (_bit_flags& __stored_flags, _bit_flags __old_flags)
{
	return (__stored_flags &= ~__old_flags);
}

//////////////////////////////////////////////////////////////////////////////
// toggle_flags ()
//////////////////////////////////////////////////////////////////////////////
//
/// \brief  Mnumonic function for toggling bits.
/// 
/// \param[in]  __stored_flags  Existing flags to receive new flags
/// \param[in]  __toggled_flags Flags to toggle
/// \param[out]                 Value of updated flags 
//
/// \par Note on Usage:
//
/// Use of the bitwise XOR assignment operator "^=" of the toggled_flags
/// is preferred. This function template therefore is vestigial, acting for 
/// the most part as a reminder to use the builtin C/C++ operation. This 
/// function replaces the TOGGLE_BIT and TOGGLE macros under the old 
/// implementation.
//
/// \par Example:
//
/// stored_flags ^= toggled_flags
//
//////////////////////////////////////////////////////////////////////////////
template <typename _bit_flags>
inline _bit_flags
toggle_flags (_bit_flags& __stored_flags, _bit_flags __toggled_flags)
{
	return (__stored_flags ^= __toggled_flags);
}

#ifdef __APPLE__
#undef MAX
#undef MIN
#endif

template <typename _value>
inline _value
MIN (const _value __x, const _value __y)
{
	return (__x < __y ? __x : __y);
}

template <typename _value>
inline _value
MAX (const _value __x, const _value __y)
{
	return (__x > __y ? __x : __y);
}


// The following are defined in utility.cpp
char* one_argument (char* argument, char* arg_first);
std::string one_argument (std::string& argument, std::string& arg_first);

struct ci_equal_to : std::binary_function <std::string, std::string, bool>
{
	struct compare_equal 
		: public std::binary_function <unsigned char, unsigned char,bool>
	{
		bool operator() (const unsigned char& c1, const unsigned char& c2) const;
	};  // end of compare_equal

	bool operator() (const std::string & s1, const std::string & s2) const;
}; // end of ci_equal_to
bool ciStringEqual (const std::string & s1, const std::string & s2);


#endif // _rpie_utility_h
