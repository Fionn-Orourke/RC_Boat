const char MAIN_page[] PROGMEM = R"=====( 
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Dashboard</title>
</head>
<body>
    <h1>ESP32 Data</h1>
    <div>
        <h2>Tilt Angle:</h2>
        <p id="tilt-angle">Loading...</p>
    </div>

    <div>
        <h2>Servo Position:</h2>
        <p id="servo-position">Loading...</p>
    </div>

    <script>
        // Fetch tilt data every second
        setInterval(fetchTiltData, 50);

        function fetchTiltData() {
            fetch('/readADC')  // Call the /readADC endpoint
                .then(response => response.text())
                .then(data => {
                    document.getElementById('tilt-angle').innerText = data; // Update tilt angle
                })
                .catch(error => console.error('Error fetching tilt data:', error));

            fetch('/readServo')  // Call the /readServo endpoint
                .then(response => response.text())
                .then(data => {
                    document.getElementById('servo-position').innerText = data; // Update servo position
                })
                .catch(error => console.error('Error fetching servo data:', error));
        }
    </script>
</body>
</html>
)====="