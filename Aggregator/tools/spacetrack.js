/*jslint node: true */
"use strict";

var Q = require('q');
var request = require("request");

var SpaceTrack = function() {
  this.req = request.defaults({jar: true});
  this.LoginURL = "https://www.space-track.org/ajaxauth/login";
  this.TLEURL = "https://www.space-track.org/basicspacedata/query/class/tle_latest/ORDINAL/1/EPOCH/%3Enow-30/orderby/NORAD_CAT_ID/format/3le";

};

SpaceTrack.prototype.login = function(username, password) {
  var def = Q.defer();
  console.log("Trying to login to SpaceTrack");
  this.req.post(this.LoginURL, {
    form: {
      identity: username,
      password: password
    }
  }, function(error, response, body) {
    if (error || response.statusCode != 200)
      def.reject(error, response, body);
    else {
      console.log("Logged in to SpaceTrack");
      def.resolve(response, body);
    }
  });

  return def.promise;
};

SpaceTrack.prototype.downloadTLEs = function() {
  var def = Q.defer();

  this.req.get(this.TLEURL, function (error, response, body) {
    if (!error && response.statusCode == 200) {
      var tles = body.toString().match(/(.*\r\n){1,3}/g);
      console.log("Received "+tles.length+" TLEs");
      def.resolve(tles);
    } else {
      def.reject(error, response, body);
    }
  });

  return def.promise;
};

SpaceTrack.prototype.fillTLEs = function(tles, satdatabase) {
  var def = Q.defer();
  var updated = 0;
  var toUpdate = tles.length;
  var checkTLE = function() {
    if (updated === toUpdate) {
      process.stdout.write("\n");
      def.resolve();
    } else {
      process.stdout.write("Progress "+updated+"/"+toUpdate+" ("+Math.floor((updated/toUpdate)*100)+"%)\r");
      setTimeout(function() { checkTLE(); }, 300);
    }
  };

  for (var i in tles) {
    satdatabase.addOrUpdateTLE(tles[i].trim(), function(err, data) {
      if (err) {
        if (!data)
          console.error("Error adding TLE: "+err);
      } else {
        console.log("Added TLE for Satellite "+data.satellite_number+" ("+data.name+")");
      }
      updated++;
    });
  }

  setTimeout(function() { checkTLE(); }, 300);
  return def.promise;
};

module.exports = SpaceTrack;