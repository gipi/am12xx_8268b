var CGI_URL='./cgi-bin/conference_control.cgi?'
var IMAGE_FOLDER='./img/conference/'
var intervalID;
var i,onoff_flag;

var LanguageEnglish  = {    	
    Conf_Control          			: 'Conference Control',
    ConfCtrl_on_display   			: 'ON',
    ConfCtrl_off_display  			: 'OFF',
    HostName              			: 'User Name',
    CtrlAction            			: 'Display Location',
    IpAddress             			: 'IP address'   
}

var LanguageGerman  = {    	
    Conf_Control          			: 'Konferenzsteuerung',
    ConfCtrl_on_display   			: 'Ein',
    ConfCtrl_off_display  			: 'Aus',
    HostName              			: 'Benutzername',
    CtrlAction            			: 'Anzeigeort',
    IpAddress             			: 'IP-Adresse'   
}

var LanguageFrench  = {    	
    Conf_Control          			: 'Contrôle de conférence ',
    ConfCtrl_on_display   			: 'Activ.',
    ConfCtrl_off_display  			: 'Désact.',
    HostName              			: 'Non dutilisateur',
    CtrlAction            			: 'Emplacement daffichage',
    IpAddress             			: 'Adresse IP '   
}

var LanguageItalian  = {    	
    Conf_Control          			: 'Controllo conferenza',
    ConfCtrl_on_display   			: 'ON',
    ConfCtrl_off_display  			: 'OFF',
    HostName              			: 'Nome utente',
    CtrlAction            			: 'Posizione display',
    IpAddress             			: 'Indirizzo IP'   
}

var LanguageSpain  = {    	
    Conf_Control          			: 'Control de conferencias',
    ConfCtrl_on_display   			: 'Activado',
    ConfCtrl_off_display  			: 'Desactivado',
    HostName              			: 'Nombre de usuario',
    CtrlAction            			: 'Ubicación de visualización',
    IpAddress             			: 'Dirección IP'   
}

var LanguagePortuguese  = {    	
    Conf_Control          			: 'Controlo de conferência',
    ConfCtrl_on_display   			: 'Ligar',
    ConfCtrl_off_display  			: 'Desligar',
    HostName              			: 'Nome do utilizador',
    CtrlAction            			: 'Mostrar localização',
    IpAddress             			: 'Endereço IP'   
}

var LanguageJP  = {
    Conf_Control                    : '会議のコントロール',
    ConfCtrl_on_display             : 'オン',
    ConfCtrl_off_display            : 'オフ',
    HostName                    	: 'ユーザー名',
    CtrlAction                      : '表示場所',
    IpAddress                       : 'IP アドレス'    
}
                                	
var LanguageTC  = {         		
    Conf_Control           			: '會議控制',
    ConfCtrl_on_display    			: '開啟',
    ConfCtrl_off_display   			: '關閉',
    HostName               			: '使用者名稱',
    CtrlAction             		  	: '顯示器位置',
    IpAddress              	  		: 'IP 位址'     
}

var LanguageSC  = {
    Conf_Control                    : '会议控制',
    ConfCtrl_on_display             : '打开',
    ConfCtrl_off_display            : '关闭',
    HostName                        : '用户名',
    CtrlAction                      : '显示位置',
    IpAddress                       : 'IP 地址'    
}

var LanguagePolish  = {    	
    Conf_Control          			: 'Sterowanie konferencyjne',
    ConfCtrl_on_display   			: 'Wł.',
    ConfCtrl_off_display  			: 'Wył.',
    HostName              			: 'Nazwa użytkownika',
    CtrlAction            			: 'Lokalizacja wyświetlacza',
    IpAddress             			: 'Adres IP'   
}

var LanguageRussian  = {    	
    Conf_Control          			: 'Управление конференцией',
    ConfCtrl_on_display   			: 'Вкл.',
    ConfCtrl_off_display  			: 'Выкл.',
    HostName              			: 'Имя пользователя',
    CtrlAction            			: 'Местоположение дисплея',
    IpAddress             			: 'IP-адрес'   
}

