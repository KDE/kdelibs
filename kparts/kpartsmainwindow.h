#ifndef __KGUIBUILDER_H
#define __KGUIBUILDER_H

#include <qlist.h>
#include <qaction.h>

#include <ktmainwindow.h>
#include <ktoolbar.h>
#include <kmenubar.h>

#include "kxmlgui.h"
#include "kpart.h"

class QPopupMenu;
class QString;
class KStatusBar;

/**
 * @short a KPart-aware main window, whose user interface is described in XML
 *
 * Inherit your main window from this class.
 *
 * It implements all internal interfaces in the case of a KTMainWindow as host:
 * the builder and servant interface (for menu merging)
 */
class KPartsMainWindow : public KTMainWindow, public KXMLGUIBuilder, public KXMLGUIServant
{
  Q_OBJECT
 public:
  KPartsMainWindow( const char *name = 0L, WFlags f = WDestructiveClose );
  virtual ~KPartsMainWindow();

  //
  virtual void setXMLFile( const QString &file );
  //
  virtual void setXML( const QString &document );

  // Add your actions to this collection
  QActionCollection *actionCollection() { return &m_actionCollection; }

  // ---

  // KXMLGUIServant interface (internal)
  virtual QAction *action( const QDomElement &element );
  virtual QDomDocument document() { QDomDocument doc; doc.setContent( m_xml ); return doc; }

  // KXMLGUIBuilder interface (internal)
  virtual QWidget *createContainer( QObject *parent, const QDomElement &element );

protected slots:

  /**
   * Create the GUI (by merging the host's and the active part's)
   * Called on startup and whenever the active part changes
   * For this you need to connect this slot to the
   * KPartManager::activePartChanged signal
   * @param part the active part (set to 0L if no part)
   */
  virtual void createGUI( KPart * part );

protected:
  virtual void clearGUI();

private:
  QString m_xml;
  KStatusBar *m_statusBar;
  QList<KToolBar> m_toolBars;
  QActionCollection m_actionCollection;
  QGuardedPtr<KPart> m_activePart;
};

#endif
