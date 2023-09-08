const express = require('express');
const router = express.Router();
const Item = require('../models/Sensor');

// @route   GET api/items
router.get('/', async (req, res) => {
    try {
        const items = await Item.find();
        res.json(items);
    } catch (err) {
        console.error(err.message);
        res.status(500).send('Server Error');
    }
});

// @route   POST api/items
router.post('/', async (req, res) => {
    const { tempertaturaAmbiente, humedadAmbiente, humedadSuelo } = req.body;

    try {
        // Crear una nueva instancia del modelo Sensor
        const newItem = new Item({
            tempertaturaAmbiente,
            humedadAmbiente,
            humedadSuelo
        });

        // Guardar el nuevo item en la base de datos
        const item = await newItem.save();
        res.json(item); // devolver el item guardado
    } catch (err) {
        console.error(err.message);
        res.status(500).send('Server Error');
    }
});


module.exports = router;
