/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 KJSEmbed Authors
    See included AUTHORS file.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "iosupport.h"
#include "static_binding.h"
#include "global.h"

#include <kjs/object.h>
#include <qdebug.h>

#include <QStringList>
#include <QProcess>

using namespace KJSEmbed;

KJS::JSValue *callPrint( KJS::ExecState *exec, KJS::JSObject */*self*/, const KJS::List &args )
{
    (*KJSEmbed::conout()) << args[0]->toString(exec).qstring();
    return KJS::Null();
}

KJS::JSValue * callPrintLn( KJS::ExecState *exec, KJS::JSObject */*self*/, const KJS::List &args )
{
    (*KJSEmbed::conout()) << args[0]->toString(exec).qstring() << endl;
    return KJS::Null();
}

KJS::JSValue * callDebug( KJS::ExecState *exec, KJS::JSObject */*self*/, const KJS::List &args )
{
    //(*KJSEmbed::conerr())  << "Debug: " << args[0]->toString(exec).qstring() << endl;
    qDebug()  << "Debug: " << args[0]->toString(exec).qstring();
    return KJS::Null();
}

KJS::JSValue * callReadLine( KJS::ExecState *exec, KJS::JSObject */*self*/, const KJS::List &args )
{
    QString line = conin()->readLine();
    return KJS::String( line );
}

KJS::JSValue * callSystem( KJS::ExecState *exec, KJS::JSObject */*self*/, const KJS::List &args )
{
    QProcess systemProcess;
    QStringList processArgs = args[0]->toString(exec).qstring().split( ' ' );
    QString app = processArgs[0];
    processArgs.pop_front();

    systemProcess.start( app, processArgs );
    if (!systemProcess.waitForStarted())
    {
        return KJS::throwError(exec, KJS::GeneralError, "Application could not start.");
        // return KJSEmbed::throwError(exec,"Application could not start.");
    }
    if (!systemProcess.waitForFinished())
    {
        return KJS::throwError(exec, KJS::GeneralError, "Application crashed.");
        // return KJSEmbed::throwError(exec,"Application crashed.");
    }
    return KJS::String( systemProcess.readAll().data() );
}

const Method IoFactory::IoMethods[] =
{
    {"debug", 1, KJS::DontDelete|KJS::ReadOnly, &callDebug },
    {"print", 1, KJS::DontDelete|KJS::ReadOnly, &callPrint },
    {"println", 1, KJS::DontDelete|KJS::ReadOnly, &callPrintLn },
    {"readln", 0, KJS::DontDelete|KJS::ReadOnly, &callReadLine },
    {"system", 1, KJS::DontDelete|KJS::ReadOnly, &callSystem },
    {0, 0, 0, 0 }
};

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
