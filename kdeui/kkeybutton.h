/* This file is part of the KDE libraries
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>

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

#ifndef _KKEYBUTTON_H_
#define _KKEYBUTTON_H_

#include <qpushbutton.h>
#include <kshortcut.h>

/**
 *  A push button that looks like a keyboard key.
 *  @short A push button that looks like a keyboard key.
 *  @author Mark Donohoe <donohoe@kde.org>
 *  @internal
 */
class KKeyButton: public QPushButton
{
	Q_OBJECT

 public:
	/**
	* Constructs  key button widget.
	*/
	KKeyButton( QWidget *parent = 0, const char *name = 0 );
	/**
	* Destructs the key button widget.
	*/
	virtual ~KKeyButton();

	void setShortcut( const KShortcut& cut );
	const KShortcut& shortcut() const
		{ return m_cut; }

	/**
	* Reimplemented for internal purposes.
	*/
	void setText( const QString& text );

 signals:
	void capturedShortcut( const KShortcut& );

 public slots:
	/**
	 * Call this method to capture a shortcut from the keyboard.
	 * If it succeeds, the @ref capturedShortcut() will be emitted.
	 */
	void captureShortcut();

 protected:
	KShortcut m_cut;
	bool m_bEditing;

	/**
	* Reimplemented for internal reasons.
	*/
	void drawButton( QPainter* _painter );

 private:
	class KKeyButtonPrivate* d;
};


#endif
