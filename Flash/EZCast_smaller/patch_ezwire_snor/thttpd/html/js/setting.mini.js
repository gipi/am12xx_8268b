var text_indeed;
var start;
var stop;
var text_string;
var defaultmode_index = -1;
var save_defaultmode_index;
var show_waiting_tick_id = -1;
var show_waiting_tick = 11;
var warn_msg;
var default_mode_str;
var defaultmode_global_text;
var function_index;
var show_warn_dialog_tick = 10;
var show_warn_dialog_id = -1;
var delay_get_data_id = -1;
var resolution_array = new Array();
var resolution_array = ["1024x768_60P","1280x720_60P", "1280x800_60P","1920x1080_24P", "1920x1080_30P", "1920x1080_60P"];
var resolution_list_value=new Array (3,0,4,1,2,5);
var resolution_index = 0;
var current_resolution_index = -1;
var resolution_current_value;
var resolution_current_value_num;
var defaultmode_array = new Array();
var defaultmode_array = ["Mirror Only", "Mirror+Streaming"];
var upgrade_bar = 0;
var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();

function show_wait() {
    $("#waiting").css('display', 'block');
    $("#waitbk").css('display', 'block');
}

function show_ui() {
    $("#waiting").css('display', 'none');
    $("#waitbk").css('display', 'none');
    $("#list_load").css('display', 'block');
	
}

function delaydisp_popup() {
    $('#warn').hide();
    $('#warn_time').hide();
  
}

function Get_text_from_textstring(text_string) {
    var text_string_tmp = text_string.substring(start, text_string_length);
    stop = text_string_tmp.indexOf("\n");
    start = 0;
    text_indeed = text_string_tmp.substring(start, stop);
    start = stop + 1;
    text_string_global = text_string_tmp;
    return text_indeed;
}



function Set_hdmimode() {
    $.post('cgi-bin/set_resolution_POST.cgi', $('#config-form').serialize());
}

function resolution_set_default_value() {
    var i=0;
    $('#res_value').val("" + resolution_current_value_num);
    $('#current_res_value').val("" + 0);
    resolution_index = current_resolution_index;
    $('#resolution_sel').text(resolution_array[resolution_index]);
    resolution_current_value = $('#resolution_sel').text();
}

