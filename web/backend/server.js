const express = require('express')
const app = express()
const port = 3000

app.use(express.json());

temperature = 0;
humidity = 0; 

app.post('/ping', (req, res) => {

    temperature = req.body.temperature;
    humidity = req.body.humidity;

    console.log("Temperature: %s\n Humidity: %s\n", temperature, humidity);
    res.send( { status: "ok" })   
})

app.listen(port, () => {
    console.log(`Listening on the port ${port}`)
})    