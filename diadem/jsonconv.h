
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <cstdlib>

#include <wts/parser.h>

static void ReplaceAll(wts::String &str,const char *from,const char *to)
{
	size_t len=strlen(from);
	size_t lento=strlen(to);
	for(size_t i=str.find(from);i!=-1;i=str.find(from,i+lento))
		str.replace(i,len,to);
}

static void Unescape(wts::String &str)
{
	ReplaceAll(str,"\\\"","\"");
	ReplaceAll(str,"\\\\","\\");
	ReplaceAll(str,"\\/","/");
	ReplaceAll(str,"\\\b","\b");
	ReplaceAll(str,"\\\f","\f");
	ReplaceAll(str,"\\\n","\n");
	ReplaceAll(str,"\\\r","\r");
	ReplaceAll(str,"\\\t","\t");
}

static void Escape(wts::String &str)
{
	ReplaceAll(str,"\"","\\\"");
	ReplaceAll(str,"\\","\\\\");
	ReplaceAll(str,"/","\\/");
	ReplaceAll(str,"\b","\\\b");
	ReplaceAll(str,"\f","\\\f");
	ReplaceAll(str,"\n","\\\n");
	ReplaceAll(str,"\r","\\\r");
	ReplaceAll(str,"\t","\\\t");
}

static wts::String Escape(const wts::String &cstr)
{
	wts::String str=cstr;
	ReplaceAll(str,"\"","\\\"");
	ReplaceAll(str,"\\","\\\\");
	ReplaceAll(str,"/","\\/");
	ReplaceAll(str,"\b","\\\b");
	ReplaceAll(str,"\f","\\\f");
	ReplaceAll(str,"\n","\\\n");
	ReplaceAll(str,"\r","\\\r");
	ReplaceAll(str,"\t","\\\t");
	return str;
}

namespace jsp
{
    using namespace parser;

	typedef CharRange<'0','9'> tNum;
	typedef CharRange<'1','9'> tNonZeroNum;
	typedef Or<Rule<tNonZeroNum,Any<tNum> >,Char<'0'> > tUnsignedInt;
	typedef Rule<Option<Char<'+'> >,tUnsignedInt> tPositiveNum;
	typedef Rule<Char<'-'>,tUnsignedInt > tNegativeNum;
	typedef Or<tPositiveNum,tNegativeNum> tInt;

	typedef Or<tNum,CharRange<'a','f'>,CharRange<'A','F'> > tHex;

	typedef Rule<Char<'E','e'>,Option<Char<'+','-'> >,More<tNum> > tLog;
	typedef Rule<tInt,Option<tLog> > tLogInt;

	typedef Rule<tInt,Char<'.'>,More<tNum>,Option<tLogInt> > tFloat;

	typedef More<Or<Char<0x20>,Char<0x09>,Char<0x0d>,Char<0x0a> > > tS;
	typedef Option<tS> tOS;

	typedef Rule<Char<'\\'>,Or<Char<'"','\\','/','b','f','n','r','t'>,Rule<Char<'u'>,tHex,tHex,tHex,tHex> > > Escape;

	typedef Any<Or<Escape,Not<Char<'"','\\'> > > > tStringBody;

	struct KeyBody
		:public Action<tStringBody>
	{
		wts::String key;
		void Hit(const char *text,type &r)
		{
			key.assign(text);
			Unescape(key);
		}
	};

	typedef Rule<Char<'"'>,KeyBody,Char<'"'> > Key;

	struct valInt
		:public Action<tLogInt>
	{
		int val;
		void Hit(const char *text,type &r)
		{
			val=atoi(text);
		}
	};

	struct valFloat
		:public Action<tFloat>
	{
		float val;
		void Hit(const char *text,type &r)
		{
			val=(float)atof(text);
		}
	};

	struct valStringBody
		:public Action<tStringBody>
	{
		wts::String val;
		void Hit(const char *text,type &r)
		{
			val.assign(text);
			Unescape(val);
		}
	};
	typedef Rule<Char<'"'>,valStringBody,Char<'"'> > valString;

	typedef Rule<tOS,Option<Char<','> >,tOS,Key,tOS,Char<':'> > single;
	typedef Rule<Option<single>,tOS,valInt,tOS> intPair;
	typedef Rule<Option<single>,tOS,valFloat,tOS> floatPair;
	typedef Rule<Option<single>,tOS,valString,tOS> stringPair;

	typedef Rule<Option<single>,tOS,Char<'{'>,tOS> blockStart;
	typedef Rule<tOS,Char<'}'>,tOS> blockEnd;
	typedef Rule<tOS,Char<'['>,tOS> arrayStart;
	typedef Rule<tOS,Char<','>,tOS> arrayDot;
	typedef Rule<tOS,Char<']'>,tOS> arrayEnd;

}

namespace json
{
	struct Writer
	{
		int indent;
		wts::Array<int> members;

		Writer():indent(0){members.push_back(0);}
	};

