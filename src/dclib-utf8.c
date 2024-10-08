
/***************************************************************************
 *                                                                         *
 *                     _____     ____                                      *
 *                    |  __ \   / __ \   _     _ _____                     *
 *                    | |  \ \ / /  \_\ | |   | |  _  \                    *
 *                    | |   \ \| |      | |   | | |_| |                    *
 *                    | |   | || |      | |   | |  ___/                    *
 *                    | |   / /| |   __ | |   | |  _  \                    *
 *                    | |__/ / \ \__/ / | |___| | |_| |                    *
 *                    |_____/   \____/  |_____|_|_____/                    *
 *                                                                         *
 *                       Wiimms source code library                        *
 *                                                                         *
 ***************************************************************************
 *                                                                         *
 *        Copyright (c) 2012-2022 by Dirk Clemens <wiimm@wiimm.de>         *
 *                                                                         *
 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   See file gpl-2.0.txt or http://www.gnu.org/licenses/gpl-2.0.txt       *
 *                                                                         *
 ***************************************************************************/

#define _GNU_SOURCE 1

#include <string.h>
#include <limits.h>
#include <arpa/inet.h>

#include "dclib/dclib-basics.h"
#include "dclib/dclib-debug.h"
#include "dclib/dclib-utf8.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define u0 DC_UTF8_ILLEGAL
#define u1 DC_UTF8_1CHAR
#define u2 DC_UTF8_2CHAR
#define u3 DC_UTF8_3CHAR
#define u4 DC_UTF8_4CHAR
#define uc DC_UTF8_CONT_ANY

const unsigned short TableUTF8Mode[256] =
{
	u1,u1,u1,u1, u1,u1,u1,u1, u1,u1,u1,u1, u1,u1,u1,u1, // 0000xxxx
	u1,u1,u1,u1, u1,u1,u1,u1, u1,u1,u1,u1, u1,u1,u1,u1, // 0001xxxx
	u1,u1,u1,u1, u1,u1,u1,u1, u1,u1,u1,u1, u1,u1,u1,u1, // 0010xxxx
	u1,u1,u1,u1, u1,u1,u1,u1, u1,u1,u1,u1, u1,u1,u1,u1, // 0011xxxx

	u1,u1,u1,u1, u1,u1,u1,u1, u1,u1,u1,u1, u1,u1,u1,u1, // 0100xxxx
	u1,u1,u1,u1, u1,u1,u1,u1, u1,u1,u1,u1, u1,u1,u1,u1, // 0101xxxx
	u1,u1,u1,u1, u1,u1,u1,u1, u1,u1,u1,u1, u1,u1,u1,u1, // 0110xxxx
	u1,u1,u1,u1, u1,u1,u1,u1, u1,u1,u1,u1, u1,u1,u1,u1, // 0111xxxx

	uc,uc,uc,uc, uc,uc,uc,uc, uc,uc,uc,uc, uc,uc,uc,uc, // 1000xxxx
	uc,uc,uc,uc, uc,uc,uc,uc, uc,uc,uc,uc, uc,uc,uc,uc, // 1001xxxx
	uc,uc,uc,uc, uc,uc,uc,uc, uc,uc,uc,uc, uc,uc,uc,uc, // 1010xxxx
	uc,uc,uc,uc, uc,uc,uc,uc, uc,uc,uc,uc, uc,uc,uc,uc, // 1011xxxx

	u2,u2,u2,u2, u2,u2,u2,u2, u2,u2,u2,u2, u2,u2,u2,u2, // 1100xxxx
	u2,u2,u2,u2, u2,u2,u2,u2, u2,u2,u2,u2, u2,u2,u2,u2, // 1101xxxx
	u3,u3,u3,u3, u3,u3,u3,u3, u3,u3,u3,u3, u3,u3,u3,u3, // 1110xxxx
	u4,u4,u4,u4, u4,u4,u4,u4, u0,u0,u0,u0, u0,u0,u0,u0  // 1111xxxx
};

#undef u0
#undef u1
#undef u2
#undef u3
#undef u4
#undef uc

///////////////////////////////////////////////////////////////////////////////

#define CheckUTF8Mode(ch) ((dcUTF8Mode)TableUTF8Mode[(unsigned char)(ch)])

///////////////////////////////////////////////////////////////////////////////

int GetUTF8CharLength ( u32 code )
{
    // returns the length of the char 'code'
    //  0 : illegal code

    return code <= DCLIB_UNICODE_MAX_UTF8_1
		? 1
		: code <= DCLIB_UNICODE_MAX_UTF8_2
			? 2
			: code <= DCLIB_UNICODE_MAX_UTF8_3
				? 3
				: code <= DCLIB_UNICODE_MAX_UTF8_4
					? 4
					: 0;
}

///////////////////////////////////////////////////////////////////////////////

char * NextUTF8Char ( ccp ptr )
{
    // goto next UTF8 character

    switch (CheckUTF8Mode(*ptr))
    {
	case DC_UTF8_1CHAR:
	    return *ptr ? (char*)ptr+1 : (char*)ptr;

	case DC_UTF8_4CHAR:
	    if ( CheckUTF8Mode(*++ptr) != DC_UTF8_CONT_ANY )
	    break;

	// fall through

	case DC_UTF8_3CHAR:
	case DC_UTF8_CONT_ANY:
	    if ( CheckUTF8Mode(*++ptr) != DC_UTF8_CONT_ANY )
	    break;

	// fall through

	case DC_UTF8_2CHAR:
	    if ( CheckUTF8Mode(*++ptr) != DC_UTF8_CONT_ANY )
	    break;

	// fall through

	default: // DC_UTF8_ILLEGAL
	    return (char*)ptr+1;
    }
    return (char*)ptr;
}

//-----------------------------------------------------------------------------

char * NextUTF8CharE( ccp ptr, ccp end )
{
    // goto next UTF8 character

    if ( ptr >= end )
	return (char*)end;

    switch (CheckUTF8Mode(*ptr))
    {
	case DC_UTF8_1CHAR:
	    return (char*)ptr+1;

	case DC_UTF8_4CHAR:
	    if ( ++ptr >= end || CheckUTF8Mode(*ptr) != DC_UTF8_CONT_ANY )
	    break;

	// fall through

	case DC_UTF8_3CHAR:
	case DC_UTF8_CONT_ANY:
	    if ( ++ptr >= end || CheckUTF8Mode(*ptr) != DC_UTF8_CONT_ANY )
	    break;

	// fall through

	case DC_UTF8_2CHAR:
	    if ( ++ptr >= end || CheckUTF8Mode(*ptr) != DC_UTF8_CONT_ANY )
	    break;

	// fall through

	default: // DC_UTF8_ILLEGAL
	    return (char*)ptr+1;
    }
    return (char*)ptr;
}

///////////////////////////////////////////////////////////////////////////////

char * NextEUTF8Char ( ccp ptr )
{
    return NextUTF8Char(SkipEscapes(ptr));
}

//-----------------------------------------------------------------------------

char * NextEUTF8CharE ( ccp ptr, ccp end )
{
    return NextUTF8CharE(SkipEscapesE(ptr,end),end);
}

///////////////////////////////////////////////////////////////////////////////

char * PrevUTF8Char ( ccp str )
{
    // go to the previous UTC character

    // kleine Optimierung für den Standardfall
    if ( (uchar)str[-1] < (uint)DCLIB_UNICODE_MAX_UTF8_1 )
	return (char*)str-1;

    ccp ptr = str;
    int n = 0;
    dcUTF8Mode mode = CheckUTF8Mode(*--ptr);
    while ( n < 3 && mode == DC_UTF8_CONT_ANY )
    {
	n++;
	mode = CheckUTF8Mode(*--ptr);
    }

    switch (mode)
    {
	case DC_UTF8_1CHAR:
	    return (char*)( n<1 ? ptr : ptr+1 );

	case DC_UTF8_2CHAR:
	    return (char*)( n<2 ? ptr : ptr+2 );

	case DC_UTF8_3CHAR:
	    return (char*)( n<3 ? ptr : ptr+3 );

	case DC_UTF8_4CHAR:
	    return (char*)( n<4 ? ptr : ptr+4 );

	case DC_UTF8_CONT_ANY:
	    return (char*)str-3;

	default:
	    return (char*)str-1;
    }
}

//-----------------------------------------------------------------------------

char * PrevUTF8CharB ( ccp str, ccp begin )
{
    // go to the previous UTC character

    ccp ptr = str;
    if ( ptr <= begin )
	return (char*)begin;

    // kleine Optimierung für den Standardfall
    if ( (uchar)ptr[-1] < (uint)DCLIB_UNICODE_MAX_UTF8_1 )
	return (char*)ptr-1;

    int n = 0;
    dcUTF8Mode mode = CheckUTF8Mode(*--ptr);
    while ( ptr > begin && n < 3 && mode == DC_UTF8_CONT_ANY )
    {
	n++;
	mode = CheckUTF8Mode(*--ptr);
    }
    switch (mode)
    {
	case DC_UTF8_1CHAR:
	    return (char*)( n<1 ? ptr : ptr+1 );

	case DC_UTF8_2CHAR:
	    return (char*)( n<2 ? ptr : ptr+2 );

	case DC_UTF8_3CHAR:
	    return (char*)( n<3 ? ptr : ptr+3 );

	case DC_UTF8_4CHAR:
	    return (char*)( n<4 ? ptr : ptr+4 );

	case DC_UTF8_CONT_ANY:
	    return (char*)str-3;

	default:
	    return (char*)str-1;
    }
}

///////////////////////////////////////////////////////////////////////////////

char * SkipUTF8Char ( ccp str, int skip )
{
    if (str)
    {
	if ( skip < 0 )
	    skip += ScanUTF8Length(str);
	while ( skip-- > 0 )
	{
	    ccp next = NextUTF8Char(str);
	    if ( str == next )
		break;
	    str = next;
	}
    }
    return (char*)str;
}

//-----------------------------------------------------------------------------

char * SkipUTF8CharE ( ccp str, ccp end, int skip )
{
    if (str)
    {
	if ( skip < 0 )
	    skip += ScanUTF8LengthE(str,end);
	while ( skip-- > 0 )
	{
	    ccp next = NextUTF8CharE(str,end);
	    if ( str == next )
		break;
	    str = next;
	}
    }
    return (char*)str;
}

//-----------------------------------------------------------------------------

char * SkipEUTF8Char ( ccp str, int skip )
{
    if (str)
    {
	if ( skip < 0 )
	    skip += ScanEUTF8Length(str);
	while ( skip-- > 0 )
	{
	    ccp next = NextEUTF8Char(str);
	    if ( str == next )
		break;
	    str = next;
	}
    }
    return (char*)str;
}

//-----------------------------------------------------------------------------

char * SkipEUTF8CharE ( ccp str, ccp end, int skip )
{
    if (str)
    {
	if ( skip < 0 )
	    skip += ScanEUTF8LengthE(str,end);
	while ( skip-- > 0 )
	{
	    ccp next = NextEUTF8CharE(str,end);
	    if ( str == next )
		break;
	    str = next;
	}
    }
    return (char*)str;
}

///////////////////////////////////////////////////////////////////////////////

u32 GetUTF8Char ( ccp str )
{
    // fast scan of a UTF8 char -> ignore errors in continuation bytes

    const u32 result = (uchar)*str;
    switch (CheckUTF8Mode(result))
    {
	case DC_UTF8_1CHAR:
	    return result;

	case DC_UTF8_2CHAR:
	    return ( result & 0x1f ) <<  6
	     | ( str[1] & 0x3f );

	case DC_UTF8_3CHAR:
	    return ( result & 0x0f ) << 12
	     | ( str[1] & 0x3f ) <<  6
	     | ( str[2] & 0x3f );

	case DC_UTF8_4CHAR:
	    return ( result & 0x07 ) << 18
	     | ( str[1] & 0x3f ) << 12
	     | ( str[2] & 0x3f ) <<  6
	     | ( str[3] & 0x3f );

	case DC_UTF8_CONT_ANY:
	    return ( result & 0x3f ) << 12
	     | ( str[1] & 0x3f ) <<  6
	     | ( str[2] & 0x3f );

	default: // DC_UTF8_ILLEGAL
	    return result & 0x7f | S32_MIN;
    }
}

