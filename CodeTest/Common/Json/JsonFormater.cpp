/*
	Copyright 3h6a@163.com all rights reserved
*/
#include "pch.h"

#include "JsonFormater.h"

JsonFormater::JsonFormater(void)
{
}

JsonFormater::~JsonFormater(void)
{
}

JString JsonFormater::Format(const JsonValue& Json, int Indent)
{
	JString result;

	JString type = Json.GetType();
	if (type.Equal("object"))
	{
		result = FormatObject(Json, 0, Indent);
	}
	else if (type.Equal("array"))
	{
		result = FormatArray(Json, 0, Indent);
	}
	else
	{
		//Error
	}

	return result;
}

JString JsonFormater::FormatArray(const JsonValue& Json, int Level, int Indent)
{
	JString result;
	// first line begin
	if (Indent > 0)
	{
		result += JString(' ', Indent * Level);
	}
	result += '[';
	if (Indent > 0)
	{
		result += '\n';
	}
	// first line end
	for (int i = 0, l = Json.GetLength(); i < l;)
	{
		JString type = Json[i].GetType();
		if (type.Equal("string"))
		{
			// begin of the line
			result += JString(' ', Indent * (Level + 1));

			result += FormatString(Json[i].GetString());
		}
		else if (type.Equal("object"))
		{
			result += FormatObject(Json[i], Level + 1, Indent);
		}
		else if (type.Equal("array"))
		{
			result += FormatArray(Json[i], Level + 1, Indent);
		}
		else
		{
			// begin of the line
			result += JString(' ', Indent * (Level + 1));

			result += Json[i].GetString();
		}
		if (++i < l)  //Last Value
		{
			result += ',';
		}
		// end of the line
		if (Indent > 0)
		{
			result += '\n';
		}
	}
	// last line begin
	if (Indent > 0)
	{
		result += JString(' ', Indent * Level);
	}
	result += ']';
	// last line end
	return result;
}

JString JsonFormater::FormatObject(const JsonValue& Json, int Level, int Indent)
{
	JString result;
	// first line begin
	if (Indent > 0)
	{
		result += JString(' ', Indent * Level);
	}
	result += '{';
	if (Indent > 0)
	{
		result += '\n';
	}
	// first line end
	for (int i = 0, l = Json.GetLength(); i < l;)
	{
		// begin of the line
		result += JString(' ', Indent * (Level + 1));

		//string begin
		result += FormatString(Json.GetName(i));
		result += ':';
		result += ' ';
		//string end

		JString type = Json[i].GetType();
		if (type.Equal("string"))
		{
			result += FormatString(Json[i].GetString());
		}
		else if (type.Equal("object"))
		{
			if (Indent > 0)
			{
				result += '\n';
			}
			result += FormatObject(Json[i], Level + 1, Indent);
		}
		else if (type.Equal("array"))
		{
			if (Indent > 0)
			{
				result += '\n';
			}
			result += FormatArray(Json[i], Level + 1, Indent);
		}
		else
		{
			result += Json[i].GetString();
		}
		if (++i < l) // Last pair
		{
			result += ',';
		}
		// end of the line
		if (Indent > 0)
		{
			result += '\n';
		}
	}
	// last line begin
	if (Indent > 0)
	{
		result += JString(' ', Indent * Level);
	}
	result += '}';
	// last line end
	return result;
}

JString JsonFormater::FormatString(const char* pString)
{
	JString result;

	result += '\"';
	if (pString!= NULL)
	{
		while (*pString != 0)
		{
			char chr = *pString;
			switch (chr)
			{
			case '\"':
				result += '\\';
				result += '\"';
				break;
			case '\\':
				if (pString[1] != 'u')
				{
					result += '\\';
					result += '\\';
				}
				else
				{
					result += '\\';
					result += *++pString;
					result += *++pString;
					result += *++pString;
					result += *++pString;
					result += *++pString;
				}
				break;
			//case '/':
			//	result += '\\';
			//	result += '/';
			//	break;
			case '\b':
				result += '\\';
				result += 'b';
				break;
			case '\f':
				result += '\\';
				result += 'f';
				break;
			case '\n':
				result += '\\';
				result += 'n';
				break;
			case '\r':
				result += '\\';
				result += 'r';
				break;
			case '\t':
				result += '\\';
				result += 't';
				break;
			default:
				result += chr;
			}
			++pString;
		}
	}
	result += '\"';

	return result;
}
