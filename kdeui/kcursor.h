/* This file is part of the KDE libraries
   Copyright (C) 1998 Kurt Granroth (granroth@kde.org)

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
/*
 * $Id$
 *
 * $Log$
 * Revision 1.1  1998/11/28 06:30:59  granroth
 * Added KCursor -- a simple wrapper around QCursor allowing for "themable"
 * cursors.  Currently, it only supports a 'handCursor'.
 *
 */
#ifndef _KCURSOR_H
#define _KCURSOR_H

class QCursor;

/**
 * A wrapper around QCursor that allows for "themed" cursors.
 *
 * Currently, the only themed cursor is a 'hand' cursor.
 *
 * A typical usage would be
 * <PRE>
 * 	setCursor(KCursor::handCursor());
 * </PRE>
 *
 * @short A QCursor wrapper allowing "themed" cursors.
 * @author Kurt Granroth <granroth@kde.org>
 */
class KCursor
{
public:
	/**
	 * Constructor.  Does not do anything so far
	 */
	KCursor();

	/**
	 * Static function returning the proper hand cursor according to
	 * the current GUI style.
	 */
	static QCursor handCursor();

	/**
	 * Static function returning the proper arrow cursor according to
	 * the current GUI style.
	 */
	static QCursor arrowCursor();

	/**
	 * Static function returning the proper up arrow cursor according to
	 * the current GUI style.
	 */
	static QCursor upArrowCursor();

	/**
	 * Static function returning the proper cross-hair cursor according to
	 * the current GUI style.
	 */
	static QCursor crossCursor();

	/**
	 * Static function returning the proper hourglass cursor according to
	 * the current GUI style.
	 */
	static QCursor waitCursor();

	/**
	 * Static function returning the proper text cursor according to
	 * the current GUI style.
	 */
	static QCursor ibeamCursor();

	/**
	 * Static function returning the proper vertical resize cursor
	 * according to the current GUI style.
	 */
	static QCursor sizeVerCursor();

	/**
	 * Static function returning the proper horizontal resize cursor
	 * according to the current GUI style.
	 */
	static QCursor sizeHorCursor();

	/**
	 * Static function returning the proper diagonal resize (/) cursor
	 * according to the current GUI style.
	 */
	static QCursor sizeBDiagCursor();

	/**
	 * Static function returning the proper diagonal resize (\) cursor
	 * according to the current GUI style.
	 */
	static QCursor sizeFDiagCursor();

	/**
	 * Static function returning the proper all directions resize cursor
	 * according to the current GUI style.
	 */
	static QCursor sizeAllCursor();

	/**
	 * Static function returning a blank or invisible cursor
	 */
	static QCursor blankCursor();
};
#endif // _KCURSOR_H
