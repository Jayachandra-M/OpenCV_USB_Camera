/*Author : e-con Systems (Solution Engineering Team) 
 *Date   : 24/03/2022
 *Build  : sudo g++ -ggdb -std=c++0x C++_OpenCV_Application_updation.cpp -o C++_OpenCV_Application_updation `pkg-config --cflags --libs opencv4` -lpthread -ludev -lopencv_highgui -lopencv_imgproc
 *Use    : To Set Exposure,Gain,Red,Green and Blue Gain,Brightness,Current Frame Format,Zoom,pan,Tilt,Focus,Contrast,Sharpness,Gamma,Saturation,White Balance. 
 */

#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>

#ifndef HIDIOCSFEATURE
#warning Please have your distro update the userspace kernel headers
#define HIDIOCSFEATURE(len)    _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x06, len)
#define HIDIOCGFEATURE(len)    _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x07, len)
#endif


//Header Files
#include "opencv2/opencv.hpp"
#include <thread>
#include <unistd.h>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <sys/ioctl.h>
#include<iostream>
#include<fstream>
#include<fcntl.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>
#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstring>
#include <errno.h>
#include <inttypes.h>
#include <time.h>
#include<stdlib.h>


//Macros for device Identification, Accessing Gain at Buffers
#define Vendor_ID 0x2560
#define Product_ID 0xc0d0 //(for See3CAM_130AF)
#define CONTROL_ID 0x7B
#define GET_RED 0x2B
#define GET_GREEN 0x27
#define GET_BLUE 0x29
#define SET_RED 0x2C
#define SET_GREEN 0x28
#define SET_BLUE 0x2A

//Global Variables
const char *bus_str(int bus);
using namespace cv;
using namespace std;
int video_index =1;
int width = 1920;
int height = 1080;
struct hidraw_report_descriptor rpt_desc;
struct hidraw_devinfo info;
std::string name = "/dev/hidraw";
VideoCapture cap(video_index,CAP_V4L2);

int frame_count=0;

//Union for gathering the Frame format 
union {
    char    c[5];
    int     i;
} myfourcc;





/*
***************** Function to Input Validate *******************************************
*/

int scan_and_validate_num()
{
    int a;
   // char c;
    while((scanf("%d",&a)<=1)&&(getchar()!='\n'))
    {
	//printf("your input = %d\n",a);
        printf("Not a Valid input");
	printf("\nEnter a Valid Integer Value : ");
	while(getchar()!='\n');                    //Clears the input stream until newline
    }
    return(a);
}


/*
 **************** Function to Read Red Gain *********************************************
 */

int Read_Red_Gain(int fd)
{
    uint8_t buf[64]={0},in_buf[64]={0},res;
    buf[0] = CONTROL_ID;
    buf[1] = GET_RED;
    res = write(fd,buf,64);
    res = 0;
    while(res != 64)
    {	
        res = read(fd,in_buf,64);
        if(in_buf[1] != GET_RED)
            res = 1;
    }

    return in_buf[2];
}


/*
 **************** Function to Read Green Gain *********************************************
 */


int Read_Green_Gain(int fd)

{
    uint8_t buf[64]={0},in_buf[64]={0},res;
    buf[0] = CONTROL_ID;
    buf[1] = GET_GREEN;
    res = write(fd,buf,64);
    res = 0;
    while(res != 64)
    {	
        res = read(fd,in_buf,64);
        if(in_buf[1] != GET_GREEN)
            res = 1;
    }

    return in_buf[2];
}

/*
 **************** Function to Read Blue Gain *********************************************
 */

int Read_Blue_Gain(int fd)
{
    uint8_t buf[64]={0},in_buf[64]={0},res;
    buf[0] = CONTROL_ID;
    buf[1] = GET_BLUE;
    res = write(fd,buf,64);
    res = 0;
    while(res != 64)
    {	
        res = read(fd,in_buf,64);
        if(in_buf[1] != GET_BLUE)
            res = 1;
    }

    return in_buf[2];
}


/*
 **************** Function to Write Red Gain *********************************************
 */


void Write_Red_Gain(int fd,uint8_t value)
{
    uint8_t buf[64]={0},in_buf[64]={0},res;
    buf[0] = CONTROL_ID;
    buf[1] = SET_RED;
    buf[2] = value;
    res = write(fd,buf,64);
    res = 0;
    while(res != 64)
    {	
        res = read(fd,in_buf,64);
        if(in_buf[1] != SET_RED)
            res = 1;
    }

}


/*
 **************** Function to Write Green Gain *********************************************
 */

void Write_Green_Gain(int fd,uint8_t value)
{
    uint8_t buf[64]={0},in_buf[64]={0},res;
    buf[0] = CONTROL_ID;
    buf[1] = SET_GREEN;
    buf[2] = value;
    res = write(fd,buf,64);
    res = 0;
    while(res != 64)
    {	
        res = read(fd,in_buf,64);
        if(in_buf[1] != SET_GREEN)
            res = 1;
    }

}

/*
 **************** Function to Write Blue Gain *********************************************
 */

