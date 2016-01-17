/*jslint node: true */
"use strict";
var express = require('express'),
    Sequelize = require("sequelize"),
    SatelliteDatabase = require("./database/satellite"),
    BaseSatData = require("./database/basedata/basesatdata"),
    Q = require('q'),
    SpaceTrack = require("./tools/spacetrack");

/**
 *  For using it, export two environment variables:
 *  spacetrackuser -> Your Space Track User
 *  spacetrackpass -> Your Space Track Pass
 */

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
var spaceTrack = new SpaceTrack();

var updated = 0;
var toUpdate = 0;
tledata.initialize().then(function() {
  return initSatDB.initialize();
}).then(function() {
  var def = Q.defer();
  console.log("Base data initialization done!");
  if (SpaceTrackUser !== null && SpaceTrackPass !== null) {
    console.log("Space Track User/Pass found! Syncing");
    spaceTrack.login(SpaceTrackUser, SpaceTrackPass).then(function() {
      console.log("Downloading TLEs");
      return spaceTrack.downloadTLEs();
    }).then(function(tles) {
      console.log("Updating local database");
      return spaceTrack.fillTLEs(tles, tledata);
    }).then(function() {
      console.log("Finished adding TLEs from SpaceTrack");
      def.resolve();
    }).catch(function(error, response, body) {
      def.reject(error, response, body);
    });
  } else {
    def.resolve();
  }
  return def.promise;
}).then(function() {
  console.log("Initialization finished!");
  process.exit(0);
}).catch(function(error, data0, data1) {
  console.error("Error found: ",error);
});