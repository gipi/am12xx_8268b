var text_indeed;
var start;
var stop;
var text_string;
var text_string_global = "";
var text_string_length;
var language_index = -1;
var music_output_index = -1;
var defaultmode_index = -1;
var save_defaultmode_index;
var connection_mode_index = -1;
var current_connection_mode_index = -1;
var show_waiting_tick_id = -1;
var show_waiting_tick = 11;
var appskValue;
var appskValue_save;
var ignore_ssid_index;
var device_name;
var device_name_save;
var device_name_warn,device_name_warn1,device_name_warn2,device_name_warn3;
var warn_msg;
var default_mode_str;
var defaultmode_global_text;
var function_index;
var show_warn_dialog_tick = 10;
var show_warn_dialog_id = -1;
var current_language_index;
var waiting_set_language_id = -1;
var Set_internet_icon_txt_tick_id = -1;
var waiting_set_language_tick = 1;
var internet_icon_txt_tick = 10;
var check_OTA_first_flag = -1;
var Upgrade_listener_flag = -1;
var client_connect_show_flag = -1;
var connected_ssid_flag = 0;
var show_connections_num_id = -1;
var get_connected_ssid_id = -1;
var screen_width, screen_height, sWidth, sHeight;
var ProboxConnectStatus;
var appsk_input_flag = -1;
var upgrade_msg;
var upgrade_connect_fail_OK_CANCEL_flag = -1;
var current_music_output_index = -1;
var current_eq_mode_index = -1;
var auto_language_flag = -1;
var Check_new_version_boot_id = -1;
var get_current_language_id = -1;
var get_ota_check_status_tick = 0;
var get_new_version_timer_id = -1;
var delay_get_data_id = -1;
var AUDIO_EN;
var SPDIF_EN = 0;
var I2S_EN = 0;
var get_ota_status_id = -1;
var get_multilanguage_id = -1;
var upgrade_dialog_msg_string;
var upgrade_warn_string;
var PSW_LEN_string;
var PSW_ALLOW_string;
var LIST_WARN2_string;
var SET_AP_SSID_WARN_CHAR_string;
var SET_AP_SSID_WARN_LEN_string;
var LIST_WARN3_string="Please enter a new password to save again";
var changes_effect_string="Changes take effect after reboot.";
var waiting_string,password_string;
var Language_text_string;
var wifi_mode_status;
var wifi_connect_flag;
var WiFi_IP_Setting_string="WiFi IP Setting";
var router_string, direct_string, onlyrouter_string;
var router_warn_string, router_warn_string2, onlyrouter_w_arn_string, onlyrouter_w_arn_string2;
var direct_warn_string;
var ezchannel_enable;
var ezchannel_onoff = "off";
var ezchannel_volume = 50;
var volume_val;
var waitime_value;
var ezchannel_waittime = "30";
var playlist_val, ssid_val;
var ezchannel_playlist, ezchannel_ssid;
var multiLanguage_array = new Array(29);
var music_output_array = new Array();
//var defaultmode_array=["Audio","Radio"];
var Source_i;
var Source_Selection = new Array();
var Source_Selection = new Array("Club", "Dance", "Flat", "Classical", "Laptop speakers/headphones", "Large hall", "Party", "Pop", "Reggae", "Rock", "Soft", "Ska", "Full Bass", "Soft Rock", "Full Treble", "Full Bass & Treble", "Live", "Techno", "Custom");
var Source_max = Source_Selection.length - 1;
var flag_lan = 1; //for lan list
var resolution_array = new Array();
//var resolution_array = ["1280x720_60P", "1920x1080_24P", "1920x1080_30P"];
var resolution_array = ["1024x768_60P","1280x720_60P", "1280x800_60P","1920x1080_24P", "1920x1080_30P", "1920x1080_60P"];
var resolution_list_value=new Array (3,0,4,1,2,5);
var resolution_index = 0;
var channel_index = 0;
var channel_5G_index = 0;
var get_channel_5G_country_index = 0;
var get_channel_5G_init = 0;
var get_Bandwidth_5G_init=0;
var wifi_mode_flag = 0;
var current_resolution_index = -1;
var current_wlan_ip_value = -1;
var miracast_probox_mode=0;
var resolution_current_value;
var resolution_current_value_num;
var defaultmode_array = new Array();
var defaultmode_array = ["EZCast Pro", "EZMirror+Timer", "EZMirror+AP"];
var upgrade_bar = 0;
var ezmusic_flag = -1;
var custom_index_id = new Array("eq70", "eq180", "eq320", "eq600", "eq1K", "eq3K", "eq6K", "eq12K", "eq14K", "eq16K");
var custom_name = new Array("70Hz", "180Hz", "320Hz", "600Hz", "1KHz", "3KHz", "6KHz", "12KHz", "14KHz", "16KHz");
var Channel_text,Country_text,Channel_warn,Bandwidth_text;		
var Channel_5G_country = new Array(25);
var Bandwidth = new Array("40MHz", "20MHz");
var country1 = new Array ("Auto",36,40,44,48);//South Africa 以色列 中东(Turkey/Egypt/Tunisia/Kuwait)   中东(United Arab Emirates)  俄罗斯
var country2 = new Array ("Auto",36,40,44,48,149,153,157,161,165);//Australia, Canada  美国印度马来西亚 墨西哥  中东(Saudi Arabia)    新加坡    南美洲  
var country4 = new Array ("Auto",149,153,157,161,165);//中国 中东(Iran/Labanon/Qatar)
var country5 = new Array ("Auto",36,40,44,48,52,56,60,100,104,108,112,116,132,136,140);//Europe 
//var country8 = new Array ("Auto",36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140);//日本
var country8 = new Array ("Auto",36,40,44,48);//日本
var country9 = new Array ("Auto",36,40,44,48,149,153,157,161);//韩国
var country19=new Array ("Auto",56,60,64,149,153,157,161,165);//台湾
var Channel_selectValue = 0;
var probox_flag = 0;
var EQ_Value=[[50,50,41,34,34,34,41,50,50,50],
[25,31,44,50,50,66,69,69,50,50],
[50,50,50,50,50,50,50,50,50,50],
[50,50,50,50,50,50,69,69,69,76],
[38,22,36,60,57,46,38,25,17,12],
[23,23,34,34,50,63,63,63,50,50], 
[31,31,50,50,50,50,50,50,31,31], 
[55,38,31,30,36,53,57,57,55,55], 
[50,50,52,66,50,33,33,50,50,50], 
[30,38,65,71,60,39,26,22,22,22], 
[38,46,53,57,53,39,28,25,22,19], 
[57,63,61,52,39,34,26,25,22,25], 
[25,25,25,34,46,61,73,77,79,79], 
[39,39,44,52,61,65,60,52,42,26], 
[76,76,76,61,42,22,9,9,9,6], 
[31,34,50,69,63,46,28,22,19,19], 
[63,50,39,36,34,34,39,42,42,44], 
[30,34,50,65,63,50,30,25,25,26], 
[50,50,50,50,50,50,50,50,50,50]];
var showinfo_flag = 0;
var ok_string, cancle_string, upgrad_string;
var ezairmode_index = 0,ezairmode_select = 0,select_txt,ezarimode_txt="EZAir mode";
var softapconnect_select=0,softapconnect_index=0,softapconnect_select_txt;
var MAXUSER_WARN_string = "Note: The more users are linking, the lower bandwidth they may have.";
var ezwire_audio_index = 0,ezwire_audio_select = 0;
var plat_flag=0;
var Autochannel_24G_val,Autochannel_5G_val;
var advance_flag=0;
var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();

// creat object
var totalData=new Object();
//init data
totalData.wifimode="Auto";
totalData.lanset="Auto";
totalData.language=0;
totalData.devcename="abc";
totalData.defaultmode=0;
totalData.resolution=0;
totalData.connection=0;
totalData.ssid="abc";
totalData.password="abc";

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
function clear_timer_id(timer_id) {
    if (timer_id > 0) {
        clearInterval(timer_id);
        timer_id = -1;
    }
}
function show_setting_dialog() {
    var scroll_left = document.body.scrollLeft;
    var scroll_top = document.body.scrollTop;
    document.getElementById("light").style.display = "block";
    document.getElementById("light").style.position = "fixed";
    document.getElementById("light").style.left = (screen_width - 344) / 2 + "px";
    document.getElementById("light").style.top = (screen_height - 236) / 2 + "px";
    document.getElementById("fade").style.display = "block";
    //document.getElementById("fade").style.height=sHeight+"px";
}
function hide_setting_dialog() {
    document.getElementById("light").style.display = "none";
    document.getElementById("fade").style.display = "none";
}
function hide_upgrade_dialog() {
    document.getElementById("upgrade_light").style.display = "none";
    document.getElementById("upgrade_light").style.height = "100%";
    document.getElementById("fade").style.display = "none";
    $('#server_text').hide();
    $('#local_version').hide();
    $('#local_text').hide();
    $('#server_version').hide();
    $('#upgrade_OK').hide();
    $('#upgrade_CANCEL').hide();
    $('#upgrade_tick').hide();
    $('#upgrade_time_tick_msg').hide();
}
function show_upgrade_waiting_dialog() {
    $('#upgrade_dialog_msg').text(waiting_string);
    $('#ota_fail').popup('open');
}
function hide_upgrade_waiting_dialog() {
    $('#ota_fail').popup('close');
}
function show_upgrade_dialog() {
    var scroll_left = document.body.scrollLeft;
    var scroll_top = document.body.scrollTop;
    $('#server_text').show();
    $('#local_version').show();
    $('#local_text').show();
    $('#server_version').show();
    $('#upgrade_OK').show();
    $('#upgrade_CANCEL').show();
    $('#upgrade_dialog_msg').show();
    $('#upgrade_warn').show();
    document.getElementById("upgrade_light").style.display = "block";
    document.getElementById("upgrade_light").style.position = "fixed";
    document.getElementById("upgrade_light").style.left = (screen_width - 344) / 2 + "px";
    document.getElementById("upgrade_light").style.top = (screen_height - 236) / 2 + "px";
    document.getElementById("fade").style.display = "block";
}
function hide_OTA_warn_dialog() {
    document.getElementById("upgrade_light").style.display = "none";
    document.getElementById("fade").style.display = "none";
    $('#upgrade_warn').hide();
}
function reload_page_network() {
        window.location.href = "networksetup.html?changepassword=1";
}
function reload_page() {
    if(advance_flag)
        window.location.href = "websetting.html?advanced=1";
    else
        window.location.href = "websetting.html";
}
function lan_button() //language setting button 
{
    $("#lan_ok").click(function() {
        language_index = $('input:radio[name="lanlist"]:checked').val();
        if (current_language_index != language_index) {
            $('#lantxt').text(language_index);
            $('#lan_index').val(language_index);
            $('#language').text(multiLanguage_array[language_index]);
            $.post('cgi-bin/set_lan_POST.cgi', $('#config-form').serialize());
            setTimeout(reload_page, 1000);
        }
    });

    $('input:radio[name="lanlist"]').click(function() {
        var that = $(this);
        var b = $(this).prev().text();
        $("#lantxt").text(b);
    });

    $('#lanselt').click(function() {
        $('input:radio[name="lanlist"]').each(function() {
            $(this).prev().addClass("ui-radio-off");
            $(this).prev().removeClass("ui-radio-on");
            $(this).attr("checked", " ");
        });
        var index = current_language_index;
        $("label[for=lan_" + index + "]").removeClass("ui-radio-off");
        $("label[for=lan_" + index + "]").addClass("ui-radio-on");
        set_langradio();
    });
}
//language select css set
function set_langradio() {
    $("input[type=radio][name=lanlist][value=" + current_language_index + "]").attr("checked", "true");
}
//use js create language list
function Creat_lanlist() {
    flag_lan = 0;
    $("#bbb").text(current_language_index);
    var i = 0;
    var bb = $("#bbb").text();
    $("#bbb").text(bb + "-|-" + multiLanguage_array.length);
    for (i = 0; i < multiLanguage_array.length; i++) {
        var index = i;
        var id = "lan_" + i;
        var val = index.toString();
        var multi_string = multiLanguage_array[i];
        var innerhtml = "<div class=\"ui-radio\"><label for='" + id + "'  class=\"ui-btn ui-corner-all ui-btn-inherit ui-btn-icon-right ui-radio-off\" >" + multi_string + "</label><input type=\"radio\" name=\"lanlist\" id='" + id + "' value='" + val + "' ></div>";
        $("#lang").append(innerhtml);
    }
    set_langradio();
    lan_button();
}
function Get_current_language() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    if (get_current_language_id > 0) {
        clearInterval(get_current_language_id);
        get_current_language_id = -1;
    }
    $.get(cgiurl, {type: 'get_lan_index'}, function(get_lan_index) {
        var ret = get_lan_index.indexOf("auto");
        if (ret >= 0) {
            //$('#language').text("" + multiLanguage_array[multiLanguage_array.length - 1]);
            var lan_index = get_lan_index.substring(0, get_lan_index.length - 4);
            current_language_index = multiLanguage_array.length - 1;
            auto_language_flag = 1;
        } else {
            //$('#language').text("" + multiLanguage_array[get_lan_index]);
            current_language_index = Number(get_lan_index);
            auto_language_flag = 0;
        }
        language_index = current_language_index;
        $('#lantxt').text(language_index);
        Creat_lanlist();
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
            get_current_language_id = self.setInterval(Get_current_language, 300);
        }
    },"text");
}
//start read multi language
function Get_multilanguage_timer() {
    if (get_multilanguage_id > 0) {
        clearInterval(get_multilanguage_id);
        get_multilanguage_id = -1;
    }
    get_multilanguage_id = self.setInterval(Get_multilanguage, 700);
    $('#language').text(totalData.language);
    self.setTimeout(show_ui,1500);   //delay show ui

}
function select_eq_mode_down() {
    if(Source_i==0)
        Source_i=Source_max;
    else
        Source_i--;
    $('#EQ_default_mode').val(Source_Selection[Source_i]);
    document.getElementById("EQ_default_mode").style.backgroundImage = 'url(img/bar.png)';
    document.getElementById("select_down_eq").src = "img/select-Down.png";
    document.getElementById("select_up_eq").src = "img/select-up.png";

    document.getElementById("language").style.backgroundImage = 'url(img/bar_cover.png)';
    document.getElementById("select_down_language").src = "img/select-Down_cover.png";
    document.getElementById("select_up_language").src = "img/select-up_cover.png";

    document.getElementById("defaultmode").style.backgroundImage = 'url(img/bar_cover.png)';
    document.getElementById("select_down_defaultmode").src = "img/select-Down_cover.png";
    document.getElementById("select_up_defaultmode").src = "img/select-up_cover.png";

    document.getElementById("resolution").style.backgroundImage = 'url(img/bar_cover.png)';
    document.getElementById("select_down_res").src = "img/select-Down_cover.png";
    document.getElementById("select_up_res").src = "img/select-up_cover.png";
}
function select_eq_mode_up() {
    if(Source_i==Source_max)
        Source_i=0;
    else 
        Source_i++;
    $('#EQ_default_mode').val(Source_Selection[Source_i]);
    document.getElementById("EQ_default_mode").style.backgroundImage = 'url(img/bar.png)';
    document.getElementById("select_down_eq").src = "img/select-Down.png";
    document.getElementById("select_up_eq").src = "img/select-up.png";

    document.getElementById("language").style.backgroundImage = 'url(img/bar_cover.png)';
    document.getElementById("select_down_language").src = "img/select-Down_cover.png";
    document.getElementById("select_up_language").src = "img/select-up_cover.png";

    document.getElementById("defaultmode").style.backgroundImage = 'url(img/bar_cover.png)';
    document.getElementById("select_down_defaultmode").src = "img/select-Down_cover.png";
    document.getElementById("select_up_defaultmode").src = "img/select-up_cover.png";

    document.getElementById("resolution").style.backgroundImage = 'url(img/bar_cover.png)';
    document.getElementById("select_down_res").src = "img/select-Down_cover.png";
    document.getElementById("select_up_res").src = "img/select-up_cover.png";

}
function select_language_down() {
    if(language_index<=0)
        language_index=multiLanguage_array.length;
    language_index = (language_index - 1) % multiLanguage_array.length;
    $('#lan_index').val("" + language_index);
    $('#language').val("" + multiLanguage_array[language_index]);
    document.getElementById("resolution").style.backgroundImage = 'url(img/bar_cover.png)';
    document.getElementById("select_down_res").src = "img/select-Down_cover.png";
    document.getElementById("select_up_res").src = "img/select-up_cover.png";

    document.getElementById("language").style.backgroundImage = 'url(img/bar.png)';
    document.getElementById("select_down_language").src = "img/select-Down.png";
    document.getElementById("select_up_language").src = "img/select-up.png";

    document.getElementById("defaultmode").style.backgroundImage = 'url(img/bar_cover.png)';
    document.getElementById("select_down_defaultmode").src = "img/select-Down_cover.png";
    document.getElementById("select_up_defaultmode").src = "img/select-up_cover.png";

    document.getElementById("EQ_default_mode").style.backgroundImage = 'url(img/bar_cover.png)';
    document.getElementById("select_down_eq").src = "img/select-Down_cover.png";
    document.getElementById("select_up_eq").src = "img/select-up_cover.png";
}
function select_language_up() {
    language_index = language_index + 1;
    if (language_index > multiLanguage_array.length - 1) 
        language_index = 0;
    $('#lan_index').val("" + language_index);
    $('#language').val("" + multiLanguage_array[language_index]);
    document.getElementById("resolution").style.backgroundImage = 'url(img/bar_cover.png)';
    document.getElementById("select_down_res").src = "img/select-Down_cover.png";
    document.getElementById("select_up_res").src = "img/select-up_cover.png";

    document.getElementById("language").style.backgroundImage = 'url(img/bar.png)';
    document.getElementById("select_down_language").src = "img/select-Down.png";
    document.getElementById("select_up_language").src = "img/select-up.png";

    document.getElementById("defaultmode").style.backgroundImage = 'url(img/bar_cover.png)';
    document.getElementById("select_down_defaultmode").src = "img/select-Down_cover.png";
    document.getElementById("select_up_defaultmode").src = "img/select-up_cover.png";

    document.getElementById("EQ_default_mode").style.backgroundImage = 'url(img/bar_cover.png)';
    document.getElementById("select_down_eq").src = "img/select-Down_cover.png";
    document.getElementById("select_up_eq").src = "img/select-up_cover.png";
}
function select_music_output_up() {
    music_output_index = music_output_index + 1;
    if (music_output_index > music_output_array.length - 1)
        music_output_index = 0;
    switch (music_output_index) {
	    case 0:
	        $('#output_mode_index').val("0");
	        break;
	    case 1:
	        $('#output_mode_index').val("1");
	        break;
	    case 2:
	        $('#output_mode_index').val("2");
	        break;
	    case 3:
	        $('#output_mode_index').val("3");
	        break;
    }
    $('#resolution').val("" + music_output_array[music_output_index]);
    document.getElementById("resolution").style.backgroundImage = 'url(img/bar.png)';
    document.getElementById("select_down_res").src = "img/select-Down.png";
    document.getElementById("select_up_res").src = "img/select-up.png";

    document.getElementById("language").style.backgroundImage = 'url(img/bar_cover.png)';
    document.getElementById("select_down_language").src = "img/select-Down_cover.png";
    document.getElementById("select_up_language").src = "img/select-up_cover.png";

    document.getElementById("defaultmode").style.backgroundImage = 'url(img/bar_cover.png)';
    document.getElementById("select_down_defaultmode").src = "img/select-Down_cover.png";
    document.getElementById("select_up_defaultmode").src = "img/select-up_cover.png";

    document.getElementById("EQ_default_mode").style.backgroundImage = 'url(img/bar_cover.png)';
    document.getElementById("select_down_eq").src = "img/select-Down_cover.png";
    document.getElementById("select_up_eq").src = "img/select-up_cover.png";
}
function select_music_output_down() {
    music_output_index = music_output_index - 1;
    if (music_output_index < 0) 
        music_output_index = music_output_array.length - 1;
    switch (music_output_index) {
	    case 0:
	        $('#output_mode_index').val("0");
	        break;
	    case 1:
	        $('#output_mode_index').val("1");
	        break;
	    case 2:
	        $('#output_mode_index').val("2");
	        break;
	    case 3:
	        $('#output_mode_index').val("3");
	        break;
    }
    $('#resolution').val("" + music_output_array[music_output_index]);
    document.getElementById("resolution").style.backgroundImage = 'url(img/bar.png)';
    document.getElementById("select_down_res").src = "img/select-Down.png";
    document.getElementById("select_up_res").src = "img/select-up.png";

    document.getElementById("language").style.backgroundImage = 'url(img/bar_cover.png)';
    document.getElementById("select_down_language").src = "img/select-Down_cover.png";
    document.getElementById("select_up_language").src = "img/select-up_cover.png";

    document.getElementById("defaultmode").style.backgroundImage = 'url(img/bar_cover.png)';
    document.getElementById("select_down_defaultmode").src = "img/select-Down_cover.png";
    document.getElementById("select_up_defaultmode").src = "img/select-up_cover.png";

    document.getElementById("EQ_default_mode").style.backgroundImage = 'url(img/bar_cover.png)';
    document.getElementById("select_down_eq").src = "img/select-Down_cover.png";
    document.getElementById("select_up_eq").src = "img/select-up_cover.png";
}
function select_defaultmode() {
    document.getElementById("resolution").style.backgroundImage = 'url(img/bar_cover.png)';
    document.getElementById("select_down_res").src = "img/select-Down_cover.png";
    document.getElementById("select_up_res").src = "img/select-up_cover.png";

    document.getElementById("language").style.backgroundImage = 'url(img/bar_cover.png)';
    document.getElementById("select_down_language").src = "img/select-Down_cover.png";
    document.getElementById("select_up_language").src = "img/select-up_cover.png";

    document.getElementById("defaultmode").style.backgroundImage = 'url(img/bar.png)';
    document.getElementById("select_down_defaultmode").src = "img/select-Down.png";
    document.getElementById("select_up_defaultmode").src = "img/select-up.png";
}
function Get_last_UI() {
    $.get("cgi-bin/wifi_info_GET.cgi", {type: 'get_last_ui'},function(last_ui_index) {
        var ret = last_ui_index.indexOf("1");
        if (ret >= 0) {
            $('#defaultmode').text("Audio");
            defaultmode_index = 0;
            save_defaultmode_index = defaultmode_index;
        } else {
            ret = last_ui_index.indexOf("2");
            if (ret >= 0) {
                $('#defaultmode').val("Radio");
                defaultmode_index = 1;
                save_defaultmode_index = defaultmode_index;
            }
        }
    },"text");
}
//修改密码
function change_psw() {
    $.post('cgi-bin/password_POST.cgi', $('#appsk').serialize());
    setTimeout(reload_page_network, 1000);
}
//修改ssid状态
function change_psw_ignoressid() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    var hide_value=$("#ignoressid_val").val();
    var psk_value=$("#appsk").val();
    var str=hide_value+"\n"+psk_value+"\n"+"set_psk_ignoressid";
    $.get(cgiurl, {type: str},function(ret) {
         
    },"text");
     setTimeout(reload_page_network, 1000);
}
//退出 html 
function back_ezcastapp() {
    window.location = "ezcast://ezcast.app/back";
}
//设置password
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
//密码设置按键响应
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
    });
}

function change_ap_ssid() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    var ssid_value=$('#set_ap_ssid').val();
    var str=ssid_value+"set_softap_ssid";
    $.get(cgiurl, {type: str},function(ret) {

    },"text");
     setTimeout(reload_page_network, 1000);
}

function set_ap_ssid() {
    var errorFlag = 0;
    var regx = /^[A-Za-z0-9]+$/;
    var ssid_value=$('#set_ap_ssid').val();
 if (ssid_value != totalData.ssid)
    {
        if (!regx.test(ssid_value))
    {
            $('#ssid_warn_txt').text(SET_AP_SSID_WARN_CHAR_string);
            errorFlag = 1;
       }
    else
    {
            if (ssid_value.length < 1 || ssid_value.length > 32)
        {
                   $('#ssid_warn_txt').text(SET_AP_SSID_WARN_LEN_string);
                   $('#set_ap_ssid').text("" + totalData.ssid);
                   errorFlag = 1;
                }
         else
        {
                 errorFlag = 0;
               }
        }
        if (!errorFlag)
     {
            //$('#app_sk').text(appskValue);
            $('#psk_warn_txt').text("SSID modified successfully!");
            change_ap_ssid();
        }
    }
    else
    {
        $('#ssid_warn_txt').text(LIST_WARN3_string);  //"Please enter a new password to save again");
    }
}

function Set_ssid_listener() {
    $("#set_ap_ssid_ok").click(function() {
        set_ap_ssid();
    });

    $("#set_ap_ssid").keyup(function(){
        if(event.keyCode==13){
            set_appsk();
        }
    });
    $("#softap_set_ssid").click(function() {
        $('#set_ap_ssid').val($('#ap_ssid').text());
    });
}

