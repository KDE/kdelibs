
#ifndef __kxmlgui_h__
#define __kxmlgui_h__

#include <qobject.h>
#include <qaction.h>
#include <qdom.h>

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

class KXMLGUIShellServant : public KXMLGUIServant
{
 public:

  /*
   * return 0L if you handled the element yourself (like for Separators
   * for example)
   */
  virtual QWidget *createContainer( QObject *parent, const QDomElement &element ) = 0;

};

class KXMLGUIFactory
{
 public:
  static void createGUI( KXMLGUIShellServant *shell, KXMLGUIServant *part );

 private:
  static void buildRecursive( const QDomElement &shellElement, KXMLGUIShellServant *shellServant,
                              const QDomElement &partElement, KXMLGUIServant *partServant,
                              QObject *parent = 0 );

};

#endif
