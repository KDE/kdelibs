
var intermediate_dir = 'intermediate/';
var output_dir = 'output/';

/*
 * Generate the header file for binding the class specified in the passed DOM element.
 */
function write_header( class_doc )
{
  compounddef = class_doc.firstChild().toElement();
  includes = compounddef.firstChildElement('includes').toElement().toString();
  compoundname = compounddef.firstChildElement('compoundname').toElement().toString();

  template =
    '#ifndef BIND_' + compoundname + '_H\n' +
    '#define BIND_' + compoundname + '_H\n' +
    '\n' +
    '#include <' + includes + '>\n' +
    '#include <kjsembed/qobject_binding.h>\n' +
    '\n' +
    'KJS_BINDING( BINDING' + compoundname + ' )\n' +
    '' +
    '#endif // BIND_' + compoundname + '_H';

  var fname = output_dir + compoundname + '.h';
  header = new File( fname );
  
  if( !header.open( File.WriteOnly ) )
    throw "Unable to open output header";

  header.writeln( template );
  header.close();
}

function process_method( method_elem )
{
  var type = method_elem.firstChildElement('type').toElement().toString();
  var name = method_elem.firstChildElement('name').toElement().toString();
  var args = method_elem.firstChildElement('argsstring').toElement().toString();
  println( '   Return: ' + type + '  Method: ' + name + ' args ' + args );
}

function process_class_info( class_doc )
{
  write_header( class_doc );

  // List the methods
  var methodList = class_doc.elementsByTagName( "memberdef" );
  for( idx = 0; idx < methodList.length(); ++idx ) {
	var member_elem = methodList.item(idx).toElement();
	var kind = member_elem.attribute( 'kind' );

	if ( kind == 'function' ) {
	  process_method( member_elem );
	}
  }
}

function process_class( compound_elem )
{
  var name = compound_elem.firstChild().toElement().toString();
  println( "Found class: " + name );

  // Find the class description file
  var fname = intermediate_dir + compound_elem.attribute( 'refid' ) + '.xml';

  classinfo = new File( fname );
  content = '';
  if ( !classinfo.open( File.ReadOnly ) )
    throw "Could not open class info file" + fname;

  // Read the index
  content = classinfo.readAll();

  // Create the DOM
  var class_doc = new QDomDocument("class");
  class_doc.setContent( content );
  process_class_info( class_doc.documentElement().toElement() );
}


//
// Main
//
println( 'About to load class list...' );

var input = new File( intermediate_dir + 'index.xml' );

var content = '';
if( !input.open( File.ReadOnly ) )
  throw "Unable to open class list";

// Read the index
content = input.readAll();


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
