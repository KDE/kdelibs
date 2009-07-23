#!/usr/bin/perl -w

#   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU Lesser General Public License as published by
#   the Free Software Foundation; either version 2.1 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public License
#   along with this program; if not, write to the
#   Free Software Foundation, Inc.,
#   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .


use Config::IniFiles 2.52;

(defined $ARGV[0] and defined $ARGV[1]) or die('Too few parameters');

my $ini = new Config::IniFiles -file => $ARGV[0], -nocase => 1 or die "Unable to open input file: $ARGV[0]";

open OUT, "> $ARGV[1]" or die "Unable to open output file: $ARGV[1]";

$header = <<END;
<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE policyconfig PUBLIC
"-//freedesktop//DTD PolicyKit Policy Configuration 1.0//EN"
"http://www.freedesktop.org/standards/PolicyKit/1.0/policyconfig.dtd">
<policyconfig>
END

print OUT $header;

for my $action ($ini->Sections)
{
    # Check the action name syntax
    $action =~ /^[a-z]+(\.[a-z]+)*$/ or die("Wrong action syntax: $action");
    
    # Check mandatory options
    $ini->exists($action, 'description') or die("Description option missing in action: $action");
    $ini->exists($action, 'message') or die("Message option missing in action: $action");
    $ini->exists($action, 'policy') or die("Policy option missing in action: $action");
    
    my $description = $ini->val($action, 'description');
    my $message = $ini->val($action, 'message');
    my $policy = $ini->val($action, 'policy');
    
    if($ini->exists($action, 'persistence'))
    {
        $ini->val($action, 'persistence') =~ /^(always|session)$/ or die "Invalid persistence value in action: $action";
        $policy = $policy.'_keep_'.$ini->val($action, 'persistence');
    }
    
    my $tag = <<TAG;
    <action id="$action">
        <description>$description</description>
        <message>$message</message>
        <defaults>
            <allow_inactive>no</allow_inactive>
            <allow_active>$policy</allow_active>
        </defaults>
    </action>
TAG
    
    print OUT $tag;
}

print OUT "</policyconfig>\n";