#ifndef __KACCELPRIVATE_H
#define __KACCELPRIVATE_H

class KAccelPrivate : public QObject, public KAccelBase
{
	Q_OBJECT
 public:
	KAccel* m_pAccel;
	int m_nIDAccelNext;
	QMap<int, KKey> m_mapIDToSpec;
	QTimer m_timerShowMenu;

	KAccelPrivate( KAccel* pParent );

	virtual void setEnabled( bool );

	virtual bool removeAction( const QString& sAction );

	virtual bool emitSignal( KAccelBase::Signal );
	virtual bool connectKey( KAccelAction&, const KKey& );
	virtual bool connectKey( const KKey& );
	virtual bool disconnectKey( KAccelAction&, const KKey& );
	virtual bool disconnectKey( const KKey& );

	void setAutoUpdateTemp( bool b ) { m_bAutoUpdate = b; }

 signals:
	void menuItemActivated();

 private slots:
	void slotKeyPressed( int id );
	void slotShowMenu();
	void slotMenuActivated( int iAction );
};

#endif // !__KACCELPRIVATE_H
