
#ifndef __kpartmanager_h__
#define __kpartmanager_h__

#include <qobject.h>
#include <qwidget.h>
#include <qlist.h>

#include <kparts/event.h>

namespace KParts
{

class Part;

class PartActivateEvent : public Event
{
public:
  PartActivateEvent( bool activated, Part *part, QWidget *widget ) : Event( s_strPartActivateEvent ), m_bActivated( activated ), m_part( part ), m_widget( widget ) {}

  bool activated() const { return m_bActivated; }

  Part *part() const { return m_part; }
  QWidget *widget() const { return m_widget; }

  static bool test( const QEvent *event ) { return Event::test( event, s_strPartActivateEvent ); }

private:
  static const char *s_strPartActivateEvent;
  bool m_bActivated;
  Part *m_part;
  QWidget *m_widget;
};

class PartSelectEvent : public Event
{
public:
  PartSelectEvent( bool selected, Part *part, QWidget *widget ) : Event( s_strPartSelectEvent ), m_bSelected( selected ), m_part( part ), m_widget( widget ) {}

  bool selected() const { return m_bSelected; }

  Part *part() const { return m_part; }
  QWidget *widget() const { return m_widget; }

  static bool test( const QEvent *event ) { return Event::test( event, s_strPartSelectEvent ); }

private:
  static const char *s_strPartSelectEvent;
  bool m_bSelected;
  Part *m_part;
  QWidget *m_widget;
};

class PartManagerPrivate;

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
  // the default policy of a PartManager is Direct!
  enum SelectionPolicy { Direct, TriState };

  /**
   * Create a part manager.
   *
   * @param parent The toplevel widget (window / dialog).
   */
  PartManager( QWidget * parent, const char * name = 0L );
  virtual ~PartManager();

  void setSelectionPolicy( SelectionPolicy policy );
  SelectionPolicy selectionPolicy() const;

  /**
   * Specify whether the partmanager should handle/allow nested parts or not.
   * This is a property the shell has to set/specify. Per default we assume that the
   * shell cannot handle nested parts. However in case of a KOffice shell for example
   * we allow nested parts.
   * A Part is nested (a child part) if its parent object inherits KParts::Part.
   * If a child part is activated and nested parts are not allowed/handled, then the top parent
   * part in the tree is activated.
   */
  void setAllowNestedParts( bool allow );
  bool allowNestedParts() const;

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
  virtual void setActivePart( Part *part, QWidget *widget = 0L );
  /**
   * Retrieve the active part.
   **/
  virtual Part *activePart() const;

  virtual QWidget *activeWidget() const;

  virtual void setSelectedPart( Part *part, QWidget *widget = 0L );

  virtual Part *selectedPart() const;

  virtual QWidget *selectedWidget() const;

  /**
   * Set the window caption.
   **/
  virtual void setWindowCaption( const QString & caption )
  { ((QWidget *)parent())->setCaption( caption ); }

  /**
   * Retrieve a list of parts managed being managed.
   **/
  const QList<Part> *parts() const;

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

  void slotWidgetDestroyed();

private:
  Part * findPartFromWidget( QWidget * widget, const QPoint &pos );

  PartManagerPrivate *d;
};

};

#endif

