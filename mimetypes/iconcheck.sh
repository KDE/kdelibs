#!/bin/sh

# Script that checks that the mimetypes point to an existing icon
# Shows only errors, unless -verbose is used

verbose=0
grepopt="-q"
if [ $# = 1 ]; then
  if [ $1 = "-verbose" ]; then verbose=1; grepopt="-q"; fi
fi
for i in */*top ; do
    icon=`grep Icon $i | sed -e s/Icon=//` 
    if [ -z "$icon" ]; then
       echo "$i : NO ICON"
    else
      if [ $verbose = 1 ]; then
        echo $i
      fi
      if ! find ../pics | grep $grepopt "mime\-$icon\.png" ; then
       if [ $verbose = 0 ]; then
         echo -n "$i : "
       fi
       echo $icon NOT FOUND 
      fi 
    fi 
done  
