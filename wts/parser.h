
#ifndef WTS_PARSER_H_
#define WTS_PARSER_H_

//#ifdef _WIN32
//#pragma warning(disable:4503)
//#pragma warning(disable:4584)
//#endif

namespace parser
{
	struct end
	{
	};

	struct good
	{
		inline static bool Parse(const char *&text)
		{
            UNUSED(text);
			return true;
		}
	};

	template<int Any>
	struct bad
	{
		inline static bool Parse(const char *&text)
		{
            UNUSED(text);
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
            UNUSED(text);
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
	struct TextEqual
		:public TextEqual<template_char_use_next>
	{
		static const int length=1+TextEqual<template_char_use_next>::length;
		static bool Equal(const char *c)
		{
			return c1==*c&&TextEqual<template_char_use_next>::Equal(c+1);
		}
	};

	template<template_char_specialize>
	struct TextEqual<0,template_char_use_next>
	{
		static const int length=0;
		static bool Equal(const char *c)
		{
            UNUSED(c);
			return true;
		}
	};

	template<template_char_prototype>
	struct Text
		:public TextEqual<template_char_use>
	{
		static bool Parse(const char *&text)
		{
			const char *src=text;
			if(TextEqual<template_char_use>::Equal(text))
			{
				text+=TextEqual<template_char_use>::length;
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
		:virtual public T
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
			if(t1::Parse(text)&&
				t2::Parse(text)&&
				t3::Parse(text)&&
				t4::Parse(text)&&
				t5::Parse(text)&&
				t6::Parse(text)&&
				t7::Parse(text)&&
				t8::Parse(text))
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
        inline virtual ~Action(){}
	};

}

#endif
