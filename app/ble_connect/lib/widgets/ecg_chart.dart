import 'package:flutter/material.dart';
import 'package:fl_chart/fl_chart.dart';

class EcgChartWidget extends StatelessWidget {
  /// Η λίστα με τα σημεία που θα εμφανιστούν στην οθόνη (π.χ. τα τελευταία 300)
  final List<double> dataPoints;
  
  /// Το σταθερό πλάτος του άξονα Χ (ώστε να μην αυξομειώνεται το γράφημα)
  final double windowSize;

  const EcgChartWidget({
    Key? key,
    required this.dataPoints,
    this.windowSize = 300.0,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    // Μετατροπή των απλών double σε FlSpot (συντεταγμένες X, Y) για το fl_chart
    List<FlSpot> spots = dataPoints.asMap().entries.map((entry) {
      return FlSpot(entry.key.toDouble(), entry.value);
    }).toList();

    return LineChart(
      LineChartData(
        // Αφαιρούμε τα νούμερα από τους άξονες για πιο "καθαρό" ιατρικό look
        titlesData: const FlTitlesData(show: false),
        
        // Σχεδιασμός του πλέγματος (σαν μιλιμετρέ χαρτί)
        gridData: FlGridData(
          show: true,
          drawVerticalLine: true,
          getDrawingHorizontalLine: (value) => FlLine(
            color: Colors.redAccent.withOpacity(0.2),
            strokeWidth: 1,
          ),
          getDrawingVerticalLine: (value) => FlLine(
            color: Colors.redAccent.withOpacity(0.2),
            strokeWidth: 1,
          ),
        ),
        
        // Το περίγραμμα του γραφήματος
        borderData: FlBorderData(
          show: true,
          border: Border.all(color: Colors.redAccent.withOpacity(0.5)),
        ),
        
        // Σταθεροποιούμε τον άξονα X ώστε η γραμμή να φαίνεται ότι "κυλάει"
        minX: 0,
        maxX: windowSize,
        
        // Τα δεδομένα της γραμμής
        lineBarsData: [
          LineChartBarData(
            spots: spots,
            isCurved: false, // Το ΗΚΓ έχει αιχμηρές κορυφές, όχι καμπύλες
            color: Colors.red,
            barWidth: 1.5,
            dotData: const FlDotData(show: false), // ΣΗΜΑΝΤΙΚΟ: Κρύβει τις τελείες για τέλειο performance
          ),
        ],
      ),
      // Διάρκεια zero για να μην λαγκάρει στο συνεχές real-time update
      duration: Duration.zero,
    );
  }
}