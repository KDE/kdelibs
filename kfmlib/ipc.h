#ifndef ipc_h
#define ipc_h

struct stringList
{
    int elements;
    char **list;
};

struct intList
{
    int elements;
    int *list;
};

struct doubleList
{
    int elements;
    double *list;
};

struct charList
{
    int elements;
    char *list;
};

#define boolList charList
#define write_bool write_char
#define read_bool read_char
#define free_bool free_char
#define free_boolList free_charList
#define len_bool len_char
#define len_boolList len_boolList

void write_int( int _fd, int _value );
void write_double( int _fd, double _value );
void write_char( int _fd, char _value );
void write_string( int _fd, const char* _value );
void write_intList( int _fd, intList* _list );
void write_doubleList( int _fd, doubleList* _list );
void write_charList( int _fd, charList* _list );
void write_stringList( int _fd, stringList* _list );
char* read_string( char *_data, int &_pos, int _len );
int read_int( char *_data, int &_pos, int _len );
char read_char( char *_data, int &_pos, int _len );
double read_double( char *_data, int &_pos, int _len );
void read_stringList( char *_data, int &_pos, int _len, stringList *_list );
void read_intList( char *_data, int &_pos, int _len, intList *_list );
void read_doubleList( char *_data, int &_pos, int _len, doubleList *_list );
void read_charList( char *_data, int &_pos, int _len, charList *_list );

#define free_int( x ); ;
#define free_char( x ); ;
#define free_double( x ); ;

void free_string( char *_str );
void free_stringList( stringList *_list );
void free_intList( intList *_list );
void free_doubleList( doubleList *_list );
void free_charList( charList *_list );

int len_int( int _value );
int len_double( double _value );
int len_char( char _value );
int len_string( const char *_str );
int len_stringList( stringList *_list );
int len_intList( intList *_list );
int len_doubleList( doubleList *_list );
int len_charList( charList *_list );

#endif
