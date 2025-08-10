# Profit-Maximization-in-Edge-Cloud-Storage-System-using-Dynamic-Space-Partitioning-with-Erasure-C

This repository contains the codebase and datasets for the proposed work on DSPE: Profit Maximization in Edge-Cloud Storage System using Dynamic Space Partitioning with
Erasure Code
---

## 📁 Directory Structure

### 1. `Datasets/`
Contains all the datasets used in the experiments.

  - `RequestTrace_0R.txt`: Dataset generated from DataGeneration.cpp
  - `netflix_input1`: Netflix Dataset.
  - `input1.txt`: Spotify dataset.

- **User Request/**
  - Each User Request contains 5 fields:
    - `arr_time`: arrival time of request.
    - `X_id`: Content id requested.
    - `deadline`: deadline for the request.
    - `AP`: Access Point on which request arrived .
    - `Profit`: profit from the request.

---

### 2. `Code/`
Contains the implementation of all algorithms.

- ` netflix.cpp`: for running code on real-life dataset i.e. for both netflix and spotify dataset.
-   Load_Requests function is used to read dataset. Change file_name to run on specific file.

-> " DSP.cpp ": Split No Erasure method. Read/generate real-life or synthetic dataset to run on sepefic dataset.
-> " E and DSPE.cpp ": No Split with Erasure and Split with Erasure method. 

-> E and DSPE is method in which we have tried to both
	- (1) split into public and private section only.
	- (2) split into public and private + spitting private section also (DSPE).
	- used function nospilt for the first method.
	- uesd function split for the second method. 
	
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
