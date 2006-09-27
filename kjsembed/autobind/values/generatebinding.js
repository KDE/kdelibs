function write_ctor( compoundDef, compoundEnums )
{
    var compoundName = compoundDef.firstChildElement('compoundname').toElement().toString();
    var ctor =
        'const Constructor ' + compoundData + '::p_constructor = \n' +
        '{'+
        '"' + compoundName + '", 0, KJS::DontDelete|KJS::ReadOnly, &' + compoundData + '::ctorMethod, p_statics, p_enums, p_methods };\n' +
        'KJS::JSObject *' + compoundData + '::ctorMethod( KJS::ExecState *exec, const KJS::List &args )\n' +
        '{\n';

    // Generate the ctor bindings
    var methodList = compoundDef.elementsByTagName( "memberdef" );
    var methodListList = new Array;
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
                if ( memberName.indexOf(compoundName) != -1 ) // This _is_ a ctor
                {
                    if ( memberName.indexOf('~') == -1 ) // This is _not_ a dtor
                    {
                        var args = memberArgList.count();
                        if(!methodListList[args]) {
                            methodListList[args] = new Array;
                        }
                        methodListList[args].push( memberElement );
                    }
                }
            }
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
            tmpArgs = compoundName + '()';
        }

        for ( argIdx = 0; argIdx < memberArgList.count(); ++argIdx )
        {
            var param = memberArgList.item(argIdx).toElement();
            var paramVar = param.firstChildElement('declname').toElement().toString();
//             ctor += extract_parameter(param, argIdx, compoundEnums);
            tmpArgs += paramVar + ', ';
        }
        ctor += extract_parameter_const(methodListList[idx], idx, compoundEnums);

        var tmpIdx = tmpArgs.lastIndexOf(',');
        tmpArgs = tmpArgs.substr(0, tmpIdx);
        ctor +=
        '        return new KJSEmbed::' + compoundName + 'Binding(exec, ' + compoundName + '(' + tmpArgs + '))\n' +
        '    }\n';
    }
    ctor += '}';
    return ctor;
}

function extract_parameter_const(methodList, numArgs, compoundEnums)
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
        for(var argIdx = 0; argIdx < numArgs; ++argIdx)
        {
            var parameter = memberArgList.item(argIdx).toElement();
            var paramType = parameter.firstChildElement('type').toElement().toString();
debug(paramType);
            if(isVariant(paramType))
                params += 'object'+argIdx+' && object'+argIdx+'->inherits(&ColorBinding::info)';
            else
                params += 'isBasic(value'+argIdx+')';
            if(argIdx < numArgs-1)
                params += '&& ';
            variables += '    ' + extract_parameter(parameter, argIdx, compoundEnums);
        }
        params += ')\n';
        params += '        {\n';
        params += variables;
        params += '        }\n';
    }
    return params;
}

