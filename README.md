# Profit-Maximization-in-Edge-Cloud-Storage-System-using-Dynamic-Space-Partitioning-with-Erasure-C

This repository contains the codebase and datasets for the proposed work on DSPE: Profit Maximization in Edge-Cloud Storage System using Dynamic Space Partitioning with
Erasure Code
---

## 📁 Directory Structure

### 1. `Dataset/`
Contains all the datasets used in the experiments.

  - `RequestTrace.txt`: Synthetic Dataset generated for simulation.
  - `netflix_input1`: Netflix Dataset.
  - `input1.txt`: Spotify dataset.

- **User Request/**
  - Each User Request contains 5 fields:
    - `arr_time`: arrival time of request.
    - `c_id`: 5% deviation for existing tasks.
    - `task_actual_10_percent_deviation.csv`: 10% deviation for existing tasks.
    - `task_new_5_percent_deviation.csv`: 5% deviation with new unpredicted tasks.
    - `task_new_10_percent_deviation.csv`: 10% deviation with new unpredicted tasks.



---

### 2. `Code/`
Contains the implementation of all scheduling algorithms.

#### └─ `offline/`
- `infinitebattery_offline.cpp`: Implements Algo 2 + Algo 4.
- `finitebattery_offline.cpp`: Implements Algo 2 + Algo 6.

#### └─ `online/`
- `online_solar_infinite_battery.cpp`: Implements Algo 2 + Algo 4 + Algo 5.
- `online_solar_finitebattery.cpp`: Implements Algo 2 + Algo 6 + Algo 5.

#### └─ `stateofart/`
Baseline methods from the literature:
- `NPEDF_*`: Non-preemptive Earliest Deadline First (finite/infinite battery).
- `EA_*`: Execute-on-Arrival methods.
- `asap_HUF_*`: As Soon As Possible – Highest Utilization First.
- `asap_LUF_*`: As Soon As Possible – Lowest Utilization First.

---

## 🔧 Usage

- **Compile the C++ files** with `g++`:
  ```bash
  g++ infinitebattery_offline.cpp -o infinitebattery
  ./infinitebattery
