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
       private:
           static const KJS::ClassInfo info;
           virtual const KJS::ClassInfo* classInfo() const { return &info; }
   };

   KJS_BINDING( QLinearGradient )

}
#endif // BIND_QLinearGradient_H


