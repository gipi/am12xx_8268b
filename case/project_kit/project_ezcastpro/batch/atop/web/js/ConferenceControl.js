var CGI_URL='/form/conferlink_cgi?'
var IMAGE_FOLDER='image/'
var intervalID;

window.onload = function() 
{
		StartConf();
}

function Disconnect(obj)
{
     var tbl=document.getElementById('Connection_Status');
     var ip = tbl.rows[obj].cells[3].innerHTML;

    $.ajax({
            type: 'GET',
			cache: false,
            url: CGI_URL+"type=control&control=control_disconnect&ip_address="+ip,
            dataType: 'text',
            success: function (data) {
            	//alert(data);
				if(data == 1)
				{
					ConfCrtl_onoff();
				}
			},
           error: function (e) {
           	//alert("DISCONNECT : return err");
               }
           });
}
///--no use
function Stop(obj)
{
     var tbl=document.getElementById('Connection_Status');
     var ip = tbl.rows[obj].cells[3].innerHTML;

    $.ajax({
            type: 'GET',
			cache: false,
            url: CGI_URL+"type=control&control=control_stop&ip_address="+ip,
            dataType: 'text',
            success: function (data) {
            	//alert(data);
				if(data == 1)
				{
					ConfCrtl_onoff();
				}
           },
           error: function (e) {
           //	alert("Stop : return err");
               }
           });
}

function Change(obj, splitCount, position)
{
     var tbl=document.getElementById('Connection_Status');
     var ip = tbl.rows[obj].cells[3].innerHTML;
    // alert("type=control&control=control_change&split_count="+splitCount+"&position="+position+"&ip_address="+ip);

    $.ajax({
            type: 'GET',
			cache: false,
            url: CGI_URL+"type=control&control=control_change&split_count="+splitCount+"&position="+position+"&ip_address="+ip,
            dataType: 'text',
            success: function (data) {
            	//alert(data);
				if(data == 1)
				{
					//alert("change :"+data);
					ConfCrtl_onoff();
				}
                },
           error: function (e) {
           	//alert("Change : return err");
               }
           });
}

function Start()
{
	$.ajax({
            type: 'GET',
            cache: false,
            async: false,
            url: CGI_URL+"type=conf&conf=conf_start",
            dataType: 'text',
            success: function (data) {
            	//alert(data);
				if(data == 1) {
					return 1;
	        }
	            return 1;
           },
	        error: function (e) {
	        	//alert("Start : return err");
               }
           });
		   
	return -1;
}

function End()
{
    $.ajax({
        type: 'get',
        cache: false,
        async: false,
        url: CGI_URL + "type=conf&conf=conf_end",
        dataType: 'text',
        success: function (data) {
        	//alert(data);
            //if (data == 1) {
             //   return 1;
            //}
            //return 1;
            document.getElementById('logoutbtn').click();
            return 1;
        },
        error: function (e) {
        	//alert("End : return err");
        	document.getElementById('logoutbtn').click();
        	return 1;
        }
    });
		   
	return -1;
}

