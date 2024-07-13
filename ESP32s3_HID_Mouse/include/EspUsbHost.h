#ifndef ESP_USB_HOST_H
#define ESP_USB_HOST_H

#include <Arduino.h>
#include <usb/usb_host.h>
#include <class/hid/hid.h>
#include <rom/usb/usb_common.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include "freertos/queue.h"

#define LOG_QUEUE_SIZE 20
#define LOG_MESSAGE_SIZE 128

static void usb_lib_task(void *arg);
static void usb_client_task(void *arg);

class EspUsbHost
{
public:
    bool isReady = false;
    uint8_t interval;
    bool isClientRegistering = false;
    QueueHandle_t logQueue;
    bool deviceSuspended = false;
    uint32_t last_activity_time;

    struct endpoint_data_t
    {
        uint8_t bInterfaceNumber;
        uint8_t bInterfaceClass;
        uint8_t bInterfaceSubClass;
        uint8_t bInterfaceProtocol;
        uint8_t bCountryCode;
    };
    
    endpoint_data_t endpoint_data_list[17];
    uint8_t _bInterfaceNumber;
    uint8_t _bInterfaceClass;
    uint8_t _bInterfaceSubClass;
    uint8_t _bInterfaceProtocol;
    uint8_t _bCountryCode;
    esp_err_t claim_err;

    usb_host_client_handle_t clientHandle;
    usb_device_handle_t deviceHandle;
    usb_transfer_t *usbTransfer[16];
    uint8_t usbTransferSize;
    uint8_t usbInterface[16];
    uint8_t usbInterfaceSize;

    TaskHandle_t usbTaskHandle = nullptr;
    TaskHandle_t clientTaskHandle = nullptr;

    struct HIDReportDescriptor
    {
        uint8_t buttonSize;
        uint8_t xAxisSize;
        uint8_t yAxisSize;
        uint8_t wheelSize;
    };

    static struct HIDReportDescriptor HIDReportDesc;

    void begin(void);
    static void logTask(void *arg); // Make logTask static
    static void _clientEventCallback(const usb_host_client_event_msg_t *eventMsg, void *arg);
    static void _onReceiveControl(usb_transfer_t *transfer);
    static void monitor_inactivity_task(void *arg);
    static void _onReceive(usb_transfer_t *transfer);

    void get_device_status();
    void suspend_device();
    void resume_device();
    bool logMessage(const char *format, ...);
    void onConfig(const uint8_t bDescriptorType, const uint8_t *p);
    static String getUsbDescString(const usb_str_desc_t *str_desc);
    static void _printPcapText(const char *title, uint16_t function, uint8_t direction, uint8_t endpoint, uint8_t type, uint8_t size, uint8_t stage, const uint8_t *data);
    esp_err_t submitControl(const uint8_t bmRequestType, const uint8_t bDescriptorIndex, const uint8_t bDescriptorType, const uint16_t wInterfaceNumber, const uint16_t wDescriptorLength);
    void _configCallback(const usb_config_desc_t *config_desc);
    static HIDReportDescriptor parseHIDReportDescriptor(uint8_t *data, int length);
    static void handleHIDReportDescriptor(HIDReportDescriptor descriptor);
    virtual void onReceive(const usb_transfer_t *transfer) {};
    virtual void onGone(const usb_host_client_event_msg_t *eventMsg) {};
    virtual void onMouse(hid_mouse_report_t report, uint8_t last_buttons);
    virtual void onMouseButtons(hid_mouse_report_t report, uint8_t last_buttons);
    virtual void onMouseMove(hid_mouse_report_t report);

};

#endif // ESP_USB_HOST_H
