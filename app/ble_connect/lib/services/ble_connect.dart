import 'dart:async';
import 'package:universal_ble/universal_ble.dart';
import '../globals.dart';

Future<bool> connectBle(String deviceId) async {
  //get ble permissions
  try {
    await UniversalBle.requestPermissions();
  } catch (e) {
    return false;
  }
  // check ble's status
  final state = await UniversalBle.getBluetoothAvailabilityState();
  if (state != AvailabilityState.poweredOn) return false;
  //runs if the device is already connected
  try {
    final systemDevices = await UniversalBle.getSystemDevices(withServices: [SERVICE_UUID]);
    final already = systemDevices.where((d) => d.deviceId == deviceId).firstOrNull;
    if (already != null) {
      await already.connect();
      connectedDevice = already;
      return true;
    }
  } catch (_) {}
  //scan device based on the mac we passed when scanning the qr
  final completer = Completer<BleDevice>();
  StreamSubscription? sub;

  sub = UniversalBle.scanStream.listen((device) {
    if (device.deviceId == deviceId && !completer.isCompleted) {
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