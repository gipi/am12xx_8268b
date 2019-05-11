var urlinfo,len,offset,newsidinfo,newsids,hrefpath,hrefname,clickflag,heard_txt;
var dir_number;
var clickflag=0;
//tmpname 
var tmpname="";
var Cdir="";
var idx;
function  delay_get_data()
{
/*
show();
hrefpath="airusb/sdcard/mp3/aaa"	
var index=hrefpath.indexOf("sdcard")
heard_txt=hrefpath.substr(index);

	$("#path_txt").text(heard_txt);
if(index)
$("#disk_title").text("SD Card");
else
$("#disk_title").text("USB Device");
*/
//var hrefname="";

//alert("---start-!!!----");
urlinfo=window.location.href; 
len=urlinfo.length; 
offset=urlinfo.indexOf("?");   
//alert(urlinfo);     
	if(offset==-1){ 
	  alert("Please ask this html from Airdisk");
	}
	else
	{
	    newsidinfo=urlinfo.substr(offset+1,len)
	    newsids=newsidinfo.split("=");
		
		var type=decodeURI(newsids[1]);
		$('#dir').value=type;
		$('#filename').value=type;
		hrefpath=type;  //path

var index=hrefpath.indexOf("sdcard")
heard_txt=hrefpath.substr(index);

//$("#path_txt").text(heard_txt);
/*
if(index)
 $("#disk_title").text("SD Card");
else
 $("#disk_title").text("USB Device");
*/	
		//alert("hrefpath=path="+hrefpath);	
	  //  
	}
//if cookie exist   ,change the value else create a cookie
//setCookie("mediaflag","",7);

	document.getElementById("get").style.display="none"; 
	document.getElementById("video").style.display="none"; 
	document.getElementById("img").style.display="none"; 
	document.getElementById("audio").style.display="none"; 


showdir(hrefpath); 

//document.getElementById("play").style.display="none"; \ 
//var id=0;
//get the hrefname when href was clicked  then send to cgi  and clear it 

}

function refr_list(value)
{

//alert("---refr_list!!!----");
$("ul li").remove();
var type=decodeURI(value);
$('#dir').value=type;
$('#filename').value=type;
hrefpath=type;  //path


	document.getElementById("get").style.display="none"; 
	document.getElementById("video").style.display="none"; 
	document.getElementById("img").style.display="none"; 
	document.getElementById("audio").style.display="none"; 


showdir(hrefpath); 

//document.getElementById("play").style.display="none"; \ 
//var id=0;
//get the hrefname when href was clicked  then send to cgi  and clear it 

}
var a=0;
function show()
{	
	
	
	addRow("0","mnq",a);
		a++;

	addRow("0","LORhdhgdDggggggggggggggg",a);
	a++;

	addRow("1","sajlkfjreareqwfdddddddddddddddddddrqwrqwreqwrqewreqwrqewrqwreqwrqwereqwwer.MP4",a);
		a++;

	addRow("2","LORhdhgdDgggggggggggggggggggggggg.mp3",a);
		a++;

	addRow("2","ezca  st.mp3",a);
		a++;

	addRow("0","mnq",a);
		a++;

	addRow("0","LORhdhgdDggggggggggggggg",a);
	a++;

	addRow("1","sajlkfjreareqwfdddddddddddddddddddrqwrqwreqwrqewreqwrqewrqwreqwrqwereqwwer.MP4",a);
		a++;

	addRow("2","LORhdhgdDgggggggggggggggggggEgggggggggggggggggggggggg.mp3",a);
	a++;

	addRow("4","fadsfhfgh ggggggggggggggggggggggggggggggggggggda1.doc",a);
		a++;

	addRow("5","gggggggggggggggggggggggggggggggggggggggggggggg1.nnw",a);
		a++;

	addRow("2","LORhdhgdDgggggggEgggggggggdfggg.mp3",a);

	//alert(getCookie("mediaflag"));
	a++;
}

