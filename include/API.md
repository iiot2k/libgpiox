# iiot2k/libgpiox API

C++ GPIO Library for Raspberry Pi

### class c_chip

The class **c_chip** is instantiated only once and passed to the class c_gpio.<br>

```#include "gpiox.h"```

#### Public Member Functions

```c_chip()```<br>
class constuctor

```~c_chip()```<br>
class destructor

```int32_t get_fd()```<br>
returns chip handle

### class c_gpio

The class **c_gpio** is instantiated for each gpio pin.<br>

```#include "gpiox.h"```

#### Public Member Functions

```c_gpio()```<br>
class constuctor
 
```c_gpio(c_chip *chip, bool print_msg=false)```<br>
class constuctor
 
```void setchip(c_chip *chip, bool print_msg=false)```<br>
set chip and message flag
 
```~c_gpio()```<br>
class destructor
 
```void deinit()```<br>
deinits gpio pin
 
```bool print_err(const char *msg=NULL)```<br>
prints error message if enabled on stderr
 
```int32_t get_pin()```<br>
returns gpio pin number
 
```bool init(uint32_t pin, uint32_t mode, uint32_t setval=0, uint32_t edge=GPIO_EDGE_NONE)```<br>
inits gpio pin
 
```int32_t read(bool invert=false)```<br>
reads gpio
 
```bool write(int32_t val, bool invert=false)```<br>
sets pin state
 
```bool toggle(bool invert=false)```<br>
toggle output
 
```bool watch(uint32_t &edge)```<br>
watch gpio for changes

### class c_worker

The **c_worker** class is a simple thread wrapper implementation.<br>

```#include "c_worker.h"```

#### Public Member Functions

```virtual ~c_worker()```<br>
virtual class destructor
 
```virtual void Execute()```<br>
called once in thread
 
```void Queue(bool wait=false)```<br>
start thread

### class c_priority

The **c_priority** class is for set high priority on time critical I/O operation.<br>

```#include "c_priority.h"```

#### Public Member Functions

```c_priority(bool manual=false)```<br>
switch priority
 
```~c_priority()```<br>
restore previous priority on destroy
 
```void set()```<br>
switch priority manual
 
```void restore()```<br>
restore priority manual

### class c_timer

The **c_timer** class is for timer delay and sleep.<br>

```#include "c_timer.h"```

#### Public Member Functions

```void delay(int64_t sec, int64_t nsec)```<br>
delays current thread
 
```void delay_ns(int64_t nsec)```<br>
 
```void delay_us(int64_t usec)```<br>
 
```void delay_ms(int64_t msec)```<br>
 
```void delay_s(int64_t sec)```<br>
 
```void sleep(int64_t sec, int64_t nsec)```<br>
sleeps current thread
 
```void sleep_ns(int64_t nsec)```<br>
 
```void sleep_us(int64_t usec)```<br>
 
```void sleep_ms(int64_t msec)```<br>
 
```void sleep_s(int64_t sec)```<br>

