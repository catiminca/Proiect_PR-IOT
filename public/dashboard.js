const GAS_THRESHOLD = 500;
const AIR_QUALITY_THRESHOLD = 400;

document.getElementById("login-form").addEventListener("submit", async (event) => {
  event.preventDefault();

  const ssid = document.getElementById("ssid").value;
  const password = document.getElementById("password").value;

  try {
    const response = await fetch("https://localhost:8443/api/login", {
      method: "POST",
      headers: {
        "Content-Type": "application/json"
      },
      body: JSON.stringify({ ssid, password })
    });

    const data = await response.json();
    if (data.success) {
      document.getElementById("login-container").style.display = "none";
      document.getElementById("dashboard").style.display = "block";
      fetchData();
    } else {
      document.getElementById("login-error").style.display = "block";
    }
  } catch (error) {
    console.error("Eroare la autentificare:", error);
    document.getElementById("login-error").innerText = "Eroare la conectare!";
    document.getElementById("login-error").style.display = "block";
  }
});

const fetchData = async () => {
  try {
    const response = await fetch("https://localhost:8443/api/data", {
      method: "GET"
    });
    const data = await response.json();
    document.getElementById("temperature").innerText = data.temperature;
    document.getElementById("humidity").innerText = data.humidity;
    document.getElementById("photoresistor").innerText = data.photoresistor;
    document.getElementById("airQuality").innerText = data.airQuality;
    document.getElementById("gas").innerText = data.gas;

    if (data.gas > GAS_THRESHOLD) {
      showAlert("Nivelul de gaz este ridicat!");
    } else if (data.airQuality > AIR_QUALITY_THRESHOLD) {
      showAlert("Calitatea aerului este proastă!");
    } else {
      hideAlert();
    }
  } catch (error) {
    console.error("Error fetching data:", error);
    document.getElementById("alert").innerText = "Eroare la preluarea datelor!";
    document.getElementById("alert").style.display = "block";
  }
};

let blinkInterval;

const showAlert = (message) => {
  document.body.style.backgroundColor = "#ff0000"; // roșu
  const alertElement = document.getElementById("alert");
  alertElement.innerText = message;
  alertElement.style.display = "block"; // alerta
  startBlinking(); 
};

const hideAlert = () => {
  document.body.style.backgroundColor = "#121212"; // negru
  document.getElementById("alert").style.display = "none";
  stopBlinking();
};

const startBlinking = () => {
  const alertElement = document.getElementById("alert");
  let isVisible = true;
  blinkInterval = setInterval(() => {
    alertElement.style.visibility = isVisible ? "hidden" : "visible";
    isVisible = !isVisible;
  }, 500);
};

const stopBlinking = () => {
  clearInterval(blinkInterval);
  const alertElement = document.getElementById("alert");
  alertElement.style.visibility = "visible";
};

setInterval(fetchData, 5000);