void Write_Blue_Gain(int fd,uint8_t value)
{
    uint8_t buf[64]={0},in_buf[64]={0},res;
    buf[0] = CONTROL_ID;
    buf[1] = SET_BLUE;
    buf[2] = value;
    res = write(fd,buf,64);
    res = 0;
    while(res != 64)
    {	
        res = read(fd,in_buf,64);
        if(in_buf[1] != SET_BLUE)
            res = 1;
    }

}

/******************************* Function to roll back to Default Settings********************************/
void Default(int fd)
{

/*+++++++++++++++++++++++++++++ Note Order Should not be changed or else it won't work +++++++++++++++++++++*/
    
    
    cap.set(CAP_PROP_AUTO_WB, 0);
    usleep(10000);
    cap.set(CAP_PROP_WB_TEMPERATURE,4600); 
    usleep(10000);
    cap.set(CAP_PROP_PAN,0);                          //Pan
    usleep(10000);
    cap.set(CAP_PROP_TILT,565200);                          //Tilt
    usleep(10000);
    cap.set(CAP_PROP_SATURATION,16);                   //Saturation
    usleep(10000);
    cap.set(CAP_PROP_CONTRAST,10);                      //Contrast                   
    usleep(10000);
    cap.set(CAP_PROP_SHARPNESS,16);                    //Sharpness
    usleep(10000);
    cap.set(CAP_PROP_ZOOM,100);                        //Zoom
    usleep(10000);
    cap.set(CAP_PROP_GAMMA,220);                       //Gamma
    usleep(10000);
    cap.set(CAP_PROP_BRIGHTNESS,0);                  //Brightness
    usleep(10000);                                                                
    cap.set(CAP_PROP_AUTO_EXPOSURE,1);            //Auto Exposure
    usleep(10000);
    cap.set(CAP_PROP_EXPOSURE,200);                   //Exposure
    usleep(10000);
    cap.set(CAP_PROP_GAIN,2);                          //Gain
    usleep(400000);                                                         //It Takes (400 ms) to set Gain : This is Must ! 
    cap.set(CAP_PROP_AUTOFOCUS,0);                 //Auto Focus
    usleep(1000);
    cap.set(CAP_PROP_FOCUS,46);                        //Focus 
    usleep(10000);
    Write_Green_Gain(fd,32);
    Write_Blue_Gain(fd,32);
    Write_Red_Gain(fd,24);	
    		
    return;
}

