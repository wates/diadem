#include "diadem.h"
#include "storage.h"

#include <jpncode/jpncode.h>

#include <time.h>
#include <iostream>
#include <fstream>

std::ofstream logger;
Storage *storage;

std::string Decode(const std::string &str)
{
	jpncode::unicode *uni;
	jpncode::Result unires;
	{
		unires=jpncode::utf8_unicode_charactors(str.c_str());
		uni=new jpncode::unicode[unires.charactors+1];
		jpncode::utf8_decode(str.c_str(),uni);
		uni[unires.charactors]=0;
	}

	std::string ret;
	int i;
	for(i=0;i<(int)unires.charactors;i++)
	{
		if(uni[i]>=128)
		{
			int start=i;
			for(;i<(int)unires.charactors;i++)
			{
				if(uni[i]<128)
					break;
			}
			jpncode::unicode tempc=uni[i];
			uni[i]=0;
			jpncode::Result res=jpncode::jis0208_multibyte_charactors(uni+start);
			char *enc=new char[res.charactors+1];
			jpncode::jis0208_encode(uni+start,enc);
			enc[res.charactors]=0;

			ret.push_back(0x1b);
			ret.push_back(0x24);
			ret.push_back(0x42);
			ret+=enc;
			ret.push_back(0x1b);
			ret.push_back(0x28);
			ret.push_back(0x42);

			delete[]enc;
			uni[i]=tempc;
			i--;
		}
		else
		{
			ret.push_back(uni[i]);
		}
	}
	delete uni;
	return ret;
}

std::string Encode(const std::string &str)
{
	std::string ret;
	int i;
	for(i=0;i<(int)str.size()-6;i++)
	{
		if(str[i+0]==0x1b&&
			str[i+1]==0x24&&
			str[i+2]==0x42)
		{
			i+=3;
			size_t start=i;

			for(;i<(int)str.size()-2;i++)
			{
				if(str[i+0]==0x1b&&
					str[i+1]==0x28&&
					str[i+2]==0x42)
				{
					size_t end=i;
					std::string from(str.c_str()+start,str.c_str()+end);
					jpncode::Result res;
					res=jpncode::jis0208_unicode_charactors(from.c_str());
					jpncode::unicode *uni=new jpncode::unicode[res.charactors+1];
					res=jpncode::jis0208_decode(from.c_str(),uni);
					uni[res.charactors]=0;

					res=jpncode::utf8_multibyte_charactors(uni);
					char *utf8=new char [res.charactors+1];
					res=jpncode::utf8_encode(uni,utf8);
					utf8[res.charactors]=0;
					ret+=utf8;
					delete utf8;
					delete uni;

					i+=2;
					break;
				}
			}
		}
		else
		{
			ret.push_back(str[i]);
		}
	}
	ret.insert(ret.end(),str.begin()+i,str.end());
	return ret;
}

void LineParser::Open()
{
	Next()->Open();
}

void LineParser::Close()
{
	Next()->Close();
}

int LineParser::Transfer(const uint8_t *buffer,int length)
{
	char crlf[]="\r\n";
	if(const uint8_t *hit=wts::memmem(buffer,length,crlf,2))
	{
		Next()->Transfer(buffer,(hit-buffer));
		return (int)(hit-buffer)+2;
	}

	return 0;
}

IRCClient::IRCClient(wts::Observer *obs,const ServerConfig &conf)
:config(conf)
,enable(false)
{
	send_buf=new wts::Buffer;
	tcp=new wts::TcpConnection;
	recv_buf=new wts::Buffer;
	lp=new LineParser;

	send_buf->SetNext(tcp);
	tcp->SetNext(recv_buf);
	recv_buf->SetNext(lp);
	lp->SetNext(this);

	obs->Append(send_buf);
	obs->Append(tcp);
	obs->Append(recv_buf);
	obs->Append(this);

	printf("target: %s:%d\n",config.server_address.c_str(),config.port);
	tcp->SetTarget(config.server_address.c_str(),config.port);
	send_buf->Open();
}

void IRCClient::Open()
{
	enable=true;
	status=ST_PASS;

	std::cout<<"Open"<<std::endl;
}

void IRCClient::Close()
{
	enable=false;
	std::cout<<"Close"<<std::endl;
}

#include <ctime>

