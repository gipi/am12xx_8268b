//#include <assert.h>
#include <stdio.h>
#include <string.h>
//#include <unistd.h>
#include <libusb.h> 
//#include <alsa/asoundlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <assert.h>

/* 
 * AOA 2.0 protocol
 * ref. https://source.android.com/accessories/aoa2.html 
 */
// LIBUSB_ENDPOINT_IN == DEVICE_TO_HOST
// LIBUSB_ENDPOINT_OUT == HOST_TO_DEVICE
#define AOA_CTRL_OUT (LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT)
#define AOA_CTRL_IN (LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN)

#define AOA_REQ_PROTOCOL				(51)
#define AOA_REQ_SETPROTO				(52)
#  define AOA_PROTO_MANUFACTURE_INDEX			(0)
#  define AOA_PROTO_MODEL_INDEX 			(1)
#  define AOA_PROTO_DESCRIPTION_INDEX			(2)
#  define AOA_PROTO_VERSION_INDEX			(3)
#  define AOA_PROTO_URI_INDEX				(4)
#  define AOA_PROTO_SERIAL_INDEX			(5)
#define AOA_REQ_ACCESSORY				(53)
#define AOA_REQ_REGISTER_HID				(54)
#define AOA_REQ_UNREGISTER_HID				(55)
#define AOA_REQ_SET_HID_REPORT				(56)
#define AOA_SEND_HID_EVENT				(57)
#define AOA_REQ_AUDIO					(58)
#define VID_GOOGLE					(0x18d1)
#define AOA_PID_BASE					(0x2d00) /* accessory */
#define AOA_PID_WITH_ADB				(0x2d01) /* accessory + adb */
#define AOA_PID_AUDIO_ONLY				(0x2d02) /* audio */
#define AOA_PID_AUDIO_WITH_ADB				(0x2d03) /* audio + adb */
#define AOA_PID_WITH_AUDIO				(0x2d04) /* accessory + audio */
#define AOA_PIO_WITH_AUDIO_ADB				(0x2d05) /* accessory + audio + adb */

/*
 * from adk2012/board/library/usbh.h
 * usb accssory filter in ADK2012.apk
 * see source ./res/xml/usb_accessory_filter.xml
 *   Manufacture "Google, Inc."
 *   Model       "DemoKit"
 *   Version     "2.0"
 */
#define ADK2012_MANUFACTURE_STRING			("Actions, Inc.")//("Google, Inc.")
#define ADK2012_MODEL_STRING				("UsbAudio")//("DemoKit")
#define ADK2012_DESCRIPTION_STRING			("Usb Audio Board")//("DemoKit Arduino Board")
#define ADK2012_VERSION_STRING				("2.0")//("2.0")
#define ADK2012_URI_STRING				("http://www.iezvu.com")//("http://www.android.com")
#define ADK2012_SERIAL_STRING				("12345678-001") //("0000000012345678")

#define HUB_DEV			0x09

//android app needs to match this
#define BT_ADK_UUID	0x1d, 0xd3, 0x50, 0x50, 0xa4, 0x37, 0x11, 0xe1, 0xb3, 0xdd, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66

#define SETTINGS_MAGIX					(0xAF)
typedef struct AdkSettings {
		uint8_t magix;
		uint8_t ver;

// v1 settings:
		uint8_t R, G, B, brightness, volume, alramHour, alramMin, alramOn;
		char btName[249];
		char btPIN[17];

		uint16_t  alramSnooze;
		char		alramTune[256];

		uint8_t speed, displayMode;
// later settings
} AdkSettings;

typedef struct libusb_device_descriptor libusb_device_descriptor;
typedef struct libusb_config_descriptor libusb_config_descriptor;
typedef struct libusb_interface libusb_interface;
typedef struct libusb_interface_descriptor libusb_interface_descriptor;
/*
 * ADK2012 oem/user protocol definition
 * from adk2012/board/MakefileBasedBuild/app/main.c
 * maximum packet size is 260 bytes, 256b payload + header
 *	command header 4 bytes
 *		u8 command opcode
 *		u8 sequence
 *		u16 size
 *
 *
 *	data formats:
 *		timespec = (year, month, day, hour, min, sec) (u16,u8,u8,u8,u8,u8)
 */
// command opcode defined
#define CMD_MASK_REPLY				(0x80)
#define BT_CMD_GET_PROTO_VERSION		(1)
#define BT_CMD_GET_SENSORS			(2)
#define BT_CMD_FILE_LIST			(3)
#define BT_CMD_FILE_DELETE			(4)
#define BT_CMD_FILE_OPEN			(5)
#define BT_CMD_FILE_WRITE			(6)
#define BT_CMD_FILE_CLOSE			(7)
#define BT_CMD_GET_UNIQ_ID			(8)
#define BT_CMD_BT_NAME				(9)
#define BT_CMD_BT_PIN				(10)
#define BT_CMD_TIME				(11)
#define BT_CMD_SETTINGS				(12)
#define BT_CMD_ALARM_FILE			(13)
#define BT_CMD_GET_LICENSE			(14)
#define BT_CMD_DISPLAY_MODE			(15)
#define BT_CMD_LOCK				(16)

