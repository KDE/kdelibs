package kdocParseDoc;

use Ast;
use strict;

use vars qw/ $buffer $docNode %extraprops $currentProp $propType /;

=head1 kdocParseDoc

	Routines for parsing of javadoc comments.

=head2 newDocComment

	Parameters: begin (starting line of declaration)

	Reads a doc comment to the end and creates a new doc node.

	Read a line
	check if it changes the current context
		yes
			flush old context
			check if it is a non-text tag 
						(ie internal/deprecated etc)
				yes
					reset context to text
					set associated property
				no
					set the new context
					assign text to new buffer
		no	add to text buffer
			continue
	at end
		flush anything pending.

=cut

sub newDocComment
{
	my( $text ) = @_;
	return undef unless $text =~ m#/\*\*+#;

	setType( "DocText", 2 );
	$text =~ m#/\*#; # need to do the match again, otherwise /***/ doesn't parse
	### TODO update this method from kdoc
	$buffer = $'; # everything after the first \*
	$docNode = undef;
	%extraprops = ();	# used for textprops when flushing.
	my $finished = 0;
	my $inbounded = 0;

	if ( $buffer =~ m#\*/# ) {
		$buffer = $`;
		$finished = 1;
	}

PARSELOOP:
	while ( defined $text && !$finished ) {
		# read text and remove leading junk
		$text = main::readSourceLine();
		next if !defined $text;
		$text =~ s#^\s*\*(?!\/)##;

#		if ( $text =~ /^\s*<\/pre>/i ) {
#			flushProp();
#			$inbounded = 0;
#		}
		if( $inbounded ) {
			if ( $text =~ m#\*/# ) {
				$finished = 1;
				$text = $`;
			}
			$buffer .= $text;
			next PARSELOOP;
		}
#		elsif ( $text =~ /^\s*<pre>/i ) {
#			textProp( "Pre" );
#			$inbounded = 1;
#		}
		elsif ( $text =~ /^\s*$/ ) {
			textProp( "ParaBreak", "\n" );
		}
		elsif ( $text =~ /^\s*\@internal\s*/ ) {
			codeProp( "Internal", 1 );
		}
		elsif ( $text =~ /^\s*\@deprecated\s*/ ) {
			codeProp( "Deprecated", 1 );
		}
		elsif ( $text =~ /^\s*\@reimplemented\s*/ ) {
			codeProp( "Reimplemented", 1 );
		}
		elsif ( $text =~ /^\s*\@group\s*/ ) {
			# logical group tag in which this node belongs
			# multiples allowed

			my $groups = $';
			$groups =~ s/^\s*(.*?)\s*$/$1/;

			if ( $groups ne "" ) {
				foreach my $g ( split( /[^_\w]+/, $groups) ) {

					codeProp( "InGroup", $g );
				}
			}
		}
		elsif ( $text =~ /^\s*\@defgroup\s+(\w+)\s*/ ) {
			# parse group tag and description
			my $grptag = $1;
			my $grpdesc = $' eq "" ? $grptag : $';
			
			# create group node
			my $grpnode = Ast::New( $grptag );
			$grpnode->AddProp( "Desc", $grpdesc );
			$grpnode->AddProp( "NodeType", "GroupDef" );

			# attach
			codeProp( "Groups", $grpnode );
		}
		elsif ( $text =~ /^\s*\@see\s*/ ) {
			docListProp( "See" );
		}
		elsif( $text =~ /^\s*\@short\s*/ ) {
			docProp( "ClassShort" );
		}
		elsif( $text =~ /^\s*\@author\s*/ ) {
			docProp( "Author" );

		}
		elsif( $text =~ /^\s*\@version\s*/ ) {
			docProp( "Version" );
		}
		elsif( $text =~ /^\s*\@id\s*/ ) {

			docProp( "Id" );
		}
		elsif( $text =~ /^\s*\@since\s*/ ) {
			docProp( "Since" );
		}
		elsif( $text =~ /^\s*\@returns?\s*/ ) {
			docProp( "Returns" );
		}
		elsif( $text =~ /^\s*\@(?:throws|exception|raises)\s*/ ) {
			docListProp( "Throws" );
		}
		elsif( $text =~ /^\s*\@image\s+([^\s]+)\s*/ ) {
			textProp( "Image" );
			$extraprops{ "Path" } = $1;
		}
		elsif( $text =~ /^\s*\@param\s+(\w+)\s*/ ) {
			textProp( "Param" );
			$extraprops{ "Name" } = $1;
		}
		elsif( $text =~ /^\s*\@sect\s+/ ) {

			textProp( "DocSection" );
		}
		elsif( $text =~ /^\s*\@li\s+/ ) {

			textProp( "ListItem" );
		}
		elsif ( $text =~ /^\s*\@libdoc\s+/ ) {
			# Defines the text for the entire library
			docProp( "LibDoc" );
		}
		else {
			if ( $text =~ m#\*/# ) {
				$finished = 1;
				$text = $`;
			}
			$buffer .= $text;
		}
	}

	flushProp();


	return undef if !defined $docNode;

# postprocess docnode

	# add a . to the end of the short if required.
	my $short = $docNode->{ClassShort};

	if ( defined $short ) {
		if ( !($short =~ /\.\s*$/) ) {
			$docNode->{ClassShort} =~ s/\s*$/./;
		}
	}
	else {
		# use first line of normal text as short name.
		if ( defined $docNode->{Text} ) {
			my $node;
			foreach $node ( @{$docNode->{Text}} ) {
				next if $node->{NodeType} ne "DocText";
				$short = $node->{astNodeName};
				$short = $`."." if $short =~ /\./;
				$docNode->{ClassShort} = $short;
				goto shortdone;
			}
		}
	}
