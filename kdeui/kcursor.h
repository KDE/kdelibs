/* This file is part of the KDE libraries
   Copyright (C) 1998 Kurt Granroth (granroth@kde.org)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

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
 */
#ifndef _KCURSOR_H
#define _KCURSOR_H

class QCursor;

/**
 * A wrapper around @ref QCursor that allows for "themed" cursors.
 *
 * Currently, the only themed cursor is a hand shaped cursor.
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
	 * Constructor.  
	 *
	 * Does not do anything so far.
	 **/
	KCursor();

	/**
	 * Retrieve the proper hand cursor according to
	 * the current GUI style (static function).
	 */
	static QCursor handCursor();

	/**
	 * Retrieve the proper arrow cursor according to
	 * the current GUI style (static function).
	 */
	static QCursor arrowCursor();

	/**
	 * Retrieve the proper up arrow cursor according to
	 * the current GUI style (static function).
	 */
	static QCursor upArrowCursor();

	/**
	 * Retrieve the proper cross-hair cursor according to
	 * the current GUI style (static function).
	 */
	static QCursor crossCursor();

	/**
	 * Retrieve the proper hourglass cursor according to
	 * the current GUI style (static function).
	 */
	static QCursor waitCursor();

	/**
	 * Retrieve the proper text cursor according to
	 * the current GUI style (static function).
	 */
	static QCursor ibeamCursor();

	/**
	 * Retrieve the proper vertical resize cursor
	 * according to the current GUI style (static function).
	 */
	static QCursor sizeVerCursor();

	/**
	 * Retrieve the proper horizontal resize cursor
	 * according to the current GUI style (static function).
	 */
	static QCursor sizeHorCursor();

	/**
	 * Retrieve the proper diagonal resize (/) cursor
	 * according to the current GUI style (static function).
	 */
	static QCursor sizeBDiagCursor();

	/**
	 * Retrieve the proper diagonal resize (\) cursor
	 * according to the current GUI style (static function).
	 */
	static QCursor sizeFDiagCursor();

	/**
	 * Retrieve the proper all-directions resize cursor
	 * according to the current GUI style (static function).
	 */
	static QCursor sizeAllCursor();

	/**
	 * Retrieve a blank or invisible cursor (static function).
	 */
	static QCursor blankCursor();
};
#endif // _KCURSOR_H