//读取小机密码
function Get_password() {
    $('#app_sk').text(totalData.password);
    $('#ap_psk').text(totalData.password);
    appskValue_save = totalData.password;
    /*
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_softap_psk'},function(appsk) {
        $('#app_sk').text(appsk);
        $('#ap_psk').text(appsk);
        appskValue_save = $('#app_sk').text();
    },"text");
    */
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
function Get_text_from_textstring_eq(text_string) {
    var text_string_tmp = text_string.substring(text_start, text_string_length);
    text_stop = text_string_tmp.indexOf("\n");
    text_start = 0;
    var text_temp = text_string_tmp.substring(text_start, text_stop);
    var start_indeed = text_temp.indexOf(":") + 1;
    text_indeed = text_temp.substring(start_indeed, text_stop);
    text_start = text_stop + 1;
    text_string_global = text_string_tmp;
    return text_indeed;
}
//读取多国语 for ezmusic
function Get_index_language_ezmusic() {
    $.get("cgi-bin/wifi_info_GET.cgi", {type: 'get_index_text'},function(index_text_string) {
        if (index_text_string.length) {
            start = 0;
            text_string_length = 0;
            text_string = index_text_string;
            text_string_length = index_text_string.length;
            $('#resolution_txt').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#password_txt').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#internet_txt').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#language_txt').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#defaultmode_txt').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#upgrade_txt').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            var temp = Get_text_from_textstring(text_string);
            temp = temp + " :";
            text_string = text_string_global;
            var temp = Get_text_from_textstring(text_string);
            temp = temp + " :";
            text_string = text_string_global;
            $('#upgrade_OK').text(Get_text_from_textstring(text_string));
            $('#OK').val($('#upgrade_OK').val());
            text_string = text_string_global;
            $('#upgrade_CANCEL').text(Get_text_from_textstring(text_string));
            $('#CANCEL').text($('#upgrade_CANCEL').val());
        } else {
            $('#resolution_txt').text("Resolution");
            $('#password_txt').text("Password");
            $('#internet_txt').text("Internet");
            $('#language_txt').text("Language");
            $('#defaultmode_txt').text("Default Mode");
            $('#upgrade_txt').text("Upgrade");
            $('#ap_ssid_txt').text("SSID : ");
            $('#ap_psk_txt').text("Password : ");
            $('#upgrade_OK').text("OK");
            $('#upgrade_CANCEL').text("CANCEL");
            $('#OK').text("OK");
            $('#CANCEL').text("CANCEL");
        }
    },"text");
}
//读取多国语 ,期它平台
function Get_index_language() {
    var tmp_string="";
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_index_text'},function(index_text_string) {
        if (index_text_string.length) {
            start = 0;
            text_string_length = 0;
            text_string = index_text_string;
            text_string_length = index_text_string.length;
            $('#resolution_txt').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            password_string =	Get_text_from_textstring(text_string);//Password
            $('#password_txt').text("WIFI "+password_string);
            $('#le_password').text(password_string+":");//Password
            text_string = text_string_global;
            $('#internet_txt').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#language_txt').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#defaultmode_txt').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#upgrade_txt').text(Get_text_from_textstring(text_string));//升级
            text_string = text_string_global;
            LIST_WARN3_string = Get_text_from_textstring(text_string);//Please enter a new password to save again
            text_string = text_string_global;
            ok_string = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            cancle_string = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            $('#connection_mode_txt').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            router_string = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            direct_string = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            onlyrouter_string = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            $('#music_output_txt').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#adminpas_txt').text(Get_text_from_textstring(text_string));//Admin Password 
            text_string = text_string_global;
            $('#Enterprise_txt').text(Get_text_from_textstring(text_string));//WiFi Enterprise
            text_string = text_string_global;
            $('#hidessidmode_title').text(Get_text_from_textstring(text_string));//Hide SSID
            $('#ignoressid_txt').text($('#hidessidmode_title').text());
            text_string = text_string_global;
            changes_effect_string = Get_text_from_textstring(text_string);//Changes take effect after reboot.
            $('#hide_psk_warn_txt').text(changes_effect_string);
            $('#castcode_warn_txt').text(changes_effect_string);
            $('#miracode_warn_txt').text(changes_effect_string);
            $('#ezairmode_warn_txt').text(changes_effect_string);
            text_string = text_string_global;
            $('#con_dialog_msg3').text(Get_text_from_textstring(text_string));//New setting will take effect after reboot!
            text_string = text_string_global;
            ezarimode_txt = Get_text_from_textstring(text_string);

            text_string=text_string_global;	
            $('label[for=mode_0]').text(Get_text_from_textstring(text_string)); //Mirror only
            text_string=text_string_global;	
            $('label[for=mode_1]').text(Get_text_from_textstring(text_string)); //Mirror+Streaming
            $("#mode_0_txt").append("<span class='warn_text'>(Recommended)</span>");
            $("#mode_1_txt").append("<span class='warn_text'>(for iOS11.0/11.1)</span>");
		/*	
            text_string = text_string_global;
            $('#mode_0_txt').text(Get_text_from_textstring(text_string));//Mirror only
            text_string = text_string_global;
            $('#mode_1_txt').text(Get_text_from_textstring(text_string));//Video Streaming
            */
            text_string = text_string_global;
            $('#Autoplay_tit').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#autoplya_txt').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#hdmi_audio_txt').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#hdmi_warn_txt').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#airvew_time_txt').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            tmp_string = Get_text_from_textstring(text_string);//seconds
            $("#rate_val option").eq(0).text("5 "+tmp_string);//5 seconds
            $("#rate_val option").eq(1).text("15 "+tmp_string);//15 seconds
            $("#rate_val option").eq(2).text("30 "+tmp_string);//30 seconds
            text_string = text_string_global;
            $("#rate_val option").eq(3).text("1 "+Get_text_from_textstring(text_string));// 1 minute
            text_string = text_string_global;
            $("#rate_val option").eq(4).text(Get_text_from_textstring(text_string));// Manual update
	
            text_string = text_string_global;
            $('#txt_miracode').text(Get_text_from_textstring(text_string));
            $('#miracode_text').text($('#txt_miracode').text());
            text_string = text_string_global;
            $('#softapconnect_title').text(Get_text_from_textstring(text_string));
            text_string=text_string_global;
	     tmp_string = Get_text_from_textstring(text_string); //user
            $('label[for=mun_0]').text("16 "+tmp_string);
            $('label[for=mun_1]').text("32 "+tmp_string);
            $('label[for=mun_2]').text("64 "+tmp_string);
            text_string=text_string_global;
	     MAXUSER_WARN_string = Get_text_from_textstring(text_string); 
			
            text_string = text_string_global;
            $('#info_string').text(Get_text_from_textstring(text_string)); //info
            text_string = text_string_global;
			
            Get_router_warn_txt();
            Get_router_ctl();
            updata_button_mutilanguage();
            $("#rate_val-button").children("span").text($("#rate_val").find("option[value="+airview_refershTime+"]").text());	
 			
        } else {
            $('#resolution_txt').text("Resolution");
            $('#password_txt').text("WIFI Password");
            $('#internet_txt').text("Internet");
            $('#language_txt').text("Language");
            $('#defaultmode_txt').text("Default Mode");
            $('#upgrade_txt').text("Upgrade");
            $('#ap_ssid_txt').text("SSID : ");
            $('#ap_psk_txt').text("Password : ");
            $('#adminpas_txt').text("Admin Password");//Admin Password 
            $('#upgrade_CANCEL').val("CANCEL");
            $('#OK').val("OK");
            $('#CANCEL').val("CANCEL");
            $('#connection_mode_txt').text("Connection");
            router_string = "Via Router Allowed";
            direct_string = "Direct Link Only";
            onlyrouter_string = "Via Router Only";
            $('#info_string').text("info");
            $('#music_output_txt').text("Music Output"); 
            router_warn_string = "        Due to the streaming is from your phone through router to TV, throughput is relatively small and playback may be choppy. ";
            router_warn_string2 = "      Do you want to allow the connection to \"Through Router Allow\" mode?";
            direct_warn_string = "    Direct Link provides better local multimedia streaming throughput!";
            onlyrouter_w_arn_string = "        Due to the streaming is from your phone through router to TV, throughput is relatively small and playback may be choppy.";
            onlyrouter_w_arn_string2 = "      Do you want to allow the connection to \"Through Router Only\" mode?";
            Get_router_ctl();
        }
		
    },"text");
}
//读多国语

function updata_button_mutilanguage(){

        $('#OK').val(ok_string);
        $('#CANCEL').val(cancle_string);
        $('#music_output_ok').text(ok_string);
        $('#music_output_cancel').text(cancle_string);
        $('#set_appsk_ok').text(ok_string);
        $('#set_appsk_cel').text(cancle_string);
        $('#ezchannel_ok').text(ok_string);
        $('#ezchannel_cel').text(cancle_string);
        $('#upgrade_OK').text(ok_string);
        $('#upgrade_CANCEL').text(cancle_string);
        $('#ota_awrn_ok').text(ok_string);
        $('#defmode_ok').text(ok_string);
        $('#defmode_cel').text(cancle_string);
        $('#set_resolution_ok').text(ok_string);
        $('#set_resolution_cal').text(cancle_string);
        $('#resolution_ent').text(ok_string);
        $('#resolution_cancle').text(cancle_string);
        $('#set_connection_ok').text(ok_string);
        $('#set_connection_cal').text(cancle_string);
        $('#connection_ent').text(ok_string);
        $('#refersh').text(ok_string);
        $('#connection_cancle').text(cancle_string);
        $('#eq_ok').text(ok_string);
        $('#eq_cancel').text(cancle_string);
        $('#custom_home').text(ok_string);
        $('#lan_ok').text(ok_string);
        $('#lan_cancel').text(cancle_string);
        $('#lan_ent').text(ok_string);
        $('#lan_set_cancel').text(cancle_string);
        $('#wifi_ent').text(ok_string);
        $('#wifi_set_cancel').text(cancle_string);
        $('#sel_channel_ok').text(ok_string);
        $('#sel_channel_cal').text(cancle_string);
        $('#set_channel_warn').text(ok_string);
        $('#set_channel_cal').text(cancle_string);
        $('#set_channel_ok').text(ok_string);
        $('#warn_24G_cancle').text(cancle_string);
        $('#set_5Gchannel_warn').text(ok_string);
        $('#set_5Gchannel_cal').text(cancle_string);
        $('#set_5Gchannel_ok').text(ok_string);
        $('#warn_5G_cancle').text(cancle_string);
        $('#ezairmode_ok').text(ok_string);
        $('#ezairmode_cel').text(cancle_string);
        $('#ezwire_audio_ok').text(ok_string);
        $('#ezwire_audio_cel').text(cancle_string);
        $('#set_autoplay_ok').text(ok_string);
        $('#set_autoplay_cel').text(cancle_string);
        $('#set_hdmicece_ok').text(ok_string);
        $('#set_hdmicece_cel').text(cancle_string);
        $('#ignoressid_ok').text(ok_string);
        $('#ignoressid_cel').text(cancle_string);
        $('#set_appsk_ok').text(ok_string);
        $('#set_appsk_cel').text(cancle_string);
        $('#wifipassword_hide_ok').text(ok_string);
        $('#wifipassword_hide_cel').text(cancle_string);
        $('#ezairmode_ok').text(ok_string);
        $('#ezairmode_cel').text(cancle_string);
        $('#set_airview_ok').text(ok_string);
        $('#set_airview_cel').text(cancle_string);
        $('#set_miracode_ok').text(ok_string);
        $('#set_miracode_cel').text(cancle_string);
        $('#softapconnect_ok').text(ok_string);
        $('#softapconnect_cel').text(cancle_string);
		

}

function Get_router_warn_txt() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'gtroutwarntxt'},function(index_text_string) {
        if (index_text_string.length) {
            //var JSONObject = eval("(" + index_text_string + ")");
            var JSONObject = JSON.parse(index_text_string); 
            direct_warn_string = JSONObject.router_warn1;
            router_warn_string = JSONObject.router_warn2;
            onlyrouter_w_arn_string = JSONObject.router_warn3;

            var split_string_array = new Array();
            split_string_array = router_warn_string.split("\n");
            router_warn_string = split_string_array[0];
            router_warn_string2 = split_string_array[1];
            split_string_array = onlyrouter_w_arn_string.split("\n");
            onlyrouter_w_arn_string = split_string_array[0];
            onlyrouter_w_arn_string2 = split_string_array[1];
        }
    },"text");
}
//devicename check 
function devicename_val_check() {
    //var regx = /^[\x00-\x7F]+$/;
    //var regx = /^[\x00-\x7F\#\-\@\_\s]+$/; 
    var regx = /^[A-Za-z0-9\#\-\@\_\s]+$/; 
    device_name = $('#devicename').val();
    if (device_name != device_name_save) {
        if (regx.test(device_name)) {
            if (device_name.length <= 17) {
                $('#devicename').val(device_name);
                $('#device_name').text(device_name);
                device_name_save = device_name;
                $('#devicename_warn_ok').text(device_name_warn);
                return 1;
            } else {
                $('#devicename_warn1').css("color","red");
                $('#devicename_warn1').text(device_name_warn1);
                return 0;
            }
        } else {
            $('#devicename_warn1').css("color","red");
            $('#devicename_warn1').text(device_name_warn2);
            return 0;
        }
    } else {
        $('#devicename_warn1').css("color","red");
        $('#devicename_warn1').text(device_name_warn3);
        return 0;
    }
}
//devicename  set error warn 
function devicename_warn() {
    $('#Devicename_warn').popup('open');
    setTimeout(function() {
        $('#Devicename_warn').popup('close');
    },
    3000);
}
//devicename  set button
function Set_devicename_listener() {
    $("#devname_save").click(function() {
        if (devicename_val_check()) {
            setTimeout(devicename_warn, 500);
            $('#Devicenameset').popup('close');
            $("#deviceimg").attr("src", "img/devicename_noactive.png");
            $.post('cgi-bin/set_devicename.cgi', $('#devicename').serialize());
        }
    });

    $("#devname_Reboot").click(function() {
        if (devicename_val_check()) {
            $('#Devicenameset').popup('close');
            setTimeout(devicename_warn, 500);
            $("#deviceimg").attr("src", "img/devicename_active.png");
            $.post('cgi-bin/set_devicename_reboot.cgi', $('#devicename').serialize());
        }
    });

    $("#device_sel").click(function() {
        $('#devicename').val($('#device_name').text());
        $('#devicename_warn1').text("");
    });

}
//read devicename
function Get_devicename() {
/*
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_device_name'},function(devicename) {
        $('#device_name').text(devicename);
        device_name_save = $('#device_name').text();
        $('#devicename').val(device_name_save);
        device_name = device_name_save;
    },"text");
*/
    $('#device_name').text(totalData.devcename);
    device_name_save = $('#device_name').text();
    $('#devicename').val(device_name_save);
    device_name = device_name_save;
}
//save devicename
function change_devicename(val) {
    document.getElementById("devicename_change").style.display = "none";
    $("#OK6").show();
    $("#OK5").show();
    $("#CANCEL4").show();

    if (val) {
        switch (val) {
            case 1:
                document.getElementById("devicename_active").style.backgroundImage = 'url(img/devicename_noactive.png)';
                $.post('cgi-bin/set_devicename.cgi', $('#config-form').serialize());
                break;
            case 2:
                document.getElementById("devicename_active").style.backgroundImage = 'url(img/devicename_active.png)';
                $.post('cgi-bin/set_devicename_reboot.cgi', $('#config-form').serialize());
                break;
        }

    } else {
        $('#device_name').val(device_name_save);
    }
}
function show_devcicename_change_warn() {
    var scroll_left = document.body.scrollLeft;
    var scroll_top = document.body.scrollTop;
    document.getElementById("devicename_change").style.display = "block";
    document.getElementById("devicename_change").style.position = "fixed";
    document.getElementById("devicename_change").style.left = (screen_width - 344) / 2 + "px";
    document.getElementById("devicename_change").style.top = (screen_height - 236) / 2 + "px";
    document.getElementById("fade").style.display = "block";
    $('#OK6').show();
    $('#OK5').show();
    $('#CANCLE4').show();
    $("#devicename_warn_ok").show();
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
            $('#upg_txt_4').text(defaultmode_index);

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

    });

    $(".ui-popup-screen").click(function() {
        $(document.body).css("overflow", "visible");
    });

    $("#li_defaultmode").click(function() {
        $('input:radio[name="de_item"]').each(function() {
            $(this).prev().addClass("ui-radio-off");
            $(this).prev().removeClass("ui-radio-on");
            $(this).attr("checked", " ");
        });
        set_defaultmode_radio();
        $(document.body).css("overflow", "hidden");
    });

}
//set default_mode css
function set_defaultmode_radio() {
    var that = $("input[type=radio][name=de_item][value=" + defaultmode_index + "]");
    that.attr("checked", "true");
    that.prev().removeClass("ui-radio-off");
    that.prev().addClass("ui-radio-on");

}
//read default_mode val
function Get_defaultmode_UI() {
/*
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_last_ui'},function(last_ui_index) {
        if (last_ui_index.indexOf("1") >= 0) {
            defaultmode_index = 1;//EZCast Pro
            save_defaultmode_index = defaultmode_index;
        } else if (last_ui_index.indexOf("2") >= 0) {
            defaultmode_index = 2;//EZMirror+Timer
            save_defaultmode_index = defaultmode_index;
        } else if (last_ui_index.indexOf("3") >= 0) {
            save_defaultmode_index = defaultmode_index = 3;//EZMirror+AP
        }
        $('#default_mode').text("" + defaultmode_array[defaultmode_index - 1]);
        $('#upg_txt_4').text(defaultmode_index);
        set_defaultmode_radio();
    },"text");
*/
        if (totalData.defaultmode.indexOf("1") >= 0) {
            defaultmode_index = 1;//EZCast Pro
            save_defaultmode_index = defaultmode_index;
        } else if (totalData.defaultmode.indexOf("2") >= 0) {
            defaultmode_index = 2;//EZMirror+Timer
            save_defaultmode_index = defaultmode_index;
        } else if (totalData.defaultmode.indexOf("3") >= 0) {
            save_defaultmode_index = defaultmode_index = 3;//EZMirror+AP
        }
        $('#default_mode').text("" + defaultmode_array[defaultmode_index - 1]);
        $('#upg_txt_4').text(defaultmode_index);
        set_defaultmode_radio();

}
//get defaultmode_language text
function Get_defaultmode_language() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_defaultmodetext'},function(Default_modetext) {
        if (Default_modetext.length) {
            defaultmode_global_text = "" + Default_modetext;
        } else {
            defaultmode_global_text = "Set boot default to";
        }
    },"text");
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
//get EQ text for ui
function Get_eq_mode_text() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_eq_text'},function(restext) {
        if (restext.length) {
            start = 0;
            text_string_length = 0;
            text_string = restext;
            text_string_length = restext.length;
            $('#dialog_msg').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#time_tick_msg').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
        } else {
            $('#dialog_msg').text("Use the new default eq mode ?");
            $('#time_tick_msg').text("sec left to return your setting last time.");
        }
    },"text");
}
//music_output button click
function resolution_listener() {
    $('#resolution').click(function(e) {
        Get_eq_mode_text();
        e.preventDefault();
        if (music_output_index != current_music_output_index) {
            $('#tick').html("12");
            Set_hdmimode();
            Get_eq_mode_text();
            $('#FORGET').hide();
            $('#tick').show();
            $('#time_tick_msg').show();
            //$('#reboot_msg').show();
            show_setting_dialog();
            set_resolution_timetick();
            current_music_output_index = music_output_index;
            function_index = 1;
        }
    });
}
//EQ button click
function eq_default_mode_listener() {
    $('#EQ_default_mode').click(function(e) {
        Get_eq_mode_text();
        e.preventDefault();
        if (18 == Source_i) {//custom
            //location.href="eq_Vertical.html";
            if (screen_width >= 800) {
                location.href = "eq_Horizontal.html";
            } else {
                location.href = "eq_Vertical.html";
            }
        } else {
            if (Source_i != current_eq_mode_index) {
                $('#tick').html("12");
                Get_eq_mode_text();
                $('#FORGET').hide();
                $('#tick').show();
                $('#time_tick_msg').show();
                set_resolution_timetick();
                show_setting_dialog();
                function_index = 4;
            }
        }
    });
}
//get Language,devicename text
function Get_language_txt() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_lan_text'},function(lan_text) {
        if (lan_text.length) {
            start = 0;
            text_string_length = 0;
            text_string = lan_text;
            text_string_length = lan_text.length;
            Language_text_string = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            waiting_string = Get_text_from_textstring(text_string);
            text_string = text_string_global;;
            $('#devname_Reboot').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#devname_save').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#CANCEL4').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#devicename_txt').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            device_name_warn = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            device_name_warn1 = Get_text_from_textstring(text_string);//The device name can not be more than 17 characters
            text_string = text_string_global;
            device_name_warn2 = Get_text_from_textstring(text_string);//Only numbers and letters are allowed!
            text_string = text_string_global;
            device_name_warn3 = Get_text_from_textstring(text_string);//Please enter a new name
        } else {
            Language_text_string = "Use the new Language ?";
            waiting_string = "Please wait";
            $('#devname_Reboot').text("Save & Reboot");
            $('#devname_save').text("Save Only");
            $('#CANCEL4').text("Cancel");
            $('#devicename_txt').text("Device Name");
            text_string = text_string_global;
            device_name_warn = "New device name will be Changed after reboot!";
        }
    },"text");
}
//Language button click
function Language_listener() {
    $('#language').click(function(e) {
        e.preventDefault();
        if (current_language_index != language_index) {
            $('#FORGET').hide();
            $('#tick').hide();
            $('#time_tick_msg').hide();
            $('#reboot_msg').hide();
            $('#dialog_msg').text(Language_text_string);
            Get_language_txt();
            show_setting_dialog();
            function_index = 3;
        }
    });
}
function waiting_set_language_tick_function() {
    if (waiting_set_language_tick == 0) {
        if (waiting_set_language_id > 0) {
            clearInterval(waiting_set_language_id);
            waiting_set_language_id = -1;
            $('#OK').show();
            $('#CANCEL').show();
            waiting_set_language_tick = 1;
        }
        hide_setting_dialog();
        if (screen_width >= 800) {
            location.href = "websetting_Horizontal.html";
        } else {
            location.href = "websetting_Vertical.html";
        }
    } else {
        waiting_set_language_tick = waiting_set_language_tick - 1;
    }
}
function waiting_set_language() {
    if (waiting_set_language_id > 0) {
        clearInterval(waiting_set_language_id);
        waiting_set_language_id = -1;
    }
    waiting_set_language_id = self.setInterval(waiting_set_language_tick_function, 1000);
    $('#OK').hide();
    $('#CANCEL').hide();
    $('#dialog_msg').text(waiting_string);
}
function Set_internet_icon_txt_tick_function() {
    if (internet_icon_txt_tick == 0) {
        clearInterval(Set_internet_icon_txt_tick_id);
        Set_internet_icon_txt_tick_id = -1;
        internet_icon_txt_tick = 10;
        $('#internet_icon_txt').hide();
    }
    internet_icon_txt_tick = internet_icon_txt_tick - 1;
}
function Set_internet_icon_txt_tick() {
    if (connected_ssid_flag == 0) {
        connected_ssid_flag = 1;
        if (Set_internet_icon_txt_tick_id > 0) {
            clearInterval(Set_internet_icon_txt_tick_id);
            Set_internet_icon_txt_tick_id = -1;
        }
        Set_internet_icon_txt_tick_id = self.setInterval(Set_internet_icon_txt_tick_function, 1000);
    }
}
function Get_upgrade_language() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_OTA_text'},function(OTA_string) {
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
        $('#ota_fail').popup('close');
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

