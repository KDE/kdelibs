#ifndef BIND_QSizeF_H
#define BIND_QSizeF_H

#include <value_binding.h>
#include <static_binding.h>

class QSizeF;

namespace KJSEmbed
{
   class QSizeFBinding : public ValueBinding
   {
       public:
           QSizeFBinding( KJS::ExecState *exec, const QSizeF &value );
   };

   KJS_BINDING( QSizeF )

}
#endif // BIND_QSizeF_H


