/***************************************************************************
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
*/
#include <stdio.h>
#include <string.h>
#include <QtCore/qglobal.h>

#include "QtTest/private/qxmltestlogger_p.h"
#include "QtTest/private/qtestresult_p.h"

namespace QtTest {

    static const char* xmlMessageType2String(QAbstractTestLogger::MessageTypes type)
    {
        switch (type) {
        case QAbstractTestLogger::Warn:
            return "warn";
        case QAbstractTestLogger::QSystem:
            return "system";
        case QAbstractTestLogger::QDebug:
            return "qdebug";
        case QAbstractTestLogger::QWarning:
            return "qwarn";
        case QAbstractTestLogger::QFatal:
            return "qfatal";
        case QAbstractTestLogger::Skip:
            return "skip";
        case QAbstractTestLogger::Info:
            return "info";
        }
        return "??????";
    }

    static const char* xmlIncidentType2String(QAbstractTestLogger::IncidentTypes type)
    {
        switch (type) {
        case QAbstractTestLogger::Pass:
            return "pass";
        case QAbstractTestLogger::XFail:
            return "xfail";
        case QAbstractTestLogger::Fail:
            return "fail";
        case QAbstractTestLogger::XPass:
            return "xpass";
        }
        return "??????";
    }

}


QXmlTestLogger::QXmlTestLogger()
{

}

QXmlTestLogger::~QXmlTestLogger()
{

}


void QXmlTestLogger::startLogging()
{
    char buf[1024];
    QtTest::qt_snprintf(buf, sizeof(buf),
            "<Environment>\n"
            "    <QtVersion>%s</QtVersion>\n"
            "</Environment>\n", qVersion());
    puts(buf);
}

void QXmlTestLogger::stopLogging()
{
}

void QXmlTestLogger::enterTestFunction(const char *function)
{
    char buf[1024];
    QtTest::qt_snprintf(buf, sizeof(buf), "<TestFunction name=\"%s\">", function);
    puts(buf);
}

void QXmlTestLogger::leaveTestFunction()
{
    puts("</TestFunction>\n");
}

namespace QtTest
{

inline static bool isEmpty(const char *str)
{
    return !str || !str[0];
}

static const char *incidentFormatString(bool noDescription, bool noTag)
{
    if (noDescription) {
        if (noTag)
            return "<Incident type=\"%s\" file=\"%s\" line=\"%d\" />";
        else
            return "<Incident type=\"%s\" file=\"%s\" line=\"%d\">\n"
                   "    <DataTag><![CDATA[%s%s]]></DataTag>\n"
                   "</Incident>";
    } else {
        if (noTag)
            return "<Incident type=\"%s\" file=\"%s\" line=\"%d\">\n"
                   "    <Description><![CDATA[%s%s]]></Description>\n"
                   "</Incident>";
        else
            return "<Incident type=\"%s\" file=\"%s\" line=\"%d\">\n"
                   "    <DataTag><![CDATA[%s]]></DataTag>\n"
                   "    <Description><![CDATA[%s]]></Description>\n"
                   "</Incident>";
    }
}

static const char *messageFormatString(bool noDescription, bool noTag)
{
    if (noDescription) {
        if (noTag)
            return "<Message type=\"%s\" file=\"%s\" line=\"%d\" />";
        else
            return "<Message type=\"%s\" file=\"%s\" line=\"%d\">\n"
                   "    <DataTag><![CDATA[%s%s]]></DataTag>\n"
                   "</Message>";
    } else {
        if (noTag)
            return "<Message type=\"%s\" file=\"%s\" line=\"%d\">\n"
                   "    <Description><![CDATA[%s%s]]></Description>\n"
                   "</Message>";
        else
            return "<Message type=\"%s\" file=\"%s\" line=\"%d\">\n"
                   "    <DataTag><![CDATA[%s]]></DataTag>\n"
                   "    <Description><![CDATA[%s]]></Description>\n"
                   "</Message>";
    }
}

} // namespace

void QXmlTestLogger::addIncident(IncidentTypes type, const char *description,
                                 const char *file, int line)
{
    char buf[1024];
    const char *tag = QtTestResult::currentDataTag();

    QtTest::qt_snprintf(buf, sizeof(buf),
            QtTest::incidentFormatString(!QtTest::isEmpty(description), !QtTest::isEmpty(tag)),
            QtTest::xmlIncidentType2String(type),
            file ? file : "", line,
            tag ? tag : "",
            description ? description : "");

    puts(buf);
}

void QXmlTestLogger::addMessage(MessageTypes type, const char *message,
                                const char *file, int line)
{
    char buf[1024];
    const char *tag = QtTestResult::currentDataTag();

    QtTest::qt_snprintf(buf, sizeof(buf),
            QtTest::messageFormatString(!QtTest::isEmpty(message), !QtTest::isEmpty(tag)),
            QtTest::xmlMessageType2String(type),
            file ? file : "", line,
            tag ? tag : "",
            message ? message : "");

    puts(buf);
}

