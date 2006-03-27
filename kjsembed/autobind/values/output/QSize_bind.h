#ifndef BIND_QSize_H
#define BIND_QSize_H

#include <value_binding.h>
#include <static_binding.h>

class QSize;

namespace KJSEmbed
{
   class QSizeBinding : public ValueBinding
   {
       public:
           QSizeBinding( KJS::ExecState *exec, const QSize &value );
   };

   KJS_BINDING( QSize )

}
#endif // BIND_QSize_H


