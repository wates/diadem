

#ifndef WTS_CONVERTER_H_
#define WTS_CONVERTER_H_


#include "container.h"
#include "parser.h"

namespace wts
{
    ///////////////////////////////
    // for POD

    template<typename T>
    inline bool Convert(T &c,char &v,const char *info)
    {
        return ConvertPOD(c,v,info);
    }
    template<typename T>
    inline bool Convert(T &c,unsigned char &v,const char *info)
    {
        return ConvertPOD(c,v,info);
    }
    template<typename T>
    inline bool Convert(T &c,short &v,const char *info)
    {
        return ConvertPOD(c,v,info);
    }
    template<typename T>
    inline bool Convert(T &c,unsigned short &v,const char *info)
    {
        return ConvertPOD(c,v,info);
    }
    template<typename T>
    inline bool Convert(T &c,int &v,const char *info)
    {
        return ConvertPOD(c,v,info);
    }
    template<typename T>
    inline bool Convert(T &c,unsigned int &v,const char *info)
    {
        return ConvertPOD(c,v,info);
    }
    template<typename T>
    inline bool Convert(T &c,float &v,const char *info)
    {
        return ConvertPOD(c,v,info);
    }

	///////////////////////////////////////
	// Binary Writer

	struct BinaryWriter
	{
		RawArray<char> output;
	};

	template<typename T>
	inline bool ConvertPOD(BinaryWriter &c,T &v,const char *info)
	{
        UNUSED(info);
        if(2==sizeof(T))
        {
            uint16_t val;
            val = (static_cast<uint16_t>(reinterpret_cast<const uint8_t*>(&v)[0]));
            val |= (static_cast<uint16_t>(reinterpret_cast<const uint8_t*>(&v)[1]))<<8;
            c.output.Append((char*)&val,sizeof(T));
        }
        else if(4==sizeof(T))
        {
            uint32_t val;
            val = (static_cast<uint32_t>(reinterpret_cast<const uint8_t*>(&v)[0]));
            val |= (static_cast<uint32_t>(reinterpret_cast<const uint8_t*>(&v)[1]))<<8;
            val |= (static_cast<uint32_t>(reinterpret_cast<const uint8_t*>(&v)[2]))<<16;
            val |= (static_cast<uint32_t>(reinterpret_cast<const uint8_t*>(&v)[3]))<<24;
            c.output.Append((char*)&val,sizeof(T));
        }
        else if(1==sizeof(T))
        {
            c.output.Append((char*)&v,sizeof(T));
        }
        else
        {
            *((int*)0)=0;
        }
		return true;
	}
	template<typename ArrayType>
	inline bool Convert(BinaryWriter &c,BasicString<ArrayType> &v,const char *info)
	{
        UNUSED(info);
        typedef typename ArrayType::T Char;
		c.output.Append((char*)v.Data(),sizeof(Char)*(v.Size()+1));           
		return true;
	}

    template<typename T>
    inline bool Convert(BinaryWriter &c,Array<T> &v,const char *info)
    {
        unsigned int count=v.Size();
        if(!Convert(c,count,info))
            return false;
        for(unsigned int i=0;i<count;i++)
            if(!Convert(c,v[i],info))
                return false;
        return true;
    }

    template<typename T,typename TypeSize>
    inline bool Convert(BinaryWriter &c,RawArray<T,TypeSize> &v,const char *info)
    {
        TypeSize count=v.Size();
        if(!Convert(c,count,info))
            return false;
        for(TypeSize i=0;i<count;i++)
            if(!Convert(c,v[i],info))
                return false;
        return true;
    }
    template<typename T,typename TypeSize>
    inline bool Convert(BinaryWriter &c,Array<T,TypeSize> &v,const char *info)
    {
        TypeSize count=v.size();
        if(!Convert(c,count,info))
            return false;
        for(TypeSize i=0;i<count;i++)
            if(!Convert(c,v[i],info))
                return false;
        return true;
    }

	inline bool ObjectStart(BinaryWriter &w,const char *info)
	{
        UNUSED(w);
        UNUSED(info);
		return true;
	}

	inline bool ObjectEnd(BinaryWriter &w)
	{
        UNUSED(w);
		return true;
	}

