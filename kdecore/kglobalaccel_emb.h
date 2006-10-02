#ifndef _KGLOBALACCEL_EMB_H
#define _KGLOBALACCEL_EMB_H

#include "kaccelbase.h"
#include "kshortcut.h"

class KGlobalAccelPrivate
{
public:
	KGlobalAccelPrivate();

	virtual void setEnabled( bool bEnabled );

	virtual bool connectKey( KAccelAction&, KKeySequence );
	virtual bool disconnectKey( KAccelAction&, KKeySequence );
};

#endif // _KGLOBALACCEL_EMB_H
