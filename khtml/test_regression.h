/*
 * This file is part of the KDE project
 *
 * (C) 2001 Peter Kelly (pmk@post.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */

#ifndef TEST_REGRESSION_H
#define TEST_REGRESSION_H

#include <khtml_part.h>
#include <kurl.h>
#include <kjs/object.h>
#include <kjs/ustring.h>
#include <kjs/function.h>

class RegressionTest;

/**
 * @internal
 */
class PageLoader : public QObject
{
  Q_OBJECT
public:
    static void loadPage(KHTMLPart *part, KURL url);
    bool m_started;
    bool m_completed;
public slots:
    void partCompleted();
};

/**
 * @internal
 */
class RegTestObject : public KJS::HostImp
{
public:
    RegTestObject(RegressionTest *_regTest);
    KJS::KJSO get(const KJS::UString &p) const;
    void put(const KJS::UString &p, const KJS::KJSO& v);
private:
    RegressionTest *m_regTest;
};

/**
 * @internal
 */
class RegTestFunction : public KJS::InternalFunctionImp
{
public:
    RegTestFunction(RegressionTest *_regTest, int _id, int length);
    KJS::Completion execute(const KJS::List &);
    enum { ReportResult, CheckOutput };
private:
    RegressionTest *m_regTest;
    int id;
};

/**
 * @internal
 */
class KHTMLPartObject : public KJS::HostImp
{
public:
    KHTMLPartObject(KHTMLPart *_part);
    KJS::KJSO get(const KJS::UString &p) const;
    void put(const KJS::UString &p, const KJS::KJSO& v);
private:
    KHTMLPart *m_part;
};

/**
 * @internal
 */
class KHTMLPartFunction : public KJS::InternalFunctionImp
{
public:
    KHTMLPartFunction(KHTMLPart *_part, int _id, int length);
    KJS::Completion execute(const KJS::List &);
    enum { OpenPage, Open, Write, Close };
private:
    KHTMLPart *m_part;
    int id;
};

/**
 * @internal
 */
class RegressionTest : public QObject
{
  Q_OBJECT
public:

    RegressionTest(KHTMLPart *part, QString _sourceFilesDir,
		   QString _resultFilesDir, bool _genOutput);

    QByteArray getPartOutput();
    void testStaticFile(QString filename);
    void testJSFile(QString filename);
    bool checkOutput(QString againstFilename, QByteArray data);
    void runTests(QString relDir = "");
    void reportResult(bool passed, QString testname = "", QString description = "");

    KHTMLPart *m_part;
    QString m_sourceFilesDir;
    QString m_outputFilesDir;
    bool m_genOutput;
    QString m_currentBase;

    QString m_currentCategory;
    QString m_currentTest;

    bool m_getOutput;
    int m_totalPassed;
    int m_totalFailed;

    static RegressionTest *curr;
};

#endif