function addRow(filetype,filename,idx){
//alert("---start--111---");	
var temp_img,hrefname2,filename2;
var filelist;
hrefname2=$("#dir").value;
	switch(filetype)
	{
		case "0"://dir
		temp_img ="<img src='img/dir.png' class='ui-li-icon'>";
		break;
		case "1"://video
		temp_img ="<img src='img/video.png' class='ui-li-icon'>";
		break;
		case "2"://audio
		temp_img ="<img src='img/audio.png' class='ui-li-icon'>";
		break;
		case "3"://pic
		temp_img ="<img src='img/img.png' class='ui-li-icon'>";
		break;
		case "4"://doc
		case "5":
		temp_img ="<img src='img/doc.png' class='ui-li-icon'>";
		break;
		default:
			temp_img="";
			break;
		
	}
//alert("---temp_img---"+temp_img);	
//alert(idx);
	//dir 
	if((filename!="")&&(filetype=="0"))
	{
	filelist = "<li id="+idx+"  class='ui-li-has-count ui-li-has-icon'><a href='javascript:;' onClick=\"getclick("+"\'"+filename+"\'"+")\" class='ui-btn ui-btn-icon-right ui-icon-carat-r'>"+temp_img+filename+"</a></li>";
	}
	//audio
	if((filename!="")&&(filetype=="2"))
	{
	filelist = "<li id="+idx+" class='ui-li-has-count ui-li-has-icon' ><a href='javascript:;'  onClick=\"audioplay("+"\'"+filename+"\'"+","+idx+")\" class='ui-btn ui-btn-icon-right ui-icon-carat-r'>"+temp_img+filename+"</a></li>";
	}
	// img file 
	if((filename!="")&&(filetype=="3"))
	{
	filelist = "<li id="+idx+" class='ui-li-has-count ui-li-has-icon'><a href='javascript:;' onClick=\"imgplay("+"\'"+filename+"\'"+","+idx+")\" class='ui-btn ui-btn-icon-right ui-icon-carat-r'>"+temp_img+filename+"</a></li>";
	}
	
	//video file
	if((filename!="")&&((filetype=="1")))
	{
	filelist = "<li id="+idx+" class='ui-li-has-count ui-li-has-icon'><a href='javascript:;' onClick=\"videoplay("+"\'"+filename+"\'"+","+idx+")\" class='ui-btn ui-btn-icon-right ui-icon-carat-r'>"+temp_img+filename+"</a></li>";
	}
	//doc file

	if((filename!="")&&((filetype=="4"))||((filetype=="5")))
	{
	filelist = "<li  id="+idx+" class='ui-li-has-count ui-li-has-icon'><a href='javascript:;' onClick=\"docplay("+"\'"+filename+"\'"+","+idx+")\" class='ui-btn ui-btn-icon-right ui-icon-carat-r'>"+temp_img+filename+"</a></li>";
	}
	//if(filename=="")
	//filelist=" ";
	
$("ul").append(filelist);
//$("#listfile").append(filelist);

}

function addRow2(filetype,filename){
//alert("---start--111---");	
var temp_img,hrefname2,filename2;
var filelist;
id++;
hrefname2=$("#dir").value;

	switch(filetype)
	{
		case "0"://dir
		temp_img ="<img src='img/dir.png'>";
		break;
		case "1"://video
		temp_img ="<img src='img/video.png' >";
		break;
		case "2"://audio
		temp_img ="<img src='img/audio.png' >";
		break;
		case "3"://pic
		temp_img ="<img src='img/img.png' >";
		break;
		case "4"://doc
		case "5":
		temp_img ="<img src='img/doc.png' >";
		break;
		default:
			temp_img="";
			break;
		
	}
//alert("---temp_img---"+temp_img);	

	//dir 
	if((filename!="")&&(filetype=="0"))
	{
	filelist = "<li><a id="+filename+"  href='javascript:;' onClick=\"getclick("+"\'"+filename+"\'"+")\" >"+temp_img+filename+"</a></li>";
	}
	//audio
	if((filename!="")&&(filetype=="2"))
	{
	filelist = "<li><a id="+filename+"  href='javascript:;' onClick=\"audioplay("+"\'"+filename+"\'"+")\" >"+temp_img+filename+"</a></li>";
	}
	// img file 
	if((filename!="")&&(filetype=="3"))
	{
	filelist = "<li><a id="+filename+"  href='javascript:;' onClick=\"imgplay("+"\'"+filename+"\'"+")\" >"+temp_img+filename+"</a></li>";
	}
	
	//video file
	if((filename!="")&&((filetype=="1")))
	{
	filelist = "<li><a id="+filename+"  href='javascript:;' onClick=\"videoplay("+"\'"+filename+"\'"+")\"  >"+temp_img+filename+"</a></li>";
	}
	//doc file

	if((filename!="")&&((filetype=="4"))||((filetype=="5")))
	{
	filelist = "<li><a id="+filename+"  href='javascript:;' onClick=\"docplay("+"\'"+filename+"\'"+")\" >"+temp_img+filename+"</a></li>";
	}
	//if(filename=="")
	//filelist=" ";
	
$("ul").append(filelist);
//$("#listfile").append(filelist);

}

