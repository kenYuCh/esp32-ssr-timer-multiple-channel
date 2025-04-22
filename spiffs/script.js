function updateStatus() {
    fetch("/status")
    .then(response => response.json())
    .then(data => {
        // Wi-Fi 狀態
        const wifiStatus = document.getElementById("wifi_status");
        if (data.wifi_status === 1) {
            wifiStatus.classList.add("green");
            wifiStatus.classList.remove("red");
        } else {
            wifiStatus.classList.add("red");
            wifiStatus.classList.remove("green");
        }

        // MQTT 狀態
        const mqttStatus = document.getElementById("mqtt_status");
        if (data.mqtt_status === 1) {
            mqttStatus.classList.add("green");
            mqttStatus.classList.remove("red");
        } else {
            mqttStatus.classList.add("red");
            mqttStatus.classList.remove("green");
        }
    })
    .catch(error => {
        console.error("Error fetching status:", error);
        // alert("No network connection. Unable to fetch status.");
    });
}

// every 5 secs and update.
setInterval(updateStatus, 5000);
updateStatus(); //