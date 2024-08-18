const char STYLE_CSS[] PROGMEM = R"=====(

:root {
    --primary-color: #f2b63f;
    --background-color: #000b5e;
    --background-in-color: #00233a;
    --background-popup: #474444;
    --background-inputs: lightgrey;
    --frame-color: #ffe95c;
    --header-background-color: #1c1c1c;
    --font-size-small: 2vmin;
    --font-size-large: 5vmin;
}

* {
    box-sizing: border-box;
}

body {
    background: var(--background-color);
    font-family: sans-serif;
    font-size: 2vmin;
    color: var(--primary-color);
}

hr {
    border-top: 1px solid var(--primary-color);
}

/* reload bar at the top */
.remainBar {
    display: flex;
    align-items: center;
    justify-content: center;
    transition: width 0.25s;
    height: 0.2%;
    background-color: var(--primary-color);
    border-radius: 2px;
}


/* Style the header */
.header {
    height: 12%;
    margin-bottom: 1%;

    display: flex;
    justify-content: center;
    align-items: center;
    text-align: center;
    background-color: var(--header-background-color);
    /* font-size: 4.5vmin; */
    font-size: 1.8vmax;

    border-radius: 10px;
    border-width: 1px;
    border-color: var(--primary-color);
    border-style: solid;
}

.row {
    height: 73%;

    /* border-color: #f3213d;
    border-radius: 5px;
    border-width: 1px;
    border-style: solid; */
    padding-right: 0.3%;
}

/* Clear floats after the columns */
.row:after {
    content: "";
    display: table;
    clear: both;
}

/* Create three equal columns that floats next to each other */
.column {
    float: left;
    width: 33.033%;
    height: 49.6%;
    padding: 1%;

    margin-bottom: 0.3%;
    margin-left: 0.3%;

    border-radius: 10px;
    border-width: 1px;
    border-color: var(--frame-color);
    border-style: solid;

    font-size: 3vmin;
}

.panelValueBox {
    padding-top: 4vmin;
}

.panelValue {
    float: left;
    font-size: 7.5vmin;
    width: 100%;
    padding-bottom: 5%;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
}

.panelValueSmall {
    font-size: 4vmin;
}

.panelValueButton {
    font-size: 3vmin;
    border-width: 1px;
    border-color: var(--frame-color);
    border-style: solid;
    border-radius: 5px;
    padding-left: 2%;
    margin-right: 2%;
}

.panelValueButton:hover {
    border-color: white;
    border-width: 2px;
    cursor: pointer;
}

.panelValueBoxDetail {
    float: left;
    width: 50%;
    color: var(--primary-color);
    font-size: 3vmin;
    /* border-width: 1px;
    border-color: #21f333;
    border-style: solid; */
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
}

.panelHead {
    padding-bottom: 2px;
    font-size: x-small;
}

/* animated change of color if value chnaged */
.valueText {
    transition: color 0.75s;
}


/* Style the footer */
.footer {
    height: 12%;
    margin-top: 1%;

    display: flex;
    justify-content: center;
    align-items: center;

    font-size: 2em;

    background-color: var(--header-background-color);
    text-align: center;
    border-radius: 10px;

    border-width: 1px;
    border-color: var(--primary-color);
    border-style: solid;
}

.footerButton {
    float: left;
    padding-left: 20px;
}

.menuButton {
    padding-right: 20px;
    float: right;
    cursor: pointer;
}

.notification {
    text-decoration: none;
    position: relative;
    display: inline-block;
}

.notification:hover {
    color: white;
}

.badge {
    font-size: x-small;
}

.notification .badge {

    position: absolute;
    top: 10%;
    right: 15%;
    /* padding: 5px 8px; */
    border-radius: 100%;
    background: red;
    color: white;
    height: 2vh;
    width: 2vh;
}

