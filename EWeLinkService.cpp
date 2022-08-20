#include "MicroBitConfig.h"
#include "EWeLinkService.h"
#include "MicroBitEvent.h"

//================================================================
#if MICROBIT_CODAL
//================================================================

const uint8_t EWeLinkService::service_base_uuid[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb
};

const uint8_t EWeLinkService::char_base_uuid[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb
};

const uint16_t EWeLinkService::serviceUUID = 0x4b5a;
const uint16_t EWeLinkService::charUUID[mbbs_cIdxCOUNT] = {0x4b5c, 0x4b5b};

EWeLinkService::EWeLinkService(BLEDevice &_ble) : ble(_ble)
{
    // Initialise our characteristic values.
    memset(&rxBuffer, 0, sizeof(rxBuffer));
    memset(&txBuffer, 0, sizeof(txBuffer));

    receivedBytes = 0;

    // Register the base UUID and create the service.
    RegisterBaseUUID(service_base_uuid);
    CreateService(serviceUUID);

    RegisterBaseUUID(char_base_uuid);
    CreateCharacteristic(
        mbbs_cIdxRX, charUUID[mbbs_cIdxRX],
        (uint8_t *)&rxBuffer, 1,
        sizeof(rxBuffer), microbit_propWRITE
    );

    CreateCharacteristic(
        mbbs_cIdxTX, charUUID[mbbs_cIdxTX],
        (uint8_t *)&txBuffer, 1,
        sizeof(txBuffer), microbit_propREAD
    );
}

uint8_t EWeLinkService::read(uint8_t *data)
{
    uint8_t len = receivedBytes;
    memcpy(data, &rxBuffer, len);
    return len;
}

/**
 * A callback function for whenever a Bluetooth device writes to our RX characteristic.
 */
void EWeLinkService::onDataWritten(const microbit_ble_evt_write_t *params)
{
    if (params->handle == valueHandle(mbbs_cIdxRX) && params->len > 0)
    {
        receivedBytes = params->len;
        memcpy(&rxBuffer, params->data, params->len);
        MicroBitEvent(MICROBIT_ID_EWELINK, MICROBIT_EWELINK_EVT_ON_WRITTEN);
    }
}

//================================================================
#else // MICROBIT_CODAL
//================================================================

#include "ble/UUID.h"

EWeLinkService::EWeLinkService(BLEDevice &_ble) : ble(_ble)
{
    GattCharacteristic rxCharacteristic(
        EWeLinkRxCharacteristicUUID, (uint8_t *)&rxBuffer, 0,
        sizeof(rxBuffer), GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE);

    GattCharacteristic txCharacteristic(
        EWeLinkTxCharacteristicUUID, (uint8_t *)&txBuffer, 0,
        sizeof(txBuffer), GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);

    // Initialise our characteristic values.
    memset(&rxBuffer, 0, sizeof(rxBuffer));
    memset(&txBuffer, 0, sizeof(txBuffer));

    receivedBytes = 0;

    // Set default security requirements
    rxCharacteristic.requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);
    txCharacteristic.requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);

    // setup GATT table
    GattCharacteristic *characteristics[] = {&rxCharacteristic, &txCharacteristic};
    GattService service(EWeLinkServiceUUID, characteristics, sizeof(characteristics) / sizeof(GattCharacteristic *));
    ble.addService(service);

    // retreive handles
    rxCharacteristicHandle = rxCharacteristic.getValueHandle();
    txCharacteristicHandle = txCharacteristic.getValueHandle();

    // initialize data
    ble.gattServer().write(txCharacteristicHandle, (uint8_t *)&txBuffer, sizeof(txBuffer));
    ble.gattServer().onDataWritten(this, &EWeLinkService::onDataWritten);
}

void EWeLinkService::write(const uint8_t *data, uint8_t len)
{
    if (ble.getGapState().connected)
    {
        len = len > EWELINK_DATA_LENGTH ? EWELINK_DATA_LENGTH : len;
        memcpy(&txBuffer, data, len);
        ble.gattServer().notify(txCharacteristicHandle, (uint8_t *)&txBuffer, len);
    }
}

uint8_t EWeLinkService::read(uint8_t *data)
{
    uint8_t len = receivedBytes;
    memcpy(data, &rxBuffer, len);
    return len;
}

/**
 * A callback function for whenever a Bluetooth device writes to our RX characteristic.
 */
void EWeLinkService::onDataWritten(const GattWriteCallbackParams *params)
{
    if (params->handle == this->rxCharacteristicHandle && params->len > 0)
    {
        receivedBytes = params->len;
        memcpy(&rxBuffer, params->data, params->len);
        MicroBitEvent(MICROBIT_ID_EWELINK, MICROBIT_EWELINK_EVT_ON_WRITTEN);
    }
}

const uint8_t EWeLinkServiceUUID[] = {
    0x00, 0x00, 0x4b, 0x5a, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb
};

const uint8_t EWeLinkRxCharacteristicUUID[] = {
    0x00, 0x00, 0x4b, 0x5c, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb
};

const uint8_t EWeLinkTxCharacteristicUUID[] = {
    0x00, 0x00, 0x4b, 0x5b, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb
};

//================================================================
#endif // MICROBIT_CODAL
//================================================================
