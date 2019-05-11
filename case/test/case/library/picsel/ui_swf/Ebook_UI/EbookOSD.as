import Actions.EBook;
import Actions.FileList;
import Actions.Locale;
import Actions.SystemInfo;

var checkStateEbook:Number = 0;
trace("ebookOSD");
function update_osd(){
	//trace("update osd");
	file_info.fielname.text = MainSWF.ebookfilename;
	file_info.nowpage.text = EBook.GetPageNo();
	file_info.totalpage.text = EBook.GetPageTotal();
	file_info.zoomsize.text = EBook.GetZoomSize();
}

checkStateEbook = setInterval(update_osd, 500);