	////////////////////////////////
	// Binary Reader

	struct BinaryReader
	{
		const char *position;
		const char *end;
	};

	template<typename T>
	inline bool ConvertPOD(BinaryReader &c,T &v,const char *info)
	{
        UNUSED(info);
		if(c.position+sizeof(T)>c.end)
			return false;
#ifdef CONVERTER_LIST_NAMES
		if(typelist.find(typeid(T).name())==typelist.end())
			typelist[typeid(T).name()]=0;
		typelist[typeid(T).name()]++;
#endif

#if 1 
		if(2==sizeof(T))
		{
            *reinterpret_cast<uint16_t*>(&v) = (static_cast<uint16_t>(reinterpret_cast<const uint8_t*>(c.position)[0]));
            *reinterpret_cast<uint16_t*>(&v) |= (static_cast<uint16_t>(reinterpret_cast<const uint8_t*>(c.position)[1]))<<8;
		}
		else if(4==sizeof(T))
		{
            *reinterpret_cast<uint32_t*>(&v) = (static_cast<uint32_t>(reinterpret_cast<const uint8_t*>(c.position)[0]));
            *reinterpret_cast<uint32_t*>(&v) |= (static_cast<uint32_t>(reinterpret_cast<const uint8_t*>(c.position)[1]))<<8;
            *reinterpret_cast<uint32_t*>(&v) |= (static_cast<uint32_t>(reinterpret_cast<const uint8_t*>(c.position)[2]))<<16;
            *reinterpret_cast<uint32_t*>(&v) |= (static_cast<uint32_t>(reinterpret_cast<const uint8_t*>(c.position)[3]))<<24;
		}
		else
		{
			v=*(T*)c.position;
		}
#else
		v=*(T*)c.position;
#endif
		c.position+=sizeof(T);
		return true;
	}

	//template<typename T>
	//inline bool Convert(BinaryReader &c,Array<T> &v,const char *info)
	//{
	//	unsigned int count;
	//	if(!Convert(c,count,info))
	//		return false;
	//	v.Resize(count);
	//	for(unsigned int i=0;i<count;i++)
	//		if(!Convert(c,v[i],info))
	//			return false;
	//	return true;
	//}

	template<typename T,typename TypeSize>
	inline bool Convert(BinaryReader &c,RawArray<T,TypeSize> &v,const char *info)
	{
		TypeSize count;
		if(!Convert(c,count,info))
			return false;
		v.Resize(count);
		for(TypeSize i=0;i<count;i++)
			if(!Convert(c,v[i],info))
				return false;
		return true;
	}
	template<typename T,typename TypeSize>
	inline bool Convert(BinaryReader &c,Array<T,TypeSize> &v,const char *info)
	{
		TypeSize count;
		if(!Convert(c,count,info))
			return false;
		v.Resize(count);
		for(TypeSize i=0;i<count;i++)
			if(!Convert(c,v[i],info))
				return false;
		return true;
	}

	template<typename ArrayType>
	inline bool Convert(BinaryReader &c,BasicString<ArrayType> &v,const char *info)
	{
        typedef typename ArrayType::T Char;
		Char *begin=(Char*)c.position;
		Char *delim=(Char*)c.position;
		Char *end=(Char*)c.end;
		while(delim<end)
		{
			if(0==*delim)
			{
				v=begin;
				c.position=(char*)(delim+1);
				return true;
			}
            ++delim;
		}
		return false;
	}

	inline bool ObjectStart(BinaryReader &w,const char *info)
	{
        UNUSED(w);
        UNUSED(info);
		return true;
	}

	inline bool ObjectEnd(BinaryReader &w)
	{
        UNUSED(w);
		return true;
	}

    ///////////////////////////////////////
    // Json Writer

	struct JsonWriter
	{
		RawArray<char> output;

        struct Indent
        {
            bool first;
            bool is_array;
            Indent()
                :first(true),is_array(false)
            {
            }
        };

