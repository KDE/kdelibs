/*
	libvcard - vCard parsing library for vCard version 3.0
	
	Copyright (C) 1999 Rik Hemsley rik@kde.org
	
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

#ifndef  ENUM_H
#define  ENUM_H

#include <qcstring.h>

namespace VCARD
{

extern const QCString paramNames [];

enum EntityType {
	EntityName,
	EntityProfile,
	EntitySource,
	EntityFullName,
	EntityN,
	EntityNickname,
	EntityPhoto,
	EntityBirthday,
	EntityAddress,
	EntityLabel,
	EntityTelephone,
	EntityEmail,
	EntityMailer,
	EntityTimeZone,
	EntityGeo,
	EntityTitle,
	EntityRole,
	EntityLogo,
	EntityAgent,
	EntityOrganisation,
	EntityCategories,
	EntityNote,
	EntityProductID,
	EntityRevision,
	EntitySortString,
	EntitySound,
	EntityUID,
	EntityURL,
	EntityVersion,
	EntityClass,
	EntityKey,
	EntityExtension,
	EntityUnknown
};

enum ValueType {
	ValueSound,
	ValueAgent,
	ValueAddress,
	ValueTel,
	ValueTextBin,
	ValueOrg,
	ValueN,
	ValueUTC,
	ValueURI,
	ValueClass,
	ValueFloat,
	ValueImage,
	ValueDate,
	ValueTextList,
	ValueText,
	ValueGeo,
	ValueUnknown
};

enum ParamType {
	ParamUnknown,
	ParamNone,
	ParamSource,
	ParamText,
	ParamImage,
	ParamDate,
	ParamAddrText,
	ParamTel,
	ParamEmail,
	ParamMailer,
	ParamAgent,
	ParamTextBin,
	ParamTextNS,
	ParamSound
};

extern const ParamType paramTypesTable[];

ParamType EntityTypeToParamType(EntityType);
ValueType EntityTypeToValueType(EntityType);
QCString EntityTypeToParamName(EntityType);
EntityType EntityNameToEntityType(const QCString &);

char * encodeBase64(const char *, unsigned long, unsigned long &);
char * decodeBase64(const char *, unsigned long, unsigned long &);

}

#endif

