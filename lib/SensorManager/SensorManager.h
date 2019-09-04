#ifndef SensorManager_h
#define SensorManager_h

#include <UltrasonicPCF8574.h>
#include <FlowMeter.h>

class SensorManager
{

  public:
    static const int FLOW_METER_MAX_SIZE = 2;
    static const int ULTRASONIC_MAX_SIZE = 2;
    SensorManager(FlowMeter fmeters[], int flowsize, UltrasonicPCF8574 ultrasonics[], int ultrsize);
    ~SensorManager();

    void begin();
    void handle();

  private:
    FlowMeter *_fmeters[FLOW_METER_MAX_SIZE];
    UltrasonicPCF8574 *_ultrasonics[ULTRASONIC_MAX_SIZE];

    uint8_t _fmeter_current_size;
    uint8_t _ultrasonics_current_size;
};

#endif