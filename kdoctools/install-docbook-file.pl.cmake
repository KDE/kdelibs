#!/usr/bin/perl
use File::Copy;
use File::Find;

my $srcdir = '${CMAKE_SOURCE_DIR}/kdoctools/';
my $DESTDIR = '${CMAKE_INSTALL_PREFIX}';
my $kde_datadir = '${DATA_INSTALL_DIR}';

foreach my $subdir qw(customization docbook) {
    rcopy("$srcdir/$subdir", "$DESTDIR$kde_datadir/ksgmltools2/$subdir");
}

sub rcopy {
    my ($src, $dest) = @_;

    File::Find::find({
	wanted => sub {
	    return if /\.svn/;
	    my $out = $_;
	    $out =~ s/^\Q$src/$dest/i;
	    if (-d) {
		mkdir $out;
	    } else {
		File::Copy::copy($_ => $out);
	    }
	},
	no_chdir => 1,
    }, $src);
}

