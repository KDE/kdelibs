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

#include "config.h"

#include <qcstring.h>
#include <qstring.h>

#include "qresolver.h"
#include "qresolver_p.h"
#include "qresolverworkerbase.h"

ADDON_USE
using namespace ADDON_NAMESPACE::Internal;

QResolverWorkerBase::QResolverWorkerBase()
  : th(0L), input(0L), m_finished(0), m__reserved(0)
{
}

QResolverWorkerBase::~QResolverWorkerBase()
{
}

QString QResolverWorkerBase::nodeName() const
{
  if (input)
    return input->node;
  return QString::null;
}

QString QResolverWorkerBase::serviceName() const
{
  if (input)
    return input->service;
  return QString::null;
}

int QResolverWorkerBase::flags() const
{
  if (input)
    return input->flags;
  return 0;
}

int QResolverWorkerBase::familyMask() const
{
  if (input)
    return input->familyMask;
  return 0;
}

int QResolverWorkerBase::socketType() const
{
  if (input)
    return input->socktype;
  return 0;
}

int QResolverWorkerBase::protocol() const
{
  if (input)
    return input->protocol;
  return 0;
}

QCString QResolverWorkerBase::protocolName() const
{
  QCString res;
  if (input)
    res = input->protocolName;
  return res;
}

void QResolverWorkerBase::finished()
{
  m_finished = true;
}

bool QResolverWorkerBase::postprocess()
{
  return true;			// no post-processing is a always successful postprocessing
}

bool QResolverWorkerBase::enqueue(QResolverWorkerBase* worker)
{
  RequestData *myself = th->data;
  RequestData *newrequest = new RequestData;
  newrequest->obj = 0;
  newrequest->input = input; // same input
  newrequest->requestor = myself;
  newrequest->nRequests = 0;
  newrequest->worker = worker;
  myself->nRequests++;
  QResolverManager::manager()->dispatch(newrequest);
  return true;
}

void QResolverWorkerFactoryBase::registerNewWorker(QResolverWorkerFactoryBase* factory)
{
  QResolverManager::manager()->registerNewWorker(factory);
}