#define BT_PROTO_VERSION_1			(1)

#define BT_PROTO_VERSION_CURRENT		BT_PROTO_VERSION_1

static libusb_context *context = NULL;
static libusb_device **list = NULL;
static libusb_device *found = NULL;
static libusb_device_descriptor desc_dev = {0};
static int isCloseAudio = 0;

/*
 * Audio only support 2 channel, 16-bit PCM audio format 
 * to usage standard USB audio class
 *
 */

int getProtocol(libusb_device_handle *dev, uint16_t* protocol) {
	return libusb_control_transfer(dev,
		AOA_CTRL_IN | LIBUSB_RECIPIENT_DEVICE, // bmRequestType
		AOA_REQ_PROTOCOL, // bRequest
		0, // value
		0, // index
		(uint8_t*) protocol, // data buffer
		2,    // 2 byte
		500); // timeout 500ms
}

int setProto(libusb_device_handle *dev, int idx,const char* str)
{
	return libusb_control_transfer(dev,
			AOA_CTRL_OUT | LIBUSB_RECIPIENT_DEVICE,
			AOA_REQ_SETPROTO,
			0,
			idx,
			(unsigned char*)str,
			strlen(str) + 1,
			500); // timeout
}

int switchToAccessoryMode(libusb_device_handle *dev)
{
	return libusb_control_transfer(dev,
			AOA_CTRL_OUT | LIBUSB_RECIPIENT_DEVICE,
			AOA_REQ_ACCESSORY,
			0,
			0,
			NULL,
			0,
			500);
}

int setAudioMode(libusb_device_handle *dev, bool mode)
{
	int value = mode ? 1 : 0;
	return libusb_control_transfer(dev,
			AOA_CTRL_OUT | LIBUSB_RECIPIENT_DEVICE,
			AOA_REQ_AUDIO,
			value,
			0,
			NULL,
			0,
			500);
}

const char* formatLibUsbError(int errcode);
bool isInteresting(uint16_t vid, uint16_t pid);
bool isAccessoryDevice(libusb_device_descriptor  *desc);
void prt_dev_desc(libusb_device_descriptor *desc);
void prt_dev_conf(libusb_config_descriptor *desc);
void prt_dev_if(const libusb_interface *If);



static libusb_device*  findAccessoryDevice(libusb_device **list, size_t sz)
{
	int rc = 0;
	size_t idx = 0;
	libusb_device* dev = NULL;

	for (idx = 0; idx < sz; ++idx) {
		libusb_device *device = list[idx];
		libusb_device_descriptor desc = {0};
		rc = libusb_get_device_descriptor(device, &desc);
		assert(rc == 0);
		if (isAccessoryDevice(&desc)) {
			dev = device;
			break;
		}
	}
	return dev;
}

