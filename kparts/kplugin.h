#ifndef PLUGIN_H
#define PLUGIN_H

#include <qobject.h>
#include <qaction.h>

/**
 * (TODO what is a plugin)
 *
 * If you want this plugin to be used by a part, you need to
 * install the rc file under the directory
 * "data" (KDEDIR/share/apps usually)+"/instancename/pluginactions/"
 * where instancename is the name of the part's instance.
 */
class KPlugin : public QObject
{
    Q_OBJECT
public:
    KPlugin( QObject* parent = 0, const char* name = 0 );
    ~KPlugin();

    QAction* action( const char* name );
    QActionCollection* actionCollection();

private:
    QActionCollection m_collection;
};

#endif
