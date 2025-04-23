# iiot2k/libgpiox

C++ GPIO Library for Raspberry Pi

The **libgpiox** library uses the Linux GPIO character device interface (V2).<br>
The library consists of only one header file **gpiox.h**, which makes it very easy to use.<br>
Therefore the **libgpiox** library does not need to be built.<br>

```c++
#include "../include/gpiox.h"
```

Some examples are in the **examples** folder.

### Constants

|Mode-Constant|Function|
|:--|:--|
|GPIO_MODE_INPUT_NOPULL|floating input|
|GPIO_MODE_INPUT_PULLDOWN|input with pull-down resistor, 1 on connect to +3.3V|
|GPIO_MODE_INPUT_PULLUP|input with pull-up resistor, 1 on connect to ground|
|GPIO_MODE_OUTPUT|output|
|GPIO_MODE_OUTPUT_SOURCE|output source, Hi-Z on 0, +3.3V on 1|
|GPIO_MODE_OUTPUT_SINK|output sink, Hi-Z on 0, connect to ground on 1|

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
#define INPUT_PIN 21 // gpio pin number
#define OUTPUT_PIN 21 // gpio pin number
#define DEBOUNCE_US 10000 // debounce in us

// init input
if (!gpio1.init(INPUT_PIN, GPIO_MODE_INPUT_PULLUP, DEBOUNCE_US))
    return false;

// init output
if (!gpio2.init(OUTPUT_PIN, GPIO_MODE_OUTPUT))
    return 1;
```

After initialization, input/output functions can be called.<br>
The input/output functions are thread-safe.<br>

```c++
uint32_t input_val;

// read input
if (!gpio1.read(input_val))
    return false;

// write output 0
if (!gpio2.write(0))
    return false:

// toggle output
if (!gpio2.toggle())
    return false:
```

Monitoring the change of the input pin is possible with the **watch** function.<br>

```c++
// init input with edge
if (!gpio1.init(INPUT_PIN, GPIO_MODE_INPUT_PULLUP, DEBOUNCE_US, GPIO_EDGE_BOTH))
    return false;

uint32_t edge;

// watch changes
while(gpio1.watch(edge))
{
    if (edge == GPIO_EDGE_RISING)
        puts("rising edge occurs");
    else
        puts("falling edge occurs");
}
```