function delaydisp_popup() {
    $('#warn').hide();
    $('#warn_time').hide();
    $('#ota').popup('open');
}
function set_version_string(ota_upgrade_string) {
    if (ota_upgrade_string.length > 0) {
        //if ((check_OTA_first_flag == 1) || (Upgrade_listener_flag == 1)) {
        //}
        var warn_ret = ota_upgrade_string.indexOf("newest");
        if (warn_ret < 0) {
            $('#ota_fail').popup('close'); 
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
                $('#local_version').val(local_version_string);
                var server_version_string = ota_upgrade_string.substring(ret + 1, ota_upgrade_string.length);
                $('#server_version').val(server_version_string);
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
                $('#ota_fail').popup('close');
                $('#warn').hide();
                $('#warn_time').hide();

            }
        }
    }

}
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
function get_new_version_function() {
    $.get(cgiurl, {type: 'get_OTA_version'},function(ota_upgrade_string) {
        if (ota_upgrade_string.length) {
            set_version_string(ota_upgrade_string);
        }
    },"text");
}
//read ssid
function Get_AP_ssid() {
    /*
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_ap_ssid'},function(ap_ssid) {
        if (ap_ssid.length) {
            $('#ap_ssid').text(ap_ssid);
        }
    },"text");
    */
    $('#ap_ssid').text(totalData.ssid);
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
function hide_faild_connect_dialog() {
    document.getElementById("upgrade_light").style.display = "none";
    document.getElementById("fade").style.display = "none";
    $('#server_text').show();
    $('#local_version').show();
    $('#local_text').show();
    $('#server_version').show();
    $('#upgrade_OK').show();
    $('#upgrade_CANCEL').show();
}
function show_faild_connect_tick_function() {
    if (show_warn_dialog_tick < 1) 
    {
        $('#ota_fail').popup('close');
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
    $('#ota_fail').popup('open');
    $('#warn').hide();
    $('#warn_time').show();
    if (show_warn_dialog_id > 0) {
        clearInterval(show_warn_dialog_id);
        show_warn_dialog_id = -1;
    }
    show_warn_dialog_id = self.setInterval(show_faild_connect_tick_function, 1000);
    upgrade_connect_fail_OK_CANCEL_flag = 1;
}
function show_alert_ok() {
    var scroll_left = document.body.scrollLeft;
    var scroll_top = document.body.scrollTop;
    $('#server_text').hide();
    $('#local_version').hide();
    $('#local_text').hide();
    $('#server_version').hide();
    $('#upgrade_OK').hide();
    $('#upgrade_CANCEL').hide();
    $('#upgrade_tick').show();
    $('#upgrade_time_tick_msg').show();
    $('#upgrade_warn').hide();
    $('#upgrade_ent').hide();
    $('#upgrade_dialog_msg').text("Please wait for reboot.");
    $('#alert_ok').show();
    document.getElementById("upgrade_light").style.display = "block";
    document.getElementById("upgrade_light").style.position = "fixed";
    document.getElementById("upgrade_light").style.left = (screen_width - 344) / 2 + "px";
    document.getElementById("upgrade_light").style.top = (screen_height - 236) / 2 + "px";
    document.getElementById("fade").style.display = "block";
    clear_timer_id(show_warn_dialog_id);
    show_warn_dialog_tick = 0;
    show_warn_dialog_id = self.setInterval(function() {
        if (show_warn_dialog_tick < 30) {
            show_warn_dialog_tick++;
            $('#upgrade_tick').text(show_warn_dialog_tick);
        } else {
            clear_timer_id(show_warn_dialog_id);
            window.location = "ezcast://ezcast.app/back";
        }
    },
    1000);
}
function show_alert_wait() {
    var scroll_left = document.body.scrollLeft;
    var scroll_top = document.body.scrollTop;
    $('#server_text').hide();
    $('#local_version').hide();
    $('#local_text').hide();
    $('#server_version').hide();
    $('#upgrade_OK').hide();
    $('#upgrade_CANCEL').hide();
    $('#upgrade_tick').hide();
    $('#alert_ok').hide();
    $('#upgrade_time_tick_msg').hide();
    $('#upgrade_warn').hide();
    $('#upgrade_ent').hide();
    $('#upgrade_dialog_msg').text(upgrade_dialog_msg_string);
    $('#upgrade_warn').text(upgrade_warn_string);
    $('#upgrade_warn').show();
    $('#upgrade_tick').show();
    $('#upgrade_tick').html("0%");

    document.getElementById("upgrade_light").style.display = "block";
    document.getElementById("upgrade_light").style.position = "fixed";
    document.getElementById("upgrade_light").style.left = (screen_width - 344) / 2 + "px";
    document.getElementById("upgrade_light").style.top = (screen_height - 236) / 2 + "px";
    document.getElementById("fade").style.display = "block";
}
function hide_alert_wait() {
    document.getElementById("upgrade_light").style.display = "none";
    document.getElementById("fade").style.display = "none";
    $('#server_text').show();
    $('#local_version').show();
    $('#local_text').show();
    $('#server_version').show();
    $('#upgrade_OK').show();
    $('#upgrade_CANCEL').show();
    window.location = "ezcast://ezcast.app/back";
}

function show_psw_change() {
    var scroll_left = document.body.scrollLeft;
    var scroll_top = document.body.scrollTop;
    $('#change_psw_warn').text(LIST_WARN2_string);
    document.getElementById("psw_change").style.display = "block";
    document.getElementById("psw_change").style.position = "fixed";
    document.getElementById("psw_change").style.left = (screen_width - 344) / 2 + "px";
    document.getElementById("psw_change").style.top = (screen_height - 236) / 2 + "px";
    document.getElementById("fade").style.display = "block";
}
function show_psw_change_warn() {
    var scroll_left = document.body.scrollLeft;
    var scroll_top = document.body.scrollTop;
    document.getElementById("psw_change").style.display = "block";
    document.getElementById("psw_change").style.position = "fixed";
    document.getElementById("psw_change").style.left = (screen_width - 344) / 2 + "px";
    document.getElementById("psw_change").style.top = (screen_height - 236) / 2 + "px";
    document.getElementById("fade").style.display = "block";
    $('#OK3').hide();
    $('#CANCEL2').hide();
    $("#paw_warn_ok").show();

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
            $('#ota_fail').popup('close');
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
        wifi_mode_status = wifi_mode.indexOf("9");
        if (wifi_mode_status >= 0) {
            if (!wifi_connect_flag) 
            {
                Get_faild_connect_language();
                show_faild_connect_dialog(); //no wifi connect warn
            } else {
                if (Upgrade_listener_flag == 1) {
                    $('#upgrade_dialog_msg').text(waiting_string);
                    $('#ota_fail').popup('open');
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

function get_wlan_mode() {
    var ret;
    $.get("cgi-bin/wifi_info_GET.cgi", {type: 'getLanSettingVal'},function(LanSettingVal) {
        var ret = LanSettingVal;
        if (ret.length >= 0) {
            if (Upgrade_listener_flag == 1) {
                $('#upgrade_dialog_msg').text(waiting_string);
                $('#ota_fail').popup('open');
                $('#warn').show();
                $('#warn_time').hide();
            }
            $.get(cgiurl, {type: 'get_OTA_conf'},function() {

            },"text");
            get_ota_check_status();

        } else {
            Get_faild_connect_language();
            show_faild_connect_dialog(); //no wifi connect warn
        }
    },"text");
}

function progressbat() {

    var progressbar = $("#progressbar"),
    progressLabel = $(".progress-label"),
    progressval = $(".ui-progressbar-value");

    if (upgrade_bar >= 100) {
        progressLabel.text("100%");
        progressval.css("width", "100%");
    } else {
        progressLabel.text(upgrade_bar + "%");
        progressval.css("width", upgrade_bar + "%");
        $('#aa_txt').text(upgrade_bar + "%");
        $('#upg_txt_3').text(upgrade_bar + "%");
    }

}
function Get_ota_status_tick() {
    var progressbar = $("#progressbar"),
    progressLabel = $(".progress-label"),
    progressval = $(".ui-progressbar-value");

    var cgiurll = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurll, {type: 'get_ota_status'},function(ota_status)
    {
        upgrade_bar = ota_status;
        progressLabel.text(upgrade_bar + "%");
        progressval.css("width", upgrade_bar + "%");
        if (ota_status >= 100) {
            clearInterval(get_ota_status_id);
            get_ota_status_id = -1;
            back_ezcastapp();
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

    $('#ota_awrn_ok').click(function() {
        $('#ota_fail').popup('close');
        $('#warn').show();
        $('#warn_time').hide();
        clearInterval(show_warn_dialog_id);
        show_warn_dialog_id = -1;
        show_warn_dialog_tick = 11;
    });

}
function Upgrade_CANCEL_listener() {
    $('#upgrade_CANCEL').click(function(e) {
        e.preventDefault();
        hide_upgrade_dialog();
    });
}
//ota upgrade button click
function Upgrade_listener() {
    $('#upgrade').click(function(e) {
        var ret;
        e.preventDefault();
        Upgrade_listener_flag = 1;
        $.get("cgi-bin/wifi_info_GET.cgi", {type: 'get_netlink'},function(get_netlink) {
            ret = Number(get_netlink);
            if (ret) {
                get_wlan_mode();
            } else {
                get_wifi_mode();
            }
        },"text");

    });
}
function internet_listener() {
    $('#internet').click(function(e) {
        e.preventDefault();
        if (show_connections_num_id > 0) {
            clearInterval(show_connections_num_id);
            show_connections_num_id = -1;
        }
        if (screen_width >= 800) {
            location.href = "wifi_list_Horizontal.html";
        } else {
            location.href = "wifi_list_Vertical.html";
        }
    });
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
            text_string = text_string_global;
            $('#ip1_txt').text(Get_text_from_textstring(text_string));
        }
    },"text");
}
//read Lan mac
function Get_Lan_mac_ip() {
    $.get("cgi-bin/wifi_info_GET.cgi", {type: 'getLan_mac_ip'},function(mac_lan_ip) {
        if (mac_lan_ip.length) {
            $('#LAN_mac_address').text(mac_lan_ip);
        }
    },"text");
}
//read Lan ip for probox lan only
function Get_Lan_ip() {
    $.get("cgi-bin/wifi_info_GET.cgi", {type: 'get_lan_ip'},function(lan_ip) {
        if (lan_ip.length) {
            $('#ip1_txt').text(lan_ip);
        }
    },"text");
}
//read 2.4G mac
function Get_24G_mac_ip() {
    $.get("cgi-bin/wifi_info_GET.cgi", {type: '24\nget_mac_ip'},function(mac_wlan3_ip) {
        if (mac_wlan3_ip.length) {
            start = 0;
            text_string_length = 0;
            text_string = mac_wlan3_ip;
            text_string_length = mac_wlan3_ip.length;
            $('#24G_mac_address').text(Get_text_from_textstring(text_string));
        }
    },"text");
}
function init_function() {
    $('#internet_icon').hide();
    document.getElementById("upgrade_status").src = "img/dongle.png";
}
function Set_music_output() {
    $.post('cgi-bin/set_music_output_POST.cgi', $('#config-form').serialize());
}
function OK_listener() {
    $('#OK').click(function(e) {
        e.preventDefault();
        if (function_index == 1) {
            Set_music_output();
            hide_setting_dialog();
        } else if (function_index == 2) {
            $.post('cgi-bin/set_defaultmode_POST.cgi', $('#config-form').serialize());
            save_defaultmode_index = defaultmode_index;
            hide_setting_dialog();
        } else if (function_index == 3) {
            $.post('cgi-bin/set_lan_POST.cgi', $('#config-form').serialize());
            waiting_set_language();
        } else if (function_index == 4) {
            hide_setting_dialog();
            current_eq_mode_index = Source_i;
            //--send date to cgi-----
            //eq_index=Source_Selection[Source_i];
            /********set  custom value to selection default*********/
            $('#custom_or_other').val("1");
            $('#eq_value').val("" + ( - 1));
            custom_index = Source_i;
            $('#custom_index').val("" + custom_index);
            $.post('cgi-bin/set_EQ.cgi', $('#config-form').serialize());
            /**********************************************/
            $('#custom_or_other').val("0");
            $('#eq_index').val("" + Source_i);
            $.post('cgi-bin/set_EQ.cgi', $('#config-form').serialize());
        }
    });
}
function CANCEL_listener() {
    $('#CANCEL').click(function(e) {
        e.preventDefault();
        if (function_index == 1) {
            show_waiting_tick = 11;
            $('#tick').html("12");
            $('#output_mode_index').val("" + current_music_output_index);
            $('#resolution').val("" + music_output_array[current_music_output_index]);
            music_output_index = current_music_output_index;
            hide_setting_dialog();
            if (show_waiting_tick_id > 0) {
                clearInterval(show_waiting_tick_id);
                show_waiting_tick_id = -1;
            }
        } else if (function_index == 2) {
            defaultmode_index = save_defaultmode_index;
            $('#defaultmode').val("" + defaultmode_array[defaultmode_index]);
            hide_setting_dialog();
        } else if (function_index == 3) {
            hide_setting_dialog();
            language_index = current_language_index;
            if (auto_language_flag == 1) 
	    	$('#language').val("" + multiLanguage_array[multiLanguage_array.length - 1]);
            else 
	    	$('#language').val("" + multiLanguage_array[current_language_index]);
        } else if (function_index == 4) {
            hide_setting_dialog();
            show_waiting_tick = 11;
            $('#tick').html("12");
            Source_i = current_eq_mode_index;
            $('#EQ_default_mode').val(Source_Selection[Source_i]);
            if (show_waiting_tick_id > 0) {
                clearInterval(show_waiting_tick_id);
                show_waiting_tick_id = -1;
            }
        }
    });
}
function show_connections_num_tick() {
    $.get("cgi-bin/wifi_info_GET.cgi", {type: 'get_connections'},function(connections_num) {
        if (connections_num.length) {
            var ret = connections_num.indexOf("-1");
            if (ret < 0) {
                $('#dongle_icon_txt').text("( " + connections_num + " )");
                document.getElementById("dongle_connect_status").src = "img/dongle_connected.png";
            } else {
                $('#dongle_icon_txt').text("( " + 0 + " )");
                document.getElementById("dongle_connect_status").src = "img/dongle_not_connected.png";
            }
        }
    },"text");
}
function show_connections_num() {
    show_connections_num_tick();
    if (show_connections_num_id > 0) {
        clearInterval(show_connections_num_id);
        show_connections_num_id = -1;
    }
    show_connections_num_id = self.setInterval(show_connections_num_tick, 3000);
}
var aa = 0;
var lan_con = 0;
function Get_connected_ssid_tick() {
    aa++;
    if (aa > 10) {
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
        var ret = connected_ssid.indexOf("to_wifi_list");
        if (ret >= 0) {
            lan_con = connected_ssid.indexOf("to_wifi_list&lan");
            if (lan_con >= 0) 
                lan_con = 1; 
            else if(miracast_probox_mode==0){
                if(plat_flag =="networsetup")
		    location.href = "wifilist.html?advanced=1";
                else if(plat_flag =="devicemanagement"){
		    var urlinfo=window.location.href; 
		    var offset=urlinfo.indexOf("advanced=1");   
		    if(offset<0)
		        location.href = "wifilist.html";  //from airsetup jump
                }
            	}
        } else {
            if (connected_ssid.length && lan_con != 1) {
                if (connected_ssid_flag == 0) {
                    $('#connected_ssid').text(connected_ssid);
                    $('#connected_ssid').show();
                    wifi_connect_flag = 1;
                    connected_ssid_flag = 1;
                    if (ezmusic_flag>1) {
                        $('#ip0show').show();
                    }
                    Get_wlan0_ip_address();
                    show_connections_num_id = self.setInterval(Get_local_version, 3000);
                }
            } else {
                $('#connected_ssid').hide();
                $('#connected_ssid').text("no wifi connect!");
                wifi_connect_flag = 0;
                connected_ssid_flag = 0;
            }
        }
    },"text");
}

function Get_connected_ssid() {
    Get_connected_ssid_tick();
    if (get_connected_ssid_id > 0) {
        clearInterval(get_connected_ssid_id);
        get_connected_ssid_id = -1;
    }
    aa = 0;
    get_connected_ssid_id = self.setInterval(Get_connected_ssid_tick, 3000);
    self.setTimeout(show_ui,1000);   //delay show ui
}

var ii = 0;
//read current version
function Get_local_version() {
    ii++;
    if (ii > 5) {
        clearInterval(show_connections_num_id);
        show_connections_num_id = -1;
    }
    $.get(cgiurl, {type: 'get_OTA_version'},function(ota_upgrade_string) {
        if (ota_upgrade_string.length) {
            start = 0;
            text_string_length = 0;
            text_string = ota_upgrade_string;
            text_string_length = ota_upgrade_string.length;
            var local_version_string = Get_text_from_textstring(text_string);
            $('#current_ver').text(local_version_string);
            $('#ezwire_ver').text(local_version_string);
            text_string = text_string_global;
            var server_version_string = Get_text_from_textstring(text_string);
            $.get(cgiurl, {type: 'get_wifi_mode'},function(wifi_mode) {
                wifi_mode_status = wifi_mode.indexOf("9");
                if (wifi_mode_status >= 0) {
                    var ret = server_version_string.indexOf("newest");
                    if (ret < 0) {
                        Get_new_version_text();
                        clearInterval(show_connections_num_id);
                        show_connections_num_id = -1;
                        ii = 6;
                        clearInterval(get_connected_ssid_id);
                        get_connected_ssid_id = -1;
                        aa = 20;
                    } else {
                        $('#new_version').show();
                        $('#new_version').text("no new version!");
                    }
                } else {
                    $('#upg_txt').text("no wifi_mode_status");
                    $('#new_version').hide();
                    $('#new_version').text("not wifi connect!");
                }
            },"text");
        }
    },"text");
}
function Check_new_version_boot() {
    $.get(cgiurl, {type: 'get_wifi_mode'},function(wifi_mode) {
        wifi_mode_status = wifi_mode.indexOf("9");
        if (wifi_mode_status >= 0) {
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
function deal_OTA_url() {
    var temp = location.href;
    var ret = temp.indexOf("back_connect_ap");
    if (ret >= 0) {
        check_OTA_first_flag = 1;
        show_upgrade_waiting_dialog();
        get_new_version_function();
    } else {
        check_OTA_first_flag = 1;
        Check_new_version_boot();
    }
}
//  2.4G css
function ezprochannel_off_style() {
    $('#ezpro_Channel_val-button').addClass("ui-state-disabled");
    $('#ezpro_Channel_val-button').attr("aria-disabled", "true");
}
function ezprochannel_on_style() {
    $('#ezpro_Channel_val-button').removeClass("mobile-slider-disabled");
    $('#ezpro_Channel_val-button').removeClass("ui-state-disabled");
    $('#ezpro_Channel_val-button').removeAttr("aria-disabled");
}
//5G css
function channel_5G_off_style() {

    $('#5G_country_val-button').addClass("ui-state-disabled");
    $('#5G_country_val-button').attr("aria-disabled", "true");
    $('#5G_Channel_val-button').addClass("ui-state-disabled");
    $('#5G_Channel_val-button').attr("aria-disabled", "true");
}
function channel_5G_on_style() {

    $('#5G_country_val-button').removeClass("mobile-slider-disabled");
    $('#5G_country_val-button').removeClass("ui-state-disabled");
    $('#5G_country_val-button').removeAttr("aria-disabled");
    $('#5G_Channel_val-button').removeClass("mobile-slider-disabled");
    $('#5G_Channel_val-button').removeClass("ui-state-disabled");
    $('#5G_Channel_val-button').removeAttr("aria-disabled");

}
//channel button
function Channel_listener() {

    $("#set_channel_ok").click(function() {
        channel_index = Number($('#ezpro_Channel_val').val());
        $('#channel_txt').text(channel_index);
        var str = channel_index + "\n" + $('#G24_Bandwidth_val').val() + "\n" +"set_channel\n";
        $.get(cgiurl, {type: str},function(ret) {
         },"text");
        $("#Channel_24G").popup('close');
    });

    $("#set_channel_warn").click(function() {
        if (wifi_mode_flag == 2) {
            // /2.4G connect
            $("#Channel_24G").popup('close');
        } else {
            $("#menu_24G").css('display', 'none');
            $("#menu_24G_warn").css('display', 'block');
        }
    });
	
    $("#set_5Gchannel_warn").click(function() {
        if (wifi_mode_flag == 1) {
            // /5G connect
            $("#Channel_5G").popup('close');
        } else {
            $("#channel_5Gsel").css('display', 'none');
            $("#menu_5G_warn").css('display', 'block');
        }
    });

    $("#set_channel_cal").click(function() {
    });

    $('#channel_set').click(function() {
        $("#ezpro_Channel_val-button").css({ "width":"40px","top":"-10px"});
        if (channel_index == 0) 
            $("#ezpro_Channel_val-button").children("span").text("Auto");
        else 
            $("#ezpro_Channel_val-button").children("span").text(channel_index);

            var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
            $.get(cgiurl, {type: 'get_ProboxConnectStatus'}, function(ProboxConnectStatus) {
                wifi_mode_flag = Number(ProboxConnectStatus);
                if (wifi_mode_flag == 2) {
                    // /2.4G connect
                    ezprochannel_off_style();
                    channel_5G_on_style();
                } else if (wifi_mode_flag == 1) {
                    channel_5G_off_style();
                    ezprochannel_on_style();
                }
				
            },"text");



        $("#menu_24G").css('display', 'block');
        $("#menu_24G_warn").css('display', 'none');
        $("#channel_5Gsel").css('display', 'block');
        $("#menu_5G_warn").css('display', 'none');

    });

    $('input:radio[name="menu_choose"]').click(function() {
        var that = $(this).val();
        Channel_selectValue = that;
    });

    $("#sel_channel_ok").click(function() {
        var i;
        switch (Number(Channel_selectValue)) {
        case 1:
            $('#Channel').popup('close');
            setTimeout(function() {
                $('#Channel_24G').popup('open');
            },
            300);
            break;
        case 2:
            $('#Channel').popup('close');
            setTimeout(function() {
                $('#Channel_5G').popup('open');
            },
            300);
            $('#5G_country_txt').next().css('width', '80%');
            $('#5G_Channel_txt').next().css('width', '100%');
            Get_softap_5g_initvalue();//read 5G init value
            break;
        default:
            break;
        }
    });

    $("#set_5Gchannel_ok").click(function() {
        if($('#5G_Channel_val').val()=="Auto")
            channel_5G_index=0;
        else
            channel_5G_index = $('#5G_Channel_val').val();

        get_channel_5G_country_index = $('#5G_country_val').val();
        var str = get_channel_5G_country_index + "\n" + channel_5G_index + "\n" + $('#5G_Bandwidth_val').val()+ "\nset_5G_country\n";
        $.get(cgiurl, {type: str},function(ret) {
        },"text");

        $("#5G_country_val-button").children("span").text(Channel_5G_country[get_channel_5G_country_index]);
        $("#5G_Channel_val-button").children("span").text(channel_5G_index);
        $("#5G_Bandwidth_val-button").children("span").text(Bandwidth[Number($('#5G_Bandwidth_val').val())]);
        $("#Channel_5G").popup('close');
    });

}
//get 5g value
function getvalue(obj) {
    var i;
    var m = $('#5G_country_val').val();
    var sel = document.getElementById("5G_Channel_val");
    sel.options.length = 0;
    switch (Number(m)) {
        case 0:
        case 6:
        case 14:
        case 15:
        case 16:
        case 17:
        case 19:
        case 20:
            $("#5G_Channel_val-button").children("span").text(country1[0]);
            for (i = 0; i < country1.length; i++) {
                sel.add(new Option(country1[i], country1[i]));
            }
            break;
        case 1:
        case 2:
        case 5:
        case 9:
        case 10:
        case 18:
        case 21:
        case 22:
        case 24:
            $("#5G_Channel_val-button").children("span").text(country2[0]);
            for (i = 0; i < country2.length; i++) {
                sel.add(new Option(country2[i], country2[i]));
            }
            break;
        case 3:
        case 11:
        case 12:
        case 13:
            $("#5G_Channel_val-button").children("span").text(country4[0]);
            for (i = 0; i < country4.length; i++) {
                sel.add(new Option(country4[i], country4[i]));
            }
            break;
        case 4:
            $("#5G_Channel_val-button").children("span").text(country5[0]);
            for (i = 0; i < country5.length; i++) {
                sel.add(new Option(country5[i], country5[i]));
            }
            break;
        case 7:
            $("#5G_Channel_val-button").children("span").text(country8[0]);
            for (i = 0; i < country8.length; i++) {
                sel.add(new Option(country8[i], country8[i]));
            }
            break;
        case 8:
            $("#5G_Channel_val-button").children("span").text(country9[0]);
            for (i = 0; i < country9.length; i++) {
                sel.add(new Option(country9[i], country9[i]));
            }
            break;
        case 23:
            $("#5G_Channel_val-button").children("span").text(country19[0]);
            for (i = 0; i < country19.length; i++) {
                sel.add(new Option(country19[i], country19[i]));
            }
            break;
       default:
            //alert("--------error-------");
            break;
    }

}

function resolution_set_default_value() {
    var i=0;
    $('#res_value').val("" + resolution_current_value_num);
    $('#current_res_value').val("" + 0);
    resolution_index = current_resolution_index;
    if(ezmusic_flag!=6) {
        for(i=0;i<resolution_list_value.length;i++){
            if(resolution_list_value[i]==resolution_index)
                break;
        }
        $('#resolution_sel').text(resolution_array[i]);
    }
    else {
        $('#resolution_sel').text(resolution_array[resolution_index]);
    }
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
            $("#Resolution").popup('close');
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
//resolution button
function Set_resolution_listener() 
{
    $("#set_resolution_ok").click(function() {
        Get_resolution_text();
        if (resolution_index != current_resolution_index) {
    		if(resolution_index==5 && ezmusic_flag==7)//7:ezwire   5: FMT_1920x1080_60P
                $('#dialog_msg2').show();
    		else
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
        if(ezmusic_flag!=6) {
            for(i=0;i<resolution_list_value.length;i++){
                if(resolution_list_value[i]==resolution_index)
                    break;
            }
            $('#resolution_sel').text(resolution_array[i]);
        }
        else {
            $('#resolution_sel').text(resolution_array[resolution_index]);
        }
        resolution_current_value = $('#resolution_sel').text();
        current_resolution_index=resolution_index;
        Set_hdmimode();
        $("#Resolution").popup('close');
    });

    $("#resolution_cancle").click(function() {
        clearInterval(show_waiting_tick_id);
        show_waiting_tick_id = -1;
        resolution_set_default_value();
        Set_hdmimode();
        $("#Resolution").popup('close');
    });

    $("#set_resolution_cal").click(function() {
        resolution_index = current_resolution_index;
    });

    $('input:radio[name="resolution_choose"]').click(function() {
        var that = $(this).val();
        var b = $(this).prev().text();
        resolution_index = Number(that);
        $('#res_text').text(resolution_index);
        switch (resolution_index) {
            case 0:
                if (probox_flag) $('#res_value').val("4"); //HDMI_1920x1080_24P 
                else $('#res_value').val("5"); //FMT_1650x750_1280x720_60P
                break;
            case 1:
                if (probox_flag) $('#res_value').val("2"); //1920x1080_30P  for  hdmi&&VGA add 100 for flag
                else $('#res_value').val("4"); //FMT_2750x1125_1920x1080_24P
                break;
            case 2:
                if (probox_flag) $('#res_value').val("117"); //1920x1080_60P
                else $('#res_value').val("2"); //FMT_2200x1125_1920x1080_30P
                break;
            case 3:
                if (probox_flag) $('#res_value').val("116"); //1280x800_60P
                else $('#res_value').val("15"); //FMT_1250x810_1024x768_60P
                break;
            case 4:
                if (probox_flag) $('#res_value').val("105"); //1280x720_60P
                else $('#res_value').val("16"); //FMT_1280x800_60P
                break;
            case 6:
                if (probox_flag) $('#res_value').val("200"); //VGA_1280x1024     for  VGA add 200 for flag
                break;
            case 5:
                if (probox_flag) $('#res_value').val("115"); //1024x768
                else $('#res_value').val("17"); //FMT_1920x1080_60P
                break;
        }
        $('#res_text2').text($('#res_value').val() + "-|-" + resolution_current_value_num);
    });

    $('#resolution_all').click(function() {
        $('input:radio[name="resolution_choose"]').each(function() {
            $(this).prev().addClass("ui-radio-off");
            $(this).prev().removeClass("ui-radio-on");
            $(this).attr("checked", " ");
        });
        set_resolutionradio();
        $("#res_sel").css('display', 'block');
        $("#res_warn").css('display', 'none');
    });

}
//set resolution css
function set_resolutionradio() {
    var that = $("input[type=radio][name=resolution_choose][value=" + resolution_index + "]");
    that.attr("checked", "true");
    that.prev().removeClass("ui-radio-off");
    that.prev().addClass("ui-radio-on");
}
//Creat resolution list
function Creat_resolution_list() {
    if(ezmusic_flag == 6 || ezmusic_flag == 9){ //ezprobox/box lan only
        resolution_array = ["HDMI_1920x1080_24P", "HDMI_1920x1080_30P", "1920x1080_60P", "1280x800_60P", "1280x720_60P", "1024x768_60P", "VGA_1280x1024"];
    }
    $('#res_text').text("--array leng: " + resolution_array.length);
    for (var i = 0; i < resolution_array.length; i++) {
        var that=$("label[for=res_" + i + "]");
        that.text(resolution_array[i]);
        if(ezmusic_flag!=6) {
            that.next().val(resolution_list_value[i]);
        }
    }
    $('#res_text2').text("Creat_resolution_list: " + resolution_array.length);
    switch (ezmusic_flag) {
        case 2:
            //ezcast
        case 3:
            //EZWILAN_ENABLE
            $("#res_0_txt,#res_0,#res_2_txt,#res_2,#res_5_txt,#res_5,#res_6_txt,#res_6").remove();
            //$("#res_3_txt,#res_3,#res_4_txt,#res_4,#res_5_txt,#res_5,#res_6_txt,#res_6").remove();
            $("#res_ezpro").find('.ui-radio').last().remove();
            break;
        case 4:
            //ezpro dongle
            $("#res_6_txt,#res_6").remove();
            $("#res_ezpro").find('.ui-radio').last().remove();
            break;
        case 5:
            //ezproboxWILAN
            $("#res_6_txt,#res_6").remove();
            $("#res_ezpro").find('.ui-radio').last().remove();
            break;
        case 7:
        case 8:
            //ezwire
            $("#res_0_txt,#res_0,#res_3_txt,#res_3,#res_6_txt,#res_6").remove();
            //$("#res_3_txt,#res_3,#res_1_txt,#res_1,#res_6_txt,#res_6").remove();
            $("#res_ezpro").find('.ui-radio').last().remove();
            break;
        default:
            break;
    }
    $('input:radio[name="resolution_choose"]').last().prev().addClass("ui-last-child");
    set_resolutionradio();
}
//read resolution value
function Get_resolution() {
    if (ezmusic_flag == 6 || ezmusic_flag == 9) //ezprobox/box lan only
    {
        probox_flag = 1;
    }
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
 //   $.get(cgiurl, { type: 'get_resolution'},function(resolution) {

    //while(totalData.password == "abc");
        var resolution = totalData.resolution;
        var Probox_res = Number(resolution);
        if (probox_flag) {
            switch (Probox_res) {
                case 4:
                    $('#res_value').val("4");
                    resolution_index = 0;
                    $('#resolution_sel').text("HDMI_1920x1080_24P");
                    break;
                case 2:
                    $('#res_value').val("2");
                    resolution_index = 1;
                    $('#resolution_sel').text("HDMI_1920x1080_30P");
                    break;
                case 17:
                    $('#res_value').val("117");
                    resolution_index = 2;
                    $('#resolution_sel').text("1920x1080_60P");
                    break;
                case 16:
                    $('#res_value').val("116");
                    resolution_index = 3;
                    $('#resolution_sel').text("1280x800_60P");
                    break;
                case 5:
                    $('#res_value').val("105");
                    resolution_index = 4;
                    $('#resolution_sel').text("1280x720_60P");
                    break;
                case 200:
                    $('#res_value').val("200");
                    resolution_index = 6;
                    $('#resolution_sel').text("VGA_1280x1024");
                    break;
                case 15:
                    $('#res_value').val("115");
                    resolution_index = 5;
                    $('#resolution_sel').text("1024x768_60P");
                    break;
                default:
                    $('#res_value').val("105");
                    resolution_index = 4;
                    $('#resolution_sel').text("1280x720_60P");
                    break;
            }
        } else {
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
                            } else {
                                res = resolution.indexOf("16");
                                if (res >= 0) {
                                    $('#res_value').val("16");
                                    resolution_index = 4;
                                    $('#resolution_sel').text("1280x800_60P"); //FMT_1280x800_60P
                                }
                            }
                        }
                    }
                }
            }
        }
        current_resolution_index = resolution_index;
        resolution_current_value_num = $('#res_value').val();
        resolution_current_value = $('#resolution_sel').text();
        $('#upg_txt_4').text(resolution);
        $('#upg_txt').text(current_resolution_index);
        Creat_resolution_list();

  //  },"text");
}

function Get_cpu_frequency() {
    $.get("cgi-bin/wifi_info_GET.cgi", {type: 'get_frequency'},function(cpu_frequency) {
        var ret = Number(cpu_frequency);
        if (ret == 1) {
            $('#res_ezmusic').remove();
            resolution_array.push("1024x768_60P", "1280x800_60P", "1920x1080_60P");
            $('#res_text').text("--array leng: " + resolution_array.length);
        } else {
            $('#res_ezpro').remove();
        }
    },"text");
}

function Set_hdmimode() {
    $.post('cgi-bin/set_resolution_POST.cgi', $('#config-form').serialize());
}
//read music output value
function Get_music_output() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_music_output'}, function(music_output) {
        var ret = music_output.indexOf("default");
        if (ret >= 0) {
            music_output_index = 0;
            $('#mus_output').text("" + music_output_array[music_output_index]);
        } else {
            music_output_index = Number(music_output);
            if (2 == music_output_index) //I2S
            $('#mus_output').text("I2S");
            else if (1 == music_output_index) //SPDIF2.1	
            $('#mus_output').text("Optical x2");
            else //SPDIF5.1	
            $('#mus_output').text("Optical x5.1");
        }
        if ((0 == SPDIF_EN) && (1 == I2S_EN)) {
            music_output_index = 0;
            $('#mus_output').text("I2S");
        }
        current_music_output_index = music_output_index;
    },"text");
}
//read music output text
function Get_music_output_en() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_en_music_output'},function(music_output_en_string) {
        if (music_output_en_string.length) {
            start = 0;
            text_string_length = 0;
            text_string = music_output_en_string;
            text_string_length = music_output_en_string.length;
            SPDIF_EN = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            I2S_EN = Get_text_from_textstring(text_string);
            $("#aaa").text("SPDIF_EN:" + SPDIF_EN + "-" + I2S_EN);
            if ((1 == SPDIF_EN) && (0 == I2S_EN)) 
	    	music_output_array = ["Optical x5.1", "Optical x2"];
            else if ((0 == SPDIF_EN) && (1 == I2S_EN)) 
	    	music_output_array = ["I2S"];
            else if ((1 == SPDIF_EN) && (1 == I2S_EN)) 
	    	music_output_array = ["Optical x5.1", "Optical x2", "I2S"];
            Creat_music_output_list();
        }
    },"text");
}

