#!/bin/perl
while(<>)
{
    $useproxy = 1 if /UseProxy=true/;
    $usepac = 1 if /Proxy Config Script=.*\S/;
}
printf ("# DELETE UseProxy\n");
$proxytype = $useproxy ? ($usepac ? 2 : 1) : 0;
print "ProxyType=$proxytype\n";
