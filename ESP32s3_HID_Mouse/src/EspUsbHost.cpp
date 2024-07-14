#include "EspUsbHost.h"
#include "esp_task_wdt.h"
#include <driver/timer.h>
#include "freertos/queue.h"

#define USB_TASK_STACK_SIZE 4096
#define USB_TASK_PRIORITY 1
#define CLIENT_TASK_PRIORITY 2
#define USB_FEATURE_SELECTOR_REMOTE_WAKEUP 1
bool EspUsbHost::deviceConnected = false;
EspUsbHost::HIDReportDescriptor EspUsbHost::HIDReportDesc;

void EspUsbHost::begin(void)
{
  usbTransferSize = 0;
  deviceSuspended = false;
  last_activity_time = millis();

  const usb_host_config_t host_config = {
      .intr_flags = ESP_INTR_FLAG_LEVEL1,
  };

  esp_err_t err = usb_host_install(&host_config);
  Serial0.print("usb_host_install() status: ");
  Serial0.println(err);
  if (err != ESP_OK)
  {
    return;
  }

  logQueue = xQueueCreate(LOG_QUEUE_SIZE, LOG_MESSAGE_SIZE);

  xTaskCreate(logTask, "LogTask", 2048, this, 1, NULL);
  xTaskCreate(usb_lib_task, "usbLibTask", USB_TASK_STACK_SIZE, this, USB_TASK_PRIORITY, NULL);
  xTaskCreate(usb_client_task, "usbClientTask", USB_TASK_STACK_SIZE, this, CLIENT_TASK_PRIORITY, NULL);
  xTaskCreate(monitor_inactivity_task, "MonitorInactivityTask", 2048, this, 1, NULL);
}

void EspUsbHost::logTask(void *arg)
{
  EspUsbHost *instance = static_cast<EspUsbHost *>(arg);
  char logMsg[LOG_MESSAGE_SIZE];
  while (true)
  {
    if (xQueueReceive(instance->logQueue, &logMsg, portMAX_DELAY) == pdPASS)
    {
      Serial1.print(logMsg);
    }
  }
}

static void usb_lib_task(void *arg)
{
  EspUsbHost *instance = static_cast<EspUsbHost *>(arg);

  while (true)
  {
    uint32_t event_flags;
    esp_err_t err = usb_host_lib_handle_events(portMAX_DELAY, &event_flags);

    if (err != ESP_OK)
    {
      ESP_LOGI("EspUsbHost", "usb_host_lib_handle_events() err=%x", err);
      continue; // Handle the error and continue polling
    }

    // Check if the client is not registered and re-register if needed
    if (instance->clientHandle == NULL || !instance->isClientRegistering)
    {
      ESP_LOGI("EspUsbHost", "registering client...");
      const usb_host_client_config_t client_config = {
          .max_num_event_msg = 5,
          .async = {
              .client_event_callback = instance->_clientEventCallback,
              .callback_arg = instance,
          }};

      err = usb_host_client_register(&client_config, &instance->clientHandle);
      ESP_LOGI("EspUsbHost", "usb_host_client_register() status: %d", err);
      if (err != ESP_OK)
      {
        ESP_LOGI("EspUsbHost", "Failed to re-register client, retrying...");
        vTaskDelay(100);
      }
      else
      {
        ESP_LOGI("EspUsbHost", "Client registered successfully.");
        instance->isClientRegistering = true;
      }
    }
  }
}

static void usb_client_task(void *arg)
{
  EspUsbHost *instance = static_cast<EspUsbHost *>(arg);

  while (true)
  {
    if (!instance->isClientRegistering)
    {
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }
    esp_err_t err = usb_host_client_handle_events(instance->clientHandle, portMAX_DELAY);
  }
}

bool EspUsbHost::logMessage(const char *format, ...)
{
  char logMsg[LOG_MESSAGE_SIZE];
  va_list args;
  va_start(args, format);
  vsnprintf(logMsg, sizeof(logMsg), format, args);
  va_end(args);

  // Attempt to send the log message to the queue without blocking
  if (xQueueSend(logQueue, &logMsg, 0) != pdPASS)
  {
    return false; // Queue full, message not logged
  }
  return true;
}

void EspUsbHost::monitor_inactivity_task(void *arg)
{
  EspUsbHost *usbHost = (EspUsbHost *)arg;

  while (true)
  {
    vTaskDelay(pdMS_TO_TICKS(100));

    if (EspUsbHost::deviceConnected && !usbHost->deviceSuspended && millis() - usbHost->last_activity_time > 10000) // 10 seconds of inactivity
    {
      Serial0.println("No activity for 10 seconds, suspending device");
      usbHost->suspend_device();
    }
  }
}

void EspUsbHost::get_device_status()
{
  if (!EspUsbHost::deviceConnected)
  {
    return;
  }

  usb_transfer_t *transfer;
  esp_err_t err = usb_host_transfer_alloc(8 + 2, 0, &transfer);
  if (err != ESP_OK)
  {
    Serial0.print("usb_host_transfer_alloc() err=");
    Serial0.println(err, HEX);
    return;
  }

  // Set up the control transfer for GET_STATUS
  transfer->num_bytes = 8 + 2;
  transfer->data_buffer[0] = USB_BM_REQUEST_TYPE_DIR_IN | USB_BM_REQUEST_TYPE_TYPE_STANDARD | USB_BM_REQUEST_TYPE_RECIP_DEVICE;
  transfer->data_buffer[1] = USB_B_REQUEST_GET_STATUS;
  transfer->data_buffer[2] = 0x00;
  transfer->data_buffer[3] = 0x00;
  transfer->data_buffer[4] = 0x00;
  transfer->data_buffer[5] = 0x00;
  transfer->data_buffer[6] = 0x02;
  transfer->data_buffer[7] = 0x00;

  transfer->device_handle = deviceHandle;
  transfer->bEndpointAddress = 0x00;
  transfer->callback = [](usb_transfer_t *transfer)
  {
    EspUsbHost *usbHost = (EspUsbHost *)transfer->context;

    if (transfer->status == USB_TRANSFER_STATUS_COMPLETED)
    {
      uint16_t status = (transfer->data_buffer[9] << 8) | transfer->data_buffer[8];
      Serial0.print("Device status: ");
      Serial0.println(status, HEX);

      // Check the Remote Wakeup status
      if (status & (1 << USB_FEATURE_SELECTOR_REMOTE_WAKEUP))
      {
        Serial0.println("Remote Wakeup is enabled.");
      }
      else
      {
        Serial0.println("Remote Wakeup is disabled.");
      }
    }
    else
    {
      Serial0.print("GET_STATUS transfer failed with status=");
      Serial0.println(transfer->status, HEX);
    }

    usb_host_transfer_free(transfer);
  };
  transfer->context = this;

  // Submit the control transfer
  err = usb_host_transfer_submit_control(clientHandle, transfer);
  if (err != ESP_OK)
  {
    Serial0.print("usb_host_transfer_submit_control() err=");
    Serial0.println(err, HEX);
    usb_host_transfer_free(transfer);
    return;
  }
}

