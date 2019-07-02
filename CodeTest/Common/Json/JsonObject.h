/*
	Copyright 3h6a@163.com all rights reserved

	JsonObject class
*/
#ifndef JSON_OBJECT_H
#define JSON_OBJECT_H

#include "IJsonValue.h"
#include "JsonSimple.h"
#include "JPointer.h"
#include <vector>

class JsonObject : public IJsonValue
{
protected:
	class JsonPair : public IJsonValue
	{
		friend JsonObject;

	public:
		JsonPair(const char* pName, IJsonValue* pValue) : m_Name(pName), m_pValue(pValue)
		{
		}

		JsonPair(const char* pName, BYTE* pBuffer) : m_Name(pName), m_pValue(pBuffer)
		{
		}

		virtual JString ToString() const
		{
			JString value;
			if (m_pValue != NULL)
			{
				value = m_pValue->ToString();
			}
			else
			{
				value = JsonNull().ToString();
			}
			return JString().Format("\"%s\":%s", (char*)m_Name, (char*)value);
		}

		virtual JString GetType() const
		{
			JString result;
			if (m_pValue != NULL)
			{
				result = m_pValue->GetType();
			}
			else
			{
				result = JsonNull().GetType();
			}
			return result;
		}

		virtual JString GetString() const
		{
			JString result;
			if (m_pValue != NULL)
			{
				result = m_pValue->GetString();
			}
			else
			{
				result = JsonNull().GetString();
			}
			return result;
		}

		virtual int GetInteger() const
		{
			if (m_pValue != NULL)
			{
				return m_pValue->GetInteger();
			}
			else
			{
				return JsonNull().GetInteger();
			}
		}

		virtual bool GetBoolean() const
		{
			return GetInteger() != 0;
		}

	public:
		operator char*() const
		{
			return m_Name;
		}

		operator IJsonValue*() const
		{
			return m_pValue;
		}

		BYTE* GetBuffer() const
		{
			return m_pValue.GetBuffer();
		}

	private:
		JString              m_Name;
		JPointer<IJsonValue> m_pValue;
	};

public:
	JsonObject()
	{
	}

	virtual JString ToString() const
	{
		JString result("{");
		for (int i = 0, l = (int)m_List.size(); i < l; ++i)
		{
			if (i > 0) result += ", ";
			result += m_List[i].ToString();
		}
		result += "}";
		return result;
	}

	virtual JString GetType() const
	{
		return JString("object");
	}

	virtual JString GetString() const
	{
		return JString("object");
	}

	virtual int GetInteger() const
	{
		return GetLength();
	}

	virtual bool GetBoolean() const
	{
		return GetInteger() != 0;
	}

public:
	int GetLength() const
	{
		return m_List.size();
	}

	bool Contain(int Index) const
	{
		return 0 <= Index && Index < GetLength();
	}
	
	JString GetName(int Index) const
	{
		JString result;
		if (Contain(Index))
		{
			result = (char*)m_List[Index];
		}
		return result;
	}

	IJsonValue* GetPointer(int Index) const
	{
		IJsonValue* result = NULL;
		if (Contain(Index))
		{
			result = m_List[Index];
		}
		return result;
	}

	BYTE* GetBuffer(int Index) const
	{
		BYTE* result = NULL;
		if (Contain(Index))
		{
			result = m_List[Index].GetBuffer();
		}
		return result;
	}

	int IndexOf(const char* pName) const
	{
		int result = -1; 
		for (int i = 0, l = GetLength(); i < l; ++i)
		{
			if (0 == strcmp(pName, m_List[i]))
			{
				result = i;
				break;
			}
		}
		return ++result;
	}

	IJsonValue* GetPointer(const char* Name) const
	{
		IJsonValue* result = NULL;
		int index = IndexOf(Name);
		if (index > 0)
		{
			result = m_List[--index];
		}
		return result;
	}

	BYTE* GetBuffer(const char* Name) const
	{
		BYTE* result = NULL;
		int index = IndexOf(Name);
		if (index > 0)
		{
			result = m_List[--index].GetBuffer();
		}
		return result;
	}

	JsonObject* Append(const char* pName, IJsonValue* pValue)
	{
		m_List.push_back(JsonPair(pName, pValue));

		return this;
	}

	JsonObject* Append(const char* pName, BYTE* pBuffer)
	{
		m_List.push_back(JsonPair(pName, pBuffer));

		return this;
	}

protected:
	std::vector<JsonPair> m_List;
};

#endif //End of JSON_OBJECT_H
