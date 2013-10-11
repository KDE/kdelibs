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
#include "qformbuilder_binding.h"

#include <QWidget>
#include <QtDesigner/QFormBuilder>
#include <QtCore/QFile>
#include <QtCore/QDebug>

#include <kjs/object.h>

#include "static_binding.h"
#include "qwidget_binding.h"
#include "kjseglobal.h"

using namespace KJSEmbed;
NO_METHODS( FormBuilder )
NO_ENUMS( FormBuilder )
NO_STATICS( FormBuilder )

START_CTOR( FormBuilder, Form, 2 )
    if( args.size() > 0 )
    {
        QFormBuilder *formBuilder = new QFormBuilder();
        QWidget *parentWidget = 0;
        KJSEmbed::QObjectBinding *parentImp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, args[1] );
        if( parentImp )
        {
            parentWidget = parentImp->object<QWidget>();
        }
        QString fileName = toQString(args[0]->toString(exec));
        QFile uiFile(fileName);
        if( uiFile.open(QIODevice::ReadOnly | QIODevice::Text) )
        {
            QWidget *returnWidget = formBuilder->load(&uiFile,parentWidget);
            uiFile.close();
            if( returnWidget == 0 )
            {
                delete formBuilder;
                return KJS::throwError(exec, KJS::GeneralError, i18n("There was an error reading the file '%1'",
                                    fileName));
            }
            KJS::JSObject *form = new QWidgetBinding( exec, returnWidget );
            delete formBuilder;
            return form;
        }
        delete formBuilder;
        return KJS::throwError(exec, KJS::GeneralError, i18n("Could not read file '%1'",
                                fileName));
    }
    return KJS::throwError(exec, KJS::GeneralError, i18n("Must supply a filename."));
END_CTOR

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
