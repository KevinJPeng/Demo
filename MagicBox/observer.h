#ifndef _OBSERVER_H_
#define _OBSERVER_H_

// ����۲���
class IObserver
{
public:
	virtual void Update(int _iRate) = 0;  // 
};

#endif