
#ifndef WTS_SYSTEM_H_
#define WTS_SYSTEM_H_

#include "container.h"

namespace wts
{
	class CriticalSection
	{
	public:
		virtual void Enter()=0;
		virtual void Leave()=0;

        inline virtual ~CriticalSection(){};
	};

	CriticalSection* CreateCriticalSection();
	void DeleteCriticalSection(CriticalSection *cs);

	class CriticalSectionBlock
	{
    public:
		inline CriticalSectionBlock(CriticalSection *cs)
			:cs_(cs)
		{
			cs_->Enter();
		}
		inline ~CriticalSectionBlock()
		{
			cs_->Leave();
		}
    private:
        CriticalSection *cs_;
	};

	void StartThread(void(*routine)(void*),int stack_size,void *parameter);

    struct CalendarTime
    {
        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;
        int milliseconds;
    };
    void GetCalendarTime(CalendarTime* time);

	void Sleep(uint32_t millsec);
	uint64_t GetTime();


	bool ReadFile(const char *filename,RawArray<char> &out);
	bool WriteFile(const char *filename,RawArray<char> &in);

	void LogPut(const char *string);
	struct DebugTrace
	{
    public:
		DebugTrace(const char *func_name);
		~DebugTrace();
    private:
		char name[252];
		int indent;
	};

    void MoveExecutableDirectory();

    namespace socket
    {
        enum ThreeState
        {
            Running,
            Succeed,
            Failed
        };

        void StartupSocket();
        void CleanupSocket();

        bool GetHostByName(unsigned int &ip,const char *addr);

        bool TcpSocket(int &newsock);
        bool CloseSocket(int sock);
        bool Nonblock(int sock);
        bool Nodelay(int sock,bool on_off);
        bool InetAddr(unsigned int &ip,const char *addr);
        bool Connect(int sock,unsigned int ip,unsigned short port);
        bool Send(int sock,const uint8_t *buffer,int length,int *sended);
        bool Receive(int sock,uint8_t *buffer,int length,int *received);
        ThreeState IsConnect(int sock);
        bool Bind(int sock,unsigned short port,unsigned int addr);
        bool Listen(int sock);
        bool IsAcceptable(int sock);
        bool Accept(int sock,int &newsocket);
        bool IsWouldblock();
        bool ReuseAddress(int sock);
        bool ShutDownSend(int sock);
    }
}
#ifdef DEBUG
#ifdef _WIN32
#define SET_TRACE wts::DebugTrace dt_##__LINE__(__FUNCTION__);
#else
#define SET_TRACE wts::DebugTrace dt_##__LINE__(__func__);
#endif
#else
#define SET_TRACE (void)0
#endif

#endif
