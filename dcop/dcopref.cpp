/*****************************************************************

Copyright (c) 1999 Preston Brown <pbrown@kde.org>
Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>
 
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
 
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
 
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
******************************************************************/

#include "dcopref.h"
#include "dcopclient.h"
#include "dcopobject.h"

#include <qdatastream.h>

#include <kapp.h>

DCOPRef::DCOPRef()
{
}

DCOPRef::DCOPRef( const DCOPRef& ref )
{
    m_app = ref.app();
    m_obj = ref.object();
}

DCOPRef::DCOPRef( const QCString& app, const QCString& obj )
    : m_app( app ), m_obj( obj )
{
}

bool DCOPRef::isNull() const
{
    return ( m_app.isEmpty() || m_obj.isEmpty() );
}

QCString DCOPRef::app() const
{
    return m_app;
}

QCString DCOPRef::object() const
{
    return m_obj;
}

DCOPRef& DCOPRef::operator=( const DCOPRef& ref )
{
    m_app = ref.app();
    m_obj = ref.object();

    return *this;
}

void DCOPRef::setRef( const QCString& app, const QCString& obj )
{
    m_app = app;
    m_obj = obj;
}

void DCOPRef::clear()
{
    m_app = "";
    m_obj = "";
}

QDataStream& operator<<( QDataStream& str, const DCOPRef& ref )
{
    str << ref.app();
    str << ref.object();

    return str;
}

QDataStream& operator>>( QDataStream& str, DCOPRef& ref )
{
    QCString a, o;
    str >> a >> o;

    ref.setRef( a, o );

    return str;
}