var LanguageThai  = {    	
    Conf_Control          			: 'การควบคุมในการประชุม',
    ConfCtrl_on_display   			: 'เปิด',
    ConfCtrl_off_display  			: 'ปิด',
    HostName              			: 'ชื่อผู้ใช้',
    CtrlAction            			: 'แสดงตำแหน่ง',
    IpAddress             			: 'ที่อยู่ IP'   
}

var LanguageDutch  = {    	
    Conf_Control          			: 'Vergaderingbesturing',
    ConfCtrl_on_display   			: 'Aan',
    ConfCtrl_off_display  			: 'Uit',
    HostName              			: 'Gebruikersnaam',
    CtrlAction            			: 'Locatie weergeven',
    IpAddress             			: 'IP-adres'   
}

var LanguageCzech  = {    	
    Conf_Control          			: 'Konferenční ovládání',
    ConfCtrl_on_display   			: 'Zapnuto',
    ConfCtrl_off_display  			: 'Vypnuto',
    HostName              			: 'Uživatelské jméno',
    CtrlAction            			: 'Umístění displeje',
    IpAddress             			: 'Adresa IP'   
}

var LanguageSwedish  = {    	
    Conf_Control          			: 'Konferenskontroll',
    ConfCtrl_on_display   			: 'På',
    ConfCtrl_off_display  			: 'Av',
    HostName              			: 'Användarnamn',
    CtrlAction            			: 'Skärmplats',
    IpAddress             			: 'IP-adress'   
}

var LanguageTurkish  = {    	
    Conf_Control          			: 'Konferans kontrolü',
    ConfCtrl_on_display   			: 'Açık',
    ConfCtrl_off_display  			: 'Kapalı',
    HostName              			: 'Kullanıcı Adı',
    CtrlAction            			: 'Gösterim Konumu',
    IpAddress             			: 'IP Adresi'   
}

var LanguageKorean  = {    	
    Conf_Control          			: '회의 관리',
    ConfCtrl_on_display   			: '켜짐',
    ConfCtrl_off_display  			: '꺼짐',
    HostName              			: '사용자 이름',
    CtrlAction            			: '디스플레이 위치',
    IpAddress             			: 'IP 주소'   
}

var LanguageHungarian  = {    	
    Conf_Control          			: 'Konferencia vezérlés',
    ConfCtrl_on_display   			: 'Be',
    ConfCtrl_off_display  			: 'Ki',
    HostName              			: 'Felhasználónév',
    CtrlAction            			: 'Kijelző helye',
    IpAddress             			: 'IP cím'   
}

var LanguageCroatian  = {    	
    Conf_Control          			: 'Konferencijsko upravljanje',
    ConfCtrl_on_display   			: 'Uklj.',
    ConfCtrl_off_display  			: 'Isklj.',
    HostName              			: 'Korisničko ime',
    CtrlAction            			: 'Smještaj zaslona',
    IpAddress             			: 'IP adresa'   
}

var LanguageRomanian  = {    	
    Conf_Control          			: 'Control conferinţă',
    ConfCtrl_on_display   			: 'Pornit',
    ConfCtrl_off_display  			: 'Oprit',
    HostName              			: 'Nume utilizator',
    CtrlAction            			: 'Locaţie afişaj',
    IpAddress             			: 'Adresă IP'   
}

var LanguageNorwegian  = {    	
    Conf_Control          			: 'Konferansekontroll',
    ConfCtrl_on_display   			: 'På',
    ConfCtrl_off_display  			: 'Av',
    HostName              			: 'Brukernavn',
    CtrlAction            			: 'Visning fra',
    IpAddress             			: 'IP-adresse'   
}

var LanguageDanish  = {    	
    Conf_Control          			: 'Konferencekontrol',
    ConfCtrl_on_display   			: 'Til',
    ConfCtrl_off_display  			: 'Fra',
    HostName              			: 'Brugernavn',
    CtrlAction            			: 'Skærmplacering',
    IpAddress             			: 'IP-adresse'   
}

