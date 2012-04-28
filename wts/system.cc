
#include "system.h"

#include <stdio.h>

namespace wts
{
    bool ReadFile(const char *filename,RawArray<char> &out)
    {
        FILE *f;
        f=fopen(filename,"rb");
        if(!f)
            return false;
        fseek(f,0,SEEK_END);
        int len=(int)ftell(f);
        fseek(f,0,SEEK_SET);
        out.Resize(len);
        fread(out.Data(),1,len,f);
        fclose(f);
        return true;
    }

    bool WriteFile(const char *filename,RawArray<char> &in)
    {
        FILE *f;
        f=fopen(filename,"wb");
        if(!f)
            return false;
        fwrite(in.Data(),1,in.Size(),f);
        fclose(f);
        return true;
    }

	static int g_indent=0;
	static int g_log_count=0;

	DebugTrace::DebugTrace(const char *func_name)
	{
		indent=g_indent++;
		strcpy(name,func_name);
		char buf[1024];
		memset(buf,' ',indent);
		sprintf(buf+indent,"in  %06d: %s\n",g_log_count++,name);
		LogPut(buf);
	}

	DebugTrace::~DebugTrace()
	{
		g_indent--;
		char buf[1024];
		memset(buf,' ',indent);
		sprintf(buf+indent,"out %06d: %s\n",g_log_count++,name);
		LogPut(buf);
	}
}
