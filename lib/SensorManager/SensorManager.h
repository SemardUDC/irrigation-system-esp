#ifndef SensorManager_h
#define SensorManager_h

#include <UltrasonicPCF8574.h>
#include <FlowMeter.h>
#include <pH4502c.h>
#include <Press.h>

class SensorManager
{

  public:
    static const int FLOW_METER_MAX_SIZE = 2;
    static const int ULTRASONIC_MAX_SIZE = 2;
    static const int PH_METER_MAX_SIZE = 1;
    static const int PRESS_SENSOR_MAX_SIZE = 2;

    SensorManager(FlowMeter fmeters[], int flowsize,
                  UltrasonicPCF8574 ultrasonics[], int ultrsize,
                  pH4502c phmeters[], int phsize,
                  Press press_sensors[], int presssize);
    ~SensorManager();

    void begin();
    void handle();

  private:
    FlowMeter *_fmeters[FLOW_METER_MAX_SIZE];
    UltrasonicPCF8574 *_ultrasonics[ULTRASONIC_MAX_SIZE];
    pH4502c *_phmeters[PH_METER_MAX_SIZE];
    Press *_press_sensors[PRESS_SENSOR_MAX_SIZE];

    uint8_t _fmeters_current_size;
    uint8_t _ultrasonics_current_size;
    uint8_t _phmeters_current_size;
    uint8_t _press_sensors_current_size;
};

#endif