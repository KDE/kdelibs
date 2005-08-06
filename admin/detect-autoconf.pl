#!/usr/bin/env perl

# Try to locate best version of auto*
# By Michael Pyne <michael.pyne@kdemail.net>
#
# Copyright (c) 2005.
# This code is public domain.  You may use it however you like (including
# relicensing).

# Emulate the 'which' program.
sub which
{
    my $prog = shift;
    my @paths = split(/:/, $ENV{'PATH'});

    for $path (@paths)
    {
	return "$path/$prog" if -x "$path/$prog";
    }

    return "";
}

# Subroutine to determine the highest installed version of the given program,
# searching from the given paths.
sub findBest
{
    my ($program, @paths) = @_;
    my $best_version_found = '0'; # Deliberately a string.
    my %versions;

    # Allow user to use environment variable to override search.
    return $ENV{uc $program} if $ENV{uc $program};

    for $prefix (@paths)
    {
	@files = glob "$prefix/$program*";
	for $file (@files)
	{
	    # Don't check non-executable scripts.
	    next unless -x $file;

	    ($version) = $file =~ /$program-?(.*)$/;
	    $version =~ s/-|\.//g;

	    # If no version suffix then use it in favor of a versioned autotool
	    # since the ever-popular WANT_AUTOFOO should then work (in theory).
	    return $file unless $version;

	    # Emulate 'which', and abort if we've already seen this version.
	    next if exists $versions{$version};

	    # Save filename of program.
	    $versions{$version} = $file;

	    # Use string comparison so that e.g. 253a will be > 253 but < 254.
	    if ($version gt $best_version_found)
	    {
		$best_version_found = $version;
	    }
	}
    }

    return $versions{$best_version_found};
}

# Find an appropriate "which" program for later use by the shell script calling
# us.
sub findWhich
{
    for $candidate ('type -p', 'which', 'type')
    {
	$test = `$candidate sh 2>/dev/null`;
	chomp $test;

	return $candidate if -x $test;
    }
}

# Uses which() to find a program unless the user provided its path in the
# environment (the upper case program name is searched).
sub findProgram
{
    $suffix = ""; # For use if @_ has only one param.
    my ($program, $suffix) = @_;

    return $ENV{uc $program} if $ENV{uc $program};
    return which("$program$suffix");
}

# SCRIPT STARTS.

# Search in path.
@paths = split(/:/, $ENV{'PATH'});

# Make sure at least /usr/bin and /usr/local/bin are in this search.
unshift @paths, '/usr/local/bin' unless grep $_ eq '/usr/local/bin', @paths;
unshift @paths, '/usr/bin' unless grep $_ eq '/usr/bin', @paths;

$autoconf = findBest('autoconf', @paths);
($autoconf_suffix) = $autoconf =~ /.*autoconf(.*)$/;

# Find matching autoconf companions.
$autoheader = findProgram('autoheader', $autoconf_suffix);
$autom4te = findProgram('autom4te', $autoconf_suffix);

# Get best automake, and look for unsermake to possibly override it.
$automake = findBest('automake', @paths);
$unsermake = ($ENV{'UNSERMAKE'} ne 'no') ? findProgram('unsermake') : "";

($automake_suffix) = $automake =~ /.*automake(.*)$/;

# Use unsermake if we found it.
$automake = "$unsermake -c" if $unsermake;

# Find matching automake companions.
$aclocal = findProgram('aclocal', $automake_suffix);

$which = findWhich();

# Make sure we have all of the needed programs.
for $i (qw'autoconf autoheader autom4te automake aclocal')
{
    unless(${$i})
    {
	print "# Unable to find $i!!\n";
	exit 1;
    }
}

# Print results in eval-able form.
print <<EOF;
AUTOCONF="$autoconf"
AUTOHEADER="$autoheader"
AUTOM4TE="$autom4te"

AUTOMAKE="$automake"
ACLOCAL="$aclocal"

WHICH="$which"

export AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE ACLOCAL WHICH
EOF

exit 0;
