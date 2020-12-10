//Example config that UI recieves
var configuration = {
    "version": "UniClock 2.0",
    "maxDigits": 4,
    "maxBrightness": 10,
    "currentDate": "2020.10.24",
    "currentTime": "15:01",
    "temperature": 0,
    "humidity": -1,
    "utc_offset": 1,
    "enableDST": true,
    "set12_24": true,
    "showZero": true,
    "blinkEnabled": true,
    "interval": 15,
    "enableAutoShutoff": true,
    "dayTime": "7:00",
    "nightTime": "22:00",
    "dayBright": 10,
    "nightBright": 3,
	"manualOverride": true,
    "animMode": 6,
    "alarmEnabled": 0,
    "alarmTime": "6:30",
	"alarmPeriod": 15,
    "rgbEffect": 1,
    "rgbBrightness": 100,
    "rgbFixColor": 150,
    "rgbSpeed": 50
};

//Runs, when HTML document is fully loaded
$(document).ready(function(){
    getConfiguration(); 

    //Set current infos, like time, humidity and temperature
    setCurrentInfos();
});

function getConfiguration(){
    $.get('/getConfiguration/').done(function(data){
        configuration = data;
    }).always(function(){
        Init();
    });
}

function sendMsgToArduino(key,value) {
    $.post('/saveSetting/', {"key" : key ,"value" : value }).done(function(data){
        console.log(data);
    });
}

//Contains the most important initializes
function Init(){

    document.title = configuration['version'];
    $('#versionHeader').html(configuration['version']);

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
        let step = !!$(this).attr('step') ? !!$(this).attr('step') : 1;
        
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
            i += step;
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
        else if(index == 'version'){
            $('#'+index).html(value);
        }
        else if(index == 'utc_offset' || index == 'maxBrightness' || 
                index == 'dayBright' || index == 'nightBright' || 
                index == 'animMode' || index == 'rgbBrightness' ||
                index == 'rgbFixColor' || index == 'rgbSpeed' ||
                index == 'rgbEffect' || index == 'interval' ||
				index == 'alarmPeriod'
            ){
            $('#'+index).val(value);
        }
        else if((index == 'enableDST' || index == 'set12_24' ||
                index == 'showZero' || index == 'enableBlink' ||
                index == 'enableAutoShutoff' || index == 'alarmEnable' ||
                index == 'rgbDir' || index == 'manualOverride'
                ) && !!value
            ){
            $('#'+index).prop('checked',true);
        }
        else if(index == "maxDigits"){
            //TODO
        }
    }
    $('.humidity-holder').toggleClass('hidden',configuration['humidity'] == 255);
    $('.temperature-holder').toggleClass('hidden',configuration['temperature'] == 255);
    $('.rgb-holder').toggleClass('hidden',configuration['rgbEffect'] == 255);

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
    },200);
}

function setCurrentInfos(){
    $.get('/getCurrentInfos/').done(function(data){
        $('#currentTime').html(data["currentDate"] + " " + data["currentTime"]);
        $('#humidity').html(data["humidity"]);
        $('#temperature').html(data["temperature"]);
        setTimeout(setCurrentInfos,5000);   //refreshes time every 5 second by calling itself
    }).always(function(){
        
    });
}

//Gets the current time from browser and gives back as readable format
function getCurrentTime(){
    var today = new Date();
    var dd = formatToTwoDigit(today.getDate());
    var mm = formatToTwoDigit(today.getMonth()+1); 
    var yyyy = today.getFullYear();
    var hour = formatToTwoDigit(today.getHours());
    var minute = formatToTwoDigit(today.getMinutes());
    return yyyy+"-"+mm+"-"+dd+" "+hour+":"+minute;
}

//if number is lower than 10, adds a zero
function formatToTwoDigit(number){
    return ("0" + number).slice(-2);
}