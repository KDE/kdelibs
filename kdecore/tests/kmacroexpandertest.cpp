#include <kmacroexpander.h>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

#include <stdlib.h>

bool check(QString txt, QString s, QString a, QString b)
{
  if (a.isEmpty())
     a = QString::null;
  if (b.isEmpty())
     b = QString::null;
  if (a == b)
    kdDebug() << txt << " (" << s << ") : '" << a << "' - ok" << endl;
  else {
    kdDebug() << txt << " (" << s << ") : got '" << a << "' but expected '" << b << "' - KO!" << endl;
    exit(1);
  }
  return true;
}

class MyCExpander : public KCharMacroExpander {
public:
  MyCExpander() : KCharMacroExpander() {}
protected:
  bool expandMacro(QChar chr, QStringList &ret)
  {
    if (chr == 'm') {
      ret = QString("expanded");
      return true;
    }
    return false;
  }
};

class MyWExpander : public KWordMacroExpander {
public:
  MyWExpander() : KWordMacroExpander() {}
protected:
  bool expandMacro(const QString &str, QStringList &ret)
  {
    if (str == "macro") {
      ret = QString("expanded");
      return true;
    }
    return false;
  }
};

int main(int argc, char *argv[])
{
  KCmdLineArgs::init(argc, argv, ":", "", "", "");
  KApplication app(false,false);
  QString s, s2;

  QMap<QChar,QStringList> map1;
  map1.insert('n', "Restaurant \"Chew It\"");
  QStringList li;
  li << "element1" << "'element2'";
  map1.insert('l', li);

  s = "text %l %n text";
  check( "KMacroExpander::expandMacros", s, KMacroExpander::expandMacros(s, map1), "text element1 'element2' Restaurant \"Chew It\" text");
  check( "KMacroExpander::expandMacrosShellQuote", s, KMacroExpander::expandMacrosShellQuote(s, map1), "text 'element1' ''\\''element2'\\''' 'Restaurant \"Chew It\"' text");
  s = "text \"%l %n\" text";
  check( "KMacroExpander::expandMacrosShellQuote", s, KMacroExpander::expandMacrosShellQuote(s, map1), "text \"element1 'element2' Restaurant \\\"Chew It\\\"\" text");

  QMap<QChar,QString> map;
  map.insert('a', "%n");
  map.insert('f', "filename.txt");
  map.insert('u', "http://www.kde.org/index.html");
  map.insert('n', "Restaurant \"Chew It\"");

  s = "Title: %a - %f - %u - %n - %%";
  check( "KMacroExpander::expandMacros", s, KMacroExpander::expandMacros(s, map), "Title: %n - filename.txt - http://www.kde.org/index.html - Restaurant \"Chew It\" - %");

  s = "kedit --caption %n %f";
  check( "KMacroExpander::expandMacrosShellQuote", s, KMacroExpander::expandMacrosShellQuote(s, map), "kedit --caption 'Restaurant \"Chew It\"' 'filename.txt'");

  map.replace('n', "Restaurant 'Chew It'");
  s = "kedit --caption %n %f";
  check( "KMacroExpander::expandMacrosShellQuote", s, KMacroExpander::expandMacrosShellQuote(s, map), "kedit --caption 'Restaurant '\\''Chew It'\\''' 'filename.txt'");

  s = "kedit --caption \"%n\" %f";
  check( "KMacroExpander::expandMacrosShellQuote", s, KMacroExpander::expandMacrosShellQuote(s, map), "kedit --caption \"Restaurant 'Chew It'\" 'filename.txt'");

  map.replace('n', "Restaurant \"Chew It\"");
  s = "kedit --caption \"%n\" %f";
  check( "KMacroExpander::expandMacrosShellQuote", s, KMacroExpander::expandMacrosShellQuote(s, map), "kedit --caption \"Restaurant \\\"Chew It\\\"\" 'filename.txt'");

  map.replace('n', "Restaurant $HOME");
  s = "kedit --caption \"%n\" %f";
  check( "KMacroExpander::expandMacrosShellQuote", s, KMacroExpander::expandMacrosShellQuote(s, map), "kedit --caption \"Restaurant \\$HOME\" 'filename.txt'");

  map.replace('n', "Restaurant `echo hello`");
  s = "kedit --caption \"%n\" %f";
  check( "KMacroExpander::expandMacrosShellQuote", s, KMacroExpander::expandMacrosShellQuote(s, map), "kedit --caption \"Restaurant \\`echo hello\\`\" 'filename.txt'");

  map.replace('n', "Restaurant `echo hello`");
  s = "kedit --caption \"`echo %n`\" %f";
  check( "KMacroExpander::expandMacrosShellQuote", s, KMacroExpander::expandMacrosShellQuote(s, map), "kedit --caption \"$( echo 'Restaurant `echo hello`')\" 'filename.txt'");

  QMap<QString,QString> smap;
  smap.insert("foo", "%n");
  smap.insert("file", "filename.txt");
  smap.insert("url", "http://www.kde.org/index.html");
  smap.insert("name", "Restaurant \"Chew It\"");

  s = "Title: %foo - %file - %url - %name - %";
  check( "KMacroExpander::expandMacros", s, KMacroExpander::expandMacros(s, smap), "Title: %n - filename.txt - http://www.kde.org/index.html - Restaurant \"Chew It\" - %");

  s = "Title: %{foo} - %{file} - %{url} - %{name} - %";
  check( "KMacroExpander::expandMacros", s, KMacroExpander::expandMacros(s, smap), "Title: %n - filename.txt - http://www.kde.org/index.html - Restaurant \"Chew It\" - %");

  s = "Title: %foo-%file-%url-%name-%";
  check( "KMacroExpander::expandMacros", s, KMacroExpander::expandMacros(s, smap), "Title: %n-filename.txt-http://www.kde.org/index.html-Restaurant \"Chew It\"-%");

  s = "Title: %{file} %{url";
  check( "KMacroExpander::expandMacros", s, KMacroExpander::expandMacros(s, smap), "Title: filename.txt %{url");

  MyCExpander mx1;
  s = "subst %m but not %n equ %%";
  s2 = s;
  mx1.expandMacros(s2);
  check( "MyCExpander::expandMacros", s, s2, "subst expanded but not %n equ %");

  MyWExpander mx2;
  s = "subst %macro but not %not equ %%";
  s2 = s;
  mx2.expandMacros(s2);
  check( "MyWExpander::expandMacros", s, s2, "subst expanded but not %not equ %");

  kdDebug() << endl << "Test OK!" << endl;
}

