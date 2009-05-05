
#ifndef KNOTIFICATIONITEMTEST_H
#define KNOTIFICATIONITEMTEST_H

#include <QObject>

namespace Experimental
{
    class KNotificationItem;
} // namespace Experitmental

class KNotificationItemTest : public QObject
{
  Q_OBJECT

public:
    KNotificationItemTest(QObject *parent, Experimental::KNotificationItem *tray);
    //~KNotificationItemTest();

public Q_SLOTS:
    void setNeedsAttention();
    void setActive();
    void setPassive();
private:
    Experimental::KNotificationItem *m_tray;
};

#endif
