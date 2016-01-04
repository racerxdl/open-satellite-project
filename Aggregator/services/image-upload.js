/*jslint node: true */
"use strict";

var fs = require('fs');
var busboy = require('connect-busboy');

var ImageUploadService = function(app, sequelize) {
  console.log("Initializing Image Upload Service");
  app.use(busboy());
  app.post('/image-service/upload', app.oauth.authorise(), function(req, res) {
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
};

module.exports = ImageUploadService;