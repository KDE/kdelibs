#ifndef BIND_QColor_H
#define BIND_QColor_H

#include <value_binding.h>
#include <static_binding.h>

class QColor;

namespace KJSEmbed
{
   class QColorBinding : public ValueBinding
   {
       public:
           QColorBinding( KJS::ExecState *exec, const QColor &value );
       private:
           static const KJS::ClassInfo info;
           virtual const KJS::ClassInfo* classInfo() const { return &info; }
   };

   KJS_BINDING( QColor )

}
#endif // BIND_QColor_H


