=head1 kdocAstUtil

	Utilities for syntax trees.

=cut


package kdocAstUtil;

use Ast;
use Carp;
use File::Basename;
use kdocUtil;
use Iter;
use strict;

use vars qw/ $depth $refcalls $refiters @noreflist %noref /;

sub BEGIN {
# statistics for findRef

	$depth = 0;
	$refcalls = 0;
	$refiters = 0;

# findRef will ignore these words

	@noreflist = qw( const int char long double template 
		unsigned signed float void bool true false uint 
		uint32 uint64 extern static inline virtual operator );

	foreach my $r ( @noreflist ) {
		$noref{ $r } = 1;
	}
}


=head2 findNodes

	Parameters: outlist ref, full list ref, key, value

	Find all nodes in full list that have property "key=value".
	All resulting nodes are stored in outlist.

=cut

sub findNodes
{
	my( $rOutList, $rInList, $key, $value ) = @_;

	my $node;

	foreach $node ( @{$rInList} ) {
		next if !exists $node->{ $key };
		if ( $node->{ $key } eq $value ) {
			push @$rOutList, $node;
		}
	}
}

=head2 allTypes

	Parameters: node list ref
	returns: list

	Returns a sorted list of all distinct "NodeType"s in the nodes 
	in the list.

=cut

sub allTypes
{
	my ( $lref ) = @_;

	my %types = ();
	foreach my $node ( @{$lref} ) {
		$types{ $node->{NodeType} } = 1;
	}

	return sort keys %types;
}




=head2 findRef

	Parameters: root, ident, report-on-fail
	Returns: node, or undef

	Given a root node and a fully qualified identifier (:: separated),
	this function will try to find a child of the root node that matches
	the identifier.

=cut

