#ifndef _KGLOBALACCEL_MAC_H
#define _KGLOBALACCEL_MAC_H

#include <qwidget.h>

#include "kshortcut.h"
#include "kaccelbase.h"

class KGlobalAccelPrivate: public KAccelBase
{
public:
    KGlobalAccelPrivate()
        : KAccelBase(KAccelBase::NATIVE_KEYS)
    {}

    // reimplemented pure virtuals
    void setEnabled( bool )
    {}
    bool emitSignal( Signal )
    { return false; }
    bool connectKey( KAccelAction&, const KKeyServer::Key& )
    { return false; }
    bool connectKey( const KKeyServer::Key& )
    { return false; }
    bool disconnectKey( KAccelAction&, const KKeyServer::Key& )
    { return false; }
    bool disconnectKey( const KKeyServer::Key& )
    { return false; }
    void disableBlocking( bool disable ) {}
    static void blockShortcuts( bool block ) {}
};

#endif // _KGLOBALACCEL_EMB_H
