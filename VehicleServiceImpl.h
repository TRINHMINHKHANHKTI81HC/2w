#ifndef _VEHICLE_SERVICE_IMPL_H_
#define _VEHICLE_SERVICE_IMPL_H_

#include "json/json.h"

#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include "SDLTypes.h"
#include "Service.h"
#include "VehicleService.h"
#include "ConnectionManager.h"
#include "ConfigHandle.h"
#include "SocketCan.h"
#include "CanUtils.h"

namespace sdl {

class VehicleConfig : public utils::ConfigHandle {
 public:
    VehicleConfig()
        : mCanInterface()
        , mReadTimeoutMs(0) {}

    bool resultConfig() {
       bool isValid = true;
       mCanInterface = getString("can-interface", &isValid, Range(-1, INT_MAX), "VCAN0");
       mReadTimeoutMs = getInt("read-timeoutMs", &isValid, Range(0, 5), 3);
       return isValid;
    }

    std::string mCanInterface;
    int32_t mReadTimeoutMs;
};

class VehicleServiceImpl : public Service {
 public:

    VehicleServiceImpl();
    virtual ~VehicleServiceImpl() {}

    void registerCallbacks(VehicleServiceCallbacks* callback);
    void setConfigItem(const std::string& inKey, const std::string& inValue);
    void init(void);
    void shutdown(void);
    void handleMessage(const Json::Value& message) override;
    uint32_t id(void) const override;

    const Json::Value getVehicleData(const Json::Value& params);
    int subscribeVehicleData(const Json::Value& params);
    int unsubscribeVehicleData(const Json::Value& params);
    template <size_t N>
    void splitString(string (&arr)[N], string str);
    void loadData();

    // TODO: Change to member variable when we have configure file

    /*
    static utils::canConfigure mCanConfigSpeedData;
    static utils::canConfigure mCanConfigMCUDCVoltage;
    static utils::canConfigure mCanConfigSideStand;
    static utils::canConfigure mCanConfigBrake;
    static utils::canConfigure mCanConfigSpeedMode;
    static utils::canConfigure mCanConfigTotalOdometerMileage;
    static utils::canConfigure mCanConfigThrottlePercentage;
    static utils::canConfigure mCanConfigBrakeFault;
    static utils::canConfigure mCanConfigPositionSensorFault;
    static utils::canConfigure mCanConfigDriveMode;
    */

    static utils::canConfigure mCanConfigSOCDisplay;
private:
    VehicleServiceCallbacks* mCallback;
    uint32_t mId;
    VehicleConfig mConfig;
    SocketCan mSocketCan;

    bool isNotification(const Json::Value& message);
    bool isResponse(const Json::Value& message);
    void processNotification(const Json::Value& message);
    void processResponse(const Json::Value& message);
    void processRequest(const Json::Value& message);
    std::shared_ptr<Device> getDeviceById(const std::string& id);
    double getValueFromCan(utils::canConfigure inCanConfig);
    inline uint64_t convertCanFrameToUint64(uint8_t* data, uint8_t len) const {
        uint64_t result = 0;
        for (int i = 0; i < len; i++) {
            result <<= 8;
            result |= (uint64_t) data[i];
        }
        return result;
    }
};

}

#endif  // _VEHICLE_SERVICE_IMPL_H_
