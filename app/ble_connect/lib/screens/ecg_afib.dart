import 'package:flutter/material.dart';
import '../services/afib_reconise.dart';
import '../widgets/ecg_chart.dart';
import '../services/receive_ble.dart';
import '../globals.dart';

class EcgAfib extends StatefulWidget {
    const EcgAfib({super.key});

  @override
  State<EcgAfib> createState() => EcgAfibState();
}

class EcgAfibState extends State<EcgAfib> {
  // Buffers
  static const int ai_list_size = 3600;
  static const int ecg_list_size = 300;

  final List<double> ai_list = [];
  final List<double> ecg_list = [];

  // State Variables
  bool isLoading = false;
  int? result;

@override
  void initState() {
    super.initState();
    // 1. Ξεκινάμε τη λήψη δεδομένων αμέσως μόλις ανοίξει το Widget
    startReceivingData(); 
  }
  @override
  void dispose() {
    // 3. Σταματάμε το BLE όταν κλείνει η οθόνη
    // ΣΗΜΕΙΩΣΗ: Αν η listenBle επιστρέφει StreamSubscription, αποθήκευσέ το 
    // σε μια μεταβλητή (π.χ. bleSubscription) και κάνε εδώ bleSubscription?.cancel();
    stopBleStream(connectedDevice!, 'beb5483e-36e1-4688-b7f5-ea07361b26a8');
    super.dispose();
  }

  void startReceivingData() async {
    // Κλήση της δικής σου external συνάρτησης!

    try{
      await listenBle(
      connectedDevice!,
      onData: (double val) {
        setState(() {
          // Κυλάει το γράφημα
          ecg_list.add(val);
          if (ecg_list.length > ecg_list_size) ecg_list.removeAt(0);

          // Γεμίζει η λίστα του AI
          ai_list.add(val);
          if (ai_list.length > ai_list_size) ai_list.removeAt(0);
        });
      },
    );
  }catch (e) {
      debugPrint("Erro: $e");
    }

  }


  // Καλεί την εξωτερική σου συνάρτηση AI
  void triggerAnalysis() async {
    setState(() {
      isLoading = true;
      
    });

    // Καλούμε την afibAnalysis από το άλλο σου αρχείο!
    int? tempResult = await afibAnalysis(List.from(ai_list));

    setState(() {
      result = tempResult;
      isLoading = false;
      ai_list.clear();
    });
  }

  @override
  Widget build(BuildContext context) {
    bool isAiReady = ai_list.length == ai_list_size;

    return Scaffold(
      body: Column(
        children: [
          // Το γράφημα (Αν δεν το έχεις σε ξεχωριστό αρχείο, κάνε επικόλληση την class του εδώ κάτω)
          Expanded(
            child: Padding(
              padding: const EdgeInsets.all(16.0),
              child: EcgChartWidget(
                dataPoints: ecg_list,
                windowSize: ecg_list_size.toDouble(),
              ),
            ),
          ),
          
          // Το UI για το αποτέλεσμα και το κουμπί
          Container(
            padding: const EdgeInsets.all(24.0),
            child: Column(
              mainAxisSize: MainAxisSize.min,
              children: [
                if (result != null && result != -1)
                  Padding(
                    padding: const EdgeInsets.only(bottom: 16.0),
                    child: Text(
                      result == 1 ? "Probable AFib" : "Normal cardiac rythm",
                      style: TextStyle(
                        fontSize: 18,
                        fontWeight: FontWeight.bold,
                        color: result == 1 ? Colors.red : Colors.green,
                      ),
                      textAlign: TextAlign.center,
                    ),
                  ),

                SizedBox(
                  width: double.infinity,
                  height: 55,
                  child: ElevatedButton.icon(
                    onPressed: (isAiReady && !isLoading) ? triggerAnalysis : null,
                    icon: isLoading 
                        ? const CircularProgressIndicator(color: Colors.white)
                        : const Icon(Icons.auto_awesome),
                    label: Text(
                      isLoading 
                          ? 'Analyzing...'
                          : isAiReady 
                              ? 'Ai analysis'
                              : 'Collecting ${((ai_list.length / ai_list_size) * 100).toStringAsFixed(0)}%',
                      style: const TextStyle(fontSize: 16),
                    ),
                    style: ElevatedButton.styleFrom(
                      backgroundColor: const Color.fromARGB(255, 125, 90, 184),
                      foregroundColor: Colors.white,
                    ),
                  ),
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }
}