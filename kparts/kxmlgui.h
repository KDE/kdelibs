
#ifndef __kxmlgui_h__
#define __kxmlgui_h__

#include <qobject.h>
#include <qaction.h>
#include <qdom.h>

class KXMLGUIBuilder;

// Serves actions and xml to the GUI factory

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

class KXMLGUIFactory
{
 public:
  static QString readConfigFile( const QString &filename );

  static void createGUI( KXMLGUIServant *shell, KXMLGUIServant *part, KXMLGUIBuilder *builder );

 private:
  KXMLGUIFactory( KXMLGUIServant *shellServant, KXMLGUIServant *partServant, KXMLGUIBuilder *builder );

  void buildRecursive( const QDomElement &shellElement,
                       const QDomElement &partElement,
                       QObject *parent = 0L );

  QDomElement findMatchingElement( const QDomElement &shellElement, const QDomElement &partElement );

  KXMLGUIServant *m_shellServant;
  KXMLGUIServant *m_partServant;
  KXMLGUIBuilder *m_builder;

};

#endif
