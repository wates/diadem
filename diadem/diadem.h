
#include <wts/pipe_tcp.h>
#include "jsonconv.h"
#include <map>

struct ConfigChannel
{
	std::string name;
	std::string password;
};

JSON_CONVERT_BEGIN(ConfigChannel)
JSON_CONVERT_MEMBER(name)
JSON_CONVERT_MEMBER(password)
JSON_CONVERT_END

struct ServerConfig
{
	std::string server_address;
	int port;
	std::string password;
	std::string nick;
	std::string username;
	std::string realname;
	std::vector<ConfigChannel> auto_channel;
};

JSON_CONVERT_BEGIN(ServerConfig)
JSON_CONVERT_MEMBER(server_address)
JSON_CONVERT_MEMBER(port)
JSON_CONVERT_MEMBER(password)
JSON_CONVERT_MEMBER(nick)
JSON_CONVERT_MEMBER(username)
JSON_CONVERT_MEMBER(realname)
JSON_CONVERT_MEMBER(auto_channel)
JSON_CONVERT_END

struct Config
{
    std::string db_address;
    int db_port;
    std::string db_database;
    std::string db_user;
    std::string db_password;
    ServerConfig irc_server;
};

JSON_CONVERT_BEGIN(Config)
JSON_CONVERT_MEMBER(db_address)
JSON_CONVERT_MEMBER(db_port)
JSON_CONVERT_MEMBER(db_database)
JSON_CONVERT_MEMBER(db_user)
JSON_CONVERT_MEMBER(db_password)
JSON_CONVERT_MEMBER(irc_server)
JSON_CONVERT_END



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
	typedef std::vector<std::string> Parameter;

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
		std::string nick;
		std::string name;
		std::string address;
	};

	struct Channel
	{
		char type;
		std::vector<std::string> names;
	};

	std::map<std::string,Channel> channels;

public:
    inline void SetNext(Endpoint*p){next_=p;}
    inline Endpoint*Next(){return next_;}

	//irc status

	std::string MOTD_first;
	std::string MOTD;


    // from endpoint
	void Open();
	void Close();
	int Transfer(const uint8_t *buffer,int length);

    //from subject
	void Update(int something);

	Nickname SplitNickname(std::string msg);
	Parameter SplitParam(std::string msg);

	void MsgPass(const std::string &pass);
	void MsgNick(const std::string &nick);
	void MsgUser(const std::string &username,const std::string &realname);
	void MsgJoin(const std::string &name,const std::string &password);
	void MsgPong(const std::string &target);
public:
	unsigned int last_ping;
    IRCClient(wts::Observer *obs,const ServerConfig &conf);
	void SendMsg(const std::string &command,const Parameter &param,const std::string &message="");
    Endpoint *next_;
};

std::string Encode(const std::string &str);

