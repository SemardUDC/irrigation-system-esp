#include "SensorManager.h"

SensorManager::SensorManager(FlowMeter fmeters[], int flowsize, 
                                UltrasonicPCF8574 ultrasonics[], int ultrsize,
                                pH4502c phmeters[], int phsize)
    : _fmeters_current_size(0), _ultrasonics_current_size(0)
{
    for (int i = 0; i < FLOW_METER_MAX_SIZE && i < flowsize; i++)
    {
        _fmeters[i] = &fmeters[i];
        _fmeters_current_size++;
    }

    for (int i = 0; i < ULTRASONIC_MAX_SIZE && i < ultrsize; i++)
    {
        _ultrasonics[i] = &ultrasonics[i];
        _ultrasonics_current_size++;
    }

    for (int i = 0; i < PH_METER_MAX_SIZE && i < phsize; i++)
    {
        _phmeters[i] = &phmeters[i];
        _phmeters_current_size++;
    }
}

SensorManager::~SensorManager()
{
    for (int i = 0; i < _fmeters_current_size; i++)
    {
        delete _fmeters[i];
    }

    for (int i = 0; i < _ultrasonics_current_size; i++)
    {
        delete _ultrasonics[i];
    }

    for (int i = 0; i < _phmeters_current_size; i++)
    {
        delete _phmeters[i];
    }
}

void SensorManager::begin()
{
    for (int i = 0; i < _fmeters_current_size; i++)
    {
        _fmeters[i]->begin();
    }
}

void SensorManager::handle()
{
    for (int i = 0; i < _fmeters_current_size; i++)
    {
        _fmeters[i]->handle();
    }

    static int seconds = 0;

    if (millis() - seconds > 10000)
    {
        seconds = millis();
        for (int i = 0; i < _ultrasonics_current_size; i++)
        {
            _ultrasonics[i]->handle();
        }
        for (int i = 0; i < _phmeters_current_size; i++)
        {
            _phmeters[i]->lecturaAnalogica();
        }
    }
}