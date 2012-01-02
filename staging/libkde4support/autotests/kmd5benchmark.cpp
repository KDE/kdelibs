/*
    This file is part of the KDE libraries

    Copyright (C) 2011 Volker Krause <vkrause@kde.org>

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

#include <kmd5.h>
#include <qtest_kde.h>

#include <QCryptographicHash>

class KMd5Benchmark : public QObject {
    Q_OBJECT

private:
    static QByteArray makeByteArray( int size )
    {
        QByteArray b;
        b.resize( size );
        for ( int i = 0; i < size; ++i )
            b[i] = static_cast<char>( i % 255 );
        return b;
    }

    static void makeData()
    {
        QTest::addColumn<QByteArray>("input");
        QTest::newRow("empty") << QByteArray();
        QTest::newRow("32") << makeByteArray(32);
	QTest::newRow("128") << makeByteArray(128);
	QTest::newRow("1024") << makeByteArray(1 << 10);
	QTest::newRow("1M") << makeByteArray(1 << 20);
	QTest::newRow("16M") << makeByteArray(1 << 24);
    }

private Q_SLOTS:
    void benchmarkKMd5_data()
    {
        makeData();
    }
    
    void benchmarkKMd5()
    {
        QFETCH( QByteArray, input );
        QBENCHMARK {
            KMD5 md5( input );
            md5.rawDigest();
        }
    }

    void benchmarkKMd5WithReset_data()
    {
        makeData();
    }

    void benchmarkKMd5WithReset()
    {
        QFETCH( QByteArray, input );
        KMD5 md5;
        QBENCHMARK {
            md5.reset();
            md5.update( input );
	    md5.rawDigest();
        }
    }

    void benchmarkQCH_data()
    {
        makeData();
    }

    void benchmarkQCH()
    {
        QFETCH( QByteArray, input );
        QBENCHMARK {
            QCryptographicHash h( QCryptographicHash::Md5 );
            h.addData( input );
            h.result();
        }
    }

    void benchmarkQCHWithReset_data()
    {
        makeData();
    }

    void benchmarkQCHWithReset()
    {
        QFETCH( QByteArray, input );
        QCryptographicHash h( QCryptographicHash::Md5 );
        QBENCHMARK {
            h.reset();
            h.addData( input );
            h.result();
        }
    }

};

QTEST_KDEMAIN_CORE( KMd5Benchmark )

#include "kmd5benchmark.moc"
