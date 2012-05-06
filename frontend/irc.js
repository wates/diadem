
var IRC = new Object();

var API = new Object;
var channels = new Array;
var lastres;

function getChannel(name) {
  for (var i = 0; i < channels.length; i++) {
    if (channels[i].name == name) {
      return channels[i];
    }
  }
  var ch = new Channel(name);
  channels.push(ch);
  return ch;
}

function showChannel(name) {
  $("#system").hide();
  for (var i = 0; i < channels.length; i++) {
    if (channels[i].name == name) {
      channels[i].root.show();
    }
    else {
      channels[i].root.hide();
    }
  }
  $("#channel").show();
}

function showAll() {
  $("#system").show();
  for (var i = 0; i < channels.length; i++) {
    channels[i].root.show();
  }
  $("#channel").show();
}


API.sendMessage = function(obj) {
  var param = JSON.stringify(obj);
  $.ajax({
    type: "POST",
    url: "gw.cgi",
    data: param,
    success: AjaxResponse
  });
}

function AjaxResponse(res, type) {
  try {
    var obj = JSON.parse(res);
  } catch (n) {
    return;
  }
  if (obj.method == "channel") {
    if (obj.channel) {
      for (var i = 0; i < obj.channel.length; i++) {
        getChannel(obj.channel[i].name);
      }
    }
  }
  if (obj.method == "event" && (null == API.last_event_response_time || API.last_event_response_time < obj.response_time)) {
    API.last_event_response_time = obj.response_time;
    for (var i = 0; i < obj.event.length; i++) {
      var e = obj.event[i];
      if (e.command == "PRIVMSG") {
        var ch = getChannel(e.param1);
        var li = $("<li>")
          .attr("class", "primsg")
          .text(e.time.split(" ")[1] + " [" + e.prefix + "] " + e.param2);
        ch.log.append(li);
        ch.root.scrollTop += 17;
      }
      else if (e.command == "NOTICE") {
      var ch = getChannel(e.param1);
      var li = $("<li>")
          .attr("class", "notice")
          .text(e.time.split(" ")[1] + " [" + e.prefix + "] " + e.param2);
      ch.log.append(li);
      ch.root.scrollTop += 17;
    }
      else if (e.command == "JOIN") {
        var ch=getChannel(e.param1);
        var li = $("<li>")
          .attr("class", "join")
          .text( e.time.split(" ")[1] + " " + e.prefix + " join");
        ch.log.append(li);
        ch.root.scrollTop += 17;
//        Chs[e.param1].appendNick(e.prefix);
      }
      else if (e.command == "PART") {
        var ch = getChannel(e.param1);
        var li = $("<li>")
            .attr("class", "part")
            .text(e.time.split(" ")[1] + " " + e.prefix + " part - " + e.param2);
        ch.log.append(li);
        ch.root.scrollTop += 17;
//        Chs[e.param1].removeNick(e.prefix);
      }
      else if (e.command == "NICK") {
        for (var i = 0; i < channels.length; i++) {
          if (channels[i].findNick(e.prefix) == 1) {
            var ch = getChannel(e.param1);
            var li = $("<li>")
            .attr("class", "nick")
            .text(e.time.split(" ")[1] + " " + e.prefix + " nick -> " + e.param1);
//            Chs[c].removeNick(e.prefix);
//            Chs[c].appendNick(e.param1);
          }
        }
      }
      else if (e.command == "QUIT") {
        for (var c in Chs) {
          if (Chs[c].findNick(e.prefix) == 1) {
            var li = document.createElement("li");
            li.setAttribute("class", "quit");
            var text = e.time.split(" ")[1] + " " + e.prefix + " quit - ";
            if (e.param1) {
              text += e.param1;
            }
            InText(li, text);
            Chs[c].removeNick(e.prefix);
            Chs[c].msg.appendChild(li);
            Chs[c].root.scrollTop += 17;
          }
        }
      }
    }
  }
  if (obj.method == "nick") {
    for (var i = 0; i < obj.nick.length; i++) {
      var nick = obj.nick[i];
      Chs[nick.channel].appendNick(nick.nick);
    }
    next_polling_count = 1;
  }

  $("#last_response_time").text(obj.response_time);
}


API.rawMessage = function(obj) {
  var param = JSON.stringify(obj);
  $.ajax({
    type: "POST",
    url: "gw.cgi",
    data: param,
    success: AjaxResponse
  });
}

API.getEvent = function() {
  var rpc = new Object();
  rpc.method = "getEvent";
  if (this.last_event_response_time) {
    rpc.last_response_time = this.last_event_response_time;
  }
  this.rawMessage(rpc);
}

API.getChannel = function() {
  var rpc = new Object();
  rpc.method = "getChannel";
  this.rawMessage(rpc);
}

API.sendPrivmsg = function(channel, msg) {
  var rpc = new Object();
  rpc.method = "Privmsg";
  rpc.channel = channel;
  rpc.message = msg;
  this.rawMessage(rpc);
}

API.sendJoin = function(channel) {
  var rpc = new Object();
  rpc.method = "Join";
  rpc.channel = channel;
  this.rawMessage(rpc);
}

API.sendPart = function(channel, msg) {
  var rpc = new Object();
  rpc.method = "Part";
  rpc.channel = channel;
  rpc.message = msg;
  this.rawMessage(rpc);
}

var Channel = function(name) {
  this.name = name;
  this.scroll_pos = 0;
  var short_name = name.slice(1);

  $("#menu_list").append($("<div>")
    .attr('class', 'span1')
      .append($("<button>")
      .attr('id', "button_" + short_name)
      .attr('class', "btn channel_button ")
      .text(name)));

  this.root = $("#template_channel").clone();
  this.root.attr("id", "");
  this.log = this.root.find(".channel_log");

  this.root.find(".channel_name").text(name);
  $("#button_" + short_name).live("click", function() { showChannel(name); });

  $("#channel").append(this.root);

  this.findNick = function(name) {
    var hit = 0;
    return hit;
  }
  this.appendNick = function(name) {
  }
  this.removeNick = function(name) {
  }
}

function joinChannel() {
  var form = document.getElementById("join_channel");
  if (form.value) {
    API.sendJoin(form.value);
    form.value = "";
  }
}

function partChannel() {
  if (active_channel) {
    API.sendPart(active_channel, "");
  }
}

function Say() {
  var form = document.getElementById("say_command");
  if (active_channel && form.value) {
    API.sendPrivmsg(active_channel, form.value);
    form.value = "";
    next_polling_count = 1;
  }
}

function polling() {
  API.getEvent();
}

function Keydown(e) {
  if (13 == e.keyCode) {
    Say();
    return false;
  }
}

function main() {
  $("#menu_system").live("click", function() { $("#channel").hide(); $("#system").show(); });
  $("#menu_all").live("click", showAll);
  $("#say_command").keydown(Keydown);
  $("#say_button").live("click", Say);
  $("#join_channel_button").live("click", joinChannel);

  API.getChannel();
  setInterval("polling()", 5000);
}

$(main);