function AddStatus(obj,splitCount,position,RowId)
{
  var ConnectionStatus="<table cellpadding=\"0\" cellspacing=\"0\" ><tbody><tr>";
  
  if(splitCount == 1 && position == 1)
     ConnectionStatus = ConnectionStatus + " <td><img border=0 src=\""+IMAGE_FOLDER+"full_disable.gif\"  /></td>";
  else
     ConnectionStatus = ConnectionStatus +" <td><a href=\"#\"><img border=0 src=\""+IMAGE_FOLDER+"full_enable.gif\" onclick =\"Change("+RowId+",1,1)\" /></a></td>";
  
  if(splitCount == 2 && position == 1)
        ConnectionStatus = ConnectionStatus + " <td><img border=0 src=\""+IMAGE_FOLDER+"L.bmp\"  /></td>";
  else
      ConnectionStatus = ConnectionStatus + " <td><a href=\"#\"><img border=0 src=\""+IMAGE_FOLDER+"L-1.bmp\"  onclick =\"Change("+RowId+",2,1)\" /></a></td>";
  
  if(splitCount == 2 && position == 2)
        ConnectionStatus = ConnectionStatus + " <td><img border=0 src=\""+IMAGE_FOLDER+"R.bmp\"   /></td>";
  else
      ConnectionStatus = ConnectionStatus + " <td><a href=\"#\"><img border=0 src=\""+IMAGE_FOLDER+"UPR.bmp\"   onclick =\"Change("+RowId+",2,2)\" /></a></td>";
	
  var ConnectionStatus = ConnectionStatus +"<td>&nbsp&nbsp</td><td><table cellpadding=0 cellspacing=0 height=20 ><tbody><tr>";
  if(splitCount == 4 && position == 1)
        ConnectionStatus = ConnectionStatus + "<td><img border=0 src=\""+IMAGE_FOLDER+"1_disable.gif\" width=\"14\" height=\"10\" />";
  else
      ConnectionStatus = ConnectionStatus + "<td><a href=\"#\"><img border=0 src=\""+IMAGE_FOLDER+"1_enable.gif\" onclick =\"Change("+RowId+",4,1)\"></a></td>";
	  
  if(splitCount == 4 && position == 2)
        ConnectionStatus = ConnectionStatus + "<td><img border=0 src=\""+IMAGE_FOLDER+"2_disable.gif\"   />";
  else
      ConnectionStatus = ConnectionStatus + "<td><a href=\"#\"><img border=0 src=\""+IMAGE_FOLDER+"2_enable.gif\" onclick =\"Change("+RowId+",4,2)\"></a></td>";

  ConnectionStatus = ConnectionStatus + "</tr><tr>";
  
  if(splitCount == 4 && position == 3)
        ConnectionStatus = ConnectionStatus + "<td><img border=0 src=\""+IMAGE_FOLDER+"3_disable.gif\"   /></td>";
  else
      ConnectionStatus = ConnectionStatus + "<td><a href=\"#\"><img border=0 src=\""+IMAGE_FOLDER+"3_enable.gif\"  onclick =\"Change("+RowId+",4,3)\"></a></td>";

	  
  if(splitCount == 4 && position == 4)
        ConnectionStatus = ConnectionStatus + "<td><img border=0 src=\""+IMAGE_FOLDER+"4_disable.gif\"  /></td>";
  else
      ConnectionStatus = ConnectionStatus + "<td><a href=\"#\"><img border=0 src=\""+IMAGE_FOLDER+"4_enable.gif\"  onclick =\"Change("+RowId+",4,4)\"></a></td>";
    
  ConnectionStatus = ConnectionStatus + "</tr></tbody></table></td>";
  
  ConnectionStatus = ConnectionStatus + "<td><a href=\"#\"><img border=0 src=\""+IMAGE_FOLDER+"stop_enable.gif\"  onclick = \"Disconnect("+RowId+")\"/></a></td>";
  ConnectionStatus = ConnectionStatus + "</tr></tbody></table>";
  
  obj.innerHTML = ConnectionStatus;
}

function AddNewRow(statusFrame, RowId, hn, splitCount, position, ip)
{
    var newTR = statusFrame.insertRow(statusFrame.rows.length);
	var HeadPicture = "<img src=\""+IMAGE_FOLDER+"UserIconb.bmp\"/ >"
	if(RowId%2==1)
	{
		newTR.style.backgroundColor="#626262";
		var HeadPicture = "<img src=\""+IMAGE_FOLDER+"UserIconw.bmp\"/ width=\"40\" height=\"37\"  >"
	}
	var head = newTR.insertCell(0);
	head.innerHTML = HeadPicture;
	head.width = 40;
	
    var newHostname = newTR.insertCell(1);
	newHostname.width = 190;
	newHostname.align = "center";
	var newstatus = newTR.insertCell(2);
	newstatus.width=150;
	var newipaddress = newTR.insertCell(3);
	newipaddress.align = "center";
    newHostname.innerHTML = hn;
    AddStatus(newstatus,splitCount,position,RowId);
    newipaddress.innerHTML = ip;
}

function ConfCrtl_onoff()
{
	
	window.clearTimeout(intervalID);

	var statusFrame = document.getElementById('Connection_Status');
	//AddNewRow(statusFrame, 1, "hostname", 1, 1, "192.168.100.10");
	for(var i=statusFrame.rows.length-1;i>=0;i--)
	{
		statusFrame.deleteRow(i);
	}
	if(1)
	{
	    $.ajax({
	        type: 'GET',
	        cache: false,
	        url: CGI_URL + 'type=query_tcp_connections',
	        dataType: 'xml',
	        success: function (data) {
	        	//alert(data);
	            var splitCount = $(data).find("ConnectInfo").find("SplitCount").text();
	            var statusFrame = document.getElementById('Connection_Status');
	            var rowID = 0;
	            $(data).find("Connection").each(function () {
	                var $Connection = $(this);
	                var hostname = $Connection.find("HostName").text();
	                var position = $Connection.find("Position").text();
	                var ipaddress = $Connection.find("IPAddress").text();
	                AddNewRow(statusFrame, rowID++, hostname, splitCount, position, ipaddress);
	            });

	        },
	        error: function (e) {
	        	//alert("query_tcp_connections : return err");
	        }
	    });

	    $.ajax({
	        type: 'GET',
	        cache: false,
	        async: false,
	        url: CGI_URL + "type=getconf",
	        dataType: 'text',
	        success: function (data) {
	        	//alert(data);
	            if (data == 1) {
	                DoEndConf();
	                return;
	            }
	        },
	        error: function (e) {
	        //	alert("getconf : return err");
	        }
	    });
	}
	intervalID = window.setTimeout("ConfCrtl_onoff();",15000);
}

function StartConf() 
{
    Start();
	DoStartConf();
}

function DoStartConf()
{
	ConfCrtl_onoff();
}

function EndConf()
{
    End();
	//DoEndConf();
}

function DoEndConf()
{
	ConfCrtl_onoff();
}
