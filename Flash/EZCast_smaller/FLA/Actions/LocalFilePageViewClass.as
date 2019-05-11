

import Actions.FileList;
import mx.utils.Delegate;
import LineMenuSet;
import TextRollClass;


dynamic class LocalFilePageViewClass extends Object{

	public static var FL_STAT_FIND_NOFILE:Number = 0;
	public static var FL_STAT_FIND_ONE:Number = 1;
	public static var FL_STAT_FIND_COMPLETE:Number = 2;
	
	
	public static var FILELIST_USE_DEAMON:Number = 1;

	public static var MAX_DIR_DEPTH:Number = 10;
	
	public static var DIR_TYPE:Number = FileList.FT_DIR;
	public static var MUSIC_FILE_TYPE:Number = FileList.FT_MUSIC;
	public static var VIDEO_FILE_TYPE:Number = FileList.FT_VIDEO;
	public static var IMAGE_FILE_TYPE:Number = FileList.FT_IMAGE;
	public static var TXT_FILE_TYPE:Number = FileList.FT_TXT;
	public static var PDF_FILE_TYPE:Number = FileList.FT_TXT + 1;
	public static var DOC_FILE_TYPE:Number = FileList.FT_TXT + 2;
	public static var XLS_FILE_TYPE:Number = FileList.FT_TXT + 3;
	public static var PPT_FILE_TYPE:Number = FileList.FT_TXT + 4;
	public static var PPS_FILE_TYPE:Number = FileList.FT_TXT + 5;
	public static var PREV_DIR_TYPE:Number = 200;
	public static var OTHER_FILE_TYPE:Number = 1000;

	public static var mFileTypeArr:Array = [
		{
			type:PDF_FILE_TYPE,
			file:["pdf"]
		},
		{
			type:DOC_FILE_TYPE,
			file:["doc", "docx"]
		},
		{
			type:XLS_FILE_TYPE,
			file:["xls", "xlsx"]
		},
		{
			type:PPT_FILE_TYPE,
			file:["ppt", "pptx", "pps", "ppsx"]
		},
		{
			type:TXT_FILE_TYPE,
			file:["txt"]
		}
	];



	public var mNameArr:Array;
	public var mTypeArr:Array;
	public var mUrlArr:Array;
	public var mSizeArr:Array;
	public var mFileIndex:Array;

	
	public var mMenuSet:LineMenuSet;
	public var mTextRoll:TextRollClass;

	public var mHandle:Number;
	public var mSearchPath:String;
	public var mCurrentPath:String;
	public var mFindType:String;
	public var mBrowse:Number;
	public var mFileTotal:Number;
	public var mSort:Number;
	public var mSearchDone:Number;
	
	public var mMovieClip:Number;
	
	public var mDirIndex:Array;
	public var mDirDepth:Number;

	var mFilePageCallBack:Function;

	public var mMenuLayOut:Array;
	var mRankTotal:Number;
	var mLineTotal:Number;
	var mShowTextLen:Number;
	var mShowTextTime:Number;


	public function LocalFilePageViewClass(attachMc:MovieClip, currentPath:String, findType:String, 
		browse:Number, sort:Number, callBack:Function, 
		menuLayout:Array, lineTotal:Number, rankTotal:Number,
		showTextLen:Number, showTextTime:Number)
	{
		var i:Number = 0;

		if(undefined == menuLayout){
			trace("[LocalFilePageViewClass] menuLayout error!");
		}

		mMenuLayOut = menuLayout;
		mRankTotal = rankTotal;
		mLineTotal = lineTotal;

		mShowTextLen = showTextLen;
		mShowTextTime = showTextTime;

		mMovieClip = attachMc;
		mSearchPath = mCurrentPath = currentPath;
		mFindType = findType;
		mBrowse = browse;
		mSort = sort;
		mSearchDone = 0;
		mTotal = 0;
		
		mFilePageCallBack = callBack;


		if(1 == FILELIST_USE_DEAMON){
			mHandle = FileList.setPathdeamon(mSearchPath, mFindType,  mBrowse, 1);
		}else{
			mHandle = FileList.setPathNew(mSearchPath, mFindType,  mBrowse, 0);
		}


		if(mHandle >= 0){
			mNameArr = [];
			mTypeArr = [];
			mUrlArr = [];
			mSizeArr = [];
			mFileIndex = [];

			mDirIndex = [];

			for(i=0; i<mMenuLayOut.length; ++i){
				mMenuLayOut[i]._menuCallBack = mMenuLayOut[i].callBack;
				mMenuLayOut[i].callBack = Delegate.create(this, localFilePageViewCallBack);
			}
			
			mMovieClip.onEnterFrame = Delegate.create(this, searchFileLoop);
			
		}
		
		trace("mHandle = " + mHandle);
	
	}


	public function localFilePageViewCallBack(cmd:Number, obj:Object)
	{
		trace("[localFilePageViewCallBack] cmd = " + cmd);
		var mc =  mMenuSet.mButtonArr[mMenuSet.mCurIndex];

		mc._menu._menuCallBack(cmd, obj);
		
		switch(cmd){
			case LineMenuSet.UP_REFRESH_MENU_PAGE_ARR:
			case LineMenuSet.DOWN_REFRESH_MENU_PAGE_ARR:
				refreshMenuSet(mMenuSet.mPageValue.calcPageStartIndex());
				break;
		}
	}

	public function releaseLocalFilePageViewClass()
	{
		trace("[releaseLocalFilePageViewClass] mHandle = " + mHandle);
		
		if(mHandle >= 0){

			if(mTextRoll){
				mTextRoll.deleteSelfFromRoll();
			}

			if(mMenuSet){
				mMenuSet.deleteLineMenuSet();
			}

			
			if(mSort){
				FileList.detachSortviewNew(mHandle);
			}

			if(1 == FILELIST_USE_DEAMON){
				FileList.stopDeamonTask();
			}
			
			FileList.releasePathNew(mHandle);
			mHandle = -1;
			
			mNameArr.splice(0);
			mTypeArr.splice(0);
			mUrlArr.splice(0);
			mSizeArr.splice(0);
			mFileIndex.splice(0);
			
			mDirIndex.splice(0);
		}
	}


	
	public function searchFileLoop()
	{  
		var stat:Number;
		var total:Number;

		if(mHandle >=0 && !mSearchDone){
			stat = FileList.getScanTaskStatNew(mHandle); //checking the status of  file-scan daemon

			total = FileList.getTotalNew(mHandle); //get the total number of files, maybe during the scanning is processing
			mTotal = total;
			
			if(stat==FileList.FL_STAT_FIND_NOFILE){
				mTotal = 0;
				mSearchDone = 1;
				releaseLocalFilePageViewClass();
				delete mMovieClip.onEnterFrame;
				mFilePageCallBack(FL_STAT_FIND_NOFILE);
				
			}else if(stat==FileList.FL_STAT_FIND_COMPLETE){  // the scanning done

				mSearchDone = 1; //only check stat once after the SWF is loaded

				delete mMovieClip.onEnterFrame;

				if(total == 0){
					releaseLocalFilePageViewClass();
					mFilePageCallBack(FL_STAT_FIND_NOFILE);
				}else{
					if(mSort){
						FileList.attachSortviewNew(mHandle); // sort the filelist
					}
					
					mFilePageCallBack(FL_STAT_FIND_COMPLETE);

					refreshMenuSet(0);
					mMenuSet = LineMenuSet(mMovieClip, mMenuLayOut, 0, mLineTotal, mRankTotal);
					if(mShowTextTime > 0){
						mTextRoll = TextRollClass(Delegate.create(this, getTextRollFunc),
							Delegate.create(this, getStringRollFunc), mShowTextLen);
					}
				}

				trace("[searchFileLoop] Done");
			}else if(stat==FileList.FL_STAT_FIND_FIRST_FILE){
				mFilePageCallBack(FL_STAT_FIND_ONE);
			}

		}else if(mHandle < 0){
			trace("[searchFile] handle < 0" );
			mTotal = 0;
			mSearchDone = 1;
			releaseLocalFilePageViewClass();
			delete mMovieClip.onEnterFrame;
			mFilePageCallBack(FL_STAT_FIND_NOFILE);
		}else{
			delete mMovieClip.onEnterFrame;
		}
	}


	public function refreshMenuSet(startIndex:Number)
	{
		refreshPageData(startIndex);
	}


	public function refreshPageData(startIndex:Number):Boolean
	{

		var i:Number = 0;
		
		trace("[refreshPage] startIndex = " + startIndex);
		for(i = 0; i<mMenuLayOut.length; i++){
			
			if((i + startIndex) < mTotal){
				mMenuLayOut[i].valid = 1;
				
				mMenuLayOut[i].name = mMenuLayOut[i]._name = FileList.getLongnameNew(mHandle, i + startIndex);	
				mMenuLayOut[i]._type = FileList.getFileTypeNew(mHandle, i + startIndex);
				mMenuLayOut[i]._url = FileList.getPathNew(mHandle, i + startIndex);
				mMenuLayOut[i]._fileIndex = i + startIndex;

				
				if(mMenuLayOut[i]._type == FileList.FT_FILE){
					mMenuLayOut[i]._type = __getFileType(mMenuLayOut[i]._name);
				}
			}else{
				mMenuLayOut[i].valid = 0;
			}
		}
		
		return true;
		
	}


	public function getCurrentTIndex():Number
	{
		return mMenuSet.mButtonArr[mMenuSet.mCurIndex]._menu._fileIndex;
	}

	public function refreshHandle()
	{
		var oldTotal:Number = mTotal;
		
		FileList.refreshFileListNew(mHandle);
		mTotal = FileList.getTotalNew(mHandle);


		trace("[reFreshHandle]mTotal == " + mTotal);
		if(mTotal != oldTotal){
			if(mSort){
				FileList.detachSortviewNew(mHandle);
				FileList.attachSortviewNew(mHandle); // sort the filelist
			}
			
			var oldPage = mMenuSet.mPageValue.mCurPage;

			trace("[reFreshHandle]oldPage == " + oldPage + ",  pageTotal = " + mMenuSet.mPageValue.mPageTotal);
			
			mMenuSet.setTotal(mTotal);

			if(oldPage < PageViewClass.calcPageTotal(mTotal, mMenuLayOut.length)){
				mMenuSet.mPageValue.mCurPage = oldPage;
			}
			
			refreshMenuSet(mMenuSet.mPageValue.calcPageStartIndex());
			mMenuSet.freshPage(0);
			
			trace("[reFreshHandle]newPage == " + mMenuSet.mPageValue.mCurPage);
		}
	}


	public function enterDir():Number
	{  
		var  mc = mMenuSet.mButtonArr[mMenuSet.mCurIndex];
		if(mDirDepth >= MAX_DIR_DEPTH){ 
			return -1;
		}

		
		mDirIndex[mDirDepth] = mc._menu._fileIndex;
		
		FileList.enterDirNew(mHandle, mDirIndex[mDirDepth]); // this filelistHandle is on the current depth
		if(mSort){
			FileList.detachSortviewNew(mHandle);
			FileList.attachSortviewNew(mHandle);
		}
		
		mTotal = FileList.getTotalNew(mHandle);

		mDirDepth++;
		mMenuSet.setTotal(mTotal);
		refreshMenuSet(0);
		mMenuSet.freshPage(0);
		
		return 0;
	}


	public function exitDir():Number
	{

		var index:Number=-1;
		
		if(mDirDepth == 0){		//the root dir
			return -1;
		}else{
			mDirDepth--;
			index = mDirIndex[mDirDepth]; //get the true index(page+index) of upper level 
			
			FileList.exitDirNew(mHandle, index);//what is the meaning of this index?
			if(mSort){
				FileList.detachSortviewNew(mHandle);
				FileList.attachSortviewNew(mHandle);
			}
			
			mTotal = FileList.getTotalNew(mHandle);

			var curIndex = index%mMenuLayOut.length;
			var curPage = PageViewClass.calcCurPage(index, mMenuLayOut.length);

			mMenuSet.setTotal(mTotal);
			mMenuSet.mPageValue.mCurPage = curPage;

			trace("[exitDir]curPage == " + curPage);
			
			refreshMenuSet(mMenuSet.mPageValue.calcPageStartIndex());
			mMenuSet.freshPage(curIndex);

			
			return index;
		}
	}



	//获取所有文件的type
	public function getFileType(idx:Number):Number
	{
		var type:Number = 0;
		
		type = FileList.getFileTypeNew(mHandle, idx);
		if(type == FileList.FT_FILE){
			type = __getFileType(FileList.getLongnameNew(mHandle, idx));
		}

		return type;

	}


	private function __getFileType(name:String):Number
	{
		var type:Number = FileList.FT_FILE;
		var lowName:String; 
		var fm:String;
		var i:Number = 0;
		var j:Number = 0;

		lowName = name.toLowerCase();
		fm = lowName.slice(lowName.lastIndexOf(".")+1, lowName.length);

		for(; i < mFileTypeArr.length; i++){
			for(j=0; j < mFileTypeArr[i].file.length; j++){
				if(fm == mFileTypeArr[i].file[j]){
					type = mFileTypeArr[i].type;
					break;
				}
			}

			if(type != FileList.FT_FILE){
				break;
			}
		}

		return type;

	}
	
	private function getTextRollFunc():Object
	{	
		var mc = mMenuSet.mButtonArr[mMenuSet.mCurIndex];
		return mc[mc._menu.nameMc];
	}

	private function getStringRollFunc():String
	{
		var mc = mMenuSet.mButtonArr[mMenuSet.mCurIndex];
		return mc._menu._fileName;
	}

	

}


