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
#include "kjsembed.h"
#include "binding_support.h"

#include "qobject_binding.h"
#include "value_binding.h"
#include "static_binding.h"

#include "iosupport.h"
#include "qformbuilder_binding.h"
#include "qpainter_binding.h"
#include "qwidget_binding.h"
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
#include "bind_qlcdnumber.h"
#include "bind_qtimer.h"
#include "application.h"

#include "builtins.h"

#include <kjs/interpreter.h>
#include <kjs/completion.h>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QObject>

#include <qdebug.h>


/**
* Implement QString-KJS::UString conversion methods. These methods are declared
* by KJS, but libkjs doesn't actually contain their implementations.
* because we link against khtml , those functions are already implemented there.
*
*/
namespace KJS {

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
};

void setup( KJS::ExecState *exec, KJS::JSObject *parent )
{
    StaticBinding::publish( exec, parent, IoFactory::methods() ); // Global methods
    StaticBinding::publish( exec, parent, FileDialog::methods() ); // Global methods
    StaticBinding::publish( exec, parent, BuiltinsFactory::methods() ); // Global methods

    StaticConstructor::add( exec, parent, FormBuilder::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, Widget::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, Layout::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, Action::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, Font::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, Pen::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, Brush::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, Image::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, Pixmap::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, Point::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, Size::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, Rect::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, Color::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, Painter::constructor() ); // Ctor
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
    StaticConstructor::add( exec, parent, Settings::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, SvgRenderer::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, SvgWidget::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, LCDNumber::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, Timer::constructor() ); // Ctor
    StaticConstructor::add( exec, parent, CoreApplication::constructor() );
}

Engine::Engine()
{
    dptr = new EnginePrivate( );
    setup( dptr->m_interpreter->globalExec(), dptr->m_interpreter->globalObject() );
}

Engine::~Engine()
{
    delete dptr;
}

KJS::JSObject *Engine::addObject( QObject *obj, KJS::JSObject *parent, const KJS::UString &name ) const
{
    KJS::ExecState *exec = dptr->m_interpreter->globalExec();
    KJS::JSObject *returnObject = KJSEmbed::createQObject(exec , obj, KJSEmbed::ObjectBinding::CPPOwned );
    KJS::Identifier jsName( !name.isEmpty() ? name : obj->objectName() );

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
    KJS::UString code;
    QFile file( fileName.qstring() );
    if( file.open( QFile::ReadOnly ) )
    {
        QTextStream ts( &file );

        QString line;
        while( !ts.atEnd() )
        {
            line = ts.readLine();
            if( line[0] != '#' ) code += line + "\n";
        }
        file.close();
    }
    else
    {
        code = "println('Could not open file.');";
        qWarning() << "Could not open file " << fileName.qstring();
    }
    
    return interpreter->evaluate( KJS::UString(""), 0, code, 0 );
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
}

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
