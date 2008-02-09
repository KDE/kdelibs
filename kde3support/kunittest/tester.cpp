/**
 * Copyright (C)  2005  Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tester.h"

#include <iostream>
using namespace std;

#include <QtCore/QMetaEnum>
#include <QtCore/QRect>
#include <QtCore/QVector>

namespace KUnitTest
{
    SlotTester::SlotTester() : Tester()
    {
        m_total = m_results;
    }


    SlotTester::~SlotTester()
    {
        qDeleteAll( m_resultsList );
    }


    void SlotTester::invokeMember(const QString &str)
    {
        QString slotname = QString::number(QSLOT_CODE) + str;
        connect(this, SIGNAL(invoke()), this, slotname.toAscii().constData());
        emit invoke();
        disconnect(this, SIGNAL(invoke()), this, slotname.toAscii().constData());
    }

    void SlotTester::allTests()
    {
        QVector<QByteArray> allSlots;
        const int methodCount = metaObject()->methodCount();
        const int methodOffset = metaObject()->methodOffset();
        allSlots.reserve( methodCount );
        for ( int i=0 ; i < methodCount; ++i )
        {
            QMetaMethod method = metaObject()->method( methodOffset + i );
            if ( method.methodType() == QMetaMethod::Slot )
                allSlots.append( method.signature() );
        }

        if ( allSlots.contains("setUp()") )
            invokeMember("setUp()");

        foreach ( const QByteArray &sl, allSlots )
        {
            if ( sl.startsWith("test") )
            {
                m_results = results(sl);
                Q_ASSERT( m_results );
                m_results->clear();

                cout << "KUnitTest_Debug_BeginSlot[" << sl.data() << "]" << endl;
                invokeMember(sl);
                cout << "KUnitTest_Debug_EndSlot[" << sl.data() << "]" << endl;
            }
        }

        if ( allSlots.contains("tearDown()") )
            invokeMember("tearDown()");

        m_total->clear();
    }

    TestResults *SlotTester::results(const char *sl)
    {
        if ( !m_resultsList.contains(sl) )
            m_resultsList.insert(sl, new TestResults());

        return m_resultsList[sl];
    }
}

QTextStream& operator<<( QTextStream& str, const QRect& r ) {
    str << "[" << r.x() << "," << r.y() << " - " << r.width() << "x" << r.height() << "]";
    return str;
}

QTextStream& operator<<( QTextStream& str, const QPoint& r ) {
    str << "(" << r.x() << "," << r.y() << ")";
    return str;
}

QTextStream& operator<<( QTextStream& str, const QSize& r ) {
    str << "[" << r.width() << "x" << r.height() << "]";
    return str;
}

#include "tester.moc"
