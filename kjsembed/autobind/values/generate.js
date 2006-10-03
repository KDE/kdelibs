#!/usr/bin/eval kjscmd -e
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

var intermediate_dir = 'intermediate/';
var output_dir = 'output/';

include ( '../shared/util.js' )
include( 'generateheader.js' )
include( 'generatebinding.js' )

// function process_class_info( classDoc )
//   classDoc - The DOM document that contains the compound objects description.// Processes a class document and creates binding header and source files.
function process_class_info( classDoc )
{
    // Construct the compound doc for the compound object
    var compound = {};

    // Store the root of the class document
    compound.doc = classDoc;

    // Stores the base of the definition
    compound.def = classDoc.firstChild().toElement();

    // Stores the name of the compound object
    compound.name = compound.def.firstChildElement('compoundname').toElement().toString();

    // Stores the name the compound object data will be stored under
    compound.data = compound.name + "Data";

    // Stores the name the compound object binding will be known by
    compound.binding = compound.name + "Binding";

    // Stores a list of elements that define all the members of the object.
    compound.memberList = compound.def.elementsByTagName( "memberdef" );

    // Store the cached enum types with our compound object.
    // compound.enums = enum_array;

    // choose the bindings (TODO: Add ObjectBinding and QObjectBinding).
    if (isVariant( compound.name ))
    {
        compound.bindingFactory = "VariantFactory";
        compound.bindingBase = "VariantBinding";
        compound.isVariant = true;
    }
    else
    {
        compound.bindingFactory = "ValueFactory";
        compound.bindingBase = "ValueBinding";
        compound.isVariant = false;
    }

    println("   Writing Header");
    write_header( compound );

    println("   Writing Binding");
    write_binding_new( compound );
}

function process_class( compound_elem )
{
    var className = compound_elem.firstChild().toElement().toString();
    println( "Found class: " + className );

    // Find the class description file
    var fileName = intermediate_dir + compound_elem.attribute( 'refid' ) + '.xml';

    println( "Loading class file: " + fileName);
    var classInfo = new File( fileName );
    if ( !classInfo.open( File.ReadOnly ) )
        throw "Could not open class info file" + fileName;

    // Read the index
    var content = classInfo.readAll();

    // Create the DOM
    var classDoc = new QDomDocument("class");
    classDoc.setContent( content );
    return classDoc.documentElement();
}


//
// Main
//
println( 'Generating bindings for values...' );

// Read the index
var input = new File( intermediate_dir + 'index.xml' );
if( !input.open( File.ReadOnly ) )
  throw "Unable to open class list";

var content = input.readAll();

// Create the DOM
var index_doc = new QDomDocument("index");
index_doc.setContent( content );
var root = index_doc.documentElement();

// List the classes
var nodeList = root.elementsByTagName( "compound" );

// First pass to gather pertinent data
//   Enums       ... check
//   Inheritance ... not done
var enum_array = {};
for( x = 0; x < nodeList.length(); ++x )
{
    var compoundElement = nodeList.item(x).toElement();
    var compoundKind = compoundElement.attribute('kind');
    if ( compoundKind == 'class' )
    {
        var classRootElement = process_class( compoundElement );
        var compoundDef = classRootElement.firstChildElement('compounddef').toElement();
        var compoundName = compoundDef.firstChildElement('compoundname').toElement().toString();
        var memberList = compoundDef.elementsByTagName( "memberdef" );
        for ( y = 0; y < memberList.length(); ++y )
        {
            var memberElement = memberList.item(y).toElement();
            var memberKind = memberElement.attribute('kind');
            if ( memberKind == 'enum' )
            {
                var enumName = memberElement.firstChildElement('name').toElement().toString();
                var qualifiedEnum = compoundName + "::" + enumName;
                enum_array[enumName] = 1;
                enum_array[qualifiedEnum] = 1;
                println("   Added enum " + qualifiedEnum);
 
/*
                var enumList = memberElement.elementsByTagName( "enumvalue" );
                for ( z = 0; z < enumList.length(); ++z )
                {
                    var enumValue = enumList.item(z).toElement().firstChildElement('name').toElement().toString();
                    var qualifiedEnum = enumName + "::" + enumValue;
                    enum_array[qualifiedEnum] = 1;
                    println( "   Added enum " + qualifiedEnum );
                }
*/
            }
        }
    }
}

// Second pass to actually process classes
for( x = 0; x < nodeList.length(); ++x )
{
    var compound_elem =  nodeList.item(x).toElement();
    var kind = compound_elem.attribute( 'kind' );

    if ( kind == 'class' )
    {
        var classRootElement = process_class( compound_elem );
        process_class_info( classRootElement );
    }
}

println( 'Done' );
