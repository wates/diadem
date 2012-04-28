#include "cp932.inl"
#include "jis0201.inl"
#include "jis0208.inl"
//#include "jis0212.inl"
//#include "shiftjis.inl"
#include "jpncode.h"

namespace jpncode
{

	Result sjis_decode(const char *from,unicode *to)
	{
		Result res={0,0,0};
		while(from[res.read])
		{
			unsigned char first=(unsigned char)from[res.read];
			if(cp932::decode_00[first])
			{
				if(to)
					to[res.charactors]=cp932::decode_00[first];
				res.charactors++;
				res.read++;
				continue;
			}
			else if(cp932::decode[first])
			{
				res.read++;
				unsigned char second=(unsigned char)from[res.read];
				if(0==second)
				{
					res.errors++;
					break;
				}
				else if(cp932::decode[first][second])
				{
					if(to)
						to[res.charactors]=cp932::decode[first][second];
					res.read++;
					res.charactors++;
				}
				else
				{
					res.errors++;
				}
			}
			else
			{
				res.read++;
				res.errors++;
			}
		}
		return res;
	}

	Result sjis_unicode_charactors(const char *from)
	{
		return sjis_decode(from,0);
	}

	Result sjis_encode(const unicode *from,char *to,const char *error_string)
	{
		Result res={0,0,0};
		while(from[res.read])
		{
			unicode unic=from[res.read++];
			unsigned char upper=(unsigned char)(unic>>8);
			unsigned char lower=(unsigned char)unic;
			if(cp932::encode[upper])
			{
				unsigned short mb=cp932::encode[upper][lower];
				if(mb)
				{
					if(mb<256)
					{
						if(to)
							to[res.charactors]=mb&0xff;
					}
					else
					{
						if(to)
							to[res.charactors]=mb>>8;
						res.charactors++;
						if(to)
							to[res.charactors]=mb&0xff;
					}
					res.charactors++;
				}
				else
				{
					if(to&&error_string)
					{
						for(int i=0;error_string[i];i++)
						{
							to[res.charactors++]=error_string[i];
						}
					}
					res.errors++;
				}
			}
			else
			{
				if(to&&error_string)
				{
					for(int i=0;error_string[i];i++)
					{
						to[res.charactors++]=error_string[i];
					}
				}
				res.errors++;
			}
		}
		return res;
	}

	Result sjis_multibyte_charactors(const unicode *from)
	{
		return sjis_encode(from,0,0);
	}

	Result jis0208_decode(const char *from,unicode *to)
	{
		Result res={0,0,0};
		while(from[res.read])
		{
			unsigned char first=(unsigned char)from[res.read];
			if(jis0208::decode[first])
			{
				res.read++;
				unsigned char second=(unsigned char)from[res.read];
				if(0==second)
				{
					res.errors++;
					break;
				}
				else if(jis0208::decode[first][second])
				{
					if(to)
						to[res.charactors]=jis0208::decode[first][second];
					res.read++;
					res.charactors++;
				}
				else
				{
					res.errors++;
				}
			}
			else
			{
				res.read++;
				res.errors++;
			}
		}
		return res;
	}

	Result jis0208_unicode_charactors(const char *from)
	{
		return jis0208_decode(from,0);
	}

	Result jis0208_encode(const unicode *from,char *to,const char *error_string)
	{
		Result res={0,0,0};
		while(from[res.read])
		{
			unicode unic=from[res.read++];
			unsigned char upper=(unsigned char)(unic>>8);
			unsigned char lower=(unsigned char)unic;
			if(jis0208::encode[upper])
			{
				unsigned short mb=jis0208::encode[upper][lower];
				if(mb)
				{
					if(mb<256)
					{
						if(to)
							to[res.charactors]=mb&0xff;
					}
					else
					{
						if(to)
							to[res.charactors]=mb>>8;
						res.charactors++;
						if(to)
							to[res.charactors]=mb&0xff;
					}
					res.charactors++;
				}
				else
				{
					if(to&&error_string)
					{
						for(int i=0;error_string[i];i++)
						{
							to[res.charactors++]=error_string[i];
						}
					}
					res.errors++;
				}
			}
			else
			{
				if(to&&error_string)
				{
					for(int i=0;error_string[i];i++)
					{
						to[res.charactors++]=error_string[i];
					}
				}
				res.errors++;
			}
		}
		return res;
	}

	Result jis0208_multibyte_charactors(const unicode *from)
	{
		return jis0208_encode(from,0,0);
	}

	Result euc_decode(const char *from,unicode *to)
	{
		Result res={0,0,0};
		while(from[res.read])
		{
			unsigned char first=(unsigned char)from[res.read];
			if(first>0x80&&jis0208::decode[first-0x80])
			{
				res.read++;
				unsigned char second=(unsigned char)from[res.read];
				if(0==second)
				{
					res.errors++;
					break;
				}
				else if(0x80>second)
				{
					res.errors++;
					continue;
				}
				first-=0x80;
				second-=0x80;
				if(jis0208::decode[first][second])
				{
					if(to)
						to[res.charactors]=jis0208::decode[first][second];
					res.read++;
					res.charactors++;
				}
				else
				{
					res.errors++;
				}
			}
			else if(jis0201::decode_00[first])
			{
				if(to)
					to[res.charactors]=jis0201::decode_00[first];
				res.read++;
				res.charactors++;
			}
			else
			{
				if(to)
					to[res.charactors]=first;
				res.charactors++;
				res.read++;
//				res.errors++;
			}
		}
		return res;
	}

	Result euc_unicode_charactors(const char *from)
	{
		return euc_decode(from,0);
	}