static int enableAccessoryMode(libusb_device *dev)
{
	int ret = 0;
	int rc = 0;
	libusb_device_descriptor desc = {0};
	rc = libusb_get_device_descriptor(dev, &desc);
	assert(rc == 0);
	printf("found a device, %04x:%04x\n", desc.idVendor, desc.idProduct);

	libusb_device_handle* handle = NULL;

	int err = 0;
	err = libusb_open(dev, &handle);
	if (err < 0) {
		ret = -1;
		printf("open device failed, errcode:%d, description:%s\n",
				err, formatLibUsbError(err));
		goto exit;
	}

	uint16_t protocol;
	err = getProtocol(handle, &protocol);
	if (err < 0) {
		ret = -2;
		printf("it is not android-powerd device, errcode:%d, description:%s\n",
				err, formatLibUsbError(err));
		goto error;
	}

#if 0
	setProto(handle, AOA_PROTO_MANUFACTURE_INDEX, "KunYi Chen");
	setProto(handle, AOA_PROTO_MODEL_INDEX, "PC Host");
	setProto(handle, AOA_PROTO_DESCRIPTION_INDEX, "PC Host to emulation an android accessory");
	setProto(handle, AOA_PROTO_VERSION_INDEX, "0.1");
	setProto(handle, AOA_PROTO_URI_INDEX, "kunyichen.wordpress.com");
	setProto(handle, AOA_PROTO_SERIAL_INDEX, "12345678-001");
#else
/*
	setProto(handle, AOA_PROTO_MANUFACTURE_INDEX, ADK2012_MANUFACTURE_STRING);
	setProto(handle, AOA_PROTO_MODEL_INDEX, ADK2012_MODEL_STRING);
	setProto(handle, AOA_PROTO_DESCRIPTION_INDEX, ADK2012_DESCRIPTION_STRING);
	setProto(handle, AOA_PROTO_VERSION_INDEX, ADK2012_VERSION_STRING);
	setProto(handle, AOA_PROTO_URI_INDEX, ADK2012_URI_STRING);
	setProto(handle, AOA_PROTO_SERIAL_INDEX, ADK2012_SERIAL_STRING);
	*/
#endif

	printf("[%s-%d] protocol: %d, isCloseAudio: %d\n", __func__, __LINE__, protocol, isCloseAudio);
	if (protocol == 2) {
		if(isCloseAudio == 0)
			setAudioMode(handle, true);
		else
			setAudioMode(handle, false);
	}
	switchToAccessoryMode(handle);

error:
	if (handle)
		libusb_close(handle);
exit:
	return ret;
}
static libusb_device*  findInterseting(libusb_device **list, size_t sz)
{
	int rc = 0;
	libusb_device* dev = NULL;
	int ret = -1;
	int i = 0, j = 0;
	size_t idx = 0;
	struct libusb_config_descriptor *config = NULL;
	const struct libusb_interface_descriptor *altsetting = NULL;
	const struct libusb_interface *interface = NULL;

	for (idx = 0; idx < sz; ++idx) {
		libusb_device *device = list[idx];
		libusb_device_descriptor desc = {0};
		rc = libusb_get_device_descriptor(device, &desc);
		assert(rc == 0);
		printf("[%s-%d] desc.idVendor: 0x%04x, desc.idProduct: 0x%04x, desc.bDeviceClass: 0x%04x\n", __func__, __LINE__, desc.idVendor, desc.idProduct, desc.bDeviceClass);
		if(desc.idVendor == 0x18d1 && (desc.idProduct >= 0x2D00 && desc.idProduct <= 0x2D05))		// is aoa mode
			continue;

		if (desc.bDeviceClass != HUB_DEV)
		{
			ret = libusb_get_active_config_descriptor(device, &config);
			if(ret != 0 || config == NULL){
				printf("Get config discriptor fail!\n");
				continue;
			}
			int desNum = 0;
			for(i=0; i<config->bNumInterfaces; i++){
				interface = &config->interface[i];
				for(j=0; j<interface->num_altsetting; j++){
					altsetting = &interface->altsetting[j];
					desNum++;
					printf("bInterfaceClass: 0x%02x, bInterfaceSubClass: 0x%02x, bInterfaceProtocol: 0x%02x\n", altsetting->bInterfaceClass, altsetting->bInterfaceSubClass, altsetting->bInterfaceProtocol);
				}
			}
			if(config != NULL)
				libusb_free_config_descriptor(config);
			config = NULL;
			printf("2018.03.22[%s-%d]\n", __func__, __LINE__);
			enableAccessoryMode(device);
			//if(desNum > 1)
			{
				dev = device;
				//break;
			}
		}
	}
	return dev;
}
int main(int argc, char **argv) {
	int rc = 0;
	long tid = 1;
	int ret = 0;
	int i = 0;
	size_t count = 0;
	printf("[%s-%d] ------------ \n", __func__, __LINE__);

	if(argc > 1 && strcmp(argv[1], "noaudio") == 0)
	{
		isCloseAudio = 1;
	}
	else
	{
		isCloseAudio = 0;
	}

	rc = libusb_init(&context);
	printf("[%s-%d] rc: %d\n", __func__, __LINE__, rc);
	assert(rc == 0);

	count = libusb_get_device_list(context, &list);
	printf("[%s-%d] count: %d\n", __func__, __LINE__, count);
	assert(count > 0);
	found = findInterseting(list, count);
//	if(found) enableAccessoryMode(found);
	libusb_free_device_list(list,1);
	if (!found) {
		printf("Failed!! not found android device\n");
		ret = 1;
		goto exit1;
	}

	sleep(2); // for device re-attached again
	//
	// enumeration USB device again, to get droid accessory mode device
	//
	count = libusb_get_device_list(context, &list);
	assert(count > 0);
	found = findAccessoryDevice(list, count);

	if (!found) {
		printf("Faild!! switch to accessory mode\n");
		ret = 2;
		goto exit;
	}

	//
	//  read device configuration
	//
	libusb_get_device_descriptor(found, &desc_dev);
	prt_dev_desc(&desc_dev);
	libusb_config_descriptor *config_desc;
	for(i = 0; i < desc_dev.bNumConfigurations; i++) {
		libusb_get_config_descriptor(found, i, &config_desc);
		prt_dev_conf(config_desc);
		libusb_free_config_descriptor(config_desc);
	}

exit:
	libusb_free_device_list(list,1);
exit1:
	libusb_exit(context);
	return ret;
}

