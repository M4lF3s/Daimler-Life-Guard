/**
 * MeasurementsController
 *
 * @description :: Server-side logic for managing Measurements
 * @help        :: See http://sailsjs.org/#!/documentation/concepts/Controllers
 */

module.exports = {
  maxData: 1000,
  sensorData: [],

	post: function(req, res) {
    var self = this;
    req.body;

    self.sensorData = [req.body].concat(self.sensorData);
    if(self.sensorData.length > self.maxData) {
      self.sensorData.pop();
    }
    console.log(this.sensorData.length);
    res.ok();
  }
};

