import 'package:flutter/material.dart';
import 'screens/shake_page.dart';
import 'package:flutter_dotenv/flutter_dotenv.dart';

Future<void> main() async {
  //load the environment variables and run the app
  await dotenv.load(fileName: ".env");
  runApp(const MainApp());
}


class MainApp extends StatelessWidget {
  const MainApp({super.key});
  //change to shake_page.dart
  @override
  Widget build(BuildContext context) {
    return const MaterialApp(
      home: Scaffold(body: Center(child: startConnect())),
    );
  }
}




