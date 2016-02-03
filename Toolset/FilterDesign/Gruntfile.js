/*jslint node: true */
"use strict";


module.exports = function(grunt) {

  grunt.initConfig({
    pkg: grunt.file.readJSON("package.json"),

    uglify: {
      dist: {
        files: {
          "dist/filterdesign.min.js": [ "dist/filterdesign.min.js" ]
        },
        options: {
          mangle: false
        }
      }
    },

    clean: {
      temp: {
        src: [ "tmp" ]
      },
      dist: {
        src: [ "dist/*.js", "dist/*.css" ]
      }
    },

    concat: {
      dist: {
        src: ["3rdparty/jquery/dist/jquery.min.js", "3rdparty/requestAnimationFrame/app/requestAnimationFrame.js", "3rdparty/material-design-lite/material.min.js", "js/*.js" ],
        dest: "dist/filterdesign.min.js"
      },
      dist_css: {
        src: ["3rdparty/material-design-lite/material.min.css", "css/*.css" ],
        dest: "dist/filterdesign.min.css"
      }
    },

    jshint: {
      all: [ "Gruntfile.js", "js/*.js" ]
    },

    connect: {
      server: {
        options: {
          hostname: "localhost",
          port: 8082
        }
      }
    },

    watch: {
      dev: {
        files: [ "Gruntfile.js", "js/*.js", "css/*.css", "3rdparty/requestAnimationFrame/app/requestAnimationFrame.js", "3rdparty/jquery/dist/jquery.min.js" ],
        tasks: [ "jshint", "concat:dist_css", "concat:dist", "clean:temp" ],
        options: {
          atBegin: true
        }
      },
      min: {
        files: [ "Gruntfile.js", "js/*.js", "css/*.css", "3rdparty/requestAnimationFrame/app/requestAnimationFrame.js", "3rdparty/jquery/dist/jquery.min.js"],
        tasks: [ "jshint", "concat:dist", "clean:temp", "uglify:dist" ],
        options: {
          atBegin: true
        }
      }
    },

    compress: {
      dist: {
        options: {
          archive: "dist/<%= pkg.name %>-<%= pkg.version %>.zip"
        },
        files: [{
          src: [ "dist/*.js", "dist/*.css" ]
        }]
      }
    },
  });

  grunt.loadNpmTasks("grunt-contrib-jshint");
  grunt.loadNpmTasks("grunt-contrib-less");
  grunt.loadNpmTasks("grunt-contrib-clean");
  grunt.loadNpmTasks("grunt-contrib-connect");
  grunt.loadNpmTasks("grunt-contrib-compress");
  grunt.loadNpmTasks("grunt-contrib-concat");
  grunt.loadNpmTasks("grunt-contrib-uglify");
  grunt.loadNpmTasks("grunt-contrib-watch");

  grunt.registerTask("dev", [ "clean:dist", "connect:server", "watch:dev" ]);
  grunt.registerTask("test", [ "clean:dist", "jshint" ]);
  grunt.registerTask("junit", [ "clean:dist", "jshint" ]  );
  grunt.registerTask("minified", [ "clean:dist", "connect:server", "watch:min" ]);
  grunt.registerTask("package", [ "clean:dist", "jshint", "concat:dist", "concat:dist_css",
    "uglify:dist", "less:dist", "clean:temp", "compress:dist" ]);
};
