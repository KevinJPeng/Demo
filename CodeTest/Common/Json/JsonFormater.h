/*
	Copyright 3h6a@163.com all rights reserved

	JsonFormater class
*/
#ifndef JSON_FROMATER_H
#define JSON_FROMATER_H

#include "JsonValue.h"

class JsonFormater
{
public:
	JsonFormater(void);
	~JsonFormater(void);

public:
	JString Format(const JsonValue& Json, int Indent);

private:
	JString FormatArray(const JsonValue& Json, int Level, int Indent);
	JString FormatObject(const JsonValue& Json, int Level, int Indent);
	JString FormatString(const char* pString);
};

#endif //End of JSON_FROMATER_H

