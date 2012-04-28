
#include "jpncode.h"

#include <gtest/gtest.h>

#include <iostream>
#include <fstream>
#include <vector>


#include "moji.h"

using namespace jpncode;


TEST(Decode,Hiragana)
{
    const char src[]={0x82,0xa0,0x82,0xa2,0x82,0xa4,0x82,0xa6,0x82,0xa8,0};

    Result r=sjis_unicode_charactors(src);
    EXPECT_EQ(5,r.charactors);
    EXPECT_EQ(0,r.errors);
    EXPECT_EQ(10,r.read);
}

TEST(Decode,Katakana)
{
    const char src[]={0x83,0x41,0x83,0x43,0x83,0x45,0x83,0x47,0x83,0x49,0};

    Result r=sjis_unicode_charactors(src);
    EXPECT_EQ(5,r.charactors);
    EXPECT_EQ(0,r.errors);
    EXPECT_EQ(10,r.read);
}

TEST(Decode,Kanji)
{
    const char src[]={0x88,0x9f,0x8b,0x8f,0x89,0x4c,0x93,0xbe,0x8c,0xe4,0};

    Result r=sjis_unicode_charactors(src);
    EXPECT_EQ(5,r.charactors);
    EXPECT_EQ(0,r.errors);
    EXPECT_EQ(10,r.read);
}

TEST(Decode,Long)
{
    std::string str(moji::rfc2812_j_sjis_txt,moji::rfc2812_j_sjis_txt+moji::rfc2812_j_sjis_txt_size);

    jpncode::Result r;
    r=jpncode::sjis_unicode_charactors(str.c_str());

    EXPECT_EQ(0,r.errors);

    //jpncode::unicode *unistr=new jpncode::unicode[er.charactors+1];
    //jpncode::sjis_decode(str.c_str(),unistr);
    //unistr[er.charactors]=0;

    //wchar_t *wstr=new wchar_t[1048576];
    //er=jpncode::utf16le_encode(unistr,(unsigned short*)wstr);
    //size_t len=jpncode::unilen(unistr);
    //WriteFile("moji_16le.txt",wstr,er.charactors*2);

    //char *encstr=new char[1048576];
    //er=jpncode::sjis_encode(unistr,encstr);
    //WriteFile("moji_cp932.txt",encstr,er.charactors);

    //char *eucstr=new char[1048576];
    //er=jpncode::euc_encode(unistr,eucstr);
    //WriteFile("moji_enc_euc.txt",eucstr,er.charactors);

    //er=jpncode::utf8_encode(unistr,encstr);
    //WriteFile("moji_utf8.txt",encstr,er.charactors);

}
	//{
	//	std::string str;
	//	ReadFile("moji_euc.txt",str);
	//	jpncode::Result er;
	//	er=jpncode::euc_unicode_charactors(str.c_str());
	//	jpncode::unicode *unistr=new jpncode::unicode[er.charactors+1];
	//	jpncode::euc_decode(str.c_str(),unistr);
	//	unistr[er.charactors]=0;

	//	char *encstr=new char[1048576];
	//	er=jpncode::sjis_encode(unistr,encstr);
	//	WriteFile("moji_cp932.txt",encstr,er.charactors);
	//}

