
#ifndef JPNCODE_H_INCLUDED
#define JPNCODE_H_INCLUDED

namespace jpncode
{
	typedef unsigned int unicode;

	struct Result
	{
		unsigned int errors;
		unsigned int charactors;
		unsigned int read;
	};

	struct ErrorProcessor
	{
		virtual void Error(const unicode *from,const char *to,Result res)=0;
	};

	struct InsertQuestion
		:public ErrorProcessor
	{
		void Error(const unicode *from,const char *to,Result res);
	};

	Result sjis_decode(const char *from,unicode *to);
	Result sjis_unicode_charactors(const char *from);
	Result sjis_encode(const unicode *from,char *to,const char *error_string="?");
	Result sjis_multibyte_charactors(const unicode *from);

	Result jis0208_decode(const char *from,unicode *to);
	Result jis0208_unicode_charactors(const char *from);
	Result jis0208_encode(const unicode *from,char *to,const char *error_string="?");
	Result jis0208_multibyte_charactors(const unicode *from);

	Result euc_decode(const char *from,unicode *to);
	Result euc_unicode_charactors(const char *from);
	Result euc_encode(const unicode *from,char *to,const char *error_string="?");
	Result euc_multibyte_charactors(const unicode *from);

	Result utf8_decode(const char *from,unicode *to);
	Result utf8_unicode_charactors(const char *from);
	Result utf8_encode(const unicode *from,char *to);
	Result utf8_multibyte_charactors(const unicode *from);

	unsigned int unilen(const unicode *str);

	Result utf16le_encode(const unicode *from,unsigned short *to);
}


#endif
