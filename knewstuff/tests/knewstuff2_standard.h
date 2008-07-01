#ifndef KNEWSTUFF2_TEST_STANDARD_H
#define KNEWSTUFF2_TEST_STANDARD_H

#include <knewstuff2/core/entry.h>

#include <QtCore/QObject>

namespace KNS
{
class Engine;
}

class KNewStuff2Standard : public QObject
{
    Q_OBJECT
public:
    KNewStuff2Standard();
    void run(bool upload, bool modal, QString file);
private:
    KNS::Engine *m_engine;
};

#endif
