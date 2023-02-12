/*
 * 
 * Version History
 * 1.0
 * - first version
 * 2.0
 * - Adding a dashboard with a ESP-DASH component (not used any more)
 * 2.1
 * - Adding a http://ip/status URL to get all data in JSON format
 * - Adding a simple webpage to show current values
 */

char msg[50];
String mqttStat = "";
String message = "";
unsigned long lastTick, uptime, seconds, lastWifiCheck, lastRGB, lastmodbus;
int readcycle = 0;
bool ledoff = false;
char newclientid[80];
char buildversion[12]="v2.1";
int overflow;
char json[1024];
char modbusstatus[50];
char systemstatus[300];
bool validdata;

struct modbus_input_registers 
{
    float voltage,current,active_power,power_factor,frequency,total_active_energy;
};

struct modbus_input_registers modbusdata;
