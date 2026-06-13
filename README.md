# TyreTrace QF — RFID Based Tyre Inspection Traceability System

A digitalized tyre inspection verification system designed to solve a 
real accountability gap identified during a quality control internship 
at a tyre manufacturing plant.

## Background
During my internship in the Quality Finishing department at a tyre 
manufacturer, I identified a flaw in the manual inspection process: 
inspectors stamped tyres after inspection, but stamps were often not 
used correctly, making mis-inspections untraceable when defects were 
found later. I proposed a digitalized verification concept to my 
supervisor. This project is a working simulation of that proposal.

## Description
Inspectors slot an RFID badge into a card reader at their inspection 
booth. The system verifies their identity and authorization level for 
that station. Inspectors then scan tyre barcodes using a handheld 
scanner — each scan is logged with inspector identity, tyre details, 
station, and timestamp, creating a full traceability record.

## System Design
- 3 inspection stations: 1st inspection (newly cured tyres), 2nd 
  inspection (re-check), and OL inspection (export-grade tyres, 
  veteran inspectors only)
- Each station has multiple booths with fixed station assignment
- RFID card determines inspector identity and authorization level
- Barcode scan identifies tyre model, type, and destination customer
- System validates: unknown cards, unauthorized station access, and 
  Normal tyres incorrectly routed to OL stations

## Features
- RFID-based inspector authentication
- Station-level authorization control
- Tyre traceability logging (who, what, when, where)
- Error handling: unknown badge, unauthorized access, wrong tyre type
- LCD feedback for inspector
- Green/red LED and buzzer feedback

## Components
- ESP32
- MFRC522 RFID reader
- LCD 16x2 I2C
- Green LED, Red LED, Buzzer

## Example Log Output
INSPECTION LOG

Booth     : 3A

Station   : 3 - OL Inspection

Inspector : R. Hamdan

Emp No    : QF810104

Tyre ID   : TY-10003

Model     : 225/45R17

Type      : OL

Customer  : Toyota

Status    : PASSED

## Simulation
Run the live simulation here: [Open in Wokwi]((https://wokwi.com/projects/466129664831931393))

## Tools Used
- Arduino IDE / Wokwi simulator
- Embedded C
- RFID (MFRC522)
