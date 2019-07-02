https://gitee.com/a6h3/JsonLite

========================================================================
    JsonLite 项目概述
========================================================================

JsonLite是一个JSON的C++实现，可以生成和解析JSON，除了依赖C/C++标准库
以外不依赖任何第三方库，可以移植到任何平台上。

JsonLite暂时不支持JSON String的Unicode转义。

/////////////////////////////////////////////////////////////////////////////
下面介绍组成 JsonLite的主要文件的内容。

IJsonValue
    JsonValue的接口
    GetType()    : 取Value类型, 可能返回的值有null, boolean, number, string,
                   array, object
    GetString()  : 取String类型的值
    GetInteger() : 取Integer类型的值
    GetBoolean() : 取Boolean类型的值

JsonValue
    JsonValue的封装类。带引用计数的浅挎贝类，通过挎贝构造和赋值操作获得
    的实例与原始实例指向同一个对象的指针。

JsonObjectBuilder
    JsonObject的构造器。

JsonArrayBuilder
    JsonArray的构造器。

JsonParser
    String到Json的解析器。

JsonFormater
    Json到String的转换器。

/////////////////////////////////////////////////////////////////////////////
其他标准文件：

JString
    JsonLite的字符串类。

JPointer
    JsonLite的智能指针。
