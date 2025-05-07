# iiot2k/ds18b20 API

C++ Library for ds18b20 temperature sensor

### class c_ds18b20

```#include "c_18b20.h"```

#### Public Member Functions

```c_ds18b20()```<br>
class constuctor

```const char* get_error()```<br>
returns error message

```bool idtostr(sensor_id &id, char *s_id, size_t len)```<br>
convert 64bit id to string id

```bool idtostr(sensor_id& id, string& s_id)```<br>
convert 64bit id to string id
 
```bool strtoid(const char *s_id, sensor_id &id)```<br>
convert string id to 64bit id

```sensor_id strtoid(const char* s_id)```<br>
convert string id to 64bit id
 
```bool init(c_chip *chip, uint32_t pin, bool print_msg=false)```<br>
inits sensor gpio pin
 
```bool set_resolution(uint32_t res)```<br>
set resolution on all sensors on bus

```bool scan_sensor(vector<sensor_id>& idlist, uint8_t repeat = 10)```<br>
scan for sensors

```bool read_sensor(const char *s_id, bool fh, double &temp, uint8_t repeat = 10)```<br>
read one sensors with given id

```bool read_sensors(vector< sensor_id > &idlist, bool fh, vector< double > &templist, uint8_t repeat = 10)```<br>
read sensors from id list