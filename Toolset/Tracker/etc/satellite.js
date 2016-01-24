/*jslint node: true */
"use strict";

var satjs = require("satellite.js").satellite;
var deg2rad = Math.PI / 180;

var Satellite = function(TLE) {
  this.TLE = TLE.split("\r\n");
  this.sat = this.TLE.length == 3 ? satjs.twoline2satrec(this.TLE[1], this.TLE[2]) : satjs.twoline2satrec(this.TLE[0], this.TLE[1]);
  this.events = {
    "outOfRange": [],
    "outOfAlmostInRange": [],
    "almostInRange": [],
    "inRange": []
  };

  this.inRange = false;
  this.almostInRange = false;
};

Satellite.prototype.update = function(time, observer) {
  time = time || new Date();

  var positionAndVelocity = satjs.propagate(
    this.sat,
    time.getUTCFullYear(),
    time.getUTCMonth() + 1, // Note, this function requires months in range 1-12.
    time.getUTCDate(),
    time.getUTCHours(),
    time.getUTCMinutes(),
    time.getUTCSeconds()
  );

  this.gmst = satjs.gstimeFromDate(
    time.getUTCFullYear(),
    time.getUTCMonth() + 1, // Note, this function requires months in range 1-12.
    time.getUTCDate(),
    time.getUTCHours(),
    time.getUTCMinutes(),
    time.getUTCSeconds()
  );

  this.positionEci = positionAndVelocity.position;
  this.velocityEci = positionAndVelocity.velocity;
  this.position = satjs.eciToEcf(positionAndVelocity.position, this.gmst);
  this.velocity = satjs.eciToEcf(positionAndVelocity.velocity, this.gmst);

  var coord = satjs.eciToGeodetic(this.positionEci, this.gmst);
  this.latitude = coord.latitude / deg2rad;
  this.longitude = coord.longitude / deg2rad;
  this.altitude = coord.height;

  this.latitude = this.latitude  < -90 ? this.latitude + 180 : this.latitude;
  this.longitude = this.longitude < -179.9 ? this.longitude + 360 : this.longitude;

  if (observer !== null && observer !== undefined) {
    this.observerEcf    = satjs.geodeticToEcf(observer);
    this.lookAngles     = satjs.ecfToLookAngles(observer, this.position);
    this.dopplerFactor  = satjs.dopplerFactor(this.observerEcf, this.position, this.velocity);

    if (this.inRange != this.lookAngles.elevation > 0) {
      this.inRange      = this.lookAngles.elevation > 0;
      if (inRange) {
        this.callEvent("inRange");
      } else {
        this.callEvent("outOfRange");
      }
    }

    if (this.almostInRange != this.lookAngles.elevation > -0.2) {
      this.almostInRange = this.lookAngles.elevation > -0.2;
      if (almostInRange) {
        this.callEvent("almostInRange");
      } else {
        this.callEvent("outOfAlmostInRange");
      }
    }
  }
};

Satellite.prototype.callEvent = function(name, data) {
  if (this.events.hasOwnProperty(name)) {
    for (var i in this.events[name]) {
      this.events[name][i](data);
    }
  }
};

Satellite.prototype.on = function(name, cb) {
  if (this.events.hasOwnProperty(name)) {
    this.events[name].push(function(data) {
      cb(data);
    });
  }
};

Satellite.prototype.clearEvents = function() {
  this.events = {
    "almostInRange": [],
    "inRange": []
  };
};

module.exports = Satellite;