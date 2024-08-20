const char INDEX_HTML[] PROGMEM = R"=====(

<html lang="de">

<head>
    <meta http-equiv="Content-Type" content="text/html; charset=windows-1252">
    <title id="title">energyCostSaverSupport</title>
    <meta name="viewport"
        content="user-scalable=no, initial-scale=1, maximum-scale=1, minimum-scale=1, width=device-width">
    <link rel="stylesheet" type="text/css" href="style.css">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css">
    <script src="jquery.min.js"></script>
</head>

<body>
    <div style="width: 100%;float:left;">
        <div class="remainBar" id="updateTime" style="width: 100%;">
        </div>
    </div>
    <div class="alert" id="alertBox">
        <button onclick="this.parentElement.style.display='none';" style="margin-right: 10px;">X</button>
        <div id="alertText">...</div>
    </div>
    <div class="popup" id="changeSettings">
        <div class="popupHeader">
            <div class="popupHeaderTitle" id="#popHeadTitle">settings <i style="font-size: x-small;float:right;"><a
                        href="/config" target=_blank>advanced config</a></i>
                <!-- <h2>settings</h2> -->
            </div>
            <div class="popupHeaderTabs">
                <div>consumer device</div>
                <div>energy costs</div>
                <div class="selected">wifi</div>
            </div>
        </div>
        <div class="popupContent" id="wifi" style="display: block;">
            <div id="wifiSearchBox"
                style="display: none; position: relative;top: 45%;z-index: 1;text-align: center;background-color: lightgray;">
                <h2 id="wifiSearch">searching for wifi networks ... </h2>
            </div>
            <div id="wifiContent">
                <div style="padding-bottom: 10px;">
                    <p>available wifi's (<b id="networkCount">0</b>) - currently connected: <b id="wifiSSID"></b>
                    </p>
                    <div id="networks">
                    </div>
                </div>
                <div>
                    connect to wifi:
                </div>
                <div>
                    <input type="text" id="wifiSSIDsend" value="please choose above or type in" required maxlength="64">
                </div>
                <div>
                    wifi password (<i class="passcheck" value="invisible">show</i>):
                </div>
                <div>
                    <input type="password" id="wifiPASSsend" value="admin12345" required maxlength="64">
                </div>
                <div style="text-align: center;">
                    <b onclick="changeWifiData()" id="btnSaveWifiSettings" class="form-button btn">save</b>
                    <b onclick="hide('#changeSettings')" id="btnSettingsClose" class="form-button btn">close</b>
                </div>
            </div>
        </div>
        <div class="popupContent" id="consumer device">
            <div>
                Name of your device (dishwasher, washing machine, ...):
            </div>
            <div>
                <input type="text" id="deviceName">
            </div>
            <hr>
            <div>
                duration / runtime of the program in hours (round up to full hours):
            </div>
            <div>
                <input type="number" id="tgtDurationInHours" min="1" max="20" placeholder="3">
            </div>
            <div>
                average energy consumption in kWh for the program:
            </div>
            <div>
                <input type="number" id="tgtDurationConsumption" min="0.0" max="50.0" step="0.1" placeholder="3.6">
            </div>
            <div>
                <input type="checkbox" id="deviceDelayModeForward"> show delay hours until program finish <br><small>(On
                    =
                    shows the time in hours until the program shopuld be finished, Off = shows the time until the
                    program should be
                    start - depends on the kind of machine)</small><br>
            </div>
            <hr>
            <div>
                maximum wait time in hours for the start of the program:
            </div>
            <div>
                <input type="number" id="maxWaitTime" min="0" max="23" placeholder="12">
            </div>
            <hr>
            <div style="text-align: center;">
                <b onclick="changeDeviceData()" id="btnSaveDeviceSettings" class="form-button btn">save</b>
                <b onclick="hide('#changeSettings')" id="btnSettingsClose" class="form-button btn">close</b>
            </div>
        </div>
        <div class="popupContent" id="energy costs">
            <div>
                energy price per kWh will be calculated<br>
                - by the EPEX spot market price<br>
                - the fix costs per kWh without additional taxes<br>
                - the fix costs per kWh with additional taxes (e.g. 16% VAT)<br>
                example:<br>
                0.10 &euro; + 0.05 &euro; + 0.02 &euro; + ((0.10 &euro; + 0.02 &euro;) * 0.16) = 0.1892 &euro; per kWh
            </div>
            <hr>
            <div>
                fix costs per kWh in &euro; without additonal tax/ fee:
            </div>
            <div>
                <input type="number" id="fixedPricePerKWh" placeholder="0.0000" step="0.0001">
            </div>
            <hr>
            <div>
                fix costs per kWh in &euro; non tax free:
            </div>
            <div>
                <input type="number" id="fixedTaxPricePerKWh" min="0" max="10" placeholder="0.00001" step="0.00001">
            </div>
            <hr>
            <div>
                tax rate in % (e.g. 16%):
            </div>
            <div>
                <input type="number" id="taxVarPricePerKWH" min="0" max="100" placeholder="0">
            </div>
            <hr>
            <div style="text-align: center;">
                <b onclick="changeEnergyCostsData()" id="btnSaveEnergySettings" class="form-button btn">save</b>
                <b onclick="hide('#changeSettings')" id="btnSettingsClose" class="form-button btn">close</b>
            </div>
        </div>
    </div>
    </div>
    <div class="popup" id="updateMenu">
        <h2>Update</h2>
        <div style="padding-bottom: 10px;">

            <div style="padding-bottom: 10px;"></div>

            <label class="switch">
                <input type="checkbox" checked onChange="changeUpdateType(this.checked)">
                <span class="slider"></span>
            </label>
            <label id="updateSwitch">manual/ auto</label>
        </div>
        <label id="updateType" style="color: gray;">direct online update - </label>
        <i id="updateState" style="color: gray;">currently no update available</i>
        <div id="autoUpdate" style="color: gray;">
            <div id="updateInfo">
                <div>
                    <div class="tableCell" style="text-align:right;">
                        <div id="firmwareVersion"></div>
                        <i>installed version</i>
                    </div>
                    <div class="tableCell">
                        <div id="builddateVersion"></div>
                        <i>release date</i>
                    </div>
                </div>
                <div>
                    <div class="tableCell" style="text-align:right;">
                        <div id="firmwareVersionServer"></div>
                        <i>available version</i>
                    </div>
                    <div class="tableCell">
                        <div id="builddateVersionServer"></div>
                        <i>release date</i>
                    </div>
                </div>
            </div>
            <hr>
            <div style="display: grid;align-items: center;justify-content: center;width:100%;">
                <div onclick="changeReleaseChannel(0)" id="relChanStable" class="updateChannel selected"
                    style="border-radius: 5px 0px 0px 5px;">stable</div>
                <div onclick="changeReleaseChannel(1)" id="relChanSnapshot" class="updateChannel"
                    style="border-radius: 0px 5px 5px 0px;position:relative;top:-1.25em;left:50%;color: gray;">snapshot
                </div>
                <i style="font-size:x-small;">switch update channels (stable/ latest snapshot)</i>
            </div>
            <hr>
            <div style="text-align: center;">
                <!-- <input id="btnUpdateStart" class="btn" type="submit" name="doUpdate" value="Update starten"> -->
                <b onclick="" id="btnUpdateStart" class="form-button btn">start update</b>
            </div>
        </div>
        <div id="updateManual" style="text-align: center; padding-top: 20px; display:none;">
            <!-- <input type='file' name='update'> -->
            <input type="file" id="fileInput" style="display: none;" accept=".bin" onchange="showFileName(this);">
            <label for="fileInput" class="form-button btn">choose file</label>
            <div id="fileNameDisplay" style="padding:20px 0 20px 0;"></div>
            <b id="manualUpdateStart" onClick="updateManualWithFile()" class="form-button btn"
                style="display: none;">update firmware</b>
        </div>
        <hr>
        <div style="text-align: center;">
            <b onclick="hide('#updateMenu')" class="form-button btn">close</b>
        </div>
    </div>
    </div>
    <div class="popup" id="updateProgress">
        <h2>Update</h2>
        <hr>
        <div style="padding-bottom: 10px;text-align: center;">
            <p id="updateStateNow">update to version <span id="newVersionProgress">0.0.0</span> in progress
            </p>
            <p id="remainingTime">remaining time: <span id="updateTimeout"></span></p>
        </div>
        <div class="updateProgressBarFrame">
            <div id="progressbar" class="ui-progressbar-value" style="width:0%;">&nbsp;</div>
        </div>
        <b>
            <p id="updateProgressPercent" style="text-align: center;"></p>
        </b>
        <hr>
    </div>
    <div id="frame">
        <div class="header">
            <b>energy Cost Saver Support<br /> <i id="deviceNameHeader">device</i></b>
        </div>
        <div class="row">
            <div class="column" id="cost_diagram" style="width: 100%;">
                <div>
                    cost diagram
                </div>
                <div class="panelValueBox" style="padding-top: 0vmin;">
                    <div class="bar-diagram">
                    </div>
                </div>
            </div>

            <div class="column" id="user_display">
                <div>
                    user display
                </div>
                <div class="panelValueBox">
                    <div id="delayMode">finish in</div>
                    <b id="tgtDelayHours" class="panelValue valueText">-- h</b>
                    <div class="panelValueBoxDetail">
                        <small>price now<br></small>
                        <b id="price_now" class="panelValueSmall valueText">0.00</b><i class="currency">&euro;</i>
                    </div>
                    <div class="panelValueBoxDetail">
                        <small>price delay<br></small>
                        <b id="price_delay" class="panelValueSmall valueText">0.00</b><i class="currency">&euro;</i>
                    </div>
                </div>
            </div>
            <div class="column" id="time">
                <div>
                    current time
                </div>
                <div class="panelValueBox">
                    <b id="gwtime" class="panelValue">00:00:00</b><br>
                    <b id="gwtime2" class="panelValueSmall">00.00.</b>
                </div>
            </div>
            <div class="column" id="connection_state">
                <div>
                    state
                </div>
                <div class="panelValueBox">
                    <div class="panelValueBoxDetail">
                        <small class="panelHead">last EPEX update</small><br>
                        <b id="last_response" class="panelValueSmall">00:00:00</b>
                    </div>
                    <div class="panelValueBoxDetail">
                        <small class="panelHead">system ntp</small><br>
                        <b id="gwNTPtime" class="panelValueSmall">00:00:00</b>
                    </div>
                    <div class="panelValueBoxDetail">
                        <small class="panelHead">system start</small><br>
                        <b id="gwStartTime" class="panelValueSmall">00:00:00</b>
                    </div>

                </div>
            </div>
        </div>
        <div class="footer">
            <div id="footer_left">
                <div class="footerButton">
                    <i class="fa fa-hourglass-start" alt="uptime"></i>
                    <b id="uptime" style="text-align: right;top: 20px; font-size: 2vmin;">00:00:00</b>
                </div>
                <div class="footerButton">
                    <i class="fa fa-wifi" alt="wifi local"></i>
                    <span id="rssitext_local" style="text-align: right;top: 20px; font-size: 2vmin;">50 %</span>
                </div>
            </div>
            <div id="footer_center">
                <br>
                <i id="firmware">version: 0.0.00</i>
                <br>
                <!-- <i id="builddate">Jan 01 2023 - 00:00:00</i> -->
            </div>
            <div id="footer_right">
                <div class="menuButton notification">
                    <i class="fa fa-cloud-download" onclick="show('#updateMenu')" alt="update" id="updateBtn"></i>
                    <span class="badge" id="updateBadge" style="display: none;"></span>
                </div>
                <div class="menuButton notification">
                    <i class="fa fa-sliders" onclick="show('#changeSettings')" alt="settings" id="settingsBtn"></i>
                    <!-- <span class="badge">0</span> -->
                </div>
            </div>
        </div>
    </div>

    <script>
        let timerRemainingProgess = 0;
        let waitTime = 60000 * 15;

        let timerInfoUpdate = 0;
        let cacheInfoData = {};
        let cacheData = {};

        $(document).ready(function () {
            console.log("document loading done");
            initValueChanges();
            // first data refresh
            getDataValues();
            getInfoValues();
            requestVersionData();

            window.setInterval(function () {
                getDataValues();
                remainingResponse();
            }, 1000);

            timerInfoUpdate = window.setInterval(function () {
                getInfoValues();
            }, 5000);

            // check every minute (62,5s) for an available update
            window.setInterval(function () {
                requestVersionData();
            }, 300000);

            // timerRemainingProgess = window.setInterval(function () {
            //     remainingResponse();
            // }, 100);
        });

        // switching in popups between tabs
        $(document).on("click", ".popupHeaderTabs>div", function (event) {
            $('.popupHeaderTabs>div').each(function () {
                $(this).removeClass("selected");
                if ($(this).html() == event.target.innerHTML) $(this).addClass("selected");
            });

            $('.popup>.popupContent').each(function () {
                $(this).css("display", "none");
                if ($(this).attr("id") == event.target.innerHTML) $(this).css("display", "block");;
            });
        })

        // grey'ing the bindings sections according to activation
        $("input[type='checkbox']").change(function () {
            if ($(this).closest('div').get(0).id != '') {
                if (this.checked) {
                    $(this).closest('div').css('color', '');
                } else {
                    $(this).closest('div').css('color', 'grey');
                }
            }
        });

        var show = function (id) {
            console.log("show " + id)
            $(id).show(200);
            if (id == '#changeSettings') {
                getWIFIdata();
                getEnergyData();
                getDeviceData();
            }
            if (id == '#updatePowerLimit') {
                getPowerLimitData();
                $('#powerLimitSetNew').focus();
            }
        }

        var hide = function (id) {
            console.log("hide " + id)
            $(id).hide(200);
        }


        var initModeStarted = false
        function checkInitToSettings(data) {
            // if not configured then start directly with settings dialogue
            //console.log("checkInitToSettings: - check");
            var startUptext = "settings --- startup config mode";
            if (data.initMode == 1 && $('#popHeadTitle').text() != startUptext && !initModeStarted) {
                console.log("checkInitToSettings: - start init mode");
                initModeStarted = true;
                show('#changeSettings');
                remainingTime = 0.1; // no countdown on top of the site
                $('#popHeadTitle').text(startUptext);
                // disable close button
                $('#btnSettingsClose').css('opacity', '0.3');
                $('#btnSettingsClose').attr('onclick', "")
            }
        }

        function remainingResponse() {

            var lastUpdate = (cacheData.lastResponse) * 1000;
            var currentTime = new Date().getTime();
            var diff = currentTime - lastUpdate;

            var remainingTime_width = 100 - (diff / waitTime) * 100;
            if (remainingTime_width > 100)
                remainingTime_width = 100;
            else if (remainingTime_width < 0)
                remainingTime_width = 0;
            $('#updateTime').width(remainingTime_width + "%");
        }

        function refreshData(data) {

            // $('#price_now').html((data.energyCostNow).toFixed(2) + " &euro;");
            checkValueUpdate('#price_now', (data.result.energyCostNow).toFixed(2));
            // $('#price_delay').html((data.energyCostSave).toFixed(2) + " &euro;");
            checkValueUpdate('#price_delay', (data.result.energyCostSave).toFixed(2));
            // $('#tgtDelayHours').html(data.tgtDelayHours + " h");
            checkValueUpdate('#tgtDelayHours', data.result.tgtDelayHours + " h");

            if (data.device.deviceDelayModeForward == 0) {
                $('#delayMode').html("start in");
            } else {
                $('#delayMode').html("finish in");
            }

            $('#gwtime').html(getTime(data.ntpStamp));
            $('#gwtime2').html(getTime(data.ntpStamp, "date"));

            $('#last_response').html(getTime(data.lastResponse));

            $('#gwNTPtime').html(getTime(data.ntpStamp));

            $('#gwStartTime').html(getTime(data.starttime, "dateShort") + "&nbsp;" + getTime(data.starttime, "timeShort"));

            $('#uptime').html(getTime(data.lastResponse));

            return true;
        }

        function refreshInfo(data) {

            var wifiGWPercent = Math.round(data.wifiConnection.rssiGW);
            // $('#rssitext_local').html(wifiGWPercent + '%');
            checkValueUpdate('#rssitext_local', wifiGWPercent + '%');

            // $('#firmware').html("fw version: " + data.firmware.version);
            checkValueUpdate('#firmware', "fw version: " + data.firmware.version);

            if (data.firmware.selectedUpdateChannel == 0) { $("#relChanStable").addClass("selected"); $("#relChanSnapshot").removeClass("selected"); }
            else { $("#relChanStable").removeClass("selected"); $("#relChanSnapshot").addClass("selected"); }

            return true;
        }

        function getWIFIdata() {
            // 
            $('#btnSaveWifiSettings').css('opacity', '1.0');
            $('#btnSaveWifiSettings').attr('onclick', "changeWifiData();")

            requestWifiScan();
            cacheInfoData.wifiConnection.wifiScanIsRunning = 1;

            let intervalId = setInterval(() => {
                console.log("Interval action");
                getInfoValues();
                displayWIFIdata();
                if (cacheInfoData.wifiConnection.wifiScanIsRunning == 0) {
                    clearInterval(intervalId);
                    $('#wifiSearchBox').hide();
                    $('#wifiContent').css('opacity', '1.0');
                    //console.log("Interval ended due to scan ends");
                }
            }, 250);

            setTimeout(() => {
                clearInterval(intervalId);
                $('#wifiSearchBox').hide();
                $('#wifiContent').css('opacity', '1.0');
                //console.log("Interval ended after 15 seconds");
            }, 15000);

            displayWIFIdata();
        }

        function displayWIFIdata() {
            // opacity until wifi scan done
            if (cacheInfoData.wifiConnection.wifiScanIsRunning == 1) {
                $('#wifiContent').css('opacity', '0.3');
                $('#wifiSearchBox').show();
            }

            wifiData = cacheInfoData.wifiConnection;
            wifiDataNw = wifiData.foundNetworks;
            // get networkdata
            //console.log("wifi scan: " + wifiData.wifiScanIsRunning);
            $('#wifiSSID').html(wifiData.wifiSsid);
            $('#wifiPASSsend').val(wifiData.wifiPassword);
            $('#networkCount').html(wifiData.networkCount);
            $('#networks').empty();
            wifiDataNw.sort(compare);
            for (let index = 0; index < wifiData.networkCount; index++) {
                var selected = "";
                if ($('#wifiSSIDsend').val() == wifiDataNw[index].name) selected = "checked";
                $('#networks').append('<label><input type="radio" id="wifi' + index + '" name="wifiselect" value="wifi' + index + '" style="width: auto; height: auto; display:inline" ' + selected + '> ' + wifiDataNw[index].wifi + ' % - ch: ' + wifiDataNw[index].chan + ' - ' + wifiDataNw[index].name + '</label><br>');
            }

            $('input[type=radio][name=wifiselect]').change(function () {
                console.log("select: " + this.value + " - " + (this.value).split("wifi")[1]);
                $('#wifiSSIDsend').val(wifiDataNw[(this.value).split("wifi")[1]].name);
            });
        }

        // ----------------------

        function drawBarDiagram(data) {
            var jsonData = data.energyCosts;
            var currentHour = data.result.currentHour;
            var currentHourBar = currentHour;
            var tgtStartHour = data.result.tgtStartHour;
            var lastValidHour = data.energyCostsLastEntry;
            var duration = data.device.tgtDurationInHours;
            var barDiagram = document.getElementsByClassName("bar-diagram")[0];

            $('#deviceNameHeader').text(data.device.deviceName);

            // before appending new data, clear the old one
            while (barDiagram.firstChild) {
                barDiagram.removeChild(barDiagram.firstChild);
            }

            // iterate over all data to find the highest value
            var maxValue = Number.MIN_VALUE;
            var minValue = Number.MAX_VALUE;
            for (var i = 0; i < jsonData.length; i++) {
                var entry = jsonData[i];
                var value = entry.value;
                if (value > maxValue) {
                    maxValue = value;
                }
                if (value < minValue) {
                    minValue = value;
                }
            }
            minValue = minValue - 0.01;


            // calculate the height percentage for each bar based on the highest value
            for (var i = 0; i < jsonData.length; i++) {
                var entry = jsonData[i];
                var value = entry.value;
                var heightPercentage = (((value) - minValue) / (maxValue - minValue)) * 100;
                // console.log("(" + i + ")value: " + value + " %: " + heightPercentage);

                // create the bar container
                var barContainer = document.createElement("div");
                barContainer.className = "bar-container";

                // create the bar value element
                var barValue = document.createElement("div");
                barValue.className = "bar-value";
                barValue.innerHTML = entry.value.toFixed(2);

                // create the bar element
                var bar = document.createElement("div");
                bar.className = "bar";
                bar.style.height = heightPercentage + "%";

                // set the color of the bar based where program should run
                if ((currentHour % 24) >= tgtStartHour && (currentHour % 24) < (tgtStartHour + duration)) {
                    bar.style.backgroundColor = "green";
                } else if (value > maxValue * 0.9) {
                    bar.style.backgroundColor = "darkred";
                }
                // if data not available
                if (i > lastValidHour) {
                    bar.style.backgroundColor = "darkgrey";
                    barValue.innerHTML = "?";
                }

                // create the bar time element
                var barTime = document.createElement("div");
                barTime.className = "bar-time";
                barTime.innerHTML = ((currentHour++) % 24) + ":00";
                barContainer.appendChild(barTime);

                // append the elements to the bar container
                barContainer.appendChild(barValue);
                barContainer.appendChild(bar);

                // append the bar container to the bar diagram
                barDiagram.appendChild(barContainer);
            }
        }

        function getEnergyData() {
            $('#btnSaveEnergySettings').css('opacity', '1.0');
            $('#btnSaveEnergySettings').attr('onclick', "changeEnergyCostsData();")

            $('#fixedPricePerKWh').val(cacheData.energyCostSettings.fixedPricePerKWh);
            $('#fixedTaxPricePerKWh').val(cacheData.energyCostSettings.fixedTaxPricePerKWh);
            $('#taxVarPricePerKWH').val(cacheData.energyCostSettings.taxVarPricePerKWH);
        }

        function getDeviceData() {
            // active
            $('#btnSaveDeviceSettings').css('opacity', '1.0');
            $('#btnSaveDeviceSettings').attr('onclick', "changeDeviceData();")

            $('#deviceName').val(cacheData.device.deviceName);
            $('#tgtDurationInHours').val(cacheData.device.tgtDurationInHours);
            $('#tgtDurationConsumption').val(cacheData.device.tgtDurationConsumption);
            $('#maxWaitTime').val(cacheData.device.maxWaitTime);
            if (cacheData.device.deviceDelayModeForward) {
                $('#deviceDelayModeForward').prop("checked", true);
            } else {
                $('#deviceDelayModeForward').prop("checked", false);
            }
        }

        $('.passcheck').click(function () {
            console.log("passcheck stat: " + $(this).attr("value") + " - id: " + $(this).attr("id"))
            if ($(this).attr("value") == 'invisible') {
                $('#wifiPASSsend').attr('type', 'text');
                $('#dtuPassword').attr('type', 'text');
                $('#mqttPassword').attr('type', 'text');
                $('.passcheck').attr('value', 'visibile');
                $('.passcheck').html("hide");
            } else {
                $('#wifiPASSsend').attr('type', 'password');
                $('#dtuPassword').attr('type', 'password');
                $('#mqttPassword').attr('type', 'password');
                $('.passcheck').attr('value', 'invisible');
                $('.passcheck').html("show");
            }
        });

        function changeWifiData() {
            var ssid = $('#wifiSSIDsend').val();
            var pwd = $('#wifiPASSsend').val();
            var data = {};
            data["wifiSSIDsend"] = ssid;
            data["wifiPASSsend"] = pwd;

            console.log("send to server: wifi: " + ssid + " - pass: " + pwd);

            const urlEncodedDataPairs = [];

            // Turn the data object into an array of URL-encoded key/value pairs.
            for (const [name, value] of Object.entries(data)) {
                urlEncodedDataPairs.push(
                    `${encodeURIComponent(name)}=${encodeURIComponent(value)}`,
                );
                console.log("push: " + name);
            }

            // Combine the pairs into a single string and replace all %-encoded spaces to
            // the '+' character; matches the behavior of browser form submissions.
            const urlEncodedData = urlEncodedDataPairs.join("&").replace(/%20/g, "+");


            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("POST", "/updateWifiSettings", false); // false for synchronous request
            xmlHttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

            // Finally, send our data.
            xmlHttp.send(urlEncodedData);

            strResult = JSON.parse(xmlHttp.responseText);
            console.log("got from server: " + strResult);
            console.log("got from server - strResult.wifiSSIDUser: " + strResult.wifiSSIDUser + " - cmp with: " + ssid);
            console.log("got from server - strResult.wifiPassUser: " + strResult.wifiPassUser + " - cmp with: " + pwd);

            if (strResult.wifiSSIDUser == ssid && strResult.wifiPassUser == pwd) {
                console.log("check saved data - OK");
                showAlert('Wifi access data changed', 'connect to the choosen wifi and to the new ip within your network', 'alert-success');
            } else {
                showAlert('Some error occured!', 'change Wifi access data could not be saved. Please try again!', 'alert-danger');
            }

            $('#btnSaveWifiSettings').css('opacity', '0.3');
            $('#btnSaveWifiSettings').attr('onclick', "")

            hide('#changeSettings');
            return;
        }

        function changeEnergyCostsData() {
            var fixedPricePerKWhSend = $('#fixedPricePerKWh').val();
            var fixedTaxPricePerKWhSend = $('#fixedTaxPricePerKWh').val();
            var taxVarPricePerKWHSend = $('#taxVarPricePerKWH').val();

            var data = {};
            data["fixedPricePerKWhSend"] = fixedPricePerKWhSend;
            data["fixedTaxPricePerKWhSend"] = fixedTaxPricePerKWhSend;
            data["taxVarPricePerKWHSend"] = taxVarPricePerKWHSend;

            console.log("send to server: fixedPricePerKWhSend: " + fixedPricePerKWhSend + " fixedTaxPricePerKWhSend: " + fixedTaxPricePerKWhSend + " taxVarPricePerKWHSend: " + taxVarPricePerKWHSend);

            const urlEncodedDataPairs = [];

            // Turn the data object into an array of URL-encoded key/value pairs.
            for (const [name, value] of Object.entries(data)) {
                urlEncodedDataPairs.push(
                    `${encodeURIComponent(name)}=${encodeURIComponent(value)}`,
                );
                console.log("push: " + name);
            }

            // Combine the pairs into a single string and replace all %-encoded spaces to
            // the '+' character; matches the behavior of browser form submissions.
            const urlEncodedData = urlEncodedDataPairs.join("&").replace(/%20/g, "+");


            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("POST", "/updateErnergyCostSettings", false); // false for synchronous request
            xmlHttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

            // Finally, send our data.
            xmlHttp.send(urlEncodedData);

            strResult = JSON.parse(xmlHttp.responseText);
            console.log("got from server: " + strResult);
            console.log("got from server - strResult.fixedPricePerKWh: " + strResult.fixedPricePerKWh + " - cmp with: " + fixedPricePerKWhSend);
            console.log("got from server - strResult.fixedTaxPricePerKWh: " + strResult.fixedTaxPricePerKWh + " - cmp with: " + fixedTaxPricePerKWhSend);
            console.log("got from server - strResult.taxVarPricePerKWH: " + strResult.taxVarPricePerKWH + " - cmp with: " + taxVarPricePerKWHSend);

            if (strResult.fixedPricePerKWh == fixedPricePerKWhSend && strResult.fixedTaxPricePerKWh == fixedTaxPricePerKWhSend && strResult.taxVarPricePerKWH == taxVarPricePerKWHSend) {
                console.log("check saved data - OK");
                showAlert('Energy Costs Data settings changed', 'The new settings will be applied.', 'alert-success');
            } else {
                showAlert('Some error occured!', 'change Energy Costs Data settings could not be saved. Please try again!', 'alert-danger');
            }
            hide('#changeSettings');
            return;
        }

        function changeDeviceData() {
            var deviceNameSend = $('#deviceName').val();
            var tgtDurationInHoursSend = $('#tgtDurationInHours').val();
            var tgtDurationConsumptionSend = $('#tgtDurationConsumption').val();
            var maxWaitTimeSend = $('#maxWaitTime').val();
            var deviceDelayModeForwardSend = 0;
            if ($("#deviceDelayModeForward").is(':checked')) {
                deviceDelayModeForwardSend = 1;
            } else {
                deviceDelayModeForwardSend = 0;
            }

            var data = {};
            data["deviceNameSend"] = deviceNameSend;
            data["tgtDurationInHoursSend"] = tgtDurationInHoursSend;
            data["tgtDurationConsumptionSend"] = tgtDurationConsumptionSend;
            data["maxWaitTimeSend"] = maxWaitTimeSend;
            data["deviceDelayModeForwardSend"] = deviceDelayModeForwardSend;

            console.log("send to server: deviceNameSend: " + deviceNameSend);
            console.log("send to server: deviceDelayModeForwardSend: " + deviceDelayModeForwardSend);

            const urlEncodedDataPairs = [];

            // Turn the data object into an array of URL-encoded key/value pairs.
            for (const [name, value] of Object.entries(data)) {
                urlEncodedDataPairs.push(
                    `${encodeURIComponent(name)}=${encodeURIComponent(value)}`,
                );
                console.log("push: " + name + " - value: " + value);
            }

            // Combine the pairs into a single string and replace all %-encoded spaces to
            // the '+' character; matches the behavior of browser form submissions.
            const urlEncodedData = urlEncodedDataPairs.join("&").replace(/%20/g, "+");


            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("POST", "/updateDeviceDataSettings", false); // false for synchronous request
            xmlHttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

            // Finally, send our data.
            xmlHttp.send(urlEncodedData);

            strResult = JSON.parse(xmlHttp.responseText);
            console.log("got from server: " + strResult);
            console.log("got from server - strResult.deviceName: " + strResult.deviceName + " - cmp with: " + deviceNameSend);
            console.log("got from server - strResult.tgtDurationInHours: " + strResult.tgtDurationInHours + " - cmp with: " + tgtDurationInHoursSend);
            console.log("got from server - strResult.maxWaitTime: " + strResult.maxWaitTime + " - cmp with: " + maxWaitTimeSend);

            if (strResult.deviceName == deviceNameSend &&
                strResult.tgtDurationInHours == tgtDurationInHoursSend &&
                strResult.tgtDurationConsumption == tgtDurationConsumptionSend &&
                strResult.maxWaitTime == maxWaitTimeSend) {
                console.log("check saved data - OK");
                showAlert('change DeviceData settings', 'Your settings were successfully saved and will be applied.', 'alert-success');
            } else {
                showAlert('Some error occured!', 'change DeviceData settings could not be saved. Please try again!', 'alert-danger');
            }

            hide('#changeSettings');
            return;
        }

        function changeReleaseChannel(channel) {
            if (cacheInfoData.firmware.selectedUpdateChannel == channel) return;

            cacheInfoData.firmware.versionServer = "reloading";
            cacheInfoData.firmware.versiondateServer = "reloading";
            cacheInfoData.firmware.selectedUpdateChannel = channel;
            cacheInfoData.firmware.updateAvailable = 0;

            getVersionData(cacheInfoData);
            refreshInfo(cacheInfoData);

            clearInterval(timerInfoUpdate);
            timerInfoUpdate = window.setInterval(function () {
                getInfoValues();
            }, 7500);

            var data = {};
            data["releaseChannel"] = channel;

            console.log("send to server: releaseChannel: " + channel);

            const urlEncodedDataPairs = [];

            // Turn the data object into an array of URL-encoded key/value pairs.
            for (const [name, value] of Object.entries(data)) {
                urlEncodedDataPairs.push(
                    `${encodeURIComponent(name)}=${encodeURIComponent(value)}`,
                );
                console.log("push: " + name);
            }

            // Combine the pairs into a single string and replace all %-encoded spaces to
            // the '+' character; matches the behavior of browser form submissions.
            const urlEncodedData = urlEncodedDataPairs.join("&").replace(/%20/g, "+");

            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("POST", "/updateOTASettings", false); // false for synchronous request
            xmlHttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

            // Finally, send our data.
            xmlHttp.send(urlEncodedData);

            try {
                strResult = '';//JSON.parse(xmlHttp.responseText);
                console.log("got from server: " + strResult);
                //console.log("got from server - strResult.dtuHostIpDomain: " + strResult.dtuHostIpDomain + " - cmp with: " + dtuHostIpDomainSend);

                //if (strResult.dtuHostIpDomain == dtuHostIpDomainSend && strResult.dtuSsid == dtuSsidSend && strResult.dtuPassword == dtuPasswordSend) {
                //    console.log("check saved data - OK");
                //    showAlert('change release channel', 'Your settings were successfully changed.','alert-success');
                //} else {
                //    showAlert('Some error occured!', 'change release channel could not be saved. Please try again!','alert-danger');
                //}
            } catch (error) {
                console.log("error at request change release channel: " + error);
            }

            hide('#changeSettings');
            return;
        }

        function compare(a, b) {
            if (a.wifi > b.wifi) {
                return -1;
            }
            if (a.wifi < b.wifi) {
                return 1;
            }
            return 0;
        }

        function getVersionData(data) {
            if (data.firmware.selectedUpdateChannel == 1) { $('#firmwareVersionServer').html(data.firmware.versionServer); $('#builddateVersionServer').html(data.firmware.versiondateServer); }
            else { $('#firmwareVersionServer').html(data.firmware.versionServerRelease); $('#builddateVersionServer').html(data.firmware.versiondateServerRelease); }

            $('#firmwareVersion').html(data.firmware.version);
            $('#builddateVersion').html(data.firmware.versiondate);

            if (data.firmware.updateAvailable == 1) {
                $('#updateState').html("new update available");
                $('#btnUpdateStart').css('opacity', '1.0');
                $('#updateBadge').show();
                $('#btnUpdateStart').attr('onclick', "startUpdate()")
            } else {
                $('#updateState').html("no update available");
                $('#btnUpdateStart').css('opacity', '0.3');
                $('#updateBadge').hide();
                $('#btnUpdateStart').attr('onclick', "")
            }
        }

        function requestVersionData() {
            $.ajax({
                url: '/updateGetInfo',
                type: 'GET',
                contentType: false,
                processData: false,
                timeout: 2500,
                success: function (data) {
                    console.log("requestVersionData - success");
                },
                error: function () {
                    console.log("requestVersionData - error");
                }
            });
        }

        function startUpdate() {
            hide('#updateMenu');
            show('#updateProgress');
            $('#newVersionProgress').html(cacheInfoData.firmware.versionServer);

            var timeoutStart = 50.0;
            var timeout = timeoutStart;
            var progress = 0;

            $('#updateProgressPercent').html("0 %");
            $('#updateTimeout').html("0 s");

            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("GET", "/updateRequest", false); // false for synchronous request
            xmlHttp.send(null);
            // //<!-- return xmlHttp.responseText; -->

            $('#btnUpdateStart').animate({ opacity: 0.3 });

            let timerTO = window.setInterval(function () {
                progress = (timeoutStart - timeout) / timeoutStart * 100;
                $('#progressbar').width(progress + "%");
                $('#updateProgressPercent').html(Math.round(progress) + " %");
                $('#updateTimeout').html(timeout.toFixed(0) + " s");

                console.log("check OTA progress - " + timeout);
                timeout = timeout - 0.25;
                if (timeout < 0 || cacheInfoData.updateAvailable == 0) {
                    clearInterval(timerTO);
                    window.location.href = "/";
                }
            }, 250);

            $('#btnUpdateStart').animate({ opacity: 1 });

            return;
        }

        function checkValueUpdate(elemId, value, unit = "") {
            if ($(elemId).html() != value + " " + unit) {
                $(elemId).html(value + " " + unit);
            }
            return true;
        }

        function changeUpdateType(checked) {
            if (checked) {
                $('#updateType').text("direct online update - ");
                $('#updateType').css('color', 'gray');
                $('#updateState').text(" currently no update available");
                $('#updateState').css('color', 'gray');
                $('#updateState').show();
                $('#autoUpdate').show();
                $('#updateManual').hide();
            } else {
                $('#updateType').text("manual update with firmware file");
                $('#updateType').css('color', '');
                $('#updateState').hide();
                $('#autoUpdate').hide();
                $('#updateManual').show();
            }
        }

        function showFileName(input) {
            var fileInput = document.getElementById('fileInput');
            var fileNameDisplay = document.getElementById('fileNameDisplay');
            if (fileInput.files.length > 0) {
                var fileName = fileInput.files[0].name;
                var fileSize = input.files[0].size / 1024 / 1024; // size in MB
                fileSize = fileSize.toFixed(2); // keeping two decimals
                // Display file size in the HTML
                fileName += ` (${fileSize} MB)`;

                $('#fileNameDisplay').html("<small>selected firmware file:</small> " + fileName);
                $('#manualUpdateStart').show();
            } else {
                fileNameDisplay.textContent = '';
            }
        }

        function updateManualWithFile() {
            var fileInput = document.getElementById('fileInput');
            if (fileInput.files.length === 0) {
                console.log("No file selected.");
                return;
            }
            var file = fileInput.files[0];
            var formData = new FormData();
            formData.append("fileInput", file);

            var xmlHttp = new XMLHttpRequest();
            xmlHttp.open("POST", "/doupdate", true); // true for asynchronous request

            //xmlHttp.onload = function () {
            //    if (xmlHttp.status === 200) {
            //        var strResult = JSON.parse(xmlHttp.responseText);
            //        console.log("got from server: ", strResult);
            //        showAlert('manual update', 'update was started', 'alert-success');
            //    } else {
            //        console.log("Error", xmlHttp.statusText);
            //    }
            //};

            // Send the FormData
            xmlHttp.send(formData);
            showAlert('manual update', 'update was started', 'alert-success');
            startManualUpdate();
        }

        function startManualUpdate() {
            hide('#updateMenu');
            show('#updateProgress');
            $('#remainingTime').hide();
            $('#updateTimeout').hide();
            $('#updateStateNow').html("installing new firmware");
            $('#newVersionProgress').html("manual update");

            var timeoutStart = 50.0;
            var timeout = timeoutStart;
            var progress = 0;
            var updateRunning = 1;

            $('#updateProgressPercent').html("0 %");
            $('#updateTimeout').html("0 s");

            $('#btnUpdateStart').animate({ opacity: 0.3 });

            let timerTO = window.setInterval(function () {
                $.ajax({
                    //url: 'api/data',
                    url: '/updateState',
                    type: 'GET',
                    contentType: false,
                    processData: false,
                    timeout: 2000,
                    success: function (response) {
                        console.log("check OTA progress - " + response.updateProgress + " - run: " + response.updateRunning);
                        progress = response.updateProgress;
                        updateRunning = response.updateRunning;
                        $('#progressbar').width(progress + "%");
                        $('#updateProgressPercent').html(Math.round(progress) + " %");
                        if (progress > 0 && updateRunning == 0) {
                            clearInterval(timerTO);
                            showAlert('manual update', 'DONE', 'alert-success');
                            location.reload();
                        }
                    },
                    error: function () {
                        showAlert('manual update', 'got no response for progress', 'alert-danger');
                    }
                });

                console.log("check OTA progress TO - " + timeout);
                timeout = timeout - 0.25;
                if (timeout < 0 || updateRunning == 0) {
                    clearInterval(timerTO);
                    showAlert('manual update', 'DONE', 'alert-success');
                    location.reload();
                }
            }, 250);

            $('#btnUpdateStart').animate({ opacity: 1 });

            return;
        }

        function initValueChanges() {
            $(".valueText").map(function () {
                observer = new MutationObserver(function (mutationsList, observer) {
                    //console.log(mutationsList);
                    const elem = mutationsList[0].target;
                    // elem.classList.add("animateValue");
                    elem.style.color = "#eee";
                    //elem.style.fontSize = "9vmin";
                    //console.log("event change in value for " + elem.id + " new innerHtml: " + elem.innerHTML);
                    setTimeout(function () {
                        elem.style.color = "";
                        //elem.style.fontSize = "6.5vmin";
                        // console.log("timeout --- event change in value for " + elem.id + " new innerHtml: " + elem.innerHTML);
                    }, 4000);

                });
                observer.observe(this, { characterData: false, childList: true, attributes: false });
            }).get();
        }

        function getTime(unix_timestamp, dateTime = "time") {
            var date = new Date(unix_timestamp * 1000);
            var day = ("0" + date.getDate()).substr(-2);
            var mon = ("0" + (date.getMonth() + 1)).substr(-2);
            var year = date.getFullYear();
            var hours = ("0" + date.getHours()).substr(-2);
            var minutes = ("0" + date.getMinutes()).substr(-2);
            var seconds = ("0" + date.getSeconds()).substr(-2);

            if (dateTime == "date") {
                return day + "." + mon + "." + year;
            } else if (dateTime == "dateShort") {
                return day + "." + mon + ".";
            } else if (dateTime == "timeShort") {
                return hours + ':' + minutes;
            } else {
                return hours + ':' + minutes + ':' + seconds;
            }
        }

        // alarmState = alert-success, alert-danger, alert-warning
        function showAlert(text, info, alarmState = "") {
            $('#alertBox').attr('class', "alert " + alarmState);
            $('#alertText').html('<b>' + text + '</b><br><small>' + info + '</small>');
            $('#alertBox').css('display', 'flex');

            setTimeout(function () {
                //$('#alertBox').css('display', 'none');
                $('#alertBox').fadeOut();
            }, 5000);
        }

        // alternative if fontAwsome is not reachable
        document.addEventListener('DOMContentLoaded', function () {
            // Check if Font Awesome styles are applied to an element
            var iconElement = document.createElement('i');
            iconElement.className = 'fa';
            document.body.appendChild(iconElement);
            // Check if the 'fa' class is applied, indicating successful loading
            if (window.getComputedStyle(iconElement).fontFamily !== 'FontAwesome') {
                handleFontAwesomeError();
            }
            // Remove the temporary element
            document.body.removeChild(iconElement);
        });

        function handleFontAwesomeError() {
            var iconElement = document.getElementById('settingsBtn');
            if (iconElement) iconElement.innerHTML = '<span style="font-size: 4vmin;">Set</span>';
            var iconElement = document.getElementById('updateBtn');
            if (iconElement) iconElement.innerHTML = '<span style="font-size: 4vmin;">Upd</span>';
        }

        function requestWifiScan() {
            $.ajax({
                url: 'getWifiNetworks',

                type: 'GET',
                contentType: false,
                processData: false,
                timeout: 2000,
                success: function (data) {
                    console.log("requestWifiScan - success: " + data.wifiNetworks);
                },
                error: function () {
                    console.log("timeout getting data in local network");
                }
            });
        }

        function getDataValues() {
            $.ajax({
                url: 'api/data.json',
                //url: 'http://192.168.1.122/api/data.json',

                type: 'GET',
                contentType: false,
                processData: false,
                timeout: 2000,
                success: function (data) {
                    cacheData = data;
                    refreshData(data);
                    drawBarDiagram(data);
                },
                error: function () {
                    console.log("timeout getting data in local network");
                }
            });
        }

        function getInfoValues() {
            $.ajax({
                url: 'api/info.json',

                type: 'GET',
                contentType: false,
                processData: false,
                timeout: 2000,
                success: function (info) {
                    cacheInfoData = info;
                    checkInitToSettings(info);
                    refreshInfo(info);
                    getVersionData(info);
                },
                error: function () {
                    console.log("timeout getting data in local network");
                }
            });
        }

    </script>

</body>

</html>

)=====";