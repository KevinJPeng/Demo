/*
	Copyright 3h6a@163.com all rights reserved

	JsonBuilder class
*/
#ifndef JSON_BUILDER_H
#define JSON_BUILDER_H

#include "JsonValue.h"

#include "JsonArray.h"
#include "JsonObject.h"

//----------------------------------------------------------------------------------------------------------------------
// JsonObjectBuilder

class JsonObjectBuilder
{
public:
	JsonObjectBuilder();

public:
	JsonObjectBuilder& Append(const char* pName, bool Value);
	JsonObjectBuilder& Append(const char* pName, int Value);
	JsonObjectBuilder& Append(const char* pName, double Value);
	JsonObjectBuilder& Append(const char* pName, const char* pValue);
	JsonObjectBuilder& Append(const char* pName, const JsonValue& Value);

	JsonValue Build();

private:
	JPointer<JsonObject> m_pObject;
};

//----------------------------------------------------------------------------------------------------------------------
// JsonArrayBuilder

class JsonArrayBuilder
{
public:
	JsonArrayBuilder();

public:
	JsonArrayBuilder& Append(bool Value);
	JsonArrayBuilder& Append(int Value);
	JsonArrayBuilder& Append(double Value);
	JsonArrayBuilder& Append(const char* pValue);
	JsonArrayBuilder& Append(const JsonValue& Value);

	JsonValue Build();

private:
	JPointer<JsonArray> m_pArray;
};

#endif //End of JSON_BUILDER_H
