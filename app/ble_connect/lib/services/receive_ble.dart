import 'dart:convert';
import 'package:universal_ble/universal_ble.dart';
import 'dart:async';
import '../globals.dart';

Future<List<double>> listenBle(
  BleDevice service, {
  //for use when calling then fuction
  void Function(int current, int total)? onProgress,
}) async {
  final completer = Completer<List<double>>();
  var ecg_list = <double>[];
  var target = 3600;
  BleCharacteristic characteristic = await service.getCharacteristic(
    '2a56',
    service: SERVICE_UUID,
  );
  StreamSubscription? subscription;
  subscription = characteristic.onValueReceived.listen((value) {
    final val = double.tryParse(utf8.decode(value));
    if (val != null) {
      ecg_list.add(val);
      //increase the callback value
      onProgress?.call(ecg_list.length, target);
    }
    if (ecg_list.length >= target) {
      subscription?.cancel();
      //stops the data flow
      characteristic.unsubscribe(); 
      if (!completer.isCompleted) completer.complete(ecg_list);
    }
  });

  return completer.future;
}
