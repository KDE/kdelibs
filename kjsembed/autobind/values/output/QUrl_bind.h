#ifndef BIND_QUrl_H
#define BIND_QUrl_H

#include <value_binding.h>
#include <static_binding.h>

class QUrl;

namespace KJSEmbed
{
   class QUrlBinding : public ValueBinding
   {
       public:
           QUrlBinding( KJS::ExecState *exec, const QUrl &value );
   };

   KJS_BINDING( QUrl )

}
#endif // BIND_QUrl_H