        Array<Indent> indent;
        JsonWriter()
        {
            PushIndent(true);
        }
        void PushIndent(bool is_array)
        {
            indent.Push();
            indent.Back().is_array=is_array;
        }
        bool IsArray()
        {
            return indent.Back().is_array;
        }
        void PopIndent()
        {
            indent.Pop();
        }
        void Separate()
        {
            if(indent.Back().first)
            {
                indent.Back().first=false;
            }
            else
            {
                output.Push(',');
            }
            if(!indent.Back().is_array)
            {
                output.Push('\n');
                for(int i=0;i<indent.Size()-1;i++)
                {
                    output.Push(' ');
                    output.Push(' ');
                }
            }
        }
	};

	template<typename T>
	inline bool ConvertPOD(JsonWriter &c,T &v,const char *info)
	{
        String s;
        c.Separate();
        if(!c.IsArray())
        {
            s<<"\""<<info<<"\":";
        }
        s<<v;
        c.output.Append(s.Data(),s.Size());
		return true;
	}
	template<typename ArrayType>
	inline bool Convert(JsonWriter &c,BasicString<ArrayType> &v,const char *info)
	{
        String s;
        c.Separate();
        if(!c.IsArray())
        {
            s<<"\""<<info<<"\":";
        }
        s<<"\""<<v<<"\"";
        c.output.Append(s.Data(),s.Size());
		return true;
	}

    template<typename T,typename TypeSize>
    inline bool Convert(JsonWriter &c,BasicArray<T,TypeSize> &v,const char *info)
    {
        TypeSize count=v.Size();
        //{
        //    String s;
        //    s<<info<<"_length";
        //    if(!Convert(c,(int)count,s.Data()))
        //        return false;
        //}
        {
            c.Separate();
            String s;
            if(!c.IsArray())
            {
                s<<"\""<<info<<"\":";
            }
            c.output.Append(s.Data(),s.Size());
            c.output.Push('[');
            c.PushIndent(true);
            for(TypeSize i=0;i<count;i++)
                if(!Convert(c,v[i],""))
                    return false;
            c.PopIndent();
            c.output.Push(']');
        }
        return true;
    }

	template<int N,typename T>
	inline bool Convert(JsonWriter &c,T (&v)[N],const char *info)
	{
        typedef int TypeSize;
        TypeSize count=N;
        //{
        //    String s;
        //    s<<info<<"_length";
        //    if(!Convert(c,count,s.Data()))
        //        return false;
        //}
        {
            c.Separate();
            String s;
            if(!c.IsArray())
            {
                s<<"\""<<info<<"\":";
            }
            c.output.Append(s.Data(),s.Size());
            c.output.Push('[');
            c.PushIndent(true);
            for(TypeSize i=0;i<count;i++)
                if(!Convert(c,v[i],""))
                    return false;
            c.PopIndent();
            c.output.Push(']');
        }
        return true;
		for(size_t i=0;i<N;i++)
			if(!Convert(c,v[i],info))
				return false;
		return true;
	}


	inline bool ObjectStart(JsonWriter &c,const char *info)
	{
        String s;
        c.Separate();
        if(!c.IsArray())
        {
            s<<"\""<<info<<"\": ";
        }
        c.output.Append(s.Data(),s.Size());
        c.output.Push('{');
        c.PushIndent(false);
		return true;
	}

	inline bool ObjectEnd(JsonWriter &c)
	{
        c.PopIndent();
        c.output.Push('}');
		return true;
	}

    //////////////////////////////
    //
    namespace json
    {
        using namespace parser;

        static void ReplaceAll(wts::String &str,const char *from,const char *to)
        {
	        size_t len=strlen(from);
	        size_t lento=strlen(to);
	        for(int i=str.Find(from);i!=-1;i=str.Find(from,i+lento))
		        str.Replace(i,len,to);
        }

        static void Unescape(wts::String &str)
        {
	        ReplaceAll(str,"\\\"","\"");
	        ReplaceAll(str,"\\/","/");
	        ReplaceAll(str,"\\\b","\b");
	        ReplaceAll(str,"\\\f","\f");
	        ReplaceAll(str,"\\\n","\n");
	        ReplaceAll(str,"\\\r","\r");
	        ReplaceAll(str,"\\\t","\t");
	        ReplaceAll(str,"\\\\","\\");
        }

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

