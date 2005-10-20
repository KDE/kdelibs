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
#include "type.h"

/*
 * Writes the stubs header
 */
void generateStub( const QString& idl, const QString& filename, QDomElement de)
{
    QFile stub( filename );
    if ( !stub.open( QIODevice::WriteOnly ) )
	qFatal("Could not write to %s", filename.toLocal8Bit().data() );
	
    QTextStream str( &stub );

    str << "/****************************************************************************" << endl;
    str << "**" << endl;
    str << "** DCOP Stub Definition created by dcopidl2cpp from " << idl << endl;
    str << "**" << endl;
    str << "** WARNING! All changes made in this file will be lost!" << endl;
    str << "**" << endl;
    str << "*****************************************************************************/" << endl;
    str << endl;

    QString ifdefstring = idl.toUpper();
    int pos = idl.lastIndexOf( '.' );
    if ( pos != -1 )
	ifdefstring = ifdefstring.left( pos );

    QString ifdefsuffix = "_STUB__";
    str << "#ifndef __" << ifdefstring.replace("\\","_") << ifdefsuffix << endl;
    str << "#define __" << ifdefstring.replace("\\","_") << ifdefsuffix << endl << endl;

    str << "#include <dcopstub.h>" << endl;
    str << "#include <kdatastream.h>" << endl;

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
        if ( e.tagName() != "CLASS" )
	    continue;

	str << endl;
    
	QDomElement n = e.firstChild().toElement();
	Q_ASSERT( n.tagName() == "NAME" );
	QString className = n.firstChild().toText().data() + ( "_stub" );

	//add link scope, if available
	n = n.nextSibling().toElement();
	QString linkScope;
	if (n.tagName()=="LINK_SCOPE") {
		linkScope = n.firstChild().toText().data() + " ";
		n = n.nextSibling().toElement();
	}

	// find dcop parent ( rightmost super class )
	QString DCOPParent;
	for( ; !n.isNull(); n = n.nextSibling().toElement() ) {
	    if ( n.tagName() == "SUPER" )
		DCOPParent = n.firstChild().toText().data();
	}

	if( DCOPParent != "DCOPObject" ) { // we need to include the .h file for the base stub
	    if( all_includes.contains( DCOPParent + ".h" ))
		str << "#include <" << DCOPParent << "_stub.h>" << endl;
	    else if( all_includes.contains( DCOPParent.toLower() + ".h" ))
		str << "#include <" << DCOPParent.toLower() << "_stub.h>" << endl;
	    else {// damn ... let's assume it's the last include
		QString stub_h = all_includes.last();
		unsigned int pos = stub_h.indexOf( ".h" );
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
	    int pos = namespace_tmp.indexOf( "::" );
	    if( pos < 0 ) {
		className = namespace_tmp;
		break;
	    }
	    str << "namespace " << namespace_tmp.left( pos ) << " {" << endl;
	    ++namespace_count;
	    namespace_tmp = namespace_tmp.mid( pos + 2 );
	}

	str << endl;

	// Stub class definition
	str << "class " << linkScope << className;

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
	str << "    " << className << "( const DCOPCString& app, const DCOPCString& id );" << endl;
	str << "    " << className << "( DCOPClient* client, const DCOPCString& app, const DCOPCString& id );" << endl;
	str << "    explicit " << className << "( const DCOPRef& ref );" << endl;

	n = e.firstChild().toElement();
	for( ; !n.isNull(); n = n.nextSibling().toElement() ) {
	    if (n.tagName() != "FUNC")
		continue;
	    QDomElement r = n.firstChild().toElement();
	    str << "    virtual "; // KDE4 - I really don't think these need to be virtual
	    writeType( str, r );

	    r = r.nextSibling().toElement();
	    Q_ASSERT ( r.tagName() == "NAME" );
	    str << r.firstChild().toText().data() << "(";

	    bool first = true;
	    r = r.nextSibling().toElement();
	    for( ; !r.isNull(); r = r.nextSibling().toElement() ) {
		if ( !first )
		    str << ", ";
		else
		    str << " ";
		first = false;
		Q_ASSERT( r.tagName() == "ARG" );
		QDomElement a = r.firstChild().toElement();
		writeType( str, a );
		a = a.nextSibling().toElement();
		if ( a.tagName() == "NAME" )
		    str << a.firstChild().toText().data();
	    }
	    if ( !first )
		str << " ";
	    str << ")";

	    //const methods stubs can't compile, they need to call setStatus().
	    //if ( n.hasAttribute("qual") )
	    //  str << " " << n.attribute("qual");
	    str << ";" << endl;
	}

	// needed for inherited stubs
	str << "protected:" << endl;
	str << "    " << className << "() : DCOPStub( never_use ) {};" << endl;

	str << "};" << endl;
	str << endl;

	for(; namespace_count > 0; --namespace_count )
	    str << "} // namespace" << endl;
	str << endl;
    }

    str << "#endif" << endl;
    stub.close();
}

// :set expandtab!<RETURN>:set ts=8<RETURN>:set sts=4<RETURN>:set sw=4<RETURN>
