#!/usr/bin/perl
while(<>)
{
    if (($key, $domain, $useragent, $comment) = ($_ =~ /(Entry\d)=\.?([^:]+)::([^:]+)::([^\n]+)/))
    {
       printf ("[%s]\n", $domain);
       printf ("UserAgent=%s\n", $useragent);
       printf ("# DELETE %s\n", $key);
    }
}
printf ("# DELETE EntriesCount\n");
