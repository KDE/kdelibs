/*****************************************************************
Copyright (c) 1999 Torben Weis <weis@kde.org>
Copyright (c) 2000 Matthias Ettrich <ettrich@kde.org>

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
#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstring.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "main.h"

/**
 * Writes the stub implementation
 */
void generateStubImpl( const QString& idl, const QString& header, const QString& filename, QDomElement de )
{
    QFile impl( filename );
    if ( !impl.open( IO_WriteOnly ) )
	qFatal("Could not write to %s", filename.latin1() );

    QTextStream str( &impl );

    str << "/****************************************************************************" << endl;
    str << "**" << endl;
    str << "** DCOP Stub Implementation created by dcopidl2cpp from " << idl << endl;
    str << "**" << endl;
    str << "** WARNING! All changes made in this file will be lost!" << endl;
    str << "**" << endl;
    str << "*****************************************************************************/" << endl;
    str << endl;

    str << "#include <" << header  << ">" << endl;
    str << "#include <dcopclient.h>" << endl << endl;
    str << "#include <kapp.h>" << endl << endl;
    str << "#include <kdatastream.h>" << endl << endl;

    QDomElement e = de.firstChild().toElement();
    for( ; !e.isNull(); e = e.nextSibling().toElement() ) {
	if ( e.tagName() == "CLASS" ) {
	    QDomElement n = e.firstChild().toElement();
	    ASSERT( n.tagName() == "NAME" );
	    QString className = n.firstChild().toText().data() + "_stub";
	
	    // find dcop parent ( rightmost super class )
	    QString DCOPParent;
	    QDomElement s = n.nextSibling().toElement();
	    for( ; !s.isNull(); s = s.nextSibling().toElement() ) {
		if ( s.tagName() == "SUPER" )
		    DCOPParent = s.firstChild().toText().data();
	    }
	
	    // Write constructor
	    str << className << "::" << className << "( const QCString& app, const QCString& obj )" << endl;
	    str << "  : ";
	
	    if ( DCOPParent.isEmpty() || DCOPParent == "DCOPObject" )
		str << "DCOPStub( app, obj )" << endl;
	    else
		str << DCOPParent << "( app, obj )" << endl;

	    str << "{" << endl;
	    str << "}" << endl << endl;

	    // Write marshalling code
	    s = e.firstChild().toElement();
	    for( ; !s.isNull(); s = s.nextSibling().toElement() ) {
		if ( s.tagName() == "FUNC" ) {
		    QDomElement r = s.firstChild().toElement();
		    ASSERT( r.tagName() == "TYPE" );
		    QString result = r.firstChild().toText().data();
		    bool async = result == "ASYNC";
		    if ( async)
			result = "void";
		    if ( r.hasAttribute( "qleft" ) )
			str << r.attribute("qleft") << " ";
		    str << result;
		    if ( r.hasAttribute( "qright" ) )
			str << r.attribute("qright") << " ";
		    else
			str << " ";

		    r = r.nextSibling().toElement();
		    ASSERT ( r.tagName() == "NAME" );
		    QString funcName = r.firstChild().toText().data();
		    str << className << "::" << funcName << "(";

		    QStringList args;
		    QStringList argtypes;
		    bool first = TRUE;
		    r = r.nextSibling().toElement();
		    for( ; !r.isNull(); r = r.nextSibling().toElement() ) {
			if ( !first )
			    str << ", ";
			else
			    str << " ";
			first = FALSE;
			ASSERT( r.tagName() == "ARG" );
			QDomElement a = r.firstChild().toElement();
			ASSERT( a.tagName() == "TYPE" );
			if ( a.hasAttribute( "qleft" ) )
			    str << a.attribute("qleft") << " ";
			argtypes.append( a.firstChild().toText().data() );
			str << argtypes.last();
			if ( a.hasAttribute( "qright" ) )
			    str << a.attribute("qright") << " ";
			else
			    str << " ";
			args.append( QString("arg" ) + QString::number( args.count() ) ) ;
			str << args.last();
		    }
		    if ( !first )
			str << " ";
		    str << ")";

		    if ( s.hasAttribute("qual") )
			str << " " << s.attribute("qual");
		    str << endl;
		
		    str << "{" << endl ;

		
		    funcName += "(";
		    first = TRUE;
		    for( QStringList::Iterator it = argtypes.begin(); it != argtypes.end(); ++it ){
			if ( !first )
			    funcName += ",";
			first = FALSE;
			funcName += *it;
		    }
		    funcName += ")";
		
		    if ( async ) {
			str << "    QByteArray data;" << endl;
			if ( !args.isEmpty() ) {
			    str << "    QDataStream arg( data, IO_WriteOnly );" << endl;
			    for( QStringList::Iterator args_count = args.begin(); args_count != args.end(); ++args_count ){
				str << "    arg << " << *args_count << ";" << endl;
			    }
			}
			str << "    kapp->dcopClient()->send( app(), obj(), \"" << funcName << "\", data );" << endl;
			str << "    setStatus( CallSucceeded );" << endl;
		    } else {
		
			if ( result != "void" )
			    str << "    " << result << " result;" << endl;

			str << "    QByteArray data, replyData;" << endl;
			str << "    QCString replyType;" << endl;
		
			if ( !args.isEmpty() ) {
			    str << "    QDataStream arg( data, IO_WriteOnly );" << endl;
			    for( QStringList::Iterator args_count = args.begin(); args_count != args.end(); ++args_count ){
				str << "    arg << " << *args_count << ";" << endl;
			    }
			}
			str << "    if ( kapp->dcopClient()->call( app(), obj(), \"" << funcName << "\",";
			str << " data, replyType, replyData ) ) {" << endl;
			if ( result != "void" ) {
			    str << "\tif ( replyType == \"" << result << "\" ) {" << endl;
			    str << "\t    QDataStream _reply_stream( replyData, IO_ReadOnly );"  << endl;
			    str << "\t    _reply_stream >> result;" << endl;
			    str << "\t    setStatus( CallSucceeded );" << endl;
			    str << "\t} else {" << endl;
			    str << "\t    callFailed();" << endl;
			    str << "\t}" << endl;
			} else {
			    str << "\tsetStatus( CallSucceeded );" << endl;
			}
			str << "    } else { " << endl;
			str << "\tcallFailed();" << endl;
			str << "    }" << endl;
			if ( result != "void" )
			    str << "    return result;" << endl;
		    }
		    str << "}" << endl << endl;
		}
	    }
	}
    }
    impl.close();
}
