
=head1 kdocLib

Writes out a library file.

NOTES ON THE NEW FORMAT

	Stores: class name, members, hierarchy
	node types are not stored


	File Format Spec
	----------------

	header
	zero or more members, each of
		method
		member
		class, each of
			inheritance
			zero or more members



	Unrecognized lines  ignored.

	Sample
	------

	<! KDOC Library HTML Reference File>
	<VERSION="2.0">
	<BASE URL="http://www.kde.org/API/kdecore/">

	<C NAME="KApplication" REF="KApplication.html">
		<IN NAME="QObject">
		<ME NAME="getConfig" REF="KApplication.html#getConfig">
		<M NAME="" REF="">
	</C>

=cut

package kdocLib;
use strict;

use Carp;
use File::Path;
use File::Basename;

use Ast;
use kdocAstUtil;
use kdocUtil;


use vars qw/ $exe $lib $root $plang $outputdir $docpath $url $compress /;

BEGIN {
	$exe = basename $0;
}

sub writeDoc
{
	( $lib, $root, $plang, $outputdir, $docpath, $url, 
		$compress ) = @_;
	my $outfile = "$outputdir/$lib.kalyptus";
	$url = $docpath unless defined $url;

	mkpath( $outputdir ) unless -f $outputdir;

	if( $compress ) {
		open( LIB, "| gzip -9 > \"$outfile.gz\"" ) 
			|| die "$exe: couldn't write to $outfile.gz\n";

	}
	else {
		open( LIB, ">$outfile" ) 
			|| die "$exe: couldn't write to $outfile\n";
	}

	my $libdesc = "";
	if ( defined $root->{LibDoc} ) {
			$libdesc="<LIBDESC>".$root->{LibDoc}->{astNodeName}."</LIBDESC>";
	}
	
	print LIB<<LTEXT;
<! KDOC Library HTML Reference File>
<VERSION="$main::Version">
<BASE URL="$url">
<PLANG="$plang">
<LIBNAME>$lib</LIBNAME>
$libdesc

LTEXT

	writeNode( $root, "" );
	close LIB;
}

sub writeNode
{
	my ( $n, $prefix ) = @_;
	return if !exists $n->{Compound};
	return if exists $n->{Forward} && !exists $n->{KidAccess};

	if( $n != $root ) {
		$prefix .= $n->{astNodeName};
		print LIB "<C NAME=\"", $n->{astNodeName},
			"\" REF=\"$prefix.html\">\n";
	}
	else {
		print LIB "<STATS>\n";
		my $stats = $root->{Stats};
		foreach my $stat ( keys %$stats ) {
			print LIB "<STAT NAME=\"$stat\">",
				$stats->{$stat},"</STAT>\n";
		}
		print LIB "</STATS>\n";
	}

	if( exists $n->{Ancestors} ) {
		my $in;
		foreach $in ( @{$n->{Ancestors}} ) {
			$in =~ s/\s+//g;
			print LIB "<IN NAME=\"",$in,"\">\n";
		}
	}

	return if !exists $n->{Kids};
	my $kid;
	my $type;

	foreach $kid ( @{$n->{Kids}} ) {
		next if exists $kid->{ExtSource}
			|| $kid->{Access} eq "private";

		if ( exists $kid->{Compound} ) {
			if( $n != $root ) {
				writeNode( $kid, $prefix."::" );
			}
			else {
				writeNode( $kid, "" );
			}
			next;
		}

		$type = $kid->{NodeType} eq "method" ? 
			"ME" : "M";

		print LIB "<$type NAME=\"", $kid->{astNodeName},
			"\" REF=\"$prefix.html#", $kid->{astNodeName}, "\">\n";
	}

	if( $n != $root ) {
		print LIB "</C>\n";
	}
}

sub readLibrary
{
	my( $rootsub, $name, $path, $relurl ) = @_;
	$path = "." unless defined $path;
	my $real = $path."/".$name.".kalyptus";
	my $url = ".";
	my @stack = ();
	my $version = "2.0";
	my $new;
	my $root = undef;
	my $n = undef;
	my $havecomp = -r "$real.gz";
	my $haveuncomp = -r "$real";
	
	if ( $haveuncomp ) {
		open( LIB, "$real" ) || die "Can't read lib $real\n";
	}

	if( $havecomp ) {
		if ( $haveuncomp ) {
			warn "$exe: two libs exist: $real and $real.gz. "
				."Using $real\n";
		}
		else {
			open( LIB, "gunzip < \"$real.gz\"|" ) 
			|| die "Can't read pipe gunzip < \"$real.gz\": $?\n";
		}
	}

	while( <LIB> ) {
		next if /^\s*$/;
		if ( !/^\s*</ ) {
			close LIB;
			#readOldLibrary( $root, $name, $path );
			return;
		}

		if( /<VER\w+\s+([\d\.]+)>/ ) {
			# TODO: what do we do with the version number?
			$version = $1;
		}
		elsif ( /<BASE\s*URL\s*=\s*"(.*?)"/ ) {
			$url = $1;
			$url .= "/" unless $url =~ m:/$:;

			my $test = kdocUtil::makeRelativePath( $relurl, $url );
			$url = $test;
		}
		elsif( /<PLANG\s*=\s*"(.*?)">/ ) {
			$root = $rootsub->( $1 );
			$n = $root;
		}
		elsif ( /<C\s*NAME="(.*?)"\s*REF="(.*?)"\s*>/  ) {
			# class
			$new = Ast::New( $1 );
			$new->AddProp( "NodeType", "class" );
			$new->AddProp( "Compound", 1 );
			$new->AddProp( "ExtSource", $name );

			# already escaped at this point!
			$new->AddProp( "Ref", $url.$2 );

			$root = $n = $rootsub->( "CXX" ) unless defined $root;
			kdocAstUtil::attachChild( $n, $new );
			push @stack, $n;
			$n = $new;
		}
		elsif ( m#<IN\s*NAME\s*=\s*"(.*?)"\s*># ) {
			# ancestor
			kdocAstUtil::newInherit( $n, $1 );
		}
		elsif ( m#</C># ) {
			# end class
			$n = pop @stack;
		}
		elsif ( m#<(M\w*)\s+NAME="(.*?)"\s+REF="(.*?)"\s*># ) {
			# member
			$new = Ast::New( $2 );
			$new->AddProp( "NodeType", $1 eq "ME" ? "method" : "var" );
			$new->AddProp( "ExtSource", $name );
			$new->AddProp( "Flags", "" );
			$new->AddProp( "Ref", $url.$3 );

			kdocAstUtil::attachChild( $n, $new );
		}
	}
}

1;
