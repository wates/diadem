
#ifdef _WIN32

#include "storage.h"

struct QueryBody
	:public Query
{
	wts::String dmy;
	void Insert(const wts::String &)
	{
	}
	void OrderBy(const wts::String &,bool)
	{
	}
	void Limit(int ,int )
	{
	}

	void Update(const wts::String &)
	{
	}
	void Replace(const wts::String &)
	{
	}
	void Select(const wts::String &)
	{
	}
	void Delete(const wts::String &)
	{
	}
    void Where(const char *,const wts::String&)
	{
	}
	void Set(const char *,const wts::String&,bool)
	{
	}
};

class StorageBody
	:public Storage
{
	bool Open(const char *,int ,const char *,const char *,const char *)
	{
		return true;
	}
	void ClearContent(const wts::String &)
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
