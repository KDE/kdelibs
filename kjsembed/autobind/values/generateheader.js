/*
 * Generate the header file for binding the class specified in the passed DOM element.
 */
function write_header( classDoc )
{
    var compounddef = classDoc.firstChild().toElement();
    var includes = compounddef.firstChildElement('includes').toElement().toString();
    var compoundName = compounddef.firstChildElement('compoundname').toElement().toString();

    
    var template =
        '#ifndef BIND_' + compoundName + '_H\n' +
        '#define BIND_' + compoundName + '_H\n' +
        '\n' +
        '#include <value_binding.h>\n' +
        '#include <static_binding.h>\n' +
        '\n' +
        'class ' + compoundName + ';\n' +
        '\n' +
        'namespace KJSEmbed\n' +
        '{\n' +
        '   class ' + compoundName + 'Binding : public ValueBinding\n' +
        '   {\n' +
        '       public:\n' +
        '           ' + compoundName + 'Binding( KJS::ExecState *exec, const ' + compoundName + ' &value );\n' +
        '       private:\n' +
        '           static const KJS::ClassInfo info;\n' +
        '           virtual const KJS::ClassInfo* classInfo() const { return &info; }\n' +
        '   };\n\n' +
        '   class ' + compoundName + 'Data \n' +
        '   { \n' +
        '       public: \n' + 
        '           static const KJSEmbed::Method p_methods[]; \n' +
        '           static const KJSEmbed::Method p_statics[]; \n' +
        '           static const KJSEmbed::Enumerator p_enums[]; \n' +
        '           static const KJSEmbed::Constructor p_constructor; \n' +
        '           static KJS::JSObject *ctorMethod( KJS::ExecState *exec, const KJS::List &args );\n' +
        '           static const KJSEmbed::Enumerator *enums() { return p_enums;} \n' +
        '           static const KJSEmbed::Method *methods() { return p_methods;} \n' +
        '           static const KJSEmbed::Method *statics() { return p_statics;} \n' +
        '           static const KJSEmbed::Constructor *constructor() { return &p_constructor;} \n' +
        '           static const KJS::JSObject *construct(KJS::ExecState *exec, const KJS::List &args)\n' +
        '                                           { return (*p_constructor.construct)(exec,args); } \n' +
        '   };\n\n' +
        '}\n' +
        '#endif // BIND_' + compoundName + '_H\n\n';

    var fileName = output_dir + compoundName + '_bind.h';
    header = new File( fileName );

    if( !header.open( File.WriteOnly ) )
        throw "Unable to open output header, " + fileName;

    header.writeln( template );
    header.close();
}
