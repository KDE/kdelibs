/* This file is part of the KDE project
 *
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "ksslx509v3.h"


KSSLX509V3::KSSLX509V3() {
	_sslCAType = false;
	_emailCAType = false;
	_codeCAType = false;
}


KSSLX509V3::~KSSLX509V3() {

}


bool KSSLX509V3::certTypeSSLCA() {
	return _sslCAType;
}


bool KSSLX509V3::certTypeEmailCA() {
	return _emailCAType;
}


bool KSSLX509V3::certTypeCodeCA() {
	return _codeCAType;
}


void KSSLX509V3::setCAType(bool ssl, bool email, bool code) {
	_sslCAType = ssl;
	_emailCAType = email;
	_codeCAType = code;
}




