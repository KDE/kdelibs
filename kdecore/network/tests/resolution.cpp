/*  -*- C++ -*-
 *  Copyright (C) 2003-2005 Thiago Macieira <thiago@kde.org>
 *
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <ksocketaddress.h>
#include "kdebug.h"
#include "resolution.h"

using namespace KNetwork;

QString family(int fam)
{
  QString result;

  switch (fam)
    {
    case AF_INET:
      result = QLatin1String("AF_INET (%1)");
      break;
     
#ifdef AF_INET6
    case AF_INET6:
      result = QLatin1String("AF_INET6 (%1)");
      break;
#endif

    case AF_UNIX:
      result = QLatin1String("AF_UNIX (%1)");
      break;

    case AF_UNSPEC:
      result = QLatin1String("AF_UNSPEC (%1)");
      break;

    default:
      result = QLatin1String("Unknown (%1)");
      break;
    }

  return result.arg(fam);
}

QString socktype(int st)
{
  QString result;

  switch (st)
    {
    case SOCK_STREAM:
      result = QLatin1String("SOCK_STREAM (%1)");
      break;

    case SOCK_DGRAM:
      result = QLatin1String("SOCK_DGRAM (%1)");
      break;

    case SOCK_RAW:
      result = QLatin1String("SOCK_RAW (%1)");
      break;

    default:
      result = QLatin1String("Unknown (%1)");
      break;
    }

  return result.arg(st);
}

void showResults(const KResolverResults& r)
{
  if (r.size() > 0)
    {
      for (int i = 0; i < r.size(); ++i)
	kDebug() << i << ": family=" << family(r.at(i).family())
		  << ", len=" << r.at(i).length()
		  << ", address=" << r.at(i).address().toString()
		  << ", socktype=" << socktype(r.at(i).socketType())
		  << ", protocol=" << r.at(i).protocol()
		  << ", canon=" << r.at(i).canonicalName()
		  << ", encoded=" << QLatin1String(r.at(i).encodedName())
		  << endl;
    }
  else
    kDebug() << "Resolution failed: " 
	      << KResolver::errorString(r.error(), r.systemError())
	      << endl;
}

Test::Test()
{
}

void Test::results(const KResolverResults& r)
{
  showResults(r);
  QCoreApplication::exit(0);
}

int main(int argc, char **argv)
{
  QCoreApplication a(argc, argv);

  if (argc < 2)
    return 1;

  kDebug() << "Attempting synchronous resolution" << endl;
  showResults(KResolver::resolve(QString::fromLocal8Bit(argv[1]), 
				 QString::fromLocal8Bit(argv[2]),
				 KResolver::CanonName));

  kDebug() << endl << "Attempting asynchronous resolution" << endl;
  Test t;
  KResolver::resolveAsync(&t, SLOT(results(const KNetwork::KResolverResults&)),
			  QString::fromLocal8Bit(argv[1]), 
			  QString::fromLocal8Bit(argv[2]),
			  KResolver::CanonName);

  return a.exec();
}

#include "resolution.moc"

