/*jslint node: true */
"use strict";

var fs = require('fs');
var busboy = require('connect-busboy');
var Q = require('q');

var ImageUploadService = function(app, sequelize) {
  this.app = app;
  this.sequelize = sequelize;
};

ImageUploadService.prototype.initialize = function() {
  var def = Q.defer();

  console.log("Initializing Image Upload Service");
  this.app.use(busboy());
  this.app.post('/image-service/upload', this.app.oauth.authorise(), function(req, res) {
    var fstream;
    req.pipe(req.busboy);
    req.busboy.on('file', function (fieldname, file, filename) {
      // TODO: Register Image at Database
      console.log("Receiving: " + filename);
      fstream = fs.createWriteStream(__dirname + '/files/' + filename);
      file.pipe(fstream);
      fstream.on('close', function () {
        res.redirect('back');
      });
    });
  });

  console.log("Image Upload Service Initialized");
  def.resolve();
  return def.promise;
};

module.exports = ImageUploadService;