function docplay(value,idx)
{
	clickflag=1;
	var i=document.getElementById(idx);
	var y=i.offsetTop;
	div_pos_cha("get",y);
	document.getElementById("get").style.display="block"; 
	
	document.getElementById("audio").style.display="none";
	//document.getElementById("get").style.display="none"; 
	document.getElementById("video").style.display="none"; 
	document.getElementById("img").style.display="none"; 
	//alert("s");
	//alert(filepath);
	var filename=hrefpath.concat("/",value);
  	var title=value;
/*
	var path="dddd";
	var filename=document.getElementById("filename").value;
	var index=filename.lastIndexOf("/");
	var title=filename.substr(index+1);
	*/
	
	document.title=title;
	filename=filename.substr(23);
//	alert("4: "+filename);

	var todown_ezc_val=filename;
//	alert("5:todown_ezc_val: "+todown_ezc_val);
	
	filename=encodeURI(filename);
	
	var hrefid=document.getElementById("todownload");
  	hrefid.innerHTML="<a href='javascript:'  target='_blank'  onClick=\"toDownload("+"\'"+todown_ezc_val+"\'"+")\" ><img src=\"img/get.png\" width=40px height=40px></a>";
	setTimeout("clear_flag()",500);

}
function videoplay(value,idx)
{

	  
	clickflag=1;

	//document.getElementById("tbbg").style.display="none";
var i=document.getElementById(idx);
var y=i.offsetTop;
	div_pos_cha("video",y);

	document.getElementById("video").style.display="block"; 
	document.getElementById("audio").style.display="none";
	document.getElementById("get").style.display="none"; 
	///document.getElementById("video").style.display="none"; 
	document.getElementById("img").style.display="none"; 

	var filename=hrefpath.concat("/",value);
	var title=value;
  
  
 	// var href="play.html?filename=";
 /* 	var filename=document.getElementById("filename").value;
 	var index=filename.lastIndexOf("/");
	var title=filename.substr(index+1);
	*/
	
	document.title=title;
	 // href=href.concat(filename);
  	//href=encodeURI(href);
	filename=filename.substr(23);
	var todown_ezc_val=filename;

	filename=encodeURI(filename);
 	var hrefid=document.getElementById("videoplay");
 	// filename="";
    hrefid.innerHTML="<a href="+filename+" target='_self' onclick='nochange()' ><img src=\"img/play.png\" width=40px height=40px></a>";
	var hrefid=document.getElementById("video_ezchannel");
  	hrefid.innerHTML="<a href='javascript:'  onClick=\"toEZchannel("+"\'"+todown_ezc_val+"\'"+")\" ><img src=\"img/ezchannel.png\" width=40px height=40px></a>";
	setTimeout("clear_flag()",500);
}