bool isAccessoryDevice(libusb_device_descriptor* desc)
{
	return ((desc->idVendor == VID_GOOGLE) && 
		(desc->idProduct >= 0x2d00) &&
		(desc->idProduct <= 0x2d05)) ? true : false;
}

void prt_dev_desc(libusb_device_descriptor *desc)
{
	printf("--------------------------------------------------------------------------------\n");
	printf("dump device description\n");
	printf("Accessory(%04x:%04x)\n", desc->idVendor, desc->idProduct);
	printf("DeviceClass:0x%02x, Subclass:0x%02x\n", desc->bDeviceClass, desc->bDeviceSubClass);
	printf("Device Protocol:0x%02x, MaxPacketSize of ep0:%4d\n", desc->bDeviceProtocol, desc->bMaxPacketSize0);
	printf("Num of Configurations:%d\n", desc->bNumConfigurations);
	printf("--------------------------------------------------------------------------------\n");
}

void prt_dev_conf(libusb_config_descriptor *desc)
{
	int idx = 0;
	printf("--------------------------------------------------------------------------------\n");
	printf("dump configuration description\n");
	printf("wTotalLength:%d\n", desc->wTotalLength);
	printf("bNumInterfaces:%d\n", desc->bNumInterfaces);
	printf("bConfigurationValue:%d\n", desc->bConfigurationValue);
	printf("iConfiguration:%d\n", desc->iConfiguration);
	printf("bmAttributes:0x%02x\n", desc->bmAttributes);
	for(idx = 0; idx < desc->bNumInterfaces; idx++)
	{
		const libusb_interface* If = &(desc->interface[idx]);
		prt_dev_if(If);
	}
	printf("--------------------------------------------------------------------------------\n");
}

void prt_dev_if(const libusb_interface *If)
{
	const libusb_interface_descriptor *IfDesc = (If->altsetting);
	printf("--------------------------------------------------------------------------------\n");
	printf("interface num:%d\n", If->num_altsetting);
	printf("bInterfaceNumber:%d\n", IfDesc->bInterfaceNumber);
	printf("bAlternateSetting:%d\n", IfDesc->bAlternateSetting);
	printf("bNumEndpoints:%d\n", IfDesc->bNumEndpoints);
	printf("bInterfaceClass:%d, bInterfaceSubClass:%d, bInterfaceProtocol:%d\n", IfDesc->bInterfaceClass,
			IfDesc->bInterfaceSubClass, IfDesc->bInterfaceProtocol);
	printf("--------------------------------------------------------------------------------\n");
}

const char* formatLibUsbError(int errcode)
{
	const char* result = NULL;
	struct {
		int code;
		const char* string;
	} err_table[] = {
		{ LIBUSB_SUCCESS, "LIBUSB_SUCCESS" },
		{ LIBUSB_ERROR_IO, "LIBUSB_ERROR_IO" },
		{ LIBUSB_ERROR_INVALID_PARAM, "LIBUSB_ERROR_INVALID_PARAM" },
		{ LIBUSB_ERROR_ACCESS, "LIBUSB_ERROR_ACCESS" },
		{ LIBUSB_ERROR_NO_DEVICE, "LIBUSB_ERROR_NO_DEVICE" },
		{ LIBUSB_ERROR_NOT_FOUND, "LIBUSB_ERROR_NOT_FOUND" },
		{ LIBUSB_ERROR_BUSY, "LIBUSB_ERROR_BUSY" },
		{ LIBUSB_ERROR_TIMEOUT, "LIBUSB_ERROR_TIMEOUT" },
		{ LIBUSB_ERROR_OVERFLOW, "LIBUSB_ERROR_OVERFLOW" },
		{ LIBUSB_ERROR_PIPE, "LIBUSB_ERROR_PIPE" },
		{ LIBUSB_ERROR_INTERRUPTED, "LIBUSB_ERROR_INTERRUPTED" },
		{ LIBUSB_ERROR_NO_MEM, "LIBUSB_ERROR_NO_MEM" },
		{ LIBUSB_ERROR_NOT_SUPPORTED, "LIBUSB_ERROR_NOT_SUPPORTED" },
		{ LIBUSB_ERROR_OTHER, "LIBUSB_ERROR_OTHER" },
		{ 0, NULL },
	};

	if ((errcode <= 0) && (errcode >= LIBUSB_ERROR_NOT_SUPPORTED)) {
		result = err_table[0-errcode].string;
	} else {
		result = err_table[0-LIBUSB_ERROR_OTHER].string;
	}
	return result;
}
