/* This file is part of the KDE libraries

   Copyright (c) 2000 Dawit Alemayehu <adawit@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QtCore/QMap>
#include <QtCore/QObject>

#include <kcompletion.h>

class KCompletionBasePrivate
{
public:
    KCompletionBasePrivate()
      // Assign the default completion type to use.
      : m_iCompletionMode( KGlobalSettings::completionMode() )
      , m_delegate( 0 )
    {
    }
    ~KCompletionBasePrivate()
    {
        if( m_bAutoDelCompObj && m_pCompObj )
        {
            delete m_pCompObj;
        }
    }
    // Flag that determined whether the completion object
    // should be deleted when this object is destroyed.
    bool m_bAutoDelCompObj;
    // Determines whether this widget handles completion signals
    // internally or not
    bool m_bHandleSignals;
    // Determines whether this widget fires rotation signals
    bool m_bEmitSignals;
    // Stores the completion mode locally.
    KGlobalSettings::Completion m_iCompletionMode;
    // Pointer to Completion object.
    QPointer<KCompletion> m_pCompObj;
    // Keybindings
    KCompletionBase::KeyBindingMap m_keyMap;
    // we may act as a proxy to another KCompletionBase object
    KCompletionBase *m_delegate;
};

KCompletionBase::KCompletionBase()
    : d( new KCompletionBasePrivate )
{
    // Initialize all key-bindings to 0 by default so that
    // the event filter will use the global settings.
    useGlobalKeyBindings();

    // By default we initialize everything except hsig to false.
    // All the variables would be setup properly when
    // the appropriate member functions are called.
    setup( false, true, false );
}

KCompletionBase::~KCompletionBase()
{
    delete d;
}

void KCompletionBase::setDelegate( KCompletionBase *delegate )
{
    d->m_delegate = delegate;

    if ( delegate ) {
        delegate->d->m_bAutoDelCompObj = d->m_bAutoDelCompObj;
        delegate->d->m_bHandleSignals  = d->m_bHandleSignals;
        delegate->d->m_bEmitSignals    = d->m_bEmitSignals;
        delegate->d->m_iCompletionMode = d->m_iCompletionMode;
        delegate->d->m_keyMap          = d->m_keyMap;
    }
}

KCompletionBase *KCompletionBase::delegate() const
{
    return d->m_delegate;
}

KCompletion* KCompletionBase::completionObject( bool hsig )
{
    if ( d->m_delegate )
        return d->m_delegate->completionObject( hsig );
    
    if ( !d->m_pCompObj )
    {
        setCompletionObject( new KCompletion(), hsig );
        d->m_bAutoDelCompObj = true;
    }
    return d->m_pCompObj;
}

void KCompletionBase::setCompletionObject( KCompletion* compObj, bool hsig )
{
    if ( d->m_delegate ) {
        d->m_delegate->setCompletionObject( compObj, hsig );
        return;
    }
    
    if ( d->m_bAutoDelCompObj && compObj != d->m_pCompObj )
        delete d->m_pCompObj;

    d->m_pCompObj = compObj;

    // We emit rotation and completion signals
    // if completion object is not NULL.
    setup( false, hsig, !d->m_pCompObj.isNull() );
}

// BC: Inline this function and possibly rename it to setHandleEvents??? (DA)
void KCompletionBase::setHandleSignals( bool handle )
{
    if ( d->m_delegate )
        d->m_delegate->setHandleSignals( handle );
    else
        d->m_bHandleSignals = handle;
}

bool KCompletionBase::isCompletionObjectAutoDeleted() const
{
    return d->m_delegate ? d->m_delegate->isCompletionObjectAutoDeleted()
                      : d->m_bAutoDelCompObj;
}

void KCompletionBase::setAutoDeleteCompletionObject( bool autoDelete )
{
    if ( d->m_delegate )
        d->m_delegate->setAutoDeleteCompletionObject( autoDelete );
    else
        d->m_bAutoDelCompObj = autoDelete;
}

void KCompletionBase::setEnableSignals( bool enable )
{
    if ( d->m_delegate )
        d->m_delegate->setEnableSignals( enable );
    else
        d->m_bEmitSignals = enable;
}

bool KCompletionBase::handleSignals() const
{
    return d->m_delegate ? d->m_delegate->handleSignals() : d->m_bHandleSignals;
}

bool KCompletionBase::emitSignals() const
{
    return d->m_delegate ? d->m_delegate->emitSignals() : d->m_bEmitSignals;
}

void KCompletionBase::setCompletionMode( KGlobalSettings::Completion mode )
{
    if ( d->m_delegate ) {
        d->m_delegate->setCompletionMode( mode );
        return;
    }
    
    d->m_iCompletionMode = mode;
    // Always sync up KCompletion mode with ours as long as we
    // are performing completions.
    if( d->m_pCompObj && d->m_iCompletionMode != KGlobalSettings::CompletionNone )
        d->m_pCompObj->setCompletionMode( d->m_iCompletionMode );
}

KGlobalSettings::Completion KCompletionBase::completionMode() const
{
    return d->m_delegate ? d->m_delegate->completionMode() : d->m_iCompletionMode;
}

bool KCompletionBase::setKeyBinding( KeyBindingType item, const KShortcut& cut )
{
    if ( d->m_delegate )
        return d->m_delegate->setKeyBinding( item, cut );


    if( !cut.isEmpty() )
    {
        for( KeyBindingMap::Iterator it = d->m_keyMap.begin(); it != d->m_keyMap.end(); ++it )
            if( it.value() == cut )  return false;
    }
    d->m_keyMap.insert( item, cut );
    return true;
}

KShortcut KCompletionBase::getKeyBinding( KeyBindingType item ) const
{
    return d->m_delegate ? d->m_delegate->getKeyBinding( item ) : d->m_keyMap[ item ];
}

void KCompletionBase::useGlobalKeyBindings()
{
    if ( d->m_delegate ) {
        d->m_delegate->useGlobalKeyBindings();
        return;
    }
    
    d->m_keyMap.clear();
    d->m_keyMap.insert( TextCompletion, KShortcut() );
    d->m_keyMap.insert( PrevCompletionMatch, KShortcut() );
    d->m_keyMap.insert( NextCompletionMatch, KShortcut() );
    d->m_keyMap.insert( SubstringCompletion, KShortcut() );
}

KCompletion* KCompletionBase::compObj() const
{
    return d->m_delegate ? d->m_delegate->compObj()
                      : static_cast<KCompletion*>(d->m_pCompObj);
}

KCompletionBase::KeyBindingMap KCompletionBase::getKeyBindings() const
{
    return d->m_delegate ? d->m_delegate->getKeyBindings() : d->m_keyMap;
}

void KCompletionBase::setup( bool autodel, bool hsig, bool esig )
{
    if ( d->m_delegate ) {
        d->m_delegate->setup( autodel, hsig, esig );
        return;
    }
    
    d->m_bAutoDelCompObj = autodel;
    d->m_bHandleSignals = hsig;
    d->m_bEmitSignals = esig;
}

void KCompletionBase::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

