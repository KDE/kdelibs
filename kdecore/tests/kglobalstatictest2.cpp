#include <QCoreApplication>
#include <kglobal.h>

bool success = false;

class Test
{
    public:
        Test()
            : m_deleted(false)
        {}

        ~Test()
        {
            Q_ASSERT(!m_deleted);
            m_deleted = true;
            success = true;
            Q_ASSERT(QCoreApplication::instance());
        }

    private:
        bool m_deleted;
};

K_GLOBAL_STATIC(Test, test);

int main(int argc, char **argv)
{
    {
        QCoreApplication app(argc, argv);
        Test *x = test();
        Q_ASSERT(x != 0);
        Test *y = test();
        Q_ASSERT(x == y);
    }
    Q_ASSERT(success);
    Test *x = test();
    Q_ASSERT(x == 0);
    return (success ? 0 : 1);
}
