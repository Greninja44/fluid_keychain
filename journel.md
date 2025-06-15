---
title: "Fluid_keychain"
author: "greninja44"
description: "A fluid simulation keychain inspired  from https://youtu.be/jis1MC5Tm8k?si=dFwDLe44DeG1LyNZ"
created_at: "2025-06-15"
Total time spent: 6h
---
# June 15 Morning : Got the screen to work!
I did some research abt the pendant , i learnt about imu( inertial measurement unit) which basically gives us gyroscopic and accelro values which we calculate and get the position of the body.
using the gyro we integrate with an esp32 s3( we need wifi and bluetooth also to control it) ,   **instead of pendant how abt a keychain**

![image](https://github.com/user-attachments/assets/8f9479e1-95f5-42c1-bca6-c2dcd256a836)

so we are making something like this today 


Found out about esp32-s3 rgb matrix board  so using this im gonna make keychain
lol we have a problem i wanna make it handy so i need a lipo battery but the esp32 -s3 doesnt have an inbuilt charger
so lets research about that....... omg so i have to design a pcb for it ( bruhhh esp32 needs 5v and battery is 3.7  v , so gonna design a booster too)
---
its even compact perfect for my project
![image](https://github.com/user-attachments/assets/8ef8b9e0-9919-4b3f-b8b1-fe29b2a956ce)


DONE with my circuit design:( i mean schemtics lol)
![image](https://github.com/user-attachments/assets/d9ee0184-4f78-43a6-b977-155074360a0b)

---
![image](https://github.com/user-attachments/assets/e854b06e-9236-496c-a101-d4fbaec9ba64)
---

![image](https://github.com/user-attachments/assets/ea003b84-510f-48fa-8e18-667c9b739c6c)

![Screenshot 2025-06-15 111432](https://github.com/user-attachments/assets/7d132b98-a2c0-4591-8653-95d10a4c9954)

i will be 3d printing the case and would cut acrylic sheet and drill it in the back