function resolution_timetick_function() {
    $('#resolution_tick').html(show_waiting_tick);
    if (show_waiting_tick == 0) {
        if (show_waiting_tick_id > 0) {
            clearInterval(show_waiting_tick_id);
            show_waiting_tick_id = -1;
            resolution_set_default_value();
            //alert("--resolution_timetick--");
            Set_hdmimode();
            show_waiting_tick = 11;
            $("#Resolution").hide();
        }
    } else show_waiting_tick = show_waiting_tick - 1;

}
function set_resolution_timetick() {
    if (show_waiting_tick_id > 0) {
        clearInterval(show_waiting_tick_id);
        show_waiting_tick_id = -1;
    }
    show_waiting_tick = 15;
    show_waiting_tick_id = self.setInterval(resolution_timetick_function, 1000);
}
//get resolution text for ui
function Get_resolution_text() {
        $('#dialog_msg').text("Use the new Resolution ?");
        $('#time_tick_msg').text("sec left to return your setting last time.");
        $('#reboot_msg').text("System will reboot if you choose OK!");
        $('#dialog_msg2').text("Please switch to other resolutions if the display is choppy when streaming high bit rate videos for 1920x1080 60P.");
}
//resolution button
function Set_resolution_listener() 
{
    $("#set_resolution_ok").click(function() {
        Get_resolution_text();
        if (resolution_index != current_resolution_index) {
                $('#dialog_msg2').hide();
    		
            $("#res_sel").css('display', 'none');
            $("#res_warn").css('display', 'block');
            $('#resolution_tick').html("15");//time cnt
            $('#current_res_value').val("" + resolution_current_value_num);
            Set_hdmimode();
            if (((1 == current_resolution_index) && (2 == resolution_index)) || ((2 == current_resolution_index) && (1 == resolution_index))) 
                $('#reboot_msg').hide();
            else if (((17 == current_resolution_index) && (2 == resolution_index)) || ((2 == current_resolution_index) && (17 == resolution_index))) 
                $('#reboot_msg').hide();
            else 
                $('#reboot_msg').show();
            set_resolution_timetick();
        }
    });


    $("#resolution_ent").click(function() {
        var i=0;
        clearInterval(show_waiting_tick_id);
        show_waiting_tick_id = -1;
        var temp = $('#res_value').val();
        resolution_current_value_num = $('#res_value').val();
        temp = temp + "OK";
        $('#res_value').val("" + temp);


            for(i=0;i<resolution_list_value.length;i++){
                if(resolution_list_value[i]==resolution_index)
                    break;
            }
            $('#resolution_sel').text(resolution_array[i]);

		
        resolution_current_value = $('#resolution_sel').text();
        current_resolution_index=resolution_index;
        Set_hdmimode();
        $("#Resolution").hide();
    });

    $("#resolution_cancle").click(function() {
		clearInterval(show_waiting_tick_id);
		show_waiting_tick_id = -1;
		resolution_set_default_value();
		Set_hdmimode();
		$("#Resolution").hide();
    });

    $("#set_resolution_cal").click(function() {
		resolution_index = current_resolution_index;
		$('#Resolution').hide();

    });

    $('input:radio[name="resolution_choose"]').click(function() {
        var that = $(this).val();
        var b = $(this).prev().text();
        resolution_index = Number(that);
        $('#res_text').text(resolution_index);
        switch (resolution_index) {
            case 0:
                $('#res_value').val("5"); //FMT_1650x750_1280x720_60P
                break;
            case 1:
                $('#res_value').val("4"); //FMT_2750x1125_1920x1080_24P
                break;
            case 2:
                $('#res_value').val("2"); //FMT_2200x1125_1920x1080_30P
                break;
            case 3:
                $('#res_value').val("15"); //FMT_1250x810_1024x768_60P
                break;
            case 4:
                $('#res_value').val("16"); //FMT_1280x800_60P
                break;
            case 5:
                $('#res_value').val("17"); //FMT_1920x1080_60P
                break;
        }
          console.log("select   "+resolution_index);

    });

    $('#resolution_all').click(function() {
		console.log("resolution_all  click"+resolution_index);
        $('input:radio[name="resolution_choose"]').each(function() {
			console.log("each radio: "+$(this).val());
			// $(this).removeAttr("checked");
			$(this).attr("checked", " ");
        });
        set_resolutionradio();
        $('#Resolution').show();
        $("#res_sel").css('display', 'block');
        $("#res_warn").css('display', 'none');
    });

}
//set resolution css
function set_resolutionradio() {
    var that = $("input[type=radio][name=resolution_choose][value=" + resolution_index + "]");
    that.attr("checked", true);
}
//Creat resolution list
function Creat_resolution_list() {
    for (var i = 0; i < resolution_array.length; i++) {
        var that=$("label[for=res_" + i + "]");
        that.text(resolution_array[i]);
        that.prev().val(resolution_list_value[i]);
    }
    get_ddr_type();
   // $("#res_0_txt,#res_0,#res_2_txt,#res_2").remove();
   // set_resolutionradio();
}
//read resolution value
function Get_resolution() {

    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, { type: 'get_resolution'},function(resolution) {
		console.log("Get_resolution : "+resolution);
            var res = resolution.indexOf("15");
            if (res >= 0) {
                $('#res_value').val("15");
                resolution_index = 3;
                $('#resolution_sel').text("1024x768_60P"); //FMT_1024x768_60P
            } else {
                res = resolution.indexOf("4");
                if (res >= 0) {
                    $('#res_value').val("4");
                    resolution_index = 1;
                    $('#resolution_sel').text("1920x1080_24P");
                } else {
                    res = resolution.indexOf("2");
                    if (res >= 0) {
                        $('#res_value').val("2");
                        resolution_index = 2;
                        $('#resolution_sel').text("1920x1080_30P");
                    } else {
                        res = resolution.indexOf("17");
                        if (res >= 0) {
                            $('#res_value').val("17");
                            resolution_index = 5;
                            $('#resolution_sel').text("1920x1080_60P"); //FMT_1920x1080_60P
                        } else {
                            res = resolution.indexOf("5");
                            if (res >= 0) {
                                $('#res_value').val("5");
                                resolution_index = 0;
                                $('#resolution_sel').text("1280x720_60P");
                            }else{
                                res=resolution.indexOf("13");
                                if(res>=0){
                                    $('#res_value').val("13");
                                    resolution_index=4;
                                    $('#resolution_sel').text("720x480_60P"); //FMT_720x480_60P
                                }
                            }
                        }
                    }
                }
            }
        current_resolution_index = resolution_index;
        resolution_current_value_num = $('#res_value').val();
        resolution_current_value = $('#resolution_sel').text();
        Creat_resolution_list();

    },"text");
}



