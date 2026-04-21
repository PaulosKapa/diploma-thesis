import 'dart:convert';
import 'package:http/http.dart' as http;
import 'package:flutter_dotenv/flutter_dotenv.dart';

class ApiClient {
  //load the api endpoint from the .env file. We add /predict to match the endpoint in the backend
  final String _url = "${dotenv.env['API_URL']}/predict";
  //it gets the data from the backend
  Future<Map<String, dynamic>> postSignal(List<double> signal) async {
    try {
      final response = await http.post(
        Uri.parse(_url),
        headers: {'Content-Type': 'application/json'},
        body: jsonEncode({'signal': signal}),
      ).timeout(const Duration(seconds: 10)); 
      //if there isn't an error
      if (response.statusCode == 200) {
        return jsonDecode(response.body);
      } else {
        throw Exception("Server Error: ${response.statusCode}");
      }
    } catch (e) {
      throw Exception("Failed to connect: $e");
    }
  }
}