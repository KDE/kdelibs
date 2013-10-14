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
#include "kjsembed.h"
#include "binding_support.h"

#include "qobject_binding.h"
#include "variant_binding.h"
#include "static_binding.h"

#include "iosupport.h"
#include "quiloader_binding.h"
#ifdef KJSEMBED_FORMBUILDER_BINDING
#include "qformbuilder_binding.h"
#endif
#include "qpainter_binding.h"
#include "qwidget_binding.h"
#include "qaction_binding.h"
#include "qlayout_binding.h"
#include "svg_binding.h"
#include "filedialog_binding.h"
#include "settings.h"
#include "fileio.h"
#include "color.h"
#include "rect.h"
#include "size.h"
#include "point.h"
#include "image.h"
#include "pixmap.h"
#include "brush.h"
#include "pen.h"
#include "font.h"
#include "dom.h"
#include "url.h"
#include "application.h"

#include "builtins.h"

#include <kjs/interpreter.h>
#include <kjs/completion.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QObject>

#include <QtCore/QDebug>


/**
* Implement QString-KJS::UString conversion methods. These methods are declared
* by KJS, but libkjs doesn't actually contain their implementations.
* because we link against khtml , those functions are already implemented there.
*
*/
namespace KJS {
#ifndef QTONLY_WEBKIT
    UString::UString( const QString &d )
    {
        uint len = d.length();
        UChar *dat = static_cast<UChar*>(fastMalloc(sizeof(UChar)*len));
        memcpy( dat, d.unicode(), len * sizeof(UChar) );
        m_rep = UString::Rep::create(dat, len);
    }

    QString UString::qstring() const
    {
        return QString((QChar*) data(), size());
    }

    QString Identifier::qstring() const
    {
        return QString((QChar*) data(), size());
    }
#endif
}

