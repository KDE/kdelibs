#ifndef PLUGIN_H
#define PLUGIN_H

#include <qobject.h>
#include <qaction.h>
#include <kxmlgui.h>

class KInstance;

namespace KParts
{

/**
 * A plugin is the way to add actions to an existing @ref KParts application,
 * or to a @ref Part.
 *
 * The XML of those plugins looks exactly like of the shell or parts, with
 * one small difference: The document tag, named kpartplugin should have an additional
 * attribute, named "library", and contain the name of the library implementing
 * the plugin.
 *
 * If you want this plugin to be used by a part, you need to
 * install the rc file under the directory
 * "data" (KDEDIR/share/apps usually)+"/instancename/kpartplugins/"
 * where instancename is the name of the part's instance.
 */
class Plugin : public QObject, KXMLGUIServant
{
    Q_OBJECT
public:
    Plugin( QObject* parent = 0, const char* name = 0 );
    ~Plugin();

    /**
     * Retrieve an action by name.
     **/
    QAction* action( const char* name );
    /**
     * Retrieve the entire action collection for the plugin.
     **/
    QActionCollection* actionCollection();

    /**
     * Retrieve an action for a given @ref QDomElement.
     **/
    virtual QAction *action( const QDomElement &element );

    /**
     * Set the @ref QDomDocument describing the part.
     **/
    virtual void setDocument( QDomDocument doc );
    /**
     * Retrieve the @ref QDomDocument describing the part.
     **/
    virtual QDomDocument document() const;

    /**
     * Load the plugin libraries from the directories appropriate
     * to @p instance.
     */
    static void loadPlugins( QObject *parent, const KInstance * instance );

    /**
     * Load the plugin libraries specified by the list @p docs.
     */
    static void loadPlugins( QObject *parent, const QValueList<QDomDocument> &docs );

    static QValueList<KXMLGUIServant *> pluginServants( QObject *parent );

protected:
    /**
     * Look for plugins in the @p instance's "data" directory (+"/kpartplugins")
     * @return A list of @ref QDomDocument s, containing the parsed xml documents returned by plugins.
     */
    static const QValueList<QDomDocument> pluginDocuments( const KInstance * instance );

    /**
     * @internal
     * @return The plugin created from the library @p libname
     */
    static Plugin* loadPlugin( QObject * parent, const char* libname );

private:
    QActionCollection m_collection;
    QDomDocument m_doc;
};

};

#endif
