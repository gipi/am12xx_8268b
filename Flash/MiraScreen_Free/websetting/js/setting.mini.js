var text_indeed;
var start;
var stop;
var text_string;
var text_string_length;
var defaultmode_index = -1;
var save_defaultmode_index;
var connection_mode_index = -1;
var current_connection_mode_index = -1;
var show_waiting_tick_id = -1;
var show_waiting_tick = 11;
var appskValue;
var appskValue_save;
var warn_msg;
var default_mode_str;
var defaultmode_global_text;
var function_index;
var show_warn_dialog_tick = 10;
var show_warn_dialog_id = -1;
var check_OTA_first_flag = -1;
var Upgrade_listener_flag = -1;
var client_connect_show_flag = -1;
var connected_ssid_flag = 0;
var not_wifi_connect_string = "";
var show_connections_num_id = -1;
var get_connected_ssid_id = -1;
var screen_width, screen_height, sWidth, sHeight;
var appsk_input_flag = -1;
var upgrade_msg;
var upgrade_connect_fail_OK_CANCEL_flag = -1;
var Check_new_version_boot_id = -1;
var get_ota_check_status_tick = 0;
var get_new_version_timer_id = -1;
var delay_get_data_id = -1;
var get_ota_status_id = -1;
var No_new_version_string = "";
var upgrade_dialog_msg_string;
var upgrade_warn_string;
var PSW_LEN_string = "Password must be between 8 and 15 characters long.";
var PSW_ALLOW_string = "Only the letters and numbers are allowed!";
var LIST_WARN2_string = "Access Point will be disconnected if you press OK.";
var LIST_WARN3_string="Please enter a new password to save again";
var changes_effect_string="Changes take effect after reboot.";
var waiting_string,password_string;
var wifi_mode_status;
var wifi_connect_flag;
var resolution_array = new Array();
var resolution_array = ["1024x768_60P","1280x720_60P", "720x480_60P","1920x1080_24P", "1920x1080_30P", "1920x1080_60P"];
var resolution_list_value=new Array (3,0,4,1,2,5);
//var resolution_array = ["1280x720_60P", "1920x1080_24P", "1920x1080_30P", "1920x1080_60P"];
//var resolution_list_value=new Array (0,1,2,5);
var resolution_index = 0;
var wifi_mode_flag = 0;
var current_resolution_index = -1;
var current_wlan_ip_value = -1;
var resolution_current_value;
var resolution_current_value_num;
var CONFIG_3TO1_value = -1;
var defaultmode_array = new Array();
var defaultmode_array = ["Airplay", "Miracast"];
var ezairmode_index = 0,ezairmode_select = 0,select_txt,ezarimode_txt="EZAir mode";
var ezairmode_array = new Array();
var ezairmode_array = ["Mirror+Stream","Mirror Only"];
var upgrade_bar = 0;
var ota_auto_flag=0;
var multiLanguage_array = new Array(29);
var auto_language_flag = -1;
var get_current_language_id = -1;
var get_multilanguage_id = -1;
var language_index = -1;
var current_language_index;
var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
var lanArr = ["en","fr","de","es","pl","zh-CN","zh","ja","ko","it","cs","da","ru","nl","fi","nb","pt","hu","ro","sk","tr","sv","el","ar","id","he","th","fa"];

function Get_height_width_of_screen() {

    sWidth = document.documentElement.scrollWidth || document.body.scrollWidth;
    sHeight = document.documentElement.scrollHeight || document.body.scrollHeight;

    if ( !! (window.attachEvent && !window.opera)) {
        screen_height = document.documentElement.clientHeight;
        screen_width = document.documentElement.clientWidth;
    } else {
        screen_height = window.innerHeight;
        screen_width = window.innerWidth;
    }
    //document.getElementById('msg').value  ='width:' + screen_width + '; height:'+screen_height;
}


window.onresize = window.onload = function() {
    Get_height_width_of_screen();
}

function show_wait() {
    $("#waiting").css('display', 'block');
    $("#waitbk").css('display', 'block');
}

function show_ui() {
    $("#waiting").css('display', 'none');
    $("#waitbk").css('display', 'none');
    $("#list_load").css('display', 'block');
	
}


function To_wifilist() {
     setTimeout("location.href='wifilist.html';",3000); 
}

