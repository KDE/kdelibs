#ifndef BIND_QRect_H
#define BIND_QRect_H

#include <value_binding.h>
#include <static_binding.h>

class QRect;

namespace KJSEmbed
{
   class QRectBinding : public ValueBinding
   {
       public:
           QRectBinding( KJS::ExecState *exec, const QRect &value );
   };

   KJS_BINDING( QRect )

}
#endif // BIND_QRect_H


