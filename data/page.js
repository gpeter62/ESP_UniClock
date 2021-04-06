var isTest = (location.hostname === "localhost" || location.hostname === "127.0.0.1" || location.href.indexOf("file://") != -1);

var controlInfos = {
    "version": '<a href="https://github.com/gpeter62/ESP_UniClock" target="_blank">https://github.com/gpeter62/ESP_UniClock</a>',
    "maxDigits": "Number of tubes",
    "maxBrightness": "MAX Brightness",
    "currentDate": "Date",
    "currentTime": "Time",
    "temperature": "Temperature sensor #1",
	"temperature2": "Temperature sensor #2",
    "humidity": "Humidity sensor #1",
	"humidity2": "Humidity sensor #2",
	"pressure": "Pressure sensor #1",
	"lux": "Lux sensor current value",
    "wifiMode": "WiFi Client or Access Point",
	"alarmEnable": "Switch ON/OFF alarm",
    "alarmTime": "Alarm time (hour/minute)",
	"alarmPeriod": "Alarm maximum length (sec)",
	
	//Tube display settings
    "utc_offset": 'Timezone setting. <a href="https://en.wikipedia.org/wiki/List_of_time_zones_by_country" target="_blank"></a>https://en.wikipedia.org/wiki/List_of_time_zones_by_country',
    "enableDST": "Daylight Saving Time ON/OFF",
    "set12_24": "Clock display 12 / 24 hours",
    "showZero": "Show starting zero on hours",
    "enableBlink": "ON / OFF",
    "interval": "Cathode protect interval (minutes)",
    "enableAutoShutoff": "Enable automatic day/night switch",
    "dayTime": "Day starts (manual brightness control)",
    "nightTime": "Night starts (manual brightness control)",
    "dayBright": "Day brightness",
    "nightBright": "Night brightness",
	"manualOverride": "Manual Day/Night change",
    "animMode": "Tube animation mode: 0=OFF, 1-5: Animations 6: Random",
	"dateMode": "0:dd/mm/yyyy 1:mm/dd/yyyy 2:yyyy/mm/dd",
	"tempCF": "Celsius / Fahrenheit display",              
	"enableTimeDisplay": "Enable time&date display (Use OFF for thermometers)" ,
	"dateStart": "Date display start: Date is shown between Start/End seconds",
	"dateEnd": "Date display end",    
	"tempStart": "Temperature display time slice start",
	"tempEnd": "Temperature display end",  
	"humidStart": "Humidity display time slice start",
	"humidEnd": "Humidity display end",
	"dateRepeatMin": "Date is shown in every xx minutes. 0 = Disable date display",            
	"enableDoubleBlink": "Enable both blinking dots",             
	"enableAutoDim": "Enable auto brightness control (if sensor is installed)",  
	"enableRadar": "Enable Radar/Pir tube switch (if sensor is installed)",   
	"radarTimeout": "If Radar is used, automatic switch off after this timeout (minutes)",         

    //RGB settings	
    "rgbDir" : "RGB animation direction (left/right)",
    "rgbEffect": "RGB animation#",
    "rgbBrightness": "RGB leds brightness 0..255",
    "rgbFixColor": "RGB color, if FixColor Effect is used",
    "rgbSpeed": "RGB animation speed",
	
    //Wifi settings
	"wifiSsid": "WiFi network SSID to connect",
	"wifiPsw": "WiFI password",
	"ApSsid": "AccessPoint name, if standalone mode is used",
	"ApPsw": "AP password",
	"NtpServer": "Timeserver name, using WiFi. (Default is: pool.ntp.org)",
	"mqttBrokerAddr": "MQTT server address, if MQTT used", 
	"mqttBrokerUser": "MQTT user",
	"mqttBrokerPsw": "MQTT password",
	"mqttBrokerRefresh": "MQTT send data in every xx sec",
	"mqttEnable": "Enable/Disable MQTT refresh",
	"firmware": "http server + firmware name to download new firmware",
	"corrT0": "Temperature sensor#1 correction",
	"corrT1": "Temperature sensor#2 correction",
	"corrH0": "Humidity sensor#1 correction",
	"corrH1": "Humidity sensor#2 correction"	,
	"cathProtMin": "Cathode Protect procedure (minutes)"
};

