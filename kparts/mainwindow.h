#ifndef __KGUIBUILDER_H
#define __KGUIBUILDER_H

#include <qlist.h>
#include <kaction.h>

#include <ktmainwindow.h>

#include <kxmlgui.h>
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
class MainWindow : public KTMainWindow, virtual public PartBase
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
  MainWindowPrivate *d;
};

};

#endif
