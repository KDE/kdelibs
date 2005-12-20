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

use strict;
no strict "subs";

use vars qw/$libname $rootnode $outputdir $opt $debug %canonicalType/;

# Converted from the old dcopidl's yacc file, under int_type:
%canonicalType = (
  'signed' => 'signed int',
  'unsigned' => 'unsigned int',
  'signed short' => 'signed short int',
  'signed long' => 'signed long int',
  'unsigned short' => 'unsigned short int',
  'unsigned long' => 'unsigned long int',
  'long' => 'long int',
  'short' => 'short int'
);

BEGIN
{
}

sub writeDoc
{
	( $libname, $rootnode, $outputdir, $opt ) = @_;

	$debug = $main::debuggen;

	print STDERR "Preparsing...\n" if ($debug);

	# Preparse everything, to prepare some additional data in the classes and methods
	Iter::LocalCompounds( $rootnode, sub { preParseClass( shift ); } );

	kdocAstUtil::dumpAst($rootnode) if ($debug);

	print STDERR "Writing dcopidl...\n" if ($debug);

	print STDOUT "<!DOCTYPE DCOP-IDL><DCOP-IDL>\n";

	print STDOUT "<SOURCE>".@{$rootnode->{Sources}}[0]->{astNodeName}."</SOURCE>\n";

	print STDOUT map { "<INCLUDE>$_</INCLUDE>\n" } @main::includes_list;

	Iter::LocalCompounds( $rootnode, sub { 
	    my ($node) = @_;

	    my ($methodCode) = generateAllMethods( $node );
	    my $className = join "::", kdocAstUtil::heritage($node);

	    if ($node->{DcopExported}) {
		print STDOUT "<CLASS>\n";
		print STDOUT "    <NAME>$className</NAME>\n";
		print STDOUT "    <LINK_SCOPE>$node->{Export}</LINK_SCOPE>\n" if ($node->{Export});
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
	
	print STDERR "Done.\n" if ($debug);
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
	    $classNode->{NodeType} eq 'union' # Skip unions for now, e.g. QPDevCmdParam
	  ) {
	    print STDERR "Skipping $className\n" if ($debug);
	    print STDERR "Skipping union $className\n" if ( $classNode->{NodeType} eq 'union');
	    delete $classNode->{Compound}; # Cheat, to get it excluded from Iter::LocalCompounds
	    return;
	}
}


sub generateMethod($$)
{
    my( $classNode, $m ) = @_;	# input
    my $methodCode = '';	# output

    my $name = $m->{astNodeName}; # method name
    my @heritage = kdocAstUtil::heritage($classNode);
    my $className  = join( "::", @heritage );

    # Check some method flags: constructor, destructor etc.
    my $flags = $m->{Flags};

    if ( !defined $flags ) {
	warn "Method ".$name.  " has no flags\n";
    }

    my $returnType = $m->{ReturnType};
    $returnType = undef if ($returnType eq 'void');
    # For compat with KDE3, we call QCString the type holding a DCOPCString;
    # this matches the dcopTypeName(DCOPCString) being equal to "QCString"
    $returnType = 'QCString' if ($returnType eq 'DCOPCString');

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

    my $args = "";

    foreach my $arg ( @{$m->{ParamList}} ) {

	print STDERR "  Param ".$arg->{astNodeName}." type: ".$arg->{ArgType}." name:".$arg->{ArgName}." default: ".$arg->{DefaultValue}."\n" if ($debug);

	my $argType = $arg->{ArgType};

	my $x_isConst = ($argType =~ s/const//);
	my $x_isRef = ($argType =~ s/&//);

	my $typeAttrs = "";
	$typeAttrs .= "  qleft=\"const\"" if $x_isConst;
	$typeAttrs .= " qright=\"&amp;\"" if $x_isRef;

	$argType =~ s/^\s*(.*?)\s*$/$1/; # stripWhiteSpace
	$argType =~ s/\s/ /g;            # simplifyWhiteSpace
	$argType =~ s/</&lt;/g;
	$argType =~ s/>/&gt;/g;
	$argType = $canonicalType{$argType} if ( exists $canonicalType{$argType} );

	$args .= "        <ARG><TYPE$typeAttrs>$argType</TYPE><NAME>$arg->{ArgName}</NAME></ARG>\n";
    }

    my $qual = "";
    $qual .= " qual=\"const\"" if $flags =~ "c";

    $returnType = "void" unless $returnType;
    $returnType =~ s/</&lt;/g;
    $returnType =~ s/>/&gt;/g;
    $returnType = $canonicalType{$returnType} if ( exists $canonicalType{$returnType} );

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

    return ( $methodCode );
}

sub generateAllMethods
{
    my ($classNode) = @_;
    my $methodCode = '';

    # Then all methods
    Iter::MembersByType ( $classNode, undef,
			  sub {	my ($classNode, $methodNode ) = @_;

        if ( $methodNode->{NodeType} eq 'method' ) {
	    next unless $methodNode->{Flags} =~ /(d|z|y)/;
	    my ($meth) = generateMethod( $classNode, $methodNode );
	    $methodCode .= $meth;
	}
			      }, undef );

    return ( $methodCode );
}

1;
