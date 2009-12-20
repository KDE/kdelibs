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

/**
 * @File ktypelistutils.h
 * Various templates to handle typelists in meta-programming.
 */

#ifndef KTYPELISTUTILS_H
#define KTYPELISTUTILS_H

#include "ktypelist.h"
#include <cstdlib>

// NO CODE (ignore some preprocessor stuff)
#define NC(...) __VA_ARGS__

/**
 * Push Type.
 */
template<class Types, typename Type>
struct KTypeListPush
{
    typedef KTypeList<typename Types::Head, typename KTypeListPush<typename Types::Tail, Type>::Result> Result;
};

template<typename Type>
struct KTypeListPush<KDE::NullType, Type>
{
    typedef K_TYPELIST_1(Type) Result;
};

/**
 * Pop last element.
 */
template<class Types>
struct KTypeListPop
{
    typedef KTypeList<typename Types::Head, typename KTypeListPop<typename Types::Tail>::Result> Result;
};

template<class Type>
struct KTypeListPop<K_TYPELIST_1(Type)>
{
    typedef KDE::NullType Result;
};

/**
 * Get last element.
 */
template<class Types>
struct KTypeListEnd
{
    typedef typename KTypeListEnd<typename Types::Tail>::Result Result;
};

template<class Type>
struct KTypeListEnd<K_TYPELIST_1(Type)>
{
    typedef Type Result;
};

/**
 * Concatenates two typelists.
 */
template<class List1, class List2>
struct KTypeListAppend
{
    typedef KTypeList<typename List1::Head, typename KTypeListAppend<typename List1::Tail, List2>::Result> Result;
};

template<class List2>
struct KTypeListAppend<KDE::NullType, List2>
{
    typedef List2 Result;
};

/**
 * @return A typelist with size(list) == size(Types) and list[i] == Replace[i] (for i in 0..min(size(Types) - 1, size(Replace) - 1))
 */
template<class Types, class Replace>
struct KTypeListReplaceBegin
{
    typedef KTypeList<typename Replace::Head, typename KTypeListReplaceBegin<typename Types::Tail, typename Replace::Tail>::Result> Result;
};

template<class Types>
struct KTypeListReplaceBegin<Types, KDE::NullType>
{
    typedef Types Result;
};

template<class Replace>
struct KTypeListReplaceBegin<KDE::NullType, Replace>
{
    typedef KDE::NullType Result;
};

/**
 * Index-Operator
 */
template<class Types, std::size_t pos>
struct KTypeListAt
{
    typedef typename KTypeListAt<
        typename Types::Tail, pos - 1>::Result
        Result;
};

template<class Types>
struct KTypeListAt<Types, 0>
{
    typedef typename Types::Head Result;
};

template<std::size_t pos>
struct KTypeListAt<KDE::NullType, pos>
{
};

template<>
struct KTypeListAt<KDE::NullType, 0>
{
};

/**
 * Like KTypeListAt. If pos >= size(Types): Default..
 */
template<class Types, std::size_t pos, typename Default>
struct KTypeListAtWithDefault
{
    typedef typename KTypeListAt<
            typename Types::Tail, pos - 1>::Result
        Result;
};

template<class Types, typename Default>
struct KTypeListAtWithDefault<Types, 0, Default>
{
    typedef typename Types::Head Result;
};

template<std::size_t pos, typename Default>
struct KTypeListAtWithDefault<KDE::NullType, pos, Default>
{
    typedef Default Result;
};

template<typename Default>
struct KTypeListAtWithDefault<KDE::NullType, 0, Default>
{
    typedef Default Result;
};

/**
 * @return A typelist. (list[i] == Manip\<Types[i]>::Result, for all i)
 */
template<class Types, template<typename> class Manip>
class KTypeListForeach
{
public:
    typedef KTypeList<typename Manip<typename Types::Head>::Result, typename KTypeListForeach<typename Types::Tail, Manip>::Result> Result;
};

template<template<typename> class Manip>
struct KTypeListForeach<KDE::NullType, Manip>
{
    typedef KDE::NullType Result;
};

/**
 * @return true if Types contains Type.
 */