void IRCClient::Update(int something)
{
	if(!enable)
		return;

	static time_t last_time=time(0);
	if(last_time==time(0))
		return;
	last_time=time(0);

	if(ST_PASS==status)
	{
		if(config.password.size())
			MsgPass(config.password);
		status=ST_NICK;
	}
	else if(ST_NICK==status)
	{
		MsgNick(config.nick);
		status=ST_USER;
	}
	else if(ST_USER==status)
	{
		MsgUser(config.username,config.realname);
		status=ST_WAIT_MOTD;
	}
	else if(ST_JOIN_WAIT==status)
	{
		storage->ClearContent("join_nick");
		storage->ClearContent("channel");
		Query &q=*storage->NewQuery();
		q.Insert("event");
		q["prefix"]="IRCdb";
		q["command"]="STARTUP";
		storage->Go(&q);
		for(int i=0;i<(int)config.auto_channel.size();i++)
		{
			MsgJoin(config.auto_channel[i].name,
				config.auto_channel[i].password);
		}
		status=ST_READY;
	}
	else if(ST_READY==status)
	{
		Query &q=*storage->NewQuery();
		q.Select("queue");
		q.OrderBy("time");
		storage->Go(&q);
		for(int i=0;i<storage->Result().size();i++)
		{
			Parameter param=SplitParam(storage->Result()[i][2]);
			SendMsg(storage->Result()[i][1],param,storage->Result()[i][3]);
		}

		storage->ClearContent("queue");
	}
}

void IRCClient::SendMsg(const std::string &command,const Parameter &param,const std::string &message)
{
	std::string line;
	line+=command;
	for(int i=0;i<(int)param.size();i++)
	{
		line+=" "+Decode(param[i]);
	}
	if(message.size())
		line+=" :"+Decode(message);
	std::cout<<"SendMsg=\""<<line<<"\""<<std::endl;
	if(command=="PRIVMSG")
	{
		this->Transfer(reinterpret_cast<const uint8_t*>(line.c_str()),line.size());
	}
	line+="\r\n";
	send_buf->Transfer(reinterpret_cast<const uint8_t*>(line.c_str()),line.size());
}

void IRCClient::MsgPass(const std::string &pass)
{
	Parameter p;
	p.push_back(pass);
	SendMsg("PASS",p);
}

void IRCClient::MsgNick(const std::string &nick)
{
	Parameter p;
	p.push_back(nick);
	SendMsg("NICK",p);
}

void IRCClient::MsgUser(const std::string &username,const std::string &realname)
{
	Parameter p;
	p.push_back(username);
	p.push_back("*");
	p.push_back("*");
	SendMsg("USER",p,realname);
}

void IRCClient::MsgJoin(const std::string &name,const std::string &password)
{
	Parameter p;
	p.push_back(name);
	if(password.size())
		p.push_back(password);
	SendMsg("JOIN",p);
}

void IRCClient::MsgPong(const std::string &target)
{
	Parameter p;
	p.push_back(target);
	SendMsg("PONG",p);
}

IRCClient::Nickname IRCClient::SplitNickname(std::string msg)
{
	Nickname nc;
	int exc=msg.find('!');
	if(exc!=msg.npos)
	{
		nc.nick.assign(msg.c_str(),msg.c_str()+exc);
		int at=msg.find('@',exc);
		if(at!=msg.npos)
		{
			nc.name.assign(msg.c_str()+exc+1,msg.c_str()+at);
			nc.address.assign(msg.c_str()+at+1);
		}
	}
	Query &q=*storage->NewQuery();
	q.Replace("user");
	q["nick"]=nc.nick;
	q["realname"]=nc.name;
	q["hostname"]=nc.address;
	storage->Go(&q);
	return nc;
}

IRCClient::Parameter IRCClient::SplitParam(std::string msg)
{
	Parameter p;
	while(msg.size())
	{
		std::string str;
		unsigned int n=msg.find(' ');
		if(n==msg.npos)
		{
			str=msg;
			msg.clear();
		}
		else
		{
			str.assign(msg.c_str(),msg.c_str()+n);
			msg.erase(0,n+1);
		}
		p.push_back(str);
	}
	return p;
}

