// XXX Automatically generated. DO NOT EDIT! XXX //

public:
V_ParamValue();
V_ParamValue(const V_ParamValue&);
V_ParamValue(const QCString&);
V_ParamValue & operator = (V_ParamValue&);
V_ParamValue & operator = (const QCString&);
bool operator ==(V_ParamValue&);
bool operator !=(V_ParamValue& x) {return !(*this==x);}
bool operator ==(const QCString& s) {V_ParamValue a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~V_ParamValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();

// End of automatically generated code           //
