/* This file is part of the KDE libraries
    Copyright (C) 2002,2003 Ellis Whitehead <ellis@kde.org>

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

#ifndef _KSHORTCUTDIALOG_H_
#define _KSHORTCUTDIALOG_H_

#include "kdialogbase.h"
#include "kshortcut.h"

class KVBox;
class KPushButton;
class KShortcutDialogSimple;
class KShortcutDialogAdvanced;

/**
 * @short Dialog for configuring a shortcut.
 *
 * This dialog allows configuring a single KShortcut. KKeyDialog
 * should be usually used instead.
 *
 * @internal
 * @see KKeyDialog
 * @since 3.4
 */
class KDEUI_EXPORT KShortcutDialog : public KDialog
{
	Q_OBJECT
public:
	KShortcutDialog( const KShortcut& shortcut, bool bQtShortcut, QWidget* parent = 0 );
	~KShortcutDialog();

	void setShortcut( const KShortcut & shortcut );
	const KShortcut& shortcut() const { return m_shortcut; }

private:
	// true if qt shortcut, false if native shortcut
	bool m_bQtShortcut;

	KShortcut m_shortcut;
	bool m_bGrab;
	KPushButton* m_ptxtCurrent;
	uint m_iSeq;
	uint m_iKey;
	bool m_bRecording;
	uint m_mod;
	KShortcutDialogSimple *m_simple;
	KShortcutDialogAdvanced *m_adv;
	KVBox *m_stack;
	
	void updateShortcutDisplay();
	//void displayMods();
	void keyPressed( KKey key );
	void updateDetails();

	#ifdef Q_WS_X11
	virtual bool x11Event( XEvent *pEvent );
	//void x11EventKeyPress( XEvent *pEvent );
	void x11KeyPressEvent( XEvent* pEvent );
	void x11KeyReleaseEvent( XEvent* pEvent );
	#endif
	#ifdef Q_WS_WIN
	virtual void keyPressEvent( QKeyEvent * e );
	virtual bool event(QEvent * e);
	#endif

protected slots:
	void slotDetails();
	void slotSelectPrimary();
	void slotSelectAlternate();
	void slotClearShortcut();
	void slotClearPrimary();
	void slotClearAlternate();
	void slotMultiKeyMode( bool bOn );

private:
        class KShortcutDialogPrivate* d;
        static bool s_showMore;
};

#endif // _KSHORTCUTDIALOG_H_