function write_method_lut( compoundDef )
{
    var compoundName = compoundDef.firstChildElement('compoundname').toElement().toString();
    var lut_template =
        '\n' +
        'const Method ' + compoundData + '::p_methods[] = \n' +
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
                    lut_template += '    { '+ memberName +', '+ numParams +', KJS::DontDelete|KJS::ReadOnly, &' + compoundName + 'NS::' + memberName + ' },\n';
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

function write_binding_new( class_doc )
{
    // This is just looking at brush.cpp and determining the order of the source..
    var includes = '';
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
        "const KJS::ClassInfo " + compoundName + "Binding::info = { \""+ compoundName + "\", &ValueBinding::info, 0, 0 };\n" +
        compoundName + 'Binding::' + compoundName + 'Binding( KJS::ExecState *exec, const ' + compoundName +' &value )\n' +
        '   : ValueBinding(exec, value)\n' +
        '{\n' +
        '    StaticBinding::publish(exec, this, ' + compoundData + '::methods() );\n' +
        '    StaticBinding::publish(exec, this, ValueFactory::methods() );\n' +
        '}\n\n';

    // Methods
    methods +=
        'namespace ' + compoundName + 'NS\n' +
        '{\n';

    enums +=
        '\n' +
        'const Enumerator ' + compoundData + '::p_enums[] = {\n';

    var hasEnums = false;
    var memberList = class_doc.elementsByTagName( "memberdef" );
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
                        var methodArgs = memberElement.firstChildElement('argsstring').toElement().toString();
                        methods +=
                            '// ' + methodType + ' ' + memberName + methodArgs + '\n' +
                            'KJS::JSValue *'+ memberName + '( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args ) \n' +
                            '{ \n' +
                            '    KJS::JSValue *result = KJS::Null(); \n' +
                            '    KJSEmbed::ValueBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::ValueBinding>(exec, self); \n' +
                            '    if( imp ) \n' +
                            '    { \n' +
                            '        ' + compoundName + ' value = imp->value<' + compoundName + '>();\n';

                        // Handle arguments
                        var methodArgList = memberElement.elementsByTagName('param');
                        if ( methodArgList.count() == 0 )
                        {
                            if (methodType == "void")
                            {
                            }
                            else if (compoundEnums[methodType]) 
                            {
                                methods +=
                                '       ' + compoundEnums[methodType] + '::' + methodType + ' tmp = value.' + memberName + '();\n';
                                methods += 
                                '        result = KJS::Number( tmp );\n';
                            }
                            else
                            {
                                methods +=
                                '        ' + methodType + ' tmp = value.' + memberName + '();\n';
                                if ( methodType.indexOf('Qt::') != -1 ) // Enum Value
                                {
                                    methods += 
                                    '       result = KJS::Number( tmp );\n';
                                }
                                else
                                {
                                    methods +=
                                    "       result = KJSEmbed::createValue( exec, \"" + methodType + "\", tmp );\n";
                                }
                            }
                        }

                        for ( paramIdx = 0; paramIdx < methodArgList.count(); ++paramIdx )
                        {
                            var param = methodArgList.item(paramIdx).toElement();
                            var paramVar = param.firstChildElement('declname').toElement().toString();
                            var paramVarElement = param.firstChildElement('declname').toElement();
                            var paramDefault = param.firstChildElement('defval').toElement();
                            methods += extract_parameter(param, paramIdx, compoundEnums);
                        }
                        if ( memberName.indexOf('set') != -1 )
                        {   // setter, we can handle this for now
                            if ( paramVarElement.isNull() )
                                methods += '        value.' + memberName + '(arg0);\n';
                            else
                                methods += '        value.' + memberName + '(' + paramVar + ');\n';
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
        else if ( memberKind == 'enum' )
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

    methods +=
        '}\n';

    if ( hasEnums )
    {
        enums +=
            '    {0, 0}\n' +
            '};\n';
    }
    else {
        enums = 'const Enumerator ' + compoundData + '::p_enums[] = {{0, 0 }};\n';
    }


    // Statics
    statics += 'NO_STATICS( ' + compoundData + ' )';

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

// An array of primitive Qt types, this is annoying but seems to be necessary
var variant_types = {
    // Actual variant types
    "QBitArray" : 1, "QBitmap" : 1, "bool" : 1, "QBrush" : 1,
    "QByteArray" : 1, "QChar" : 1, "QColor" : 1, "QCursor" : 1,
    "QDate" : 1, "QDateTime" : 1, "double" : 1, "QFont" : 1,
    "QIcon" : 1, "QImage" : 1, "int" : 1, "QKeySequence" : 1,
    "QLine" : 1, "QLineF" : 1, "QVariantList" : 1, "QLocale" : 1,
    "qlonglong" : 1, "QVariantMap" : 1, "QPalette" : 1, "QPen" : 1,
    "QPixmap" : 1, "QPoint" : 1, "QPointArray" : 1, "QPointF" : 1,
    "QPolygon" : 1, "QRect" : 1, "QRectF" : 1, "QRegExp" : 1,
    "QRegion" : 1, "QSize" : 1, "QSizeF" : 1, "QSizePolicy" : 1,
    "QString" : 1, "QStringList" : 1, "QTextFormat" : 1,
    "QTextLength" : 1, "QTime" : 1, "uint" : 1, "qulonglong" : 1,
    "QUrl" : 1, 

     // Other necessary qglobal.h types.
     "qreal" : 1, "qint8" : 1, "quint8" : 1, "qint16" : 1, "quint16" : 1, 
     "qint32" : 1, "quint32" : 1, "qint64" : 1, "quint64" : 1, 
     "qlonglong" : 1, "qulonglong" : 1,
     "uchar" : 1, "ushort" : 1, "uint" : 1, "ulong" : 1
    
};

function isVariant( variable )
{
//    debug(variable + " isVariant " + variant_types[variable]);
    if (variant_types[variable] == 1)
      return true;
    else
      return false;
}

// Regular expression used to spot const & type args (i.e. 'const QString &').
const_ref_rx = /const\s+(\w+)\s*&/;

function extract_parameter( parameter, paramIdx, compoundEnums )
{
    var extracted = '';
    var paramType = parameter.firstChildElement('type').toElement().toString();
    var paramVar = parameter.firstChildElement('declname').toElement().toString();
    var paramVarElement = parameter.firstChildElement('declname').toElement();
    var paramDefault = parameter.firstChildElement('defval').toElement();

    if (paramVarElement.isNull())
        paramVar = 'arg' + paramIdx;

    var coreParamTypeMatch = const_ref_rx.exec(paramType);

    // We want the core parameter type
    var coreParamType;
    if (coreParamTypeMatch == null)
        coreParamType = paramType;
    else
        coreParamType = coreParamTypeMatch[1];

    if ( paramType.indexOf('Qt::') != -1 )  // Enum Value
    {
        extracted +=
            '        ' + paramType + ' ' + paramVar + ' = static_cast<' + paramType + '>(KJSEmbed::extractInt(exec, args, ' + paramIdx + ', ';

        if (!paramDefault.isNull())
            extracted += paramDefault.toString() + '));\n';
        else
            extracted += '0));\n';

        return extracted;
    }
    else if ( compoundEnums[paramType] )  // Enum Value
    {
        extracted +=
            '       ' + compoundEnums[paramType] + '::' + paramType + ' ' + paramVar + ' = static_cast<' + compoundEnums[paramType] + '::' + paramType + '>(KJSEmbed::extractInt(exec, args, ' + paramIdx + ', ';

        if (!paramDefault.isNull())
            extracted += compoundEnums[paramType] + '::' + paramDefault.toString() + '));\n';
        else
            extracted += '0));\n';

        return extracted;
    }
    else if ( isVariant(coreParamType) )
    {
        //extracted += 'if(args['+paramIdx+'].getObject() != 0 && QByteArray(args['+paramIdx+'].getObject()->classInfo()->className) == "'+paramType+'");\n';
        extracted +=
            '        ' + coreParamType + ' ' + paramVar + ' = KJSEmbed::extractValue<' + coreParamType + '>(exec, args, ' + paramIdx + ');\n';
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
