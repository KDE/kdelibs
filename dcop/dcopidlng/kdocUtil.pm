
package kdocUtil;

use strict;


=head1 kdocUtil

	General utilities.

=head2 countReg

	Parameters: string, regexp

	Returns the number of times of regexp occurs in string.

=cut

sub countReg
{
	my( $str, $regexp ) = @_;
	my( $count ) = 0;

	while( $str =~ /$regexp/s ) {
		$count++;
		
		$str =~ s/$regexp//s;
	}

	return $count;
}

=head2 findCommonPrefix

	Parameters: string, string

	Returns the prefix common to both strings. An empty string
	is returned if the strings have no common prefix.

=cut

sub findCommonPrefix
{
	my @s1 = split( "/", $_[0] );
	my @s2 = split( "/", $_[1] );
	my $accum = "";
	my $len = ($#s2 > $#s1 ) ? $#s1 : $#s2;

	for my $i ( 0..$len ) {
#		print "Compare: $i '$s1[$i]', '$s2[$i]'\n";
		last if $s1[ $i ] ne $s2[ $i ];
		$accum .= $s1[ $i ]."/";
	}

	return $accum;
}

=head2 makeRelativePath

	Parameters: localpath, destpath
	
	Returns a relative path to the destination from the local path,
	after removal of any common prefix.

=cut

sub makeRelativePath
{
	my ( $from, $to ) = @_;

	# remove prefix
	$from .= '/' unless $from =~ m#/$#;
	$to .= '/' unless $to =~ m#/$#;

	my $pfx = findCommonPrefix( $from, $to );

	if ( $pfx ne "" ) {
		$from =~ s/^$pfx//g;
		$to =~ s/^$pfx//g;
	}
#	print "Prefix is '$pfx'\n";
	
	$from =~ s#/+#/#g;
	$to =~ s#/+#/#g;
	$pfx = countReg( $from, '\/' );

	my $rel = "../" x $pfx;
	$rel .= $to;

	return $rel;
}

sub hostName
{
	my $host = "";
	my @hostenvs = qw( HOST HOSTNAME COMPUTERNAME );

	# Host name
	foreach my $evar ( @hostenvs ) {
			next unless defined $ENV{ $evar };

			$host = $ENV{ $evar };
			last;
	}

	if( $host eq "" ) {
			$host = `uname -n`;
			chop $host;
	}

	return $host;
}

sub userName
{
	my $who = "";
	my @userenvs = qw( USERNAME USER LOGNAME );

	# User name
	foreach my $evar ( @userenvs ) {
			next unless defined $ENV{ $evar };

			$who = $ENV{ $evar };
			last;
	}

	if( $who eq "" ) {
		if ( $who = `whoami` ) {
				chop $who;
		}
		elsif ( $who - `who am i` ) {
				$who = ( split (/ /, $who ) )[0];
		}
	}

	return $who;
}

=head2 splitUnnested
	Helper to split a list using a delimiter, but looking for
	nesting with (), {}, [] and <>.
        Example: splitting   int a, QPair<c,b> d, e=","
	on ',' will give 3 items in the list.

	Parameter: delimiter, string
	Returns: array, after splitting the string

	Thanks to Ashley Winters
=cut
sub splitUnnested($$) {
    my $delim = shift;
    my $string = shift;
    my(%open) = (
        '[' => ']',
        '(' => ')',
        '<' => '>',
        '{' => '}',
    );
    my(%close) = reverse %open;
    my @ret;
    my $depth = 0;
    my $start = 0;
    my $indoublequotes = 0;
    while($string =~ /($delim|<<|>>|[][}{)(><\"])/g) {
        my $c = $1;
        if(!$depth and !$indoublequotes and $c eq $delim) {
            my $len = pos($string) - $start - 1;
            push @ret, substr($string, $start, $len);
            $start = pos($string);
        } elsif($open{$c}) {
            $depth++;
        } elsif($close{$c}) {
            $depth--;
        } elsif($c eq '"') {
	    if ($indoublequotes) {
		$indoublequotes = 0;
	    } else {
		$indoublequotes = 1;
	    }
	}
    }

    my $subs = substr($string, $start);
    push @ret, $subs if ($subs);
    return @ret;
}

1;

