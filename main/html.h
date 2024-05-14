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
    table,
    th,
    td {
        border: 1px solid black;
    }

    label {
        display: inline-block;
        width: 120px;
        text-align: right;
    }
</style>

<body>
    <table>
        <tr>
            <td>DATETIME</td>
        </tr>
        <tr>
            <td>
                <form id="timeForm">
                    <label>Hour:</label><input type="text" name="hour"><br>
                    <label>Minute:</label><input type="text" name="minute"><br>
                    <label>Second:</label><input type="text" name="second"><br>
                    <label>Year:</label><input type="text" name="year"><br>
                    <label>Month:</label><input type="text" name="month"><br>
                    <label>Day:</label><input type="text" name="day"><br>
                </form>
            </td>
        </tr>
        <tr>
            <td><button onclick="getClockTime()">Refresh</button></td>
        </tr>
        <tr>
            <td><button onclick="setClockTime()">Send</button></td>
        </tr>
        <tr>
            <td><button onclick="syncClockTime()">Sync to local time</button></td>
        </tr>
    </table>
    <table>
        <tr>
            <td>SETTINGS</td>
        </tr>
        <tr>
            <td>
                <form id="settingsForm">
                    <label>TEACHER:</label><input type="text" name="teacherName"><br>
                    <label>SCHOOL:</label><input type="text" name="school"><br>
                </form>
            </td>
        </tr>
        <tr>
            <td><button onclick="getSettings()">Refresh</button></td>
        </tr>
        <tr>
            <td><button onclick="setSettings()">Send</button></td>
        </tr>
    </table>
        <table>
        <tr>
            <td>TURN OFF AP</td>
        </tr>
        <tr>
            <td><button onclick="sendServerKill()">Shutoff</button></td>
        </tr>
    </table>


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

        getClockTime();

    </script>
</body>

</html>
)rawliteral";