function Creat_music_output_list() {
    var bb = $("#aaa").text();
    $("#aaa").text(bb + "-/-" + music_output_array.length);
    for (var i = 0; i < music_output_array.length; i++) {
        var index = i;
        var id = "output" + i;
        var val = index.toString();
        var output_string = music_output_array[i];
        var innerhtml = "<div class=\"ui-radio\"><label for='" + id + "'  class=\"ui-btn ui-corner-all ui-btn-inherit ui-btn-icon-right ui-radio-off\" >" + output_string + "</label><input type=\"radio\" name=\"music_output\" id='" + id + "' value='" + val + "' ></div>";
        $("#creat_musiclist").find(".ui-controlgroup-controls").append(innerhtml);
    }

    $("#output0").prev().addClass("ui-first-child");
    $("#output" + (music_output_array.length - 1) + "").prev().addClass("ui-last-child");

    index = music_output_index;
    $("label[for=output" + index + "]").removeClass("ui-radio-off");
    $("label[for=output" + index + "]").addClass("ui-radio-on");

    music_output_listener();

}

function music_output_listener() {

    $('input:radio[name="music_output"]').click(function() {
        var that = $(this);
        var b = $(this).prev().text();
        $("#modetxt").text(b);

        $('input:radio[name="music_output"]').each(function() {
            $(this).prev().addClass("ui-radio-off");
            $(this).prev().removeClass("ui-radio-on");
            $(this).attr("checked", " ");

        });
        that.prev().addClass("ui-radio-on");
        that.prev().removeClass("ui-radio-off");
        that.attr("checked", "true");
    });

    $("#music_output_ok").click(function() {
        music_output_index = $('input:radio[name="music_output"]:checked').val();
        if ((0 == SPDIF_EN) && (1 == I2S_EN)) {
            $.get('cgi-bin/set_music_output_POST.cgi', {output_mode_index: '2'},function(dir_list) {
            },"text");
        } else if (current_music_output_index != music_output_index) {
            current_music_output_index = music_output_index;
            $("#modetxt").text(music_output_index);
            $('#mus_output').text(music_output_array[music_output_index]);
            $('#output_mode_index').val(music_output_index);
            $.post('cgi-bin/set_music_output_POST.cgi', $('#config-form').serialize());
        }
    });

    $('#modelout').click(function() {
        $('input:radio[name="music_output"]').each(function() {
            $(this).prev().addClass("ui-radio-off");
            $(this).prev().removeClass("ui-radio-on");
            $(this).attr("checked", " ");
        });
        var index = current_music_output_index;
        $("label[for=output" + index + "]").removeClass("ui-radio-off");
        $("label[for=output" + index + "]").addClass("ui-radio-on");
    });

}
//read EQ value
function get_EQ_default() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_EQ_default'},function(EQ_default_mode) {
        Source_i = Number(EQ_default_mode);
        $('#EQ_default_mode').text(Source_Selection[Source_i]);
        current_eq_mode_index = Source_i;
    },"text");
}

function Creat_EQ_list() {
    for (var i = 0; i < Source_Selection.length; i++) {
        var index = i;
        var id = "eq" + i;
        var val = index.toString();
        var output_string = Source_Selection[i];
        var innerhtml = "<div class=\"ui-radio\"><label for='" + id + "'  class=\"ui-btn ui-corner-all ui-btn-inherit ui-btn-icon-right ui-radio-off\" >" + output_string + "</label><input type=\"radio\" name=\"eq\" id='" + id + "' value='" + val + "' ></div>";
        $("#creat_eqlist").append(innerhtml);
    }
    $("input[type=radio][name=eq][value=" + Source_i + "]").attr("checked", "true");
}

function Set_EQ_listener() {
    $("#eq_ok").click(function() {
        Source_i = $('input:radio[name="eq"]:checked').val();
        if (Source_i == 18) //custom
        {
            $("#custom_show").hide();
            setTimeout(delay, 100);
            get_EQ_custom();
            this.href = "#eq_custom";
        } else if (Source_i != current_eq_mode_index) {
            $('#EQ_default_mode').text(Source_Selection[Source_i]);
            current_eq_mode_index = Source_i;
            //--send date to cgi-----
            //eq_index=Source_Selection[Source_i];
            /********set  custom value to selection default*********/
            $('#custom_or_other').val("1");
            $('#eq_value').val("" + ( - 1));
            custom_index = Source_i;
            $('#custom_index').val("" + custom_index);
            $.post('cgi-bin/set_EQ.cgi', $('#config-form').serialize());
            /**********************************************/
            $('#custom_or_other').val("0");
            $('#eq_index').val("" + Source_i);
            $.post('cgi-bin/set_EQ.cgi', $('#config-form').serialize());
            this.href = "#pagehome";
        }
    });

    $('#Set_EQ_mode').click(function() {
        $('input:radio[name="eq"]').each(function() {
            $(this).prev().addClass("ui-radio-off");
            $(this).prev().removeClass("ui-radio-on");
            $(this).attr("checked", " ");
        });
        var index = current_eq_mode_index;
        $("label[for=eq" + index + "]").removeClass("ui-radio-off");
        $("label[for=eq" + index + "]").addClass("ui-radio-on");
        $("input[type=radio][name=eq][value=" + current_eq_mode_index + "]").attr("checked", "true");
    });

}
function set_hdmicec_listener() {
    $("#set_hdmicece_ok").click(function(e) {
        set_HDMI_cec_text();
        $('#hdmi_cec_set').popup("close");
    });
    $("#set_hdmicece_cel").click(function(e) {
        $('#hdmi_cec_set').popup("close");
        if($('#hdme_cec_txt').text()=="ON"){
            cec_on_style();
        } else {
            cec_off_style();
        }
    });
    $("#hdmi_cec").click(function(e) {
        Get_HDMI_cec_text();
    });	

}
function set_HDMI_cec_text() {
    var value_cec,value_audio;
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); 
    var value_cec = $('#cec_val').val();
    var value_audio = $('#hdmi_audio_val').val();
    var str = value_cec + "\n" +value_audio + "\n" + "set_hdmicec\n";
    if(Number(value_cec)) {
        $('#hdme_cec_txt').text("ON");
    } else {
        $('#hdme_cec_txt').text("OFF");
    }
    $.get(cgiurl, {type: str},function(ret) {
    }, "text");

}
function cec_on_style() {
    $("#cec_val").val(1);
    $("#cec_control").children().addClass("ui-flipswitch-active");
    $("#cec_val option").eq(1).attr("selected", true); //on
}
function cec_off_style() {
    $("#cec_val").val(0);
    $("#cec_control").children().removeClass("ui-flipswitch-active");
    $("#cec_val option").eq(0).attr("selected", true); //off
}
function hdmi_audio_on_style() {
    $("#hdmi_audio_val").val(1);
    $("#hdmi_audio_control").children().addClass("ui-flipswitch-active");
    $("#hdmi_audio_val option").eq(1).attr("selected", true); //on
}
function hdmi_audio_off_style() {
    $("#hdmi_audio_val").val(0);
    $("#hdmi_audio_control").children().removeClass("ui-flipswitch-active");
    $("#hdmi_audio_val option").eq(0).attr("selected", true); //off
}
//read hdmi cec text
function Get_HDMI_cec_text() {
    var value_cec,value_audio;
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_hdmicec'},function(ret) {
        if (ret.length) {
            start = 0;
            text_string_length = 0;
            text_string = ret;
            text_string_length = ret.length;
            value_cec = Number(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            value_audio = Number(Get_text_from_textstring(text_string));
	    $('#cec_val').val(value_cec);
	    if (value_cec) {
		cec_on_style();
		$('#hdme_cec_txt').text("ON");
	    } else {
		cec_off_style();
		$('#hdme_cec_txt').text("OFF");
	    }
	    $('#hdmi_audio_val').val(value_audio);
	    if (value_audio) {
		hdmi_audio_on_style();
	    } else {
		hdmi_audio_off_style();
	    }
        }
    },"text");
}
//creat airdiskautoplay_list
function add_airdiskautoplay_li() {
    var str ="<li class='ui-li-has-count ui-li-has-icon ui-last-child' id='Autoplay'><a class='ui-btn ui-btn-icon-right ui-icon-carat-r' aria-expanded='false' aria-haspopup='true' aria-owns='Autoplay_set' style='border-width: 0px 0px 1px; border-style: none none inset; border-color: currentColor currentColor rgb(183, 183, 187); border-image: none; color: rgb(255, 255, 255); font-weight: normal; text-shadow: none; background-color: transparent;' href='#Autoplay_set' data-position-to='window' data-rel='popup'><img class='ui-li-icon' alt='Autoplay_set' src='img/udisk-autoplay.png'><h1 id='Autoplay_tit' style='width: 65%;'>Airdisk Autoplay</h1><span class='ui-li-count ui-body-inherit text' id='Autoplay_txt' style='border-color: rgba(85, 85, 85, 0); border-radius: 0px; width: 30%; text-align: right; right: 2.6em; color: rgb(255, 255, 255); text-shadow: none; background-color: transparent;'></span></a></li>";
    $("#ezair").after(str);
}
//airdiskautoplay button
function airdiskautoplay_listener() {
    $("#set_autoplay_ok").click(function(e) {
        $('#Autoplay_set').popup("close");
        set_airdiskautoplay_text();
    });

    $("#set_autoplay_cel").click(function(e) {
        $('#Autoplay_set').popup("close");
        if ($('#Autoplay_txt').text() == "ON") {
            airdiskautoplay_on_style();
        } else {
            airdiskautoplay_off_style();
        }
    });

}
//set airdiskautoplay css
function set_airdiskautoplay_text() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    var value = $('#Autoplay_val').val();//on/off
    $('#ezchannel_open').val(value);
    $('#ezchannel_cmd').val("SET_AUTOPLAY_ENABLE");
    $.post('cgi-bin/set_autoplay.cgi', $('#ezchanel_data').serialize());
    if (Number(value)) {
        $('#Autoplay_txt').text("ON");
    } else {
        $('#Autoplay_txt').text("OFF");
    }
    //set playway=Local
    value = "1set_ezchannel_playway";
    $.get(cgiurl, {type: value},function(ret) {
    }, "text");

}
//read airdiskautoplay value and set css
function Get_airdiskautoplay_text() {
    var value;
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'getAutoplayEnable'},function(autoplay) {
        value = Number(autoplay);
        $('#Autoplay_val').val(value);
        if (value) {
            $('#Autoplay_txt').text("ON");
            airdiskautoplay_on_style();
        } else {
            $('#Autoplay_txt').text("OFF");
            airdiskautoplay_off_style();
        }
    },"text");
}
//airdiskautoplay set css
function airdiskautoplay_on_style() {
    $("#Autoplay_val").val(1);
    $("#Autoplay_control").children().addClass("ui-flipswitch-active");
    $("#Autoplay_val option").eq(1).attr("selected", true); //on
}
//airdiskautoplay set css
function airdiskautoplay_off_style() {
    $("#Autoplay_val").val(0);
    $("#Autoplay_control").children().removeClass("ui-flipswitch-active");
    $("#Autoplay_val option").eq(0).attr("selected", true); //off
}

//creat case code
function add_casecode_li() {
    var str ="<li class='ui-li-has-count ui-li-has-icon'><a class='ui-btn ui-btn-icon-right ui-icon-carat-r' aria-expanded='false' aria-haspopup='true' aria-owns='castcode_set' style='border-width: 0px 0px 1px; border-style: none none inset; border-color: currentColor currentColor rgb(183, 183, 187); border-image: none; color: rgb(255, 255, 255); font-weight: normal; text-shadow: none; background-color: transparent;' href='#castcode_set' data-position-to='window' data-rel='popup'><img class='ui-li-icon' alt='' src='img/Castcode_control.png'><h1 id='txt_castcode' style='width: 65%;'>passcode control</h1></a></li>";
    //$("#hdmi_cec").after(str);
    var ul = document.getElementById("list_load_ul");
    ul.innerHTML = ul.innerHTML + str;
}

function add_miracode_li() {
    var str ="<li class='ui-li-has-count ui-li-has-icon'><a class='ui-btn ui-btn-icon-right ui-icon-carat-r' aria-expanded='false' aria-haspopup='true' aria-owns='miracode_set' style='border-width: 0px 0px 1px; border-style: none none inset; border-color: currentColor currentColor rgb(183, 183, 187); border-image: none; color: rgb(255, 255, 255); font-weight: normal; text-shadow: none; background-color: transparent;' href='#miracode_set' data-position-to='window' data-rel='popup'><img class='ui-li-icon' alt='' src='img/Miracode_control.png'><h1 id='txt_miracode' style='width: 65%;'>Miracode control</h1><span class='ui-li-count ui-body-inherit text' id='miracode_txt' style='border-color: rgba(85, 85, 85, 0); border-radius: 0px; width: 30%; text-align: right; right: 2.6em; color: rgb(255, 255, 255); text-shadow: none; background-color: transparent;'></span></a></li>";
    //$("#hdmi_cec").after(str);
    var ul = document.getElementById("list_load_ul");
    ul.innerHTML = ul.innerHTML + str;
}

//creat air view on/off
function add_airview_li() {
    var str ="<li class='ui-li-has-count ui-li-has-icon' id='airview_sel'><a class='ui-btn ui-btn-icon-right ui-icon-carat-r' aria-expanded='false' aria-haspopup='true' aria-owns='airview_set' style='border-width: 0px 0px 1px; border-style: none none inset; border-color: currentColor currentColor rgb(183, 183, 187); border-image: none; color: rgb(255, 255, 255); font-weight: normal; text-shadow: none; background-color: transparent;' href='#airview_set' data-position-to='window' data-rel='popup'><img class='ui-li-icon' alt='' src='img/Air_View.png'><h1 id='airview_1_title' style='width: 65%;'>AirView</h1><span class='ui-li-count ui-body-inherit text' id='airview_txt' style='border-color: rgba(85, 85, 85, 0); border-radius: 0px; width: 30%; text-align: right; right: 2.6em; color: rgb(255, 255, 255); text-shadow: none; background-color: transparent;'></span></a></li>";
    //$("#hdmi_cec").after(str);
    var ul = document.getElementById("list_load_ul");
    ul.innerHTML = ul.innerHTML + str;
}
//airview button
function airview_listener() {
    $("#set_airview_ok").click(function(e) {
        $('#airview_set').popup("close");
        set_airview_text();
    });

    $("#set_airview_cel").click(function(e) {
        $('#airview_set').popup("close");
        if ($('#airview_txt').text() == "ON") {
            airview_on_style();
        } else {
            airview_off_style();
        }
    });

}
	//set airview css
function set_airview_text() {
	var cgiurl= "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); //To solve the IE cache ,by cxf
	var	value=$('#air_sw').val();
	var str=value+"\n"+"set_airview\n";
	$.get(cgiurl, {type:str}, function(ret)	{	
	}, "text");
	if (Number(value)) {
	    $('#airview_txt').text("ON");
	} else {
	    $('#airview_txt').text("OFF");
	}
	var that = $("#rate_val").val();
	var str=that+"\nset\nairview_rate\nCONFNAME_WR\n";
	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
	$.get(cgiurl, {type:str}, function(ret)
	{	
	}, "text");
    	airview_refershTime = Number(that);
    	$("#rate_val").find("option[value="+airview_refershTime+"]").attr("selected",true); 
	$("#rate_val-button").children("span").text($("#rate_val").find("option[value="+airview_refershTime+"]").text());	

}
//read airview value and set css
function Get_airview_text() {
    var value;
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type:"get_airview"}, function(ret)
    {	
        value=Number(ret);
        //$('#air_sw').val(value);
        if (value) {
            $('#airview_txt').text("ON");
            airview_on_style();
        } else {
            $('#airview_txt').text("OFF");
            airview_off_style();
        }
	airview_rate_time_value();
    }, "text");
}
//read rate time
function airview_rate_time_value(){

	var value;
	var cgiurl = "cgi-bin/wifi_info_GET.cgi";
	$.get(cgiurl, {type:'1\nhost_val\nconferent\nCONFNAME_WR\n'}, function(ret)
	//$.get(cgiurl, {type:'host_val'}, function(ret)
	{
		value=ret;
		host_authority_array=value.split(",");

		airview_refershTime=Number(host_authority_array[8]); //airview_rate
		$('#rate_val').val(airview_refershTime);
	    	$("#rate_val").find("option[value="+airview_refershTime+"]").attr("selected",true); 
	    	$("#rate_val-button").children("span").text($("#rate_val").find("option[value="+airview_refershTime+"]").text());	
		/*
		if (airview_refershTime == 5000) {
		    //$("#rate_val-button").children("span").text("5 seconds");
		    $("#rate_val-button").children("span").text($("#rate_val option").eq(0).text());
		} else if (airview_refershTime == 15000) {
		    //$("#rate_val-button").children("span").text("15 seconds");
		    $("#rate_val-button").children("span").text($("#rate_val option").eq(1).text());
		} else if (airview_refershTime == 30000) {
		    //$("#rate_val-button").children("span").text("30 seconds");
		    $("#rate_val-button").children("span").text($("#rate_val option").eq(2).text());
		} else if (airview_refershTime == 60000) {//300-256
		    //$("#rate_val-button").children("span").text("1 minute");
		    $("#rate_val-button").children("span").text($("#rate_val option").eq(3).text());
		} else {
		    //$("#rate_val-button").children("span").text("Manual update");
		    $("#rate_val-button").children("span").text($("#rate_val option").eq(4).text());
		}
		*/

	}, "text");

}
//airview set css
function airview_on_style() {
    $("#air_sw").val(1);
    $("#switch_onoff").children().addClass("ui-flipswitch-active");
    $("#air_sw option").eq(1).attr("selected",true);  //on
}
//airview set css
function airview_off_style() {
    $("#air_sw").val(0);
    $("#switch_onoff").children().removeClass("ui-flipswitch-active");
    $("#air_sw option").eq(0).attr("selected",true);  //off
}

