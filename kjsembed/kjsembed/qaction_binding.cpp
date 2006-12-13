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
#include "qaction_binding.h"
#include "static_binding.h"
#include "kjseglobal.h"

#include <kjs/object.h>
#include <qdebug.h>

#include <QAction>
#include <QtUiTools/QUiLoader>
#include <QtDebug>

namespace KJSEmbed
{
	QUiLoader *uiLoader();
}


using namespace KJSEmbed;

KJSO_SIMPLE_BINDING_CTOR( Action, QAction, QObjectBinding )

NO_METHODS( Action )
NO_ENUMS( Action )
NO_STATICS( Action )

START_CTOR( Action, QAction, 0 )
    QObject *parent = KJSEmbed::extractObject<QObject>(exec, args, 0, 0);
    QString actionName = KJSEmbed::extractQString(exec, args, 1);

    QAction *action = uiLoader()->createAction(parent, actionName);
    if( action )
    {
        KJS::JSObject *actionObject = new Action( exec, action );
        return actionObject;
    }
    else
    {
        return KJS::throwError(exec, KJS::GeneralError, i18n("Action takes 2 args."));
		}
END_CTOR


KJSO_SIMPLE_BINDING_CTOR( ActionGroup, QActionGroup, QObjectBinding )

NO_METHODS( ActionGroup )
NO_ENUMS( ActionGroup )
NO_STATICS( ActionGroup )

START_CTOR( ActionGroup, QActionGroup, 0 )
    if( args.size() == 2 )
    {
        QObject *parent = KJSEmbed::extractObject<QObject>(exec, args, 0, 0);
        QString actionName = KJSEmbed::extractQString(exec, args, 1);

        QActionGroup *action = uiLoader()->createActionGroup(parent, actionName);
        if( action )
        {
            KJS::JSObject *actionObject = new ActionGroup( exec, action );
            return actionObject;
        }
        else
        {
            return KJS::throwError(exec, KJS::GeneralError, i18n("ActionGroup takes 2 args."));
            // return KJSEmbed::throwError(exec, i18n("ActionGroup takes 2 args."));
        }
    }
    // Trow error incorrect args
    return KJS::throwError(exec, KJS::GeneralError, i18n("Must supply a valid parent."));
    // return KJSEmbed::throwError(exec, i18n("Must supply a valid parent."));
END_CTOR

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
