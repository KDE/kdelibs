#ifndef PLUGIN_H
#define PLUGIN_H

#include <qobject.h>
#include <qaction.h>
#include <kxmlgui.h>

namespace KParts
{

/**
 * A plugin is the way to add actions to an existing KParts application,
 * or to a Part. (David : it looks like only the latter is true currently !)
 *
 * The XML of those plugins looks exactly like of the shell or parts, with
 * one small difference: the <Action> tag should have an additional
 * attribute, named "plugin".
 *
 * If you want this plugin to be used by a part, you need to
 * install the rc file under the directory
 * "data" (KDEDIR/share/apps usually)+"/instancename/kpartplugins/"
 * where instancename is the name of the part's instance.
 */
class Plugin : public QObject
{
    Q_OBJECT
public:
    Plugin( QObject* parent = 0, const char* name = 0 );
    ~Plugin();

    QAction* action( const char* name );
    QActionCollection* actionCollection();

    /**
     * @internal
     * @return the plugin created from the library @libname
     */
    static Plugin* loadPlugin( QObject * parent, const char* libname );

private:
    QActionCollection m_collection;
};

/**
 * @internal
 * The @ref XMLGUIServant for a @ref Plugin, providing actions
 * and XML for their layout to the merging engine.
 *
 * This is not merged with Plugin, because the Plugin is loaded
 * only when one of its actions is activated.
 */
class PluginGUIServant : public QObject, public XMLGUIServant
{
  Q_OBJECT
 public:
  PluginGUIServant( Part *part, const QDomDocument &document );

  virtual QAction *action( const QDomElement &element );

  virtual QDomDocument document() const;

 private:
  Part *m_part;
  QDomDocument m_doc;
};

};

#endif
