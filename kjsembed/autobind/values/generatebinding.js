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
    var cppParamType = coreParamType;

    if (isTypedef(coreParamType, compound.typedefs))
        coreParamType = compound.typedefs[coreParamType];

    if ( isBool(coreParamType) )
    {
        extracted +=
            cppParamType + ' ' + paramVar + ' = KJSEmbed::extractBool(exec, args, ' + paramIdx + ');\n';
        return extracted;
    }
    else if ( isPointer(coreParamType) )
    {
        extracted +=
            cppParamType + ' ' + paramVar + ' =';

        baseParamType = strip(cppParamType);
        if ( isObject(baseParamType, compound.objectTypes) )
            extracted += ' KJSEmbed::extractObject<' + coreParamType + '>(exec, args, ' + paramIdx + ');\n';
        else
            extracted += ' KJSEmbed::extractValue<' + coreParamType + '>(exec, args, ' + paramIdx + ');\n';

        return extracted;
    }
    else if ( isInteger(coreParamType) )  // integer value
    {
        extracted +=
            cppParamType + ' ' + paramVar + ' = KJSEmbed::extractInteger<' + coreParamType + '>(exec, args, ' + paramIdx;

        if (!paramDefault.isNull())
            extracted += ', ' + paramDefault.toString() + ');\n';
        else
            extracted += ');\n';

        return extracted;
    }
    else if ( isNumber(coreParamType) )  // integral value
    {
        extracted +=
            cppParamType + ' ' + paramVar + ' = KJSEmbed::extractNumber<' + coreParamType + '>(exec, args, ' + paramIdx;

        if (!paramDefault.isNull())
            extracted += ', ' + paramDefault.toString() + ');\n';
        else
            extracted += ');\n';

        return extracted;
    }
    else if ( compound.enums[paramType] )  // Enum Value
    {
        var enumHeading = compound.enums[paramType] + '::';

        extracted +=
            enumHeading + paramType + ' ' + paramVar + ' = KJSEmbed::extractInteger<' + enumHeading + paramType + '>(exec, args, ' + paramIdx;

        if (!paramDefault.isNull())
            extracted += ', ' + enumHeading + paramDefault.toString() + ');\n';
        else
            extracted += ');\n';

        return extracted;
    }
    else if ( compound.globalEnums[paramType] )  // Enum Value
    {
        extracted +=
            paramType + ' ' + paramVar + ' = KJSEmbed::extractInteger<' + paramType + '>(exec, args, ' + paramIdx;

        if (!paramDefault.isNull())
            extracted += ', ' + paramDefault.toString() + ');\n';
        else
            extracted += ');\n';

        return extracted;
    }
    else if (coreParamType == 'QString')
    {
        extracted +=
            cppParamType + ' ' + paramVar + ' = KJSEmbed::extractQString(exec, args, ' + paramIdx;

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
            cppParamType + ' ' + paramVar + ' = KJSEmbed::extractString<' + coreParamType + '>(exec, args, ' + paramIdx;

        if (!paramDefault.isNull())
            extracted += ', ' + paramDefault.toString() + ');\n';
        else
            extracted += ');\n';

        return extracted;
    }
    else if ( isVariant(coreParamType) )
    {
        extracted +=
            cppParamType + ' ' + paramVar + ' = KJSEmbed::extractVariant<' + coreParamType + '>(exec, args, ' + paramIdx + ');\n';
        return extracted;
    }
    else if ( isNotImplemented(coreParamType) )
    {
        extracted += 
            '// Not currently implemented\n' +
            '// ' + cppParamType + ' ' + paramVar + ' =\n';
        return extracted; // Shortcut adding the ) here because it's not implemented.
    }
    else    // It's an object, or something else?
    {
        extracted +=
            cppParamType + ' ' + paramVar + ' = KJSEmbed::extractValue<' + coreParamType + '>(exec, args, ' + paramIdx;
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


            if (isTypedef(coreParamType, compound.typedefs))        // I _think_ this works for all cases
                coreParamType = compound.typedefs[coreParamType];



            if (isBool(coreParamType))
                params += 'object'+argIdx+' && object'+argIdx+'->isBoolean()';
            else if ( isNumber(coreParamType) || 
                      isEnum(coreParamType, compound.globalEnums, compound.enums ) )
                params += 'object'+argIdx+' && object'+argIdx+'->isNumber()';
            else if (coreParamType == 'QString' ||
                    contains(coreParamType, 'uchar') ||
                    contains(coreParamType, 'char'))
                params += 'object'+argIdx+' && object'+argIdx+'->isString()';
            else if (isNotImplemented(coreParamType))
                continue;
            else
            {
                if ( isPointer(coreParamType) )
                    coreParamType = strip(coreParamType);

                if (coreParamType != compound.name)
                    compound.externalBindings[coreParamType] = true;
                params += 'object'+argIdx+' && object'+argIdx+'->inherits(&' + coreParamType + 'Binding::info)';
            }

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



function write_method_new(compound, memberName, overloadList)
{
    // Handle arguments
    var header =
        '//  ' + memberName  + '\n' +
        'KJS::JSValue *'+ memberName + '( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) \n' +
        '{ \n';

    header += indent + 'Q_UNUSED(args); \n';

    header +=
        indent + 'KJS::JSValue *result = KJS::jsNull(); \n' +
        indent + 'KJSEmbed::' + compound.bindingBase + ' *imp = KJSEmbed::extractBindingImp<KJSEmbed::' + compound.bindingBase + '>(exec, self); \n' +
        indent + 'if( !imp ) \n' +
        indent + indent + 'return KJS::throwError(exec, KJS::GeneralError, "No implementation? Huh?");\n' +
        '\n' +
        indent + compound.name + ' value = imp->value<' + compound.name + '>();\n';

    var footer =
        indent + 'return KJS::throwError(exec, KJS::SyntaxError, "Syntax error for ' + compound.name + '.' + memberName + '"); \n' +
        '}\n\n';


    var segments = extract_method_segments(compound, memberName, overloadList);




    // This is all we need to do to write the method, make sure everything is good before this point.
    var method = '';
    method += header;

    for (var idx = 0; idx < segments.length; ++idx)
    {
        var segment = segments[idx];
        method += segment;
    }

    method += footer;
    return header;
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
    var tab = '    ';
    var indent = tab;

    var method = '';

    // Handle arguments
    var methodHeader =
        '//  ' + memberName  + '\n' +
        'KJS::JSValue *'+ memberName + '( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) \n' +
        '{ \n';

    methodHeader += indent + 'Q_UNUSED(args); \n';

    methodHeader +=
        indent + 'KJS::JSValue *result = KJS::jsNull(); \n' +
        indent + 'KJSEmbed::' + compound.bindingBase + ' *imp = KJSEmbed::extractBindingImp<KJSEmbed::' + compound.bindingBase + '>(exec, self); \n' +
        indent + 'if( !imp ) \n' +
        indent + indent + 'return KJS::throwError(exec, KJS::GeneralError, "No implementation? Huh?");\n' +
        '\n' +
        indent + compound.name + ' value = imp->value<' + compound.name + '>();\n';

    method += methodHeader;


    for (var idx = 0; idx < overloadList.length; ++idx)
    {
        if (!overloadList[idx])
            continue;

        var methodSegment = '';
        methodSegment += indent + 'if (args.size() == ' + idx + ' )\n' +
                  indent + '{\n';

        indent = tab + tab;

        // First we make all the value/object pairs
        var tempParams = '';
        var variables = '';
        for (var tempArgIdx = 0; tempArgIdx < idx; ++tempArgIdx)
        {
            variables += indent + 'KJS::JSValue* value'+tempArgIdx+'=args['+tempArgIdx+'];\n';
            variables += indent + 'KJS::JSObject* object'+tempArgIdx+'=value'+tempArgIdx+'->toObject(exec);\n';
        }

        methodSegment += variables; // soon move this down

        var overloadArray = overloadList[idx];
        for (var idx2 = 0; idx2 < overloadArray.length; ++idx2)
        {
            var tempMethodElement = overloadArray[idx2].toElement();
            var tempMethodName = tempMethodElement.firstChildElement('name').toElement().toString();
            var tempMethodType = tempMethodElement.firstChildElement('type').toElement().toString();
            var tempCoreMethodType = findCoreParamType(tempMethodType);
            var tempMethodArgList = tempMethodElement.elementsByTagName('param');

            var checkParams = '';
            if (tempMethodArgList.count() != 0)
                checkParams += indent + 'if(';

            if (isTypedef(tempCoreMethodType, compound.typedefs))
                tempCoreMethodType = compound.typedefs[tempCoreMethodType];

            for(var tempArgIdx = 0; tempArgIdx < idx; ++tempArgIdx)
            {
                var tempParam = tempMethodArgList.item(tempArgIdx).toElement();
                var tempParamVarElement = tempParam.firstChildElement('declname').toElement();
                var tempParamVar = tempParamVarElement.toString();
                if (tempParamVarElement.isNull())
                    tempParamVar = 'arg' + tempArgIdx;

                var tempParamType = tempParam.firstChildElement('type').toElement().toString();
                var tempCoreParamType = findCoreParamType(tempParamType);
                var tempParamDefault = tempParam.firstChildElement('defval').toElement();

                if (!tempParamDefault.isNull())
                    checkParams += '( ( ';

//                print(tempCoreParamType + ' ' + tempParamVar + '...');
                if (isTypedef(tempCoreParamType, compound.typedefs))        // I _think_ this works for all cases
                {
//                     print('isTypdef\n');
                    tempCoreParamType = compound.typedefs[tempCoreParamType];
                }

                if (isBool(tempCoreParamType))
                {
//                     print('isBool\n');
                    checkParams += 'object'+tempArgIdx+' && object'+tempArgIdx+'->isBoolean()';
                }
                else if ( isNumber(tempCoreParamType) || 
                        isEnum(tempCoreParamType, compound.globalEnums, compound.enums ) )
                {
//                     print('isNumber || isEnum\n');
                    checkParams += 'object'+tempArgIdx+' && object'+tempArgIdx+'->isNumber()';
                }
                else if (tempCoreParamType == 'QString' ||
                        contains(tempCoreParamType, 'uchar') ||
                        contains(tempCoreParamType, 'char'))
                {
//                     print('isString\n');
                    checkParams += 'object'+tempArgIdx+' && object'+tempArgIdx+'->isString()';
                }
                else if (isNotImplemented(tempCoreParamType))
                {
//                     print('Not Implemented\n');
                    notImplementedHit = 1;
                    checkParams += '0';  // unf
                    continue;
                }
                else
                {
//                    print('is[Object,Value,Variant]\n');
                    if ( isPointer(tempCoreParamType) )
                        tempCoreParamType = strip(tempCoreParamType);

                    if (tempCoreParamType != compound.name)
                        compound.externalBindings[tempCoreParamType] = true;
                    checkParams += 'object'+tempArgIdx+' && object'+tempArgIdx+'->inherits(&' + tempCoreParamType + 'Binding::info)';
                }

                if (!tempParamDefault.isNull())
                    checkParams += ' ) || !object'+tempArgIdx+' )';

                if(tempArgIdx < idx-1)
                    checkParams += ' && ';
            }

            if (tempMethodArgList.count() != 0)
            {
                checkParams += ')\n' +
                          indent + '{\n';
                indent = tab + tab + tab;
            }


            methodSegment += checkParams;


            // Get all the args into C++
            var parameters = '';
            for(var tempArgIdx = 0; tempArgIdx < idx; ++tempArgIdx)
            {
                var tempParam = tempMethodArgList.item(tempArgIdx).toElement();
                parameters += indent + extract_parameter(compound, tempParam, tempArgIdx);
            }

            // Manipulate the internal value
            var tempMethodCallStart = '';
            var tempMethodCallEnd = ');\n' + indent;
            if (isNotImplemented(tempCoreParamType))    // NOTE: it is tempCoreParam type not tempCoreMethodType
            {
                tempMethodCallStart += '// value\n';
                tempMethodCallEnd += 'result = KJS::jsNull();\n';
            }
            else if (contains(tempCoreMethodType, 'void')) // since method type is void, we don't need to assign anything
            {
                tempMethodCallStart += 
                    'value.' + memberName + '(';
            }
            else if (compound.enums[tempCoreMethodType]) // local scope enum value
            {
                tempMethodCallStart +=
                    compound.enums[tempCoreMethodType] + '::' + tempCoreMethodType + ' tmp = value.' + memberName + '(';
                tempMethodCallEnd += 
                    'result = KJS::jsNumber( tmp );\n';
            }
            else
            {
                tempMethodCallStart +=
                    tempCoreMethodType + ' tmp = value.' + memberName + '(';
                if ( compound.globalEnums[tempCoreMethodType] ||  // Enum Value
                    isInteger(tempCoreMethodType) )
                {
                    tempMethodCallEnd +=
                        'result = KJS::jsNumber( tmp );\n';
                }
                else if (isBool(tempCoreMethodType))
                {
                    tempMethodCallEnd +=
                        'result = KJS::Boolean( tmp );\n';
                }
                else if (isVariant(tempCoreMethodType))
                {
                    tempMethodCallEnd +=
                        'result = KJSEmbed::createVariant( exec, "' + tempCoreMethodType + '", tmp );\n';
                }
                else
                {
                    tempMethodCallEnd +=
                                indent + '// Not currently implemented\n' +
                                indent + '// ' + tempCoreMethodType + '\n' +
                                indent + 'result = KJS::jsNull();\n';
                }
            }

            if (!isNotImplemented(tempCoreParamType)) // Sweet little double negative here ;)
            {
                if (compound.isVariant)
                    tempMethodCallEnd += indent + 'imp->setValue(qVariantFromValue(value)); \n';
                else
                    tempMethodCallEnd += indent + 'imp->setValue(value); \n';
            }

            tempMethodCallEnd +=
                indent + 'return result; \n';

            if (!isNotImplemented(tempCoreParamType))     // NOTE: so must be a better way to do all this
            {                                           // set a flag somewhere that we're working with
                if (tempMethodArgList.count() != 0)     // an unspported method...
                {
                    var tempTmpArgs = '';
                    for ( tempParamIdx = 0; tempParamIdx < tempMethodArgList.count(); ++tempParamIdx )
                    {
                        var tempParam = tempMethodArgList.item(tempParamIdx).toElement();
                        var tempParamVarElement = tempParam.firstChildElement('declname').toElement();
                        var tempParamVar = tempParamVarElement.toString();
                        if (tempParamVarElement.isNull())
                            tempParamVar = 'arg' + tempParamIdx;
                        tempTmpArgs += tempParamVar + ', ';
                    }

                    if (tempTmpArgs != '') 
                    {
                        var tempTmpIdx = tempTmpArgs.lastIndexOf(',');
                        tempTmpArgs = tempTmpArgs.substr(0, tempTmpIdx);
                    }

                    parameters += indent + tempMethodCallStart + tempTmpArgs + tempMethodCallEnd;
                }
                else
                    parameters += indent + tempMethodCallStart + tempMethodCallEnd;
            }

            methodSegment += parameters

            if (tempMethodArgList.count() != 0)
            {
                indent = tab + tab;
                methodSegment += 
                        indent + '}\n';
            }
//            println('name: ' + tempMethodName);
//            println('type: ' + tempMethodType);
//            println('coreType: ' + tempCoreMethodType);
//            println('method: ' + tempMethod);
            method += methodSegment;
        }

        indent = tab;
        method +=
                indent + '}\n';
    }


    indent = tab;
    method +=
        '\n' +
        indent + 'return KJS::throwError(exec, KJS::SyntaxError, "Syntax error in parameter list for ' + compound.name + '.' + memberName + '"); \n' +
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
        var memberStatic = memberElement.attribute('static');
        var memberName = memberElement.firstChildElement('name').toElement().toString();

        if ( ( memberKind == 'function' ) && // Make sure we're working with a function here
             ( memberProt == 'public' ) &&
             ( memberName.indexOf('operator') == -1 ) &&     // Not an operator
             ( memberName.indexOf(compound.name) == -1 ) &&  // Not a ctor
             ( contains(memberStatic, 'no') ) &&             // Not a static method
             ( hasNoProblematicTypes(memberElement) ) )
        {
            var memberCacheString = memberName + memberKind;    // Need to get around some js keywords with this
            if (processed[memberCacheString])
            {
                println( '      Skipping method overload ' + memberName );
                continue;
            }

            println( '      Processing method ' + memberName );
            processed[memberCacheString] = true;

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
        var memberStatic = memberElement.attribute('static');
        var memberName = memberElement.firstChildElement('name').toElement().toString();

        var numParams = memberElement.elementsByTagName("param").count();
        if ( ( memberKind == 'function' ) &&
             ( memberProt == 'public' ) &&
             ( memberName.indexOf('operator') == -1 ) &&     // Not an operator.
             ( memberName.indexOf(compound.name) == -1 ) &&  // Not a ctor or dtor
             ( contains(memberStatic, "no") ) &&             // Not a static method
             ( hasNoProblematicTypes(memberElement) ) )
        {
            var memberCacheId = memberName + numParams;      // Only one entry per member
            if (processed[memberCacheId])
                continue;
            processed[memberCacheId] = true;

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
        if (( memberKind == 'function' ) &&                 // Is a function
            ( memberProt == 'public' ) &&                   // Is public
            ( memberName.indexOf('operator') == -1 ) &&     // Not an operator.
            ( memberName.indexOf(compound.name) != -1 ) &&  // Is a ctor
            ( memberName.indexOf('~') == -1 ) &&            // Not a dtor
            ( hasNoProblematicTypes(memberElement) ))
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

    compound.enums = {};
    compound.externalBindings = {};
    compound.memberList = compound.def.elementsByTagName( "memberdef" );

    // Eventually all of these should be moved to their own functions, but once again
    // lets get it working first..

    // These are vars we need for all of this to work correctly
    var compoundIncludes = compound.def.firstChildElement('includes').toElement().toString();


//    println ( "compoundIncludes: " + compoundIncludes );
    includes += '#include "' + compound.filebase + '_bind.h"\n';
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
        bindingCtor += '   : ' + compound.bindingBase + '(exec, "' + compound.name + '", value)\n';
    else
        bindingCtor += '   : ' + compound.bindingBase + '(exec, value)\n';

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
    var fileName = output_dir + compound.filebase + '_bind.cpp';
    var bindingFile = new File( fileName );
    if( !bindingFile.open( File.WriteOnly ) )
        throw "Unable to open output binding, " + fileName;

    println('Required External Bindings:');
    for (var i in compound.externalBindings)
    {
        println('    ' + i);
        includes += '#include "' + i.replace(/::/g, '_') + '_bind.h"\n';
        includes += '#include <' + i + '>\n'; 
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
