#include <kconfigskeleton.h>

class MyPrefs : public KConfigSkeleton
{
  public:
    MyPrefs( const QString &a ) : KConfigSkeleton( a ) {}
};
