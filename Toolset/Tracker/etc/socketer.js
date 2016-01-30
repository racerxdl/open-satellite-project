/*jslint node: true */
"use strict";
var dgram = require('dgram');

var Socketer = function(trackerName) {
  var _this = this;

  this.trackerName = trackerName || "OSP-Tracker";
  this.broadcastPort = 6024;
  this.broadcastAddr = "127.255.255.255";
  this.server = dgram.createSocket("udp4");

  this.server.bind(function() {
    _this.server.setBroadcast(true);
    setInterval(function() {
      _this.__keepAlive();
    }, 10000);
  });
};

Socketer.prototype.__keepAlive = function() {
  this.notify("keepAlive", {
    "timeStamp": (new Date()).getTime(),
    "trackerName": this.trackerName
  });
};

Socketer.prototype.notify = function(name, data) {
  var packetData = {"name": name, "data": data};
  var packet = new Buffer(JSON.stringify(packetData));
  this.server.send(packet, 0, packet.length, this.broadcastPort, this.broadcastAddr, function() {});
};

module.exports = Socketer;