/************************************user new copy*************************************/
function updata_button_mutilanguage(){

        $('#set_appsk_ok').text(ok_string);
        $('#set_appsk_cel').text(cancle_string);
        $('#ezairmode_ok').text(ok_string);
        $('#ezairmode_cel').text(cancle_string);
        $('#defmode_ok').text(ok_string);
        $('#defmode_cel').text(cancle_string);
        $('#set_resolution_ok').text(ok_string);
        $('#set_resolution_cal').text(cancle_string);
        $('#resolution_ent').text(ok_string);
        $('#resolution_cancle').text(cancle_string);
        $('#upgrade_OK').text(ok_string);
        $('#upgrade_CANCEL').text(cancle_string);
        $('#ota_awrn_ok').text(ok_string);
        $('#reboot_ok').text(ok_string);
        $('#reboot_cancel').text(cancle_string);
        $('#reset_ok').text(ok_string);
        $('#reset_ent').text(ok_string);
        $('#reset_cancel').text(cancle_string);
        $('#reboot_refresh').text(ok_string);
        $('#lang_ok').text(ok_string);
        $('#lang_cancel').text(cancle_string);

}
//¶ÁÈ¡¶à¹úÓï ,ÆÚËüÆ½Ì¨
var index_text_string=-1;
function Get_multi_language() {
    var tmp_string="";
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_webmultilanguage_text'},function(index_text_string) {
	
        if (index_text_string.length) {
            start = 0;
            text_string_length = 0;
            text_string = index_text_string;
            text_string_length = index_text_string.length;
            $('#heard').text(Get_text_from_textstring(text_string)); //Setting
            text_string = text_string_global;
            $('#internet_txt').text(Get_text_from_textstring(text_string)); //Internet
            text_string = text_string_global;
            password_string =	Get_text_from_textstring(text_string);//Password
            $('#password_txt').text("WIFI "+password_string);
            $('#le_password').text(password_string+":");//Password
            text_string = text_string_global;
            $('#resolution_txt').text(Get_text_from_textstring(text_string));//Resolution
            text_string = text_string_global;
            $('#language_txt').text(Get_text_from_textstring(text_string));//Language
            text_string = text_string_global;
            $('#defaultmode_txt').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            ezarimode_txt = Get_text_from_textstring(text_string); //EZAir mode
            $("#ezairmode_title").text(ezarimode_txt);
            text_string=text_string_global;	
            ezairmode_array[1] = Get_text_from_textstring(text_string);
            $('label[for=mode_0]').text(ezairmode_array[1]); //Mirror only
            text_string=text_string_global;	
            ezairmode_array[0] = Get_text_from_textstring(text_string);
            $('label[for=mode_1]').text(ezairmode_array[0]); //Mirror+Streaming
            text_string = text_string_global;
            changes_effect_string = Get_text_from_textstring(text_string);//Changes take effect after reboot.
            $('#ezairmode_warn_txt').text(changes_effect_string);
            text_string = text_string_global;
            $('#upgrade_txt').text(Get_text_from_textstring(text_string));//Upgrade
            text_string = text_string_global;
            ok_string = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            cancle_string = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            LIST_WARN3_string = Get_text_from_textstring(text_string);//Please enter a new password to save again
            text_string=text_string_global;
            $('#title_rest').text(Get_text_from_textstring(text_string)); //reset to default
            text_string=text_string_global;
            $('#title_root').text(Get_text_from_textstring(text_string)); //restatr system
            text_string=text_string_global;
            waiting_string=Get_text_from_textstring(text_string); //Please wait
            text_string=text_string_global;
            not_wifi_connect_string=Get_text_from_textstring(text_string); //not wifi connect!
            text_string=text_string_global;
            No_new_version_string=Get_text_from_textstring(text_string); //No new version!

            updata_button_mutilanguage();
 			
        } else {
            $('#heard').text("Setting");
            $('#resolution_txt').text("Resolution");
            $('#password_txt').text("WIFI Password");
            $('#le_password').text("Password : ");
            $('#internet_txt').text("Internet");
            $('#language_txt').text("Language");
            $('#upgrade_txt').text("Upgrade");
            $("#ezairmode_title").text("EZAir mode");
            $('#title_rest').text("Reset to Default");
            $('#title_root').text("Restart System");
        }
		
    },"text");
}



function Get_ota_status_tick() {

    var cgiurll = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurll, {type: 'get_ota_status'},function(ota_status)
    {
        upgrade_bar = ota_status;
	console.log("upgrade_% :"+upgrade_bar)
        $("#progress_label").text(upgrade_bar + "%");
        $("#progressbar").css("width", upgrade_bar + "%");
		
        if (ota_status >= 100) {
            clearInterval(get_ota_status_id);
            get_ota_status_id = -1;
        }
    },"text");
}
function Get_ota_status() {
    if (get_ota_status_id > 0) {
        clearInterval(get_ota_status_id);
        get_ota_status_id = -1;
    }
    get_ota_status_id = self.setInterval(Get_ota_status_tick, 1000);
}



function Get_upgrade_language() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_OTA_text'},function(OTA_string) {
		console.log("Get_upgrade_language: "+ OTA_string);
        if (OTA_string.length) {
            start = 0;
            text_string_length = 0;
            text_string = OTA_string;
            text_string_length = OTA_string.length;
            $('#upgrade_dialog_msg').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#local_text').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#server_text').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#upgrade_warn').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            upgrade_dialog_msg_string = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            upgrade_warn_string = Get_text_from_textstring(text_string);
            text_string = text_string_global;
        } else {
            $('#upgrade_dialog_msg').text("Do you want to upgrade?");
            $('#server_text').text("server version :");
            $('#local_text').text("local version :");
            $('#upgrade_warn').text("Access Point will be disconnected if you press OK."); //add by cxf	
            upgrade_dialog_msg_string = "Please wait for downloading.";
            upgrade_warn_string = "After download, wifi will be disconnected and please wait for system upgrading and reboot.";
        }
    },"text");
}
function show_warn_dialog_tick_function() {
    if (show_warn_dialog_tick < 1)
    {
        $('#ota_fail').hide();
        $('#warn').hide();
        $('#warn_time').hide();

        clearInterval(show_warn_dialog_id);
        show_warn_dialog_id = -1;
        show_warn_dialog_tick = 11;
    } else {
        show_warn_dialog_tick = show_warn_dialog_tick - 1;
        $('#upgrade_tick').text("" + show_warn_dialog_tick);
    }
}
function show_OTA_warn_dialog() {
    show_warn_dialog_tick = 10;
    $('#upgrade_tick').text("" + show_warn_dialog_tick);
    //$('#ota_fail').popup('open');
    if (show_warn_dialog_id > 0) {
        clearInterval(show_warn_dialog_id);
        show_warn_dialog_id = -1;
    }
    show_warn_dialog_id = self.setInterval(show_warn_dialog_tick_function, 1000);
}


