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
#include "quiloader_binding.h"

#include <QWidget>
#include <QtCore/QFile>
#include <QtCore/QDebug>

#include "qwidget_binding.h"
#include "qobject_binding.h"
#include "qaction_binding.h"
#include "qlayout_binding.h"
#include "static_binding.h"
#include "kjseglobal.h"

using namespace KJSEmbed;

KJSO_SIMPLE_BINDING_CTOR( UiLoaderBinding, QUiLoader, QObjectBinding )
KJSO_QOBJECT_BIND( UiLoaderBinding, QUiLoader )

KJSO_START_CTOR( UiLoaderBinding, QUiLoader, 1 )
{
  QUiLoader *uiLoader = 0;
  if ( args.size() == 0 )
  {
    uiLoader = new QUiLoader();
  }
  else if( args.size() == 1 )
  {
    QObject *arg0 = KJSEmbed::extractObject<QObject>(exec, args, 0, 0);
    uiLoader = new QUiLoader(arg0);
  }
  else
    return KJS::throwError(exec, KJS::GeneralError, i18n("Not enough arguments."));
  
  KJS::JSObject *uiLoaderBinding = new UiLoaderBinding( exec, uiLoader );
//  qDebug() << "UiLoaderBinding::CTOR() args.size()=" << args.size() << " uiLoader=" << uiLoader << " uiLoaderBinding=" << uiLoaderBinding;
  return uiLoaderBinding;
}
KJSO_END_CTOR

namespace UiLoaderNS {
START_QOBJECT_METHOD(createAction, QUiLoader )
{
  QObject* parent = KJSEmbed::extractObject<QObject>(exec, args, 0, 0);
  QString actionName = KJSEmbed::extractQString(exec, args, 1);
  QAction* action = object->createAction(parent, actionName);
  if ( action )
    return KJSEmbed::createQObject( exec, action );
  else
    return KJS::throwError(exec, KJS::GeneralError, i18n("Failed to create Action.") );
}
END_QOBJECT_METHOD

START_QOBJECT_METHOD(createActionGroup, QUiLoader )
  QObject* parent = KJSEmbed::extractObject<QObject>(exec, args, 0, 0);
  QString actionName = KJSEmbed::extractQString(exec, args, 1);
  QActionGroup* actionGroup = object->createActionGroup(parent, actionName);
  if ( actionGroup )
    return KJSEmbed::createQObject( exec, actionGroup );
  else
    return KJS::throwError(exec, KJS::GeneralError, i18n("Failed to create ActionGroup.") );
END_QOBJECT_METHOD

START_QOBJECT_METHOD(createLayout, QUiLoader )
{
  QString className = KJSEmbed::extractQString(exec, args, 0);
  if (className.isEmpty())
    return KJS::throwError(exec, KJS::SyntaxError, i18n("No classname specified"));
  QObject* parent = KJSEmbed::extractObject<QObject>(exec, args, 1, 0);
  QString name = KJSEmbed::extractQString(exec, args, 2);
  QLayout* layout = object->createLayout(className, parent, name);
  if ( layout )
    return KJSEmbed::createQObject( exec, layout );
  else
    return KJS::throwError(exec, KJS::GeneralError, i18n("Failed to create Layout.") );
}
END_QOBJECT_METHOD

START_QOBJECT_METHOD(createWidget, QUiLoader )
{
  QString className = KJSEmbed::extractQString(exec, args, 0);
  if (className.isEmpty())
    return KJS::throwError(exec, KJS::SyntaxError, i18n("No classname specified."));
  QWidget* parent = KJSEmbed::extractObject<QWidget>(exec, args, 1, 0);
  QString name = KJSEmbed::extractQString(exec, args, 2);
  QWidget* widget = object->createWidget(className, parent, name);
  if ( widget )
    return KJSEmbed::createQObject( exec, widget );
  else
    return KJS::throwError(exec, KJS::GeneralError, i18n("Failed to create Widget.") );
}
END_QOBJECT_METHOD

START_QOBJECT_METHOD(load, QUiLoader )
{
  QString fileName = KJSEmbed::extractQString(exec, args, 0);
  if (fileName.isEmpty())
    return KJS::throwError(exec, KJS::SyntaxError, i18n("Must supply a filename."));
  
  QFile uiFile(fileName);
  if (! uiFile.open(QIODevice::ReadOnly | QIODevice::Text) )
      return KJS::throwError(exec, KJS::GeneralError, i18n("Could not open file '%1': %2", fileName, uiFile.errorString() ) );
  
  QWidget* parent = KJSEmbed::extractObject<QWidget>(exec, args, 1, 0);

  QWidget* widget = object->load(&uiFile, parent);
  uiFile.close();
  if (! widget )
    return KJS::throwError(exec, KJS::GeneralError, i18n("Failed to load file '%1'", fileName));

  KJS::JSObject* result = KJSEmbed::createQObject( exec, widget );
//  qDebug() << "UiLoaderBinding::load(): fileName=" << fileName << "widget " << widget << " result=" << result << "(" << result->toString(exec).ascii() << ")";
  return result;  
}
END_QOBJECT_METHOD

START_QOBJECT_METHOD(pluginPaths, QUiLoader )
//  qDebug() << "UiLoader::pluginPaths(): " << object->pluginPaths();
  result = KJSEmbed::convertToValue( exec, QVariant(object->pluginPaths()) );
END_QOBJECT_METHOD

}

START_METHOD_LUT( UiLoaderBinding )
    {"createAction", 0, KJS::DontDelete|KJS::ReadOnly, &UiLoaderNS::createAction},
    {"createActionGroup", 0, KJS::DontDelete|KJS::ReadOnly, &UiLoaderNS::createActionGroup},
    {"createLayout", 1, KJS::DontDelete|KJS::ReadOnly, &UiLoaderNS::createLayout},
    {"createWidget", 1, KJS::DontDelete|KJS::ReadOnly, &UiLoaderNS::createWidget},
    {"load", 1, KJS::DontDelete|KJS::ReadOnly, &UiLoaderNS::load},
    {"load", 2, KJS::DontDelete|KJS::ReadOnly, &UiLoaderNS::load},
    {"pluginPaths", 0, KJS::DontDelete|KJS::ReadOnly, &UiLoaderNS::pluginPaths}
END_METHOD_LUT

NO_ENUMS( UiLoaderBinding )
NO_STATICS( UiLoaderBinding )


//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
