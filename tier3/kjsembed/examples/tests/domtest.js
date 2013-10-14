/* This file is part of the KDE libraries
    Copyright (C) 2005, 2006 KJSEmbed Authors
    See included AUTHORS file.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


var doc = new QDomDocument("MyXML");
var root = doc.createElement("MyXML");
doc.appendChild(root);

var tag = doc.createElement("Greeting");
tag.setAttribute("Property", "Hello World");
root.appendChild( tag );

var text = doc.createTextNode("SomeText");
text.setNodeValue("Some text value");
tag.appendChild( text );

println("Generated doc: " + doc);

var myxml = "<MyXML><Greeting Property=\"Hello World\"/></MyXML>";
var newDoc = new QDomDocument("MyXML");
newDoc.setContent(myxml);
newDoc.documentElement().setAttribute("Property", "Goodbye World");
println("Read doc:" + newDoc );


var nodeList = root.elementsByTagName( "Greeting" );
for( x = 0; x < nodeList.length(); ++x)
{
	var elem =  nodeList.item(x).toElement();
	elem.setAttribute("New-Property", 100 );
	elem.setAttribute("Next-Property", new QColor("blue") );
}

println("Modified doc:" + doc );
