/*
	Copyright 3h6a@163.com all rights reserved
*/
#include "pch.h"

#include "JsonParser.h"

#include "JsonSimple.h"
#include "JsonArray.h"
#include "JsonObject.h"

inline bool SkipSpace(const char** ppText)
{
	const char* &p = *ppText;
	while (isspace(*p))	++p;
	return *p != 0;
}

JsonParser::JsonParser()
{
}

JsonParser::~JsonParser()
{
}

JsonValue JsonParser::Parse(const char* pText)
{
	IJsonValue* result = NULL;

	if (*pText == '{')
	{
		result = ParseObject(&pText);
	}
	else if (*pText == '[')
	{
		result = ParseArray(&pText);
	}
	else
	{
		//Error
	}

	return JsonValue(result);
}

IJsonValue* JsonParser::ParseValue(const char** ppText)
{
	IJsonValue* result = NULL;

	const char* &p = *ppText;
	if (*p == '\"')
	{
		JString value = ParseString(&p);
		++p; //'\"'
		result = new JsonString(value);
	}
	else if (*p == '[')
	{
		result = ParseArray(&p);
		++p; //']'
	}
	else if (*p == '{')
	{
		result = ParseObject(&p);
		++p; //'}'
	}
	else
	{
		JString value;
		while (*p != ',' && *p != '}' && *p != ']' && *p != 0)
		{
			value += *p++;
		}
		value.Trim();

		if (value.Equal("true"))
		{
			result = new JsonBoolean(true);
		}
		else if (value.Equal("false"))
		{
			result = new JsonBoolean(false);
		}
		else if (value.Equal("null"))
		{
			result = new JsonNull();
		}
		else
		{
			result = new JsonNumber(value);
		}
	}

	return result;
}

IJsonValue* JsonParser::ParseArray(const char** ppText)
{
	JsonArray* pArray = new JsonArray();

	const char* &p = ++*ppText;
	while (*p != 0)
	{
		//[]value
		if (!SkipSpace(&p)) break;

		//[value]
		pArray->Append(ParseValue(&p));

		//value[]
		SkipSpace(&p);

		if (*p == ',')
		{
			++p;
		}
		else if (*p == ']')
		{
			break;
		}
		else if (*p == '}') //Error
		{
			break;
		}
		else
		{
			//May be '\0' only.
		}
	}

	return pArray;
}

IJsonValue* JsonParser::ParseObject(const char** ppText)
{
	JsonObject* pObject = new JsonObject();

	const char* &p = ++*ppText;
	while (*p != 0)
	{
		JString name;

		//[]"string":value
		if (!SkipSpace(&p)) break;

		//["]string":value
		if (*p != '\"')
		{
			break;
		}
		
		//"[string]":value
		name = ParseString(&p);

		//"string["]:value
		if (*p++ != '\"')
		{
			break;
		}

		//"string"[]:value
		if (!SkipSpace(&p)) break;

		//"string"[:]value
		if (*p++ != ':')
		{
			break;
		}

		//"string":[]value
		if (!SkipSpace(&p)) break;

		//"string":[value]
		pObject->Append(name, ParseValue(&p));

		//value[]
		SkipSpace(&p);

		if (*p == ',')
		{
			++p;
		}
		else if (*p == '}')
		{
			break;
		}
		else if (*p == ']') //Error
		{
			break;
		}
		else
		{
			//May be '\0' only.
		}
	}

	return pObject;
}

JString JsonParser::ParseString(const char** ppText)
{
	JString result;
	const char* &p = ++*ppText;
	while (*p != '\"' && *p != 0)
	{
		if (*p == '\\')
		{
			switch(*++p)
			{
			case '\"':
				result += '\"';
				break;
			case '\\':
				result += '\\';
				break;
			//case '/':
			//	result += '/';
			//	break;
			case 'b':
				result += '\b';
				break;
			case 'f':
				result += '\f';
				break;
			case 'n':
				result += '\n';
				break;
			case 'r':
				result += '\r';
				break;
			case 't':
				result += '\t';
				break;
			case 'u':
				result += '\\';
				result += 'u';
				result += *++p;
				result += *++p;
				result += *++p;
				result += *++p;
				break;
			default:
				;
			}
			++p;
		}
		else
		{
			result += *p;
			++p;
		}
	}
	return result;
}
