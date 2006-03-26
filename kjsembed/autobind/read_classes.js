
function process_method( method_elem )
{
  var name = method_elem.firstChildElement('definition').toElement().toString();
  println( "  Method: " + name );
}

function process_class_info( class_doc )
{
  // List the methods
  var methodList = class_doc.documentElement().toElement().elementsByTagName( "memberdef" );
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
  var fname = 'intermediate/' + compound_elem.attribute( 'refid' ) + '.xml';

  classinfo = new File( fname );
  content = '';
  if ( classinfo.open( File.ReadOnly ) ) {
    // Read the index
    while( !classinfo.atEnd() ) {
      content += classinfo.readln();
    }
  }
  else {
    println( 'Could not open file ' + fname );
  }

  // Create the DOM
  var class_doc = new QDomDocument("class");
  class_doc.setContent( content );
  process_class_info( class_doc );
}


//
// Main
//
println( 'About to load class list...' );

var input = new File("intermediate/index.xml");

var content = '';
if( input.open( File.ReadOnly ) )
{
  // Read the index
  while( !input.atEnd() ) {
    content += input.readln();
  }

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

}	


println( 'Done' );
