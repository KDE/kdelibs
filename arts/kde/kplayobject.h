    /*

    Copyright (C) 2001 Nikolas Zimmermann <wildfox@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Permission is also granted to link this program with the Qt
    library, treating Qt like a library that normally accompanies the
    operating system kernel, whether or not that is in fact the case.

    */

#ifndef KPLAYOBJECT_H
#define KPLAYOBJECT_H

#include <kmedia2.h>
#include <qobject.h>

class KPlayObject : private QObject
{
    Q_OBJECT
    public:
	KPlayObject();
	KPlayObject(Arts::PlayObject playobject, bool isStream);
	~KPlayObject();

	void setObject(Arts::PlayObject playObject);
	Arts::PlayObject object();

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
