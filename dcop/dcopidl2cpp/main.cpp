#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstring.h>

#include <string.h>
#include <stdio.h>
#include <unistd.h>

void usage()
{
    fprintf( stderr, "dcopidl2cpp [ --no-skel | --no-stub ] [--c++-suffix <suffix>] file\n" );
}

int main( int argc, char** argv )
{
    int argpos = 1;
    bool generate_skel = TRUE;
    bool generate_stub = TRUE;

    QString suffix = "cpp";

    while (argc > 2) {

	if ( strcmp( argv[argpos], "--no-skel" ) == 0 )
	{
	    generate_skel = FALSE;
	    for (int i = argpos; i < argc - 1; i++) argv[i] = argv[i+1];
	    argc--;
	}
	else if ( strcmp( argv[argpos], "--no-stub" ) == 0 )
	{
	    generate_stub = FALSE;
	    for (int i = argpos; i < argc - 1; i++) argv[i] = argv[i+1];
	    argc--;
	} 
	else if ( strcmp( argv[argpos], "--c++-suffix" ) == 0)
	{
	    if (argc - 1 < argpos) {
		usage();
		exit(1);
	    }
	    suffix = argv[argpos+1];
	    for (int i = argpos; i < argc - 2; i++) argv[i] = argv[i+2];
	    argc -= 2;
	} else {
	    usage();
	    exit(1);

	}

    }
	
    QFile in( argv[argpos] );
    if ( !in.open( IO_ReadOnly ) )
    {
	qDebug("Could not read %s", argv[argpos] );
	exit(1);
    }
    QDomDocument doc( &in );

    QDomElement de = doc.documentElement();
    ASSERT( de.tagName() == "DCOP-IDL" );
	
    QString basename( argv[argpos] );
    int pos = basename.findRev( "." );
    if ( pos != -1 )
	basename = basename.left( pos );

    if ( generate_skel )
    {
    /**
     * Write the skeleton
     */
    QFile skel( basename + "_skel." + suffix );
    bool b = skel.open( IO_WriteOnly );
    if ( !b )
    {
	qDebug("Could not write to %s", ( basename + "_skel." + suffix).latin1() );
	exit(1);
    }

    {
	QTextStream str( &skel );
	
	str << "#include <" << basename << ".h>" << endl << endl;
	
	QDomElement e = de.firstChild().toElement();
	for( ; !e.isNull(); e = e.nextSibling().toElement() )
        {
	    if ( e.tagName() == "CLASS" )
	    {
		// Write constructor
		/* str << e.attribute("name") << "::" << e.attribute("name") << "( const QCString& _id )" << endl;
		str << "\t: ";
		QDomElement k = e.firstChild().toElement();
		bool first_super = TRUE;
		for( ; !k.isNull(); k = k.nextSibling().toElement() )
	        {
		    if ( k.tagName() == "SUPER" )
		    {
			if ( first_super )
			    str << k.attribute("name") << "( _id )";
			else
			    str << ", " << k.attribute("name") << "( _id )";
			first_super = FALSE;
		    }
		}
		str << endl;
		str << "{" << endl;
		str << "}" << endl << endl;; */
		
		// Write dispatcher
		str << "bool " << e.attribute("name");
		str << "::process(const QCString &fun, const QByteArray &data, QCString& replyType, QByteArray &replyData)" << endl;
		str << "{" << endl;

		QDomElement s = e.firstChild().toElement();
		for( ; !s.isNull(); s = s.nextSibling().toElement() )
	        {
		    if ( s.tagName() == "FUNC" )
		    {
			QDomElement r = s.firstChild().toElement();
			ASSERT( r.tagName() == "RET" );
			QString ret = r.attribute("type");
			r = r.nextSibling().toElement();
			QDomElement args = r;

			QString funcname = s.attribute("name");
			funcname += "(";
			bool first = TRUE;
			QDomElement f = args;
			for( ; !f.isNull(); f = f.nextSibling().toElement() )
		        {
			    if ( !first )
				funcname += ",";
			    funcname += f.attribute("type");
			    first = FALSE;
			}
			funcname += ")";
			
			str << "\tif ( fun == \"" << funcname << "\" )" << endl;
			
			str << "\t{" << endl;
			if ( !args.isNull() )
		        {
			    str << "\t\tQDataStream str( data, IO_ReadOnly );" << endl;
			}
			
			for( ; !r.isNull(); r = r.nextSibling().toElement() )
		        {
			    ASSERT( r.tagName() == "ARG" );
			    str << "\t\t" << r.attribute("type") << " " << r.attribute("name") << ";" << endl;
			    str << "\t\tstr >> " << r.attribute("name") << ";" << endl;
			}

			str << "\t\treplyType = \"" << ret << "\";" << endl;
			if ( ret == "void" )
			    str << "\t\t" << s.attribute("name") << "(";
			else
		        {
			    str << "\t\tQDataStream out( replyData, IO_WriteOnly );"  << endl;
			    str << "\t\tout << " << s.attribute("name") << "(";
			}
			
			first = TRUE;
			for( ; !args.isNull(); args = args.nextSibling().toElement() )
		        {
			    if ( !first )
				str << ", ";
			    str << args.attribute("name");
			    first = FALSE;
			}
			
			str << " );" << endl;
			str << "\t\treturn TRUE;" << endl;
			str << "\t}" << endl;
		    }
		}
		
		s = e.firstChild().toElement();
		for( ; !s.isNull(); s = s.nextSibling().toElement() )
	        {
		    if ( s.tagName() == "SUPER" )
	            {
			str << "\tif ( " << s.attribute("name") << "::process( fun, data, replyType, replyData ) )" << endl;
			str << "\t\treturn TRUE;" << endl;
		    }
		}

		str << "\treturn FALSE;" << endl;
		str << "}" << endl << endl;
	    }
	}
    }

    skel.close();

    }

    if ( generate_stub )
    {
    /**
     * Write the stubs header
     */
    QFile stub( basename + "_stub.h" );
    bool b = stub.open( IO_WriteOnly );
    if ( !b )
    {
	qDebug("Could not write to %s", ( basename + "_stub." + suffix ).latin1() );
	exit(1);
    }

    {
	QTextStream str( &stub );
	
	str << "#ifndef __" << basename << "_STUB__" << endl;
	str << "#define __" << basename << "_STUB__" << endl << endl;

	str << "#include <dcopstub.h>" << endl;
	
	QDomElement e = de.firstChild().toElement();
	for( ; !e.isNull(); e = e.nextSibling().toElement() )
        {
	    if ( e.tagName() == "INCLUDE" )
	    {
		str << "#include <" << e.attribute( "file" ) << ">" << endl;
	    }
	    else if ( e.tagName() == "CLASS" )
	    {
		// Include all needed stubs
		QDomElement s = e.firstChild().toElement();
		for( ; !s.isNull(); s = s.nextSibling().toElement() )
	        {
		    if ( s.tagName() == "SUPER" && s.attribute("name") != "DCOPObject" )
			str << "#include <" << s.attribute("name") << "_stub.h>" << endl;
		}
		
		str << endl;
		
		str << "class " << e.attribute("name") << "_stub";
		
		bool start = TRUE;
		bool header = FALSE;
		bool derived = FALSE;
		s = e.firstChild().toElement();
		for( ; !s.isNull(); s = s.nextSibling().toElement() )
	        {
		    if ( s.tagName() == "SUPER" )
	            {
			ASSERT( !header );
			
			if ( s.attribute("name") != "DCOPObject" )
		        {
			    derived = TRUE;
			    if ( start )
				str << " : ";
			    else
				str << ", ";
			    start = FALSE;
			    str << "virtual public " << s.attribute("name") << "_stub";
			}
		    }
		    else
		    {
			if ( !header )
		        {
			    if ( !derived )
				str << " : virtual public DCOPStub";
			    header = TRUE;
			    str << endl;
			    str << "{" << endl;
			    str << "public:" << endl;
			    str << "\t" << e.attribute("name") << "_stub( const QCString& app, const QCString& id );" << endl;
			}
			
			if ( s.tagName() == "FUNC" )
		        {
			    QDomElement r = s.firstChild().toElement();
			    ASSERT( r.tagName() == "RET" );
			    str << "\tvirtual ";
			    if ( r.hasAttribute( "qleft" ) )
				str << r.attribute("qleft") << " ";
			    str << r.attribute("type");
			    if ( r.hasAttribute( "qright" ) )
				str << r.attribute("qright") << " ";
			    else
				str << " ";
			
			    str << s.attribute("name") << "(";
			
			    bool first = TRUE;
			    r = r.nextSibling().toElement();
			    for( ; !r.isNull(); r = r.nextSibling().toElement() )
			    {
				if ( !first )
				    str << ", ";
				ASSERT( r.tagName() == "ARG" );
				if ( r.hasAttribute( "qleft" ) )
				    str << r.attribute("qleft") << " ";
				str << r.attribute("type");
				if ( r.hasAttribute( "qright" ) )
				    str << r.attribute("qright") << " ";
				else
				    str << " ";
			
				str << r.attribute("name");
				
				first = FALSE;
			    }
			
			    str << ")";
			
			    if ( s.hasAttribute("qual") )
				str << " " << s.attribute("qual");
			    str << ";" << endl;
			}
		    }
		}
		
		str << "};" << endl;
		str << endl;
	    }
	}
	
	str << "#endif" << endl;
    }

    stub.close();



    /**
     * Write the stub implementation
     */
    QFile s2( basename + "_stub." + suffix );
    b = s2.open( IO_WriteOnly );
    if ( !b )
    {
	qDebug("Could not write to %s", ( basename + "_stub." + suffix).latin1() );
	exit(1);
    }

    {
	QTextStream str( &s2 );
	
	str << "#include <" << basename << "_stub.h>" << endl;
	str << "#include <dcopclient.h>" << endl << endl;
	str << "#include <kapp.h>" << endl << endl;
		
	QDomElement e = de.firstChild().toElement();
	for( ; !e.isNull(); e = e.nextSibling().toElement() )
        {
	    if ( e.tagName() == "CLASS" )
	    {
		// Write constructor
		str << e.attribute("name") << "_stub::" << e.attribute("name") << "_stub"
		    << "( const QCString& _app, const QCString& _id )" << endl;
		str << "\t: ";
		QDomElement k = e.firstChild().toElement();
		bool first_super = TRUE;
		for( ; !k.isNull(); k = k.nextSibling().toElement() )
	        {
		    if ( k.tagName() == "SUPER" && k.attribute("name") != "DCOPObject" )
		    {
			if ( first_super )
			    str << k.attribute("name") << "( _app, _id )";
			else
			    str << ", " << k.attribute("name") << "( _app, _id )";
			first_super = FALSE;
		    }
		}
		if ( first_super )
		    str << "DCOPStub( _app, _id )" << endl;
		str << endl;
		str << "{" << endl;
		str << "}" << endl << endl;
		
		// Write marshalling code
		QDomElement s = e.firstChild().toElement();
		for( ; !s.isNull(); s = s.nextSibling().toElement() )
	        {
		    if ( s.tagName() == "FUNC" )
		    {
			QDomElement r = s.firstChild().toElement();
			ASSERT( r.tagName() == "RET" );
			if ( r.hasAttribute( "qleft" ) )
			    str << r.attribute("qleft") << " ";
			str << r.attribute("type");
			if ( r.hasAttribute( "qright" ) )
			    str << r.attribute("qright") << " ";
			else
			    str << " ";
			
			str << e.attribute("name") << "_stub::" << s.attribute("name") << "(";
			
			bool first = TRUE;
			r = r.nextSibling().toElement();
			for( ; !r.isNull(); r = r.nextSibling().toElement() )
		        {
			    if ( !first )
				str << ", ";
			    ASSERT( r.tagName() == "ARG" );
			    if ( r.hasAttribute( "qleft" ) )
				str << r.attribute("qleft") << " ";
			    str << r.attribute("type");
			    if ( r.hasAttribute( "qright" ) )
				str << r.attribute("qright") << " ";
			    else
				str << " ";
			
			    str << r.attribute("name");
				
			    first = FALSE;
			}
			
			str << ")";
			
			if ( s.hasAttribute("qual") )
			    str << " " << s.attribute("qual");
			str << endl;
			str << "{" << endl ;
			
			r = s.firstChild().toElement();
			ASSERT( r.tagName() == "RET" );
			QString ret = r.attribute("type");
			if ( ret != "void" )
			    str << "\t" << r.attribute("type") << " _ret_;" << endl;
			
			str << "\tQByteArray snd;" << endl;
			str << "\tQByteArray rcv;" << endl;
			str << "\tQCString _type_;" << endl;
			str << "\t{" << endl;
			r = r.nextSibling().toElement();
			QDomElement args = r;
			if ( !args.isNull() )
		        {
			    str << "\t\tQDataStream str( snd, IO_WriteOnly );" << endl;
			}
			
			for( ; !r.isNull(); r = r.nextSibling().toElement() )
		        {
			    ASSERT( r.tagName() == "ARG" );
			    str << "\t\tstr << " << r.attribute("name") << ";" << endl;
			}

			str << "\t}" << endl;

			QString funcname( s.attribute("name") );
			funcname += "(";
			first = TRUE;
			for( ; !args.isNull(); args = args.nextSibling().toElement() )
		        {
			    if ( !first )
				funcname += ",";
			    funcname += args.attribute("type");
			}
			funcname += ")";
			
			str << "\tkapp->dcopClient()->call( app(), obj(), \"" << funcname << "\",";
			str << " snd, _type_, rcv );" << endl;
			
			str << "\tASSERT( _type_ == \"" << ret << "\" );" << endl;
						
			if ( ret != "void" )
		        {
			    str << "\tQDataStream out( rcv, IO_ReadOnly );"  << endl;
			    str << "\tout >> _ret_;" << endl;
			    str << "\treturn _ret_;" << endl;
			}
			
			str << "}" << endl << endl;
		    }
		}
	    }
	}
    }

    s2.close();

    }

    return 0;
}