void EspUsbHost::suspend_device()
{
  usb_transfer_t *transfer;
  esp_err_t err = usb_host_transfer_alloc(8 + 1, 0, &transfer);
  if (err != ESP_OK)
  {
    Serial0.print("usb_host_transfer_alloc() err=");
    Serial0.println(err, HEX);
    return;
  }

  // Set up the control transfer for SET_FEATURE (Remote Wakeup)
  transfer->num_bytes = 8;
  transfer->data_buffer[0] = USB_BM_REQUEST_TYPE_DIR_OUT | USB_BM_REQUEST_TYPE_TYPE_STANDARD | USB_BM_REQUEST_TYPE_RECIP_DEVICE;
  transfer->data_buffer[1] = USB_B_REQUEST_SET_FEATURE;          
  transfer->data_buffer[2] = USB_FEATURE_SELECTOR_REMOTE_WAKEUP; 
  transfer->data_buffer[3] = 0x00;
  transfer->data_buffer[4] = 0x00;
  transfer->data_buffer[5] = 0x00;
  transfer->data_buffer[6] = 0x00;
  transfer->data_buffer[7] = 0x00;

  transfer->device_handle = deviceHandle;
  transfer->bEndpointAddress = 0x00;
  transfer->callback = _onReceiveControl;
  transfer->context = this;

  // Submit the control transfer
  err = usb_host_transfer_submit_control(clientHandle, transfer);
  if (err != ESP_OK)
  {
    Serial0.print("usb_host_transfer_submit_control() err=");
    Serial0.println(err, HEX);
  }

  vTaskDelay(pdMS_TO_TICKS(10));

  deviceSuspended = true;

  Serial0.println("Device suspended successfully.");

  get_device_status();
}

void EspUsbHost::resume_device()
{
  if (!EspUsbHost::deviceConnected)
  {
    return;
  }

  usb_transfer_t *transfer;
  esp_err_t err = usb_host_transfer_alloc(8 + 1, 0, &transfer);
  if (err != ESP_OK)
  {
    Serial0.print("usb_host_transfer_alloc() err=");
    Serial0.println(err, HEX);
    return;
  }

  // Set up the control transfer for CLEAR_FEATURE (Remote Wakeup)
  transfer->num_bytes = 8;
  transfer->data_buffer[0] = USB_BM_REQUEST_TYPE_DIR_OUT | USB_BM_REQUEST_TYPE_TYPE_STANDARD | USB_BM_REQUEST_TYPE_RECIP_DEVICE;
  transfer->data_buffer[1] = USB_B_REQUEST_CLEAR_FEATURE;       
  transfer->data_buffer[2] = USB_FEATURE_SELECTOR_REMOTE_WAKEUP; 
  transfer->data_buffer[3] = 0x00;
  transfer->data_buffer[4] = 0x00;
  transfer->data_buffer[5] = 0x00;
  transfer->data_buffer[6] = 0x00;
  transfer->data_buffer[7] = 0x00;

  transfer->device_handle = deviceHandle;
  transfer->bEndpointAddress = 0x00;
  transfer->callback = _onReceiveControl;
  transfer->context = this;

  // Submit the control transfer
  err = usb_host_transfer_submit_control(clientHandle, transfer);
  if (err != ESP_OK)
  {
    Serial0.print("usb_host_transfer_submit_control() err=");
    Serial0.println(err, HEX);
  }

  deviceSuspended = false;

  Serial0.println("Device resumed successfully.");

  get_device_status();
}

void EspUsbHost::_printPcapText(const char *title, uint16_t function, uint8_t direction, uint8_t endpoint, uint8_t type, uint8_t size, uint8_t stage,
                                const uint8_t *data)
{
#if (ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO)
  uint8_t urbsize = 0x1c;
  if (stage == 0xff)
  {
    urbsize = 0x1b;
  }

  String data_str = "";
  for (int i = 0; i < size; i++)
  {
    if (data[i] < 16)
    {
      data_str += "0";
    }
    data_str += String(data[i], HEX) + " ";
  }

  printf("\n");
  printf("[PCAP TEXT]%s\n", title);
  printf("0000  %02x 00 00 00 00 00 00 00 00 00 00 00 00 00 %02x %02x\n", urbsize, (function & 0xff), ((function >> 8) & 0xff));
  printf("0010  %02x 01 00 01 00 %02x %02x %02x 00 00 00", direction, endpoint, type, size);
  if (stage != 0xff)
  {
    printf(" %02x\n", stage);
  }
  else
  {
    printf("\n");
  }
  printf("00%02x  %s\n", urbsize, data_str.c_str());
  printf("\n");
#endif
}

String EspUsbHost::getUsbDescString(const usb_str_desc_t *str_desc)
{
  String str = "";
  if (str_desc == NULL)
  {
    return str;
  }

  for (int i = 0; i < str_desc->bLength / 2; i++)
  {
    if (str_desc->wData[i] > 0xFF)
    {
      continue;
    }
    str += char(str_desc->wData[i]);
  }
  return str;
}

