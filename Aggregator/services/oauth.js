/*jslint node: true */
"use strict";

var bodyParser = require('body-parser');
var oauthserver = require('oauth2-server');
var OAuthDatabase = require("../database/oauth");

var OAuthService = function(app, sequelize) {
  console.log("Initializing OAuth Service");

  this.oAuthData = new OAuthDatabase(sequelize);

  app.use(bodyParser.urlencoded({ extended: true }));
  app.use(bodyParser.json());

  app.oauth = oauthserver({
    model: this.oAuthData,
    grants: ['password'],
    debug: true
  });

  app.imageserviceoauth = oauthserver({
    model: this.oAuthData,
    grants: ['image-service'],
    debug: true
  });

  app.all('/oauth/login', app.oauth.grant());
  app.all('/oauth/image-login', app.imageserviceoauth.grant());

  app.use(app.oauth.errorHandler());
  app.use(app.imageserviceoauth.errorHandler());

  console.log("OAuth Service Initialized");
};

module.exports = OAuthService;