#ifndef _KGLOBALACCEL_X11_H
#define _KGLOBALACCEL_X11_H

#include <qmap.h>
#include <qwidget.h>

#include "kaccelbase.h"
#include "kshortcut.h"

class CodeMod
{
 public:
	uchar code;
	uint mod;

	bool operator < ( const CodeMod& b ) const
	{
		if( code < b.code ) return true;
		if( code == b.code && mod < b.mod ) return true;
		return false;
	}
};
typedef QMap<CodeMod, KAccelAction*> CodeModMap;

class KGlobalAccelPrivate : public QWidget, public KAccelBase
{
	friend class KGlobalAccel;
	Q_OBJECT
 public:
	KGlobalAccelPrivate();

	virtual void setEnabled( bool );

	virtual bool emitSignal( Signal );
	virtual bool connectKey( KAccelAction&, const KKey& );
	virtual bool connectKey( const KKey& );
	virtual bool disconnectKey( KAccelAction&, const KKey& );
	virtual bool disconnectKey( const KKey& );

 signals:
	void activated();
	void activated( int );
	void activated( const QString& sAction, const QString& sDesc, const KKeySequence& seq );

 protected:
	CodeModMap m_rgCodeModToAction;

	/**
	 * @param bGrab Set to true to grab key, false to ungrab key.
	 */
	bool grabKey( const KKey&, bool bGrab, KAccelAction* );

	/**
	 * Filters X11 events ev for key bindings in the accelerator dictionary.
	 * If a match is found the activated activated is emitted and the function
	 * returns true. Return false if the event is not processed.
	 *
	 * This is public for compatibility only. You do not need to call it.
	 */
	virtual bool x11Event( XEvent* );
	void x11MappingNotify();
	bool x11KeyPress( const XEvent *pEvent );
	void activate( KAccelAction* pAction, const KKeySequence& seq );

 protected slots:
	void slotActivated( int iAction );
};

#endif // _KGLOBALACCEL_X11_H