///////////////////////////////////////////////////////////////////////////////

u32 ScanUTF8Char ( ccp * p_str )
{
    // scan a UTF8 char. Set bit LONG_MAX if an error seen.
    // *str is set to next next char if on no error.

    int n_cont;
    ccp ptr = *p_str;
    u32 result = (uchar)*ptr++;
    switch (CheckUTF8Mode(result))
    {
	case DC_UTF8_1CHAR:
	    if (result)
		(*p_str)++;
	    return result;

	case DC_UTF8_2CHAR:
	    result &= 0x1f;
	    n_cont = 1;
	    break;

	case DC_UTF8_3CHAR:
	    result &= 0x0f;
	    n_cont = 2;
	    break;

	case DC_UTF8_4CHAR:
	    result &= 0x07;
	    n_cont = 3;
	    break;

	case DC_UTF8_CONT_ANY:
	    if ( CheckUTF8Mode((uchar)*ptr) && CheckUTF8Mode((uchar)*++ptr) )
		ptr++;
	    *p_str = ptr;
	    return S32_MIN;

	default: // DC_UTF8_ILLEGAL
	    (*p_str)++;
	    return result & 0x7f | S32_MIN;
    }

    while ( n_cont-- > 0 )
    {
	if ( CheckUTF8Mode(*ptr) == DC_UTF8_CONT_ANY )
	    result = result << 6 | *ptr++ & 0x3f;
	else
	{
	    result |= S32_MIN;
	    break;
	}
    }

    *p_str = ptr;
    return result;
}

//-----------------------------------------------------------------------------

u32 ScanUTF8CharE ( ccp * p_str, ccp end )
{
    // scan a UTF8 char. Set bit S32_MIN if an error seen
    // *str is set to next next char if on no error.

    ccp ptr = *p_str;
    if ( ptr >= end )
	return S32_MIN;

    int n_cont;
    u32 result = (uchar)*ptr++;
    switch (CheckUTF8Mode(result))
    {
	case DC_UTF8_1CHAR:
	    (*p_str)++;
	    return result;

	case DC_UTF8_2CHAR:
	    if ( ptr >= end )
		return S32_MIN;
	    result &= 0x1f;
	    n_cont = 1;
	    break;

	case DC_UTF8_3CHAR:
	    if ( ptr+1 >= end )
		return S32_MIN;
	    result &= 0x0f;
	    n_cont = 2;
	    break;

	case DC_UTF8_4CHAR:
	    if ( ptr+2 >= end )
		return S32_MIN;
	    result &= 0x07;
	    n_cont = 3;
	    break;

	case DC_UTF8_CONT_ANY:
	    if (     ptr < end && CheckUTF8Mode((uchar)*ptr)
	    && ++ptr < end && CheckUTF8Mode((uchar)*ptr) )
		ptr++;
	    *p_str = ptr;
	    return S32_MIN;

	default: // DC_UTF8_ILLEGAL
	    (*p_str)++;
	    return result & 0x7f | S32_MIN;
    }

    while ( n_cont-- > 0 )
    {
	if ( CheckUTF8Mode(*ptr) == DC_UTF8_CONT_ANY )
	    result = result << 6 | *ptr++ & 0x3f;
	else
	{
	    result |= S32_MIN;
	    break;
	}
    }

    *p_str = ptr;
    return result;
}

///////////////////////////////////////////////////////////////////////////////

u32 ScanUTF8CharInc ( ccp * p_str )
{
    // scan a UTF8 char. Set bit LONG_MAX if an error seen
    // *str is set to next next char => it is incremetned always!

    int n_cont;
    ccp ptr = *p_str;
    u32 result = (uchar)*ptr++;
    switch (CheckUTF8Mode(result))
    {
	case DC_UTF8_1CHAR:
	    (*p_str)++;
	    return result;

	case DC_UTF8_2CHAR:
	    result &= 0x1f;
	    n_cont = 1;
	    break;

	case DC_UTF8_3CHAR:
	    result &= 0x0f;
	    n_cont = 2;
	    break;

	case DC_UTF8_4CHAR:
	    result &= 0x07;
	    n_cont = 3;
	    break;

	case DC_UTF8_CONT_ANY:
	    if ( CheckUTF8Mode((uchar)*ptr) && CheckUTF8Mode((uchar)*++ptr) )
		ptr++;
	    *p_str = ptr;
	    return S32_MIN;

	default: // DC_UTF8_ILLEGAL
	    (*p_str)++;
	    return result & 0x7f | S32_MIN;
    }

    while ( n_cont-- > 0 )
    {
	if ( CheckUTF8Mode(*ptr) == DC_UTF8_CONT_ANY )
	    result = result << 6 | *ptr++ & 0x3f;
	else
	{
	    result |= S32_MIN;
	    break;
	}
    }

    *p_str = ptr;
    return result;
}

//-----------------------------------------------------------------------------

u32 ScanUTF8CharIncE ( ccp * p_str, ccp end )
{
    // scan a UTF8 char. Set bit LONG_MAX if an error seen
    // *str is set to next next char => it is incremetned always!

    ccp ptr = *p_str;
    if ( ptr >= end )
    {
	(*p_str)++;
	return S32_MIN;
    }

    int n_cont = 0;
    u32 result = (uchar)*ptr++;
    switch (CheckUTF8Mode(result))
    {
	case DC_UTF8_1CHAR:
	    (*p_str)++;
	    return result;

	case DC_UTF8_2CHAR:
	    result &= 0x1f;
	    n_cont = 1;
	    break;

	case DC_UTF8_3CHAR:
	    result &= 0x0f;
	    n_cont = 2;
	    break;

	case DC_UTF8_4CHAR:
	    result &= 0x07;
	    n_cont = 3;
	    break;

	case DC_UTF8_CONT_ANY:
	    if (     ptr < end && CheckUTF8Mode((uchar)*ptr)
		&& ++ptr < end && CheckUTF8Mode((uchar)*ptr) )
		    ptr++;
	    *p_str = ptr;
	    return S32_MIN;

	default: // DC_UTF8_ILLEGAL
	    (*p_str)++;
	    return result & 0x7f | S32_MIN;
    }

    while ( n_cont-- > 0 )
    {
	if ( ptr < end && CheckUTF8Mode(*ptr) == DC_UTF8_CONT_ANY )
	    result = result << 6 | *ptr++ & 0x3f;
	else
	{
	    result |= S32_MIN;
	    break;
	}
    }

    *p_str = ptr;
    return result;
}

//////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

u32 GetUTF8AnsiChar ( ccp str )
{
    // scan an UTF8 char. On error scan an ANSI char!

    const uchar * ptr = (const uchar *)str;
    u32 result = *ptr++;
    switch (CheckUTF8Mode(result))
    {
	case DC_UTF8_2CHAR:
	    if ( CheckUTF8Mode(*ptr) == DC_UTF8_CONT_ANY )
	    {
		return ( result & 0x1f ) << 6 | *ptr & 0x3f;
	    }
	    return result;

	case DC_UTF8_3CHAR:
	    if (   CheckUTF8Mode(ptr[0]) == DC_UTF8_CONT_ANY
		&& CheckUTF8Mode(ptr[1]) == DC_UTF8_CONT_ANY )
	    {
		return (( result & 0x0f ) << 6
		    | ptr[0] & 0x3f ) << 6
		    | ptr[1] & 0x3f;
	    }
	    return result;

	case DC_UTF8_4CHAR:
	    if (   CheckUTF8Mode(ptr[0]) == DC_UTF8_CONT_ANY
		&& CheckUTF8Mode(ptr[1]) == DC_UTF8_CONT_ANY
		&& CheckUTF8Mode(ptr[2]) == DC_UTF8_CONT_ANY )
	    {
		return ((( result & 0x07 ) << 6
		     | ptr[0] & 0x3f ) << 6
		     | ptr[1] & 0x3f ) << 6
		     | ptr[2] & 0x3f;
	    }
	    return result;

	default:
	    return result;
    }
}

///////////////////////////////////////////////////////////////////////////////

u32 ScanUTF8AnsiChar ( ccp * p_str )
{
    // scan an UTF8 char. On error scan an ANSI char!

    const uchar * ptr = (const uchar *)*p_str;
    u32 result = *ptr++;
    switch (CheckUTF8Mode(result))
    {
	case DC_UTF8_2CHAR:
	    if ( CheckUTF8Mode(*ptr) == DC_UTF8_CONT_ANY )
	    {
		result = ( result & 0x1f ) << 6 | *ptr++ & 0x3f;
	    }
	    break;

	case DC_UTF8_3CHAR:
	    if (   CheckUTF8Mode(ptr[0]) == DC_UTF8_CONT_ANY
		&& CheckUTF8Mode(ptr[1]) == DC_UTF8_CONT_ANY )
	    {
		result = (( result & 0x0f ) << 6
		    | ptr[0] & 0x3f ) << 6
		    | ptr[1] & 0x3f;
		ptr += 2;
	    }
	    break;

	case DC_UTF8_4CHAR:
	    if (   CheckUTF8Mode(ptr[0]) == DC_UTF8_CONT_ANY
		&& CheckUTF8Mode(ptr[1]) == DC_UTF8_CONT_ANY
		&& CheckUTF8Mode(ptr[2]) == DC_UTF8_CONT_ANY )
	    {
		result = ((( result & 0x07 ) << 6
		    | ptr[0] & 0x3f ) << 6
		    | ptr[1] & 0x3f ) << 6
		    | ptr[2] & 0x3f;
		ptr += 3;
	    }
	    break;

	default:
	    break;
    }
    *p_str = (ccp)ptr;
    return result;
}

//-----------------------------------------------------------------------------

