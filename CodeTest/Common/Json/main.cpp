
#include "JSON.h"

#include <tchar.h>
#include <stdio.h>
#include <conio.h>

int _tmain(int argc, _TCHAR* argv[])
{
	JsonObjectBuilder builder;
	builder.Append("boolean1", true);
	builder.Append("boolean2", false);
	builder.Append("number1",  100);
	builder.Append("number2",  1.1);
	builder.Append("string",  "ÎÒµÄJsonLite");
	builder.Append("string1",  "123\"456");
	builder.Append("string2",  "123\\456");
	builder.Append("string3",  "123/456");
	//builder.Append("string4",  "123\b456");
	//builder.Append("string5",  "123\f456");
	builder.Append("string6",  "123\n456");
	//builder.Append("string7",  "123\r456");
	builder.Append("string8",  "123\t456");
	builder.Append("string9",  "\\u1234\\u4567");
	builder.Append("object",  
		JsonObjectBuilder()
			.Append("boolean1", true)
			.Append("boolean2", false)
			.Build()
		);
	builder.Append("array",  
		JsonArrayBuilder()
			.Append(true)
			.Append(false)
			.Append(100)
			.Append(1.1)
			.Append("xyz")
			.Build()
		);
	JString str0  = builder.Build()["string"].GetString();
	JString text0 = JSON::Format(builder.Build(), 2);

	//JsonValue json = JsonParser().Parse("{ \"name\": \"100\", \"boolean\": true }");
	JsonValue json = JSON::Parse(text0);
	JString str1  = json["string1"].GetString();
	JString str2  = json["string2"].GetString();
	JString str3  = json["string3"].GetString();
	//JString str4  = json["string4"].GetString();
	//JString str5  = json["string5"].GetString();
	JString str6  = json["string6"].GetString();
	//JString str7  = json["string7"].GetString();
	JString str8  = json["string8"].GetString();
	JString str9  = json["string9"].GetString();
	JString text1 = json.ToString();
	JString text2 = JSON::Format(json, 4);

	//puts(str0);
	puts(text0);

	printf("string1: %s\n", (char*)str1);
	printf("string2: %s\n", (char*)str2);
	printf("string3: %s\n", (char*)str3);
	//printf("string4: %s\n", (char*)str4);
	//printf("string5: %s\n", (char*)str5);
	printf("string6: %s\n", (char*)str6);
	//printf("string7: %s\n", (char*)str7);
	printf("string8: %s\n", (char*)str8);
	printf("string9: %s\n", (char*)str9);

	//puts(text1);
	puts(text2);

	_getch();
	return 0;
}
