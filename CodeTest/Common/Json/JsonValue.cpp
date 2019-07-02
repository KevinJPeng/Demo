/*
	Copyright 3h6a@163.com all rights reserved
*/
#include "pch.h"

#include "JsonValue.h"

#include "JsonSimple.h"
#include "JsonArray.h"
#include "JsonObject.h"

JsonValue::JsonValue(IJsonValue* pValue) : m_pValue(pValue)
{
}

JsonValue::JsonValue(BYTE* pBuffer) : m_pValue(pBuffer)
{
}

JString JsonValue::ToString() const
{
	if (!m_pValue.IsNull())
	{
		return m_pValue->ToString();
	}
	else
	{
		return JsonNull().ToString();
	}
}

JString JsonValue::GetType() const
{
	if (!m_pValue.IsNull())
	{
		return m_pValue->GetType();
	}
	else
	{
		return JsonNull().GetType();
	}
}

JString JsonValue::GetString() const
{
	if (!m_pValue.IsNull())
	{
		return m_pValue->GetString();
	}
	else
	{
		return JsonNull().GetString();
	}
}

int JsonValue::GetInteger() const
{
	if (!m_pValue.IsNull())
	{
		return m_pValue->GetInteger();
	}
	else
	{
		return JsonNull().GetInteger();
	}
}

bool JsonValue::GetBoolean() const
{
	if (!m_pValue.IsNull())
	{
		return m_pValue->GetBoolean();
	}
	else
	{
		return JsonNull().GetBoolean();
	}
}

//

double JsonValue::GetDouble() const
{
	double result = 0.0;
	JsonNumber* pNumber = dynamic_cast<JsonNumber*>((IJsonValue*)m_pValue);
	if (pNumber != NULL)
	{
		result = *pNumber;
	}
	return result;
}

IJsonValue* JsonValue::GetPointer() const
{
	return m_pValue;
}

BYTE* JsonValue::GetBuffer() const
{
	return m_pValue.GetBuffer();
}

int JsonValue::GetLength() const
{
	int result = 0;

	JsonArray* pArray = dynamic_cast<JsonArray*>((IJsonValue*)m_pValue);
	if (pArray != NULL)
	{
		result = pArray->GetLength();
	}
	else 
	{
		JsonObject* pObject = dynamic_cast<JsonObject*>((IJsonValue*)m_pValue);
		if (pObject != NULL)
		{
			result = pObject->GetLength();
		}
	}

	return result;
}

JsonValue JsonValue::operator[](int Index) const
{
	BYTE* result = NULL;

	JsonArray* pArray = dynamic_cast<JsonArray*>((IJsonValue*)m_pValue);
	if (pArray != NULL)
	{
		result = pArray->GetBuffer(Index);
	}
	else 
	{
		JsonObject* pObject = dynamic_cast<JsonObject*>((IJsonValue*)m_pValue);
		if (pObject != NULL)
		{
			result = pObject->GetBuffer(Index);
		}
	}

	return JsonValue(result);
}

JString JsonValue::GetName(int Index) const
{
	JString result;

	JsonObject* pObject = dynamic_cast<JsonObject*>((IJsonValue*)m_pValue);
	if (pObject != NULL)
	{
		result = pObject->GetName(Index);
	}

	return result;
}

bool JsonValue::Contain(const char* Name) const
{
	bool result = false;

	JsonObject* pObject = dynamic_cast<JsonObject*>((IJsonValue*)m_pValue);
	if (pObject != NULL)
	{
		result = pObject->IndexOf(Name) > 0;
	}

	return result;
}

JsonValue JsonValue::operator[](const char* Name) const
{
	BYTE* result = NULL;

	JsonObject* pObject = dynamic_cast<JsonObject*>((IJsonValue*)m_pValue);
	if (pObject != NULL)
	{
		result = pObject->GetBuffer(Name);
	}

	return JsonValue(result);
}
