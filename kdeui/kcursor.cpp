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
#include <kcursor.h>

#include <qbitmap.h>
#include <qcursor.h>

#include <kglobal.h>
#include <kconfig.h>

KCursor::KCursor()
{
}

QCursor KCursor::handCursor()
{
	static QCursor *hand_cursor = 0;

	if (hand_cursor == 0)
	{
		KConfig *config = KGlobal::config();
		KConfigGroupSaver saver( config, "General" );

		if ( config->readEntry("handCursorStyle", "Windows") == "Windows" )
		{
			unsigned char HAND_BITS[] = {
				0x80, 0x01, 0x00, 0x40, 0x02, 0x00, 0x40, 0x02, 0x00, 0x40, 0x02,
				0x00, 0x40, 0x02, 0x00, 0x40, 0x02, 0x00, 0x40, 0x1e, 0x00, 0x40,
				0xf2, 0x00, 0x40, 0x92, 0x01, 0x70, 0x92, 0x02, 0x50, 0x92, 0x04,
				0x48, 0x80, 0x04, 0x48, 0x00, 0x04, 0x48, 0x00, 0x04, 0x08, 0x00,
				0x04, 0x08, 0x00, 0x04, 0x10, 0x00, 0x04, 0x10, 0x00, 0x04, 0x20,
				0x00, 0x02, 0x40, 0x00, 0x02, 0x40, 0x00, 0x01, 0xc0, 0xff, 0x01};
			unsigned char HAND_MASK_BITS[] = {
				0x80, 0x01, 0x00, 0xc0, 0x03, 0x00, 0xc0, 0x03, 0x00, 0xc0, 0x03,
				0x00, 0xc0, 0x03, 0x00, 0xc0, 0x03, 0x00, 0xc0, 0x1f, 0x00, 0xc0,
				0xff, 0x00, 0xc0, 0xff, 0x01, 0xf0, 0xff, 0x03, 0xf0, 0xff, 0x07,
				0xf8, 0xff, 0x07, 0xf8, 0xff, 0x07, 0xf8, 0xff, 0x07, 0xf8, 0xff,
				0x07, 0xf8, 0xff, 0x07, 0xf0, 0xff, 0x07, 0xf0, 0xff, 0x07, 0xe0,
				0xff, 0x03, 0xc0, 0xff, 0x03, 0xc0, 0xff, 0x01, 0xc0, 0xff, 0x01};
			QBitmap hand_bitmap(22, 22, HAND_BITS, true); 
			QBitmap hand_mask(22, 22, HAND_MASK_BITS, true); 
			hand_cursor = new QCursor(hand_bitmap, hand_mask, 7, 0);
		}
		else
			hand_cursor = new QCursor(PointingHandCursor);
	}

	CHECK_PTR(hand_cursor);
	return *hand_cursor;
}

/**
 * All of the follow functions will return the Qt default for now regardless
 * of the style.  This will change at some later date
 */
QCursor KCursor::arrowCursor()
{
    return Qt::arrowCursor;
}


QCursor KCursor::upArrowCursor()
{
	return Qt::upArrowCursor;
}


QCursor KCursor::crossCursor()
{
	return Qt::crossCursor;
}


QCursor KCursor::waitCursor()
{
	return Qt::waitCursor;
}


QCursor KCursor::ibeamCursor()
{
	return Qt::ibeamCursor;
}


QCursor KCursor::sizeVerCursor()
{
	return Qt::sizeVerCursor;
}


QCursor KCursor::sizeHorCursor()
{
	return Qt::sizeHorCursor;
}


QCursor KCursor::sizeBDiagCursor()
{
	return Qt::sizeBDiagCursor;
}


QCursor KCursor::sizeFDiagCursor()
{
	return Qt::sizeFDiagCursor;
}


QCursor KCursor::sizeAllCursor()
{
	return Qt::sizeAllCursor;
}


QCursor KCursor::blankCursor()
{
	return Qt::blankCursor;
}

