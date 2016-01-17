/*jslint node: true */
"use strict";
var express = require('express'),
    Sequelize = require("sequelize"),
    OAuthService = require("./services/oauth"),
    ImageUploadService = require("./services/image-upload");

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
var oauthservice = new OAuthService(app, database);
var imageuploadservice = new ImageUploadService(app, database);

oauthservice.initialize().then(function() {
  return imageuploadservice.initialize();
}).then(function() {
  console.log("Starting server at 0.0.0.0:3000");
  app.listen(3000);
});