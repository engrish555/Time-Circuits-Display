/*
 * -------------------------------------------------------------------
 * CircuitSetup.us Time Circuits Display
 * Code adapted from Marmoset Electronics 
 * https://www.marmosetelectronics.com/time-circuits-clock
 * by John Monaco
 * -------------------------------------------------------------------
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "tc_menus.h"

int displayNum;                                           // selected display
uint8_t autoInterval = 1;                                 // array element of autoTimeIntervals[], set's time between automatically displayed times
const uint8_t autoTimeIntervals[5] = {0, 5, 15, 30, 60};  // time options, first must be 0, this is the off option.
uint8_t timeout = 0;  // for tracking idle time in menus, reset to 0 on each button press

clockDisplay* displaySet;  // the display being updated during setting

Preferences prefAutoInt; //to save preferences in namespaces

void menu_setup() {
    
    //Set up namespace
    prefAutoInt.begin("autoint_pref", false);
}

void enter_menu() {
    Serial.print("Menu");
    // Enter button held, get display
    displayNum = 0;  // this is really setting function number

    // Load the times
    destinationTime.load();  // this wipes out currently displayed times if on auto,
                             // brightness may do a save, so need stored times any
    departedTime.load();

    displayHighlight(displayNum);

    timeout = 0;  // Reset on each press
}

void displaySelect(int& number) {
    Serial.println("Display Select");
    // Selects which display to update 
    if (isEnterKeyPressed && menuFlag) {
        isEnterKeyPressed = false;
        number++;
        timeout = 0;  // button pressed, reset timeout
        if (number == (6)) number = 0;
        displayHighlight(number);   // Show only the selected display and set pointer to it
    }
}  // displaySelect

void displayHighlight(int& number) {
    Serial.println("Display Highlight");
    // Turns on the currently selected display during setting
    // Also sets the displaySet pointer to the display being updated

    switch (number) {
        case 0:  // Destination Time
            destinationTime.on();
            destinationTime.setColon(false);
            destinationTime.show();
            presentTime.off();
            departedTime.off();
            displaySet = &destinationTime;
            break;
        case 1:  // Presetnt Time
            destinationTime.off();
            presentTime.on();
            presentTime.setColon(false);
            presentTime.show();
            departedTime.off();
            displaySet = &presentTime;
            break;
        case 2:  // Last Time Departed
            destinationTime.off();
            presentTime.off();
            departedTime.on();
            departedTime.setColon(false);
            departedTime.show();
            displaySet = &departedTime;
            break;
        case 3:  // auto enable
            destinationTime.showOnlySettingVal(
                "PRE", -1, true);  // display end, no numbers, clear rest of screen
            presentTime.showOnlySettingVal(
                "SET", -1, true);  // display end, no numbers, clear rest of screen
            destinationTime.on();
            presentTime.on();
            // destinationTime.setColon(false);
            // destinationTime.show();
            // presentTime.off();
            departedTime.off();
            displaySet = NULL;
            break;
        case 4:  // Brt
            destinationTime.showOnlySettingVal(
                "BRI", -1, true);  // display end, no numbers, clear rest of screen
            presentTime.showOnlySettingVal(
                "GHT", -1, true);  // display end, no numbers, clear rest of screen
            destinationTime.on();
            presentTime.on();
            // destinationTime.setColon(false);
            // destinationTime.show();
            // presentTime.off();
            departedTime.off();
            displaySet = NULL;
            break;
        case 5:  // end
            destinationTime.showOnlySettingVal(
                "END", -1, true);  // display end, no numbers, clear rest of screen
            destinationTime.on();
            destinationTime.setColon(false);
            // destinationTime.show();
            presentTime.off();
            departedTime.off();
            displaySet = NULL;
            break;
    }
}

void fieldSelect() {
    // called in tc_keypad
    if (!checkTimeOut() && menuFlag) {
        isEnterKeyPressed = false;
        // in menu mode enter key will now go to next field
        displaySelect(displayNum);
        delay(50);

        // Have a selected display pointed to by displaySet if displayNum 0-2
        if (displayNum <= 2) {  // Doing display times
            uint16_t yearSet;   // hold new times that will be set, all need to be same
                                // type since they're passed by ref
            uint16_t monthSet;
            uint16_t daySet;
            uint16_t minSet;
            uint16_t hourSet;

            if (displaySet->isRTC() == 1) {  // this is the RTC, get the current time.
                DateTime currentTime = rtc.now();

                yearSet = currentTime.year();
                monthSet = currentTime.month();
                daySet = currentTime.day();
                minSet = currentTime.minute();
                hourSet = currentTime.hour();
            } else {
                // non RTC, get the time info from the object
                yearSet = displaySet->getYear();
                monthSet = displaySet->getMonth();
                daySet = displaySet->getDay();
                minSet = displaySet->getMinute();
                hourSet = displaySet->getHour();
            }

            // Get year
            /*
            Fields
            0 - month
            1 - day
            2 - year
            3 - hour
            4 - min
            */
            setUpdate(yearSet, 2);  // show only the year field - value,display field
            if (!checkTimeOut() && isEnterKeyPressed && menuFlag) {
                isEnterKeyPressed = false;
                setField(yearSet, 2, 0, 0);  // number to adjust, field
            }

            // Get month
            setUpdate(monthSet, 0);
            if (!checkTimeOut() && isEnterKeyPressed && menuFlag) {
                isEnterKeyPressed = false;
                setField(monthSet, 0, 0, 0);  // setting, which display, which field, starting from left
            }

            // Get day
            setUpdate(daySet, 1);
            if (!checkTimeOut() && isEnterKeyPressed && menuFlag) {
                isEnterKeyPressed = false;
                setField(daySet, 1, yearSet, monthSet);  // this depends on the month and year
            }

            // Get hour
            setUpdate(hourSet, 3);
            if (!checkTimeOut() && isEnterKeyPressed && menuFlag) {
                isEnterKeyPressed = false;
                setField(hourSet, 3, 0, 0);
            }

            // Get minute
            setUpdate(minSet, 4);
            if (!checkTimeOut() && isEnterKeyPressed && menuFlag) {
                isEnterKeyPressed = false;
                setField(minSet, 4, 0, 0);
            }

            // Have new date & time
            if (!checkTimeOut()) {               // Do nothing if there was a timeout waiting for
                                                 // button presses
                if (displaySet->isRTC() == 1) {  // this is the RTC display, set the RTC as quickly as
                                                 // possible, as soon as set button pressed
                    rtc.adjust(DateTime(yearSet, monthSet, daySet, hourSet, minSet, 0));
                } else {                 // Not rtc, setting a static display, turn off auto changing
                    autoInterval = 0;    // set to the 0 interval, disables auto
                    saveAutoInterval();  // save it
                }
                ///////////////////////////////////}
                displaySet->showOnlySave();  // Show a save message

                // wait to do this, update rtc as fast as possible
                // update the object
                displaySet->setMonth(monthSet);
                displaySet->setDay(daySet);
                displaySet->setYear(yearSet);
                displaySet->setHour(hourSet);
                displaySet->setMinute(minSet);
                displaySet->save();  // save to eeprom
                delay(1000);
            }  // checktimeout

            // displaySet->off(); // turn it off, prevents minute from showing for 1
            // second

        } else if (displayNum == 4) {
            // Get brightness settings
            if (!checkTimeOut()) doGetBrightness(&destinationTime);
            if (!checkTimeOut()) doGetBrightness(&presentTime);
            if (!checkTimeOut()) doGetBrightness(&departedTime);
            allOff();
        } else if (displayNum == 3) {  // auto times
            doGetAutoTimes();
        } else {  // if(displayNum < 3)
            // END option, turn off displays, do nothing.
            allOff();
        }
        // Return dest/dept displays to where they should be
        // Menuing system wipes out auto times
        if (autoTimeIntervals[autoInterval] == 0) {
            destinationTime.load();
            departedTime.load();
        } else {
            destinationTime.setFromStruct(&destinationTimes[autoTime]);
            departedTime.setFromStruct(&departedTimes[autoTime]);
        }

        // Done, turn off displays, then turn on
        allOff();

        delay(1000);
        Serial.println("Update Present Time 3");
        presentTime.setDateTime(rtc.now());  // Set the current time in the display,
                                             // set times are 2+ seconds old

        // all displays on and show
        animate();  // show all with month showing last
                    // then = millis(); // start count to prevent double animate if it's been
                    // too long
    }
}
void setUpdate(uint16_t& number, int field) {
    Serial.println("Set Update");
    // Shows only the field being updated
    switch (field) {
        case 0:
            displaySet->showOnlyMonth(number);
            break;
        case 1:
            displaySet->showOnlyDay(number);
            break;
        case 2:
            displaySet->showOnlyYear(number);
            break;
        case 3:
            displaySet->showOnlyHour(number);
            break;
        case 4:
            displaySet->showOnlyMinute(number);
            break;
    }
}