//Example config that UI recieves
var configuration = {
	//Main screen	
    "version": "UniClock 3.0",
    "maxDigits": 6,
    "maxBrightness": 10,
    "currentDate": "2020.10.24",
    "currentTime": "15:01",
    "temperature": 22.1,
	"temperature2": 255,
    "humidity": 45,
	"humidity2": 255,
	"pressure": 255,
	"lux": 100,
	"wifiMode": true,
    "alarmEnable": false,
    "alarmTime": "6:30",
	"alarmPeriod": 15,
	
	//Tube display settings
    "utc_offset": 1,
    "enableDST": true,
    "set12_24": true,
    "showZero": true,
    "enableBlink": true,
    "interval": 15,
    "enableAutoShutoff": true,
    "dayTime": "7:00",
    "nightTime": "22:00",
    "dayBright": 10,
    "nightBright": 3,
	"manualOverride": true,
    "animMode": 6,
	"dateMode": 2,              // 0:dd/mm/yyyy 1:mm/dd/yyyy 2:yyyy/mm/dd
	"tempCF": false,               //Temperature Celsius / Fahrenheit
	"enableTimeDisplay": true,       
	"dateStart": 45,                
	"dateEnd": 50,    
	"tempStart": 35,                  
	"tempEnd": 40,  
	"humidStart": 40,                 
	"humidEnd": 45,
	"dateRepeatMin": 3,            
	"enableDoubleBlink": true,             
	"enableAutoDim": false,  
	"enableRadar": false,   
	"radarTimeout": 5,         

    //RGB settings	
    "rgbDir" : true,
    "rgbEffect": 1,
    "rgbBrightness": 100,
    "rgbFixColor": 150,
    "rgbSpeed": 50,
	
    //Wifi settings
	"wifiSsid": "mywifi",
	"wifiPsw": "mypsw",
	"ApSsid": "UniClock",
	"ApPsw": "uniclock",
	"NtpServer": "pool.ntp.org",
	"mqttBrokerAddr": "10.0.99.12", 
	"mqttBrokerUser": "mqtt",
	"mqttBrokerPsw": "mqttPW",
	"mqttBrokerRefresh": 30,
	"mqttEnable": false,
	"firmware": "http://server/fw.bin",
	"corrT0": 0.0,
	"corrT1": 0.0,
	"corrH0": 0.0,
	"corrH1": 0.0,
	"cathProtMin": 5	
};

var isMouseDown = 0;
//Runs, when HTML document is fully loaded
$(document).ready(function(){    
    $(document).on('mousedown, touchstart',function(event) {
        ++isMouseDown;
    });
    $(document).on('mouseup, touchend',function(event) {
        --isMouseDown;
    });

    if(isTest){
        Init();
        setCurrentInfos();
    }
    else{
        getConfiguration(); 
        //Set current infos, like time, humidity and temperature
        getCurrentInfos();
    }
});

function setLocalPagination(){
    $('[page]').not('[hidden]').each(function(){
        var page = $(this).attr('page');
        $('#menu tr').append('<td><div class="menu-item" pageMenu="'+page+'">'+page+'</div></td>');
    });
}

function getConfiguration(){
    $.get('/getConfiguration/').done(function(data){
        configuration = data;
    }).always(function(){
        Init();
    });
}

function sendMsgToArduino(key,value) {
    if(isTest){
        console.log(key + " " + value);
    }
    else{
        $.post('/saveSetting/', {"key" : key ,"value" : value }).done(function(data){
            console.log(data);
        });
    }
}

