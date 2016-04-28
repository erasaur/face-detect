module.exports = function (grunt) {
  grunt.initConfig({
    pkg: grunt.file.readJSON('package.json'),
    watch: {
      files: [
        'client/js/editor.js'
      ],
      tasks: ['browserify']
    },
    browserify: {
      dist: {
        files: {
          'client/js/client.bundle.js': 'client/js/client.js'
        }
      }
    }
  });

  grunt.loadNpmTasks('grunt-contrib-watch');
  grunt.loadNpmTasks('grunt-browserify');
};
