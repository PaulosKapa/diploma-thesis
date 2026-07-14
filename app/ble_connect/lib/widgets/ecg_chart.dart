import 'package:flutter/material.dart';
import 'package:fl_chart/fl_chart.dart';

class EcgChartWidget extends StatelessWidget {
  final List<double> dataPoints;
  
  final double windowSize;

  const EcgChartWidget({
    Key? key,
    required this.dataPoints,
    this.windowSize = 300.0,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    //change floats to flspot so they work with flchart
    List<FlSpot> spots = dataPoints.asMap().entries.map((entry) {
      return FlSpot(entry.key.toDouble(), entry.value);
    }).toList();

    return LineChart(
      LineChartData(
        titlesData: const FlTitlesData(show: false),
        
        //make the grid
        gridData: FlGridData(
          show: true,
          drawVerticalLine: true,
          getDrawingHorizontalLine: (value) => FlLine(
            color: const Color.fromARGB(255, 73, 38, 143).withValues(alpha: 0.2),
            strokeWidth: 1,
          ),
          getDrawingVerticalLine: (value) => FlLine(
            color: const Color.fromARGB(255, 73, 38, 143).withValues(alpha: 0.2),
            strokeWidth: 1,
          ),
        ),
        
        //border of the chart
        borderData: FlBorderData(
          show: true,
          border: Border.all(color: const Color.fromARGB(255, 57, 5, 170).withValues(alpha: 0.5)),
        ),
        
        minX: 0,
        maxX: windowSize,
        //the line's data
        lineBarsData: [
          LineChartBarData(
            spots: spots,
            isCurved: false,
            color: const Color.fromARGB(255, 108, 78, 170),
            barWidth: 1.5,
            dotData: const FlDotData(show: false),
          ),
        ],
      ),
      duration: Duration.zero,
    );
  }
}