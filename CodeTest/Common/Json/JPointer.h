/*
	Copyright 3h6a@163.com all rights reserved

	JPointer template class
*/
#ifndef JPOINTER_H
#define JPOINTER_H

template<class T>
class JPointer
{
public:
	JPointer(T* pPointer)
	{
		m_pBuffer = new BYTE[sizeof(int) + sizeof(T*)];
		if (m_pBuffer != NULL)
		{
			int* p = (int*)m_pBuffer;
			*p = 1;
			*(T**)(++p) = pPointer;
		}
	}

	JPointer(BYTE* pBuffer) : m_pBuffer(pBuffer)
	{
		AddRef();
	}

	JPointer(const JPointer& Instance) : m_pBuffer(Instance.m_pBuffer)
	{
		AddRef();
	}

	JPointer& operator=(const JPointer& Instance)
	{
		if (this != &Instance)
		{
			Release();
			m_pBuffer = Instance.m_pBuffer;
			AddRef();
		}

		return *this;
	}
	
	~JPointer()
	{
		Release();
	}

	void AddRef()
	{
		if (m_pBuffer != NULL)
		{
			++*(int*)m_pBuffer;
		}
	}

	int Release()
	{
		int result = 0;
		if (m_pBuffer != NULL)
		{
			result = --*(int*)m_pBuffer;
			if (result == 0)
			{
				//
				delete GetPointer();

				delete[] m_pBuffer;
				m_pBuffer = NULL;
			}
		}
		return result;
	}

public:
	bool IsNull() const
	{
		return m_pBuffer == NULL || GetPointer() == NULL;
	}

	T* GetPointer() const
	{
		return *(T**)(m_pBuffer + sizeof(int));
	}

	BYTE* GetBuffer() const
	{
		return m_pBuffer;
	}

	T* operator->() const
	{
		return GetPointer();
	}

	operator T*() const
	{
		return GetPointer();
	}

private:
	BYTE* m_pBuffer;
};

#endif //End of JPOINTER_H