/*
	libvcard - vCard parsing library for vCard version 3.0

	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <qcstring.h>
#include <ctype.h>

#include <VCardEnum.h>

using namespace VCARD;

// There are 31 possible types, not including extensions.
	const QCString
VCARD::paramNames [] =
{
	"NAME",
	"PROFILE",
	"SOURCE",
	"FN",
	"N",
	"NICKNAME",
	"PHOTO",
	"BDAY",
	"ADR",
	"LABEL",
	"TEL",
	"EMAIL",
	"MAILER",
	"TZ",
	"GEO",
	"TITLE",
	"ROLE",
	"LOGO",
	"AGENT",
	"ORG",
	"CATEGORIES",
	"NOTE",
	"PRODID",
	"REV",
	"SORT-STRING",
	"SOUND",
	"UID",
	"URL",
	"VERSION",
	"CLASS",
	"KEY"
};

	const ParamType
VCARD::paramTypesTable[] = {
	ParamNone,		// NAME
	ParamNone,		// PROFILE
	ParamSource,		// SOURCE
	ParamText,		// FN
	ParamText,		// N
	ParamText,		// NICKNAME
	ParamImage,		// PHOTO (inline/refer)
	ParamDate,		// BDAY ("VALUE = "date-time/date)
	ParamAddrText,		// ADR (adr-param/text-param)
	ParamAddrText,		// LABEL (adr-param/text-param)
	ParamTel,		// TEL
	ParamEmail,		// EMAIL
	ParamText,		// MAILER
	ParamNone,		// TZ
	ParamNone,		// GEO
	ParamText,		// TITLE
	ParamText,		// ROLE
	ParamImage,		// LOGO
	ParamAgent,		// AGENT
	ParamText,		// ORG
	ParamText,		// CATEGORIES
	ParamText,		// NOTE
	ParamNone,		// PRODID
	ParamDate,		// REV
	ParamText,		// SORT-STRING
	ParamSound,		// SOUND
	ParamNone,		// UID
	ParamNone,		// URL
	ParamNone,		// VERSION
	ParamNone,		// CLASS
	ParamTextBin,		// KEY
	ParamTextNS		// X
};

	ParamType
VCARD::EntityTypeToParamType(EntityType e)
{
	ParamType t(ParamUnknown);

	switch (e) {
	
	//---------------------------------------------------------------//	
		case EntityAgent:		t = ParamAgent;		break;
	//---------------------------------------------------------------//	
		case EntitySound:		t = ParamSound;		break;
	//---------------------------------------------------------------//	
		case EntitySource:		t = ParamSource;	break;
	//---------------------------------------------------------------//	
		case EntityTelephone:		t = ParamTel;		break;
	//---------------------------------------------------------------//	
		case EntityEmail:		t = ParamEmail;		break;
	//---------------------------------------------------------------//	
		case EntityKey:			t = ParamTextBin;	break;
	//---------------------------------------------------------------//	
		case EntityExtension:		t = ParamTextNS;	break;
	//---------------------------------------------------------------//	
		case EntityAddress:
		case EntityLabel:		t = ParamAddrText;	break;
	//---------------------------------------------------------------//	
		case EntityBirthday:
		case EntityRevision:		t = ParamDate;		break;
	//---------------------------------------------------------------//	
		case EntityPhoto:
		case EntityLogo:		t = ParamImage;		break;
	//---------------------------------------------------------------//	
		case EntityOrganisation:
		case EntityTitle:
		case EntityRole:
		case EntityFullName:
		case EntityMailer:
		case EntityN:
		case EntitySortString:
		case EntityNickname:
		case EntityCategories:
		case EntityNote:		t = ParamText;		break;
	//---------------------------------------------------------------//		
		case EntityProductID:
		case EntityTimeZone:
		case EntityUID:
		case EntityURL:
		case EntityClass:
		case EntityGeo:
		case EntityName:
		case EntityVersion:
		case EntityProfile:
		default:			t = ParamNone;		break;
	//---------------------------------------------------------------//	

	}
	
	return t;
}

	ValueType
VCARD::EntityTypeToValueType(EntityType e)
{
	ValueType t(ValueUnknown);

	switch (e) {
	
	//---------------------------------------------------------------//	
		case EntitySound:		t = ValueSound;		break;
	//---------------------------------------------------------------//	
		case EntityAgent:		t = ValueAgent;		break;
	//---------------------------------------------------------------//	
		case EntityAddress:		t = ValueAddress;	break;
	//---------------------------------------------------------------//	
		case EntityTelephone:		t = ValueTel;		break;
	//---------------------------------------------------------------//	
		case EntityKey:			t = ValueTextBin;	break;
	//---------------------------------------------------------------//	
		case EntityOrganisation:	t = ValueOrg;		break;
	//---------------------------------------------------------------//	
		case EntityN:			t = ValueN;		break;
	//---------------------------------------------------------------//	
		case EntityTimeZone:		t = ValueUTC;		break;
	//---------------------------------------------------------------//		
		case EntityClass:		t = ValueClass;		break;
	//---------------------------------------------------------------//		
		case EntityGeo:			t = ValueGeo;		break;
	//---------------------------------------------------------------//		
		case EntitySource:
		case EntityURL:			t = ValueURI;		break;
	//---------------------------------------------------------------//		
		case EntityPhoto:
		case EntityLogo:		t = ValueImage;		break;
	//---------------------------------------------------------------//	
		case EntityBirthday:
		case EntityRevision:		t = ValueDate;		break;
	//---------------------------------------------------------------//	
		case EntityCategories:
		case EntityNickname:		t = ValueTextList;	break;
	//---------------------------------------------------------------//	
		case EntityLabel:
		case EntityExtension:
		case EntityEmail:
		case EntityTitle:
		case EntityRole:
		case EntityFullName:
		case EntityMailer:
		case EntityProductID:
		case EntityName:
		case EntitySortString:
		case EntityVersion:
		case EntityProfile:
		case EntityUID:
		case EntityNote:
		default:			t = ValueText;		break;
	//---------------------------------------------------------------//	

	}
	
	return t;
}

	QCString
VCARD::EntityTypeToParamName(EntityType e)
{
	if ( e > EntityUnknown ) e = EntityUnknown;
	return paramNames[ int( e ) ];
}

	EntityType
VCARD::EntityNameToEntityType(const QCString & s)
{
	if (s.isEmpty()) return EntityUnknown;
	
	EntityType t(EntityUnknown);
	
	switch (s[0]) {

		case 'A':
			if (s == "ADR")
				t = EntityAddress;
			else if (s == "AGENT")
				t = EntityAgent;
			break;

		case 'B':
			if (s == "BDAY")
				t = EntityBirthday;
			break;

		case 'C':
			if (s == "CATEGORIES")
				t = EntityCategories;
			else if (s == "CLASS")
				t = EntityClass;
			break;

		case 'E':
			if (s == "EMAIL")
				t = EntityEmail;
			break;

		case 'F':
			if (s == "FN")
				t = EntityFullName;
			break;

		case 'G':
			if (s == "GEO")
				t = EntityGeo;
			break;

		case 'K':
			if (s == "KEY")
				t = EntityKey;
			break;

		case 'L':
			if (s == "LABEL")
				t = EntityLabel;
			else if (s == "LOGO")
				t = EntityLogo;
			break;

		case 'M':
			if (s == "MAILER")
				t = EntityMailer;
			break;
			
		case 'N':
			if (s == "N")
				t = EntityN;
			else if (s == "NAME")
				t = EntityName;
			else if (s == "NICKNAME")
				t = EntityNickname;
			else if (s == "NOTE")
				t = EntityNote;
			break;

		case 'O':
			if (s == "ORG")
				t = EntityOrganisation;
			break;

		case 'P':
			if (s == "PHOTO")
				t = EntityPhoto;
			else if (s == "PRODID")
				t = EntityProductID;
			else if (s == "PROFILE")
				t = EntityProfile;
			break;
		
		case 'R':
			if (s == "REV")
				t = EntityRevision;
			else if (s == "ROLE")
				t = EntityRole;
			break;
			
		case 'S':
			if (s == "SORT-STRING")
				t = EntitySortString;
			else if (s == "SOUND")
				t = EntitySound;
			else if (s == "SOURCE")
				t = EntitySource;
			break;

		case 'T':
			if (s == "TEL")
				t = EntityTelephone;
			else if (s == "TITLE")
				t = EntityTitle;
			else if (s == "TZ")
				t = EntityTimeZone;
			break;

		case 'U':
			if (s == "UID")
				t = EntityUID;
			else if (s == "URL")
				t = EntityURL;
		case 'V':
			if (s == "VERSION")
				t = EntityVersion;
			break;

		case 'X':
			if (s.left(2) == "X-")
				t = EntityExtension;
			break;
			
		default:
			
			t = EntityUnknown;
	}
	
	return t;
}

// The copyright notice below refers to the base64 codec functions used below,
// which are modified from the original sources.

/*
 * Original version Copyright 1988 by The Leland Stanford Junior University
 * Copyright 1998 by the University of Washington
 *
 *  Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notices appear in all copies and that both the
 * above copyright notices and this permission notice appear in supporting
 * documentation, and that the name of the University of Washington or The
 * Leland Stanford Junior University not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written prior
 * permission.  This software is made available "as is", and
 * THE UNIVERSITY OF WASHINGTON AND THE LELAND STANFORD JUNIOR UNIVERSITY
 * DISCLAIM ALL WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD TO THIS SOFTWARE,
 * INCLUDING WITHOUT LIMITATION ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE UNIVERSITY OF
 * WASHINGTON OR THE LELAND STANFORD JUNIOR UNIVERSITY BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, TORT (INCLUDING NEGLIGENCE) OR STRICT LIABILITY, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

static char B64[] = 
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// the mime base64 disctionary used for decoding
static signed char b64dec[] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 10
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 20
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 30
	-1, -1, -1,-19, -1, -1, -1,-16, -4, -4, // 40 -19 == '+' -16 == '/'
	-4, -4, -4, -4, -4, -4, -4, -4, -1, -1, // 50 -4 == '0'
	-1,  0, -1, -1, -1, 65, 65, 65, 65, 65, // 60 0 == '=' 65 == 'A'
	65, 65, 65, 65, 65, 65, 65, 65, 65, 65, // 70
	65, 65, 65, 65, 65, 65, 65, 65, 65, 65, // 80
	65, -1, -1, -1, -1, -1, -1, 71, 71, 71, // 90 71 == 'a'
	71, 71, 71, 71, 71, 71, 71, 71, 71, 71, // 100
	71, 71, 71, 71, 71, 71, 71, 71, 71, 71, // 110
	71, 71, 71, -1, -1, -1, -1, -1, -1, -1, // 120
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 130
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 140
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 150
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 160
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 170
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 180
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 190
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 200
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 210
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 220
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 230
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 240
	-1, -1, -1, -1, -1,	-1, -1				// 250
};

	char *
VCARD::decodeBase64(const char * s, unsigned long srcl, unsigned long & len)
{
	register char c;
	register unsigned long e(0);
	len = 0;
	unsigned const char * src = (unsigned const char *)s;
	char * ret = new char[srcl + (srcl / 4 + 1)];
	register char *d = ret;
	while (srcl--) { // Critical loop
		c = *src++;
		int dec = b64dec[c];
		if (dec == -1) continue;
		if (c == '=') {
			switch (e++) {
				case 3: e = 0;								break;
				case 2: if (*src == '=')					break;
				default: delete [] ret; ret = 0; return 0;	break;
			}
			continue;
		}
		c -= dec;
		if (e == 0) { *d = c << 2; ++e; continue; }
		switch (e) {
			case 1: *d |= c >> 4; *++d = c << 4;	break;
			case 2: *d |= c >> 2; *++d = c << 6;	break;
			case 3: *d++ |= c; e = 0; continue;		break;
		}
		++e;
	}
	len = d - (char *)ret;
	return ret;
}


	char *
VCARD::encodeBase64(const char * src, unsigned long srcl, unsigned long & destl)
{
	register const unsigned char *s = (unsigned char *)src;
	register unsigned long i = ((srcl + 2) / 3) * 4;
	destl = i += 2 * ((i / 60) + 1);
	i = 0;
	char * ret = new char[destl];
	register unsigned char *d((unsigned char *)ret);
	while (srcl != 0) { // Critical loop
		*d++ = B64[s[0] >> 2];
		*d++ = B64[((s[0] << 4) + (--srcl == 0 ? 0 : s[1] >> 4)) & 0x3f];
		*d++ = srcl == 0 ? '=' :
			B64[((s[1] << 2) + (--srcl == 0 ? 0 : s[2] >> 6)) & 0x3f];
		*d++ = srcl == 0 ?	'=' : B64[s[2] & 0x3f];
		if (srcl != 0) srcl--;
		if (++i == 15) { i = 0; *d++ = '\r'; *d++ = '\n'; }
		s += 3;
	}
	*d = '\r'; *++d = '\n'; *++d = '\0';
	return ret;
}

