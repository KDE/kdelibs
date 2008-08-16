#include "qtest_kde.h"

// A signal spy which exits the event loop when the signal is called,
// and remembers that the signal was emitted.
class KDESignalSpy : public QObject
{
    Q_OBJECT
public:
    KDESignalSpy(QObject *obj, const char *signal, int timeout)
        : QObject(0), m_obj(obj), m_emitted(false)
    {
        connect(obj, signal, this, SLOT(slotSignalEmitted()));
        if (timeout > 0) {
            QObject::connect(&m_timer, SIGNAL(timeout()), &m_loop, SLOT(quit()));
            m_timer.setSingleShot(true);
            m_timer.start(timeout);
        }
        m_loop.exec();
    }
    bool signalEmitted() const { return m_emitted; }

private Q_SLOTS:
    void slotSignalEmitted()
    {
        m_emitted = true;
        disconnect(m_obj, 0, this, 0);
        m_timer.stop();
        m_loop.quit();
    }
private:
    QObject* m_obj;
    bool m_emitted;
    QEventLoop m_loop;
    QTimer m_timer;
};

// Unit test for this code: tests/kglobaltest.cpp

bool QTest::kWaitForSignal(QObject *obj, const char *signal, int timeout )
{
    KDESignalSpy spy(obj, signal, timeout);
    return spy.signalEmitted();
}

#include "qtest_kde.moc"
