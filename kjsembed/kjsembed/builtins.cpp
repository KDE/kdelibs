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

#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QDebug>
#include <QMetaType>

#ifndef QT_ONLY
#include <kstandarddirs.h>
#endif // QT_ONLY


#include "variant_binding.h"
#include "object_binding.h"
#include "static_binding.h"
#include "kjsembed.h"
#include "builtins.h"

using namespace KJSEmbed;

KJS::JSValue *callExec( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED(exec);
    Q_UNUSED(self);
    Q_UNUSED(args);
    return KJS::Boolean( QApplication::exec() );
}

KJS::JSValue *callDump( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED(self);
    if( args.size() == 1)
    {
        KJS::JSObject *object = args[0]->toObject(exec);
    }
    return KJS::Null();
}

KJS::JSValue *callInclude( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED(self);
    if( args.size() == 1)
    {
        KJS::UString filename = args[0]->toString(exec);
        qDebug() << "include: " << filename.qstring();
        KJS::Completion c = Engine::runFile( exec->dynamicInterpreter(), filename );
        qDebug("\tCompletionType: %d", c.complType());

        if (c.isValueCompletion())
            return c.value();
    }
    else
    {
        return throwError(exec, KJS::URIError, 
                          i18n("include only takes 1 argument, not %1.", args.size()));
    }

    return KJS::Null();
}

#ifndef QT_ONLY

KJS::JSValue *callLibrary( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED(self);
    if( args.size() == 1)
    {
        KJS::UString filename = args[0]->toString(exec);
        QString qualifiedFilename = KStandardDirs::locate( "scripts", filename.qstring() );
        if ( !qualifiedFilename.isEmpty() )
            Engine::runFile( exec->dynamicInterpreter(), qualifiedFilename );
        else {
            QString msg = i18n( "File %1 not found.", filename.qstring() );
            return throwError( exec, KJS::URIError, msg );
        }
    }
    else {
        
    }

    return KJS::Null();
}

#endif // QT_ONLY

KJS::JSValue *callAlert( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED(self)
    if (args.size() == 1)
    {
        (*KJSEmbed::conerr()) << "callAlert";
        QString message = args[0]->toString(exec).qstring();
        QMessageBox::warning(0, "Alert", message, QMessageBox::Ok, QMessageBox::NoButton); 
    }
    return KJS::Null();
}

KJS::JSValue *callConfirm( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED(self)
    if (args.size() == 1)
    {
        QString message = args[0]->toString(exec).qstring();
        int result = QMessageBox::question (0, "Confirm", message, QMessageBox::Yes, QMessageBox::No);
        if (result == QMessageBox::Yes)
            return KJS::Boolean(true);
    }
    return KJS::Boolean(false);
}

KJS::JSValue *callIsVariantType( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED(self)
    if (args.size() == 1)
    {
        QString thetypename = args[0]->toString(exec).qstring();
        return KJS::Boolean( QMetaType::type( thetypename.toLatin1().data() ) );
    }
    return KJS::Boolean(false);
}

KJS::JSValue *callIsVariant( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED(self)
    if (args.size() == 1)
    {
        KJS::JSObject *obj = args[0]->toObject(exec);
        if (obj->inherits(&VariantBinding::info))
        {
            return KJS::Boolean(true);
        }
    }
    return KJS::Boolean(false);
}

KJS::JSValue *callIsObject( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED(self)
    if (args.size() == 1)
    {
        KJS::JSObject *obj = args[0]->toObject(exec);
        if (obj->inherits(&ObjectBinding::info))
        {
            return KJS::Boolean(true);
        }
    }
    return KJS::Boolean(false);
}

const Method BuiltinsFactory::BuiltinMethods[] =
{
    {"exec", 0, KJS::DontDelete|KJS::ReadOnly, &callExec},
    {"dump", 1, KJS::DontDelete|KJS::ReadOnly, &callDump},
    {"include", 1, KJS::DontDelete|KJS::ReadOnly, &callInclude},
#ifndef QT_ONLY
    {"library", 1, KJS::DontDelete|KJS::ReadOnly, &callLibrary},
#endif // QT_ONLY
    {"alert", 1, KJS::DontDelete|KJS::ReadOnly, &callAlert},
    {"confirm", 1, KJS::DontDelete|KJS::ReadOnly, &callConfirm},
    {"isVariantType", 1, KJS::DontDelete|KJS::ReadOnly, &callIsVariantType},
    {"isVariant", 1, KJS::DontDelete|KJS::ReadOnly, &callIsVariant},
    {"isObject", 1, KJS::DontDelete|KJS::ReadOnly, &callIsObject},
    {0, 0, 0, 0 }
};
//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
