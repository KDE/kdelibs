/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)

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
//-----------------------------------------------------------------------------
// KDE color selection combo box

// layout managment added Oct 1997 by Mario Weilguni 
// <mweilguni@sime.com>


#ifndef _KCOLORCOMBO_H__
#define _KCOLORCOMBO_H__

#include <qcombobox.h>
#include <kcolordlg.h>
#include "kselect.h"


/**
 * Combobox for colours.
 */
class KColorCombo : public QComboBox
{
	Q_OBJECT
public:
	KColorCombo( QWidget *parent, const char *name = 0L );

	void setColor( const QColor &col );
	/** 
	 * Retrieve the currently selected color. 
	 **/
	QColor color();
public slots:
	void slotActivated( int index );
	void slotHighlighted( int index );

signals:
	void activated( const QColor &col );
	void highlighted( const QColor &col );

protected:
        /**
         * @reimplemented
         */
	virtual void resizeEvent( QResizeEvent *re );

private:
	void addColors();
	QColor customColor;
	QColor internalcolor;

    class KColorComboPrivate;
    KColorComboPrivate *d;
};

#endif	// __KCOLORCOMBO_H__
