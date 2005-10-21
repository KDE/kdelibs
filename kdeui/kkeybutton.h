/* This file is part of the KDE libraries
    Copyright (C) 2001, 2002 Ellis Whitehead <ellis@kde.org>

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
 * @short A push button that looks like a keyboard key.
 *
 * You must call setShortcut() to set the widget's currently displayed key.
 * You can call captureShortcut() to get a new shortcut from the user.
 * If captureShortcut() succeeds, then the capturedShortcut() signal will be
 * emitted with the value of the new shortcut.  The widget containing
 * a KKeyButton widget must connect to this signal and check if the shortcut
 * is valid.  If it is, you will need to call setShortcut() with the new
 * value in order make it the key currently displayed.
 *
 * @author Mark Donohoe <donohoe@kde.org>
 * @internal
 */
class KDEUI_EXPORT KKeyButton: public QPushButton
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

	/** @deprecated Use setShortcut( cut, false ) instead */
	void setShortcut( const KShortcut& cut ) KDE_DEPRECATED;
	/// @since 3.1
	void setShortcut( const KShortcut& cut, bool bQtShortcut );
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
	 * If it succeeds, the capturedShortcut() signal will be emitted.
	 */
	void captureShortcut();

 protected:
	KShortcut m_cut;
	bool m_bEditing;

	/**
	* Reimplemented for internal reasons.
	*/
	void paintEvent( QPaintEvent* pe );

 protected:
	virtual void virtual_hook( int id, void* data );
 private:
	class KKeyButtonPrivate* d;
};

#endif