/************************************ Function to list Current Settings **************************************/
void list_current(int fd)
{
    usleep(10000);
    bool flag = false;
    
    do
    {
        
        if(cap.get(CAP_PROP_EXPOSURE)==-1)
        {
            usleep(10000);
            continue;
        }
        else if(cap.get(CAP_PROP_EXPOSURE)>=1)
        {
            
            flag=true;
            break;
        }
        else
        {
            continue;
        }
    } while (cap.get(CAP_PROP_EXPOSURE)==-1);
    if(flag == true)
    {
        cout<<"\nExposure_Absolute     : " << cap.get(CAP_PROP_EXPOSURE) << endl;

    }
    
    usleep(1000);
    cout<<"Gain                  : " << cap.get(CAP_PROP_GAIN) << endl;
    usleep(10000);
    cout<<"Exposure_Auto         : " <<cap.get(CAP_PROP_AUTO_EXPOSURE) << endl;
    usleep(1000);
    cout<<"Brightess             : " << cap.get(CAP_PROP_BRIGHTNESS) << endl;
    usleep(1000); 
    cout<<"Contrast              : " <<cap.get(CAP_PROP_CONTRAST) << endl;
    usleep(1000);
    cout<<"Saturation            : " <<cap.get(CAP_PROP_SATURATION) << endl;
    usleep(1000);
    cout<<"Sharpness             : " <<cap.get(CAP_PROP_SHARPNESS) << endl;
    usleep(1000);
    cout<<"Gamma                 : " <<cap.get(CAP_PROP_GAMMA) << endl;
    usleep(10000);
    cout<<"Pan                   : " <<cap.get(CAP_PROP_PAN) << endl;
    usleep(1000);
    cout<<"Tilt                  : " <<cap.get(CAP_PROP_TILT) << endl;
    usleep(1000);
    cout<<"Zoom                  : " <<cap.get(CAP_PROP_ZOOM) << endl;
    usleep(10000);
    cout<<"Focus_Absolute        : " <<cap.get(CAP_PROP_FOCUS) << endl;
    usleep(10000);
    cout<<"Focus_Auto            : " <<cap.get(CAP_PROP_AUTOFOCUS) << endl;
    usleep(1000);
    cout<<"WhiteBalance_absolute : " <<cap.get(CAP_PROP_WB_TEMPERATURE) << endl;
    usleep(1000);
    cout<<"WhiteBalance_auto     : " <<cap.get(CAP_PROP_AUTO_WB) << endl;
    usleep(1000);
    cout << "Red : " << Read_Red_Gain(fd) << " Green : " << Read_Green_Gain(fd) << " Blue : " << Read_Blue_Gain(fd) << endl ;
    cout << "\n";
    usleep(1000);
    return;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++ Thread Function (stream) to continuously run streaming +++++++++++++++++++++++++++++++++++++++++++++++++++*/

int stream()
{
    Mat myImage;//Declaring a matrix to load the frames//
    namedWindow("Video Player");//Declaring the video to show the video//
	

    //Setting Height and width of the frame
    cap.set(CAP_PROP_FRAME_WIDTH, width);
    cap.set(CAP_PROP_FRAME_HEIGHT, height);
    
    //Opens video frame infinitely
    for (;;)    
    { 
        
        //Reading the frame
        cap.read(myImage); 

        //Condition that checks if frame is captured or not
        if (myImage.empty()) 
        {   
            break;
        }
        
        //Displaying the frame 
        imshow("Video Player", myImage);

        //needed to get frame count untill first 10 frames for handling parallel execution of threads
        if(frame_count<=10)
        {
            frame_count++;
        }
        
        
        waitKey(30);
        
        //numFramesCaptured++;
        
        
    }
    cap.release();//Releasing the buffer memory//
    return 0;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++ Thread Function (uvc_controls) to change gain,exposure,rgb_gain +++++++++++++++++++++++++++++++++++++++++++++++++++*/

void uvc_controls(uint8_t buf[64], uint8_t in_buf[64],int res, int fd)
{
    while(true)
    //Checking frame count and displaying after the 10th frame for proper execution of controls
    if(frame_count>=10)
    {
        int red,green,blue;
        unsigned short oper;
            // Create a std::promise object
        std::promise<void> exitSignal;
        //Fetch std::future object associated with promise
        std::future<void> futureObj = exitSignal.get_future();
        //sleep(1);
        
        //Rolling back to default settings 
        Default(fd);
        while(true){
            //usleep(50000);
            cout << "\n                  0 - exit\n";
            cout << "                  1  : Exposure \n";
            cout << "                  2  : Gain \n";
            cout << "                  3  : Brightness \n";
            cout << "                  4  : Gamma\n";
            cout << "                  5  : Zoom\n";
            cout << "                  6  : Pan\n";
            cout << "                  7  : Tilt\n";
            cout << "                  8  : Sharpness\n";
            cout << "                  9  : Saturation\n";
            cout << "                  10 : Contrast\n";
            cout << "                  11 : Focus\n";
            cout << "                  12 : Current Frame Format\n";
            cout << "                  13 : White Balance\n";
            cout << "                  14 : Read Red,Green,Blue - Gain Value\n";
            cout << "                  15 : Set Red,Green,Blue  - Gain Value\n";
            cout << "                  16 : List Current Settings\n";
            cout << "                  17 : Back to Default Settings\n\n";
            
            cout << "Enter your choice : ";
            //cin >> oper;
            int oper = scan_and_validate_num();
            cap.get(CAP_PROP_EXPOSURE);
            switch (oper) {
                case 1:
                    cap.get(CAP_PROP_EXPOSURE);
                    //Exposure Control
                    //usleep(1000);
                    int exposure;
                    cout << "1 : Auto \n";
                    cout << "2 : Manual\n";
                    
                    cout <<"Enter your Choice : ";
                    //cin >> exposure;
                    exposure = scan_and_validate_num();
                    
                    if(exposure == 1) //Auto Exposure
                    {
                        cap.set(CAP_PROP_AUTO_EXPOSURE, 0);
                        cout << "Exposure Set to Auto Mode...";
                    }
                    else if(exposure == 2)//Manul mode exposure`
                    {
                        int manual_expo;
                        //do to handle if wrong input condition is attempted
                        do
                        {
                            do
                            {
                                
                                if(cap.get(CAP_PROP_EXPOSURE)==-1)
                                {
                                    usleep(10000);
                                    continue;
                                }
                                else
                                {
                                    cout<<"Exposure_Absolute : " << cap.get(CAP_PROP_EXPOSURE) << endl;
                                    break;
                                }

                            } while (cap.get(CAP_PROP_EXPOSURE)==-1);
                            //cout << "Current Exposure : " << cap.get(CAP_PROP_EXPOSURE) << endl; 
                            usleep(1000);
                            cap.set(CAP_PROP_AUTO_EXPOSURE, 1);
                            cout << "min-max ~ (1 - 10000)\n";
                            cout << "Enter the Value of exposure : " ;
                            //cin >> manual_expo;
                            manual_expo = scan_and_validate_num();
                            if(manual_expo >=1 && manual_expo <=10000)
                            {
                                usleep(1000);
                                cap.set(CAP_PROP_EXPOSURE, manual_expo);
                                cout << "Manual Exposure set to : " << manual_expo << endl;
                                break;
                            }
                            else
                            {
                                cout << "Exposure is Limited from (1 - 10000)...";
                                int opti;
                                do
                                {
                                    cout << "Continue Setting exposure ?\n";
                                    cout << "1 : yes\n";
                                    cout << "2 : No\n";      
                                    cout <<"Enter your choice : ";
                                    opti = scan_and_validate_num();
                                    if(opti>=1 && opti<=2)
                                    {
                                        if(opti == 2)
                                        {
                                            break;
                                        }
                                        else if (opti == 1)
                                        {
                                            break;
                                        }
                                        else
                                        {
                                            cout << "Invalid Option\n";
                                        }
                                    }
                                    else
                                    {
                                        cout << "Invalid Option\n";

                                    }
                                    
                                }while(!(opti>=1 && opti<=2));
                                if(opti==2)
                                {
                                    break;
                                }
                            }
                        }while(!(manual_expo>=1 && manual_expo<=10000));
                        
                    }
                    else
                    {
                        cout << "Exposure Set Only Auto and Manual Allowed(1 or 2 )\n";
                    }

                    break;
                case 2:
                    //Gain Control
                    
                    usleep(1000);
                    if(!(cap.get(CAP_PROP_AUTO_EXPOSURE)==1))
                    {
                        cout << "\nCurrent Mode : Auto Mode\n";
                        cout << "You Can't use uvc Controls in Auto mode Switch to Manual Mode...\n\n";
                        break;

                    }
                    usleep(1000);
                    
                    int Gain;
                    
                    do
                    {
                        usleep(1000);
                        cout << "Current Gain : " << cap.get(CAP_PROP_GAIN) << endl; 
                        cout << "min-max ~ (1 - 8)\n";
                        cout << "Enter the Gain Value to be Set : ";
                        Gain = scan_and_validate_num();
                        if(Gain>=1&&Gain<=8)
                        {
                            usleep(1000);
                            cap.set(CAP_PROP_GAIN,Gain);
                            cout << "Gain set to : "<< Gain <<endl;
                            break;
                        }
                        else
                        {
                            cout << "Gain is Limited from (1 - 8)...";
                            int opti;
                            do
                                {
                                    cout << "Continue Setting Gain ?\n";
                                    cout << "1 : yes\n";
                                    cout << "2 : No\n";      
                                    cout <<"Enter your choice : ";
                                    //cin >> opti;
                                    opti = scan_and_validate_num();
                                    if(opti>=1 && opti<=2)
                                    {
                                        if(opti == 2)
                                        {
                                            break;
                                        }
                                        else if (opti == 1)
                                        {
                                            break;
                                        }
                                        else
                                        {
                                            cout << "Invalid Option\n";
                                        }
                                    }
                                    else
                                    {
                                        cout << "Invalid Option\n";

                                    }
                                    
                                }while(!(opti>=1 && opti<=2));
                                if(opti==2)
                                {
                                    break;
                                }
                        }
                    }while(!(Gain>=1&&Gain<=8));
                    
                    break;
                
                case 3:
                    //Set Brightness
                    int brightness;
                    do
                    {
                        cout << "min-max ~ (-15 to +15)\n";
                        usleep(1000);
                        cout << "Current Brightness is : " << cap.get(CAP_PROP_BRIGHTNESS) << endl ;
                        cout << "Enter the brightness Value to be set : ";
                        cin >> brightness;
                        if(brightness>=-15 && brightness<=15)
                        {    
                            usleep(1000);
                            cap.set(CAP_PROP_BRIGHTNESS,brightness);
                            break;
                        }
                        else
                        {
                            cout << "Brightness is Limited to (-15 to +15)...";
                            int opti;
                            do
                                {
                                    cout << "Continue Setting brightness ?\n";
                                    cout << "1 : yes\n";
                                    cout << "2 : No\n";      
                                    cout <<"Enter your choice : ";
                                    //cin >> opti;
                                    opti = scan_and_validate_num();
                                    if(opti>=1 && opti<=2)
                                    {
                                        if(opti == 2)
                                        {
                                            break;
                                        }
                                        else if (opti == 1)
                                        {
                                            break;
                                        }
                                        else
                                        {
                                            cout << "Invalid Option\n";
                                        }
                                    }
                                    else
                                    {
                                        cout << "Invalid Option\n";

                                    }
                                    
                                }while(!(opti>=1 && opti<=2));
                                if(opti==2)
                                {
                                    break;
                                }
                        }

                    }while (!(brightness>=-15 && brightness<=15));
                        

                    break;
                case 4:
                    //Set Gamma
                    int gamma;
                    do
                    {
                        cout << "min-max ~ (40 to 500)\n";
                        cout << "Current Gamma Value is : " <<cap.get(CAP_PROP_GAMMA) << endl;
                        cout << "Enter the value of gamma to be set : ";
                        //cin >> gamma;
                        gamma = scan_and_validate_num();
                        if(gamma>=40 && gamma <=500)
                        {
                            usleep(1000);
                            cap.set(CAP_PROP_GAMMA, gamma);
                            break;
                        }
                        else
                        {
                            cout << "Gamma is Limited to (40 to 500)...";
                            int opti;
                            do
                                {
                                    cout << "Continue Setting Gamma ?\n";
                                    cout << "1 : yes\n";
                                    cout << "2 : No\n";      
                                    cout <<"Enter your choice : ";
                                    //cin >> opti;
                                    opti = scan_and_validate_num();
                                    if(opti>=1 && opti<=2)
                                    {
                                        if(opti == 2)
                                        {
                                            break;
                                        }
                                        else if (opti == 1)
                                        {
                                            break;
                                        }
                                        else
                                        {
                                            cout << "Invalid Option\n";
                                        }
                                    }
                                    else
                                    {
                                        cout << "Invalid Option\n";

                                    }
                                    
                                }while(!(opti>=1 && opti<=2));
                                if(opti==2)
                                {
                                    break;
                                }
                        }

                    }while (!(gamma>=40 && gamma <=500));
                    
                    break;
                case 5: 
                    //Set Zoom
                    int zoom;
                    do
                    {
                        cout << "min-max ~ (100 to 800)\n";
                        usleep(1000);
                        cout << "Current Zoom Value is : " <<cap.get(CAP_PROP_ZOOM) << endl;
                        cout << "Enter the value of Zoom to be set : ";
                        //cin >> zoom;
                        zoom = scan_and_validate_num();
                        if(zoom>=100 && zoom<=800)
                        {
                            usleep(1000);
                            cap.set(CAP_PROP_ZOOM,zoom);
                            break;
                        }
                        else
                        {
                            cout << "Zoom is Limited to (100 to 800)...";
                            int opti;
                            do
                                {
                                    cout << "Continue Setting Zoom ?\n";
                                    cout << "1 : yes\n";
                                    cout << "2 : No\n";      
                                    cout <<"Enter your choice : ";
                                    //cin >> opti;
                                    opti = scan_and_validate_num();
                                    if(opti>=1 && opti<=2)
                                    {
                                        if(opti == 2)
                                        {
                                            break;
                                        }
                                        else if (opti == 1)
                                        {
                                            break;
                                        }
                                        else
                                        {
                                            cout << "Invalid Option\n";
                                        }
                                    }
                                    else
                                    {
                                        cout << "Invalid Option\n";

                                    }
                                    
                                }while(!(opti>=1 && opti<=2));
                                if(opti==2)
                                {
                                    break;
                                }
                        }

                    }while (!(zoom>=100 && zoom<=800));
                    break;
                case 6:
                    //Set pan
                    //Set Tilt
                    int pan;
                    do
                    {
                        cout << "min-max ~ (-648000 to 648000)\n";
                        usleep(1000);
                        cout << "Current Pan Value is : " <<cap.get(CAP_PROP_PAN) << endl;
                        cout << "Enter the value of Pan to be set : ";
                        //cin >> pan;
                        pan = scan_and_validate_num();
                        if(pan>=-648000 && pan <=648000)
                        {
                            usleep(1000);
                            cap.set(CAP_PROP_PAN,pan);
                            break;
                        }
                        else
                        {
                            cout << "Pan is Limited to (-648000 to 648000)...";
                            int opti;
                            do
                                {
                                    cout << "Continue Setting Pan ?\n";
                                    cout << "1 : yes\n";
                                    cout << "2 : No\n";      
                                    cout <<"Enter your choice : ";
                                    //cin >> opti;
                                    opti = scan_and_validate_num();
                                    if(opti>=1 && opti<=2)
                                    {
                                        if(opti == 2)
                                        {
                                            break;
                                        }
                                        else if (opti == 1)
                                        {
                                            break;
                                        }
                                        else
                                        {
                                            cout << "Invalid Option\n";
                                        }
                                    }
                                    else
                                    {
                                        cout << "Invalid Option\n";

                                    }
                                    
                                }while(!(opti>=1 && opti<=2));
                                if(opti==2)
                                {
                                    break;
                                }
                        }

                    }while (!(pan>=-648000 && pan <=648000));

                    
                    break;
                case 7:
                    //Set Tilt
                    int tilt;
                    do
                    {
                        cout << "min-max ~ (-648000 to 648000)\n";
                        usleep(1000);
                        cout << "Current tilt Value is : " <<cap.get(CAP_PROP_TILT) << endl;
                        cout << "Enter the value of tilt to be set : ";
                        //cin >> tilt;
                        tilt = scan_and_validate_num();
                        if(tilt>=-648000 && tilt <=648000)
                        {
                            usleep(1000);
                            cap.set(CAP_PROP_TILT,tilt);
                            break;
                        }
                        else
                        {
                            cout << "Tilt is Limited to (-648000 to 648000)...";
                            int opti;
                            do
                            {
                                cout << "Continue Setting Tilt ?\n";
                                cout << "1 : yes\n";
                                cout << "2 : No\n";      
                                cout <<"Enter your choice : ";
                                //cin >> opti;
                                opti = scan_and_validate_num();
                                if(opti>=1 && opti<=2)
                                {
                                    if(opti == 2)
                                    {
                                        break;
                                    }
                                    else if (opti == 1)
                                    {
                                        break;
                                    }
                                    else
                                    {
                                        cout << "Invalid Option\n";
                                    }
                                    
                                }
                                else
                                {
                                    cout << "Invalid Option\n";
                                    continue;

                                }
                                
                                
                            }while(!(opti>=1 && opti<=2));
                            if(opti==2)
                            {
                                break;
                            }

                        }

                    }while (!(tilt>=-648000 && tilt <=648000));

                    
                    break;
                case 8:
                    //Set Sharpness
                    int Sharpness;
                    do
                    {
                        cout << "min-max ~ (0 to 127)\n";
                        usleep(1000);
                        cout << "Current Sharpness Value is : " <<cap.get(CAP_PROP_SHARPNESS) << endl;
                        cout << "Enter the value of Sharpness to be set : ";
                        //cin >> Sharpness;
                        Sharpness = scan_and_validate_num();
                        if(Sharpness>=0 && Sharpness <=127)
                        {
                            usleep(1000);
                            cap.set(CAP_PROP_SHARPNESS,Sharpness);
                            break;
                        }
                        else
                        {
                            cout << "Sharpness is Limited to (0 to 127)...";
                            int opti;
                            do
                                {
                                    cout << "Continue Setting Sharpness ?\n";
                                    cout << "1 : yes\n";
                                    cout << "2 : No\n";      
                                    cout <<"Enter your choice : ";
                                    //cin >> opti;
                                    opti = scan_and_validate_num();
                                    if(opti>=1 && opti<=2)
                                    {
                                        if(opti == 2)
                                        {
                                            break;
                                        }
                                        else if (opti == 1)
                                        {
                                            break;
                                        }
                                        else
                                        {
                                            cout << "Invalid Option\n";
                                        }
                                    }
                                    else
                                    {
                                        cout << "Invalid Option\n";

                                    }
                                    
                                }while(!(opti>=1 && opti<=2));
                                if(opti==2)
                                {
                                    break;
                                }
                        }

                    }while (!(Sharpness>=0 && Sharpness <=127));

                    
                    break;
                
                case 9:
                    //Set Saturation
                    int saturation;
                    do
                    {
                        cout << "min-max ~ (0 to 60)\n";
                        cout << "Current saturation Value is : " <<cap.get(CAP_PROP_SATURATION) << endl;
                        cout << "Enter the value of Saturation to be set : ";
                        //cin >> saturation;
                        saturation = scan_and_validate_num();
                        if(saturation>=0 && saturation <=60)
                        {
                            usleep(1000);
                            cap.set(CAP_PROP_SATURATION,saturation);
                            break;
                        }
                        else
                        {
                            cout << "Saturation is Limited to (0 to 60)...";
                            int opti;
                            do
                                {
                                    cout << "Continue Setting Saturation ?\n";
                                    cout << "1 : yes\n";
                                    cout << "2 : No\n";      
                                    cout <<"Enter your choice : ";
                                    //cin >> opti;
                                    opti = scan_and_validate_num();
                                    if(opti>=1 && opti<=2)
                                    {
                                        if(opti == 2)
                                        {
                                            break;
                                        }
                                        else if (opti == 1)
                                        {
                                            break;
                                        }
                                        else
                                        {
                                            cout << "Invalid Option\n";
                                        }
                                    }
                                    else
                                    {
                                        cout << "Invalid Option\n";

                                    }
                                    
                                }while(!(opti>=1 && opti<=2));
                                if(opti==2)
                                {
                                    break;
                                }
                        }

                    }while (!(saturation>=0 && saturation <=60));
                    break;
                case 10:
                    //Set Contrast
                    int contrast;
                    do
                    {
                        cout << "min-max ~ (0 to 30)\n";
                        usleep(1000);
                        cout << "Current contrast Value is : " <<cap.get(CAP_PROP_CONTRAST) << endl;
                        cout << "Enter the value of Contrast to be set : ";
                        //cin >> contrast;
                        contrast = scan_and_validate_num();
                        if(contrast>=0 && contrast <=30)
                        {
                            usleep(1000);
                            cap.set(CAP_PROP_CONTRAST,contrast);
                            break;
                        }
                        else
                        {
                            cout << "Contrast is Limited to (0 to 30)...";
                            int opti;
                            do
                                {
                                    cout << "Continue Setting Contrast ?\n";
                                    cout << "1 : yes\n";
                                    cout << "2 : No\n";      
                                    cout <<"Enter your choice : ";
                                    //cin >> opti;
                                    opti = scan_and_validate_num();
                                    if(opti>=1 && opti<=2)
                                    {
                                        if(opti == 2)
                                        {
                                            break;
                                        }
                                        else if (opti == 1)
                                        {
                                            break;
                                        }
                                        else
                                        {
                                            cout << "Invalid Option\n";
                                        }
                                    }
                                    else
                                    {
                                        cout << "Invalid Option\n";

                                    }
                                    
                                }while(!(opti>=1 && opti<=2));
                                if(opti==2)
                                {
                                    break;
                                }
                        }

                    }while (!(contrast>=0 && contrast <=30));

                    break;
                case 11:
                    //Focus Control
                    int focus;
                    cout << "1 : Auto \n";
                    cout << "2 : Manual\n";
                    cout <<"Enter your Choice : ";
                    //cin >> focus;
                    focus = scan_and_validate_num();
                    if(focus == 1) //Auto Focus
                    {
                        usleep(1000);
                        cap.set(CAP_PROP_AUTOFOCUS, true);
                        cout << "Focus Set to Auto Mode...";
                    }
                    else if(focus == 2)//Manul Focus
                    {
                        cap.set(CAP_PROP_AUTOFOCUS, false);
                        int manual_focus;
                        do
                        {
                            usleep(1000);
                            cout << "Current Focus : " << cap.get(CAP_PROP_FOCUS) << endl; 
                            cout << "min-max ~ (0 to 255)\n";
                            cout << "Enter the Value of Focus : " ;
                            cin >> manual_focus;
                            if(manual_focus>=0 && manual_focus<=255)
                            {
                                usleep(1000);
                                cap.set(CAP_PROP_FOCUS, manual_focus);
                                break;
                            }
                            else
                            {
                                cout << "Focus is Limited to (0 to 255)...";
                                int opti;
                                do
                                {
                                    cout << "Continue Setting Focus ?\n";
                                    cout << "1 : yes\n";
                                    cout << "2 : No\n";      
                                    cout <<"Enter your choice : ";
                                    //cin >> opti;
                                    opti = scan_and_validate_num();
                                    if(opti>=1 && opti<=2)
                                    {
                                        if(opti == 2)
                                        {
                                            break;
                                        }
                                        else if (opti == 1)
                                        {
                                            break;
                                        }
                                        else
                                        {
                                            cout << "Invalid Option\n";
                                        }
                                    // break;
                                    }
                                    else
                                    {
                                        cout << "Invalid Option\n";

                                    }
                                    
                                }while(!(opti>=1 && opti<=2));
                                if(opti==2)
                                {
                                    break;
                                }
                            }

                        }while (!(manual_focus>=0 && manual_focus <=255));
                    }
                    else
                    {
                        cout << "\nFocus Set Only Auto and Manual Allowed(1 or 2 )\n";
                    }
                    break;
                case 12:
                    usleep(1000);
                    myfourcc.i = cap.get(CAP_PROP_FOURCC);
                    myfourcc.c[4] = '\0';
                    cout << "\nCurrent Frame Format : " << myfourcc.c << "\n" << endl;
                    cout<<cap.get(CAP_PROP_FRAME_WIDTH)<<'x'<<cap.get(CAP_PROP_FRAME_HEIGHT)<<"\n\n" << endl;
                    break;
                
                case 13:
                    //White Balance
                    int white_balance;
                    cout << "1 : Auto \n";
                    cout << "2 : Manual\n";
                    cout <<"Enter your Choice : ";
                    //cin >> white_balance;
                    white_balance = scan_and_validate_num();
                    if(white_balance == 1) //Auto White_Balance
                    {
                        usleep(1000);
                        cap.set(CAP_PROP_AUTO_WB, true);
                        cout << "White Balance Set to Auto Mode...";
                    }
                    else if(white_balance == 2)//Manul White_Balance
                    {
                        usleep(1000);
                        cap.set(CAP_PROP_AUTO_WB, false);
                        int manual_white;
                        
                        do
                        {
                            usleep(1000);
                            cout << "Current White Balance : " << cap.get(CAP_PROP_WB_TEMPERATURE) << endl; 
                            cout << "min-max ~ (1000 to 10000)\n";
                            cout << "Enter the Value of White_Balance : " ;
                            //cin >> manual_white;
                            manual_white = scan_and_validate_num();
                            if(manual_white>=1000 && manual_white <=10000)
                            {
                                usleep(1000);
                                cap.set(CAP_PROP_WB_TEMPERATURE, manual_white);
                                cout << "White Balance set to : " << manual_white << endl;
                                break;
                            }
                            else
                            {
                                cout << "White Balance is Limited to (1000 to 10000)...";
                                int opti;
                                do
                                {
                                    cout << "Continue Setting White Balance ?\n";
                                    cout << "1 : yes\n";
                                    cout << "2 : No\n";      
                                    cout <<"Enter your choice : ";
                                    //cin >> opti;
                                    opti = scan_and_validate_num();
                                    if(opti>=1 && opti<=2)
                                    {
                                        if(opti == 2)
                                        {
                                            break;
                                        }
                                        else if (opti == 1)
                                        {
                                            break;
                                        }
                                        else
                                        {
                                            cout << "Invalid Option\n";
                                        }
                                    // break;
                                    }
                                    else
                                    {
                                        cout << "Invalid Option\n";

                                    }
                                    //break;
                                    
                                }while(!(opti>=1 && opti<=2));
                                if(opti==2)
                                {
                                    break;
                                }
                            }
                            
                        }while(!(manual_white>=1000 && manual_white <=10000));
                    }
                    else
                    {
                        cout << "\nWhite Balance Set Only Auto and Manual Allowed(1 or 2 )\n";
                    }
                    break;
                case 14:
                    //Read RGB Gains
                    if(true)
                    {
                        cout << "\n\nListing R G B Gain Values ...\n\n";
                        
                        cout << "Red Gain Value is   : " << Read_Red_Gain(fd) << '\n' ;		
                        
                        cout << "Green Gain Value is : " << Read_Green_Gain(fd) << '\n' ;
                        
                        cout << "Blue Gain Value is  : " << Read_Blue_Gain(fd) << "\n" ;
                    }   

                    break;

                case 15:
                    //Set RGB Gains
                    cout << "\nCurrent R G B Gain Values ...\n";
                        
                    cout << "Red Gain Value is : " << Read_Red_Gain(fd) << '\n' ;		
                    
                    cout << "Green Gain Value is : " << Read_Green_Gain(fd) << '\n' ;
                    
                    cout << "Blue Gain Value is : " << Read_Blue_Gain(fd) << "\n\n" ;
                    
                    cout << "Enter the Value for R G and B to set Gains Separately...\n";
                    
                    cout << "Enter the value of Red   : ";
                    cin >> red;
                    cout << "Enter the value of Green : ";
                    cin >> green;
                    cout << "Enter the value of Blue  : ";
                    cin >> blue;
                    //for validating if rgb value is below 255
                    if((red < 0 || red > 255) || (green < 0 || green > 255) || (blue < 0 || blue > 255))
                    {
                        printf("\n\rInvalid values\n");
                    }
                    else
                    {
                        
                        Write_Green_Gain(fd,green);
                        Write_Blue_Gain(fd,blue);
                        Write_Red_Gain(fd,red);	
                        
                        cout << "\n\r Successfully written\n";
                    }
                    break;
                case 16:
                //Lists the current Values of uvc controls
                    list_current(fd);
                    break;
                case 17:
                //Rolls back to default values from current uvc values
                    Default(fd);
                    break;
                case 0:
                    //Exitting the stream and terminating thread,exitting the execution
                    cap.release();
                    cv::destroyAllWindows();
                    exitSignal.set_value();
                    exit(10);
                default:
                    //If User Choices invalid Choice
                    cout << "\nInvalid Option\n";
                    break;
            }
        }
    }
}
 
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ MAIN Function +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/ 

int main( int argc, char** argv )
{
    //Checks if a video Node is Available 
    //If Video Node Not Available trying with other video nodes for available of camera eg : /dev/video0 is not available goes for /dev/video1 ...,etc till 200.
    if(!(cap.isOpened()))
    {
        do
        {
            if(video_index < 200)
            {
                video_index+=1;
                cap.open(video_index,CAP_V4L2);
            }
            else
            {
                cout << "No Video Device Detected ...";
                break;
            }
        } while (!(cap.isOpened()));
    }
    system("clear");
    system("clear");
    cout << "Video device Opened at Index : /dev/video" << video_index << endl;

    
    //Variable Declaration

    
    int fd;
    int i, res, desc_size = 0;
    uint8_t buf[64],in_buf[64];
    int count =0;
    char hidraw[20];
    int number=0;
    int hid_count =0;
    
    //char device[16] = "/dev/hidraw";
    int Split_number[4] = {0},temp;
    int digit_count,itr,value,camera_found=0,choice=0,red=32,green=32,blue=32;
    memset(&rpt_desc, 0x0, sizeof(rpt_desc));
    memset(&info, 0x0, sizeof(info));
    memset(buf, 0x0, sizeof(buf));

    
    //For Gathering HID Details
    
    do
    {
        if(hid_count<200)
        {
            snprintf(hidraw, sizeof(hidraw), "%s%d", name.c_str(), count);
            //cout<<hidraw<<'\n';
            fd = open(hidraw, O_RDWR|O_NONBLOCK ,0777);

            count++;
            hid_count++;
        }
        else
        {
            cout << "No hid camera Device Detected...";
            break;
        }

        if (fd > 0) 
        {
            res = ioctl(fd, HIDIOCGRDESCSIZE, &desc_size);
            if (res < 0)
            {
                perror("HIDIOCGRDESCSIZE");
            }
            rpt_desc.size = desc_size;
            res = ioctl(fd, HIDIOCGRDESC, &rpt_desc);
            if (res < 0)
            {
                perror("HIDIOCGRDESC");
            }
            res = ioctl(fd, HIDIOCGRAWPHYS(256), buf);
            if (res < 0)
                perror("HIDIOCGRAWPHYS");
            res = ioctl(fd, HIDIOCGRAWINFO, &info);
            if (res < 0) 
            {
                perror("HIDIOCGRAWINFO");
            }
            if((info.vendor & 0xFFFF) == Vendor_ID && (info.product & 0xFFFF)== Product_ID)
            {
                camera_found = 1;
                res = ioctl(fd, HIDIOCGRAWNAME(256), buf);
                if (res < 0)
                    perror("HIDIOCGRAWNAME");
                else
                    //printf("\n\nConnected Device: %s\n", buf);
                    

                break;
            }
            else
            {
                continue;
            }
            
        }
    }while(!((info.vendor & 0xFFFF) == Vendor_ID && (info.product & 0xFFFF)== Product_ID));


	if(camera_found == 0)
	{
		printf("See3CAM_130 cannot be found\n");
		close(fd);
		return 1;
	}
    int option;
    do
    {
        cout << "\n+++++++++++++++C++ UVC Controls+++++++++++++++++++\n";
        cout << "             OpenCV C++ App version 1.1              \n";
        cout << "             OpenCV Version : " << CV_VERSION << "\n" << endl;
        cout << "                  0  : Exit\n";
        cout << "                  1  : "<<buf<< "\n" <<  endl;
        
        cout << "Enter your Choice : ";
        cin >> option;

        if(option == 0)
        {
            exit(10);
        }
        else if(option == 1)
        {
            //Creating threads for streaming and Uvc Controls
            std::thread th1(stream);
            // cout << "\n+++++++++++++++C++ UVC Controls+++++++++++++++++++\n";
            // cout << "             OpenCV C++ App version 1.0              \n\n";
            std::thread th2(uvc_controls,buf,in_buf,res,fd);
            th1.join();
            sleep(1);
            th2.join();
        }
        else
        {
            cout << "Invalid Option\n";
            
        }
    }while(!(option>=0 && option<=1));
    



   return 0;

}

const char *
bus_str(int bus)
{
	switch (bus) {
	case BUS_USB:
		return "USB";
		break;
	case BUS_HIL:
		return "HIL";
		break;
	case BUS_BLUETOOTH:
		return "Bluetooth";
		break;
	case BUS_VIRTUAL:
		return "Virtual";
		break;
	default:
		return "Other";
		break;
	}
}