var LanguageBulgarian  = {    	
    Conf_Control          			: 'Управление на конференция',
    ConfCtrl_on_display   			: 'Вкл.',
    ConfCtrl_off_display  			: 'Изкл.',
    HostName              			: 'Потребителско име',
    CtrlAction            			: 'Местоположение на дисплея',
    IpAddress             			: 'IP адрес'   
}

var LanguageFinnish  = {    	
    Conf_Control          			: 'Konferenssin ohjaus',
    ConfCtrl_on_display   			: 'Päälle',
    ConfCtrl_off_display  			: 'Pois',
    HostName              			: 'Käyttäjänimi',
    CtrlAction            			: 'Näytön sijainti',
    IpAddress             			: 'IP-osoite'   
}

function LanguageSelection()
{
	var lang = navigator.language || navigator.userLanguage;

	L10NString = LanguageEnglish;
	
	if (lang.substring(0,2) == "de")
	{
		L10NString = LanguageGerman;
	}
	else if (lang.substring(0,2) == "fr")
	{
		L10NString = LanguageFrench;	
	}
	else if (lang.substring(0,2) == "it")
	{
		L10NString = LanguageItalian;	
	}
	else if (lang.substring(0,2) == "es")
	{
		L10NString = LanguageSpain;	
	}
	else if (lang.substring(0,2) == "pt")
	{
		L10NString = LanguagePortuguese;	
	}
	else if (lang.substring(0,2) == "ja")
	{
		L10NString = LanguageJP;	
	}
	else if (lang.substring(0,2) == "zh")
	{
		if (lang.toLowerCase() == "zh-tw" || lang.toLowerCase() == "zh-hk")
		{
			L10NString = LanguageTC;
		}
		else
		{
			L10NString = LanguageSC;
		}
	}
	else if (lang.substring(0,2) == "pl")
	{
		L10NString = LanguagePolish;	
	}
	else if (lang.substring(0,2) == "ru")
	{
		L10NString = LanguageRussian;	
	}
	else if (lang.substring(0,2) == "ts")
	{
		L10NString = LanguageThai;	
	}
	else if (lang.substring(0,2) == "nl")
	{
		L10NString = LanguageDutch;	
	}
	else if (lang.substring(0,2) == "cs")
	{
		L10NString = LanguageCzech;	
	}
	else if (lang.substring(0,2) == "sv")
	{
		L10NString = LanguageSwedish;	
	}
	else if (lang.substring(0,2) == "tr")
	{
		L10NString = LanguageTurkish;	
	}
	else if (lang.substring(0,2) == "ko")
	{
		L10NString = LanguageKorean;	
	}
	else if (lang.substring(0,2) == "hu")
	{
		L10NString = LanguageHungarian;	
	}
	else if (lang.substring(0,2) == "hr")
	{
		L10NString = LanguageCroatian;	
	}
	else if (lang.substring(0,2) == "ro")
	{
		L10NString = LanguageRomanian;	
	}
	else if (lang.substring(0,2) == "no")
	{
		L10NString = LanguageNorwegian;	
	}
	else if (lang.substring(0,2) == "da")
	{
		L10NString = LanguageDanish;	
	}
	else if (lang.substring(0,2) == "bg")
	{
		L10NString = LanguageBulgarian;	
	}
	else if (lang.substring(0,2) == "fi")
	{
		L10NString = LanguageFinnish;	
	}

$("#Conf_Control").text(L10NString['Conf_Control']);
$("#ConfCtrl_on_display").text(L10NString['ConfCtrl_on_display']);
$("#ConfCtrl_off_display").text(L10NString['ConfCtrl_off_display']);
$("#ipaddress").text(L10NString['IpAddress']);
$("#ctrlaction").text(L10NString['CtrlAction']);
$("#hostname").text(L10NString['HostName']);
/*
	document.getElementById('Conf_Control').innerHTML = L10NString['Conf_Control'];
	document.getElementById('ConfCtrl_on_display').innerHTML = L10NString['ConfCtrl_on_display'];
	document.getElementById('ConfCtrl_off_display').innerHTML = L10NString['ConfCtrl_off_display'];
	document.getElementById('HostName').innerHTML = L10NString['HostName'];
	document.getElementById('CtrlAction').innerHTML = L10NString['CtrlAction'];
	document.getElementById('IpAddress').innerHTML = L10NString['IpAddress'];
*/
}

