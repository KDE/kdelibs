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
//debug('generatebinding.js');

// function extract_parameter(param, paramIdx, compoundEnums)
//   param         - Parameter DOM element
//   paramIdx      - Parameter index
//   compoundEnums - Associative array of enum types to containing objects
// Constructs a string to extract the passed in param from the arguments.
//   NOTE: Will probably be moved to a central location after more polish.
function extract_parameter( param, paramIdx, compoundEnums )
{
    var extracted = '';
    var paramType = param.firstChildElement('type').toElement().toString();
    var paramVarElement = param.firstChildElement('declname').toElement();
    var paramVar = paramVarElement.toString();
    var paramDefault = param.firstChildElement('defval').toElement();

    if (paramVarElement.isNull())
        paramVar = 'arg' + paramIdx;

    coreParamType = findCoreParamType(paramType);
    if ( isBool(coreParamType) )
    {
        extracted +=
            '        ' + coreParamType + ' ' + paramVar + ' = KJSEmbed::extractBool(exec, args, ' + paramIdx;

        if (!paramDefault.isNull())
            extracted += ', ' + paramDefault.toString() + ');\n';
        else
            extracted += ');\n';

        return extracted;
    }
    else if ( isInteger(coreParamType) )  // integral value
    {
        extracted +=
            '        ' + coreParamType + ' ' + paramVar + ' = KJSEmbed::extractInteger<' + coreParamType + '>(exec, args, ' + paramIdx;

        if (!paramDefault.isNull())
            extracted += ', ' + paramDefault.toString() + ');\n';
        else
            extracted += ');\n';

        return extracted;
    }
    else if ( isNumber(coreParamType) )  // integral value
    {
        extracted +=
            '        ' + coreParamType + ' ' + paramVar + ' = KJSEmbed::extractNumber<' + coreParamType + '>(exec, args, ' + paramIdx;

        if (!paramDefault.isNull())
            extracted += ', ' + paramDefault.toString() + ');\n';
        else
            extracted += ');\n';

        return extracted;
    }
    else if ( paramType.indexOf('Qt::') != -1 )  // Enum Value
    {
        extracted +=
            '        ' + paramType + ' ' + paramVar + ' = KJSEmbed::extractInteger<' + coreParamType + '>(exec, args, ' + paramIdx;

        if (!paramDefault.isNull())
            extracted += ', ' + paramDefault.toString() + ');\n';
        else
            extracted += ');\n';

        return extracted;
    }
    else if ( compoundEnums[paramType] )  // Enum Value
    {
        extracted +=
            '       ' + compoundEnums[paramType] + '::' + paramType + ' ' + paramVar + ' = KJSEmbed::extractInteger<' + compoundEnums[paramType] + '::' + paramType + '>(exec, args, ' + paramIdx;

        if (!paramDefault.isNull())
            extracted += ', ' + compoundEnums[paramType] + '::' + paramDefault.toString() + ');\n';
        else
            extracted += ');\n';

        return extracted;
    }
    else if ( isVariant(coreParamType) )
    {
        extracted +=
            '        ' + coreParamType + ' ' + paramVar + ' = KJSEmbed::extractVariant<' + coreParamType + '>(exec, args, ' + paramIdx + ');\n';
        return extracted;
    }
    else    // It's an object, or something else?
    {
        extracted +=
            '        ' + coreParamType + ' ' + paramVar + ' = KJSEmbed::extractObject<' + coreParamType + '>(exec, args, ' + paramIdx + ', ';
    }

    if (!paramDefault.isNull())
        extracted += paramDefault.toString() + ');\n';
    else
        extracted += '0);\n';

    return extracted;
}