function show_faild_connect_tick_function() {
    if (show_warn_dialog_tick < 1) 
    {
        $('#ota_fail').hide();
        clearInterval(show_warn_dialog_id);
        show_warn_dialog_id = -1;
        show_warn_dialog_tick = 11;
    } else {
        show_warn_dialog_tick = show_warn_dialog_tick - 1;
        $('#upgrade_tick').text("" + show_warn_dialog_tick);
    }
}


function show_faild_connect_dialog() {
    show_warn_dialog_tick = 10;
    $('#upgrade_tick').text("" + show_warn_dialog_tick);
    $('#ota_fail').show();
    $('#warn').hide();
    $('#warn_time').show();
    if (show_warn_dialog_id > 0) {
        clearInterval(show_warn_dialog_id);
        show_warn_dialog_id = -1;
    }
    show_warn_dialog_id = self.setInterval(show_faild_connect_tick_function, 1000);
    upgrade_connect_fail_OK_CANCEL_flag = 1;
   
}

function Get_faild_connect_language() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_OTA_faild_connect'},function(OTA_faild_connect) {
        if (OTA_faild_connect.length) {
            start = 0;
            text_string_length = 0;
            text_string = OTA_faild_connect;
            text_string_length = OTA_faild_connect.length;
            $('#upgrade_dialog_msg2').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#upgrade_time_tick_msg').text(Get_text_from_textstring(text_string));
        } else {
            $('#upgrade_dialog_msg2').text("Faild to connect,please try again.");
            $('#upgrade_time_tick_msg').text("sec left to return your setting last time.");
        }
    },"text");
}

function delaydisp_popup() {
    $('#warn').hide();
    $('#warn_time').hide();
    $('#ota').show();
}

function set_version_string(ota_upgrade_string) {
    if (ota_upgrade_string.length > 0) {
        //if ((check_OTA_first_flag == 1) || (Upgrade_listener_flag == 1)) {
        //}
        var warn_ret = ota_upgrade_string.indexOf("newest");
        if (warn_ret < 0) {
            $('#ota_fail').hide(); 
            if (0 == connected_ssid_flag) {
                var ret = ota_upgrade_string.indexOf("\n");
                var local_version_string = ota_upgrade_string.substring(0, ret);
                $('#new_version').val(local_version_string);
            } else {
                Get_new_version_text();
            }
            if ((check_OTA_first_flag == 1) || (Upgrade_listener_flag == 1)) {
                if (check_OTA_first_flag == 1) 
                    check_OTA_first_flag = -1;
                else if (Upgrade_listener_flag == 1) 
                    Upgrade_listener_flag = -1;
                Get_upgrade_language();
                var ret = ota_upgrade_string.indexOf("\n");
                var local_version_string = ota_upgrade_string.substring(0, ret);
                console.log("local_version: "+local_version_string);
                $('#local_version').val(local_version_string);
                var server_version_string = ota_upgrade_string.substring(ret + 1, ota_upgrade_string.length);
                $('#server_version').val(server_version_string);
                console.log("server_version: "+server_version_string);
                setTimeout(delaydisp_popup, 500);
            }
        } else //show newest 
        {
            if (Upgrade_listener_flag == 1) {
                Upgrade_listener_flag = -1;
                check_OTA_first_flag = -1;
                get_OTA_warn_language(1);
                $('#warn').hide();
                $('#warn_time').show();
                show_OTA_warn_dialog();
                upgrade_connect_fail_OK_CANCEL_flag = 1;
            } else {
                var ret = ota_upgrade_string.indexOf("\n");
                var local_version_string = ota_upgrade_string.substring(0, ret);
                $('#new_version').val(local_version_string);
                $('#ota_fail').hide();
                $('#warn').hide();
                $('#warn_time').hide();

            }
        }
    }

}
/*
function get_new_version_tick_function() {
    if (get_new_version_timer_id > 0) {
        clearInterval(get_new_version_timer_id);
        get_new_version_timer_id = -1;
    }
    $.get(cgiurl, {type: 'get_OTA_version'},function(ota_upgrade_string) {
        if (ota_upgrade_string.length) 
            set_version_string(ota_upgrade_string);
    },"text");
}
*/
function get_new_version_function() {
    $.get(cgiurl, {type: 'get_OTA_version'},function(ota_upgrade_string) {
        if (ota_upgrade_string.length) {
            set_version_string(ota_upgrade_string);
        }
    },"text");
}


