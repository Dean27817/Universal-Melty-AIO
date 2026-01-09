#include "Arduino.h"
#include "reciver.h"

//sets up the input for the reciver
void Reciver::begin()
{
     //connect to the controller
     this -> xbox.begin();

     //wait for connection
     while( !xbox.isConnected() && !xbox.isWaitingForFirstNotification() )
     {
          xbox.onLoop();
          delay( 100 );
     }

}

//reads the input from the IBus reciver
void Reciver::loop()
{
     this -> xbox.onLoop();
     if( xbox.isConnected() )
     {
     input.x = xbox.xboxNotif.joyRHori ;
     input.y = xbox.xboxNotif.joyRVert ;
     input.btnLB = xbox.xboxNotif.btnLB ;
     input.btnUp = xbox.xboxNotif.btnDirUp ;
     input.btnDown = xbox.xboxNotif.btnDirDown ;
     }

     else
     {
     input.x = 0;
     input.y = 0;
     input.btnLB = 0;
     input.btnUp = 0;
     input.btnDown = 0;
     }
}