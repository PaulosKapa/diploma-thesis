import 'dart:convert';
import 'package:universal_ble/universal_ble.dart';

Future <List> listenBle(BleDevice service) async { 
  var ecg_list = <double?>[];
  BleCharacteristic characteristic = await service.getCharacteristic('2a56', service: '');
  await characteristic.notifications.subscribe();
  characteristic.onValueReceived.listen((value) {
    String s = utf8.decode(value);
    double? val = double.tryParse(s);
    ecg_list.add(val);
     
});
  if(ecg_list.length==3600){
      return ecg_list;
    } 
  throw Exception("Didn't get the messages");
}
