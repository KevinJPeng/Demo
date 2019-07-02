/*
	Copyright 3h6a@163.com all rights reserved
*/
#include "pch.h"

#include "JsonBuilder.h"

//----------------------------------------------------------------------------------------------------------------------
// JsonObjectValue

JsonObjectBuilder::JsonObjectBuilder() : m_pObject(new JsonObject)
{
}

JsonObjectBuilder& JsonObjectBuilder::Append(const char* pName, bool Value)
{
	m_pObject->Append(pName, new JsonBoolean(Value));

	return *this;
}

JsonObjectBuilder& JsonObjectBuilder::Append(const char* pName, int Value)
{
	m_pObject->Append(pName, new JsonNumber(Value));

	return *this;
}

JsonObjectBuilder& JsonObjectBuilder::Append(const char* pName, double Value)
{
	m_pObject->Append(pName, new JsonNumber(Value));

	return *this;
}

JsonObjectBuilder& JsonObjectBuilder::Append(const char* pName, const char* pValue)
{
	m_pObject->Append(pName, new JsonString(pValue));

	return *this;
}

JsonObjectBuilder& JsonObjectBuilder::Append(const char* pName, const JsonValue& Value)
{
	m_pObject->Append(pName, Value.GetBuffer());

	return *this;
}

JsonValue JsonObjectBuilder::Build()
{
	return JsonValue(m_pObject.GetBuffer());
}

//----------------------------------------------------------------------------------------------------------------------
// JsonArrayValue

JsonArrayBuilder::JsonArrayBuilder() : m_pArray(new JsonArray)
{
}

JsonArrayBuilder& JsonArrayBuilder::Append(bool Value)
{
	m_pArray->Append(new JsonBoolean(Value));

	return *this;
}

JsonArrayBuilder& JsonArrayBuilder::Append(int Value)
{
	m_pArray->Append(new JsonNumber(Value));

	return *this;
}

JsonArrayBuilder& JsonArrayBuilder::Append(double Value)
{
	m_pArray->Append(new JsonNumber(Value));

	return *this;
}

JsonArrayBuilder& JsonArrayBuilder::Append(const char* pValue)
{
	m_pArray->Append(new JsonString(pValue));

	return *this;
}

JsonArrayBuilder& JsonArrayBuilder::Append(const JsonValue& Value)
{
	m_pArray->Append(Value.GetBuffer());

	return *this;
}

JsonValue JsonArrayBuilder::Build()
{
	return JsonValue(m_pArray.GetBuffer());
}