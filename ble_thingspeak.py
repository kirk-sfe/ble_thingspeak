

import sys
import time
import platform
import logging
import asyncio
import struct
import requests

from bleak import BleakClient

## Local logging
log = logging.getLogger(__name__)
log.setLevel(logging.DEBUG)
h = logging.StreamHandler(sys.stdout)
h.setLevel(logging.DEBUG)
log.addHandler(h)

## BLE Parameters

## On Mac:
##  UUID Address of the target BLE Device - The best way to find this UUID is to
##  turn on the target device, then run the get_device_id.py file in this repository.
##  Then scan the results for your target device.
##
## Mac
ADDRESS = "F0581182-BFBE-4EEB-BA08-D023AF049601"

## Windows
## On Windows - use the address printed out via serial from the device
#ADDRESS = "C0:97:D4:C9:1B:02"

## The UUID of the char that triggers the notification (temperature in this example)
NOTIFICATION_UUID = "BEB5483E-36E1-4688-B7F5-EA07311B260E"

## Other char UUIDs -- used to grab additional data
HUMIDITY_UUID =     "beb5483e-36e1-4688-b7f5-ea07311b260d"
PRESSURE_UUID =     "beb5483e-36e1-4688-b7f5-ea07311b260f"


## Things speak things
TS_URL      = "https://api.thingspeak.com/update"
#key from ThingSpeak
TS_WR_API_KEY  = ""

# dictionary to store our data post parameters

data_post = {"api_key": TS_WR_API_KEY, "field1": 0, "field2": 0, "field3": 0}


####################################################################
# This routine connects to the BLE characteristic that has notifications
# enabled. When a notification takes plac,e the callback_handler() is called.
#
# Data is pushed into a queue - which is then consumed by another asynch routine


async def run_ble_client(address: str, queue: asyncio.Queue):

    # inner function -- callback handler for notifications
    # async def callback_handler(sender, data):
        # await queue.put((data, None, None))

    # connect to the notification characteristic
    async with BleakClient(address) as client:

        # inner function -- callback handler for notifications
        # NOTE: we are in the scope of "client" so we can easily read
        #       other characteristics
        async def callback_handler(sender, data):

            # Read the other data values -- note we wait for these
            humidity = await client.read_gatt_char(HUMIDITY_UUID);
            pressure = await client.read_gatt_char(PRESSURE_UUID);

            # jam in the queue
            await queue.put((data, humidity, pressure))


        log.info(f"Connected: {client.is_connected}")

        # wait for notifications -- send to callback_handler()
        await client.start_notify(NOTIFICATION_UUID, callback_handler)

        # basically our message pump - if the device disconnects, we end
        # otherwise, loop and sleep
        while True:
            if not client.is_connected:
                # this will send a "end" to the queue
                await queue.put((None, None, None))
                break
            await asyncio.sleep(1.0)


## The routine that pulls data from the observation/notification queue and posts to thingspeak.
async def run_queue_consumer(queue: asyncio.Queue):
    while True:
        # Use await asyncio.wait_for(queue.get(), timeout=1.0) if you want a timeout for getting data.
        data1, data2, data3 = await queue.get()
        if data1 is None:
            log.info(
                "Got message from client about disconnection. Exiting consumer loop..."
            )
            break
        else:
            value1 = struct.unpack('f', data1[0:4])[0]
            value2 = struct.unpack('f', data2[0:4])[0]
            value3 = struct.unpack('f', data3[0:4])[0]
            log.info(f"Received callback data {value1} {value2} {value3}")

            # place the updated data in the data_post dict, and send it to ThingSpeak
            data_post["field1"] = value1
            data_post["field2"] = value2
            data_post["field3"] = value3
            x = requests.post(TS_URL, data=data_post)
            print(x.status_code)

##
async def main(address: str):
    queue = asyncio.Queue()
    client_task = run_ble_client(address, queue)
    consumer_task = run_queue_consumer(queue)
    await asyncio.gather(client_task, consumer_task)
    log.info("Main method done.")


# start up the system and dispatch execution
loop = asyncio.get_event_loop()
loop.run_until_complete(main(ADDRESS))
