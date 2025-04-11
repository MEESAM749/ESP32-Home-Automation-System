# 🏠 Home Underground Water Pump Automation using ESP32 + LoRa

Welcome to a DIY water pump automation and control setup using ESP32 and long-range LoRa communication.  
You can automate or control a wide range of electrical appliances — **as long as you're using components rated for your specific application** (e.g., proper relays for high voltage).

---

## 📦 Components Required

- 2x **ESP32-WROOM-32**
- 2x **SX1278 (Ra-02) LoRa Modules**
- 2x **433MHz SMA Antennae**
- 2x **SMA Female Jack to IPX / U.FL RF Pigtail Cables**
- 1x **SRD-05VDC-SL-C Relay Module**
- 1x **AJ-SR04M Waterproof Ultrasonic Sensor**

---

## 🔧 Schematic

> _(Click to enlarge)_

<img width="2000" alt="Schematic" src="https://github.com/user-attachments/assets/be5b64c1-4705-4fa5-8390-d5da07377d7c" />  

![IMG_20250409_161252(1)(1)](https://github.com/user-attachments/assets/cdc66752-a359-4d24-b17a-8894f50f7ef4)   

![gitimage](https://github.com/user-attachments/assets/af000dc7-1c48-42fe-a29d-3a7291abf9cd)    

#### Prototype Video:
https://youtu.be/mG5voW4rZHY  





---

## ⚙️ How It Works

### 📤 Transmission Section:
- The ultrasonic sensor sends distance measurements to the ESP32.
- The ESP32 transmits this data via the LoRa module to the receiver.

### 📥 Receiver Section:
- The receiving LoRa module sends data to its connected ESP32.
- Based on the received data, the ESP32 decides whether to activate or deactivate the load (motor) via a **5V relay**.
- This ESP32 is also connected to a **home Wi-Fi network**, giving it access to the internet.
- A **web server** is configured on this ESP32, allowing **remote control** of the motor (e.g., via a smartphone).

### 🧰 Manual Override:
A **manual override switch** is also installed — it can cut power to the motor regardless of the ESP32’s state. Because sometimes you just need to shut it down the old-school way.

---

## ⚠️ Disclaimer

This project is for **educational purposes only**.  
Always use **components rated for your specific voltage and current requirements**, and ensure safe wiring practices.  
**I’m not responsible** if your toaster toasts itself, your fuse box explodes, or your cat launches into orbit.

---

## 🪪 License

This project is licensed under the **MIT License**.  
You’re free to use, modify, or launch it into orbit — just don’t sue me if something goes sideways.

See the `LICENSE` file for full details.
