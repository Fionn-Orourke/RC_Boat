const char MAIN_page[] PROGMEM = R"=====( 
<!DOCTYPE html>
<html lang="en">
<style>body {
    display: flex;
    justify-content: center;
    align-items: center;
    height: 100vh;
    margin: 0;
    background-color: #f0f0f0;
}

.container {
    display: grid;
    grid-template-columns: repeat(2, 1fr);
    grid-template-rows: repeat(2, 1fr);
    gap: 20px;
    width: 80%;
    height: 80%;
}

.box {
    background-color: white;
    border: 2px solid #333;
    border-radius: 10px;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    position: relative;
    padding: 20px;
}

.circle {
    position: relative;
    width: 150px;
    height: 150px;
    border: 5px solid #333;
    border-radius: 50%;
    display: flex;
    justify-content: center;
    align-items: center;
}

.needle {
    position: absolute;
    width: 5px;
    height: 70px;
    background-color: red;
    bottom: 50%;
    transform-origin: bottom center;
    transform: rotate(0deg);
}

.half-circle {
    position: relative;
    width: 150px;
    height: 75px;
    border: 5px solid #333;
    border-radius: 75px 75px 0 0;
    overflow: hidden;
    display: flex;
    justify-content: center;
    align-items: flex-end;
}

.tilt-needle {
    position: absolute;
    width: 5px;
    height: 60px;
    background-color: green;
    bottom: 0;
    transform-origin: bottom center;
    transform: rotate(0deg);
}



.angle-display {
    margin-top: 10px;
    font-size: 20px;
}</style>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Boat UI</title>
    
</head>
<body>
    <div class="container">
        <div class="box speedometer">
            <h2>Speedometer</h2>
            <div class="half-circle">
                <div class="needle" id="speedometer-needle"></div>
            </div>
            <div class="speed-display">0 km/h</div>
        </div>
        
        <div class="box compass">
            <h2>Compass</h2>
            <div class="circle">
                <div class="needle" id="compass-needle"></div>
            </div>
        </div>
        
        <div class="box tilt-indicator">
            <h2>Tilt Indicator</h2>
            <div class="half-circle">
                <div class="tilt-needle" id="tilt-needle"></div>
            </div>
            
            <div class="angle-display" id = "tiltdisp" >0°</div>
        </div>
        
        <div class="box tilt-indicator inverted">
            <h2>Tilt offset </h2>
            <div class="half-circle">
                <div class="tilt-needle inverted" id="inverted-tilt-needle"></div>
            </div>
            
            <div class="angle-display inverted" id = "invert" >0°</div>
        </div>
    </div>
    <script>
        let speedometerAngle = 45;
        let compassAngle = 57;
        let tiltAngle = 30;
        let invertedTiltAngle = -30; 

        // Function to rotate the speedometer needle
        //function setspeedometer() {
        //    document.getElementById('speedometer-needle').style.transform = `rotate(${speedometerAngle}deg)`;

        //}
        setInterval(fetchTiltData, 300);

        function fetchTiltData() {
            fetch('/readADC')  // Call the /readADC endpoint
                .then(response => response.text())
                .then(data => {

                  document.getElementById('inverted-tilt-needle').style.transform = `rotate(${data-90}deg)`;
                  document.getElementById('invert').innerText = data; // Update servo position
                    
                })
                .catch(error => console.error('Error fetching tilt data:', error));

            fetch('/readServo')  // Call the /readServo endpoint
                .then(response => response.text())
                .then(data => {
                  
                  document.getElementById('tilt-needle').style.transform = `rotate(${data-90}deg)`; // Update tilt angle
                  document.getElementById('tiltdisp').innerText = data; // Update servo position
                })
                .catch(error => console.error('Error fetching servo data:', error));
        }
          


        
        // Function to set the tilt needle position
        //function setTiltNeedles() {
        //    document.getElementById('tilt-needle').style.transform = `rotate(${tiltAngle}deg)`;
        //    document.getElementById('inverted-tilt-needle').style.transform = `rotate(${invertedTiltAngle}deg)`;
        //}
       // setInterval(fetchTilt, 1000);


    </script>
</body>
</html>
)=====";