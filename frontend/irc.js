function IRC() {
  this.channels = new Array;
}

IRC.prototype.getChannel = function (name) {
  for (var i = 0; i < this.channels.length; i++) {
    if (this.channels[i].name == name) {
      return this.channels[i];
    }
  }
  var ch = new Channel(name);
  this.channels.push(ch);
  return ch;
}

IRC.prototype.showChannel = function (name) {
  $("#system").hide();
  for (var i = 0; i < this.channels.length; i++) {
    if (this.channels[i].name == name) {
      this.channels[i].root.show();
    }
    else {
      this.channels[i].root.hide();
    }
  }
  $("#channel").show();
}

IRC.prototype.showAll = function () {
  $("#system").show();
  for (var i = 0; i < this.channels.length; i++) {
    this.channels[i].root.show();
  }
  $("#channel").show();
}

var irc = new IRC();



var API = new Object;
var lastres;

API.sendMessage = function (obj) {
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
  if (obj.motd) {
    $("#motd").text(obj.motd);
  }
  if (obj.method == "channel") {
    if (obj.channel) {
      for (var i = 0; i < obj.channel.length; i++) {
        var ch = irc.getChannel(obj.channel[i].name);
        for (var j = 0; j < obj.channel[i].nick.length; j++) {
          ch.appendNick(obj.channel[i].nick[j].nick, obj.channel[i].nick[j].op);
        }
      }
    }
  }
  if (obj.method == "event" && (null == API.last_event_response_time || API.last_event_response_time < obj.response_time)) {
    API.last_event_response_time = obj.response_time;
    for (var i = 0; i < obj.event.length; i++) {
      var e = obj.event[i];
      if (e.command == "PRIVMSG") {
        var ch = irc.getChannel(e.param1);
        var li = $("<li>")
          .attr("class", "primsg")
          .text(e.time.split(" ")[1] + " [" + e.prefix + "] " + e.param2);
        ch.log.append(li);
        ch.root.scrollTop += 17;
      }
      else if (e.command == "NOTICE") {
        var ch = irc.getChannel(e.param1);
        var li = $("<li>")
          .attr("class", "notice")
          .text(e.time.split(" ")[1] + " [" + e.prefix + "] " + e.param2);
        ch.log.append(li);
        ch.root.scrollTop += 17;
      }
      else if (e.command == "JOIN") {
        var ch = irc.getChannel(e.param1);
        var li = $("<li>")
          .attr("class", "join")
          .text(e.time.split(" ")[1] + " " + e.prefix + " join");
        ch.log.append(li);
        ch.root.scrollTop += 17;
        ch.appendNick(e.prefix);
      }
      else if (e.command == "PART") {
        var ch = irc.getChannel(e.param1);
        var li = $("<li>")
            .attr("class", "part")
            .text(e.time.split(" ")[1] + " " + e.prefix + " part - " + e.param2);
        ch.log.append(li);
        ch.root.scrollTop += 17;
        ch.removeNick(e.prefix);
      }
      else if (e.command == "NICK") {
        for (var i = 0; i < channels.length; i++) {
          if (irc.channels[i].findNick(e.prefix) == 1) {
            var ch = irc.getChannel(e.param1);
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


API.rawMessage = function (obj) {
  var param = JSON.stringify(obj);
  $.ajax({
    type: "POST",
    url: "gw.cgi",
    data: param,
    success: AjaxResponse
  });
}

API.getEvent = function () {
  var rpc = new Object();
  rpc.method = "getEvent";
  if (this.last_event_response_time) {
    rpc.last_response_time = this.last_event_response_time;
  }
  this.rawMessage(rpc);
}

API.getChannel = function () {
  var rpc = new Object();
  rpc.method = "getChannel";
  this.rawMessage(rpc);
}

API.sendPrivmsg = function (channel, msg) {
  var rpc = new Object();
  rpc.method = "Privmsg";
  rpc.channel = channel;
  rpc.message = msg;
  this.rawMessage(rpc);
}

API.sendJoin = function (channel) {
  var rpc = new Object();
  rpc.method = "Join";
  rpc.channel = channel;
  this.rawMessage(rpc);
}

API.sendPart = function (channel, msg) {
  var rpc = new Object();
  rpc.method = "Part";
  rpc.channel = channel;
  rpc.message = msg;
  this.rawMessage(rpc);
}

var Channel = function (name) {
  this.root_id = "channel_" + name.slice(1);
  this.root = $("#template_channel").clone();
  this.root.attr("id", this.root_id);
  this.name = name;
  this.button_id = "button_" + name.slice(1);
  this.nick_list = new Array;
  this.scroll_pos = 0;
  this.log = this.root.find(".channel_log");

  $("#menu_list").append($("<div>")
    .attr('class', 'span1')
      .append($("<button>")
      .attr('id', this.button_id)
      .attr('class', "btn channel_button ")
      .text(name)));

  this.root.find(".channel_name").text(name);
  this.root.find(".channel_log").flickable();

  $("#" + this.button_id).live("click", function () { irc.showChannel(name); });

  $("#channel").append(this.root);

  this.updateNickList = function () {
    var nick = this.root.find(".channel_nick");
    nick.empty();
    for (var i = 0; i < this.nick_list.length; i++) {
      nick.append($("<li>")
        .text(this.nick_list[i].name));
    }
  }
  this.findNick = function (name) {
    var hit = 0;
    return hit;
  }
  this.appendNick = function (name, op) {
    this.nick_list.push({
      name: name,
      op: op
    });
    this.updateNickList();
  }
  this.removeNick = function (name) {
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
  $("#menu_system").live("click", function () { $("#channel").hide(); $("#system").show(); });
  $("#menu_all").live("click", function () { irc.showAll(); });
  $("#say_command").keydown(Keydown);
  $("#say_button").live("click", Say);
  $("#join_channel_button").live("click", joinChannel);

  API.getChannel();
  setInterval("polling()", 20000);
}

$(main);

