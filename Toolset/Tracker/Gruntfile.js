/*jslint node: true */
"use strict";


module.exports = function(grunt) {

  grunt.initConfig({
    pkg: grunt.file.readJSON("package.json"),

    jshint: {
      all: [ "Gruntfile.js", "tracker.js", "etc/*.js" ]
    },

    watch: {
      dev: {
        files: [ "Gruntfile.js", "tracker.js", "etc/*.js" ],
        tasks: [ "jshint" ],
        options: {
          atBegin: true
        }
      }
    },
  });

  grunt.loadNpmTasks("grunt-contrib-jshint");
  grunt.loadNpmTasks("grunt-contrib-clean");
  grunt.loadNpmTasks("grunt-contrib-watch");

  grunt.registerTask("dev", [ "watch:dev" ]);
};
