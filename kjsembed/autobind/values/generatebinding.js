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

// function extract_parameter( compound, param, paramIdx )
//   compound      - The compound object
//   param         - Parameter DOM element
//   paramIdx      - Parameter index
// Constructs a string to extract the passed in param from the arguments.
//   NOTE: Will probably be moved to a central location after more polish.
function extract_parameter( compound, param, paramIdx )
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
    else if ( compound.enums[paramType] )  // Enum Value
    {
        var enumHeading = "";
        if (compound.enums[paramType] != 1)
            enumHeading = compound.enums[paramType] + "::";

        extracted +=
            '       ' + enumHeading + paramType + ' ' + paramVar + ' = KJSEmbed::extractInteger<' + enumHeading + paramType + '>(exec, args, ' + paramIdx;

        if (!paramDefault.isNull())
            extracted += ', ' + enumHeading + paramDefault.toString() + ');\n';
        else
            extracted += ');\n';

        return extracted;
    }
    else if (coreParamType == 'QString')
    {
        extracted +=
            '        ' + coreParamType + ' ' + paramVar + ' = KJSEmbed::extractQString(exec, args, ' + paramIdx;

        if (!paramDefault.isNull())
            extracted += ', ' + paramDefault.toString() + ');\n';
        else
            extracted += ');\n';

        return extracted;
    }
    else if (contains(coreParamType, 'uchar') ||
             contains(coreParamType, 'char'))
    {
        extracted +=
            '        ' + coreParamType + ' ' + paramVar + ' = KJSEmbed::extractString<' + coreParamType + '>(exec, args, ' + paramIdx;

        if (!paramDefault.isNull())
            extracted += ', ' + paramDefault.toString() + ');\n';
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
            '        ' + coreParamType + ' ' + paramVar + ' = KJSEmbed::extractValue<' + coreParamType + '>(exec, args, ' + paramIdx;
    }

    if (!paramDefault.isNull())
        extracted +=  ', ' + paramDefault.toString() + ');\n';
    else
        extracted += ');\n';

    return extracted;
}

