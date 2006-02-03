#ifndef SIZE_H
#define SIZE_H

#include <value_binding.h>
#include <static_binding.h>

class QSize;

namespace KJSEmbed
{
    class SizeBinding : public ValueBinding
    {
        public:
            SizeBinding( KJS::ExecState *exec, const QSize &value );
    };

    KJS_BINDING( Size )

}
#endif
