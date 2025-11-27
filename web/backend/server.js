const express = require('express')
const app = express()
const axios = require('axios');
const cors = require('cors');
const port = 3000
const FIRMWARE_URL = "http://192.168.0.100:80/fan"

app.use(cors()); // everyone can access this backend(better to give this allowance only to ip of our frontend)
app.use(express.json());
app.use(express.text());
temperature = 0;
humidity = 0; 

async function sendToFirmware(command)
{
    axios.post(FIRMWARE_URL, command, {
        headers: {
            'Content-Type': 'text/plain'
        }
    })
}

app.post('/ping', (req, res) => {

    temperature = req.body.temperature;
    humidity = req.body.humidity;

    console.log("Temperature: %s\n Humidity: %s\n", temperature, humidity);
    res.send( { status: "ok" }); 
})

app.post('/backend', (req,res) => {
    sendToFirmware(req.body);
    res.send( {status: "ok"});
})

app.listen(port, () => {
    console.log(`Listening on the port ${port}`)
})    