// XXX Automatically generated. DO NOT EDIT! XXX //

public:
V_Name();
V_Name(const V_Name&);
V_Name(const QCString&);
V_Name & operator = (V_Name&);
V_Name & operator = (const QCString&);
bool operator ==(V_Name&);
bool operator !=(V_Name& x) {return !(*this==x);}
bool operator ==(const QCString& s) {V_Name a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~V_Name();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();

// End of automatically generated code           //