.alert {
    display: none;
    /* display: flex; */
    position: absolute;
    width: 42%;
    left: 57%;
    height: 5%;
    top: 2%;
    align-items: center;

    padding: .75rem 1.25rem;
    margin-bottom: 1rem;
    border: 1px solid transparent;
    border-radius: .25rem;

    color: #004085;
    background-color: #cce5ff;
    border-color: #b8daff;

    z-index: 21;
    font-size: large;
}

.alert-success {
    color: #155724;
    background-color: #d4edda;
    border-color: #c3e6cb;
}

.alert-danger {
    color: #721c24;
    background-color: #f8d7da;
    border-color: #f5c6cb;
}

.alert-warning {
    color: #856404;
    background-color: #fff3cd;
    border-color: #ffeeba;
}

.popup {
    display: none;
    position: absolute;
    padding: 25px;
    width: 80%;
    left: 10%;
    height: 80%;
    top: 10%;
    background: var(--background-popup);
    border-width: 1px;
    border-color: var(--primary-color);
    border-style: solid;
    border-radius: 10px;
    z-index: 20;
    font-size: 2.4vmin;
}

/* .popup>.popupHeader>.selected { */
/* .popup>.popupHeader { */
.popupHeader {
    position: absolute;
    top: 5%;
    left: 25px;
    right: 25px;
    bottom: 80%;

    /* border-bottom: 1px solid; */
    /* border: 1px solid #e73701; */
    /* background-color: var(--primary-color); */
    color: var(--primary-color);
    z-index: 21;
}

.popupHeader>.popupHeaderTitle {
    position: relative;
    height: 50%;

    font-weight: bold;
    font-size: 1.5em;

    /* border: 1px solid #790079; */
    /* border-radius: 0px 0px 0px 0px; */
}

.popupHeader>.popupHeaderTabs {
    position: relative;
    height: 50%;

    /* border: 1px solid #e3f300; */
}

.popupHeader>.popupHeaderTabs>div {
    position: relative;
    height: 100%;
    padding: 1%;

    float: right;
    width: 33.33%;

    border: 1px solid var(--primary-color);
    border-radius: 5px 5px 0px 0px;

    display: grid;
    align-items: center;
    justify-content: center;

    cursor: pointer;

    font-size: 3.0vmin;
}

.selected {
    background-color: var(--primary-color);
    color: #FFF;
}

.popupHeader>.popupHeaderTabs>.selected {
    border-left: 1px solid var(--frame-color);
    border-top: 1px solid var(--frame-color);
    border-right: 1px solid var(--frame-color);
    border-bottom: 0px;
}

.popup>.popupContent {
    /* at start not visible, controlled by jQuery */
    display: none;
    position: absolute;
    top: 20%;
    left: 25px;
    right: 25px;

    bottom: 1%;
    overflow-y: scroll;

    padding: 1%;

    border-left: 1px solid;
    border-right: 1px solid;
    border-bottom: 1px solid;
    border-radius: 0px 0px 5px 5px;
}

#popup2:after {
    position: fixed;
    content: "";
    top: 0;
    left: 0;
    bottom: 0;
    right: 0;
    background: rgba(0, 0, 0, 0.5);
    z-index: -2;
}

#popup2:before {
    position: absolute;
    content: "";
    top: 0;
    left: 0;
    bottom: 0;
    right: 0;
    background: #FFF;
    border-width: 1px;
    border-color: var(--frame-color);
    border-style: solid;
    border-radius: 10px;
    z-index: -1;
}

.tableCell {
    float: left;
    width: 50%;
    padding: 1vmin;
}

.tablecell>div {
    border-bottom: 1px solid;
    font-weight: bold;
}

.tablecell>i {
    font-size: 1.3vmin;
}

#networks {
    max-height: 20%;
    overflow-x: hidden;
    overflow-y: auto;
    background-color: var(--background-inputs);
    color: black;
    padding: 5px;
    border-radius: 4px;
    scrollbar-width: thin;
}

