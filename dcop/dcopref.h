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

#ifndef DCOPREF_H
#define DCOPREF_H

#include <qcstring.h>

class QDataStream;
class DCOPObject;

/**
 * If you want to return a reference to another DCOP
 * object in a DCOP interface, then you have to return
 * a DCOPRef object.
 *
 * @auther Torben Weis <weis@kde.org>
 */
class DCOPRef
{
public:
    /**
     * Creates a null reference.
     */
    DCOPRef();
    /**
     * Copy constructor.
     */
    DCOPRef( const DCOPRef& ref );
    /**
     * Creates a reference to a certain object in a
     * certain application.
     *
     * @param app is the name of an application as registered
     *            be the dcopserver.
     */
    DCOPRef( const QCString& app, const QCString& obj );

    /**
     * Tests wether it is a null reference.
     */
    bool isNull() const;

    /**
     * Name of the application in which the object resides.
     */
    QCString app() const;
    /**
     * Object ID of the referenced object.
     */
    QCString object() const;

    DCOPRef& operator=( const DCOPRef& );

    /**
     * Change the referenced object.
     */
    void setRef( const QCString& app, const QCString& obj );

    /**
     * Make it a null reference.
     */
    void clear();
    
private:
    QCString m_app;
    QCString m_obj;
};

/**
 * Writes the reference (NOT the object itself) to the stream.
 */
QDataStream& operator<<( QDataStream&, const DCOPRef& ref );
/**
 * Reads a reference from the stream.
 */
QDataStream& operator>>( QDataStream&, DCOPRef& ref );

#endif
