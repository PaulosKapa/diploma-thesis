import 'package:flutter/material.dart';
import '../services/receive_afib.dart';

class PredictionScreen extends StatefulWidget {
  const PredictionScreen({super.key});

  @override
  State<PredictionScreen> createState() => _PredictionScreenState();
}

class _PredictionScreenState extends State<PredictionScreen> {
  final PredictionHandler _handler = PredictionHandler();
  int? _result; // Θα κρατάει το 0 ή 1
  bool _isLoading = false;

  void _startAnalysis() async {
    setState(() => _isLoading = true);

    // Εδώ κανονικά θα έρχονται τα δεδομένα από το ESP32
    // Για το τεστ, φτιάχνουμε μια τυχαία λίστα 3600 στοιχείων
    List<double> dummySignal = List.generate(3600, (index) => 0.5);

    try {
      int prediction = await _handler.getPrediction(dummySignal);
      setState(() {
        _result = prediction;
        _isLoading = false;
      });
    } catch (e) {
      setState(() => _isLoading = false);
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text("Error: $e")),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text("ECG AI Monitor")),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            if (_isLoading) const CircularProgressIndicator(),
            if (!_isLoading && _result != null)
              Column(
                children: [
                  Icon(
                    _result == 1 ? Icons.warning : Icons.check_circle,
                    size: 100,
                    color: _result == 1 ? Colors.red : Colors.green,
                  ),
                  Text(
                    _result == 1 ? "Atrial Fibrillation" : "Normal Sinus Rhythm",
                    style: const TextStyle(fontSize: 24, fontWeight: FontWeight.bold),
                  ),
                ],
              ),
            const SizedBox(height: 30),
            ElevatedButton(
              onPressed: _isLoading ? null : _startAnalysis,
              child: const Text("Start AI Analysis"),
            ),
          ],
        ),
      ),
    );
  }
}