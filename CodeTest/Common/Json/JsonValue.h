/*
	Copyright 3h6a@163.com all rights reserved

	Wrapper class of IJsonValue
*/
#ifndef JSON_VALUE_H
#define JSON_VALUE_H

#include "IJsonValue.h"
#include "JPointer.h"

class JsonValue : public IJsonValue
{
	friend class JsonObjectBuilder;
	friend class JsonArrayBuilder;

public:
	JsonValue(IJsonValue* pValue);
	JsonValue(BYTE* pBuffer);

public:
	virtual JString ToString() const;
	virtual JString GetType() const;
	virtual JString GetString() const;
	virtual int     GetInteger() const;
	virtual bool    GetBoolean() const;

public:
	double GetDouble() const;

	IJsonValue* GetPointer() const;
	BYTE* GetBuffer() const;

	int GetLength() const;
	JsonValue operator[](int Index) const;

	JString GetName(int Index) const;

	bool Contain(const char* Name) const;
	JsonValue operator[](const char* Name) const;

protected:
	JPointer<IJsonValue> m_pValue;
};

#endif //End of JSON_VALUE_H
