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
class KPartsMainWindowPrivate;

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

  // Hack! ;-) (just needed it for testing the addServant/removeServant stuff :) (Simon)
  KXMLGUIFactory *guiFactory() const;

  // ---

  // KXMLGUIServant interface (internal)
  virtual QAction *action( const QDomElement &element );
  virtual QDomDocument document();

  // KXMLGUIBuilder interface (internal)
  virtual QObject *createContainer( QWidget *parent, int index, const QDomElement &element, const QByteArray &containerStateBuffer );

  // KXMLGUIBuilder interface (internal)
  virtual QByteArray removeContainer( QObject *container, QWidget *parent );

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
//should be obsolete (Simon)
//  virtual void clearGUI();

private:
  QActionCollection m_actionCollection;
  KXMLGUIFactory *m_factory;

  KPartsMainWindowPrivate *d;
};

#endif
