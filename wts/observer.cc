
#include "observer.h"

#include "container.h"

namespace wts
{
    Subject::~Subject()
    {
    }

    Observer::~Observer()
    {
    }
}

namespace wts
{

    class BasicObserver
        :public Observer
    {
    private:
        bool Append(Subject *any);
        bool Remove(Subject *any);
        void Update(int something);

        Array<Subject*> subject_;
    };

    bool BasicObserver::Append(Subject *any)
    {
        subject_.Push(any);
        return true;
    }

    bool BasicObserver::Remove(Subject *any)
    {
        if(Subject **hit=subject_.Find(any))
        {
            subject_.Erase(hit-subject_.Data(),1);
            return true;
        }
        else
        {
            return false;
        }
    }

    void BasicObserver::Update(int something)
    {
        for(int i=0;i<subject_.Size();i++)
        {
            subject_[i]->Update(something);
        }
    }

    Observer* CreateBasicObserver()
    {
        return new BasicObserver();
    }

    void DeleteBasicObserver(Observer *obs)
    {
        delete static_cast<BasicObserver*>(obs);
    }

}

