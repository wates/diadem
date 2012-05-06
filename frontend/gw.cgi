#!/usr/bin/python

import sys
import cgi
import MySQLdb
import json
import time
import datetime

print("Content-Type:text/html; charset=utf-8\r\n\r\n")


def MySQLExecute(request):
    connect = MySQLdb.connect(db="ircdb", host="localhost", port=3306, user="root", passwd="")
    cur = connect.cursor()
    cur.execute("set names utf8")
    if param:
        cur.execute(request,param)
    else:
        cur.execute(request)
    rows = cur.fetchall()
    cur.close()
    connect.close()
    return rows
def Escape(str):
    return MySQLdb.escape_string(str.encode('utf-8'));

request=json.load(sys.stdin)

param=()

response={}

def getChannelNick(name):
    req="select nick_name,is_operator from join_nick where channel_name='%s'"%(Escape(name))
    res=MySQLExecute(req)
    ret=[]
    for r in res:
        nick={};
        nick["nick"]=r[0]
        nick["op"]=r[1]
        ret.append(nick)
    return ret
    

if request["method"]=="getChannel":
    req="select name,topic from channel"
    rows=MySQLExecute(req)
    response["method"]="channel"
    response["channel"]=[]
    
    for r in rows:
        channel={};
        channel["name"]=r[0]
        channel["nick"]=getChannelNick(r[0])
        if(r[1]):
            channel["topic"]=r[1]
        response["channel"].append(channel)
        
elif request["method"]=="getEvent":
    req=""
    if(request.has_key("last_response_time")):
        req="select * from event where time>'%s' order by time"%(MySQLdb.escape_string(request["last_response_time"].encode('utf-8')))
    else:
        req="select * from event where time>(now()-interval 12 hour) order by time"
    rows=MySQLExecute(req)
    response["method"]="event"
    response["event"]=[]
    response["response_time"]=rows[-1][0].__str__()

    for r in rows:
        event={};
        event["time"]=r[0].__str__()
        if r[1]:
            event["prefix"]=r[1]
        else:
            event["prefix"]=""
        event["command"]=r[2]
        for i in range(0,12):
            if not r[i+3]:
                break
            event["param%d"%(i+1)]=r[i+3]
        response["event"].append(event)
elif request["method"] == "Privmsg":
    req="insert into queue set command='PRIVMSG',parameter='%s',message='%s'"%(MySQLdb.escape_string(request["channel"].encode('utf-8')),MySQLdb.escape_string(request["message"].encode('utf-8')))
    MySQLExecute(req)
elif request["method"] == "Join":
    req="insert into queue set command='JOIN',parameter='%s'"%(MySQLdb.escape_string(request["channel"].encode('utf-8')))
    MySQLExecute(req)
elif request["method"] == "Part":
    req="insert into queue set command='PART',parameter='%s',message='%s'"%(MySQLdb.escape_string(request["channel"].encode('utf-8')),MySQLdb.escape_string(request["message"].encode('utf-8')))
    MySQLExecute(req)

if not response.has_key("response_time"):
    response["response_time"]=datetime.datetime.today().strftime("%Y-%m-%d %H:%M:%S")


print (json.dumps(response)).encode('utf-8')