function setCookie(c_name,value,expiredays)
{
	var exdate=new Date()
	exdate.setDate(exdate.getDate()+expiredays)
	document.cookie=c_name+ "=" +escape(value)+ ((expiredays==null) ? "" : ";expires="+exdate.toGMTString())
}

function getCookie(c_name)
{
	if (document.cookie.length>0)
	{
		c_start=document.cookie.indexOf(c_name + "=")
		if (c_start!=-1)
		{ 
			c_start=c_start + c_name.length+1 
			c_end=document.cookie.indexOf(";",c_start)
			if (c_end==-1) c_end=document.cookie.length
			return unescape(document.cookie.substring(c_start,c_end))
		} 
	}
	return ""
}

window.onload = function() 
{
	//LanguageSelection();
	onoff_flag=1;
/*
	if (getCookie('ConfCtrl') == 'on')
	{
	$("#ConfCtrl_on_off_display").text("ON");
	onoff_flag=1;
		//$("#conference_on").checked = true;
		//document.getElementById('conference_on').checked = true;
	}
	else{
		$("#ConfCtrl_on_off_display").text("OFF");
		onoff_flag=0;
		}
	//alert("----");

	$("#ConfCtrl_on_off_display").click(function(){
		if(onoff_flag){
			$("#ConfCtrl_on_off_display").text("OFF");
			onoff_flag=0;

		}
		else{
			$("#ConfCtrl_on_off_display").text("ON");
			onoff_flag=1;
		}
		ConfCrtl_onoff();
		});
*/
	
	ConfCrtl_onoff();
}

function Disconnect(obj)
{
 //    var tbl=document.getElementById('Connection_Status');
 //    var ip = tbl.rows[obj+2].cells[5].innerHTML;
var ip=obj;
    $.ajax({
            type: 'GET',
			cache: false,
            url: CGI_URL+"type=control&control=control_disconnect&ip_address="+ip,
            dataType: 'text',
            success: function (data) {
				if(data == 1)
				{
					ConfCrtl_onoff();
				}
			},
           error: function (e) {
               }
           });
}

function Stop(obj)
{
 //    var tbl=document.getElementById('Connection_Status');
 //    var ip = tbl.rows[obj+2].cells[5].innerHTML;
var ip=obj;

    $.ajax({
            type: 'GET',
			cache: false,
            url: CGI_URL+"type=control&control=control_stop&ip_address="+ip,
            dataType: 'text',
            success: function (data) {
				if(data == 1)
				{
					ConfCrtl_onoff();
				}
           },
           error: function (e) {
               }
           });
}

