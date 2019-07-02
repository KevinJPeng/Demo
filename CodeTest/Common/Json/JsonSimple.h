/*
	Copyright 3h6a@163.com all rights reserved

	Simple JSON classes
*/
#ifndef JSON_SIMPLE_H
#define JSON_SIMPLE_H

#include "IJsonValue.h"
#include <stdlib.h>

//----------------------------------------------------------------------------------------------------------------------
// JsonNull

class JsonNull : public IJsonValue
{
public:
	JsonNull()
	{
	}

	virtual JString ToString() const
	{
		return GetString();
	}

	virtual JString GetType() const
	{
		return JString("null");
	}

	virtual JString GetString() const
	{
		return JString("null");
	}

	virtual int GetInteger() const
	{
		return 0;
	}

	virtual bool GetBoolean() const
	{
		return false;
	}
};

//----------------------------------------------------------------------------------------------------------------------
// JsonBoolean

class JsonBoolean : public IJsonValue
{
public:
	JsonBoolean(bool Value) : m_Value(Value)
	{
	}

	virtual JString ToString() const
	{
		return GetString();
	}

	virtual JString GetType() const
	{
		return JString("boolean");
	}

	virtual JString GetString() const
	{
		if (m_Value)
		{
			return JString("true");
		}
		else
		{
			return JString("false");
		}
	}

	virtual int GetInteger() const
	{
		if (m_Value)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	virtual bool GetBoolean() const
	{
		return m_Value;
	}

private:
	bool m_Value;
};

//----------------------------------------------------------------------------------------------------------------------
// JsonNumber

class JsonNumber : public IJsonValue
{
public:
	JsonNumber(const char* Value) : m_Value(Value)
	{
	}

	JsonNumber(int Value)
	{
		m_Value.Format("%d", Value);
	}

	JsonNumber(double Value)
	{
		m_Value.Format("%g", Value);
	}

	virtual JString ToString() const
	{
		return GetString();
	}

	virtual JString GetType() const
	{
		return JString("number");
	}

	virtual JString GetString() const
	{
		return m_Value;
	}

	virtual int GetInteger() const
	{
		return atol(m_Value);
	}

	virtual bool GetBoolean() const
	{
		return GetInteger() != 0;
	}

	virtual double GeDouble() const
	{
		return atof(m_Value);
	}

public:
	operator int()
	{
		return GetInteger();
	}

	operator double()
	{
		return atof(m_Value);
	}

private:
	JString m_Value;
};

//----------------------------------------------------------------------------------------------------------------------
// JsonString

class JsonString : public IJsonValue
{
public:
	JsonString(const char* pString) : m_Value(pString)
	{
	}

	virtual JString ToString() const
	{
		return JString().Format("\"%s\"", (char*)GetString());
	}

	virtual JString GetType() const
	{
		return JString("string");
	}

	virtual JString GetString() const
	{
		return m_Value;
	}

	virtual int GetInteger() const
	{
		return atol(m_Value);
	}

	virtual bool GetBoolean() const
	{
		return GetInteger() != 0;
	}

private:
	JString m_Value;
};

#endif //End of JSON_SIMPLE_H
