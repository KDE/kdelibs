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


/**
 * Writes the stubs header
 */
void generateStub( const QString& idl, const QString& filename, QDomElement de)
{
    QFile stub( filename );
    if ( !stub.open( IO_WriteOnly ) )
	qFatal("Could not write to %s", filename.local8Bit().data() );
	
    QTextStream str( &stub );

    str << "/****************************************************************************" << endl;
    str << "**" << endl;
    str << "** DCOP Stub Definition created by dcopidl2cpp from " << idl << endl;
    str << "**" << endl;
    str << "** WARNING! All changes made in this file will be lost!" << endl;
    str << "**" << endl;
    str << "*****************************************************************************/" << endl;
    str << endl;

    QString ifdefstring = idl.upper();
    int pos = idl.findRev( '.' );
    if ( pos != -1 )
	ifdefstring = ifdefstring.left( pos );

    QString ifdefsuffix = "_STUB__";
    str << "#ifndef __" << ifdefstring << ifdefsuffix << endl;
    str << "#define __" << ifdefstring << ifdefsuffix << endl << endl;

    str << "#include <dcopstub.h>" << endl;

    QStringList includeslist, all_includes;
    QDomElement e = de.firstChild().toElement();
    for( ; !e.isNull(); e = e.nextSibling().toElement() ) {
	if ( e.tagName() == "INCLUDE" ) {
            // dcopidl lists the includes in reversed order because of the used yacc/bison gramatic
            // so let's reverse it back, as the order may be important
	    includeslist.prepend( e.firstChild().toText().data());
            continue;
	}
        if( !includeslist.empty()) {
            for( QStringList::ConstIterator it = includeslist.begin();
                 it != includeslist.end();
                 ++it ) {
    	        str << "#include <" << ( *it ) << ">" << endl;
                all_includes.append( *it );
            }
            includeslist.clear();
        }
        if ( e.tagName() == "CLASS" ) {
	    str << endl;
	
	    QDomElement n = e.firstChild().toElement();
	    Q_ASSERT( n.tagName() == "NAME" );
	    QString className = n.firstChild().toText().data() 
                         + ( "_stub" );
	
	    // find dcop parent ( rightmost super class )
	    QString DCOPParent;
	    QDomElement s = n.nextSibling().toElement();
	    for( ; !s.isNull(); s = s.nextSibling().toElement() ) {
		if ( s.tagName() == "SUPER" )
		    DCOPParent = s.firstChild().toText().data();
	    }
            
            if( DCOPParent != "DCOPObject" ) { // we need to include the .h file for the base stub
                if( all_includes.contains( DCOPParent + ".h" ))
                    str << "#include <" << DCOPParent << "_stub.h>" << endl;
                else if( all_includes.contains( DCOPParent.lower() + ".h" ))
                    str << "#include <" << DCOPParent.lower() << "_stub.h>" << endl;
                else {// damn ... let's assume it's the last include
                    QString stub_h = all_includes.last();
                    unsigned int pos = stub_h.find( ".h" );
                    if( pos > 0 ) {
                        stub_h = stub_h.remove( pos, 100000 );
                        str << "#include <" << stub_h << "_stub.h>" << endl;
                    }
                    else
                        str << "#include <" << stub_h << ">" << endl;
                }
            }

            QString classNameFull = className; // class name with possible namespaces prepended
                                               // namespaces will be removed from className now
            int namespace_count = 0;
            QString namespace_tmp = className;
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

	    // Stub class definition
	    str << "class " << className;

	    // Parent : inherited interface stub or dcopstub
            if ( !DCOPParent.isEmpty() && DCOPParent != "DCOPObject" ) {
               str << " : ";
               str << "virtual public " << DCOPParent << "_stub";
            } else {
               str << " : virtual public DCOPStub";
            }

	    str << endl;
	    str << "{" << endl;
	    str << "public:" << endl;
	
            // Constructors
            str << "    " << className << "( const QCString& app, const QCString& id );" << endl;
            str << "    " << className << "( DCOPClient* client, const QCString& app, const QCString& id );" << endl;

	    s = e.firstChild().toElement();
	    for( ; !s.isNull(); s = s.nextSibling().toElement() ) {
		if (s.tagName() == "FUNC") {
		    QDomElement r = s.firstChild().toElement();
		    Q_ASSERT( r.tagName() == "TYPE" );
		    str << "    ";
		    if ( r.hasAttribute( "qleft" ) )
			str << r.attribute("qleft") << " ";
		    str << r.firstChild().toText().data();
		    if ( r.hasAttribute( "qright" ) )
			str << r.attribute("qright") << " ";
		    else
			str << " ";

		    r = r.nextSibling().toElement();
		    Q_ASSERT ( r.tagName() == "NAME" );
		    str << r.firstChild().toText().data() << "(";

		    bool first = TRUE;
		    r = r.nextSibling().toElement();
		    for( ; !r.isNull(); r = r.nextSibling().toElement() ) {
			if ( !first )
			    str << ", ";
			else
			    str << " ";
			first = FALSE;
			Q_ASSERT( r.tagName() == "ARG" );
			QDomElement a = r.firstChild().toElement();
			Q_ASSERT( a.tagName() == "TYPE" );
			if ( a.hasAttribute( "qleft" ) )
			    str << a.attribute("qleft") << " ";
			str << a.firstChild().toText().data();
			if ( a.hasAttribute( "qright" ) )
			    str << a.attribute("qright") << " ";
			else
			    str << " ";
			a = a.nextSibling().toElement();
			if ( a.tagName() == "NAME" )
			    str << a.firstChild().toText().data();
		    }
		    if ( !first )
			str << " ";
		    str << ")";

		    if ( s.hasAttribute("qual") )
			str << " " << s.attribute("qual");
		    str << ";" << endl;
		}
	    }

            // needed for inherited stubs
	    str << "protected:" << endl;
            str << "    " << className << "() {};" << endl;

	    str << "};" << endl;
	    str << endl;

            for(;
                 namespace_count > 0;
                 --namespace_count )
                str << "} // namespace" << endl;
            str << endl;

	}
    }

    str << "#endif" << endl;
    stub.close();
}

