#ifndef _KACCEL_H
#define _KACCEL_H

#include <qobject.h>
#include <kshortcuts.h>
#include <kstdaccel.h>

class QPopupMenu;
class QWidget;
class KAccelActions;
class KConfig;

class KAccel : public QObject
{
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

	virtual bool insertAction( const QString& sAction, const QString& sDesc,
	                 const KShortcuts& cutsDef3, const KShortcuts& cutsDef4,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 int nIDMenu = 0, QPopupMenu* pMenu = 0,
	                 bool bConfigurable = true, bool bEnabled = true );
	bool insertAction( const char* psAction, const char* psShortcuts,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 int nIDMenu = 0, QPopupMenu* pMenu = 0,
	                 bool bConfigurable = true, bool bEnabled = true );
	bool insertAction( KStdAccel::StdAccel id,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 int nIDMenu = 0, QPopupMenu* pMenu = 0,
	                 bool bConfigurable = true, bool bEnabled = true );
	// BCI: remove 'virtual's.
	virtual bool removeAction( const QString& sAction );
	bool setActionSlot( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot );
	virtual bool setActionEnabled( const QString& sAction, bool bEnabled );

	bool updateConnections();
	bool setShortcuts( const QString& sAction, const KShortcuts& );

	void readSettings( KConfig* pConfig = 0 );
	void writeSettings( KConfig* pConfig = 0 ) const;

	// Source compatibility to KDE 2.x
	virtual bool insertItem( const QString& sDesc, const QString& sAction,
	                 const char* psKey,
	                 int nIDMenu = 0, QPopupMenu* pMenu = 0, bool bConfigurable = true );
	bool insertItem( const QString& sDesc, const QString& sAction,
	                 int key,
	                 int nIDMenu = 0, QPopupMenu* pMenu = 0, bool bConfigurable = true );
	bool connectItem( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot, bool bActivate = true );
	// OBSOLETE!
	bool connectItem( KStdAccel::StdAccel accel, const QObject* pObjSlot, const char* psMethodSlot )
		{ return insertAction( accel, pObjSlot, psMethodSlot ); }
	void changeMenuAccel( QPopupMenu *menu, int id, const QString& action );
	void changeMenuAccel( QPopupMenu *menu, int id, KStdAccel::StdAccel accel );
	static int stringToKey( const QString& );

	/**
	* Retrieve the key code of the accelerator item with the action name
	* @p action, or zero if either the action name cannot be
	* found or the current key is set to no key.
	*/
	int currentKey( const QString& action ) const;

	/**
	 * Return the name of the accelerator item with the keycode @p key,
	 * or @ref QString::null if the item cannot be found.
	 */
	QString findKey( int key ) const;

	// This is temporary until Friday, when I'll make QAccel
	//  a parent class of KAccel.
	QAccel* qaccelPtr();

private:
	class KAccelPrivate* d;
};

#endif // _KACCEL_H