//audio  img play
function audioplay(value,idx)
{
	clickflag=1;
//alert("2:"+idx);
var i=document.getElementById(idx);
var x=i.offsetLeft;
var y=i.offsetTop;
//alert("left:"+x+"top:"+y);
	div_pos_cha("audio",y);
	//document.getElementById("audio").style.display="none";
	document.getElementById("get").style.display="none"; 
	document.getElementById("video").style.display="none"; 
	document.getElementById("img").style.display="none"; 
	document.getElementById("audio").style.display="block"; 
	
	//document.getElementById("play").style.display="none";
	//document.getElementById("upload").style.display="none"; 
//alert("1: "+hrefpath);
	var filename=hrefpath.concat("/",value);
//alert("2: "+value);
//alert("3: "+filename);

	var title=value;
/*
	var path="dddd";
	var filename=document.getElementById("filename").value;
	var index=filename.lastIndexOf("/");
	var title=filename.substr(index+1);
	*/
	
	document.title=title;
	filename=filename.substr(23);
//	alert("4: "+filename);

	var todown_ezc_val=filename;
//	alert("5:todown_ezc_val: "+todown_ezc_val);
	
	filename=encodeURI(filename);
 // 	alert("6: "+todown_ezc_val);
  
	var hrefid=document.getElementById("audioplay");
  	hrefid.innerHTML="<a href="+filename+"  target='_self' onclick='nochange()'><img src=\"img/play.png\" width=40px height=40px></a>";

	var hrefid=document.getElementById("audio_download");
  	hrefid.innerHTML="<a href='javascript:'  target='_blank'  onClick=\"toDownload("+"\'"+todown_ezc_val+"\'"+")\" ><img src=\"img/get.png\" width=40px height=40px></a>";

	var hrefid=document.getElementById("audio_ezchannel");
  	hrefid.innerHTML="<a href='javascript:'  onClick=\"toEZchannel("+"\'"+todown_ezc_val+"\'"+")\" ><img src=\"img/ezchannel.png\" width=40px height=40px></a>";


  	setTimeout("clear_flag()",500);
}
function imgplay(value,idx)
{
	clickflag=1;
	
var i=document.getElementById(idx);
var y=i.offsetTop;
	div_pos_cha("img",y);

	//document.getElementById("audio").style.display="none";
	document.getElementById("get").style.display="none"; 
	document.getElementById("video").style.display="none"; 
	document.getElementById("img").style.display="block"; 
	document.getElementById("audio").style.display="none"; 
	
	//document.getElementById("play").style.display="none";
	//document.getElementById("upload").style.display="none"; 
	var filename=hrefpath.concat("/",value);
	var title=value;
 	
/*	
	var filename=document.getElementById("filename").value;
	var index=filename.lastIndexOf("/");
	var title=filename.substr(index+1);
*/	
	document.title=title;
	filename=filename.substr(23);
	var todown_ezc_val=filename;

	filename=encodeURI(filename);
    
	var hrefid=document.getElementById("imgplay");
	//if(ind<0) //audio href
	//{
  	//setCookie("mediaflag","airdisk",7);
  	hrefid.innerHTML="<a href="+filename+"  target='_self'  onclick='nochange()'><img src=\"img/play.png\" width=40px height=40px></a>";
	//}
	//else //img
	//{
		//hrefid.innerHTML="<a href=\"javascript:\" onclick=\"toPlay()\"><img src=\"img/play.png\"></a>";
	//}

	var hrefid=document.getElementById("img_download");
  	hrefid.innerHTML="<a href='javascript:'  target='_blank'  onClick=\"toDownload("+"\'"+todown_ezc_val+"\'"+")\" ><img src=\"img/get.png\" width=40px height=40px></a>";
  	setTimeout("clear_flag()",500);
}
		
	//get the href click filename and turn to the dir  
	
