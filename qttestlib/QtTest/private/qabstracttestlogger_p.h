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
#ifndef QABSTRACTTESTLOGGER_H
#define QABSTRACTTESTLOGGER_H

class QAbstractTestLogger
{
public:
    enum IncidentTypes {
        Pass,
        XFail,
        Fail,
        XPass
    };


    enum MessageTypes {
        Warn,
        QWarning,
        QDebug,
        QSystem,
        QFatal,
        Skip,
        Info
    };


    QAbstractTestLogger() {}
    virtual ~QAbstractTestLogger() {}

    virtual void startLogging();
    virtual void stopLogging();

    virtual void enterTestFunction(const char *function) = 0;
    virtual void leaveTestFunction() = 0;

    virtual void addIncident(IncidentTypes type, const char *description,
                             const char *file = 0, int line = 0) = 0;

    virtual void addMessage(MessageTypes type, const char *message,
                            const char *file = 0, int line = 0) = 0;

    static void outputString(const char *msg);
};

#endif

