/*jslint node: true */
"use strict";
var express = require('express'),
    bodyParser = require('body-parser'),
    oauthserver = require('oauth2-server'),
    OAuthDatabase = require("./database/oauth"),
    Sequelize = require("sequelize");

var database = new Sequelize('OSP', 'osp', 'osp', {
  host: 'localhost',
  dialect: 'postgres',
  pool: {
    max: 5,
    min: 0,
    idle: 10000
  }
});

var oAuthData = new OAuthDatabase(database);

var app = express();

app.use(bodyParser.urlencoded({ extended: true }));

app.use(bodyParser.json());

app.oauth = oauthserver({
  model: oAuthData, // See below for specification
  grants: ['password'],
  debug: true
});

app.all('/oauth/token', app.oauth.grant());

app.get('/', app.oauth.authorise(), function (req, res) {
  res.send('Secret area');
});

app.use(app.oauth.errorHandler());

app.listen(3000);