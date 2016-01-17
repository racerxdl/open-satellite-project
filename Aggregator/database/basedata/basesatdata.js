/*jslint node: true */
"use strict";

var fs = require("fs");
var csv = require("fast-csv");
var Q = require('q');

/**
 *  Base TLE Data points to tledata.txt
 *  Base Transponder Data points to Transponders.csv
 *  Base TrackSats Data points to TrackSats.csv
 **/
var InitializeBaseData = function(satdatabase, basetledata, basetransdata, basetrackdata) {
  this.basetledata = basetledata;
  this.basetransdata = basetransdata;
  this.basetrackdata = basetrackdata;
  this.satdatabase = satdatabase;
};

InitializeBaseData.prototype.initialize = function() {
  var _this = this;
  this.def = Q.defer();

  console.log("Initializing base data");

  fs.readFile(this.basetledata, 'utf8', function(err, data) {
    if (err) throw err;
    data = data.toString().match(/(.*\r\n){1,3}/g);
    _this.addedData = 0;
    _this.totalData = data.length;
    for (var i in data) {
      _this.satdatabase.addOrUpdateTLE(data[i].trim(), function(err, tledata) {
        if (err) {
          if (!tledata)
            console.error("Error adding TLE: "+err, data[i].trim());
        } else {
          console.log("Added TLE for Satellite "+tledata.satellite_number+" ("+tledata.name+")");
        }
        _this.addedData++;
      });
    }

    setTimeout(function() {
      _this.checkAddedTLE();
    }, 100);
  });

  return this.def.promise;
}

InitializeBaseData.prototype.checkAddedTLE = function() {
  var _this = this;
  process.stdout.write("Progress "+this.addedData+"/"+this.totalData+" ("+Math.floor((this.addedData/this.totalData)*100)+"%)\r");
  if (this.addedData != this.totalData) {
    setTimeout(function() {
      _this.checkAddedTLE();
    }, 100);
  } else {
    console.log("Finished adding TLE data                 ");
    this.addTransponderData();
  }
};

InitializeBaseData.prototype.addTransponderData = function() {
  var _this = this;
  var stream = fs.createReadStream(this.basetransdata);
  this.addedData = 0;
  this.totalData = 0;

  csv.fromStream(stream, {headers : true})
  .on("data", function(data){
    _this.totalData++;
    _this.satdatabase.addOrUpdateTransponder(data, function(err, tdata) {
      if (err)
        console.error("Error adding transponder "+data.name+": ",err);
      _this.addedData++;
    });
  }).on("end", function(){
    setTimeout(function() {
      _this.checkAddedTransponder();
    }, 100);
  });
};

InitializeBaseData.prototype.checkAddedTransponder = function() {
  var _this = this;
  process.stdout.write("Progress "+this.addedData+"/"+this.totalData+" ("+Math.floor((this.addedData/this.totalData)*100)+"%)\r");
  if (this.addedData != this.totalData) {
    setTimeout(function() {
      _this.checkAddedTransponder();
    }, 100);
  } else {
    console.log("Finished adding Transponder data                 ");
    this.addTrackSatData();
  }
};

InitializeBaseData.prototype.addTrackSatData = function() {
  var _this = this;
  var stream = fs.createReadStream(this.basetrackdata);
  this.addedData = 0;
  this.totalData = 0;

  csv.fromStream(stream, {headers : true})
  .on("data", function(data){
    _this.totalData++;
    _this.satdatabase.addOrUpdateTrackSat(data, function(err, tdata) {
      if (err)
        console.error("Error adding Satellite Track "+data.name+": ",err);
      _this.addedData++;
    });
  }).on("end", function(){
    setTimeout(function() {
      _this.checkAddedSatTrack();
    }, 100);
  });
};

InitializeBaseData.prototype.checkAddedSatTrack = function() {
  var _this = this;
  process.stdout.write("Progress "+this.addedData+"/"+this.totalData+" ("+Math.floor((this.addedData/this.totalData)*100)+"%)\r");
  if (this.addedData != this.totalData) {
    setTimeout(function() {
      _this.checkAddedSatTrack();
    }, 100);
  } else {
    console.log("Finished adding Satellite Track Data data                 ");
    this.finish();
  }
};

InitializeBaseData.prototype.finish = function() {
  console.log("Finish initialization");
  this.def.resolve();
};

module.exports = InitializeBaseData;