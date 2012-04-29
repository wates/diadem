
#include <string>
#include <vector>

struct Query
{
	virtual void Insert(const std::string &table)=0;
	virtual void Replace(const std::string &table)=0;
	virtual void Update(const std::string &table)=0;
	virtual void Select(const std::string &table)=0;
	virtual void Delete(const std::string &table)=0;
	virtual void OrderBy(const std::string &key,bool desc=false)=0;
	virtual void Limit(int start,int records)=0;
	virtual std::string& Where(const char *key)=0;
	virtual std::string& operator[](const char *name)=0;
	virtual std::string& Set(const char *name,bool Escape=true)=0;
};

typedef std::vector<std::vector<std::string> > StorageResult;

class Storage
{
public:
	virtual bool Open()=0;
	virtual inline ~Storage(){};

	virtual Query* NewQuery()=0;
	virtual bool Go(Query* q)=0;
	virtual void ClearContent(const std::string &table)=0;
	virtual StorageResult &Result()=0;
};


Storage* CreateStorage();
