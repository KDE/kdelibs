#ifndef bison2cpp_h
#define bison2cpp_h

#define OP_ASSIGN 1

#define OP_MUL 20
#define OP_ADD 21
#define OP_DIV 22
#define OP_SUB 23
#define OP_SL 24
#define OP_SR 25
#define OP_BAND 26
#define OP_BOR 27
#define OP_BXOR 28

#define OP_AND 30
#define OP_OR 31

#define OP_EQ 40
#define OP_NEQ 41
#define OP_LT 42
#define OP_GT 43
#define OP_LEQ 44
#define OP_GEQ 45

#ifndef ____CPP____

void* newJSInteger( int _value );
void* newJSBinaryOperator( int _op, void* _left, void *_right );
void* newJSAssignment( int _op, void* _left, void *_right );
void* newJSIdentifier( char* _name );
void* newJSStatement( void *_code, void *_next_code );
void* newJSFunction( const char *_name, void *_param, void *_code );
void* newJSParameter( const char *_name, void *_next );
void* newJSArgument( void *_code, void *_next );
void* newJSFunctionCall( void *_function, void *_arguments );
void* newJSConstructorCall( void *_function, void *_arguments );
void* newJSMember( void *_obj, char* _member );
void* newJSArrayAccess( void *_array, void *_index );
void* newJSString( char *_string );
void* newJSBool( char );
void* newJSFloat( double );
void* newJSThis();
void* newJSNull();

void jsAppendCode( void *_code );
void mainParse( const char *_code );

#else

extern "C" void mainParse( const char *_code );

#endif


#endif
