

#ifndef WTS_CONVERTER_H_
#define WTS_CONVERTER_H_


#include "container.h"

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
