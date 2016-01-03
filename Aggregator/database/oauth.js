/*jslint node: true */
"use strict";

var Sequelize = require("sequelize");
var bcrypt = require('bcrypt');

function encryptPassword(password) {
  return bcrypt.hashSync(password, 8);
}

var OAuthDatabase = function(sequelize) {

  this.OAuthAcessToken = sequelize.define("OAuthAcessToken", {
    access_token: { type: Sequelize.TEXT, allowNull: false, primaryKey: true },
    client_id: { type: Sequelize.TEXT, allowNull: false },
    user_id: { type: Sequelize.UUID, allowNull: false},
    expires: { type: Sequelize.DATE, allowNull: false}
  });

  this.OAuthClient = sequelize.define("OAuthClient", {
    client_id: { type: Sequelize.TEXT, allowNull: false, primaryKey: true, unique: "client_unique" },
    client_secret: { type: Sequelize.TEXT, allowNull: false, unique: "client_unique" },
    redirect_uri: { type: Sequelize.TEXT, allowNull: false },
    permissions: { type: Sequelize.TEXT, defaultValue: '[]',
      get: function() {
        return JSON.parse(this.getDataValue("permissions"));
      },
      set: function(val) {
        this.setDataValue('password', JSON.stringify(val));
      }
    }
  });

  this.OAuthRefreshToken = sequelize.define("OAuthRefreshToken", {
    refresh_token: { type: Sequelize.TEXT, allowNull: false, primaryKey: true },
    client_id: { type: Sequelize.TEXT, allowNull: false },
    user_id: { type: Sequelize.UUID, allowNull: false},
    expires: { type: Sequelize.DATE, allowNull: false}
  });

  this.User = sequelize.define("User", {
    id: { type: Sequelize.UUID, allowNull: false, primaryKey: true },
    username: { type: Sequelize.TEXT, allowNull: false },
    password: { type: Sequelize.TEXT, allowNull: false,
      set: function(val) {
        return this.setDataValue('password', encryptPassword(val));
      }
    }
  }, {
    indexes: [
      {
        name: 'users_username_password',
        method: 'BTREE',
        fields: ['username', 'password']
      }
    ],
    instanceMethods: {
      checkPassword: function(password) {
        return bcrypt.compareSync(password, this.password);
      }
    }
  });

  this.OAuthAcessToken.sync();
  this.OAuthClient.sync();
  this.OAuthRefreshToken.sync();
  this.User.sync();
  this.cleanExpiredTokens();
};

OAuthDatabase.prototype.cleanExpiredTokens = function() {
  this.OAuthAcessToken.findAll({
    where: {
      expires: {
        $lte: new Date()
      }
    }
  }).then(function(expired) {
    for (var i in expired)
      expired[i].destroy();
  });
};


OAuthDatabase.prototype.getAccessToken = function (bearerToken, callback) {
  if (bearerToken === null)
    return callback("Token not found");

  this.OAuthAcessToken.findOne({
    where: {
      access_token: bearerToken
    }
  }).then(function(tokenData) {
    if (tokenData === null)
      return callback("Token not found");

    callback(null, {
      accessToken: tokenData.access_token,
      clientId: tokenData.client_id,
      expires: tokenData.expires,
      userId: tokenData.userId
    });
  });
};

OAuthDatabase.prototype.getClient = function (clientId, clientSecret, callback) {
  if (clientSecret === null || clientId === null)
    return callback();

  this.OAuthClient.find({
    where: {
      client_id: clientId
    }
  }).then(function(client) {
    if (client === null)
      return callback("Client not found");

    if (client.client_secret !== clientSecret) return callback();

    callback(null, {
      clientId: client.client_id,
      clientSecret: client.client_secret
    });
  });
};

OAuthDatabase.prototype.getRefreshToken = function (bearerToken, callback) {
  if (bearerToken === null)
    return callback("Token not found", false);

  this.OAuthRefreshToken.find({
    where: {
      refresh_token: bearerToken
    }
  }).then(function(refreshToken) {
    if (refreshToken === null)
      return callback("Token not found", false);

    callback(null, refreshToken);
  });
};

OAuthDatabase.prototype.grantTypeAllowed = function (clientId, grantType, callback) {
  this.OAuthClient.find({
    where: {
      client_id: clientId
    }
  }).then(function(client) {
    return callback(false, client.permissions.indexOf(grantType) >= 0);
  });
};

OAuthDatabase.prototype.saveAccessToken = function (accessToken, clientId, expires, user, callback) {
  this.OAuthAcessToken.build({
    access_token: accessToken,
    client_id: clientId,
    user_id: user.id,
    expires: expires
  }).save().then(function() {
    callback(null);
  }).catch(function(error) {
    callback(error);
  });
};

OAuthDatabase.prototype.saveRefreshToken = function (refreshToken, clientId, expires, userId, callback) {
  this.OAuthRefreshToken.build({
    refresh_token: refreshToken,
    client_id: clientId,
    user_id: userId,
    expires: expires
  }).save().then(function() {
    callback(null);
  }).catch(function(error) {
    callback(error);
  });
};

/*
 * Required to support password grant type
 */
OAuthDatabase.prototype.getUser = function (username, password, callback) {
  if (username === null || password === null)
    return callback("Username or Password not found");

  this.User.find({
    where: {
      username: username
    }
  }).then(function(user) {
    if (user === null)
      return callback("Username or Password not found");

    callback(null, user.checkPassword(password) ? user : false);
  });
};

module.exports = OAuthDatabase;