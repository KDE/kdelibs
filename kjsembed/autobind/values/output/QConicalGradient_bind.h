#ifndef BIND_QConicalGradient_H
#define BIND_QConicalGradient_H

#include <value_binding.h>
#include <static_binding.h>

class QConicalGradient;

namespace KJSEmbed
{
   class QConicalGradientBinding : public ValueBinding
   {
       public:
           QConicalGradientBinding( KJS::ExecState *exec, const QConicalGradient &value );
   };

   KJS_BINDING( QConicalGradient )

}
#endif // BIND_QConicalGradient_H


