
var intermediate_dir = 'intermediate/';
var output_dir = 'output/';

/*
 * Generate the header file for binding the class specified in the passed DOM element.
 */
function write_header( classDoc )
{
    var compounddef = classDoc.firstChild().toElement();
    var includes = compounddef.firstChildElement('includes').toElement().toString();
    var compoundname = compounddef.firstChildElement('compoundname').toElement().toString();

    var template =
        '#ifndef BIND_' + compoundname + '_H\n' +
        '#define BIND_' + compoundname + '_H\n' +
        '\n' +
        '#include <value_binding.h>\n' +
        '#include <static_binding.h>\n' +
        '\n' +
        'class ' + compoundname + ';\n' +
        '\n' +
        'namespace KJSEmbed\n' +
        '{\n' +
        '   class ' + compoundname + 'Binding : public ValueBinding\n' +
        '   {\n' +
        '       public:\n' +
        '           ' + compoundname + 'Binding( KJS::ExecState *exec, const ' + compoundname + ' &value );\n' +
        '   };\n\n' +
        '   KJS_BINDING( ' + compoundname + ' )\n\n' +
        '}\n' +
        '#endif // BIND_' + compoundname + '_H\n\n';

    var fileName = output_dir + compoundname + '_bind.h';
    header = new File( fileName );

    if( !header.open( File.WriteOnly ) )
        throw "Unable to open output header, " + fileName;

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

function write_method_lut( compounddef )
{
  var compoundname = compounddef.firstChildElement('compoundname').toElement().toString();

  var lut_template =
    '\n' +
    'START_METHOD_LUT( ' + compoundname + ')\n';

  // Generate the binding for each method
  var methodList = compounddef.elementsByTagName( "memberdef" );
  for( var idx = 0; idx < methodList.length(); ++idx )
  {
        var member_elem = methodList.item(idx).toElement();
        var kind = member_elem.attribute( 'kind' );
        var name = member_elem.firstChildElement('name').toElement().toString();

        var numParams = member_elem.elementsByTagName("param").count();
        if ( kind == 'function' )
        {
            lut_template += '{'+ name +', '+ numParams +', KJS::DontDelete|KJS::ReadOnly, &' + compoundname + 'NS::' + name + ' },\n';
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

    var includeFiles = "#include <QDebug>\n";

    var template =
        '\n' +
        '#include <' + compoundname + '_bind.h>\n' +
        '\n' +
        'using namespace KJSEmbed;\n' +
        '\n' +
        compoundname + 'Binding::' + compoundname + '( KJS::ExecState *exec, const ' + compoundname +' &value )\n' +
        '   : ValueBinding(exec, value)\n' +
        '{\n' +
        '   StaticBinding::publish(exec, this, ' + compoundname + '::methods() );\n' +
        '   StaticBinding::publish(exec, this, ValueFactory::methods() );\n' +
        '}\n\n' +
        'namespace ' + compoundname + 'NS\n' +
        '{\n';

    // Generate the binding for each method
    var methodList = class_doc.elementsByTagName( "memberdef" );
    var methodType = '';
    var methodName = '';
    var methodArgs = '';
    var methodArgList;
    var paramType = '';
    var paramVar = '';
    var paramDefault = '';
    for( idx = 0; idx < methodList.length(); ++idx )
    {
        var member_elem = methodList.item(idx).toElement();
        var kind = member_elem.attribute( 'kind' );
        methodName = member_elem.firstChildElement('name').toElement().toString();

        if ( kind == 'function' ) // Make sure we're working with a function here
        {
            if ( methodName.indexOf('operator') == -1 ) // Make sure this is not an operator.
            {
                // These should all be eventually refactored to separate functions
                // but get it working first ;)

                if ( methodName.indexOf(compoundname) == -1 ) // Not a ctor
                {
                    methodType = member_elem.firstChildElement('type').toElement().toString();
                    methodArgs = member_elem.firstChildElement('argsstring').toElement().toString();
                    template = template +
                        '\n' +
                        '// ' + methodType + ' ' + methodName + methodArgs + '\n' +
                        'START_VARIANT_METHOD( ' + methodName + ', ' + compoundname + ' )\n';

                    // Handle arguments
                    methodArgList = member_elem.elementsByTagName('param');
                    for ( idx2 = 0; idx2 < methodArgList.length(); ++idx2 )
                    {
                        var param = methodArgList.item(idx2).toElement();
                        paramType = param.firstChildElement('type').toElement().toString();
                        paramVar = param.firstChildElement('declname').toElement().toString();
                        paramDefault = param.firstChildElement('defval').toElement();

                        if ( paramType.indexOf('Qt::') != -1 )  // Enum Value
                        {
                            println('enum value');
                            template = template +
                                '   ' + paramType + ' ' + paramVar + ' = (' + paramType + ')KJSEmbed::extractInt(exec, args, ' + idx2 + ', ';
                        }
                        else
                        {
                            template = template +
                                '   ' + paramType + ' ' + paramVar + ' = KJSEmbed::extractObject<' + paramType + '>(exec, args, ' + idx2 + ', ';
                        }

                        if (!paramDefault.isNull())
                            template = template + paramDefault.toString() + ');\n';
                        else
                            template = template + '0);\n';
                    }

                    template = template + 'END_VARIANT_METHOD\n';
                }

 //             if ( methodName.indexOf('~') != -1 ) // This is a ctor
                else
                {
                    println('ctor');
                    methodArgList = member_elem.elementsByTagName('param');

                    template = template +
                        '\n' +
                        'START_CTOR(' + compoundname + ', ' + compoundname + ', ' + methodArgList.count() + ' )\n';

                    var tmpArgs = '';
                    for ( idx3 = 0; idx3 < methodArgList.count(); ++idx3 )
                    {
                        var param = methodArgList.item(idx3).toElement();
                        paramType = param.firstChildElement('type').toElement().toString();
                        paramVar = param.firstChildElement('declname').toElement().toString();
                        paramDefault = param.firstChildElement('defval').toElement();


                        if ( paramType.indexOf('Qt::') != -1 )  // Enum Value
                        {
                            template = template +
                                '   ' + paramType + ' ' + paramVar + ' = (' + paramType + ')KJSEmbed::extractInt(exec, args, ' + idx3 + ', ';
                        }
                        else
                        {
                            template = template +
                                '   ' + paramType + ' ' + paramVar + ' = KJSEmbed::extractObject<' + paramType + '>(exec, args, ' + idx3 + ', ';
                        }

                        if (!paramDefault.isNull())
                            template = template + paramDefault.toString() + ');\n';
                        else
                            template = template + '0);\n';

                        tmpArgs = tmpArgs + paramVar + ', ';
                    }

                    var tmpIdx = tmpArgs.lastIndexOf(',');
                    tmpArgs = tmpArgs.substr(0, tmpIdx);

                    template = template + 
                        '   return new KJSEmbed::' + compoundname + 'Binding(exec, ' +
                        compoundname + '(' + tmpArgs + '))\n' +
                    'END_CTOR\n';
                }
            }
        }
    }

    template += '}\n\n';
    template += write_method_lut( compounddef );

    var fileName = output_dir + compoundname + '_bind.cpp';

    binding = new File( fileName );
    if( !binding.open( File.WriteOnly ) )
        throw "Unable to open output binding, " + fileName;

    binding.writeln( includeFiles );
    binding.writeln( template );
    binding.close();
}

function process_class_info( classDoc )
{
    println("   Writing Header");
    write_header( classDoc );
    println("   Writing Binding");
    write_binding( classDoc );
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
