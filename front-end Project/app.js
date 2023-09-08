
document.addEventListener('DOMContentLoaded', () => {
    const API_URL = 'http://192.168.1.53:5000/api'; // Ajusta la ruta según tu configuración

    axios.get(API_URL)
        .then(response => {
            const data = response.data;

            // Ordena los datos por fecha de forma ascendente (del más antiguo al más reciente)
            data.sort((a, b) => {
                const dateA = new Date(a.date);
                const dateB = new Date(b.date);
                return dateA - dateB;
            });

            // Procesa los datos para el gráfico
            const labels = data.map(item => {
                // Extrae la hora de la fecha
                const date = new Date(item.date);
                const hours = date.getHours();
                const minutes = date.getMinutes();
                return `${hours}:${minutes}`;
            });

            const temperatureData = data.map(item => item.tempertaturaAmbiente);
            // Después de ordenar los datos, el valor más reciente estará en el primer elemento
            const latestData = data[data.length - 1];
            const humidityPercentage = latestData.humedadAmbiente;
            const humidityFloorPercentage = latestData.humedadSuelo;
            createProgress(humidityPercentage, "Humedad Ambiente", "humidityProgressContainer");
            createProgress(humidityFloorPercentage, "Humedad del suelo", "humidityFloorProgressContainer");

            // Llama a una función para crear el gráfico con Chart.js
            createChart(labels, temperatureData);
        })
        .catch(error => {
            console.error('Error al obtener los datos:', error);
        });
});

function createChart(labels, temperatureData) {
    const ctx = document.getElementById('myChart').getContext('2d');
    const chart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: labels,
            datasets: [{
                label: 'Temperatura',
                data: temperatureData,
                borderColor: 'rgb(75, 192, 192)',
                borderWidth: 2,
                fill: true,
            }],
        },
        options: {
            scales: {
                x: {
                    title: {
                        display: true,
                        text: 'Tiempo',
                    },
                    ticks: {
                        autoSkip: true, // Permite el ajuste automático de las etiquetas
                        maxTicksLimit: 10, // Limita el número máximo de etiquetas
                        callback: function (value, index, values) {
                            // value debe ser el índice en este caso
                            if (value < labels.length) {
                                return labels[value] + " h"; // Usa las fechas del array labels
                            }
                            return '';
                        },
                    },
                },
                y: {
                    title: {
                        display: true,
                        text: 'Temperatura',
                    },
                    ticks: {
                        callback: function (value, index, values) {
                            return value + '°C';
                        },
                    },
                },
            },
        },
    });
}

function createProgress(humidityPercentage, title, containerID) {
    const container = document.getElementById(containerID);

    // Crea una instancia de la barra de progreso semicircular
    const humidityBar = new ProgressBar.SemiCircle(container, {
        strokeWidth: 8, // Ancho de la línea de la barra
        color: '#77dd77', // Color de la barra de progreso
        trailColor: '#eee', // Color de fondo de la barra
        trailWidth: 2, // Ancho de la línea de fondo
        easing: 'easeInOut', // Efecto de animación
        duration: 1400, // Duración de la animación en milisegundos
        svgStyle: {
            display: 'block',
            margin: 'auto',
        },
        text: {
            value: `${humidityPercentage}%`, // Valor de texto (porcentaje)
            className: 'progressbar-text', // Clase para aplicar estilo al texto
        },
        from: { color: '#d8f8e1' },
        to: { color: '#77dd77' },
        step: (state, bar) => {
            bar.path.setAttribute('stroke', state.color);
            const value = Math.round(bar.value() * 100);
            bar.setText(`${value}%`); // Agregar el % al valor

            // bar.text.style.color = state.color;
            bar.text.style.fontSize = '1.5rem';
            
        },
    });

    // Estilo del texto
    humidityBar.text.style.fontFamily = '"Raleway", Helvetica, sans-serif';
    humidityBar.text.style.fontSize = '2rem'; 
    humidityBar.text.style.color = '#77dd77';
    humidityBar.text.style.position = 'relative';
    humidityBar.text.style.top = '-25px'; // Ajusta la posición vertical

    // Agrega un título arriba de la gráfica
    const titleElement = document.createElement('div');
    titleElement.innerText = title;
    titleElement.style.textAlign = 'center';
    titleElement.style.marginBottom = '10px';
    titleElement.style.fontWeight = 'bold';

    // Insertar el título antes de la barra de progreso
    container.parentNode.insertBefore(titleElement, container);

    // Inicializa la barra de progreso con el valor de humedad
    humidityBar.animate(humidityPercentage / 100);  // Valor de 0.0 a 1.0
}
