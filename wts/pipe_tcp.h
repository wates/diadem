
#ifndef WTS_PIPE_TCP_H
#define WTS_PIPE_TCP_H

#include "pipe.h"

namespace wts
{
    namespace IPAddressType
    {
        enum IPAddressType
        {
        };
    }

    inline uint32_t IPAddress(uint8_t a,uint8_t b,uint8_t c,uint8_t d)
    {
        return (a<<24)|(b<<16)|(c<<8)|d;
    }

    class TcpConnection
        :public Simplex
        ,public Subject
    {
    private:
        enum ConnectionState
        {
            CS_CON,
            CS_NAMERESOLUTION,
            CS_OPEN,
            CS_CONNECTING,
            CS_ESTABLISHED,
            CS_CLOSED,
            CS_ERROR
        };

    public:
        TcpConnection();

        void Open();
        void Close();
        int Transfer(const uint8_t *buffer,int length);

        inline void SetNext(Endpoint*p){next_=p;}
        inline Endpoint*Next(){return next_;}

        void Update(int something);
        bool SetEstablishedSocket(int socket);
        int GetSocket();
        bool SetNodelay(bool on_off);
        void SetTarget(const char *address,unsigned short port);
        void SetTarget(unsigned int address,unsigned short port);
    private:

        ConnectionState state_;
        Endpoint *next_;
        int socket_;

        char target_address_[256];
        unsigned short target_port_;
        unsigned int target_ip_;
        bool setnodelay_on_off_pool_;

    };

    class TcpListener
        :public Subject
    {
    private:
        enum ListenerState
        {
            LS_CON,
            LS_BIND,
            LS_OPENING,
            LS_LISTENING,
            TS_ERROR
        };
    public:
        TcpListener();

        void Open();
        void Close();
        void AppendConnection(TcpConnection *connection);
        void Bind(unsigned short port,unsigned int addr);
        
        void Update(int something);
    private:
        ListenerState state_;

        int socket_;
        unsigned short listen_port_;
        unsigned int listen_addr_;
        Array<TcpConnection*> connection_pool_;
    };

}


#endif
