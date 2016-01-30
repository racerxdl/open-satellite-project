/*jslint node: true */
"use strict";

var Q = require('q');

var TrackService = function(app, satdatabase) {
  this.app = app;
  this.satdatabase = satdatabase;
};

TrackService.prototype.initialize = function() {
  var _this = this;
  var def = Q.defer();
  console.log("Initializing Satellite Tracking Service");

  this.app.get('/track-service/track-list', function (req, res) {
    _this.satdatabase.getTrackSats(function(error, data) {
      if (error) {
        return res.status(500).json({'error': error.toString()});
      } else {
        return res.status(200).json({'error': null, 'data': data});
      }
    });
  });

  this.app.get('/track-service/satdata/:satnumber/transponders', function(req, res) {
    console.log("Satellite number: "+req.params.satnumber);
    _this.satdatabase.getTransponders(req.params.satnumber, function(err, data) {
      res.status(200).json(data);
    });
  });

  def.resolve();

  return def.promise;
};

module.exports = TrackService;