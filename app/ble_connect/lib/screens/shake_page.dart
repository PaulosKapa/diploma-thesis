import 'package:flutter/material.dart';
import '../widgets/start_shake.dart';

class startConnect extends StatelessWidget {
  const startConnect({super.key});
  //start the start_shake.dart widget
  @override
  Widget build(BuildContext context) {
    return const MaterialApp(
      home: Scaffold(body: Center(child: startShake())),
    );
  }
}