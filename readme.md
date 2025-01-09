# GuardianCall : Multipurposed elctronic pager IoT device

GuardianCall, is a electronic pager that is developed for the ease of caretakers and bed-ridden inidividuals. This device is intended for both single patient - single caretaker and many patients - many caretakers situations.

## Market pitch

Designed by [Venumi Gihansa Gunasekara](https://github.com/venumigihansa)
![Market Pitch](./Assets/marketpitch_techwizards.png)


## System Block Diagram

There was a major bug in the ESP32 libraries (even remaining unresolved for years in the forums), not operating WiFi and ESP Now at the same time even when the wifi channels are configured. There was no errors or execeptions throwed so we had to try different appoaches.
So after 3 system desgin iterations and considering their pros and cons we implemented our device using the following architecture.

![Block diagram](./Assets/Drawing.jpeg)

## PCB Design

### Main Unit

![Main PCB](./Assets/presentation_tech%20wizards-10.png)


### Button

Desgined by [Imansha Manuka Priyanjana](https://github.com/imansha321?tab=followers)
![Button PCB](./Assets/presentation_tech%20wizards-11.png)

<div style="display: flex; justify-content: space-around; text-align: center;">
    <img src="Assets/20240614_232553.jpg" alt="Image 3" width="200" style="margin: 20px ;">
    <img src="Assets/20240614_232625.jpg" alt="Image 3" width="200" style="margin: 20px ;">
</div>

## Enclosure Design

Designed by [Induwara Illukumbura](https://github.com/induwara-iluk)
![Main unit enclosure](/Assets/presentation_tech%20wizards-12.png)
![Button enclosure](/Assets/presentation_tech%20wizards-13.png)

## App overview

App was designed scalability and giving best user experience in mind. 

![UI of the android app](./Assets/GuardianCall_Manual-2.png)
Image credit [Venumi Gihansa Gunasekara](https://github.com/venumigihansa)

For the backend we used firebase and it's BaaS tools and the realtime database was also structured for scalabiluty making overall system architecture cabale of adding any number of new devices, users and any number of buttons for each device (but from the IoT device's firmware it is limite to 100)

![Structure of the Realtime Database](./Assets/Screenshot%20(18).png)

Refer below for the source files and distributions

[Original Girhub Repo for the App](https://github.com/KiranGunathilaka/guardian-call-app)

To securely register a caretaker with a device, following sign-up cards are needed.

![Sign-Up Card](/Assets/20240831_091508.jpg)

## Actual Device

![Image of the device and pcbs](Assets/presentation_tech%20wizards-20.png)

## Some videos in the prototyping phase

[Video 1, before the final circuit implmentation](Assets/VID-20240509-WA0002.mp4)

[Video 2, before using ESP01 for the button, for the proposal evalution](/Assets/VID-20240501-WA0005.mp4)

Furthur updates to the main unit is possible via a USB to TTL converter, so it is possible to make furthur improvements to the hardware after assembling.

For more information, refer to the project report, presentation pdfs above.

