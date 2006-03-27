#ifndef BIND_QRadialGradient_H
#define BIND_QRadialGradient_H

#include <value_binding.h>
#include <static_binding.h>

class QRadialGradient;

namespace KJSEmbed
{
   class QRadialGradientBinding : public ValueBinding
   {
       public:
           QRadialGradientBinding( KJS::ExecState *exec, const QRadialGradient &value );
   };

   KJS_BINDING( QRadialGradient )

}
#endif // BIND_QRadialGradient_H


