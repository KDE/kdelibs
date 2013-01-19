/* This file is part of the KDE libraries

    Copyright 2012 David Faure <faure@kde.org>
    Copyright 2012 Kai Dombrowe <just89@gmx.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kstartupinfo.h>
#include <QSignalSpy>
#include <qtest_widgets.h>

class KStartupInfo_UnitTest : public QObject
{
    Q_OBJECT
public:
    KStartupInfo_UnitTest()
        : m_listener(true, this),
        m_receivedCount(0) {
        connect(&m_listener, SIGNAL(gotNewStartup(KStartupInfoId, KStartupInfoData)),
                this, SLOT(slotNewStartup(KStartupInfoId, KStartupInfoData)));
    }

protected Q_SLOTS:
    void slotNewStartup(const KStartupInfoId &id, const KStartupInfoData &data)
    {
        ++m_receivedCount;
        m_receivedId = id;
        m_receivedData = data;
        emit ready();
    }
Q_SIGNALS:
    void ready();

private Q_SLOTS:
    void testStart();

private:
    KStartupInfo m_listener;

    int m_receivedCount;
    KStartupInfoId m_receivedId;
    KStartupInfoData m_receivedData;
};

void KStartupInfo_UnitTest::testStart()
{
    KStartupInfoId id;
    id.initId(KStartupInfo::createNewStartupId());

    KStartupInfoData data;
    const QString appId = "/dir with space/kstartupinfo_unittest.desktop";
    data.setApplicationId(appId);
    const QString iconPath = "/dir with space/kstartupinfo_unittest.png";
    data.setIcon(iconPath);
    const QString description = "A description";
    data.setDescription(description);
    const QString name = "A name";
    data.setName(name);
    const int pid = 12345;
    data.addPid(pid);
    const QString bin = "dir with space/kstartupinfo_unittest";
    data.setBin(bin);

    KStartupInfo::sendStartup(id, data);
    KStartupInfo::sendFinish(id, data);

    QSignalSpy spy(this, SIGNAL(ready()));
    spy.wait(5000);

    QCOMPARE(m_receivedCount, 1);
    // qDebug() << m_receivedId.id(); // something like "$HOSTNAME;1342544979;490718;8602_TIME0"
    QCOMPARE(m_receivedData.name(), name);
    QCOMPARE(m_receivedData.description(), description);
    QCOMPARE(m_receivedData.applicationId(), appId);
    QCOMPARE(m_receivedData.icon(), iconPath);
    QCOMPARE(m_receivedData.bin(), bin);
    //qDebug() << m_receivedData.bin() << m_receivedData.name() << m_receivedData.description() << m_receivedData.icon() << m_receivedData.pids() << m_receivedData.hostname() << m_receivedData.applicationId();
}

QTEST_MAIN(KStartupInfo_UnitTest)

#include "kstartupinfo_unittest.moc"