void EspUsbHost::onConfig(const uint8_t bDescriptorType, const uint8_t *p)
{
  switch (bDescriptorType)
  {
  case USB_DEVICE_DESC:
    ESP_LOGI("EspUsbHost", "USB_DEVICE_DESC(0x01)");
    break;
  case USB_CONFIGURATION_DESC:
  {
#if (ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO)
    const usb_config_desc_t *config_desc = (const usb_config_desc_t *)p;
    ESP_LOGI("EspUsbHost", "USB_CONFIGURATION_DESC(0x02)\n"
                           "# bLength             = %d\n"
                           "# bDescriptorType     = %d\n"
                           "# wTotalLength        = %d\n"
                           "# bNumInterfaces      = %d\n"
                           "# bConfigurationValue = %d\n"
                           "# iConfiguration      = %d\n"
                           "# bmAttributes        = 0x%x\n"
                           "# bMaxPower           = %dmA",
             config_desc->bLength,
             config_desc->bDescriptorType,
             config_desc->wTotalLength,
             config_desc->bNumInterfaces,
             config_desc->bConfigurationValue,
             config_desc->iConfiguration,
             config_desc->bmAttributes,
             config_desc->bMaxPower * 2);
#endif
    break;
  }
  case USB_STRING_DESC:
  {
#if (ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO)
    const usb_standard_desc_t *desc = (const usb_standard_desc_t *)p;
    String data_str = "";
    for (int i = 0; i < (desc->bLength - 2); i++)
    {
      if (desc->val[i] < 16)
      {
        data_str += "0";
      }
      data_str += String(desc->val[i], HEX) + " ";
    }
    ESP_LOGI("EspUsbHost", "USB_STRING_DESC(0x03) bLength=%d, bDescriptorType=0x%x, data=[%s]",
             desc->bLength,
             desc->bDescriptorType,
             data_str);
#endif
    break;
  }
  case USB_INTERFACE_DESC:
  {
    const usb_intf_desc_t *intf = (const usb_intf_desc_t *)p;
    ESP_LOGI("EspUsbHost", "USB_INTERFACE_DESC(0x04)\n"
                           "# bLength            = %d\n"
                           "# bDescriptorType    = %d\n"
                           "# bInterfaceNumber   = %d\n"
                           "# bAlternateSetting  = %d\n"
                           "# bNumEndpoints      = %d\n"
                           "# bInterfaceClass    = 0x%x\n"
                           "# bInterfaceSubClass = 0x%x\n"
                           "# bInterfaceProtocol = 0x%x\n"
                           "# iInterface         = %d",
             intf->bLength,
             intf->bDescriptorType,
             intf->bInterfaceNumber,
             intf->bAlternateSetting,
             intf->bNumEndpoints,
             intf->bInterfaceClass,
             intf->bInterfaceSubClass,
             intf->bInterfaceProtocol,
             intf->iInterface);

    this->claim_err = usb_host_interface_claim(this->clientHandle, this->deviceHandle, intf->bInterfaceNumber, intf->bAlternateSetting);
    if (this->claim_err != ESP_OK)
    {
      ESP_LOGI("EspUsbHost", "usb_host_interface_claim() err=%x", claim_err);
    }
    else
    {
      ESP_LOGI("EspUsbHost", "usb_host_interface_claim() ESP_OK");
      this->usbInterface[this->usbInterfaceSize] = intf->bInterfaceNumber;
      this->usbInterfaceSize++;
      _bInterfaceNumber = intf->bInterfaceNumber;
      _bInterfaceClass = intf->bInterfaceClass;
      _bInterfaceSubClass = intf->bInterfaceSubClass;
      _bInterfaceProtocol = intf->bInterfaceProtocol;
    }
    break;
  }
  case USB_ENDPOINT_DESC:
  {
    const usb_ep_desc_t *ep_desc = (const usb_ep_desc_t *)p;
    ESP_LOGI("EspUsbHost", "USB_ENDPOINT_DESC(0x05)\n"
                           "# bLength          = %d\n"
                           "# bDescriptorType  = %d\n"
                           "# bEndpointAddress = 0x%x(EndpointID=%d, Direction=%s)\n"
                           "# bmAttributes     = 0x%x(%s)\n"
                           "# wMaxPacketSize   = %d\n"
                           "# bInterval        = %d",
             ep_desc->bLength,
             ep_desc->bDescriptorType,
             ep_desc->bEndpointAddress, USB_EP_DESC_GET_EP_NUM(ep_desc), USB_EP_DESC_GET_EP_DIR(ep_desc) ? "IN" : "OUT",
             ep_desc->bmAttributes,
             (ep_desc->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK) == USB_BM_ATTRIBUTES_XFER_CONTROL ? "CTRL" : (ep_desc->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK) == USB_BM_ATTRIBUTES_XFER_ISOC ? "ISOC"
                                                                                                                : (ep_desc->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK) == USB_BM_ATTRIBUTES_XFER_BULK   ? "BULK"
                                                                                                                : (ep_desc->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK) == USB_BM_ATTRIBUTES_XFER_INT    ? "Interrupt"
                                                                                                                                                                                                             : "",
             ep_desc->wMaxPacketSize,
             ep_desc->bInterval);

    if (this->claim_err != ESP_OK)
    {
      ESP_LOGI("EspUsbHost", "claim_err skip");
      return;
    }

    this->endpoint_data_list[USB_EP_DESC_GET_EP_NUM(ep_desc)].bInterfaceNumber = _bInterfaceNumber;
    this->endpoint_data_list[USB_EP_DESC_GET_EP_NUM(ep_desc)].bInterfaceClass = _bInterfaceClass;
    this->endpoint_data_list[USB_EP_DESC_GET_EP_NUM(ep_desc)].bInterfaceSubClass = _bInterfaceSubClass;
    this->endpoint_data_list[USB_EP_DESC_GET_EP_NUM(ep_desc)].bInterfaceProtocol = _bInterfaceProtocol;
    this->endpoint_data_list[USB_EP_DESC_GET_EP_NUM(ep_desc)].bCountryCode = _bCountryCode;

    if ((ep_desc->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK) != USB_BM_ATTRIBUTES_XFER_INT)
    {
      ESP_LOGI("EspUsbHost", "err ep_desc->bmAttributes=%x", ep_desc->bmAttributes);
      return;
    }

    if (ep_desc->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK)
    {
      esp_err_t err = usb_host_transfer_alloc(ep_desc->wMaxPacketSize + 1, 0, &this->usbTransfer[this->usbTransferSize]);
      if (err != ESP_OK)
      {
        this->usbTransfer[this->usbTransferSize] = NULL;
        ESP_LOGI("EspUsbHost", "usb_host_transfer_alloc() err=%x", err);
        return;
      }
      else
      {
        ESP_LOGI("EspUsbHost", "usb_host_transfer_alloc() ESP_OK data_buffer_size=%d", ep_desc->wMaxPacketSize + 1);
      }

      this->usbTransfer[this->usbTransferSize]->device_handle = this->deviceHandle;
      this->usbTransfer[this->usbTransferSize]->bEndpointAddress = ep_desc->bEndpointAddress;
      this->usbTransfer[this->usbTransferSize]->callback = this->_onReceive;
      this->usbTransfer[this->usbTransferSize]->context = this;
      this->usbTransfer[this->usbTransferSize]->num_bytes = ep_desc->wMaxPacketSize;
      interval = ep_desc->bInterval;
      isReady = true;
      this->usbTransferSize++;

      ESP_LOGI("EspUsbHost", "usb_host_transfer_submit for endpoint 0x%x", ep_desc->bEndpointAddress);

      // Submit the transfer only if the endpoint is 0x81
      // if (ep_desc->bEndpointAddress == 0x81)  // Removed to fix some devices that report incorrectly
      {
        err = usb_host_transfer_submit(this->usbTransfer[this->usbTransferSize - 1]);
        if (err != ESP_OK)
        {

          ESP_LOGI("EspUsbHost", "usb_host_transfer_submit() err=%x", err);
        }
      }
    }
    break;
  }
  case USB_INTERFACE_ASSOC_DESC:
  {
#if (ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO)
    const usb_iad_desc_t *iad_desc = (const usb_iad_desc_t *)p;
    ESP_LOGI("EspUsbHost", "USB_INTERFACE_ASSOC_DESC(0x0b)\n"
                           "# bLength           = %d\n"
                           "# bDescriptorType   = %d\n"
                           "# bFirstInterface   = %d\n"
                           "# bInterfaceCount   = %d\n"
                           "# bFunctionClass    = 0x%x\n"
                           "# bFunctionSubClass = 0x%x\n"
                           "# bFunctionProtocol = 0x%x\n"
                           "# iFunction         = %d",
             iad_desc->bLength,
             iad_desc->bDescriptorType,
             iad_desc->bFirstInterface,
             iad_desc->bInterfaceCount,
             iad_desc->bFunctionClass,
             iad_desc->bFunctionSubClass,
             iad_desc->bFunctionProtocol,
             iad_desc->iFunction);
#endif
    break;
  }
  case USB_HID_DESC:
  {
    const tusb_hid_descriptor_hid_t *hid_desc = (const tusb_hid_descriptor_hid_t *)p;
    ESP_LOGI("EspUsbHost", "USB_HID_DESC(0x21)\n"
                           "# bLength         = %d\n"
                           "# bDescriptorType = 0x%x\n"
                           "# bcdHID          = 0x%x\n"
                           "# bCountryCode    = 0x%x\n"
                           "# bNumDescriptors = %d\n"
                           "# bReportType     = 0x%x\n"
                           "# wReportLength   = %d",
             hid_desc->bLength,
             hid_desc->bDescriptorType,
             hid_desc->bcdHID,
             hid_desc->bCountryCode,
             hid_desc->bNumDescriptors,
             hid_desc->bReportType,
             hid_desc->wReportLength);
    _bCountryCode = hid_desc->bCountryCode;

    submitControl(0x81, 0x00, 0x22, _bInterfaceNumber, hid_desc->wReportLength);
    break;
  }
  default:
  {
#if (ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO)
    const usb_standard_desc_t *desc = (const usb_standard_desc_t *)p;
    String data_str = "";
    for (int i = 0; i < (desc->bLength - 2); i++)
    {
      if (desc->val[i] < 16)
      {
        data_str += "0";
      }
      data_str += String(desc->val[i], HEX) + " ";
    }
    ESP_LOGI("EspUsbHost", "USB_???_DESC(%02x) bLength=%d, bDescriptorType=0x%x, data=[%s]",
             bDescriptorType,
             desc->bLength,
             desc->bDescriptorType,
             data_str);
#endif
    break;
  }
  }
}