.form-button,
input {
    width: 100%;
    height: 44px;
    border-radius: 4px;
    margin: 10px auto;
    font-size: 15px;
    display: block;
    font-weight: bold;
}

input {
    background: lightgrey;
    border: 0;
    padding: 0 15px
}

input[type=checkbox] {
    width: 1.5em;
    height: 1.5em;
    display: inline;
    position: relative;
    top: 0.3em;
}

input[type=file] {
    width: 1.5em;
    height: 1.5em;
    display: inline;
    position: relative;
    top: 0.3em;
}

#frame {
    background: var(--background-in-color);
    /* max-width: 100%; */
    /* margin: 5px; */
    padding: 20px;
    border-radius: 10px;
    height: 100%;
    text-align: center;
}

#file-input {
    padding: 0;
    border: 1px solid var(--frame-color);
    line-height: 44px;
    text-align: left;
    display: block;
    cursor: pointer;
}

#remainBar,
#prgbar {
    background-color: var(--frame-color);
    border-radius: 10px
}

#remainBar {
    background-color: var(--background-color);
    width: 0%;
    height: 10px
}

.btn {
    background: var(--primary-color);
    color: #fff;
    padding: 5px;
    cursor: pointer;
    padding-top: 0.8em;
}

table {
    font-size: 14px;
    width: 100%;
}

.col2 {
    text-align: right;
}

td {
    /* border-bottom-style: groove;
    border-bottom-width: 1px; */
    text-align: center;
}

.switch {
    top: -13px;
    position: relative;
    display: inline-block;
    width: 60px;
    /* Adjusted width */
    height: 34px;
    /* Adjusted height */
}

.switch input {
    opacity: 0;
    width: 0;
    height: 0;
}

.slider {
    position: absolute;
    cursor: pointer;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    background-color: #ccc;
    -webkit-transition: .4s;
    transition: .4s;
    border-radius: 34px;
    /* Adjusted for roundness */
}

.slider:before {
    position: absolute;
    content: '';
    height: 26px;
    /* Adjusted size */
    width: 26px;
    /* Adjusted size */
    left: 4px;
    bottom: 4px;
    background-color: white;
    -webkit-transition: .4s;
    transition: .4s;
    border-radius: 50%;
}

input:checked+.slider {
    background-color: var(--primary-color);
}

input:checked+.slider:before {
    -webkit-transform: translateX(26px);
    transform: translateX(26px);
}

#debug,
#tempOut,
#apiOut {
    background-color: lightgray;
    font-family: monospace;
    font-size: small;
    border-radius: 4px;
    text-align: left;
    padding: 4px;
}

#firmware,
#builddate {
    color: var(--primary-color);
    font-size: 2vmin;
    font-style: italic;
}

#footer_left {
    float: left;
    width: 40%;
}

#footer_center {
    float: left;
    width: 20%;
}

#footer_right {
    float: left;
    width: 40%;
}

#delayMode {
    font-size: x-small;
    margin-top: -2vmin;
}

/* @media (max-width: 820px) { */
/* @media (max-width: 739px) or (max-aspect-ratio: 10/9) { */
/* @media (max-aspect-ratio: 10/8) { */