function get_OTA_warn_language(flag) {
    if (flag) {
        var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
        $.get(cgiurl, {type: 'get_OTA_warn1_text'},function(OTA_string) {
            if (OTA_string.length) {
                start = 0;
                text_string_length = 0;
                text_string = OTA_string;
                text_string_length = OTA_string.length;
                $('#upgrade_dialog_msg2').text(Get_text_from_textstring(text_string));
                text_string = text_string_global;
                $('#upgrade_time_tick_msg').text(Get_text_from_textstring(text_string));
            } else {
                $('#upgrade_dialog_msg2').text("Your firmware is the latest version.");
                $('#upgrade_time_tick_msg').text("sec left to return your setting last time.");
            }
        },"text");
    } else {
        var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
        $.get(cgiurl, {type: 'get_OTA_warn2_text'},function(OTA_string) {
            if (OTA_string.length) {
                start = 0;
                text_string_length = 0;
                text_string = OTA_string;
                text_string_length = OTA_string.length;
                $('#upgrade_dialog_msg2').text(Get_text_from_textstring(text_string));
                text_string = text_string_global;
                $('#upgrade_time_tick_msg').text(Get_text_from_textstring(text_string));
            } else {
                $('#upgrade_dialog_msg2').text("The server version of firmware is unavailable.");
                $('#upgrade_time_tick_msg').text("sec left to return your setting last time.");
            }
        },"text");

    }
}

function get_ota_check_status_tick_function() {
    $.get(cgiurl, {type: 'get_ota_check_status'},function(new_ver_flag) {
        get_ota_check_status_tick++;
        if (get_ota_check_status_tick > 50) //20
        {
            get_ota_check_status_tick = 0;
            if (Check_new_version_boot_id > 0) {
                clearInterval(Check_new_version_boot_id);
                Check_new_version_boot_id = -1;
            }
            $('#ota_fail').hide();
            $('#warn').hide();
            $('#warn_time').hide();

        } else {
            var ret = Number(new_ver_flag);
            if (1 == ret) //check finish
            {
                get_ota_check_status_tick = 0;
                if (Check_new_version_boot_id > 0) {
                    clearInterval(Check_new_version_boot_id);
                    Check_new_version_boot_id = -1;
                }
                get_new_version_function();
            }
        }
    },"text");
}
function get_ota_check_status() {
    if (Check_new_version_boot_id > 0) {
        clearInterval(Check_new_version_boot_id);
        Check_new_version_boot_id = -1;
    }
    Check_new_version_boot_id = self.setInterval(get_ota_check_status_tick_function, 1000);
}

function get_wifi_mode() {
    $.get(cgiurl, {type: 'get_wifi_mode'},function(wifi_mode) {
		console.log("ota ->get_wifi_mode :"+wifi_mode);
		console.log("ota ->Upgrade_listener_flage :"+Upgrade_listener_flag);
		console.log("ota ->wifi_connect_flag :"+wifi_connect_flag);
        wifi_mode_status = connected_ssid_flag;//wifi_mode.indexOf("9");
        if (wifi_mode_status > 0) {
            if (!wifi_connect_flag) 
            {
                Get_faild_connect_language();
                show_faild_connect_dialog(); //no wifi connect warn
            } else {
                if (Upgrade_listener_flag == 1) {
                    $('#upgrade_dialog_msg').text(waiting_string);
                    $('#ota_fail').show();
                    $('#warn').show();
                    $('#warn_time').hide();
                }
                $.get(cgiurl, {type: 'get_OTA_conf'},function() {

                }, "text");
                get_ota_check_status();
            }
        } else {
            Get_faild_connect_language();
            show_faild_connect_dialog(); //no wifi connect warn
        }
    },"text");
}


//ota upgrade button click
function Upgrade_listener() {
    $('#upgrade').click(function(e) {
        var ret;
        e.preventDefault();
        Upgrade_listener_flag = 1;
        get_wifi_mode();
    });
}
function changeota() {
    $("#1").css('display', 'none');
    $("#2").css('display', 'block');
}
//ota upgrade button click
function Upgrade_OK_listener() {
    $('#upgrade_OK').click(function() {
        changeota();
        if (upgrade_connect_fail_OK_CANCEL_flag == 1) {
            if (show_connections_num_id > 0) {
                clearInterval(show_connections_num_id);
                show_connections_num_id = -1;
            }
        } else {
            $.get(cgiurl, {type: 'get_down_bin'},function(return_val) {
                var ret = return_val.indexOf("OK");
                if (ret >= 0) {
                    Get_ota_status();
                }
            },"text");
            if (show_connections_num_id > 0) {
                clearInterval(show_connections_num_id);
                show_connections_num_id = -1;
            }
            $('#upgrade_warn').text(upgrade_warn_string);
        }
    });


    $('#upgrade_CANCEL').click(function() {
        $('#ota').hide();
    });

    $('#ota_awrn_ok').click(function() {
        $('#ota_fail').hide();
        $('#warn').show();
        $('#warn_time').hide();
        clearInterval(show_warn_dialog_id);
        show_warn_dialog_id = -1;
        show_warn_dialog_tick = 11;
    });

}


