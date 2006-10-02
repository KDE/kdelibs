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
    void setEnabled( bool bEnabled )
    { Q_UNUSED(bEnabled); }
    bool emitSignal( Signal signal )
    { Q_UNUSED(signal); return false; }
    bool connectKey( KAccelAction& action, const KKeyServer::Key& key)
    { Q_UNUSED(action); Q_UNUSED(key); return false; }
    bool connectKey( const KKeyServer::Key& key)
    { Q_UNUSED(key); return false; }
    bool disconnectKey( KAccelAction&, const KKeyServer::Key& key)
    { Q_UNUSED(key); return false; }
    bool disconnectKey( const KKeyServer::Key& )
    { return false; }
};

#endif // _KGLOBALACCEL_EMB_H
