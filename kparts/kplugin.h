#ifndef PLUGIN_H
#define PLUGIN_H

#include <qobject.h>
#include <qaction.h>
#include <kxmlgui.h>

class KInstance;

namespace KParts
{

/**
 * A plugin is the way to add actions to an existing KParts application,
 * or to a Part.
 *
 * The XML of those plugins looks exactly like of the shell or parts, with
 * one small difference: the document tag, named kpartplugin should have an additional
 * attribute, named "library", and containing the name of the library implementing
 * the plugin.
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

    virtual QAction *action( const QDomElement &element );

    virtual void setDocument( QDomDocument doc );
    virtual QDomDocument document() const;

    /**
     * ... :)
     */
    static void loadPlugins( QObject *parent, const KInstance * instance );

    static QValueList<XMLGUIServant *> pluginServants( QObject *parent );

protected:
    /**
     * Look for plugins in the @p instance's "data" directory (+"/kpartplugins")
     * @return a list of QDomDocuments, containing the parsed xml documents returned by plugins
     */
    static const QValueList<QDomDocument> pluginDocuments( const KInstance * instance );

    /**
     * @internal
     * @return the plugin created from the library @libname
     */
    static Plugin* loadPlugin( QObject * parent, const char* libname );

private:
    QActionCollection m_collection;
    QDomDocument m_doc;
};

};

#endif