var loopnum = 0;
var lan_con = 0;
var connected_ssid = "";
function Get_connected_ssid_tick() {
    loopnum++;
    if (loopnum > 10) {
        clearInterval(get_connected_ssid_id);
        get_connected_ssid_id = -1;
    }
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); 
    $.get(cgiurl, {type: 'get_connected_ssid'},function(connected_ssid) {
        if (!Array.indexOf) {
            Array.prototype.indexOf = function(obj) {
                for (var i = 0; i < this.length; i++) {
                    if (this[i] == obj) {
                        return i;
                    }
                }
                return - 1;
            }
        }
	console.log("Get_connected_ssid_tick: "+connected_ssid);
        var ret = connected_ssid.indexOf("to_wifi_list");
        if (ret >= 0) {
            lan_con = connected_ssid.indexOf("to_wifi_list&lan");
            if (lan_con >= 0) 
                lan_con = 1;
            else 
                $('#connected_ssid').hide();
        } else {
            if (connected_ssid.length && lan_con != 1) {
                if (connected_ssid_flag == 0) {
                    $('#connected_ssid').text(connected_ssid);
                    $('#connected_ssid').show();
                    wifi_connect_flag = 1;
                    connected_ssid_flag = 1;
                    show_connections_num_id = self.setInterval(Get_local_version, 3000);
                }
            } else {
                //$('#connected_ssid').hide();
                $('#connected_ssid').text(not_wifi_connect_string);
                wifi_connect_flag = 0;
                connected_ssid_flag = 0;
            }
        }
    },"text");
}


function Get_new_version_text() {
    $.get(cgiurl, {type: 'get_new_ver_text'},function(new_version_text) {
        if (new_version_text.length) {
            $('#new_version').show();
            $('#new_version').text(new_version_text);
            $('#upg_txt_2').text("--2_New Version!--"); 
        } else {
            $('#new_version').show();
            $('#new_version').text("New Version!");
            $('#upg_txt').text("New Version!");

        }
    },"text");
}

function Get_connected_ssid() {
    Get_connected_ssid_tick();
    if (get_connected_ssid_id > 0) {
        clearInterval(get_connected_ssid_id);
        get_connected_ssid_id = -1;
    }
    loopnum = 0;
    get_connected_ssid_id = self.setInterval(Get_connected_ssid_tick, 3000);
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

//read current version
var loopnum_currver = 0;
function Get_local_version() {
    loopnum_currver++;
    if (loopnum_currver > 5) {
        clearInterval(show_connections_num_id);
        show_connections_num_id = -1;
    }
    $.get(cgiurl, {type: 'get_OTA_version'},function(ota_upgrade_string) {
		console.log("Get_local_version: "+ ota_upgrade_string);
        if (ota_upgrade_string.length) {
            start = 0;
            text_string_length = 0;
            text_string = ota_upgrade_string;
            text_string_length = ota_upgrade_string.length;
            var local_version_string = Get_text_from_textstring(text_string);
            $('#current_ver').text(local_version_string);
            $('#new_version').text(local_version_string);
            text_string = text_string_global;
            var server_version_string = Get_text_from_textstring(text_string);
            $.get(cgiurl, {type: 'get_wifi_mode'},function(wifi_mode) {
                wifi_mode_status = connected_ssid_flag;//wifi_mode.indexOf("9");
                if (wifi_mode_status > 0) {
                    var ret = server_version_string.indexOf("newest");
                    if (ret < 0) {
                        Get_new_version_text();
                        clearInterval(show_connections_num_id);
                        show_connections_num_id = -1;
                        loopnum_currver = 6;
                        clearInterval(get_connected_ssid_id);
                        get_connected_ssid_id = -1;
                        loopnum = 20;
                    } else {
                        $('#new_version').show();
                        $('#new_version').text(No_new_version_string);
                    }
                } else {
                    $('#upg_txt').text("no wifi_mode_status");
                    //$('#new_version').hide();
                    $('#new_version').text(not_wifi_connect_string);
                }
            },"text");
        }
    },"text");
}
function Check_new_version_boot() {
    $.get(cgiurl, {type: 'get_wifi_mode'},function(wifi_mode) {
        wifi_mode_status = connected_ssid_flag;//wifi_mode.indexOf("9");
        if (wifi_mode_status > 0) {
            $.get(cgiurl, {type: 'enter_check_ota_boot'},function(return_value) {
                var ret = Number(return_value);
                if (0 == ret) {
                    $.get(cgiurl, {type: 'get_OTA_conf'},function() {

                    }, "text");
                    get_ota_check_status();
                }
            },"text");
        }
    },"text");
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
function get_ddr_type() {
    var cgiurll = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurll, {type: 'read_ddrtype'},function(type)
    {
        //$("#res_0_txt,#res_0,#res_2_txt,#res_2").remove();
		$("#res_0_txt,#res_0").remove();
        if (Number(type)==2) {
			
             $("#res_5_txt,#res_5").remove();
        }
   	 set_resolutionradio();
    },"text");
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
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_res_text'},function(restext) {
        if (restext.length) {
            start = 0;
            text_string_length = 0;
            text_string = restext;
            text_string_length = restext.length;
            $('#dialog_msg').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#time_tick_msg').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#reboot_msg').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#dialog_msg2').text(Get_text_from_textstring(text_string));
            //document.getElementById("reboot_msg").style.color="red";
        } else {
            $('#dialog_msg').text("Use the new Resolution ?");
            $('#time_tick_msg').text("sec left to return your setting last time.");
            $('#reboot_msg').text("System will reboot if you choose OK!");
            $('#dialog_msg2').text("Please switch to other resolutions if the display is choppy when streaming high bit rate videos for 1920x1080 60P.");
            //document.getElementById("reboot_msg").style.color="red";
        }
    },"text");
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
                $('#res_value').val("13"); //FMT_1280x800_60P  16   FMT_720x480_60P  
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
	$("#res_0_txt,#res_0,#res_3_txt,#res_3,#res_4_txt,#res_4").remove();
    //get_ddr_type();
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
            $("#defaultmode").hide();
        }
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

    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_last_ui'},function(last_ui_index) {
	//console.log("Get_defaultmode_UI: "+last_ui_index);
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

