/*
    Copyright (c) 2002 Ellis Whitehead <ellis@kde.org>

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

#include <qkeysequence.h>

#include <QKeyEvent>

#include "kaccelaction.h"
#include <kdebug.h>
#include <kglobalsettings.h>
#include "kshortcutmenu.h"
//#include <kkeynative.h>

KShortcutMenu::KShortcutMenu( QWidget* pParent, KAccelActions* pActions, KKeySequence seq )
:	QMenu( pParent ),
	m_pActions( pActions ),
	m_seq( seq )
{
	kDebug() << seq.toStringInternal() << endl;
}

bool KShortcutMenu::insertAction( uint iAction, KKeySequence seq )
{
	KAccelAction* pAction = m_pActions->actionPtr( iAction );
	
	if( pAction ) {
		insertItem( "", iAction );
		m_seqs[indexOf(iAction)] = seq;
		return true;
	} else
		return false;
}


void KShortcutMenu::updateShortcuts()
{
	setTitle( m_seq.toString() + ",..." );
	
	for( uint iItem = 0; iItem < actions().count(); iItem++ ) {
		int iAction = idAt( iItem );
		if( iAction >= 0 ) {
			KAccelAction* pAction = m_pActions->actionPtr( iAction );
			if( pAction ) {
				KKeySequence seq = m_seqs[iItem];
				QString sSeq = seq.key(m_seq.count()).toString();
				for( uint iKey = m_seq.count() + 1; iKey < seq.count(); iKey++ )
					sSeq += QString(",") + seq.key(iKey).toString();

				kDebug(125) << "seq = " << seq.toStringInternal() << " sSeq = " << sSeq << endl;
				changeItem( iAction, pAction->label() + "\t" + sSeq );
			}
		}
	}
}

void KShortcutMenu::keyPressEvent( QKeyEvent* pEvent )
{
	kDebug() << "keypress; " << pEvent->key() << endl;
	KKey key( pEvent );
	
	switch( pEvent->key() ) {
	 case Qt::Key_Shift:
	 case Qt::Key_Control:
	 case Qt::Key_Alt:
	 case Qt::Key_Meta:
	 case Qt::Key_Super_L:
	 case Qt::Key_Super_R:
	 case Qt::Key_Hyper_L:
	 case Qt::Key_Hyper_R:
		break;
	 default:
		int iItem = searchForKey( key );
		// If key not found, look for unmodified version.
		if( iItem == -1 ) {
			key = pEvent->key();
			iItem = searchForKey( key );
		}
			
		if( iItem == -1 ) {
			// Let Up and Down keys navigate menu,
			// And permit Enter, Return to select the item.
			if( pEvent->key() == Qt::Key_Up    || pEvent->key() == Qt::Key_Down ||
			    pEvent->key() == Qt::Key_Enter || pEvent->key() == Qt::Key_Return )
				QMenu::keyPressEvent( pEvent );
			else
				close();
		}
		else if( iItem == 0 )
			keepItemsMatching( key );
		else
			activateItemAt( iItem );
	}
}

int KShortcutMenu::searchForKey( KKey key )
{
	int iItemFound = -1; // -1 indicates no match
	uint iKey = m_seq.count();
	
	for( uint iItem = 0; iItem < actions().count(); iItem++ ) {
		if( m_seqs.contains( iItem ) ) {
			KKey keyItem = m_seqs[iItem].key( iKey );
			//kDebug(125) << "iItem = " << iItem << " key = " << key.toStringInternal() << " keyItem = " << keyItem.toStringInternal() << endl;
			if( key == keyItem ) {
				if( iItemFound == -1 )
					iItemFound = iItem;
				else
					return 0; // 0 indicates duplicate matches
			}
		}
	}
	
	return iItemFound;
}

void KShortcutMenu::keepItemsMatching( KKey key )
{
	kDebug(125) << "MyAccel::keepItemsMatching( " << key.toStringInternal() << " )" << endl;
	
	uint iKey = m_seq.count();
	m_seq.setKey( iKey, key );
	
	for( uint iItem = 0; iItem < actions().count(); iItem++ ) {
		if( m_seqs.contains( iItem ) ) {
			KKey keyItem = m_seqs[iItem].key( iKey );
			if( key != keyItem ) {
				m_seqs.remove( iItem );
				removeItemAt( iItem-- );
			}
		}
	}
	
	updateShortcuts();
}

#include "kshortcutmenu.moc"
