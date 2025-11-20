const express = require('express')
const app = express()
const port = 3000

app.get('/ping', (req, res) => {
    res.send( { status: "ok" })   
})

app.listen(port, () => {
    console.log(`Listening on the port ${port}`)
})    