//creat Audio Output
function add_ezwire_audio_li() {
    var str = "<li class='ui-li-has-count ui-li-has-icon' id='Audio_Output'><a class='ui-btn ui-btn-icon-right ui-icon-carat-r' aria-expanded='false' aria-haspopup='true' aria-owns='ezwire_audio' style='border-width: 0px 0px 1px; border-style: none none inset; border-color: currentColor currentColor rgb(183, 183, 187); border-image: none; color: rgb(255, 255, 255); font-weight: normal; text-shadow: none; background-color: transparent;' href='#ezwire_audio' data-position-to='window' data-rel='popup'><img class='ui-li-icon' alt='Resolution' src='img/ezwireaudio.png'><h1 id='ezwire_audio_tit' style='width: 65%;'>Audio_Output</h1><span class='ui-li-count ui-body-inherit text' id='ezwire_audio_txt' style='border-color: rgba(85, 85, 85, 0); border-radius: 0px; width: 30%; text-align: right; right: 2.6em; color: rgb(255, 255, 255); text-shadow: none; background-color: transparent;'></span></a></li>";
    $("#resolution_all").before(str);
}
//read ezwire_audio text for ui
function get_ezwire_audio__text() {
    $.get("cgi-bin/wifi_info_GET.cgi", {type: 'get_ezwir_text'},function(lan_text_string) {
        if (lan_text_string.length) {
            start = 0;
            text_string_length = 0;
            text_string = lan_text_string;
            text_string_length = lan_text_string.length;
            $('#ezwire_audio_tit').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#ezwire_audio_warn').text(Get_text_from_textstring(text_string));
        } else {
            $('#ezwire_audio_tit').text("Audio Output");
            $('#ezwire_audio_warn').text("For some android devices, USB audio is not supported. Please switch to 'Phone' if no sound or no video comes from the dongle.");
        }
    },"text");
}

//Audio Output button
function ezwire_audio_listener() {
    $("#ezwire_audio_ok").click(function(e) {
        set_ezwire_audio_ui();
        $('#ezwire_audio').popup("close");
    });

    $("#ezwire_audio_cel").click(function(e) {
        if (ezwire_audio_select != ezwire_audio_index) {
            ezwire_audio_select = ezwire_audio_index;
            clean_ezwire_audio_style();
            set_ezwire_audio_style();
        }
        $('#ezwire_audio').popup("close");
    });

    $('input:radio[name="ezwire_audio_item"]').click(function() {
        var that = $(this).val();
        select_txt = $(this).prev().text();
        ezwire_audio_select = that;
    });

}
//set ezwire_audio value
function set_ezwire_audio_ui() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    var str = ezwire_audio_select + "\n" + "set_ezwireaudio\n";
    $.get(cgiurl, {type: str},function(ret) {
    }, "text");
    $("#ezwire_audio_txt").text(select_txt);
    ezwire_audio_index = ezwire_audio_select;
}
//read ezwire_audio value
function Get_ezwire_audio_ui() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_ezwireaudio'},function(ret) {
        var value = Number(ret);
        ezwire_audio_index = value;
        ezwire_audio_select = ezwire_audio_index;
        set_ezwire_audio_style();
    }, "text");
}
//set ezwire_audio css
function set_ezwire_audio_style() {
    var that = $("input[type=radio][name=ezwire_audio_item][value=" + ezwire_audio_index + "]");
    that.attr("checked", "true");
    that.prev().removeClass("ui-radio-off");
    that.prev().addClass("ui-radio-on");
    select_txt = that.prev().text();
    $("#ezwire_audio_txt").text(select_txt);
}
//set ezwire_audio css
function clean_ezwire_audio_style() {
    $('input:radio[name="ezwire_audio_item"]').each(function() {
        $(this).prev().addClass("ui-radio-off");
        $(this).prev().removeClass("ui-radio-on");
        $(this).attr("checked", " ");
    });
}
//creat ezair_list
function add_ezair_li() {
    var str = "<li class='ui-li-has-count ui-li-has-icon' id='ezair'><a class='ui-btn ui-btn-icon-right ui-icon-carat-r' aria-expanded='false' aria-haspopup='true' aria-owns='ezairmode' style='border-width: 0px 0px 1px; border-style: none none inset; border-color: currentColor currentColor rgb(183, 183, 187); border-image: none; color: rgb(255, 255, 255); font-weight: normal; text-shadow: none; background-color: transparent;' href='#ezairmode' data-position-to='window' data-rel='popup'><img class='ui-li-icon' id='ezairmode_icon' alt='ezairmode' src='img/ezair2.png'><h1 id='ezairmode_title' style='width: 65%;'>EZAir</h1><span class='ui-li-count ui-body-inherit text' id='ezairmode_txt' style='border-color: rgba(85, 85, 85, 0); border-radius: 0px; width: 30%; text-align: right; right: 2.6em; color: rgb(255, 255, 255); text-shadow: none; background-color: transparent;'></span></a></li>";
    $("#resolution_all").after(str);
}
//ezair button
function ezairmode_listener() {
    $("#ezairmode_ok").click(function(e) {
        set_ezairmode_ui();
        $('#ezairmode').popup("close");
    });

    $("#ezairmode_cel").click(function(e) {
        if (ezairmode_select != ezairmode_index) {
            ezairmode_select = ezairmode_index;
            clean_ezairmode_style();
            set_ezairmode_style();
        }
        $('#ezairmode').popup("close");
    });

    $('input:radio[name="ezairmodemode_item"]').click(function() {
        var that = $(this).val();
        select_txt = $(this).prev().text();
        ezairmode_select = that;
    });

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
//read ezair value
function Get_ezairmode_ui() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_ezairmode'},function(ret) {
        var value = Number(ret);
        ezairmode_index = value;
        ezairmode_select = ezairmode_index;
        set_ezairmode_style();
    }, "text");
}
//set ezari css
function set_ezairmode_style() {
    var that = $("input[type=radio][name=ezairmodemode_item][value=" + ezairmode_index + "]");
    that.attr("checked", "true");
    that.prev().removeClass("ui-radio-off");
    that.prev().addClass("ui-radio-on");
    select_txt = that.prev().text();
    $("#ezairmode_txt").text(select_txt);
}
//set ezari css
function clean_ezairmode_style() {
    $('input:radio[name="ezairmodemode_item"]').each(function() {
        $(this).prev().addClass("ui-radio-off");
        $(this).prev().removeClass("ui-radio-on");
        $(this).attr("checked", " ");
    });
}

//creat ignoressid list
function add_ignoressid_li() {
    var str = "<li class='ui-li-has-count ui-li-has-icon' id='ignoressid_sel'><a class='ui-btn ui-btn-icon-right ui-icon-carat-r' aria-expanded='false' aria-haspopup='true' aria-owns='Hidessid' style='border-width: 0px 0px 1px; border-style: none none inset; border-color: currentColor currentColor rgb(183, 183, 187); border-image: none; color: rgb(255, 255, 255); font-weight: normal; text-shadow: none; background-color: transparent;' href='#ignoressidmode' data-position-to='window' data-rel='popup'><img class='ui-li-icon' id='ignoressidmode_icon' alt='hidessidmode' src='img/Hide_ssid.png'><h1 id='hidessidmode_title' style='width: 65%;'>Hide SSID</h1><span class='ui-li-count ui-body-inherit text' id='ignoressidmode_txt' style='border-color: rgba(85, 85, 85, 0); border-radius: 0px; width: 30%; text-align: right; right: 2.6em; color: rgb(255, 255, 255); text-shadow: none; background-color: transparent;'></span></a></li>";
    $("#upgrade").before(str);
}
//ignoressid button
function ignoressidmode_listener() {
    $("#ignoressid_ok").click(function(e) {
	var ignoressid_value=Number($("#ignoressid_val").val());
	 if (ignoressid_value != ignore_ssid_index) {
            ignore_ssid_index = ignoressid_value;
	     set_ignoressid_value();
	     set_ignoressidmode_ui();
        }
        $('#ignoressidmode').popup("close");
    });

    $("#ignoressid_cel").click(function(e) {      
        $('#ignoressidmode').popup("close");
	 if (ignore_ssid_index) {
            ignoressid_on_style();
        } else {
            ignoressid_off_style();
        }
    });


}
//ignoressid css
function ignoressid_on_style(){
    $("#ignoressid_val").val(1);
    $("#ignoressid").children().addClass("ui-flipswitch-active");
    $("#ignoressid_val option").eq(1).attr("selected",true);  //on
}
//ignoressid css
function ignoressid_off_style(){
    $("#ignoressid_val").val(0);
    $("#ignoressid").children().removeClass("ui-flipswitch-active");
    $("#ignoressid_val option").eq(0).attr("selected",true);  //off
}
function set_ignoressid_value(){
    if(ignore_ssid_index)
	 $("#ignoressidmode_txt").text("ON");	
    else
	 $("#ignoressidmode_txt").text("OFF");
			
}
//set ignoressid value
function set_ignoressidmode_ui() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    var str = ignore_ssid_index+ "set_ignoressid";
    $.get(cgiurl, {type: str},function(ret) {
    }, "text");
   
}
//read ignoressid value
function Get_ignoressidmode_ui() {
    var value;
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type:'get_ignoressid'}, function(ret) {	
        value=Number(ret);
	  ignore_ssid_index=value;
	  set_ignoressid_value();
        $('#ignoressid_val').val(ret);
        if(value){
            ignoressid_on_style();
        }else{
            ignoressid_off_style();
        }
    }, "text");	
}
//creat softapconnect
function add_softapconnect_li() {
    var str ="<li class='ui-li-has-count ui-li-has-icon ui-last-child' id='softap_conmun'><a class='ui-btn ui-btn-icon-right ui-icon-carat-r' aria-expanded='false' aria-haspopup='true' aria-owns='softapconnect' style='border-width: 0px 0px 1px; border-style: none none inset; border-color: currentColor currentColor rgb(183, 183, 187); border-image: none; color: rgb(255, 255, 255); font-weight: normal; text-shadow: none; background-color: transparent;' href='#softapconnect' data-position-to='window' data-rel='popup'><img class='ui-li-icon' alt='softapconntmun' src='img/max_connections.png'><h1 id='softapconnect_title' style='width: 65%;'>Max Connections</h1><span class='ui-li-count ui-body-inherit text' id='softapconnect_txt' style='border-color: rgba(85, 85, 85, 0); border-radius: 0px; width: 30%; text-align: right; right: 2.6em; color: rgb(255, 255, 255); text-shadow: none; background-color: transparent;'></span></a></li>";
    //$("#hdmi_cec").after(str);
    var ul = document.getElementById("list_load_ul");
    ul.innerHTML = ul.innerHTML + str;
}
//softapconnect button
function softapconnect_listener() {
    $("#softapconnect_ok").click(function(e) {
        set_softapconnect_ui();
        $('#softapconnect').popup("close");
    });

    $("#softapconnect_cel").click(function(e) {
        if (softapconnect_select != softapconnect_index) {
            softapconnect_select = softapconnect_index;
            clean_softapconnect_style();
            set_softapconnect_style();
        }
        $('#softapconnect').popup("close");
    });

    $('input:radio[name="softapconnect_item"]').click(function() {
        var that = $(this).val();
        softapconnect_select_txt = $(this).prev().text();
        softapconnect_select = that;
        if(Number(that)>16)
            $("#softapconnect_warn").text(MAXUSER_WARN_string);//"Note: The more users are linking, the lower bandwidth they may have."
        else
            $("#softapconnect_warn").text("");
    });

}
//set softapconnect value
function set_softapconnect_ui() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    var str = softapconnect_select+"set_max_num";
    $.get(cgiurl, {type: str},function(ret) {
    }, "text");
    $("#softapconnect_txt").text(softapconnect_select_txt);
    softapconnect_index = softapconnect_select;
}
//read softapconnect value
function Get_softapconnect_ui() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_max_num'},function(ret) {
        var value = Number(ret);
        if(value<16){
            value =16;
            var str = value+"set_max_num";
            $.get(cgiurl, {type: str},function(ret) {
            }, "text");
        }
        softapconnect_index = value;
        softapconnect_select = softapconnect_index;
        set_softapconnect_style();
    }, "text");
}
//set softapconnect css
function set_softapconnect_style() {
    var that = $("input[type=radio][name=softapconnect_item][value=" + softapconnect_index + "]");
    that.attr("checked", "true");
    that.prev().removeClass("ui-radio-off");
    that.prev().addClass("ui-radio-on");
    softapconnect_select_txt = that.prev().text();
    $("#softapconnect_txt").text(softapconnect_select_txt);
    if(Number(softapconnect_index)>16)
        $("#softapconnect_warn").text("The more users are linking, the lower bandwidth they may have.");
    else
        $("#softapconnect_warn").text("");
}
//set softapconnect css
function clean_softapconnect_style() {
    $('input:radio[name="softapconnect_item"]').each(function() {
        $(this).prev().addClass("ui-radio-off");
        $(this).prev().removeClass("ui-radio-on");
        $(this).attr("checked", " ");
    });
}

//read password text
function Get_set_password_text() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_password_text'},function(OTA_string) {
        if (OTA_string.length) {
            start = 0;
            text_string_length = 0;
            text_string = OTA_string;
            text_string_length = OTA_string.length;
            PSW_LEN_string = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            PSW_ALLOW_string = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            LIST_WARN2_string = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            SET_AP_SSID_WARN_CHAR_string = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            SET_AP_SSID_WARN_LEN_string = Get_text_from_textstring(text_string);
            text_string = text_string_global;
        } else {
            PSW_LEN_string = "Password must be between 8 and 15 characters long.";
            PSW_ALLOW_string = "Only the letters and numbers are allowed!";
            LIST_WARN2_string = "Access Point will be disconnected if you press OK.";
            SET_AP_SSID_WARN_CHAR_string = "Only the letters and numbers are allowed!";
            SET_AP_SSID_WARN_LEN_string = "SSID length must be between 1 and 32 characters.";
        }
    },"text");
}
//set password button
function appsk_apply_listener() {
    $('#set_appsk_apply').click(function(e) {
        var regx = /^[A-Za-z0-9]+$/;
        e.preventDefault();
        appskValue = $('#appsk').val();
        if (appskValue != appskValue_save) {
            $('#change_psw_warn').text(LIST_WARN2_string);
            document.getElementById("resolution").style.backgroundImage = 'url(img/bar_cover.png)';
            document.getElementById("select_down_res").src = "img/select-Down_cover.png";
            document.getElementById("select_up_res").src = "img/select-up_cover.png";

            if (!regx.test(appskValue)) {
                $('#change_psw_warn').text(PSW_ALLOW_string);
                show_psw_change_warn();
                errorFlag = 1;
            } else {
                if (appskValue.length < 8 || appskValue.length > 64) {
                    $('#change_psw_warn').text(PSW_LEN_string);
                    show_psw_change_warn();
                    $('#appsk').val("" + appskValue_save);
                    document.getElementById("appsk").style.align = "center";
                    errorFlag = 1;
                } else {
                    errorFlag = 0;
                }
            }
            if (!errorFlag) {
                show_psw_change();
            }
        }
    });
}

function ezchannel_listener() {
    $("#EZchanel").click(function() {
        var chanel = Number($("#EZchanelon").val());
        if (chanel) {
            ezchannel_on_style();
        } else {
            ezchannel_off_style();
        }
    });

    $("#EZchanel").bind("swipeleft", function() {
        ezchannel_off_style();
    });

    $("#EZchanel").bind("swiperight", function() {
        ezchannel_on_style();
    });

    $("#ezchannel_cel").click(function() {
        if (ezchannel_onoff) {
            $("#ezchannel_switch").text("ON");
        } else {
            $("#ezchannel_switch").text("OFF");
        }
    });

    $("#ezchannelmode").click(function() {
        //----	
        if (ezchannel_onoff) {
            $("#ezchannel_switch").text("ON");
            $("#EZchanelon").val(1);
            $("#EZchanel").children().addClass("ui-flipswitch-active");
            $("#EZchanelon option").eq(1).attr("selected", true); //on
            ezchannel_on_style();
        } else {
            $("#ezchannel_switch").text("OFF");
            $("#EZchanelon").val(0);
            $("#EZchanel").children().removeClass("ui-flipswitch-active");
            $("#EZchanelon option").eq(0).attr("selected", true); //off
            ezchannel_off_style();
        }
        //----
        $('#ezch_points').val(volume_val);
        var tmp_solid = (volume_val * 100) / 15;
        $('.ui-slider-bg').css("width", tmp_solid + "%");
        $('.ui-slider-handle').css("left", tmp_solid + "%");
        //--
        $("#ezch_waittime").val(waitime_value);
        if (waitime_value == 10) {
            $("#ezch_waittime-button").children("span").text("10 seconds");
        } else if (waitime_value == 30) {
            $("#ezch_waittime-button").children("span").text("30 seconds");
        } else if (waitime_value == 60) {
            $("#ezch_waittime-button").children("span").text("1 minutes");
        } else if (waitime_value == 300) { //300-256
            $("#ezch_waittime-button").children("span").text("5 minutes");
        }
        //---
        if (playlist_val) {
            $('#ezch_list').val(1);
            $('#ezch_list').parent().addClass("ui-flipswitch-active");
            $("#ezch_list option").eq(1).attr("selected", true); //on
        } else {
            $('#ezch_list').val(0);
            $('#ezch_list').parent().removeClass("ui-flipswitch-active");
            $("#ezch_list option").eq(0).attr("selected", true); //off
        }
        //---
        if (ssid_val) {
            $('#ezch_ssid').val(1);
            $('#ezch_ssid').parent().addClass("ui-flipswitch-active");
            $("#ezch_ssid option").eq(1).attr("selected", true); //on
        } else {
            $('#ezch_ssid').val(0);
            $('#ezch_ssid').parent().removeClass("ui-flipswitch-active");
            $("#ezch_ssid option").eq(0).attr("selected", true); //off
        }
    });

    $("#ezchannel_ok").click(function() {
        var temp;
        temp = $('#EZchanelon').val(); //on/off
        $('#ezchannel_open').val(temp);
        ezchannel_onoff = Number(temp);
        ezchannel_enable = ezchannel_onoff;
        if (ezchannel_onoff) {
            $("#ezchannel_switch").text("ON");
        } else {
            $("#ezchannel_switch").text("OFF");
        }
        $('#ezchannel_cmd').val("SET_AUTOPLAY_ENABLE");
        $.post('cgi-bin/set_autoplay.cgi', $('#ezchanel_data').serialize());

        temp = $('#ezch_points').val(); 
        $('#ezchannel_open').val(temp);
        ret = $('#ezchannel_open').val();
        volume_val = Number(ret);
        $('#ezchannel_cmd').val("SET_AUTOPLAY_VOLUME");
        $.post('cgi-bin/set_autoplay.cgi', $('#ezchanel_data').serialize());

        temp = $('#ezch_waittime').val(); 
        $('#ezchannel_open').val(temp);
        ret = $('#ezchannel_open').val();
        waitime_value = Number(ret);
        $('#ezchannel_cmd').val("SET_AUTOPLAY_WAITIME");
        $.post('cgi-bin/set_autoplay.cgi', $('#ezchanel_data').serialize());

        temp = $('#ezch_list').val(); //playlist
        $('#ezchannel_open').val(temp);
        ret = $('#ezchannel_open').val();
        playlist_val = Number(ret);
        $('#ezchannel_cmd').val("SET_AUTOPLAY_LIST");
        $.post('cgi-bin/set_autoplay.cgi', $('#ezchanel_data').serialize());

        $('#ezchannel_cmd').val("SET_AUTOPLAY_HOST_AP"); // host ap
        $.post('cgi-bin/set_autoplay.cgi', $('#ezchanel_data').serialize());

        temp = $('#ezch_ssid').val(); //ssid
        $('#ezchannel_open').val(temp);
        ret = $('#ezchannel_open').val();
        ssid_val = Number(ret);
        $('#ezchannel_cmd').val("SET_AUTOPLAY_PROGRESSIVE");
        $.post('cgi-bin/set_autoplay.cgi', $('#ezchanel_data').serialize());

        var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
        temp = $('#ezch_play_way').val() + "set_ezchannel_playway";
        $.get(cgiurl, {type: temp},function(ret) {
		}, "text");

    });

}
function ezchannel_off_style() {
    $('#ezch_points').attr("disabled");
    $('#ezch_points').addClass("ui-state-disabled");
    $('#ezch_points').addClass("mobile-slider-disabled");
    $('#ezch_points').next().addClass("ui-state-disabled");
    $('#ezch_points').attr("aria-disabled", "true");
    $('#ezch_waittime-button').addClass("ui-state-disabled");
    $('#ezch_waittime-button').attr("aria-disabled", "true");
    $('#ezch_play_way-button').addClass("ui-state-disabled");
    $('#ezch_play_way-button').attr("aria-disabled", "true");
    $('#ezch_list').parent().addClass("ui-state-disabled mobile-slider-disabled");
    $('#ezch_ssid').parent().addClass("ui-state-disabled mobile-slider-disabled");
}

function ezchannel_on_style() {
    $('#ezch_points').removeAttr("disabled");
    $('#ezch_points').removeClass("ui-state-disabled");
    $('#ezch_points').removeClass("mobile-slider-disabled");
    $('#ezch_points').next().removeClass("ui-state-disabled");
    $('#ezch_points').removeAttr("aria-disabled");

    $('#ezch_waittime-button').removeClass("mobile-slider-disabled");
    $('#ezch_waittime-button').removeClass("ui-state-disabled");
    $('#ezch_waittime-button').removeAttr("aria-disabled");

    $('#ezch_play_way-button').removeClass("mobile-slider-disabled");
    $('#ezch_play_way-button').removeClass("ui-state-disabled");
    $('#ezch_play_way-button').removeAttr("aria-disabled");
    $('#ezch_list').parent().removeClass("ui-state-disabled mobile-slider-disabled");
    $('#ezch_ssid').parent().removeClass("ui-state-disabled mobile-slider-disabled");

}
function Get_ezchannel() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'getAutoplayEnable'},function(ezchannel) {
        $('#ezchannel_open').val(ezchannel);
        var ret = $('#ezchannel_open').val();
        ezchannel_enable = Number(ret);
        if (ezchannel_enable) {
            $("#ezchannel_switch").text("ON");
            $("#EZchanelon").val(1);
            $("#EZchanel").children().addClass("ui-flipswitch-active");
            $("#EZchanelon option").eq(1).attr("selected", true); //on
            ezchannel_onoff = 1;
            ezchannel_on_style();
        }
        else {
            $("#ezchannel_switch").text("OFF");
            $("#EZchanelon").val(0);
            $("#EZchanel").children().removeClass("ui-flipswitch-active");
            $("#EZchanelon option").eq(0).attr("selected", true); //off
            ezchannel_onoff = 0;
            ezchannel_off_style();
        }
    },"text");

}

function Get_ezchannel_volume() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'getAutoplayVolume'},function(ezchannel_volume) {
        $('#ezch_points').val(ezchannel_volume);
        volume_val = $('#ezch_points').val();
        var tmp_solid = (ezchannel_volume * 100) / 15;
        $('.ui-slider-bg').css("width", tmp_solid + "%");
        $('.ui-slider-handle').css("left", tmp_solid + "%");
    },"text");
}

function Get_ezchannel_waittime() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'getAutoplayWaitime'},function(ezchannel_waittime) {
        $('#ezch_waittime').val(ezchannel_waittime);
        var ret = $('#ezch_waittime').val();
        waitime_value = Number(ret);
        if (waitime_value == 10) {
            $("#ezch_waittime-button").children("span").text("10 seconds");
        } else if (waitime_value == 30) {
            $("#ezch_waittime-button").children("span").text("30 seconds");
        } else if (waitime_value == 60) {
            $("#ezch_waittime-button").children("span").text("1 minutes");
        } else if (waitime_value == 300) {//300-256
        	$("#ezch_waittime-button").children("span").text("5 minutes");
        }
    },"text");

}
function Get_ezchannel_playway() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_ezchannel_playway'},function(ret) {
        $('#ezch_play_way').val(ret);
        var val = Number(ret);
        if (val == 0) {
            $("#ezch_play_way-button").children("span").text("Internet");
        } else if (val == 1) {
            $("#ezch_play_way-button").children("span").text("Local");
        }
    },"text");

}
function Get_ezchannel_playlist() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'getAutoplaylist'},function(ezchannel_playlist) {
        var ret = Number(ezchannel_playlist);
        playlist_val = ret;
        if (ret) {
            $('#ezch_list').val(1);
            $('#ezch_list').parent().addClass("ui-flipswitch-active");
            $("#ezch_list option").eq(1).attr("selected", true); //on
        } else {
            $('#ezch_list').val(0);
            $('#ezch_list').parent().removeClass("ui-flipswitch-active");
            $("#ezch_list option").eq(0).attr("selected", true); //off
        }
    },"text");
}
function Get_ezchannel_ssid() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'getssid'},function(ezchannel_ssid) {
        var ret = Number(ezchannel_ssid);
        ssid_val = ret;
        if (ret) {
            $('#ezch_ssid').val(1);
            $('#ezch_ssid').parent().addClass("ui-flipswitch-active");
            $("#ezch_ssid option").eq(1).attr("selected", true); //on
        } else {
            $('#ezch_ssid').val(0);
            $('#ezch_ssid').parent().removeClass("ui-flipswitch-active");
            $("#ezch_ssid option").eq(0).attr("selected", true); //off
        }
    },"text");
}

function show_ezchannel() {
    var scroll_left = document.body.scrollLeft;
    var scroll_top = document.body.scrollTop;
    document.getElementById("ezchannel_all").style.display = "block";
    document.getElementById("ezchannel_all").style.position = "fixed";
    document.getElementById("ezchannel_all").style.left = (screen_width - 344) / 2 + "px";
    document.getElementById("ezchannel_all").style.top = (screen_height - 236) / 2 + "px";
    document.getElementById("fade").style.display = "block";
    if (ezchannel_enable) {
        document.getElementById("ezchannel_choose").style.backgroundImage = 'url(img/ezchanel_on.png)';
        $('#ezchannel_choose').show();
        $('#ezchannel_volume_all').show();
        $('#ezchannel_waittime_all').show();
    } else {
        document.getElementById("ezchannel_choose").style.backgroundImage = 'url(img/ezchanel_off.png)';
        $('#ezchannel_choose').show();
        $('#ezchannel_volume_all').hide();
        $('#ezchannel_waittime_all').hide();
    }

}

