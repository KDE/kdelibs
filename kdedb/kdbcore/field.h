/*
   This file is part of the KDB libraries
   Copyright (c) 2000 Praduroux Alessandro <pradu@thekompany.com>
 
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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/     

#ifndef KDB_FIELD_H
#define KDB_FIELD_H "$Id$"

#include <qlist.h>
#include <qstring.h>
#include <qdatetime.h>

#include <kdb/kdb.h>
#include <kdb/object.h>
#include <kdb/handler.h>
#include <kdb/value.h>

namespace KDB {


class Field;
typedef QList<Field>         FieldList;
typedef QListIterator<Field> FieldIterator;

/**
 * Representation of a field object.
 *
 * It knows its name, its datatype, constraints when available (like
 * null/not null) and, when it is part of a record, its value.
 *
 * The field object can manage only the data type of the underlying column. 
 * Conversion operators are provided for all common C++ datatypes. If a
 * conversion is not allowed, a @ref KDB::ConversionException is raised.
 *
 * It is responsibility of the plugin to instantiate a correct field object
 * for the underlying datatype. For datatypes not supported natively by C++,
 * the Plugin can handle either QStrings or QByteArrays.
 *
 * The datatype supported are the following:
 *
 * @li  QString (can be used for unknown datatypes)
 * @li  QStringList (for sets and one-dimensional array)
 * @li  QDateTime
 * @li  QByteArray (for BLOBs and Unknown datatypes)
 * @li  char
 * @li  short
 * @li  int
 * @li  long
 * @li  float
 * @li  double
 * @li  long double
 * @li  bool
 *
 * All numeric types are handled both in signed and unsigned form.
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @version kdbcore 0.0.2
 */

 //BIG TODO: conversion operators and logic
class Field :public Object {
    
    friend class Recordset;
    friend class RecordsetIterator;
    friend class Table;
    
    Q_OBJECT
  
 public:

    ~Field();

    void setPrecision(unsigned int size);
    unsigned int precision();

    void setSize(unsigned int size);
    unsigned int size();

    void setType(DataType t);
    DataType type() ;

    void setNativeType(const QString &type);
    QString nativeType();

    void setDefaultValue(const Value &val);
    Value defaultValue() ;

    void setComment(const QString &val);
    QString comment() ;

    void setConstraint(const QString &val);
    QString constraint() ;

    void setAcceptNull(bool);
    bool acceptNull();

    bool isNull();

    Value value();
    void setValue(const Value &v, bool init = false);

    Field &operator = (const Value &v);
    
    Value *operator ->();

    bool changed();
 private:
    Field (const QString &name, QObject *parent = 0);
    Field (HandlerPtr h,  const QString &name, QObject *parent = 0);

    Field & operator = (const Field &) { return *this; };
    
    Value m_val;
    Value m_default;

    HandlerPtr m_handler;

    DataType m_type;
    QString m_native;
    unsigned int m_size;
    unsigned int m_precision;
    QString m_constraint;
    bool m_acceptNull;
    QString m_comment;
    bool m_changed;
};
 
}

#endif
