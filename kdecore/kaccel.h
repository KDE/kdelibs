#ifndef _KACCEL_H
#define _KACCEL_H

#include <qaccel.h>
#include <kshortcuts.h>
#include <kstdaccel.h>

class QPopupMenu;
class QWidget;
class KAccelAction;
class KAccelActions;
class KConfig;

class KAccel : public QAccel
{
	Q_OBJECT
 public:
	KAccel( QWidget* pParent, const char* psName = 0 );
	virtual ~KAccel();

	class KAccelBase* basePtr();
	KAccelActions& actions();

	bool isEnabled();
	void setEnabled( bool bEnabled );

	bool getAutoUpdate();
	// return value of AutoUpdate flag before this call.
	bool setAutoUpdate( bool bAuto );

	KAccelAction* insertAction( const QString& sAction, const QString& sDesc,
	                 const KShortcuts& cutsDef3, const KShortcuts& cutsDef4,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 int nIDMenu = 0, QPopupMenu* pMenu = 0,
	                 bool bConfigurable = true, bool bEnabled = true );
	KAccelAction* insertAction( const QString& sAction, const QString& sDesc,
	                 const QString& cutsDef,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 int nIDMenu = 0, QPopupMenu* pMenu = 0,
	                 bool bConfigurable = true, bool bEnabled = true );
	KAccelAction* insertAction( const QString& sAction, const QString& sDesc,
	                 KKeySequence cutsDef,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 int nIDMenu = 0, QPopupMenu* pMenu = 0,
	                 bool bConfigurable = true, bool bEnabled = true );
	KAccelAction* insertAction( const char* psAction, const char* psShortcuts,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 int nIDMenu = 0, QPopupMenu* pMenu = 0,
	                 bool bConfigurable = true, bool bEnabled = true );
	KAccelAction* insertAction( KStdAccel::StdAccel id,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 int nIDMenu = 0, QPopupMenu* pMenu = 0,
	                 bool bConfigurable = true, bool bEnabled = true );

	bool removeAction( const QString& sAction );
	bool setActionSlot( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot );
	bool setActionEnabled( const QString& sAction, bool bEnabled );

	bool updateConnections();
	bool setShortcuts( const QString& sAction, const KShortcuts& );

	void readSettings( KConfig* pConfig = 0 );
	void writeSettings( KConfig* pConfig = 0 ) const;
	void setConfigGroup( const QString& );

	void emitKeycodeChanged();

 signals:
	void keycodeChanged();

 public:
	// Source compatibility to KDE 2.x
	bool insertItem( const QString& sDesc, const QString& sAction,
	                 const char* psKey,
	                 int nIDMenu = 0, QPopupMenu* pMenu = 0, bool bConfigurable = true );
	bool insertItem( const QString& sDesc, const QString& sAction,
	                 int key,
	                 int nIDMenu = 0, QPopupMenu* pMenu = 0, bool bConfigurable = true );
	bool insertStdItem( KStdAccel::StdAccel id, const QString& descr = QString::null );
	bool connectItem( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot, bool bActivate = true );
	// OBSOLETE!
	bool connectItem( KStdAccel::StdAccel accel, const QObject* pObjSlot, const char* psMethodSlot )
		{ return insertAction( accel, pObjSlot, psMethodSlot ); }
	bool removeItem( const QString& sAction );
	bool setItemEnabled( const QString& sAction, bool bEnable );
	void changeMenuAccel( QPopupMenu *menu, int id, const QString& action );
	void changeMenuAccel( QPopupMenu *menu, int id, KStdAccel::StdAccel accel );
	static int stringToKey( const QString& );

	/**
	 * Obsolete.
	 * Retrieve the key code of the accelerator item with the action name
	 * @p action, or zero if either the action name cannot be
	 * found or the current key is set to no key.
	 */
	int currentKey( const QString& action ) const;

	/**
	 * Obsolete.
	 * Return the name of the accelerator item with the keycode @p key,
	 * or @ref QString::null if the item cannot be found.
	 */
	QString findKey( int key ) const;

	// This is temporary until Friday, when I'll make QAccel
	//  a parent class of KAccel.  Where did I put this? -- ellis
	QAccel* qaccelPtr();

 private:
	class KAccelPrivate* d;
};

#endif // _KACCEL_H