int IRCClient::Transfer(const uint8_t *buffer,int length)
{
	if(length<1)
		return length;
	std::string msg;
	msg.assign(buffer,buffer+length);
	logger<<Encode(msg)<<std::endl;

	std::string prefix;
	std::string command;
	std::vector<std::string> param;
	if(msg[0]==':')
	{
		//prefix
		unsigned int n=msg.find(' ');
		if(n==msg.npos)
		{
			prefix=msg;
			msg.clear();
		}
		else
		{
			prefix.assign(msg.c_str()+1,msg.c_str()+n);
			msg.erase(0,n+1);
		}
	}
	bool bcom=false;
	while(msg.size())
	{
		std::string str;
		if(msg[0]==':')
		{
			str=msg.c_str()+1;
			msg.clear();
		}
		else
		{
			unsigned int n=msg.find(' ');
			if(n==msg.npos)
			{
				str=msg;
				msg.clear();
			}
			else
			{
				str.assign(msg.c_str(),msg.c_str()+n);
				msg.erase(0,n+1);
			}
		}
		if(!bcom)
		{
			command=str;
			bcom=true;
		}
		else
		{
			param.push_back(Encode(str));
		}
	}

	last_ping=time(0);

	if(command=="375")
	{
		//start of motd
		MOTD.clear();
		if(param.size()>1)
			MOTD_first=param[1];
	}
	else if(command=="372" && param.size()>1)
	{
		//motd msg
		MOTD+=param[1]+"\r\n";
	}
	else if(command=="376")
	{
		//end of motd
		Query &q=*storage->NewQuery();
		q.Insert("motd");
		q["server"]=MOTD_first;
		q["message"]=MOTD;
		storage->Go(&q);

		status=ST_JOIN_WAIT;
	}
	else if(command=="JOIN"&&param.size()>0)
	{
		{
			Query &q=*storage->NewQuery();
			q.Replace("channel");
			q["name"]=param[0];
			storage->Go(&q);
		}
		{
			Nickname nc;
			nc=SplitNickname(prefix);
			Query &q=*storage->NewQuery();
			q.Select("join_nick");
			q.Where("nick_name")=nc.nick;
			q.Where("channel_name")=param[0];
			storage->Go(&q);
			if(storage->Result().size()==0)
			{
				Query &q=*storage->NewQuery();
				q.Insert("join_nick");
				q["nick_name"]=nc.nick;
				q["channel_name"]=param[0];
				storage->Go(&q);
			}
			else
			{
				Query &q=*storage->NewQuery();
				q.Update("join_nick");
				q.Where("nick_name")=nc.nick;
				q.Where("channel_name")=param[0];
				q.Set("time",false)="CURRENT_TIMESTAMP";
				storage->Go(&q);
			}
		}
	}
	else if((command=="PART"||command=="KICK")&&param.size()>0)
	{
		Nickname nc;
		nc=SplitNickname(prefix);
		Query &q=*storage->NewQuery();
		q.Delete("join_nick");
		q.Where("nick_name")=nc.nick;
		q.Where("channel_name")=param[0];
		storage->Go(&q);
	}
	else if(command=="QUIT")
	{
		Nickname nc;
		nc=SplitNickname(prefix);
		Query &q=*storage->NewQuery();
		q.Delete("join_nick");
		q.Where("nick_name")=nc.nick;
		storage->Go(&q);
	}
	else if(command=="332"&&param.size()>2)
	{
		Query &q=*storage->NewQuery();
		q.Update("channel");
		q.Where("name")=param[1];
		q["topic"]=param[2];
		storage->Go(&q);
	}
	else if(command=="353"&&param.size()>2)
	{
		//"( "=" / "*" / "@" ) <channel>
		//:[ "@" / "+" ] <nick> *( " " [ "@" / "+" ] <nick> )
		//- "@"はシークレットチャンネルに、"*"はプライベートチャンネルに、そ
		//して"="がほかのチャンネル(パブリックチャンネル)に使われます。
		char type=param[1][0];
		if(type==':')
		{
			//nick
			type=param[1][1];
		}
		else
		{
			Channel &ch=channels[param[2]];
			ch.type=type;
			Parameter names=SplitParam(param[3]);

			for(int i=0;i<(int)names.size();i++)
			{
				bool is_operator=false;
				//drop '@'
				if(names[i][0]=='@')
				{
					names[i].erase(0,1);
					is_operator=true;
				}
				Query &q=*storage->NewQuery();
				q.Select("join_nick");
				q.Where("nick_name")=names[i];
				q.Where("channel_name")=param[2];
				storage->Go(&q);
				if(storage->Result().size()==0)
				{
					Query &q=*storage->NewQuery();
					q.Insert("join_nick");
					if(is_operator)
						q["is_operator"]="1";
					q["nick_name"]=names[i];
					q["channel_name"]=param[2];
					storage->Go(&q);
				}
				else
				{
					Query &q=*storage->NewQuery();
					q.Update("join_nick");
					q.Where("nick_name")=names[i];
					q.Where("channel_name")=param[2];
					q.Set("time",false)="CURRENT_TIMESTAMP";
					storage->Go(&q);
				}
			}

			ch.names.insert(ch.names.begin(),names.begin(),names.end());
		}
	}
	else if(command=="PING")
	{
		if(param.size()==1)
		{
			MsgPong(param[0]);
		}
	}
	else if((command=="NOTICE"||command=="PRIVMSG" )&&param.size()>1)
	{
		std::string notice=command=="NOTICE"?"1":"0";
		Query &q=*storage->NewQuery();
		q.Insert("message");
		if(prefix.size())
		{
			Nickname nc;
			nc=SplitNickname(prefix);
			q["nick"]=nc.nick;
		}
		else
		{
			q["nick"]="";
		}
		//param[0] : channel or user
		//param[1] : message
		q["target"]=param[0];
		q["text"]=param[1];
		q["is_notice"]=notice;
		storage->Go(&q);

	}
	else if(command=="NICK"&&param.size()>0)
	{
		Nickname nc;
		nc=SplitNickname(prefix);
		Query &q=*storage->NewQuery();
		q.Update("join_nick");
		q.Where("nick_name")=nc.nick;
		q["nick_name"]=param[0];
		storage->Go(&q);
	}
	else if(command=="MODE"&&param.size()>1)
	{
		std::string &ch=param[0];
		std::string mode=param[1];
		char type=mode[0];
		for(int i=0;i<mode.length()-1&&i+2<param.size();i++)
		{
			char c=mode[i+1];
			std::string &target=param[i+2];
			if('o'==c)
			{
				if('+'==type)
				{
					Query &q=*storage->NewQuery();
					q.Update("join_nick");
					q.Where("nick_name")=target;
					q.Where("channel_name")=ch;
					q.Set("time",false)="CURRENT_TIMESTAMP";
					q["is_operator"]="1";
					storage->Go(&q);
				}
				else if('-'==type)
				{
					Query &q=*storage->NewQuery();
					q.Update("join_nick");
					q.Where("nick_name")=target;
					q.Where("channel_name")=ch;
					q.Set("time",false)="CURRENT_TIMESTAMP";
					q["is_operator"]="0";
					storage->Go(&q);
				}
			}

		}

	}

	int numeric=0;
	{
		std::stringstream ss;
		ss<<command;
		ss>>numeric;
	}
	if(command=="NICK"||
		command=="MODE"||
		command=="PART"||
		command=="TOPIC"||
		command=="QUIT"||
		command=="JOIN"||
		command=="INVITE"||
		command=="KICK"||
		command=="KILL"||
		command=="AWAY"||
		command=="PRIVMSG"||
		command=="NOTICE"||
		(numeric>=400&&numeric<=599))
	{
		Query &q=*storage->NewQuery();
		q.Insert("event");
		if(!prefix.empty())
		{
			Nickname nc;
			nc=SplitNickname(prefix);
			q["prefix"]=nc.nick;
		}
		q["command"]=command;
		for(int i=0;i<param.size();i++)
		{
			std::stringstream ss;
			ss<<"param"<<(i+1);
			q[ss.str().c_str()]=param[i];
		}
		storage->Go(&q);
	}

	wts::BinaryPrint bp;
	return bp.Transfer(reinterpret_cast<const uint8_t*>(buffer),length);
}