namespace KJSEmbed {

class EnginePrivate {
public:
    EnginePrivate ( )
    {
        m_interpreter = new KJS::Interpreter( );
        m_interpreter->initGlobalObject();
	m_interpreter->ref();
    }
    ~EnginePrivate()
    {
        m_interpreter->deref();
    }
    KJS::Interpreter *m_interpreter;
    KJS::Completion m_currentResult;
    bool m_bindingsEnabled;
};

void setup( KJS::ExecState *exec, KJS::JSObject *parent )
{
    StaticBinding::publish( exec, parent, IoFactory::methods() ); // Global methods
    StaticBinding::publish( exec, parent, FileDialog::methods() ); // Global methods
    StaticBinding::publish( exec, parent, BuiltinsFactory::methods() ); // Global methods
    StaticConstructor::add( exec, parent, FileIO::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, DomNode::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, DomDocument::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, DomElement::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, DomAttr::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, DomDocumentType::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, DomNodeList::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, DomNamedNodeMap::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, DomText::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, Url::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, SettingsBinding::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, CoreApplicationBinding::constructor() );
    StaticConstructor::add( exec, parent, Point::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, Size::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, Rect::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, Color::constructor() ); // Ctor

    // check if this is a GUI application
    QApplication* app = ::qobject_cast<QApplication*>(QCoreApplication::instance());
    if (app)
    {
        //qDebug("Loading GUI Bindings");

#ifdef KJSEMBED_FORMBUILDER_BINDING
        StaticConstructor::add( exec, parent, FormBuilder::constructor() ); // Ctor
#endif
        StaticConstructor::add( exec, parent, UiLoaderBinding::constructor() ); // Ctor
        StaticConstructor::add( exec, parent, QWidgetBinding::constructor() ); // Ctor
        StaticConstructor::add( exec, parent, Layout::constructor() ); // Ctor
        StaticConstructor::add( exec, parent, Action::constructor() ); // Ctor
        StaticConstructor::add( exec, parent, ActionGroup::constructor() ); // Ctor
        StaticConstructor::add( exec, parent, Font::constructor() ); // Ctor
        StaticConstructor::add( exec, parent, Pen::constructor() ); // Ctor
        StaticConstructor::add( exec, parent, Brush::constructor() ); // Ctor
        StaticConstructor::add( exec, parent, Image::constructor() ); // Ctor
        StaticConstructor::add( exec, parent, Pixmap::constructor() ); // Ctor
        StaticConstructor::add( exec, parent, Painter::constructor() ); // Ctor
        StaticConstructor::add( exec, parent, SvgRenderer::constructor() ); // Ctor
        StaticConstructor::add( exec, parent, SvgWidget::constructor() ); // Ctor
        StaticConstructor::add( exec, parent, ApplicationBinding::constructor() );
    }
}

Engine::Engine( bool enableBindings )
{
    dptr = new EnginePrivate( );
    if ( enableBindings )
        setup( dptr->m_interpreter->globalExec(), dptr->m_interpreter->globalObject() );
    dptr->m_bindingsEnabled =  enableBindings;
}

Engine::~Engine()
{
    delete dptr;
}

bool Engine::isBindingsEnabled() const
{
    return dptr->m_bindingsEnabled;
}

KJS::JSObject *Engine::addObject( QObject *obj, KJS::JSObject *parent, const KJS::UString &name ) const
{
    KJS::ExecState *exec = dptr->m_interpreter->globalExec();
    KJS::JSObject *returnObject = KJSEmbed::createQObject(exec , obj, KJSEmbed::ObjectBinding::CPPOwned );
    KJS::Identifier jsName( !name.isEmpty() ? name : toUString(obj->objectName()) );

    parent->putDirect(jsName, returnObject, KJS::DontDelete|KJS::ReadOnly );
    return returnObject;
}

KJS::JSObject *Engine::addObject( QObject *obj, const KJS::UString &name ) const
{
    return addObject( obj, dptr->m_interpreter->globalObject(), name );
}

KJS::Completion Engine::completion() const
{
    return dptr->m_currentResult;
}

KJS::Interpreter *Engine::interpreter() const
{
    return dptr->m_interpreter;
}

KJS::Completion Engine::runFile( KJS::Interpreter *interpreter, const KJS::UString &fileName )
{
//    qDebug() << "runFile: " << toQString(fileName);
    KJS::UString code;
    QFile file( toQString(fileName) );
    if( file.open( QFile::ReadOnly ) )
    {
        QTextStream ts( &file );

        QString line;
        while( !ts.atEnd() )
        {
            line = ts.readLine();
            if( line[0] != '#' ) code += toUString(line + '\n');
        }
        file.close();
    }
    else
    {
        code = "println('Could not open file.');";
        qWarning() << "Could not open file " << toQString(fileName);
    }

//    qDebug() << "Loaded code: " << toQString(code);

    return interpreter->evaluate( fileName, 0, code, 0 );
}

Engine::ExitStatus Engine::runFile( const KJS::UString &fileName )
{
   dptr->m_currentResult = runFile( dptr->m_interpreter, fileName );

   if( dptr->m_currentResult.complType() == KJS::Normal )
     return Engine::Success;
   else if ( dptr->m_currentResult.complType() == KJS::ReturnValue)
     return Engine::Success;
   else
     return Engine::Failure;
}

Engine::ExitStatus Engine::execute( const KJS::UString &code )
{
    dptr->m_currentResult = dptr->m_interpreter->evaluate(KJS::UString(""), 0, code, 0);
    if( dptr->m_currentResult.complType() == KJS::Normal )
        return Engine::Success;
    else if ( dptr->m_currentResult.complType() == KJS::ReturnValue)
        return Engine::Success;
    else
        return Engine::Failure;
}

KJS::JSObject *Engine::construct( const KJS::UString &className, const KJS::List &args ) const
{
    KJS::JSObject *global = dptr->m_interpreter->globalObject();
    KJS::ExecState *exec = dptr->m_interpreter->globalExec();
    return StaticConstructor::construct( exec, global, className, args );
}

KJS::JSValue *Engine::callMethod( const KJS::UString &methodName, const KJS::List &args )
{
    KJS::JSObject *global = dptr->m_interpreter->globalObject();
    KJS::ExecState *exec = dptr->m_interpreter->globalExec();

    KJS::Identifier id = KJS::Identifier( KJS::UString( methodName ) );
    KJS::JSObject *fun = global->get( exec, id )->toObject( exec );
    KJS::JSValue *retValue;

    if ( !fun->implementsCall() ) {
	QString msg = i18n( "%1 is not a function and cannot be called.", toQString(methodName) );
	return throwError( exec, KJS::TypeError, msg );
    }

    retValue = fun->call( exec, global, args );

    if( exec->hadException() )
	return exec->exception();

    return retValue;
}

KJS::JSValue *Engine::callMethod(  KJS::JSObject *parent,
				   const KJS::UString &methodName, const KJS::List &args )
{
    KJS::ExecState *exec = dptr->m_interpreter->globalExec();

    KJS::Identifier id = KJS::Identifier( methodName);
    KJS::JSObject *fun = parent->get( exec, id )->toObject( exec );
    KJS::JSValue *retValue;

    if ( !fun->implementsCall() ) {
	QString msg = i18n( "%1 is not a function and cannot be called.", toQString(methodName) );
	return throwError( exec, KJS::TypeError, msg );
    }

    retValue = fun->call( exec, parent, args );

    if( exec->hadException() )
	return exec->exception();

    return retValue;
}


} // namespace KJS

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