// function construct_parameters(methodList, numArgs, funcCallStart, funcCallEnd, compoundEnums)
//   methodList    - The method description for this
//   numArgs       - The number of parameters (arguments)
//   funcCallStart - The start of the function call, sans argument list
//   funcCallEnd   - The closing portion of the function call
// Constructs the function binding for a particular method overload containing
// numArgs arguments.
//   NOTE: Will probably be moved to a central location after more polish.
function construct_parameters(methodList, numArgs, funcCallStart, funcCallEnd, compoundEnums)
{
    var params = '';
    for(var argIdx = 0; argIdx < numArgs; ++argIdx)
    {
        params += '        KJS::JSValue* value'+argIdx+'=args['+argIdx+'];\n';
        params += '        KJS::JSObject* object'+argIdx+'=value'+argIdx+'->toObject(exec);\n';
    }
    for(var idx = 0; idx < methodList.length; ++idx)
    {
        var memberArgList = methodList[idx].elementsByTagName('param');
        var variables = '';
        params += '        if(';
        var tmpArgs = '';
        for(var argIdx = 0; argIdx < numArgs; ++argIdx)
        {
            var param = memberArgList.item(argIdx).toElement();
            var paramVarElement = param.firstChildElement('declname').toElement();
            var paramVar = paramVarElement.toString();
            if (paramVarElement.isNull())
                paramVar = 'arg' + paramIdx;

            tmpArgs += paramVar + ', ';
            var paramType = param.firstChildElement('type').toElement().toString();
            var coreParamType = findCoreParamType(paramType);
//debug('paramType: "' + paramType + '" coreParamType = "' + coreParamType + '"');
            var paramDefault = param.firstChildElement('defval').toElement();

            if (!paramDefault.isNull())
                params += '( ( ';

            if (isBool(coreParamType))
                params += 'object'+argIdx+' && object'+argIdx+'->isBoolean()';
            else if ( isNumber(coreParamType) || 
                      isEnum(coreParamType, compoundEnums ) )
                params += 'object'+argIdx+' && object'+argIdx+'->isNumber()';
            else if (coreParamType == 'QString')
                params += 'object'+argIdx+' && object'+argIdx+'->isString()';
            else //if(isVariant(paramType))
                params += 'object'+argIdx+' && object'+argIdx+'->inherits(&' + coreParamType + 'Binding::info)';
//            else
//                params += 'isBasic(value'+argIdx+')';

            if (!paramDefault.isNull())
                params += ' ) || !object'+argIdx+' )';

            if(argIdx < numArgs-1)
                params += ' && ';

            variables += '    ' + extract_parameter(param, argIdx, compoundEnums);
        }

        var tmpIdx = tmpArgs.lastIndexOf(',');
        tmpArgs = tmpArgs.substr(0, tmpIdx);

        params += ')\n';
        params += '        {\n';
        params += variables;
        params += '            ' + funcCallStart + tmpArgs + funcCallEnd;
        params += '        }\n';
    }
    return params;
}

// function write_enums(compoundName, compoundData, memberList, compoundEnums)
//   compoundName  - The name of the compound object
//   compoundData  - The data object to use for the compound object
//   compoundDef   - The base element of the compound objects definition
//   compoundEnums - An associative array of enums defined within the 
//                   compound object.
// Constructs and fills out the enums
function write_enums(compoundName, compoundData, memberList, compoundEnums)
{
    var enums = '';

    // Process the enums
    enums +=
        '\n' +
        'const Enumerator KJSEmbed::' + compoundData + '::p_enums[] = {\n';

    var hasEnums = false;
    for( idx = 0; idx < memberList.length(); ++idx )
    {
        var memberElement = memberList.item(idx).toElement();
        var memberKind = memberElement.attribute('kind');
        var memberProt = memberElement.attribute('prot');
        var memberName = memberElement.firstChildElement('name').toElement().toString();
        if ( memberKind == 'enum' )
        {
            if ( memberProt == 'public' )
            {
                println( '      Processing enum ' + memberName );
                hasEnums = true;
                compoundEnums[memberName] = compoundName;
                var enumValueList = memberElement.elementsByTagName( 'enumvalue' );
                for( enumidx = 0; enumidx < enumValueList.length(); ++enumidx )
                {
                    var valueName = enumValueList.item( enumidx ).toElement().firstChildElement('name').toElement().toString();
                    println( '        ' + valueName );
                    enums += '    {"' + valueName + '", ' + compoundName + '::' + valueName + ' },\n';
                }
            }
        }
    }

    if ( hasEnums )
    {
        enums +=
            '    {0, 0}\n' +
            '};\n';
    }
    else {
        enums = 'const Enumerator KJSEmbed::' + compoundData + '::p_enums[] = {{0, 0 }};\n';
    }

    return enums;
}


