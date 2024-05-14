#include <Arduino.h>

const char indexHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="X-UA-Compatible" content="ie=edge">
    <title>Hall Pass Printer</title>
</head>

<style>

    html, body {
        background-color: #34435E;
        font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    }

    .container {
        margin: 0.5rem;
        display: grid;
        grid-template-rows: 1fr;
        row-gap: 0.5rem;
    }

    h1, .form-container {
        padding: 0.5rem;
        border-radius: 0.5rem;
        background-color: #FCF6BD;
        color: #34435E;
        margin: 1rem 0rem 0rem 0rem;
    }

    h1, h2, h3 {
        text-align: center;
    }

    h2, h3 {
        background-color: #34435E;
        color: #FCF6BD;
        padding: 0.25rem;
        border-radius: 0.5rem;
        margin: 0rem;
    }

    .caption {
        color: #34435E;
        margin: 0rem;
        font-size: small;
        font-weight: bold;
    }

    .form-container {
        border: 0.5rem #FCF6BD solid;
        display: grid;
        row-gap: 1rem;
    }

    .form-row {
        display: grid;
        grid-template-columns: 1fr 2fr;
    }

    form {
        display: grid;
        row-gap: 0.25rem;
    }

    label {
        text-align: right;
        margin-right: 1rem;
        padding: 0.25rem;
        border-radius: 0.25rem;
        color: #34435E;
        font-weight: bold;
    }

    input {
        border-radius: 0.5rem;
        color: #AA5042;
        padding-left: 0.5rem;
        font-weight: bold;
    }

    .button-container {
        display: grid;
        column-gap: 1rem;
    }

    #time-button-container {
        grid-template-columns: repeat(3, 1fr);
    }

    #settings-button-container {
        grid-template-columns: repeat(2, 1fr);
    }

    #wifi-button-container {
        grid-template-columns: 1fr;
    }

    button {
        background-color: #AA5042;
        color: #FCF6BD;
        font-weight: bold;
        border-radius: 0.5rem;
        box-shadow: none;
        min-height: 3rem;
    }

</style>