void EspUsbHost::_clientEventCallback(const usb_host_client_event_msg_t *eventMsg, void *arg)
{
  EspUsbHost *instance = static_cast<EspUsbHost *>(arg);
  EspUsbHost *usbHost = (EspUsbHost *)arg;

  esp_err_t err;
  switch (eventMsg->event)
  {
  case USB_HOST_CLIENT_EVENT_NEW_DEV:
    ESP_LOGI("EspUsbHost", "New USB device connected");
    err = usb_host_device_open(usbHost->clientHandle, eventMsg->new_dev.address, &usbHost->deviceHandle);
    if (err != ESP_OK)
    {
      ESP_LOGI("EspUsbHost", "usb_host_device_open() err=%x", err);
    }
    else
    {
      ESP_LOGI("EspUsbHost", "usb_host_device_open() ESP_OK");
      usbHost->isReady = true;
      EspUsbHost::deviceConnected = true;
    }

    usb_device_info_t dev_info;
    err = usb_host_device_info(usbHost->deviceHandle, &dev_info);
    if (err != ESP_OK)
    {
      ESP_LOGI("EspUsbHost", "usb_host_device_info() err=%x", err);
    }
    else
    {
      ESP_LOGI("EspUsbHost", "usb_host_device_info() ESP_OK\n"
                             "# speed                 = %d\n"
                             "# dev_addr              = %d\n"
                             "# vMaxPacketSize0       = %d\n"
                             "# bConfigurationValue   = %d\n"
                             "# str_desc_manufacturer = \"%s\"\n"
                             "# str_desc_product      = \"%s\"\n"
                             "# str_desc_serial_num   = \"%s\"\n",
               dev_info.speed,
               dev_info.dev_addr,
               dev_info.bMaxPacketSize0,
               dev_info.bConfigurationValue,
               getUsbDescString(dev_info.str_desc_manufacturer).c_str(),
               getUsbDescString(dev_info.str_desc_product).c_str(),
               getUsbDescString(dev_info.str_desc_serial_num).c_str());
    }

    const usb_device_desc_t *dev_desc;
    err = usb_host_get_device_descriptor(usbHost->deviceHandle, &dev_desc);
    if (err != ESP_OK)
    {
      ESP_LOGI("EspUsbHost", "usb_host_get_device_descriptor() err=%x", err);
    }
    else
    {
      const uint8_t setup[8] = {
          0x80,
          0x06,
          0x00,
          0x01,
          0x00,
          0x00,
          0x12,
          0x00};
      _printPcapText("GET DESCRIPTOR Request DEVICE", 0x000b, 0x00, 0x80, 0x02, sizeof(setup), 0x00, setup);
      _printPcapText("GET DESCRIPTOR Response DEVICE", 0x0008, 0x01, 0x80, 0x02, sizeof(usb_device_desc_t), 0x03, (const uint8_t *)dev_desc);

      ESP_LOGI("EspUsbHost", "usb_host_get_device_descriptor() ESP_OK\n"
                             "#### DESCRIPTOR DEVICE ####\n"
                             "# bLength            = %d\n"
                             "# bDescriptorType    = %d\n"
                             "# bcdUSB             = 0x%x\n"
                             "# bDeviceClass       = 0x%x\n"
                             "# bDeviceSubClass    = 0x%x\n"
                             "# bDeviceProtocol    = 0x%x\n"
                             "# bMaxPacketSize0    = %d\n"
                             "# idVendor           = 0x%x\n"
                             "# idProduct          = 0x%x\n"
                             "# bcdDevice          = 0x%x\n"
                             "# iManufacturer      = %d\n"
                             "# iProduct           = %d\n"
                             "# iSerialNumber      = %d\n"
                             "# bNumConfigurations = %d\n",
               dev_desc->bLength,
               dev_desc->bDescriptorType,
               dev_desc->bcdUSB,
               dev_desc->bDeviceClass,
               dev_desc->bDeviceSubClass,
               dev_desc->bDeviceProtocol,
               dev_desc->bMaxPacketSize0,
               dev_desc->idVendor,
               dev_desc->idProduct,
               dev_desc->bcdDevice,
               dev_desc->iManufacturer,
               dev_desc->iProduct,
               dev_desc->iSerialNumber,
               dev_desc->bNumConfigurations);
    }

    const usb_config_desc_t *config_desc;
    err = usb_host_get_active_config_descriptor(usbHost->deviceHandle, &config_desc);
    if (err != ESP_OK)
    {
      ESP_LOGI("EspUsbHost", "usb_host_get_active_config_descriptor() err=%x", err);
    }
    else
    {
      const uint8_t setup[8] = {
          0x80,
          0x06,
          0x00,
          0x02,
          0x00,
          0x00,
          0x09,
          0x00};
      _printPcapText("GET DESCRIPTOR Request CONFIGURATION", 0x000b, 0x00, 0x80, 0x02, sizeof(setup), 0x00, setup);
      _printPcapText("GET DESCRIPTOR Response CONFIGURATION", 0x0008, 0x01, 0x80, 0x02, sizeof(usb_config_desc_t), 0x03, (const uint8_t *)config_desc);

      ESP_LOGI("EspUsbHost", "usb_host_get_active_config_descriptor() ESP_OK\n"
                             "# bLength             = %d\n"
                             "# bDescriptorType     = %d\n"
                             "# wTotalLength        = %d\n"
                             "# bNumInterfaces      = %d\n"
                             "# bConfigurationValue = %d\n"
                             "# iConfiguration      = %d\n"
                             "# bmAttributes        = 0x%x\n"
                             "# bMaxPower           = %dmA\n",
               config_desc->bLength,
               config_desc->bDescriptorType,
               config_desc->wTotalLength,
               config_desc->bNumInterfaces,
               config_desc->bConfigurationValue,
               config_desc->iConfiguration,
               config_desc->bmAttributes,
               config_desc->bMaxPower * 2);
    }

    usbHost->isReady = true;
    usbHost->_configCallback(config_desc);
    break;

  case USB_HOST_CLIENT_EVENT_DEV_GONE:
    ESP_LOGI("EspUsbHost", "USB device disconnected");
    usbHost->isReady = false;
    EspUsbHost::deviceConnected = false;

    // Log start of cleanup
    ESP_LOGI("EspUsbHost", "Starting cleanup of USB device");

    for (int i = 0; i < usbHost->usbTransferSize; i++)
    {
      if (usbHost->usbTransfer[i] == NULL)
      {
        continue;
      }

      err = usb_host_transfer_free(usbHost->usbTransfer[i]);
      if (err != ESP_OK)
      {
        ESP_LOGI("EspUsbHost", "usb_host_transfer_free() err=%x, usbTransfer=%x", err, usbHost->usbTransfer[i]);
      }
      else
      {
        ESP_LOGI("EspUsbHost", "usb_host_transfer_free() ESP_OK, usbTransfer=%x", usbHost->usbTransfer[i]);
      }

      usbHost->usbTransfer[i] = NULL;
    }
    usbHost->usbTransferSize = 0;

    for (int i = 0; i < usbHost->usbInterfaceSize; i++)
    {
      err = usb_host_interface_release(usbHost->clientHandle, usbHost->deviceHandle, usbHost->usbInterface[i]);
      if (err != ESP_OK)
      {
        ESP_LOGI("EspUsbHost", "usb_host_interface_release() err=%x, clientHandle=%x, deviceHandle=%x, Interface=%x", err, usbHost->clientHandle, usbHost->deviceHandle, usbHost->usbInterface[i]);
      }
      else
      {
        ESP_LOGI("EspUsbHost", "usb_host_interface_release() ESP_OK, clientHandle=%x, deviceHandle=%x, Interface=%x", usbHost->clientHandle, usbHost->deviceHandle, usbHost->usbInterface[i]);
      }

      usbHost->usbInterface[i] = 0;
    }
    usbHost->usbInterfaceSize = 0;

    err = usb_host_device_close(usbHost->clientHandle, usbHost->deviceHandle);
    if (err != ESP_OK)
    {
      ESP_LOGI("EspUsbHost", "usb_host_device_close() err=%x", err);
    }
    else
    {
      ESP_LOGI("EspUsbHost", "usb_host_device_close() ESP_OK");
    }

    usbHost->isReady = false;

    ESP_LOGI("EspUsbHost", "Cleanup complete, device gone event handled");
    usbHost->isClientRegistering = false;

    // Call the onGone event
    usbHost->onGone(eventMsg);
    break;

  default:
    ESP_LOGI("EspUsbHost", "clientEventCallback() default %d", eventMsg->event);
    break;
  }
}

