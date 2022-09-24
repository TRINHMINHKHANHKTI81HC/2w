#include "json/json.h"
#include "json_rc_constants.h"
#include "jsoncpp_reader_wrapper.h"
#include "VehicleServiceImpl.h"
#include "CoreTypes.h"
#include "Log.h"
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <cstdint>
#include <iterator>
#include <sstream>
#include "CanUtils.h"




using namespace std;

namespace sdl {
    
    template <size_t N>

    void VehicleServiceImpl::splitString(string (&arr)[N], string str)
    {   
    unsigned int n = 0;
    istringstream iss(str);
    for (auto it = istream_iterator<string>(iss); it != istream_iterator <string>() && n < N; ++it, ++n){
        arr[n] = *it;
        }
    }
    
    void VehicleServiceImpl::loadData(){
    ifstream myfile("CAN_config.txt");
    string line;
    static int number_of_lines;
    while (getline (myfile, line))
    {
        string splitdata[8];
        string split = "" + line;
        splitString(splitdata,split);
        //Explicit type conversion

        unsigned int canidconvert = stoi(splitdata[0]);
        unsigned int canlengthconvert = stoi(splitdata[1]);
        unsigned int startbitconvert = stoi(splitdata[3]);
        unsigned int lengthconvert = stoi(splitdata[4]);
        double scaleconvert = stod(splitdata[5]);
        double offsetconvert = stod(splitdata[6]);
        bool endian = false;

        //Check the split data
        /*cout << canidconvert << " " << canlengthconvert << " " << splitdata[2] << " " << startbitconvert << " " << lengthconvert << " " <<
        offsetconvert << " " << splitdata[7] << endl;
         canlengthconvert;
        */

        
        utils::canconfigure[number_of_lines].canId = canidconvert;
        utils::canconfigure[number_of_lines].canLength = canlengthconvert;
        utils::canconfigure[number_of_lines].name_variables = splitdata[2];
        utils::canconfigure[number_of_lines].startBit = startbitconvert;
        utils::canconfigure[number_of_lines].length = lengthconvert;
        utils::canconfigure[number_of_lines].scale = scaleconvert;
        utils::canconfigure[number_of_lines].offset = offsetconvert;
        utils::canconfigure[number_of_lines].isBigendian = endian;

        /*
        utils::canConfigure canconfigure[number_of_lines].canId = canidconvert;
        utils::canConfigure canconfigure[number_of_lines].canLength = canlengthconvert;
        utils::canConfigure canconfigure[number_of_lines].name_variables = splitdata[2];
        utils::canConfigure canconfigure[number_of_lines].startBit = startbitconvert;
        utils::canConfigure canconfigure[number_of_lines].length = lengthconvert;
        utils::canConfigure canconfigure[number_of_lines].scale = scaleconvert;
        utils::canConfigure canconfigure[number_of_lines].offset = offsetconvert;
        utils::canConfigure canconfigure[number_of_lines].isBigendian = endian;
        */


        /*
        cout << utils::canconfigure[number_of_lines].canId << " ";
        cout << utils::canconfigure[number_of_lines].canLength << " ";
        cout << utils::canconfigure[number_of_lines].name_variables << " ";
        cout << utils::canconfigure[number_of_lines].startBit << " ";
        cout << utils::canconfigure[number_of_lines].length << " ";
        cout << utils::canconfigure[number_of_lines].scale << " ";
        cout << utils::canconfigure[number_of_lines].offset << " ";
        cout << utils::canconfigure[number_of_lines].isBigendian << endl;
        */

        number_of_lines++;
    }
    }
    
