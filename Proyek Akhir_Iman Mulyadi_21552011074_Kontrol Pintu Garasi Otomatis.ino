#define BLYNK_TEMPLATE_ID "TMPL6pg0LTa6h"
#define BLYNK_TEMPLATE_NAME "Kontrol Garasi"
#define BLYNK_AUTH_TOKEN "iI-HHkXIf0S2n5UCgUkvvfsnSlonjQc1"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

// WiFi & Blynk
char ssid[] = "Wokwi-GUEST";
char pass[] = "";
char auth[] = BLYNK_AUTH_TOKEN;

// Pin HC-SR04 & Komponen
#define TRIG_PIN     5
#define ECHO_PIN     18
#define LED_PIN      25
#define SERVO_PIN    13

// Objek
Servo servo;
BlynkTimer timer;

// Variabel status
bool pintuTerbuka = false;
bool modeManual = false;
bool tombolManual = false;
int ambangJarak = 30; // ambang buka otomatis

// Fungsi membaca jarak
long bacaJarak() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long durasi = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms
  long jarak = durasi * 0.034 / 2;

  if (jarak == 0 || durasi == 0 || jarak > 400) {
    return 400;
  }

  return jarak;
}

// Fungsi membuka pintu
void bukaPintu() {
  servo.write(90);
  digitalWrite(LED_PIN, HIGH);
  Blynk.virtualWrite(V3, 255);
  pintuTerbuka = true;
  Serial.println("Pintu dibuka.");
}

// Fungsi menutup pintu
void tutupPintu() {
  servo.write(0);
  digitalWrite(LED_PIN, LOW);
  Blynk.virtualWrite(V3, 0); // LED indikator mati
  Serial.println("Pintu ditutup.");
  pintuTerbuka = false;
}

// Fungsi otomatisasi jarak
void cekJarak() {
  long jarak = bacaJarak();

  if (jarak == 400) {
    Serial.println("Gagal membaca jarak.");
    return;
  }

  Blynk.virtualWrite(V1, jarak);
  Serial.print("Jarak: ");
  Serial.print(jarak);
  Serial.println(" cm");

  // Jika tidak manual, pakai mode otomatis
  if (!modeManual) {
    if (jarak < ambangJarak && !pintuTerbuka) {
      bukaPintu();
    } else if (jarak >= ambangJarak && pintuTerbuka) {
      tutupPintu();
    }
  }
}

// Fungsi menerima tombol dari Blynk
BLYNK_WRITE(V2) {
  int state = param.asInt();
  tombolManual = state;
  modeManual = true; // Saat tombol ditekan, aktifkan mode manual

  if (tombolManual && !pintuTerbuka) {
    bukaPintu();
  } else if (!tombolManual && pintuTerbuka) {
    tutupPintu();
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  servo.attach(SERVO_PIN);
  servo.write(0); // posisi awal tertutup
  Blynk.virtualWrite(V3, 0); // Status awal indikator pintu tertutup

  Blynk.begin(auth, ssid, pass);
  timer.setInterval(300L, cekJarak);
}

void loop() {
  Blynk.run();
  timer.run();
}
