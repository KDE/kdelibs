/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef PREDICATEPARSE_H
#define PREDICATEPARSE_H

void PredicateLexer_unknownToken(const char* text);

void PredicateParse_setResult(void *result);
void PredicateParse_errorDetected(const char* error);
void PredicateParse_destroy(void *pred);

void *PredicateParse_newAtom(char *interface, char *property, void *value);
void *PredicateParse_newMaskAtom(char *interface, char *property, void *value);
void *PredicateParse_newIsAtom(char *interface);
void *PredicateParse_newAnd(void *pred1, void *pred2);
void *PredicateParse_newOr(void *pred1, void *pred2);
void *PredicateParse_newStringValue(char *val);
void *PredicateParse_newBoolValue(int val);
void *PredicateParse_newNumValue(int val);
void *PredicateParse_newDoubleValue(double val);
void *PredicateParse_newEmptyStringListValue();
void *PredicateParse_newStringListValue(char *name);
void *PredicateParse_appendStringListValue(char *name, void *list);

#endif
