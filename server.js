const express = require("express");
const bodyParser = require("body-parser");
const cors = require("cors");
const helmet = require("helmet");
const https = require("https");
const fs = require("fs");

const options = {
    key: fs.readFileSync('key.pem'),
    cert: fs.readFileSync('cert.pem')
};

const app = express();

app.use(bodyParser.json());
app.use(cors());
app.use(helmet());
app.use(express.static("public"));

let sensorData = {
    temperature: 0,
    humidity: 0,
    photoresistor: 0,
    airQuality: 0,
    gas: 0,
};

app.use(bodyParser.json());

const wifiCredentials = {
    ssid: "Caty",
    password: "GDe5-6M2"
};

let activeSession = false;

app.post("/api/login", (req, res) => {
    console.log(req.body);
    const { ssid, password } = req.body;
    if (ssid === wifiCredentials.ssid && password === wifiCredentials.password) {
        activeSession = true;
        return res.status(200).json({ success: true, message: "Autentificare reușită!" });
    } else {
        return res.status(401).json({ success: false, message: "Credențiale incorecte!" });
    }
});

const isAuthenticated = (req, res, next) => {
    if (!activeSession) {
        return res.status(403).json({ error: "Acces interzis! Trebuie să vă autentificați." });
    }
    next();
};

app.post("/api/data", (req, res) => {
    const { temperature, humidity, photoresistor, airQuality, gas } = req.body;
    
    const isValidPayload = (data) => {
        const requiredKeys = ["temperature", "humidity", "photoresistor", "airQuality", "gas"];
        if (!requiredKeys.every(key => Object.hasOwnProperty.call(data, key))) {
            return false;
        }

        if (typeof data.temperature !== "number" || data.temperature < -50 || data.temperature > 100) {
            return false;
        }
        if (typeof data.humidity !== "number" ) {
            return false;
        }
        if (typeof data.photoresistor !== "number" || data.photoresistor < 0 || data.photoresistor > 1023) {
            return false;
        }
        if (typeof data.airQuality !== "number" || data.airQuality < 0) {
            return false; 
        }
        if (typeof data.gas !== "number" || data.gas < 0 ) {
            return false; 
        }
        return true;
    };

    if (!isValidPayload(req.body)) {
        console.log("Payload invalid:", req.body);
        return res.status(400).json({ error: "Payload invalid: structură sau valori incorecte" });
    }

    if (
        photoresistor !== undefined &&
        temperature !== undefined &&
        humidity !== undefined &&
        airQuality !== undefined &&
        gas !== undefined
    ) {
        sensorData = { temperature, humidity, photoresistor, airQuality, gas };

        console.log("Date primite de la ESP:", sensorData);
        return res.status(200).json({ message: "Datele au fost primite cu succes" });
    } else {
        console.log("error for undefined");

        return res.status(400).json({ error: "Format invalid al datelor" });

    }
});

app.get("/dashboard/", (req, res) => {
    res.sendFile("public/dashboard.html" , { root: __dirname });
});

app.get("/dashboard/dashboard.js", (req, res) => {
    res.sendFile("public/dashboard.js", { root: __dirname });
});

app.get("/api/data", isAuthenticated, (req, res) => {
    res.status(200).json(sensorData);
});

const httpsServer = https.createServer(options, app);
httpsServer.listen(8443, '0.0.0.0', () => {
    console.log("Server rulează pe https://localhost:8443");
});
