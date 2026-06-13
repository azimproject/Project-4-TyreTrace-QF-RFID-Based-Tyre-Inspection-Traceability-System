// TyreTrace QF - RFID Based Tyre Inspection Traceability System
// Simulated on Wokwi | ESP32 + MFRC522 + LCD I2C

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SS_PIN   5
#define RST_PIN  27
#define GREEN_LED 26
#define RED_LED   25
#define BUZZER    33

// ---- BOOTH CONFIGURATION (change per booth) ----
#define STATION  3        // 1, 2, or 3 (OL)
#define BOOTH_ID "3A"

MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---- Employee database ----
struct Employee {
  String uid;
  String name;
  String empNo;
  int authStation; // highest station authorised (1, 2, or 3)
};

Employee employees[] = {
  {"AA BB CC DD", "M. Khilal",   "QF810101", 2},
  {"11 22 33 44", "A. Fariz",    "QF810102", 2},
  {"55 66 77 88", "S. Nabilah",  "QF810103", 2},
  {"01 02 03 04", "R. Hamdan",   "QF810104", 3}
};
const int NUM_EMPLOYEES = 4;

// ---- Tyre database ----
struct Tyre {
  String barcode;
  String model;
  String type;     // Normal or OL
  String customer;
};

Tyre tyres[] = {
  {"TY-10001", "195/65R15", "Normal", "-"},
  {"TY-10002", "205/55R16", "Normal", "-"},
  {"TY-10003", "225/45R17", "OL",     "Toyota"},
  {"TY-10004", "235/50R18", "OL",     "Honda"},
  {"TY-10005", "215/60R16", "OL",     "Proton"}
};
const int NUM_TYRES = 5;

// ---- Session state ----
bool cardPresent = false;
int currentEmp = -1;

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();

  lcd.init();
  lcd.backlight();

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  showIdle();
  Serial.println("=================================");
  Serial.println(" TyreTrace QF - Booth " + String(BOOTH_ID));
  Serial.println(" Station " + String(STATION) + " ready");
  Serial.println("=================================");
}

void loop() {
  if (!cardPresent) {
    checkCard();
  } else {
    checkBarcodeInput();
  }
}

// ---- Show idle screen ----
void showIdle() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Booth " + String(BOOTH_ID) + " - Stn " + String(STATION));
  lcd.setCursor(0, 1);
  lcd.print("Slot card...");
}

// ---- Check for RFID card ----
void checkCard() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) uid += " ";
  }
  uid.toUpperCase();

  // Look up employee
  int empIndex = -1;
  for (int i = 0; i < NUM_EMPLOYEES; i++) {
    if (employees[i].uid == uid) {
      empIndex = i;
      break;
    }
  }

  if (empIndex == -1) {
    // Unknown card
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Unknown badge!");
    lcd.setCursor(0, 1);
    lcd.print("Access denied");
    denyBeep();
    Serial.println("[DENIED] Unknown card UID: " + uid);
    delay(2000);
    showIdle();
    return;
  }

  // Check station authorisation
  if (employees[empIndex].authStation < STATION) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(employees[empIndex].name);
    lcd.setCursor(0, 1);
    lcd.print("Not authorised");
    denyBeep();
    Serial.println("[DENIED] " + employees[empIndex].name + " (" + employees[empIndex].empNo + ") not authorised for Station " + String(STATION));
    delay(2000);
    showIdle();
    return;
  }

  // Authorised - start session
  cardPresent = true;
  currentEmp = empIndex;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome " + employees[empIndex].name);
  lcd.setCursor(0, 1);
  lcd.print(employees[empIndex].empNo + " Stn" + String(STATION));

  grantBeep();
  Serial.println("");
  Serial.println("[SESSION START] " + employees[empIndex].name + " (" + employees[empIndex].empNo + ") at Booth " + String(BOOTH_ID));

  delay(1500);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ready. Scan tyre");
  lcd.setCursor(0, 1);
  lcd.print("barcode now...");
}

// ---- Simulate barcode scan via Serial input ----
void checkBarcodeInput() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    input.toUpperCase();

    if (input == "REMOVE") {
      // End session
      Serial.println("[SESSION END] " + employees[currentEmp].name + " removed card from Booth " + String(BOOTH_ID));
      Serial.println("");
      cardPresent = false;
      currentEmp = -1;
      showIdle();
      return;
    }

    // Look up tyre
    int tyreIndex = -1;
    for (int i = 0; i < NUM_TYRES; i++) {
      if (tyres[i].barcode == input) {
        tyreIndex = i;
        break;
      }
    }

    if (tyreIndex == -1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Unknown tyre");
      lcd.setCursor(0, 1);
      lcd.print("barcode!");
      denyBeep();
      Serial.println("[ERROR] Unknown tyre barcode: " + input);
      delay(1500);
      backToReady();
      return;
    }

    // Station 3 warning if not OL tyre
    if (STATION == 3 && tyres[tyreIndex].type != "OL") {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Warning: not");
      lcd.setCursor(0, 1);
      lcd.print("an OL tyre!");
      denyBeep();
      Serial.println("[WARNING] " + tyres[tyreIndex].barcode + " is Normal type, sent to OL Station 3");
      delay(1500);
      backToReady();
      return;
    }

    // Valid inspection - log it
    grantBeep();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(tyres[tyreIndex].barcode + " " + tyres[tyreIndex].model);
    lcd.setCursor(0, 1);
    lcd.print(employees[currentEmp].empNo + " PASSED");

    Serial.println("---------------------------------");
    Serial.println("INSPECTION LOG");
    Serial.println("Booth     : " + String(BOOTH_ID));
    Serial.println("Station   : " + String(STATION) + getStationName());
    Serial.println("Inspector : " + employees[currentEmp].name);
    Serial.println("Emp No    : " + employees[currentEmp].empNo);
    Serial.println("Tyre ID   : " + tyres[tyreIndex].barcode);
    Serial.println("Model     : " + tyres[tyreIndex].model);
    Serial.println("Type      : " + tyres[tyreIndex].type);
    Serial.println("Customer  : " + tyres[tyreIndex].customer);
    Serial.println("Status    : PASSED");
    Serial.println("---------------------------------");

    delay(2000);
    backToReady();
  }
}

void backToReady() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(employees[currentEmp].name);
  lcd.setCursor(0, 1);
  lcd.print("Scan next tyre");
}

String getStationName() {
  if (STATION == 1) return " - 1st Inspection";
  if (STATION == 2) return " - 2nd Inspection";
  return " - OL Inspection";
}

void grantBeep() {
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(BUZZER, HIGH);
  delay(150);
  digitalWrite(BUZZER, LOW);
  delay(700);
  digitalWrite(GREEN_LED, LOW);
}

void denyBeep() {
  digitalWrite(RED_LED, HIGH);
  digitalWrite(BUZZER, HIGH);
  delay(400);
  digitalWrite(BUZZER, LOW);
  delay(700);
  digitalWrite(RED_LED, LOW);
}
