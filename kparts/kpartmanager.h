
#ifndef __kpartmanager_h__
#define __kpartmanager_h__

#include <qobject.h>
#include <qwidget.h>

class KPart;

/**
 * The part manager is an object which knows about all parts
 * (even nested ones) and handles activation/deactivation.
 *
 * Applications that want to embed parts without merging GUIs
 * only use a KPartManager. Those who want to merge GUIs use a
 * KPartsMainWindow for example, in addition to a part manager.
 *
 * Parts know about the part manager, to add nested parts to it,
 * and to get access to the window caption
 */
class KPartManager : public QObject
{
  Q_OBJECT
public:
  /**
   * Create a part manager
   * @param parent the toplevel widget (window / dialog...)
   */
  KPartManager( QWidget * parent, const char * name = 0L );
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

  virtual void setWindowCaption( const QString & caption )
  { ((QWidget *)parent())->setCaption( caption ); }

  const QList<KPart> *parts() const { return &m_parts; }

signals:
  void partAdded( KPart *part );
  void partRemoved( KPart *part );
  void activePartChanged( KPart *newPart );

protected slots:
  void slotObjectDestroyed();

private:
  KPart * findPartFromWidget( QWidget * widget );
  KPart * m_activePart;
  QList<KPart> m_parts;
};

#endif

