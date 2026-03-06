# SomnaCore — Project Context

## Project Goal

SomnaCore is a system that turns a **regular gym machine with a weight stack** into a **smart machine** that automatically records workouts.

The system detects:

* selected weight
* number of repetitions
* range of motion (ROM)
* sets
* user identity

without modifying the machine mechanics.

The system must work using **external sensors only**.

---

# Core Idea

Instead of force sensors or cameras, SomnaCore uses **laser time-of-flight distance sensors** to observe the movement of the weight stack.

Two sensors are used:

1. **Top sensor**
2. **Bottom sensor**

These sensors measure the movement of the stack plates.

---

# Hardware

Current prototype hardware:

ESP32 NodeMCU
VL53L0X laser distance sensors (GY-530)
PN532 NFC reader
OLED display 128x64 (I2C)

Communication bus:

I2C

---

# Sensor Roles

## Top Sensor

Mounted above the weight stack.

Purpose:

Detect **movement of the top plate**.

Used to calculate:

* repetitions
* range of motion (ROM)

Distance decreases when the stack rises.

---

## Bottom Sensor

Mounted at the bottom of the stack.

Purpose:

Detect **which plate is selected by the pin**.

The sensor measures distance to the plate column through a **narrow tunnel / mask**.

Each plate corresponds to a calibrated distance.

Example calibration:

D1 → plate 1
D2 → plate 2
D3 → plate 3

Plate index is converted to weight using a table.

---

# Measurement Logic

The system observes stack motion over time.

Repetition pattern:

distance_top:

HIGH → LOW → HIGH

This corresponds to:

start position → contraction → return

---

# Range of Motion (ROM)

ROM is calculated as:

ROM = max_distance - min_distance

This allows detecting:

* incomplete reps
* cheating
* partial range movement

---

# Set Detection

The firmware uses a simple **state machine (FSM)**.

States:

IDLE
READ_WEIGHT
IN_SET
END_SET

---

## FSM Flow

IDLE
Waiting for movement.

↓

READ_WEIGHT
Weight is read when stack is at rest at the bottom.

↓

IN_SET
Movement detected → counting repetitions.

↓

END_SET
Stack stops moving for a defined timeout.

---

# User Identification

User is identified via **NFC wristband (SomnaBand)**.

PN532 reads UID.

UID is mapped to a user in the system.

Example:

UID → user_id

---

# Data Recorded

For each set the system stores:

user_id
exercise_id
weight_kg
repetitions
ROM
timestamp

---

# Engineering Constraints

The system must follow these rules:

* no force sensors
* no electronics in the pin
* minimal number of sensors
* deterministic detection
* works on existing gym machines
* low cost hardware

---

# Long Term Goal

Create a **scalable system for gyms** where every machine can automatically track workouts.

Potential features:

automatic workout logging
training analytics
progress tracking
gym management system

---

# Prototype Platform

Firmware:

ESP32 (SomnaNode)
Arduino framework / PlatformIO

Sensors:

VL53L0X via I2C

---

# Development Philosophy

The system should remain:

simple
deterministic
low-cost
robust in gym environments

---

# Summary

SomnaCore is a **machine perception system for gym equipment** that automatically records workouts using distance sensors instead of cameras or force sensors.