//read ezair value
function Get_ezairmode_ui() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_ezairmode'},function(ret) {
        var value = Number(ret);
	console.log("Get_ezairmode_ui: "+value);
        ezairmode_index = value;
        ezairmode_select = ezairmode_index;
        set_ezairmode_style();
    }, "text");
}

//ezair button
function ezairmode_listener() {
    $("#ezairmode_ok").click(function(e) {
        set_ezairmode_ui();
        $("#ezairmode").hide();
    });

    $("#ezairmode_cel").click(function(e) {
        if (ezairmode_select != ezairmode_index) {
            ezairmode_select = ezairmode_index;
        }
        $("#ezairmode").css("display","none");
    });

    $('input:radio[name="ezairmodemode_item"]').click(function() {
        var that = $(this).val();
        ezairmode_select = that;
        select_txt = ezairmode_array[ezairmode_select];
    });

    $("#ezair").click(function() {
	$("#ezairmode").show();
        $('input:radio[name="ezairmodemode_item"]').each(function() {
            $(this).attr("checked", "");
        });
        set_ezairmode_style();
	
    });

}

//set ezari css
function set_ezairmode_style() {
    var that = $("input[type=radio][name=ezairmodemode_item][value=" + ezairmode_index + "]");
    that.attr("checked", "checked");
    select_txt = ezairmode_array[ezairmode_index];
    if(ezairmode_index)
	$("#ezairmode_txt").text($('label[for=mode_0]').text());
    else
	$("#ezairmode_txt").text($('label[for=mode_1]').text());
}

//set ezair value
function set_ezairmode_ui() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    var str = ezairmode_select + "\n" + "set_ezairmode\n";
    $.get(cgiurl, {type: str},function(ret) {
    }, "text");
    $("#ezairmode_txt").text(select_txt);
    ezairmode_index = ezairmode_select;
}

function reload_page() {
    window.location.href = "websetting.html";
}
//ÐÞ¸ÄÃÜÂë
function change_psw() {
    $.post('cgi-bin/password_POST.cgi', $('#appsk').serialize());
    setTimeout(reload_page, 1000);
}
//ÉèÖÃpassword
function set_appsk() {
    var errorFlag = 0;
    var regx = /^[A-Za-z0-9]+$/;
    appskValue = $('#appsk').val();
 if (appskValue != appskValue_save)
    {
    	if (!regx.test(appskValue))
	{
            $('#psk_warn_txt').text(PSW_ALLOW_string);
            errorFlag = 1;
       } 
	else 
	{
        	if (appskValue.length < 8 || appskValue.length > 15)
	 	{
	               $('#psk_warn_txt').text(PSW_LEN_string);
	               $('#app_sk').text("" + appskValue_save);
	               errorFlag = 1;
            	} 
	   	 else
		{
               	 errorFlag = 0;
               }
        }
        if (!errorFlag) 
	 {
            $('#app_sk').text(appskValue);
            $('#psk_warn_txt').text("Password modified successfully!");
            change_psw();
        }
    }
    else 
    {
     	$('#psk_warn_txt').text(LIST_WARN3_string);  //"Please enter a new password to save again");
    }
}
//ÃÜÂëÉèÖÃ°´¼üÏìÓ¦
function Set_pws_listener() {
    $("#set_appsk_ok").click(function() {
        set_appsk();
    });
	
    $("#appsk").keyup(function(){
        //alert(event.keyCode);
        if(event.keyCode==13){
            set_appsk();
        }
    });
    $("#pass_sel").click(function() {
        $('#appsk').val($('#app_sk').text());
        $('#Password').show();
    });
    $("#set_appsk_cel").click(function() {
        $('#Password').hide();
    });
}
//¶ÁÈ¡Ð¡»úÃÜÂë
function Get_password() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_softap_psk'},function(appsk) {
		console.log("Get_password"+appsk);
        $('#app_sk').text(appsk);
        $('#appsk').text(appsk);
        appskValue_save = $('#app_sk').text();
    },"text");
}
//Language click
function Language_listener() {
    $('#li_language').click(function(e) {
        $('input:radio[name="lang_list"]').each(function() {
            $(this).attr("checked", "");
        });
        set_langradio();
        $('#language_sel').show();
    });
    $('#lang_cancel').click(function(e) {
        $('#language_sel').hide();
    });
    $("#lang_ok").click(function() {
        language_index = $('input:radio[name="lang_list"]:checked').val();
        if (current_language_index != language_index) {            
            if(language_index == multiLanguage_array.length - 1){//multiLanguage_array.length - 1 === 28
                for(var i = 0 ; lanArr[i]; i++){
                    if(lanArr[i] == navigator.language){
                        console.log("language_index: "+i);
                        language_index =i;
                        break;
                    }
                    else if(lanArr[i] == navigator.language.substring(0,2)) {
                        console.log("language_index: "+i);
                        language_index =i;
                        break;
                    }
                }
                if(language_index == multiLanguage_array.length - 1) language_index = 0;// 沒有對應語言 -> Eng
                $('#lang_sel').text(multiLanguage_array[language_index]);
                $('#lan_index').val(language_index);
                $.post('cgi-bin/set_lan_POST.cgi', $('#config-form').serialize());
                setTimeout(function(){
                    Get_multilanguage();
                    $('#lang_sel').text(multiLanguage_array[multiLanguage_array.length - 1]);
                    $('#lan_index').val(multiLanguage_array.length - 1);
                    $.post('cgi-bin/set_lan_POST.cgi', $('#config-form').serialize());
                    $('#language_sel').hide();
                }, 300);
            }
            else{                
                $('#lang_sel').text(multiLanguage_array[language_index]);
                $('#lan_index').val(language_index);
                $.post('cgi-bin/set_lan_POST.cgi', $('#config-form').serialize());
                setTimeout(function(){
                    Get_multi_language();
                    Get_reboot_reset_language();
                    $('#language_sel').hide();
                }, 300);                
                setTimeout(function(){
                    set_ezairmode_style();
                    Get_local_version();
                },1000);
            }
            current_language_index = language_index;
        }
    });
    $('input:radio[name="lang_list"]').click(function() {
        var that = $(this);
        var b = $(this).prev().text();
        $("#lantxt").text(b);
    });
}

