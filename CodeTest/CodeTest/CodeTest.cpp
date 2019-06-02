// CodeTest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include "utility.h"

int main()
{
	CUtility utility;
	//cin可以连续从键盘读取想要的数据，以空格、tab或换行作为分隔符
	int iIn;
	while (1)
	{
		std::cout
			<< "\n\n"
			<< "测试项:\n"
			<< "0: 退出\n"
			<< "1: log4z test\n"
			<< "2: tinyxml2 test\n"
			<< "3: StringConvert test\n"
			<< std::endl;

		std::cin >> iIn;
		if (0 == iIn)
		{
			break;
		}
		else if (1 == iIn)
		{
			utility.log4zTest();
		}
		else if (2 == iIn)
		{
			utility.tinyxml2Test();
		}
		else if (3 == iIn)
		{
			utility.StringConvertTest();
		}
		else
		{
			std::cout << "无效指令\n";
		}

	}
}

