1.0.0
- Initial release

1.0.1
- Change mode GPIO_MODE_INPUT_NOPULL to GPIO_MODE_INPUT
- Add setchip function in c_gpio
- Change print_err function
- Add invert parameter on i/o functions
- Add c_priority, c_timer class
- Adapt examples
- Add hx711 and ds18b20 examples

1.0.2
- Add API.md to all class
- Add read_sensors, strtoid and idtostr function in c_ds18b20.h
- Add example read_multi in ds18b20

1.0.3
- Add scan_sensor in c_ds18b20.h
- Add example scan and scan_read in ds18b20

1.0.4
- Add store error message in gpiox
- Add clear_error and get_error functions in gpiox, ds18b20 and hx711
- Bugfix in ds18b20
- Add counter example
