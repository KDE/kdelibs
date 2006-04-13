#ifndef BIND_QGradient_H
#define BIND_QGradient_H

#include <value_binding.h>
#include <static_binding.h>

class QGradient;

namespace KJSEmbed
{
   class QGradientBinding : public ValueBinding
   {
       public:
           QGradientBinding( KJS::ExecState *exec, const QGradient &value );
       private:
           static const KJS::ClassInfo info;
           virtual const KJS::ClassInfo* classInfo() const { return &info; }
   };

   KJS_BINDING( QGradient )

}
#endif // BIND_QGradient_H


