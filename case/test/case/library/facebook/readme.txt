用命令行控制，以下命令行中的email和pwd为用户名称和密码

1、获取用户基本信息
facebook_test.app email pwd user_info

2、获取用户头像
facebook_test.app email pwd user_picture

3、获取用户所有albums的信息
facebook_test.app email pwd albums_info

4、下载某个album的封面图片
which_album指的是album index
facebook.app email pwd album_cover which_album

5、获取用户某个album中的photo信息
facebook_test.app email pwd albumphoto_info whichalbum

6、下载photo
start_photo为开始下载的photo index
end_photo为结束下载的photo index
另外，下载大图需要用download_photo_big，下载小图需要用download_photo_small
facebook_test.app email pwd download_photo_big which_album start_photo end_photo

7、获取好友列表
用户的friends可能会分为几个group，例如work、classmate等
利用此接口获取不到不属于任何group的friends成员
facebook_test.app email pwd friends_list

8、获取指定好友列表中的member成员
which_friendslist为指定的friendslist index
facebook_test.app email pwd friendslist_members which_friendslist

9、获取全部好友
facebook_test.app email pwd friends

10、获取某个好友的albums信息
which_friend为friend index
facebook_test.app email pwd friend_albums_info which_friend

11、下载好友某个album的封面图片
facebook_test.app email pwd friend_album_cover which_friend which_album

12、获取某个好友的某个album中的photo信息
which_friend为friend index
which_album指的是album index
facebook_test.app email pwd friend_albumphoto_info which_friend which_album

13、下载好友的photo
which_friend为friend index
which_album指的是album index
start_photo为开始下载的photo index
end_photo为结束下载的photo index
另外，下载大图需要用download_photo_big，下载小图需要用download_photo_small
facebook_test.app email pwd friend_download_photo_big which_friend which_album start_photo end_photo