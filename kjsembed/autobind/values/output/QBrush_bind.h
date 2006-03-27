#ifndef BIND_QBrush_H
#define BIND_QBrush_H

#include <value_binding.h>
#include <static_binding.h>

class QBrush;

namespace KJSEmbed
{
   class QBrushBinding : public ValueBinding
   {
       public:
           QBrushBinding( KJS::ExecState *exec, const QBrush &value );
   };

   KJS_BINDING( QBrush )

}
#endif // BIND_QBrush_H