//default_mode button click
function default_mode_listener() {
    $('#defmode_ok').click(function(e) {
        $(document.body).css("overflow", "visible");
        if (save_defaultmode_index != defaultmode_index) {
            $('#Default_mode_value').val(defaultmode_index);
            $('#default_mode').text("" + defaultmode_array[defaultmode_index - 1]);
            $.post('cgi-bin/set_defaultmode_POST.cgi', $('#Default_mode_value').serialize());
		
            save_defaultmode_index = defaultmode_index;
           
        }
        $("#defaultmode").hide();
    });

    $('input:radio[name="de_item"]').click(function() {
        var that = $(this).val();
        var b = $(this).prev().text();
        defaultmode_index = that;
        $('#def-mode_text').text(that + b); //print user
        $('#Default_mode_value').val(that);

    });

    $("#defmode_cel").click(function() {
        defaultmode_index = save_defaultmode_index;
        $(document.body).css("overflow", "visible");
        $("#defaultmode").css("display","none");

    });


    $("#li_defaultmode").click(function() {
		console.log("---$(#li_defaultmode).click-----");
	$("#defaultmode").show();

        $('input:radio[name="de_item"]').each(function() {
            $(this).attr("checked", "");
        });
        set_defaultmode_radio();
        //$(document.body).css("overflow", "hidden");
	
    });

}
//set default_mode css
function set_defaultmode_radio() {
    var that = $("input[type=radio][name=de_item][value=" + defaultmode_index + "]");
    that.attr("checked", "checked");

}
//read default_mode val
function Get_defaultmode_UI() {
    var cgiurl = "cgi-bin/get_defaultmode.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_default_mode'},function(last_ui_index) {
        if (last_ui_index.indexOf("1") >= 0) {
            defaultmode_index = 1;//EZCast Pro
            save_defaultmode_index = defaultmode_index;
        } else if (last_ui_index.indexOf("2") >= 0) {
            defaultmode_index = 2;//EZMirror+Timer
            save_defaultmode_index = defaultmode_index;
        }
        $('#default_mode').text("" + defaultmode_array[defaultmode_index - 1]);
        set_defaultmode_radio();
    },"text");
}


function reload_page() {
    window.location.href = "websetting.html";
}
//any button click
function click_listener() {

    clearInterval(delay_get_data_id);
    delay_get_data_id = -1;
    default_mode_listener();

    //Set_resolution_listener();

}

function delay_get_data_function() {
    if (delay_get_data_id > 0) {
        clearInterval(delay_get_data_id);
        delay_get_data_id = -1;
    }
    Get_defaultmode_UI();
   // Get_resolution();
    delay_get_data_id = self.setInterval(click_listener, 1000);

}

//first run
function delay_get_data() {
    if (delay_get_data_id > 0) {
        clearInterval(delay_get_data_id);
        delay_get_data_id = -1;
    }
    delay_get_data_id = self.setInterval(delay_get_data_function, 100); 
}
