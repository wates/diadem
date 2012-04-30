
#include <wts/container.h>

struct Query
{
	virtual void Insert(const wts::String &table)=0;
	virtual void Replace(const wts::String &table)=0;
	virtual void Update(const wts::String &table)=0;
	virtual void Select(const wts::String &table)=0;
	virtual void Delete(const wts::String &table)=0;
	virtual void OrderBy(const wts::String &key,bool desc=false)=0;
	virtual void Limit(int start,int records)=0;
	virtual wts::String& Where(const char *key)=0;
	virtual wts::String& operator[](const char *name)=0;
	virtual wts::String& Set(const char *name,bool Escape=true)=0;
};

typedef wts::Array<wts::Array<wts::String> > StorageResult;

class Storage
{
public:
	virtual bool Open(const char *address,int port,const char *database,const char *user,const char *password)=0;
	virtual inline ~Storage(){};

	virtual Query* NewQuery()=0;
	virtual bool Go(Query* q)=0;
	virtual void ClearContent(const wts::String &table)=0;
	virtual StorageResult &Result()=0;
};


Storage* CreateStorage();