        typedef Text<'t','r','u','e'> tTrue;
        typedef Text<'f','a','l','s','e'> tFalse;
        typedef Text<'n','u','l','l'> tNull;

	    typedef More<Or<Char<0x20>,Char<0x09>,Char<0x0d>,Char<0x0a> > > tS;
	    typedef Option<tS> tOS;

	    typedef Rule<Char<'\\'>,Or<Char<'"','\\','/','b','f','n','r','t'>,Rule<Char<'u'>,tHex,tHex,tHex,tHex> > > tEscape;

	    typedef Any<Or<tEscape,Not<Char<'"','\\'> > > > tStringBody;

	    struct KeyBody
		    :public Action<tStringBody>
	    {
		    wts::String key;
		    void Hit(const char *text,type &r)
		    {
                UNUSED(r);

			    key=text;
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
                UNUSED(r);

			    val=atoi(text);
		    }
	    };

	    struct valFloat
		    :public Action<tFloat>
	    {
		    float val;
		    void Hit(const char *text,type &r)
		    {
                UNUSED(r);

			    val=(float)atof(text);
		    }
	    };

	    struct valStringBody
		    :public Action<tStringBody>
	    {
		    wts::String val;
		    void Hit(const char *text,type &r)
		    {
                UNUSED(r);

			    val=text;
			    Unescape(val);
		    }
	    };
	    typedef Rule<Char<'"'>,valStringBody,Char<'"'> > valString;

	    typedef Rule<tOS,Option<Char<','> >,tOS,Key,tOS,Char<':'> > single;
	    typedef Rule<Option<single>,tOS,valInt,tOS> intPair;
	    typedef Rule<Option<single>,tOS,valFloat,tOS> floatPair;
	    typedef Rule<Option<single>,tOS,valString,tOS> stringPair;

	    typedef Rule<Option<single>,tOS,Char<'{'>,tOS> objectStart;
	    typedef Rule<tOS,Char<'}'>,tOS> objectEnd;
	    typedef Rule<tOS,Char<'['>,tOS> arrayStart;
	    typedef Rule<tOS,Char<','>,tOS> arraySeparate;
	    typedef Rule<tOS,Char<']'>,tOS> arrayEnd;

        template<typename T>struct PairSelector;

        template<>struct PairSelector<int>{typedef intPair type;};
        template<>struct PairSelector<float>{typedef floatPair type;};
        template<>struct PairSelector<wts::String>{typedef stringPair type;};
    }

  	struct JsonReader
	{
		const char *position;
		const char *end;
	};

    template<typename T>
    bool ConvertPair(JsonReader &c,T &value,const char *info)
    {
        UNUSED(info);
        typename json::PairSelector<T>::type p;
        if(!p.Parse(c.position) /*&& p.key != info */)
            return false;
        value=p.val;
        return true;
    }

    bool Convert(JsonReader &c,int &value,const char *info)
    {
        return ConvertPair(c,value,info);
    }

    bool Convert(JsonReader &c,float &value,const char *info)
    {
        return ConvertPair(c,value,info);
    }

    bool Convert(JsonReader &c,wts::String &value,const char *info)
    {
        return ConvertPair(c,value,info);
    }

    template<typename T,typename TypeSize>
    inline bool Convert(JsonReader &c,Array<T,TypeSize> &v,const char *info)
    {
        UNUSED(info);
        json::single si;
        json::arrayStart as;
        json::arraySeparate ap;
        json::arrayEnd ae;
        if(si.Parse(c.position) && as.Parse(c.position) )
        {
            for(int n=0;;n++)
            {
                if(n&&!ap.Parse(c.position))
                    return false;
                if(!Convert(c,v.Push(),""))
                    return false;
                if(ae.Parse(c.position))
                    break;
            }
            return true;
        }
        return false;
    }

	//template<int N,typename T>
	//inline bool Convert(JsonReader &c,T (&v)[N],const char *info)
	//{
	//	return true;
	//}

	inline bool ObjectStart(JsonReader &c,const char *info)
	{
        UNUSED(info);
        json::objectStart p;
		return p.Parse(c.position);
	}

	inline bool ObjectEnd(JsonReader &c)
	{
        json::objectEnd p;
        return p.Parse(c.position);
	}

    /////////////////////////////
	//

