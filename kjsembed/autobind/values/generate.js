var intermediate_dir = 'intermediate/';
var output_dir = 'output/';

include( 'generateheader.js' )
include( 'generatebinding.js' )

function process_class_info( classDoc )
{
    println("   Writing Header");
    write_header( classDoc );
    println("   Writing Binding");
    write_binding_new( classDoc );
}

function process_class( compound_elem )
{
    var className = compound_elem.firstChild().toElement().toString();
    println( "Found class: " + className );

    // Find the class description file
    var fileName = intermediate_dir + compound_elem.attribute( 'refid' ) + '.xml';

    var classInfo = new File( fileName );
    if ( !classInfo.open( File.ReadOnly ) )
        throw "Could not open class info file" + fileName;

    // Read the index
    var content = classInfo.readAll();

    // Create the DOM
    var classDoc = new QDomDocument("class");
    classDoc.setContent( content );
    process_class_info( classDoc.documentElement().toElement() );
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
for( x = 0; x < nodeList.length(); ++x )
{
    var compound_elem =  nodeList.item(x).toElement();
    var kind = compound_elem.attribute( 'kind' );

    if ( kind == 'class' )
    {
        process_class( compound_elem );
    }
}

println( 'Done' );
