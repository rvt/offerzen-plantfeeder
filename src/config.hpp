// How often we are updating the mqtt state in ms
#define MQTT_LASTWILL                           "lastwill"
#define MQTT_STATUS                             "status"
#define MQTT_NAME                             "plantfeeder"

constexpr uint8_t PUMP_PIN = CONFIG_PUMP_PIN;     //  OUT
constexpr uint8_t MOIST_PIN = CONFIG_MOIST_PIN;   // IN
constexpr uint8_t MOISTA_PIN = CONFIG_MOISTA_PIN; // Analog

#define CONTROLLER_CONFIG_FILENAME "/controllerCfg.conf"
#define CONFIG_FILENAME "/hwCfg.conf"