function Change(obj, splitCount, position)
{
  //   var tbl=document.getElementById('Connection_Status');
 //    var ip = tbl.rows[obj+2].cells[5].innerHTML;
var ip=obj;
    $.ajax({
            type: 'GET',
			cache: false,
            url: CGI_URL+"type=control&control=control_change&split_count="+splitCount+"&position="+position+"&ip_address="+ip,
            dataType: 'text',
            success: function (data) {
				if(data == 1)
				{
					ConfCrtl_onoff();
				}
                },
           error: function (e) {
               }
           });
}
/*
function AddStatus(obj,splitCount,position,RowId)
{
  var ConnectionStatus = " <a href=\"#\"><img src=\""+IMAGE_FOLDER+"screen_none.png\" width=\"21\" height=\"20\" border=\"0\" style=' margin:0 3px 0 3px' onclick = \"Disconnect("+RowId+")\"/></a>";

  if(splitCount == 1 && position == 1)
     ConnectionStatus = ConnectionStatus + " <img src=\""+IMAGE_FOLDER+"screen_full_hl.png\" width=\"21\" height=\"20\" border=\"0\" style=' margin:0 3px 0 3px' />";
  else
     ConnectionStatus = ConnectionStatus + " <a href=\"#\"><img src=\""+IMAGE_FOLDER+"screen_full.png\" width=\"21\" height=\"20\" border=\"0\" style=' margin:0 3px 0 3px' onclick =\"Change("+RowId+",1,1)\" /></a>";

  if(splitCount == 2 && position == 1)
        ConnectionStatus = ConnectionStatus + " <img src=\""+IMAGE_FOLDER+"2-1.png\" width=\"21\" height=\"20\" border=\"0\" style=' margin:0 3px 0 3px' />";
  else
      ConnectionStatus = ConnectionStatus + " <a href=\"#\"><img src=\""+IMAGE_FOLDER+"2.png\" width=\"21\" height=\"20\" border=\"0\" style=' margin:0 3px 0 3px' onclick =\"Change("+RowId+",2,1)\" /></a>";

  if(splitCount == 2 && position == 2)
        ConnectionStatus = ConnectionStatus + " <img src=\""+IMAGE_FOLDER+"3-1.png\" width=\"21\" height=\"20\" border=\"0\" style=' margin:0 3px 0 3px' />";
  else
      ConnectionStatus = ConnectionStatus + " <a href=\"#\"><img src=\""+IMAGE_FOLDER+"3.png\" width=\"21\" height=\"20\" border=\"0\" style=' margin:0 3px 0 3px' onclick =\"Change("+RowId+",2,2)\" /></a>";

  if(splitCount == 4 && position == 1)
        ConnectionStatus = ConnectionStatus + " <img src=\""+IMAGE_FOLDER+"4-1.png\" width=\"21\" height=\"20\" border=\"0\" style=' margin:0 3px 0 3px' />";
  else
      ConnectionStatus = ConnectionStatus + " <a href=\"#\"><img src=\""+IMAGE_FOLDER+"4.png\" width=\"21\" height=\"20\" border=\"0\" style=' margin:0 3px 0 3px' onclick =\"Change("+RowId+",4,1)\" /></a>";

  if(splitCount == 4 && position == 2)
        ConnectionStatus = ConnectionStatus + " <img src=\""+IMAGE_FOLDER+"5-1.png\" width=\"21\" height=\"20\" border=\"0\" style=' margin:0 3px 0 3px' />";
  else
      ConnectionStatus = ConnectionStatus + " <a href=\"#\"><img src=\""+IMAGE_FOLDER+"5.png\" width=\"21\" height=\"20\" border=\"0\" style=' margin:0 3px 0 3px' onclick =\"Change("+RowId+",4,2)\" /></a>";

  if(splitCount == 4 && position == 3)
        ConnectionStatus = ConnectionStatus + " <img src=\""+IMAGE_FOLDER+"6-1.png\" width=\"21\" height=\"20\" border=\"0\" style=' margin:0 3px 0 3px' />";
  else
      ConnectionStatus = ConnectionStatus + " <a href=\"#\"><img src=\""+IMAGE_FOLDER+"6.png\" width=\"21\" height=\"20\" border=\"0\" style=' margin:0 3px 0 3px' onclick =\"Change("+RowId+",4,3)\" /></a>";

  if(splitCount == 4 && position == 4)
        ConnectionStatus = ConnectionStatus + " <img src=\""+IMAGE_FOLDER+"7-1.png\" width=\"21\" height=\"20\" border=\"0\" style=' margin:0 3px 0 3px' />";
  else
      ConnectionStatus = ConnectionStatus + " <a href=\"#\"><img src=\""+IMAGE_FOLDER+"7.png\" width=\"21\" height=\"20\" border=\"0\" style=' margin:0 3px 0 3px' onclick =\"Change("+RowId+",4,4)\" /></a>";
    
  if(position == 0)
     ConnectionStatus = ConnectionStatus + " <img src=\""+IMAGE_FOLDER+"8.png\" width=\"21\" height=\"20\" border=\"0\" style=' margin:0 3px 0 3px' />";
  else
     ConnectionStatus = ConnectionStatus + " <a href=\"#\"><img src=\""+IMAGE_FOLDER+"8-1.png\" width=\"21\" height=\"20\" border=\"0\" style=' margin:0 3px 0 3px' onclick = \"Stop("+RowId+")\" /></a>";

  obj.innerHTML = ConnectionStatus;
}
*/

