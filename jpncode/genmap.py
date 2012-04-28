#/usr/bin/python

import sys

map={}

f=open(sys.argv[1],"r")
for line in f:
    line=line.split('#')[0]
    fromto=line.split('\t')
    if len(fromto)>=2:
        base=fromto[0][2:]
        unic=fromto[1][2:]
        try:
            map[int(base,16)]=int(unic,16)
        except:
            pass
f.close()

type=sys.argv[1].split('.')[0]

type=type.lower()

def cinz(prefix,mapping):
    code=""
    base_code=[]
    tbl="static const unsigned short* const %s[256]={"%(prefix)

    for i in range(256):
        base_code.append("")
        num=""
        had=0
        for j in range(0,256):
            if mapping.has_key(i*256+j):
                had+=1
                num=num+"%s,"%(mapping[i*256+j])
            else:
                num=num+"0,"
        if had >= 1:
            num=num[:-1]
            base_code[i]="static const unsigned short %s_%02X[256]={%s};\n"%(prefix,i,num)
            tbl=tbl+"%s_%02X,"%(prefix,i)
        else:
            tbl=tbl+"0,"
    tbl=tbl[:-1]
    tbl=tbl+"};\n"

    for s in base_code:
        if len(s)>0:
            code=code+s
    code=code+tbl
    return code


invmap={}
for k,v in map.iteritems():
    invmap[v]=k

code="namespace jpncode{\nnamespace %s{\n"%type
code=code+cinz("decode",map)+cinz("encode",invmap)
code=code+"}}\n\n"

print(code)

f=open(type+".inl","w")
f.write(code)
f.close()