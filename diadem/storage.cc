
#ifdef _WIN32

#include "storage.h"

struct QueryBody
	:public Query
{
	wts::String dmy;
	void Insert(const wts::String &table)
	{
	}
	wts::String &operator[](const char *name)
	{
		return dmy;
	}
	wts::String& Set(const char *name,bool Escape=true)
	{
		return dmy;
	}
	void OrderBy(const wts::String &key,bool desc=false)
	{
	}
	void Limit(int low,int high)
	{
	}

	void Update(const wts::String &name)
	{
	}
	void Replace(const wts::String &name)
	{
	}
	void Select(const wts::String &name)
	{
	}
	void Delete(const wts::String &name)
	{
	}
	wts::String& Where(const char *key)
	{
		return dmy;
	}
};

class StorageBody
	:public Storage
{
	bool Open(const char *address,int port,const char *database,const char *user,const char *password)
	{
		return true;
	}
	void ClearContent(const wts::String &table)
	{
	}
	Query* NewQuery()
	{
		return new QueryBody;
	}
	bool Go(Query*q)
	{
		delete(QueryBody*)q;
		return true;
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
