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


#ifndef _KSSLKEYGEN_H
#define _KSSLKEYGEN_H

#include <qstring.h>
#include <qstringlist.h>
#include <kdialog.h>


class KOpenSSLProxy;

class KSSLKeyGen : public KDialog {
	Q_OBJECT
 
public:
	KSSLKeyGen(QWidget *parent=0, const char *name=0, bool modal=false);
	virtual ~KSSLKeyGen();

	// Static methods for various tasks
	static QStringList supportedKeySizes();
	static int generateCSR(QString name, QString pass, int bits, int e = 0x10001);

	// this is an index into supportedKeySizes();
	void setKeySize(int idx) { _idx = idx; }

private:
 
	class KSSLKeyGenPrivate;
	KSSLKeyGenPrivate *d;
	int _idx;
	KOpenSSLProxy *kossl;
 
protected:

};

#endif

