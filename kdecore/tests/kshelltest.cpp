#include <kshell.h>

#include <iostream>

static QByteArray
ps(const QString &s)
{
  if (s.isNull())
    return "(null)";
  else
    return s.toLocal8Bit();
}

static void
tx(const char *t)
{
    std::cout << t << " -> " << ps(KShell::tildeExpand(t)).data() << std::endl;
}

static void
sj(const char *t, int flg)
{
    int err;
    std::cout << t << " (" << flg << ") -> " << ps(KShell::joinArgsDQ(KShell::splitArgs(t, flg, &err))).data() << " (" << err << ")" << std::endl;
}

int main()
{
#if 1
    tx("~");
    tx("~/sulli");
    tx("~root");
    tx("~root/sulli");
    tx("~sulli");
#endif
#if 1
    QStringList lst;
    lst << "this" << "is" << "text";
    std::cout << KShell::joinArgs(lst).latin1() << std::endl;
#endif
#if 1
    sj("\"~sulli\" 'text' 'jo'\"jo\" $'crap' $'\\\\\\'\\ca\\e\\x21' ha\\ lo ", KShell::NoOptions);
    sj("\"~sulli\" 'text'", KShell::TildeExpand);
    sj("~\"sulli\" 'text'", KShell::TildeExpand);
    sj("~/\"sulli\" 'text'", KShell::TildeExpand);
    sj("~ 'text' ~", KShell::TildeExpand);
    sj("~sulli ~root", KShell::TildeExpand);
#endif
    sj("say \" error", KShell::NoOptions);
    sj("say `echo no error`", KShell::NoOptions);
    sj("say \" still error", KShell::AbortOnMeta);
    sj("say `echo meta error`", KShell::AbortOnMeta);
    sj("BLA=say echo meta", KShell::AbortOnMeta);
    sj("B\"L\"A=say FOO=bar echo meta", KShell::AbortOnMeta);
}