// function write_enums(compoundName, compoundData, memberList, compoundEnums)
//   compoundName  - The name of the compound object
//   compoundData  - The data object to use for the compound object
//   compoundDef   - The base element of the compound objects definition
//   compoundEnums - An associative array of enums defined within the 
//                   compound object.
// Constructs and fills out the enums
function write_methods(compoundName, compoundData, memberList, compoundEnums)
{
    var methods = 'namespace ' + compoundName + 'NS\n' +
        '{\n';

    // Process Methods
    for( idx = 0; idx < memberList.length(); ++idx )
    {
        var memberElement = memberList.item(idx).toElement();
        var memberKind = memberElement.attribute('kind');
        var memberProt = memberElement.attribute('prot');
        var memberName = memberElement.firstChildElement('name').toElement().toString();

        if ( memberKind == 'function' ) // Make sure we're working with a function here
        {
            if ( memberProt == 'public' )
            {
                if ( memberName.indexOf('operator') == -1 ) // Make sure this is not an operator.
                {
                    if ( memberName.indexOf(compoundName) == -1 ) // Not a ctor
                    {
                        var methodType = memberElement.firstChildElement('type').toElement().toString();
                        var coreMethodType = findCoreParamType(methodType);
                        var methodArgs = memberElement.firstChildElement('argsstring').toElement().toString();
                        // Handle arguments
                        var methodArgList = memberElement.elementsByTagName('param');

                        methods +=
                            '// ' + methodType + ' ' + memberName + methodArgs + '\n' +
                            'KJS::JSValue *'+ memberName + '( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) \n' +
                            '{ \n';

                        if (methodArgList.count() == 0)
                            methods +=
                                '    Q_UNUSED(args); \n';

                        methods +=
                            '    KJS::JSValue *result = KJS::Null(); \n' +
                            '    KJSEmbed::VariantBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::VariantBinding>(exec, self); \n' +
                            '    if( imp ) \n' +
                            '    { \n' +
                            '        ' + compoundName + ' value = imp->value<' + compoundName + '>();\n';

                        if ( methodArgList.count() == 0 )
                        {
                            if (methodType == "void")
                            {
                            }
                            else if (compoundEnums[coreMethodType]) 
                            {
                                methods +=
                                '       ' + compoundEnums[coreMethodType] + '::' + coreMethodType + ' tmp = value.' + memberName + '();\n';
                                methods += 
                                '         result = KJS::Number( tmp );\n';
                            }
                            else
                            {
                                methods +=
                                '        ' + methodType + ' tmp = value.' + memberName + '();\n';
                                if ( coreMethodType.indexOf('Qt::') != -1 ) // Enum Value
                                {
                                    methods += 
                                    '        result = KJS::Number( tmp );\n';
                                }
                                else if (isVariant(coreMethodType))
                                {
                                    methods +=
                                    "        result = KJSEmbed::createVariant( exec, \"" + coreMethodType + "\", tmp );\n";
                                }
                                else
                                {
                                    methods +=
                                    "        result = KJSEmbed::createObject( exec, \"" + coreMethodType + "\", tmp );\n";
                                }
                            }
                        }

                        var tmpArgs = '';
                        for ( paramIdx = 0; paramIdx < methodArgList.count(); ++paramIdx )
                        {
                            var param = methodArgList.item(paramIdx).toElement();
                            var paramVarElement = param.firstChildElement('declname').toElement();
                            var paramVar = paramVarElement.toString();
                            if (paramVarElement.isNull())
                                paramVar = 'arg' + paramIdx;
                            var paramDefault = param.firstChildElement('defval').toElement();
                            methods += extract_parameter(param, paramIdx, compoundEnums);

                            tmpArgs += paramVar + ', ';
                        }

                        if (tmpArgs != '') 
                        {
                            var tmpIdx = tmpArgs.lastIndexOf(',');
                            tmpArgs = tmpArgs.substr(0, tmpIdx);
                            var tmpIdx =
                                    methods += '        value.' + memberName + '(' + tmpArgs + ');\n';
                        }

                        methods +=
                        '        imp->setValue(qVariantFromValue(value)); \n' +
                        '    }\n' +
                        '    else \n' +
                        '    {\n' +
                        "        KJS::throwError(exec, KJS::GeneralError, \"We have a problem baby\");\n" +
                        '    }\n\n' +
                        '    return result; \n' +
                        '}\n\n';
                    }
                }
            }
        }
    }

    methods +=
        '}\n';
}

