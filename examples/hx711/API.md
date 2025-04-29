# iiot2k/hx711 API

C++ Library for load cell hx711 adc

### class c_hx711

```#include "c_hx711.h"```

#### Public Member Functions
 
```void power_down()```<br>
power down hx711
 
```bool init(c_chip *chip, uint32_t pin_dt, uint32_t pin_cl, bool print_msg=false)```<br>
inits dt + cl pins and hx711
 
```bool read(double &value, uint32_t gain=GAIN_A128, uint32_t nread=10)```<br>
read hx711