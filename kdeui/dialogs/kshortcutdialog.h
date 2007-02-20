/* This file is part of the KDE libraries
    Copyright (C) 2002,2003 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
    Copyright (C) 2007 Roberto Raggi <roberto@kdevelop.org>

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

#include "kdialog.h"

class KShortcut;
class KShortcutDialogPrivate;

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
	explicit KShortcutDialog(const KShortcut &shortcut, QWidget *parent = 0);
	~KShortcutDialog();

	void setShortcut(const KShortcut &shortcut);
	const KShortcut& shortcut() const;

protected:
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void keyReleaseEvent(QKeyEvent *event);

private:
	Q_PRIVATE_SLOT(d, void slotButtonClicked(KDialog::ButtonCode code))
	Q_PRIVATE_SLOT(d, void slotSelectPrimary())
	Q_PRIVATE_SLOT(d, void slotSelectAlternate())
	Q_PRIVATE_SLOT(d, void slotClearShortcut())
	Q_PRIVATE_SLOT(d, void slotClearPrimary())
	Q_PRIVATE_SLOT(d, void slotClearAlternate())
	Q_PRIVATE_SLOT(d, void slotMultiKeyMode(bool bOn))

private:
	friend class KShortcutDialogPrivate;
	KShortcutDialogPrivate* const d;
        
        Q_DISABLE_COPY(KShortcutDialog)
};

#endif // _KSHORTCUTDIALOG_H_
