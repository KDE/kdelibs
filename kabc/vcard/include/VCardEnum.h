/*
	libvcard - vCard parsing library for vCard version 3.0
	
	Copyright (C) 1999 Rik Hemsley rik@kde.org
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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

