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


#ifndef _KCARDSTATUS_H
#define _KCARDSTATUS_H

#include <kpcsc.h>
#include <kcardreader.h>
#include <winscard.h>
#include <qstring.h>
#include <qmemarray.h>

typedef QMemArray<unsigned char> KCardATR;


class KCardStatus {
public:
	KCardStatus();
	KCardStatus(const KCardStatus &x);
	KCardStatus(long _ctx, QString reader, unsigned long oldState = SCARD_STATE_UNAWARE);
	~KCardStatus();

	bool update(unsigned long timeout = 3);
	bool isPresent();
	KCardATR getATR();

	KCardStatus& operator=(const KCardStatus &y);

private:
	class KCardStatusPrivate;
	KCardStatusPrivate *d;

private:
	SCARD_READERSTATE_A _state;
	QString _name;
	long _ctx;
	bool _present;
	KCardATR _atr;
	char *_c_name;
};



#endif

