/*jslint node: true */
"use strict";
var express = require('express'),
    Sequelize = require("sequelize"),
    SatelliteDatabase = require("./database/satellite"),
    request = require("request"),
    BaseSatData = require("./database/basedata/basesatdata"),
    Q = require('q');

/**
 *  For using it, export two environment variables:
 *  spacetrackuser -> Your Space Track User
 *  spacetrackpass -> Your Space Track Pass
 */

var SpaceTrackLoginUrl = "https://www.space-track.org/ajaxauth/login";
var SpaceTrackUrl = "https://www.space-track.org/basicspacedata/query/class/tle_latest/ORDINAL/1/EPOCH/%3Enow-30/orderby/NORAD_CAT_ID/format/3le";
var SpaceTrackUser = process.env.spacetrackuser;
var SpaceTrackPass = process.env.spacetrackpass;

var database = new Sequelize('OSP', 'osp', 'osp', {
  host: 'localhost',
  dialect: 'postgres',
  logging: false,
  pool: {
    max: 5,
    min: 0,
    idle: 10000
  }
});

var tledata = new SatelliteDatabase(database);
var initSatDB = new BaseSatData(tledata, "./database/basedata/tledata.txt", "./database/basedata/Transponders.csv", "./database/basedata/TrackSats.csv");
var updated = 0;
var toUpdate = 0;
tledata.initialize().then(function() {
  return initSatDB.initialize();
}).then(function() {
  var def = Q.defer();
  console.log("Base data initialization done!");
  if (SpaceTrackUser !== null && SpaceTrackPass !== null) {
    console.log("Space Track User/Pass found! Syncing");
    var req = request.defaults({jar: true});
    console.log("Logging into SpaceTrack. Please only run this at maximum of one per day.");

    var checkTLE = function() {
      if (updated === toUpdate) {
        process.stdout.write("\n");
        def.resolve();
      } else {
        process.stdout.write("Progress "+updated+"/"+toUpdate+" ("+Math.floor((updated/toUpdate)*100)+"%)\r");
        setTimeout(function() { checkTLE(); }, 300);
      }
    };

    req.post(SpaceTrackLoginUrl, {
      form: {
        identity: SpaceTrackUser,
        password: SpaceTrackPass
      }
    }, function(error, response, body) {
      if (response.statusCode != 200) {
        console.error("Error: ",error,body);
        return;
      }
      console.log("Logged in. Downloading TLEs");
      req.get(SpaceTrackUrl, function (error, response, body) {
        if (!error && response.statusCode == 200) {
          var tles = body.toString().match(/(.*\r\n){1,3}/g);
          toUpdate = tles.length;
          updated = 0;
          for (var i in tles) {
            tledata.addOrUpdateTLE(tles[i].trim(), function(err, data) {
              if (err) {
                if (!data)
                  console.error("Error adding TLE: "+err);
              } else {
                console.log("Added TLE for Satellite "+data.satellite_number+" ("+data.name+")");
              }
              updated++;
            });
            setTimeout(function() { checkTLE(); }, 300);
          }
        }
      });
    });
  } else {
    def.resolve();
  }
  return def.promise;
}).then(function() {
  console.log("Initialization finished!");
  process.exit(0);
})