// function write_method_lut( compoundDef)
//   compoundDef   - The base element of the compound objects definition
// Constructs the method look-up-table (lut) for the compound object
function write_method_lut( compoundDef )
{
    var compoundName = compoundDef.firstChildElement('compoundname').toElement().toString();
    var lut_template =
        '\n' +
        'const Method KJSEmbed::' + compoundData + '::p_methods[] = \n' +
        '{\n';

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
                    lut_template += '    { "'+ memberName +'", '+ numParams +', KJS::DontDelete|KJS::ReadOnly, &' + compoundName + 'NS::' + memberName + ' },\n';
                    }
                }
            }
        }
    }

    lut_template +=
    '    {0, 0, 0, 0 }\n' +
    '};\n';

    return lut_template;
}

// function write_ctor( compoundDef, compoundEnums )
//   compoundDef   - The base element of the compound objects definition
//   compoundEnums - An associative array of enums defined within the 
//                   compound object.
// Constructs the constructor for handling the creation of the object
function write_ctor( compoundDef, compoundEnums )
{
    var compoundName = compoundDef.firstChildElement('compoundname').toElement().toString();
    var ctor =
        'const Constructor KJSEmbed::' + compoundData + '::p_constructor = \n' +
        '{'+
        '"' + compoundName + '", 0, KJS::DontDelete|KJS::ReadOnly, &' + compoundData + '::ctorMethod, p_statics, p_enums, KJSEmbed::' + compoundData + '::p_methods };\n' +
        'KJS::JSObject *KJSEmbed::' + compoundData + '::ctorMethod( KJS::ExecState *exec, const KJS::List &args )\n' +
        '{\n';

    // Generate the ctor bindings
    var methodList = compoundDef.elementsByTagName( "memberdef" );
    var methodListList = new Array;
    for( var idx = 0; idx < methodList.length(); ++idx )
    {
        var memberElement = methodList.item(idx).toElement();
        var memberKind = memberElement.attribute( 'kind' );
        var memberProt = memberElement.attribute('prot');
        var memberName = memberElement.firstChildElement('name').toElement().toString();
        var memberArgList = memberElement.elementsByTagName('param');
        if (( memberKind == 'function' ) && // Constructor is a function
            ( memberProt == 'public' ) && // Make sure it is public
            ( memberName.indexOf('operator') == -1 ) && // Make sure this is not an operator.
            ( memberName.indexOf(compoundName) != -1 ) && // This _is_ a ctor
            ( memberName.indexOf('~') == -1 )) // This is _not_ a dtor
        {
            var args = memberArgList.count();
            if(!methodListList[args]) {
                methodListList[args] = new Array;
            }
            methodListList[args].push( memberElement );
        }
    }

    for(var idx = 0; idx < methodListList.length; ++idx)
    {
        if(!methodListList[idx]) continue;
        var memberElement = methodListList[idx][0];
        var memberArgList = memberElement.elementsByTagName('param');
        ctor += '    if (args.size() == ' + memberArgList.count() + ' )\n' +
        '    {\n';

        var tmpArgs = '';
        if ( memberArgList.count() == 0 )
        {
            tmpArgs =  + '()';
            ctor += 
            '        return new KJSEmbed::' + compoundName + 'Binding(exec, ' + compoundName + '());\n';
        }
        else
        {
            for ( argIdx = 0; argIdx < memberArgList.count(); ++argIdx )
            {
                var param = memberArgList.item(argIdx).toElement();
                var paramVarElement = param.firstChildElement('declname').toElement();
                var paramVar = paramVarElement.toString();
                if (paramVarElement.isNull())
                    paramVar = 'arg' + paramIdx;

                tmpArgs += paramVar + ', ';
            }

            var tmpIdx = tmpArgs.lastIndexOf(',');
            tmpArgs = tmpArgs.substr(0, tmpIdx);
//            var funcCall = 'return new KJSEmbed::' + compoundName + 'Binding(exec, ' + compoundName + '(' + tmpArgs + '));\n';
            var funcCallStart = 'return new KJSEmbed::' + compoundName + 'Binding(exec, ' + compoundName + '('; 
            var funcCallEnd = '));\n';

            ctor += construct_parameters(methodListList[idx], idx, funcCallStart, funcCallEnd, compoundEnums);
        }

        ctor += '    }\n';
    }
    ctor += '    return KJS::throwError(exec, KJS::SyntaxError, "Syntax error in parameter list for ' + compoundName + '");\n'
    + '}';
    return ctor;
}

