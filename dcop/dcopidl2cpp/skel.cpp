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

static int const primes[] =
{
    2,  3,  5,  7, 11, 13, 17, 19, 23, 29,
    31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
    127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
    179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
    233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
    283, 293, 307, 311, 313, 317, 331, 337, 347, 349,
    353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
    419, 421, 431, 433, 439, 443, 449, 457, 461, 463,
    467, 479, 487, 491, 499, 503, 509, 521, 523, 541,
    547, 557, 563, 569, 571, 577, 587, 593, 599, 601,0
};

/**
 * Writes the skeleton
 */
void generateSkel( const QString& idl, const QString& filename, QDomElement de )
{
    QFile skel( filename );
    if ( !skel.open( IO_WriteOnly ) )
	qFatal("Could not write to %s", filename.latin1() );

    QTextStream str( &skel );

    str << "/****************************************************************************" << endl;
    str << "**" << endl;
    str << "** DCOP Skeleton created by dcopidl2cpp from " << idl << endl;
    str << "**" << endl;
    str << "** WARNING! All changes made in this file will be lost!" << endl;
    str << "**" << endl;
    str << "*****************************************************************************/" << endl;
    str << endl;

    QDomElement e = de.firstChild().toElement();
    if ( e.tagName() == "SOURCE" ) {
	str << "#include \"" << e.firstChild().toText().data() << "\"" << endl << endl;
    }

    for( ; !e.isNull(); e = e.nextSibling().toElement() ) {
	if ( e.tagName() == "CLASS" ) {
	    QDomElement n = e.firstChild().toElement();
	    ASSERT( n.tagName() == "NAME" );
	    QString className = n.firstChild().toText().data();
	    // find dcop parent ( rightmost super class )
	    QString DCOPParent;
	    QDomElement s = n.nextSibling().toElement();
	    for( ; !s.isNull(); s = s.nextSibling().toElement() ) {
		if ( s.tagName() == "SUPER" )
		    DCOPParent = s.firstChild().toText().data();
	    }
	
	    // get function table
	    QStringList funcNames;
	    s = n.nextSibling().toElement();
	    for( ; !s.isNull(); s = s.nextSibling().toElement() ) {
		if ( s.tagName() == "FUNC" ) {
		    QDomElement r = s.firstChild().toElement();
		    ASSERT( r.tagName() == "TYPE" );
		    QString result = r.firstChild().toText().data();
		    r = r.nextSibling().toElement();
		    ASSERT ( r.tagName() == "NAME" );
		    QString funcName = r.firstChild().toText().data();
		    QStringList argtypes;
		    r = r.nextSibling().toElement();
		    for( ; !r.isNull(); r = r.nextSibling().toElement() ) {
			ASSERT( r.tagName() == "ARG" );
			QDomElement a = r.firstChild().toElement();
			ASSERT( a.tagName() == "TYPE" );
			argtypes.append( a.firstChild().toText().data() );
		    }
		    funcName += "(";
		    bool first = TRUE;
		    for( QStringList::Iterator it = argtypes.begin(); it != argtypes.end(); ++it ){
			if ( !first )
			    funcName += ",";
			first = FALSE;
			funcName += *it;
		    }
		    funcName += ")";
		    funcNames.append( funcName );
		}
	    }

	    // create static tables
	
	    int fhash = funcNames.count() + 1;
	    for ( int i = 0; primes[i]; i++ ) {
		if ( primes[i] > (int) funcNames.count() ) {
		    fhash = primes[i];
		    break;
		}
	    }
	
	    str << "#include <kdatastream.h>" << endl;

	    bool useHashing = funcNames.count() > 7;
	    if ( useHashing ) {
		str << "#include <qasciidict.h>" << endl;
		str << "static const int " << className << "_fhash = " << fhash << ";" << endl;
	    }
	    str << "static const char* const " << className << "_ftable[ " << funcNames.count() + 1 << " ] = {" << endl;
	    for( QStringList::Iterator it = funcNames.begin(); it != funcNames.end(); ++it ){
		str << "    \"" << *it << "\"," << endl;
	    }
	    str << "    0" << endl;
	    str << "};" << endl;
	
	    str << endl;
	
	
	    // Write dispatcher
	    str << "bool " << className;
	    str << "::process(const QCString &fun, const QByteArray &data, QCString& replyType, QByteArray &replyData)" << endl;
	    str << "{" << endl;
	    if ( useHashing ) {
		str << "    static QAsciiDict<int>* fdict = 0;" << endl;
	
		str << "    if ( !fdict ) {" << endl;
		str << "\tfdict = new QAsciiDict<int>( " << className << "_fhash, TRUE, FALSE );" << endl;
		str << "\tfor ( int i = 0; " << className << "_ftable[i]; i++ )" << endl;
		str << "\t    fdict->insert( " << className << "_ftable[i],  new int( i ) );" << endl;
		str << "    }" << endl;
	
		str << "    int* fp = fdict->find( fun );" << endl;
		str << "    switch ( fp?*fp:-1) {" << endl;
	    }
	    s = n.nextSibling().toElement();
	    int fcount = 0;
	    bool firstFunc = TRUE;
	    for( ; !s.isNull(); s = s.nextSibling().toElement() ) {
		if ( s.tagName() == "FUNC" ) {
		    QDomElement r = s.firstChild().toElement();
		    ASSERT( r.tagName() == "TYPE" );
		    QString result = r.firstChild().toText().data();
		    if ( result == "ASYNC" )
			result = "void";
		    r = r.nextSibling().toElement();
		    ASSERT ( r.tagName() == "NAME" );
		    QString funcName = r.firstChild().toText().data();
		    QStringList args;
		    QStringList argtypes;
		    r = r.nextSibling().toElement();
		    for( ; !r.isNull(); r = r.nextSibling().toElement() ) {
			ASSERT( r.tagName() == "ARG" );
			QDomElement a = r.firstChild().toElement();
			ASSERT( a.tagName() == "TYPE" );
			argtypes.append( a.firstChild().toText().data() );
			args.append( QString("arg" ) + QString::number( args.count() ) );
		    }
		    QString plainFuncName = funcName;
		    funcName += "(";
		    bool first = TRUE;
		    for( QStringList::Iterator it = argtypes.begin(); it != argtypes.end(); ++it ){
			if ( !first )
			    funcName += ",";
			first = FALSE;
			funcName += *it;
		    }
		    funcName += ")";
			
		    if ( useHashing ) {
			str << "    case " << fcount++ << ": { // " << funcName << endl;
		    } else {
			if ( firstFunc )
			    str << "    if ( fun == " << className << "_ftable[" << fcount++ << "] ) {" << endl;
			else
			    str << " else if ( fun == " << className << "_ftable[" << fcount++ << "] ) {" << endl;
			firstFunc = FALSE;
		    }
		    if ( !args.isEmpty() ) {
			QStringList::Iterator ittypes = argtypes.begin();
			for( QStringList::Iterator it = args.begin(); it != args.end(); ++it ){
			    str << "\t"<< *ittypes << " " << *it << ";" <<  endl;
			    ++ittypes;
			}
			str << "\tQDataStream arg( data, IO_ReadOnly );" << endl;
			for( QStringList::Iterator it = args.begin(); it != args.end(); ++it ){
			    str << "\targ >> " << *it << ";" << endl;
			}
		    }

		    str << "\treplyType = \"" << result << "\";" << endl;
		    if ( result == "void" ) {
			str << "\t" << plainFuncName << "(";
		    } else {
			str << "\tQDataStream _reply_stream( replyData, IO_WriteOnly );"  << endl;
			str << "\t_reply_stream << " << plainFuncName << "(";
		    }

		    first = TRUE;
		    for( QStringList::Iterator it = args.begin(); it != args.end(); ++it ){
			if ( !first )
			    str << ", ";
			first = FALSE;
			str << *it;
		    }
		    str << " );" << endl;
		    if (useHashing ) {
			str << "    } break;" << endl;
		    } else {
			str << "    }";
		    }
		}
	    }
	    if ( useHashing ) {
		str << "    default: " << endl;
	    } else {
		str << " else {" << endl;
	    }
	    if (!DCOPParent.isEmpty()) {
		str << "\treturn " << DCOPParent << "::process( fun, data, replyType, replyData );" << endl;
	    } else {
		str << "\treturn FALSE;" << endl;
	    }
	    str << "    }" << endl;
	    str << "    return TRUE;" << endl;
	    str << "}" << endl << endl;
	
	    str << "QCString " << className;
	    str << "::functions()" << endl;
	    str << "{" << endl;
	    if (!DCOPParent.isEmpty()) {
		str << "    QCString s = " << DCOPParent << "::functions();" << endl;
	    } else {
		str << "    QCString s;" << endl;
	    }
	    str << "    for ( int i = 0; " << className << "_ftable[i]; i++ ) {" << endl;
	    str << "\ts += " << className << "_ftable[i];" << endl;
	    str << "\ts += ';';" << endl;
	    str << "    };" << endl;
	    str << "    return s;" << endl;
	    str << "}" << endl << endl;
	}
    }
	
    skel.close();
}
