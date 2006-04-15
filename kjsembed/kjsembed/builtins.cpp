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

#include "static_binding.h"
#include "kjsembed.h"
#include "builtins.h"

using namespace KJSEmbed;

KJS::JSValue *callExec( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
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
        Engine::runFile( exec->interpreter(), filename );
    }
    return KJS::Null();
}

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

const Method BuiltinsFactory::BuiltinMethods[] =
{
    {"exec", 0, KJS::DontDelete|KJS::ReadOnly, &callExec},
    {"dump", 1, KJS::DontDelete|KJS::ReadOnly, &callDump},
    {"include", 1, KJS::DontDelete|KJS::ReadOnly, &callInclude},
    {"alert", 1, KJS::DontDelete|KJS::ReadOnly, &callAlert},
    {"confirm", 1, KJS::DontDelete|KJS::ReadOnly, &callConfirm},
    {"isVariantType", 1, KJS::DontDelete|KJS::ReadOnly, &callIsVariantType},
    {0, 0, 0, 0 }
};
