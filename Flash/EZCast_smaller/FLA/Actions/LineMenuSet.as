

import mx.utils.Delegate;
import Actions.PageViewClass;


/**
 *	@author   Charles
 *	@version 1.0
 *    @date: 2013/03/18
 *	@description  
 *    LineMenuSet Class
 *	支持 X*X 的界面, 例如1行*3列, 2行*3列, 8行*1列, ....
 *    创建这些界面, 并为这些界面提供按键处理
 */

dynamic class Actions.LineMenuSet extends Object{

	//call back define
	static public var ATTACH_MC_INIT:Number = 0;
	static public var REMOVE_MC_RELEASE:Number = 1;
	static public var UP_REFRESH_MENU_PAGE_ARR:Number = 2;
	static public var DOWN_REFRESH_MENU_PAGE_ARR:Number = 3;
	static public var MC_PRESS_EVENT:Number = 4;
	static public var MC_RELEASE_EVENT:Number = 5;
	static public var MC_RELEASE_OUT_SIDE_EVENT:Number = 6;

	static public var MC_NAME:String = "LineMenu_mc";

	public var mPageValue:PageViewClass;
	
	var mButtonArr:Array;
	//{tag:, id:, name:, nameMc:, menuArr:, selectDo:, unSelectDo:, enterDo:, escDo:, callBack:, mcAtt:{_x:, _y:, }}
	
	var mKeyObj:Object;//按键处理对象
	
	public var mCurIndex:Number;
	var mMenuArr:Array;
	var mTotal:Number;
	var mLineTotal:Number;//当前页行的总数
	var mRankTotal:Number;//当前页列总数

	var mAttachMc:Object;

	/**
	@brief  LineMenuSet 构造函数
	@param[in] attachMc	: attach 上去的mc
	@param[in] menuArr	: 需要显示的数组, length 为 PAGE_SIZE
	@brief {tag:, valid:, id:, name:, nameMc:, selectDo:, unSelectDo:, enterDo:, escDo:, callBack:, mcAtt:{_x:, _y:, }}
	@param[in] index :  显示为选中的index
	@param[in] total	:  一共需要显示的总数
	@param[in] lineTotal	: 行数
	@param[in] rankTotal : 列数
	 */
	public function LineMenuSet(attachMc:MovieClip, menuArr:Array, index:Number, total:Number, lineTotal:Number, rankTotal:Number)
	{

		if(menuArr.length == 0 || attachMc == undefined){
			trace("error 1 [LineMenuSet]");
			return ;
		}

		//页的大小由 menuArr 的长度决定
		if(total == undefined){
			total = menuArr.length;
		}

		if(lineTotal == undefined){
			lineTotal = 1;
		}

		if(rankTotal == undefined){
			rankTotal = menuArr.length;
		}

		if(menuArr.length > rankTotal*lineTotal){
			trace("menuArr.length error!!! len  == " + menuArr.length);
		}

		trace("[LineMenuSet]total attachMc == " + attachMc);
		//init member
		mTotal = total;
		mLineTotal = lineTotal;
		mRankTotal = rankTotal;
		
		mAttachMc = attachMc;
		mMenuArr = menuArr;
		mButtonArr = [];
		mCurIndex = index;//first index show select
		mKeyObj = new Object();
		mKeyObj.onKeyDown = Delegate.create(this, dealLineMenuKey);


		mPageValue = new PageViewClass(mTotal, mMenuArr.length);
		
		attachMenuMovieClip();		
		mButtonArr[mCurIndex].selectDo(mButtonArr[mCurIndex]);
		Key.addListener(mKeyObj);

		trace("mButtonArr.length == "  + mButtonArr.length);
	}


	public function setMenuArr(menuArr:Array)
	{
		if(menuArr){
			trace("[setMenuArr]");
			mMenuArr = menuArr;
			
			if(menuArr.length > mRankTotal*mLineTotal){
				trace("menuArr.length error!!! len  == " + menuArr.length);
			}
			
		}
	}

	public function showCurrentIndex(en:Boolean)
	{
		if(en){
			mButtonArr[mCurIndex].selectDo(mButtonArr[mCurIndex]);
		}else{
			mButtonArr[mCurIndex].unSelectDo(mButtonArr[mCurIndex]);
		}
	}
	
	public function freshPage(index:Number)
	{
		if(index == undefined || index < 0){
			index = 0;
		}

		attachMenuMovieClip();

		mCurIndex = index;
		if(mCurIndex>mButtonArr.length-1){
			mCurIndex = 0;
		}else if(mCurIndex < 0){
			trace("[freshPage] mCurIndex == " + mCurIndex);			
		}

		mButtonArr[mCurIndex].selectDo(mButtonArr[mCurIndex]);
	}

	/**
	@brief  addPage 增加减少页数
	 */
	public function addPage(pageNum:Number)
	{
		var i:Number = 0;

		if(pageNum > 0){
			for(i=0; i<pageNum; ++i){
				mPageValue.addPage();
			}
		}else if(pageNum < 0){
			for(i=0; i<-pageNum; ++i){
				mPageValue.subPage();
			}
		}
	}

	public function removeMenuMovieClip()
	{
		var i:Number = 0;
		var mc:Object;
		var len:Number = mButtonArr.length;

		for(i=0; i<len; ++i){
			mc = mButtonArr.shift();
			mc.callBack(REMOVE_MC_RELEASE);
			mc.removeMovieClip();
		}
	}


	public function deleteLineMenuSet()
	{
		if(mButtonArr.length > 0){
			mButtonArr.splice(0);
		}

		Key.removeListener(mKeyObj);
		mKeyObj = undefined;
		mPageValue = undefined;
	}


	public function realeseListenerKey()
	{
		Key.removeListener(mKeyObj);
	}


	public function addListenerKey()
	{
		Key.addListener(mKeyObj);
	}




	private function mcOnPress()
	{
		this.selectDo(this);
		this.callBack(MC_PRESS_EVENT);
	}


	private function mcOnRelease()
	{
		this.unSelectDo(this);
		this.enterDo(this);
		this.callBack(MC_RELEASE_EVENT);
	}


	private function mcOnReleaseOutside()
	{
		this.unSelectDo(this);
		this.callBack(MC_RELEASE_OUT_SIDE_EVENT);
	}


	private function attachMenuMovieClip()
	{
		var mc:MovieClip;
		var i:Number = 0;
		var index:Number = 0;

		var attachMc:Object = mAttachMc;
		var menuArr:Array = mMenuArr;

		this.removeMenuMovieClip();
		
		//attach movie
		for(i=0; i<menuArr.length; ++i){
			if(menuArr[i].valid){
				mc = attachMc.attachMovie(menuArr[i].id, MC_NAME+i, attachMc.getNextHighestDepth(), menuArr[i].mcAtt);
				mc[menuArr[i].nameMc].text = menuArr[i].name;

				mc.index = index++;
				mc.selectDo = menuArr[i].selectDo;
				mc.unSelectDo = menuArr[i].unSelectDo;
				mc.enterDo = menuArr[i].enterDo;
				mc.escDo = menuArr[i].escDo;
				mc.callBack = menuArr[i].callBack;
				mc.tag = menuArr[i].tag;
				mc.menuArr = menuArr;

				mc.onPress = mcOnPress;
				mc.onRelease = mcOnRelease;
				mc.onReleaseOutside = mcOnReleaseOutside;
				
				mButtonArr.push(mc);
				mc.unSelectDo(mc);
				mc.callBack(ATTACH_MC_INIT);
			}
		}
	}


	private function refreshNewPage(cmd:Number)
	{
		trace("mButtonArr.length == " + mButtonArr.length);

		mButtonArr[0].callBack(cmd);
		attachMenuMovieClip();
	}



	private function dealLineMenuKey()
	{
		trace("mCurIndex = " + mCurIndex);

		switch(Key.getCode())
		{
			case Key.LEFT:
				if(mButtonArr.length > 1 && mRankTotal > 1){
				 	mButtonArr[mCurIndex].unSelectDo(mButtonArr[mCurIndex]);
					mCurIndex--;
					if(mCurIndex < 0){
						if(mMenuArr.length < mTotal){
							mPageValue.subPage();
							refreshNewPage(UP_REFRESH_MENU_PAGE_ARR);
						}
						mCurIndex = mButtonArr.length - 1;
					}
					mButtonArr[mCurIndex].selectDo(mButtonArr[mCurIndex]);
				}

				mButtonArr[mCurIndex].callBack(Key.LEFT);
			break;

			case Key.RIGHT:
				if(mButtonArr.length > 1 && mRankTotal > 1){
				 	mButtonArr[mCurIndex].unSelectDo(mButtonArr[mCurIndex]);
					mCurIndex++;
					
					if(mCurIndex > mButtonArr.length - 1){
						if(mMenuArr.length < mTotal){
							mPageValue.addPage();
							refreshNewPage(DOWN_REFRESH_MENU_PAGE_ARR);
						}
						mCurIndex = 0;
					}
					mButtonArr[mCurIndex].selectDo(mButtonArr[mCurIndex]);
				}

				mButtonArr[mCurIndex].callBack(Key.RIGHT);
			break;

			case Key.UP:
				if(mButtonArr.length > 1 && mTotal > mRankTotal){
					mButtonArr[mCurIndex].unSelectDo(mButtonArr[mCurIndex]);
					mCurIndex-=mRankTotal;
					if(mCurIndex < 0){
						if(mMenuArr.length < mTotal){
							mPageValue.subPage();
							refreshNewPage(UP_REFRESH_MENU_PAGE_ARR);
						}
						mCurIndex = mButtonArr.length - 1;
					}
					mButtonArr[mCurIndex].selectDo(mButtonArr[mCurIndex]);
				}
				mButtonArr[mCurIndex].callBack(Key.UP);
			break;
			
			case Key.DOWN:
				if(mButtonArr.length > 1 && mTotal > mRankTotal){
					mButtonArr[mCurIndex].unSelectDo(mButtonArr[mCurIndex]);
					mCurIndex+=mRankTotal;
					if(mCurIndex > mButtonArr.length - 1){
						if(mMenuArr.length < mTotal){
							mPageValue.addPage();
							refreshNewPage(DOWN_REFRESH_MENU_PAGE_ARR);
						}
						mCurIndex = 0;
					}
					mButtonArr[mCurIndex].selectDo(mButtonArr[mCurIndex]);
				}
				mButtonArr[mCurIndex].callBack(Key.DOWN);
			break;

			case Key.ENTER:
				mButtonArr[mCurIndex].enterDo(mButtonArr[mCurIndex]);
			break;

			case Key.ESCAPE:
				mButtonArr[mCurIndex].escDo(mButtonArr[mCurIndex]);
			break;
		}
	}
}


