#include "ds3ps2.h"
#include <thbase.h>
#include <sifcmd.h>
#include <usbd.h>
#include <usbd_macro.h>
#include <string.h>

void  rpc_thread(void *data);
void *rpc_server_func(int command, void *buffer, int size);

SifRpcDataQueue_t  rpc_queue  __attribute__((aligned(64)));
SifRpcServerData_t rpc_server __attribute__((aligned(64)));

static int _rpc_buffer[512] __attribute((aligned(64)));
static u8 data_buf[DS3PS3_INPUT_LEN] __attribute((aligned(64)));
//static int inputEndp;
//static int outputEndp;
static int controlEndp;

int usb_probe(int devId);
int usb_connect(int devId);
int usb_disconnect(int devId);

UsbDriver driver = { NULL, NULL, "ds3ps2", usb_probe, usb_connect, usb_disconnect };

static void request_data(int result, int count, void *arg);
static void config_set(int result, int count, void *arg);
static void ds3_set_operational();
static int send_ledsrumble();
static void set_led(unsigned char n);

int _start()
{
    iop_thread_t th = {
        .attr      = TH_C,
        .thread    = rpc_thread,
        .priority  = 40,
        .stacksize = 0x800,
        .option    = 0
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
    
    if (dev->idVendor == DS3_VID && dev->idProduct == DS3_PID)
        return 1;
    
    return 0;
}

int usb_connect(int devId)
{
    UsbDeviceDescriptor    *dev;
    UsbConfigDescriptor    *conf;
    //UsbInterfaceDescriptor *intf;
    //UsbEndpointDescriptor  *endp;
    
    int configEndp;

    dev = UsbGetDeviceStaticDescriptor(devId, NULL, USB_DT_DEVICE);
    conf = UsbGetDeviceStaticDescriptor(devId, dev, USB_DT_CONFIG);
    configEndp = UsbOpenEndpoint(devId, NULL);
    
    controlEndp = configEndp;
    /*
    intf = (UsbInterfaceDescriptor *) ((char *) conf + conf->bLength);
    endp = (UsbEndpointDescriptor *) ((char *) intf + intf->bLength); // HID endpoint
    controlEndp = UsbOpenEndpoint(devId, endp);
    endp = (UsbEndpointDescriptor *) ((char *) endp + endp->bLength); // Interrupt Input endpoint
    inputEndp = UsbOpenEndpoint(devId, endp);
    endp = (UsbEndpointDescriptor *) ((char *) endp + endp->bLength); // Interrupt Output endpoint
    outputEndp = UsbOpenEndpoint(devId, endp);
    */
    
    UsbSetDevicePrivateData(devId, NULL);
    UsbSetDeviceConfiguration(configEndp, conf->bConfigurationValue, config_set, NULL);
    return 0;
}

int usb_disconnect(int devId)
{
    return 0;
}

static void config_set(int result, int count, void *arg)
{
    //Set operational
    ds3_set_operational();
    //Set LED
    set_led(1);
    send_ledsrumble();
    //Start reading!
    request_data(0, 0, NULL);
}

#define INTERFACE_GET (USB_DIR_IN|USB_TYPE_CLASS|USB_RECIP_INTERFACE)
#define INTERFACE_SET (USB_DIR_OUT|USB_TYPE_CLASS|USB_RECIP_INTERFACE)
#define USB_REPTYPE_INPUT       0x01
#define USB_REPTYPE_OUTPUT      0x02
#define USB_REPTYPE_FEATURE     0x03

static void request_data(int result, int count, void *arg)
{
    UsbControlTransfer(controlEndp,
        INTERFACE_GET,
        USB_REQ_GET_REPORT,
        (USB_REPTYPE_INPUT<<8) | 0x01,
        0x0,
        DS3PS3_INPUT_LEN,
        data_buf,
        request_data,
        NULL);
}

u8 opbuf[17] __attribute((aligned(64)));
static void ds3_set_operational()
{
    UsbControlTransfer(controlEndp,
        INTERFACE_GET,
        USB_REQ_GET_REPORT,
        (USB_REPTYPE_FEATURE<<8) | 0xf2,
        0x0,
        17,
        opbuf,
        NULL, NULL);
}

static const u8 led_pattern[] = {0x0, 0x02, 0x04, 0x08, 0x10, 0x12, 0x14, 0x18};
static u8 __attribute__((aligned(64))) ledsrumble_buf[] =
{
    0x52,
    0x00, 0x00, 0x00, 0x00, //Rumble
    0xff, 0x80,             //Gyro
    0x00, 0x00,
    0x02, //* LED_1 = 0x02, LED_2 = 0x04, ... */
    0xff, 0x27, 0x10, 0x00, 0x32, /* LED_4 */
    0xff, 0x27, 0x10, 0x00, 0x32, /* LED_3 */
    0xff, 0x27, 0x10, 0x00, 0x32, /* LED_2 */
    0xff, 0x27, 0x10, 0x00, 0x32, /* LED_1 */
};

static int send_ledsrumble()
{
    return UsbControlTransfer(controlEndp,
        INTERFACE_SET,
        USB_REQ_SET_REPORT,
        (USB_REPTYPE_OUTPUT<<8) | 0x01,
        0x0,
        sizeof(ledsrumble_buf),
        ledsrumble_buf,
        NULL, NULL);  
}

static void set_led(unsigned char n)
{
    ledsrumble_buf[9] = led_pattern[n];
}

static void set_rumble(unsigned char power_r, unsigned char time_r, 
    unsigned char power_l, unsigned char time_l)
{
    ledsrumble_buf[1] = time_r;
    ledsrumble_buf[2] = power_r;
    ledsrumble_buf[3] = time_l;
    ledsrumble_buf[4] = power_l;
}

void *rpc_server_func(int command, void *buffer, int size)
{
    u8 *b8 = (u8*)buffer;
    
    switch (command) {
    case DS3PS2_SET_LED:
        set_led(b8[0]);
        break;
    case DS3PS2_SET_RUMBLE:
        set_rumble(b8[0], b8[1], b8[2], b8[3]);
        break;
    case DS3PS2_SEND_LEDSRUMBLE:
        send_ledsrumble();
        break;
    case DS3PS2_GET_INPUT:
        memcpy(buffer, data_buf, size);
        break;
    }
    
    return buffer;
}

