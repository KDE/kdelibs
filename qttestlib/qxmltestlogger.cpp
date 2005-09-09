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
    QAbstractTestLogger::startLogging();

    char buf[1024];
    QtTest::qt_snprintf(buf, sizeof(buf),
            "<Environment>\n"
            "    <QtVersion>%s</QtVersion>\n"
            "</Environment>\n", qVersion());
    outputString(buf);
}

void QXmlTestLogger::stopLogging()
{
    QAbstractTestLogger::stopLogging();
}

void QXmlTestLogger::enterTestFunction(const char *function)
{
    char buf[1024];
    QtTest::qt_snprintf(buf, sizeof(buf), "<TestFunction name=\"%s\">", function);
    outputString(buf);
}

void QXmlTestLogger::leaveTestFunction()
{
    outputString("</TestFunction>\n");
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
            return "<Incident type=\"%s\" file=\"%s\" line=\"%d\" />\n";
        else
            return "<Incident type=\"%s\" file=\"%s\" line=\"%d\">\n"
                   "    <DataTag><![CDATA[%s%s%s%s]]></DataTag>\n"
                   "</Incident>\n";
    } else {
        if (noTag)
            return "<Incident type=\"%s\" file=\"%s\" line=\"%d\">\n"
                   "    <Description><![CDATA[%s%s%s%s]]></Description>\n"
                   "</Incident>\n";
        else
            return "<Incident type=\"%s\" file=\"%s\" line=\"%d\">\n"
                   "    <DataTag><![CDATA[%s%s%s]]></DataTag>\n"
                   "    <Description><![CDATA[%s]]></Description>\n"
                   "</Incident>\n";
    }
}

static const char *messageFormatString(bool noDescription, bool noTag)
{
    if (noDescription) {
        if (noTag)
            return "<Message type=\"%s\" file=\"%s\" line=\"%d\" />\n";
        else
            return "<Message type=\"%s\" file=\"%s\" line=\"%d\">\n"
                   "    <DataTag><![CDATA[%s%s%s%s]]></DataTag>\n"
                   "</Message>\n";
    } else {
        if (noTag)
            return "<Message type=\"%s\" file=\"%s\" line=\"%d\">\n"
                   "    <Description><![CDATA[%s%s%s%s]]></Description>\n"
                   "</Message>\n";
        else
            return "<Message type=\"%s\" file=\"%s\" line=\"%d\">\n"
                   "    <DataTag><![CDATA[%s%s%s]]></DataTag>\n"
                   "    <Description><![CDATA[%s]]></Description>\n"
                   "</Message>\n";
    }
}

} // namespace

void QXmlTestLogger::addIncident(IncidentTypes type, const char *description,
                                 const char *file, int line)
{
    char buf[1536];
    const char *tag = QtTestResult::currentDataTag();
    const char *gtag = QtTestResult::currentGlobalDataTag();
    const char *filler = (tag && gtag) ? ":" : "";
    const bool notag = QtTest::isEmpty(tag) && QtTest::isEmpty(gtag);

    QtTest::qt_snprintf(buf, sizeof(buf),
            QtTest::incidentFormatString(QtTest::isEmpty(description), notag),
            QtTest::xmlIncidentType2String(type),
            file ? file : "", line,
            gtag ? gtag : "",
            filler,
            tag ? tag : "",
            description ? description : "");

    outputString(buf);
}

void QXmlTestLogger::addMessage(MessageTypes type, const char *message,
                                const char *file, int line)
{
    char buf[1536];
    char msgbuf[1024];
    const char *tag = QtTestResult::currentDataTag();
    const char *gtag = QtTestResult::currentGlobalDataTag();
    const char *filler = (tag && gtag) ? ":" : "";
    const bool notag = QtTest::isEmpty(tag) && QtTest::isEmpty(gtag);

    QtTest::qt_snprintf(msgbuf, sizeof(msgbuf), "%s",
                        message ? message : "");

    QtTest::qt_snprintf(buf, sizeof(buf),
            QtTest::messageFormatString(QtTest::isEmpty(message), notag),
            QtTest::xmlMessageType2String(type),
            file ? file : "", line,
            gtag ? gtag : "",
            filler,
            tag ? tag : "",
            msgbuf);

    outputString(buf);
}