function show_waittime() {
    var scroll_left = document.body.scrollLeft;
    var scroll_top = document.body.scrollTop;
    document.getElementById("waitime_all").style.display = "block";
    document.getElementById("waitime_all").style.position = "fixed";
    document.getElementById("waitime_all").style.left = (screen_width - 64) / 2 + "px";
    document.getElementById("waitime_all").style.top = (screen_height + 70) / 2 + "px";
    document.getElementById("fade").style.display = "block";
    if (waitime_value == 10) {
        document.getElementById("ezchannel_waitime_10s").style.backgroundImage = 'url(img/dropdownbox_blue.png)';
        document.getElementById("ezchannel_waitime_30s").style.backgroundImage = 'url(img/dropdownbox_bg.png)';
        document.getElementById("ezchannel_waitime_1min").style.backgroundImage = 'url(img/dropdownbox_bg.png)';
        document.getElementById("ezchannel_waitime_5min").style.backgroundImage = 'url(img/dropdownbox_bg.png)';
    } else if (waitime_value == 30) {
        document.getElementById("ezchannel_waitime_10s").style.backgroundImage = 'url(img/dropdownbox_bg.png)';
        document.getElementById("ezchannel_waitime_30s").style.backgroundImage = 'url(img/dropdownbox_blue.png)';
        document.getElementById("ezchannel_waitime_1min").style.backgroundImage = 'url(img/dropdownbox_bg.png)';
        document.getElementById("ezchannel_waitime_5min").style.backgroundImage = 'url(img/dropdownbox_bg.png)';
    } else if (waitime_value == 60) {
        document.getElementById("ezchannel_waitime_10s").style.backgroundImage = 'url(img/dropdownbox_bg.png)';
        document.getElementById("ezchannel_waitime_30s").style.backgroundImage = 'url(img/dropdownbox_bg.png)';
        document.getElementById("ezchannel_waitime_1min").style.backgroundImage = 'url(img/dropdownbox_blue.png)';
        document.getElementById("ezchannel_waitime_5min").style.backgroundImage = 'url(img/dropdownbox_bg.png)';
    } else if (waitime_value == 300) {
        document.getElementById("ezchannel_waitime_10s").style.backgroundImage = 'url(img/dropdownbox_bg.png)';
        document.getElementById("ezchannel_waitime_30s").style.backgroundImage = 'url(img/dropdownbox_bg.png)';
        document.getElementById("ezchannel_waitime_1min").style.backgroundImage = 'url(img/dropdownbox_bg.png)';
        document.getElementById("ezchannel_waitime_5min").style.backgroundImage = 'url(img/dropdownbox_blue.png)';
    }
    $('#ezchannel_waitime_10s').val("10 seconds");
    $('#ezchannel_waitime_30s').val("30 seconds");
    $('#ezchannel_waitime_1min').val("1 minutes");
    $('#ezchannel_waitime_5min').val("5 minutes");
    $('#ezchannel_waitime_10s').show();
    $('#ezchannel_waitime_30s').show();
    $('#ezchannel_waitime_5min').show();
    $('#ezchannel_waitime_1min').show();
}
function change_waitime(cmd) {
    var ret;
    document.getElementById("fade").style.display = "none";
    $('#ezchannel_cmd').val("SET_AUTOPLAY_WAITIME");
    switch (cmd) {
        case 0:
            $('#ezchannel_waittime').val("10 seconds");
            $('#ezchannel_open').val("10");
            document.getElementById("waitime_all").style.display = "none";
            break;
        case 1:
            $('#ezchannel_waittime').val("30 seconds");
            $('#ezchannel_open').val("30");
            document.getElementById("waitime_all").style.display = "none";
            break;
        case 2:
            $('#ezchannel_waittime').val("1 minutes");
            $('#ezchannel_open').val("60");
            document.getElementById("waitime_all").style.display = "none";
            break;
        case 3:
            $('#ezchannel_waittime').val("5 minutes");
            $('#ezchannel_open').val("300");
            document.getElementById("waitime_all").style.display = "none";
            break;
    }
    ret = $('#ezchannel_open').val();
    waitime_value = Number(ret);
    $.post('cgi-bin/set_autoplay.cgi', $('#config-form').serialize());

}
//set ezchannel value
function change_ezchannel(cmd, val) {
    var ret;
    document.getElementById("fade").style.display = "none";

    switch (cmd) {
        case 0:
            $('#ezchannel_cmd').val("SET_AUTOPLAY_ENABLE");
            if (ezchannel_enable) {
                $('#ezchannel_open').val("0");
                document.getElementById("ezchannel_choose").style.backgroundImage = 'url(img/ezchanel_off.png)';
                document.getElementById("ezchannel").style.backgroundImage = 'url(img/off.png)';
                $('#ezchannel_volume_all').hide();
                $('#ezchannel_waittime_all').hide();
                ezchannel_enable = 0;
            } else {
                $('#ezchannel_open').val("1");
                document.getElementById("ezchannel_choose").style.backgroundImage = 'url(img/ezchanel_on.png)';
                document.getElementById("ezchannel").style.backgroundImage = 'url(img/on.png)';
                $('#ezchannel_volume_all').show();
                $('#ezchannel_waittime_all').show();
                ezchannel_enable = 1;
            }
            $("#ezchannel_choose").show();
            $.post('cgi-bin/set_autoplay.cgi', $('#config-form').serialize());
            break;
        case 1:
            if (val == 1) {
                ret = $('#ezchannel_volume').val();
                ret++;
                if (ret > 15) ret = 15;
                $('#ezchannel_volume').val(ret);
                $('#ezchannel_cmd').val("SET_AUTOPLAY_VOLUME");
                $('#ezchannel_open').val(ret);
                document.getElementById("ezchannel_volume_minus").src = "img/minus.png";
                document.getElementById("ezchannel_volume_plus").src = "img/plus_sel.png";
            } else if (val == 0) {
                ret = $('#ezchannel_volume').val();
                ret--;
                if (ret < 0) ret = 0;
                $('#ezchannel_volume').val(ret);
                $('#ezchannel_cmd').val("SET_AUTOPLAY_VOLUME");
                $('#ezchannel_open').val(ret);
                document.getElementById("ezchannel_volume_minus").src = "img/minus_sel.png";
                document.getElementById("ezchannel_volume_plus").src = "img/plus.png";
            } else if (val == 2) {
                document.getElementById("ezchannel_volume_plus").src = "img/plus.png";
            } else if (val == 3) {
                document.getElementById("ezchannel_volume_minus").src = "img/minus.png";
            }
            $("#ezchannel_volume").show();
            $.post('cgi-bin/set_autoplay.cgi', $('#config-form').serialize());
            break;
        case 2:
            show_waittime();
            break;
        case 3:
            document.getElementById("ezchannel_all").style.display = "none";
            break;
    }
}
//ezchannel button
function show_ezchannel_listener() {
    $('#ezchannel').click(function(e) {
        e.preventDefault();
        show_ezchannel();
    });
}

function Set_router_listener() //click response
{
    $("#set_connection_ok").click(function() {
        if (connection_mode_index != current_connection_mode_index) {
            if (0 == connection_mode_index) //router
            {
                $('#con_dialog_msg').text(router_warn_string);
                $('#con_dialog_msg2').text(router_warn_string2);
            } else if (1 == connection_mode_index) {
                $('#con_dialog_msg').text(direct_warn_string);
                $('#con_dialog_msg2').text("");
            } else if (2 == connection_mode_index) {
                $('#con_dialog_msg').text(onlyrouter_w_arn_string);
                $('#con_dialog_msg2').text(onlyrouter_w_arn_string2);
            }
        }
        $("#con_sel").css('display', 'none');
        $("#con_warn").css('display', 'block');
    });

    $("#connection_ent").click(function() {
        current_connection_mode_index = connection_mode_index;
        if (0 == current_connection_mode_index) {
            if (1 == connected_ssid_flag) {
                $('#ip0_txt').show();
                $('#ip0').show();
            }
        } else {
            $('#ip0_txt').hide();
            $('#ip0').hide();
        }
        switch (connection_mode_index) {
            case 0:
                $('#connection_mode_value').val("1");
                $('#connection_mode').text(router_string);
                $("#connection_mode_icon").attr("src", "img/connection_mode_router1.png");
                break;
            case 1:
                $('#connection_mode_value').val("0");
                $('#connection_mode').text(direct_string);
                $("#connection_mode_icon").attr("src", "img/connection_mode_direct1.png");
                break;
            case 2:
                $('#connection_mode_value').val("2");
                $('#connection_mode').text(onlyrouter_string); //"Via Router Only"
                $("#connection_mode_icon").attr("src", "img/connection_mode_router1.png");
                break;
        }
        $.post('cgi-bin/set_router_ctl_POST.cgi', $('#config-form').serialize());
        $("#connection_sel").popup('close');

    });

    $("#connection_cancle").click(function() {
        $("#connection_sel").popup('close');
    });

    $('input:radio[name="connection_choose"]').click(function() {
        var that = $(this).val();
        var b = $(this).prev().text();
        connection_mode_index = Number(that);
        $('#con_text').text(connection_mode_index);
    });

    $("#set_connection_cal").click(function() {
        connection_mode_index = current_connection_mode_index;
    });

    $('#connectionmode').click(function() {
        $('input:radio[name="connection_choose"]').each(function() {
            $(this).prev().addClass("ui-radio-off");
            $(this).prev().removeClass("ui-radio-on");
            $(this).attr("checked", " ");
        });

        set_routerradio();
        $("#con_sel").css('display', 'block');
        $("#con_warn").css('display', 'none');
    });

}

function set_router_only_text() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    var value = $('#router_only_val').val();
    var str = value + "\n" + "set_routeronly\n";
    if (Number(value)) {
        $('#router_only_txt').text("ON");
    } else {
        $('#router_only_txt').text("OFF");
    }

    $.get(cgiurl, {type: str},function(ret) {
	},"text");

}
function router_on_style() {
    $("#router_only_val").val(1);
    $("#router_only_control").children().addClass("ui-flipswitch-active");
    $("#router_only_val option").eq(1).attr("selected", true); //on
}
function router_off_style() {
    $("#router_only_val").val(0);
    $("#router_only_control").children().removeClass("ui-flipswitch-active");
    $("#router_only_val option").eq(0).attr("selected", true); //off
}
//read router_only_text
function Get_router_only_text() {
    var value;
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_routeronly'},function(ret) {
        value = Number(ret);
        $('#router_only_val').val(value);
        if (value) {
            $('#router_only_txt').text("ON");
            router_on_style();
        } else {
            $('#router_only_txt').text("OFF");
            router_off_style();
        }
    },"text");
}
function Set_router_only_listener() {
    $("#set_routeronly_ok").click(function(e) {
        set_router_only_text();
        $('#router_only_set').popup("close");
    });
    $("#set_routeronly_cel").click(function(e) {
        $('#router_only_set').popup("close");
        if ($('#router_only_txt').text() == "ON") {
            router_on_style();
        } else {
            router_off_style();
        }
    });

}
function set_routerradio() {
    var that = $("input[type=radio][name=connection_choose][value=" + connection_mode_index + "]");
    that.attr("checked", "true");
    that.prev().removeClass("ui-radio-off");
    that.prev().addClass("ui-radio-on");
}
function Creat_router_list() {
    $("label[for=con_0]").text(router_string);
    $("label[for=con_1]").text(direct_string);
    $("label[for=con_2]").text(onlyrouter_string);
    $.get("cgi-bin/wifi_info_GET.cgi", {type: 'ROUTER_ONLY_ENABLE'},function(ret) {
        if (Number(ret) == 0) {
            $("#con_2_txt").remove();
            $('#con_2').remove();
            $("#con_1_txt").addClass("ui-last-child");
        }
    },"text");
    set_routerradio();
}
//read wlan0 ip value
function Get_wlan0_ip_address() {
    $.get("cgi-bin/wifi_info_GET.cgi", {type: 'get_wlan0_ip'},function(wlan0_ip) {
        if (wlan0_ip.length) {
            $('#ip0_txt').text(": "+wlan0_ip);
		
        }
    },"text");
}
//read connection_mode value
function Get_router_ctl() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    //$.get(cgiurl, {type: 'get_router_ctl'},function(router_ctl) {
        //var ret = Number(router_ctl);
        var ret = Number(totalData.connection);
        if (ret == 0) //direct
        {
            $("#connection_mode_icon").attr("src", "img/connection_mode_direct1.png");
            $('#connection_mode').text(direct_string);
            connection_mode_index = 1;
            $('#connection_mode_value').val("0");
            $('#ip0_txt').hide();
            $('#ip0').hide();
        } else if (ret == 1) //router
        {
            $("#connection_mode_icon").attr("src", "img/connection_mode_router1.png");
            $('#connection_mode').text(router_string);
            connection_mode_index = 0;
            $('#connection_mode_value').val("1");
            if (1 == connected_ssid_flag) {
                $('#ip0_txt').show();
                $('#ip0').show();
            }
        } else if (ret == 2) //only router
        {
            $("#connection_mode_icon").attr("src", "img/connection_mode_direct1.png");
            $('#connection_mode').text(onlyrouter_string);
            connection_mode_index = 2;
            $('#connection_mode_value').val("2");
            $('#ip0_txt').hide();
            $('#ip0').hide();
        }
        $('#ip1').show();
        $('#ip1_txt').show();
        current_connection_mode_index = connection_mode_index;

        Creat_router_list();
    //},"text");
}
//read 2.4G channel value
function Get_softap_channel() {
    var ret=0,Bandwidth_val=0;
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_softap_channel'},function(channel) {
        if (channel.length) {
            start = 0;
            text_string_length = 0;
            text_string = channel;
            text_string_length = channel.length;
            ret = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            Bandwidth_val = Get_text_from_textstring(text_string);
        }
		
        $("#G24_Bandwidth_val-button").children("span").text(Bandwidth[Bandwidth_val]);
        $("#G24_Bandwidth_val").find("option[value="+Bandwidth_val+"]").attr("selected",true); 
		
        channel_index = Number(ret);
        if (channel_index == 0) {
            $('#channel_txt').text("Auto");
            $("#ezpro_Channel_val-button").children("span").text("Auto");
            //$('#current_24channel').text("2.4G:Auto");
            Get_auto_channel_val();
        } else {
            $('#channel_txt').text(channel);
            $("#ezpro_Channel_val-button").children("span").text(channel);
            $("#ezpro_Channel_val option").eq(channel_index).attr("selected", true);
            $('#current_24channel').text("2.4G:"+channel);
        }
    },"text");

}
//get auto channel value
function Get_auto_channel_val() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_auto_channel_val'},function(channel) {
        if (channel.length > 0) {
            start = 0;
            text_string_length = 0;
            text_string = channel;
            text_string_length = channel.length;
            Autochannel_24G_val = Get_text_from_textstring(text_string);
            $('#current_24channel').text("2.4G: Auto("+Autochannel_24G_val+")");	
            text_string = text_string_global;
            Autochannel_5G_val = Get_text_from_textstring(text_string);
            text_string = text_string_global;
        }
    },"text");
}

window.onresize = window.onload = function() {
    Get_height_width_of_screen();
    $(".white_content").css("display", "none");

}

function show_wait() {
    $("#waiting").css('display', 'block');
    $("#waitbk").css('display', 'block');
}

function show_ui() {
    if($("#log_txt").text().indexOf("admin") == -1 && host_flag && psk_visibility_idx){
        $('#ap_psk').text("********");
        $('#ap_psk').css("vertical-align","middle");
    }
    $("#waiting").css('display', 'none');
    $("#waitbk").css('display', 'none');

    $("#list_load").css('display', 'block');
    $("#showinfo").css('display', 'block');
    $("#anyinfo").css('display', 'block');
    $("#wifiset_tit").text(WiFi_IP_Setting_string);
    $("#ezairmode_title").text(ezarimode_txt);
	
}
//any button click
function click_listener() {

    clearInterval(delay_get_data_id);
    delay_get_data_id = -1;
	/*
    $("#waiting").css('display', 'none');
    $("#waitbk").css('display', 'none');

    $("#list_load").css('display', 'block');
    $("#showinfo").css('display', 'block');
    */
    //move here:
    default_mode_listener();
    Get_music_output_en();
    Get_music_output();
    //Get_router_ctl();
    //Get_softap_channel();
    Get_set_password_text();
    Get_HDMI_cec_text();
    Get_router_only_text();
	
    Creat_EQ_list();
    Set_EQ_listener();
    Set_pws_listener();
    Set_ssid_listener();
    Set_devicename_listener();
    Set_resolution_listener();
    Set_router_listener();
    Set_router_only_listener();
    set_hdmicec_listener();
    Upgrade_listener();
    Upgrade_OK_listener();

    $("#showinfo").click(function() {
        if (showinfo_flag) {
            $("#more_info").hide();
            showinfo_flag = 0;
            $("#showinfo_ico").removeClass("info_ico_d");
            $("#showinfo_ico").addClass("info_ico_u");
        } else {
            $("#more_info").show();
            showinfo_flag = 1;
            $("#showinfo_ico").removeClass("info_ico_u");
            $("#showinfo_ico").addClass("info_ico_d");
        }
    });

    for (var i = 0; i < 10; i++) {
        $("#" + custom_index_id[i]).attr('data-highlight', true);
        //$("#"+custom_index_id[i]).attr('max',90);
    }

    $("#custom_ent").click(function() {
        $("#custom_show").hide();
        setTimeout(delay, 100);
        get_EQ_custom();

    });

    $("#custom_home").click(function() {
        setValue_all();
    });

    Channel_listener();
    updata_proboxlanonly_info();
}
function delay() {
    var i = 0;
    $(".ui-slider-handle.ui-btn.ui-shadow").each(function() {
        $(this).text(custom_name[i - 1]);
        $(this).css('width', '50px');
        i++;
    });
    $("#custom_show").show();
    $("#ezch_points").parent().find("a.ui-slider-handle").css('width', '30px');
}
//set EQ value
function setValue(index, textId) {
    return 0;
    var xx = $('#' + textId).val();
    $('#EQ_default_mode').text(Source_Selection[18]);
    Source_i = 18;
    current_eq_mode_index = Source_i;
    //--send date to cgi-----
    /********set index to custom*********/
    $('#custom_or_other').val("0");
    $('#eq_index').val("" + 18);
    $.post('cgi-bin/set_EQ.cgi', $('#config-form').serialize());
    /********************************/
    $('#custom_or_other').val("1");
    $('#eq_value').val("" + xx);
    custom_index = index;
    $('#custom_index').val("" + custom_index);
    $.post('cgi-bin/set_EQ.cgi', $('#config-form').serialize());
}
//read EQ value
function setValue_all() { 
    $('#EQ_default_mode').text(Source_Selection[18]);
    Source_i = 18;//custom
    current_eq_mode_index = Source_i;

    for (var i = 0; i < 10; i++) {
        var xx = $("#" + custom_index_id[i]).val();
        //--send date to cgi-----
        /********set index to custom*********/
        $('#custom_or_other').val("0");
        $('#eq_index').val("" + 18);
        $.post('cgi-bin/set_EQ.cgi', $('#config-form').serialize());
        /********************************/

        $('#custom_or_other').val("1");
        $('#eq_value').val("" + xx);
        // custom_index=i;
        $('#custom_index').val("" + i);
        $.post('cgi-bin/set_EQ.cgi', $('#config-form').serialize());
    }
}
//read custom EQ value
function get_EQ_Custom2() {
    var wd;
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_EQ_Custom'},function(EQ_Custom_value) {
        if (EQ_Custom_value.length > 0) {
            text_start = 0;
            text_string = EQ_Custom_value;
            text_string_length = EQ_Custom_value.length;
            for (var i = 0; i < 10; i++) {
                wd = Number(Get_text_from_textstring_eq(text_string));
                text_string = text_string_global;
                $("#" + custom_index_id[i]).val(wd);
                $("#" + custom_index_id[i]).parent().find(".ui-slider-bg").css("width", wd + "%");
                $("#" + custom_index_id[i]).parent().find("a.ui-slider-handle").css("left", wd + "%");

            }
        }
    },"text");

}
//read EQ value
function get_EQ_custom() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_EQ_default'},function(EQ_default_mode) {
        Source_i = Number(EQ_default_mode);
        if (18 == Source_i) {//custom
            get_EQ_Custom2();
        } else {
            for (var i = 0; i < 10; i++) {
                $("#" + custom_index_id[i]).val(EQ_Value[Source_i][i]);
                $("#" + custom_index_id[i]).parent().find(".ui-slider-bg").css("width", EQ_Value[Source_i][i] + "%");
                $("#" + custom_index_id[i]).parent().find("a.ui-slider-handle").css("left", EQ_Value[Source_i][i] + "%");
            }
        }
    },"text");
}

//creat wifi_client_mode
function add_wifi_client_mode_li() {
    var str ="<li id='setwifi_ip' class='ui-li-has-icon'><a href='#wifi_client_set' data-rel='popup' data-transition='none' data-position-to='window' aria-haspopup='true' aria-owns='wifi_setip' aria-expanded='false' class='ui-btn ui-btn-icon-right ui-icon-carat-r' style='font-weight: normal; color: rgb(255, 255, 255); text-shadow: none; border-width: 0px 0px 1px; border-bottom-style: inset; border-bottom-color: rgb(183, 183, 187); background-color: transparent;'><img src='img/wifi_IP_Setting.png' alt='wifiset' class='ui-li-icon'><h1 id='wifiset_tit' style='width: 65%;'>WiFi IP Setting</h1><span class='ui-li-count ui-body-inherit text' id='wifi_ssid' style='border-color: rgba(85, 85, 85, 0); border-radius: 0px; width: 30%; text-align: right; right: 2.6em; color: rgb(255, 255, 255); text-shadow: none; background-color: transparent;'></span></a></li>";
    $("#wifilist").after(str);
    $("#wifiset_tit").text(WiFi_IP_Setting_string);
}
//wifi_client_mode button click
function wifi_client_mode_listener() {
    var chanel;
    $("#setwifi_ip").click(function() {
        get_wifiAutomatic();
        $("#wifi_client_ip_format").text("");
        window.clearTimeout(intervalID);
    });
	
    $("#wifi_client_setonoff").click(function() {
        chanel = Number($("#wifi_client_lanset").val());
        if (chanel) {
            wifi_client_off_style();
        } else {
            wifi_client_on_style();
        }
    });

    $("#wifi_client_lanset").bind("swipeleft",function() {
        chanel = Number($("#wifi_client_lanset").val());
        if (chanel) {
            wifi_client_off_style();
        } else {
            wifi_client_on_style();
        }
    });

    $("#wifi_client_lanset").bind("swiperight",function() {
        chanel = Number($("#wifi_client_lanset").val());
        if (chanel) {
            wifi_client_off_style();
        } else {
            wifi_client_on_style();
        }
    });

    $("#wifi_ent").click(function() {
        chanel = Number($("#wifi_client_lanset").val());
        if (chanel) {
            set_wifiAutomatic();
            get_wifiSetting_auto_ip();
            $("#wifi_client_set").popup("close");
        } else {
            set_wifimanual_info();
        }

    });
}

//lan button click
function ezlan_listener() {
    var chanel;
    self.setTimeout(show_ui,1500);   //networksetup delay show ui

    $("#lanlist").click(function() {
        get_LanAutomatic();
        $("#ip_format").text("");
        window.clearTimeout(intervalID);
    });
	
    $("#lansetonoff").click(function() {
        chanel = Number($("#lanset").val());
        if (chanel) {
            ezlan_off_style();
        } else {
            ezlan_on_style();
        }
    });

    $("#lanset").bind("swipeleft",function() {
        chanel = Number($("#lanset").val());
        if (chanel) {
            ezlan_off_style();
        } else {
            ezlan_on_style();
        }
    });

    $("#lanset").bind("swiperight",function() {
        chanel = Number($("#lanset").val());
        if (chanel) {
            ezlan_off_style();
        } else {
            ezlan_on_style();
        }
    });

    $("#lan_ent").click(function() {
        var chanel = Number($("#lanset").val());
        if (chanel) {
            set_LanAutomatic();
            get_LanSetting_auto_ip();
            $("#wlanset").popup("close");
        } else {
            set_Lanmanual_info();
        }

    });
	

}
//check ip value
function ip_num_check(value) {
    var regx = /^[0-9]+$/;
    var checkstring = value;
    if (!regx.test(checkstring)) {
        $('#error_txt').text("Only the letters and numbers are allowed!");
        return 1;
    } else if (checkstring.length < 6 || checkstring.length > 64) {
        $('#error_txt').text("Password must be between 7 to 64 characters long.");
        return 1;
    }
    return 0;
}

