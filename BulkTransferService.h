/*
 * BulkTransferService.h
 *
 *  Created on: 2016-02-02
 *      Author: KUANG Qi
 */

#ifndef __BLE_BULK_TRANSFER_SERVICE_H__
#define __BLE_BULK_TRANSFER_SERVICE_H__

#include "ble/BLE.h"

typedef void (*RequestCallback)(int addr, int len);

#define PAYLOAD_SIZE 16
#define UUID_BULK_DATA          0x1111
#define UUID_BULK_CONTROL_POINT 0x2222

class BulkTransferService
{
public:
    //typedef void (*callback_t)(AlertLevel_t level);

    /**
     * @param[ref] ble
     *               BLE object for the underlying controller.
     */
    BulkTransferService(BLE &bleIn, uint8_t *dataBuffer, int bufSize, RequestCallback callback) :
            ble(bleIn),
            bufferBase(dataBuffer),
			bufferCursor(dataBuffer),
            bulkTransferData(UUID_BULK_DATA, bufferCursor, PAYLOAD_SIZE, PAYLOAD_SIZE, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
            bulkTransferControlPoint(UUID_BULK_CONTROL_POINT, (uint8_t *)controlPointBase, 8, 8, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE)
    {
        static bool serviceAdded = false; /* We should only ever add one Immediate Alert service. */
        bufferSize = bufSize;
        requestData = callback;
        lengthRequested = 0;
        addressRequested = 0;
        bytesInBuffer = 0;
        bytesInTransaction = 0;
        addressCursor = 0;
        isSubscibed = false;

        if (serviceAdded)
        {
            return;
        }

        GattCharacteristic *charTable[] = {&bulkTransferData, &bulkTransferControlPoint};
        GattService bulkTransferService(0x1825, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
        ble.gattServer().addService(bulkTransferService);
        serviceAdded = true;

        ble.gattServer().onDataWritten(this, &BulkTransferService::onDataWritten);
        ble.gattServer().onDataSent(this, &BulkTransferService::onDataSent);
        ble.gattServer().onUpdatesEnabled(FunctionPointerWithContext<uint16_t>(this, &BulkTransferService::onUpdatesEnabled));
        ble.gattServer().onUpdatesDisabled(FunctionPointerWithContext<uint16_t>(this, &BulkTransferService::onUpdatesDisables));
    }

    uint32_t getAddress()
    {
    	return addressRequested;
    }

    uint32_t getLength()
    {
    	return lengthRequested;
    }

    void dataPrepared(int length)
    {
    	if(isSubscibed)
    	{
    		bytesInBuffer = length;
    		bufferCursor = bufferBase;
    		sendBuffer();
    	}
    }

protected:
    /**
     * This callback allows receiving updates to the Bulk Transfer Control Point characteristic.
     *
     * @param[in] params
     *     Information about the characteristic being updated.
     */
    virtual void onDataWritten(const GattWriteCallbackParams *params)
    {
        if (params->handle == bulkTransferControlPoint.getValueHandle())
        {
        	uint8_t *ptr = (uint8_t *)controlPointBase;
        	for(int i = 0; i < 8; i++) ptr[i] = params->data[i];
        	addressRequested = controlPointBase[0];
        	lengthRequested = controlPointBase[1];
        	addressCursor = addressRequested;
        	bytesInTransaction = lengthRequested;
            if(bytesInTransaction > 0)
            {
            	printf("%d bytes of data from 0x%06x are requested\r\n", lengthRequested, addressRequested);
            	requestData(addressCursor, bytesInTransaction < bufferSize ? bytesInTransaction : bufferSize);
            }
        }
    }

    void onDataSent(unsigned count)
    {
    	sendBuffer();
    }

    virtual void onUpdatesEnabled(uint16_t handle)
    {
    	if(handle == bulkTransferData.getValueHandle()) isSubscibed = true;
    }

    virtual void onUpdatesDisables(uint16_t handle)
    {
    	if(handle == bulkTransferData.getValueHandle()) isSubscibed = false;
    }

    virtual void sendBuffer()
    {
    	int len;
    	ble_error_t didSendValue = BLE_ERROR_NONE;
    	while(bytesInBuffer > 0)
    	{
    		len = bytesInBuffer < PAYLOAD_SIZE ? bytesInBuffer : PAYLOAD_SIZE;
			didSendValue = ble.gattServer().write(bulkTransferData.getValueHandle(), bufferCursor, len);
			if(didSendValue != BLE_ERROR_NONE) break;
			//for(int i = 0; i < len; i++) printf("%x", (int)bufferCursor[i]);
			//printf("\r\n");
			bufferCursor += len;
			bytesInBuffer -= len;
			bytesInTransaction -= len;
			addressCursor += len;
    	}
    	if(bytesInTransaction != 0 && bytesInBuffer == 0)
		{
			requestData(addressCursor, bytesInTransaction < bufferSize ? bytesInTransaction : bufferSize);
		}
    }

protected:
    BLE &ble;

    // Pointer to the data buffer
    uint8_t *bufferBase;
    // Cursor to the data buffer
    uint8_t *bufferCursor;
    // Length of the data buffer
    int      bufferSize;
    // Memory of Bulk Transfer Control Point
    uint32_t controlPointBase[2]; // [0]: address  [1]: length

    // Address of data (in flash) which are requested by the client
    int addressRequested;
    // A cursor of data (in flash)
    int addressCursor;
    // Length of data (in flash) which are requested by the client
    int lengthRequested;

    // Number of bytes to be sent in the buffer
    int bytesInBuffer;
    // Number of bytes to be sent in a requested transaction
    int bytesInTransaction;
    // Characteristic Bulk Transfer Data is subscribed by client
    bool isSubscibed;

    // BLE characteristics
    GattCharacteristic bulkTransferData;
    GattCharacteristic bulkTransferControlPoint;

    RequestCallback    requestData;
};

#endif /* __BLE_BULK_TRANSFER_SERVICE_H__ */
