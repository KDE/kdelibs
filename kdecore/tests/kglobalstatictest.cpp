#include <kglobal.h>
#include <QtDebug>

class A
{
    public:
        A() : i(1) {}
        int i;
};

K_GLOBAL_STATIC(A, globalA);
K_GLOBAL_STATIC(A, globalA2);

class B
{
    public:
        ~B()
        {
            Q_ASSERT(globalA.isDestroyed());
            qDebug() << "B::~B()";
        }
};

K_GLOBAL_STATIC(B, globalB);

int main(int, char**)
{
    Q_ASSERT(globalB);

    Q_ASSERT(!globalA.isDestroyed());
    A *a = globalA;
    Q_ASSERT(a);
    Q_ASSERT(a == globalA);
    Q_ASSERT(globalA->i == 1);
    Q_ASSERT(!globalA.isDestroyed());

    Q_ASSERT(!globalA2.isDestroyed());
    Q_ASSERT(globalA2);
    Q_ASSERT(globalA2->i == 1);
    Q_ASSERT(!globalA2.isDestroyed());
    globalA2.destroy();
    Q_ASSERT(globalA2.isDestroyed());

    return 0;
}