bool ReadFile(std::string filename,std::vector<char> &out)
{
    std::ifstream f;
    f.open(filename.c_str(),std::ios_base::in|std::ios_base::binary);
    if(!f.is_open())
        return false;
    f.seekg(0,std::ios::end);
    std::streamsize sz=f.tellg();
    f.seekg(0,std::ios::beg);
    out.resize(sz);
    f.read(&out.front(),sz);
    f.close();
    return true;
}

bool ReadFile(std::string filename,std::string &out)
{
    std::ifstream f;
    f.open(filename.c_str(),std::ios_base::in);
    if(!f.is_open())
        return false;
    while(!f.eof()&&f.peek())
        out.push_back(f.get());
    f.close();
    return true;
}

template<typename T>
bool ConvertRead(std::string &in,T &out)
{
    json::Reader r;
    return json::Convert(r,in,std::string(),out);
}
#ifndef _WIN32
#include <unistd.h>
#endif

#include "wts/system.h"

int main(int argc,char **argv)
{
    std::string config_path;

    if(2 == argc)
    {
        config_path=argv[1];
    }
    else
    {
        config_path="../config.txt";
    }

	std::string text;
    ReadFile(config_path.c_str(),text);
	ServerConfig conf;
	ConvertRead(text,conf);


	{
		int sock;
		wts::socket::StartupSocket();
		wts::socket::TcpSocket(sock);
		if(!wts::socket::Bind(sock,34567,0))
		{
			std::cout<<"Already boot."<<std::endl;
			wts::socket::CloseSocket(sock);
			return -1;
		}
	}

	std::cout<<"Startup.."<<std::endl;
	storage=CreateStorage();
	if(storage->Open())
	{
		std::cout<<"Storage open."<<std::endl;
	}
	else
	{
		std::cout<<"Storage fail."<<std::endl;
		return 0;
	}

	logger.open("log.txt");

    wts::Observer *obs=wts::CreateBasicObserver();

	IRCClient *irc=new IRCClient(obs,conf);

	irc->last_ping=time(0);
	for(;;)
	{
		obs->Update(0);
#ifndef _WIN32
		usleep(1000);
#endif
		if(irc->last_ping+1200<time(0))
		{
			break;
		}
	}
	
	delete irc;


}
