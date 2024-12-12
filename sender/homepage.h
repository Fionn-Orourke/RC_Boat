const char MAIN_page[] PROGMEM = R"=====( 
<!DOCTYPE html>
<html>
<style>
.card {
    max-width: 400px;
    min-height: 250px;
    background: #02b875;
    padding: 30px;
    box-sizing: border-box;
    color: #FFF;
    margin: 20px;
    box-shadow: 0px 2px 18px -4px rgba(0,0,0,0.75);
    border-radius: 8px;
    text-align: center;
}

h1 {
    font-size: 24px;
}

p {
    font-size: 18px;
    font-weight: bold;
}

</style>
<body>
<div class="card">
    <h1>Joystick Position</h1>
    <p id="ADCValue">0</p>
</div>
<div class="card">
    <h1>Servo Position</h1>
    <p id="servoPos">0</p>
</div>

<script>
setInterval(function() {
    getJoystickData();
    getServoData();
}, 1000); 

function getJoystickData() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("ADCValue").innerHTML = this.responseText;
        }
    };
    xhttp.open("GET", "readADC", true);
    xhttp.send();
}

function getServoData() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("servoPos").innerHTML = this.responseText;
        }
    };
    xhttp.open("GET", "readServo", true);
    xhttp.send();
}
</script>
</body>
</html>
)=====";