shortdone:

# Join and break all word list props so that they are one string per list 
# node, ie remove all commas and spaces.

	recombineOnWords( $docNode, "See" );
	recombineOnWords( $docNode, "Throws" );

	return $docNode;
}

=head3 setType

	Parameters: propname, proptype ( 0 = single, 1 = list, 2 = text )

	Set the name and type of the pending property.

=cut

sub setType
{
	( $currentProp, $propType ) = @_;
}

=head3 flushProp

	Flush any pending item and reset the buffer. type is set to DocText.

=cut

sub flushProp
{
	return if $buffer eq "";
	initDocNode() unless defined $docNode;

	if( $propType == 1 ) {
		# list prop
		$docNode->AddPropList( $currentProp, $buffer );
	}
	elsif ( $propType == 2 ) {
		# text prop
		my $textnode = Ast::New( $buffer );
		$textnode->AddProp( 'NodeType', $currentProp );
		$docNode->AddPropList( 'Text', $textnode );
		
		foreach my $prop ( keys %extraprops ) {
			$textnode->AddProp( $prop, 
				$extraprops{ $prop } );
		}

		%extraprops = ();
	}
	else {
		# one-off prop
		$docNode->AddProp( $currentProp, $buffer );
	}

	# reset buffer
	$buffer = "";
	setType( "DocText", 2 );
}

=head3 codeProp

	Flush the last node, add a new property and reset type to DocText.

=cut

sub codeProp
{
	my( $prop, $val ) = @_;

	flushProp();

	initDocNode() unless defined $docNode;
	$docNode->AddPropList( $prop, $val );
	
	setType( "DocText", 2 );

}

=head3 docListProp

	The next item is a list property of docNode.

=cut

sub docListProp
{
	my( $prop ) = @_;

	flushProp();

	$buffer = $';
	setType( $prop, 1 );
}

=head3 docProp

	The next item is a simple property of docNode.

=cut

sub docProp
{
	my( $prop ) = @_;
	
	flushProp();

	$buffer = $';
	setType( $prop, 0 );
}

=head3 textProp

	Parameters: prop, val

	Set next item to be a 'Text' list node. if val is assigned, the
	new node is assigned that text and flushed immediately. If this
	is the case, the next item is given the 'DocText' text property.

=cut

sub textProp
{
	my( $prop, $val ) = @_;

	flushProp();

	if ( defined $val ) {
		$buffer = $val;
		setType( $prop, 2 );
		flushProp();
		$prop = "DocText";
	}

	setType( $prop, 2 );
	$buffer = $';
}


=head3 initDocNode

	Creates docNode if it is not defined.

=cut

sub initDocNode
{
	$docNode = Ast::New( "Doc" );
	$docNode->AddProp( "NodeType", "DocNode" );
}

sub recombineOnWords
{
	my ( $docNode, $prop ) = @_;

	if ( exists $docNode->{$prop} ) {
		my @oldsee = @{$docNode->{$prop}};
		@{$docNode->{$prop}} = split (/[\s,]+/, join( " ", @oldsee ));
	}
}

###############

=head2 attachDoc

Connects a docnode to a code node, setting any other properties
if required, such as groups, internal/deprecated flags etc.

=cut

sub attachDoc
{
	my ( $node, $doc, $rootnode ) = @_;

	$node->AddProp( "DocNode", $doc );
	$node->AddProp( "Internal", 1 ) if defined $doc->{Internal};
	$node->AddProp( "Deprecated", 1 ) if defined $doc->{Deprecated};

	# attach group definitions if they exist
	if ( defined $doc->{Groups} ) {
		my $groupdef = $rootnode->{Groups};
		if( !defined $groupdef ) {
			$groupdef = Ast::New( "Groups" );
			$rootnode->AddProp( "Groups", $groupdef );
		}

		foreach my $grp ( @{$doc->{Groups}} ) {
			if ( defined $groupdef->{ $grp->{astNodeName} } ) {
				$groupdef->{ $grp->{ astNodeName}
				}->AddProp( "Desc", $grp->{Desc} );
			}
			else {
				$groupdef->AddProp( $grp->{astNodeName}, $grp );
			}
		}
	}

	# attach node to group index(es)
	# create groups if not found, they may be parsed later.

	if ( defined $doc->{InGroup} ) {
		my $groupdef = $rootnode->{Groups};

		foreach my $grp ( @{$doc->{InGroup}} ) {
			if ( !exists $groupdef->{$grp} ) {
				my $newgrp = Ast::New( $grp );
				$newgrp->AddProp( "Desc", $grp );
				$newgrp->AddProp( "NodeType", "GroupDef" );
				$groupdef->AddProp( $grp, $newgrp );
			}

			$groupdef->{$grp}->AddPropList( "Kids", $node );
		}
	}
}

1;
