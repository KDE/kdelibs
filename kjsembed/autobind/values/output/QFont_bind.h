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
       private:
           static const KJS::ClassInfo info;
           virtual const KJS::ClassInfo* classInfo() const { return &info; }
   };

   KJS_BINDING( QFont )

}
#endif // BIND_QFont_H