// function write_binding_new( class_doc )
//   class_doc - The root element of the class DOM document.
// Writes the binding for the class described in class_doc to a file
// with a name composed as '{ClassName}_bind.cpp'.
function write_binding_new( class_doc )
{
    // This is just looking at brush.cpp and determining the order of the source..
    var includes = '';
    var bindingCtor = '';
    var statics = '';
    var ctor = '';
    var methodLut = '';

    // Eventually all of these should be moved to their own functions, but once again
    // lets get it working first..

    // These are vars we need for all of this to work correctly
    var compoundDef = class_doc.firstChild().toElement();
    var compoundIncludes = compoundDef.firstChildElement('includes').toElement().toString();
    var compoundName = compoundDef.firstChildElement('compoundname').toElement().toString();
    var compoundEnums = {};
    
    compoundData = compoundName + "Data";

//    println ( "compoundIncludes: " + compoundIncludes );
    includes += '#include "' + compoundName + '_bind.h"\n';
    includes += "#include <" + compoundIncludes + ">\n";
    includes += "#include <object_binding.h>\n";

    // Binding Ctor
    bindingCtor +=
        '\n' +
        'using namespace KJSEmbed;\n' +
        '\n' +
        "const KJS::ClassInfo " + compoundName + "Binding::info = { \""+ compoundName + "\", &VariantBinding::info, 0, 0 };\n" +
        compoundName + 'Binding::' + compoundName + 'Binding( KJS::ExecState *exec, const ' + compoundName +' &value )\n' +
        '   : VariantBinding(exec, value)\n' +
        '{\n' +
        '    StaticBinding::publish(exec, this, ' + compoundData + '::methods() );\n' +
        '    StaticBinding::publish(exec, this, ValueFactory::methods() );\n' +
        '}\n\n';

    var memberList = class_doc.elementsByTagName( "memberdef" );

    var enums = write_enums(compoundName, compoundData, memberList, compoundEnums);

    var methods = write_methods(compoundName, compoundData, memberList, compoundEnums);

    // Statics
    statics += 'NO_STATICS( KJSEmbed::' + compoundData + ' )';

    // Ctor
    ctor = write_ctor( compoundDef, compoundEnums );

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
