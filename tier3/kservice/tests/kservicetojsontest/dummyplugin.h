#ifndef DUMMYPLUGIN_H
#define DUMMYPLUGIN_H

#include <QObject>

class DummyPlugin : public QObject
{
    Q_OBJECT
public:
    DummyPlugin(QObject*, const QVariantList&);
};

#endif /* DUMMYPLUGIN_H */
