
var intermediate_dir = 'intermediate/';
var output_dir = 'output/';

/*
 * Generate the header file for binding the class specified in the passed DOM element.
 */
function write_header( class_doc )
{
  var compounddef = class_doc.firstChild().toElement();
  var includes = compounddef.firstChildElement('includes').toElement().toString();
  var compoundname = compounddef.firstChildElement('compoundname').toElement().toString();

  var template =
    '#ifndef BIND_' + compoundname + '_H\n' +
    '#define BIND_' + compoundname + '_H\n' +
    '\n' +
    '#include <' + includes + '>\n' +
    '#include <kjsembed/qobject_binding.h>\n' +
    '\n' +
    'KJS_BINDING( Binding' + compoundname + ' )\n' +
    '' +
    '#endif // BIND_' + compoundname + '_H';

  var fname = output_dir + compoundname + '_bind.h';
  header = new File( fname );
  
  if( !header.open( File.WriteOnly ) )
    throw "Unable to open output header, " + fname;

  header.writeln( template );
  header.close();
}

function write_binding_method( compounddef, method_elem )
{
  var compoundname = compounddef.firstChildElement('compoundname').toElement().toString();

  var type = method_elem.firstChildElement('type').toElement().toString();
  var name = method_elem.firstChildElement('name').toElement().toString();
  var args = method_elem.firstChildElement('argsstring').toElement().toString();

  var method_template =
    '\n' +
    '// ' + type + ' ' + name + args + '\n' +
    'START_QOBJECT_METHOD( ' + name + ', ' + compoundname + ')\n' +
    '/* stuff */\n' +
    'END_QOBJECT_METHOD\n';

  return method_template;
}

function write_binding_methodlut( compounddef )
{
  var compoundname = compounddef.firstChildElement('compoundname').toElement().toString();

  var lut_template =
    '\n' +
    'START_METHOD_LUT( Bind' + compoundname + ')\n';

  // Generate the binding for each method
  var methodList = compounddef.elementsByTagName( "memberdef" );
  for( var idx = 0; idx < methodList.length(); ++idx ) {
	var member_elem = methodList.item(idx).toElement();
	var kind = member_elem.attribute( 'kind' );
	var name = member_elem.firstChildElement('name').toElement().toString();

	if ( kind == 'function' ) {
	  lut_template += '{' + name + ', 0, KJS::DontDelete|KJS::ReadOnly, &Binding' + compoundname + 'NS::' + name + ' },\n';
	}
  }


  lut_template += 'END_METHOD_LUT\n';

  return lut_template;
}

function write_binding( class_doc )
{
  var compounddef = class_doc.firstChild().toElement();
  var includes = compounddef.firstChildElement('includes').toElement().toString();
  var compoundname = compounddef.firstChildElement('compoundname').toElement().toString();

  var template = 
    '#include <QDebug>\n' +
    '\n' +
    '#include <kjsembed/object_binding.h>\n' +
    '#include <kjsembed/value_binding.h>\n' +
    '#include <kjs/object.h>\n' +
    '\n' +
    '#include <' + compoundname + '_bind.h>\n' +
    '\n' +
    'using namespace KJSEmbed;\n' +
    '\n' +
    'namespace Binding' + compoundname + 'NS {\n';

  // Generate the binding for each method
  var methodList = class_doc.elementsByTagName( "memberdef" );
  for( idx = 0; idx < methodList.length(); ++idx ) {
	var member_elem = methodList.item(idx).toElement();
	var kind = member_elem.attribute( 'kind' );

	if ( kind == 'function' ) {
	  template += write_binding_method( compounddef, member_elem );
	}
  }

  template += '};\n';
  template += '\n';
  template += write_binding_methodlut( compounddef );

  var fname = output_dir + compoundname + '_bind.cpp';

  binding = new File( fname );
  if( !binding.open( File.WriteOnly ) )
    throw "Unable to open output binding, " + fname;

  binding.writeln( template );
  binding.close();
}

function process_class_info( class_doc )
{
  write_header( class_doc );
  write_binding( class_doc );
}

function process_class( compound_elem )
{
  var name = compound_elem.firstChild().toElement().toString();
  println( "Found class: " + name );

  // Find the class description file
  var fname = intermediate_dir + compound_elem.attribute( 'refid' ) + '.xml';

  var classinfo = new File( fname );
  if ( !classinfo.open( File.ReadOnly ) )
    throw "Could not open class info file" + fname;

  // Read the index
  var content = classinfo.readAll();

  // Create the DOM
  var class_doc = new QDomDocument("class");
  class_doc.setContent( content );
  process_class_info( class_doc.documentElement().toElement() );
}


//
// Main
//
println( 'About to load class list...' );


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
for( x = 0; x < nodeList.length(); ++x ) {
  var compound_elem =  nodeList.item(x).toElement();
  var kind = compound_elem.attribute( 'kind' );
  
  if ( kind == 'class' ) {
    process_class( compound_elem );
  }
}

println( 'Done' );