	struct Reader
	{
	};

	inline
	bool Blockstart(Writer &c,wts::String &text,const wts::String &key)
	{
		text+="\r\n";
		for(int i=c.indent;i--;)
		{
			text+="\t";
		}
		if(c.members.back()++)
			text+=",";
		if(key.size())
		{
			text+="\"";
			text+=Escape(key);
			text+="\":";
		}
		text+="{\r\n";
		++c.indent;
		c.members.push_back(0);
		return true;
	}

	inline
		bool Blockstart(Reader &c,wts::String &text,const wts::String &key)
	{
		jsp::blockStart p;
		const char *l=text.c_str();
		const char *f=l;
		if(!p.Parse(l))
			return false;
		text.erase(0,l-f);
		return true;
	}

	inline
	bool Blockend(Writer &c,wts::String &text)
	{
		c.members.pop_back();
		--c.indent;
		for(int i=c.indent;i--;)
		{
			text+="\t";
		}
		text+="}\r\n";
		return true;
	}

	inline
	bool Blockend(Reader &c,wts::String &text)
	{
		jsp::blockEnd p;
		const char *l=text.c_str();
		const char *f=l;
		if(!p.Parse(l))
			return false;
		text.erase(0,l-f);
		return true;
	}

	template<bool dc,typename T>
	bool RawWrite(Writer &c,wts::String &text,const wts::String &key,T value)
	{
		wts::Stringstream ss;
		if(key.size())
		{
			for(int i=c.indent;i--;)
			{
				ss<<"\t";
			}
			if(c.members.back()++)
				ss<<",";
			ss<<"\""<<Escape(key)<<"\":";
		}
		if(dc)
			ss<<"\"";
		ss<<value;
		if(dc)
			ss<<"\"";
		if(key.size())
		{
			ss<<std::endl;
		}
		text+=ss.str();
		return true;
	}

	inline
	bool Convert(Writer &c,wts::String &text,const wts::String &key,const int value)
	{
		return RawWrite<false>(c,text,key,value);
	}

	inline
	bool Convert(Reader &c,wts::String &text,const wts::String &key,int &value)
	{
		return RawRead<jsp::intPair>(c,text,key,value);
	}

	inline
	bool Convert(Writer &c,wts::String &text,const wts::String &key,const float value)
	{
		return RawWrite<false>(c,text,key,value);
	}

	inline
	bool Convert(Reader &c,wts::String &text,const wts::String &key,float &value)
	{
		return RawRead<jsp::floatPair>(c,text,key,value);
	}

	inline
	bool Convert(Writer &c,wts::String &text,const wts::String &key,const wts::String &value)
	{
		return RawWrite<true>(c,text,key,Escape(value));
	}

	inline
	bool Convert(Reader &c,wts::String &text,const wts::String &key,wts::String &value)
	{
		return RawRead<jsp::stringPair>(c,text,key,value);
	}

	template<typename T>
	bool Convert(Writer &c,wts::String &text,const wts::String &key,wts::Array<T> &value)
	{
		for(int i=c.indent;i--;)
		{
			text+="\t";
		}
		if(c.members.back()++)
			text+=",";
		text+="\"";
		text+=Escape(key);
		text+="\":[";
		c.members.push_back(0);
		for(size_t i=0;i<value.size();i++)
		{
			Convert(c,text,"",value[i]);
		}
		c.members.pop_back();
		text+="]\r\n";
		return true;
	}

	template<typename T>
	bool Convert(Reader &c,wts::String &text,const wts::String &key,wts::Array<T> &value)
	{
		const char *l=text.c_str();
		const char *f=l;
		jsp::single sing;
		jsp::arrayStart as;
		jsp::arrayDot ad;
		jsp::arrayEnd ae;
		if(!sing.Parse(l))
			return false;
		if(!as.Parse(l))
			return false;
		for(int n=0;;n++)
		{
			if(n&&!ad.Parse(l))
				break;
			text.erase(0,l-f);
			T t;
			if(!Convert(c,text,"",t))
				break;
			f=l=text.c_str();
			value.push_back(t);
		}
		if(!ae.Parse(l))
			return false;
		text.erase(0,l-f);
		return true;
	}

	template<typename T>
	bool ConvertWrite(wts::String &out,T &in)
	{
		json::Writer w;
		return json::Convert(w,out,wts::String(),in);
	}

	template<typename T>
	bool ConvertRead(wts::String &in,T &out)
	{
		json::Reader r;
		return json::Convert(r,in,wts::String(),out);
	}

}


#define JSON_CONVERT_BEGIN(type)\
namespace json\
{\
	template<typename D>\
	bool Convert(D &dir,wts::String &text,const wts::String &key,type &value)\
	{\
		return Blockstart(dir,text,key)

#define JSON_CONVERT_MEMBER(member)\
	&& Convert(dir,text,#member,value.member)

#define JSON_CONVERT_END\
				&& Blockend(dir,text);\
	}\
}
