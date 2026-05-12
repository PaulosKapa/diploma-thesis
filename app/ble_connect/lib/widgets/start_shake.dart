import 'package:flutter/material.dart';
import '../screens/qr_scanner.dart';
import 'package:shake/shake.dart';
//the class that handles the shake event
class startShake extends StatefulWidget{
  const startShake({super.key});
  @override
  State<startShake> createState() => _startShakePage();
}
class _startShakePage extends State<startShake>{
  //start the shake detector
  ShakeDetector? _detector;
  //temp string until we change with audio/image
  String _shakeInfo = 'Shake phone';
  //initiate the method
  @override
  void initState() {
    super.initState();
    _startDetector();
  }
  void _startDetector() {
    // Stop previous detector if exists
    _detector?.stopListening();
    //change the shake sensitivity to 1.1, since the default was very high
    _detector = ShakeDetector.autoStart(
      shakeThresholdGravity: 1.1,
      //when a shake is detected check if the force was more than a value
      //then stop the detector and go to the qr_scanner.dart page
      onPhoneShake: (ShakeEvent event) {
        _shakeInfo = event.force.toString();
        if(event.force>1.25){
        _detector?.stopListening();
        Navigator.push(
            context,
            MaterialPageRoute(builder: (context) => QrScannerPage()));
      }
      },
    );
  }
  //dispose
  @override
  void dispose() {
    _detector?.stopListening();
    super.dispose();
  }
  //a simple ui. will probably add image or audio soon
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Center(
        child: Text(
          _shakeInfo,
          textAlign: TextAlign.center,
        ),
      ),
    );
  }
}
