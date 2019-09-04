#include "ActionManager.h"

ActionManager::ActionManager(SolenoidValve valves[], uint8_t valves_size)
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
        int index = find(identification);

        if (index == -1)
            return;
        else if (json_message["state"] == "open")
            open(index);
        else if (json_message["state"] == "close")
            close(index);
    }
    else if (json_message["identification"].is<char *>() && 
                json_message["identification"] == "*")
    {
        if (json_message["state"] == "open")
            openAll();
        else if (json_message["state"] == "close")
            closeAll();
    }
}

int ActionManager::find(int id)
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

void ActionManager::open(int index)
{
    if (index > -1 && index < _valves_current_size)
    {
        SolenoidValve target_valve = *_valves[index];
        target_valve._valve.abrir(target_valve._callback);
    }
}

void ActionManager::close(int index)
{
    if (index > -1 && index < _valves_current_size)
    {
        SolenoidValve target_valve = *_valves[index];
        target_valve._valve.cerrar(target_valve._callback);
    }
}

void ActionManager::openAll()
{
    for (int i = 0; i < _valves_current_size; i++)
    {
        open(i);
    }
}

void ActionManager::closeAll()
{
    for (int i = 0; i < _valves_current_size; i++)
    {
        close(i);
    }
}