	Result euc_encode(const unicode *from,char *to,const char * /*error_string*/)
	{
		Result res={0,0,0};
		while(from[res.read])
		{
			unicode unic=from[res.read++];
			unsigned char upper=(unsigned char)(unic>>8);
			unsigned char lower=(unsigned char)unic;
			if(jis0201::encode[upper]&&jis0201::encode[upper][lower])
			{
				unsigned short mb=jis0201::encode[upper][lower];
				if(mb<256)
				{
					if(to)
						to[res.charactors]=mb&0xff;
				}
				else
				{
					if(to)
						to[res.charactors]=mb>>8;
					res.charactors++;
					if(to)
						to[res.charactors]=mb&0xff;
				}
				res.charactors++;
			}
			else if(jis0208::encode[upper]&&jis0208::encode[upper][lower])
			{
				unsigned short mb=jis0208::encode[upper][lower];
				if(mb<256)
				{
					if(to)
						to[res.charactors]=(mb|0x80)&0xff;
				}
				else
				{
					if(to)
						to[res.charactors]=(mb>>8)|0x80;
					res.charactors++;
					if(to)
						to[res.charactors]=(mb|0x80)&0xff;
				}
				res.charactors++;
			}
			else
			{
				if(to)
					to[res.charactors++]=lower;
				//if(to&&error_string)
				//{
				//	for(int i=0;error_string[i];i++)
				//	{
				//		to[res.charactors++]=error_string[i];
				//	}
				//}
				res.errors++;
			}
		}
		return res;
	}

	Result euc_multibyte_charactors(const unicode *from)
	{
		return euc_encode(from,0,0);
	}

	Result utf8_decode(const char *from,unicode *to)
	{
		Result res={0,0,0};
		while(from[res.read])
		{
			if((from[res.read]&0xf8)==0xf0)
			{
				if(from[res.read+1]&&from[res.read+2]&&from[res.read+3]&&
					(from[res.read+1]&from[res.read+2]&from[res.read+3]&0xc0)==0x80)
				{
					if(to)
					{
						unicode unic;
						unic=(unicode)(from[res.read+0]&0x07)<<18;
						unic|=(unicode)(from[res.read+1]&0x3f)<<12;
						unic|=(unicode)(from[res.read+2]&0x3f)<<6;
						unic|=from[res.read+3]&0x3f;
						to[res.charactors]=unic;
					}
					res.charactors++;
					res.read+=4;
				}
				else
				{
					res.errors++;
					res.read++;
				}
			}
			else if((from[res.read]&0xf0)==0xe0)
			{
				if(from[res.read+1]&&from[res.read+2]&&
					(from[res.read+1]&from[res.read+2]&0xc0)==0x80)
				{
					if(to)
					{
						unicode unic;
						unic=(unicode)(from[res.read+0]&0x0f)<<12;
						unic|=(unicode)(from[res.read+1]&0x3f)<<6;
						unic|=(from[res.read+2]&0x3f);
						to[res.charactors]=unic;
					}
					res.charactors++;
					res.read+=3;
				}
				else
				{
					res.errors++;
					res.read++;
				}
			}
			else if((from[res.read]&0xe0)==0xc0)
			{
				if(from[res.read+1]&&
					(from[res.read+1]&0xc0)==0x80)
				{
					if(to)
					{
						unicode unic;
						unic=(unicode)(from[res.read+0]&0x1f)<<6;
						unic|=from[res.read+1]&0x3f;
						to[res.charactors]=unic;
					}
					res.charactors++;
					res.read+=2;
				}
				else
				{
					res.errors++;
					res.read++;
				}
			}
			else if((from[res.read]&0x80)==0)
			{
				if(to)
					to[res.charactors]=from[res.read];
				res.charactors++;
				res.read++;
			}
			else
			{
				res.errors++;
				res.read++;
			}
		}
		return res;
	}

	Result utf8_unicode_charactors(const char *from)
	{
		return utf8_decode(from,0);
	}

	Result utf8_encode(const unicode *from,char *to)
	{
		Result res={0,0,0};
		while(from[res.read])
		{
			unicode unic=from[res.read];
			if(unic<0x80)
			{
				if(to)
					to[res.charactors]=unic&0x7f;
				res.charactors++;
			}
			else if(unic<0x800)
			{
				if(to)
				{
					to[res.charactors+0]=((unic>>6)&0x1f)|0xc0;
					to[res.charactors+1]=((unic>>0)&0x3f)|0x80;
				}
				res.charactors+=2;
			}
			else if(unic<0x10000)
			{
				if(to)
				{
					to[res.charactors+0]=((unic>>12)&0x0f)|0xe0;
					to[res.charactors+1]=((unic>>6)&0x3f)|0x80;
					to[res.charactors+2]=((unic>>0)&0x3f)|0x80;
				}
				res.charactors+=3;
			}
			else if(unic<0x110000)
			{
				if(to)
				{
					to[res.charactors+0]=((unic>>18)&0x07)|0xf0;
					to[res.charactors+1]=((unic>>12)&0x3f)|0x80;
					to[res.charactors+2]=((unic>>6)&0x3f)|0x80;
					to[res.charactors+3]=((unic>>0)&0x3f)|0x80;
				}
				res.charactors+=4;
			}
			else
			{
			}
			res.read++;
		}
		return res;
	}

	Result utf8_multibyte_charactors(const unicode *from)
	{
		return utf8_encode(from,0);
	}

	unsigned int unilen(const unicode *str)
	{
		unsigned int count=0;
		while(*str++)
			count++;
		return count;
	}

	Result utf16le_encode(const unicode *from,unsigned short *to)
	{
		Result res={0,0,0};
		while(from[res.read])
		{
            //TODO surrogate pair
			to[res.charactors]=from[res.read]&0xffff;
			res.charactors++;
			res.read++;
		}
		return res;
	}
}