var spit_ip_array = new Array();
//check ip value format
function check_ip_format(ipname) {
    var tmp;
    var regx = /^[0-9]+$/;
    tmp = $('#' + ipname).val();
    spit_ip_array = tmp.split(".");
    if (spit_ip_array.length < 4) 
        return 0;

    for (var i = 0; i < spit_ip_array.length; i++) {
        if (!regx.test(spit_ip_array[i])) 
            return 0;
        else if (Number(spit_ip_array[i]) > 255) 
            return 0;
    }

    return 1;
}
//check ip value
function check_ip() {

    if ($('#ipaddr').val() == "" || $('#netmask').val() == "" || $('#gateway').val() == "" || $('#dns1').val() == "") 
        return 0;
    if (!check_ip_format("ipaddr")) 
        return 0;
    if (!check_ip_format("netmask")) 
        return 0;
    if (!check_ip_format("gateway")) 
        return 0;
    if (!check_ip_format("dns1")) 
        return 0;

    return 1;

}
//check wifi_client value
function check_wifi_client_ip() {

    if ($('#wifi_client_ipaddr').val() == "" || $('#wifi_client_netmask').val() == "" || $('#wifi_client_gateway').val() == "" || $('#wifi_client_dns1').val() == "") 
        return 0;
    if (!check_ip_format("wifi_client_ipaddr")) 
        return 0;
    if (!check_ip_format("wifi_client_netmask")) 
        return 0;
    if (!check_ip_format("wifi_client_gateway")) 
        return 0;
    if (!check_ip_format("wifi_client_dns1")) 
        return 0;

    return 1;

}
//set lan ip value
function set_Lanmanual_info() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); 
    if (check_ip() == 0) {
        $("#ip_format").text(ip_format_string);
    } else {
        var str = $('#ipaddr').val() + "\n" + $('#netmask').val() + "\n" + $('#gateway').val() + "\n" + $('#dns1').val() + "\n" + $('#dns2').val() + "\n" + "writeLanManualInfo\n";
        $.get(cgiurl, {type: str},function(flag) {
            if (flag == "-1" || flag == "0"){
                $("#ip_format").text(ip_format_string); //Please enter the correct format!
            } else {
                $('#lan_ssid').text($('#ipaddr').val());
                $("#wlanset").popup("close");
                set_LanAutomatic_off();
            }
        },
        "text");
    }
}
//set wifi ip value
function set_wifimanual_info() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); 
    if (check_wifi_client_ip() == 0) {
        $("#wifi_client_ip_format").text(ip_format_string);
    } else {
        var str = $('#wifi_client_ipaddr').val() + "\n" + $('#wifi_client_netmask').val() + "\n" + $('#wifi_client_gateway').val() + "\n" + $('#wifi_client_dns1').val() + "\n" + $('#wifi_client_dns2').val() + "\n" + "writeWiFiManualInfo\n";
        $.get(cgiurl, {type: str},function(flag) {
            if (flag == "-1" || flag == "0"){
                $("#wifi_client_ip_format").text(ip_format_string); //Please enter the correct format!
            } else {
                $('#wifi_ssid').text($('#wifi_client_ipaddr').val());
                $("#wifi_client_set").popup("close");
                set_wifiAutomatic_off();
            }
        },
        "text");
    }
}
function set_LanAutomatic_off() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); 
    $.get(cgiurl, {type: 'setLanAuto_off'},function() {
    },"text");
}

function set_LanAutomatic() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); 
    $.get(cgiurl, {type: 'setLanAutomatic'},function() {
    },"text");
}
function set_wifiAutomatic_off() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); 
    $.get(cgiurl, {type: 'setWiFiAuto_off'},function() {
    },"text");
}

function set_wifiAutomatic() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); 
    $.get(cgiurl, {type: 'setWiFiAutomatic'},function() {
    },"text");
}
var intervalID;
function Check_get_netlink() {
    window.clearTimeout(intervalID);
    var ret;
    $.get("cgi-bin/wifi_info_GET.cgi", {type: 'get_netlink'},function(get_netlink) {
        ret = Number(get_netlink);
        if (ret) {
            $("#lanlist").show();
            aa = 11; //close wifi get_connected_ssid
            get_LanSettingVal();
            Get_Lan_mac_ip();
        }
        else {
            a = 1;
        }

    },"text");
    intervalID = window.setTimeout("Check_get_netlink();", 10000);
}
//lan ui css
function ezlan_on_style() {
    $('#ipaddr').parent().removeClass("ui-state-disabled ");
    $('#gateway').parent().removeClass("ui-state-disabled ");
    $('#netmask').parent().removeClass("ui-state-disabled ");
    $('#dns1').parent().removeClass("ui-state-disabled ");
    $('#dns2').parent().removeClass("ui-state-disabled ");
    if($('#ipaddr').val()=="Auto"){
        $('#ipaddr').val("");
        $('#gateway').val("");
        $('#netmask').val("");
        $('#dns1').val("");
        $('#dns2').val("");
    }
}
//lan ui css
function ezlan_off_style() {
    $('#ipaddr').parent().addClass("ui-state-disabled ");
    $('#gateway').parent().addClass("ui-state-disabled ");
    $('#netmask').parent().addClass("ui-state-disabled ");
    $('#dns1').parent().addClass("ui-state-disabled ");
    $('#dns2').parent().addClass("ui-state-disabled ");
    if($('#ipaddr').val()==""){
        $('#ipaddr').val("Auto");
        $('#gateway').val("Auto");
        $('#netmask').val("Auto");
        $('#dns1').val("Auto");
        $('#dns2').val("Auto");
    }
}	
//lan ui css
function wifi_client_on_style() {
    $('#wifi_client_ipaddr').parent().removeClass("ui-state-disabled ");
    $('#wifi_client_gateway').parent().removeClass("ui-state-disabled ");
    $('#wifi_client_netmask').parent().removeClass("ui-state-disabled ");
    $('#wifi_client_dns1').parent().removeClass("ui-state-disabled ");
    $('#wifi_client_dns2').parent().removeClass("ui-state-disabled ");
    if($('#wifi_client_ipaddr').val()=="Auto"){
        $('#wifi_client_ipaddr').val("");
        $('#wifi_client_gateway').val("");
        $('#wifi_client_netmask').val("");
        $('#wifi_client_dns1').val("");
        $('#wifi_client_dns2').val("");
    }
}
//lan ui css
function wifi_client_off_style() {
    $('#wifi_client_ipaddr').parent().addClass("ui-state-disabled ");
    $('#wifi_client_gateway').parent().addClass("ui-state-disabled ");
    $('#wifi_client_netmask').parent().addClass("ui-state-disabled ");
    $('#wifi_client_dns1').parent().addClass("ui-state-disabled ");
    $('#wifi_client_dns2').parent().addClass("ui-state-disabled ");
    if($('#wifi_client_ipaddr').val()==""){
        $('#wifi_client_ipaddr').val("Auto");
        $('#wifi_client_gateway').val("Auto");
        $('#wifi_client_netmask').val("Auto");
        $('#wifi_client_dns1').val("Auto");
        $('#wifi_client_dns2').val("Auto");
    }
}
//set lan ip ui css
function get_LanAutomatic() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime(); 
    $.get(cgiurl, {type: 'getLanAutomatic'},function(LanAutomatic) {
        var ret = Number(LanAutomatic);
        if (ret) {
            $("#lanset").val(1);
            $("#lansetonoff").children().addClass("ui-flipswitch-active");
            $("#lanset option").eq(1).attr("selected", true); //on
            get_LanSetting_auto_ip();
            ezlan_off_style();
        } else {
            $("#lanset").val(0);
            $("#lansetonoff").children().removeClass("ui-flipswitch-active");
            $("#lanset option").eq(0).attr("selected", true); //off
            get_LanSettingVal();
            ezlan_on_style();
        }
    },"text");

}
//set wifi ip ui css
function get_wifiAutomatic() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
      $.get(cgiurl, {type: 'getWiFiAutomatic'},function(LanAutomatic) {
	        var ret = Number(LanAutomatic);
	        if (ret) {
	            $("#wifi_client_lanset").val(1);
	            $("#wifi_client_setonoff").children().addClass("ui-flipswitch-active");
	            $("#wifi_client_lanset option").eq(1).attr("selected", true); //on
	            get_wifiSetting_auto_ip();
	            wifi_client_off_style();
	        } else {
	            $("#wifi_client_lanset").val(0);
	            $("#wifi_client_setonoff").children().removeClass("ui-flipswitch-active");
	            $("#wifi_client_lanset option").eq(0).attr("selected", true); //off
	            get_wifiSettingVal();
	            wifi_client_on_style();
	        }
	    },"text");

}
//read lan auto ip value
function get_LanSetting_auto_ip() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'getLanSettingautoip'},function(LanSettingVal) {
        var ret = LanSettingVal;
        if (LanSettingVal.length) {
            start = 0;
            text_string_length = 0;
            text_string = LanSettingVal;
            text_string_length = LanSettingVal.length;
            $('#ipaddr').val(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#gateway').val(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#netmask').val(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#dns1').val(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#dns2').val(Get_text_from_textstring(text_string));

            current_wlan_ip_value = ($('#ipaddr').val() + $('#gateway').val() + $('#netmask').val() + $('#dns1').val() + $('#dns2').val());

            $('#lan_ssid').text($('#ipaddr').val());
            $('#lan_ssid').show();

        } else {
            $('#lan_ssid').hide();
        }

    },"text");
}
//read wifi auto ip value
function get_wifiSetting_auto_ip() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'getWiFi_auto_ip'},function(wifiSettingVal) {
        var ret = wifiSettingVal;
        if (wifiSettingVal.length) {
            start = 0;
            text_string_length = 0;
            text_string = wifiSettingVal;
            text_string_length = wifiSettingVal.length;
            $('#wifi_client_ipaddr').val(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#wifi_client_gateway').val(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#wifi_client_netmask').val(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#wifi_client_dns1').val(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#wifi_client_dns2').val(Get_text_from_textstring(text_string));

            current_wlan_ip_value = ($('#wifi_client_ipaddr').val() + $('#wifi_client_gateway').val() + $('#wifi_client_netmask').val() + $('#wifi_client_dns1').val() + $('#wifi_client_dns2').val());

            $('#wifi_ssid').text($('#wifi_client_ipaddr').val());
            $('#wifi_ssid').show();

        } else {
            $('#wifi_ssid').hide();
        }

    },"text");
}
//read lan ip value
function get_LanSettingVal() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'getLanSettingVal'},function(LanSettingVal) {
        var ret = LanSettingVal;
        if (LanSettingVal.length) {
            start = 0;
            text_string_length = 0;
            text_string = LanSettingVal;
            text_string_length = LanSettingVal.length;
            $('#ipaddr').val(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#gateway').val(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#netmask').val(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#dns1').val(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#dns2').val(Get_text_from_textstring(text_string));

            current_wlan_ip_value = ($('#ipaddr').val() + $('#gateway').val() + $('#netmask').val() + $('#dns1').val() + $('#dns2').val());

            $('#lan_ssid').text($('#ipaddr').val());
            $('#lan_ssid').show();
        } else {
            $('#lan_ssid').hide();
        }

    },"text");
}
//read wifi ip value
function get_wifiSettingVal() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'getWiFiSetting_Val'},function(LanSettingVal) {
        var ret = LanSettingVal;
        if (LanSettingVal.length) {
            start = 0;
            text_string_length = 0;
            text_string = LanSettingVal;
            text_string_length = LanSettingVal.length;
            $('#wifi_client_ipaddr').val(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#wifi_client_gateway').val(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#wifi_client_netmask').val(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#wifi_client_dns1').val(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#wifi_client_dns2').val(Get_text_from_textstring(text_string));

            current_wlan_ip_value = ($('#wifi_client_ipaddr').val() + $('#wifi_client_gateway').val() + $('#wifi_client_netmask').val() + $('#wifi_client_dns1').val() + $('#wifi_client_dns2').val());

            $('#wifi_ssid').text($('#wifi_client_ipaddr').val());
            $('#wifi_ssid').show();
        } else {
            $('#wifi_ssid').hide();
        }

    },"text");
}

var lan_text_string = -1;
var ip_format_string;
//read lan text for ui
function get_Lan__language() {
    $.get("cgi-bin/wifi_info_GET.cgi", {type: 'gtmulilan'},function(lan_text_string) {
        if (lan_text_string.length) {
            start = 0;
            text_string_length = 0;
            text_string = lan_text_string;
            text_string_length = lan_text_string.length;
            $('#lan_txt').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#mode_lan').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#ip_lan').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#gateway_lan').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            $('#netmast_lan').text(Get_text_from_textstring(text_string));
            text_string = text_string_global;
            warn_msg=Get_text_from_textstring(text_string);//Network cable is plugged in and turn off wireless scanning! 
            text_string = text_string_global;
            ip_format_string = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            Channel_text = Get_text_from_textstring(text_string);//Channel 
            text_string = text_string_global;
            Country_text = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            Bandwidth_text = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            $('#channel_tip_text').text(Get_text_from_textstring(text_string));//Wireless Channel Setting for this product
            text_string = text_string_global;
            WiFi_IP_Setting_string = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            $('#wifi_pass_hide_title').text(Get_text_from_textstring(text_string));//WIFI密码隐藏
            $('#txt_passhid').text($('#wifi_pass_hide_title').text()+": ");
            text_string = text_string_global;
            $('#lan_txt').text(Get_text_from_textstring(text_string));//LAN IP Setting

            $('#channel_show_txt').text("Wifi "+Channel_text);
            $('#menu_0_txt').text("2.4G "+Channel_text);
            $('#menu_1_txt').text("5G  "+Channel_text);
            $('#ezpro_Channel_txt').text("2.4G "+Channel_text+":");
            $('#5G_country_txt').text(Country_text+":");
            $('#5G_Channel_txt').text(Channel_text+":");
            $('#5G_Bandwidth_txt').text(Bandwidth_text);//Bandwidth:
            $('#G24_Bandwidth_txt').text(Bandwidth_text);
            //wifi client mode language
            $('#wifi_client_mode_lan').text($('#mode_lan').text());
            $('#wifi_client_ip_lan').text($('#ip_lan').text());
            $('#wifi_client_gateway_lan').text($('#gateway_lan').text());
            $('#wifi_client_netmast_lan').text($('#netmast_lan').text());

            //no mulang:
            Channel_warn="Wireless will disconnect if you choose OK!";
            $('#dialog_24G_warn').text(Channel_warn);
            $('#dialog_5G_warn').text(Channel_warn);
        } else {
            warn_msg="Network cable is plugged in and turn off wireless scanning!"; 
            //no mulang:
            Channel_warn="Wireless will disconnect if you choose OK!";
            $('#dialog_24G_warn').text(Channel_warn);
            $('#dialog_5G_warn').text(Channel_warn);
            $('#lan_txt').text("LAN IP Setting");//

        }
    },"text");
}
function  check_default_mode()
{
 	var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
 	$.get(cgiurl, {type: 'get_last_ui'},function(last_ui_index) {
	if (last_ui_index.indexOf("2") >= 0) {
		//AP+LAN+P2P
 		miracast_probox_mode=1;
		$("#wifilist").hide();
	}
	else
		miracast_probox_mode=0;
		Get_connected_ssid();
	},"text");
}

//for network setup
function Check_network_setup_flag() {
    $('#ip0show').hide(); //first hide ip0text
    $("#lanlist").hide();
    $("#list_load").css('display', 'none');

    $.get("cgi-bin/wifi_info_GET.cgi", {type: 'EZMUSIC_ENABLE'},function(ezmusic_enable) {
        ezmusic_flag = Number(ezmusic_enable);
        if (ezmusic_flag == 3||ezmusic_flag ==5||ezmusic_flag == 6 || ezmusic_flag == 9){//muse first run
            Check_lan_in();
        }
        switch (ezmusic_flag) {
            case 1:
                break;
            case 2:
                //ezcast
                //window.location.replace("ezcastmain.html");
                //$("#set_title").text("SETUP");
                //$("#set_title").css('display', 'block');
                //$("#set_title").remove();
                //$("#set_title_admin").css('display', 'block');
                $("#modelout").remove();
                $("#Set_EQ_mode").remove();
                $("#ezch_play_way_t").remove();
                $("#channel_set").remove();
                $("#hdmi_cec").remove();
                $("#ignoressid_control").remove();
                $("#ezcast_opt").css('display', 'block');
                //Get_cpu_frequency(); 
                //Get_connected_ssid();
                break;
            case 3:
                //EZWILAN_ENABLE
                //window.location.replace("ezcastmain.html");
                $("#set_network_title").css('display', 'block');
                $("#modelout").remove();
                $("#Set_EQ_mode").remove();
                $("#ezch_play_way_t").remove();
                $("#channel_set").remove();
                $("#hdmi_cec").remove();
                $("#ignoressid_control").remove();
                $("#ezcast_opt").css('display', 'block');
                $("#log_txt").css('display', 'block');
                //Check_get_netlink(); 
                $("#lanlist").show();
                //$("#wifilist").hide();
                //$("#pass_sel").remove();
                //add_ezair_li();
                //Get_ezairmode_ui();
                //ezairmode_listener();
                //Get_resolution();
                //get_Lan__language();
                get_LanSettingVal();
                get_LanAutomatic();
                Get_Lan_mac_ip();
                ezlan_listener();
                Get_connected_ssid();
                break;
            case 4:
                //EZCASTPRO_MODE
                //$("#set_network_title").text("Network setup");
                $("#set_network_title").css('display', 'block');
                $("#modelout").remove();
                $("#Set_EQ_mode").remove();
                $("#ezch_play_way_t").remove();
                $("#ezchannelmode").remove();
                $("#ezcast_opt").css('display', 'block');
                $("#log_txt").css('display', 'block');
                //$("#wifilist").hide();
                //$("#pass_sel").remove();
                //add_ezair_li();
                //Get_ezairmode_ui();
                //ezairmode_listener();
                //Get_resolution();
                Check_get_netlink();
                ezlan_listener();
                Get_connected_ssid();
                get_wifiAutomatic();
                add_wifi_client_mode_li();
                wifi_client_mode_listener();
                break;
            case 5:
                //EZCast Pro LAN
                $("#li_defaultmode").remove();
                $("#hdmi_audio_select").remove();
            case 6:
                //pro box
                //$("#set_network_title").text("Network setup");
                $("#set_network_title").css('display', 'block');
                $("#modelout").remove();
                $("#Set_EQ_mode").remove();
                $("#ezch_play_way_t").remove();
                $("#ezchannelmode").remove();
                //$("#channel_set").remove();
                //$("#wifilist").hide();
                //$("#pass_sel").remove();
                //add_ezair_li();
                //Get_ezairmode_ui();
                //ezairmode_listener();
                $("#ezcast_opt").css('display', 'block');
                $("#log_txt").css('display', 'block');
                //Get_resolution();
                Check_get_netlink();
                $("#lanlist").show();
                //get_Lan__language();
                //get_LanSettingVal();
                get_LanAutomatic();
                get_wifiAutomatic();
                check_default_mode();
                Get_Lan_mac_ip();
                ezlan_listener();
                //Get_connected_ssid();
                Get_5GChannel_country();
                add_wifi_client_mode_li();
                wifi_client_mode_listener();
                //Get_softapconnect_ui();
                //softapconnect_listener();
                break;
            case 9:
                //pro box lan only
                //$("#set_network_title").text("Network setup");
                $("#set_network_title").css('display', 'block');
                $("#modelout").remove();
                $("#Set_EQ_mode").remove();
                $("#ezch_play_way_t").remove();
                $("#ezchannelmode").remove();
                //$("#channel_set").remove();
                $("#wifilist").remove();
                $("#pass_sel").remove();
                $("#wifi_pass_hide").remove();
                $("#channel_set").remove();
                //add_ezair_li();
                //Get_ezairmode_ui();
                //ezairmode_listener();
                $("#ezcast_opt").css('display', 'block');
                $("#log_txt").css('display', 'block');
                //Get_resolution();
                Check_get_netlink();
                $("#lanlist").show();
                //get_Lan__language();
                //get_LanSettingVal();
                get_LanAutomatic();
                //get_wifiAutomatic();
                check_default_mode();
                Get_Lan_mac_ip();
                ezlan_listener();
                add_ignoressid_li();
                Get_ignoressidmode_ui() ;
                ignoressidmode_listener();
                //Get_connected_ssid();
                //Get_5GChannel_country();
                //add_wifi_client_mode_li();
                //wifi_client_mode_listener();
                //Get_softapconnect_ui();
                //softapconnect_listener();
                break;
            case 7:
                //EZCast wire
                //break;
            default:
                $("#modelout").remove();
                $("#Set_EQ_mode").remove();
                $("#ezch_play_way_t").remove();
                $("#ezchannelmode").remove();
                $("#hdmi_cec").remove();
                $("#channel_set").remove();
                $("#ignoressid_control").remove();
                break;
        }
        if (ezmusic_flag == 5 || ezmusic_flag == 4) {
            $("#channel_set").children("a").attr('href', '#Channel_24G');
        }
        if (ezmusic_flag == 6) {
	     $("#wlan0").text("5G MAC : ");
	     Get_24G_mac_ip();
	     $("#for_box_mac").show();
        }

    },"text");
}

function reset_listener(){
	$('#reset_init').click(function(e)   //diolog
	{
		$("#dialog1").show();
		$("#dialog2").hide();
		$("#reset_message_txt").text(reset_message);
		$("#resetpop").popup("open");

	});	
	
	$('#reset_ent').click(function(e)   //reset to default
	{
		$("#dialog1").hide();
		$("#dialog2").show();


	});	
	
	$('#reset_ok').click(function(e)   //reset to default
	{
		var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
		//alert("---$('#reset_ok').click---"+cgiurl);
		$("#resetpop").popup("close");
		$.get(cgiurl, {type:"reset_default"}, function()
		{	
		}, "text");
		//window.location.reload();
	});

}

//read plat flag
function Check_admin_setup_flag() {
    $('#ip0show').hide(); //first hide ip0text
    $("#lanlist").hide();
    $("#list_load").css('display', 'none');
    $.get("cgi-bin/wifi_info_GET.cgi", {type: 'EZMUSIC_ENABLE'},function(ezmusic_enable) {
        ezmusic_flag = Number(ezmusic_enable);
        if (ezmusic_flag == 3||ezmusic_flag ==5||ezmusic_flag == 6 ||ezmusic_flag == 9){//muse first run
            Check_lan_in();
        }
        switch (ezmusic_flag) {
            case 1:
                //ezmusic
                break;
            case 2:
                //ezcast
                //window.location.replace("ezcastmain.html");
                //$("#set_title").text("SETUP");
                $("#set_title").css('display', 'block');
                $("#ignoressid_control").remove();
                $("#ezcast_opt").css('display', 'block');
                $("#pro_1").remove();
                $("#pro_2").remove();
                $("#log_txt").remove();
                $("#for_ezmu_esca").css('display', 'block');
                break;
            case 3:
                //EZWILAN_ENABLE
                //window.location.replace("ezcastmain.html");
                //$("#set_title").css('display', 'block');
                $("#modelout").remove();
                $("#Set_EQ_mode").remove();
                $("#ezch_play_way_t").remove();
                $("#channel_set").remove();
                $("#hdmi_cec").remove();
                $("#ignoressid_control").remove();
                $("#ezcast_opt").css('display', 'block');
                $("#log_txt").css('display', 'block');
                //Check_get_netlink(); 
                $("#lanlist").remove();
                //get_Lan__language();
                //get_LanSettingVal();
                //get_LanAutomatic();
                //Get_Lan_mac_ip();
                //ezlan_listener();
                Get_connected_ssid();
                break;
            case 4:
                //EZCASTPRO_MODE
                //$("#set_title").css('display', 'block');
                $("#modelout").remove();
                $("#Set_EQ_mode").remove();
                $("#ezch_play_way_t").remove();
                $("#ezchannelmode").remove();
                $("#ezcast_opt").css('display', 'block');
                $("#log_txt").css('display', 'block');
                Check_get_netlink();
                Get_connected_ssid();
                add_ignoressid_li();
                Get_ignoressidmode_ui() ;
                ignoressidmode_listener();
              //  get_ignore_ssid();
                //ezlan_listener();
                //get_wifiAutomatic();
                //add_wifi_client_mode_li();
                //wifi_client_mode_listener();
                //Get_softapconnect_ui();
                //softapconnect_listener();
                Get_control_access__language();
                get_internet_access_control_text();
                set_internet_access_control_listener();
                Get_reboot_reset_language();
                reset_listener();
                break;
            case 5:
                //8251 EZCast Pro LAN
            case 6:
                //pro box
                //$("#set_title").css('display', 'block');
                $("#modelout").remove();
                $("#Set_EQ_mode").remove();
                $("#ezch_play_way_t").remove();
                $("#ezchannelmode").remove();
                $("#li_defaultmode").remove();
                //$("#channel_set").remove();
                add_ignoressid_li();
                Get_ignoressidmode_ui() ;
                ignoressidmode_listener();
                $("#ezcast_opt").css('display', 'block');
                $("#log_txt").css('display', 'block');
                Check_get_netlink();
                $("#lanlist").remove();
                //get_Lan__language();
                //get_LanSettingVal();
                //get_LanAutomatic();
                //get_wifiAutomatic();
                check_default_mode();
                Get_Lan_mac_ip();
             //   get_ignore_ssid();
                //ezlan_listener();
                //add_wifi_client_mode_li();
                //wifi_client_mode_listener();
                //Get_softapconnect_ui();
                //softapconnect_listener();
                Get_control_access__language();
                get_internet_access_control_text();
                set_internet_access_control_listener();
                Get_reboot_reset_language();
                reset_listener();
                break;
            case 9:
                //pro box Lan only
                //$("#set_title").css('display', 'block');
                $("#modelout").remove();
                $("#Set_EQ_mode").remove();
                $("#ezch_play_way_t").remove();
                $("#ezchannelmode").remove();
                $("#li_defaultmode").remove();
                $("#WiFiEnterprise").remove();
                $("#connectionmode").remove();
                //$("#channel_set").remove();
                $("#ezcast_opt").css('display', 'block');
                $("#log_txt").css('display', 'block');
                Check_get_netlink();
                $("#lanlist").remove();
                //get_Lan__language();
                //get_LanSettingVal();
                //get_LanAutomatic();
                //get_wifiAutomatic();
                check_default_mode();
                Get_Lan_mac_ip();
                //get_ignore_ssid();
                //ezlan_listener();
                //add_wifi_client_mode_li();
                //wifi_client_mode_listener();
                //Get_softapconnect_ui();
                //softapconnect_listener();
                Get_control_access__language();
                get_internet_access_control_text();
                set_internet_access_control_listener();
                Get_reboot_reset_language();
                reset_listener();
                break;
            case 7:
                //EZCast wire
                //$("#set_title").css('display', 'block');
                $("#li_defaultmode").remove();
                $("#menu_left,#menu_right").remove();
                $("#connectionmode").remove();
                $("#hdmi_cec").remove();
                $("#ezcast_opt").css('display', 'none');
                $("#channel_set").remove();
                $("#ignoressid_control").remove();
                $("#modelout,#Set_EQ_mode,#wifilist,#lanlist,#pass_sel,#device_sel,#ezchannelmode,#channel_set").remove();
                $("#showinfo,#anyinfo,#more_info").remove();
                $("#ezwire_info").show();
                add_ezwire_audio_li();
                Get_ezwire_audio_ui();
                ezwire_audio_listener();
                get_ezwire_audio__text();
                //Get_connected_ssid();
                break;
            default:
                $("#modelout").remove();
                $("#Set_EQ_mode").remove();
                $("#ezch_play_way_t").remove();
                $("#ezchannelmode").remove();
                $("#hdmi_cec").remove();
                $("#channel_set").remove();
                $("#ignoressid_control").remove();
                break;
        }
        if (ezmusic_flag == 6 ) {
            $("#wlan0").text("5G MAC : ");
            Get_24G_mac_ip();
            $("#for_box_mac").show();
        } 

    },"text");
}

