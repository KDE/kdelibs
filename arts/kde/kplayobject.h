	/*

	Copyright (C) 2001 Nikolas Zimmermann <wildfox@kde.org>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.
  
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public License
	along with this library; see the file COPYING.LIB.  If not, write to
	the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	Boston, MA 02111-1307, USA.

	*/

#ifndef KPLAYOBJECT_H
#define KPLAYOBJECT_H

#include <kmedia2.h>
#include <qobject.h>

class KPlayObject : public QObject
{
Q_OBJECT
public:
	KPlayObject();
	KPlayObject(Arts::PlayObject playobject, bool isStream);
	~KPlayObject();

	void setObject(Arts::PlayObject playObject);
	Arts::PlayObject object();

	/**
	 * return true if both this!=0, and object.isNull()
	 *
	 * in essence, ((KPlayObject*)0)->isNull() will not 
	 * crash
	 **/
	bool isNull();

	bool stream();

	void play();
	void seek(Arts::poTime newTime);
	void pause();
	void halt();
	
	QString description();
	Arts::poTime currentTime();
	Arts::poTime overallTime();
	Arts::poCapabilities capabilities();
	QString mediaName();
	Arts::poState state();

private:
	Arts::PlayObject m_playObject;
	bool m_isStream;
};

#endif
