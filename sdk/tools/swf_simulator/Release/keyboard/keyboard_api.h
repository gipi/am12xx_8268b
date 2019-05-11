typedef struct keyboard_rect{
	int width; //the width of keyborad
	int height; //the height of keyboard
}keyboard_rect;
//分配keyboard需要的资源
struct keyboard_rect *keyboard_open(void);
//释方keyboard使用的资源 
int keyboard_close(void);
//使能keyboard界面的输出
int keyboard_enable(void);
//关闭keyboard界面的输出
int keyboard_disable(void);
enum KEYBOARD_FORMAT{
	K_RGB565=1,
	K_YUV422=2,
	K_RGB888=3
};
//更新keyboard参数
/*
(x,y):keyboard相对坐标
*/
int keyboard_config_position(int x,int y);
/*
frame:输出buffer
stride:窗口宽度
height:窗口高度
*/
int keyboard_config_frame(char *frame,int stride,int height);
/*
format:图像格式
alpha:0：不显示，255:全色显示
osd:	1:use osd 0:don't usb osd
scale:	0~100, 50 means displaying 50%,100 means displaying 100% 
*/
int keyboard_config_display_mode(enum KEYBOARD_FORMAT format,int alpha,int osd,int scale);
void keyboard_updata(void);
//int keyboard_config(char *frame,int x,int y,int stride,int width,int height,enum KEYBOARD_FORMAT format,int alpha);
enum KEYBOARD_EVENT{
	K_LBUTTONDOWN=1,
	K_MOVE=2,
	K_LBUTTONUP=3
};
//处理鼠标动作，触发有效按键时返回相应按键ascii值，并标记keyboard状态
int keyboard_event_to_keyvalue(enum KEYBOARD_EVENT event,int x,int y);
int keyboard_is_drag(int x,int y);
int keyboard_config_position(int x,int y);

int keyboard_config_display_mode(enum KEYBOARD_FORMAT format,int alpha,int osd,int scale);

int keyboard_config_lang(int english);

void keyboard_Enable_Chinese(int flag);
int is_valid_key_for_freepy(int key);