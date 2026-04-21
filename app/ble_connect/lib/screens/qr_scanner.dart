import 'package:flutter/material.dart';
import 'package:mobile_scanner/mobile_scanner.dart';
import '../services/ble_service.dart';
import '../services/parser.dart';
import 'afib_recognise.dart';


//!!! for this to work with a raspberry pi you should run a ble GATT script on the pi!!!
class QrScannerPage extends StatefulWidget {
  const QrScannerPage({super.key});

  @override
  State<QrScannerPage> createState() => _QrScannerPageState();
}

//the class that handles the qr code scanner
class _QrScannerPageState extends State<QrScannerPage> {
  //this variable is to check whether or not we already use the scanner
  bool isProcessing = false;
  MobileScannerController controller = MobileScannerController();
  //method tostop the camera
  @override
  void dispose() {
    controller.dispose(); 
    super.dispose();
  }
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: MobileScanner(
        controller: controller,
        onDetect: (result) async {
          if (isProcessing) return;
          //get the value from the scanner and cast it as a string
          final String? rawValue = result.barcodes.first.rawValue;
          if (rawValue != null) {
            //use the parse function to get the mac address
            String? cleanMac = parseMacAddress(rawValue);
            //when we get the mac address the qr code scanner stops
            if (cleanMac != null) {
              setState(() => isProcessing = true);
              await controller.stop();
              
              //go to the afib page
              if (mounted){
                //connect to the device
                bool connected = await connectBle(cleanMac);
                if(connected){
                  await Navigator.push(context, 
                    MaterialPageRoute(builder: (context) => PredictionScreen())
      );
                }
                else{
                  setState(()  => isProcessing = false);
                  controller.start();
                }
              } 
              
            }
          }
        },
      ),
    );
  }
}