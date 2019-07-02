/*
	Copyright 3h6a@163.com all rights reserved

	JString class
*/
#ifndef JSTRING_H
#define JSTRING_H

#include "Common.h"

#define _CRT_SECURE_NO_WARNINGS

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

class JString
{
public:
	JString(void) : m_pBuffer(NULL), m_pString(NULL)
	{
	}

	JString(unsigned int Length) : m_pBuffer(NULL), m_pString(NULL)
	{
		Allocate(Length);
	}

	JString(char Fill, unsigned int Length) : m_pBuffer(NULL), m_pString(NULL)
	{
		if (Allocate(Length))
		{
			memset(m_pString, Fill, Length);
		}
	}

	JString(const char *pString) : m_pBuffer(NULL), m_pString(NULL)
	{
		if (pString != NULL && Allocate(strlen(pString)))
		{
			strcpy(m_pString, pString);
		}
	}

	JString(const char *pString, unsigned int Length) : m_pBuffer(NULL), m_pString(NULL)
	{
		if (Allocate(Length) && pString != NULL)
		{
			strncpy(m_pString, pString, Length);
		}
	}

	JString(const JString &Instance) : m_pBuffer(Instance.m_pBuffer), m_pString(Instance.m_pString)
	{
		AddRef();
	}

	JString &operator=(const JString &Instance)
	{
		if (this != &Instance)
		{
			Release();

			m_pBuffer = Instance.m_pBuffer;
			m_pString = Instance.m_pString;

			AddRef();
		}

		return *this;
	}

	~JString(void)
	{
		Release();
	}

private:
	void Copy()
	{
		if (m_pBuffer != NULL)
		{
			int& ref = *(int*)m_pBuffer;
			if (ref > 1)
			{
				char* pString = m_pString;
				if (Allocate(strlen(pString)))
				{
					strcpy(m_pString, pString);
				}

				--ref;
			}
		}
	}

	bool Allocate(unsigned int Length)
	{
		m_pString = NULL;

		m_pBuffer = new char[sizeof(int) + sizeof(int) + 1 + Length + 1];
		if (m_pBuffer != NULL)
		{
			int* p = (int*)m_pBuffer;
			*p = 1;
			++p;
			*p = Length;
			++p;
			m_pString = (char*)p;
			memset(m_pString, 0, 1 + Length + 1);
			++m_pString;
		}

		return m_pString != NULL;
	}

	void AddRef()
	{
		if (m_pBuffer != NULL)
		{
			++(*(int*)m_pBuffer);
		}
	}

	void Release()
	{
		if (m_pBuffer != NULL)
		{
			if (--*(int*)m_pBuffer == 0)
			{
				delete[] m_pBuffer;

				m_pBuffer = NULL;
				m_pString = NULL;
			}
		}
	}

public:
	operator char*() const
	{
		return m_pString;
	}

	int Capacity() const
	{
		int result = 0;
		if (m_pBuffer != NULL)
		{
			result = *(int*)(m_pBuffer + sizeof(int));
		}
		return result;
	}

	int GetLength() const
	{
		int result = 0;
		if (m_pString != NULL)
		{
			result = strlen(m_pString);
		}
		return result;
	}

	char* GetString() const
	{
		return m_pString;
	}

	bool IsEmpty() const
	{
		return m_pString == NULL || *m_pString == 0;
	}

	bool Equal(const char* pString) const
	{
		bool result = false;
		if (m_pString != NULL && pString != NULL)
		{
			result = strcmp(*this, pString) == 0;
		}
		return result;
	}

	void Empty()
	{
		*this = JString();
	}

	JString& operator += (char chr)
	{
		Copy();

		int length = GetLength();
		if (length >= Capacity())
		{
			*this = JString(m_pString, length + 16); //Step = 16
		}
		m_pString[length] = chr;

		return *this;
	}

	JString& operator += (const char* pString)
	{
		if (pString != NULL && *pString != 0)
		{
			Copy();

			int length = GetLength() + strlen(pString);
			if (length > Capacity())
			{
				*this = JString(m_pString, length + 256); //Step = 256
			}
			strcat(m_pString, pString);
		}

		return *this;
	}

	JString& Format(const char* Format, ...)
	{
		Copy();

		va_list argptr;
		va_start(argptr, Format);

		//int length = _vscprintf(Format, argptr);
		int length = vsnprintf(NULL, 0, Format, argptr);
		if (Capacity() < length)
		{
			*this = JString(length);
		}
		vsprintf(m_pString, Format, argptr);

		return *this;
	}

	JString& Trim()
	{
		if (m_pString != NULL)
		{
			Copy();

			while (isspace(*m_pString))
			{
				++m_pString;
			}

			char* p = m_pString + strlen(m_pString); 
			while (isspace(*--p))
			{
				*p = 0;
			}
		}

		return *this;
	}

	JString& Trim(const char* pString)
	{
		if (m_pString != NULL)
		{
			Copy();

			while (strchr(pString, *m_pString) != NULL)
			{
				++m_pString;
			}

			char* p = m_pString + strlen(m_pString); 
			while (strchr(pString, *--p) != NULL)
			{
				*p = 0;
			}
		}

		return *this;
	}

private:
	char* m_pBuffer;
	char* m_pString;
};

inline bool operator < (const JString& String1, const JString& String2)
{
	return strcmp(String1, String2) < 0;
}

inline JString operator + (const JString& String1, const JString& String2)
{
	return JString(String1) += String2;
}

#endif //End of JSTRING_H