sub findRef
{
	my( $root, $name, $r ) = @_;

	confess "findRef: no name" if !defined $name || $name eq "";

	$name =~ s/\s+//g;	
	return undef if exists $noref{ $name };

	$name =~ s/^#//g;

	my ($iter, @tree) = split /(?:\:\:|#)/, $name;
	my $kid;

	$refcalls++;

	# Upward search for the first token
	return undef if !defined $iter;

	while ( !defined findIn( $root, $iter ) ) {
		return undef if !defined $root->{Parent};
		$root = $root->{Parent};
	}
	$root = $root->{KidHash}->{$iter};
	carp if !defined $root;

	# first token found, resolve the rest of the tree downwards
	foreach $iter ( @tree ) {
		confess "iter in $name is undefined\n" if !defined $iter;
		next if $iter =~ /^\s*$/;

		unless ( defined findIn( $root, $iter ) ) {
			confess "findRef: failed on '$name' at '$iter'\n"
				if defined $r;
			return undef;
		}

		$root = $root->{KidHash}->{ $iter };	
		carp if !defined $root;
	}

	return $root;
}

=head2 findIn

	node, name: search for a child

=cut

sub findIn
{
	return undef unless defined $_[0]->{KidHash};

	my $ret =  $_[0]->{KidHash}->{ $_[1] };

	return $ret;
}

=head2 linkReferences

	Parameters: root, node

	Recursively links references in the documentation for each node
	to real nodes if they can be found.  This should be called once
	the entire parse tree is filled.

=cut

sub linkReferences
{
	my( $root, $node ) = @_;

	if ( exists $node->{DocNode} ) {
		linkDocRefs( $root, $node, $node->{DocNode} );

		if( exists $node->{Compound} ) {
			linkSee( $root, $node, $node->{DocNode} );
		}
	}

	my $kids = $node->{Kids};
	return unless defined $kids;

	foreach my $kid ( @$kids ) {
		# only continue in a leaf node if it has documentation.
		next if !exists $kid->{Kids} && !exists $kid->{DocNode};
		if( !exists $kid->{Compound} ) {
			linkSee( $root, $node, $kid->{DocNode} );
		}
		linkReferences( $root, $kid );
	}
}

sub linkNamespaces
{
	my ( $node ) = @_;

	if ( defined $node->{ImpNames} ) {
		foreach my $space ( @{$node->{ImpNames}} ) {
			my $spnode = findRef( $node, $space );

			if( defined $spnode ) {
				$node->AddPropList( "ExtNames", $spnode );
			}
			else {
				warn "namespace not found: $space\n";
			}
		}
	}

	return unless defined $node->{Compound} || !defined $node->{Kids};


	foreach my $kid ( @{$node->{Kids}} ) {
		next unless localComp( $kid );

		linkNamespaces( $kid );
	}
}

sub calcStats
{
		my ( $stats, $root, $node ) = @_;
# stats:
# num types
# num nested
# num global funcs
# num methods


		my $type = $node->{NodeType};

		if ( $node eq $root ) {
			# global methods
			if ( defined $node->{Kids} ) {
				foreach my $kid ( @{$node->{Kids}} ) {
						$stats->{Global}++ if $kid->{NodeType} eq "method";
				}
			}

			$node->AddProp( "Stats", $stats );
		}
		elsif ( kdocAstUtil::localComp( $node ) 
					|| $type eq "enum" || $type eq "typedef" ) {
				$stats->{Types}++;
				$stats->{Nested}++ if $node->{Parent} ne $root;
		}
		elsif( $type eq "method" ) {
				$stats->{Methods}++;
		}

		return unless defined $node->{Compound} || !defined $node->{Kids};

		foreach my $kid ( @{$node->{Kids}} ) {
				next if defined $kid->{ExtSource};
				calcStats( $stats, $root, $kid );
		}
}

=head2 linkDocRefs

	Parameters: root, node, docnode

	Link references in the docs if they can be found.  This should
	be called once the entire parse tree is filled.

=cut

sub linkDocRefs
{
	my ( $root, $node, $docNode ) = @_;
	return unless exists $docNode->{Text};

	my ($text, $ref, $item, $tosearch);

	foreach $item ( @{$docNode->{Text}} ) {
		next if $item->{NodeType} ne 'Ref';

		$text = $item->{astNodeName};

		if ( $text =~ /^(?:#|::)/ ) {
			$text = $';
			$tosearch = $node;
		}
		else {
			$tosearch = $root;
		}

		$ref = findRef( $tosearch, $text );
		$item->AddProp( 'Ref', $ref ) if defined $ref;

		confess "Ref failed for ", $item->{astNodeName},
		"\n" unless defined $ref;
	}
}

sub linkSee
{
	my ( $root, $node, $docNode ) = @_;
	return unless exists $docNode->{See};

	my ( $text, $tosearch, $ref );

	foreach $text ( @{$docNode->{See}} ) {
		if ( $text =~ /^\s*(?:#|::)/ ) {
			$text = $';
			$tosearch = $node;
		}
		else {
			$tosearch = $root;
		}

		$ref = findRef( $tosearch, $text );
		$docNode->AddPropList( 'SeeRef', $ref )
			if defined $ref;
	}
}



#
# Inheritance utilities
#

=head2 makeInherit

	Parameter: $rootnode, $parentnode

	Make an inheritance graph from the parse tree that begins
	at rootnode. parentnode is the node that is the parent of
	all base class nodes.

=cut

sub makeInherit
{
	my( $rnode, $parent ) = @_;

	foreach my $node ( @{ $rnode->{Kids} } ) {
		next if !defined $node->{Compound};

		# set parent to root if no inheritance

		if ( !exists $node->{InList} ) {
			newInherit( $node, "Global", $parent );
			$parent->AddPropList( 'InBy', $node );

			makeInherit( $node, $parent );
			next;
		}

		# link each ancestor
		my $acount = 0;
ANITER:
		foreach my $in ( @{ $node->{InList} } ) {
			unless ( defined $in ) {
				Carp::cluck "warning: $node->{astNodeName} "
					." has undef in InList.";
				next ANITER;
			}

			my $ref = kdocAstUtil::findRef( $rnode, 
					$in->{astNodeName} );

			if( !defined $ref ) {
				# ancestor undefined
				warn "warning: ", $node->{astNodeName},
					" inherits unknown class '",
						$in->{astNodeName},"'\n";

				$parent->AddPropList( 'InBy', $node );
			}
			else {
				# found ancestor
				$in->AddProp( "Node", $ref );
				$ref->AddPropList( 'InBy', $node );
				$acount++;
			}
		}

		if ( $acount == 0 ) {
			# inherits no known class: just parent it to global
			newInherit( $node, "Global", $parent );
			$parent->AddPropList( 'InBy', $node );
		}
		makeInherit( $node, $parent );
	}
}

=head2 newInherit

	p: $node, $name, $lnode?

	Add a new ancestor to $node with raw name = $name and
	node = lnode.
=cut

sub newInherit
{
	my ( $node, $name, $link ) = @_;

	my $n = Ast::New( $name );
	$n->AddProp( "Node", $link ) unless !defined $link;

	$node->AddPropList( "InList", $n );
	return $n;
}

=head2 inheritName

	pr: $inheritance node.

	Returns the name of the inherited node. This checks for existence
	of a linked node and will use the "raw" name if it is not found.

=cut

sub inheritName
{
	my ( $innode ) = @_;

	return defined $innode->{Node} ? 
		$innode->{Node}->{astNodeName}
		: $innode->{astNodeName};
}

=head2 inheritedBy

	Parameters: out listref, node

	Recursively searches for nodes that inherit from this one, returning
	a list of inheriting nodes in the list ref.

=cut

sub inheritedBy
{
	my ( $list, $node ) = @_;

	return unless exists $node->{InBy};

	foreach my $kid ( @{ $node->{InBy} } ) {
		push @$list, $kid;
		inheritedBy( $list, $kid );
	}
}

=head2 hasLocalInheritor

	Parameter: node
	Returns: 0 on fail

	Checks if the node has an inheritor that is defined within the
	current library. This is useful for drawing the class hierarchy,
	since you don't want to display classes that have no relationship
	with classes within this library.

	NOTE: perhaps we should cache the value to reduce recursion on 
	subsequent calls.

=cut

sub hasLocalInheritor
{
	my $node = shift;

	return 0 if !exists $node->{InBy};

	my $in;
	foreach $in ( @{$node->{InBy}} ) {
		return 1 if !exists $in->{ExtSource}
			|| hasLocalInheritor( $in );
	}

	return 0;
}



=head2 allMembers

	Parameters: hashref outlist, node, $type

	Fills the outlist hashref with all the methods of outlist,
	recursively traversing the inheritance tree.

	If type is not specified, it is assumed to be "method"

=cut

sub allMembers
{
	my ( $outlist, $n, $type ) = @_;
	my $in;
	$type = "method" if !defined $type;

	if ( exists $n->{InList} ) {

		foreach $in ( @{$n->{InList}} ) {
			next if !defined $in->{Node};
			my $i = $in->{Node};

			allMembers( $outlist, $i ) 
				unless $i == $main::rootNode;
		}
	}

	return unless exists $n->{Kids};

	foreach $in ( @{$n->{Kids}} ) {
		next if $in->{NodeType} ne $type;

		$outlist->{ $in->{astNodeName} } = $in;
	}
}

=head2 findOverride

	Parameters: root, node, name

	Looks for nodes of the same name as the parameter, in its parent
	and the parent's ancestors. It returns a node if it finds one.

=cut

sub findOverride
{
	my ( $root, $node, $name ) = @_;
	return undef if !exists $node->{InList};

	foreach my $in ( @{$node->{InList}} ) {
		my $n = $in->{Node};
		next unless defined $n && $n != $root && exists $n->{KidHash};

		my $ref  = $n->{KidHash}->{ $name };
		
		return $n if defined $ref && $ref->{NodeType} eq "method";

		if ( exists $n->{InList} ) {
			$ref = findOverride( $root, $n, $name );
			return $ref if defined $ref;
		}
	}

	return undef;
}

=head2 attachChild

	Parameters: parent, child

	Attaches child to the parent, setting Access, Kids
	and KidHash of respective nodes.

=cut

sub attachChild
{
	my ( $parent, $child ) = @_;
	confess "Attempt to attach ".$child->{astNodeName}." to an ".
		"undefined parent\n" if !defined $parent;

	$child->AddProp( "Access", $parent->{KidAccess} );
	$child->AddProp( "Parent", $parent );

	$parent->AddPropList( "Kids", $child );

	if( !exists $parent->{KidHash} ) {
		my $kh = Ast::New( "LookupTable" );
		$parent->AddProp( "KidHash", $kh );
	}

	$parent->{KidHash}->AddProp( $child->{astNodeName},
		$child );
}

=head2 makeClassList

	Parameters: node, outlist ref

	fills outlist with a sorted list of all direct, non-external
	compound children of node.

=cut

sub makeClassList
{
	my ( $rootnode, $list ) = @_;

	@$list = ();

	Iter::LocalCompounds( $rootnode,
		sub { 
				my $node = shift;

				my $her = join ( "::", heritage( $node ) );
				$node->AddProp( "FullName", $her );

				if ( !exists $node->{DocNode}->{Internal} ||
				     !$main::skipInternal ) {
					push @$list, $node;
				}
	} );

	@$list = sort { $a->{FullName} cmp $b->{FullName} } @$list;
}

#
# Debugging utilities
#

=head2 dumpAst

	Parameters: node, deep
	Returns: none

	Does a recursive dump of the node and its children.
	If deep is set, it is used as the recursion property, otherwise
	"Kids" is used.

=cut

sub dumpAst
{
	my ( $node, $deep ) = @_;

	$deep = "Kids" if !defined $deep;

	print "\t" x $depth, $node->{astNodeName}, 
		" (", $node->{NodeType}, ")\n";

	my $kid;

	foreach $kid ( $node->GetProps() ) {
		print "\t" x $depth, "  -\t", $kid, " -> ", $node->{$kid},"\n"
			unless $kid =~ /^(astNodeName|NodeType|$deep)$/;
	}
	if ( exists  $node->{InList} ) {
		print "\t" x $depth, "  -\tAncestors -> ";
		foreach my $innode ( @{$node->{InList}} ) {
			print $innode->{astNodeName} . ",";
		}
		print "\n";
	}

	print "\t" x $depth, "  -\n" if (defined $node->{ $deep } && scalar(@{$node->{ $deep }}) != 0);

	$depth++;
	foreach $kid ( @{$node->{ $deep }} ) {
		dumpAst( $kid );
	}

	print "\t" x $depth, "Documentation nodes:\n" if defined 
		@{ $node->{Doc}->{ "Text" }};

	foreach $kid ( @{ $node->{Doc}->{ "Text" }} ) {
		dumpAst( $kid );
	}

	$depth--;
}

=head2 testRef

	Parameters: rootnode

	Interactive testing of referencing system. Calling this
	will use the readline library to allow interactive entering of
	identifiers. If a matching node is found, its node name will be
	printed.

=cut

sub testRef {
	require Term::ReadLine;

	my $rootNode = $_[ 0 ];

	my $term = new Term::ReadLine 'Testing findRef';

	my $OUT = $term->OUT || *STDOUT{IO};
	my $prompt = "Identifier: ";

	while( defined ($_ = $term->readline($prompt)) ) {

		my $node = kdocAstUtil::findRef( $rootNode, $_ );

		if( defined $node ) {
			print $OUT "Reference: '", $node->{astNodeName}, 
			"', Type: '", $node->{NodeType},"'\n";
		}
		else {
			print $OUT "No reference found.\n";
		}

		$term->addhistory( $_ ) if /\S/;
	}
}

sub printDebugStats
{
	print "findRef: ", $refcalls, " calls, ", 
		$refiters, " iterations.\n";
}

sub External
{
	return defined $_[0]->{ExtSource};
}

sub Compound
{
	return defined $_[0]->{Compound};
}

sub localComp
{
	my ( $node ) = $_[0];
	return defined $node->{Compound} 
		&& !defined $node->{ExtSource} 
		&& $node->{NodeType} ne "Forward";
}

sub hasDoc
{
	return defined $_[0]->{DocNode};
}

### Warning: this returns the list of parents, e.g. the 3 words in KParts::ReadOnlyPart::SomeEnum
### It has nothing do to with inheritance.
sub heritage
{
		my $node = shift;
		my @heritage;

		while( 1 ) {
				push @heritage, $node->{astNodeName};

				last unless defined $node->{Parent};
				$node = $node->{Parent};
				last unless defined $node->{Parent};
		}

		return reverse @heritage;
}

sub refHeritage
{
		my $node = shift;
		my @heritage;

		while( 1 ) {
				push @heritage, $node;

				last unless defined $node->{Parent};
				$node = $node->{Parent};
				last unless defined $node->{Parent};
		}

		return reverse @heritage;

}


1;
