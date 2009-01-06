/*
 *  Copyright (C) 2005 David Faure   <faure@kde.org>
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

#ifndef KMIMETYPETEST_H
#define KMIMETYPETEST_H

#include <QtCore/QObject>
#include <kmimemagicrule_p.h>

class KMimeTypeTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testByName();
    void testIcons();
    void testFindByUrl();
    void testFindByPathUsingFileName_data();
    void testFindByPathUsingFileName();
    void testAdditionalGlobs_data();
    void testAdditionalGlobs();
    void testFindByPathWithContent();
    void testFindByNameAndContent();
    void testFindByContent();
    void testFindByContent_data();
    void testFindByFileContent();
    void testAllMimeTypes();
    void testAlias();
    void testMimeTypeParent();
    void testMimeTypeInheritancePerformance();
    void testMimeTypeTraderForTextPlain();
    void testMimeTypeTraderForDerivedMimeType();
    void testMimeTypeTraderForAlias();
    void testHasServiceType1();
    void testHasServiceType2();
    void testPatterns_data();
    void testPatterns();
    void testExtractKnownExtension_data();
    void testExtractKnownExtension();
    void testParseMagicFile_data();
    void testParseMagicFile();
private:
    QList<KMimeMagicRule> m_rules;
};


#endif
