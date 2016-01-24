/*jslint node: true */
"use strict";
var express = require('express'),
    Sequelize = require("sequelize"),
    SatelliteDatabase = require("./database/satellite"),
    OAuthService = require("./services/oauth"),
    ImageUploadService = require("./services/image-upload"),
    TrackService = require("./services/track-service");

var database = new Sequelize('OSP', 'osp', 'osp', {
  host: 'localhost',
  dialect: 'postgres',
  pool: {
    max: 5,
    min: 0,
    idle: 10000
  }
});

var app = express();
var satdatabase = new SatelliteDatabase(database);
var oauthservice = new OAuthService(app, database);
var imageuploadservice = new ImageUploadService(app, database);
var trackingservice = new TrackService(app, satdatabase);

oauthservice.initialize().then(function() {
  return imageuploadservice.initialize();
}).then(function() {
  return trackingservice.initialize();
}).then(function() {
  console.log("Starting server at 0.0.0.0:3000");
  app.listen(3000);
});