void adjustBrightness(clockDisplay* displaySet) {
    Serial.println("Bright button");
    int8_t number = displaySet->getBrightness();
    // Store actual brightness as 0 to 15, but only show 5 levels, starting at 3
    // Update the clockDisplay and show the value
    if (isEnterKeyPressed && menuFlag) {
        isEnterKeyPressed = false;
        number = number + 3;
        timeout = 0;  // button pressed, reset timeout
        if (number > 15) number = 3;
        displaySet->setBrightness(number);
        displaySet->showOnlySettingVal("LVL", number / 3, false);
    }
}

void setField(uint16_t& number, uint8_t field, int year = 0, int month = 0) {
    // number - a value we're updating
    // displayNum - the display number being modified
    // field - field being modified, this will be displayed on displayNum as it
    // is updated

    // displayNum:
    //  0 Destination Time
    //  1 Present Time
    //  2 Last Time Departed

    // field: 0 month, 1 day, 2 year, 3 hour, 4 minute
    int upperLimit;
    int lowerLimit;

    switch (field) {
        case 0:  // month
            upperLimit = 12;
            lowerLimit = 1;
            break;
        case 1:  // day
            upperLimit = daysInMonth(month, year);
            lowerLimit = 1;
            break;
        case 2:  // year
            if (displaySet->isRTC() == 1) {  // year is limited for RTC, RTC is on display 1
                upperLimit = 2099;
                lowerLimit = 2020;
            } else {
                upperLimit = 9999;
                lowerLimit = 0;
            }
            break;
        case 3:  // hour
            upperLimit = 23;
            lowerLimit = 0;
            break;
        case 4:  // minute
            upperLimit = 59;
            lowerLimit = 0;
            break;
    }

    unsigned long start = millis();

    if (isEnterKeyPressed && menuFlag) {
        isEnterKeyPressed = false;
        Serial.println("Enter Next...");

        //TODO: modify for number input
        timeout = 0;
        // Instantly respond to first press
        start = millis();
        
        if (number == (upperLimit + 1)) number = lowerLimit;
        setUpdate(number, field);
        
    }
}  

