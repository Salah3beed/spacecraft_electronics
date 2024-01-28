# Spacecraft Altimeter using Barometer
This repository is for a project conducted at the end of the "Spacecraft Electronics" Practical Lab Course. 

In the "resources" directory, one can easily find all the info about the project itself, both from the technical and non-technical side, including: 
1. Bill Of Materials
2. Gantt Chart
3. Block Functional Diagram
4. Schematics 
5. User Manual
6. Project Report

## Block Diagram of the Project
![Alt text](resources/images/block.png)
## Schematics 
![Alt text](resources/images/schematics.png)
## Altitude measurement 
There are two ways used in order to find the Altitude using the pressure and temprature readings from the device. 

In either formula: 

$$
\begin{align}
   P_0 =& \text{ reference pressure at sea level (1013.25hPa) \\
P =& \text{ measured pressure (Pa) from the sensor} \\
h=& \text{altitude (m)}
\end{align}
$$

### Hypsometric Formula

$$
h=\frac{\left( \left( \frac{P_{0}}{P}^{\frac{1}{5.2757}} \right)-1 \right)*(T+273.15)}{0.0065}
$$
### Barometric Formula 
$$
h=44330*(1-(P/P_0)^{\frac{1}{5.255}})
$$


## List of the Tests done to the Device

### Altitude measurement at ground level

### Altitude measurement at the first-floor level 
> This test was done in comparison to the ground floor measurement

### Altitude measurement at roof Level
> This test was done in comparison to the ground floor measurement

### Testing of our altimeter in motion to check how it accounts for wind with regard to pressure

### Testing of our altimeter under cold and warm conditions

### Comparison of altitude difference with the Bosch Laser Measurement to check for accuracy