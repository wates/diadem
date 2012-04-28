
#include "pipe_tcp.h"
#include "system.h"

// tcp

namespace wts
{
    TcpConnection::TcpConnection()
    {
        state_=CS_CON;
        socket_=0xffffffff;
        target_ip_=0xffffffff;
        setnodelay_on_off_pool_=false;
    }

    void TcpConnection::Open()
    {
        if(CS_CON==state_ ||
            CS_CLOSED==state_)
            state_=CS_NAMERESOLUTION;
    }

    void TcpConnection::Close()
    {
        socket::ShutDownSend(socket_);
    }

    int TcpConnection::Transfer( const uint8_t *buffer,int length )
    {
        int sended;
        if(!socket::Send(socket_,buffer,length,&sended))
            return 0;
        return sended;
    }

    void TcpConnection::SetTarget(const char *address,unsigned short port)
    {
        strcpy(target_address_,address);
        target_ip_=0xffffffff;
        target_port_=port;
    }

    void TcpConnection::SetTarget(unsigned int address,unsigned short port)
    {
        target_address_[0]=0;
        target_ip_=address;
        target_port_=port;
    }

    bool TcpConnection::SetEstablishedSocket(int socket)
    {
        if(CS_CON==state_ ||
            CS_CLOSED==state_)
        {
            this->socket_=socket;
            state_=CS_ESTABLISHED;
            if(Next())
                Next()->Open();
            return true;
        }
        else
            return false;
    }

    int TcpConnection::GetSocket()
    {
        return this->socket_;
    }

    bool TcpConnection::SetNodelay(bool on_off)
    {
        if(-1==socket_)
        {
            setnodelay_on_off_pool_=on_off;
            return true;
        }
        else
        {
            return socket::Nodelay(socket_,on_off);
        }
    }

    void TcpConnection::Update( int something )
    {
        UNUSED(something);
        if(CS_NAMERESOLUTION==state_)
        {
            if(target_ip_==0xffffffff)
            {
                if(!socket::InetAddr(target_ip_,target_address_))
                {
                    if(!socket::GetHostByName(target_ip_,target_address_))
                    {
                        state_=CS_ERROR;
                        return;
                    }
                }
            }
            state_=CS_OPEN;
        }
        if(CS_OPEN==state_)
        {
            socket::TcpSocket(socket_);
            socket::Nonblock(socket_);
            socket::Connect(socket_,target_ip_,target_port_);
            state_=CS_CONNECTING;
        }
        if(CS_CONNECTING==state_)
        {
            socket::ThreeState ret=socket::IsConnect(socket_);
            if(socket::Succeed==ret)
            {
                state_=CS_ESTABLISHED;
                if(setnodelay_on_off_pool_)
                    socket::Nodelay(socket_,setnodelay_on_off_pool_);
                if(Next())
                    Next()->Open();
            }
            else if(socket::Failed==ret)
            {
                socket::CloseSocket(socket_);
                state_=CS_ERROR;
            }
        }
        if(CS_ESTABLISHED==state_)
        {
            uint8_t buffer[10240];
            int recved;
            if(socket::Receive(socket_,buffer,sizeof(buffer),&recved))
            {
                if(0==recved)
                {
                    socket::CloseSocket(socket_);
                    if(Next())
                        Next()->Close();
                    state_=CS_CLOSED;
                }
                else if(Next())
                {
                    Next()->Transfer(buffer,recved);
                }
            }
            else if(!socket::IsWouldblock())
            {
                socket::CloseSocket(socket_);
                Next()->Close();
                state_=CS_CLOSED;
            }
        }
        if(CS_ERROR==state_)
        {
            socket::CloseSocket(socket_);
            state_=CS_CLOSED;
        }
    }

    //////////////////////////////
    // TcpListener

    TcpListener::TcpListener()
    {
        state_=LS_CON;
        listen_port_=0;
    }

    void TcpListener::Open()
    {
        if(LS_BIND==state_)
        {
            state_=LS_OPENING;
        }
    }

    void TcpListener::Close()
    {
    }

    void TcpListener::AppendConnection(TcpConnection *connection)
    {
        this->connection_pool_.Push(connection);
    }

    void TcpListener::Update( int something )
    {
        UNUSED(something);
        if(LS_OPENING==state_)
        {
            socket::TcpSocket(socket_);
            socket::Nonblock(socket_);
            socket::ReuseAddress(socket_);
            socket::Bind(socket_,listen_port_,listen_addr_);
            socket::Listen(socket_);
            state_=LS_LISTENING;
        }
        if(LS_LISTENING==state_)
        {
            int newsocket;
            if(socket::IsAcceptable(socket_)&&
                socket::Accept(socket_,newsocket))
            {
                socket::Nonblock(newsocket);

                bool good=false;
                for(int i=0;i<connection_pool_.Size();i++)
                {
                    if(connection_pool_[i]->SetEstablishedSocket(newsocket))
                    {
                        good=true;
                        break;
                    }
                }
                if(!good)
                {
                    socket::ShutDownSend(newsocket);
                    socket::CloseSocket(newsocket);
                }
            }
        }
    }

    void TcpListener::Bind(unsigned short port,unsigned int addr)
    {
        if(LS_CON==state_)
        {
            listen_port_=port;
            listen_addr_=addr;
            state_=LS_BIND;
        }
    }

}

