
#ifdef _WIN32

#include "storage.h"

struct QueryBody
	:public Query
{
	std::string dmy;
	void Insert(const std::string &table)
	{
	}
	std::string &operator[](const char *name)
	{
		return dmy;
	}
	std::string& Set(const char *name,bool Escape=true)
	{
		return dmy;
	}
	void OrderBy(const std::string &key,bool desc=false)
	{
	}
	void Limit(int low,int high)
	{
	}

	void Update(const std::string &name)
	{
	}
	void Replace(const std::string &name)
	{
	}
	void Select(const std::string &name)
	{
	}
	void Delete(const std::string &name)
	{
	}
	std::string& Where(const char *key)
	{
		return dmy;
	}
};

class StorageBody
	:public Storage
{
	bool Open()
	{
		return true;
	}
	void ClearContent(const std::string &table)
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
