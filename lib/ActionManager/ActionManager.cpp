#include "ActionManager.h"

ActionManager::ActionManager(SolenoidValve valves[], uint8_t valves_size, PumpMotor& pumpMotor)
    : _pumpMotor(pumpMotor)
{
    for (int i = 0; i < VALVES_MAX_SIZE && i < valves_size; i++)
    {
        _valves[i] = &valves[i];
        _valves_current_size++;
    }
}

ActionManager::~ActionManager()
{
    for (int i = 0; i < _valves_current_size; i++)
    {
        delete _valves[i];
    }
}

void ActionManager::handleValveMessage(char *message)
{
    StaticJsonBuffer<200> json_buffer;
    JsonObject &json_message = json_buffer.parseObject(message);

    if (json_message["identification"].is<int>())
    {
        int identification = json_message["identification"];
        int index = findValve(identification);

        if (index == -1)
            return;
        else if (json_message["state"] == "open")
            openValve(index);
        else if (json_message["state"] == "close")
            closeValve(index);
    }
    else if (json_message["identification"].is<char *>() && 
                json_message["identification"] == "*")
    {
        if (json_message["state"] == "open")
            openAllValves();
        else if (json_message["state"] == "close")
            closeAllValves();
    }
}

int ActionManager::findValve(int id)
{
    for (int i = 0; i < _valves_current_size; i++)
    {
        if (_valves[i]->_valve.getPin() == id)
        {
            return i;
        }
    }

    return -1;
}

void ActionManager::openValve(int index)
{
    if (index > -1 && index < _valves_current_size)
    {
        SolenoidValve target_valve = *_valves[index];
        target_valve._valve.abrir(target_valve._callback);
    }
}

void ActionManager::closeValve(int index)
{
    if (index > -1 && index < _valves_current_size)
    {
        SolenoidValve target_valve = *_valves[index];
        target_valve._valve.cerrar(target_valve._callback);
    }
}

void ActionManager::openAllValves()
{
    for (int i = 0; i < _valves_current_size; i++)
    {
        openValve(i);
    }
}

void ActionManager::closeAllValves()
{
    for (int i = 0; i < _valves_current_size; i++)
    {
        closeValve(i);
    }
}

void ActionManager::handlePumpMotorMessage(char *message) {
    StaticJsonBuffer<200> json_buffer;
    JsonObject &json_message = json_buffer.parseObject(message);

    if (json_message["identification"].is<int>())
    {
        int identification = json_message["identification"];
        
        if (json_message["state"] == "activate")
            activatePumpMotor();
        else if (json_message["state"] == "deactivate")
            deactivatePumpMotor();
    }
}