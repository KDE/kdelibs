/*
 *  Copyright (C) 2002-2005 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */
#ifndef KFILTERTEST_H
#define KFILTERTEST_H

#include <qobject.h>

#include <QObject>

class KFilterTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void test_block_write();
    void test_block_read();
    void test_getch();
    void test_textstream();

private:
    void test_block_write( const QString & fileName );
    void test_block_read( const QString & fileName );
    void test_getch( const QString & fileName );
    void test_textstream( const QString & fileName );

    QString pathgz;
    QString pathbz2;
    QByteArray testData;
};


#endif