void EspUsbHost::_configCallback(const usb_config_desc_t *config_desc)
{
  const uint8_t *p = &config_desc->val[0];
  uint8_t bLength;

  const uint8_t setup[8] = {
      0x80,
      0x06,
      0x00,
      0x02,
      0x00,
      0x00,
      (uint8_t)config_desc->wTotalLength,
      0x00};
  _printPcapText("GET DESCRIPTOR Request CONFIGURATION", 0x000b, 0x00, 0x80, 0x02, sizeof(setup), 0x00, setup);
  _printPcapText("GET DESCRIPTOR Response CONFIGURATION", 0x0008, 0x01, 0x80, 0x02, config_desc->wTotalLength, 0x03, (const uint8_t *)config_desc);

  for (int i = 0; i < config_desc->wTotalLength; i += bLength, p += bLength)
  {
    bLength = *p;
    if ((i + bLength) <= config_desc->wTotalLength)
    {
      const uint8_t bDescriptorType = *(p + 1);
      this->onConfig(bDescriptorType, p);
    }
    else
    {
      return;
    }
  }
}

void EspUsbHost::_onReceive(usb_transfer_t *transfer)
{
  EspUsbHost *usbHost = (EspUsbHost *)transfer->context;
  uint8_t endpoint_num = transfer->bEndpointAddress & 0x0F;

  bool has_data = (transfer->actual_num_bytes > 0);

  if (has_data)
  {
    usbHost->last_activity_time = millis();
    if (EspUsbHost::deviceConnected)
    {
      if (usbHost->deviceSuspended)
      {
        usbHost->resume_device();
      }
    }
  }

  if (usbHost->endpoint_data_list[endpoint_num].bInterfaceClass == USB_CLASS_HID)
  {
    if (usbHost->endpoint_data_list[endpoint_num].bInterfaceSubClass == HID_SUBCLASS_BOOT &&
        usbHost->endpoint_data_list[endpoint_num].bInterfaceProtocol == HID_ITF_PROTOCOL_MOUSE)
    {
      static uint8_t last_buttons = 0;
      hid_mouse_report_t report = {};

      report.buttons = transfer->data_buffer[HIDReportDesc.buttonStartByte];

      if (HIDReportDesc.xAxisSize == 12 && HIDReportDesc.yAxisSize == 12)
      {
        uint8_t xyOffset = HIDReportDesc.xAxisStartByte;

        int16_t xValue = (transfer->data_buffer[xyOffset]) |
                         ((transfer->data_buffer[xyOffset + 1] & 0x0F) << 8);

        int16_t yValue = ((transfer->data_buffer[xyOffset + 1] >> 4) & 0x0F) |
                         (transfer->data_buffer[xyOffset + 2] << 4);

        report.x = xValue;
        report.y = yValue;

        uint8_t wheelOffset = HIDReportDesc.wheelStartByte;
        report.wheel = transfer->data_buffer[wheelOffset];
      }
      else
      {
        uint8_t xOffset = HIDReportDesc.xAxisStartByte;
        uint8_t yOffset = HIDReportDesc.yAxisStartByte;
        uint8_t wheelOffset = HIDReportDesc.wheelStartByte;

        report.x = transfer->data_buffer[xOffset];
        report.y = transfer->data_buffer[yOffset];
        report.wheel = transfer->data_buffer[wheelOffset];
      }

      usbHost->onMouse(report, last_buttons);

      if (report.buttons != last_buttons)
      {
        usbHost->onMouseButtons(report, last_buttons);
        last_buttons = report.buttons;
      }

      if (report.x != 0 || report.y != 0 || report.wheel != 0)
      {
        usbHost->onMouseMove(report);
      }
    }

    if (transfer->status == USB_TRANSFER_STATUS_COMPLETED)
    {
      ESP_LOGI("EspUsbHost", "Transfer Completed Successfully: Endpoint=%x", transfer->bEndpointAddress);
    }
    else if (transfer->status == USB_TRANSFER_STATUS_STALL)
    {
      ESP_LOGI("EspUsbHost", "Transfer STALL Received: Endpoint=%x", transfer->bEndpointAddress);
    }
    else
    {
      ESP_LOGI("EspUsbHost", "Transfer Error or Incomplete: Status=%x, Endpoint=%x", transfer->status, transfer->bEndpointAddress);
    }
    if (!usbHost->deviceSuspended)
    {
      usb_host_transfer_submit(transfer);
    }
  }
}

