#!/usr/bin/perl -w

use Config::IniFiles;

defined $ARGV[0] or die 'Too few arguments';

my %ini;
tie %ini, 'Config::IniFiles', ( -file => $ARGV[0] );

check(\%ini);

$header = <<END;
<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE policyconfig PUBLIC
"-//freedesktop//DTD PolicyKit Policy Configuration 1.0//EN"
"http://www.freedesktop.org/standards/PolicyKit/1.0/policyconfig.dtd">
<policyconfig>
END

print $header;

print_tags(\%ini);

print "</policyconfig>\n";

sub print_tags
{
    for my $action (keys %ini)
    {
        my ($desc_k) = grep { lc $_ eq "description" } keys %{$ini{$action}};
        my ($msg_k) = grep { lc $_ eq "message" } keys %{$ini{$action}};
        my ($policy_k) = grep { lc $_ eq "policy" } keys %{$ini{$action}};
        my ($persistence_k) = grep { lc $_ eq "persistence" } keys %{$ini{$action}};
        
        my $desc = $ini{$action}{$desc_k};
        my $msg = $ini{$action}{$msg_k};
        my $policy = $ini{$action}{$policy_k};
        
        if(defined($persistence_k))
        {
            $policy = $policy.'_keep_'.$ini{$action}{$persistence_k}
        }
        
        $tag = <<TAG;
    <action id="$action">
        <description>$desc</description>
        <message>$msg</message>
        <defaults>
            <allow_inactive>no</allow_inactive>
            <allow_active>$policy</allow_active>
        </defaults>
    </action>
TAG
        
        print $tag;
    }
}

sub check
{
    my $ini = shift;
    
    for my $action ( keys %{$ini})
    {
        my $msg = 0;
        my $desc = 0;
        my $policy = 0;
        my $persistence = 0;
        
        $action =~ /^[a-z]+(\.[a-z]+)*$/ or die('Non va');
        for my $entry ( keys %{$ini{$action}})
        {
            lc($entry) =~ /^(message|description|policy|persistence)$/ or die "Unrecognized entry: $entry";
            
            $msg = 1 if lc($entry) eq 'message';
            $desc = 1 if lc($entry) eq 'description';
            if(lc($entry) eq 'policy')
            {
                $policy = 1;
                lc($ini{$action}{$entry}) =~ /^(yes|no|auth_self|auth_admin)$/ or die "Unrecognized policy: $ini{$action}{$entry}";
            }
            
            if(lc($entry) eq 'persistence')
            {
                lc($ini{$action}{$entry}) =~ /^(session|always)$/ or die "Unrecognized persistence option: $ini{$action}{$entry}"
            }
        }
        
        ($msg and $desc and $policy) or die "Entry missing";
    }
}

