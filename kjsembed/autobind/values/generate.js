
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


function write_ctor( compoundDef )
{
    var compoundName = compoundDef.firstChildElement('compoundname').toElement().toString();
    var ctor =
        '\n' +
        'START_CTOR( ' + compoundName + ', ' + compoundName + ', 0 )\n';

    // Generate the ctor bindings
    var methodList = compoundDef.elementsByTagName( "memberdef" );
    for( var idx = 0; idx < methodList.length(); ++idx )
    {
        var memberElement = methodList.item(idx).toElement();
        var memberKind = memberElement.attribute( 'kind' );
        var memberName = memberElement.firstChildElement('name').toElement().toString();
        var memberArgList = memberElement.elementsByTagName('param');
        if ( memberKind == 'function' )
        {
            if ( memberName.indexOf('operator') == -1 ) // Make sure this is not an operator.
            {
                if ( memberName.indexOf(compoundName) != -1 )
                {
                    if ( memberName.indexOf('~') == -1 )
                    {
                        ctor += '   if (args.size() == ' + memberArgList.count() + ' )\n' +
                                '   {\n';

                        var tmpArgs = '';
                        if ( memberArgList.count() == 0 )
                        {
                            tmpArgs = compoundName + '()';
                        }

                        for ( argIdx = 0; argIdx < memberArgList.count(); ++argIdx )
                        {
                            var param = memberArgList.item(argIdx).toElement();
                            var paramType = param.firstChildElement('type').toElement().toString();
                            var paramVar = param.firstChildElement('declname').toElement().toString();
                            var paramDefault = param.firstChildElement('defval').toElement();

                            if ( paramType.indexOf('Qt::') != -1 )  // Enum Value
                            {
                                ctor +=
                                    '       ' + paramType + ' ' + paramVar + ' = static_cast<' + paramType + '>(KJSEmbed::extractInt(exec, args, ' + argIdx + ', ';
                            }
                            else
                            {
                                ctor +=
                                    '       ' + paramType + ' ' + paramVar + ' = (KJSEmbed::extractObject<' + paramType + '>(exec, args, ' + argIdx + ', ';
                            }

                            if (!paramDefault.isNull())
                                ctor += paramDefault.toString() + '));\n';
                            else
                                ctor += '0));\n';

                            tmpArgs += paramVar + ', ';
                        }

                        var tmpIdx = tmpArgs.lastIndexOf(',');
                        tmpArgs = tmpArgs.substr(0, tmpIdx);

                        ctor +=
                            '       return new KJSEmbed::' + compoundName + 'Binding(exec, ' + compoundName + '(' + tmpArgs + '))\n' +
                            '   }\n';
                    }
                }
            }
        }
    }

    ctor += 'END_CTOR\n';
    return ctor;
}


function write_method_lut( compoundDef )
{
    var compoundName = compoundDef.firstChildElement('compoundname').toElement().toString();
    var lut_template =
        '\n' +
        'START_METHOD_LUT( ' + compoundName + ' )\n';

    // Generate the binding for each method
    var methodList = compoundDef.elementsByTagName( "memberdef" );
    for( var idx = 0; idx < methodList.length(); ++idx )
    {
        var memberElement = methodList.item(idx).toElement();
        var memberKind = memberElement.attribute('kind');
        var memberProt = memberElement.attribute('prot');
        var memberName = memberElement.firstChildElement('name').toElement().toString();

        var numParams = memberElement.elementsByTagName("param").count();
        if ( memberKind == 'function' )
        {
            if ( memberProt == 'public' )
            {
                if ( memberName.indexOf('operator') == -1 ) // Make sure this is not an operator.
                {
                    if ( memberName.indexOf(compoundName) == -1 ) // Make sure this is not a ctor or dtor
                    {
                    lut_template += '    { '+ memberName +', '+ numParams +', KJS::DontDelete|KJS::ReadOnly, &' + compoundName + 'NS::' + memberName + ' },\n';
                    }
                }
            }
        }
    }
    lut_template += 'END_METHOD_LUT\n';

    return lut_template;
}