//language select css set
function set_langradio() {
    $("input[type=radio][name=lang_list][value=" + (current_language_index) + "]").attr("checked", "true");
}
//use js create language list
function Creat_lang_list() {
    var i = 0;
    var obj;
    $("#langlist_left").html('');
    $("#langlist_right").html('');
    for (i = 0; i < multiLanguage_array.length; i++) {
        var index = i;
        var id = "lan_" + i;
        var val = index.toString();
        var multi_string = multiLanguage_array[i];
        if(i<multiLanguage_array.length/2)
		obj = $("#langlist_left");
        else
		obj = $("#langlist_right");
		
        var objChild=document.createElement("input");
		objChild.type = "radio";   
		objChild.name = "lang_list";   
		objChild.id = id;   
		objChild.value =val;  
	obj.append(objChild);
	
        var label_var = document.createElement("label");
            label_var.innerHTML = multi_string; 
            label_var.htmlFor = id;
            label_var.id = id+"_txt";
	obj.append(label_var);

       var line_bar = document.createElement("div");
            line_bar.style = "margin-bottom:5px;"; 
	obj.append(line_bar);
	
	
    }
    set_langradio();
    Language_listener();
}
function Get_current_language() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    if (get_current_language_id > 0) {
        clearInterval(get_current_language_id);
        get_current_language_id = -1;
    }
    $.get(cgiurl, {type: 'get_lan_index'}, function(get_lan_index) {
	console.log("Get_current_language: "+get_lan_index);
        var ret = get_lan_index.indexOf("auto");
        if (ret >= 0) {
            //$('#language').text("" + multiLanguage_array[multiLanguage_array.length - 1]);
            var lan_index = get_lan_index.substring(0, get_lan_index.length - 4);
            current_language_index = multiLanguage_array.length - 1;
            auto_language_flag = 1;
            for(var i = 0 ; lanArr[i]; i++){
                if(lanArr[i] == navigator.language){
                    console.log("language_index: "+i);
                    language_index =i;
                    break;
                }
                else if(lanArr[i] == navigator.language.substring(0,2)) {
                    console.log("language_index: "+i);
                    language_index =i;
                    break;
                }
            }
            if(language_index < 0 || language_index > multiLanguage_array.length - 2 ) language_index = 0;// 沒有對應語言 -> Eng
            $('#lang_sel').text(multiLanguage_array[language_index]);
            $('#lan_index').val(language_index);
            $.post('cgi-bin/set_lan_POST.cgi', $('#config-form').serialize());
            setTimeout(function(){
                Get_multi_language();
                Get_reboot_reset_language();
                $('#lang_sel').text(multiLanguage_array[current_language_index]);
                $('#lan_index').val(current_language_index);
                $.post('cgi-bin/set_lan_POST.cgi', $('#config-form').serialize());
            }, 300);
            setTimeout(function(){
                set_ezairmode_style();
                Get_local_version();
            },1000);
        } else {
            //$('#language').text("" + multiLanguage_array[get_lan_index]);
            current_language_index = Number(get_lan_index);
            auto_language_flag = 0;
        }
        language_index = current_language_index;
        $('#lang_sel').text(multiLanguage_array[language_index]);
        Creat_lang_list();
    },"text");

}
//from CGI read multi language text
function Get_multilanguage() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    if (get_multilanguage_id > 0) {
        clearInterval(get_multilanguage_id);
        get_multilanguage_id = -1;
    }
    $.get(cgiurl, {type: 'get_multilanguage'}, function(language) {
        if (language.length > 0) {
            start = 0;
            text_string_length = 0;
            text_string = language;
            text_string_length = language.length;
            for (var i = 0; i < multiLanguage_array.length; i++) {
                multiLanguage_array[i] = Get_text_from_textstring(text_string);
                text_string = text_string_global;
            }
            if (get_current_language_id > 0) {
                clearInterval(get_current_language_id);
                get_current_language_id = -1;
            }
            get_current_language_id = self.setInterval(Get_current_language, 100);
        }
    },"text");
}
//read mac
function Get_AP_mac_ip() {
    $.get("cgi-bin/wifi_info_GET.cgi", {type: '1\nget_mac_ip'},function(mac_wlan1_ip) {
        if (mac_wlan1_ip.length) {
            start = 0;
            text_string_length = 0;
            text_string = mac_wlan1_ip;
            text_string_length = mac_wlan1_ip.length;
            $('#connect_mac_address').text(Get_text_from_textstring(text_string));
        }
    },"text");
}

