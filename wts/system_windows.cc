
#ifdef WINDOWS

#include "system.h"

#include <WinSock2.h>
#include <Windows.h>

#pragma comment(lib,"ws2_32.lib")

namespace wts
{
    class CriticalSectionBody
        :public CriticalSection
    {
        CRITICAL_SECTION cs;
        void Enter()
        {
            ::EnterCriticalSection(&cs);
        }
        void Leave()
        {
            ::LeaveCriticalSection(&cs);
        }
    public:
        CriticalSectionBody()
        {
            ::InitializeCriticalSection(&cs);
        }
        ~CriticalSectionBody()
        {
            ::DeleteCriticalSection(&cs);
        }
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
    };

    static DWORD WINAPI ThreadProc(LPVOID lpParameter)
    {
        ThreadParameter *tp=(ThreadParameter*)lpParameter;
        tp->routine(tp->parameter);
        delete tp;
        ::ExitThread(0);
    }

    void StartThread(void(*routine)(void*),int stack_size,void *parameter)
    {
        ThreadParameter *tp=new ThreadParameter;
        tp->routine=routine;
        tp->parameter=parameter;
        DWORD id;
        CreateThread(NULL,(SIZE_T)stack_size,ThreadProc,tp,0,&id);
    }

    void Sleep(uint32_t millsec)
    {
        ::Sleep(millsec);
    }

    uint64_t GetTime()
    {
        static bool first=true;
        static LARGE_INTEGER frequency;
        static LARGE_INTEGER start;
        if(first)
        {
            QueryPerformanceFrequency(&frequency);
            QueryPerformanceCounter(&start);
            first=false;
            return 0;
        }
        else
        {
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            return (now.QuadPart-start.QuadPart)*1000/frequency.QuadPart;
        }
    }

    void GetCalendarTime(CalendarTime* time)
    {
        SYSTEMTIME st;
        GetLocalTime(&st);
        time->year = st.wYear;
        time->month = st.wMonth;
        time->day = st.wDay;
        time->hour = st.wHour;
        time->minute = st.wMinute;
        time->second = st.wSecond;
        time->milliseconds = st.wMilliseconds;
    }


	void LogPut(const char *string)
	{
		printf("%s",string);
	}

    namespace socket
    {
        void StartupSocket()
        {
            WSAData data;
            WORD ver=0x0002;
            WSAStartup(ver,&data);
        }

        bool TcpSocket(int &newsocket)
        {
            int ret=(int)::socket(AF_INET,SOCK_STREAM,0);
            if(ret<0)
            {
                return false;
            }
            newsocket=ret;
            return true;
        }

        bool CloseSocket(int sock)
        {
            return 0==closesocket(sock);
        }

        bool Nonblock(int sock)
        {
            unsigned long arg=1;
            return 0==ioctlsocket(sock,FIONBIO,&arg);
        }

        bool Nodelay(int sock,bool on_off)
        {
            BOOL val=on_off?TRUE:FALSE;
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
            sockaddr_in name;
            name.sin_addr.S_un.S_addr=htonl(ip);
            name.sin_family=AF_INET;
            name.sin_port=htons(port);
            connect(sock,(sockaddr*)&name,sizeof(name));
            return true;
        }

        bool Send( int sock,const uint8_t *buffer,int length,int *sended )
        {
            int ret=send(sock,reinterpret_cast<const char*>(buffer),length,0);
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

        bool Receive( int sock,uint8_t *buffer,int length,int *received )
        {
            int ret=recv(sock,reinterpret_cast<char*>(buffer),length,0);
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
            select(0,0,&w,&e,&tv);
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
            sockaddr_in name;
            memset(&name,0,sizeof(name));
            *(unsigned int*)(&name.sin_addr.S_un.S_addr)=htonl(addr);
            name.sin_family=AF_INET;
            name.sin_port=htons(port);
            return bind(sock,(sockaddr*)&name,sizeof(name))==0;
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
        bool Accept(int sock,int &newsocket)
        {
            sockaddr_in name;
            int len=sizeof(name);
            int ret=(int)accept(sock,(sockaddr*)&name,&len);
            if(ret<0)
                return false;
            newsocket=ret;
            return true;
        }

        bool IsWouldblock()
        {
            return WSAGetLastError()==WSAEWOULDBLOCK;
        }

        bool ReuseAddress(int sock)
        {
            int f=1;
            return 0<=setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char*)&f,sizeof(f));
        }

        bool ShutDownSend(int sock)
        {
            return SOCKET_ERROR!=::shutdown(sock,SD_SEND);
        }

    }
}


#endif
