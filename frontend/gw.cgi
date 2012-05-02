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
        cur.execute(req,param)
    else:
        cur.execute(req)
    rows = cur.fetchall()
    cur.close()
    connect.close()
    return rows
    

request=json.load(sys.stdin)

param=()

response={}

if request["method"]=="getChannel":
    req="select * from channel"
    rows=MySQLExecute(req)
    response["method"]="channel"
    response["channel"]=[]
    
    for r in rows:
        channel={};
        channel["time"]=r[0].__str__()
        channel["name"]=r[1]
        if(r[2]):
            channel["topic"]=r[2]
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
elif request["method"] == "getNick":
    req="select * from join_nick"
    rows=MySQLExecute(req)
    response["method"]="nick"
    response["nick"]=[];

    for r in rows:
        nick={};
        nick["nick"]=r[1]
        nick["channel"]=r[2]
        nick["op"]=r[3]
        response["nick"].append(nick)
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

