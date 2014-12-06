#include <thbase.h>
#include <thsemap.h>
#include <sifcmd.h>
#include <loadcore.h>
#include <usbd.h>
#include <usbd_macro.h>
#include <string.h>
#include "ds3ps2.h"

#define MODNAME "ds3ps2"
IRX_ID(MODNAME, 1, 1);
struct irx_export_table exp_ds3ps2;

static void  rpc_thread(void *data);
static void *rpc_server_func(int command, void *buffer, int size);

static SifRpcDataQueue_t  rpc_queue  __attribute__((aligned(64)));
static SifRpcServerData_t rpc_server __attribute__((aligned(64)));

static int _rpc_buffer[512] __attribute((aligned(64)));
static u8 data_buf[DS3PS2_MAX_SLOTS][DS3PS2_INPUT_LEN] __attribute((aligned(64)));
static u8 opbuf[17] __attribute((aligned(64)));

static int usb_probe(int devId);
static int usb_connect(int devId);
static int usb_disconnect(int devId);

static UsbDriver driver = { NULL, NULL, "ds3ps2", usb_probe, usb_connect, usb_disconnect };

static void request_data(int result, int count, void *arg);
static void config_set(int result, int count, void *arg);
static void ds3_set_operational(int slot);

static struct {
	int sema;
	int devID;
	int connected;
	int endp;
	int led;
	struct {
		int time_r, power_r;
		int time_l, power_l;
	} rumble;
} ds3_list[DS3PS2_MAX_SLOTS];

int _start()
{
	RegisterLibraryEntries(&exp_ds3ps2);
	
	iop_thread_t th = {
		.attr	   = TH_C,
		.thread	   = rpc_thread,
		.priority  = 40,
		.stacksize = 0x800,
		.option	   = 0
	};
	UsbRegisterDriver(&driver);

	int thid = CreateThread(&th);
	if (thid > 0) {
		StartThread(thid, NULL);
		return 0;
	}

	return 1;
}

void rpc_thread(void *data)
{
	int thid = GetThreadId();
	SifInitRpc(0);
	SifSetRpcQueue(&rpc_queue, thid);
	SifRegisterRpc(&rpc_server, DS3PS2_BIND_RPC_ID, rpc_server_func,
		_rpc_buffer, NULL, NULL, &rpc_queue);
	SifRpcLoop(&rpc_queue);
}

int usb_probe(int devId)
{
	UsbDeviceDescriptor *dev = NULL;
	dev = UsbGetDeviceStaticDescriptor(devId, NULL, USB_DT_DEVICE);
	if (!dev)
		return 0;
	if (dev->idVendor == DS3_VID && dev->idProduct == DS3_PID) {
		//Check if there's an available slot
		int i;
		for (i = 0; i < DS3PS2_MAX_SLOTS; i++) {
			//We've found an empy slot :)
			if (!ds3_list[i].connected)
				return 1;
		}
	}
	return 0;
}

int usb_connect(int devId)
{
	UsbDeviceDescriptor *dev;
	UsbConfigDescriptor *conf;
	dev = UsbGetDeviceStaticDescriptor(devId, NULL, USB_DT_DEVICE);
	conf = UsbGetDeviceStaticDescriptor(devId, dev, USB_DT_CONFIG);
	int controlEndp = UsbOpenEndpoint(devId, NULL);
	int slot = 0, i;
	for (i = 0; i < DS3PS2_MAX_SLOTS; i++) {
		if (!ds3_list[i].connected) {
			slot = i;
			break;
		}
	}
	ds3_list[slot].endp = controlEndp;
	ds3_list[slot].devID = devId;
	ds3_list[slot].connected = 1;
	ds3_list[slot].sema = CreateMutex(IOP_MUTEX_UNLOCKED);

	UsbSetDevicePrivateData(devId, NULL);
	UsbSetDeviceConfiguration(controlEndp, conf->bConfigurationValue, config_set, (void*)slot);
	return 0;
}

int usb_disconnect(int devId)
{
	int i;
	for (i = 0; i < DS3PS2_MAX_SLOTS; i++) {
		if (devId == ds3_list[i].devID) {
			DeleteSema(ds3_list[i].sema);
			ds3_list[i].connected = 0;
		}
	}
	return 1;
}

static void config_set(int result, int count, void *arg)
{
	int slot = (int)arg;
	//Set operational
	ds3_set_operational(slot);
	//Set LED
	ds3ps2_set_led(slot, slot+1);
	ds3ps2_send_ledsrumble(slot);
	//Start reading!
	request_data(0, 0, (void *)slot);
}

#define INTERFACE_GET (USB_DIR_IN|USB_TYPE_CLASS|USB_RECIP_INTERFACE)
#define INTERFACE_SET (USB_DIR_OUT|USB_TYPE_CLASS|USB_RECIP_INTERFACE)
#define USB_REPTYPE_INPUT		0x01
#define USB_REPTYPE_OUTPUT		0x02
#define USB_REPTYPE_FEATURE		0x03


