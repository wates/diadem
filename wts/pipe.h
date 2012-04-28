
#ifndef WTS_PIPE_H_
#define WTS_PIPE_H_

#include "wts_types.h"
#include "observer.h"
#include "container.h"

namespace wts
{
    const char* memmem(const char *p,int sz_p,const char *cmp,int sz_cmp);

    class Endpoint
    {
    public:
        virtual void Open()=0;
        virtual void Close()=0;
        virtual int Transfer(const uint8_t *buffer,int length)=0;

        inline virtual ~Endpoint(){}
    };

    class Startpoint
    {
    public:
        virtual void SetNext(Endpoint*)=0;
        virtual Endpoint* Next()=0;

        inline virtual ~Startpoint(){}
    };

    class Simplex
        :public Endpoint
        ,public Startpoint
    {
    };

    struct Duplex
    {
        Simplex *up;
        Simplex *down;
    };

    ///////////////////////////
    // addon

    class BinaryPrint
        :public Endpoint
    {
    public:
        void Open();
        void Close();
        int Transfer(const uint8_t *buffer,int length);
    };

    class ByteQueue
    {
    public:
        ByteQueue(int length=1024,int capacity=1024*1024*16);
        ~ByteQueue();
        int Size();
        bool Reserve(uint8_t **pointer,int len);
        bool Push(const uint8_t *data,int len);
        void Shrink(int len);
        uint8_t* Data();
    private:
        uint8_t *buffer_;
        int length_;
        int capacity_;
        int start_;
        int end_;
    };

    class MessageQueue
        :public Simplex
    {
    private:
        enum
        {
            CMD_OPEN=1,
            CMD_CLOSE,
            CMD_TRANSFER
        };
    public:
        MessageQueue();
        ~MessageQueue();

        void Fetch();
        inline void SetNext(Endpoint*p){next_=p;}
        inline Endpoint*Next(){return next_;}

        void Open();
        void Close();
        int Transfer(const uint8_t *buffer,int length);

        bool Lock(uint8_t **buffer,int length);
        void Unlock();
    private:
        Endpoint *next_;
        class CriticalSection *cs_;
        ByteQueue queue_;
        RawArray<uint8_t> command_buffer_;
    };

    class Buffer
        :public Simplex
        ,public Subject
    {
    public:
        Buffer();
        inline void SetNext(Endpoint*p){next_=p;}
        inline Endpoint*Next(){return next_;}

        void Open();
        void Close();
        int Transfer(const uint8_t *buffer,int length);
    protected:
        void Update(int something);
    private:
        Endpoint *next_;
        ByteQueue que_;
        bool enable_;
        bool close_flag_;
    };

    class Duplicator
        :public Simplex
    {
    public:
        inline void SetNext(Endpoint*p){next_=p;}
        inline Endpoint*Next(){return next_;}

        inline void SetSecondary(Endpoint*p){secondary_=p;}
        inline Endpoint*Secondary(){return secondary_;}

        inline void Open(){next_->Open();secondary_->Open();};
        inline void Close(){next_->Close();secondary_->Close();};
        inline int Transfer(const uint8_t *buffer,int length)
        {
            int ret=next_->Transfer(buffer,length);
            secondary_->Transfer(buffer,length);
            return ret;
        };
    private:
        Endpoint *next_;
        Endpoint *secondary_;
    };

}

#endif
