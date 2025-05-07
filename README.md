# iiot2k/libgpiox

C++ GPIO Library for Raspberry Pi

The **libgpiox** library uses the Linux GPIO character device interface (V2).<br>
That's why it works on all Raspberry Pi models and most Linux operating systems.<br>
The library consists of only one header file **gpiox.h**, which makes it very easy to use.<br>
Therefore the **libgpiox** library does not need to be built.<br>
No installation of other libraries necessary.<br>

```c++
#include "../include/gpiox.h"
```

Some examples are in the **examples** folder.

> Build examples:

```
cd examples
make
```
> Execute example:

```
./blink
```

> Extended examples:<br>

```/examples/ds18b20``` : 1-wire temperature sensor **ds18b20** library using **gpiox**.<br>
```/examples/hx711``` : Load cell adc **hx711** library using **gpiox**.<br>

All class functions are described in document **API.md**.

### Constants

|Mode-Constant|Function|
|:--|:--|
|GPIO_MODE_INPUT|floating input|
|GPIO_MODE_INPUT_PULLDOWN|input with pull-down resistor, 1: on connect to +3.3V|
|GPIO_MODE_INPUT_PULLUP|input with pull-up resistor, 1: on connect to ground|
|GPIO_MODE_OUTPUT|output, 0: connect to ground, 1: +3.3V|
|GPIO_MODE_OUTPUT_SOURCE|output source, 0: Hi-Z, 1: +3.3V|
|GPIO_MODE_OUTPUT_SINK|output sink, , 0: Hi-Z, 1: connect to ground|

>All modes are active high.

|Edge-Constant|Function|
|:--|:--|
|GPIO_EDGE_RISING|rising edge|
|GPIO_EDGE_FALLING|falling edge|
|GPIO_EDGE_BOTH|rising + falling edge|
|GPIO_EDGE_NONE|no edge|

### class c_chip
The class **c_chip** is instantiated only once and passed to the class **c_gpio**.<br>

```c++
c_chip chip;
```
### class c_gpio

The class c_gpio is instantiated for each gpio pin.<br>

```c++
#define PRINT_MSG true // turn print error message on

c_gpio gpio1(&chip, PRINT_MSG);
c_gpio gpio2(&chip, PRINT_MSG);
```

The created instance is initialized with the **init** function.<br>

```c++
#define INPUT_PIN 21 // input gpio pin number
#define OUTPUT_PIN 20 // output gpio pin number
#define DEBOUNCE_US 10000 // debounce in us

// init input
if (!gpio1.init(INPUT_PIN, GPIO_MODE_INPUT_PULLDOWN, DEBOUNCE_US))
    return false;

// init output
if (!gpio2.init(OUTPUT_PIN, GPIO_MODE_OUTPUT))
    return 1;
```

After initialization, input/output functions can be called.<br>
The input/output functions are thread-safe.<br>

```c++
// read input
int32_t input_val = gpio1.read();

if (input_val == -1)
    return false;

// read output inverted
int32_t output_val = gpio2.read(true);

if (output_val == -1)
    return false;

// write output
if (!gpio2.write(1))
    return false;

// write output inverted
if (!gpio2.write(1, true))
    return false;

// toggle output
if (!gpio2.toggle())
    return false;

```

Monitoring the change of the input pin is possible with the **watch** function.<br>

```c++
// init input with edge
if (!gpio1.init(INPUT_PIN, GPIO_MODE_INPUT_PULLUP, DEBOUNCE_US, GPIO_EDGE_BOTH))
    return false;

uint32_t edge;

// watch changes
if (!gpio1.watch(edge))
    return false;

// print edge
if (edge == GPIO_EDGE_RISING)
    puts("rising edge occurs");
else if (edge == GPIO_EDGE_FALLING)
    puts("falling edge occurs");
```

### class c_worker 
The **c_worker** class is a simple thread wrapper implementation.<br>

```c++
#include "../include/c_worker.h"

// derived worker class
class c_myworker : public c_worker
{
public:
    c_myworker(c_gpio* gpio)
    {
        m_gpio = gpio;
    }

    ~c_myworker() {}

    // execute thread 
    void Execute() override
    {
        m_gpio->write(1);
    }
private: 
    c_gpio* m_gpio;
}

// create worker thread
c_myworker* wk = new c_myworker(&gpio2);

// start worker thread and wait until ends
wk->Queue(true);

```
### class c_timer
The **c_timer** class is for timer delay and sleep.<br>

```c++
#include "../include/c_timer.h"

bool pulse()
{
    // create timer
    c_timer timer;
    
    // set output
    if (!gpio2.write(1))
        return false;
    
    // sleep for 1ms
    timer.sleep_ms(1);

    // reset output
    return gpio2.write(0);
}
```

### class c_priority
The **c_priority** class is for set high priority on time critical I/O operation.<br>

```c++
#include "../include/c_priority.h"

bool critical_io()
{
    // switch priority
    c_priority priority;

    // toggle output
    return gpio2.toggle();
}
```

