/**************************************************************************

    notearray.cc  - NoteArray class, which holds an array of notes
    This file is part of LibKMid 0.9.5
    Copyright (C) 1998,99,2000  Antonio Larrosa Jimenez
    LibKMid's homepage : http://www.arrakis.es/~rlarrosa/libkmid.html            

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
 
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    Send comments and bug fixes to Antonio Larrosa <larrosa@kde.org>

***************************************************************************/

#include "notearray.h"
#include <string.h>

NoteArray::NoteArray(void)
{
  totalAllocated=50;
  data=new noteCmd[totalAllocated];
  lastAdded=0L;
}

NoteArray::~NoteArray()
{
  delete data;
  totalAllocated=0;
}

NoteArray::noteCmd *NoteArray::pointerTo(ulong pos)
{
  if (pos<totalAllocated) return &data[pos];
  while (pos>=totalAllocated)
  {
    noteCmd *tmp=new noteCmd[totalAllocated*2];
    memcpy(tmp,data,sizeof(noteCmd)*totalAllocated);
    delete data;
    data=tmp;
    totalAllocated*=2;
  }
  return &data[pos];
}

void NoteArray::at(ulong pos, ulong ms,int chn,int cmd,int note)
{
  noteCmd *tmp=pointerTo(pos);
  tmp->ms=ms;
  tmp->chn=chn;
  tmp->cmd=cmd;
  tmp->note=note;
}

void NoteArray::at(ulong pos, noteCmd s)
{
  noteCmd *tmp=pointerTo(pos);
  tmp->ms=s.ms;
  tmp->chn=s.chn;
  tmp->cmd=s.cmd;
  tmp->note=s.note;
}

NoteArray::noteCmd NoteArray::at(int pos)
{
  return *pointerTo(pos);
}

void NoteArray::add(ulong ms,int chn,int cmd,int note)
{
  if (lastAdded==NULL)
  {
    lastAdded=data;
    last=0;
  }
  else
  {
    last++;
    if (last==totalAllocated) lastAdded=pointerTo(totalAllocated);
    else lastAdded++;
  }
  lastAdded->ms=ms;
  lastAdded->chn=chn;
  lastAdded->cmd=cmd;
  lastAdded->note=note;
}

void NoteArray::next(void)
{
  if (it==lastAdded) {it=NULL;return;};
  it++;
}

void NoteArray::moveIteratorTo(ulong ms,int *pgm)
{
  noteCmd *ncmd;
  iteratorBegin();
  ncmd=get();
  int pgm2[16];
  for (int j=0;j<16;j++) pgm2[j]=0;
  while ((ncmd!=NULL)&&(ncmd->ms<ms))
  {
    if (ncmd->cmd==2) pgm2[ncmd->chn]=ncmd->note;
    next();
    ncmd=get();
  }
  if (pgm!=NULL)
  {
    for (int i=0;i<16;i++) pgm[i]=pgm2[i];
  }
}
