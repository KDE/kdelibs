/* This file is part of the KDE libraries
   Copyright (C) 2009 Jonathan Schmidt-Dominé <devel@the-user.org>

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

#ifndef KTYPETRAITS_H
#define KTYPETRAITS_H

#include "ktypelist.h"
#include "ktypelistutils.h"
#include <stdint.h>

class QObject;
class QWidget;

#define NC(...) __VA_ARGS__

namespace KTypeTraits
{

template<class Types, bool correct>
struct TypeSelectImpl
{
        typedef typename Types::Head Result;
};

template<class Types, unsigned int size>
struct TypeSelect
{
    typedef STATIC_IF(sizeof(typename Types::Head) == size, typename Types::Head, NC(typename TypeSelect<typename Types::Tail, size>::Result)) Result;
};

template<unsigned int size>
struct TypeSelect<KDE::NullType, size>
{
    typedef KDE::NullType Result;
};

template<typename T, typename U>
struct CanConvert
{
private:
    class Big { char dummy[2]; };
    static char test(const U&);
    static Big test(...);
    static T makeT();
public:
    enum { value = sizeof(test(makeT())) == sizeof(char) };
};

template<typename T, typename U>
struct IsParentOfChildNonStrict
{
    enum { value = CanConvert<const U*, const T*>::value && !STATIC_EQUAL(T, void) };
};

template<typename T, typename U>
struct IsParentOfChild
{
    enum { value = IsParentOfChildNonStrict<T, U>::value && !STATIC_EQUAL(const T, const U) };
};

template<typename T, typename U>
struct CanDynamicCast
{
    enum { value = CanConvert<const U*, const T*>::value };
};

template<class Types>
struct MaxTypeSelect
{
    enum { isMax = (int)sizeof(typename Types::Head) > MaxTypeSelect<typename Types::Tail>::max };
    enum { max = isMax ? (int)sizeof(typename Types::Head) : MaxTypeSelect<typename Types::Tail>::max };
    typedef STATIC_IF(isMax, typename Types::Head, typename MaxTypeSelect<typename Types::Tail>::Result) Result;
};

template<>
struct MaxTypeSelect<KDE::NullType>
{
    enum { isMax = true };
    enum { max = -1 };
    typedef KDE::NullType Result;
};

template<class Types>
struct MinTypeSelect
{
    enum { isMin = sizeof(typename Types::Head) < MinTypeSelect<typename Types::Tail>::min };
    enum { min = isMin ? (int)sizeof(typename Types::Head) : (int)MinTypeSelect<typename Types::Tail>::min };
    typedef STATIC_IF(isMin, typename Types::Head, typename MinTypeSelect<typename Types::Tail>::Result) Result;
};

template<>
struct MinTypeSelect<KDE::NullType>
{
    enum { isMin = true };
    enum { min = 1000000 };
    typedef KDE::NullType Result;
};

typedef K_TYPELIST_6(signed char, wchar_t, signed short, signed int, signed long, signed long long) SignedInts;
typedef K_TYPELIST_6(unsigned char, wchar_t, unsigned short, unsigned int, unsigned long, unsigned long long) UnsignedInts;
typedef K_TYPELIST_3(float, double, long double) Floats;
typedef TypeSelect<SignedInts, 1>::Result int8;
typedef TypeSelect<SignedInts, 2>::Result int16;
typedef TypeSelect<SignedInts, 4>::Result int32;
typedef TypeSelect<SignedInts, 8>::Result int64;
typedef TypeSelect<UnsignedInts, 1>::Result uint8;
typedef TypeSelect<UnsignedInts, 2>::Result uint16;
typedef TypeSelect<UnsignedInts, 4>::Result uint32;
typedef TypeSelect<UnsignedInts, 8>::Result uint64;
typedef TypeSelect<Floats, 4>::Result float32;
typedef TypeSelect<Floats, 8>::Result float64;
typedef TypeSelect<Floats, 12>::Result float96;
typedef MaxTypeSelect<SignedInts>::Result maxint;
typedef MinTypeSelect<SignedInts>::Result minint;
typedef MaxTypeSelect<UnsignedInts>::Result maxuint;
typedef MinTypeSelect<UnsignedInts>::Result minuint;
typedef MaxTypeSelect<Floats>::Result maxfloat;
typedef MinTypeSelect<Floats>::Result minfloat;

#define TTQ_DECL(Name)         \
    template<typename T>       \
    struct Is ##Name           \
    {                          \
        enum { value = false };\
    };                         \
    template<typename T>       \
    struct Strip ##Name        \
    {                          \
        typedef T Result;      \
    };
#define TTQ_SPECIALIZATION(Name, Check)\
    template<typename T>               \
    struct Is ##Name<Check>            \
    {                                  \
        enum { value = true };         \
    };                                 \
    template<typename T>               \
    struct Strip ##Name<Check>         \
    {                                  \
        typedef T Result;              \
    };
#define TTQ_ALIAS(Name)                            \
    typedef typename Modifiers::Strip ##Name<T>::Result Strip ##Name;    \
    enum { is ##Name = Modifiers::Is ##Name<T>::value };

template<typename T>
class TypeTraits;

namespace Modifiers
{
    TTQ_DECL(Pointer)
    TTQ_DECL(Reference)
    TTQ_DECL(Const)
    TTQ_DECL(Volatile)
    TTQ_DECL(ConstReference)
    template<typename T>
    struct StripAll
    {
        typedef typename StripConst<typename StripVolatile<typename StripReference<T>::Result>::Result>::Result Result;
    };
    template<typename T>
    struct ParameterType
    {
        typedef typename TypeTraits<T>::ParameterType Result;
    };
    template<typename T>
    struct PointerType
    {
        typedef T* Result;
    };
    template<typename T>
    struct ReferenceType
    {
        typedef T& Result;
    };
    template<typename T>
    struct ReferenceType<T&>
    {
        typedef T& Result;
    };
    template<typename T>
    struct ConstReferenceType
    {
        typedef const T& Result;
    };

    template<typename T>
    struct ConstReferenceType<T&>
    {
        typedef const T& Result;
    };
};

template<typename T>
struct TypeListCheck
{
    enum { value = false };
};

template<typename Head, class Tail>
struct TypeListCheck<KTypeList<Head, Tail> >
{
    enum { value = TypeTraits<Tail>::isTypeList };
};

template<>
struct TypeListCheck<KDE::NullType>
{
    enum { value = true };
};

template<typename T>
class TypeTraits
{
public:
    TTQ_ALIAS(Volatile)
    TTQ_ALIAS(Pointer)
    TTQ_ALIAS(Reference)
    TTQ_ALIAS(Const)
    TTQ_ALIAS(ConstReference)
    enum { isSignedInt = KTypeListContains<SignedInts, T>::value };
    enum { isUnsignedInt = KTypeListContains<UnsignedInts, T>::value };
    enum { isInteger = isSignedInt || isUnsignedInt };
    enum { isFloat = KTypeListContains<Floats, T>::value };
    enum { isBoolean = STATIC_EQUAL(T, bool) };
    enum { isNumeric = isInteger || isFloat || isBoolean };
    enum { isFundamental = isPointer || isNumeric };
    enum { isQObject = IsParentOfChildNonStrict<QObject, T>::value };
    enum { isQWidget = IsParentOfChildNonStrict<QWidget, T>::value };
    typedef typename Modifiers::ReferenceType<T>::Result ReferenceType;
    typedef STATIC_IF(isFundamental || isReference, T, typename Modifiers::ReferenceType<const T>::Result) ParameterType;
    enum { isTypeList = TypeListCheck<T>::value };
    typedef typename Modifiers::StripAll<T>::Result StripAll;
};

namespace Modifiers
{
TTQ_SPECIALIZATION(Pointer, T*)
TTQ_SPECIALIZATION(Reference, T&)
TTQ_SPECIALIZATION(Const, const T)
TTQ_SPECIALIZATION(Volatile, volatile T)
TTQ_SPECIALIZATION(ConstReference, const T&)
}

template<class Types>
struct StrippedList
{
    typedef STATIC_FOREACH(Types, Modifiers::StripAll) Result;
};

template<class Types>
struct ParameterList
{
    typedef STATIC_FOREACH(Types, Modifiers::ParameterType) Result;
};

template<class Types>
struct PointerList
{
    typedef STATIC_FOREACH(Types, Modifiers::PointerType) Result;
};

template<class Types>
struct ReferenceList
{
    typedef STATIC_FOREACH(Types, Modifiers::ReferenceType) Result;
};

}

#undef TTQ_SPECIALIZATION
#undef TTQ_DECL
#undef TTQ_ALIAS

#undef NC

#endif

