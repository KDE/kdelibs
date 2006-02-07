/*
 *  Copyright (C) 2002 David Faure   <faure@kde.org>
 *  Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

// ------ this test works only if your terminal application is set to "x-term" ------

#include "kruntest.h"

#include <kapplication.h>
#include <kdebug.h>
#include <kshell.h>
#include <kservice.h>
#include <kde_file.h>
#include <kcmdlineargs.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qdir.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

const int MAXKRUNS = 100;

testKRun * myArray[MAXKRUNS];

void testKRun::foundMimeType( const QString& _type )
{
  kDebug() << "testKRun::foundMimeType " << _type << endl;
  kDebug() << "testKRun::foundMimeType URL=" << m_strURL.url() << endl;
  m_bFinished = true;
  m_timer.start( 0, true );
  return;
}

Receiver::Receiver()
{
        QVBoxLayout *lay = new QVBoxLayout(this);
        lay->setAutoAdd(true);
        QPushButton * h = new QPushButton( "Press here to terminate", this );
        start = new QPushButton( "Launch KRuns", this );
        stop = new QPushButton( "Stop those KRuns", this );
        stop->setEnabled(false);
        QObject::connect( h, SIGNAL(clicked()), kapp, SLOT(quit()) );
        QObject::connect( start, SIGNAL(clicked()), this, SLOT(slotStart()) );
        QObject::connect( stop, SIGNAL(clicked()), this, SLOT(slotStop()) );

        adjustSize();
        show();
}

void Receiver::slotStop()
{
  for (int i = 0 ; i < MAXKRUNS ; i++ )
  {
    kDebug() << " deleting krun " << i << endl;
    delete myArray[i];
  }
  start->setEnabled(true);
  stop->setEnabled(false);
}


void Receiver::slotStart()
{
  for (int i = 0 ; i < MAXKRUNS ; i++ )
  {
    kDebug() << "creating testKRun " << i << endl;
    myArray[i] = new testKRun( KUrl("file:/tmp"), window(),0, true, false /* no autodelete */ );
  }
  start->setEnabled(false);
  stop->setEnabled(true);
}

void check(QString txt, QString a, QString b)
{
  if (a.isEmpty())
     a = QString::null;
  if (b.isEmpty())
     b = QString::null;
  if (a == b)
    kDebug() << txt << " : '" << a << "' - ok" << endl;
  else {
    kDebug() << txt << " : '" << a << "' but expected '" << b << "' - KO!" << endl;
    exit(1);
  }
}

const char *bt(bool tr) { return tr?"true":"false"; }

void checkBN(QString a, bool tr, QString b)
{
  check( QString().sprintf("binaryName('%s', %s)", a.latin1(), bt(tr)), KRun::binaryName(a, tr), b);
}

void checkPDE(const KService &service, const KUrl::List &urls, bool hs, bool tf, QString b)
{
  check(
   QString().sprintf("processDesktopExec( "
      "service = {\nexec = %s\nterminal = %s, terminalOptions = %s\nsubstituteUid = %s, user = %s },"
       "\nURLs = { %s },\nhas_shell = %s, temp_files = %s )",
      service.exec().latin1(), bt(service.terminal()), service.terminalOptions().latin1(), bt(service.substituteUid()), service.username().latin1(),
       KShell::joinArgs(urls.toStringList()).latin1(), bt(hs), bt(tf)),
   KShell::joinArgs(KRun::processDesktopExec(service,urls,hs,tf)), b);
}