var rebrset_text_string=-1;
function Get_reboot_reset_language()
{
	var Refresh_txt,reboot_message,reset_message;
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:'get_rebret_text'}, function(rebrset_text_string)
	{	
		if(rebrset_text_string.length)
		{
			start=0;
			text_string_length=0;
			text_string=rebrset_text_string;
			$('#jsprintext2').text(rebrset_text_string.length+" | "+rebrset_text_string);
			text_string_length=rebrset_text_string.length;
			$('#title_rbot').text(Get_text_from_textstring(text_string));
			$('#reboot').text($('#title_rbot').text());
			text_string=text_string_global;	
			Refresh_txt=Get_text_from_textstring(text_string);//Refresh
			text_string=text_string_global;	
			reboot_message=Get_text_from_textstring(text_string);//System reboots, please wait.
			$('#message_txt').text(reboot_message);
			$('#reset_message_txt').text(reboot_message);
			text_string=text_string_global;	
			$('#rebot_warn').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			$('#reset').text(Get_text_from_textstring(text_string));
			text_string=text_string_global;	
			reset_message=Get_text_from_textstring(text_string);//The system is reboot!please wait.
			text_string=text_string_global;	
			$('#reset_warn').text(Get_text_from_textstring(text_string));
			/*
			text_string=text_string_global;	
			yes_txt=Get_text_from_textstring(text_string);//yes
			text_string=text_string_global;	
			no_txt=Get_text_from_textstring(text_string);//no

			$('#reboot_ok').text(yes_txt);
			$('#reboot_cancl').text(no_txt);
			$('#reset_ok').text(yes_txt);
			$('#reset_ent').text(yes_txt);
			$('#reset_cancl').text(no_txt);
			*/
			
		}
		else
		{
		$('#title_rbot').text("Reboot");
		$('#reboot').text("Reboot");
		$('#rebot_warn').text("Are you sure to reboot system?");
		Refresh_txt="Refresh";
		reboot_message="System reboots, please wait.";
		reset_message="The system is reboot!please wait.";
		$('#reboot_ok').text("Yes");
		$('#reboot_cancl').text("No");
		$('#reset_ok').text("Yes");
		$('#reset_cancl').text("No");

		$('#reset').text("Reset");
		$('#reset_warn').text("All the configurations will be lost! Are you sure?");
		}
	}, "text");	
}
function reset_reboot_listener(){
	
	$('#reset_init').click(function(e)   //diolog
	{
		$("#dialog1").show();
		$("#dialog2").hide();
		$("#resetpop").show();
	});	
	
	$('#reset_ent').click(function(e)   //reset to default
	{
		$("#dialog1").hide();
		$("#dialog2").show();
	});	
	
	$('#reset_cancel').click(function(e)
	{
		$("#resetpop").hide();
	});
	$('#reset_ok').click(function(e)   //reset to default
	{
		var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
		$("#resetpop").hide();
		$.get(cgiurl, {type:"reset_default"}, function()
		{	
		}, "text");
	});

	$('#reboot_init').click(function(e)   //reboot System
	{
		$("#reboot1").show();
		$("#reboot2").hide();
		$("#rebootpop").show();
	});	
	
	$('#reboot_ok').click(function(e)   //reboot
	{
		$.get("cgi-bin/wifi_info_GET.cgi", {type:"reboot"}, function()
		{	
		}, "text");
		$("#reboot1").hide();
		$("#reboot2").show();

	});
	
	$('#reboot_refresh,#reboot_cancel').click(function(e)   //reboot
	{
		$("#rebootpop").hide();
	});

}

//any button click
function click_listener() {

    clearInterval(delay_get_data_id);
    delay_get_data_id = -1;
    Set_pws_listener();
    Set_resolution_listener();
    Upgrade_listener();
    Upgrade_OK_listener();
    ezairmode_listener();
    reset_reboot_listener();
}

function ezFuiWifiReset()
{
   var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'ezFuiWifiReset'},function(str) {
  
    },"text");
}
function Get_CONFIG_3TO1()
{
   var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'CONFIG_3TO1'},function(str) {
		console.log("Get_CONFIG_3TO1: "+str);
		CONFIG_3TO1_value=str;
  
    },"text");
}
function delay_get_data_function() {
    if (delay_get_data_id > 0) {
        clearInterval(delay_get_data_id);
        delay_get_data_id = -1;
    }
    if(CONFIG_3TO1_value==2){
        $("#li_defaultmode").show();
        Get_defaultmode_UI();
        default_mode_listener();
    }else{
	$("#li_defaultmode").remove();
    }
    Get_multilanguage();
    Get_resolution();
    Get_password();
    Get_local_version();
    Get_AP_mac_ip();
    Get_multi_language();//Get multi languange string
    Get_reboot_reset_language();
    Get_connected_ssid();
    Get_ezairmode_ui();

    delay_get_data_id = self.setInterval(click_listener, 1000);

}

//first run
function delay_get_data() {
     ezFuiWifiReset();
     Get_CONFIG_3TO1();
    if (delay_get_data_id > 0) {
        clearInterval(delay_get_data_id);
        delay_get_data_id = -1;
    }
    delay_get_data_id = self.setInterval(delay_get_data_function, 100); 
}
