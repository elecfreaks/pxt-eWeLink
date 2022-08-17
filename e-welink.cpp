#include "pxt.h"
#include "MicroBit.h"
#include "MicroBitConfig.h"
#include "EWeLinkService.h"

//% weight=50 icon="\uf1eb" color=#1976d2
namespace eWeLink
{
    EWeLinkService *pService = NULL;
    bool connected = false;
    bool nameScrolling = true;

    // Creates a connected image.
    MicroBitImage connectedImage("\
        000,000,000,000,000\n\
        000,000,000,000,255\n\
        000,000,000,255,000\n\
        255,000,255,000,000\n\
        000,255,000,000,000\n");

    void onConnected(MicroBitEvent)
    {
        nameScrolling = false;
        uBit.display.stopAnimation();
        uBit.display.print(connectedImage, 0, 0, 0, 888);
        uBit.display.clear();
        connected = true;
    }

    void onDisconnected(MicroBitEvent)
    {
        uBit.reset();
    }

    void scrollFriendlyName()
    {
        while (nameScrolling)
        {
            uBit.display.scroll(microbit_friendly_name());
            uBit.sleep(1000);
        }
    }

    //%
    void startService()
    {
        if (pService != NULL)
            return;

        uBit.messageBus.listen(MICROBIT_ID_BLE, MICROBIT_BLE_EVT_CONNECTED, onConnected);
        uBit.messageBus.listen(MICROBIT_ID_BLE, MICROBIT_BLE_EVT_DISCONNECTED, onDisconnected);

        pService = new EWeLinkService(*uBit.ble);

        create_fiber(scrollFriendlyName);
    }

    //%
    bool isConnected()
    {
        return connected;
    }

    //%
    void setTransmitPower(int power)
    {
        uBit.bleManager.setTransmitPower(min(MICROBIT_BLE_POWER_LEVELS - 1, max(0, power)));
    }

    //%
    Buffer readBuffer()
    {
        auto buffer = mkBuffer(NULL, EWELINK_DATA_LENGTH);
        auto res = buffer;
        registerGCObj(buffer);
        uint8_t len = pService->read(buffer->data);
        if (len == 0)
        {
            res = mkBuffer(NULL, 0);
        }
        else
        {
            res = mkBuffer(buffer->data, len);
        }
        unregisterGCObj(buffer);
        return res;
    }
}
