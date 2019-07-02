/*
	Copyright 3h6a@163.com all rights reserved

	IJsonValue Interface
*/
#ifndef I_JSON_VALUE_H
#define I_JSON_VALUE_H

#include "JString.h"

class IJsonValue
{
public:
	virtual ~IJsonValue(){}

public:
	virtual JString ToString()   const = 0;
	virtual JString GetType()    const = 0;
	virtual JString GetString()  const = 0;
	virtual int     GetInteger() const = 0;
	virtual bool    GetBoolean() const = 0;
};

#endif //End of I_JSON_VALUE_H
