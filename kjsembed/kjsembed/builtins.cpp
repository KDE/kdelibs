/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 Ian Reinhart Geiser <geiseri@kde.org>
    Copyright (C) 2005, 2006 Matt Broadstone <mbroadst@gmail.com>
    Copyright (C) 2005, 2006 Richard J. Moore <rich@kde.org>
    Copyright (C) 2005, 2006 Erik L. Bunce <kde@bunce.us>

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

#include "builtins.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QStandardPaths>
#include <QMessageBox>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>
#include <QtCore/QMetaType>

#include "variant_binding.h"
#include "object_binding.h"
#include "static_binding.h"
#include "kjsembed.h"

using namespace KJSEmbed;

KJS::JSValue *callExec( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED(exec);
    Q_UNUSED(self);
    Q_UNUSED(args);
    return KJS::jsBoolean( QCoreApplication::exec() );
}

KJS::JSValue *callDump( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED(self);
    if( args.size() == 1)
    {
        KJS::JSObject *object = args[0]->toObject(exec);
        Q_UNUSED(object);
    }
    return KJS::jsNull();
}

KJS::JSValue *callInclude( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED(self);
    if( args.size() == 1)
    {
        KJS::UString filename = args[0]->toString(exec);
        qDebug() << "include: " << toQString(filename);

        KJS::Completion c = Engine::runFile( exec->dynamicInterpreter(), filename );

        if ( c.complType() == KJS::Normal )
            return KJS::jsNull();

        if (c.complType() == KJS::ReturnValue)
        {
            if (c.isValueCompletion())
                return c.value();

            return KJS::jsNull();
        }

        if (c.complType() == KJS::Throw)
        {
            QString message = toQString(c.value()->toString(exec));
            int line = c.value()->toObject(exec)->get(exec, "line")->toUInt32(exec);
            return throwError(exec, KJS::EvalError,
                              toUString(i18n("Error encountered while processing include '%1' line %2: %3", toQString(filename), line, message)));
        }
    }
    else
    {
        return throwError(exec, KJS::URIError,
                          toUString(i18n("include only takes 1 argument, not %1.", args.size())));
    }

    return KJS::jsNull();
}

KJS::JSValue *callLibrary( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED(self);
    if( args.size() == 1)
    {
        KJS::UString filename = args[0]->toString(exec);
        QString qualifiedFilename = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "scripts/" + toQString(filename) );
        if ( !qualifiedFilename.isEmpty() )
        {
            KJS::Completion c = Engine::runFile( exec->dynamicInterpreter(), toUString(qualifiedFilename) );
            if ( c.complType() == KJS::Normal )
                return KJS::jsNull();

            if (c.complType() == KJS::ReturnValue)
            {
                if (c.isValueCompletion())
                    return c.value();

                return KJS::jsNull();
            }

            if (c.complType() == KJS::Throw)
            {
                QString message = toQString(c.value()->toString(exec));
                int line = c.value()->toObject(exec)->get(exec, "line")->toUInt32(exec);
                return throwError(exec, KJS::EvalError,
                                  toUString(i18n("Error encountered while processing include '%1' line %2: %3", toQString(filename), line, message)));
            }
        }
        else {
            QString msg = i18n( "File %1 not found.", toQString(filename) );
            return throwError( exec, KJS::URIError, toUString(msg) );
        }
    }
    else {
        return throwError(exec, KJS::URIError,
                          toUString(i18n("library only takes 1 argument, not %1.", args.size())));
    }

    return KJS::jsNull();
}

KJS::JSValue *callAlert( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED(self)
    if (args.size() == 1)
    {
        (*KJSEmbed::conerr()) << "callAlert";
        QString message = toQString(args[0]->toString(exec));
        QMessageBox::warning(0, i18n("Alert"), message, QMessageBox::Ok, QMessageBox::NoButton);
    }
    return KJS::jsNull();
}

KJS::JSValue *callConfirm( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED(self)
    if (args.size() == 1)
    {
        QString message = toQString(args[0]->toString(exec));
        int result = QMessageBox::question (0, i18n("Confirm"), message, QMessageBox::Yes, QMessageBox::No);
        if (result == QMessageBox::Yes)
            return KJS::jsBoolean(true);
    }
    return KJS::jsBoolean(false);
}

KJS::JSValue *callIsVariantType( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED(self)
    if (args.size() == 1)
    {
        QString thetypename = toQString(args[0]->toString(exec));
        return KJS::jsBoolean( QMetaType::type( thetypename.toLatin1().data() ) );
    }
    return KJS::jsBoolean(false);
}

KJS::JSValue *callIsVariant( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED(self)
    if (args.size() == 1)
    {
        KJS::JSObject *obj = args[0]->toObject(exec);
        if (obj->inherits(&VariantBinding::info))
        {
            return KJS::jsBoolean(true);
        }
    }
    return KJS::jsBoolean(false);
}

KJS::JSValue *callIsObject( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED(self)
    if (args.size() == 1)
    {
        KJS::JSObject *obj = args[0]->toObject(exec);
        if (obj->inherits(&ObjectBinding::info))
        {
            return KJS::jsBoolean(true);
        }
    }
    return KJS::jsBoolean(false);
}

const Method BuiltinsFactory::BuiltinMethods[] =
{
    {"exec", 0, KJS::DontDelete|KJS::ReadOnly, &callExec},
    {"dump", 1, KJS::DontDelete|KJS::ReadOnly, &callDump},
    {"include", 1, KJS::DontDelete|KJS::ReadOnly, &callInclude},
    {"library", 1, KJS::DontDelete|KJS::ReadOnly, &callLibrary},
    {"alert", 1, KJS::DontDelete|KJS::ReadOnly, &callAlert},
    {"confirm", 1, KJS::DontDelete|KJS::ReadOnly, &callConfirm},
    {"isVariantType", 1, KJS::DontDelete|KJS::ReadOnly, &callIsVariantType},
    {"isVariant", 1, KJS::DontDelete|KJS::ReadOnly, &callIsVariant},
    {"isObject", 1, KJS::DontDelete|KJS::ReadOnly, &callIsObject},
    {0, 0, 0, 0 }
};
//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
