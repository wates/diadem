
#ifndef ANDROID
#ifdef __linux__

#include "system.h"

#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>


namespace wts
{
    class CriticalSectionBody
        :public CriticalSection
    {
    public:
        CriticalSectionBody()
        {
            if(0!=::pthread_mutex_init(&mutex_,NULL))
            {
                // fatal
            }
            memset(&thread_,0,sizeof(pthread_t));
            count_=0;
        }
        ~CriticalSectionBody()
        {
            ::pthread_mutex_destroy(&mutex_);
        }
    private:
        void Enter()
        {
            pthread_t self=pthread_self();
            if(count_>0&&pthread_equal(self,thread_))
            {
                count_++;
                return/* true*/;
            }
            pthread_mutex_lock(&mutex_);
            if(EINVAL==errno||EDEADLK==errno)
            {
                return/* false*/;
            }
            thread_=self;
            count_++;
            return/* true*/;
        }
        void Leave()
        {
            if(count_>1)
            {
                count_--;
                return;
            }
            memset(&thread_,0,sizeof(pthread_t));
            count_--;
            ::pthread_mutex_unlock(&mutex_);
        }

        pthread_mutex_t mutex_;
        pthread_t thread_;
        int count_;
    };

    CriticalSection* CreateCriticalSection()
    {
        return new CriticalSectionBody;
    }

    void DeleteCriticalSection(CriticalSection *cs)
    {
        delete static_cast<CriticalSectionBody*>(cs);
    }

    struct ThreadParameter
    {
        void(*routine)(void*);
        void *parameter;
        pthread_t thread_id;
    };

    static void ThreadCallback(void *p)
    {
        ThreadParameter *tp=(ThreadParameter*)p;
        tp->routine(tp->parameter);
        pthread_t thread_id=tp->thread_id;
        delete tp;
        void *fake=0;
        ::pthread_join(thread_id,&fake);
    }

    void StartThread(void(*routine)(void*),int stack_size,void *parameter)
    {
        UNUSED(stack_size);

        ThreadParameter *tp=new ThreadParameter;
        tp->routine=routine;
        tp->parameter=parameter;
        ::pthread_create(&tp->thread_id,NULL,(void*(*)(void*))ThreadCallback,tp);
    }

    void Sleep(uint32_t millsec)
    {
        ::usleep(millsec*1000);
    }

    uint64_t GetTime()
    {
        struct timeval tmv;
        gettimeofday( &tmv, NULL );
        static uint64_t startup=(uint64_t)tmv.tv_sec*1000000+tmv.tv_usec;
        static uint64_t last=startup;
        uint64_t now=(uint64_t)tmv.tv_sec*1000000;
        now+=tmv.tv_usec;
        if(now<last)
            startup-=last-now;
        last=now;
        return (now-startup)/1000;
    }

    void GetCalendarTime(CalendarTime* time)
    {
        struct tm *tm;
        timeval tv;

        gettimeofday(&tv,NULL);
        tm=localtime(&tv.tv_sec);
        time->year = tm->tm_year+1900;
        time->month = tm->tm_mon+1;
        time->day = tm->tm_mday;
        time->hour = tm->tm_hour;
        time->minute = tm->tm_min;
        time->second = tm->tm_sec;
        time->milliseconds = (tv.tv_usec/1000)%1000;
    }

    namespace socket
    {
        void StartupSocket()
        {
        }

        bool TcpSocket(int &newsock)
        {
            int ret=(int)::socket(AF_INET,SOCK_STREAM,0);
            if(ret<0)
            {
                return false;
            }
            newsock=ret;
            return true;
        }

        bool CloseSocket(int sock)
        {
            return 0==close(sock);
        }

        bool Nonblock(int sock)
        {
            int flag=fcntl(sock,F_GETFL,0);
            return fcntl(sock,F_SETFL,flag|O_NONBLOCK)>=0;
        }

        bool Nodelay(int sock,bool on_off)
        {
            int val=on_off?1:0;
            return 0==setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,(const char*)&val,sizeof(val));
        }

        bool InetAddr(unsigned int &ip,const char *addr)
        {
            unsigned long i=inet_addr(addr);
            if(0xffffffff==i)
            {
                return false;
            }
            ip=ntohl(i);
            return true;
        }

        bool GetHostByName(unsigned int &ip,const char *addr)
        {
            hostent *ent;
            ent=gethostbyname(addr);
            if(0==ent)
                return false;
            ip=ntohl(*(unsigned long*)(ent->h_addr_list[0]));
            return true;
        }

        bool Connect(int sock,unsigned int ip,unsigned short port)
        {
            sockaddr_in Addrin;
            memset(&Addrin,0,sizeof(Addrin));
            Addrin.sin_port=htons(port);
            Addrin.sin_family=AF_INET;
            u_long sin_addr=htonl(ip);
            memcpy(&Addrin.sin_addr,&sin_addr,sizeof(sin_addr));
            connect(sock,(sockaddr*)&Addrin,sizeof(Addrin));
            return true;
        }

        bool Send(int sock,const unsigned char *buffer,int length,int *sended)
        {
            int ret=send(sock,buffer,length,0);
            if(ret<0)
            {
                if(sended)
                    *sended=0;
                return false;
            }
            if(sended)
                *sended=ret;
            return true;
        }

        bool Receive(int sock,unsigned char *buffer,int length,int *received)
        {
            int ret=recv(sock,buffer,length,0);
            if(ret<0)
            {
                if(received)
                    *received=0;
                return false;
            }
            if(received)
                *received=ret;
            return true;
        }

        ThreeState IsConnect(int sock)
        {
            fd_set w,e;
            FD_ZERO(&w);
            FD_ZERO(&e);
            FD_SET(sock,&w);
            FD_SET(sock,&e);
            timeval tv;
            tv.tv_sec=0;
            tv.tv_usec=0;
            select(sock+1,0,&w,&e,&tv);
            if(FD_ISSET(sock,&w))
            {
                return Succeed;
            }
            else if(FD_ISSET(sock,&e))
            {
                return Failed;
            }
            return Running;
        }

        bool Bind(int sock,unsigned short port,unsigned int addr)
        {
            sockaddr_in addrin;
            memset(&addrin,0,sizeof(addrin));
            addrin.sin_family=AF_INET;
            *(unsigned int*)(&addrin.sin_addr)=htonl(addr);
            addrin.sin_port=htons(port);
            return bind(sock,(sockaddr*)&addrin,sizeof(addrin))>=0;
        }

        bool Listen(int sock)
        {
            return listen(sock,SOMAXCONN)==0;
        }

        bool IsAcceptable(int sock)
        {
            fd_set r;
            FD_ZERO(&r);
            FD_SET(sock,&r);
            timeval tv;
            tv.tv_sec=0;
            tv.tv_usec=0;
            select(0,&r,0,0,&tv);
            if(FD_ISSET(sock,&r))
            {
                return true;
            }
            return false;
        }
        bool Accept(int sock,int &newsock)
        {
            sockaddr_in name;
            socklen_t len=sizeof(name);
            int ret=(int)accept(sock,(sockaddr*)&name,&len);
            if(ret<0)
                return false;
            newsock=ret;
            return true;
        }

        bool IsWouldblock()
        {
            return errno==EWOULDBLOCK;
        }

        bool ReuseAddress(int sock)
        {
            int f=1;
            return 0<=setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char*)&f,sizeof(f));
        }

        bool ShutDownSend(int sock)
        {
            return -1!=::shutdown(sock,SHUT_WR);
        }

    }
}

#endif
#endif