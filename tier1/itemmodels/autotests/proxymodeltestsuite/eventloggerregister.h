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

#ifndef EVENTLOGGERREGISTER_H
#define EVENTLOGGERREGISTER_H

#include <QVector>

#include <memory>

#include "proxymodeltestsuite_export.h"

class ModelEventLogger;

class PROXYMODELTESTSUITE_EXPORT EventLoggerRegister
{
public:
  enum Behaviour {
    InstallMsgHandler,
    NoInstallMsgHandler
  };
  static EventLoggerRegister* instance(Behaviour behaviour = InstallMsgHandler);

  void registerLogger(ModelEventLogger *logger);
  void unregisterLogger(ModelEventLogger *logger);

  void writeLogs();

private:
  EventLoggerRegister(Behaviour behaviour);
  ~EventLoggerRegister();
  QVector<ModelEventLogger*> m_loggers;

  static EventLoggerRegister *s_instance;
  friend class std::auto_ptr<EventLoggerRegister>;

  static std::auto_ptr<EventLoggerRegister> s_destroyer;
  Q_DISABLE_COPY(EventLoggerRegister)
};

#endif
