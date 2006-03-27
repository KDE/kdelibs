#ifndef BIND_QFont_H
#define BIND_QFont_H

#include <value_binding.h>
#include <static_binding.h>

class QFont;

namespace KJSEmbed
{
   class QFontBinding : public ValueBinding
   {
       public:
           QFontBinding( KJS::ExecState *exec, const QFont &value );
   };

   KJS_BINDING( QFont )

}
#endif // BIND_QFont_H