u32 ScanUTF8AnsiCharE ( ccp * p_str, ccp end )
{
    // scan an UTF8 char. On error scan an ANSI char!

    const uchar * ptr = (const uchar *)*p_str;
    if ( (ccp)ptr >= end )
    {
	(*p_str)++;
	return S32_MIN;
    }
    u32 result = *ptr++;
    switch (CheckUTF8Mode(result))
    {
	case DC_UTF8_2CHAR:
	    if (   (ccp)ptr < end
		&& CheckUTF8Mode(*ptr) == DC_UTF8_CONT_ANY )
	    {
		result = ( result & 0x1f ) << 6 | *ptr++ & 0x3f;
	    }
	    break;

	case DC_UTF8_3CHAR:
	    if (   (ccp)ptr+1 < end
		&& CheckUTF8Mode(ptr[0]) == DC_UTF8_CONT_ANY
		&& CheckUTF8Mode(ptr[1]) == DC_UTF8_CONT_ANY )
	    {
		result = (( result & 0x0f ) << 6
		    | ptr[0] & 0x3f ) << 6
		    | ptr[1] & 0x3f;
		ptr += 2;
	    }
	    break;

	case DC_UTF8_4CHAR:
	    if (   (ccp)ptr+2 < end
		&& CheckUTF8Mode(ptr[0]) == DC_UTF8_CONT_ANY
		&& CheckUTF8Mode(ptr[1]) == DC_UTF8_CONT_ANY
		&& CheckUTF8Mode(ptr[2]) == DC_UTF8_CONT_ANY )
	    {
		result = ((( result & 0x07 ) << 6
		    | ptr[0] & 0x3f ) << 6
		    | ptr[1] & 0x3f ) << 6
		    | ptr[2] & 0x3f;
		ptr += 3;
	    }
	    break;

	default:
	    break;
    }
    *p_str = (ccp)ptr;
    return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int ScanUTF8Length ( ccp str )
{
    int count = 0;
    if (str)
    {
	ccp ptr = str;
	for(;;)
	{
	    const char ch = *ptr++;
	    if (!ch)
		break;
	    switch (CheckUTF8Mode(ch))
	    {
		case DC_UTF8_2CHAR:
		    if ( CheckUTF8Mode(*ptr) == DC_UTF8_CONT_ANY )
			ptr++;
		    break;

		case DC_UTF8_3CHAR:
		    if (   CheckUTF8Mode(ptr[0]) == DC_UTF8_CONT_ANY
			&& CheckUTF8Mode(ptr[1]) == DC_UTF8_CONT_ANY )
			    ptr += 2;
		    break;

		case DC_UTF8_4CHAR:
		    if (   CheckUTF8Mode(ptr[0]) == DC_UTF8_CONT_ANY
			&& CheckUTF8Mode(ptr[1]) == DC_UTF8_CONT_ANY
			&& CheckUTF8Mode(ptr[2]) == DC_UTF8_CONT_ANY )
			    ptr += 3;
		    break;

		default:
		    break;
	    }
	    count++;
	}
    }
    return count;
}

//-----------------------------------------------------------------------------

int ScanUTF8LengthE ( ccp str, ccp end )
{
    if (!end)
	return ScanUTF8Length(str);

    int count = 0;
    if (str)
    {
	ccp ptr = str;
	while ( ptr < end )
	{
	    const char ch = *ptr++;
	    switch (CheckUTF8Mode(ch))
	    {
		case DC_UTF8_2CHAR:
		    if ( CheckUTF8Mode(*ptr) == DC_UTF8_CONT_ANY )
			ptr++;
		    break;

		case DC_UTF8_3CHAR:
		    if (   CheckUTF8Mode(ptr[0]) == DC_UTF8_CONT_ANY
			&& CheckUTF8Mode(ptr[1]) == DC_UTF8_CONT_ANY )
			    ptr += 2;
		    break;

		case DC_UTF8_4CHAR:
		    if (   CheckUTF8Mode(ptr[0]) == DC_UTF8_CONT_ANY
			&& CheckUTF8Mode(ptr[1]) == DC_UTF8_CONT_ANY
			&& CheckUTF8Mode(ptr[2]) == DC_UTF8_CONT_ANY )
			    ptr += 3;
		    break;

		default:
		    break;
	    }
	    count++;
	}
    }
    return count;
}

//-----------------------------------------------------------------------------

int ScanEUTF8Length ( ccp str )
{
    int count = 0;
    if (str)
    {
	ccp ptr = str;
	for(;;)
	{
	    const char ch = *ptr++;
	    if (!ch)
		break;

	    if ( ch == '\e' )
	    {
		ptr = SkipEscapes(ptr-1);
		continue;
	    }

	    switch (CheckUTF8Mode(ch))
	    {
		case DC_UTF8_2CHAR:
		    if ( CheckUTF8Mode(*ptr) == DC_UTF8_CONT_ANY )
			ptr++;
		    break;

		case DC_UTF8_3CHAR:
		    if (   CheckUTF8Mode(ptr[0]) == DC_UTF8_CONT_ANY
			&& CheckUTF8Mode(ptr[1]) == DC_UTF8_CONT_ANY )
			    ptr += 2;
		    break;

		case DC_UTF8_4CHAR:
		    if (   CheckUTF8Mode(ptr[0]) == DC_UTF8_CONT_ANY
			&& CheckUTF8Mode(ptr[1]) == DC_UTF8_CONT_ANY
			&& CheckUTF8Mode(ptr[2]) == DC_UTF8_CONT_ANY )
			    ptr += 3;
		    break;

		default:
		    break;
	    }
	    count++;
	}
    }
    return count;
}

//-----------------------------------------------------------------------------

int ScanEUTF8LengthE ( ccp str, ccp end )
{
    if (!end)
	return ScanEUTF8Length(str);

    int count = 0;
    if (str)
    {
	ccp ptr = str;
	while ( ptr < end )
	{
	    const char ch = *ptr++;
	    if ( ch == '\e' )
	    {
		ptr = SkipEscapesE(ptr-1,end);
		continue;
	    }

	    switch (CheckUTF8Mode(ch))
	    {
		case DC_UTF8_2CHAR:
		    if ( CheckUTF8Mode(*ptr) == DC_UTF8_CONT_ANY )
			ptr++;
		    break;

		case DC_UTF8_3CHAR:
		    if (   CheckUTF8Mode(ptr[0]) == DC_UTF8_CONT_ANY
			&& CheckUTF8Mode(ptr[1]) == DC_UTF8_CONT_ANY )
			    ptr += 2;
		    break;

		case DC_UTF8_4CHAR:
		    if (   CheckUTF8Mode(ptr[0]) == DC_UTF8_CONT_ANY
			&& CheckUTF8Mode(ptr[1]) == DC_UTF8_CONT_ANY
			&& CheckUTF8Mode(ptr[2]) == DC_UTF8_CONT_ANY )
			    ptr += 3;
		    break;

		default:
		    break;
	    }
	    count++;
	}
    }
    return count;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int CalcUTF8PrintFW ( ccp str, ccp end, uint wanted_fw )
{
    const uint bytecount = end ? end - str : strlen(str);
    const uint utf8len = ScanUTF8LengthE(str,end);
    return utf8len < wanted_fw
		? wanted_fw - utf8len + bytecount
		: bytecount;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

char * PrintUTF8Char ( char * buf, u32 code )
{
    code &= DCLIB_UNICODE_CODE_MASK;

    if ( code <= DCLIB_UNICODE_MAX_UTF8_1 )
    {
	*buf++ = code;
    }
    else if ( code <= DCLIB_UNICODE_MAX_UTF8_2 )
    {
	*buf++ = code >> 6        | 0xc0;
	*buf++ = code      & 0x3f | 0x80;
    }
    else if ( code <= DCLIB_UNICODE_MAX_UTF8_3 )
    {
	*buf++ = code >> 12        | 0xe0;
	*buf++ = code >>  6 & 0x3f | 0x80;
	*buf++ = code       & 0x3f | 0x80;
    }
    else
    {
	*buf++ = code >> 18 & 0x07 | 0xf0;
	*buf++ = code >> 12 & 0x3f | 0x80;
	*buf++ = code >>  6 & 0x3f | 0x80;
	*buf++ = code       & 0x3f | 0x80;
    }
    return buf;
}

///////////////////////////////////////////////////////////////////////////////

char * PrintUTF8CharToCircBuf ( u32 code )
{
    char *buf;
    code &= DCLIB_UNICODE_CODE_MASK;

    if ( code <= DCLIB_UNICODE_MAX_UTF8_1 )
    {
	buf = GetCircBuf(2);
	buf[0] = code;
	buf[1] = 0;
    }
    else if ( code <= DCLIB_UNICODE_MAX_UTF8_2 )
    {
	buf = GetCircBuf(3);
	buf[0] = code >> 6        | 0xc0;
	buf[1] = code      & 0x3f | 0x80;
	buf[2] = 0;
    }
    else if ( code <= DCLIB_UNICODE_MAX_UTF8_3 )
    {
	buf = GetCircBuf(4);
	buf[0] = code >> 12        | 0xe0;
	buf[1] = code >>  6 & 0x3f | 0x80;
	buf[2] = code       & 0x3f | 0x80;
	buf[3] = 0;
    }
    else
    {
	buf = GetCircBuf(5);
	buf[0] = code >> 18 & 0x07 | 0xf0;
	buf[1] = code >> 12 & 0x3f | 0x80;
	buf[2] = code >>  6 & 0x3f | 0x80;
	buf[3] = code       & 0x3f | 0x80;
	buf[4] = 0;
    }
    return buf;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

exmem_t AlignUTF8 ( exmem_dest_t *dest, ccp str, int str_len, int fw, int prec )
{
    int slen = str_len < 0
		? ScanUTF8Length(str)
		: ScanUTF8LengthE(str,str+str_len);

    if ( prec >= 0 && slen > prec )
	slen = prec;

    const int align_left = fw < 0;
    if (align_left)
	fw = -fw;
    if ( fw < slen )
	fw = slen;
    int spaces = fw - slen;

    ccp end = SkipUTF8Char(str,slen);
    const int copy_len = end - str;

    exmem_t res = GetExmemDestBuf(dest,copy_len+spaces);
    char *buf = (char*)res.data.ptr;

    DASSERT(buf);
    if (align_left)
    {
	memcpy(buf,str,copy_len);
	memset(buf+copy_len,' ',spaces);
    }
    else
    {
	memset(buf,' ',spaces);
	memcpy(buf+spaces,str,copy_len);
    }
    return res;
}

///////////////////////////////////////////////////////////////////////////////

ccp AlignUTF8ToCircBuf ( ccp str, int fw, int prec )
{
    uint len = ScanUTF8Length(str);
    if ( prec >= 0 && len > prec )
	len = prec;

    const int align_left = fw < 0;
    if ( align_left)
	fw = -fw;
    if ( fw < len )
	fw = len;
    int spaces = fw - len;

    ccp end = SkipUTF8Char(str,len);
    const int copy_len = end - str;

    char *buf = GetCircBuf(copy_len+spaces+1);
    DASSERT(buf);
    if (align_left)
    {
	memcpy(buf,str,copy_len);
	memset(buf+copy_len,' ',spaces);
    }
    else
    {
	memset(buf,' ',spaces);
	memcpy(buf+spaces,str,copy_len);
    }
    buf[copy_len+spaces] = 0;
    return buf;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

exmem_t AlignEUTF8 ( exmem_dest_t *dest, ccp str, int str_len, int fw, int prec )
{
    int slen = str_len < 0
		? ScanUTF8Length(str)
		: ScanUTF8LengthE(str,str+str_len);

    if ( prec >= 0 && slen > prec )
	slen = prec;

    const int align_left = fw < 0;
    if (align_left)
	fw = -fw;
    if ( fw < slen )
	fw = slen;
    int spaces = fw - slen;

    ccp end = SkipEUTF8Char(str,slen);
    const int copy_len = end - str;

    exmem_t res = GetExmemDestBuf(dest,copy_len+spaces);
    char *buf = (char*)res.data.ptr;

    DASSERT(buf);
    if (align_left)
    {
	memcpy(buf,str,copy_len);
	memset(buf+copy_len,' ',spaces);
    }
    else
    {
	memset(buf,' ',spaces);
	memcpy(buf+spaces,str,copy_len);
    }
    return res;
}

///////////////////////////////////////////////////////////////////////////////

ccp AlignEUTF8ToCircBuf ( ccp str, int fw, int prec )
{
    uint len = ScanEUTF8Length(str);
    if ( prec >= 0 && len > prec )
	len = prec;

    const int align_left = fw < 0;
    if ( align_left)
	fw = -fw;
    if ( fw < len )
	fw = len;
    int spaces = fw - len;

    ccp end = SkipEUTF8Char(str,len);
    const int copy_len = end - str;

    char *buf = GetCircBuf(copy_len+spaces+1);
    DASSERT(buf);
    if (align_left)
    {
	memcpy(buf,str,copy_len);
	memset(buf+copy_len,' ',spaces);
    }
    else
    {
	memset(buf,' ',spaces);
	memcpy(buf+spaces,str,copy_len);
    }
    buf[copy_len+spaces] = 0;
    return buf;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// this table is directly generated from
// http://unicode.org/Public/UNIDATA/UnicodeData.txt

const dcUnicodeTripel TableUnicodeDecomp[] =
{
	{ 0x0000c0, 0x000041,0x000300 },
	{ 0x0000c1, 0x000041,0x000301 },
	{ 0x0000c2, 0x000041,0x000302 },
	{ 0x0000c3, 0x000041,0x000303 },
	{ 0x0000c4, 0x000041,0x000308 },
	{ 0x0000c5, 0x000041,0x00030a },
	{ 0x0000c7, 0x000043,0x000327 },
	{ 0x0000c8, 0x000045,0x000300 },
	{ 0x0000c9, 0x000045,0x000301 },
	{ 0x0000ca, 0x000045,0x000302 },
	{ 0x0000cb, 0x000045,0x000308 },
	{ 0x0000cc, 0x000049,0x000300 },
	{ 0x0000cd, 0x000049,0x000301 },
	{ 0x0000ce, 0x000049,0x000302 },
	{ 0x0000cf, 0x000049,0x000308 },
	{ 0x0000d1, 0x00004e,0x000303 },
	{ 0x0000d2, 0x00004f,0x000300 },
	{ 0x0000d3, 0x00004f,0x000301 },
	{ 0x0000d4, 0x00004f,0x000302 },
	{ 0x0000d5, 0x00004f,0x000303 },
	{ 0x0000d6, 0x00004f,0x000308 },
	{ 0x0000d9, 0x000055,0x000300 },
	{ 0x0000da, 0x000055,0x000301 },
	{ 0x0000db, 0x000055,0x000302 },
	{ 0x0000dc, 0x000055,0x000308 },
	{ 0x0000dd, 0x000059,0x000301 },
	{ 0x0000e0, 0x000061,0x000300 },
	{ 0x0000e1, 0x000061,0x000301 },
	{ 0x0000e2, 0x000061,0x000302 },
	{ 0x0000e3, 0x000061,0x000303 },
	{ 0x0000e4, 0x000061,0x000308 },
	{ 0x0000e5, 0x000061,0x00030a },
	{ 0x0000e7, 0x000063,0x000327 },
	{ 0x0000e8, 0x000065,0x000300 },
	{ 0x0000e9, 0x000065,0x000301 },
	{ 0x0000ea, 0x000065,0x000302 },
	{ 0x0000eb, 0x000065,0x000308 },
	{ 0x0000ec, 0x000069,0x000300 },
	{ 0x0000ed, 0x000069,0x000301 },
	{ 0x0000ee, 0x000069,0x000302 },
	{ 0x0000ef, 0x000069,0x000308 },
	{ 0x0000f1, 0x00006e,0x000303 },
	{ 0x0000f2, 0x00006f,0x000300 },
	{ 0x0000f3, 0x00006f,0x000301 },
	{ 0x0000f4, 0x00006f,0x000302 },
	{ 0x0000f5, 0x00006f,0x000303 },
	{ 0x0000f6, 0x00006f,0x000308 },
	{ 0x0000f9, 0x000075,0x000300 },
	{ 0x0000fa, 0x000075,0x000301 },
	{ 0x0000fb, 0x000075,0x000302 },
	{ 0x0000fc, 0x000075,0x000308 },
	{ 0x0000fd, 0x000079,0x000301 },
	{ 0x0000ff, 0x000079,0x000308 },
	{ 0x000100, 0x000041,0x000304 },
	{ 0x000101, 0x000061,0x000304 },
	{ 0x000102, 0x000041,0x000306 },
	{ 0x000103, 0x000061,0x000306 },
	{ 0x000104, 0x000041,0x000328 },
	{ 0x000105, 0x000061,0x000328 },
	{ 0x000106, 0x000043,0x000301 },
	{ 0x000107, 0x000063,0x000301 },
	{ 0x000108, 0x000043,0x000302 },
	{ 0x000109, 0x000063,0x000302 },
	{ 0x00010a, 0x000043,0x000307 },
	{ 0x00010b, 0x000063,0x000307 },
	{ 0x00010c, 0x000043,0x00030c },
	{ 0x00010d, 0x000063,0x00030c },
	{ 0x00010e, 0x000044,0x00030c },
	{ 0x00010f, 0x000064,0x00030c },
	{ 0x000112, 0x000045,0x000304 },
	{ 0x000113, 0x000065,0x000304 },
	{ 0x000114, 0x000045,0x000306 },
	{ 0x000115, 0x000065,0x000306 },
	{ 0x000116, 0x000045,0x000307 },
	{ 0x000117, 0x000065,0x000307 },
	{ 0x000118, 0x000045,0x000328 },
	{ 0x000119, 0x000065,0x000328 },
	{ 0x00011a, 0x000045,0x00030c },
	{ 0x00011b, 0x000065,0x00030c },
	{ 0x00011c, 0x000047,0x000302 },
	{ 0x00011d, 0x000067,0x000302 },
	{ 0x00011e, 0x000047,0x000306 },
	{ 0x00011f, 0x000067,0x000306 },
	{ 0x000120, 0x000047,0x000307 },
	{ 0x000121, 0x000067,0x000307 },
	{ 0x000122, 0x000047,0x000327 },
	{ 0x000123, 0x000067,0x000327 },
	{ 0x000124, 0x000048,0x000302 },
	{ 0x000125, 0x000068,0x000302 },
	{ 0x000128, 0x000049,0x000303 },
	{ 0x000129, 0x000069,0x000303 },
	{ 0x00012a, 0x000049,0x000304 },
	{ 0x00012b, 0x000069,0x000304 },
	{ 0x00012c, 0x000049,0x000306 },
	{ 0x00012d, 0x000069,0x000306 },
	{ 0x00012e, 0x000049,0x000328 },
	{ 0x00012f, 0x000069,0x000328 },
	{ 0x000130, 0x000049,0x000307 },
	{ 0x000134, 0x00004a,0x000302 },
	{ 0x000135, 0x00006a,0x000302 },
	{ 0x000136, 0x00004b,0x000327 },
	{ 0x000137, 0x00006b,0x000327 },
	{ 0x000139, 0x00004c,0x000301 },
	{ 0x00013a, 0x00006c,0x000301 },
	{ 0x00013b, 0x00004c,0x000327 },
	{ 0x00013c, 0x00006c,0x000327 },
	{ 0x00013d, 0x00004c,0x00030c },
	{ 0x00013e, 0x00006c,0x00030c },
	{ 0x000143, 0x00004e,0x000301 },
	{ 0x000144, 0x00006e,0x000301 },
	{ 0x000145, 0x00004e,0x000327 },
	{ 0x000146, 0x00006e,0x000327 },
	{ 0x000147, 0x00004e,0x00030c },
	{ 0x000148, 0x00006e,0x00030c },
	{ 0x00014c, 0x00004f,0x000304 },
	{ 0x00014d, 0x00006f,0x000304 },
	{ 0x00014e, 0x00004f,0x000306 },
	{ 0x00014f, 0x00006f,0x000306 },
	{ 0x000150, 0x00004f,0x00030b },
	{ 0x000151, 0x00006f,0x00030b },
	{ 0x000154, 0x000052,0x000301 },
	{ 0x000155, 0x000072,0x000301 },
	{ 0x000156, 0x000052,0x000327 },
	{ 0x000157, 0x000072,0x000327 },
	{ 0x000158, 0x000052,0x00030c },
	{ 0x000159, 0x000072,0x00030c },
	{ 0x00015a, 0x000053,0x000301 },
	{ 0x00015b, 0x000073,0x000301 },
	{ 0x00015c, 0x000053,0x000302 },
	{ 0x00015d, 0x000073,0x000302 },
	{ 0x00015e, 0x000053,0x000327 },
	{ 0x00015f, 0x000073,0x000327 },
	{ 0x000160, 0x000053,0x00030c },
	{ 0x000161, 0x000073,0x00030c },
	{ 0x000162, 0x000054,0x000327 },
	{ 0x000163, 0x000074,0x000327 },
	{ 0x000164, 0x000054,0x00030c },
	{ 0x000165, 0x000074,0x00030c },
	{ 0x000168, 0x000055,0x000303 },
	{ 0x000169, 0x000075,0x000303 },
	{ 0x00016a, 0x000055,0x000304 },
	{ 0x00016b, 0x000075,0x000304 },
	{ 0x00016c, 0x000055,0x000306 },
	{ 0x00016d, 0x000075,0x000306 },
	{ 0x00016e, 0x000055,0x00030a },
	{ 0x00016f, 0x000075,0x00030a },
	{ 0x000170, 0x000055,0x00030b },
	{ 0x000171, 0x000075,0x00030b },
	{ 0x000172, 0x000055,0x000328 },
	{ 0x000173, 0x000075,0x000328 },
	{ 0x000174, 0x000057,0x000302 },
	{ 0x000175, 0x000077,0x000302 },
	{ 0x000176, 0x000059,0x000302 },
	{ 0x000177, 0x000079,0x000302 },
	{ 0x000178, 0x000059,0x000308 },
	{ 0x000179, 0x00005a,0x000301 },
	{ 0x00017a, 0x00007a,0x000301 },
	{ 0x00017b, 0x00005a,0x000307 },
	{ 0x00017c, 0x00007a,0x000307 },
	{ 0x00017d, 0x00005a,0x00030c },
	{ 0x00017e, 0x00007a,0x00030c },
	{ 0x0001a0, 0x00004f,0x00031b },
	{ 0x0001a1, 0x00006f,0x00031b },
	{ 0x0001af, 0x000055,0x00031b },
	{ 0x0001b0, 0x000075,0x00031b },
	{ 0x0001cd, 0x000041,0x00030c },
	{ 0x0001ce, 0x000061,0x00030c },
	{ 0x0001cf, 0x000049,0x00030c },
	{ 0x0001d0, 0x000069,0x00030c },
	{ 0x0001d1, 0x00004f,0x00030c },
	{ 0x0001d2, 0x00006f,0x00030c },
	{ 0x0001d3, 0x000055,0x00030c },
	{ 0x0001d4, 0x000075,0x00030c },
	{ 0x0001d5, 0x0000dc,0x000304 },
	{ 0x0001d6, 0x0000fc,0x000304 },
	{ 0x0001d7, 0x0000dc,0x000301 },
	{ 0x0001d8, 0x0000fc,0x000301 },
	{ 0x0001d9, 0x0000dc,0x00030c },
	{ 0x0001da, 0x0000fc,0x00030c },
	{ 0x0001db, 0x0000dc,0x000300 },
	{ 0x0001dc, 0x0000fc,0x000300 },
	{ 0x0001de, 0x0000c4,0x000304 },
	{ 0x0001df, 0x0000e4,0x000304 },
	{ 0x0001e0, 0x000226,0x000304 },
	{ 0x0001e1, 0x000227,0x000304 },
	{ 0x0001e2, 0x0000c6,0x000304 },
	{ 0x0001e3, 0x0000e6,0x000304 },
	{ 0x0001e6, 0x000047,0x00030c },
	{ 0x0001e7, 0x000067,0x00030c },
	{ 0x0001e8, 0x00004b,0x00030c },
	{ 0x0001e9, 0x00006b,0x00030c },
	{ 0x0001ea, 0x00004f,0x000328 },
	{ 0x0001eb, 0x00006f,0x000328 },
	{ 0x0001ec, 0x0001ea,0x000304 },
	{ 0x0001ed, 0x0001eb,0x000304 },
	{ 0x0001ee, 0x0001b7,0x00030c },
	{ 0x0001ef, 0x000292,0x00030c },
	{ 0x0001f0, 0x00006a,0x00030c },
	{ 0x0001f4, 0x000047,0x000301 },
	{ 0x0001f5, 0x000067,0x000301 },
	{ 0x0001f8, 0x00004e,0x000300 },
	{ 0x0001f9, 0x00006e,0x000300 },
	{ 0x0001fa, 0x0000c5,0x000301 },
	{ 0x0001fb, 0x0000e5,0x000301 },
	{ 0x0001fc, 0x0000c6,0x000301 },
	{ 0x0001fd, 0x0000e6,0x000301 },
	{ 0x0001fe, 0x0000d8,0x000301 },
	{ 0x0001ff, 0x0000f8,0x000301 },
	{ 0x000200, 0x000041,0x00030f },
	{ 0x000201, 0x000061,0x00030f },
	{ 0x000202, 0x000041,0x000311 },
	{ 0x000203, 0x000061,0x000311 },
	{ 0x000204, 0x000045,0x00030f },
	{ 0x000205, 0x000065,0x00030f },
	{ 0x000206, 0x000045,0x000311 },
	{ 0x000207, 0x000065,0x000311 },
	{ 0x000208, 0x000049,0x00030f },
	{ 0x000209, 0x000069,0x00030f },
	{ 0x00020a, 0x000049,0x000311 },
	{ 0x00020b, 0x000069,0x000311 },
	{ 0x00020c, 0x00004f,0x00030f },
	{ 0x00020d, 0x00006f,0x00030f },
	{ 0x00020e, 0x00004f,0x000311 },
	{ 0x00020f, 0x00006f,0x000311 },
	{ 0x000210, 0x000052,0x00030f },
	{ 0x000211, 0x000072,0x00030f },
	{ 0x000212, 0x000052,0x000311 },
	{ 0x000213, 0x000072,0x000311 },
	{ 0x000214, 0x000055,0x00030f },
	{ 0x000215, 0x000075,0x00030f },
	{ 0x000216, 0x000055,0x000311 },
	{ 0x000217, 0x000075,0x000311 },
	{ 0x000218, 0x000053,0x000326 },
	{ 0x000219, 0x000073,0x000326 },
	{ 0x00021a, 0x000054,0x000326 },
	{ 0x00021b, 0x000074,0x000326 },
	{ 0x00021e, 0x000048,0x00030c },
	{ 0x00021f, 0x000068,0x00030c },
	{ 0x000226, 0x000041,0x000307 },
	{ 0x000227, 0x000061,0x000307 },
	{ 0x000228, 0x000045,0x000327 },
	{ 0x000229, 0x000065,0x000327 },
	{ 0x00022a, 0x0000d6,0x000304 },
	{ 0x00022b, 0x0000f6,0x000304 },
	{ 0x00022c, 0x0000d5,0x000304 },
	{ 0x00022d, 0x0000f5,0x000304 },
	{ 0x00022e, 0x00004f,0x000307 },
	{ 0x00022f, 0x00006f,0x000307 },
	{ 0x000230, 0x00022e,0x000304 },
	{ 0x000231, 0x00022f,0x000304 },
	{ 0x000232, 0x000059,0x000304 },
	{ 0x000233, 0x000079,0x000304 },
	{ 0x000344, 0x000308,0x000301 },
	{ 0x000385, 0x0000a8,0x000301 },
	{ 0x000386, 0x000391,0x000301 },
	{ 0x000388, 0x000395,0x000301 },
	{ 0x000389, 0x000397,0x000301 },
	{ 0x00038a, 0x000399,0x000301 },
	{ 0x00038c, 0x00039f,0x000301 },
	{ 0x00038e, 0x0003a5,0x000301 },
	{ 0x00038f, 0x0003a9,0x000301 },
	{ 0x000390, 0x0003ca,0x000301 },
	{ 0x0003aa, 0x000399,0x000308 },
	{ 0x0003ab, 0x0003a5,0x000308 },
	{ 0x0003ac, 0x0003b1,0x000301 },
	{ 0x0003ad, 0x0003b5,0x000301 },
	{ 0x0003ae, 0x0003b7,0x000301 },
	{ 0x0003af, 0x0003b9,0x000301 },
	{ 0x0003b0, 0x0003cb,0x000301 },
	{ 0x0003ca, 0x0003b9,0x000308 },
	{ 0x0003cb, 0x0003c5,0x000308 },
	{ 0x0003cc, 0x0003bf,0x000301 },
	{ 0x0003cd, 0x0003c5,0x000301 },
	{ 0x0003ce, 0x0003c9,0x000301 },
	{ 0x0003d3, 0x0003d2,0x000301 },
	{ 0x0003d4, 0x0003d2,0x000308 },
	{ 0x000400, 0x000415,0x000300 },
	{ 0x000401, 0x000415,0x000308 },
	{ 0x000403, 0x000413,0x000301 },
	{ 0x000407, 0x000406,0x000308 },
	{ 0x00040c, 0x00041a,0x000301 },
	{ 0x00040d, 0x000418,0x000300 },
	{ 0x00040e, 0x000423,0x000306 },
	{ 0x000419, 0x000418,0x000306 },
	{ 0x000439, 0x000438,0x000306 },
	{ 0x000450, 0x000435,0x000300 },
	{ 0x000451, 0x000435,0x000308 },
	{ 0x000453, 0x000433,0x000301 },
	{ 0x000457, 0x000456,0x000308 },
	{ 0x00045c, 0x00043a,0x000301 },
	{ 0x00045d, 0x000438,0x000300 },
	{ 0x00045e, 0x000443,0x000306 },
	{ 0x000476, 0x000474,0x00030f },
	{ 0x000477, 0x000475,0x00030f },
	{ 0x0004c1, 0x000416,0x000306 },
	{ 0x0004c2, 0x000436,0x000306 },
	{ 0x0004d0, 0x000410,0x000306 },
	{ 0x0004d1, 0x000430,0x000306 },
	{ 0x0004d2, 0x000410,0x000308 },
	{ 0x0004d3, 0x000430,0x000308 },
	{ 0x0004d6, 0x000415,0x000306 },
	{ 0x0004d7, 0x000435,0x000306 },
	{ 0x0004da, 0x0004d8,0x000308 },
	{ 0x0004db, 0x0004d9,0x000308 },
	{ 0x0004dc, 0x000416,0x000308 },
	{ 0x0004dd, 0x000436,0x000308 },
	{ 0x0004de, 0x000417,0x000308 },
	{ 0x0004df, 0x000437,0x000308 },
	{ 0x0004e2, 0x000418,0x000304 },
	{ 0x0004e3, 0x000438,0x000304 },
	{ 0x0004e4, 0x000418,0x000308 },
	{ 0x0004e5, 0x000438,0x000308 },
	{ 0x0004e6, 0x00041e,0x000308 },
	{ 0x0004e7, 0x00043e,0x000308 },
	{ 0x0004ea, 0x0004e8,0x000308 },
	{ 0x0004eb, 0x0004e9,0x000308 },
	{ 0x0004ec, 0x00042d,0x000308 },
	{ 0x0004ed, 0x00044d,0x000308 },
	{ 0x0004ee, 0x000423,0x000304 },
	{ 0x0004ef, 0x000443,0x000304 },
	{ 0x0004f0, 0x000423,0x000308 },
	{ 0x0004f1, 0x000443,0x000308 },
	{ 0x0004f2, 0x000423,0x00030b },
	{ 0x0004f3, 0x000443,0x00030b },
	{ 0x0004f4, 0x000427,0x000308 },
	{ 0x0004f5, 0x000447,0x000308 },
	{ 0x0004f8, 0x00042b,0x000308 },
	{ 0x0004f9, 0x00044b,0x000308 },
	{ 0x000622, 0x000627,0x000653 },
	{ 0x000623, 0x000627,0x000654 },
	{ 0x000624, 0x000648,0x000654 },
	{ 0x000625, 0x000627,0x000655 },
	{ 0x000626, 0x00064a,0x000654 },
	{ 0x0006c0, 0x0006d5,0x000654 },
	{ 0x0006c2, 0x0006c1,0x000654 },
	{ 0x0006d3, 0x0006d2,0x000654 },
	{ 0x000929, 0x000928,0x00093c },
	{ 0x000931, 0x000930,0x00093c },
	{ 0x000934, 0x000933,0x00093c },
	{ 0x000958, 0x000915,0x00093c },
	{ 0x000959, 0x000916,0x00093c },
	{ 0x00095a, 0x000917,0x00093c },
	{ 0x00095b, 0x00091c,0x00093c },
	{ 0x00095c, 0x000921,0x00093c },
	{ 0x00095d, 0x000922,0x00093c },
	{ 0x00095e, 0x00092b,0x00093c },
	{ 0x00095f, 0x00092f,0x00093c },
	{ 0x0009cb, 0x0009c7,0x0009be },
	{ 0x0009cc, 0x0009c7,0x0009d7 },
	{ 0x0009dc, 0x0009a1,0x0009bc },
	{ 0x0009dd, 0x0009a2,0x0009bc },
	{ 0x0009df, 0x0009af,0x0009bc },
	{ 0x000a33, 0x000a32,0x000a3c },
	{ 0x000a36, 0x000a38,0x000a3c },
	{ 0x000a59, 0x000a16,0x000a3c },
	{ 0x000a5a, 0x000a17,0x000a3c },
	{ 0x000a5b, 0x000a1c,0x000a3c },
	{ 0x000a5e, 0x000a2b,0x000a3c },
	{ 0x000b48, 0x000b47,0x000b56 },
	{ 0x000b4b, 0x000b47,0x000b3e },
	{ 0x000b4c, 0x000b47,0x000b57 },
	{ 0x000b5c, 0x000b21,0x000b3c },
	{ 0x000b5d, 0x000b22,0x000b3c },
	{ 0x000b94, 0x000b92,0x000bd7 },
	{ 0x000bca, 0x000bc6,0x000bbe },
	{ 0x000bcb, 0x000bc7,0x000bbe },
	{ 0x000bcc, 0x000bc6,0x000bd7 },
	{ 0x000c48, 0x000c46,0x000c56 },
	{ 0x000cc0, 0x000cbf,0x000cd5 },
	{ 0x000cc7, 0x000cc6,0x000cd5 },
	{ 0x000cc8, 0x000cc6,0x000cd6 },
	{ 0x000cca, 0x000cc6,0x000cc2 },
	{ 0x000ccb, 0x000cca,0x000cd5 },
	{ 0x000d4a, 0x000d46,0x000d3e },
	{ 0x000d4b, 0x000d47,0x000d3e },
	{ 0x000d4c, 0x000d46,0x000d57 },
	{ 0x000dda, 0x000dd9,0x000dca },
	{ 0x000ddc, 0x000dd9,0x000dcf },
	{ 0x000ddd, 0x000ddc,0x000dca },
	{ 0x000dde, 0x000dd9,0x000ddf },
	{ 0x000f43, 0x000f42,0x000fb7 },
	{ 0x000f4d, 0x000f4c,0x000fb7 },
	{ 0x000f52, 0x000f51,0x000fb7 },
	{ 0x000f57, 0x000f56,0x000fb7 },
	{ 0x000f5c, 0x000f5b,0x000fb7 },
	{ 0x000f69, 0x000f40,0x000fb5 },
	{ 0x000f73, 0x000f71,0x000f72 },
	{ 0x000f75, 0x000f71,0x000f74 },
	{ 0x000f76, 0x000fb2,0x000f80 },
	{ 0x000f78, 0x000fb3,0x000f80 },
	{ 0x000f81, 0x000f71,0x000f80 },
	{ 0x000f93, 0x000f92,0x000fb7 },
	{ 0x000f9d, 0x000f9c,0x000fb7 },
	{ 0x000fa2, 0x000fa1,0x000fb7 },
	{ 0x000fa7, 0x000fa6,0x000fb7 },
	{ 0x000fac, 0x000fab,0x000fb7 },
	{ 0x000fb9, 0x000f90,0x000fb5 },
	{ 0x001026, 0x001025,0x00102e },
	{ 0x001b06, 0x001b05,0x001b35 },
	{ 0x001b08, 0x001b07,0x001b35 },
	{ 0x001b0a, 0x001b09,0x001b35 },
	{ 0x001b0c, 0x001b0b,0x001b35 },
	{ 0x001b0e, 0x001b0d,0x001b35 },
	{ 0x001b12, 0x001b11,0x001b35 },
	{ 0x001b3b, 0x001b3a,0x001b35 },
	{ 0x001b3d, 0x001b3c,0x001b35 },
	{ 0x001b40, 0x001b3e,0x001b35 },
	{ 0x001b41, 0x001b3f,0x001b35 },
	{ 0x001b43, 0x001b42,0x001b35 },
	{ 0x001e00, 0x000041,0x000325 },
	{ 0x001e01, 0x000061,0x000325 },
	{ 0x001e02, 0x000042,0x000307 },
	{ 0x001e03, 0x000062,0x000307 },
	{ 0x001e04, 0x000042,0x000323 },
	{ 0x001e05, 0x000062,0x000323 },
	{ 0x001e06, 0x000042,0x000331 },
	{ 0x001e07, 0x000062,0x000331 },
	{ 0x001e08, 0x0000c7,0x000301 },
	{ 0x001e09, 0x0000e7,0x000301 },
	{ 0x001e0a, 0x000044,0x000307 },
	{ 0x001e0b, 0x000064,0x000307 },
	{ 0x001e0c, 0x000044,0x000323 },
	{ 0x001e0d, 0x000064,0x000323 },
	{ 0x001e0e, 0x000044,0x000331 },
	{ 0x001e0f, 0x000064,0x000331 },
	{ 0x001e10, 0x000044,0x000327 },
	{ 0x001e11, 0x000064,0x000327 },
	{ 0x001e12, 0x000044,0x00032d },
	{ 0x001e13, 0x000064,0x00032d },
	{ 0x001e14, 0x000112,0x000300 },
	{ 0x001e15, 0x000113,0x000300 },
	{ 0x001e16, 0x000112,0x000301 },
	{ 0x001e17, 0x000113,0x000301 },
	{ 0x001e18, 0x000045,0x00032d },
	{ 0x001e19, 0x000065,0x00032d },
	{ 0x001e1a, 0x000045,0x000330 },
	{ 0x001e1b, 0x000065,0x000330 },
	{ 0x001e1c, 0x000228,0x000306 },
	{ 0x001e1d, 0x000229,0x000306 },
	{ 0x001e1e, 0x000046,0x000307 },
	{ 0x001e1f, 0x000066,0x000307 },
	{ 0x001e20, 0x000047,0x000304 },
	{ 0x001e21, 0x000067,0x000304 },
	{ 0x001e22, 0x000048,0x000307 },
	{ 0x001e23, 0x000068,0x000307 },
	{ 0x001e24, 0x000048,0x000323 },
	{ 0x001e25, 0x000068,0x000323 },
	{ 0x001e26, 0x000048,0x000308 },
	{ 0x001e27, 0x000068,0x000308 },
	{ 0x001e28, 0x000048,0x000327 },
	{ 0x001e29, 0x000068,0x000327 },
	{ 0x001e2a, 0x000048,0x00032e },
	{ 0x001e2b, 0x000068,0x00032e },
	{ 0x001e2c, 0x000049,0x000330 },
	{ 0x001e2d, 0x000069,0x000330 },
	{ 0x001e2e, 0x0000cf,0x000301 },
	{ 0x001e2f, 0x0000ef,0x000301 },
	{ 0x001e30, 0x00004b,0x000301 },
	{ 0x001e31, 0x00006b,0x000301 },
	{ 0x001e32, 0x00004b,0x000323 },
	{ 0x001e33, 0x00006b,0x000323 },
	{ 0x001e34, 0x00004b,0x000331 },
	{ 0x001e35, 0x00006b,0x000331 },
	{ 0x001e36, 0x00004c,0x000323 },
	{ 0x001e37, 0x00006c,0x000323 },
	{ 0x001e38, 0x001e36,0x000304 },
	{ 0x001e39, 0x001e37,0x000304 },
	{ 0x001e3a, 0x00004c,0x000331 },
	{ 0x001e3b, 0x00006c,0x000331 },
	{ 0x001e3c, 0x00004c,0x00032d },
	{ 0x001e3d, 0x00006c,0x00032d },
	{ 0x001e3e, 0x00004d,0x000301 },
	{ 0x001e3f, 0x00006d,0x000301 },
	{ 0x001e40, 0x00004d,0x000307 },
	{ 0x001e41, 0x00006d,0x000307 },
	{ 0x001e42, 0x00004d,0x000323 },
	{ 0x001e43, 0x00006d,0x000323 },
	{ 0x001e44, 0x00004e,0x000307 },
	{ 0x001e45, 0x00006e,0x000307 },
	{ 0x001e46, 0x00004e,0x000323 },
	{ 0x001e47, 0x00006e,0x000323 },
	{ 0x001e48, 0x00004e,0x000331 },
	{ 0x001e49, 0x00006e,0x000331 },
	{ 0x001e4a, 0x00004e,0x00032d },
	{ 0x001e4b, 0x00006e,0x00032d },
	{ 0x001e4c, 0x0000d5,0x000301 },
	{ 0x001e4d, 0x0000f5,0x000301 },
	{ 0x001e4e, 0x0000d5,0x000308 },
	{ 0x001e4f, 0x0000f5,0x000308 },
	{ 0x001e50, 0x00014c,0x000300 },
	{ 0x001e51, 0x00014d,0x000300 },
	{ 0x001e52, 0x00014c,0x000301 },
	{ 0x001e53, 0x00014d,0x000301 },
	{ 0x001e54, 0x000050,0x000301 },
	{ 0x001e55, 0x000070,0x000301 },
	{ 0x001e56, 0x000050,0x000307 },
	{ 0x001e57, 0x000070,0x000307 },
	{ 0x001e58, 0x000052,0x000307 },
	{ 0x001e59, 0x000072,0x000307 },
	{ 0x001e5a, 0x000052,0x000323 },
	{ 0x001e5b, 0x000072,0x000323 },
	{ 0x001e5c, 0x001e5a,0x000304 },
	{ 0x001e5d, 0x001e5b,0x000304 },
	{ 0x001e5e, 0x000052,0x000331 },
	{ 0x001e5f, 0x000072,0x000331 },
	{ 0x001e60, 0x000053,0x000307 },
	{ 0x001e61, 0x000073,0x000307 },
	{ 0x001e62, 0x000053,0x000323 },
	{ 0x001e63, 0x000073,0x000323 },
	{ 0x001e64, 0x00015a,0x000307 },
	{ 0x001e65, 0x00015b,0x000307 },
	{ 0x001e66, 0x000160,0x000307 },
	{ 0x001e67, 0x000161,0x000307 },
	{ 0x001e68, 0x001e62,0x000307 },
	{ 0x001e69, 0x001e63,0x000307 },
	{ 0x001e6a, 0x000054,0x000307 },
	{ 0x001e6b, 0x000074,0x000307 },
	{ 0x001e6c, 0x000054,0x000323 },
	{ 0x001e6d, 0x000074,0x000323 },
	{ 0x001e6e, 0x000054,0x000331 },
	{ 0x001e6f, 0x000074,0x000331 },
	{ 0x001e70, 0x000054,0x00032d },
	{ 0x001e71, 0x000074,0x00032d },
	{ 0x001e72, 0x000055,0x000324 },
	{ 0x001e73, 0x000075,0x000324 },
	{ 0x001e74, 0x000055,0x000330 },
	{ 0x001e75, 0x000075,0x000330 },
	{ 0x001e76, 0x000055,0x00032d },
	{ 0x001e77, 0x000075,0x00032d },
	{ 0x001e78, 0x000168,0x000301 },
	{ 0x001e79, 0x000169,0x000301 },
	{ 0x001e7a, 0x00016a,0x000308 },
	{ 0x001e7b, 0x00016b,0x000308 },
	{ 0x001e7c, 0x000056,0x000303 },
	{ 0x001e7d, 0x000076,0x000303 },
	{ 0x001e7e, 0x000056,0x000323 },
	{ 0x001e7f, 0x000076,0x000323 },
	{ 0x001e80, 0x000057,0x000300 },
	{ 0x001e81, 0x000077,0x000300 },
	{ 0x001e82, 0x000057,0x000301 },
	{ 0x001e83, 0x000077,0x000301 },
	{ 0x001e84, 0x000057,0x000308 },
	{ 0x001e85, 0x000077,0x000308 },
	{ 0x001e86, 0x000057,0x000307 },
	{ 0x001e87, 0x000077,0x000307 },
	{ 0x001e88, 0x000057,0x000323 },
	{ 0x001e89, 0x000077,0x000323 },
	{ 0x001e8a, 0x000058,0x000307 },
	{ 0x001e8b, 0x000078,0x000307 },
	{ 0x001e8c, 0x000058,0x000308 },
	{ 0x001e8d, 0x000078,0x000308 },
	{ 0x001e8e, 0x000059,0x000307 },
	{ 0x001e8f, 0x000079,0x000307 },
	{ 0x001e90, 0x00005a,0x000302 },
	{ 0x001e91, 0x00007a,0x000302 },
	{ 0x001e92, 0x00005a,0x000323 },
	{ 0x001e93, 0x00007a,0x000323 },
	{ 0x001e94, 0x00005a,0x000331 },
	{ 0x001e95, 0x00007a,0x000331 },
	{ 0x001e96, 0x000068,0x000331 },
	{ 0x001e97, 0x000074,0x000308 },
	{ 0x001e98, 0x000077,0x00030a },
	{ 0x001e99, 0x000079,0x00030a },
	{ 0x001e9b, 0x00017f,0x000307 },
	{ 0x001ea0, 0x000041,0x000323 },
	{ 0x001ea1, 0x000061,0x000323 },
	{ 0x001ea2, 0x000041,0x000309 },
	{ 0x001ea3, 0x000061,0x000309 },
	{ 0x001ea4, 0x0000c2,0x000301 },
	{ 0x001ea5, 0x0000e2,0x000301 },
	{ 0x001ea6, 0x0000c2,0x000300 },
	{ 0x001ea7, 0x0000e2,0x000300 },
	{ 0x001ea8, 0x0000c2,0x000309 },
	{ 0x001ea9, 0x0000e2,0x000309 },
	{ 0x001eaa, 0x0000c2,0x000303 },
	{ 0x001eab, 0x0000e2,0x000303 },
	{ 0x001eac, 0x001ea0,0x000302 },
	{ 0x001ead, 0x001ea1,0x000302 },
	{ 0x001eae, 0x000102,0x000301 },
	{ 0x001eaf, 0x000103,0x000301 },
	{ 0x001eb0, 0x000102,0x000300 },
	{ 0x001eb1, 0x000103,0x000300 },
	{ 0x001eb2, 0x000102,0x000309 },
	{ 0x001eb3, 0x000103,0x000309 },
	{ 0x001eb4, 0x000102,0x000303 },
	{ 0x001eb5, 0x000103,0x000303 },
	{ 0x001eb6, 0x001ea0,0x000306 },
	{ 0x001eb7, 0x001ea1,0x000306 },
	{ 0x001eb8, 0x000045,0x000323 },
	{ 0x001eb9, 0x000065,0x000323 },
	{ 0x001eba, 0x000045,0x000309 },
	{ 0x001ebb, 0x000065,0x000309 },
	{ 0x001ebc, 0x000045,0x000303 },
	{ 0x001ebd, 0x000065,0x000303 },
	{ 0x001ebe, 0x0000ca,0x000301 },
	{ 0x001ebf, 0x0000ea,0x000301 },
	{ 0x001ec0, 0x0000ca,0x000300 },
	{ 0x001ec1, 0x0000ea,0x000300 },
	{ 0x001ec2, 0x0000ca,0x000309 },
	{ 0x001ec3, 0x0000ea,0x000309 },
	{ 0x001ec4, 0x0000ca,0x000303 },
	{ 0x001ec5, 0x0000ea,0x000303 },
	{ 0x001ec6, 0x001eb8,0x000302 },
	{ 0x001ec7, 0x001eb9,0x000302 },
	{ 0x001ec8, 0x000049,0x000309 },
	{ 0x001ec9, 0x000069,0x000309 },
	{ 0x001eca, 0x000049,0x000323 },
	{ 0x001ecb, 0x000069,0x000323 },
	{ 0x001ecc, 0x00004f,0x000323 },
	{ 0x001ecd, 0x00006f,0x000323 },
	{ 0x001ece, 0x00004f,0x000309 },
	{ 0x001ecf, 0x00006f,0x000309 },
	{ 0x001ed0, 0x0000d4,0x000301 },
	{ 0x001ed1, 0x0000f4,0x000301 },
	{ 0x001ed2, 0x0000d4,0x000300 },
	{ 0x001ed3, 0x0000f4,0x000300 },
	{ 0x001ed4, 0x0000d4,0x000309 },
	{ 0x001ed5, 0x0000f4,0x000309 },
	{ 0x001ed6, 0x0000d4,0x000303 },
	{ 0x001ed7, 0x0000f4,0x000303 },
	{ 0x001ed8, 0x001ecc,0x000302 },
	{ 0x001ed9, 0x001ecd,0x000302 },
	{ 0x001eda, 0x0001a0,0x000301 },
	{ 0x001edb, 0x0001a1,0x000301 },
	{ 0x001edc, 0x0001a0,0x000300 },
	{ 0x001edd, 0x0001a1,0x000300 },
	{ 0x001ede, 0x0001a0,0x000309 },
	{ 0x001edf, 0x0001a1,0x000309 },
	{ 0x001ee0, 0x0001a0,0x000303 },
	{ 0x001ee1, 0x0001a1,0x000303 },
	{ 0x001ee2, 0x0001a0,0x000323 },
	{ 0x001ee3, 0x0001a1,0x000323 },
	{ 0x001ee4, 0x000055,0x000323 },
	{ 0x001ee5, 0x000075,0x000323 },
	{ 0x001ee6, 0x000055,0x000309 },
	{ 0x001ee7, 0x000075,0x000309 },
	{ 0x001ee8, 0x0001af,0x000301 },
	{ 0x001ee9, 0x0001b0,0x000301 },
	{ 0x001eea, 0x0001af,0x000300 },
	{ 0x001eeb, 0x0001b0,0x000300 },
	{ 0x001eec, 0x0001af,0x000309 },
	{ 0x001eed, 0x0001b0,0x000309 },
	{ 0x001eee, 0x0001af,0x000303 },
	{ 0x001eef, 0x0001b0,0x000303 },
	{ 0x001ef0, 0x0001af,0x000323 },
	{ 0x001ef1, 0x0001b0,0x000323 },
	{ 0x001ef2, 0x000059,0x000300 },
	{ 0x001ef3, 0x000079,0x000300 },
	{ 0x001ef4, 0x000059,0x000323 },
	{ 0x001ef5, 0x000079,0x000323 },
	{ 0x001ef6, 0x000059,0x000309 },
	{ 0x001ef7, 0x000079,0x000309 },
	{ 0x001ef8, 0x000059,0x000303 },
	{ 0x001ef9, 0x000079,0x000303 },
	{ 0x001f00, 0x0003b1,0x000313 },
	{ 0x001f01, 0x0003b1,0x000314 },
	{ 0x001f02, 0x001f00,0x000300 },
	{ 0x001f03, 0x001f01,0x000300 },
	{ 0x001f04, 0x001f00,0x000301 },
	{ 0x001f05, 0x001f01,0x000301 },
	{ 0x001f06, 0x001f00,0x000342 },
	{ 0x001f07, 0x001f01,0x000342 },
	{ 0x001f08, 0x000391,0x000313 },
	{ 0x001f09, 0x000391,0x000314 },
	{ 0x001f0a, 0x001f08,0x000300 },
	{ 0x001f0b, 0x001f09,0x000300 },
	{ 0x001f0c, 0x001f08,0x000301 },
	{ 0x001f0d, 0x001f09,0x000301 },
	{ 0x001f0e, 0x001f08,0x000342 },
	{ 0x001f0f, 0x001f09,0x000342 },
	{ 0x001f10, 0x0003b5,0x000313 },
	{ 0x001f11, 0x0003b5,0x000314 },
	{ 0x001f12, 0x001f10,0x000300 },
	{ 0x001f13, 0x001f11,0x000300 },
	{ 0x001f14, 0x001f10,0x000301 },
	{ 0x001f15, 0x001f11,0x000301 },
	{ 0x001f18, 0x000395,0x000313 },
	{ 0x001f19, 0x000395,0x000314 },
	{ 0x001f1a, 0x001f18,0x000300 },
	{ 0x001f1b, 0x001f19,0x000300 },
	{ 0x001f1c, 0x001f18,0x000301 },
	{ 0x001f1d, 0x001f19,0x000301 },
	{ 0x001f20, 0x0003b7,0x000313 },
	{ 0x001f21, 0x0003b7,0x000314 },
	{ 0x001f22, 0x001f20,0x000300 },
	{ 0x001f23, 0x001f21,0x000300 },
	{ 0x001f24, 0x001f20,0x000301 },
	{ 0x001f25, 0x001f21,0x000301 },
	{ 0x001f26, 0x001f20,0x000342 },
	{ 0x001f27, 0x001f21,0x000342 },
	{ 0x001f28, 0x000397,0x000313 },
	{ 0x001f29, 0x000397,0x000314 },
	{ 0x001f2a, 0x001f28,0x000300 },
	{ 0x001f2b, 0x001f29,0x000300 },
	{ 0x001f2c, 0x001f28,0x000301 },
	{ 0x001f2d, 0x001f29,0x000301 },
	{ 0x001f2e, 0x001f28,0x000342 },
	{ 0x001f2f, 0x001f29,0x000342 },
	{ 0x001f30, 0x0003b9,0x000313 },
	{ 0x001f31, 0x0003b9,0x000314 },
	{ 0x001f32, 0x001f30,0x000300 },
	{ 0x001f33, 0x001f31,0x000300 },
	{ 0x001f34, 0x001f30,0x000301 },
	{ 0x001f35, 0x001f31,0x000301 },
	{ 0x001f36, 0x001f30,0x000342 },
	{ 0x001f37, 0x001f31,0x000342 },
	{ 0x001f38, 0x000399,0x000313 },
	{ 0x001f39, 0x000399,0x000314 },
	{ 0x001f3a, 0x001f38,0x000300 },
	{ 0x001f3b, 0x001f39,0x000300 },
	{ 0x001f3c, 0x001f38,0x000301 },
	{ 0x001f3d, 0x001f39,0x000301 },
	{ 0x001f3e, 0x001f38,0x000342 },
	{ 0x001f3f, 0x001f39,0x000342 },
	{ 0x001f40, 0x0003bf,0x000313 },
	{ 0x001f41, 0x0003bf,0x000314 },
	{ 0x001f42, 0x001f40,0x000300 },
	{ 0x001f43, 0x001f41,0x000300 },
	{ 0x001f44, 0x001f40,0x000301 },
	{ 0x001f45, 0x001f41,0x000301 },
	{ 0x001f48, 0x00039f,0x000313 },
	{ 0x001f49, 0x00039f,0x000314 },
	{ 0x001f4a, 0x001f48,0x000300 },
	{ 0x001f4b, 0x001f49,0x000300 },
	{ 0x001f4c, 0x001f48,0x000301 },
	{ 0x001f4d, 0x001f49,0x000301 },
	{ 0x001f50, 0x0003c5,0x000313 },
	{ 0x001f51, 0x0003c5,0x000314 },
	{ 0x001f52, 0x001f50,0x000300 },
	{ 0x001f53, 0x001f51,0x000300 },
	{ 0x001f54, 0x001f50,0x000301 },
	{ 0x001f55, 0x001f51,0x000301 },
	{ 0x001f56, 0x001f50,0x000342 },
	{ 0x001f57, 0x001f51,0x000342 },
	{ 0x001f59, 0x0003a5,0x000314 },
	{ 0x001f5b, 0x001f59,0x000300 },
	{ 0x001f5d, 0x001f59,0x000301 },
	{ 0x001f5f, 0x001f59,0x000342 },
	{ 0x001f60, 0x0003c9,0x000313 },
	{ 0x001f61, 0x0003c9,0x000314 },
	{ 0x001f62, 0x001f60,0x000300 },
	{ 0x001f63, 0x001f61,0x000300 },
	{ 0x001f64, 0x001f60,0x000301 },
	{ 0x001f65, 0x001f61,0x000301 },
	{ 0x001f66, 0x001f60,0x000342 },
	{ 0x001f67, 0x001f61,0x000342 },
	{ 0x001f68, 0x0003a9,0x000313 },
	{ 0x001f69, 0x0003a9,0x000314 },
	{ 0x001f6a, 0x001f68,0x000300 },
	{ 0x001f6b, 0x001f69,0x000300 },
	{ 0x001f6c, 0x001f68,0x000301 },
	{ 0x001f6d, 0x001f69,0x000301 },
	{ 0x001f6e, 0x001f68,0x000342 },
	{ 0x001f6f, 0x001f69,0x000342 },
	{ 0x001f70, 0x0003b1,0x000300 },
	{ 0x001f72, 0x0003b5,0x000300 },
	{ 0x001f74, 0x0003b7,0x000300 },
	{ 0x001f76, 0x0003b9,0x000300 },
	{ 0x001f78, 0x0003bf,0x000300 },
	{ 0x001f7a, 0x0003c5,0x000300 },
	{ 0x001f7c, 0x0003c9,0x000300 },
	{ 0x001f80, 0x001f00,0x000345 },
	{ 0x001f81, 0x001f01,0x000345 },
	{ 0x001f82, 0x001f02,0x000345 },
	{ 0x001f83, 0x001f03,0x000345 },
	{ 0x001f84, 0x001f04,0x000345 },
	{ 0x001f85, 0x001f05,0x000345 },
	{ 0x001f86, 0x001f06,0x000345 },
	{ 0x001f87, 0x001f07,0x000345 },
	{ 0x001f88, 0x001f08,0x000345 },
	{ 0x001f89, 0x001f09,0x000345 },
	{ 0x001f8a, 0x001f0a,0x000345 },
	{ 0x001f8b, 0x001f0b,0x000345 },
	{ 0x001f8c, 0x001f0c,0x000345 },
	{ 0x001f8d, 0x001f0d,0x000345 },
	{ 0x001f8e, 0x001f0e,0x000345 },
	{ 0x001f8f, 0x001f0f,0x000345 },
	{ 0x001f90, 0x001f20,0x000345 },
	{ 0x001f91, 0x001f21,0x000345 },
	{ 0x001f92, 0x001f22,0x000345 },
	{ 0x001f93, 0x001f23,0x000345 },
	{ 0x001f94, 0x001f24,0x000345 },
	{ 0x001f95, 0x001f25,0x000345 },
	{ 0x001f96, 0x001f26,0x000345 },
	{ 0x001f97, 0x001f27,0x000345 },
	{ 0x001f98, 0x001f28,0x000345 },
	{ 0x001f99, 0x001f29,0x000345 },
	{ 0x001f9a, 0x001f2a,0x000345 },
	{ 0x001f9b, 0x001f2b,0x000345 },
	{ 0x001f9c, 0x001f2c,0x000345 },
	{ 0x001f9d, 0x001f2d,0x000345 },
	{ 0x001f9e, 0x001f2e,0x000345 },
	{ 0x001f9f, 0x001f2f,0x000345 },
	{ 0x001fa0, 0x001f60,0x000345 },
	{ 0x001fa1, 0x001f61,0x000345 },
	{ 0x001fa2, 0x001f62,0x000345 },
	{ 0x001fa3, 0x001f63,0x000345 },
	{ 0x001fa4, 0x001f64,0x000345 },
	{ 0x001fa5, 0x001f65,0x000345 },
	{ 0x001fa6, 0x001f66,0x000345 },
	{ 0x001fa7, 0x001f67,0x000345 },
	{ 0x001fa8, 0x001f68,0x000345 },
	{ 0x001fa9, 0x001f69,0x000345 },
	{ 0x001faa, 0x001f6a,0x000345 },
	{ 0x001fab, 0x001f6b,0x000345 },
	{ 0x001fac, 0x001f6c,0x000345 },
	{ 0x001fad, 0x001f6d,0x000345 },
	{ 0x001fae, 0x001f6e,0x000345 },
	{ 0x001faf, 0x001f6f,0x000345 },
	{ 0x001fb0, 0x0003b1,0x000306 },
	{ 0x001fb1, 0x0003b1,0x000304 },
	{ 0x001fb2, 0x001f70,0x000345 },
	{ 0x001fb3, 0x0003b1,0x000345 },
	{ 0x001fb4, 0x0003ac,0x000345 },
	{ 0x001fb6, 0x0003b1,0x000342 },
	{ 0x001fb7, 0x001fb6,0x000345 },
	{ 0x001fb8, 0x000391,0x000306 },
	{ 0x001fb9, 0x000391,0x000304 },
	{ 0x001fba, 0x000391,0x000300 },
	{ 0x001fbc, 0x000391,0x000345 },
	{ 0x001fc1, 0x0000a8,0x000342 },
	{ 0x001fc2, 0x001f74,0x000345 },
	{ 0x001fc3, 0x0003b7,0x000345 },
	{ 0x001fc4, 0x0003ae,0x000345 },
	{ 0x001fc6, 0x0003b7,0x000342 },
	{ 0x001fc7, 0x001fc6,0x000345 },
	{ 0x001fc8, 0x000395,0x000300 },
	{ 0x001fca, 0x000397,0x000300 },
	{ 0x001fcc, 0x000397,0x000345 },
	{ 0x001fcd, 0x001fbf,0x000300 },
	{ 0x001fce, 0x001fbf,0x000301 },
	{ 0x001fcf, 0x001fbf,0x000342 },
	{ 0x001fd0, 0x0003b9,0x000306 },
	{ 0x001fd1, 0x0003b9,0x000304 },
	{ 0x001fd2, 0x0003ca,0x000300 },
	{ 0x001fd6, 0x0003b9,0x000342 },
	{ 0x001fd7, 0x0003ca,0x000342 },
	{ 0x001fd8, 0x000399,0x000306 },
	{ 0x001fd9, 0x000399,0x000304 },
	{ 0x001fda, 0x000399,0x000300 },
	{ 0x001fdd, 0x001ffe,0x000300 },
	{ 0x001fde, 0x001ffe,0x000301 },
	{ 0x001fdf, 0x001ffe,0x000342 },
	{ 0x001fe0, 0x0003c5,0x000306 },
	{ 0x001fe1, 0x0003c5,0x000304 },
	{ 0x001fe2, 0x0003cb,0x000300 },
	{ 0x001fe4, 0x0003c1,0x000313 },
	{ 0x001fe5, 0x0003c1,0x000314 },
	{ 0x001fe6, 0x0003c5,0x000342 },
	{ 0x001fe7, 0x0003cb,0x000342 },
	{ 0x001fe8, 0x0003a5,0x000306 },
	{ 0x001fe9, 0x0003a5,0x000304 },
	{ 0x001fea, 0x0003a5,0x000300 },
	{ 0x001fec, 0x0003a1,0x000314 },
	{ 0x001fed, 0x0000a8,0x000300 },
	{ 0x001ff2, 0x001f7c,0x000345 },
	{ 0x001ff3, 0x0003c9,0x000345 },
	{ 0x001ff4, 0x0003ce,0x000345 },
	{ 0x001ff6, 0x0003c9,0x000342 },
	{ 0x001ff7, 0x001ff6,0x000345 },
	{ 0x001ff8, 0x00039f,0x000300 },
	{ 0x001ffa, 0x0003a9,0x000300 },
	{ 0x001ffc, 0x0003a9,0x000345 },
	{ 0x00219a, 0x002190,0x000338 },
	{ 0x00219b, 0x002192,0x000338 },
	{ 0x0021ae, 0x002194,0x000338 },
	{ 0x0021cd, 0x0021d0,0x000338 },
	{ 0x0021ce, 0x0021d4,0x000338 },
	{ 0x0021cf, 0x0021d2,0x000338 },
	{ 0x002204, 0x002203,0x000338 },
	{ 0x002209, 0x002208,0x000338 },
	{ 0x00220c, 0x00220b,0x000338 },
	{ 0x002224, 0x002223,0x000338 },
	{ 0x002226, 0x002225,0x000338 },
	{ 0x002241, 0x00223c,0x000338 },
	{ 0x002244, 0x002243,0x000338 },
	{ 0x002247, 0x002245,0x000338 },
	{ 0x002249, 0x002248,0x000338 },
	{ 0x002260, 0x00003d,0x000338 },
	{ 0x002262, 0x002261,0x000338 },
	{ 0x00226d, 0x00224d,0x000338 },
	{ 0x00226e, 0x00003c,0x000338 },
	{ 0x00226f, 0x00003e,0x000338 },
	{ 0x002270, 0x002264,0x000338 },
	{ 0x002271, 0x002265,0x000338 },
	{ 0x002274, 0x002272,0x000338 },
	{ 0x002275, 0x002273,0x000338 },
	{ 0x002278, 0x002276,0x000338 },
	{ 0x002279, 0x002277,0x000338 },
	{ 0x002280, 0x00227a,0x000338 },
	{ 0x002281, 0x00227b,0x000338 },
	{ 0x002284, 0x002282,0x000338 },
	{ 0x002285, 0x002283,0x000338 },
	{ 0x002288, 0x002286,0x000338 },
	{ 0x002289, 0x002287,0x000338 },
	{ 0x0022ac, 0x0022a2,0x000338 },
	{ 0x0022ad, 0x0022a8,0x000338 },
	{ 0x0022ae, 0x0022a9,0x000338 },
	{ 0x0022af, 0x0022ab,0x000338 },
	{ 0x0022e0, 0x00227c,0x000338 },
	{ 0x0022e1, 0x00227d,0x000338 },
	{ 0x0022e2, 0x002291,0x000338 },
	{ 0x0022e3, 0x002292,0x000338 },
	{ 0x0022ea, 0x0022b2,0x000338 },
	{ 0x0022eb, 0x0022b3,0x000338 },
	{ 0x0022ec, 0x0022b4,0x000338 },
	{ 0x0022ed, 0x0022b5,0x000338 },
	{ 0x002adc, 0x002add,0x000338 },
	{ 0x00304c, 0x00304b,0x003099 },
	{ 0x00304e, 0x00304d,0x003099 },
	{ 0x003050, 0x00304f,0x003099 },
	{ 0x003052, 0x003051,0x003099 },
	{ 0x003054, 0x003053,0x003099 },
	{ 0x003056, 0x003055,0x003099 },
	{ 0x003058, 0x003057,0x003099 },
	{ 0x00305a, 0x003059,0x003099 },
	{ 0x00305c, 0x00305b,0x003099 },
	{ 0x00305e, 0x00305d,0x003099 },
	{ 0x003060, 0x00305f,0x003099 },
	{ 0x003062, 0x003061,0x003099 },
	{ 0x003065, 0x003064,0x003099 },
	{ 0x003067, 0x003066,0x003099 },
	{ 0x003069, 0x003068,0x003099 },
	{ 0x003070, 0x00306f,0x003099 },
	{ 0x003071, 0x00306f,0x00309a },
	{ 0x003073, 0x003072,0x003099 },
	{ 0x003074, 0x003072,0x00309a },
	{ 0x003076, 0x003075,0x003099 },
	{ 0x003077, 0x003075,0x00309a },
	{ 0x003079, 0x003078,0x003099 },
	{ 0x00307a, 0x003078,0x00309a },
	{ 0x00307c, 0x00307b,0x003099 },
	{ 0x00307d, 0x00307b,0x00309a },
	{ 0x003094, 0x003046,0x003099 },
	{ 0x00309e, 0x00309d,0x003099 },
	{ 0x0030ac, 0x0030ab,0x003099 },
	{ 0x0030ae, 0x0030ad,0x003099 },
	{ 0x0030b0, 0x0030af,0x003099 },
	{ 0x0030b2, 0x0030b1,0x003099 },
	{ 0x0030b4, 0x0030b3,0x003099 },
	{ 0x0030b6, 0x0030b5,0x003099 },
	{ 0x0030b8, 0x0030b7,0x003099 },
	{ 0x0030ba, 0x0030b9,0x003099 },
	{ 0x0030bc, 0x0030bb,0x003099 },
	{ 0x0030be, 0x0030bd,0x003099 },
	{ 0x0030c0, 0x0030bf,0x003099 },
	{ 0x0030c2, 0x0030c1,0x003099 },
	{ 0x0030c5, 0x0030c4,0x003099 },
	{ 0x0030c7, 0x0030c6,0x003099 },
	{ 0x0030c9, 0x0030c8,0x003099 },
	{ 0x0030d0, 0x0030cf,0x003099 },
	{ 0x0030d1, 0x0030cf,0x00309a },
	{ 0x0030d3, 0x0030d2,0x003099 },
	{ 0x0030d4, 0x0030d2,0x00309a },
	{ 0x0030d6, 0x0030d5,0x003099 },
	{ 0x0030d7, 0x0030d5,0x00309a },
	{ 0x0030d9, 0x0030d8,0x003099 },
	{ 0x0030da, 0x0030d8,0x00309a },
	{ 0x0030dc, 0x0030db,0x003099 },
	{ 0x0030dd, 0x0030db,0x00309a },
	{ 0x0030f4, 0x0030a6,0x003099 },
	{ 0x0030f7, 0x0030ef,0x003099 },
	{ 0x0030f8, 0x0030f0,0x003099 },
	{ 0x0030f9, 0x0030f1,0x003099 },
	{ 0x0030fa, 0x0030f2,0x003099 },
	{ 0x0030fe, 0x0030fd,0x003099 },
	{ 0x00fb1d, 0x0005d9,0x0005b4 },
	{ 0x00fb1f, 0x0005f2,0x0005b7 },
	{ 0x00fb2a, 0x0005e9,0x0005c1 },
	{ 0x00fb2b, 0x0005e9,0x0005c2 },
	{ 0x00fb2c, 0x00fb49,0x0005c1 },
	{ 0x00fb2d, 0x00fb49,0x0005c2 },
	{ 0x00fb2e, 0x0005d0,0x0005b7 },
	{ 0x00fb2f, 0x0005d0,0x0005b8 },
	{ 0x00fb30, 0x0005d0,0x0005bc },
	{ 0x00fb31, 0x0005d1,0x0005bc },
	{ 0x00fb32, 0x0005d2,0x0005bc },
	{ 0x00fb33, 0x0005d3,0x0005bc },
	{ 0x00fb34, 0x0005d4,0x0005bc },
	{ 0x00fb35, 0x0005d5,0x0005bc },
	{ 0x00fb36, 0x0005d6,0x0005bc },
	{ 0x00fb38, 0x0005d8,0x0005bc },
	{ 0x00fb39, 0x0005d9,0x0005bc },
	{ 0x00fb3a, 0x0005da,0x0005bc },
	{ 0x00fb3b, 0x0005db,0x0005bc },
	{ 0x00fb3c, 0x0005dc,0x0005bc },
	{ 0x00fb3e, 0x0005de,0x0005bc },
	{ 0x00fb40, 0x0005e0,0x0005bc },
	{ 0x00fb41, 0x0005e1,0x0005bc },
	{ 0x00fb43, 0x0005e3,0x0005bc },
	{ 0x00fb44, 0x0005e4,0x0005bc },
	{ 0x00fb46, 0x0005e6,0x0005bc },
	{ 0x00fb47, 0x0005e7,0x0005bc },
	{ 0x00fb48, 0x0005e8,0x0005bc },
	{ 0x00fb49, 0x0005e9,0x0005bc },
	{ 0x00fb4a, 0x0005ea,0x0005bc },
	{ 0x00fb4b, 0x0005d5,0x0005b9 },
	{ 0x00fb4c, 0x0005d1,0x0005bf },
	{ 0x00fb4d, 0x0005db,0x0005bf },
	{ 0x00fb4e, 0x0005e4,0x0005bf },
	{ 0x01d15e, 0x01d157,0x01d165 },
	{ 0x01d15f, 0x01d158,0x01d165 },
	{ 0x01d160, 0x01d15f,0x01d16e },
	{ 0x01d161, 0x01d15f,0x01d16f },
	{ 0x01d162, 0x01d15f,0x01d170 },
	{ 0x01d163, 0x01d15f,0x01d171 },
	{ 0x01d164, 0x01d15f,0x01d172 },
	{ 0x01d1bb, 0x01d1b9,0x01d165 },
	{ 0x01d1bc, 0x01d1ba,0x01d165 },
	{ 0x01d1bd, 0x01d1bb,0x01d16e },
	{ 0x01d1be, 0x01d1bc,0x01d16e },
	{ 0x01d1bf, 0x01d1bb,0x01d16f },
	{ 0x01d1c0, 0x01d1bc,0x01d16f },

	{0,0,0}
};

///////////////////////////////////////////////////////////////////////////////

const dcUnicodeTripel * DecomposeUnicode ( u32 code )
{
    int beg = 0;
    int end = sizeof(TableUnicodeDecomp)/sizeof(*TableUnicodeDecomp) - 2;
    while ( beg <= end )
    {
	const int pos = (beg+end)/2;
	const dcUnicodeTripel * ptr = TableUnicodeDecomp+pos;
	if ( code < ptr->code1 )
	    end = pos-1;
	else if ( code > ptr->code1 )
	    beg = pos+1;
	else
	{
	    noTRACE("DecomposeUnicode(%lx) -> %lx,%lx\n",code,ptr->code2,ptr->code3);
	    return ptr;
	}
    }
    noTRACE("DecomposeUnicode(%lx) -> NONE\n",code);
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////				END			///////////////
///////////////////////////////////////////////////////////////////////////////

