const char webPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>SDM120 Energy Meter</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<script src="https://code.jquery.com/jquery-1.10.2.min.js"></script>
<link href="https://netdna.bootstrapcdn.com/bootstrap/4.0.0-beta/css/bootstrap.min.css" rel="stylesheet">
<script src="https://netdna.bootstrapcdn.com/bootstrap/4.0.0-beta/js/bootstrap.min.js"></script>

</head>
<body>
<script>

var success = false;
var tryno = 0;

function getStatus() {

    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        var myarr = JSON.parse(this.responseText);
        if (myarr.hasOwnProperty("data")) {
          document.getElementById("c_voltage").innerHTML = myarr.data.voltage;
          document.getElementById("c_current").innerHTML = myarr.data.current;
          document.getElementById("c_activepower").innerHTML = myarr.data.activepower;
          document.getElementById("c_powerfactor").innerHTML = myarr.data.powerfactor;
          document.getElementById("c_frequency").innerHTML = myarr.data.frequency;
          document.getElementById("c_totalactiveenergy").innerHTML = myarr.data.totalactiveenergy;
        } else {
          document.getElementById("c_voltage").innerHTML = "-";
          document.getElementById("c_current").innerHTML = "-";
          document.getElementById("c_activepower").innerHTML = "-";
          document.getElementById("c_powerfactor").innerHTML = "-";
          document.getElementById("c_frequency").innerHTML = "-";       
        }

        document.getElementById("c_modbusstatus").innerHTML = myarr.modbus;
        document.getElementById("c_rssi").innerHTML = myarr.status.rssi;
        document.getElementById("c_uptime").innerHTML = myarr.status.uptime;
        document.getElementById("c_ssid").innerHTML = myarr.status.ssid;
        document.getElementById("c_ip").innerHTML = myarr.status.ip;
        document.getElementById("c_clientid").innerHTML = myarr.status.clientid;
        document.getElementById("c_version").innerHTML = myarr.status.version;
      }
    };
    xhttp.open("GET", "status", true);
    xhttp.send();
  
}
window.setInterval(getStatus, 1000);

function SendRequest(url) {
  success = false;
  tryno = 0;
  while ((!success)&&(tryno<3)) {
    var done = false;
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4) { 
        if (this.status == 200) {
          document.getElementById("reply").innerHTML = this.responseText;
          success = true;
        }
        tryno++;
        done = true;
      }
    };
    xhttp.open("GET", url, true);
    xhttp.send();
    while (!done);
  }
}

</script>

<link href="https://maxcdn.bootstrapcdn.com/font-awesome/4.3.0/css/font-awesome.min.css" rel="stylesheet">
<div class="container">
    <div class="row">
        <div class="col-md-4 col-xl-3">
            <div class="card bg-c-blue order-card">
                <div class="card-block">
                    <h6 class="m-b-20">Voltage</h6>
                    <h2 class="text-right"><i class="fa fa-bolt f-left"></i><span id="c_voltage">0</span> V</h2>
                </div>
            </div>
        </div>
        
        <div class="col-md-4 col-xl-3">
            <div class="card bg-c-blue order-card">
                <div class="card-block">
                    <h6 class="m-b-20">Current</h6>
                    <h2 class="text-right"><i class="fa fa-bolt f-left"></i><span id="c_current">0</span> A</h2>
                </div>
            </div>
        </div>
        
        <div class="col-md-4 col-xl-3">
            <div class="card bg-c-green order-card">
                <div class="card-block">
                    <h6 class="m-b-20">Active Power</h6>
                    <h2 class="text-right"><i class="fa fa-plug f-left"></i><span id="c_activepower">0</span> W</h2>
                </div>
            </div>
        </div>
        
        <div class="col-md-4 col-xl-3">
            <div class="card bg-c-yellow order-card">
                <div class="card-block">
                    <h6 class="m-b-20">Power Factor</h6>
                    <h2 class="text-right"><i class="fa fa-diamond f-left"></i><span id="c_powerfactor">0</span></h2>
                </div>
            </div>
        </div>

        <div class="col-md-4 col-xl-3">
            <div class="card bg-c-yellow order-card">
                <div class="card-block">
                    <h6 class="m-b-20">Frequency</h6>
                    <h2 class="text-right"><i class="fa fa-diamond f-left"></i><span id="c_frequency">0</span> Hz</h2>
                </div>
            </div>
        </div>

        <div class="col-md-4 col-xl-3">
            <div class="card bg-c-green order-card">
                <div class="card-block">
                    <h6 class="m-b-20">Total Active Energy</h6>
                    <h2 class="text-right"><i class="fa fa-plug f-left"></i><span id="c_totalactiveenergy">0</span> kWh</h2>
                </div>
            </div>
        </div>

        <div class="col-md-4 col-xl-3">
            <div class="card bg-c-pink order-card">
                <div class="card-block">
                    <h6 class="m-b-20">Modbus Status</h6>
                    <h2 class="text-right"><i class="fa fa-message f-left"></i><span id="c_modbusstatus">-</span></h2>
                </div>
            </div>
        </div>
       
        
  </div>
      <div class="row">
        <p class="m-b-0">RSSI: <span id="c_rssi">0</span>, Uptime: <span id="c_uptime">0</span> min(s), SSID: <span id="c_ssid"></span>, IP: <span id="c_ip"></span>, clientid: <span id="c_clientid"></span>, Version: <span id="c_version"></span></p>
        <p class="m-b-0">SDM120 Modbus Energy Meter to MQTT Gateway | <a href="mailto:csongor.varga@gmail.com">Csongor Varga</a> | <a href="">GitHub</a></p>
      </div>
</div>

<style type="text/css">
body{
    margin-top:20px;
    background:#FAFAFA;
}
.order-card {
    color: #fff;
}

.bg-c-blue {
    background: linear-gradient(45deg,#4099ff,#73b4ff);
}

.bg-c-green {
    background: linear-gradient(45deg,#2ed8b6,#59e0c5);
}

.bg-c-yellow {
    background: linear-gradient(45deg,#FFB64D,#ffcb80);
}

.bg-c-pink {
    background: linear-gradient(45deg,#FF5370,#ff869a);
}


.card {
    border-radius: 5px;
    -webkit-box-shadow: 0 1px 2.94px 0.06px rgba(4,26,55,0.16);
    box-shadow: 0 1px 2.94px 0.06px rgba(4,26,55,0.16);
    border: none;
    margin-bottom: 30px;
    -webkit-transition: all 0.3s ease-in-out;
    transition: all 0.3s ease-in-out;
}

.card .card-block {
    padding: 25px;
}

.order-card i {
    font-size: 26px;
}

.f-left {
    float: left;
}

.f-right {
    float: right;
}
</style>

</body>
</html>
)=====";
