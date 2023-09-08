const mongoose = require('mongoose');

const SensorSchema  = mongoose.Schema({
    date: {type: Date, default: Date.now },
    tempertaturaAmbiente: { type: String, required: true },
    humedadAmbiente: { type: String, required: true },
    humedadSuelo: { type: String, required: true }
});

module.exports = mongoose.model('Sensor', SensorSchema , 'sensoresArduino');
