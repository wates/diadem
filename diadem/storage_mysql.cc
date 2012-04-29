
#ifndef _WIN32

#include "storage.h"
#include <mysql/mysql.h>

#include <map>
#include <sstream>
#include <iostream>

struct QueryBody
	:public Query
{
	MYSQL *sql;

	std::string table;
	typedef std::map<std::string,std::string> NameVal;
	typedef std::map<std::string,bool> NameBool;
	NameVal values;
	NameVal wheres;
	NameBool values_escape;
	std::string order_key;
	bool order_desc;
	int limit_start;
	int limit_records;
	enum Command
	{
		COM_INSERT,
		COM_UPDATE,
		COM_REPLACE,
		COM_SELECT,
		COM_DELETE
	}com;
	void Insert(const std::string &table)
	{
		this->table=table;
		com=COM_INSERT;
	}
	std::string &operator[](const char *name)
	{
		return Set(name);
	}
	void Update(const std::string &table)
	{
		com=COM_UPDATE;
		this->table=table;
	}
	void Replace(const std::string &table)
	{
		com=COM_REPLACE;
		this->table=table;
	}
	void Select(const std::string &table)
	{
		com=COM_SELECT;
		this->table=table;
	}
	void Delete(const std::string &table)
	{
		com=COM_DELETE;
		this->table=table;
	}
	std::string& Where(const char *key)
	{
		return wheres[key];
	}
	std::string& Set(const char *name,bool Escape=true)
	{
		values_escape[name]=Escape;
		return values[name];
	}
	std::string Escape(const std::string &in)
	{
		char *buf=new char[in.size()*2+1];
		mysql_real_escape_string(sql,buf,in.c_str(),in.size());
		std::string ret(buf);
		delete buf;
		return ret;
	}
	void OrderBy(const std::string &key,bool desc=false)
	{
		order_key=key;
		order_desc=desc;
	}
	void Limit(int start,int records)
	{
		limit_start=start;
		limit_records=records;
	}

	void Go()
	{
		std::string query;
		if(com==COM_INSERT)
		{
			query+="insert into "+table+" (";
			for(NameVal::iterator i=values.begin();i!=values.end();i++)
			{
				if(i!=values.begin())
					query+=",";
				query+=Escape(i->first);
			}
			query+=") values (";
			for(NameVal::iterator i=values.begin();i!=values.end();i++)
			{
				if(i!=values.begin())
					query+=",";
				if(values_escape[i->first])
					query+="'"+Escape(i->second)+"'";
				else
					query+=i->second;
			}
			query+=")";
		}
		if(com==COM_REPLACE)
		{
			query+="replace into "+table+" (";
			for(NameVal::iterator i=values.begin();i!=values.end();i++)
			{
				if(i!=values.begin())
					query+=",";
				query+=Escape(i->first);
			}
			query+=") values (";
			for(NameVal::iterator i=values.begin();i!=values.end();i++)
			{
				if(i!=values.begin())
					query+=",";
				if(values_escape[i->first])
					query+="'"+Escape(i->second)+"'";
				else
					query+=i->second;
			}
			query+=")";
		}
		if(com==COM_UPDATE)
		{
			query+="update "+table+" set ";
			for(NameVal::iterator i=values.begin();i!=values.end();i++)
			{
				if(i!=values.begin())
					query+=",";
				query+=Escape(i->first)+"=";
				if(values_escape[i->first])
					query+="'"+Escape(i->second)+"'";
				else
					query+=i->second;
			}
			query+=" where ";
			for(NameVal::iterator i=wheres.begin();i!=wheres.end();i++)
			{
				if(i!=wheres.begin())
					query+=" and ";
				query+=Escape(i->first)+"='"+Escape(i->second)+"'";
			}
		}
		if(com==COM_SELECT)
		{
			query+="select * from "+table;
			if(wheres.size())
			{
				query+=" where ";
				for(NameVal::iterator i=wheres.begin();i!=wheres.end();i++)
				{
					if(i!=wheres.begin())
						query+=" and ";
					query+=Escape(i->first)+"='"+Escape(i->second)+"'";
				}
			}
			if(!order_key.empty())
			{
				query+=" order by "+order_key;
				if(order_desc)
					query+=" desc";
			}
			if(limit_records)
			{
				std::stringstream ss;
				ss<<limit_start<<","<<limit_records;
				query+=" limit "+ss.str();
			}
		}
		if(com==COM_DELETE)
		{
			query+="delete from "+table;
			if(wheres.size())
			{
				query+=" where ";
				for(NameVal::iterator i=wheres.begin();i!=wheres.end();i++)
				{
					if(i!=wheres.begin())
						query+=" and ";
					query+=Escape(i->first)+"='"+Escape(i->second)+"'";
				}
			}
		}
		mysql_query(sql,query.c_str());
		{
			std::string err=mysql_error(sql);
			if(err.size())
			{
				std::string errq;
				errq+="insert into query_error (query,message) values ('";
				errq+=Escape(query)+"','"+Escape(err)+"')";
				mysql_query(sql,errq.c_str());
			}
		}
	}
	explicit QueryBody(MYSQL *sql)
	{
		this->limit_start=0;
		this->limit_records=0;
		this->sql=sql;
	}
};

class StorageBody
	:public Storage
{
	MYSQL *sql;

	bool Open()
	{
		sql=mysql_init(0);
		if(!mysql_real_connect(sql,"localhost","ircdb","","",3306,0,0))
		{
			return false;
		}
		mysql_query(sql,"set names utf8");
		const char *c=mysql_character_set_name(sql);
		return true;
	}
	Query* NewQuery()
	{
		return new QueryBody(sql);
	}
	bool Go(Query *q)
	{
		QueryBody *qb=(QueryBody*)q;
		qb->Go();
		MYSQL_RES *res;
		sres.clear();
		if(res=mysql_store_result(sql))
		{
			unsigned int fields=mysql_num_fields(res);
			std::vector<std::string> rec;
			rec.resize(fields);
			MYSQL_ROW arow;
			for(MYSQL_ROW row=mysql_fetch_row(res);row;row=mysql_fetch_row(res))
			{
				arow=row;
				for(unsigned int i=0;i<fields;i++)
				{
					if(*arow)
						rec[i]=*arow;
					arow++;
				}
				sres.push_back(rec);
			}
			mysql_free_result(res);
		}
		delete qb;
		return true;
	}
	void ClearContent(const std::string &table)
	{
		std::string q;
		q+="truncate table "+table;
		mysql_query(sql,q.c_str());
	}

	StorageResult sres;

	StorageResult &Result()
	{
		return sres;
	}

public:
	StorageBody()
	{
	}
	~StorageBody()
	{
	}
};

Storage* CreateStorage()
{
	return new StorageBody();
}

#endif
