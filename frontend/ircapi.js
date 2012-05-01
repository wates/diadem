
var active_channel;
var next_polling_count=10;


var API=new Object;

var lastres;

function PrecCh(name)
{
    for(var c in Chs)
    {
        if(Chs[c].name==name)
        {
            return;
        }
    }
    Chs[name]=new Channel(name);
}

function AjaxResponse(res)
{
    var obj=eval("("+res.responseText+")");
    if(obj.method=="channel")
    {
        if(obj.channel)
        {
            for(var i=0;i<obj.channel.length;i++)
            {
                var chn=obj.channel[i].name;
                Chs[chn]=new Channel(chn);
            }
        }
        API.getNick();
    }
    if(obj.method=="event")
    {
        API.last_event_response_time=obj.response_time;
        for(var i=0;i<obj.event.length;i++)
        {
            var e=obj.event[i];
            if(e.command=="PRIVMSG")
            {
                 PrecCh(e.param1);
                 var li=document.createElement("li");
                 li.setAttribute("class","privmsg");
                 li.innerHTML=e.time.split(" ")[1]+" ["+e.prefix+"] "+e.param2;
                 Chs[e.param1].msg.appendChild(li);
                 Chs[e.param1].root.scrollTop+=17;
            }
            else if(e.command=="NOTICE")
            {
                 PrecCh(e.param1);
                 var li=document.createElement("li");
                 li.setAttribute("class","notice");
                 li.innerHTML=e.time.split(" ")[1]+" ["+e.prefix+"] "+e.param2;
                 Chs[e.param1].msg.appendChild(li);
                 Chs[e.param1].root.scrollTop+=17;
            }
            else if(e.command=="JOIN")
            {
                 PrecCh(e.param1);
                 var li=document.createElement("li");
                 li.setAttribute("class","join");
                 li.innerHTML=e.time.split(" ")[1]+" "+e.prefix+" join";
                 Chs[e.param1].msg.appendChild(li);
                 Chs[e.param1].root.scrollTop+=17;
                 Chs[e.param1].appendNick(e.prefix);
            }
            else if(e.command=="PART")
            {
                 PrecCh(e.param1);
                 var li=document.createElement("li");
                 li.setAttribute("class","part");
                 li.innerHTML=e.time.split(" ")[1]+" "+e.prefix+" part - ";
                 if(e.param2)
                 {
                     li.innerHTML+=e.param2;
                 }
                 Chs[e.param1].msg.appendChild(li);
                 Chs[e.param1].root.scrollTop+=17;
                 Chs[e.param1].removeNick(e.prefix);
            }
            else if(e.command=="NICK")
            {
                 for(var c in Chs)
                 {
                     if(Chs[c].findNick(e.prefix)==1)
                     {
                         var li=document.createElement("li");
                         li.setAttribute("class","nick");
                         li.innerHTML=e.time.split(" ")[1]+" "+e.prefix+" nick -> "+e.param1;
                         Chs[c].removeNick(e.prefix);
                         Chs[c].appendNick(e.param1);
                         Chs[c].msg.appendChild(li);
                         Chs[c].root.scrollTop+=17;
                     }
                 }
            }
            else if(e.command=="QUIT")
            {
                 for(var c in Chs)
                 {
                     if(Chs[c].findNick(e.prefix)==1)
                     {
                         var li=document.createElement("li");
                         li.setAttribute("class","quit");
                         li.innerHTML=e.time.split(" ")[1]+" "+e.prefix+" quit - ";
                         if(e.param1)
                         {
                             li.innerHTML+=e.param1;
                         }
                         Chs[c].removeNick(e.prefix);
                         Chs[c].msg.appendChild(li);
                         Chs[c].root.scrollTop+=17;
                     }
                 }
            }
        }
    }
    if(obj.method=="nick")
    {
        for(var i=0;i<obj.nick.length;i++)
        {
            var nick=obj.nick[i];
            Chs[nick.channel].appendNick(nick.nick);
        }
        next_polling_count=1;
    }
    
    var last_response=document.getElementById("last_response_time");
    last_response.innerHTML=obj.response_time;
}


API.rawMessage=function(obj)
{
    var param=JSON.stringify(obj);
    new Ajax.Request("getch.cgi",{method:'post',parameters:param,onComplete:AjaxResponse});
}

