#***************************************************************************
#            kalyptusCxxToDcopIDL.pm -  Generates idl from dcop headers
#                             -------------------
#    begin                : Fri Jan 25 12:00:00 2000
#    copyright            : (C) 2003 Alexander Kellett
#    email                : lypanov@kde.org
#    author               : Alexander Kellett
#***************************************************************************/

#/***************************************************************************
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU General Public License as published by  *
# *   the Free Software Foundation; either version 2 of the License, or     *
# *   (at your option) any later version.                                   *
# *                                                                         *
#***************************************************************************/

package kalyptusCxxToDcopIDL;

use File::Path;
use File::Basename;
use Carp;
use Ast;
use kdocAstUtil;
use kdocUtil;
use Iter;
use kalyptusDataDict;
use Data::Dumper;

use strict;
no strict "subs";

use vars qw/
	$libname $rootnode $outputdir $opt $debug
	$methodNumber
	%builtins %typeunion %allMethods %allTypes %enumValueToType %typedeflist %mungedTypeMap
	%skippedClasses /;

BEGIN
{

# Types supported by the StackItem union
# Key: C++ type  Value: Union field of that type
%typeunion = (
    'void*' => 's_voidp',
    'bool' => 's_bool',
    'char' => 's_char',
    'uchar' => 's_uchar',
    'short' => 's_short',
    'ushort' => 's_ushort',
    'int' => 's_int',
    'uint' => 's_uint',
    'long' => 's_long',
    'ulong' => 's_ulong',
    'float' => 's_float',
    'double' => 's_double',
    'enum' => 's_enum',
    'class' => 's_class'
);

# Mapping for iterproto, when making up the munged method names
%mungedTypeMap = (
     'QString' => '$',
     'QString*' => '$',
     'QString&' => '$',
     'QCString' => '$',
     'QCString*' => '$',
     'QCString&' => '$',
     'QByteArray' => '$',
     'QByteArray&' => '$',
     'QByteArray*' => '$',
     'char*' => '$',
     'QCOORD*' => '?',
     'QRgb*' => '?',
);

# Yes some of this is in kalyptusDataDict's ctypemap
# but that one would need to be separated (builtins vs normal classes)
%typedeflist =
(
   'signed char' => 'char',
   'unsigned char' => 'uchar',
   'signed short' => 'short',
   'unsigned short' => 'ushort',
   'signed' => 'int',
   'signed int' => 'int',
   'unsigned' => 'uint',
   'unsigned int' => 'uint',
   'signed long' => 'long',
   'unsigned long' => 'ulong',

# Anything that is not known is mapped to void*, so no need for those here anymore
#   'QWSEvent*'  =>  'void*',
#   'QDiskFont*'  =>  'void*',
#   'XEvent*'  =>  'void*',
#   'QStyleHintReturn*'  =>  'void*',
#   'FILE*'  =>  'void*',
#   'QUnknownInterface*'  =>  'void*',
#   'GDHandle'  =>  'void*',
#   '_NPStream*'  =>  'void*',
#   'QTextFormat*'  =>  'void*',
#   'QTextDocument*'  =>  'void*',
#   'QTextCursor*'  =>  'void*',
#   'QTextParag**'  =>  'void*',
#   'QTextParag*'  =>  'void*',
#   'QRemoteInterface*'  =>  'void*',
#   'QSqlRecordPrivate*'  =>  'void*',
#   'QTSMFI'  =>  'void*', # QTextStream's QTSManip
#   'const GUID&'  =>  'void*',
#   'QWidgetMapper*'  =>  'void*',
#   'MSG*'  =>  'void*',
#   'const QSqlFieldInfoList&'  =>  'void*', # QSqlRecordInfo - TODO (templates)

   'QPtrCollection::Item'  =>  'void*', # to avoid a warning

   'mode_t'  =>  'long',
   'QProcess::PID'  =>  'long',
   'size_type'  =>  'int', # QSqlRecordInfo
   'Qt::ComparisonFlags'  =>  'uint',
   'Qt::ToolBarDock'  =>  'int', # compat thing, Qt shouldn't use it
   'QIODevice::Offset'  =>  'ulong',
   'WState'  =>  'int',
   'WId'  =>  'ulong',
   'QRgb'  =>  'uint',
   'QCOORD'  =>  'int',
   'QTSMFI'  =>  'int',
   'Qt::WState'  =>  'int',
   'Qt::WFlags'  =>  'int',
   'Qt::HANDLE' => 'uint',
   'QEventLoop::ProcessEventsFlags' => 'uint',
   'QStyle::SCFlags' => 'int',
   'QStyle::SFlags' => 'int',
   'Q_INT16' => 'short',
   'Q_INT32' => 'int',
   'Q_INT8' => 'char',
   'Q_LONG' => 'long',
   'Q_UINT16' => 'ushort',
   'Q_UINT32' => 'uint',
   'Q_UINT8' => 'uchar',
   'Q_ULONG' => 'long',
);

}

