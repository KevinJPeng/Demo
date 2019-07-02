/*
	Copyright 3h6a@163.com all rights reserved
*/
#ifndef JSON_H
#define JSON_H

#include "JsonParser.h"
#include "JsonBuilder.h"
#include "JsonFormater.h"

namespace JSON
{

inline JString Format(const JsonValue& Value, int Indent = 0)
{
	return JsonFormater().Format(Value, Indent);
}

inline JsonValue Parse(const char* pText)
{
	return JsonParser().Parse(pText);
}

}

#endif //End of JSON_H