API.getEvent=function()
{
    var rpc=new Object();
    rpc.method="getEvent";
    if(this.last_event_response_time)
    {
        rpc.last_response_time=this.last_event_response_time;
    }
    this.rawMessage(rpc);   
}

API.getChannel=function()
{
    var rpc=new Object();
    rpc.method="getChannel";
    this.rawMessage(rpc);
}

API.getNick=function()
{
    var rpc=new Object();
    rpc.method="getNick";
    this.rawMessage(rpc);
}

API.sendPrivmsg=function(channel,msg)
{
    var rpc=new Object();
    rpc.method="Privmsg";
    rpc.channel=channel;
    rpc.message=msg;
    this.rawMessage(rpc); 
}

API.sendJoin=function(channel)
{
    var rpc=new Object();
    rpc.method="Join";
    rpc.channel=channel;
    this.rawMessage(rpc); 
}

API.sendPart=function(channel,msg)
{
    var rpc=new Object();
    rpc.method="Part";
    rpc.channel=channel;
    rpc.message=msg;
    this.rawMessage(rpc); 
}


var Chs=new Object;

var Channel=function(name)
{
    this.name=name;
    this.scroll_pos=0;
    this.root=document.createElement("div");
    
    document.getElementById("channels").appendChild(this.root);
    this.msg=document.createElement("ul");
    this.root.appendChild(this.msg);
    this.root.setAttribute("class","entry");
    this.root.style.display="none";
    
    //channel tab
    var menu=document.getElementById("menu_list");
    var a=document.createElement("a");
    this.tab=document.createElement("li");
    a.setAttribute("href","#");
    a.setAttribute("onclick","change_channel('"+name+"')");
    a.innerHTML=name;
    this.tab.appendChild(a);
    menu.appendChild(this.tab);
    
    this.nick=document.createElement("ul");
    document.getElementById("nicknames").appendChild(this.nick);
    this.nick.style.display="none";

    this.show=function()
    {
        this.root.style.display="block";
        this.nick.style.display="block";
    }
    this.findNick=function(name)
    {
        var hit=0;
        for(var n=0;n<this.nick.childNodes.length;n++)
        {
            if(this.nick.childNodes[n].getAttribute("nickname")==name)
            {
                hit=1;
            }
        }
        return hit;
    }
    this.appendNick=function(name)
    {
        if(this.findNick(name)==0)
        {
            var li=document.createElement("li");
            li.setAttribute("class","nickname");
            li.setAttribute("nickname",name);
            li.innerHTML=name;
            this.nick.appendChild(li);
        }
    }
    this.removeNick=function(name)
    {
        for(var n=0;n<this.nick.childNodes.length;n++)
        {
            if(this.nick.childNodes[n].getAttribute("nickname")==name)
            {
                this.nick.removeChild(this.nick.childNodes[n]);
            }
        }
    }
}

function allhide()
{
    for(var i in Chs)
    {
        Chs[i].root.style.display="none";
        Chs[i].nick.style.display="none";
    }
}

function joinChannel()
{
    var form=document.getElementById("join_channel");
    if(form.value)
    {
        API.sendJoin(form.value);
        form.value="";
    }
}

function partChannel()
{
    if(active_channel)
    {
        API.sendPart(active_channel,"");
    }
}

function change_channel(chname)
{
    active_channel=chname;
    allhide();
    var sys=document.getElementById("system");
    var ch=document.getElementById("channel");
    sys.style.display="none";
    ch.style.display="block";
    Chs[chname].show();
}

function change_system()
{
    allhide();
    var sys=document.getElementById("system");
    var ch=document.getElementById("channel");
    sys.style.display="block";
    ch.style.display="none";
}

function on_load()
{
    API.getChannel();
}

function Say()
{
    var form=document.getElementById("say_command");
    if(active_channel && form.value)
    {
        API.sendPrivmsg(active_channel,form.value);
        form.value="";
        next_polling_count=1;
    }
}

function polling()
{
    if(next_polling_count <= 0)
    {    
        API.getEvent();
        next_polling_count=10;
    }
    next_polling_count--;
}

setInterval("polling()",1000);