    /*
    utils::canConfigure VehicleServiceImpl::mCanConfigSpeedData = {0x610, 8, 0, 3, 1, 0, false}; // SpeedModeStatus
    utils::canConfigure VehicleServiceImpl::mCanConfigBrakeStatus = {0x610, 8, 11, 1, 1, 0, false}; // BrakeStatusStatus
    utils::canConfigure VehicleServiceImpl::mCanConfigBrakeFaultStatus = {0x610, 8, 27, 1, 1, 0, false}; // BrakeFaultStatusStatus
    utils::canConfigure VehicleServiceImpl::mCanConfigPositionSensorFault = {0x610, 8, 33, 1, 1, 0, false}; // PositionSensorFault
    utils::canConfigure VehicleServiceImpl::mCanConfigDriveModeStatus = {0x610, 8, 48, 3, 1, 0, false}; // DriveModeStatusStatus
    utils::canConfigure VehicleServiceImpl::mCanConfigSpeedData = {0x611, 7, 0, 16, 10, 0, false}; //SpeedData
    utils::canConfigure VehicleServiceImpl::mCanConfigMCUDCVoltage = {0x611, 7, 16, 16, 0.0078125, 0, false}; //MCUDCVoltage
    utils::canConfigure VehicleServiceImpl::mCanConfigTotalOdometerMileage = {0x612, 6, 16, 32, 0.1, 0, false}; //TotalOdometerMileage
    utils::canConfigure VehicleServiceImpl::mCanConfigThrottlePercentage = {0x631, 8, 16, 16, 1, 0, false}; //ThrottlePercentage
    utils::canConfigure VehicleServiceImpl::mCanConfigSide_Stand_status = {0x633, 8, 48, 1, 1, 0, false}; //Side_Stand_Status
    */

