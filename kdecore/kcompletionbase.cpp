/* This file is part of the KDE libraries

   Copyright (c) 2000 Dawit Alemayehu <adawit@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qpopupmenu.h>

#include <kstdaccel.h>
#include <kcompletion.h>
#include <klocale.h>

KCompletionBase::KCompletionBase()
{
    // Assign the default completion type to use.
    m_iCompletionMode = KGlobalSettings::completionMode();

    // By default do not emit signals.  Should be
    // enabled through member functions...
    m_bEmitSignals = false;

    // Initialize the auto delete flag to false.  This
    // will be changed accordingly when the completion
    // object is created through either completionObject
    // or setCompletionObject member functions.
    m_bAutoDelCompObj = false;

    // By default do not handle rotation & completion
    // signals.  Will be enabled as needed when completion
    // objects are created or through member funtions.
    m_bHandleSignals = false;

    // Initialize all key-bindings to 0 by default so that
    // the event filter will use the global settings.
    useGlobalKeyBindings();
}

KCompletionBase::~KCompletionBase()
{
    if( m_bAutoDelCompObj && m_pCompObj)
    {
        delete m_pCompObj;
    }    
}

KCompletion* KCompletionBase::completionObject( bool hsig )
{
    if ( !m_pCompObj )
    {
        setCompletionObject( new KCompletion(), hsig );
        // Set automatic deletion of completion object
        // to true since it was internally created...
        m_bAutoDelCompObj = true;
    }
    return m_pCompObj;
}

void KCompletionBase::setCompletionObject( KCompletion* compObj, bool hsig )
{
    m_pCompObj = compObj;
    m_bAutoDelCompObj = false;
    setHandleSignals( hsig );
    m_bEmitSignals = ( !m_pCompObj.isNull() ); // emit signals if comp object exists
}

void KCompletionBase::setHandleSignals( bool handle )
{
    connectSignals( handle );
    m_bHandleSignals = handle;
}

void KCompletionBase::setCompletionMode( KGlobalSettings::Completion mode )
{
    m_iCompletionMode = mode;
    // Always sync up KCompletion mode with ours as long as we
    // are performing completions.
    if(  m_pCompObj &&
    	 m_iCompletionMode != KGlobalSettings::CompletionNone )
    {
        m_pCompObj->setCompletionMode( m_iCompletionMode );
    }
}

bool KCompletionBase::setKeyBinding( KeyBindingType item, int key )
{
    if( key >= 0 )
    {
        if( key > 0 )
        {
            for( KeyBindingMap::Iterator it = m_keyMap.begin(); it != m_keyMap.end(); ++it )
                if( it.data() == key )  return false;
        }
        m_keyMap.replace( item, key );
        return true;
    }
    return false;
}

void KCompletionBase::useGlobalKeyBindings()
{
	m_keyMap.clear();
	m_keyMap.insert( TextCompletion, 0 );
	m_keyMap.insert( PrevCompletionMatch, 0 );
	m_keyMap.insert( NextCompletionMatch, 0 );
	m_keyMap.insert( RotateUp, 0 );
	m_keyMap.insert( RotateDown, 0 );
}

void KCompletionBase::insertDefaultMenuItems( QPopupMenu* popup ) const
{
        popup->insertItem( i18n( "Cut" ), KCompletionBase::Cut );
        popup->insertItem( i18n( "Copy" ), KCompletionBase::Copy );
        popup->insertItem( i18n( "Clear" ), KCompletionBase::Clear );        
        popup->insertItem( i18n( "Paste" ), KCompletionBase::Paste );     
        insertCompletionMenuItem( popup );
        popup->insertSeparator();
        popup->insertItem( i18n( "Unselect" ), KCompletionBase::Unselect );
        popup->insertItem( i18n( "Select All" ), KCompletionBase::SelectAll );
}

void KCompletionBase::insertCompletionMenuItem( QPopupMenu* popup, int index ) const
{
    if( index < -1 && index >= (int)popup->count() )
        return;
    // Create and insert the completion sub-menu iff
    // a completion object is present.
    if( m_pCompObj )
    {        
        QPopupMenu* subMenu = new QPopupMenu( popup );
        subMenu->insertItem( i18n("None"), KCompletionBase::NoCompletion );
        subMenu->setItemChecked( KCompletionBase::NoCompletion, m_iCompletionMode == KGlobalSettings::CompletionNone );
        subMenu->insertItem( i18n("Manual"), KCompletionBase::ShellCompletion );
        subMenu->setItemChecked( KCompletionBase::ShellCompletion, m_iCompletionMode == KGlobalSettings::CompletionShell );
        subMenu->insertItem( i18n("Automatic"), KCompletionBase::AutoCompletion );
        subMenu->setItemChecked( KCompletionBase::AutoCompletion, m_iCompletionMode == KGlobalSettings::CompletionAuto );
        subMenu->insertItem( i18n("Semi-Automatic"), KCompletionBase::SemiAutoCompletion );
        subMenu->setItemChecked( KCompletionBase::SemiAutoCompletion, m_iCompletionMode == KGlobalSettings::CompletionMan );
        if( m_iCompletionMode != KGlobalSettings::completionMode() )
        {
            subMenu->insertSeparator();
            subMenu->insertItem( i18n("Default"), KCompletionBase::Default );
        }
        int id = popup->insertItem( i18n("Completion"), subMenu, -1, index );
        popup->insertSeparator( popup->indexOf( id ) );
    }
}
