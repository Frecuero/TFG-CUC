const express = require('express');
const connectDB = require('./DB/database');

const app = express();

// Conectar a la base de datos
connectDB();
const cors = require('cors');

app.use(cors());

// Middleware
app.use(express.json());  // Reemplazando bodyParser.json() por express.json()
app.use('/api', require('./routes/sensors')); // Asumiendo que has renombrado y movido el archivo de rutas

// Rutas (ejemplo simple)
app.get('/', (req, res) => res.send('API Running'));
app.post('/', (req, res) => res.send('API POST Running'));

const PORT = process.env.PORT || 5000;

app.listen(PORT, '0.0.0.0', () => console.log(`Server started on port ${PORT}`));

// run
// npx nodemon server.js