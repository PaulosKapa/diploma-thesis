import 'package:universal_ble/universal_ble.dart';
import 'package:permission_handler/permission_handler.dart';
import '../globals.dart';

//the bluetooth function
Future<bool> connectBle(String deviceId) async {
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
      return true;
  } 
  }else {
    // Opensettings
    openAppSettings();
  }
  return false;
}
