
#pragma warning(disable:4503)
#pragma warning(disable:4584)

namespace jsp
{
	struct end
	{
	};

	struct good
	{
		inline static bool Parse(const char *&text)
		{
			return true;
		}
	};

	template<int Any>
	struct bad
	{
		inline static bool Parse(const char *&text)
		{
			return false;
		}
	};

	template<typename Any,int num>
	struct Rename
		:virtual public Any
	{
		bool Parse(const char *&text)
		{
			return Any::Parse(text);
		}
	};


	template <char min,char max>
	struct CharRange
	{
		static bool Parse(const char *&text)
		{
			if(min<=*text&&max>=*text)
			{
				text++;
				return true;
			}
			return false;
		}
	};

#define template_char_prototype char c1,char c2=0,char c3=0,char c4=0,char c5=0,char c6=0,char c7=0,char c8=0
#define template_char_specialize char c2,char c3,char c4,char c5,char c6,char c7,char c8
#define template_char_use_next c2,c3,c4,c5,c6,c7,c8
#define template_char_use c1,template_char_use_next

	template<template_char_prototype>
	struct CharEqual
		:public CharEqual<template_char_use_next>
	{
		static bool Equal(const char *&text)
		{
			return c1==*text||CharEqual<template_char_use_next>::Equal(text);
		}
	};

	template<template_char_specialize>
	struct CharEqual<0,template_char_use_next>
	{
		static bool Equal(const char *&text)
		{
			return false;
		}
	};

	template<template_char_prototype>
	struct Char
		:public CharEqual<template_char_use>
	{
		static bool Parse(const char *&text)
		{
			if(CharEqual<template_char_use>::Equal(text))
			{
				text++;
				return true;
			}
			return false;
		}
	};

	template<template_char_prototype>
	struct StringEqual
		:public StringEqual<template_char_use_next>
	{
		static const int length=1+StringEqual<template_char_use_next>::length;
		static bool Equal(const char *c)
		{
			return c1==*c&&StringEqual<template_char_use_next>::Equal(c+1);
		}
	};

	template<template_char_specialize>
	struct StringEqual<0,template_char_use_next>
	{
		static const int length=0;
		static bool Equal(const char *c)
		{
			return true;
		}
	};

	template<template_char_prototype>
	struct String
		:public StringEqual<template_char_use>
	{
		static bool Parse(const char *&text)
		{
			const char *src=text;
			if(StringEqual<template_char_use>::Equal(text))
			{
				text+=StringEqual<template_char_use>::length;
				return true;
			}
			text=src;
			return false;
		}
	};

	template<template_char_prototype>
	struct CharNotEqual
		:public CharNotEqual<template_char_use_next>
	{
		static bool NotEqual(const char &c)
		{
			return c1!=c&&CharNotEqual<template_char_use_next>::NotEqual(c);
		}
	};

	template<template_char_specialize>
	struct CharNotEqual<0,template_char_use_next>
	{
		static bool NotEqual(const char &c)
		{
			return c!=0;
		}
	};

	template<template_char_prototype>
	struct CharNot
		:public CharNotEqual<template_char_use>
	{
		static bool Parse(const char *&text)
		{
			if(CharNotEqual<template_char_use>::NotEqual(*text))
			{
				text++;
				return true;
			}
			return false;
		}
	};

	template <typename T>
	struct Option
		:public T
	{
		bool Parse(const char *&text)
		{
			T::Parse(text);
			return true;
		}
	};

	//0å¬à»è„Ç…ÉqÉbÉg
	template<typename t>
	struct Any
		:public t
	{
		bool Parse(const char *&text)
		{
			while(t::Parse(text));
			return true;
		}
	};

	//1å¬à»è„Ç…ÉqÉbÉg
	template<typename t>
	struct More
		:public t
	{
		bool Parse(const char *&text)
		{
			if(t::Parse(text))
			{
				while(t::Parse(text));
				return true;
			}
			return false;
		}
	};

	//template <typename primary,typename t2=end,typename t3=end,typename t4=end>
	//struct And
	//	:public primary
	//	,public And<t2,t3,t4>
	//{
	//	bool Parse(const char *&text)
	//	{
	//		const char *src=text;
	//		if(primary::Parse(text))
	//		{
	//			if(And<t2,t3,t4>::Parse(src))
	//				return true;
	//			text=src;
	//		}
	//		return false;
	//	}
	//};

	//template<typename primary,typename t3,typename t4>
	//struct And<primary,end,t3,t4>
	//	:public primary
	//{
	//	bool Parse(const char *&text)
	//	{
	//		return primary::Parse(text);
	//	}
	//};

	template<typename t>
	struct Not
		:public t
	{
		bool Parse(const char *&text)
		{
			const char *src=text;
			if(!t::Parse(src))
			{
				text++;
				return true;
			}
			else
				return false;
		}
	};

	template<typename t1,typename t2=bad<2>,typename t3=bad<3>,typename t4=bad<4>,
		typename t5=bad<5>,typename t6=bad<6>,typename t7=bad<7>,typename t8=bad<8> >
	struct Or
		:virtual public t1,virtual public t2,virtual public t3,virtual public t4
		,virtual public t5,virtual public t6,virtual public t7,virtual public t8
	{
		bool Parse(const char *&text)
		{
			const char *src=text;
			if(t1::Parse(text))
				return true;
			text=src;
			if(t2::Parse(text))
				return true;
			text=src;
			if(t3::Parse(text))
				return true;
			text=src;
			if(t4::Parse(text))
				return true;
			text=src;
			if(t5::Parse(text))
				return true;
			text=src;
			if(t6::Parse(text))
				return true;
			text=src;
			if(t7::Parse(text))
				return true;
			text=src;
			if(t8::Parse(text))
				return true;
			text=src;
			return false;
		}
	};

