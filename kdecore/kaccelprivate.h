#ifndef __KACCELPRIVATE_H
#define __KACCELPRIVATE_H

#include "kkeyserver_x11.h"
#include <qtimer.h>

class KAccelAction;

/**
 * @internal
 */
class KDECORE_EXPORT KAccelPrivate : public QObject, public KAccelBase
{
	Q_OBJECT
 public:
	KAccel* m_pAccel;
	QWidget* m_pWatch;
	QMap<int, int> m_mapIDToKey;
	QMap<int, KAccelAction*> m_mapIDToAction;
	QTimer m_timerShowMenu;

	KAccelPrivate( KAccel* pParent, QWidget* pWatch );

	virtual void setEnabled( bool bEnabled );

	bool setEnabled( const QString& sAction, bool bEnable );

	virtual bool removeAction( const QString& sAction );

	virtual bool emitSignal( KAccelBase::Signal signal );
	virtual bool connectKey( KAccelAction& action, const KKeyServer::Key& key );
	virtual bool connectKey( const KKeyServer::Key& key );
	virtual bool disconnectKey( KAccelAction& action, const KKeyServer::Key& key );
	virtual bool disconnectKey( const KKeyServer::Key& key );

 signals:
	void menuItemActivated();
	void menuItemActivated(KAccelAction*);

 private:
#ifndef Q_WS_WIN /** @todo TEMP: new implementation (commit #424926) didn't work */
	void emitActivatedSignal(KAccelAction*);
#endif

 private slots:
	void slotKeyPressed( int id );
	void slotShowMenu();
	void slotMenuActivated( int iAction );
	
	bool eventFilter( QObject* pWatched, QEvent* pEvent ); // virtual method from QObject
};

#endif // !__KACCELPRIVATE_H
