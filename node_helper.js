'use strict';
const NodeHelper = require('node_helper');

const {PythonShell} = require('python-shell');
var cAppStarted = false

module.exports = NodeHelper.create({

	cApp_start: function () {
		const self = this;		

		
  	},

  	// Subclass socketNotificationReceived received.
  	socketNotificationReceived: function(notification, payload) {
		const self = this;	
		if(notification === 'ObjectDetection_SetFPS') {
			if(cAppStarted) {
                		var data = {"FPS": payload}
                		//self.obj_pyshell.send(JSON.stringify(data));

         		}
       	 	}else if(notification === 'OBJECT_DETECITON_CONFIG') {
      			this.config = payload
      			if(!cAppStarted) {
        			cAppStarted = true;
        			this.cApp_start();
      			};
    		};
  	},

	stop: function() {
		const self = this;
		//self.obj_pyshell.childProcess.kill('SIGKILL');
		//self.obj_pyshell.end(function (err) {
           	if (err){
        		//throw err;
    		};
    		console.log('finished');
		});
	}
});