    //utils::canConfigure VehicleServiceImpl::mCanConfigSOCDisplay = {0x633, 8, 32, 8, 1, 0, false}; //SOC_Display

VehicleServiceImpl::VehicleServiceImpl()
    : mCallback(nullptr)
    , mId(VEHICLE_SERVICE_ID)
    , mSocketCan() {}

void VehicleServiceImpl::registerCallbacks(VehicleServiceCallbacks* callback) {
    mCallback = callback;
}

void VehicleServiceImpl::setConfigItem(const std::string& inKey, const std::string& inValue) {
    mConfig.set(inKey, inValue);
}

void VehicleServiceImpl::init() {
    ConnectionManager::instance().registerService(this);
    // Load configurations
    if (mConfig.resultConfig()) {
        (void) mSocketCan.startSocketCan(mConfig.mCanInterface, mConfig.mReadTimeoutMs);
    }
}

void VehicleServiceImpl::shutdown() {
    (void) mSocketCan.stopSocketCan();
    if (ConnectionManager::instance().running()) {
        ConnectionManager::instance().shutdown();
    }
}

void VehicleServiceImpl::handleMessage(const Json::Value& message) {
    if (isNotification(message)) {
        processNotification(message);
    } else if (isResponse(message)) {
        processResponse(message);
    } else {
        processRequest(message);
    }
}

uint32_t VehicleServiceImpl::id() const {
    return mId;
}

bool VehicleServiceImpl::isNotification(const Json::Value& message) {
    bool ret = false;
    if (false == message.isMember("id")) {
        ret = true;
    }
    return ret;
}

bool VehicleServiceImpl::isResponse(const Json::Value& message) {
    bool ret = false;
    if ((true == message.isMember("result")) || (true == message.isMember("error"))) {
        ret = true;
    }
    return ret;
}

void VehicleServiceImpl::processNotification(const Json::Value& message) {
    Json::StreamWriterBuilder builder;
    const std::string str_msg = Json::writeString(builder, message) + '\n';
    std::string method = message[json_keys::kMethod].asString();
    LOGD("VehicleServiceImpl::%s() \n%s", __func__, str_msg.c_str());
}

void VehicleServiceImpl::processResponse(const Json::Value& message) {
    //Handled in BasicCummunication.cc
}

void VehicleServiceImpl::processRequest(const Json::Value& message) {
    Json::StreamWriterBuilder builder;
    const std::string str_msg = Json::writeString(builder, message) + '\n';
    std::string id = message[json_keys::kId].asString();
    std::string method = message[json_keys::kMethod].asString();
    LOGD("VehicleServiceImpl::%s()\n%s", __func__, str_msg.c_str());

    Json::Value response;
    response[json_keys::kId] = message[json_keys::kId].asInt();
    response[json_keys::kJsonrpc] = "2.0";
    response[json_keys::kResult][json_keys::kMethod] = method;
    response[json_keys::kResult]["code"] = 0;
    bool isReply = true;

    if (method == "Vehicle.IsReady") {
        response[json_keys::kResult]["available"] = true;
    }

    if (isReply) ConnectionManager::instance().sendJsonMessage(response);
}

double VehicleServiceImpl::getValueFromCan(utils::canConfigure inCanConfig) {
    CanFrame frame;
    double value = 0;
    if (0 != mSocketCan.getCanFrame(inCanConfig.canId, frame)) {
        uint64_t inFrame = convertCanFrameToUint64(frame.data, frame.len);
        if (0 == utils::getCanValue(inCanConfig, inFrame, value)) {
            LOGE("VehicleServiceImpl::%s() cannot get can value, not yet support\n", __func__);
            value = 0;
        }
    } else {
        LOGE("VehicleServiceImpl::%s() cannot get can frame id 0x%x from socket\n", __func__, inCanConfig.canId);
    }
    return value;
}




// TODO: Change to member variable when we have configure file

const Json::Value VehicleServiceImpl::getVehicleData(const Json::Value& root) {
    Json::Value response;
    response[json_keys::kJsonrpc] = "2.0";
    response[json_keys::kResult]["code"] = 0;

    if (root.isMember("params")) {
        const Json::Value& params = root["params"];
        CanFrame frame;
        // get SpeedData - Tuong duong line 10 va line 5 trong CAN_config1.txt
        
        if (params.isMember("DriveModeStatus") && params["DriveModeStatus"].asBool()) { //[-327680|327670]
            if (getValueFromCan(utils::canconfigure[9]) == 0){//normal
                if (getValueFromCan(utils::canconfigure[4]) >= 327670){
                    response[json_keys::kResult]["SpeedData"] = 327670;          
                }else {
                    response[json_keys::kResult]["SpeedData"] = getValueFromCan(utils::canconfigure[4]);          
                }
            }else { //reverse
                if (getValueFromCan(utils::canconfigure[4]) >= 327680){
                    response[json_keys::kResult]["SpeedData"] = -327680;                              
                }else {
                    response[json_keys::kResult]["SpeedData"] = - getValueFromCan(utils::canconfigure[4]);          
                }
            }
        }
         // get MCUDCVoltage - Tuong duong line 4 trong CAN_config1.txt
        if (params.isMember("MCUDCVoltage") && params["MCUDCVoltage"].asBool()) { //[0|511.992]
            response[json_keys::kResult]["MCUDCVoltage"] = getValueFromCan(utils::canconfigure[3]);
        }
        //get Side_Stand_status - Tuond duong line 1 trong CAN_config1.txt
        if (params.isMember("Side_Stand_status") && params["Side_Stand_status"].asBool()) { //[0|1]
            if (getValueFromCan(utils::canconfigure[0]) == 0){
                response[json_keys::kResult]["Side_Stand_status"] = "Side stand up";
            } else {
                response[json_keys::kResult]["Side_Stand_status"] = "Side stand down";
            }
        }
        
        
        /* //get Soc display
        if (params.isMember("SOCDisplay") && params["SOCDisplay"].asBool()){
            response[json_keys::kResult]["SOCDisplay"] = getValueFromCan(mCanConfigSOCDisplay);
        }
        */
        
        
        
        // get BrakeStatus status - Tuong duong line 7 trong CAN_config1.txt 
        if (params.isMember("BrakeStatus") && params["BrakeStatus"].asBool()) { //[0|1]
            if (getValueFromCan(utils::canconfigure[6]) == 0){
                response[json_keys::kResult]["BrakeStatus"] = "Inactive";
            } else {
                response[json_keys::kResult]["BrakeStatus"] = "Active";
            }
        }
         // git SpeedModeStatus  - Tuong duong line 9 trong CAN_config1.txt 
        if (params.isMember("SpeedModeStatus") && params["SpeedModeStatus"].asBool()) { //[0|7]
            if (getValueFromCan(utils::canconfigure[8]) == 0){
                response[json_keys::kResult]["SpeedModeStatus"] = "Neutral";
            } else if (getValueFromCan(utils::canconfigure[8]) == 1){
                response[json_keys::kResult]["SpeedModeStatus"] = "Mode 1(low speed mode) status";
            } else if (getValueFromCan(utils::canconfigure[8]) == 2){
                response[json_keys::kResult]["SpeedModeStatus"] = "Mode 2(middle speed mode) status"; 
            } else if (getValueFromCan(utils::canconfigure[8]) == 3){
                response[json_keys::kResult]["SpeedModeStatus"] = "Mode 3(high speed mode) status";
            } else if (getValueFromCan(utils::canconfigure[8]) == 4){
                response[json_keys::kResult]["SpeedModeStatus"] = "Invalid";
            }
        }
         // get TotalOdometerMileage - Tuong duong line 2 trong CAN_config1.txt
        if (params.isMember("TotalOdometerMileage") && params["TotalOdometerMileage"].asBool()) { //[0|429497000]
            response[json_keys::kResult]["TotalOdometerMileage"] = getValueFromCan(utils::canconfigure[1]);
        } 
        // get ThrottlePercentage - Tuong duong line 3 trong CAN_config1.txt
        if (params.isMember("ThrottlePercentage") && params["ThrottlePercentage"].asBool()) { //[0|100]
            if (getValueFromCan(utils::canconfigure[2]) >= 100){
                response[json_keys::kResult]["ThrottlePercentage"] = 100;
            } else {
                    response[json_keys::kResult]["ThrottlePercentage"] = getValueFromCan(utils::canconfigure[2]);
            }
        }
         // get BrakeFaultStatus - Tuong duong line 6 trong CAN_config1.txt
        if (params.isMember("BrakeFaultStatus") && params["BrakeFaultStatus"].asBool()) { //[0|1]
            if (getValueFromCan(utils::canconfigure[5]) == 0){
                response[json_keys::kResult]["BrakeFaultStatus"] = "No Fault";
            } else {
                response[json_keys::kResult]["BrakeFaultStatus"] = "Fault";
            }
        }
         // get PositionSensorFault - Tuong duong line 8 trong CAN_config1.txt
        if (params.isMember("PositionSensorFault") && params["PositionSensorFault"].asBool()) { //[0|1]
            if (getValueFromCan(utils::canconfigure[7]) == 0){
                response[json_keys::kResult]["PositionSensorFault"] = "No Fault";
            } else {
                response[json_keys::kResult]["PositionSensorFault"] = "Fault";
            }
        }
        
       // get DriveModeStatus - Tuong duong line 10 trong CAN_config1.txt
        if (params.isMember("DriveModeStatus") && params["DriveModeStatus"].asBool()) { //[0|7]
            if (getValueFromCan(utils::canconfigure[9]) == 0){
                response[json_keys::kResult]["DriveModeStatus"] = "NormalMode Status";
            }else {
                response[json_keys::kResult]["DriveModeStatus"] = "Reverse Mode status";               
            }
        }  
    } else {
        response[json_keys::kResult]["code"] = -1;
        response[json_keys::kResult]["message"] = "Request has no param";
    }
    return response;
}



int VehicleServiceImpl::subscribeVehicleData(const Json::Value& params) {
    // TODO: do nothing
    return 0;
}

int VehicleServiceImpl::unsubscribeVehicleData(const Json::Value& params) {
    // TODO: do nothing
    return 0;
}

}
