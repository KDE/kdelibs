/**
 * This file is part of the KDE project
 *
 * (C) 2001,2003 Peter Kelly (pmk@post.com)
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
#include <qobject.h>
#include <kjs/ustring.h>
#include <kjs/object.h>
#include <kjs/interpreter.h>

class RegressionTest;

/**
 * @internal
 */
class PartMonitor : public QObject
{
  Q_OBJECT
public:
    PartMonitor(KHTMLPart *_part);
    void waitForCompletion();
    bool m_inLoop;
    bool m_completed;
    KHTMLPart *m_part;
public slots:
    void partCompleted();
};

/**
 * @internal
 */
class RegTestObject : public KJS::ObjectImp
{
public:
    RegTestObject(KJS::ExecState *exec, RegressionTest *_regTest);

private:
    RegressionTest *m_regTest;
};

/**
 * @internal
 */
class RegTestFunction : public KJS::ObjectImp
{
public:
    RegTestFunction(KJS::ExecState *exec, RegressionTest *_regTest, int _id, int length);

    bool implementsCall() const;
    KJS::Value call(KJS::ExecState *exec, KJS::Object &thisObj, const KJS::List &args);

    enum { Print, ReportResult, CheckOutput };

private:
    RegressionTest *m_regTest;
    int id;
};

/**
 * @internal
 */
class KHTMLPartObject : public KJS::ObjectImp
{
public:
    KHTMLPartObject(KJS::ExecState *exec, KHTMLPart *_part);

    virtual KJS::Value get(KJS::ExecState *exec, const KJS::UString &propertyName) const;

private:
    KHTMLPart *m_part;
};

/**
 * @internal
 */
class KHTMLPartFunction : public KJS::ObjectImp
{
public:
    KHTMLPartFunction(KJS::ExecState *exec, KHTMLPart *_part, int _id, int length);

    bool implementsCall() const;
    KJS::Value call(KJS::ExecState *exec, KJS::Object &thisObj, const KJS::List &args);

    enum { OpenPage, OpenPageAsUrl, Begin, Write, End, ExecuteScript, ProcessEvents };
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
    bool checkOutput(QString againstFilename);
    bool runTests(QString relPath = "", bool mustExist = false);
    void reportResult(bool passed, QString description = "");
    void createMissingDirs(QString path);

    KHTMLPart *m_part;
    QString m_sourceFilesDir;
    QString m_outputFilesDir;
    bool m_genOutput;
    QString m_currentBase;

    QString m_currentCategory;
    QString m_currentTest;

    bool m_getOutput;
    int m_passes;
    int m_failures;
    int m_errors;

    static RegressionTest *curr;

private slots:
  void slotOpenURL(const KURL &url, const KParts::URLArgs &args);

};

#endif
