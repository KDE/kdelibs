/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DYNAMICRESOURCEFACETTEST_H
#define DYNAMICRESOURCEFACETTEST_H

#include <QObject>
#include "ktempdir.h"
#include "tag.h"
#include "dynamicresourcefacet.h"

#include <Soprano/Model>

/**
 * The only point of deriving from DynamicResourceFacet is to not use the query service
 * which would ignore our custom model. This is done by touching the public API as little
 * as possible. The only change necessary is a friend class declaration in the dynamicresourcefacet.h
 * header.
 */
class DynamicResourceTestFacet : public Nepomuk::Utils::DynamicResourceFacet
{
public:
    DynamicResourceTestFacet();

private:
    class TestPrivate;
};


class DynamicResourceFacetTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testEmptyFacet();
    void testSetRelation();
    void testMatchOne();
    void testMatchAll();
    void testMatchAny();
    void testSetClientQuery();

private:
    KTempDir* m_storageDir;
    Soprano::Model* m_model;
    Nepomuk::Tag tag1, tag2, tag3, tag4;
};

#endif // DYNAMICRESOURCEFACETTEST_H
