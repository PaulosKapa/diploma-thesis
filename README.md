This repository includes all the code and assets used by me, Christos Pavlos Anglelis, as part of my diploma thesis in the MSc programm "Biomedical Engineering & Technology’, organized by the Department of Biomedical Engineering of the University of West Attica, Greece. <br>
The thesis with the title "", has as an objective to design and develop an ECG device <br>The object of this thesis is to design and develop a working electrocardiograph (ECG) smart device. The design includes the frontend analog and digital electronics that acquire and process the signal, an app that displays the signal and a machine learning (ML) algorithm that process the signal. The device connects to the app using bluetooth low energy (BLE), sends the signals and then the user can perform an “AI analysis”, which uses a ML algorithm to predict whether the user shows symptoms of atrial fibrillation (AFib) or not. The implementation of the above is done seamlessly, using an intuitive user interface, with ease of connectivity and without the hassle of using a right leg drive (RLD) electrode. The functionality of the device was tested using a TechPatient® CARDIO V4 ECG simulator and the results showed that all its parts work as intended. The simulator gave the confirmation that the analog electronics were working even without a dedicated RLD electrode and a physical notch filter, the code in the microprocessor is successful at receiving, digitizing, processing and sending the signal and that the ML algorithm is able to differentiate between normal and pathological heart beats. This study proved that a totally portable, efficient and low cost ECG device that can be used anytime and paired with apps that offer real time diagnostics is realistic and a solution that can be used for tele - medicine and health / cardiovascular monitoring.<br>


More information on the block diagram bellow<br><img width="1920" height="1080" alt="Patient" src="https://github.com/user-attachments/assets/038abff2-ed34-4306-9119-e59580cad895" />
# Portable AI-Powered ECG Smart Device

**Author:** Christos Pavlos Anglelis  
*MSc in Biomedical Engineering & Technology*  
Department of Biomedical Engineering, University of West Attica, Greece  

---

## 📖 Overview
This repository contains the complete code and assets for my MSc diploma thesis. The objective of this project is the design and development of a working, low-cost, and totally portable electrocardiograph (ECG) smart device. It integrates custom hardware, mobile software, and machine learning for seamless cardiovascular monitoring and telemedicine applications.

## ✨ Key Features
* **Real-time Signal Acquisition:** Custom frontend analog and digital electronics that capture and process physiological signals.
* **No Right Leg Drive (RLD):</strong> Seamless operation and ease of connectivity without the hassle of a physical RLD electrode or hardware notch filter.
* **Wireless Mobile Integration:** Real-time signal display and pairing with a mobile companion app via **Bluetooth Low Energy (BLE)**.
* **AI-Assisted Diagnostics:** Integrated machine learning (ML) algorithm to predict atrial fibrillation (AFib) symptoms from the user's ECG.
* **Validated Performance:** Rigorously tested and verified using a professional **TechPatient® CARDIO V4** ECG simulator.

## ⚙️ System Architecture & Block Diagram
The implementation of the system is built seamlessly to prioritize an intuitive user interface, portable operation, and automated analytics. More information on the system layout can be found in the block diagram below:

<p align="center">
  <img width="1920" height="1080" alt="Patient Block Diagram" src="https://github.com" style="max-width: 100%; height: auto;" />
</p>

## 🔬 Methodology & Validation
The device's functionality was evaluated using a professional ECG simulator. Experimental results confirmed:

1. The **analog electronics** extract high-fidelity signals without a dedicated RLD electrode.
2. The **microprocessor firmware** successfully digitizes, filters, and transmits the ECG data package.
3. The **ML algorithm** reliably differentiates between normal and pathological (AFib) heartbeats.

This study proves that an ultra-portable, efficient, and low-cost ECG ecosystem is highly realistic for modern tele-medicine and active cardiovascular health tracking.


