#ifndef __KACCELPRIVATE_H
#define __KACCELPRIVATE_H

#include "kkeyserver_x11.h"

class KAccelPrivate : public QObject, public KAccelBase
{
	Q_OBJECT
 public:
	KAccel* m_pAccel;
	int m_nIDAccelNext;
	QMap<int, int> m_mapIDToKey;
	QMap<int, KAccelAction*> m_mapIDToAction;
	QTimer m_timerShowMenu;

	KAccelPrivate( KAccel* pParent );

	virtual void setEnabled( bool );

	bool setEnabled( const QString& sAction, bool bEnable );

	virtual bool removeAction( const QString& sAction );

	virtual bool emitSignal( KAccelBase::Signal );
	virtual bool connectKey( KAccelAction&, const KKeyServer::Key& );
	virtual bool connectKey( const KKeyServer::Key& );
	virtual bool disconnectKey( KAccelAction&, const KKeyServer::Key& );
	virtual bool disconnectKey( const KKeyServer::Key& );

 signals:
	void menuItemActivated();

 private slots:
	void slotKeyPressed( int id );
	void slotShowMenu();
	void slotMenuActivated( int iAction );
};

#endif // !__KACCELPRIVATE_H