function getclick(filename)
{
  //div_pos_cha(); 
	clickflag=1;    
	hrefname=hrefpath.concat("/",filename);
//	alert("this file name:"+hrefname);
//	refr_list(hrefname);  //

var href="disk.html?filename=";	
	href=href.concat(hrefname);
//	alert("disk name:"+href);
	
	href=encodeURI(href);
	location.replace(href);

  //clear last div 
 // 	document.getElementById("audio").style.display="none";
//	document.getElementById("get").style.display="none"; 
//	document.getElementById("video").style.display="none"; 
//	document.getElementById("img").style.display="none";
  
	//alert("click");
	
	
	//alert(idval);
	/*
	var myhref=document.getElementById("dir");
 	hrefname=myhref.name;
	//alert(hrefname);	
	Cdir=document.getElementById("dir").value;

	var href="disk.html?filename=";
	href=href.concat(Cdir);
	href=encodeURI(href);
	location.replace(href);
	//document.getElementById("dir").value=Cdir;
	//document.getElementById("filename").value=Cdir;
*/	
	setTimeout("clear_flag()",500);
	
}
function dirback()
{
	clickflag=1;
	
	
/*  
  	document.getElementById("audio").style.display="none";
	document.getElementById("get").style.display="none"; 
	document.getElementById("video").style.display="none"; 
	document.getElementById("img").style.display="none";
  
	//alert("click");

	
	//alert(idval);
	var myhref=document.getElementById("dir");
	hrefname=myhref.name;
	
  
	//alert(hrefname);	
	Cdir=document.getElementById("dir").value;
	*/
	Cdir=hrefpath;
	
	if((Cdir!="/mnt/user1/thttpd/html/airusb/usb")&&(Cdir!="/mnt/user1/thttpd/html/airusb/sdcard")&&(Cdir!="/mnt/user1/thttpd/html/airusb/memory")&&(Cdir!="/mnt/user1/thttpd/html/airusb/windows"))
	{

		var bitindex=Cdir.lastIndexOf("/");
					
		Cdir=Cdir.substr(0,bitindex);
	//	refr_list(Cdir);

		var href="disk.html?filename=";
		href=href.concat(Cdir);
		href=encodeURI(href);
		location.replace(href);
		
	}
	else
	{
		location.replace("airdisk.html");	

	}
	setTimeout("clear_flag()",500);
}

function showtablecolor(){
	var ybgcolor='#000000';
	var mbgcolor='#46A3FF'; 
	var t=0;
	var tablename=document.getElementById("dirtable");
	var li=tablename.getElementsByTagName("td");
  	for (var i=0;i<li.length;i++){
  	
	if((i%2)==1)
	  {

	  


	   		li[i].onmouseover=function(){
		  
		 
		  	this.style.backgroundImage="url(img/stoq.png)";

			this.style.backgroundRepeat="no-repeat";
			this.style.backgroundSize="100% 100%";

		
	 		 }
			
		  
  
  
  }
  
  li[i].onmouseout=function(){this.style.backgroundImage="url(img/stoq3.png)";
  			this.style.backgroundRepeat="no-repeat";
			this.style.backgroundSize="100% 100%";
			
		}

 }
 /*
 else
 	{
 //	li[i].style.left=10px;	
 	}*/
 }

function touchstart(){
 	document.getElementById("audio").style.display="none";
	document.getElementById("get").style.display="none"; 
	document.getElementById("video").style.display="none"; 
	document.getElementById("img").style.display="none"; 
				 
}
		   
function deltable()
{
  var table=document.getElementById("dirtable");
  var len=table.rows.length;
  for(var n=table.rows.length-1;n>0;n--)
  {
  	table.deleteRow(n);
 	
  
  }
  //table.deleteRow();
  
	id=0;
	
}

