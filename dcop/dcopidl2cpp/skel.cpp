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
#include <qstringlist.h>

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


struct Function
{
    Function(){};
    Function( const QString& t, const QString& n, const QString&fn ) : type( t ), name( n ), fullName( fn ){}
    QString type;
    QString name;
    QString fullName;
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
	    QValueList<Function> functions;
	    s = n.nextSibling().toElement();
	    for( ; !s.isNull(); s = s.nextSibling().toElement() ) {
		if ( s.tagName() == "FUNC" ) {
		    QDomElement r = s.firstChild().toElement();
		    ASSERT( r.tagName() == "TYPE" );
		    QString funcType = r.firstChild().toText().data();
		    r = r.nextSibling().toElement();
		    ASSERT ( r.tagName() == "NAME" );
		    QString funcName = r.firstChild().toText().data();
		    QStringList argtypes;
		    QStringList argnames;
		    r = r.nextSibling().toElement();
		    for( ; !r.isNull(); r = r.nextSibling().toElement() ) {
			ASSERT( r.tagName() == "ARG" );
			QDomElement a = r.firstChild().toElement();
			ASSERT( a.tagName() == "TYPE" );
			argtypes.append( a.firstChild().toText().data() );
			a = a.nextSibling().toElement();
			if ( !a.isNull() ) {
			    ASSERT( a.tagName() == "NAME" );
			    argnames.append( a.firstChild().toText().data() );
			} else {
			    argnames.append( QString::null );
			}
		    }
		    funcName += '(';
		    QString fullFuncName = funcName;
		    bool first = TRUE;
		    QStringList::Iterator ittype = argtypes.begin();
		    QStringList::Iterator itname = argnames.begin();
		    while ( ittype != argtypes.end() && itname != argnames.end() ) {
			if ( !first ) {
			    funcName += ',';
			    fullFuncName += ',';
			}
			first = FALSE;
			funcName += *ittype;
			fullFuncName += *ittype;
			if ( ! (*itname).isEmpty() ) {
			    fullFuncName += ' ';
			    fullFuncName += *itname;
			}
			++ittype;
			++itname;
		    }
		    funcName += ')';
		    fullFuncName += ')';
		    functions.append( Function( funcType, funcName, fullFuncName ) );
		}
	    }

	    // create static tables
	
	    int fhash = functions.count() + 1;
	    for ( int i = 0; primes[i]; i++ ) {
		if ( primes[i] >  static_cast<int>(functions.count()) ) {
		    fhash = primes[i];
		    break;
		}
	    }
	
	    str << "#include <kdatastream.h>" << endl;

	    bool useHashing = functions.count() > 7;
	    if ( useHashing ) {
		str << "#include <qasciidict.h>" << endl;
	    }

            QString classNameFull = className; // class name with possible namespaces prepended
                                               // namespaces will be removed from className now
            int namespace_count = 0;
            QString namespace_tmp = className;
            str << endl;
            for(;;) {
                int pos = namespace_tmp.find( "::" );
                if( pos < 0 )
                    {
                    className = namespace_tmp;
                    break;
                    }
                str << "namespace " << namespace_tmp.left( pos ) << " {" << endl;
                ++namespace_count;
                namespace_tmp = namespace_tmp.mid( pos + 2 );
            }

            str << endl;

	    if ( useHashing ) {
		str << "static const int " << className << "_fhash = " << fhash << ";" << endl;
	    }
	    str << "static const char* const " << className << "_ftable[" << functions.count() + 1 << "][3] = {" << endl;
	    for( QValueList<Function>::Iterator it = functions.begin(); it != functions.end(); ++it ){
		str << "    { \"" << (*it).type << "\", \"" << (*it).name << "\", \"" << (*it).fullName << "\" }," << endl;
	    }
	    str << "    { 0, 0, 0 }" << endl;
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
		str << "\tfor ( int i = 0; " << className << "_ftable[i][1]; i++ )" << endl;
		str << "\t    fdict->insert( " << className << "_ftable[i][1],  new int( i ) );" << endl;
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
		    QString funcType = r.firstChild().toText().data();
		    if ( funcType == "ASYNC" )
			funcType = "void";
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
		    funcName += '(';
		    bool first = TRUE;
		    for( QStringList::Iterator argtypes_count = argtypes.begin(); argtypes_count != argtypes.end(); ++argtypes_count ){
			if ( !first )
			    funcName += ',';
			first = FALSE;
			funcName += *argtypes_count;
		    }
		    funcName += ')';
			
		    if ( useHashing ) {
			str << "    case " << fcount << ": { // " << funcType << " " << funcName << endl;
		    } else {
			if ( firstFunc )
			    str << "    if ( fun == " << className << "_ftable[" << fcount << "][1] ) { // " << funcType << " " << funcName << endl;
			else
			    str << " else if ( fun == " << className << "_ftable[" << fcount << "][1] ) { // " << funcType << " " << funcName << endl;
			firstFunc = FALSE;
		    }
		    if ( !args.isEmpty() ) {
			QStringList::Iterator ittypes = argtypes.begin();
			QStringList::Iterator args_count;
			for( args_count = args.begin(); args_count != args.end(); ++args_count ){
			    str << '\t'<< *ittypes << " " << *args_count << ";" <<  endl;
			    ++ittypes;
			}
			str << "\tQDataStream arg( data, IO_ReadOnly );" << endl;
			for( args_count = args.begin(); args_count != args.end(); ++args_count ){
			    str << "\targ >> " << *args_count << ";" << endl;
			}
		    }

		    str << "\treplyType = " << className << "_ftable[" << fcount++ << "][0]; " << endl;
		    if ( funcType == "void" ) {
			str << '\t' << plainFuncName << '(';
		    } else {
			str << "\tQDataStream _replyStream( replyData, IO_WriteOnly );"  << endl;
			str << "\t_replyStream << " << plainFuncName << '(';
		    }

		    first = TRUE;
		    for ( QStringList::Iterator args_count = args.begin(); args_count != args.end(); ++args_count ){
			if ( !first )
			    str << ", ";
			first = FALSE;
			str << *args_count;
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
	
	    str << "QCStringList " << className;
	    str << "::interfaces()" << endl;
	    str << "{" << endl;
	    if (!DCOPParent.isEmpty()) {
		str << "    QCStringList ifaces = " << DCOPParent << "::interfaces();" << endl;
	    } else {
		str << "    QCStringList ifaces;" << endl;
	    }
	    str << "    ifaces += \"" << classNameFull << "\";" << endl;
	    str << "    return ifaces;" << endl;
	    str << "}" << endl << endl;
	    
	    
	    str << "QCStringList " << className;
	    str << "::functions()" << endl;
	    str << "{" << endl;
	    if (!DCOPParent.isEmpty()) {
		str << "    QCStringList funcs = " << DCOPParent << "::functions();" << endl;
	    } else {
		str << "    QCStringList funcs;" << endl;
	    }
	    str << "    for ( int i = 0; " << className << "_ftable[i][2]; i++ ) {" << endl;
	    str << "\tQCString func = " << className << "_ftable[i][0];" << endl;
	    str << "\tfunc += ' ';" << endl;
	    str << "\tfunc += " << className << "_ftable[i][2];" << endl;
	    str << "\tfuncs << func;" << endl;
	    str << "    }" << endl;
	    str << "    return funcs;" << endl;
	    str << "}" << endl << endl;

            for(;
                 namespace_count > 0;
                 --namespace_count )
                str << "} // namespace" << endl;
            str << endl;

	}
    }
	
    skel.close();
}
