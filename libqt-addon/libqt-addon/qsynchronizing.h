#ifndef QSYNCHRONIZING_H
#define QSYNCHRONIZING_H

#include <qobject.h>

template<class Arg1> void synchronize(void (*)(Arg1), Arg1);

class QSynchronizing
{
public:
  typedef void (* SimpleVoidTarget)(void);

  struct ArgumentsBase
  {
    int argumentCount;

    inline ArgumentsBase(int argcount = 0)
      : argumentCount(argcount)
    { }
  };

protected:
  struct VoidArguments: public ArgumentsBase
  {
    typedef SimpleVoidTarget TargetType;
    TargetType target;

    inline VoidArguments(TargetType _target) :
      ArgumentsBase(0), target(_target)
    { }
  };
 
  template<class Arg1> struct SimpleArguments : public ArgumentsBase
  {
    typedef void (*TargetType)(Arg1);
    TargetType target;
    Arg1& arg1;

    inline SimpleArguments(TargetType _target, Arg1& _arg1) :
      ArgumentsBase(1), target(_target), arg1(_arg1)
    { }
  };

  template<class Arg1> struct SimpleRefArguments : public ArgumentsBase
  {
    typedef void (*TargetType)(Arg1);
    TargetType target;
    Arg1& arg1;

    inline SimpleRefArguments(TargetType _target, Arg1& _arg1) :
      ArgumentsBase(1), target(_target), arg1(_arg1)
    { }
  };

  template<class Object> struct ObjectVoidArguments : public ArgumentsBase
  {
    typedef void (Object::* TargetType)(void);
    TargetType target;
    Object *object;

    inline ObjectVoidArguments(TargetType _target, Object* obj) :
      ArgumentsBase(0), target(_target), object(obj)
    { }
  };

  template<class Object, class Arg1> struct ObjectArguments : public ArgumentsBase
  {
    typedef void (Object:: *TargetType)(Arg1);
    TargetType target;
    Object *object;
    Arg1& arg1;

    inline ObjectArguments(TargetType _target, Object* obj, Arg1& _arg1) :
      ArgumentsBase(1), target(_target), object(obj), arg1(_arg1)
    { }
  };

  template<class Object, class Arg1> struct ObjectRefArguments : public ArgumentsBase
  {
    typedef void (Object:: *TargetType)(Arg1&);
    TargetType target;
    Object *object;
    Arg1& arg1;

    inline ObjectRefArguments(TargetType _target, Object* obj, Arg1& _arg1) :
      ArgumentsBase(1), target(_target), object(obj), arg1(_arg1)
    { }
  };

  static void helperCaller(VoidArguments* args)
  {
    (args->target)();
  }

  template<class Arg1> static void helperCaller(SimpleArguments<Arg1>* args)
  {
    (args->target)(args->arg1);
  }

  template<class Arg1> static void helperCaller(SimpleRefArguments<Arg1>* args)
  {
    (args->target)(args->arg1);
  }

  template<class Object> static void helperCaller(ObjectVoidArguments<Object>* args)
  {
    ((args->object)->*(args->target)) ();
  }

  template<class Arg1, class Object> static void helperCaller(ObjectArguments<Object, Arg1>* args)
  {
    ((args->object)->*(args->target)) (args->arg1);
  }

  template<class Arg1, class Object> static void helperCaller(ObjectRefArguments<Object, Arg1>* args)
  {
    ((args->object)->*(args->target)) (args->arg1);
  }

  static void doSynchronize(void (*target)(ArgumentsBase*), ArgumentsBase &arguments);

public:
  static inline void synchronize(SimpleVoidTarget target)
  {
    VoidArguments arguments(target);

    void (*helper)(VoidArguments*) = helperCaller;
    doSynchronize((void (*)(ArgumentsBase*)) helper, arguments);
  }

  template<typename Arg1> 
  static inline void synchronize(void (*target)(Arg1), Arg1 arg1)
  {
    SimpleArguments<Arg1> arguments(target, arg1);

    void (*helper)(SimpleArguments<Arg1>*) = helperCaller<Arg1>;
    doSynchronize((void (*)(ArgumentsBase*))(helper), arguments);
  }

  template<typename Arg1> 
  static inline void synchronize(void (*target)(Arg1), Arg1& arg1)
  {
    SimpleRefArguments<Arg1> arguments(target, arg1);

    void (*helper)(SimpleRefArguments<Arg1>*) = helperCaller<Arg1>;
    doSynchronize((void (*)(ArgumentsBase*))(helper), arguments);
  }

  template<class Object> 
  static inline void synchronize(Object* object, void (Object::*target)())
  {
    ObjectVoidArguments<Object> arguments(target, object);

    void (*helper)(ObjectVoidArguments<Object>*) = helperCaller<Object>;
    doSynchronize((void (*)(ArgumentsBase*))helper, arguments);
  }

  template<class Object, class Arg1> 
  static inline void synchronize(Object *object, void (Object::*target)(Arg1), Arg1 arg1)
  {
    ObjectArguments<Object, Arg1> arguments(target, object, arg1);

    void (*helper)(ObjectArguments<Object, Arg1>*) = helperCaller<Arg1>;
    doSynchronize((void (*)(ArgumentsBase*))helper, arguments);
  }

  template<class Object, class Arg1> 
  static inline void synchronize(Object *object, void (Object::*target)(Arg1&), Arg1& arg1)
  {
    ObjectRefArguments<Object, Arg1> arguments(target, object, arg1);

    void (*helper)(ObjectRefArguments<Object, Arg1>*) = helperCaller<Arg1>;
    doSynchronize((void (*)(ArgumentsBase*))helper, arguments);
  }

  template<class Object>
  static inline void synchronize(Object& obj, void (Object::* target)(void))
  {
    synchronize(&obj, target);
  }

  template<class Object, typename Arg1>
  static inline void synchronize(Object& obj, void (Object::* target)(Arg1), Arg1 arg1)
  {
    synchronize(&obj, target, arg1);
  }

  inline void synchronize(void (QSynchronizing::* target)(void))
  {
    synchronize(this, target);
  }

  template<typename Arg1> 
  inline void synchronize(void (QSynchronizing::* target)(Arg1), Arg1 arg1)
  {
    synchronize(this, target, arg1);
  }

};


#endif
