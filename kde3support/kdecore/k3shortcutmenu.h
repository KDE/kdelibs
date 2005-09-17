/*  This file is part of the KDE libraries
    Copyright (C) 2002 Ellis Whitehead <ellis@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef __K3SHORTCUTMENU_H
#define __K3SHORTCUTMENU_H

#include <qmap.h>
#include <q3popupmenu.h>

#include "k3keysequence.h"

class KAccelActions;

/**
 * @internal
 */
class KDE3SUPPORT_EXPORT K3ShortcutMenu : public Q3PopupMenu
{
	Q_OBJECT
 public:
	K3ShortcutMenu( QWidget* pParent, KAccelActions* pActions, K3KeySequence seq );

	bool insertAction( uint iAction, K3KeySequence seq );

	void updateShortcuts();
 
 protected:
	void keyPressEvent( QKeyEvent* pEvent );

 private:
	int searchForKey( KKey key );
	void keepItemsMatching( KKey key );
 
 private:
	typedef QMap<uint, K3KeySequence> IndexToK3KeySequence;
	
	KAccelActions* m_pActions;
	K3KeySequence m_seq;
	IndexToK3KeySequence m_seqs;
};

#endif // __K3ShortcutMenu_H
