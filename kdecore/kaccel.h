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
	//bool insertAction( const QString& sAction, KShortcuts rgCutDefaults,
	//                 const QObject* pObjSlot, const char* psMethodSlot,
	//                 int nIDMenu = 0, QPopupMenu* pMenu = 0,
	//		 bool bConfigurable = true, bool bEnabled = true );
	bool insertAction( KStdAccel::StdAccel id,
	                 const QObject* pObjSlot, const char* psMethodSlot,
	                 int nIDMenu = 0, QPopupMenu* pMenu = 0,
			 bool bConfigurable = true, bool bEnabled = true );
	virtual bool removeAction( const QString& sAction );
	virtual bool setActionEnabled( const QString& sAction, bool bEnabled );

	bool updateConnections();
	bool setShortcuts( const QString& sAction, const KShortcuts& );

	void readSettings( KConfig* pConfig = 0 );
	void writeSettings( KConfig* pConfig = 0 ) const;

	// Functions which mimic QAccel somewhat:
	virtual bool insertItem( const QString& sDesc, const QString& sAction,
	                 KShortcuts rgCutDefaults3,
	                 int nIDMenu = 0, QPopupMenu* pMenu = 0, bool bConfigurable = true );
	bool connectItem( const QString& sAction, const QObject* pObjSlot, const char* psMethodSlot );

private:
	class KAccelPrivate* d;
};

#endif // _KACCEL_H
