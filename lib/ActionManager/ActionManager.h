#ifndef ActionManager_h
#define ActionManager_h

#include <Electrovalvula.h>
#include <ArduinoJson.h>

// Struct that maps an Electrovalvula object with a callback function.
// Since Electrovalvula doesn't store a callback as a field member, 
// the easier way to handle callback invokation is with a SolenoidValve struct.
struct SolenoidValve
{
  Electrovalvula &_valve;
  void (&_callback)(uint8_t state);

  SolenoidValve(Electrovalvula &valve, void (&callback)(uint8_t state)) : 
    _valve(valve), _callback(callback) {}
};

class ActionManager
{
public:
  const static uint8_t VALVES_MAX_SIZE = 6;

  ActionManager(SolenoidValve valves[], uint8_t valves_size);
  ~ActionManager();
  void handleValveMessage(char *message);

private:
  SolenoidValve *_valves[VALVES_MAX_SIZE];
  uint8_t _valves_current_size;

  int find(int id);
  void open(int index);
  void close(int index);
  void openAll();
  void closeAll();
};

#endif