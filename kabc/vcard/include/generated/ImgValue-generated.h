// XXX Automatically generated. DO NOT EDIT! XXX //

public:
ImgValue();
ImgValue(const ImgValue&);
ImgValue(const QCString&);
ImgValue & operator = (ImgValue&);
ImgValue & operator = (const QCString&);
bool operator ==(ImgValue&);
bool operator !=(ImgValue& x) {return !(*this==x);}
bool operator ==(const QCString& s) {ImgValue a(s);return(*this==a);} 
bool operator != (const QCString& s) {return !(*this == s);}

virtual ~ImgValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
virtual const char * className() const { return "ImgValue"; }

// End of automatically generated code           //
