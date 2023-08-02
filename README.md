# Gradys Simulations
Simulations from Project GrADyS on OMNET++ and INET framework

## Introduction

This is a repository for the simulation framework developed for the GrADyS project. This framework allows the simulation of interconnected network nodes and the implementation of UAV swarms and sensor coordination strategies with the objective of managing these autonomous UAV swarms to collect sensor data on the field autonomously and efficiently. Gradys Simulations MAVSIMNET a simulation framework for the OMNET++ discrete event simulator based on Ardupilot's Software In The Loop (SITL) simulator. It allows you to enrich OMNeT++ and INET's simulations with realistic mobility models for the mobile nodes in your network. 

It works by spawning SITL instances that provide a physical simulation of the vehicle's behaviour and connecting them to the mobility classes in your simulation. A user can transparently use these mobility modules without worrying about the details of the communication and messages being exchanged between the network simulator and the SITL instances and a developer can use the strong interface provided in the project's base mobility class to implement his own mobility modules.

![Dadca protocol showcase](./gradys_simulations_docs/assets/dadca_showcase.gif)

## Documentation
More information can be found on the [project's official documentation](https://brunoolivieri.github.io/gradys-simulations/)