# GEMSTAR UE_Lidar_Feature

[![DLR-TS](https://img.shields.io/badge/developed%20by-DLR%20TS-orange.svg?style=flat&colorA=E1523D&colorB=007D8A)](https://www.dlr.de/en/ts/about-us/the-institute-of-transportation-systems)


The Lidar Feature adds a Lidar to Unreal.

![Lidar Visualization example](Documentation/Lidar_Debug.png)


## Output
- OSI Sensor View

# Getting started

First clone the GEMSTAR Interfaces plugin inside the Plugins directory and the UE_Osivisualizer_Feature inside Plugins/Gamefeatures.

Add the plugin to your Unreal project:
```sh
mkdir Plugins/Gamefeatures
cd Plugins/Gamefeatures

git clone <this repo>
```
or:
```sh
git submodule add <this repo>
```

# Configuration

![Lidar Configuration](Documentation/Configuration.png)

**Debug Ray Traces** Show debug lines\
**Debug Reflections** Show collision points\
**Debug Normals** show normal vectors of mesh at collision point

# Test

![Lidar Visualization example](Documentation/Lidar_Visualization.png)

A test map is provided with a moving object that moves through a course of different objects.\
The test map also contains the manager for the pointcloud visualization to show the impact of the intensity model.

---------------
**Contact:** opensource-ts@dlr.de

This project uses the Open Simulation Interface ([OSI](https://github.com/OpenSimulationInterface/open-simulation-interface)), which is licensed under the
Mozilla Public License 2.0.

Inspired by: [MetaLidar](https://github.com/metabotics-ai/MetaLidar)