void EspUsbHost::onMouse(hid_mouse_report_t report, uint8_t last_buttons)
{
  ESP_LOGD("EspUsbHost", "last_buttons=0x%02x(%c%c%c%c%c), buttons=0x%02x(%c%c%c%c%c), x=%d, y=%d, wheel=%d",
           last_buttons,
           (last_buttons & MOUSE_BUTTON_LEFT) ? 'L' : ' ',
           (last_buttons & MOUSE_BUTTON_RIGHT) ? 'R' : ' ',
           (last_buttons & MOUSE_BUTTON_MIDDLE) ? 'M' : ' ',
           (last_buttons & MOUSE_BUTTON_BACKWARD) ? 'B' : ' ',
           (last_buttons & MOUSE_BUTTON_FORWARD) ? 'F' : ' ',
           report.buttons,
           (report.buttons & MOUSE_BUTTON_LEFT) ? 'L' : ' ',
           (report.buttons & MOUSE_BUTTON_RIGHT) ? 'R' : ' ',
           (report.buttons & MOUSE_BUTTON_MIDDLE) ? 'M' : ' ',
           (report.buttons & MOUSE_BUTTON_BACKWARD) ? 'B' : ' ',
           (report.buttons & MOUSE_BUTTON_FORWARD) ? 'F' : ' ',
           report.x,
           report.y,
           report.wheel);
}

void EspUsbHost::onMouseButtons(hid_mouse_report_t report, uint8_t last_buttons)
{
  if (!(last_buttons & MOUSE_BUTTON_LEFT) && (report.buttons & MOUSE_BUTTON_LEFT))
  {
    logMessage("km.left(1)\n");
  }
  if ((last_buttons & MOUSE_BUTTON_LEFT) && !(report.buttons & MOUSE_BUTTON_LEFT))
  {
    logMessage("km.left(0)\n");
  }

  if (!(last_buttons & MOUSE_BUTTON_RIGHT) && (report.buttons & MOUSE_BUTTON_RIGHT))
  {
    logMessage("km.right(1)\n");
  }
  if ((last_buttons & MOUSE_BUTTON_RIGHT) && !(report.buttons & MOUSE_BUTTON_RIGHT))
  {
    logMessage("km.right(0)\n");
  }

  if (!(last_buttons & MOUSE_BUTTON_MIDDLE) && (report.buttons & MOUSE_BUTTON_MIDDLE))
  {
    logMessage("km.middle(1)\n");
  }
  if ((last_buttons & MOUSE_BUTTON_MIDDLE) && !(report.buttons & MOUSE_BUTTON_MIDDLE))
  {
    logMessage("km.middle(0)\n");
  }

  if (!(last_buttons & MOUSE_BUTTON_FORWARD) && (report.buttons & MOUSE_BUTTON_FORWARD))
  {
    logMessage("km.side1(1)\n");
  }
  if ((last_buttons & MOUSE_BUTTON_FORWARD) && !(report.buttons & MOUSE_BUTTON_FORWARD))
  {
    logMessage("km.side1(0)\n");
  }

  if (!(last_buttons & MOUSE_BUTTON_BACKWARD) && (report.buttons & MOUSE_BUTTON_BACKWARD))
  {
    logMessage("km.side2(1)\n");
  }
  if ((last_buttons & MOUSE_BUTTON_BACKWARD) && !(report.buttons & MOUSE_BUTTON_BACKWARD))
  {
    logMessage("km.side2(0)\n");
  }
}

