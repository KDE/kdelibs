#ifndef _KGLOBALACCEL_H_
#define _KGLOBALACCEL_H_

#include <qobject.h>
#include <kshortcuts.h>

class QPopupMenu;
class QWidget;
class KAccelActions;
class KConfig;

class KGlobalAccel : public QObject
{
	friend class KGlobalAccelPrivate;
 public:
	KGlobalAccel( QObject* pParent, const char* psName = 0 );
	virtual ~KGlobalAccel();

	class KAccelBase* basePtr();
	KAccelActions& actions();
	virtual bool insertAction( const QString& sAction, const QString& sDesc,
	                 const KShortcuts& cutsDef3, const KShortcuts& cutsDef4,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 int nIDMenu = 0, QPopupMenu* pMenu = 0, bool bConfigurable = true );
	bool insertAction( const QString& sAction, KShortcuts cutsDef,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 int nIDMenu = 0, QPopupMenu* pMenu = 0, bool bConfigurable = true );
	virtual bool insertLabel( const QString& sName, const QString& sDesc );

	bool updateConnections();
	bool setShortcuts( const QString& sAction, const KShortcuts& );

	void readSettings( KConfig* pConfig = 0 );
	void writeSettings( KConfig* pConfig = 0 ) const;

	// Functions which mimic QAccel somewhat:
	//virtual bool insertItem( const QString& sDesc, const QString& sAction,
	//                 KShortcuts rgCutDefaults3,
	//                 int nIDMenu = 0, QPopupMenu* pMenu = 0, bool bConfigurable = true );

 protected:
	// Attempts to make a passive X server grab/ungrab of the specified key.
	//  Return true if successful.
	// Modifications with NumLock, CapsLock, ScrollLock, and ModeSwitch are
	//  also grabbed.
	bool grabKey( const QString& action, bool bGrab );

 private:
	class KGlobalAccelPrivate* d;

//protected:
//	KAccelActions m_rgActions;
//	int aAvailableId;
//	bool bEnabled;
//	QString aGroup;
//	bool do_not_grab;
//	KGlobalAccelPrivate* d;

 public:
	// Setting this to false shuts off processing of KeyPress events in
	//  x11EventFilter(). It will still be called, but won't act on them.
	// This is a more effecient means for briefly suspending processing
	//  than setEnabled(false) ... setEnabled(true).
	// These functions should be implemented in kglobalaccel_x11/emb.cpp
	static void setKeyEventsEnabled( bool enabled );
	static bool areKeyEventsEnabled();
};

#endif // _KGLOBALACCEL_H_
