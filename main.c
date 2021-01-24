
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <time.h>

// XDCtools Header files
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

/* TI-RTOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Swi.h>
#include <ti/sysbios/knl/Queue.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Idle.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/drivers/GPIO.h>
#include <ti/net/http/httpcli.h>

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "Board.h"

#include <sys/socket.h>
#include <arpa/inet.h>


#define TASKSTACKSIZE     4096
#define NTP_PORT            37
#define INCOMING_PORT     5030
#define SOCKETTEST_IP   "132.163.96.2"
//"132.163.96.2"
//"128.138.140.44"
//132.163.96.4

extern Mailbox_Handle mailbox0;
extern Semaphore_Handle semaphore0;
extern Semaphore_Handle semaphore1;
extern Event_Handle event0;
extern Swi_Handle swi0;
unsigned long int timestamps;
int year, month, day, hour, minutes, seconds;
char motorFunction[30];
char array1[4][10];
static int j=0;
char function[30];
void placement();
void converTime();
/*
 *  ======== printError ========
 */
void printError(char *errString, int code)
{
    System_printf("Error! code = %d, desc = %s\n", code, errString);
    BIOS_exit(code);
}
//Post SWI one per second
Void timerISR(UArg arg1){
        Swi_post(swi0);

}
//Update time according second, minutes
Void swifunc(UArg arg1, UArg arg2){
      seconds++;
      if(seconds==60){
          seconds=0;
          minutes++;
      }
      if(minutes==60){
                minutes=0;
                hour++;
      }

}
//Waits for data on which motor will run and how many seconds it will run
Void task0Function(UArg arg1, UArg arg2)
{
    int i=0;
    while(1) {
        for(i=0;i<2;i++) {     // 2 values will be retrieved
           // wait for the mailbox until the buffer is full
            Mailbox_pend(mailbox0, function, BIOS_WAIT_FOREVER);
            System_printf("\nMailbox: %s", function);
            System_flush();
            placement();

        }
        Event_post(event0, Event_Id_00);
    }
}
//Waits for motor data and EXEC command. When two data come, dc motors are running according to data.
Void task1Function(UArg arg1, UArg arg2)
{
    int time=0;
    while(1){
        Event_pend(event0, Event_Id_00 + Event_Id_01, Event_Id_NONE, BIOS_WAIT_FOREVER);

        System_printf("\nTask1 event captured\n");
        System_flush();

        if (!strcmp(array1[0], "RIGHT")){
                time= (atoi(array1[1]))*1000;
                System_printf("\ntime: %d \n", time);
                System_flush();
                GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_2,GPIO_PIN_2);
                Task_sleep(time);
                GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_2,0);
                if(!strcmp(array1[2], "LEFT")){
                    time= atoi(array1[3])*1000;
                    System_printf("\ntime: %d \n", time);
                    System_flush();
                    GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_3,GPIO_PIN_3);
                    Task_sleep(time);
                    GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_3,0);
                }
            }
       if (!strcmp(array1[0], "LEFT")){
               System_printf("Array1 is left \n");
               System_flush();
               time= atoi(array1[1])*1000;
               System_printf("\ntime: %d \n", time);
               System_flush();
               GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_3,GPIO_PIN_3);
               Task_sleep(time);
               GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_3,0);
               if(!strcmp(array1[2], "RIGHT")){
                  time= atoi(array1[3])*1000;
                  GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_2,GPIO_PIN_2);
                  Task_sleep(time);
                  System_printf("\ntime1: %d \n", time);
                  System_flush();
                  GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_2,0);
               }
      }
}


}
//It parses incoming motor data and assigns data to the array for using
void placement(){
    char *k;
    int l;

    k=strtok (function," ");
    while (k!= NULL){
        strcpy(array1[j], k);
        System_printf("/nArray1[%d]: %s",j,array1[j]);
        System_flush();
        k = strtok (NULL, " ");
        j++;
    }
    if(j==4){
        for(l=0;l<4;l++){
            System_printf("/nLast Array1[%d]: %s",l,array1[l]);
            System_flush();
        }
        j=0;
    }
}
//It initiliazes the GPIO pins to be used
void pinInitialize(){

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL)){}
    GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_2);
    GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_3);
}
//Convert timestamp to date(year,month,day,hour,minutes,seconds)
void convertTime(){
        time_t currentSeconds ;
        struct tm *currentDate;
        currentSeconds = (time_t)timestamps;
        currentDate = localtime(&currentSeconds);
        month=currentDate->tm_mon+1;
        year=currentDate->tm_year+1900;
        day=currentDate->tm_mday;
        hour=currentDate->tm_hour;
        minutes=currentDate->tm_min;
        seconds= currentDate->tm_sec;

}
//It creates TCP/IP client for connecting to NTP server and receives time from the NTP server.
bool recvTimefromNtpServer(char *serverIP, int serverPort)
{
    System_printf("\nRecvTimefromNtpServer is runnig");
    System_flush();
    int sockfd, connStat;
    bool retval=false;
    struct sockaddr_in serverAddr;
    char timedata[4];

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1) {
        System_printf("Socket not created");
        close(sockfd);
        return false;
    }
    memset(&serverAddr, 0, sizeof(serverAddr));  // clear serverAddr structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);     // convert port # to network order
    inet_pton(AF_INET, serverIP, &(serverAddr.sin_addr.s_addr));

    connStat = connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if(connStat < 0) {
        System_printf("recvTimefromNtpServer::Error while connecting to server\n");
    }
    else {
        recv(sockfd,  timedata ,4,0);
        timestamps= timedata[0]*16777216 +  timedata[1]*65536 + timedata[2]*256 + timedata[3]+3*60*60;
        System_printf("\nTime :%lu", timestamps);
        System_flush();
        convertTime();
    }

    System_flush();
    close(sockfd);
    return retval;
}
//Calling the function connecting to the NTP server
Void clientSocketTask(UArg arg0, UArg arg1)
{
    System_printf("\nClientSocketTask is runnig");
    System_flush();
        if(recvTimefromNtpServer(SOCKETTEST_IP, NTP_PORT)) {
            System_printf("clientSocketTask:: Time is received from the NTP server\n");
            System_flush();
        }

}
//It converts time data to string
void getTimeStr(char *string){
    sprintf(string,"%02d-%02d-%02d-%02d-%02d-%02d",day,month,year,hour,minutes,seconds);
}
//It creates TCP/IP server and runs according to the data coming from the client
Void serverSocketTask(UArg arg0, UArg arg1)
{
    int serverfd, new_socket, valread, len;
    struct sockaddr_in serverAddr, clientAddr;
    char buffer[30];
    char outstr[30], timestr[30];
    bool quit_protocol;

    serverfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverfd == -1) {
        System_printf("serverSocketTask::Socket not created.. quiting the task.\n");
        return;     // we just quit the tasks. nothing else to do.
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(INCOMING_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Attaching socket to the port
    //
    if (bind(serverfd, (struct sockaddr *)&serverAddr,  sizeof(serverAddr))<0) {
         System_printf("serverSocketTask::bind failed..\n");

         // nothing else to do, since without bind nothing else works
         // we need to terminate the task
         return;
    }
    if (listen(serverfd, 3) < 0) {

        System_printf("serverSocketTask::listen() failed\n");
        // nothing else to do, since without bind nothing else works
        // we need to terminate the task
        return;
    }

    while(1) {
        len = sizeof(clientAddr);
        if ((new_socket = accept(serverfd, (struct sockaddr *)&clientAddr, &len))<0) {
            System_printf("serverSocketTask::accept() failed\n");
            continue;               // get back to the beginning of the while loop
        }

        System_printf("Accepted connection\n"); // IP address is in clientAddr.sin_addr
        System_flush();

        // task while loop
        quit_protocol = false;
        do {
            // let's receive data string
            if((valread = recv(new_socket, buffer, 10, 0))<0) {

                // there is an error. Let's terminate the connection and get out of the loop
                //
                close(new_socket);
                break;
            }
            //processing according to the received string
            buffer[10]=0;
            char *p;
            char* array[2];
            strcpy(motorFunction,buffer);
            p=strtok (motorFunction," ");
            int i=0;
            while (p!= NULL){
               array[i] = p;
               printf ("%s\n",p);
               p = strtok (NULL, " ");
              i++;
            }

            if(valread<10) buffer[valread]=0;

            System_printf("message received: %s\n", buffer);
           //System_printf("message received: %s\n", motorFunction);
            System_flush();

            if(!strcmp(buffer, "HELLO")) {
                strcpy(outstr,"OK 200\n");
                send(new_socket , outstr , strlen(outstr) , 0);
                System_printf("Server <-- OK 200\n");
            }
            else if(!strcmp(buffer, "GETTIME")) {
                getTimeStr(timestr);
                strcpy(outstr, "OK ");
                strcat(outstr, timestr);
                strcat(outstr, "\n");
                send(new_socket , outstr , strlen(outstr) , 0);
            }
            else if(!strcmp(array[0], "LEFT")) {
                strcpy(outstr,"WAITING 200\n");
                System_printf("Buffer: %s",buffer);
                System_printf("Motor Function: %s",motorFunction);
                Mailbox_post(mailbox0, buffer, BIOS_NO_WAIT);
                send(new_socket , outstr , strlen(outstr) , 0);
            }
            else if(!strcmp(array[0], "RIGHT")) {
                strcpy(outstr,"WAITING 200\n");
                System_printf("Buffer: %s",buffer);
                System_printf("Motor Function: %s",motorFunction);
                Mailbox_post(mailbox0, buffer, BIOS_NO_WAIT);
                send(new_socket , outstr , strlen(outstr) , 0);
            }
            else if(!strcmp(buffer, "EXEC")) {
                strcpy(outstr,"PROCCESING 200\n");
                send(new_socket , outstr , strlen(outstr) , 0);
                Event_post(event0, Event_Id_01);
            }
            else if(!strcmp(buffer, "QUIT")) {

                strcpy(outstr, "BYE 200");
                send(new_socket , outstr , strlen(outstr) , 0);
                quit_protocol = true;     // it will allow us to get out of while loop
            }
        }
        while(!quit_protocol);

        System_flush();
        close(new_socket);
    }

    close(serverfd);
    return;
}


//  This function is called when IP Addr is added or deleted. When  IP Addr is added, task function is created.
void netIPAddrHook(unsigned int IPAddr, unsigned int IfIdx, unsigned int fAdd)
{
    // Create a HTTP task when the IP address is added
       static Task_Handle taskHandle1,taskHandle2;
       Task_Params taskParams;
       Error_Block eb;

           // Create a HTTP task when the IP address is added
       if (fAdd && !taskHandle1 && !taskHandle2 ) {
             Error_init(&eb);

      Task_Params_init(&taskParams);
      taskParams.stackSize = TASKSTACKSIZE;
      taskParams.priority = 1;
      taskHandle1 = Task_create((Task_FuncPtr)serverSocketTask, &taskParams, &eb);

      Task_Params_init(&taskParams);
      taskParams.stackSize = TASKSTACKSIZE;
      taskParams.priority = 1;
      taskHandle2 = Task_create((Task_FuncPtr)clientSocketTask, &taskParams, &eb);


       if (taskHandle1 == NULL || taskHandle2== NULL ) {
           printError("netIPAddrHook: Failed to create HTTP and Socket Tasks\n", -1);
       }
    }
}

int main(void)
{
    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    Board_initEMAC();
    pinInitialize();

    /* Turn on user LED */
    GPIO_write(Board_LED0, Board_LED_ON);

    System_printf("Starting the HTTP GET example\nSystem provider is set to "
            "SysMin. Halt the target to view any SysMin contents in ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();


    /* Start BIOS */
    BIOS_start();

    return (0);
}
