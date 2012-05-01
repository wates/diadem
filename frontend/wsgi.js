
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

function InText(s,text) {

    if (null == s.innerText) {
        s.textContent = text;
    }
    else {
        s.innerText = text;
    }
}

function AjaxResponse(res,type)
{
    var obj=eval("("+res+")");
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
    if (obj.method == "event" && (null==API.last_event_response_time || API.last_event_response_time < obj.response_time))
    {
        API.last_event_response_time=obj.response_time;
        for(var i=0;i<obj.event.length;i++)
        {
            var e=obj.event[i];
            if(e.command=="PRIVMSG")
            {
                 PrecCh(e.param1);
                 var li=document.createElement("li");
                 li.setAttribute("class", "privmsg");
                 InText(li,e.time.split(" ")[1]+" ["+e.prefix+"] "+e.param2);
                 Chs[e.param1].msg.appendChild(li);
                 Chs[e.param1].root.scrollTop+=17;
            }
            else if(e.command=="NOTICE")
            {
                 PrecCh(e.param1);
                 var li=document.createElement("li");
                 li.setAttribute("class","notice");
                 InText(li,e.time.split(" ")[1]+" ["+e.prefix+"] "+e.param2);
                 Chs[e.param1].msg.appendChild(li);
                 Chs[e.param1].root.scrollTop+=17;
            }
            else if(e.command=="JOIN")
            {
                 PrecCh(e.param1);
                 var li=document.createElement("li");
                 li.setAttribute("class","join");
                 InText(li,e.time.split(" ")[1]+" "+e.prefix+" join");
                 Chs[e.param1].msg.appendChild(li);
                 Chs[e.param1].root.scrollTop+=17;
                 Chs[e.param1].appendNick(e.prefix);
            }
            else if(e.command=="PART")
            {
                 PrecCh(e.param1);
                 var li=document.createElement("li");
                 li.setAttribute("class", "part");

                 var text=e.time.split(" ")[1]+" "+e.prefix+" part - ";
                 if(e.param2)
                 {
                     text+=e.param2;
                 }
                 InText(li,text);
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
                         InText(li,e.time.split(" ")[1]+" "+e.prefix+" nick -> "+e.param1);
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
                         li.setAttribute("class", "quit");
                         var text=e.time.split(" ")[1]+" "+e.prefix+" quit - ";
                         if(e.param1)
                         {
                             text+=e.param1;
                         }
                         InText(li, text);
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
    InText(last_response,obj.response_time);
}


API.rawMessage=function(obj)
{
    var param = JSON.stringify(obj);
    $.ajax({
        type: "POST",
        url: "getch.cgi",
        data: param,
        success: AjaxResponse
    });
    //new Ajax.Request("getch.cgi",{method:'post',parameters:param,onComplete:AjaxResponse});
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

var Channel = function (name) {
    this.name = name;
    this.scroll_pos = 0;
    this.root = document.createElement("div");

    document.getElementById("channels").appendChild(this.root);
    this.msg = document.createElement("ul");
    this.root.appendChild(this.msg);
    this.root.setAttribute("class", "entry");
    this.root.style.display = "none";

    //channel tab
    $("#menu_list").append("<li><button class=\"change_channel_button\">" + name + "</button></li>");

    this.nick = document.createElement("ul");
    document.getElementById("nicknames").appendChild(this.nick);
    this.nick.style.display = "none";

    this.show = function () {
        this.root.style.display = "block";
        this.nick.style.display = "block";
        this.root.scrollTop = this.scroll_pos;
    }
    this.findNick = function (name) {
        var hit = 0;
        for (var n = 0; n < this.nick.childNodes.length; n++) {
            if (this.nick.childNodes[n].getAttribute("nickname") == name) {
                hit = 1;
            }
        }
        return hit;
    }
    this.appendNick = function (name) {
        if (this.findNick(name) == 0) {
            var li = document.createElement("li");
            li.setAttribute("class", "nickname");
            li.setAttribute("nickname", name);
            li.innerHTML = name;
            this.nick.appendChild(li);
        }
    }
    this.removeNick = function (name) {
        for (var n = 0; n < this.nick.childNodes.length; n++) {
            if (this.nick.childNodes[n].getAttribute("nickname") == name) {
                this.nick.removeChild(this.nick.childNodes[n]);
            }
        }
    }
}

function allhide()
{
    for(var i in Chs) {
        if ("block" == Chs[i].root.style.display) {
            Chs[i].scroll_pos = Chs[i].root.scrollTop;
        }
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

function Keydown(e) {
    if (13 == e.keyCode) {
        Say();
        return false;
    }
}

function main() {
    $("#menu_system").live("click", change_system);
    $("#say_command").keydown(Keydown);
    $("#say_button").live("click", Say);
    $("#join_channel_button").live("click", joinChannel);
    $(".change_channel_button").live("click", function () { change_channel($(this).text()); });

    change_system();
    API.getChannel();
    setInterval("polling()", 1000);
}

$(main);