void EspUsbHost::onMouseMove(hid_mouse_report_t report)
{
  if (report.wheel != 0)
  {
    logMessage("km.wheel(%d)\n", report.wheel);
  }
  else
  {
    logMessage("km.move(%d,%d)\n", report.x, report.y);
  }
}

esp_err_t EspUsbHost::submitControl(const uint8_t bmRequestType,
                                    const uint8_t bDescriptorIndex,
                                    const uint8_t bDescriptorType,
                                    const uint16_t wInterfaceNumber,
                                    const uint16_t wDescriptorLength)
{
  usb_transfer_t *transfer;
  usb_host_transfer_alloc(wDescriptorLength + 8 + 1, 0, &transfer);

  transfer->num_bytes = wDescriptorLength + 8;
  transfer->data_buffer[0] = bmRequestType;
  transfer->data_buffer[1] = 0x06;
  transfer->data_buffer[2] = bDescriptorIndex;
  transfer->data_buffer[3] = bDescriptorType;
  transfer->data_buffer[4] = wInterfaceNumber & 0xff;
  transfer->data_buffer[5] = wInterfaceNumber >> 8;
  transfer->data_buffer[6] = wDescriptorLength & 0xff;
  transfer->data_buffer[7] = wDescriptorLength >> 8;

  transfer->device_handle = deviceHandle;
  transfer->bEndpointAddress = 0x00;
  transfer->callback = _onReceiveControl;
  transfer->context = this;

  esp_err_t err = usb_host_transfer_submit_control(clientHandle, transfer);
  if (err != ESP_OK)
  {
    ESP_LOGI("EspUsbHost", "usb_host_transfer_submit_control() err=%x", err);
  }
  return err;
}

void EspUsbHost::_onReceiveControl(usb_transfer_t *transfer)
{
  bool isMouse = false;
  uint8_t *p = &transfer->data_buffer[8];
  int totalBytes = transfer->actual_num_bytes;

  // Check for the specific sequence indicating a mouse device
  for (int i = 0; i < totalBytes - 3; i++)
  {
    if (p[i] == 0x05 && p[i + 1] == 0x01 && p[i + 2] == 0x09 && p[i + 3] == 0x02)
    {
      isMouse = true;
      break;
    }
  }

  if (!isMouse)
  {
    usb_host_transfer_free(transfer);
    return;
  }

  printf("GET DESCRIPTOR Response HID Report: Length: %d\n", transfer->actual_num_bytes - 8);
  printf("#### HID REPORT DESCRIPTOR ####\n");

  for (int i = 8; i < transfer->actual_num_bytes; i++)
  {
    printf("%02X ", transfer->data_buffer[i]);
    if ((i - 7) % 16 == 0)
    {
      printf("\n");
    }
  }
  printf("\n");

  HIDReportDescriptor descriptor = parseHIDReportDescriptor(&transfer->data_buffer[8], transfer->actual_num_bytes - 8);

  handleHIDReportDescriptor(descriptor);

  usb_host_transfer_free(transfer);
}

static uint8_t getItemSize(uint8_t prefix)
{
  return (prefix & 0x03) + 1;
}


static uint8_t getItemType(uint8_t prefix)
{
  return prefix & 0xFC;
}

