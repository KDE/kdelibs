
#ifndef __kpartmanager_h__
#define __kpartmanager_h__

#include <qobject.h>

class KPart;

class KPartManager : public QObject
{
  Q_OBJECT
public:
  KPartManager( QObject *parent = 0, const char *name = 0 );
  virtual ~KPartManager();
  
  virtual bool eventFilter( QObject *obj, QEvent *ev );
  
  virtual void addPart( KPart *part );
  virtual void removePart( KPart *part );

  KPart *activePart() { return m_activePart; }

signals:
  void activePartChanged( KPart *newPart, KPart *oldPart );

protected slots:
  void slotObjectDestroyed();
  
private:
  KPart * findPartFromWidget( QWidget * widget );
  KPart *m_activePart;
  QList<KPart> m_parts;
};

#endif

