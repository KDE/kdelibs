#include "value.h"

class FloatValue_impl : virtual public FloatValue_skel {
private:
	float _value;
public:
	FloatValue_impl() : _value(0) { };
	void value(float newvalue)
	{
		if(newvalue != _value)
		{
			_value = newvalue;
			value_changed(_value);
		}
	}
	float value() { return _value; }
};

REGISTER_IMPLEMENTATION(FloatValue_impl);

class StringValue_impl : virtual public StringValue_skel {
private:
	string _value;
public:
	StringValue_impl() { };
	void value(const string& newvalue)
	{
		if(newvalue != _value)
		{
			_value = newvalue;
			value_changed(_value);
		}
	}
	string value() { return _value; }
};

REGISTER_IMPLEMENTATION(StringValue_impl);

class FloatSender_impl : virtual public FloatSender_skel {
public:
	void send(const vector<float>& data)
	{
		Arts::DataPacket<float> *packet = outstream.allocPacket(data.size());
		copy(data.begin(), data.end(), packet->contents);
		packet->size = data.size();
		packet->send();
	}
};

REGISTER_IMPLEMENTATION(FloatSender_impl);

class MyEnumValue_impl : virtual public MyEnumValue_skel {
private:
	MyEnum _value;
public:
	MyEnumValue_impl() : _value(meIdle) { };
	void value(MyEnum newvalue)
	{
		if(newvalue != _value)
		{
			_value = newvalue;
			value_changed(_value);
		}
	}
	MyEnum value() { return _value; }
};

REGISTER_IMPLEMENTATION(MyEnumValue_impl);


