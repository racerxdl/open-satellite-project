/*jslint node: true */
"use strict";

var bodyParser = require('body-parser');
var oauthserver = require('oauth2-server');
var OAuthDatabase = require("../database/oauth");
var Q = require('q');

var OAuthService = function(app, sequelize) {
  this.app = app;
  this.sequelize = sequelize;
};

OAuthService.prototype.initialize = function() {
  var _this = this;
  console.log("Initializing OAuth Service");
  this.oAuthData = new OAuthDatabase(this.sequelize);

  this.app.use(bodyParser.urlencoded({ extended: true }));
  this.app.use(bodyParser.json());

  this.app.oauth = oauthserver({
    model: this.oAuthData,
    grants: ['password'],
    debug: true
  });

  this.app.imageserviceoauth = oauthserver({
    model: this.oAuthData,
    grants: ['image-service'],
    debug: true
  });

  this.app.all('/oauth/login', this.app.oauth.grant());
  this.app.all('/oauth/image-login', this.app.imageserviceoauth.grant());

  this.app.use(this.app.oauth.errorHandler());
  this.app.use(this.app.imageserviceoauth.errorHandler());

  return this.oAuthData.initialize().then(function() {
    var def = Q.defer();
    console.log("OAuth Service Initialized");
    def.resolve();
    return def.promise;
  })
};

module.exports = OAuthService;