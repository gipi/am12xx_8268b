import Actions.AudioDbEngine;

var ret:Number;
var nr:Number;
var i:Number;
var creatFlag:Number = 0;

var dir:String = "/mnt/udisk/";
var dbPath:String = "/mnt/udisk/my_test_audio.db";
ret = AudioDbEngine.Exist(dbPath);
if(ret == 0){
	trace("db already exist");
	// AudioDbEngine.Delete(dbPath);
}
else{
	AudioDbEngine.Create(dir,dbPath);	
	creatFlag=1;
}

this.onEnterFrame=function()
{
	if(creatFlag==1){
		ret = AudioDbEngine.getCreateStatus();
		if(ret == 100){
			trace("create done");
			delete this.onEnterFrame;
		}
		trace("process --> "+ret);
	}
}