	template<typename t1,typename t2=good,typename t3=good,typename t4=good,
		typename t5=good,typename t6=good,typename t7=good,typename t8=good >
	struct Rule
		:virtual public Rename<t1,1>,virtual public Rename<t2,2>,virtual public Rename<t3,3>,virtual public Rename<t4,4>
		,virtual public Rename<t5,5>,virtual public Rename<t6,6>,virtual public Rename<t7,7>,virtual public Rename<t8,8>
	{
		bool Parse(const char *&text)
		{
			const char *src=text;
			if(Rename<t1,1>::Parse(text)&&
				Rename<t2,2>::Parse(text)&&
				Rename<t3,3>::Parse(text)&&
				Rename<t4,4>::Parse(text)&&
				Rename<t5,5>::Parse(text)&&
				Rename<t6,6>::Parse(text)&&
				Rename<t7,7>::Parse(text)&&
				Rename<t8,8>::Parse(text))
				return true;
			text=src;
			return false;
		}
	};

	template<typename rule>
	struct Action
	{
		typedef rule type;
		virtual void Hit(const char *text,type &r)=0;
		bool Parse(const char *&text)
		{
			const char *src=text;
			type r;
			if(r.Parse(text))
			{
				const char t=*text;
				*const_cast<char*>(text)=0;
				Hit(src,r);
				*const_cast<char*>(text)=t;
				return true;
			}
			return false;
		}
	};

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

}

#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <cstdlib>

static void ReplaceAll(std::string &str,const char *from,const char *to)
{
	size_t len=strlen(from);
	size_t lento=strlen(to);
	for(size_t i=str.find(from);i!=-1;i=str.find(from,i+lento))
		str.replace(i,len,to);
}

static void Unescape(std::string &str)
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

static void Escape(std::string &str)
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

static std::string Escape(const std::string &cstr)
{
	std::string str=cstr;
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
	typedef Rule<Char<'\\'>,Or<Char<'"','\\','/','b','f','n','r','t'>,Rule<Char<'u'>,tHex,tHex,tHex,tHex> > > Escape;

	typedef Any<Or<Escape,Not<Char<'"','\\'> > > > tStringBody;

	struct KeyBody
		:public Action<tStringBody>
	{
		std::string key;
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
		std::string val;
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
		std::vector<int> members;

		Writer():indent(0){members.push_back(0);}
	};

	struct Reader
	{
	};

	inline
	bool Blockstart(Writer &c,std::string &text,const std::string &key)
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
		bool Blockstart(Reader &c,std::string &text,const std::string &key)
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
	bool Blockend(Writer &c,std::string &text)
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
	bool Blockend(Reader &c,std::string &text)
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
	bool RawWrite(Writer &c,std::string &text,const std::string &key,T value)
	{
		std::stringstream ss;
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

	template<typename Parser,typename Val>
	bool RawRead(Reader &c,std::string &text,const std::string &key,Val &value)
	{
		Parser p;
		const char *l=text.c_str();
		const char *f=l;
		if(!p.Parse(l))
			return false;
		value=p.val;
		text.erase(0,l-f);
		return true;
	}

	inline
	bool Convert(Writer &c,std::string &text,const std::string &key,const int value)
	{
		return RawWrite<false>(c,text,key,value);
	}

	inline
	bool Convert(Reader &c,std::string &text,const std::string &key,int &value)
	{
		return RawRead<jsp::intPair>(c,text,key,value);
	}

	inline
	bool Convert(Writer &c,std::string &text,const std::string &key,const float value)
	{
		return RawWrite<false>(c,text,key,value);
	}

	inline
	bool Convert(Reader &c,std::string &text,const std::string &key,float &value)
	{
		return RawRead<jsp::floatPair>(c,text,key,value);
	}

	inline
	bool Convert(Writer &c,std::string &text,const std::string &key,const std::string &value)
	{
		return RawWrite<true>(c,text,key,Escape(value));
	}

	inline
	bool Convert(Reader &c,std::string &text,const std::string &key,std::string &value)
	{
		return RawRead<jsp::stringPair>(c,text,key,value);
	}

	template<typename T>
	bool Convert(Writer &c,std::string &text,const std::string &key,std::vector<T> &value)
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
	bool Convert(Reader &c,std::string &text,const std::string &key,std::vector<T> &value)
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
	bool ConvertWrite(std::string &out,T &in)
	{
		json::Writer w;
		return json::Convert(w,out,std::string(),in);
	}

	template<typename T>
	bool ConvertRead(std::string &in,T &out)
	{
		json::Reader r;
		return json::Convert(r,in,std::string(),out);
	}

}


#define JSON_CONVERT_BEGIN(type)\
namespace json\
{\
	template<typename D>\
	bool Convert(D &dir,std::string &text,const std::string &key,type &value)\
	{\
		return Blockstart(dir,text,key)

#define JSON_CONVERT_MEMBER(member)\
	&& Convert(dir,text,#member,value.member)

#define JSON_CONVERT_END\
				&& Blockend(dir,text);\
	}\
}
