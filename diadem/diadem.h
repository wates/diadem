
#include <wts/pipe_tcp.h>
#include <wts/converter.h>

struct ConfigChannel
{
	wts::String name;
	wts::String password;
};

CONVERT_OBJECT_2(ConfigChannel,
    name,password);

struct ServerConfig
{
	wts::String address;
	int port;
	wts::String password;
	wts::String nick;
	wts::String username;
	wts::String realname;
	wts::Array<ConfigChannel> auto_channel;
};

CONVERT_OBJECT_7(ServerConfig,
    address,port,password,nick,username,realname,auto_channel);

struct Config
{
    wts::String db_address;
    int db_port;
    wts::String db_database;
    wts::String db_user;
    wts::String db_password;
    ServerConfig irc_server;
};

CONVERT_OBJECT_6(Config,
    db_address,db_port,db_database,db_user,db_password,irc_server);


class LineParser
	:public wts::Simplex
{
public:
    inline void SetNext(Endpoint*p){next_=p;}
    inline Endpoint*Next(){return next_;}
private:
	void Open();
	void Close();
	int Transfer(const uint8_t *buffer,int length);
    Endpoint *next_;
};

class IRCClient
	:public wts::Simplex
	,public wts::Subject
{
	typedef wts::Array<wts::String> Parameter;

	wts::Buffer *send_buf;
	wts::TcpConnection *tcp;
	wts::Buffer *recv_buf;
	LineParser *lp;

	enum Status
	{
		ST_PASS,
		ST_NICK,
		ST_USER,
		ST_WAIT_MOTD,
		ST_JOIN_WAIT,
		ST_READY,
	};

	Status status;

	ServerConfig config;
	bool enable;

	struct Nickname
	{
		wts::String nick;
		wts::String name;
		wts::String address;
	};

	struct Channel
	{
		char type;
		wts::Array<wts::String> names;
	};

	wts::OrderedMap<wts::String,Channel> channels;

public:
    inline void SetNext(Endpoint*p){next_=p;}
    inline Endpoint*Next(){return next_;}

	//irc status

	wts::String MOTD_first;
	wts::String MOTD;


    // from endpoint
	void Open();
	void Close();
	int Transfer(const uint8_t *buffer,int length);

    //from subject
	void Update(int something);

	Nickname SplitNickname(wts::String msg);
	Parameter SplitParam(wts::String msg);

	void MsgPass(const wts::String &pass);
	void MsgNick(const wts::String &nick);
	void MsgUser(const wts::String &username,const wts::String &realname);
	void MsgJoin(const wts::String &name,const wts::String &password);
	void MsgPong(const wts::String &target);
public:
	uint64_t last_ping;
    IRCClient(wts::Observer *obs,const ServerConfig &conf);
	void SendMsg(const wts::String &command,const Parameter &param,const wts::String &message="");
    Endpoint *next_;
};

wts::String Encode(const wts::String &str);