@media (orientation: portrait) {
    .header {
        height: 10%;
        font-size: 4.5vmin;
    }

    .row {
        height: 82%;
    }

    .footer {
        height: 8%;
        font-size: 3.2em;
    }

    .alert {
        left: 2%;
        width: 96%;
        height: 7%;
        font-size: small;
    }

    .column {
        width: 100%;
        height: 32.5%;
        /* (100 / 3) - (6 * 0,3) = 32,133 */
        font-size: 1.5em;
        padding-top: 0.5%;
    }

    .panelValueBoxDetail {
        width: 50%;
        float: left;
        font-size: 3.0vmin;
        border-bottom: var(--primary-color);
        border-width: 1px;
        padding-bottom: 5%;
    }

    .panelHead {
        /* float: left; */
        width: 100%;
        padding-bottom: 2px;
    }

    #time {
        display: none;
    }

    #user_display,
    #connection_state {
        width: 99.3%;
        height: 33%;
    }

    .panelValue {
        font-size: 4.5em;
    }

    .panelValueSmall {
        font-size: 3.0em;
    }

    #connection_state .panelValueSmall {
        font-size: 2.5em;
    }

    #firmware,
    #builddate,
    #screensize {
        font-size: 2.5vmin;
        font-style: italic;
    }

    #footer_left {
        /* visibility: hidden; */
        /* display: none; */
        width: 50%;
        float: left;
    }

    #footer_center {
        display: none;
        padding-top: 2.5%;
        width: 48%;
    }

    #footer_right {
        width: 50%;
    }

    .notification .badge {
        height: 2vh;
        width: 2vh;
    }

    td {
        float: left;
    }

    .bar-value {
        /* text will be displayed rotated with 90 degrees */
        transform: rotate(-45deg);
        /* text will be shifted to the right  */
        margin-left: 100%;
    }
}



@media (orientation: portrait) and (min-width: 540px) {
    .panelValueBox {
        padding-top: 1vmin;
    }

    .panelValue {
        padding-bottom: 0px;
    }

    .panelValueBoxDetail {
        margin-top: 0%;
    }

    #delayMode {
        margin-top: 0%;
    }
}

/* update progress */
.updateProgressBarFrame {
    border-color: var(--frame-color);
    border-style: solid;
    border-radius: 5px;
    border-width: 1px;
}



.ui-progressbar-value {
    transition: width 0.25s;
    -webkit-transition: width 0.5s;
    background: var(--frame-color);
}

.updateChannel {
    border: 1px solid var(--frame-color);
    text-align: center;
    cursor: pointer;
    width: 50%;
}

.passcheck {
    cursor: pointer;
}

#wifiSearch::after {
    content: '';
    display: inline-block;
    width: 20px;
    height: 20px;
    border: 2px solid var(--frame-color);
    border-top: 2px solid transparent;
    border-radius: 50%;
    margin-left: 5px;
    animation: spin 1s linear infinite;
}

@keyframes spin {
    0% {
        transform: rotate(0deg);
    }

    100% {
        transform: rotate(360deg);
    }
}


.bar-diagram {
    display: flex;
    justify-content: space-between;
    /* align-items: flex-end; */
    /* align-items: flex-start; */
    height: 80%;
    padding: 10px;
    /* border: 1px solid #ccc; */
    box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
    position: relative;
}

.bar-container {
    display: flex;
    flex-direction: column;
    align-items: center;
    width: 2.5%;
    margin: 0 0.5%;
}

.bar {
    width: 100%;
    background-color: var(--primary-color);
    transition: height 0.3s ease;
    border-radius: 8px;
}

.bar-value {
    margin-bottom: 5px;
    font-size: 0.6em;
    color: var(--primary-color);
}

.bar-time {
    margin-top: 5px;
    font-size: 0.7em;
    color: var(--primary-color);
    position: absolute;
    bottom: -8%;
}

.bar-container:nth-child(1) .bar-time,
.bar-container:nth-child(3) .bar-time,
.bar-container:nth-child(5) .bar-time,
.bar-container:nth-child(7) .bar-time,
.bar-container:nth-child(9) .bar-time,
.bar-container:nth-child(11) .bar-time,
.bar-container:nth-child(13) .bar-time,
.bar-container:nth-child(15) .bar-time,
.bar-container:nth-child(17) .bar-time,
.bar-container:nth-child(19) .bar-time,
.bar-container:nth-child(21) .bar-time,
.bar-container:nth-child(23) .bar-time,
.bar-container:nth-child(25) .bar-time {
    display: block;
}

.bar-container .bar-time {
    display: none;
}

)=====";