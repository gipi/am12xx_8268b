import Actions.SystemInfo;
import Actions.WifiEngine;
var have_show_ota_num:Boolean = false;
var sec_timer_id;
var download_end_flag=0;
var delay_upgrade=0;//for html can get the finished state
function prossbar_sec_fun()//539
{
	var ret = -1;
	var up_status = 0;
	var up_progress = 0;
	var up_state = 0;
	
	up_status =SystemInfo.getOtaUpgradeStatus();//get the OTA update status
	up_progress = up_status%256;			//get the processing(lower 8 bits)
	up_state = Math.floor(up_status/256);	//get the state(higher 8 bits)				

	if(download_end_flag == 0) 
	{
		if(up_state == SystemInfo.CARD_STATE_RUNUING) 
		{
			//dialog_con.prossbar.progress_bar_mc.gotoAndStop(up_progress/2);	
			num.text =	 Math.round(up_progress) + "%";
			prossbar_mc.bar._width = 643*Math.round(up_progress)/100;
			if(!have_show_ota_num)
			{
				trace("---------------------- Get and show ota num!!!");
				var ota_num:Number = SystemInfo.get_ota_num();
				if(ota_num > 0)
				{
					have_show_ota_num = true;
					//dialog_con.prossbar.ota_num_txt.text = "The number of upgrade this version: ";
					var tmpText:String = "Firmware Download";
					if(tmpText == "null")
						tmpText = "Firmware Download";
					warn_info.text = tmpText+"   #"+ota_num;
				}
			}
		} 
		else if(up_state == SystemInfo.CARD_STATE_FINISHED)
		{
			if(0==delay_upgrade)//for html can get the finished state
			{
				delay_upgrade=1;		
				return;
			}
			else
				download_end_flag = 1;
		}
	}
	else if(download_end_flag == 1) 
	{
		trace("---------------  Download successful  ---------------------");
		num.text = "100%"
		if(sec_timer_id!=-1)
		{
			clearInterval(sec_timer_id);
			sec_timer_id=-1;
		}
		MainSWF.main_net_status_stop();
		WifiEngine.Stop();
		WifiEngine.Close();
		WifiEngine.Wifidonglechange(WifiEngine.CLOSE_WIFI_PROCESS);
		ret = SystemInfo.ota_upgrade(2);
		download_end_flag = 0;
	}	

}
function Start_download()
{
	// Disable auto standby.
	MainSWF.sysNotBusyCountClean();
	MainSWF.AUTO_STANDBY_ENABLE = false;
	var tmpText:String = "Firmware Download";
	if(tmpText == "null")
		tmpText = "Firmware Download";
	warn_info.text = tmpText;

	if(sec_timer_id!=-1)
	{
		clearInterval(sec_timer_id);
		sec_timer_id=-1;
	}
	sec_timer_id = setInterval(prossbar_sec_fun, 1000);		
}
Start_download();