sub writeDoc
{
	( $libname, $rootnode, $outputdir, $opt ) = @_;

	print STDERR "Starting writeDoc for $libname...\n";

	$debug = $main::debuggen;

	# Define QPtrCollection::Item, for resolveType
	unless ( kdocAstUtil::findRef( $rootnode, "QPtrCollection::Item" ) ) {
		my $cNode = kdocAstUtil::findRef( $rootnode, "QPtrCollection" );
		warn "QPtrCollection not found" if (!$cNode);
		my $node = Ast::New( 'Item' );
		$node->AddProp( "NodeType", "Forward" );
		$node->AddProp( "Source", $cNode->{Source} ) if ($cNode);
		kdocAstUtil::attachChild( $cNode, $node ) if ($cNode);
		$node->AddProp( "Access", "public" );
	}

	print STDERR "Preparsing...\n";

	# Preparse everything, to prepare some additional data in the classes and methods
	Iter::LocalCompounds( $rootnode, sub { preParseClass( shift ); } );

	# Write out smokedata.cpp
	writeSmokeDataFile($rootnode);

	print STDERR "Writing dcopidl...\n";

	print STDOUT "<!DOCTYPE DCOP-IDL><DCOP-IDL>\n";

        print STDOUT "<SOURCE>".@{$rootnode->{Sources}}[0]->{astNodeName}."</SOURCE>\n";

	print STDOUT map { "<INCLUDE>$_</INCLUDE>\n" } reverse @main::includes_list;

	Iter::LocalCompounds( $rootnode, sub { 
	    my ($node) = @_;

            my ($methodCode, $switchCode, $incl) = generateAllMethods( $node );
            my $className = join "::", kdocAstUtil::heritage($node);

	    if ($node->{DcopExported}) {
		print STDOUT "<CLASS>\n";
		print STDOUT "    <NAME>$className</NAME>\n";
		print STDOUT join("\n", map { "    <SUPER>$_</SUPER>"; } grep { $_ ne "Global"; } 
			     map { 
                                my $name = $_->{astNodeName};
                                $name =~ s/</&lt;/;
                                $name =~ s/>/&gt;/;
                                my $tmpl = $_->{TmplType};
                                $tmpl =~ s/</&lt;/;
                                $tmpl =~ s/>/&gt;/;
                                $tmpl ? "$name&lt;<TYPE>$tmpl</TYPE>&gt;" : $name;
			     } @{$node->{InList}}) . "\n";
		print STDOUT $methodCode;

		print STDOUT "</CLASS>\n";
	    }
	});

	print STDOUT "</DCOP-IDL>\n";
	
	print STDERR "Done.\n";
}

=head2 preParseClass
	Called for each class
