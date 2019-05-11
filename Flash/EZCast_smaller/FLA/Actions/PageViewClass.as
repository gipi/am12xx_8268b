
import mx.utils.Delegate;


/**
 *	@author   Charles
 *	@version 1.0
 *    @date: 2013/03/18
 *	@description  
 *    PageViewClass Class
 *	支持 X*X 的界面, 例如1行*3列, 2行*3列, 3行*1列, ....
 *    为这些界面统计信息
 */

dynamic class Actions.PageViewClass extends Object{
	public var mCurPage:Number;
	public var mPageSize:Number;
	public var mPageTotal:Number;
	public var mTotal:Number;


	public function PageViewClass(total:Number, pageSize:Number)
	{

		trace("[PageViewClass] total = " + total + ",  pageSize = " + pageSize);
		mCurPage = 0;
		mTotal = total;
		mPageSize = pageSize;

		mPageTotal = calcPageTotal(mTotal, mPageSize);
	}

	public function addPage()
	{
		mCurPage++;

		if(mCurPage > mPageTotal-1){
			mCurPage = 0;
		}
	}

	public function subPage()
	{
		mCurPage--;

		if(mCurPage < 0){
			mCurPage = mPageTotal-1;
		}
	}


	public function calcPageStartIndex():Number
	{
		return mCurPage*mPageSize;
	}


	public function calcPageEndIndex():Number
	{
		return ((calcPageStartIndex()+mPageSize)<mTotal) ? (calcPageStartIndex()+mPageSize-1) : (mTotal-1);
	}

	
	public static function calcPageTotal(total:Number, pageSize:Number):Number
	{
		return Math.ceil(total/pageSize);
	}

	

}

