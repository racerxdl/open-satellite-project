/*jslint node: true */
"use strict";

// Configuration

var config = {
  trackerName: "OSP Alpha Tracker",
  apiserver: "http://localhost:3000",
  stationCoord: {
    latitude: -23.643794 * (Math.PI / 180),
    longitude: -46.653103 * (Math.PI / 180),
    height: 0.762                     //  km
  },
  generalCheckRate: 300,              //  0.3s
  loopRate: 500,                      //  1s
  showTransponders: false
};

var globalVars = {
  satsToTrack: []
};


// Code

var Q = require('q');
var request = require("request");
var Satellite = require("./etc/satellite");
var Colors = require("./etc/colors");
var Socketer = require("./etc/socketer");


var socketerServer = new Socketer(config.trackerName);

function getTrackList() {
  var def = Q.defer();

  var req = request.defaults();
  req.get(config.apiserver+"/track-service/track-list", function (error, response, body) {
    if (!error && response.statusCode == 200) {
      def.resolve([error, response, JSON.parse(body)]);
    } else {
      def.reject([error, response, body]);
    }
  });

  return def.promise;
}

function fixToN(v, n) {
  var t = 1;
  for(var i=0;i<n;i++)
    t *= 10;
  return Math.round(v * t) / t;
}

function trackLoop() {
  process.stdout.write("\x1B[2J"); // Clear Scren
  for(var i in globalVars.satsToTrack) {
    var sat = globalVars.satsToTrack[i];
    sat.sat.update(null, config.stationCoord);
    var secColor = sat.sat.inRange ? Colors.green :  ( sat.sat.almostInRange ? Colors.yellow : Colors.lightgray );
    console.log(secColor + sat.TLE.name + " [" + sat.satellite_number + "] :" + Colors.reset);
    console.log(secColor + "    Doppler Effect Factor: " + sat.sat.dopplerFactor + Colors.reset);
    console.log(secColor + "    Coordinates: " + fixToN(sat.sat.latitude, 6) + ", " + fixToN(sat.sat.longitude, 6) + "," + fixToN(sat.sat.altitude, 3) + Colors.reset);
    console.log(secColor + "    View Angles: Az=" + fixToN(sat.sat.lookAngles.azimuth, 4) + ",Ele=" + fixToN(sat.sat.lookAngles.elevation, 4) + ",range=" + fixToN(sat.sat.lookAngles.rangeSat, 2) + Colors.reset);
    if (config.showTransponders &&  sat.transponders.length > 0) {
      console.log(secColor + "    Transponders: " + Colors.reset);
      for (var i in sat.transponders) {
        var transponder = sat.transponders[i];
        console.log(secColor + "        " + transponder.name + " - D: " + transponder.downlink + " MHz - U: " + transponder.uplink + "MHz");
      }
    }
  }

  setTimeout(function() { trackLoop(); }, config.loopRate);
}

function loadTransponders(sat) {
  var def = Q.defer();

  var req = request.defaults();
  req.get(config.apiserver+"/track-service/satdata/"+sat.satellite_number+"/transponders", function (error, response, body) {
    if (!error && response.statusCode == 200) {
      sat.transponders = JSON.parse(body);
      def.resolve();
    } else {
      def.reject(error);
    }
  });

  return def.promise;
}

getTrackList().spread(function(error, response, body) {
  var def = Q.defer();

  globalVars.satsToTrack = body.data;
  console.log("Received "+globalVars.satsToTrack.length+" to track.");
  var satsToLoad = globalVars.satsToTrack.length;
  var satsLoaded = 0;

  function transLoaded(err) {
    if(err) {
      console.error(err.stack);
    }
    satsLoaded++;
  }

  for (var i in globalVars.satsToTrack) {
    var sat = globalVars.satsToTrack[i];
    console.log(" Tracking "+sat.TLE.name);
    sat.sat = new Satellite(sat.TLE.tle_data);
    loadTransponders(sat).then(transLoaded).catch(transLoaded);

    sat.sat.on("inRange", function() {
      socketerServer.notify("inRange", sat);
    });

    sat.sat.on("outOfRange", function() {
      socketerServer.notify("outOfRange", sat);
    });

    sat.sat.on("almostInRange", function() {
      socketerServer.notify("almostInRange", sat);
    });

    sat.sat.on("outOfAlmostInRange", function() {
      socketerServer.notify("outOfAlmostInRange", sat);
    });
  }

  function checkLoadedSats() {
    if (satsToLoad === satsLoaded) {
      return def.resolve();
    } else {
      setTimeout(function() {checkLoadedSats();}, config.generalCheckRate);
    }
  }

  console.log("Waiting for all transponders to be loaded.");
  checkLoadedSats();

  return def.promise;
}).then(function() {
  console.log(globalVars.satsToTrack);
  trackLoop();
}).catch(function(error) {
  console.error(error.stack);
});