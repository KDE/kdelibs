#!/usr/bin/perl -w
#
# Filter to extract comments for translation from cupsd.conf.template
#
# This code should produce strings identical to tooltips in cupsdcomment.cpp
#
my ($comment_, $example_);

load(); # Skip header

while ( <STDIN> )
{
    if(load())
    {
        print toolTip();
    }
}

# Corresponds to Comment::load in cupsdcomment.cpp
sub load
{
    $comment_ = "";
    my($current) = \$comment_;
    while ( <STDIN> )
    {
        if (/^\$\$/)
        {
            $current = \$example_;
        }
        elsif (/^\%\%/)
        {
            next; # Do nothing
        }
        elsif (/^\@\@/)
        {
            return 1;
        }
        elsif (/^[\s]*$/)
        {
            next; # Do nothing
        }
        else
        {
            last if (!/^\#/);
            ${$current} = ${$current} . $_;
        }
    }
    return 0;
}

# Corresponds to Comment::toolTip in cupsdcomment.cpp
sub toolTip
{
    my($str) = $comment_;
    $str =~ s/\"/\\\"/g;
    $str =~ s/^\#[\s]*/i18n\(\"/;
    $str =~ s/\n\#[\s]*/\"\n\"/g;
    $str =~ s/\n$/\"\n\)\;\n\n/;
    return $str;
}
