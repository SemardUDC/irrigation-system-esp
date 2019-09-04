#include "ActionManager.h"


ActionManager::ActionManager(SolenoidValve valves[], uint8_t valves_size, PumpMotor motors[], uint8_t motors_size)
{
    for (int i = 0; i < VALVES_MAX_SIZE && i < valves_size; i++)
    {
        _valves[i] = &valves[i];
        _valves_current_size++;
    }

    for (int i = 0; i < MOTORS_MAX_SIZE && i < motors_size; i++)
    {
        Serial.println("Motor agregado");
        _motors[i] = &motors[i];
        _motors_current_size++;
    }
}

ActionManager::~ActionManager()
{
    for (int i = 0; i < _valves_current_size; i++)
    {
        delete _valves[i];
    }

    for (int i = 0; i < _motors_current_size; i++)
    {
        delete _motors[i];
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
        else if (json_message["action"] == "open")
            openValve(index);
        else if (json_message["action"] == "close")
            closeValve(index);
    }
    else if (json_message["identification"].is<char *>() &&
             json_message["identification"] == "*")
    {
        if (json_message["action"] == "open")
            openAllValves();
        else if (json_message["action"] == "close")
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

void ActionManager::handlePumpMotorMessage(char *message)
{
    StaticJsonBuffer<200> json_buffer;
    JsonObject &json_message = json_buffer.parseObject(message);

    if (json_message["identification"].is<int>())
    {
        int identification = json_message["identification"];
        int index = findMotor(identification);

        Serial.println("Indice para");
        Serial.println(identification);
        Serial.println("es");
        Serial.println(index);


        if (index == -1)
            return;
        else if (json_message["action"] == "activate")
            activateMotor(index);
        else if (json_message["action"] == "deactivate")
            deactivateMotor(index);
    }
    else if (json_message["identification"].is<char *>() &&
             json_message["identification"] == "*")
    {
        if (json_message["action"] == "activate")
            activateAllMotors();
        else if (json_message["action"] == "deactivate")
            deactivateAllMotors();
    }
}

int ActionManager::findMotor(int id)
{
    for (int i = 0; i < _motors_current_size; i++)
    {
        if (_motors[i]->getPin() == id)
            return i;
    }
    return -1;
}

void ActionManager::activateAllMotors()
{
    for (int i = 0; i < _motors_current_size; i++)
    {
        activateMotor(i);
    }
}

void ActionManager::deactivateAllMotors()
{
    for (int i = 0; i < _motors_current_size; i++)
    {
        deactivateMotor(i);
    }
}

void ActionManager::activateMotor(int index)
{
    if (index > -1 && index < _motors_current_size)
        _motors[index]->activate();
}

void ActionManager::deactivateMotor(int index)
{
    if (index > -1 && index < _motors_current_size)
        _motors[index]->deactivate();
}
