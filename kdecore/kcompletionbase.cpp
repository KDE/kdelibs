/* This file is part of the KDE libraries

   Copyright (c) 2000 Dawit Alemayehu <adawit@earthlink.net>

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

#include <kstdaccel.h>
#include <kcompletion.h>


KCompletionBase::KCompletionBase()
{
    // Assign the default completion type to use.
    m_iCompletionMode = KGlobalSettings::completionMode();

    // Initialize the pointer to the completion object.
    m_pCompObj = 0;

    // By default do not emit signals.  Should be enabled
    // through member functions...
    m_bEmitSignals = false;

    // By default do not handle rotation & completion
    // signals.  Should be enabled when completion objects
    // are created or through member funtions...
    m_bHandleSignals = false;

    // Set automatic deletion of completion object
    // to true.
    m_bAutoDelCompObj = true;

    // Initialize all key-bindings to 0 by default so that
    // the event filter will use the global settings.
    m_iCompletionKey = 0;
    m_iRotateUpKey = 0;
    m_iRotateDnKey = 0;

}

KCompletionBase::~KCompletionBase()
{
    if( m_bAutoDelCompObj )
        delete m_pCompObj;
}

KCompletion* KCompletionBase::completionObject( bool hsig )
{
    if ( m_pCompObj == 0 )
        setCompletionObject( new KCompletion(), hsig );
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

bool KCompletionBase::setCompletionKey( int ckey )
{
    if( ckey >= 0 && ckey != m_iRotateDnKey && ckey != m_iRotateUpKey )
    {
        m_iCompletionKey = ckey;
        return true;
    }
    return false;
}

bool KCompletionBase::setRotateUpKey( int rUpKey )
{
    if( rUpKey >= 0 && rUpKey != m_iRotateDnKey && rUpKey != m_iCompletionKey )
    {
        m_iRotateUpKey = rUpKey;
        return true;
    }
    return false;
}

bool KCompletionBase::setRotateDownKey( int rDnKey )
{
    if ( rDnKey >= 0 && rDnKey != m_iRotateUpKey && rDnKey != m_iCompletionKey )
    {
        m_iRotateDnKey = rDnKey;
        return true;
    }
    return false;
}

void KCompletionBase::useGlobalSettings()
{
    m_iCompletionKey = 0;
    m_iRotateUpKey = 0;
    m_iRotateDnKey = 0;
}
