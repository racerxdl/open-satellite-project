/*jslint node: true */
"use strict";


module.exports = function(grunt) {

  grunt.initConfig({
    pkg: grunt.file.readJSON("package.json"),

    clean: {
      temp: {
        src: [ "tmp" ]
      },
      dist: {
        src: [ "dist/*.js", "dist/*.css" ]
      }
    },

    jshint: {
      all: [ "Gruntfile.js", "server.js", "database/*.js", "services/*.js", "js/*.js" ]
    },

    watch: {
      dev: {
        files: [ "Gruntfile.js", "server.js", "database/*.js", "services/*.js", "js/*.js" ],
        tasks: [ "jshint", "clean:temp" ],
        options: {
          atBegin: true
        }
      }
    },
  });

  grunt.loadNpmTasks("grunt-contrib-jshint");
  grunt.loadNpmTasks("grunt-contrib-clean");
  grunt.loadNpmTasks("grunt-contrib-watch");

  grunt.registerTask("dev", [ "clean:dist", "watch:dev" ]);
};