// send fullpath name to cgi and get dir list data to show 
function showdir(urlname)
{
	//alert("showdir=path2 : "+urlname);	


	//document.write("show");
//	addRow("0","/mnt");
	//addRow("4","file");
	//alert(urlname);
	var filetype= new Array(100);
	var filename=new Array(100);
	  
	$.get("cgi-bin/dir.cgi", {fullname:urlname}, function(dir_list)
		{	
		//	document.write(dir_list);
		//alert(dir_list);

		var  errindex=dir_list.indexOf("dir open Error");
		  if(errindex!=-1)
		  {
		  	//alert("please check your udisk in");	
		  }
			var i=0;
			len=dir_list.length;
			if(len>0)
			{

				//by cxf
				/*var re = new RegExp("*","g");
				var arr = dir_list.match(re);
				alert(arr.length);
				dir_number=arr.length;
				*/
				offset=dir_list.indexOf("#"); 
				if(offset<0)
				{
				 	//document.write (" read null form udisk directory  !");
				 	
				}
				else
				{ 
					
					dir=dir_list.substr(offset+1);
					//document.write(dir);
					while((dir.length)>0)
					{
						idex=dir.indexOf(",");
						filetype[i]=dir.slice((idex-1),idex);
						//document.write("type"+filetype[i]);
						idex1=dir.indexOf("*");
						filename[i]=dir.slice((idex+1),idex1);
						addRow(filetype[i],filename[i],i);
						//document.write("name"+filename[i]);
			 		 	if((idex1+1)==len)
							dir="";
						else
							dir=dir.substr((idex1+1));
						//document.write(dir);
						i++;
						
						//document.write(i);
				  }
				$('ul li:first').addClass("ui-first-child");
				$('ul li:last').addClass("ui-last-child");
				//	$("#listfile").find("li").removeClass("ui-li-has-icon");
				//	$("#listfile").find("li").removeClass("ui-li-has-icon");

					
			}
		}
		
		else
		{
			document.write("NULL");
			
		}
	
	//addRow("6","");			
	});
		
	
}
function toUpload()
{
	clickflag=1;
	var updir=document.getElementById("dir").value;
	window.location.href="usbfileupload.html?dir="+updir+"";
	setTimeout("clear_flag()",500);
	
}
function toDownload(value)
{
//alert("--toDownload--");
	clickflag=1;
	var ezcast="ezcast://ezcast.app/download?url=";
	var ip=location.host;
	var ipaddr="http://"+ip+"/";
	//var dwfilename=document.getElementById("filename").value;
	var dwfilename=value;
	
//	alert("1:"+dwfilename);
	
	var index=dwfilename.lastIndexOf("/");
	var title=dwfilename.substr(index+1);
	
	document.title=title;
	//dwfilename=dwfilename.substr(23);
	dwfilename=ipaddr+dwfilename;
//	alert("2:"+dwfilename);
	dwfilename=encodeURIComponent(dwfilename);
	dwfilename=ezcast+dwfilename;
//	alert("3:"+dwfilename);

	window.location.href=dwfilename;

	//window.location.href="cgi-bin/d ownload.cgi?name="+dwfilename+"";
	
	//var idex=dwfilename.lastIndexOf("/");
	//dwfilename=dwfilename.substr(idex+1);
	//alert(dwfilename);
//	document.getElementById("play").style.display="none"; 

	document.getElementById("audio").style.display="none";
	document.getElementById("get").style.display="none"; 
	document.getElementById("video").style.display="none"; 
	document.getElementById("img").style.display="none"; 
	setTimeout("clear_flag()",500);


}
function toEZchannel(value)
{
	clickflag=1;
	var ezcast_ch="ezchannel://ezcast.app/add?url=";
	var ip=location.host;
	//ip="http://192.168.203.1";
	var ipaddr="http://"+ip+"/";
	//var fullpath=document.getElementById("filename").value;
	var fullpath="/mnt/user1/thttpd/html/"+value;

	
	var index=fullpath.lastIndexOf("/");

	var title=fullpath.substr(index+1);  
	document.title=title;// filename
	 fullpath=fullpath.substr(23);  //airusb/filename
	fullpath=ipaddr+fullpath;  //replace /mnt/user1/thttpd/html/ with ip http://192.168.203.1/				   //fullpath=http://192.168.203.1/airusb/filename
	//var tmp_url=fullpath.substr(0,index-1);  //
	fullpath=encodeURIComponent(fullpath);  //encode URL
	title=encodeURIComponent(title);     //encode title
	//fullpath=tmp_url;  //encode URL
	//title=encodeURIComponent(title);     //encode title
	fullpath=ezcast_ch+fullpath+"&name="+title;
	fullpath=fullpath+"&source_from=airdisk&source_type=stream";
	//alert(fullpath);
	window.location.href=fullpath;
	//alert(fullpath);
	//window.location.href="cgi-bin/d ownload.cgi?name="+dwfilename+"";
	
	//var idex=dwfilename.lastIndexOf("/");
	//dwfilename=dwfilename.substr(idex+1);
	//alert(dwfilename);
//	document.getElementById("play").style.display="none"; 

	document.getElementById("img").style.display="none";
	document.getElementById("get").style.display="none"; 
	document.getElementById("video").style.display="none";
	document.getElementById("audio").style.display="none";
	setTimeout("clear_flag()",500);
}

function toPlay()  // no use
{
	clickflag=1;
	var playname=document.getElementById("filename").value;
	
	document.getElementById("play").style.display="none"; 	
	
	setTimeout("clear_flag()",500);
}