#define swap16(x) (((x&0xFF)<<8)|((x>>8)&0xFF))
static void correct_data(struct ds3_input *data)
{
	data->accelX = swap16(data->accelX);
	data->accelY = swap16(data->accelY);
	data->accelZ = swap16(data->accelZ);
	data->gyroZ  = swap16(data->gyroZ);
}

static void request_data_cb(int result, int count, void *arg)
{
	int slot = (int)arg;

	WaitSema(ds3_list[slot].sema);

	correct_data((struct ds3_input *)data_buf[slot]);
	DelayThread(1000); //1ms

	SignalSema(ds3_list[slot].sema);

	request_data(0, 0, (void *)slot);
}

static void request_data(int result, int count, void *arg)
{
	int slot = (int)arg;
	UsbControlTransfer(ds3_list[slot].endp,
		INTERFACE_GET,
		USB_REQ_GET_REPORT,
		(USB_REPTYPE_INPUT<<8) | 0x01,
		0x0,
		DS3PS2_INPUT_LEN,
		data_buf[slot],
		request_data_cb,
		arg);
}

static void ds3_set_operational(int slot)
{
	UsbControlTransfer(ds3_list[slot].endp,
		INTERFACE_GET,
		USB_REQ_GET_REPORT,
		(USB_REPTYPE_FEATURE<<8) | 0xf2,
		0x0,
		17,
		opbuf,
		NULL, NULL);
}

static const unsigned char led_pattern[] = {
	0x0, 0x02, 0x04, 0x08, 0x10, 0x12, 0x14, 0x18, 0x1A, 0x1C, 0x1E
};
static u8 __attribute__((aligned(64))) ledsrumble_buf[] =
{
	0x01, //Report ID
	0x00, //Padding
	0x00, 0x00, 0x00, 0x00, //Rumble (r, r, l, l)
	0x00, 0x00, 0x00, 0x00, //Padding
	0x00, /* LED_1 = 0x02, LED_2 = 0x04, ... */
	0xff, 0x27, 0x10, 0x00, 0x32, /* LED_4 */
	0xff, 0x27, 0x10, 0x00, 0x32, /* LED_3 */
	0xff, 0x27, 0x10, 0x00, 0x32, /* LED_2 */
	0xff, 0x27, 0x10, 0x00, 0x32, /* LED_1 */
	0x00, 0x00, 0x00, 0x00, 0x00  /* LED_5 (not soldered) */
};

int ds3ps2_send_ledsrumble(int slot)
{
	WaitSema(ds3_list[slot].sema);

	ledsrumble_buf[9] = led_pattern[ds3_list[slot].led];
	ledsrumble_buf[1] = ds3_list[slot].rumble.time_r;
	ledsrumble_buf[2] = ds3_list[slot].rumble.power_r;
	ledsrumble_buf[3] = ds3_list[slot].rumble.time_l;
	ledsrumble_buf[4] = ds3_list[slot].rumble.power_l;
	int ret = UsbControlTransfer(ds3_list[slot].endp,
		INTERFACE_SET,
		USB_REQ_SET_REPORT,
		(USB_REPTYPE_OUTPUT<<8) | 0x01,
		0x0,
		sizeof(ledsrumble_buf),
		ledsrumble_buf,
		NULL, NULL);

	SignalSema(ds3_list[slot].sema);
	return ret;
}

int ds3ps2_slot_connected(int slot)
{
	return ds3_list[slot].connected;
}

void ds3ps2_set_led(int slot, u8 n)
{
	ds3_list[slot].led = n;
}

void ds3ps2_set_rumble(int slot, u8 power_r, u8 time_r, u8 power_l, u8 time_l)
{
	ds3_list[slot].rumble.time_r = time_r;
	ds3_list[slot].rumble.power_r = power_r;
	ds3_list[slot].rumble.time_l = time_l;
	ds3_list[slot].rumble.power_l = power_l;
}

void ds3ps2_get_input(int slot, u8 *buffer)
{
	memcpy(buffer, data_buf[slot], DS3PS2_INPUT_LEN);
}

void *rpc_server_func(int command, void *buffer, int size)
{
	u8 *b8 = (u8*)buffer;
	int slot = b8[0];
	switch (command) {
	case DS3PS2_SLOT_CONNECTED:
		b8[0] = ds3ps2_slot_connected(slot);
	case DS3PS2_SET_LED:
		ds3ps2_set_led(slot, b8[1]);
		break;
	case DS3PS2_SET_RUMBLE:
		ds3ps2_set_rumble(slot, b8[1], b8[2], b8[3], b8[4]);
		break;
	case DS3PS2_SEND_LEDSRUMBLE:
		ds3ps2_send_ledsrumble(slot);
		break;
	case DS3PS2_GET_FULL_INPUT:
		ds3ps2_get_input(slot, buffer);
		break;
	}
	return buffer;
}

