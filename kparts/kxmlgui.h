
#ifndef __kxmlgui_h__
#define __kxmlgui_h__

#include <qobject.h>
#include <qaction.h>
#include <qdom.h>

// Serves actions and xml to the GUI builder

class KXMLGUIServant
{
 public:
  
  virtual QAction *action( const QDomElement &element ) = 0;

  virtual QString xml() = 0;

};

class KNullGUIServant : public KXMLGUIServant
{
 public:
  KNullGUIServant() {}

  virtual QAction *action( const QDomElement & ) { return 0L; }
  virtual QString xml() { return QString::null; }
};

// Interface for a "GUI builder", used by the GUIFactory
// Note : if the KTMainWindow GUI builder is going to be the only GUIBuilder,
// there's even no need for an interface. The factory could use the builder
// directly... But this way is cleaner though...
class KXMLGUIBuilder
{
 public:

  /*
   * Create a container (=some kind of generic notion?) from an element in the XML file
   * @param parent the parent for the widget (not a widget ?)
   *
   * return 0L if you handled the element yourself (like for Separators
   * for example)
   */
  virtual QWidget *createContainer( QObject *parent, const QDomElement &element ) = 0;

};

class KXMLGUIFactory
{
 public:
  static QString readConfigFile( const QString &filename );

  static void createGUI( KXMLGUIServant *shell, KXMLGUIServant *part, KXMLGUIBuilder *builder );

 private:
  // Note : a real class instead of only static methods could avoid passing the
  // builder as an argument each time
  static void buildRecursive( const QDomElement &shellElement, KXMLGUIServant *shellServant,
                              const QDomElement &partElement, KXMLGUIServant *partServant,
                              KXMLGUIBuilder *builder, QObject *parent = 0L );

};

#endif
