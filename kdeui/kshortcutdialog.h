/* This file is part of the KDE libraries
    Copyright (C) 2002,2003 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KSHORTCUTDIALOG_H
#define KSHORTCUTDIALOG_H

#include <kshortcut.h>

#include "kdialog.h"

class QStackedWidget;
class KPushButton;

/**
 * @short Dialog for configuring a shortcut.
 *
 * This dialog allows configuring a single KShortcut. KKeyDialog
 * should be usually used instead.
 *
 * @internal
 * @see KKeyDialog
 */
class KDEUI_EXPORT KShortcutDialog : public KDialog
{
	Q_OBJECT
public:
	KShortcutDialog( const KShortcut& shortcut, QWidget* parent = 0 );
	~KShortcutDialog();

	void setShortcut( const KShortcut & shortcut );
	const KShortcut& shortcut() const;

private:
	void updateShortcutDisplay();
	void keyPressed( int key );
	void updateDetails();
	void setRecording(bool recording);

	virtual void keyPressEvent( QKeyEvent * e );
	virtual void keyReleaseEvent( QKeyEvent * event );

protected Q_SLOTS:
	void slotButtonClicked(KDialog::ButtonCode code);
	void slotSelectPrimary();
	void slotSelectAlternate();
	void slotClearShortcut();
	void slotClearPrimary();
	void slotClearAlternate();
	void slotMultiKeyMode( bool bOn );

private:
	class KShortcutDialogPrivate* const d;
	static bool s_showMore;
};

#endif // _KSHORTCUTDIALOG_H_
