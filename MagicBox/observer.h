#ifndef _OBSERVER_H_
#define _OBSERVER_H_

// ³éÏó¹Û²ìÕß
class IObserver
{
public:
	virtual void Update(int _iRate) = 0;  // 
};

#endif