=cut
sub preParseClass
{
	my( $classNode ) = @_;
	my $className = join( "::", kdocAstUtil::heritage($classNode) );

	if( $#{$classNode->{Kids}} < 0 ||
	    $classNode->{Access} eq "private" ||
	    $classNode->{Access} eq "protected" || # e.g. QPixmap::QPixmapData
	    exists $classNode->{Tmpl} ||
	    # Don't generate standard bindings for QString, this class is handled as a native type
	    $className eq 'QString' ||
	    $className eq 'QConstString' ||
	    $className eq 'QCString' ||
	    # Don't map classes which are really arrays
	    $className eq 'QStringList' ||
            $className eq 'QCanvasItemList' ||
            $className eq 'QWidgetList' ||
            $className eq 'QObjectList' ||
	    $className eq 'QStrList' ||
	    # Those are template related
            $className eq 'QTSManip' || # cause compiler errors with several gcc versions
	    $className eq 'QGDict' ||
	    $className eq 'QGList' ||
	    $className eq 'QGVector' ||
	    $className eq 'QStrIList' ||
	    $className eq 'QStrIVec' ||
	    $className eq 'QByteArray' ||
	    $className eq 'QBitArray' ||
	    $classNode->{NodeType} eq 'union' # Skip unions for now, e.g. QPDevCmdParam
	  ) {
	    print STDERR "Skipping $className\n" if ($debug);
	    print STDERR "Skipping union $className\n" if ( $classNode->{NodeType} eq 'union');
	    $skippedClasses{$className} = 1;
	    delete $classNode->{Compound}; # Cheat, to get it excluded from Iter::LocalCompounds
	    return;
	}

	my $signalCount = 0;
	my $eventHandlerCount = 0;
	my $defaultConstructor = 'none'; #  none, public, protected or private. 'none' will become 'public'.
	my $constructorCount = 0; # total count of _all_ ctors
	# If there are ctors, we need at least one public/protected one to instanciate the class
	my $hasPublicProtectedConstructor = 0;
	# We need a public dtor to destroy the object --- ### aren't protected dtors ok too ??
	my $hasPublicDestructor = 1; # by default all classes have a public dtor!
	my $hasDestructor = 0;
	my $hasPrivatePureVirtual = 0;
	my $hasCopyConstructor = 0;
	my $hasPrivateCopyConstructor = 0;
	# Note: no need for hasPureVirtuals. $classNode{Pure} has that.

        my $doPrivate = $main::doPrivate;
	$main::doPrivate = 1;
	# Look at each class member (looking for methods and enums in particular)
	Iter::MembersByType ( $classNode, undef,
		sub {

	my( $classNode, $m ) = @_;
	my $name = $m->{astNodeName};

	if( $m->{NodeType} eq "method" ) {
	    if ( $m->{ReturnType} eq 'typedef' # QFile's EncoderFn/DecoderFn callback, very badly parsed
	       ) {
		$m->{NodeType} = 'deleted';
		next;
	    }

	    print STDERR "preParseClass: looking at $className\::$name  $m->{Params}\n" if ($debug);

	    if ( $name eq $classNode->{astNodeName} ) {
		if ( $m->{ReturnType} =~ /~/  ) {
		    # A destructor
		    $hasPublicDestructor = 0 if $m->{Access} ne 'public';
		    $hasDestructor = 1;
		} else {
		    # A constructor
		    $constructorCount++;
		    $defaultConstructor = $m->{Access} if ( $m->{Params} eq '' );
		    $hasPublicProtectedConstructor = 1 if ( $m->{Access} ne 'private' );

		    # Copy constructor?
		    if ( $#{$m->{ParamList}} == 0 ) {
			my $theArgType = @{$m->{ParamList}}[0]->{ArgType};
			if ($theArgType =~ /$className\s*\&/) {
			    $hasCopyConstructor = 1;
			    $hasPrivateCopyConstructor = 1 if ( $m->{Access} eq 'private' );
			}
		    }
		    # Hack the return type for constructors, since constructors return an object pointer
		    $m->{ReturnType} = $className."*";
		}
	    }

	    if ( $name =~ /~$classNode->{astNodeName}/ && $m->{Access} ne "private" ) { # not used
		$hasPublicDestructor = 0 if $m->{Access} ne 'public';
		$hasDestructor = 1;
	    }

	    if ( $m->{Flags} =~ "p" && $m->{Access} =~ /private/ ) {
                $hasPrivatePureVirtual = 1; # ouch, can't inherit from that one
	    }

	    # All we want from private methods is to check for virtuals, nothing else
	    next if ( $m->{Access} =~ /private/ );

	    my $argId = 0;
	    my $firstDefaultParam;
	    foreach my $arg ( @{$m->{ParamList}} ) {
		# Look for first param with a default value
		if ( defined $arg->{DefaultValue} && !defined $firstDefaultParam ) {
		    $firstDefaultParam = $argId;
		}

		if ( $arg->{ArgType} eq '...' # refuse a method with variable arguments
		     or $arg->{ArgType} eq 'image_io_handler' # QImage's callback
		     or $arg->{ArgType} eq 'DecoderFn' # QFile's callback
		     or $arg->{ArgType} eq 'EncoderFn' # QFile's callback
		     or $arg->{ArgType} =~ /bool \(\*\)\(QObject/ # QMetaObject's ctor
		     or $arg->{ArgType} eq 'QtStaticMetaObjectFunction' # QMetaObjectCleanUp's ctor with func pointer
		     or $arg->{ArgType} eq 'const QTextItem&' # ref to a private class in 3.2.0b1
		     or $arg->{ArgType} eq 'FILE*' # won't be able to handle that I think
		) {
		    $m->{NodeType} = 'deleted';
		}
		else
		{
		    # Resolve type in full, e.g. for QSessionManager::RestartHint
		    # (x_QSessionManager doesn't inherit QSessionManager)
		    $arg->{ArgType} = kalyptusDataDict::resolveType($arg->{ArgType}, $classNode, $rootnode);
		    registerType( $arg->{ArgType} );
		    $argId++;
		}
	    }
	    $m->AddProp( "FirstDefaultParam", $firstDefaultParam );
	    $m->{ReturnType} = kalyptusDataDict::resolveType($m->{ReturnType}, $classNode, $rootnode) if ($m->{ReturnType});
	    registerType( $m->{ReturnType} );
	}
	elsif( $m->{NodeType} eq "enum" ) {
	    my $fullEnumName = $className."::".$m->{astNodeName};
	    $classNode->{enumerations}{$m->{astNodeName}} = $fullEnumName
		if $m->{astNodeName} and $m->{Access} ne 'private';

	    # Define a type for this enum
	    registerType( $fullEnumName );

	    # Remember that it's an enum
	    findTypeEntry( $fullEnumName )->{isEnum} = 1;

	    #print STDERR "$fullEnumName is an enum\n";
 	}
	elsif( $m->{NodeType} eq 'var' ) {
	    my $varType = $m->{Type};
	    # We are interested in public static vars, like QColor::blue
	    if ( $varType =~ s/static\s+// && $m->{Access} ne 'private' )
	    {
		$varType =~ s/const\s+(.*)\s*&/$1/;
		$varType =~ s/\s*$//;
		print STDERR "var: $m->{astNodeName} '$varType'\n" if ($debug);

		# Register the type
		registerType( $varType );

	    } else {
		# To avoid duplicating the above test, we just get rid of any other var
		$m->{NodeType} = 'deleted';
	    }
	}
		},
		undef
	);
	$main::doPrivate = $doPrivate;

	print STDERR "$className: ctor count: $constructorCount, hasPublicProtectedConstructor: $hasPublicProtectedConstructor, hasCopyConstructor: $hasCopyConstructor:, defaultConstructor: $defaultConstructor, hasPublicDestructor: $hasPublicDestructor, hasPrivatePureVirtual:$hasPrivatePureVirtual\n" if ($debug);

	# We will derive from the class only if it has public or protected constructors.
	# (_Even_ if it has pure virtuals. But in that case the x_ class can't be instantiated either.)
	$classNode->AddProp( "BindingDerives", $hasPublicProtectedConstructor );
}


=head2 writeClassDoc

	Called by writeDoc for each series of classes to be written out

=cut

  sub writeClassDoc
  {
  }

# Generate the prototypes for a method (one per arg with a default value)
# Helper for makeprotos
sub iterproto($$$$$) {
    my $classidx = shift; # to check if a class exists
    my $method = shift;
    my $proto = shift;
    my $idx = shift;
    my $protolist = shift;

    my $argcnt = scalar @{ $method->{ParamList} } - 1;
    if($idx > $argcnt) {
	push @$protolist, $proto;
	return;
    }
    if(defined $method->{FirstDefaultParam} and $method->{FirstDefaultParam} <= $idx) {
	push @$protolist, $proto;
    }

    my $arg = $method->{ParamList}[$idx]->{ArgType};

    my $typeEntry = findTypeEntry( $arg );
    my $realType = $typeEntry->{realType};

    # A scalar ?
    $arg =~ s/\bconst\b//g;
    $arg =~ s/\s+//g;
    if($typeEntry->{isEnum} || $allTypes{$realType}{isEnum} || exists $typeunion{$realType} || exists $mungedTypeMap{$arg})
    {
	my $id = '$'; # a 'scalar
	$id = '?' if $arg =~ /[*&]{2}/;
	$id = $mungedTypeMap{$arg} if exists $mungedTypeMap{$arg};
	iterproto($classidx, $method, $proto . $id, $idx + 1, $protolist);
	return;
    }

    # A class ?
    if(exists $classidx->{$realType}) {
	iterproto($classidx, $method, $proto . '#', $idx + 1, $protolist);
	return;
    }

    # A non-scalar (reference to array or hash, undef)
    iterproto($classidx, $method, $proto . '?', $idx + 1, $protolist);
    return;
}

# Generate the prototypes for a method (one per arg with a default value)
sub makeprotos($$$) {
    my $classidx = shift;
    my $method = shift;
    my $protolist = shift;
    iterproto($classidx, $method, $method->{astNodeName}, 0, $protolist);
}

# Return the string containing the signature for this method (without return type).
# If the 2nd arg is not the size of $m->{ParamList}, this method returns a
# partial signature (this is used to handle default values).
sub methodSignature($$) {
    my $method = shift;
    my $last = shift;
    my $sig = $method->{astNodeName};
    my @argTypeList;
    my $argId = 0;
    foreach my $arg ( @{$method->{ParamList}} ) {
	last if $argId > $last;
	push @argTypeList, $arg->{ArgType};
	$argId++;
    }
    $sig .= "(". join(", ",@argTypeList) .")";
    $sig .= " const" if $method->{Flags} =~ "c";
    return $sig;
}

sub coerce_type($$$$) {
    #my $m = shift;
    my $union = shift;
    my $var = shift;
    my $type = shift;
    my $new = shift; # 1 if this is a return value, 0 for a normal param

    my $typeEntry = findTypeEntry( $type );
    my $realType = $typeEntry->{realType};

    my $unionfield = $typeEntry->{typeId};
    die "$type" unless defined( $unionfield );
    $unionfield =~ s/t_/s_/;

    $type =~ s/\s+const$//; # for 'char* const'
    $type =~ s/\s+const\s*\*$/\*/; # for 'char* const*'

    my $code = "$union.$unionfield = ";
    if($type =~ /&$/) {
	$code .= "(void*)&$var;\n";
    } elsif($type =~ /\*$/) {
	$code .= "(void*)$var;\n";
    } else {
	if ( $unionfield eq 's_class' 
		or ( $unionfield eq 's_voidp' and $type ne 'void*' )
		or $type eq 'QString' ) { # hack
	    $type =~ s/^const\s+//;
	    if($new) {
	        $code .= "(void*)new $type($var);\n";
	    } else {
	        $code .= "(void*)&$var;\n";
	    }
	} else {
	    $code .= "$var;\n";
	}
    }

    return $code;
}

# Generate the list of args casted to their real type, e.g.
# (QObject*)x[1].s_class,(QEvent*)x[2].s_class,x[3].s_int
sub makeCastedArgList
{
    my @castedList;
    my $i = 1; # The args start at x[1]. x[0] is the return value
    my $arg;
    foreach $arg (@_) {
	my $type = $arg;
	my $cast;

	my $typeEntry = findTypeEntry( $type );
	my $unionfield = $typeEntry->{typeId};
	die "$type" unless defined( $unionfield );
	$unionfield =~ s/t_/s_/;

	$type =~ s/\s+const$//; # for 'char* const'
	$type =~ s/\s+const\s*\*$/\*/; # for 'char* const*'

	my $v .= "$unionfield";
	if($type =~ s/&$//) {
	    $cast = "{($type *)}";
	} elsif($type =~ /\*$/) {
	    $cast = "$type";
        } elsif($type =~ /\(\*\)\s*\(/) { # function pointer ... (*)(...)
            $cast = "$type";
	} else {
	    if ( $unionfield eq 's_class'
		or ( $unionfield eq 's_voidp' and $type ne 'void*' )
		or $type eq 'QString' ) { # hack
	        $cast = "{*($type *)}";
	    } else {
	        $cast = "$type";
	    }
	}
	push @castedList, "<TYPE>$cast</TYPE>$v";
	$i++;
    }
    return @castedList;
}

# Adds the header for node $1 to be included in $2 if not already there
# Prints out debug stuff if $3
sub addIncludeForClass($$$)
{
    my ( $node, $addInclude, $debugMe ) = @_;
    my $sourcename = $node->{Source}->{astNodeName};
    $sourcename =~ s!.*/(.*)!$1!m;
    die "Empty source name for $node->{astNodeName}" if ( $sourcename eq '' );
    unless ( defined $addInclude->{$sourcename} ) {
	print "  Including $sourcename\n" if ($debugMe);
	$addInclude->{$sourcename} = 1;
    }
    else { print "  $sourcename already included.\n" if ($debugMe); }
}

sub checkIncludesForObject($$)
{
    my $type = shift;
    my $addInclude = shift;

    my $debugCI = 0; #$debug
    #print "checkIncludesForObject $type\n";
    $type =~ s/const\s+//;
    my $it = $type;
    if (!($it and exists $typeunion{$it}) and $type !~ /\*/
         #and $type !~ /&/  # in fact we also want refs, due to the generated code
        ) {
	$type =~ s/&//;
	print "  Detecting an object by value/ref: $type\n" if ($debugCI);
	my $node = kdocAstUtil::findRef( $rootnode, $type );
	if ($node) {
	    addIncludeForClass( $node, $addInclude, $debugCI );
	}
	else { print " No header found for $type\n" if ($debugCI); }
    }
}

sub generateMethod($$$)
{
    my( $classNode, $m, $addInclude ) = @_;	# input
    my $methodCode = '';	# output

    my $name = $m->{astNodeName}; # method name
    my @heritage = kdocAstUtil::heritage($classNode);
    my $className  = join( "::", @heritage );
    my $xClassName  = "x_" . join( "__", @heritage );

    # Check some method flags: constructor, destructor etc.
    my $flags = $m->{Flags};

    if ( !defined $flags ) {
	warn "Method ".$name.  " has no flags\n";
    }

    my $returnType = $m->{ReturnType};
    $returnType = undef if ($returnType eq 'void');

    # Don't use $className here, it's never the fully qualified (A::B) name for a ctor.
    my $isConstructor = ($name eq $classNode->{astNodeName} );
    my $isDestructor = ($returnType eq '~');

    if ($debug) {
        print STDERR " Method $name";
	print STDERR ", is DTOR" if $isDestructor;
	print STDERR ", returns $returnType" if $returnType;
	#print STDERR " ($m->{Access})";
	print STDERR "\n";
    }

    # Don't generate anything for destructors
    return if $isDestructor;

    return if ( $m->{SkipFromSwitch} ); # pure virtuals, etc.

#    # Skip internal methods, which return unknown types
#    # Hmm, the C# bindings have a list of those too.
#    return if ( $returnType =~ m/QGfx\s*\*/ );
#    return if ( $returnType eq 'CGContextRef' );
#    return if ( $returnType eq 'QWSDisplay *' );
#    # This stuff needs callback, or **
#    return if ( $name eq 'defineIOHandler' or $name eq 'qt_init_internal' );
#    # Skip casting operators, but not == < etc.
#    return if ( $name =~ /operator \w+/ );
#    # QFile's EncoderFn/DecoderFn
#    return if ( $name =~ /set[ED][ne]codingFunction/ );
#    # How to implement this? (QXmlDefaultHandler/QXmlEntityResolver::resolveEntity, needs A*&)
#    return if ( $name eq 'resolveEntity' and $className =~ /^QXml/ );
#    return if ( $className eq 'QBitArray' && $m->{Access} eq 'protected' );

    #print STDERR "Tests passed, generating.\n";

    # Detect objects returned by value
    checkIncludesForObject( $returnType, $addInclude ) if ($returnType);

    my $argId = 0;

    my @argTypeList=();

    my $args = "";

    foreach my $arg ( @{$m->{ParamList}} ) {

	print STDERR "  Param ".$arg->{astNodeName}." type: ".$arg->{ArgType}." name:".$arg->{ArgName}." default: ".$arg->{DefaultValue}."\n" if ($debug);

	my $argType = $arg->{ArgType};

	my $x_isConst = ($argType =~ s/const//);
	my $x_isRef = ($argType =~ s/&//);

	my $typeAttrs = "";
	$typeAttrs .= "  qleft=\"const\"" if $x_isConst;
	$typeAttrs .= " qright=\"&amp;\"" if $x_isRef;

	$argType =~ s/^\s*(.*?)\s*$/$1/;
	$argType =~ s/</&lt;/g;
	$argType =~ s/>/&gt;/g;
	$argType =~ s/\s//g;

	$args .= "        <ARG><TYPE$typeAttrs>$argType</TYPE><NAME>$arg->{ArgName}</NAME></ARG>\n";

	push @argTypeList, $argType;

	# Detect objects passed by value
	checkIncludesForObject( $argType, $addInclude );
    }

#    my @castedArgList = makeCastedArgList( @argTypeList );

    my $isStatic = ($flags =~ "s"); 
    my $extra = "";
    $extra .= "static " if $isStatic || $isConstructor;

    my $qual = "";
    $qual .= " qual=\"const\"" if $flags =~ "c";

    my $this = $classNode->{BindingDerives} > 0 ? "this" : "xthis";

    # We iterate as many times as we have default params
    my $firstDefaultParam = $m->{FirstDefaultParam};
    $firstDefaultParam = scalar(@argTypeList) unless defined $firstDefaultParam;

    my $xretCode = '';
    if($returnType) {
	$xretCode .= coerce_type('x[0]', 'xret', $returnType, 1); }

    $returnType = "void" unless $returnType;
    $returnType =~ s/</&lt;/g;
    $returnType =~ s/>/&gt;/g;

    my $methodCode = "";

    my $tagType = ($flags !~ /z/) ? "FUNC" : "SIGNAL";
    my $tagAttr = "";
    $tagAttr .= " hidden=\"yes\"" if $flags =~ /y/;

    if (!$isConstructor) {
	$methodCode .= "    <$tagType$tagAttr$qual>\n";
	$methodCode .= "        <TYPE>$returnType</TYPE>\n";
	$methodCode .= "        <NAME>$name</NAME>\n";
	$methodCode .= "$args";
	$methodCode .= "     </$tagType>\n";
    }

    $methodNumber++;

    return ( $methodCode, "" );
}

## Called by writeClassDoc
sub generateAllMethods
{
    my ($classNode) = @_;
    my $methodCode = '';

    my $sourcename = $classNode->{Source}->{astNodeName};
    $sourcename =~ s!.*/(.*)!$1!m;
    die "Empty source name for $classNode->{astNodeName}" if ( $sourcename eq '' );

    my %addInclude = ( $sourcename => 1 );

    # Then all methods
    Iter::MembersByType ( $classNode, undef,
			  sub {	my ($classNode, $methodNode ) = @_;

        if ( $methodNode->{NodeType} eq 'method' ) {
	    next unless $methodNode->{Flags} =~ /(d|z|y)/;
	    my ($meth, $swit) = generateMethod( $classNode, $methodNode, \%addInclude );
	    $methodCode .= $meth;
	}
			      }, undef );

    return ( $methodCode, "", \%addInclude );
}

# Known typedef? If so, apply it.
sub applyTypeDef($)
{
    my $type = shift;
    # Parse 'const' in front of it, and '*' or '&' after it
    my $prefix = $type =~ s/^const\s+// ? 'const ' : '';
    my $suffix = $type =~ s/\s*([\&\*]+)$// ? $1 : '';

    if (exists $typedeflist{$type}) {
	return $prefix.$typedeflist{$type}.$suffix;
    }
    return $prefix.$type.$suffix;
}

# Register type ($1) into %allTypes if not already there
sub registerType($$) {
    my $type = shift;
    #print "registerType: $type\n" if ($debug);

    $type =~ s/\s+const$//; # for 'char* const'
    $type =~ s/\s+const\s*\*$/\*/; # for 'char* const*'

    return if ( $type eq 'void' or $type eq '' or $type eq '~' );
    die if ( $type eq '...' );     # ouch

    # Let's register the real type, not its known equivalent
    #$type = applyTypeDef($type);

    # Enum _value_ -> get corresponding type
    if (exists $enumValueToType{$type}) {
	$type = $enumValueToType{$type};
    }

    # Already in allTypes
    if(exists $allTypes{$type}) {
        return;
    }

    die if $type eq 'QTextEdit::UndoRedoInfo::Type';
    die if $type eq '';

    my $realType = $type;

    # Look for references (&) and pointers (* or **)  - this will not handle *& correctly.
    # We do this parsing here because both the type list and iterproto need it
    if($realType =~ s/&$//) {
	$allTypes{$type}{typeFlags} = 'Smoke::tf_ref';
    }
    elsif($realType ne 'void*' && $realType =~ s/\*$//) {
	$allTypes{$type}{typeFlags} = 'Smoke::tf_ptr';
    }
    else {
	$allTypes{$type}{typeFlags} = 'Smoke::tf_stack';
    }

    if ( $realType =~ s/^const\s+// ) { # Remove 'const'
	$allTypes{$type}{typeFlags} .= ' | Smoke::tf_const';
    }

    # Apply typedefs, and store the resulting type.
    # For instance, if $type was Q_UINT16&, realType will be ushort
    $allTypes{$type}{realType} = applyTypeDef( $realType );

    # In the first phase we only create entries into allTypes.
    # The values (indexes) are calculated afterwards, once the list is full.
    $allTypes{$type}{index} = -1;
    #print STDERR "Register $type. Realtype: $realType\n" if($debug);
}

# Get type from %allTypes
# This returns a hash with {index}, {isEnum}, {typeFlags}, {realType}
# (and {typeId} after the types array is written by writeSmokeDataFile)
sub findTypeEntry($) {
    my $type = shift;
    my $typeIndex = -1;
    $type =~ s/\s+const$//; # for 'char* const'
    $type =~ s/\s+const\s*\*$/\*/; # for 'char* const*'

    return undef if ( $type =~ '~' or $type eq 'void' or $type eq '' );

    # Enum _value_ -> get corresponding type
    if (exists $enumValueToType{$type}) {
	$type = $enumValueToType{$type};
    }

    die "type not known: $type" unless defined $allTypes{$type};
    return $allTypes{ $type };
}

# List of all super-classes for a given class
sub superclass_list($)
{
    my $classNode = shift;
    my @super;
    Iter::Ancestors( $classNode, $rootnode, undef, undef, sub {
			push @super, @_[0];
			push @super, superclass_list( @_[0] );
		     }, undef );
    return @super;
}

=head2
	Write out the smokedata.cpp file containing all the arrays.
=cut

sub writeSmokeDataFile($) {
    my $rootnode = shift;

    # Make list of classes
    my %allIncludes; # list of all header files for all classes
    my @classlist;
    push @classlist, ""; # Prepend empty item for "no class"
    my %enumclasslist;
    Iter::LocalCompounds( $rootnode, sub {
	my $classNode = $_[0];
	my $className = join( "::", kdocAstUtil::heritage($classNode) );
	push @classlist, $className;
	$enumclasslist{$className}++ if keys %{$classNode->{enumerations}};
	$classNode->{ClassIndex} = $#classlist;
	addIncludeForClass( $classNode, \%allIncludes, undef );
    } );

 kdocAstUtil::dumpAst($rootnode) if ($debug);

    my %classidx = do { my $i = 0; map { $_ => $i++ } @classlist };

    # Prepare descendants information for each class
    my %descendants; # classname -> list of descendant nodes
    Iter::LocalCompounds( $rootnode, sub {
	my $classNode = shift;
	# Get _all_ superclasses (up any number of levels)
	# and store that $classNode is a descendant of $s
	my @super = superclass_list($classNode);
	for my $s (@super) {
	    my $superClassName = join( "::", kdocAstUtil::heritage($s) );
	    Ast::AddPropList( \%descendants, $superClassName, $classNode );
	}
    } );

    # Iterate over all classes, to write the xtypecast function
    Iter::LocalCompounds( $rootnode, sub {
	my $classNode = shift;
	my $className = join( "::", kdocAstUtil::heritage($classNode) );
	# @super will contain superclasses, the class itself, and all descendants
	my @super = superclass_list($classNode);
	push @super, $classNode;
        if ( defined $descendants{$className} ) {
	    push @super, @{$descendants{$className}};
	}
	my $cur = $classidx{$className};
	$cur = -1;
	for my $s (@super) {
	    my $superClassName = join( "::", kdocAstUtil::heritage($s) );
	    next if !defined $classidx{$superClassName}; # inherits from unknown class, see below
	    next if $classidx{$superClassName} == $cur;    # shouldn't happen in Qt
	    $cur = $classidx{$superClassName};
	}
    } );

    # Write inheritance array
    # Imagine you have "Class : public super1, super2"
    # The inheritlist array will get 3 new items: super1, super2, 0
    my %inheritfinder;  # key = (super1, super2) -> data = (index in @inheritlist). This one allows reuse.
    my %classinherit;   # we store that index in %classinherit{className}
    # We don't actually need to store inheritlist in memory, we write it
    # directly to the file. We only need to remember its current size.
    my $inheritlistsize = 1;

    Iter::LocalCompounds( $rootnode, sub {
	my $classNode = shift;
	my $className = join( "__", kdocAstUtil::heritage($classNode) );
	print STDERR "inheritanceList: looking at $className\n" if ($debug);

	# Make list of direct ancestors
	my @super;
	Iter::Ancestors( $classNode, $rootnode, undef, undef, sub {
			     my $superClassName = join( "::", kdocAstUtil::heritage($_[0]) );
			     push @super, $superClassName;
		    }, undef );
	# Turn that into a list of class indexes
	my $key = '';
	foreach my $superClass( @super ) {
	    if (defined $classidx{$superClass}) {
		$key .= ', ' if ( length $key > 0 );
		$key .= $classidx{$superClass};
	    }
	}
	if ( $key ne '' ) {
	    if ( !defined $inheritfinder{$key} ) {
		my $index = $inheritlistsize; # Index of first entry (for this group) in inheritlist
		foreach my $superClass( @super ) {
		    if (defined $classidx{$superClass}) {
			$inheritlistsize++;
		    }
		}
		$inheritlistsize++;
		my $comment = join( ", ", @super );
		$inheritfinder{$key} = $index;
	    }
	    $classinherit{$className} = $inheritfinder{$key};
	} else { # No superclass
	    $classinherit{$className} = 0;
	}
    } );

    for my $className (keys %enumclasslist) {
	my $c = $className;
	$c =~ s/::/__/g;
    }
    my $firstClass = 1;
    for my $className (@classlist) {
	if ($firstClass) {
	    $firstClass = 0;
	    next;
	}
	my $c = $className;   # make a copy
	$c =~ s/::/__/g;
    }

    # Write class list afterwards because it needs offsets to the inheritance array.
    my $firstClass = 1;
    Iter::LocalCompounds( $rootnode, sub {
	my $classNode = shift;
	my $className = join( "__", kdocAstUtil::heritage($classNode) );

	if ($firstClass) {
	    $firstClass = 0;
	}
	my $c = $className;
	$c =~ s/::/__/g;
	my $xcallFunc = "xcall_$c";
	my $xenumFunc = "0";
	$xenumFunc = "xenum_$c" if exists $enumclasslist{$className};
	# %classinherit needs Foo__Bar, not Foo::Bar?
	die "problem with $className" unless defined $classinherit{$c};

	my $xClassFlags = 0;
	$xClassFlags =~ s/0\|//; # beautify
    } );


    my $typeCount = 0;
    $allTypes{''}{index} = 0; # We need an "item 0"
    for my $type (sort keys %allTypes) {
	$allTypes{$type}{index} = $typeCount;      # Register proper index in allTypes
	if ( $typeCount == 0 ) {
	    $typeCount++;
	    next;
	}
	my $isEnum = $allTypes{$type}{isEnum};
	my $typeId;
	my $typeFlags = $allTypes{$type}{typeFlags};
	my $realType = $allTypes{$type}{realType};
	die "$type" if !defined $typeFlags;
	die "$realType" if $realType =~ /\(/;
	# First write the name
	# Then write the classId (and find out the typeid at the same time)
	if(exists $classidx{$realType}) { # this one first, we want t_class for QBlah*
	    $typeId = 't_class';
	}
	elsif($type =~ /&$/ || $type =~ /\*$/) {
	    $typeId = 't_voidp';
	}
	elsif($isEnum || $allTypes{$realType}{isEnum}) {
	    $typeId = 't_enum';
	    if($realType =~ /(.*)::/) {
		my $c = $1;
	    }
	}
	else {
	    $typeId = $typeunion{$realType};
	    if (defined $typeId) {
		$typeId =~ s/s_/t_/; # from s_short to t_short for instance
	    }
	    else {
		# Not a known class - ouch, this happens quite a lot
		# (private classes, typedefs, template-based types, etc)
		if ( $skippedClasses{$realType} ) {
#		    print STDERR "$realType has been skipped, using t_voidp for it\n";
		} else {
		    unless( $realType =~ /</ ) { # Don't warn for template stuff...
			print STDERR "$realType isn't a known type (type=$type)\n";
		    }
		}
		$typeId = 't_voidp'; # Unknown -> map to a void *
	    }
	}
	# Then write the flags
	die "$type" if !defined $typeId;
	$typeCount++;
	# Remember it for coerce_type
	$allTypes{$type}{typeId} = $typeId;
    }
}

1;
