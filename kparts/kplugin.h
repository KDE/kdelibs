#ifndef PLUGIN_H
#define PLUGIN_H

#include <qobject.h>
#include <qaction.h>

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