<body>
    <div class="container">

        <h1>Receipt Printer Configuration</h1>
        
        <div class="form-container">
            <h2>Date & Time</h2>

            <h3 class="date-time">
                <div id="date-display">Mon, Jan 01, 2000</div>
                <div id="time-display">12:00:00 AM</div>
            </h3>

            <form id="timeForm">

                <div class="form-row">
                    <label for="hour">Hour:</label>
                    <input type="text" name="hour">
                </div>

                <div class="form-row">
                    <label for="minute">Minute:</label>
                    <input type="text" name="minute">
                </div>

                <div class="form-row">
                    <label for="second">Second:</label>
                    <input type="text" name="second">
                </div>

                <div class="form-row">
                    <label for="year">Year:</label>
                    <input type="text" name="year">
                </div>

                <div class="form-row">
                    <label for="month">Month:</label>
                    <input type="text" name="month">
                </div>

                <div class="form-row">
                    <label for="day">Day:</label>
                    <input type="text" name="day">
                </div>
            </form>

            <div class="button-container" id="time-button-container">
                <button onclick="getClockTime()">Refresh</button>
                <button onclick="setClockTime()">Send to Printer</button>
                <button onclick="syncClockTime()">Sync with Device</button>
            </div>
        </div>

        <div class="form-container">
            <h2>Settings</h2>

            <form id="settingsForm">
                <div class="form-row">
                    <label for="teacherName">Teacher:</label>
                    <input type="text" name="teacherName">
                </div>

                <div class="form-row">
                    <label for="school">School:</label>
                    <input type="text" name="school">
                </div>
            </form>

            <div class="button-container" id="settings-button-container">
                <button onclick="getSettings()">Refresh</button>
                <button onclick="setSettings()">Send</button>
            </div>
        </div>

        <div class="form-container">
            <h2>WiFi Settings</h2>
            <div class="button-container" id="wifi-button-container">
                <button onclick="sendServerKill()">Disable</button>
                <div class="caption">Once the WiFi is disabled, you will need to power-cycle the device to re-enable it.</div>
            </div>
        </div>

    </div>

    <script>
        const dateElem = document.getElementById("date-display");
        const timeElem = document.getElementById("time-display");
        const dateOptions = {
            weekday: 'short',
            year: 'numeric',
            month: 'short',
            day: 'numeric',
        }

        // Set the date & time
        const updateTime = setInterval(function() {
            const currentDate = new Date(Date.now());
            dateElem.innerHTML = currentDate.toLocaleDateString(undefined, dateOptions);
            timeElem.innerHTML = currentDate.toLocaleTimeString();
        }, 1000);

    </script>

    <script>
        class EasyHTTP {

            // Make an HTTP GET Request  
            async get(url) {

                // Awaiting for fetch response 
                const response = await fetch(url);

                // Awaiting for response.json() 
                const resData = await response.json();

                // Returning result data 
                return resData;
            }

            // Make an HTTP PUT Request 
            async put(url, data) {

                // Awaiting fetch which contains method, 
                // headers and content-type and body 
                const response = await fetch(url, {
                    method: 'PUT',
                    headers: {
                        'Content-type': 'application/json'
                    },
                    body: JSON.stringify(data)
                });

                // Awaiting response.json() 
                const resData = await response.json();

                // Return response data  
                return resData;
            }

            async delete(url) {

                // Awaiting fetch which contains  
                // method, headers and content-type 
                const response = await fetch(url, {
                    method: 'DELETE',
                    headers: {
                        'Content-type': 'application/json'
                    }
                });

                // Awaiting for the resource to be deleted 
                const resData = 'resource deleted...';

                // Return response data  
                return resData;
            }
        }
    </script>
    <script>

        function getClockTime() {

            const http = new EasyHTTP;

            http.get(
                '/time')

                // Resolving promise for response data 
                .then(data => {
                    document.getElementById("timeForm").elements["hour"].value = data[0]["hour"];
                    document.getElementById("timeForm").elements["minute"].value = data[0]["minute"];
                    document.getElementById("timeForm").elements["second"].value = data[0]["second"];
                    document.getElementById("timeForm").elements["year"].value = data[0]["year"];
                    document.getElementById("timeForm").elements["month"].value = data[0]["month"];
                    document.getElementById("timeForm").elements["day"].value = data[0]["day"];

                })

                // Resolving promise for error 
                .catch(err => console.log(err));
        }

        function setClockTime() {
            const http = new EasyHTTP;

            const data = {
                hour: document.getElementById("timeForm").elements["hour"].value,
                minute: document.getElementById("timeForm").elements["minute"].value,
                second: document.getElementById("timeForm").elements["second"].value,
                year: document.getElementById("timeForm").elements["year"].value,
                month: document.getElementById("timeForm").elements["month"].value,
                day: document.getElementById("timeForm").elements["day"].value,

            }

            console.log(data);

            http.put(
                '/time',
                data)

                // Resolving promise for response data 
                .then(data => {
                    document.getElementById("timeForm").elements["hour"].value = data[0]["hour"];
                    document.getElementById("timeForm").elements["minute"].value = data[0]["minute"];
                    document.getElementById("timeForm").elements["second"].value = data[0]["second"];
                    document.getElementById("timeForm").elements["year"].value = data[0]["year"];
                    document.getElementById("timeForm").elements["month"].value = data[0]["month"];
                    document.getElementById("timeForm").elements["day"].value = data[0]["day"];
                })

                // Resolving promise for error 
                .catch(err => console.log(err));

        }

        function syncClockTime() {
            const http = new EasyHTTP;
            // User Data 
            const date = new Date();
            const hour = date.getHours();
            const min = date.getMinutes();
            const second = date.getSeconds();
            const day = date.getDate();
            const month = date.getMonth() + 1;
            const year = date.getFullYear();


            const data = {
                hour: hour,
                minute: min,
                second: second,
                day: day,
                month: month,
                year: year
            }

            console.log(data);

            // Update Post 
            http.put(
                '/time',
                data)

                // Resolving promise for response data 
                .then(data => {
                    document.getElementById("timeForm").elements["hour"].value = data[0]["hour"];
                    document.getElementById("timeForm").elements["minute"].value = data[0]["minute"];
                    document.getElementById("timeForm").elements["second"].value = data[0]["second"];
                    document.getElementById("timeForm").elements["year"].value = data[0]["year"];
                    document.getElementById("timeForm").elements["month"].value = data[0]["month"];
                    document.getElementById("timeForm").elements["day"].value = data[0]["day"];
                })

                // Resolving promise for error 
                .catch(err => console.log(err));
        }

        function sendServerKill() {

            const http = new EasyHTTP;

            http.get(
                '/serverkill')

                // Resolving promise for error 
                .catch(err => console.log(err));
        }

        function getSettings() {

            const http = new EasyHTTP;

            http.get(
                '/settings')

                // Resolving promise for response data 
                .then(data => {
                    document.getElementById("settingsForm").elements["teacherName"].value = data[0]["teacherName"];
                    document.getElementById("settingsForm").elements["school"].value = data[0]["school"];
                    

                })

                // Resolving promise for error 
                .catch(err => console.log(err));
        }

        function setSettings() {

            const http = new EasyHTTP;

            const data = {
                teacherName: document.getElementById("settingsForm").elements["teacherName"].value,
                school: document.getElementById("settingsForm").elements["school"].value
            }

            console.log(data);

            // Update Post 
            http.put(
                '/settings',
                data)

                // Resolving promise for response data 
                .then(data => {
                    document.getElementById("settingsForm").elements["teacherName"].value = data[0]["teacherName"];
                    document.getElementById("settingsForm").elements["school"].value = data[0]["school"];
                })

                // Resolving promise for error 
                .catch(err => console.log(err));
        }

        getSettings();
        syncClockTime();
        getClockTime();

    </script>
</body>

</html>
)rawliteral";