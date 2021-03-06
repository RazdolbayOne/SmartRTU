#include "WgSockets.h"

#include <wiringPi.h> /*for socket stuff*/

#include<iostream> /*cout*/

#include "timetable.h"/*needed timetable*/
#include "Timer.h"/*StrNow()*/


WgSockets::WgSockets()
{
    wiringPiSetup();
    for (int i = 0; i < SOCKETS_COUNT; ++i)
    {
        current_states_[i] = false;
        pinMode(SOCKETS_PINS[i], OUTPUT);
    }
    std::cout << StrNow() << "\t" << "WgSockets widget object was created\n";
}

WgSockets::~WgSockets()
{
    std::cout << StrNow() << "\t" << "WgSockets widget object was deleted\n";
}

bool WgSockets::update()
{
    bool state=false;
    for (int i = 0; i < SOCKETS_COUNT; ++i)
    {
        state = timetable->GetCurrentSocketState(i + 1);
        digitalWrite(SOCKETS_PINS[i], state ? HIGH : LOW);
        if (state != current_states_[i])
        {
            std::cout<<StrNow()<<"\tSocket "
            <<i + 1<<" is turned "<<(state ? "on" : "off")<<" \n";
            current_states_[i] = state;
        }
    }
    return false;
}

void WgSockets::render(){
    
}