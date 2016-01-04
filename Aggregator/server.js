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

app.listen(3000);