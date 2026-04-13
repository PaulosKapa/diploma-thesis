import 'package:flutter/material.dart';
import 'package:universal_ble/universal_ble.dart';
import 'package:permission_handler/permission_handler.dart';

//the bluetooth function
Future<bool> connectBle(BuildContext context, String deviceId) async {
  //ask the permissions
  var status = await Permission.bluetoothScan.request();
  var connectStatus = await Permission.bluetoothConnect.request();
  //when the permissions are granted
  if (status.isGranted && connectStatus.isGranted) {
    AvailabilityState state = await UniversalBle.getBluetoothAvailabilityState();
    //wait for bluetooth to turn on
    if (state == AvailabilityState.poweredOn) {
      //connect to the device using the mac address
      await UniversalBle.connect(deviceId);
      if (context.mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text("Connected to $deviceId")),
        );
      }
      return true;
  } 
  }else {
    // Open settings
    openAppSettings();
  }
  return false;
}