function Changeimg(ip,splitCount,position)
{
  var ConnectionStatus = " <a href=\"#\"><img src=\""+IMAGE_FOLDER+"screen_none.png\"  class=\"img2\" onclick = \"Disconnect(\'"+ip+"\')\"/></a>";

  if(splitCount == 1 && position == 1)
     ConnectionStatus = ConnectionStatus + " <img src=\""+IMAGE_FOLDER+"screen_full_hl.png\" class=\"img2\" >";
  else
     ConnectionStatus = ConnectionStatus + " <a href=\"#\"><img src=\""+IMAGE_FOLDER+"screen_full.png\" class=\"img2\" onclick =\"Change(\'"+ip+"\',1,1)\" /></></a>";
  if(splitCount == 2 && position == 1)
        ConnectionStatus = ConnectionStatus + " <img src=\""+IMAGE_FOLDER+"2-1.png\" class=\"img2\" />";
  else
      ConnectionStatus = ConnectionStatus + " <a href=\"#\"><img src=\""+IMAGE_FOLDER+"2.png\" class=\"img2\" onclick =\"Change(\'"+ip+"\',2,1)\" /></a>";

  if(splitCount == 2 && position == 2)
        ConnectionStatus = ConnectionStatus + " <img src=\""+IMAGE_FOLDER+"3-1.png\" class=\"img2\" />";
  else
      ConnectionStatus = ConnectionStatus + " <a href=\"#\"><img src=\""+IMAGE_FOLDER+"3.png\" class=\"img2\" onclick =\"Change(\'"+ip+"\',2,2)\" /></a>";

  if(splitCount == 4 && position == 1)
        ConnectionStatus = ConnectionStatus + " <img src=\""+IMAGE_FOLDER+"4-1.png\" class=\"img2\" />";
  else
      ConnectionStatus = ConnectionStatus + " <a href=\"#\"><img src=\""+IMAGE_FOLDER+"4.png\" class=\"img2\" onclick =\"Change(\'"+ip+"\',4,1)\" /></a>";

  if(splitCount == 4 && position == 2)
        ConnectionStatus = ConnectionStatus + " <img src=\""+IMAGE_FOLDER+"5-1.png\" class=\"img2\" />";
  else
      ConnectionStatus = ConnectionStatus + " <a href=\"#\"><img src=\""+IMAGE_FOLDER+"5.png\" class=\"img2\" onclick =\"Change(\'"+ip+"\',4,2)\" /></a>";

  if(splitCount == 4 && position == 3)
        ConnectionStatus = ConnectionStatus + " <img src=\""+IMAGE_FOLDER+"6-1.png\" class=\"img2\" />";
  else
      ConnectionStatus = ConnectionStatus + " <a href=\"#\"><img src=\""+IMAGE_FOLDER+"6.png\" class=\"img2\" onclick =\"Change(\'"+ip+"\',4,3)\" /></a>";

  if(splitCount == 4 && position == 4)
        ConnectionStatus = ConnectionStatus + " <img src=\""+IMAGE_FOLDER+"7-1.png\" class=\"img2\" />";
  else
      ConnectionStatus = ConnectionStatus + " <a href=\"#\"><img src=\""+IMAGE_FOLDER+"7.png\" class=\"img2\" onclick =\"Change(\'"+ip+"\',4,4)\" /></a>";
    
  if(position == 0)
     ConnectionStatus = ConnectionStatus + " <img src=\""+IMAGE_FOLDER+"8.png\" class=\"img2\" />";
  else
     ConnectionStatus = ConnectionStatus + " <a href=\"#\"><img src=\""+IMAGE_FOLDER+"8-1.png\" class=\"img2\" onclick = \"Stop(\'"+ip+"\')\" /></a>";

 return ConnectionStatus;
}