function div_pos_cha(divid,value_y)
{
	
	var oLeft=window.event.clientX+document.body.scrollLeft;
	var oTop=window.event.clientY+document.body.scrollTop-59;
	//alert(document.body.scrollTop);
	//alert(window.event.clientY);
	//alert("div_y:"+value_y);
	oTop=value_y-10;
	if(oTop<50)
		oTop=50;
	//oLeft=0+"px";
	oTop=oTop+"px"

	document.getElementById(divid).style.top=oTop;
	//document.getElementById(divid).style.left=oLeft;
	

		
}


function showtbbg(top,id)
{

	if(id==1)
	{
		var oLeft=50;
		var oTop=window.event.clientY+document.body.scrollTop;
		 oTop-=7;
		//oTop+=window.screenTop +document.documentElement.scrollTop - document.documentElement.clientTop;
		oTop=oTop+"px";
		document.getElementById("tbbg").style.top=oTop;
		
		
		document.getElementById("tbbg").style.display="block"; 
	}
	else if(id==0)
	{
		document.getElementById("tbbg").style.display="none"; 
	}

}


document.onclick=function()
{
	
	if(clickflag==0)
	{
		 
		document.getElementById("audio").style.display="none";
		document.getElementById("get").style.display="none"; 
		document.getElementById("video").style.display="none"; 
		document.getElementById("img").style.display="none"; 

		document.getElementById("upload").style.display="block"; 
		
		
	}
}
function nochange() // no use
{
	clickflag=1;
	
	document.getElementById("audio").style.display="none";
	document.getElementById("get").style.display="none"; 
	document.getElementById("video").style.display="none"; 
	document.getElementById("img").style.display="none"; 
	//window.location.reload();	
}
	

function clear_flag()
{
	clickflag=0;
	
}

function size()// no use
{
	//test=document.getElementById("block");
     //test.style.width=(document.body.clientWidth-45+"px");
	 test1=document.getElementById("content");
     test1.style.height=(document.body.clientHeight-40+"px");
    

	/*
	if((navigator.userAgent.match(/iPad/i))&&(window.orientation==90||window.orientation==-90))
		{
	 test2=document.getElementById("wrap");
	 test2.style.height=(document.body.clientHeight*0.68+"px");
		}

	if((navigator.userAgent.match(/iPad/i))&&(window.orientation==180||window.orientation==0))
		{
	 test2=document.getElementById("wrap");
	 test2.style.height=(document.body.clientHeight*0.9+"px");
	}
	*/
}


///cookies
function setCookie(name,value) 
{ 
var Days = 30; 
var exp = new Date(); 
exp.setTime(exp.getTime() + Days*24*60*60*1000); 
document.cookie = name + "="+ escape (value) + ";expires=" + exp.toGMTString(); 
} 
//??????cookies 
function getCookie(name) 
{ 
var arr,reg=new RegExp("(^| )"+name+"=([^;]*)(;|$)"); 
if(arr=document.cookie.match(reg)) return unescape(arr[2]); 
else return null; 
} 
//иж??3???es 
function delCookie(name) 
{ 
var exp = new Date(); 
exp.setTime(exp.getTime() - 1); 
var cval=getCookie(name); 
if(cval!=null) document.cookie= name + "="+cval+";expires="+exp.toGMTString(); 
} 
/*
$(document).ready(function()
 {
	//  show();
alert("---start-----");	
	 test1=document.getElementById("content");
     test1.style.height=(document.body.clientHeight-40+"px");
	 //var directory=document.getElementById("dir");
	// setCookie("directroy",directory); 
	 
	 /*
	if((navigator.userAgent.match(/iPad/i))&&(window.orientation==90||window.orientation==-90))
		{
		
	
	 test2=document.getElementById("wrap");
	 test2.style.height=(document.body.clientHeight*0.68+"px");
	}


	
if((navigator.userAgent.match(/iPad/i))&&(window.orientation==180||window.orientation==0))
		{
		
	 test2=document.getElementById("wrap");
	 test2.style.height=(document.body.clientHeight*0.9+"px");
	}
	*/
//});

/*
function set(s) {  
	
	size=s;
          document.body.style.cssText = document.body.style.cssText + '; -webkit-transform: scale(' + size + '); ';  
} 
*/


