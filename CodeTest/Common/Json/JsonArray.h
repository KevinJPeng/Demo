/*
	Copyright 3h6a@163.com all rights reserved
	
	JsonArray class
*/
#ifndef JSON_ARRAY_H
#define JSON_ARRAY_H

#include "IJsonValue.h"
#include "JPointer.h"
#include <vector>

class JsonArray : public IJsonValue
{
public:
	JsonArray()
	{
	}

	virtual JString ToString() const
	{
		JString result;
		for (int i = 0, l = (int)m_List.size(); i < l; ++i)
		{
			if (i > 0) result += ", ";
			result += m_List[i]->ToString();
		}
		return "[" + result + "]";
	}

	virtual JString GetType() const
	{
		return JString("array");
	}

	virtual JString GetString() const
	{
		return JString("array");
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

	JsonArray* Append(IJsonValue* pValue)
	{
		m_List.push_back(JPointer<IJsonValue>(pValue));

		return this;
	}

	JsonArray* Append(BYTE* pBuffer)
	{
		m_List.push_back(JPointer<IJsonValue>(pBuffer));

		return this;
	}

protected:
	std::vector<JPointer<IJsonValue>> m_List;
};

#endif //End of JSON_ARRAY_H
