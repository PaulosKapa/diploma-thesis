import 'dart:convert';
import 'package:http/http.dart' as http;
import 'package:flutter_dotenv/flutter_dotenv.dart';

class ApiClient {
  final String _url = "${dotenv.env['API_URL']}/predict";

  Future<Map<String, dynamic>> postSignal(List<double> signal) async {
    try {
      final response = await http.post(
        Uri.parse(_url),
        headers: {'Content-Type': 'application/json'},
        body: jsonEncode({'signal': signal}),
      ).timeout(const Duration(seconds: 10)); // Timeout για να μην κολλάει το κινητό

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