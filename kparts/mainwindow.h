#ifndef __KGUIBUILDER_H
#define __KGUIBUILDER_H

#include <qlist.h>
#include <qaction.h>

#include <ktmainwindow.h>

#include <kparts/xmlgui.h>
#include <kparts/part.h>

class QString;

namespace KParts
{

class MainWindowPrivate;

/**
 * A KPart-aware main window, whose user interface is described in XML.
 *
 * Inherit your main window from this class
 * and don't forget to call @ref setXMLFile() in the inherited constructor.
 *
 * It implements all internal interfaces in the case of a @ref KTMainWindow as host:
 * the builder and servant interface (for menu merging).
 */
class MainWindow : public KTMainWindow, public XMLGUIBuilder, public XMLGUIServant
{
  Q_OBJECT
 public:
  /**
   * Constructor, same signature as @ref KTMainWindow.
   */
  MainWindow( const char *name = 0L, WFlags f = WDestructiveClose );
  /**
   * Destructor.
   */
  virtual ~MainWindow();

  /**
   * Set the name of the GUI description (XML) file.
   *
   * Call this in the inherited class constructor
   * to set the name of the rc file containing the XML for the mainwindow
   *
   * @param file Either an absolute path for the file, or simply the filename,
   *             which will then be assumed to be installed in the "data" resource,
   *             under a directory named like the application.
   */
  virtual void setXMLFile( QString file );

  /**
   * Call this instead of @ref setXMLFile() if your XML doesn't come from a file,
   * but rather from a string.
   */
  //TODO: return bool, to make it possible for the part to check whether its xml is invalid (Simon)
  // Why only here and not in setXMLFile, which could be invalid as well ?
  // Or perhaps the checks should rather be done in those methods anyway (David)
  virtual void setXML( const QString &document );

  /**
   * Add your actions to this collection.
   */
  QActionCollection *actionCollection() { return &m_actionCollection; }

  XMLGUIFactory *guiFactory() const;

  // ---

  // KXMLGUIServant interface (internal)
  virtual QAction *action( const QDomElement &element );
  virtual QDomDocument document() const;

  // KXMLGUIBuilder interface (internal)
  virtual QWidget *createContainer( QWidget *parent, int index, const QDomElement &element, const QByteArray &containerStateBuffer, int &id );

  // KXMLGUIBuilder interface (internal)
  virtual QByteArray removeContainer( QWidget *container, QWidget *parent, int id );

  virtual int insertSeparator( QWidget *parent, int index );
  virtual void removeSeparator( QWidget *parent, int id );

protected slots:

  /**
   * Create the GUI (by merging the host's and the active part's)
   *
   * Called on startup and whenever the active part changes
   * For this you need to connect this slot to the
   * @ref KPartManager::activePartChanged() signal
   * @param part The active part (set to 0L if no part).
   */
  virtual void createGUI( KParts::Part * part );

  /**
   * Called when the active part wants to change the statusbar message
   * Reimplement if your mainwindow has a complex statusbar
   * (with several items)
   */
  virtual void slotSetStatusBarText( const QString & );

private:
  QActionCollection m_actionCollection;
  XMLGUIFactory *m_factory;

  MainWindowPrivate *d;
};

};

#endif
