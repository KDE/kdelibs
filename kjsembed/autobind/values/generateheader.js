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

// function write_header( compound )
//   compound  - The compound object provides information about the compound
//               object being processed and a place to save compound specific
//               state information.
// Generate the header file for binding the class specified in the passed DOM element.
function write_header( compound )
{
    var template =
        '#ifndef BIND_' + compound.name + '_H\n' +
        '#define BIND_' + compound.name + '_H\n' +
        '\n' +
        '#include <variant_binding.h>\n' +
        '#include <static_binding.h>\n' +
        '\n' +
        'class ' + compound.name + ';\n' +
        '\n' +
        'namespace KJSEmbed\n' +
        '{\n' +
        '   class ' + compound.binding + ' : public ' + compound.bindingBase + '\n' +
        '   {\n' +
        '       public:\n' +
        '           ' + compound.binding + '( KJS::ExecState *exec, const ' + compound.name + ' &value );\n' +
        '           static const KJS::ClassInfo info;\n' +
        '           virtual const KJS::ClassInfo* classInfo() const { return &info; }\n' +
        '   };\n\n' +
        '   class ' + compound.data + ' \n' +
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
        '#endif // BIND_' + compound.name + '_H\n\n';

    var fileName = output_dir + compound.name + '_bind.h';
    header = new File( fileName );

    if( !header.open( File.WriteOnly ) )
        throw "Unable to open output header, " + fileName;

    header.writeln( template );
    header.close();
}
