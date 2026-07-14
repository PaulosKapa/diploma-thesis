import 'package:flutter/material.dart';
import 'receive_afib.dart';
import 'normalization.dart';
import 'dart:isolate';

Future<int?> afibAnalysis(List<double> signal) async {
  //the class from receive_afib.dart
  final handler = getAfibResult();
  //var for the result (0/1) and wether or not the ml algorithm tries to find if there is afib or not
  //it may be nothing in case the ml doesn't work
  int? result;

  //try to get the result, if there is an error change isLoading to false. Else change both isLoading and the result
  try {
    //run on a seperate thread so the app doesn't freeze
    int prediction = await Isolate.run(() async {
      //first normalize the signal
      final normalizedSignal = normalizeSignal(signal);
      //then do the prediction
      return await handler.getPrediction(normalizedSignal);
    });
    result = prediction;
    debugPrint(result.toString());
    

  } catch (e) {
    debugPrint("Erro: $e");
  }
  return(result);
}
