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


#ifndef _KCARDREADER_H
#define _KCARDREADER_H

#include <kpcsc.h>
#include <qstring.h>

typedef QMemArray<unsigned char> KCardATR;


class KCardReader {
friend class KPCSC;
public:
	~KCardReader();

	bool isCardPresent();

	int beginTransaction();
	int endTransaction();
	int cancelTransaction();
	bool inTransaction();

	KCardATR getATR();

	int doCommand(QString command, QString& response);
	int doCommand(KCardCommand command, KCardCommand& response);

private:
	class KCardReaderPrivate;
	KCardReaderPrivate *d;

protected:
	KCardReader();
	void setCard(long ctx, QString name, long card, unsigned long protocol);

private:
	long _ctx;
	long _card;
	unsigned long _state;
	unsigned long _protocol;
	QString _name;
	bool _transacting;
	unsigned long _readerLen, _atrLen;
};


#endif

