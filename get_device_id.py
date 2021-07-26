


import asyncio

from bleak import BleakScanner

####################################################################
## Simple method to dump out the addresses of scanned devices
async def scan_devices():
    devices = await BleakScanner.discover()
    for d in devices:
        print(d)


# start up the system and dispatch execution
loop = asyncio.get_event_loop()
loop.run_until_complete(scan_devices())
