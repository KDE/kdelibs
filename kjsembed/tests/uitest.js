#!/bin/env kjscmd

var widget = new Form("test.ui", this);
print( "widget value: " + widget );
for( y in widget ) print("widget property: " + y);
widget.show();

exec()
