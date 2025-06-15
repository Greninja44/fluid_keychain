---
title: "Fluid_keychain"
author: "greninja44"
description: "A fluid simulation keychain inspired by [this video](https://youtu.be/jis1MC5Tm8k?si=dFwDLe44DeG1LyNZ)"
created_at: "2025-06-15"
Total time spent: 6h
---

# Project Summary

**Fluid_keychain** is a compact, interactive keychain that uses an **ESP32-S3 RGB matrix** to simulate motion-based fluid effects. It’s inspired by fluid pendants and driven by an **IMU sensor**, letting you visualize your movement in a creative way. It includes **wireless capabilities**, a custom **battery-powered PCB**, and a **3D printed case** with an acrylic window.

---

# What I Did (Full Flow)

###  June 15 – Morning Vibes       ( ~~~~~~2.5-3 hours)

Started off the day vibing with some weeknd music( weeknd is the goat), and landed on this fluid pendant video — boom, brain lit up . Looked into IMUs, and learned they give you values from gyroscope and accelerometer. Basically, if you move the device, you get orientation data — super handy for simulating movement. Was originally thinking pendant-style, but then I was like — "bro, pendant is a bit couple vibe, let’s go with a keychain. Much cooler, more carryable."

Then I found the **ESP32-S3 RGB dev board** by Waveshare. Mad compact, and it has RGBs? Insta-buy in my head. But there was a catch — no battery charging built-in 😭. So I had to dive into power circuits. Learnt about the **TP4056 module** for charging and the **MT3608** to boost 3.7V to 5V. ESP32 needs solid 5V, so yeah... time to design a PCB too 💀.

---

###  Afternoon to Evening (This is Where It Gets Real)   ( ~~~~~~2 to 3.5 hours)

Started building the schematic on Easyeda — pretty chill once you get the hang of it(i was consoling myself by saying u have designed a usb hub before this aint any shit) I added the charger, booster, a slide switch for power, some caps, and a diode for safety. and everything’s routed properly now. Here's how the schematic ended up:

![Schematic](https://github.com/user-attachments/assets/d9ee0184-4f78-43a6-b977-155074360a0b)
BEST MOMENT :
Then, I jumped into designing the 3D case. Had to keep it tight ‘cause it’s a keychain, but also leave room for the battery and board. Designed mounts for the screen, USB-C cutout, and a back panel where I’ll drill in a clear acrylic sheet — proper aesthetic.

![Case design 1](https://github.com/user-attachments/assets/8ef8b9e0-9919-4b3f-b8b1-fe29b2a956ce)  
![Case with screen](https://github.com/user-attachments/assets/ea003b84-510f-48fa-8e18-667c9b739c6c)  
![Case Screenshot](https://github.com/user-attachments/assets/7d132b98-a2c0-4591-8653-95d10a4c9954)





#  Code Coming Soon

I will upload the firmware and UI code after testing on the final hardware version.

## Coding Start (Just Started 😅) - 9:30 PM
Once I wrapped up the schematic and sorted out all the parts, I started looking into how to actually control the RGB matrix. I found this FastLED library — and bro, it’s so cooooool. Everyone’s( most people on reddit) using it for WS2812 LEDs, and the code looks pretty simple. I haven’t uploaded or tested anything yet, but from the examples I checked out, it seems super easy to get stuff running like gradients and rainbow animations.Then I also found Wowki Animator — it's this online tool where you can make LED animations by just drawing frames, and it literally gives you Arduino code for it. Looked mad cool, so I bookmarked it for later. Plan is to make some fluid-style animations with it once the hardware’s ready.   update people: so i was working on wowki animater i tried making animation on my oled screen paired up with esp32
![image](https://github.com/user-attachments/assets/ee8f3593-0c62-4131-8ee9-e426bdcc03b9)


Haven’t tested anything yet because I’m still waiting to finish up the PCB and solder the board, but ngl I’m hyped to see it in action. Feels like once it’s ready, the whole keychain’s gonna light up like Diwali 😭🔥.
---

@greninja44  
> Still a work-in-progress, but this is gonna be awesome 😎  