//read plat flag
function Check_EZMUSIC_ENABLE_flag() {
    $('#ip0show').hide(); //first hide ip0text
    $("#lanlist").hide();
    $("#list_load").css('display', 'none');
    $.get("cgi-bin/wifi_info_GET.cgi", {type: 'EZMUSIC_ENABLE'},function(ezmusic_enable) {
        ezmusic_flag = Number(ezmusic_enable);
        if (ezmusic_flag == 3||ezmusic_flag ==5||ezmusic_flag == 6 ||ezmusic_flag == 9){//muse first run
            Check_lan_in();
        }
        switch (ezmusic_flag) {
            case 1:
                //ezmusic
                //$("#set_title").text("EZMusic | setup");
                //$("#set_title").css('display', 'block');
                $("#set_title").remove();
                $("#set_title_admin").css('display', 'block');
                $("#li_defaultmode").remove();
                $("#resolution_all").remove();
                $("#connectionmode").remove();
                $("#hdmi_cec").remove();
                $("#ezcast_opt").css('display', 'none');
                $("#channel_set").remove();
                $("#ignoressid_control,#reset_init,#adminpas,#li_defaultmode").remove();
                get_EQ_default();
                Get_connected_ssid();
                break;
            case 2:
                //ezcast
                //window.location.replace("ezcastmain.html");
                //$("#set_title").text("SETUP");
                //$("#set_title").css('display', 'block');
                $("#set_title").remove();
                $("#set_title_admin").css('display', 'block');
                $("#modelout").remove();
                $("#Set_EQ_mode").remove();
                $("#ezch_play_way_t").remove();
                $("#channel_set").remove();
                $("#hdmi_cec").remove();
                $("#ignoressid_control,#reset_init,#adminpas,#li_defaultmode").remove();
                $("#ezcast_opt").css('display', 'block');
                //Get_cpu_frequency(); 
                Get_defaultmode_UI();// first read data
                add_ezair_li();
                Get_ezairmode_ui();
                ezairmode_listener();
                Get_resolution();
                Get_connected_ssid();
                break;
            case 3:
                //EZWILAN_ENABLE
                //window.location.replace("ezcastmain.html");
                $("#set_title_admin").remove();
                //$("#set_title").text("Device management");
                $("#set_title").css('display', 'block');
                $("#modelout").remove();
                $("#Set_EQ_mode").remove();
                $("#ezch_play_way_t").remove();
                $("#channel_set").remove();
                $("#hdmi_cec").remove();
                $("#ignoressid_control").remove();
                $("#connectionmode").remove();
                $("#channel_set").remove();
                $("#ezcast_opt").css('display', 'block');
                $("#log_txt").css('display', 'block');
                //Check_get_netlink(); 
                $("#lanlist").show();
                $("#wifilist").hide();
                //$("#pass_sel").remove();
                add_ezair_li();
                Get_ezairmode_ui();
                ezairmode_listener();
                Get_resolution();   //first read 
                Get_defaultmode_UI();// first read data
                //get_Lan__language();
                //get_LanSettingVal();
                //get_LanAutomatic();
                Get_Lan_mac_ip();
                //ezlan_listener();
                Get_connected_ssid();
                break;
            case 4:
                //EZCASTPRO_MODE
                $("#set_title_admin").remove();
                //$("#set_title").text("Device management");
                $("#set_title").css('display', 'block');
                $("#modelout").remove();
                $("#Set_EQ_mode").remove();
                $("#ezch_play_way_t").remove();
                $("#ezchannelmode").remove();
                $("#ezcast_opt").css('display', 'block');
                $("#log_txt").css('display', 'block');

                if(advance_flag)
		    $("#wifilist").hide();
				
                $("#li_defaultmode").remove();
                $("#pass_sel,#channel_set,#upgrade").remove();
                $("#connectionmode").remove();
                $("#hdmi_audio_select").remove();
                add_ezair_li();
                Get_ezairmode_ui();
                ezairmode_listener();
                Get_resolution();   //first read 
                Get_defaultmode_UI();// first read data
                Check_get_netlink();
                //ezlan_listener();
                Get_connected_ssid();
                //get_wifiAutomatic();
                //add_wifi_client_mode_li();
                //wifi_client_mode_listener();
                add_softapconnect_li();
                Get_softapconnect_ui();
                softapconnect_listener();
                add_miracode_li();
                get_miracode_text();
                set_miracode_listener();
                add_casecode_li();
                get_passcode_text();
                set_passcode_listener();
                Get_control_access__language();
                add_airview_li();
                airview_listener();
                Get_airview_text();
                break;
            case 5:
                //EZCast Pro LAN
                $("#li_defaultmode").remove();
                $("#hdmi_audio_select").remove();
                $("#current_5channel").remove();
            case 6:
                //pro box
                $("#set_title_admin").remove();
                //$("#set_title").text("Device management");
                $("#set_title").css('display', 'block');
                $("#modelout").remove();
                $("#Set_EQ_mode").remove();
                $("#ezch_play_way_t").remove();
                $("#ezchannelmode").remove();
                if(advance_flag)
		    $("#wifilist").hide();
                $("#pass_sel,#channel_set,#upgrade").remove();
                $("#connectionmode").remove();
                add_ezair_li();
                Get_ezairmode_ui();
                ezairmode_listener();
                $("#ezcast_opt").css('display', 'block');
                $("#log_txt").css('display', 'block');
                Get_resolution();
                Check_get_netlink();
                $("#lanlist").remove();
                //get_Lan__language();
                //get_LanSettingVal();
                //get_LanAutomatic();
                //get_wifiAutomatic();
                //ezlan_listener();
                //Get_connected_ssid();
                check_default_mode();
                Get_Lan_mac_ip();
                //Get_5GChannel_country();
                //add_wifi_client_mode_li();
                //wifi_client_mode_listener();
                add_softapconnect_li();
                Get_softapconnect_ui();
                softapconnect_listener();
                add_miracode_li();
                get_miracode_text();
                set_miracode_listener();
                add_casecode_li();
                get_passcode_text();
                set_passcode_listener();
                Get_control_access__language();
                add_airview_li();
                airview_listener();
                Get_airview_text();
                break;
            case 9:
                //pro box Lan only
                $("#set_title_admin").remove();
                //$("#set_title").text("Device management");
                $("#set_title").css('display', 'block');
                $("#modelout").remove();
                $("#Set_EQ_mode").remove();
                $("#ezch_play_way_t").remove();
                $("#ezchannelmode").remove();
                $("#wifilist").hide();
                $("#pass_sel,#channel_set,#upgrade").remove();
                $("#connectionmode").remove();
                add_ezair_li();
                Get_ezairmode_ui();
                ezairmode_listener();
                $("#ezcast_opt").css('display', 'block');
                $("#log_txt").css('display', 'block');
                Get_resolution();
                Check_get_netlink();
                $("#lanlist").remove();
                //get_Lan__language();
                //get_LanSettingVal();
                //get_LanAutomatic();
                //get_wifiAutomatic();
                //Get_Lan_mac_ip();
                //ezlan_listener();
                //Get_connected_ssid();
                check_default_mode();
                //Get_5GChannel_country();
                //add_wifi_client_mode_li();
                //wifi_client_mode_listener();
                add_miracode_li();
                get_miracode_text();
                set_miracode_listener();
                add_casecode_li();
                get_passcode_text();
                set_passcode_listener();
                Get_control_access__language();
                add_airview_li();
                airview_listener();
                Get_airview_text();
                break; 
            case 7:
                //EZWIRE type, 0: EZWire with 8251/8251W; 2:MiraWireDuo with 8252C; 3: MiraWirePlus with 8252B/8252W; 4: EZDocking with 8251W;
                $("#set_title_admin").remove();
                $("#set_title").css('display', 'block');
                $("#li_defaultmode").remove();
                $("#menu_left,#menu_right").remove();
                $("#connectionmode").remove();
                $("#hdmi_cec").remove();
                $("#ezcast_opt").css('display', 'none');
                $("#channel_set").remove();
                $("#ignoressid_control").remove();
                $("#adminpas,#reset_init").remove();
                $("#modelout,#Set_EQ_mode,#wifilist,#lanlist,#pass_sel,#device_sel,#ezchannelmode,#channel_set").remove();
                $("#showinfo,#anyinfo,#more_info").remove();
                $("#ezwire_info").show();
                Get_resolution();
                add_ezwire_audio_li();
                Get_ezwire_audio_ui();
                ezwire_audio_listener();
                get_ezwire_audio__text();
                $('#ezwire_id').text(totalData.ssid);  //for ezwire
                //Get_connected_ssid();
                break;
            case 8:
                //not audio:  1:MiraWire with 8252B;  5:SNOR flash with 8252N; 6: MiraWire with CDC/EDM; 7: MiraWire with CDC/EDM at snor
                $("#set_title_admin").remove();
                $("#set_title").css('display', 'block');
                $("#li_defaultmode").remove();
                $("#menu_left,#menu_right").remove();
                $("#connectionmode").remove();
                $("#hdmi_cec").remove();
                $("#ezcast_opt").css('display', 'none');
                $("#channel_set").remove();
                $("#ignoressid_control").remove();
                $("#adminpas,#reset_init").remove();
                $("#modelout,#Set_EQ_mode,#wifilist,#lanlist,#pass_sel,#device_sel,#ezchannelmode,#channel_set").remove();
                $("#showinfo,#anyinfo,#more_info").remove();
                $("#ezwire_info").show();
                Get_resolution();
                $('#ezwire_id').text(totalData.ssid);  //for ezwire
                //Get_connected_ssid();
                break;
            default:
                $("#set_title_admin").remove();
                $("#modelout").remove();
                $("#Set_EQ_mode").remove();
                $("#ezch_play_way_t").remove();
                $("#ezchannelmode").remove();
                $("#hdmi_cec").remove();
                $("#channel_set").remove();
                $("#ignoressid_control").remove();
                break;
        }
        if(ezmusic_flag==1||ezmusic_flag == 2||ezmusic_flag ==3) {
            Get_ezchannel();
            Get_ezchannel_volume();
            Get_ezchannel_waittime();
            Get_ezchannel_playway();
            ezchannel_listener();
            if (ezmusic_flag !=1) {
                Get_ezchannel_playlist();  //EZMUSIC delete this function
                Get_ezchannel_ssid(); 
            }
        }
        if (ezmusic_flag == 5 || ezmusic_flag == 4) {
            $("#channel_set").children("a").attr('href', '#Channel_24G');
        }
        if (ezmusic_flag == 6 || ezmusic_flag == 9) {
            $("#li_defaultmode").remove();
	     defaultmode_array[0] = "AP+LAN";
	     defaultmode_array[1] = "AP+P2P+LAN";
	     $("#hdmecec_txt").text("HDMI");
	     $("#hdme_cec_txt").remove();
	     $("#dlt_0").text("AP+LAN");
	     $("#dlt_1").text("AP+P2P+LAN");
	     $("#wlan0").text("5G MAC : ");
	     Get_defaultmode_UI();// first read data
	     Get_24G_mac_ip();
	     $("#for_box_mac").show();
            add_airdiskautoplay_li();
            Get_airdiskautoplay_text();
            airdiskautoplay_listener();
            //get_ignore_ssid();
        } 

    },"text");
}

function To_snmp() {
        location.href = "snmp.html";
}
function To_wifiEnterprise() {
        location.href = "upload.html";
}
function To_host_authority() {
        location.href = "host_authority.html";
}
var Lan_Sta = 0;
function To_wifilist() {
    if (Lan_Sta == 0) 
        location.href = "wifilist.html";
}
function To_wifilist_advanced() {
    if (Lan_Sta == 0) 
        location.href = "wifilist.html?advanced=1";
}
function lan_status() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    var ret = 0;
    $.get(cgiurl, {type: 'get_netlink'},function(get_netlink) {
        ret = Number(get_netlink);
        Lan_Sta = ret;
    },"text");
    if (Lan_Sta == 1) {
        $('#connected_ssid').text(warn_msg);
        $('#connected_ssid').show();
    }
}
function Check_lan_in() {
    lan_status();
    setInterval(lan_status, 10000);

}
//read 5g country list text
function Get_5GChannel_country() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_ProboxConnectStatus'}, function(ProboxConnectStatus) {
        wifi_mode_flag = Number(ProboxConnectStatus);
    }, "text");
    $.get(cgiurl, {type: 'get_5G_country'}, function(country5G) {
        if (country5G.length > 0) {
            start = 0;
            text_string_length = 0;
            text_string = country5G;
            text_string_length = country5G.length;
            for (var i = 0; i < Channel_5G_country.length; i++) {
                Channel_5G_country[i] = Get_text_from_textstring(text_string);
                text_string = text_string_global;
            }
            $("#5G_country_val-button").children("span").text(Channel_5G_country[0]);
            for (i = 0; i < Channel_5G_country.length; i++) {
                document.getElementById("5G_country_val").add(new Option(Channel_5G_country[i], i));
            }
            Get_softap_5g_initvalue();
        }
    },"text");
}
//read 5G Country,Channel value
function Get_softap_5g_initvalue() {
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'get_5G_initvalue'},function(ret) {
        if (ret.length) {
            start = 0;
            text_string_length = 0;
            text_string = ret;
            text_string_length = ret.length;
            get_channel_5G_country_index = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            get_channel_5G_init = Get_text_from_textstring(text_string);
            text_string = text_string_global;
            get_Bandwidth_5G_init = Get_text_from_textstring(text_string);
            set_init_5g_channel();
        }
    },"text");
}
//creat 5G Country,Channel list
function set_init_5g_channel() {
    $("#5G_Bandwidth_val-button").children("span").text(Bandwidth[get_Bandwidth_5G_init]);
    $("#5G_Bandwidth_val").find("option[value="+get_Bandwidth_5G_init+"]").attr("selected",true); 
    $("#5G_country_val-button").children("span").text(Channel_5G_country[get_channel_5G_country_index]);
    $("#5G_country_val").find("option[value="+get_channel_5G_country_index+"]").attr("selected",true); 
    var sel = document.getElementById("5G_Channel_val");
    sel.options.length = 0;
    var vale = Number(get_channel_5G_country_index);
    switch (vale) {
        case 0:
        case 6:
        case 14:
        case 15:
        case 16:
        case 17:
        case 19:
        case 20:
            $("#5G_Channel_val-button").children("span").text(country1[0]);
            for (i = 0; i < country1.length; i++) {
                sel.add(new Option(country1[i], country1[i]));
            }
            break;
        case 1:
        case 2:
        case 5:
        case 9:
        case 10:
        case 18:
        case 21:
        case 22:
        case 24:
            $("#5G_Channel_val-button").children("span").text(country2[0]);
            for (i = 0; i < country2.length; i++) {
                sel.add(new Option(country2[i], country2[i]));
            }
            break;
        case 3:
        case 11:
        case 12:
        case 13:
            $("#5G_Channel_val-button").children("span").text(country4[0]);
            for (i = 0; i < country4.length; i++) {
                sel.add(new Option(country4[i], country4[i]));
            }
            break;
        case 4:
            $("#5G_Channel_val-button").children("span").text(country5[0]);
            for (i = 0; i < country5.length; i++) {
                sel.add(new Option(country5[i], country5[i]));
            }
            break;
        case 7:
            $("#5G_Channel_val-button").children("span").text(country8[0]);
            for (i = 0; i < country8.length; i++) {
                sel.add(new Option(country8[i], country8[i]));
            }
            break;
        case 8:
            $("#5G_Channel_val-button").children("span").text(country9[0]);
            for (i = 0; i < country9.length; i++) {
                sel.add(new Option(country9[i], country9[i]));
            }
            break;
        case 23:
            $("#5G_Channel_val-button").children("span").text(country19[0]);
            for (i = 0; i < country19.length; i++) {
                sel.add(new Option(country19[i], country19[i]));
            }
            break;
        default:
            //alert("--------error-------");
            break;
    }

    if (Number(get_channel_5G_init) > 0) {
        $("#5G_Channel_val-button").children("span").text(get_channel_5G_init);
        $("#5G_Channel_val").find("option[value="+get_channel_5G_init+"]").attr("selected",true); 
        $('#current_5channel').text("5G: "+get_channel_5G_init+"("+Channel_5G_country[get_channel_5G_country_index]+")");
    }else{
        $('#current_5channel').text("5G: "+"Auto("+Autochannel_5G_val+","+Channel_5G_country[get_channel_5G_country_index]+")");
    }
}

var ota_server_ENABLE=0;
function Check_ota_server_ENABLE2()
{
	$.get("cgi-bin/wifi_info_GET.cgi", {type:'OTA_SERVER_ENABLE'}, function(ret) 
	{	
		ota_server_ENABLE=Number(ret);
		//alert("Check_ota_server_ENABLE:"+ota_server_ENABLE);
		if(ota_server_ENABLE)
			$('#otaserver').show();
	}, "text");			
}

function check_Get_first_all_data(){
	console.log("--check_Get_first_all_data--"+totalData.ssid+" -- "+totalData.password);
	if(totalData.ssid=="abc" || totalData.password =="abc")
		Get_first_all_data();	

}
function Get_first_all_data() {
    $('#anyinfo').css('display', 'none');
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type: 'read_total_resource'},function(alldata) {
        if (alldata.length>0) {
		//alert(alldata);
            var strs= new Array(); //定义一数组
            strs=alldata.split("\t");
            totalData.wifimode=strs[0];
            totalData.lanset=strs[1];
            totalData.language=strs[2];
            totalData.devcename=strs[3];
            totalData.defaultmode=strs[4];
            totalData.resolution=strs[5];
            totalData.connection=strs[6];
            totalData.ssid=strs[7];
            totalData.password=strs[8];
            console.log(totalData.wifimode+"/t"+totalData.lanset+"/t"+totalData.language+"/t"+totalData.devcename+"/t"+totalData.defaultmode+"/t"+totalData.resolution+"/t"+totalData.connection+"/t"+totalData.password);
	    $('#language').text(totalData.language);
	    $('#device_name').text(totalData.devcename);
	    $('#app_sk').text(totalData.password);
	    $('#ap_psk').text(totalData.password);
	    $('#ap_ssid').text(totalData.ssid);
		}
	else{
            Get_first_all_data();
	}
    },"text");
    setTimeout(check_Get_first_all_data,3000);

}
function get_hidewifipass(){
    var cgiurl = "cgi-bin/wifi_info_GET.cgi?timeStamp=" + new Date().getTime();
    $.get(cgiurl, {type:'1\nget\nPSK_hide\nCONFNAME_WR\n'}, function(ret)
    {	
        psk_visibility_idx =Number(ret);
    },"text");
}
//websetting.htm -> Device Management
function delay_get_data_function() {
    if (delay_get_data_id > 0) {
        clearInterval(delay_get_data_id);
        delay_get_data_id = -1;
    }
    get_Lan__language();
    get_hidewifipass();
    Check_EZMUSIC_ENABLE_flag();
    Get_multilanguage_timer();  //call: Get_current_language: first read data
    Get_password();// first read data
    Get_AP_ssid();// first read data
    Get_devicename();// first read data
    Get_AP_mac_ip();
    Get_local_version();
    //Get_defaultmode_language(); // not user ?
    //get_EQ_default();
    Get_index_language();
    Get_language_txt(); 
    delay_get_data_id = self.setInterval(click_listener, 1000);

}

//websettint_admin.htm
function delay_get_data_admin_function() {
    if (delay_get_data_id > 0) {
        clearInterval(delay_get_data_id);
        delay_get_data_id = -1;
    }
    get_Lan__language();
    get_hidewifipass();
    Check_admin_setup_flag();
    //Get_multilanguage_timer();  //    last run: show_ui()
    Get_password();
    Get_AP_ssid();
    Get_AP_mac_ip();
    Get_local_version();
    //Get_set_password_text();
    Get_ezairmode_ui();
    ezairmode_listener();
    //Get_defaultmode_UI();
    Get_index_language();
    Get_language_txt(); 
    delay_get_data_id = self.setInterval(click_listener, 1000);
	
}

function delay_get_data_networsetup_function() {
    if (delay_get_data_id > 0) {
        clearInterval(delay_get_data_id);
        delay_get_data_id = -1;
    }
    get_Lan__language();
    get_hidewifipass();
    Check_network_setup_flag();
    //Get_multilanguage_timer();  //    last run: show_ui()
    Get_index_language();
    Get_language_txt(); 
    Get_password();
    Get_AP_ssid();
    Get_AP_mac_ip();
    Get_local_version();
    //Get_defaultmode_UI();
    //Get_defaultmode_language();
    //Get_router_ctl();
    Get_softap_channel();
    Get_router_only_text();
    Check_ota_server_ENABLE2();
    delay_get_data_id = self.setInterval(click_listener, 1000);

}

function updata_proboxlanonly_info(){
    $.get("cgi-bin/wifi_info_GET.cgi", {type: 'EZMUSIC_ENABLE'},function(ezmusic_enable) {
        ezmusic_flag = Number(ezmusic_enable);
        if (ezmusic_flag == 9) {
	        $("#ap_ssid_txt").text("ID : ");
	        $("#ap_ssid").text(totalData.devcename);//devicename
	        $("#ap_psk_txt").text("Firmware Version : ");
	        $("#ap_psk").text($('#current_ver').text());
	        $("#wlan0").text("LAN MAC : ");
	        $("#connect_mac_address").text($('#LAN_mac_address').text());
	        Get_Lan_ip();
	        $("#for_other").remove();
	        $("#for_other2").remove();
	        $("#for_box_mac").remove();
			
        }
    },"text");
}
//check from airsetup or advanced
function check_parameter(){
    var urlinfo=window.location.href; 
    var offset=urlinfo.indexOf("advanced=1");   
    if(offset>0){
        advance_flag=1;
        var str1="<a class='ui-link ui-btn-left ui-btn ui-icon-bars ui-btn-icon-notext ui-shadow ui-corner-all' id='pro_1' role='button' href='#nav-panel' data-role='button' data-iconpos='notext' data-icon='bars'>Menu</a>";
        var str2="<a class='ui-link ui-btn-right ui-btn ui-icon-gear ui-btn-icon-notext ui-shadow ui-corner-all' id='pro_2' role='button' href='#add-form' data-role='button' data-iconpos='notext' data-icon='gear'>Add</a>";
        var str3="<span id='log_txt' style='top: 13px; right: 42px; font-size: 80%; font-weight: normal; display: none; position: absolute;'></span>";
        $("[data-role='header']:first").prepend(str3);
        $("[data-role='header']:first").prepend(str2);
        $("[data-role='header']:first").prepend(str1);
    } else {
        var str1="<a class='ui-btn ui-corner-all ui-icon-refresh ui-btn-icon-notext ui-btn-left' id='menu_left' onclick='window.location.reload()' href='#' disable=''>setup</a>";
        var str2="<a class='ui-link ui-btn-right ui-btn ui-icon-back ui-btn-icon-notext ui-shadow ui-corner-all' id='menu_right' role='button' onclick='javascript:back_ezcastapp();' href='#' data-role='button' data-iconpos='notext' data-icon='back'>back</a>";
        $("[data-role='header']:first").prepend(str2);
        $("[data-role='header']:first").prepend(str1);
    }

}
//first run
function delay_get_data() {
    plat_flag="devicemanagement";
    if (delay_get_data_id > 0) {
        clearInterval(delay_get_data_id);
        delay_get_data_id = -1;
    }
    delay_get_data_id = self.setInterval(delay_get_data_function, 600); 
    check_parameter();
}
//first run
function delay_get_data_admin() {
    plat_flag="adminsetup";
    if (delay_get_data_id > 0) {
        clearInterval(delay_get_data_id);
        delay_get_data_id = -1;
    }
    delay_get_data_id = self.setInterval(delay_get_data_admin_function, 600); 
}
//first run
function delay_get_data_networsetup() {
    plat_flag="networsetup";
    if (delay_get_data_id > 0) {
        clearInterval(delay_get_data_id);
        delay_get_data_id = -1;
    }
    delay_get_data_id = self.setInterval(delay_get_data_networsetup_function, 600); 
}

function about_get_data(){
    $.get("cgi-bin/wifi_info_GET.cgi", {type: 'EZMUSIC_ENABLE'},function(ezmusic_enable) {
        ezmusic_flag = Number(ezmusic_enable);
        Get_local_version();
        Get_AP_mac_ip();
        Get_24G_mac_ip();
        Get_Lan_mac_ip();
        $("#about_ssid").text(totalData.ssid);
		
        if(ezmusic_flag == 9){
            Get_Lan_ip();
            $("#ap_ssid_txt").text("ID : ");
            $("#mac_5g").remove();
            $("#mac_24").remove();
            $("#about_ssid").text(totalData.devcename);
        }
        else if (ezmusic_flag == 5){
            $("#mac_5g").remove();
        }
        else if (ezmusic_flag == 4){
            $("#mac_5g").remove();
            $("#mac_lan").remove();
        }



        $(".row1").show();
    },"text");

}

