#include "mbed.h"
#include "ble/BLE.h"
#include "ble/services/HealthThermometerService.h"
#include "ble/services/ImmediateAlertService.h"
#include "ADT7420.h"

//BLE ble;
Serial pc(TX_PIN_NUMBER, RX_PIN_NUMBER);
ADT7420 tsensor(I2C_SDA0, I2C_SCL0, ADT7420::ADDRESS0, 100000);
Ticker alertTimer;
Ticker measureTimer;

DigitalOut greenLED(p21, 0);
DigitalOut redLED(p20, 0);

static bool shouldMeasure = true;
static const char *DEVICE_NAME = "BNU_Biosen";
static const uint16_t uuid16_list[] = {GattService::UUID_IMMEDIATE_ALERT_SERVICE,
                                       GattService::UUID_HEALTH_THERMOMETER_SERVICE};

HealthThermometerService *thermo;
ImmediateAlertService *alert;

void tickAlertHandler()
{
	redLED = !redLED;
}

void tickMeasureHandler()
{
	shouldMeasure = true;
}

void alertHandler(ImmediateAlertService::AlertLevel_t alertLevel)
{
    if(alertLevel == ImmediateAlertService::NO_ALERT)
    {
        alertTimer.detach();
        redLED = 0;
    }
    if(alertLevel == ImmediateAlertService::MILD_ALERT)
    {
        alertTimer.attach(tickAlertHandler, 0.5);
    }
    if(alertLevel == ImmediateAlertService::HIGH_ALERT)
    {
        alertTimer.attach(tickAlertHandler, 0.25);
    }
}

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *)
{
    BLE::Instance(BLE::DEFAULT_INSTANCE).gap().startAdvertising();
    greenLED = 0;
    alert->setAlertLevel(ImmediateAlertService::NO_ALERT);
    alertHandler(ImmediateAlertService::NO_ALERT);
}

void connectionCallback(const Gap::ConnectionCallbackParams_t *)
{
	greenLED = 1;
}

void writeCharCallback(const GattWriteCallbackParams *params)
{

}

void bleInitComplete(BLE::InitializationCompleteCallbackContext *params)
{
    BLE &ble          = params->ble;
    ble_error_t error = params->error;

    if (error != BLE_ERROR_NONE) {
        return;
    }

    ble.gap().onDisconnection(disconnectionCallback);
    ble.gap().onConnection(connectionCallback);
    //ble.gattServer().onDataWritten(writeCharCallback);

    /* Setup advertising */
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE); // BLE only, no classic BT
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED); // advertising type
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME)); // add name
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list)); // UUID's broadcast in advertising packet
    ble.gap().setAdvertisingInterval(100); // 100ms.

    // Temperature sensor initialization
    tsensor.setResolution(ADT7420::RES_16BIT);
    tsensor.setConvertMode(ADT7420::MODE_1SPS);
    measureTimer.attach(tickMeasureHandler, 1.0);

    /* Add our custom service */
    thermo = new HealthThermometerService(ble, (float)tsensor, HealthThermometerService::LOCATION_BODY);
    alert  = new ImmediateAlertService(ble, alertHandler);

    /* Start advertising */
    ble.gap().startAdvertising();
}

int main(void)
{
	pc.baud(115200);

	BLE& ble = BLE::Instance(BLE::DEFAULT_INSTANCE);
	ble.init(bleInitComplete);

    while (!ble.hasInitialized()) { /* spin loop */ }

    while (true) {
    	if(shouldMeasure)
    	{
    		float t = tsensor;
    		thermo->updateTemperature(t);
    		printf("%x\n\r", *((int *)(&t)));
    		shouldMeasure = false;
    	}
        ble.waitForEvent(); // allows or low power operation
    }
}
