用命令行控制，以下命令行中的email和pwd为用户名称和密码

1、下载封面图片，如果isthumbnail为0，下载大图，如果为1下载缩略图
webalbum_test.app email pwd download-album isthumbnail




2、更新Album的信息
目前默认更新album的title，summary，location为Actions_album_test
更新album的access为public,下面的whichalbum为指定哪个album，其值为0~（alubm数-1）

webalbum_test.app email pwd update-album whichalbum

3、删除album
webalbum_test.app email pwd delete-album whichalbum

4、创建album
目前默认创建的新album的title，summary，location为Actions_album_test，album的access为public,
webalbum_test.app email pwd create-album


5、下载某个album的图片
下面的whichalbum与更新album信息中的whichalbum定义相同，
isthumbnail为0表示下载大图，为1表示下载缩略图
whichphoto不指定，那么就下载全部，如果有指定则下载指定的那张图片。
其值的取值范围为0~(该album中的图片数-1)
webalbum_test.app email pwd download-photo whichalbum isthumbnail whichphoto

6、更新图片信息
下面的whichalbum与更新album信息中的whichalbum定义相同，
whichphoto需要指定，其值的取值范围为0~(该album中的图片数-1)
默认的更新photo的summary和description为Actions_photo_test
webalbum_test.app email pwd update-photo whichalbum whichphoto


7、上传图片
下面的whichalbum与更新album信息中的whichalbum定义相同，
photopath为指定的上传图片的全路径名称，将会将该图片加入到指定的album中
webalbum_test.app email pwd upload-photo whichalbum photopath

8、删除
下面的whichalbum与更新album信息中的whichalbum定义相同，
whichphoto需要指定，其值的取值范围为0~(该album中的图片数-1)
webalbum_test.app email pwd delete-photo whichalbum whichphoto

9、查看相册信息
webalbum_test.app email pwd showinfo-albums

9、查看某个相册信息
下面的which_album为指定的相册
webalbum_test.app email pwd showinfo-album which_album

10.获取好友列表
webalbum_test.app email pwd get_contact

