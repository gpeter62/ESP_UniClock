var isTest = (location.hostname === "localhost" || location.hostname === "127.0.0.1" || location.href.indexOf("file://") != -1);

var controlInfos = {
    "version": "UniClock 3.0",
    "maxDigits": 6,
    "maxBrightness": 10,
    "currentDate": "2020.10.24",
    "currentTime": "15:01",
    "temperature": 255,
	"temperature2": 255,
    "humidity": 255,
	"humidity2": 255,
	"pressure": 255,
	"lux": 255,
    "alarmEnabled": 0,
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
	"tempCF": 'C',               //Temperature Celsius / Fahrenheit
	"enableTimeDisplay": true,       
	"dateStart": 45,                
	"dateEnd": 50,    
	"tempStart": 35,                  
	"tempEnd": 40,  
	"humidStart": 40,                 
	"humidEnd": 45,
	"dateRepeatMin": 3,            
	"doubleBlink": true,             
	"enableAutoDim": false,  
	"enableRadar": false,   
	"radarTimeout": 300,         

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
	"mqttBrokerRefresh": 30
};

//Example config that UI recieves
var configuration = {
	//Main screen	
    "version": "UniClock 3.0",
    "maxDigits": 6,
    "maxBrightness": 10,
    "currentDate": "2020.10.24",
    "currentTime": "15:01",
    "temperature": 255,
	"temperature2": 255,
    "humidity": 255,
	"humidity2": 255,
	"pressure": 255,
	"lux": 255,
    "alarmEnabled": 0,
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
	"tempCF": 'C',               //Temperature Celsius / Fahrenheit
	"enableTimeDisplay": true,       
	"dateStart": 45,                
	"dateEnd": 50,    
	"tempStart": 35,                  
	"tempEnd": 40,  
	"humidStart": 40,                 
	"humidEnd": 45,
	"dateRepeatMin": 3,            
	"doubleBlink": true,             
	"enableAutoDim": false,  
	"enableRadar": false,   
	"radarTimeout": 300,         

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
	"mqttBrokerRefresh": 30
};

//Runs, when HTML document is fully loaded
$(document).ready(function(){    
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
    $('[page]').each(function(){
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
    $('#temperature').html(configuration["temperature"]);
    $('#temperature2').html(configuration["temperature2"]);
}

function getControlInfo(index){
    return controlInfos[index];
}

//Contains the most important initializes
function Init(){

    document.title = configuration['version'];
    $('#versionHeader').html(configuration['version']);

    setLocalPagination();
    $('.menu-item').on('click',function(){
        $('.menu-item.active').removeClass('active');
        $('[page].active').removeClass('active');
        $(this).addClass('active');
        $('[page="'+$(this).attr('pagemenu')+'"]').addClass('active');
    });
    $('.menu-item').eq(0).trigger('click');

    $('#rgbFixColor').on('input',function(){
        setPreviewColor();
    });

    var sliderFadeOutTimer;
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
        sliderFadeOutTimer = setTimeout(function(){
            $(_this).addClass('fading').fadeOut('slow',function(){
                $(this).remove();
            });
        },600);
    });

    $('.form label').on('click',function(){
        var info = getControlInfo($(this).closest('.control-holder').find('input, select').eq(0).attr('id'));
        if(!!info){
            $('#popup-box h2').text($(this).text());
            $('#popup-box .content').text(info);
            $('#info-popup').fadeIn(450);
        }        
    });
    $('#popup-close-btn, #info-popup').on('click',function(){
        $('#info-popup').fadeOut(450);
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
				index == 'humidStart' || index == 'humidEnd' ||
				index == 'mqttBrokerRefresh' ||
				index == 'wifiSsid' || index == 'wifiPsw' || 
				index == 'ApSsid' || index == 'ApPsw' || 
				index == 'mqttBrokerAddr' || index == 'NtpServer' ||
				index == 'mqttBrokerUser' || index =='mqttBrokerPsw'
            ){
            $('#'+index).val(value);
        }
        else if((index == 'enableDST' || index == 'set12_24' ||
                index == 'showZero' || index == 'enableBlink' ||
                index == 'enableAutoShutoff' || index == 'alarmEnable' ||
                index == 'rgbDir' || index == 'manualOverride' ||
				index == 'enableAutoDim' || index == 'enableRadar' ||
				index == 'enableDoubleBlink' || index == 'enableTimeDisplay'
                ) && !!value
            ){
            $('#'+index).prop('checked',true);
        }
        else if(index == "maxDigits"){
            //TODO
        }
    }
	$('.lux-holder').toggleClass('hidden',configuration['lux'] == 255);
    $('.pressure-holder').toggleClass('hidden',configuration['pressure'] == 255);
	$('.humidity-holder').toggleClass('hidden',configuration['humidity'] == 255);
	$('.humidity-holder2').toggleClass('hidden',configuration['humidity2'] == 255);
    $('.temperature-holder').toggleClass('hidden',configuration['temperature'] == 255);
	$('.temperature-holder2').toggleClass('hidden',configuration['temperature2'] == 255);
    $('.rgb-holder').toggleClass('hidden',configuration['rgbEffect'] == 255);
    setPreviewColor();

    //sets a possible good timezone, if not already set
    if(!configuration['utc_offset']){
        //Intl.DateTimeFormat().resolvedOptions().timeZone <- I am in this Zone
        var tryToFindCurrentZone = -(new Date().getTimezoneOffset() / 60);
        $('#utc_offset').val(tryToFindCurrentZone);
    }

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
    },200);
}

function getCurrentInfos(){
    $.get('/getCurrentInfos/').done(function(data){
        for(var i in data){
            configuration[i] = data[i];
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