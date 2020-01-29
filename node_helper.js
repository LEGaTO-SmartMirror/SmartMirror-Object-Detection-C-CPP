'use strict';
const NodeHelper = require('node_helper');
const { spawn } = require('child_process');

const {PythonShell} = require('python-shell');
var cAppStarted = false

module.exports = NodeHelper.create({

	cApp_start: function () {
		const self = this;
		console.log('starting c object detection');
		console.log('modules/' + this.name + '/object-detection/build/object_detection');
		self.objectDet = spawn('modules/' + this.name + '/object-detection/build/object_detection',['modules/' + this.name + '/object-detection/build']);
		self.objectDet.stdout.on('data', (data) => {
			try{
				var parsed_message = JSON.parse(`${data}`)

				if (parsed_message.hasOwnProperty('DETECTED_OBJECTS')){
					//console.log("[" + self.name + "] detected object: " + parsed_message.detected.name + " center in "  + parsed_message.detected.center);
					self.sendSocketNotification('DETECTED_OBJECTS', parsed_message);
				}else if (parsed_message.hasOwnProperty('OBJECT_DET_FPS')){
					//console.log("[" + self.name + "] object detection fps: " + JSON.stringify(parsed_message));
					self.sendSocketNotification('OBJECT_DET_FPS', parsed_message.OBJECT_DET_FPS);
				}
			}
			catch(err) {
				
				//console.log(err)
			}
  			//console.log(`stdout: ${data}`);
		});	
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
		
	}
});
