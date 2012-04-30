#include "diadem.h"
#include "storage.h"

#include <jpncode/jpncode.h>
#include <wts/system.h>

#include <iostream>
#include <fstream>

std::ofstream logger;
Storage *storage;

wts::String Decode(const wts::String &str)
{
	jpncode::unicode *uni;
	jpncode::Result unires;
	{
		unires=jpncode::utf8_unicode_charactors(str.Data());
		uni=new jpncode::unicode[unires.charactors+1];
		jpncode::utf8_decode(str.Data(),uni);
		uni[unires.charactors]=0;
	}

	wts::String ret;
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

			ret.Append(0x1b);
			ret.Append(0x24);
			ret.Append(0x42);
			ret+=enc;
			ret.Append(0x1b);
			ret.Append(0x28);
			ret.Append(0x42);

			delete[]enc;
			uni[i]=tempc;
			i--;
		}
		else
		{
			ret.Append(uni[i]&0x7f);
		}
	}
	delete uni;
	return ret;
}

wts::String Encode(const wts::String &str)
{
	wts::String ret;
	int i;
	for(i=0;i<(int)str.Size()-6;i++)
	{
		if(str[i+0]==0x1b&&
			str[i+1]==0x24&&
			str[i+2]==0x42)
		{
			i+=3;
			size_t start=i;

			for(;i<(int)str.Size()-2;i++)
			{
				if(str[i+0]==0x1b&&
					str[i+1]==0x28&&
					str[i+2]==0x42)
				{
					size_t end=i;
					wts::String from;
                    from.Assign(str.Data()+start,end-start);
					jpncode::Result res;
					res=jpncode::jis0208_unicode_charactors(from.Data());
					jpncode::unicode *uni=new jpncode::unicode[res.charactors+1];
					res=jpncode::jis0208_decode(from.Data(),uni);
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
			ret.Append(str[i]);
		}
	}
	ret.Append(str.Data()+i);
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

	printf("target: %s:%d\n",config.address.Data(),config.port);
	tcp->SetTarget(config.address.Data(),config.port&0xffff);
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
    UNUSED(something);

	if(!enable)
		return;

	if(ST_PASS==status)
	{
		if(config.password.Size())
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
		for(int i=0;i<(int)config.auto_channel.Size();i++)
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
		for(int i=0;i<storage->Result().Size();i++)
		{
			Parameter param=SplitParam(storage->Result()[i][2]);
			SendMsg(storage->Result()[i][1],param,storage->Result()[i][3]);
		}

		storage->ClearContent("queue");
	}
}

void IRCClient::SendMsg(const wts::String &command,const Parameter &param,const wts::String &message)
{
	wts::String line;
    line.Append(command);
	for(int i=0;i<(int)param.Size();i++)
	{
		line+=" ";
        line.Append(Decode(param[i]));
	}
	if(message.Size())
    {
		line+=" :";
        line.Append(Decode(message));
    }
    std::cout<<"SendMsg=\""<<line.Data()<<"\""<<std::endl;
	if(command=="PRIVMSG")
	{
		this->Transfer(reinterpret_cast<const uint8_t*>(line.Data()),line.Size());
	}
	line+="\r\n";
	send_buf->Transfer(reinterpret_cast<const uint8_t*>(line.Data()),line.Size());
}

void IRCClient::MsgPass(const wts::String &pass)
{
	Parameter p;
	p.Push(pass);
	SendMsg("PASS",p);
}

void IRCClient::MsgNick(const wts::String &nick)
{
	Parameter p;
	p.Push(nick);
	SendMsg("NICK",p);
}

void IRCClient::MsgUser(const wts::String &username,const wts::String &realname)
{
	Parameter p;
	p.Push(username);
	p.Push("*");
	p.Push("*");
	SendMsg("USER",p,realname);
}

void IRCClient::MsgJoin(const wts::String &name,const wts::String &password)
{
	Parameter p;
	p.Push(name);
	if(password.Size())
		p.Push(password);
	SendMsg("JOIN",p);
}

void IRCClient::MsgPong(const wts::String &target)
{
	Parameter p;
	p.Push(target);
	SendMsg("PONG",p);
}

IRCClient::Nickname IRCClient::SplitNickname(wts::String msg)
{
	Nickname nc;
	int exc=msg.Find('!');
	if(-1 != exc)
	{
		nc.nick.Assign(msg.Data(),exc);
		int at=msg.Find('@',exc);
		if(-1 != at)
		{
			nc.name.Assign(msg.Data()+exc+1,at-(exc+1));
			nc.address=msg.Data()+at+1;
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

IRCClient::Parameter IRCClient::SplitParam(wts::String msg)
{
	Parameter p;
	while(msg.Size())
	{
		wts::String str;
		unsigned int n=msg.Find(' ');
		if(-1 == n)
		{
			str=msg;
			msg.Clear();
		}
		else
		{
			str.Assign(msg.Data(),n);
			msg.Erase(0,n+1);
		}
		p.Push(str);
	}
	return p;
}

int IRCClient::Transfer(const uint8_t *buffer,int length)
{
	if(length<1)
		return length;
	wts::String msg;
	msg.Assign(reinterpret_cast<const char*>(buffer),length);
    logger<<Encode(msg).Data()<<std::endl;

	wts::String prefix;
	wts::String command;
	wts::Array<wts::String> param;
	if(msg[0]==':')
	{
		//prefix
		unsigned int n=msg.Find(' ');
		if(-1 == n)
		{
			prefix=msg;
			msg.Clear();
		}
		else
		{
			prefix.Assign(msg.Data()+1,n-1);
			msg.Erase(0,n+1);
		}
	}
	bool bcom=false;
	while(msg.Size())
	{
		wts::String str;
		if(msg[0]==':')
		{
			str=msg.Data()+1;
			msg.Clear();
		}
		else
		{
			unsigned int n=msg.Find(' ');
			if(-1 == n)
			{
				str=msg;
				msg.Clear();
			}
			else
			{
				str.Assign(msg.Data(),n);
				msg.Erase(0,n+1);
			}
		}
		if(!bcom)
		{
			command=str;
			bcom=true;
		}
		else
		{
			param.Push(Encode(str));
		}
	}

	last_ping=wts::GetTime();

	if(command=="375")
	{
		//start of motd
		MOTD.Clear();
		if(param.Size()>1)
			MOTD_first=param[1];
	}
	else if(command=="372" && param.Size()>1)
	{
		//motd msg
        MOTD.Append(param[1]);
        MOTD+="\r\n";
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
	else if(command=="JOIN"&&param.Size()>0)
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
			if(storage->Result().Size()==0)
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
	else if((command=="PART"||command=="KICK")&&param.Size()>0)
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
	else if(command=="332"&&param.Size()>2)
	{
		Query &q=*storage->NewQuery();
		q.Update("channel");
		q.Where("name")=param[1];
		q["topic"]=param[2];
		storage->Go(&q);
	}
	else if(command=="353"&&param.Size()>2)
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

			for(int i=0;i<(int)names.Size();i++)
			{
				bool is_operator=false;
				//drop '@'
				if(names[i][0]=='@')
				{
					names[i].Erase(0,1);
					is_operator=true;
				}
				Query &q=*storage->NewQuery();
				q.Select("join_nick");
				q.Where("nick_name")=names[i];
				q.Where("channel_name")=param[2];
				storage->Go(&q);
				if(storage->Result().Size()==0)
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

			ch.names.Insert(0,names.Data(),names.Size());
		}
	}
	else if(command=="PING")
	{
		if(param.Size()==1)
		{
			MsgPong(param[0]);
		}
	}
	else if((command=="NOTICE"||command=="PRIVMSG" )&&param.Size()>1)
	{
		wts::String notice=command=="NOTICE"?"1":"0";
		Query &q=*storage->NewQuery();
		q.Insert("message");
		if(prefix.Size())
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
	else if(command=="NICK"&&param.Size()>0)
	{
		Nickname nc;
		nc=SplitNickname(prefix);
		Query &q=*storage->NewQuery();
		q.Update("join_nick");
		q.Where("nick_name")=nc.nick;
		q["nick_name"]=param[0];
		storage->Go(&q);
	}
	else if(command=="MODE"&&param.Size()>1)
	{
		wts::String &ch=param[0];
		wts::String mode=param[1];
		char type=mode[0];
		for(int i=0;i<mode.Size()-1&&i+2<param.Size();i++)
		{
			char c=mode[i+1];
			wts::String &target=param[i+2];
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
        numeric=atoi(command.Data());
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
		if(prefix.Size())
		{
			Nickname nc;
			nc=SplitNickname(prefix);
			q["prefix"]=nc.nick;
		}
		q["command"]=command;
		for(int i=0;i<param.Size();i++)
		{
			wts::String ss;
			ss<<"param"<<(i+1);
			q[ss.Data()]=param[i];
		}
		storage->Go(&q);
	}

	wts::BinaryPrint bp;
	return bp.Transfer(reinterpret_cast<const uint8_t*>(buffer),length);
}


bool ReadFile(wts::String filename,wts::Array<char> &out)
{
    std::ifstream f;
    f.open(filename.Data(),std::ios_base::in|std::ios_base::binary);
    if(!f.is_open())
        return false;
    f.seekg(0,std::ios::end);
    int sz=static_cast<int>(f.tellg());
    f.seekg(0,std::ios::beg);
    out.Resize(sz+1);
    f.read(&out.Front(),sz);
    f.close();
    out.Back()='\0';
    return true;
}

#ifndef _WIN32
#include <unistd.h>
#endif

int main(int argc,char **argv)
{
    wts::String config_path;

    if(2 == argc)
    {
        config_path=argv[1];
    }
    else
    {
        config_path="../../config.txt";
    }


	wts::Array<char> filedata;
    ReadFile(config_path.Data(),filedata);

    wts::JsonReader r={filedata.Data(),filedata.Data()+filedata.Size()};
	Config conf;
	if(!Convert(r,conf,""))
    {
        std::cout<<"syntax error -> "<<r.position;
        return -1;
    }


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
    if(storage->Open(conf.db_address.Data(),conf.db_port,conf.db_database.Data(),conf.db_user.Data(),conf.db_password.Data()))
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

	IRCClient *irc=new IRCClient(obs,conf.irc_server);

    irc->last_ping=wts::GetTime();
	for(;;)
	{
		obs->Update(0);
#ifndef _WIN32
		usleep(1000);
#endif
		if(irc->last_ping+1200*1000<wts::GetTime())
		{
			break;
		}
	}
	
	delete irc;


}