var COLORSATURATION = 255;
var WHITE_INDEX = 192;
function getColorFrom(WheelPos){
    if (WheelPos == WHITE_INDEX) 
        return RgbColor(COLORSATURATION/2, COLORSATURATION/2, COLORSATURATION/2);
        
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85)  {
        return RgbColor(255 - WheelPos * 3, 0, WheelPos * 3);
    } else if(WheelPos < 170) {
        WheelPos -= 85;
        return RgbColor(0, WheelPos * 3, 255 - WheelPos * 3);
    } else  {
        WheelPos -= 170;
        return RgbColor(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
}

function RgbColor(r, g, b){
    return 'rgb('+r+','+g+','+b+')';
}

function setPreviewColor(){
    var color = getColorFrom($('#rgbFixColor').val());
    $('#selectedColor').css('background-color',color);
}

function setCurrentInfos(){
    $('#currentTime').html(getCurrentDate(configuration["currentDate"]) + " " + configuration["currentTime"]);
    $('#lux').html(configuration["lux"]);
    $('#pressure').html(configuration["pressure"]);
    $('#humidity').html(configuration["humidity"]);
    $('#humidity2').html(configuration["humidity2"]);
    $('#temperature').html(getTemperature(configuration["temperature"]));
    $('#temperature2').html(getTemperature(configuration["temperature2"]));
    $('#temperature, #temperature2').toggleClass('fahrenheit',configuration["tempCF"]);
}

function round(value, decimals) {
    return parseFloat(value).toFixed(decimals);
    //return Number(Math.round(value+'e'+decimals)+'e-'+decimals);
}

function getTemperature(temperatureInC){
    if(configuration["tempCF"]){        //Fahrenheit
        return round(temperatureInC * 1.8 + 32,1);
    }
    else{
        return round(temperatureInC,1);
    }
}

function getControlInfo(index){
    return controlInfos[index];
}

//Contains the most important initializes
function Init(){

    $('#dayBright, #nightBright').attr('max',configuration['maxBrightness']);
    document.title = configuration['version'];
    $('#versionHeader').html(configuration['version']);

    setLocalPagination();
    $('[pagemenu]').on('click',function(){
        $('.menu-item.active').removeClass('active');
        $('[page].active').removeClass('active');
        $(this).addClass('active');
        $('[page="'+$(this).attr('pagemenu')+'"]').addClass('active');
        if($(this).attr('pagemenu') == 'settings'){
            getClockDetails();
        }
    });
    $('.menu-item').eq(0).trigger('click');

    $('#rgbFixColor').on('input',function(){
        setPreviewColor();
    });

    var sliderFadeOutTimer;
    function setFadeOutTimer(element){
        setTimeout(function(){
            if(isMouseDown > 0){
                clearTimeout(sliderFadeOutTimer);
                sliderFadeOutTimer = setFadeOutTimer(element);
            }
            else{
                $(element).addClass('fading').fadeOut('slow',function(){
                    $(this).remove();
                });
            }
        },600);
    }

    $('input[type="range"]').on('input',function(){
        clearTimeout(sliderFadeOutTimer);
        var id = $(this).attr('id');
        var parent = $(this).closest('.control-holder');
        $('[sliderfor]:not(.fading)').each(function(){
            if($(this).attr('sliderfor') != id && !$(this).hasClass('fading')){
                $(this).addClass('fading').fadeOut('slow',function(){
                    $(this).remove();
                });
            }
        });
        if($(parent).children('.slider-info').length == 0){
            $(parent).append('<div class="slider-info" sliderfor="'+id+'"></div>');
            $(parent).children('.slider-info').fadeIn(300);
        }
        var _this = $(parent).children('.slider-info').text($(this).val());
        sliderFadeOutTimer = setFadeOutTimer(_this);
    });

    $('.form label').on('click',function(){
        var id = '';
        if(!!$(this).attr('for')){
            id = $(this).attr('for');
        }
        else{
            id = $(this).closest('.control-holder').find('input, select, .info').eq(0).attr('id');
        }
        
        var info = getControlInfo(id);
        if(!!info){
            showPopUp($(this).text(), info, 450, true);
        }        
    });
    $('#popup-close-btn, #info-popup').on('click',function(){
        if($('#info-popup').hasClass('cantClose')){
            return;
        }
        else{
            $('#info-popup').fadeOut(450);
        }
    });
    $("#popup-box").click(function(event){
        event.stopPropagation();
    });

    //binds custom switch functionality
    $('.switcher').on('click',function(){
        $('#'+$(this).attr('for')).prop('checked',!$('#'+$(this).attr('for')).is(":checked")).trigger('change');
    });
    
    //binds custom switch text functionality
    $('.switcher-text').on('click',function(){
        $('#'+$(this).attr('for')).prop('checked',$(this).hasClass('right')).trigger('change');
    });

    //fills autogenerated select drop downs
    /*
    Possible css class:
    two-digit   - fills with two digit values if value lower than 10
    Possible attributes:
    min    (mandatory)    - auto generation's minimum value
    max    (mandatory)    - auto generation's maximum value
    step   (optional)     - auto generation's step, if not set, default is 1
    prefix (optional)     - concatanate this string before value
    suffix (optional)     - concatanate this string after value
    */
    $('.number-select').each(function(){
        let from = $(this).attr('min')*1;
        let to = $(this).attr('max')*1;
        let step = !!$(this).attr('step') ? $(this).attr('step') : 1;
        
        for(let i = from; i < to+1; i){
            let value = $(this).hasClass('two-digit') ? formatToTwoDigit(i) : i;
            let text_value = value;
            if($(this).attr('prefix') != null){
                text_value = $(this).attr('prefix') + text_value
            }
            if($(this).attr('suffix') != null){
                text_value += $(this).attr('suffix')
            }
            $(this).append('<option value="'+value+'">'+text_value+'</option>');
            i += step*1;
        }        
    });

    //fills the inputs with configuration values
    for(var index in configuration){
        let value = configuration[index];
        if(index == 'dayTime' || index == 'nightTime' || index == 'alarmTime'){
            value = value.split(':');
            $('#'+index+'Hours').val(formatToTwoDigit(value[0]));
            $('#'+index+'Minutes').val(formatToTwoDigit(value[1]));
        }
        else if(index == 'version'
		){
            $('#'+index).html(value);
        }
        else if(index == 'utc_offset' || index == 'maxBrightness' || 
                index == 'dayBright' || index == 'nightBright' || 
                index == 'animMode' || index == 'rgbBrightness' ||
                index == 'rgbFixColor' || index == 'rgbSpeed' ||
                index == 'rgbEffect' || index == 'interval' ||
				index == 'alarmPeriod' || index == 'radarTimeout' ||
				index == 'dateRepeatMin' || index == 'dateMode' ||
				index == 'dateStart' || index == 'dateEnd' ||
				index == 'timeStart' || index == 'timeEnd' ||
				index == 'tempStart' || index == 'tempEnd' ||
				index == 'humidStart' || index == 'humidEnd' ||
				index == 'mqttBrokerRefresh' ||
				index == 'wifiSsid' || index == 'wifiPsw' || 
				index == 'ApSsid' || index == 'ApPsw' || 
				index == 'mqttBrokerAddr' || index == 'NtpServer' ||
				index == 'mqttBrokerUser' || index =='mqttBrokerPsw' ||
				index == 'firmware' || index == 'corrT0' || index == 'corrT1' ||
				index == 'corrH0' || index == 'corrH1' || index == 'cathProtMin'
            ){
            $('#'+index).val(value);
        }
        else if((index == 'enableDST' || index == 'set12_24' ||
                index == 'showZero' || index == 'enableBlink' ||
                index == 'enableAutoShutoff' || index == 'alarmEnable' ||
                index == 'rgbDir' || index == 'manualOverride' ||
				index == 'enableAutoDim' || index == 'enableRadar' ||
				index == 'enableDoubleBlink' || index == 'enableTimeDisplay' ||
				index == 'mqttEnable' || index == 'tempCF' || index == "wifiMode"
                ) && !!value
            ){
            $('#'+index).prop('checked',true);
        }
        else if(index == "maxDigits"){
            //TODO
        }
    }
	$('.radar-holder').toggleClass('hidden',configuration['radarTimeout'] == 0);
	$('.mqtt-holder').toggleClass('hidden',configuration['mqttBrokerRefresh'] == 0);
	$('.lux-holder').toggleClass('hidden',configuration['lux'] == 255);
    $('.pressure-holder').toggleClass('hidden',configuration['pressure'] == 255);
	$('.humidity-holder').toggleClass('hidden',configuration['humidity'] == 255);
	$('.humidity-holder2').toggleClass('hidden',configuration['humidity2'] == 255);
    $('.temperature-holder').toggleClass('hidden',configuration['temperature'] == 255);
	$('.temperature-holder2').toggleClass('hidden',configuration['temperature2'] == 255);
    $('.rgb-holder').toggleClass('hidden',configuration['rgbEffect'] == 255);
    setPreviewColor();

    //sets a possible good timezone, if not already set
    //if(!configuration['utc_offset']){
    //    //Intl.DateTimeFormat().resolvedOptions().timeZone <- I am in this Zone
    //    var tryToFindCurrentZone = -(new Date().getTimezoneOffset() / 60);
    //    $('#utc_offset').val(tryToFindCurrentZone);
    //}

    setTimeout(function(){
        $('input, select').on('change',function(){
            var value = '';
            if($(this).attr('id').indexOf("Hours") == -1 || $(this).attr('id').indexOf("Minutes") == -1){
                if($(this).attr('type') == 'checkbox'){
                    value = $(this).is(':checked');
                }
                else{
                    value = $(this).val();
                }
                sendMsgToArduino($(this).attr('id'), value);
            }
            else{
                var key = $(this).attr('id').replace('Hours','').replace('Minutes','');
                value = $('#'+key+'Hours').val() + ":" + $('#'+key+'Minutes').val();
                sendMsgToArduino(key, value);
            }
        });
        $('#dateMode').on('change',function(){
            configuration["dateMode"] = $(this).val();
            setCurrentInfos();
        });
        $('#tempCF').on('change',function(){
            configuration["tempCF"] = !configuration["tempCF"];
            setCurrentInfos();
        });
    },200);
}

function showPopUp(header, content, timeout, canClose){
    $('#popup-box h2').text(header);
    $('#popup-box .content').html(content);
    $('#info-popup').toggleClass('cantClose',!canClose);
    $('#info-popup').fadeIn(timeout);
}

function areYouSure(fn){
    if (confirm("Are you sure to run " + fn + "?")) {
        if(fn == "reset"){
            callReset();
        }
        else if(fn == "factoryreset"){
            callFactoryReset();
        }
        else if(fn == "firmwareupdate"){
            callFirmwareUpdate();
        }
        else if(fn == "cathodeProtect"){
            callCathodeProtect();
        }		
    }
}

function callReset(){
    if(isTest){return;}
    $.post('/reset/', {}).done(function(data){
        location.reload();
    }).always(function(){
        
    });
}

function callFactoryReset(){
    if(isTest){return;}
    $.post('/factoryreset/', {}).done(function(data){
        location.reload();
    }).always(function(){
        
    });
}

function callFirmwareUpdate(){
    if(isTest){return;}
    showPopUp("Firmware is updating", "Please do not turn off the clock", 300, false);
    $.post('/firmwareupdate/', {}).done(function(data){
        if(!!data.header){
            showPopUp(data.header, data.content, 300, data.canClose);
        }
    }).always(function(){
        
    });
}

function callCathodeProtect(){
    if(isTest){return;}
    $.post('/cathodeProtect/', {}).done(function(data){
        location.reload();
    }).always(function(){
        
    });
}

function getClockDetails(){
    if(isTest){return;}
    $.get('/getClockDetails/').done(function(data){
        $('#clock-details').html(data);
    }).always(function(){
        
    });
}

function getCurrentInfos(){
    if(isTest){return;}
    $.get('/getCurrentInfos/').done(function(data){
        for(var i in data){
            configuration[i] = data[i];
        }
        if(!!data["popupMsg"] && data["popupMsg"].length > 0){
            showPopUp("Info", data["popupMsg"], 300, true);
        }
        setCurrentInfos();
        setTimeout(getCurrentInfos,20000);   //refreshes time every 20 second by calling itself
    }).always(function(){
        
    });
}

function getCurrentDate(date){
    var splittedDate = date.split('.');
    var yyyy = splittedDate[0];
    var mm = splittedDate[1]; 
    var dd = splittedDate[2];

    if(configuration["dateMode"] == 0){
        return dd+"/"+mm+"/"+yyyy;
    }
    else if(configuration["dateMode"] == 1){
        return mm+"/"+dd+"/"+yyyy;
    }
    else if(configuration["dateMode"] == 2){
        return yyyy+"/"+mm+"/"+dd;
    }
    else{
        return yyyy+"/"+mm+"/"+dd;   //if dateMode?? // 0:dd/mm/yyyy 1:mm/dd/yyyy 2:yyyy/mm/dd
    }
}

//Gets the current time from browser and gives back as readable format
function getCurrentTime(){
    var today = new Date();
    var dd = formatToTwoDigit(today.getDate());
    var mm = formatToTwoDigit(today.getMonth()+1); 
    var yyyy = today.getFullYear();
    var hour = formatToTwoDigit(today.getHours());
    var minute = formatToTwoDigit(today.getMinutes());

    if(configuration["dateMode"] == 0){
        return dd+"-"+mm+"-"+yyyy+" "+hour+":"+minute;
    }
    else if(configuration["dateMode"] == 1){
        return mm+"-"+dd+"-"+yyyy+" "+hour+":"+minute;
    }
    else if(configuration["dateMode"] == 2){
        return yyyy+"-"+mm+"-"+dd+" "+hour+":"+minute;
    }
    else{
        return yyyy+"-"+mm+"-"+dd+" "+hour+":"+minute;   //if dateMode?? // 0:dd/mm/yyyy 1:mm/dd/yyyy 2:yyyy/mm/dd
    }
}

//if number is lower than 10, adds a zero
function formatToTwoDigit(number){
    return ("0" + number).slice(-2);
}