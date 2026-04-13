//split the string from the qr code to extract the MAC address
String? parseMacAddress(String qrContent) {
  try {
    List<String> parts = qrContent.split(';');
    String macPart = parts.firstWhere((e) => e.contains('MAC:'));
    return macPart.replaceAll('BT:MAC:', '');
  } catch (e) {
    
    return null;
  }
}