function write_binding_new( class_doc )
{
    // This is just looking at brush.cpp and determining the order of the source..
    var includes = '<QtGui>';
    var bindingCtor = '';
    var methods = '';
    var enums = '';
    var statics = '';
    var ctor = '';
    var methodLut = '';

    // Eventually all of these should be moved to their own functions, but once again
    // lets get it working first..

    // These are vars we need for all of this to work correctly
    var compoundDef = class_doc.firstChild().toElement();
    var compounIncludes = compoundDef.firstChildElement('includes').toElement().toString();
    var compoundName = compoundDef.firstChildElement('compoundname').toElement().toString();

    // Binding Ctor
    bindingCtor +=
        '\n' +
        '#include <' + compoundName + '_bind.h>\n' +
        '\n' +
        'using namespace KJSEmbed;\n' +
        '\n' +
        compoundName + 'Binding::' + compoundName + '( KJS::ExecState *exec, const ' + compoundName +' &value )\n' +
        '   : ValueBinding(exec, value)\n' +
        '{\n' +
        '   StaticBinding::publish(exec, this, ' + compoundName + '::methods() );\n' +
        '   StaticBinding::publish(exec, this, ValueFactory::methods() );\n' +
        '}\n\n';

    // Methods
    methods +=
        'namespace ' + compoundName + 'NS\n' +
        '{\n';

    var methodList = class_doc.elementsByTagName( "memberdef" );
    for( idx = 0; idx < methodList.length(); ++idx )
    {
        var methodElement = methodList.item(idx).toElement();
        var methodKind = methodElement.attribute('kind');
        var methodProt = methodElement.attribute('prot');
        var methodName = methodElement.firstChildElement('name').toElement().toString();

        if ( methodKind == 'function' ) // Make sure we're working with a function here
        {
            if ( methodProt == 'public' )
            {
                if ( methodName.indexOf('operator') == -1 ) // Make sure this is not an operator.
                {
                    if ( methodName.indexOf(compoundName) == -1 ) // Not a ctor
                    {
                        var methodType = methodElement.firstChildElement('type').toElement().toString();
                        var methodArgs = methodElement.firstChildElement('argsstring').toElement().toString();
                        methods +=
                            '\n' +
                            '// ' + methodType + ' ' + methodName + methodArgs + '\n' +
                            'START_VARIANT_METHOD( ' + methodName + ', ' + compoundName + ' )\n';

                        // Handle arguments
                        var methodArgList = methodElement.elementsByTagName('param');
                        if ( methodArgList.count() == 0 )
                        {
                            methods +=
                            '   ' + methodType + ' tmp = value.' + methodName + '();\n';

                            if ( methodType.indexOf('Qt::') != -1 )  // Enum Value
                            {
                                methods += 
                                '   result = KJS::Number( tmp );\n';
                            }
                            else
                            {
                                methods +=
                                "   result = KJSEmbed::createValue( exec, \"" + methodType + "\", tmp );\n";
                            }
                        }
                        for ( paramIdx = 0; paramIdx < methodArgList.count(); ++paramIdx )
                        {
                            var param = methodArgList.item(paramIdx).toElement();
                            var paramType = param.firstChildElement('type').toElement().toString();
                            var paramVar = param.firstChildElement('declname').toElement().toString();
                            var paramDefault = param.firstChildElement('defval').toElement();

                            if ( paramType.indexOf('Qt::') != -1 )  // Enum Value
                            {
                                methods +=
                                    '   ' + paramType + ' ' + paramVar + ' = static_cast<' + paramType + '>(KJSEmbed::extractInt(exec, args, ' + paramIdx + ', ';
                            }
                            else
                            {
                                methods +=
                                    '   ' + paramType + ' ' + paramVar + ' = (KJSEmbed::extractObject<' + paramType + '>(exec, args, ' + paramIdx + ', ';
                            }

                            if (!paramDefault.isNull())
                                methods += paramDefault.toString() + '));\n';
                            else
                                methods += '0));\n';
                        }

                        methods += 'END_VARIANT_METHOD\n';
                    }
                }
            }
        }
    }

    methods +=
        '}\n';

    // Enums
    enums += 'NO_ENUMS( ' + compoundName + ' ) \n';

    // Statics
    enums += 'NO_STATICS( ' + compoundName + ' ) \n';

    // Ctor
    ctor = write_ctor( compoundDef );

    // Method LUT
    methodLut += write_method_lut( compoundDef );


    // Write everything
    var fileName = output_dir + compoundName + '_bind.cpp';
    var bindingFile = new File( fileName );
    if( !bindingFile.open( File.WriteOnly ) )
        throw "Unable to open output binding, " + fileName;

    bindingFile.writeln( includes );
    bindingFile.writeln( bindingCtor );
    bindingFile.writeln( methods );
    bindingFile.writeln( enums );
    bindingFile.writeln( statics );
    bindingFile.writeln( ctor );
    bindingFile.writeln( methodLut );
    bindingFile.close();
}

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
