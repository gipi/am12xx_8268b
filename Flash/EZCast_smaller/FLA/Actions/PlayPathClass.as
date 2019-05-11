

import mx.utils.Delegate;


/**
 *	@author   Charles
 *	@version 1.0
 *    @date: 2013/03/18
 *	@description  
 *    LocalPlayPathClass Class
 */

dynamic class Actions.PlayPathClass extends Object{
	public static var REPEAT_OFF:Number = 0;
	public static var REPEAT_ONE:Number = 1;
	public static var REPEAT_ALL:Number = 2;

	public static var RANDOM_ON:Number = 1;
	public static var RANDOM_OFF:Number = 0;

	public static var REPLAY_ON:Number = 1;
	public static var REPLAY_OFF:Number = 0;


	var mTotal:Number;
	var mType:Number;
	var mRepeat:Number;
	var mRandom:Number;
	var mReplay:Number;
	public var mCurIndex:Number;
	
	
	var mGetFileObj:Object;
	var mGetFilePath:Function;
	var mGetFileType:Function;
	


	public function PlayPathClass(getFileObj:Object, getFilePath:Function, getFileType:Function,
		total:Number, type:Number, repeat:Number, random:Number, replay:Number)
	{

		mGetFileObj = getFileObj;
		mGetFilePath = getFilePath;
		mGetFileType = getFileType;

		mTotal = total;
		mType = type;
		mRepeat = repeat;
		mRandom = random;
		mReplay = replay;

		mCurIndex = 0;

		if(undefined == mRepeat){
			mRepeat = REPEAT_ALL;
		}

		if(undefined == mRandom){
			mRandom = RANDOM_ON;
		}

		if(undefined == mReplay){
			mReplay = REPLAY_ON;
		}
	
	}

	
	public function getPrevFile(curPath:String):String
	{
		var i:Number = 2000;
		var path = "";

		if(mTotal > 0){
			
			if(REPEAT_OFF == mRepeat){
				path = "";
			}else if(REPEAT_ONE == mRepeat){
				if(REPLAY_OFF ==mReplay){
					path = "";
				}else{
					path = curPath;
				}
			}else{
				if(RANDOM_OFF == mRandom){
					i = mTotal + mCurIndex + 1;
					do{
						mCurIndex--;
						i--;
						if(mCurIndex <0){
							if(REPLAY_OFF == mReplay){
								return "";
							}
							mCurIndex = mTotal -1;
						}
					}while(mType != mGetFileType(mGetFileObj, mCurIndex) && i > 0);

				}else{
					do{
						mCurIndex = random(mTotal);
						i--;
					}while(mType != mGetFileType(mGetFileObj, mCurIndex) && i > 0);
				}

				if(i > 0){
					path = mGetFilePath(mGetFileObj, mCurIndex);
				}
			}
		}

		trace("[getPrevFile] path == " + path);
		
		return path;
	}


	public function getNextFile(curPath):String
	{

		var i:Number = 2000;
		var path = "";
		
		
		if(mTotal > 0){
			
			if(REPEAT_OFF == mRepeat){
				path =  "";
			}else if(REPEAT_ONE == mRepeat){
				if(REPLAY_OFF == mReplay){
					path =  "";
				}else{
					path = curPath;
				}
			}else{
				if(RANDOM_OFF == mRandom){
					i = mTotal + mCurIndex + 1;
					do{
						mCurIndex++;
						if(mCurIndex > mTotal-1){
							if(REPLAY_OFF == mReplay){
								return "";
							}
							i--;
							mCurIndex = 0;
						}
					}while(mType != mGetFileType(mGetFileObj, mCurIndex) && i > 0);

				}else{
					do{
						idx = random(mTotal);
						i--;
					}while(mType != mGetFileType(mGetFileObj, mCurIndex) && i > 0);
				}

				if(i > 0){
					path = mGetFilePath(mGetFileObj, mCurIndex);
				}
			}

		}

		trace("[getNextFile] path == " + path);
		
		return path;
		
	}
	

}
