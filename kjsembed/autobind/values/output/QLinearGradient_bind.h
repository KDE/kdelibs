#ifndef BIND_QLinearGradient_H
#define BIND_QLinearGradient_H

#include <value_binding.h>
#include <static_binding.h>

class QLinearGradient;

namespace KJSEmbed
{
   class QLinearGradientBinding : public ValueBinding
   {
       public:
           QLinearGradientBinding( KJS::ExecState *exec, const QLinearGradient &value );
   };

   KJS_BINDING( QLinearGradient )

}
#endif // BIND_QLinearGradient_H


