
#ifndef __kpartmanager_h__
#define __kpartmanager_h__

#include <qobject.h>
#include <qwidget.h>

#include "kevent.h"

namespace KParts
{

class Part;

class PartActivateEvent : public Event
{
public:
  PartActivateEvent( bool activated ) : Event( s_strPartActivateEvent ), m_bActivated( activated ) {}

  bool activated() const { return m_bActivated; }

  static bool test( const QEvent *event ) { return Event::test( event, s_strPartActivateEvent ); }

private:
  static const char *s_strPartActivateEvent;
  bool m_bActivated;
};

/**
 * The part manager is an object which knows about all parts
 * (even nested ones) and handles activation/deactivation.
 *
 * Applications that want to embed parts without merging GUIs
 * only use a @ref KPartManager. Those who want to merge GUIs use a
 * @ref KPartsMainWindow for example, in addition to a part manager.
 *
 * Parts know about the part manager, to add nested parts to it,
 * and get access to the window caption.
 */
class PartManager : public QObject
{
  Q_OBJECT
public:
  /**
   * Create a part manager.
   *
   * @param parent The toplevel widget (window / dialog).
   */
  PartManager( QWidget * parent, const char * name = 0L );
  virtual ~PartManager();

  virtual bool eventFilter( QObject *obj, QEvent *ev );

  /**
   * Add a Part to the manager.
   *
   * Sets it to the active part automatically.
   */
  virtual void addPart( Part *part, bool setActive = true );
  /**
   * Remove a part.
   *
   * Sets the active part to 0 if @p part is the @ref activePart().
   */
  virtual void removePart( Part *part );

  /**
   * Set the active part.
   *
   * The active part receives events.
   **/
  virtual void setActivePart( Part *part );
  /**
   * Retrieve the active part.
   **/
  Part *activePart() { return m_activePart; }

  /**
   * Set the window caption.
   **/
  virtual void setWindowCaption( const QString & caption )
  { ((QWidget *)parent())->setCaption( caption ); }

  /**
   * Retrieve a list of parts managed being managed.
   **/
  const QList<Part> *parts() const { return &m_parts; }

signals:
  /**
   * Emitted when a new part has been added.
   * @see addPart()
   **/
  void partAdded( KParts::Part *part );
  /**
   * Emitted when a part has been removed.
   * @see removePart()
   **/
  void partRemoved( KParts::Part *part );
  /**
   * Emitted when the active part has changed.
   * @see setActivePart()
   **/
  void activePartChanged( KParts::Part *newPart );

protected slots:
    /**
     * Removes a part when it is destroyed.
     **/
  void slotObjectDestroyed();

private:
  Part * findPartFromWidget( QWidget * widget );
  Part * m_activePart;
  QList<Part> m_parts;
};

};

#endif