	template<typename Z,size_t N,typename T>
	inline bool Convert(Z &c,T (&v)[N],const char *info)
	{
		for(size_t i=0;i<N;i++)
			if(!Convert(c,v[i],info))
				return false;
		return true;
	}


}

#define CONVERT_OBJECT_0(type)\
namespace wts {\
template<typename Func>bool Convert(Func &f,type &value,const char *info){\
return ObjectStart(f,info)\
    && ObjectEnd(f);}}

#define CONVERT_OBJECT_1(type,m0)\
    namespace wts {\
    template<typename Func>bool Convert(Func &f,type &value,const char *info){\
    return ObjectStart(f,info)\
    && Convert(f,value.m0,#m0)\
    && ObjectEnd(f);}}

#define CONVERT_OBJECT_2(type,m0,m1)\
    namespace wts {\
    template<typename Func>bool Convert(Func &f,type &value,const char *info){\
    return ObjectStart(f,info)\
    && Convert(f,value.m0,#m0)\
    && Convert(f,value.m1,#m1)\
    && ObjectEnd(f);}}

#define CONVERT_OBJECT_3(type,m0,m1,m2)\
    namespace wts {\
    template<typename Func>bool Convert(Func &f,type &value,const char *info){\
    return ObjectStart(f,info)\
    && Convert(f,value.m0,#m0)\
    && Convert(f,value.m1,#m1)\
    && Convert(f,value.m2,#m2)\
    && ObjectEnd(f);}}

#define CONVERT_OBJECT_4(type,m0,m1,m2,m3)\
    namespace wts {\
    template<typename Func>bool Convert(Func &f,type &value,const char *info){\
    return ObjectStart(f,info)\
    && Convert(f,value.m0,#m0)\
    && Convert(f,value.m1,#m1)\
    && Convert(f,value.m2,#m2)\
    && Convert(f,value.m3,#m3)\
    && ObjectEnd(f);}}

#define CONVERT_OBJECT_5(type,m0,m1,m2,m3,m4)\
    namespace wts {\
    template<typename Func>bool Convert(Func &f,type &value,const char *info){\
    return ObjectStart(f,info)\
    && Convert(f,value.m0,#m0)\
    && Convert(f,value.m1,#m1)\
    && Convert(f,value.m2,#m2)\
    && Convert(f,value.m3,#m3)\
    && Convert(f,value.m4,#m4)\
    && ObjectEnd(f);}}\

#define CONVERT_OBJECT_6(type,m0,m1,m2,m3,m4,m5)\
    namespace wts {\
    template<typename Func>bool Convert(Func &f,type &value,const char *info){\
    return ObjectStart(f,info)\
    && Convert(f,value.m0,#m0)\
    && Convert(f,value.m1,#m1)\
    && Convert(f,value.m2,#m2)\
    && Convert(f,value.m3,#m3)\
    && Convert(f,value.m4,#m4)\
    && Convert(f,value.m5,#m5)\
    && ObjectEnd(f);}}

#define CONVERT_OBJECT_7(type,m0,m1,m2,m3,m4,m5,m6)\
    namespace wts {\
    template<typename Func>bool Convert(Func &f,type &value,const char *info){\
    return ObjectStart(f,info)\
    && Convert(f,value.m0,#m0)\
    && Convert(f,value.m1,#m1)\
    && Convert(f,value.m2,#m2)\
    && Convert(f,value.m3,#m3)\
    && Convert(f,value.m4,#m4)\
    && Convert(f,value.m5,#m5)\
    && Convert(f,value.m6,#m6)\
    && ObjectEnd(f);}}


#define CONVERT_OBJECT_BEGIN(type)\
	namespace wts\
{\
	template<typename Func>\
	bool Convert(Func &f,type &value,const char *info)\
{\
	return ObjectStart(f,info)

#define CONVERT_OBJECT_MEMBER(member)\
	&& Convert(f,value.member,#member)

#define CONVERT_OBJECT_MEMBER_TYPE(member,type)\
	&& Convert(f,(*reinterpret_cast<type*>(&value.member)),#member)

#define CONVERT_OBJECT_END\
	&& ObjectEnd(f);\
}\
}

#endif
