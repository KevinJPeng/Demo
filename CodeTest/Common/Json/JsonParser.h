/*
	Copyright 3h6a@163.com all rights reserved

	JsonParser class
*/
#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include "JsonValue.h"
 
class JsonParser
{
public:
	JsonParser();
	~JsonParser();

public:
	JsonValue Parse(const char* pText);

private:
	IJsonValue* ParseValue(const char** ppText);
	IJsonValue* ParseArray(const char** ppText);
	IJsonValue* ParseObject(const char** ppText);
	JString     ParseString(const char** ppText);
};
	
#endif //End of JSON_PARSER_H