bool loadAutoInterval() {
    Serial.println("Load Auto Interval");
    // loads the autoInterval
    // first location has autoInterval
    autoInterval = prefAutoInt.getUChar(AUTOINTERVAL_ADDR);
    if (autoInterval > 5) {
        autoInterval = 1;  // default to first valid option if invalid read
        Serial.println("BAD AUTO INT");
        return false;
    }
    return true;
}

void saveAutoInterval() {
    Serial.println("Save Auto Interval");
    // saves autoInterval
    prefAutoInt.putUChar(AUTOINTERVAL_ADDR, autoInterval);
}

void putAutoInt(int position) {
    prefAutoInt.putUChar(AUTOINTERVAL_ADDR, position);
}

void autoTimesButton() {
    Serial.println("Auto Times Button");
    if (get_key() == '2') {
        autoInterval++;
        timeout = 0;  // button pressed, reset timeout
        if (autoInterval > 4) autoInterval = 0;

        destinationTime.showOnlySettingVal("INT", autoTimeIntervals[autoInterval], true);

        if (autoTimeIntervals[autoInterval] == 0) {
            presentTime.showOnlySettingVal("CUS", -1, true);
            departedTime.showOnlySettingVal("TOM", -1, true);
            departedTime.on();
        } else {
            departedTime.off();
            presentTime.showOnlySettingVal("MIN", -1, true);
        }
    }
}

void doGetBrightness(clockDisplay* displaySet) {
    Serial.println("Get Brightness");
    // Adjust the brightness of the selected displaySet and save
    allLampTest();  // turn on all the segments
    displaySet->showOnlySettingVal("LVL", displaySet->getBrightness() / 3, false);

    while (!checkTimeOut() && isEnterKeyPressed && menuFlag) {
        isEnterKeyPressed = false;
        adjustBrightness(displaySet);
        delay(100);
    }

    if (!checkTimeOut()) {  // only if there wasn't a timeout
        displaySet->save();
        displaySet->showOnlySave();
        delay(1000);
        
        allLampTest();  // turn on all the segments
    }
}

void animate() {
    // Show month after a short delay
    destinationTime.showAnimate1();
    presentTime.showAnimate1();
    departedTime.showAnimate1();
    delay(80);
    destinationTime.showAnimate2();
    presentTime.showAnimate2();
    departedTime.showAnimate2();
}

void allLampTest() {
    // Activate lamp test on all displays and turn on
    destinationTime.on();
    presentTime.on();
    departedTime.on();
    destinationTime.lampTest();
    presentTime.lampTest();
    departedTime.lampTest();
}

void allOff() {
    destinationTime.off();
    presentTime.off();
    departedTime.off();
}