import 'package:flutter/material.dart';
import '../screens/qr_scanner.dart';

class startConnect extends StatelessWidget {
  const startConnect({super.key});
  @override
  Widget build(BuildContext context) {
    return TextButton(
      style: ButtonStyle(
        foregroundColor: WidgetStateProperty.all<Color>(Colors.blue),
      ),
      onPressed: () {
        Navigator.push(
          context,
          MaterialPageRoute(builder: (contect) => const QrScannerPage()),
        );
      },
      child: Text('TextButton'),
    );
  }
}