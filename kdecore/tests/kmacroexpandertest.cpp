#include <kmacroexpander.h>
#include <stdio.h>
#include <kapplication.h>
#include <stdlib.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kcharsets.h>
#include <qtextcodec.h>

bool check(QString txt, QString s, QString a, QString b)
{
  if (a.isEmpty())
     a = QString::null;
  if (b.isEmpty())
     b = QString::null;
  if (a == b) {
    kdDebug() << txt << " (" << s << ") : checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
  }
  else {
    kdDebug() << txt << " (" << s << ") : checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl;
    exit(1);
  }
  return true;
}

int main(int argc, char *argv[])
{
  KApplication app(argc,argv,"kmacroexpandertest",false,false);

  QString s,t;
  QMap<QChar,QString> map;
  map.insert('a', "%n");
  map.insert('f', "filename.txt");
  map.insert('u', "http://www.kde.org/index.html");
  map.insert('n', "Restaurant \"Chew It\"");

  s = t = "Title: %a - %f - %u - %n - %%";
  KMacroExpander::expandMacros(t, map);
  check( "KMacroExpander::expandMacros", s, t, "Title: %n - filename.txt - http://www.kde.org/index.html - Restaurant \"Chew It\" - %");

  s = t = "kedit --caption %n %f";
  KMacroExpander::expandMacrosShellQuote(t, map);
  check( "KMacroExpander::expandMacrosShellQuote", s, t, "kedit --caption 'Restaurant \"Chew It\"' 'filename.txt'");

  map.replace('n', "Restaurant 'Chew It'");
  s = t = "kedit --caption %n %f";
  KMacroExpander::expandMacrosShellQuote(t, map);
  check( "KMacroExpander::expandMacrosShellQuote", s, t, "kedit --caption 'Restaurant '\\''Chew It'\\''' 'filename.txt'");

#if 0
  s = t = "kedit --caption \"%n\" %f";
  KMacroExpander::expandMacrosShellQuote(t, map);
  check( "KMacroExpander::expandMacrosShellQuote", s, t, "kedit --caption \"\"'Restaurant '\\''Chew It'\\'''\"\" 'filename.txt'");
#endif

  map.replace('n', "Restaurant \"Chew It\"");
  s = t = "kedit --caption \"%n\" %f";
  KMacroExpander::expandMacrosShellQuote(t, map);
  check( "KMacroExpander::expandMacrosShellQuote", s, t, "kedit --caption \"Restaurant \\\"Chew It\\\"\" 'filename.txt'");

  map.replace('n', "Restaurant $HOME");
  s = t = "kedit --caption \"%n\" %f";
  KMacroExpander::expandMacrosShellQuote(t, map);
  check( "KMacroExpander::expandMacrosShellQuote", s, t, "kedit --caption \"Restaurant \\$HOME\" 'filename.txt'");

  map.replace('n', "Restaurant `echo hello`");
  s = t = "kedit --caption \"%n\" %f";
  KMacroExpander::expandMacrosShellQuote(t, map);
  check( "KMacroExpander::expandMacrosShellQuote", s, t, "kedit --caption \"Restaurant \\`echo hello\\`\" 'filename.txt'");

  map.replace('n', "Restaurant `echo hello`");
  s = t = "kedit --caption \"`echo %n`\" %f";
  KMacroExpander::expandMacrosShellQuote(t, map);
  check( "KMacroExpander::expandMacrosShellQuote", s, t, "kedit --caption \"$( echo 'Restaurant `echo hello`')\" 'filename.txt'");

  QMap<QString,QString> smap;
  smap.insert("foo", "%n");
  smap.insert("file", "filename.txt");
  smap.insert("url", "http://www.kde.org/index.html");
  smap.insert("name", "Restaurant \"Chew It\"");

  s = t = "Title: %foo - %file - %url - %name - %";
  KMacroExpander::expandMacros(t, smap);
  check( "KMacroExpander::expandMacros", s, t, "Title: %n - filename.txt - http://www.kde.org/index.html - Restaurant \"Chew It\" - %");

  s = t = "Title: %{foo} - %{file} - %{url} - %{name} - %";
  KMacroExpander::expandMacros(t, smap);
  check( "KMacroExpander::expandMacros", s, t, "Title: %n - filename.txt - http://www.kde.org/index.html - Restaurant \"Chew It\" - %");

  s = t = "Title: %foo-%file-%url-%name-%";
  KMacroExpander::expandMacros(t, smap);
  check( "KMacroExpander::expandMacros", s, t, "Title: %n-filename.txt-http://www.kde.org/index.html-Restaurant \"Chew It\"-%");

  printf("\nTest OK !\n");
}