EspUsbHost::HIDReportDescriptor EspUsbHost::parseHIDReportDescriptor(uint8_t *data, int length)
{
  int i = 0;
  uint8_t usagePage = 0;
  uint8_t usage = 0;
  uint8_t reportId = 0;
  uint8_t reportSize = 0;
  uint8_t reportCount = 0;
  int16_t logicalMin16 = 0;
  int16_t logicalMax = 0;
  int8_t logicalMin8 = 0;
  int8_t logicalMax8 = 0;
  uint8_t level = 0;
  uint8_t size = 0;
  uint8_t collection = 0;
  bool hasReportId = false;
  uint16_t currentBitOffset = 0;

  auto getValue = [](uint8_t *data, int size, bool isSigned) -> int16_t
  {
    int16_t value = 0;
    if (isSigned)
    {
      if (size == 1)
      {
        value = (int8_t)data[0];
      }
      else if (size == 2)
      {
        value = (int16_t)(data[0] | (data[1] << 8));
      }
    }
    else
    {
      if (size == 1)
      {
        value = (uint8_t)data[0];
      }
      else if (size == 2)
      {
        value = (uint16_t)(data[0] | (data[1] << 8));
      }
    }
    return value;
  };

  HIDReportDescriptor HIDReportDesc = {0};

  while (i < length)
  {
    uint8_t prefix = data[i];
    size = (prefix & 0x03);
    size = (size == 3) ? 4 : size; // Correct size for 4-byte items
    uint8_t item = prefix & 0xFC;
    bool isSigned = (item == 0x14 || item == 0x24);
    int16_t value = getValue(data + i + 1, size, isSigned);

    
    printf("\n\nRaw Bytes:\n");
    printf("* ");
    for (int j = 0; j <= size; j++)
    {
      printf("%02X ", data[i + j]);
    }
    printf("\n");

    // Print item and value
    printf("* Item: %02X, Value: ", item);
    if (value == 0)
    {
      printf("00");
    }
    else if (size == 1)
    {
      printf("%02X", (uint8_t)value);
    }
    else if (size == 2)
    {
      printf("%02X %02X", data[i + 1], data[i + 2]);
    }
    else if (size == 4)
    {
      printf("%02X %02X %02X %02X", data[i + 1], data[i + 2], data[i + 3], data[i + 4]);
    }
    printf("\n");

    switch (item)
    {
    case 0x04: // USAGE_PAGE
      usagePage = (uint8_t)value;
      printf("* Parsed USAGE_PAGE: %02X\n", usagePage);
      break;
    case 0x08: // USAGE
      usage = (uint8_t)value;
      printf("* Parsed USAGE: %02X\n", usage);
      break;
    case 0x84: // REPORT_ID
      reportId = value;
      HIDReportDesc.reportId = reportId;
      hasReportId = true;
      currentBitOffset += 8; // Increment bit offset by 8 bits (1 byte) due to report ID
      printf("* Parsed REPORT_ID: %02X\n", reportId);
      break;
    case 0x74: // REPORT_SIZE
      reportSize = value;
      printf("* Parsed REPORT_SIZE: %02X\n", reportSize);
      break;
    case 0x94: // REPORT_COUNT
      reportCount = value;
      printf("* Parsed REPORT_COUNT: %02X\n", reportCount);
      break;
    case 0x14: // LOGICAL_MINIMUM
      if (size == 1)
      {
        logicalMin8 = (int8_t)value;
        printf("* Parsed LOGICAL_MINIMUM (8-bit): %02X\n", (uint8_t)logicalMin8);
      }
      else
      {
        logicalMin16 = value;
        printf("* Parsed LOGICAL_MINIMUM (16-bit): %02X %02X\n", (uint8_t)(logicalMin16 & 0xFF), (uint8_t)((logicalMin16 >> 8) & 0xFF));
      }
      break;
    case 0x24: // LOGICAL_MAXIMUM
      if (size == 1)
      {
        logicalMax8 = (int8_t)value;
        printf("* Parsed LOGICAL_MAXIMUM (8-bit): %02X\n", (uint8_t)logicalMax8);
      }
      else
      {
        logicalMax = value;
        printf("* Parsed LOGICAL_MAXIMUM (16-bit): %02X %02X\n", (uint8_t)(logicalMax & 0xFF), (uint8_t)((logicalMax >> 8) & 0xFF));
      }
      break;
    case 0xA0: // COLLECTION
      printf("* Parsed COLLECTION: %02X\n", value);
      level++;
      collection = value;
      if (collection == 0x00)
      {
        printf("* (Physical)\n");
      }
      else if (collection == 0x01)
      {
        printf("* (Application)\n");
      }
      break;
    case 0xC0: // END_COLLECTION
      printf("* Parsed END_COLLECTION\n");
      level--;
      break;
    case 0x80: // INPUT
      printf("* Parsed INPUT: (%s, %s, %s)\n",
             (value & (1 << 0)) ? "Cnst" : "Data",
             (value & (1 << 1)) ? "Var" : "Ary",
             (value & (1 << 2)) ? "Rel" : "Abs");

      // Determine what type of input we are dealing with based on USAGE
      if (usagePage == 0x01 && (usage == 0x30 || usage == 0x31))
      { // X-axis or Y-axis
        if (logicalMax <= 2047)
        {
          // Handle 12-bit range
          HIDReportDesc.xAxisSize = 12;                        
          HIDReportDesc.yAxisSize = 12;                        
          HIDReportDesc.xAxisStartByte = currentBitOffset / 8; 
          currentBitOffset += 12;
          HIDReportDesc.yAxisStartByte = currentBitOffset / 8; 
          currentBitOffset += 12;
        }
        else
        {
          // Handle 8-bit or 16-bit ranges
          HIDReportDesc.xAxisSize = HIDReportDesc.yAxisSize =
              (logicalMax <= 127) ? 8 : 16;                    
          HIDReportDesc.xAxisStartByte = currentBitOffset / 8; 
          currentBitOffset += HIDReportDesc.xAxisSize;
          HIDReportDesc.yAxisStartByte = currentBitOffset / 8; 
          currentBitOffset += HIDReportDesc.yAxisSize;
        }
      }
      else if (usagePage == 0x01 && usage == 0x38)
      {                                                                               
        HIDReportDesc.wheelSize = (logicalMax <= 127 && logicalMax >= -128) ? 8 : 16; 
        HIDReportDesc.wheelStartByte = currentBitOffset / 8;                          
        currentBitOffset += HIDReportDesc.wheelSize;
      }
      else if (usagePage == 0x09 && usage >= 0x01 && usage <= 0x10)
      { // Buttons
        printf("* Parsed BUTTONS: Report Count: %d\n", reportCount);
        HIDReportDesc.buttonSize = reportCount * reportSize;  
        HIDReportDesc.buttonStartByte = currentBitOffset / 8; 
        currentBitOffset += HIDReportDesc.buttonSize;
        printf("* Calculated Button Bitmap Size: %d bits\n", HIDReportDesc.buttonSize);
      }
      break;
    case 0x18: // USAGE_MINIMUM
      printf("* Parsed USAGE_MINIMUM: %02X\n", value);
      break;
    case 0x28: // USAGE_MAXIMUM
      printf("* Parsed USAGE_MAXIMUM: %02X\n", value);
      break;
    default:
      printf("* Unknown item.\n");
      break;
    }

    i += size + 1;
  }

  return HIDReportDesc;
}

void EspUsbHost::handleHIDReportDescriptor(HIDReportDescriptor descriptor)
{
  HIDReportDesc = descriptor; // Ensure static member is updated

  // Debug prints to verify the descriptor values
  Serial0.println("Mouse Struct()");
  Serial0.print("Button Bitmap Size: ");
  Serial0.println(descriptor.buttonSize);

  if (descriptor.xAxisSize != 0)
  {
    Serial0.print("X-Axis Size: ");
    Serial0.println(descriptor.xAxisSize);
  }

  if (descriptor.yAxisSize != 0)
  {
    Serial0.print("Y-Axis Size: ");
    Serial0.println(descriptor.yAxisSize);
  }

  Serial0.print("Wheel Max Size: ");
  Serial0.println(descriptor.wheelSize);
  Serial0.println("-----------------------------------------------------");

  // Print the start positions of the fields
  Serial0.print("Button Start Byte: ");
  Serial0.println(descriptor.buttonStartByte);

  Serial0.print("X Axis Start Byte: ");
  Serial0.println(descriptor.xAxisStartByte);

  Serial0.print("Y Axis Start Byte: ");
  Serial0.println(descriptor.yAxisStartByte);

  Serial0.print("Wheel Start Byte: ");
  Serial0.println(descriptor.wheelStartByte);
}
