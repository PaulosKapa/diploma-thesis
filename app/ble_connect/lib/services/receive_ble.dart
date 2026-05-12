import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:universal_ble/universal_ble.dart';
import 'dart:async';
import '../globals.dart';

StreamSubscription? subscription;

Future<void> listenBle(
  BleDevice service, {
  //for use when calling then fuction
  required void Function(double value) onData,
})
async {
  String charUuid = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
  BleCharacteristic characteristic = await service.getCharacteristic(
    charUuid,
    service: SERVICE_UUID,
  );
  await UniversalBle.subscribeNotifications(
    service.deviceId,
    SERVICE_UUID, 
    charUuid
  );
  
  
  subscription = characteristic.onValueReceived.listen((value) {
    print("BLE RAW DATA: $value");
    final val = double.tryParse(utf8.decode(value));
    print("PARSED VALUE: $val");
    if (val != null) {
      //send data via the callback
      onData(val);      
    }    
  });
}

void stopBleStream(BleDevice service, String charUuid) {
  subscription?.cancel();
  
}