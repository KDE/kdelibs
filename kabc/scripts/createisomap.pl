#!/usr/bin/perl
#
# Create a translation table countryname->iso-code from the entry.desktop
# files in kdebase/l10n/*/
#
# USAGE EXAMPLE:
#     ./createisomap.pl $KDEDIR/share/locale/l10n > countrytransl.map
#
# Don't laugh at me. I put this together with an old perl book, perl
# being a language I've never used before.

@entries = <$ARGV[0]/*/entry.desktop>;
chomp @entries;
foreach $entry (@entries) {
    local ( $entryiso, @entryfile, @mappings );
    # print "--> $entry\n";
    $entryiso = $entry;
    $entryiso =~ s/$ARGV[0]\///;
    $entryiso =~ s/\/entry\.desktop//;
    # print "    $entryiso\n";
    open (IN, $entry);
    @entryfile = <IN>;
    close IN;
    chomp @entryfile;
    foreach $entryfileline (@entryfile) {
        if ( $entryfileline =~ /^Name.*=(.*)$/ ) {
            # push (@mappings, $1 . "\t" . $entryiso );
            print "$1\t$entryiso\n";
        }
    }
}

# add some convenience entries which aren't part of the entry.desktop files

print "Czech Republic\tcz\n";
