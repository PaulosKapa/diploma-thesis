import 'dart:math';
//code to do the same normalization used when training the model
List<double> normalizeSignal(List<double> signal) {
  //mean
  final mean = signal.reduce((a, b) => a + b) / signal.length;
  //std
  final variance = signal
      .map((x) => (x - mean) * (x - mean))
      .reduce((a, b) => a + b) / signal.length;
  //don't divide by 0
  double std = variance == 0 ? 1.0 : sqrt(variance); 
  //final z-score
  return signal.map((x) => (x - mean) / std).toList();
}