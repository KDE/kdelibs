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
#ifndef QXMLTESTLOGGER_H
#define QXMLTESTLOGGER_H

#include "QtTest/private/qabstracttestlogger_p.h"

class QXmlTestLogger : public QAbstractTestLogger
{
public:
    enum XmlMode { Complete = 0, Light };

    QXmlTestLogger(XmlMode mode = Complete);
    ~QXmlTestLogger();

    void startLogging();
    void stopLogging();

    void enterTestFunction(const char *function);
    void leaveTestFunction();

    void addIncident(IncidentTypes type, const char *description,
                     const char *file = 0, int line = 0);

    void addMessage(MessageTypes type, const char *message,
                    const char *file = 0, int line = 0);

private:
    XmlMode xmlmode;
};

#endif

