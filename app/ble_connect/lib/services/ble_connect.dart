import 'dart:async';
import 'package:universal_ble/universal_ble.dart';
import '../globals.dart';
import 'package:permission_handler/permission_handler.dart';

//the bluetooth function
FutureOr<bool> connectBle(String deviceId) async {
  //ask the permissions
  var status = await Permission.bluetoothScan.request();
  var connectStatus = await Permission.bluetoothConnect.request();
  var locationStatus = await Permission.location.request();
  //when the permissions are granted
  if (status.isGranted && connectStatus.isGranted) {
    AvailabilityState state =
        await UniversalBle.getBluetoothAvailabilityState();
    //wait for bluetooth to turn on
    if (state == AvailabilityState.poweredOn) {
      //scan device based on the mac we passed when scanning the qr
      final completer = Completer<BleDevice>();
      StreamSubscription? sub;

      sub = UniversalBle.scanStream.listen((device) {
        if (device.deviceId.toLowerCase() == deviceId.toLowerCase() &&
            !completer.isCompleted) {
          sub?.cancel();
          UniversalBle.stopScan();
          completer.complete(device);
        }
      });

      UniversalBle.startScan();

      BleDevice device;
      try {
        device = await completer.future.timeout(const Duration(seconds: 10));
      } catch (_) {
        sub.cancel();
        await UniversalBle.stopScan();
        return false;
      }
      //connect and save to glabals.dart
      await device.connect();
      connectedDevice = device;
      return true;
    }
  }
  return false;
}
