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

    // Initialize the pointer to the completion object.
    m_pCompObj = 0;

    // Initialize the popup menu
    m_pCompletionMenu = 0;

    // Set completion ID to -1 by default
    m_iCompletionID = -1;

    // Do not show the mode changer item
    // by default.
    m_bShowModeChanger = false;

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
    if( m_bAutoDelCompObj )
    {
        delete m_pCompObj;
        m_pCompObj = 0; // Prevent SEGFAULT on double deletions :)
    }
    delete m_pCompletionMenu;
}

KCompletion* KCompletionBase::completionObject( bool hsig )
{
    if ( m_pCompObj == 0 )
    {
        setCompletionObject( new KCompletion(), hsig );
        // Set automatic deletion of completion object to true
        // since it was internally created...
        m_bAutoDelCompObj = true;
    }
    return m_pCompObj;
}

void KCompletionBase::setCompletionObject( KCompletion* compObj, bool hsig )
{
    m_pCompObj = compObj;
    m_bAutoDelCompObj = false;
    setHandleSignals( hsig );
    m_bEmitSignals = ( m_pCompObj != 0 ); // emit signals if comp object exists
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
    if( m_pCompObj != 0 && m_iCompletionMode != KGlobalSettings::CompletionNone )
        m_pCompObj->setCompletionMode( m_iCompletionMode );
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

void KCompletionBase::insertCompletionItems( QObject* parent, const char* member )
{
    if( parent )
    {
        m_pCompletionMenu->clear();
        m_pCompletionMenu->insertItem( i18n("None"), parent, member, 0, KGlobalSettings::CompletionNone );
        m_pCompletionMenu->setItemChecked( KGlobalSettings::CompletionNone, m_iCompletionMode == KGlobalSettings::CompletionNone );
        m_pCompletionMenu->insertItem( i18n("Manual"), parent, member, 0, KGlobalSettings::CompletionShell );
        m_pCompletionMenu->setItemChecked( KGlobalSettings::CompletionShell, m_iCompletionMode == KGlobalSettings::CompletionShell );
        m_pCompletionMenu->insertItem( i18n("Automatic"), parent, member, 0, KGlobalSettings::CompletionAuto );
        m_pCompletionMenu->setItemChecked( KGlobalSettings::CompletionAuto, m_iCompletionMode == KGlobalSettings::CompletionAuto );
        m_pCompletionMenu->insertItem( i18n("Semi-Automatic"), parent, member, 0, KGlobalSettings::CompletionMan );
        m_pCompletionMenu->setItemChecked( KGlobalSettings::CompletionMan, m_iCompletionMode == KGlobalSettings::CompletionMan );
        if( m_iCompletionMode != KGlobalSettings::completionMode() )
        {
            m_pCompletionMenu->insertSeparator();
            m_pCompletionMenu->insertItem( i18n("Default"), parent, member, 0, 0 );
        }
    }
}

void KCompletionBase::insertCompletionMenu( QObject* receiver, const char* member, QPopupMenu* parent, int index )
{
    if( m_bShowModeChanger && m_iCompletionID == -1 && parent != 0 && m_pCompObj != 0 )
    {
        if( m_pCompletionMenu == 0 )
            m_pCompletionMenu = new QPopupMenu(); //Create the popup menu if not present
        parent->insertSeparator( index > 0 ? index - 1 : index );
        m_iCompletionID = parent->insertItem( i18n("Completion"), m_pCompletionMenu, -1, index );
        QObject::connect( m_pCompletionMenu, SIGNAL( aboutToShow() ), receiver, member );
    }
    else if( !m_bShowModeChanger && m_iCompletionID != -1 && parent != 0 )
    {
        parent->removeItem( m_iCompletionID );
        m_iCompletionID = -1;
    }
}
