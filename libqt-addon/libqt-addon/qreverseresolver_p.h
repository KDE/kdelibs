/*  -*- C++ -*-
 *  Copyright (C) 2003 Thiago Macieira <thiago.macieira@kdemail.net>
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

#ifndef QREVERSERESOLVER_P_H
#define QREVERSERESOLVER_P_H

struct sockaddr;

ADDON_START
namespace
{
  /* @class QReverseResolve qreverseresolver_p.h qreverseresolver_p.h
   * @brief this class produces a reverse resolution
   *
   * @author Thiago Macieira <thiago.macieira@kdemail.net>
   */
  class QReverseResolve: public QResolverWorkerBase
  {
  public:
    class Signal: public QObject
    {
      Q_OBJECT
    public:
      Signal()
      { }

      inline void emitSignal(const QReverseResults& r)
      { emit finished(r); }

    signals:
      void finished(QReverseResults& r);
    };

    QReverseResolve(const struct sockaddr* sa, Q_UINT16 salen, int flags)
      : m_sa(sa), m_salen(salen), m_flags(flags)
    { }

    virtual ~QReverseResolve()
    { }

    virtual bool preprocess()
    { return true; }

    virtual bool run();

    virtual bool postprocess();

  private:
    const struct sockaddr* m_sa;
    Q_UINT16 m_salen;
    int m_flags;

  public:
    QReverseResults results;
    Signal *signal;
  };
}

ADDON_END

#endif
