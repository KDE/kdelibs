
#ifndef __kpartmanager_h__
#define __kpartmanager_h__

#include <qobject.h>

class KPart;

class KPartManager : public QObject
{
  Q_OBJECT
public:
  KPartManager( QObject * parent = 0L, const char * name = 0L );
  virtual ~KPartManager();

  virtual bool eventFilter( QObject *obj, QEvent *ev );

  /**
   * Call this to add a Part to the manager.
   * Sets it to the active part automatically.
   */
  virtual void addPart( KPart *part );
  /**
   * Call this to remove a part
   *
   * Sets the active part to 0 if @p part is the @ref activePart.
   */
  virtual void removePart( KPart *part );

  KPart *activePart() { return m_activePart; }

signals:
  void activePartChanged( KPart *newPart );

protected slots:
  void slotObjectDestroyed();

private:
  KPart * findPartFromWidget( QWidget * widget );
  KPart * m_activePart;
  QList<KPart> m_parts;
};

#endif