function AddNewRow(statusFrame, RowId, hn, splitCount, position, ip)
{
    var newTR = statusFrame.insertRow(statusFrame.rows.length);

	newTR.insertCell(0);
    var newHostname = newTR.insertCell(1);
    newTR.insertCell(2);
	newTR.insertCell(3);
	var newstatus = newTR.insertCell(4)
	newstatus.colSpan="3"
	var newipaddress = newTR.insertCell(5);
	newTR.insertCell(6);
    newHostname.innerHTML = hn;
    AddStatus(newstatus,splitCount,position,RowId);
    newipaddress.innerHTML = ip;
}


function setsplit(val){

$('#'+val).toggleClass("on");
//��һ�¶�ʱ���ٿ�
window.clearTimeout(intervalID);
intervalID = window.setTimeout("ConfCrtl_onoff();",30000);

	/*
	if($('#'+val).css("display")=="none"){
		$('#'+val).css('display','block');
		window.clearTimeout(intervalID);
		}
	else{
		$('#'+val).css('display','none');
		intervalID = window.setTimeout("ConfCrtl_onoff();",15000);
		}
*/	
}
function Creatlist(hn, splitCount, position, ip,ty,role){

//var userlist = "<div class='list' align='center'>"+hn+splitCount+position+ip+i+"</div>";
var userlist ="<div class='list' align='center'><span class='list_left'>"+ip+"<div class='text2'>"+hn+"</div></span><span  class='list_mid'>"+ty+"</span><span class='list_right'>"+role+"</span></div>";
$("#scan_client").append(userlist);
var temp="<div class='linebk'></div>";
$("#scan_client").append(temp);

//alert("--!!!");
}
function ConfCrtl_onoff()
{
	window.clearTimeout(intervalID);

//	var statusFrame = document.getElementById('Connection_Status');
/*
	for(var i=statusFrame.rows.length-1;i>=4;i--)
	{
		statusFrame.deleteRow(i);
	}
*/
//alert($("#customers").find(".list").length);
if($("#scan_client").find(".list").length)
{
	$(".list").remove();
	$(".linebk").remove();
}
//alert($("#customers").find(".list").length);

	//if(document.getElementById('conference_on').checked == true)
	if(onoff_flag)
	{
	//alert("----on---");
		$.ajax({
			type: 'GET',
			cache: false,
			url: CGI_URL + 'type=query_tcp_connections',
			dataType: 'xml',
			success: function (data) {
		//	alert($(data).find("ConnectInfo").eq(0));
				var splitCount = $(data).find("ConnectInfo").find("SplitCount").text();
				var statusFrame = document.getElementById('Connection_Status');
			//	var rowID =2;
			//	alert(splitCount);

				$(data).find("Connection").each(function() {
					var $Connection = $(this);
					var hostname = $Connection.find("HostName").text();
					var position = $Connection.find("Position").text();
					var ipaddress = $Connection.find("IPAddress").text();
					var client_type = $Connection.find("Type").text();
					var client_role = $Connection.find("Role").text();
					
				//alert(ipaddress+"|"+position+"|"+hostname);
				Creatlist(hostname, splitCount, position, ipaddress,client_type,client_role);
				//AddNewRow(statusFrame, rowID++, hostname, splitCount, position, ipaddress);
				});
			},
			error: function (e) {
			}
		});
		
		setCookie('ConfCtrl', 'on', 1);
		onoff_flag=1;
	}
	else
	{
		//alert("----off---");
		setCookie('ConfCtrl', 'off', 1);
		onoff_flag=0;

	}
	
	intervalID = window.setTimeout("ConfCrtl_onoff();",30000);
}
