# ble_thingspeak
Example of accessing data from an BME280 sensor attached to an Artemis development board via BLE. The data results are then posted to ThingSpeak

Setup
=====
## Python
Ensure a later version of Python is installed (version 3.9 was used in development)
Install the following packages
* requests
* bleak
* logging

## On a Mac
You need to ensure that your terminal application is allowed to access Bluetooth. Go to System Preferences -> Security & Privacy > Privacy, and add your terminal to the Bluetooth section

## ThingSpeak
* Create a ThingSpeak Channel with three fields
* Get the API Key and set the value TS_WR_API_KEY to this key in ble_thingspeak.py (line 35 ish)

## Hardware
* A SparkFun Artemis board (https://www.sparkfun.com/products/15574)
* A SparkFun board with a BME280 (ENV Combo: https://www.sparkfun.com/products/14348)
* Flash the Arduino sketch contained in the Firmware directory of this repo to the Artemis board

## Find the Address for your board
After you flash the board, you need to get the address that your computer recognizes.
* Windows - The Firmware writes the address to the serial console at startup
* Mac - Run the get_device_id.py python script in this repo

In ble_thingspeak.py, set the value of ADDRESS
```python
# On a MAC
ADDRESS = "F0581182-BFBE-4EEB-BA08-D023AF049601"
```
```python
# On Windows
ADDRESS = "C0:97:D4:C9:1B:02"
```

Running the Example
===================
The basic steps for running the example:
* Complete the above setup
* Power on the Artemis board with a BME280 installed
* At the command prompt/terminal
```sh
python ble_thingspeak.py
Connected: True
Received callback data 82.48999786376953 35.9599609375 84890.171875
200
Received callback data 82.99400329589844 35.5087890625 84888.453125
200
```
If the system connects to the device, "Connected: True" is printed. After that, each line is the data values sent to ThingSpeak and then the HTTP return code (200= success, 400 = error)