// function construct_parameters(compound, numArgs, funcCallStart, funcCallEnd)
//   compound      - The compound object
//   overloadList  - The list of method overloads
//   numArgs       - The number of parameters (arguments)
//   funcCallStart - The start of the function call, sans argument list
//   funcCallEnd   - The closing portion of the function call
// Constructs the function binding for a particular method overload containing
// numArgs arguments.
//   NOTE: Will probably be moved to a central location after more polish.
function construct_parameters(compound, overloadList, numArgs, funcCallStart, funcCallEnd)
{
    var params = '';
    for (var argIdx = 0; argIdx < numArgs; ++argIdx)
    {
        params += '        KJS::JSValue* value'+argIdx+'=args['+argIdx+'];\n';
        params += '        KJS::JSObject* object'+argIdx+'=value'+argIdx+'->toObject(exec);\n';
    }

    for (var idx = 0; idx < overloadList.length; ++idx)
    {
        var memberArgList = overloadList[idx].elementsByTagName('param');
        var variables = '';
        params += '        if(';
        var tmpArgs = '';
        for(var argIdx = 0; argIdx < numArgs; ++argIdx)
        {
            var param = memberArgList.item(argIdx).toElement();
            var paramVarElement = param.firstChildElement('declname').toElement();
            var paramVar = paramVarElement.toString();
            if (paramVarElement.isNull())
                paramVar = 'arg' + argIdx;

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
                      isEnum(coreParamType, compound.enums ) )
                params += 'object'+argIdx+' && object'+argIdx+'->isNumber()';
            else if (coreParamType == 'QString' ||
                    contains(coreParamType, 'uchar') ||
                    contains(coreParamType, 'char'))
                params += 'object'+argIdx+' && object'+argIdx+'->isString()';
            else //if(isVariant(paramType))
            {
                if (coreParamType != compound.name)
                    compound.externalBindings[coreParamType] = true;
                params += 'object'+argIdx+' && object'+argIdx+'->inherits(&' + coreParamType + 'Binding::info)';
            }
//            else
//                params += 'isBasic(value'+argIdx+')';

            if (!paramDefault.isNull())
                params += ' ) || !object'+argIdx+' )';

            if(argIdx < numArgs-1)
                params += ' && ';

            variables += '    ' + extract_parameter(compound, param, argIdx);
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

// function write_enums( compound )
//   compound   - The compound object
// Constructs and fills out the enums
function write_enums( compound )
{
    var memberList = compound.memberList;
    var enums = '';

    // Process the enums
    enums +=
        '\n' +
        'const Enumerator KJSEmbed::' + compound.data + '::p_enums[] = {\n';

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
                compound.enums[memberName] = compound.name;
                var enumValueList = memberElement.elementsByTagName( 'enumvalue' );
                for( enumidx = 0; enumidx < enumValueList.length(); ++enumidx )
                {
                    var valueName = enumValueList.item( enumidx ).toElement().firstChildElement('name').toElement().toString();
                    println( '        ' + valueName );
                    enums += '    {"' + valueName + '", ' + compound.name + '::' + valueName + ' },\n';
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
        enums = 'const Enumerator KJSEmbed::' + compound.data + '::p_enums[] = {{0, 0 }};\n';
    }
 
    return enums;
}

// function find_method_overloads(memberList, startIdx, name)
//   memberList - The DOM list of all memberdef XML elements.
//   startIdx   - The index to start searching for dups at.
//   name       - The name of the method overloads to search for.
// Finds all the method overloads named name in the memberList starting at 
// startIdx
function find_method_overloads(memberList, startIdx, name)
{
    var overloadList = new Array;

    // Process Methods
    for( mIdx = startIdx; mIdx < memberList.length(); ++mIdx )
    {
        var memberElement = memberList.item(mIdx).toElement();
        var memberKind = memberElement.attribute('kind');
        var memberProt = memberElement.attribute('prot');
        var memberName = memberElement.firstChildElement('name').toElement().toString();

        if ( ( memberKind == 'function' ) && // Make sure we're working with a function here
             ( memberProt == 'public' ) &&
             ( memberName.indexOf('operator') == -1 ) && // Make sure this is not an operator.
             ( memberName == name ) )  // matching name
        {
            var methodArgList = memberElement.elementsByTagName('param');
            var args = methodArgList.count();
            if(!overloadList[args]) {
                // println('          Adding overload ' + overloadList.length + ' array for ' + args );
                overloadList[args] = new Array;
            }
            overloadList[args].push( memberElement );

            //            println( '          Found ' + overloadList[args].length + ' override with ' + args + ' arguments' );
        }
    }
     
    return overloadList;
}

// function write_method( compound, memberName, overloadList )
//   compound     - The compound object
//   memberName   - The name of the method/member being processed.
//   overloadList - An array of arrays of method overloads, with outter
//                  array indexed by parameter count, and the inner ones
//                  containg the member elements of the members with that 
//                  parameter count.
// Generates the code to handle all the overloads of an individual method
function write_method( compound, memberName, overloadList )
{
    // Handle arguments
    var method =
        '//  ' + memberName  + '\n' +
        'KJS::JSValue *'+ memberName + '( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) \n' +
        '{ \n';
    
    method += '    Q_UNUSED(args); \n';
            
    method +=
        '    KJS::JSValue *result = KJS::Null(); \n' +
        '    KJSEmbed::' + compound.bindingBase + ' *imp = KJSEmbed::extractBindingImp<KJSEmbed::' + compound.bindingBase + '>(exec, self); \n' +
        '    if( !imp ) \n' +
        '        return KJS::throwError(exec, KJS::GeneralError, "No implementation? Huh?");\n' +
        '\n' +
        '    ' + compound.name + ' value = imp->value<' + compound.name + '>();\n';
        
    for (var idx = 0; idx < overloadList.length; ++idx)
    {
        if (!overloadList[idx])
            continue;


        var memberElement = overloadList[idx][0];
        var methodType = memberElement.firstChildElement('type').toElement().toString();
        var coreMethodType = findCoreParamType(methodType);
        var methodArgList = memberElement.elementsByTagName('param');
        var indent = '';
        if (methodArgList.count() != 0)
            indent = '    ';

//        println( '      writing method with ' + idx + ' args, overrides = ' + overloadList[idx].length + '"' + indent + '"');

        method += '    if (args.size() == ' + methodArgList.count() + ' )\n' +
        '    {\n';

        var funcCallStart = '';
        var funcCallEnd = ');\n';
        if (methodType == "void")
        {
            funcCallStart = 
                'value.' + memberName + '(';
        }
        else if (compound.enums[coreMethodType]) 
        {
            funcCallStart +=
                compound.enums[coreMethodType] + '::' + coreMethodType + ' tmp = value.' + memberName + '(';
            funcCallEnd += 
                '           result = KJS::Number( tmp );\n';
        }
        else
        {
            funcCallStart +=
                methodType + ' tmp = value.' + memberName + '(';
            if ( coreMethodType.indexOf('Qt::') != -1 ) // Enum Value
            {
                funcCallEnd += indent +
                    '        result = KJS::Number( tmp );\n';
            }
            else if (isVariant(coreMethodType))
            {
                funcCallEnd += indent +
                    '        result = KJSEmbed::createVariant( exec, "' + coreMethodType + '", tmp );\n';
            }
            else
            {
                funcCallEnd += indent +
                    '        result = KJSEmbed::createValue( exec, "' + coreMethodType + '", tmp );\n';
            }
        }

        if (compound.isVariant)
            funcCallEnd += indent + '        imp->setValue(qVariantFromValue(value)); \n';
        else
            funcCallEnd += indent + '        imp->setValue(value); \n';

        funcCallEnd +=
            indent + '        return result; \n';

        if (methodArgList.count() != 0)
        {
            var tmpArgs = '';
            for ( paramIdx = 0; paramIdx < methodArgList.count(); ++paramIdx )
            {
                var param = methodArgList.item(paramIdx).toElement();
                var paramVarElement = param.firstChildElement('declname').toElement();
                var paramVar = paramVarElement.toString();
                if (paramVarElement.isNull())
                    paramVar = 'arg' + paramIdx;
                tmpArgs += paramVar + ', ';
            }
        
            if (tmpArgs != '') 
            {
                var tmpIdx = tmpArgs.lastIndexOf(',');
                tmpArgs = tmpArgs.substr(0, tmpIdx);
            }
        
            method += construct_parameters(compound, overloadList[idx], idx, funcCallStart, funcCallEnd);
        }
        else
        {
            method += '        ' + funcCallStart + funcCallEnd;
        }

        method += 
                '    }\n';
    }

    method +=
        '\n' +
        '    return KJS::throwError(exec, KJS::SyntaxError, "Syntax error in parameter list for ' + compound.name + '.' + memberName + '"); \n' +
        '}\n\n';

    return method;
}

// function write_methods( compound )
//   compound   - The compound object
// Iterates over and generates all the method implementations
function write_methods( compound )
{
    var memberList = compound.memberList;
    var methods = 'namespace ' + compound.name + 'NS\n' +
        '{\n';

    var processed = {};

    // Process Methods
    for( var idx = 0; idx < memberList.length(); ++idx )
    {
        var memberElement = memberList.item(idx).toElement();
        var memberKind = memberElement.attribute('kind');
        var memberProt = memberElement.attribute('prot');
        var memberName = memberElement.firstChildElement('name').toElement().toString();

        if ( ( memberKind == 'function' ) && // Make sure we're working with a function here
             ( memberProt == 'public' ) &&
             ( memberName.indexOf('operator') == -1 ) && // Make sure this is not an operator.
             ( memberName.indexOf(compound.name) == -1 ) ) // Not a ctor
        {
            if (processed[memberName])
            {
                println( '      Skipping method overload ' + memberName );
                continue;
            }
            
            println( '      Processing method ' + memberName );
            processed[memberName] = true;
            
            var overloadList = find_method_overloads(memberList, idx, memberName);

            methods += write_method(compound, memberName, overloadList);
        }
    }

    methods +=
        '}\n';

    return methods;
}

// function write_method_lut( compound )
//   compound   - The compound object
// Constructs the method look-up-table (lut) for the compound object
function write_method_lut( compound )
{
    var memberList = compound.memberList;
    var lut_template =
        '\n' +
        'const Method KJSEmbed::' + compound.data + '::p_methods[] = \n' +
        '{\n';

    var processed = {};

    // Generate the binding for each method
    for( var idx = 0; idx < memberList.length(); ++idx )
    {
        var memberElement = memberList.item(idx).toElement();
        var memberKind = memberElement.attribute('kind');
        var memberProt = memberElement.attribute('prot');
        var memberName = memberElement.firstChildElement('name').toElement().toString();

        var numParams = memberElement.elementsByTagName("param").count();
        if ( ( memberKind == 'function' ) &&
             ( memberProt == 'public' ) &&
             ( memberName.indexOf('operator') == -1 ) && // Make sure this is not an operator.
             ( memberName.indexOf(compound.name) == -1 ) ) // Make sure this is not a ctor or dtor
        {
            // make sure only one lut entry per member
            if (processed[memberName])
                continue;
            processed[memberName] = true;

            lut_template += '    { "'+ memberName +'", '+ numParams +', KJS::DontDelete|KJS::ReadOnly, &' + compound.name + 'NS::' + memberName + ' },\n';
        }
    }

    lut_template +=
    '    {0, 0, 0, 0 }\n' +
    '};\n';

    return lut_template;
}

// function write_ctor( compound )
//   compound.def   - The base element of the compound objects definition
// Constructs the constructor for handling the creation of the object
function write_ctor( compound )
{
    var memberList = compound.memberList;
    var ctor =
        'const Constructor KJSEmbed::' + compound.data + '::p_constructor = \n' +
        '{'+
        '"' + compound.name + '", 0, KJS::DontDelete|KJS::ReadOnly, &' + compound.data + '::ctorMethod, p_statics, p_enums, KJSEmbed::' + compound.data + '::p_methods };\n' +
        'KJS::JSObject *KJSEmbed::' + compound.data + '::ctorMethod( KJS::ExecState *exec, const KJS::List &args )\n' +
        '{\n';

    // find the ctors
    var overloadList = new Array;
    for( var idx = 0; idx < memberList.length(); ++idx )
    {
        var memberElement = memberList.item(idx).toElement();
        var memberKind = memberElement.attribute( 'kind' );
        var memberProt = memberElement.attribute('prot');
        var memberName = memberElement.firstChildElement('name').toElement().toString();
        if (( memberKind == 'function' ) && // Constructor is a function
            ( memberProt == 'public' ) && // Make sure it is public
            ( memberName.indexOf('operator') == -1 ) && // Make sure this is not an operator.
            ( memberName.indexOf(compound.name) != -1 ) && // This _is_ a ctor
            ( memberName.indexOf('~') == -1 )) // This is _not_ a dtor
        {
            var memberArgList = memberElement.elementsByTagName('param');
            var args = memberArgList.count();
            if(!overloadList[args]) {
                overloadList[args] = new Array;
            }
            overloadList[args].push( memberElement );
        }
    }

    // Generate the ctor bindings
    for(var idx = 0; idx < overloadList.length; ++idx)
    {
        if(!overloadList[idx]) continue;
        var memberElement = overloadList[idx][0];
        var memberArgList = memberElement.elementsByTagName('param');
        ctor += '    if (args.size() == ' + memberArgList.count() + ' )\n' +
        '    {\n';

        var tmpArgs = '';
        if ( memberArgList.count() == 0 )
        {
            tmpArgs =  + '()';
            ctor += 
            '        return new KJSEmbed::' + compound.binding + '(exec, ' + compound.name + '());\n';
        }
        else
        {
            for ( var argIdx = 0; argIdx < memberArgList.count(); ++argIdx )
            {
                var param = memberArgList.item(argIdx).toElement();
                var paramVarElement = param.firstChildElement('declname').toElement();
                var paramVar = paramVarElement.toString();
                if (paramVarElement.isNull())
                    paramVar = 'arg' + argIdx;

                tmpArgs += paramVar + ', ';
            }

            var tmpIdx = tmpArgs.lastIndexOf(',');
            tmpArgs = tmpArgs.substr(0, tmpIdx);
            var funcCallStart = 'return new KJSEmbed::' + compound.binding + '(exec, ' + compound.name + '('; 
            var funcCallEnd = '));\n';

            ctor += construct_parameters(compound, overloadList[idx], idx, funcCallStart, funcCallEnd);
        }

        ctor += '    }\n';
    }
    ctor += '    return KJS::throwError(exec, KJS::SyntaxError, "Syntax error in parameter list for ' + compound.name + '");\n'
    + '}';
    return ctor;
}

// function write_binding_new( compound )
//   compound  - The compound object provides information about the compound
//               object being processed and a place to save compound specific
//               state information.
// Writes the binding for the class described in class_doc to a file
// with a name composed as '{ClassName}_bind.cpp'.
function write_binding_new( compound )
{
    // This is just looking at brush.cpp and determining the order of the source..

    var includes = '';
    var bindingCtor = '';
    var statics = '';
    var ctor = '';
    var methodLut = '';

    compound.externalBindings = {};
    compound.memberList = compound.def.elementsByTagName( "memberdef" );

    // Eventually all of these should be moved to their own functions, but once again
    // lets get it working first..

    // These are vars we need for all of this to work correctly
    var compoundIncludes = compound.def.firstChildElement('includes').toElement().toString();


//    println ( "compoundIncludes: " + compoundIncludes );
    includes += '#include "' + compound.name + '_bind.h"\n';
    includes += '#include <' + compoundIncludes + '>\n';
    includes += '#include <object_binding.h>\n';
    if (compound.isVariant)
        includes += '#include <value_binding.h>\n';
    else
        includes += '#include <variant_binding.h>\n';

    // Binding Ctor
    bindingCtor +=
        '\n' +
        'using namespace KJSEmbed;\n' +
        '\n' +
        'const KJS::ClassInfo ' + compound.binding + '::info = { "'+ compound.name + '", &' + compound.bindingBase + '::info, 0, 0 };\n' +
        compound.name + 'Binding::' + compound.binding + '( KJS::ExecState *exec, const ' + compound.name +' &value )\n';


    if (compound.bindingBase == 'ValueBinding')
    {
        bindingCtor += '   : ' + compound.bindingBase + '(exec, "' + compound.name + '", value)\n';
    }
    else
    {
        bindingCtor += '   : ' + compound.bindingBase + '(exec, value)\n';
    }

    bindingCtor +=
        '{\n' +
        '    StaticBinding::publish(exec, this, ' + compound.data + '::methods() );\n' +
        '    StaticBinding::publish(exec, this, ' + compound.bindingFactory + '::methods() );\n' +
        '}\n\n';

    var enums = write_enums( compound );

    var methods = write_methods( compound );

    // Statics
    statics += 'NO_STATICS( KJSEmbed::' + compound.data + ' )';

    // Ctor
    ctor = write_ctor( compound );

    // Method LUT
    methodLut += write_method_lut( compound );


    // Write everything
    var fileName = output_dir + compound.name + '_bind.cpp';
    var bindingFile = new File( fileName );
    if( !bindingFile.open( File.WriteOnly ) )
        throw "Unable to open output binding, " + fileName;

    println('Required External Bindings:');
    for (var i in compound.externalBindings)
    {
        println('    ' + i);
        includes += '#include "' + i + '_bind.h"\n';
        includes += '#include <' + i.toLowerCase() + '.h>\n'; 
    }

    bindingFile.writeln( includes );
    bindingFile.writeln( bindingCtor );
    bindingFile.writeln( methods );
    bindingFile.writeln( enums );
    bindingFile.writeln( statics );
    bindingFile.writeln( ctor );
    bindingFile.writeln( methodLut );
    bindingFile.close();
}
