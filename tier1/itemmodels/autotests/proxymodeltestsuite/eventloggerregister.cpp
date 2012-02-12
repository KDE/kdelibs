/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "eventloggerregister.h"

#include "modeleventlogger.h"

#include <QDebug>

EventLoggerRegister::~EventLoggerRegister()
{
}

EventLoggerRegister* EventLoggerRegister::s_instance = 0;
std::auto_ptr<EventLoggerRegister> EventLoggerRegister::s_destroyer;

EventLoggerRegister* EventLoggerRegister::instance(Behaviour behaviour)
{
  if (!s_instance)
  {
    s_instance = new EventLoggerRegister(behaviour);
    s_destroyer.reset(s_instance);
  }
  return s_instance;
}

void EventLoggerRegister::registerLogger(ModelEventLogger* logger)
{
  m_loggers.append(logger);
}

void EventLoggerRegister::unregisterLogger(ModelEventLogger* logger)
{
  m_loggers.remove(m_loggers.indexOf(logger));
}

void EventLoggerRegister::writeLogs()
{
  static bool asserting = false;
  if (!asserting)
  {
    // If logger->writeLog asserts, we don't segfault
    asserting = true;
    // The destructor writes the log.
    qDeleteAll(m_loggers);
    m_loggers.clear();
    asserting = false;
  }
}

void myMessageOutput(QtMsgType type, const char *msg)
{
  switch (type) {
  case QtDebugMsg:
    fprintf(stderr, "Debug: %s\n", msg);
    break;
  case QtWarningMsg:
    fprintf(stderr, "Warning: %s\n", msg);
    break;
  case QtCriticalMsg:
    fprintf(stderr, "Critical: %s\n", msg);
    break;
  case QtFatalMsg:
    EventLoggerRegister::instance()->writeLogs();
    fprintf(stderr, "Fatal: %s\n", msg);

    abort();
  }
}

EventLoggerRegister::EventLoggerRegister(Behaviour behaviour)
{
  if (behaviour == InstallMsgHandler)
    qInstallMsgHandler(myMessageOutput);
}
