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

#ifndef  ADRPARAM_H
#define  ADRPARAM_H

#include <qcstring.h>
#include <qstrlist.h>

#include <VCardParam.h>

namespace VCARD
{
	
class AdrParam : public Param
{

#include "AdrParam-generated.h"
	
	QStrList adrTypeList()
	{ parse(); return adrTypeList_; }
	
	QCString textParam()
	{ parse(); return textParam_; }
	
	void setAdrTypeList(const QStrList & l)
	{ adrTypeList_ = l; assembled_ = false; }
	
	void setTextParam(const QCString & s)
	{ textParam_ = s; assembled_ = false; }
	
	enum AdrType {
		AdrDom, AdrIntl, AdrPostal, AdrParcel, AdrHome, AdrWork, AdrPref,
		AdrIANA, AdrX
	};
	
	private:
		
		QStrList	adrTypeList_;
		QCString	textParam_;
};
}

#endif
