#ifndef _SUBJECT_H_
#define _SUBJECT_H_

class IObserver;

// ��������
class ISubject
{
public:
	virtual void Attach(IObserver *) = 0;  // ע��۲���
	virtual void Detach(IObserver *) = 0;  // ע���۲���
	virtual void Notify() = 0;  // ֪ͨ�۲���
};

#endif