template<class Types, typename Type>
struct KTypeListContains
{
    enum { value = KTypeListContains<typename Types::Tail, Type>::value };
};

template<class Tail, typename Type>
struct KTypeListContains<KTypeList<Type, Tail>, Type>
{
    enum { value = true };
};

template<typename Type>
struct KTypeListContains<KDE::NullType, Type>
{
    enum { value = false };
};

namespace KDE
{
/**
 * @return true if T == U.
 */
template<typename T, typename U>
struct SameTypes
{
    enum { value = false };
};

template<typename T>
struct SameTypes<T, T>
{
    enum { value = true };
};
}

/**
 * @return true if type1 == type2.
 */
#define STATIC_EQUAL(type1, type2) KDE::SameTypes<type1, type2>::value

/**
 * Apply KTypeListForeach.
 */
#define STATIC_FOREACH(list, modifier) typename KTypeListForeach<list, modifier>::Result

namespace KDE
{
template<bool cond, typename T, typename U>
struct IfThenElse
{
    typedef T Result;
};

template<typename T, typename U>
struct IfThenElse<false, T, U>
{
    typedef U Result;
};

template<typename First, typename Second, typename Then, typename Else>
struct IfEqualThenElse
{
    typedef Else Result;
};

template<typename FS, typename Then, typename Else>
struct IfEqualThenElse<FS, FS, Then, Else>
{
    typedef Then Result;
};
}

/**
 * If expr: T, else: U.
 */
#define STATIC_IF(expr, T, U) typename KDE::IfThenElse<expr, T, U>::Result

/**
 * If T == U: V, else: W.
 */
#define STATIC_IF_EQUAL(T, U, V, W) typename KDE::IfEqualThenElse<T, U, V, W>::Result

template<class List, template<typename, typename> class Folder, typename Start = KDE::NullType>
struct KTypeListFold
{
    typedef typename KTypeListFold<typename List::Tail, Folder, typename Folder<Start, typename List::Head>::Result>::Result Result;
};

template<template<typename, typename> class Folder, typename Start>
struct KTypeListFold<KDE::NullType, Folder, Start>
{
    typedef Start Result;
};

/**
 * "Folds" the typelist.
 * If size(List) == 0: Start
 * Else: Fold(List::Tail, Modifier, Modifier(Start, List::Head))
 */
#define STATIC_FOLD(List, Modifier, Start) typename KTypeListFold<List, Modifier, Start>::Result

/**
 * @return Typelist in reverse order.
 */
template<class List>
struct KTypeListRevert
{
private:
    template<typename Value, typename Current>
    struct Impl
    {
        typedef KTypeList<Current, Value> Result;
    };
public:
    typedef STATIC_FOLD(List, Impl, KDE::NullType) Result;
};

template<class List, typename Pivot, template<typename A, typename B> class Comparator>
class KTypeListAfterPivot
{
    template<typename Value, typename Current>
    struct Impl
    {
        typedef STATIC_IF(NC(Comparator<Pivot, Current>::result), NC(KTypeList<Current, Value>), Value) Result;
    };
public:
    typedef STATIC_FOLD(List, Impl, KDE::NullType) Result;
};

template<class List, typename Pivot, template<typename A, typename B> class Comparator>
class KTypeListBeforePivot
{
    template<typename Value, typename Current>
    struct Impl
    {
        typedef STATIC_IF(NC(Comparator<Pivot, Current>::result), Value, NC(KTypeList<Current, Value>)) Result;
    };
public:
    typedef STATIC_FOLD(List, Impl, KDE::NullType) Result;
};

/**
 * Slow Quick-Sort. ;)
 */
template<class List, template<typename A, typename B> class Comparator>
struct KTypeListSort
{
    typedef typename KTypeListAppend<
                        typename KTypeListSort<
                            typename KTypeListBeforePivot<
                                typename List::Tail,
                                typename List::Head,
                                Comparator>::Result,
                            Comparator>::Result,
                        KTypeList<
                            typename List::Head,
                            typename KTypeListSort<
                                typename KTypeListAfterPivot<
                                    typename List::Tail,
                                    typename List::Head,
                                    Comparator>::Result,
                                Comparator>::Result
                            >
                        >::Result Result;
};

#undef NC

#endif