int main(int argc, char **argv)
{
  KCmdLineArgs::init(argc,argv,"kruntest", 0, 0, 0, 0);
  KApplication app;
  // First some non-interactive tests
  checkBN( "/usr/bin/ls", true, "ls");
  checkBN( "/usr/bin/ls", false, "/usr/bin/ls");
  checkBN( "/path/to/wine \"long argument with path\"", true, "wine" );
  checkBN( "/path/with/a/sp\\ ace/exe arg1 arg2", true, "exe" );
  checkBN( "\"progname\" \"arg1\"", true, "progname" );
  checkBN( "'quoted' \"arg1\"", true, "quoted" );
  checkBN( " 'leading space'   arg1", true, "leading space" );

  KUrl::List l0;
  KUrl::List l1; l1 << KUrl( "file:/tmp" );
  KUrl::List l2; l2 << KUrl( "http://localhost/foo" );
  KUrl::List l3; l3 << KUrl( "file:/local/file" ) << KUrl( "http://remotehost.org/bar" );

  static const char
    *execs[] = { "Exec=date -u", "Exec=echo $$PWD" },
    *terms[] = { "Terminal=false", "Terminal=true\nTerminalOptions=-T \"%f - %c\"" },
    *sus[] = { "X-KDE-SubstituteUID=false", "X-KDE-SubstituteUID=true\nX-KDE-Username=sprallo" },
    *rslts[] = {
"'date' '-u'", // 0
"'/bin/sh' '-c' 'echo $PWD '", // 1
"'x-term' '-T' ' - just_a_test' '-e' 'date' '-u'", // 2
"'x-term' '-T' ' - just_a_test' '-e' '/bin/sh' '-c' 'echo $PWD '", // 3
"'kdesu' '-u' 'sprallo' '-c' 'date -u '", // 4
"'kdesu' '-u' 'sprallo' '-c' '/bin/sh -c '\\''echo $PWD '\\'''", // 5
"'x-term' '-T' ' - just_a_test' '-e' 'su' 'sprallo' '-c' 'date -u '", // 6
"'x-term' '-T' ' - just_a_test' '-e' 'su' 'sprallo' '-c' '/bin/sh -c '\\''echo $PWD '\\'''", // 7
"'date -u '", // 8
"'echo $PWD '", // 9
"'x-term -T \" - just_a_test\"' '-e' 'date -u '", // a
"'x-term -T \" - just_a_test\"' '-e' '/bin/sh -c '\\''echo $PWD '\\'''", // b
"'kdesu' '-u' 'sprallo' '-c' ''\\''date -u '\\'''", // c
"'kdesu' '-u' 'sprallo' '-c' ''\\''/bin/sh -c '\\''\\'\\'''\\''echo $PWD '\\''\\'\\'''\\'''\\'''", // d
"'x-term -T \" - just_a_test\"' '-e' 'su' 'sprallo' '-c' ''\\''date -u '\\'''", // e
"'x-term -T \" - just_a_test\"' '-e' 'su' 'sprallo' '-c' ''\\''/bin/sh -c '\\''\\'\\'''\\''echo $PWD '\\''\\'\\'''\\'''\\'''", // f
    };
  for (int hs = 0; hs < 2; hs++)
    for (int su = 0; su < 2; su++)
      for (int te = 0; te < 2; te++)
        for (int ex = 0; ex < 2; ex++) {
          int fd = creat("kruntest.desktop", 0666);
	  FILE *f;
          if (fd < 0) abort();
	  f = KDE_fdopen(fd, "w");
          fprintf(f, "[Desktop Entry]\n"
		"Type=Application\n"
		"Name=just_a_test\n"
		"Icon=~/icon.png\n"
		"%s\n%s\n%s\n",execs[ex],terms[te],sus[su]);
          close(fd);
	  fclose(f);
          KService s(QDir::currentPath() + "/kruntest.desktop");
          unlink("kruntest.desktop");
          checkPDE( s, l0, hs, false, rslts[ex+te*2+su*4+hs*8]);
        }

  KService s1("dummy", "kate %U", "app");
  checkPDE( s1, l0, false, false, "'kate'");
  checkPDE( s1, l1, false, false, "'kate' '/tmp'");
  checkPDE( s1, l2, false, false, "'kate' 'http://localhost/foo'");
  checkPDE( s1, l3, false, false, "'kate' '/local/file' 'http://remotehost.org/bar'");
  KService s2("dummy", "kate %u", "app");
  checkPDE( s2, l0, false, false, "'kate'");
  checkPDE( s2, l1, false, false, "'kate' '/tmp'");
  checkPDE( s2, l2, false, false, "'kate' 'http://localhost/foo'");
  checkPDE( s2, l3, false, false, "'kate'");
  KService s3("dummy", "kate %F", "app");
  checkPDE( s3, l0, false, false, "'kate'");
  checkPDE( s3, l1, false, false, "'kate' '/tmp'");
  checkPDE( s3, l2, false, false, "'kfmexec' 'kate %F' 'http://localhost/foo'");
  checkPDE( s3, l3, false, false, "'kfmexec' 'kate %F' 'file:/local/file' 'http://remotehost.org/bar'");

  checkPDE( s3, l1, false, true, "'kfmexec' '--tempfiles' 'kate %F' 'file:/tmp'");
  checkPDE( s3, l1, true, true, "''\\''kfmexec'\\'' '\\''--tempfiles'\\'' '\\''kate %F'\\'' '\\''file:/tmp'\\'''");

  KService s4("dummy", "sh -c \"kate \"'\\\"'\"%F\"'\\\"'", "app");
  checkPDE( s4, l1, false, false, "'kate' '\"/tmp\"'");

  Receiver receiver;
  app.setMainWidget(&receiver);
  return app.exec();
}

#include "kruntest.moc"
