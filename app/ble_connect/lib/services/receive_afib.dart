import 'api_connect.dart';

class getAfibResult {
  //get the api class
  ApiClient client = ApiClient();
  //this is themethod for predicting the afib 
  Future<int> getPrediction(List<double> signal) async {
    //get the data from api_connect.dart
      final data = await client.postSignal(signal);    
    //returns 1 or 0 based on if there is or id there isn't afib
    if (data.containsKey('is_afib')) {
      return data['is_afib'] as int;
    }    
    throw Exception("Invalid response format");
  }
}