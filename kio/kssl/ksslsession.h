/* This file is part of the KDE project
 *
 * Copyright (C) 2003 George Staikos <staikos@kde.org>
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

#ifndef _KSSLSESSION_H
#define _KSSLSESSION_H

class KSSL;

class KSSLSessionPrivate;

/**
 * KDE SSL Session Information
 *
 * This class contains data about an SSL session.
 * It is just used as a container that is taken from or passed to KSSL objects.
 *
 * @author George Staikos <staikos@kde.org>
 * @see KSSL
 * @short KDE SSL Session Information
 */
class KSSLSession {
friend class KSSL;
public:
	/**
	 *  Destroy this instance
	 */
	~KSSLSession();

protected:
	KSSLSession();

	void *_session;

private:
	KSSLSessionPrivate *d;
};


#endif

