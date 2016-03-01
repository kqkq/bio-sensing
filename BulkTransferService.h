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

class BulkTransferService
{
public:
    //typedef void (*callback_t)(AlertLevel_t level);

    /**
     * @param[ref] ble
     *               BLE object for the underlying controller.
     */
    BulkTransferService(BLE &bleIn, uint8_t *dataBuffer, int bufferSize, RequestCallback callback) :
            ble(bleIn),
            dataptr(dataBuffer),
			cursor(dataBuffer),
            bulkTransferData(0x1111, cursor, PAYLOAD_SIZE, PAYLOAD_SIZE, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
            bulkTransferControlPoint(0x2222, (uint8_t *)config, 8, 8, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE)
    {
        static bool serviceAdded = false; /* We should only ever add one Immediate Alert service. */
        bufSize = bufferSize;
        requestData = callback;
        reqLength = 0;
        reqAddr = 0;
        remainBuf = 0;
        remainTrans = 0;
        transAddr = 0;
        dataSubscibed = false;

        if (serviceAdded)
        {
            return;
        }

        GattCharacteristic *charTable[] =
        { &bulkTransferData, &bulkTransferControlPoint };
        GattService bulkTransferService(0x1825, charTable,
                sizeof(charTable) / sizeof(GattCharacteristic *));
        ble.gattServer().addService(bulkTransferService);
        serviceAdded = true;

        ble.gattServer().onDataWritten(this, &BulkTransferService::onDataWritten);
        ble.gattServer().onUpdatesEnabled(FunctionPointerWithContext<uint16_t>(this, &BulkTransferService::onUpdatesEnabled));
        ble.gattServer().onUpdatesDisabled(FunctionPointerWithContext<uint16_t>(this, &BulkTransferService::onUpdatesDisables));
    }

    uint32_t getAddress()
    {
    	return config[0];
    }

    uint32_t getLength()
    {
    	return config[1];
    }

    void dataPrepared(int length)
    {
    	remainBuf = length;
    	if(dataSubscibed)
    	{
    		cursor = dataptr;
    		sendAPacket();
    	}
    }

protected:
    /**
     * This callback allows receiving updates to the AlertLevel characteristic.
     *
     * @param[in] params
     *     Information about the characteristic being updated.
     */
    virtual void onDataWritten(const GattWriteCallbackParams *params)
    {
    	int hdl = bulkTransferControlPoint.getValueHandle();
        if (params->handle == hdl)
        {
        	uint8_t *ptr = (uint8_t *)config;
        	for(int i = 0; i < 8; i++) ptr[i] = params->data[i];
        	reqAddr = config[0];
        	reqLength = config[1];
        	transAddr = reqAddr;
        	remainTrans = reqLength;
            if(remainTrans > 0)
            {
            	requestData(transAddr, remainTrans < bufSize ? remainTrans : bufSize);
            }
        }
    }

    virtual void onUpdatesEnabled(uint16_t handle)
    {
    	if(handle == bulkTransferData.getValueHandle())
    	{
    		dataSubscibed = true;
    		printf("Subscribed!\r\n");
    	}
    }

    virtual void onUpdatesDisables(uint16_t handle)
    {
    	if(handle == bulkTransferData.getValueHandle())
    	{
    		dataSubscibed = false;
    		printf("Unsubscribed!\r\n");
    	}
    }

    virtual int sendAPacket()
    {
    	int len = remainBuf < PAYLOAD_SIZE ? remainBuf : PAYLOAD_SIZE;

    	while(remainBuf > 0)
    	{
			ble.gattServer().write(bulkTransferData.getValueHandle(), cursor, len);
			cursor += len;
			remainBuf -= len;
			remainTrans -= len;
			transAddr += len;

			//printf("%d sent, rbuf=%d, rtrans=%d!\r\n", len, remainBuf, remainTrans);
    	}
    	if(remainTrans != 0)
		{
			requestData(transAddr, remainTrans < bufSize ? remainTrans : bufSize);
		}
    	return len;
    }

protected:
    BLE &ble;

    // Pointer to the data buffer
    uint8_t *dataptr;
    // Cursor to the data buffer
    uint8_t *cursor;
    // Length of the data buffer
    int bufSize;

    // Address of data(in flash) which are requested by the client
    int reqAddr;
    // Length of data(in flash) which are requested by the client
    int reqLength;
    //
    int transAddr;
    // Length of data(in buffer) which will be transmitted in a transaction
    volatile int remainBuf;
    // Length of data which will be transmitted in a request
    int remainTrans;

    bool dataSubscibed;

    // BLE characteristic storage
    uint32_t config[2]; //config[0]: address  config[1]: length
    GattCharacteristic bulkTransferData;
    GattCharacteristic bulkTransferControlPoint;
    RequestCallback    requestData;
};

#endif /* __BLE_BULK_TRANSFER_SERVICE_H__ */
