
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

	wts::String table;
	typedef wts::OrderedMap<wts::String,wts::String> NameVal;
	typedef wts::OrderedMap<wts::String,bool> NameBool;
	NameVal values;
	NameVal wheres;
	NameBool values_escape;
	wts::String order_key;
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
	void Insert(const wts::String &table)
	{
		this->table=table;
		com=COM_INSERT;
	}
	wts::String &operator[](const char *name)
	{
		return Set(name);
	}
	void Update(const wts::String &table)
	{
		com=COM_UPDATE;
		this->table=table;
	}
	void Replace(const wts::String &table)
	{
		com=COM_REPLACE;
		this->table=table;
	}
	void Select(const wts::String &table)
	{
		com=COM_SELECT;
		this->table=table;
	}
	void Delete(const wts::String &table)
	{
		com=COM_DELETE;
		this->table=table;
	}
	wts::String& Where(const char *key)
	{
		return wheres[key];
	}
	wts::String& Set(const char *name,bool Escape=true)
	{
		values_escape[name]=Escape;
		return values[name];
	}
	wts::String Escape(const wts::String &in)
	{
		char *buf=new char[in.Size()*2+1];
		mysql_real_escape_string(sql,buf,in.Data(),in.Size());
		wts::String ret(buf);
		delete buf;
		return ret;
	}
	void OrderBy(const wts::String &key,bool desc=false)
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
		wts::String query;
		if(com==COM_INSERT)
		{
			query<<"insert into "<<table<<" (";
			for(NameVal::Iterator i(values);i;++i)
			{
                if(&(*i) != &values.Front())
					query+=",";
                query<<Escape(i->key);
			}
			query<<") values (";
			for(NameVal::Iterator i(values);i;++i)
			{
                if(&(*i) != &values.Front())
					query<<",";
                if(values_escape[i->key])
					query<<"'"<<Escape(i->value)<<"'";
				else
					query<<i->value;
			}
			query<<")";
		}
		if(com==COM_REPLACE)
		{
			query<<"replace into "<<table<<" (";
			for(NameVal::Iterator i(values);i;++i)
			{
                if(&(*i) != &values.Front())
					query<<",";
				query<<Escape(i->key);
			}
			query<<") values (";
			for(NameVal::Iterator i(values);i;++i)
			{
                if(&(*i) != &values.Front())
					query<<",";
				if(values_escape[i->key])
					query<<"'"<<Escape(i->value)<<"'";
				else
					query<<i->value;
			}
			query<<")";
		}
		if(com==COM_UPDATE)
		{
			query<<"update "<<table<<" set ";
			for(NameVal::Iterator i(values);i;++i)
			{
                if(&(*i) != &values.Front())
					query<<",";
				query<<Escape(i->key)<<"=";
				if(values_escape[i->key])
					query<<"'"<<Escape(i->value)<<"'";
				else
					query<<i->value;
			}
			query<<" where ";
			for(NameVal::Iterator i(wheres);i;++i)
			{
                if(&(*i) != &wheres.Front())
					query<<" and ";
				query<<Escape(i->key)<<"='"<<Escape(i->value)<<"'";
			}
		}
		if(com==COM_SELECT)
		{
			query<<"select * from "<<table;
			if(wheres.Size())
			{
				query<<" where ";
    			for(NameVal::Iterator i(wheres);i;++i)
				{
                    if(&(*i) != &wheres.Front())
						query<<" and ";
					query<<Escape(i->key)<<"='"<<Escape(i->value)<<"'";
				}
			}
			if(order_key.Size())
			{
				query<<" order by "<<order_key;
				if(order_desc)
					query<<" desc";
			}
			if(limit_records)
			{
				query<<" limit "<<limit_start<<","<<limit_records;
			}
		}
		if(com==COM_DELETE)
		{
			query<<"delete from "<<table;
			if(wheres.Size())
			{
				query<<" where ";
    			for(NameVal::Iterator i(wheres);i;++i)
				{
                    if(&(*i) != &wheres.Front())
						query<<" and ";
					query<<Escape(i->key)<<"='"<<Escape(i->value)<<"'";
				}
			}
		}
		mysql_query(sql,query.Data());
		{
			wts::String err=mysql_error(sql);
			if(err.Size())
			{
				wts::String errq;
				errq<<"insert into query_error (query,message) values ('";
				errq<<Escape(query)<<"','"<<Escape(err)<<"')";
				mysql_query(sql,errq.Data());
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

	bool Open(const char *address,int port,const char *database,const char *user,const char *password)
	{
		sql=mysql_init(0);
		if(!mysql_real_connect(sql,address,user,password,database,port,0,0))
		{
            std::cout<<"cannt connect to mysqld, \""<<address<<":"<<port<<"\".\""<<database<<"\""<<std::endl;
			return false;
		}
		mysql_query(sql,"set names utf8");
		mysql_character_set_name(sql);
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
		sres.Clear();
		if((res=mysql_store_result(sql)))
		{
			unsigned int fields=mysql_num_fields(res);
			wts::Array<wts::String> rec;
			rec.Resize(fields);
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
				sres.Push(rec);
			}
			mysql_free_result(res);
		}
		delete qb;
		return true;
	}
	void ClearContent(const wts::String &table)
	{
		wts::String q;
		q<<"truncate table "<<table;
		mysql_query(sql,q.Data());
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
