
#ifndef WTS_OBSERVER_H_
#define WTS_OBSERVER_H_

#include "wts_types.h"

namespace wts
{

    class Subject
    {
    public:
        virtual void Update(int something)=0;
    protected:
        virtual ~Subject();
    private:

    };

    class Observer
    {
    public:
        virtual bool Append(Subject *any)=0;
        virtual bool Remove(Subject *any)=0;
        virtual void Update(int something)=0;
    protected:
        virtual ~Observer();
    private:
    };

    Observer* CreateBasicObserver();
    void DeleteBasicObserver(Observer *obs);

}

#endif

