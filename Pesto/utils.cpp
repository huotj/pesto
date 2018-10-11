
#include <iostream>
#include <pthread.h>
#include "port.h"
#include "socket.h"
#include "log.h"
#include "nc_driver.h"
#include "structure.h"
#include <fstream>
#include <string>

extern int biasOK;
extern int caseID,repID,buff1ID,buff2ID,buff3ID,buff4ID,pathID,buff2wayID1,buff2wayID2;//tout les ID des socket
extern Log log;
extern pthread_t pth1,pth2;//pense pas qu'on a en encore dde besoin
extern pthread_t pth3;//the go thread loop
extern pthread_t pth4;//tcs loop to update struct tcs_var
extern pthread_t pth5;//temperature loop
extern pthread_t pth6;//meteo loop
extern pthread_t pth7;//thread for returning the incrementation
extern struct TCS tcs_var;
extern struct Meteo meteo;
extern struct camParam detParam;
extern int nbrBuffer;

extern std::string adress_tcs;
extern int mode,inc,error,ROIcount,tcs_loop,meteo_loop,biasOK,isInAcq,nbrBuffer;
extern int developpement;
extern std::string nameFile;
extern int overideFW;

void delay(int ms)
{
    clock_t t0 = clock();
    while (clock()<t0+ms*1000)
        ;
}


int initializeSocket(void){

caseID = create_socket(case_port);
    if(caseID==-1)
        {

        log.writetoVerbose("Unable to create a socket on port "+std::to_string(case_port));
        log.writetoVerbose("Shutting down");
        return -1;}
buff1ID = create_socket(buff1_port);
        if(buff1ID==-1){

        log.writetoVerbose("Unable to create a socket on port "+std::to_string(buff1_port));
        log.writetoVerbose("Shutting down");
            exit(1);}
buff2ID = create_socket(buff2_port);
            if(buff2ID==-1){

        log.writetoVerbose("Unable to create a socket on port "+std::to_string(buff2_port));
        log.writetoVerbose("Shutting down");
        exit(1);}
buff3ID = create_socket(buff3_port);
        if(buff3ID==-1){

        log.writetoVerbose("Unable to create a socket on port "+std::to_string(buff3_port));
        log.writetoVerbose("Shutting down");
        exit(1);}
pathID = create_socket(path_port);
        if(pathID==-1){

        log.writetoVerbose("Unable to create a socket on port "+std::to_string(path_port));
        log.writetoVerbose("Shutting down");
        exit(1);}

repID = create_socket(reponse_port);
        if(repID==-1){
        log.writetoVerbose("Unable to create a socket on port "+std::to_string(reponse_port));
        log.writetoVerbose("Shutting down");
        exit(1);}

buff4ID = create_socket(buff4_port);
        if(buff4ID==-1){

        log.writetoVerbose("Unable to create a socket on port "+std::to_string(buff4_port));
        log.writetoVerbose("Shutting down");
        exit(1);}

buff2wayID1 = createSocket2way(buff2way1_port);
buff2wayID2 = createSocket2way(buff2way2_port);

return 0;}

void saveImageCallback(NcCam myCam, NcImageSaved* imageSaved, void* dummy)
{

    ncWriteFileHeader(imageSaved, NC_STRING, "ORIGIN", "LAE", "Laboratoire d'Astrophysique Experimentale");
    ncWriteFileHeader(imageSaved, NC_STRING, "INSTRUME", "PESTO", "Instrument name");
    ncWriteFileHeader(imageSaved, NC_STRING, "TELESCOP", "OMM", "Observatoire du Mont-Megantic");
    ncWriteFileHeader(imageSaved, NC_STRING, "OBJECT", detParam.object_name.c_str(), "Object name");
    ncWriteFileHeader(imageSaved, NC_STRING, "RA", &tcs_var.RA, "Right ascension");
    ncWriteFileHeader(imageSaved, NC_STRING, "HA", &tcs_var.HA, "Hour angle");
    ncWriteFileHeader(imageSaved, NC_STRING, "DEC", &tcs_var.DEC, "Declination");
    ncWriteFileHeader(imageSaved, NC_STRING, "EPOCH", &tcs_var.EPOCH, "Epoch of the coordinate");
    ncWriteFileHeader(imageSaved, NC_STRING, "AIRMASS", &tcs_var.AIRMASS, "Air mass");
    ncWriteFileHeader(imageSaved, NC_STRING, "ST", &tcs_var.ST, "Sideral time");
    ncWriteFileHeader(imageSaved, NC_STRING, "UT", &tcs_var.UT, "Universal time");
    ncWriteFileHeader(imageSaved, NC_STRING, "TFOCUS", &tcs_var.FOCUS, "Focus of the telescope");
    ncWriteFileHeader(imageSaved, NC_STRING, "IROTATOR", &tcs_var.TROTATOR, "Angle of the instrument rotator");
    ncWriteFileHeader(imageSaved, NC_STRING, "DOME", &tcs_var.DOME, "Angle of the dome");
    ncWriteFileHeader(imageSaved, NC_STRING, "FILTRE",detParam.current_filter.c_str() , "Filter used");
    ncWriteFileHeader(imageSaved, NC_DOUBLE, "PESTOROT", &detParam.rotAngle, "Angle of the FOV rotator");
    ncWriteFileHeader(imageSaved, NC_DOUBLE, "PESTOMIR", &detParam.rotBras, "Position of the pickup mirror (degree)");
    ncWriteFileHeader(imageSaved, NC_STRING, "OBSERV", detParam.observateur.c_str(), "Name of the observer");
    ncWriteFileHeader(imageSaved, NC_STRING, "OPERATOR", detParam.Operator.c_str(), "Name of the telescope operator");
    ncWriteFileHeader(imageSaved, NC_DOUBLE, "TMESDECT", &detParam.ccdTemp, "Temperature of the emccd (C)");
    ncWriteFileHeader(imageSaved, NC_INT, "BIAS", &biasOK, "0->bias out of date, 1-> bias up to date");
    ncWriteFileHeader(imageSaved,NC_INT,"FWOVER",&overideFW,"0->filter position set by the filter wheel, 1-> position set by the user.");
    ncWriteFileHeader(imageSaved, NC_STRING, "FICHIER", nameFile.c_str(), "Filename");

    //meteo
    ncWriteFileHeader(imageSaved, NC_STRING, "HUMIN", &meteo.Hin, " Interior humidity (%)");
    ncWriteFileHeader(imageSaved, NC_STRING, "HUMOUT", &meteo.Hout, "Exterior humidity (%)");
    ncWriteFileHeader(imageSaved, NC_STRING, "TEMPIN", &meteo.Tin, "Interior temperature (C)");
    ncWriteFileHeader(imageSaved, NC_STRING, "TEMPOUT", &meteo.Tout, "Exterior temperature (C)");
    ncWriteFileHeader(imageSaved, NC_STRING, "TEMPST", &meteo.Tstruct, "Telescope structure temperature (C)");
    ncWriteFileHeader(imageSaved, NC_STRING, "TEMPM", &meteo.Tm, "Mirror temperature (C)");

}

int openCam(NcCam *cam,struct initParam *param,struct camParam *detParam)
{
    int cameraCount;
    int i,unitNb;
    char serial[256];
    enum CommType commInterface;

    int unit, channel, width, height, present, error;

    // Get the number of available cameras
    error = ncCamGetCameraAvailable(&cameraCount);
    if (error!=0)
    {
        log.writetoVerbose("Nuvu: Error"+std::to_string(error)+"happened while retrieving the number of cameras");
        exit(1);
    }


        std::cout<<"Found camera: "<<cameraCount<<std::endl;
    log.writetoVerbose(std::to_string(cameraCount)+" camera found on the network.");
    printf("\n");
    std::cout<<":::::::::::::::::::::::::::::"<<std::endl;

    for(i = 0; i < cameraCount; ++i)
    {
        // Get the camera's serial number
        error = ncCamGetCameraSerialNumber(i, serial);
        if (error!=0)
        {

            log.writetoVerbose("Nuvu: Error"+std::to_string(error)+"happened while retrieving the serial number of the camera"+std::to_string(i));
            exit(1);
        }



        log.writetoVerbose("("+std::to_string(i)+") Camera SN: "+serial+" is connected with the following parameters:");
        error = ncCamGetCameraPort(i, &unit, &channel);
        if (error!=0)
        {
            //printf ("Error %d happened while retrieving the port of the camera %s\n", error, serial);
            log.writetoVerbose("Nuvu: Error"+std::to_string(error)+"happened while retrieving the port of the camera (1): "+serial);

            exit(1);
        }

        log.writetoVerbose("Unit: "+std::to_string(unit)+" Channel: "+std::to_string(channel));

        error = ncCamGetCameraCommInterface(i, &commInterface);
        if (error!=0)
        {
            log.writetoVerbose("Nuvu: Error"+std::to_string(error)+"happened while retrieving the port of the camera (2): "+serial);
            exit(1);
        }

        printf("Communication interface: %x\n", commInterface);


        error = ncCamGetCameraDetectorSize(i, &width, &height);
        if (error!=0)
        {
            log.writetoVerbose("Nuvu: Error"+std::to_string(error)+"happened while retrieving the port of the camera (3): "+serial);
            exit(1);
        }

        printf("Detector size: %dx%d\n", width, height);

        error = ncCamGetCameraPresent(unit, channel, &present);
        if (error!=0)
        {
            log.writetoVerbose("Nuvu: Error"+std::to_string(error)+"happened while retrieving the port of the camera (4): "+serial);
            exit(1);
        }


        log.writetoVerbose("Presence status: "+std::to_string(present));
        std::cout<<":::::::::::::::::::::::::::::"<<std::endl;
    }
    std::cout<<"Which unit do you want to use? "<<std::endl;
    std::cin>>unitNb;
    error = ncCamOpen(PT_GIGE+unitNb+1,NC_AUTO_CHANNEL ,nbrBuffer,cam);
            if (error!=0)
            {   if (ncCamOpen(VIRTUAL+unitNb,NC_AUTO_CHANNEL ,nbrBuffer,cam)){
                log.writetoVerbose("Nuvu: Error"+std::to_string(error)+"happened while retrieving the port of the camera (5): "+serial);
                exit(1);
                }
            }
    error = ncCamSetReadoutMode(*cam, param->defROMODE);
    if (error!=0)
    {
        log.writetoVerbose("Unable to set the readout mode. Shutting down the PESTO... ");
        exit(1);
    }
    else
        log.writetoVerbose("ReadOutMode set to "+std::to_string(param->defROMODE)+" succesfully");





    error = ncCamGetReadoutTime(*cam, &detParam->readoutTime);

    if (error!=0)
    {
        log.writetoVerbose("Unable to get the readout time. Shutting down the PESTO... ");
        exit(1);
    }
    error = ncCamSetExposureTime(*cam, detParam->readoutTime);
    if (error!=0)
    {
            log.writetoVerbose("Unable to set the exposure time. Shutting down the PESTO... ");
            exit(1);
    }
    // Recover the exposure time for use later
    error = ncCamGetExposureTime(*cam, 1, &detParam->exposureTime);
    if (error)
    {
        log.writetoVerbose("Unable to get the exposure time. Shutting down the PESTO... ");
        exit(1);
    }


    // We set a reasonable waiting time
    error = ncCamSetWaitingTime(*cam, 0);
    if (error!=0)
    {
        log.writetoVerbose("Unable to set the waiting time. Shutting down the PESTO... ");
    exit(1);
    }
    // Recover the waiting time for use later
    error = ncCamGetWaitingTime(*cam, 1, &detParam->waitingTime);
    if (error!=0)
    {
        log.writetoVerbose("Unable to get the waiting time of the detector. Shutting down the PESTO... ");
        exit(1);
    }
    // Set a reasonable timeout on reading an image
    // The delay between images is the sum of the waiting time, the exposure and the readout time
    // (if the waiting time is non-zero).
    error = ncCamSetTimeout(*cam, detParam->waitingTime + detParam->exposureTime + detParam->readoutTime + 1000.0 );
    if (error!=0)
    {
        log.writetoVerbose("Unable to set the timeout of the detector. Shutting down the PESTO... ");
    exit(1);
    }
    //CLOSE the shutter for the acquisition
    error = ncCamSetShutterMode( *cam, CLOSE );
    if (error!=0)
    {
        log.writetoVerbose("Unable to close the shutter of the camera. Shutting down the PESTO... ");
        exit(1);
    }
    error = ncCamSetTargetDetectorTemp(*cam,param->defCCDTEMP);
    if (error!=0)
    {
        exit(1);
    }
    error = ncCamSaveImageSetHeaderCallback(*cam, saveImageCallback, cam);
    if (error!=0)
    {
        log.writetoVerbose("Unable to set the save image callback . Shutting down");
        exit(1);
    }

    return 0;
}







int initVariable(struct initParam *param,struct camParam *detParam){
    if (log.isFile("/home/initPesto/init.txt")){
        //cherche les variables
        std::ifstream file("/home/initPesto/init.txt");
        std::string str;
        while (std::getline(file,str)){
            if (str.find("EMGAIN")==0){param->maxGain = std::stoi(str.substr(str.find(":")+1));}
            if (str.find("MINGAIN")==0){param->minGain = std::stoi(str.substr(str.find(":")+1));}

            if (str.find("PATHRACINE")==0){param->racinePath = str.substr(str.find(":")+1) ;}
            if (str.find("PATHLOG")==0){param->logPath = str.substr(str.find(":")+1);}
            if (str.find("NBBIASCONV")==0){param->nbBiasConv = std::stoi(str.substr(str.find(":")+1));}
            if (str.find("NBBIASEM")==0){param->nbBiasEm = std::stoi(str.substr(str.find(":")+1));}
            if (str.find("FW0")==0){param->FW0=str.substr(str.find(":")+1);}
            if (str.find("FW1")==0){param->FW1=str.substr(str.find(":")+1);}
            if (str.find("FW2")==0){param->FW2=str.substr(str.find(":")+1);}
            if (str.find("FW3")==0){param->FW3=str.substr(str.find(":")+1);}
            if (str.find("FW4")==0){param->FW4=str.substr(str.find(":")+1);}
            if (str.find("FW5")==0){param->FW5=str.substr(str.find(":")+1);}

            if (str.find("defROMODE")==0){param->defROMODE = std::stoi(str.substr(str.find(":")+1));}
            if (str.find("defCCDTEMP")==0){param->defCCDTEMP = std::stod(str.substr(str.find(":")+1));}
            if (str.find("PATHPYTHON")==0){param->pathPython = str.substr(str.find(":")+1);}
            if (str.find("addrTCS")==0){adress_tcs = str.substr(str.find(":")+1);}
        }
        //fix the space in front of the path and the '/' at the end
        while (param->logPath[0]!='/'){param->logPath.erase(0,1);}
        if (param->logPath[param->logPath.length()-1]!='/'){param->logPath+='/';};
        while (param->racinePath[0]!='/'){param->racinePath.erase(0,1);}
        if (param->racinePath[param->racinePath.length()-1]!='/'){param->pathPython+='/';};
        while (param->pathPython[0]!='/'){param->pathPython.erase(0,1);}
        if (param->pathPython[param->pathPython.length()-1]!='/'){param->pathPython+='/';};
        while (adress_tcs[0]!='1'){adress_tcs.erase(0,1);}


    }
    else
    {
        log.writetoVerbose("init.txt has not been found. The position of the filter wheel will not be recorded in the header. The initialize parameters will be set to the hard coded value. Please refer to the documentation for more details");
    }

    if (!param->maxGain){param->maxGain=500;}
    if (!param->minGain){param->minGain=10;}
    if (param->logPath.empty()){param->logPath="/home/data/log/";}
    if (!param->nbBiasConv){param->nbBiasConv=60;}
    if (param->racinePath.empty()){param->racinePath="/home/data/";}
    if (!param->nbBiasEm){param->nbBiasEm=200;}
    if (!param->defCCDTEMP){param->defCCDTEMP=-80;}
    if (!param->defROMODE){param->defROMODE=1;}
    if (detParam->object_name.empty()){detParam->object_name="object";}
    if (adress_tcs.empty()){adress_tcs="132.204.61.80";}
    if (param->pathPython.empty()){param->pathPython="/home/pesto/jonathan/Pesto_v2.0/python/";}



    //Need to be implemented in init (above code)
    detParam->object_name="Object";
    detParam->current_filter="N/A";
    detParam->observateur="Unknown";
    overideFW=0;//overide the filter wheel position

    //other variable
    mode=2;
    inc=1;
    error=0;
    ROIcount=0;
    tcs_loop=1;
    meteo_loop=1;
    biasOK=0;
    nbrBuffer=64;
    //detParam.nbrExp=15;
    isInAcq = 0;

    return 0;
}

std::string create_name(void){
/*
Description: This function creates the name of the files to be saved. Name are in
standard format (AAMMJJ_)
*/
char racine[32];
time_t t;
t = time(NULL);
std::string name;
struct tm tm = *localtime(&t);
if (tm.tm_hour<12){

    sprintf(racine,"%d%.2d%.2d", tm.tm_year+1900-2000,tm.tm_mon+1,tm.tm_mday-1);
    name = racine;
    name+='_';
    //strcpy(name,concat(racine,"_"));


}
if (tm.tm_hour>=12){
    sprintf(racine,"%d%.2d%.2d", tm.tm_year+1900-2000,tm.tm_mon+1,tm.tm_mday);
    name = racine;
    name+='_';

    //strcpy(name,concat(racine,"_"));
}

return name;}


bool isLeap(int year)
{
    if (year%4==0){return true;}
    else if (year%100==0){return false;}
    else if (year%400==0){return true;}
    else {return false;}

}


std::string create_dossier(int mode,struct camParam *detParam){
/*
Description: This function creates the name of the folder to be saved. Name are in
standard format (AAMMJJ/)
*/
char racine[32];
time_t t;
t = time(NULL);
std::string name;
struct tm tm = *localtime(&t);

static const int days[2][13] = {
        {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}

};

if (tm.tm_hour<12){

    int year = tm.tm_year+1900;
    int month = tm.tm_mon+1;
    int day = tm.tm_mday;

    if (day==1)
    {
        day = days[isLeap(year)][month];
    }
    else
    {
        day = day-1;
    }



    sprintf(racine,"%d%.2d%.2d", year-2000,month,day);
    name = racine;
    name+='/';
    if (mode==1)
    {
        name+="Target/";
        name+=detParam->object_name+"/";
    }
    if (mode==2){name+="SkyFlat/";}
    if (mode==3){name+="DomeFlat/";}
    if (mode==4){name+="Dark/";}
    if (mode==5){name+="Video/";}
    if (mode==6){name+="Other/";}



}
if (tm.tm_hour>=12){
    sprintf(racine,"%d%.2d%.2d", tm.tm_year+1900-2000,tm.tm_mon+1,tm.tm_mday);
    name = racine;
    name+='/';
    if (mode==1)
    {
        name+="Target/";
        name+=detParam->object_name+"/";
    }
    if (mode==2){name+="SkyFlat/";}
    if (mode==3){name+="DomeFlat/";}
    if (mode==4){name+="Dark/";}
    if (mode==5){name+="Video/";}
    if (mode==6){name+="Other/";}
}
//if (!log.isFolder(name))
//{
//    log.createFolder(name);
//}

return name;}



int selectTimeStamp(NcCam *cam,int mode){
    int error;
    switch(mode)
    {
    case 1://gps
    {   error = ncCamSetTimestampMode(*cam, GPS_TIMESTAMP);
        if (error!=0)
        {
         log.writetoVerbose("Unable to set the gps timestamp.");
         log.writeto("Nuvu error: "+std::to_string(error));
         return -1;
        }
        else
        {
            log.writetoVerbose("GPS timestamp set succesfully.");
            return 0;
        }
    }
    case 2://internal
    {
        error = ncCamSetTimestampMode(*cam, INTERNAL_TIMESTAMP);
        if (error!=0)
        {
            log.writetoVerbose("Unable to set the internal timestamp.");
            log.writeto("Nuvu error: "+std::to_string(error));
            return -1;
        }
        else
        {
            log.writetoVerbose("Internal timestamp set succesfully.");
            return 0;
        }
    }
    case 3://no time stamp
    {
        error = ncCamSetTimestampMode(*cam, NO_TIMESTAMP);
        if (error!=0)
        {
            log.writetoVerbose("Unable to set no timestamp.");
            log.writeto("Nuvu error: "+std::to_string(error));
            return -1;
        }
        else
        {
            log.writetoVerbose("No timestamp set succesfully.");
            return 0;
        }
    }

    }//switch


 return 0;
}
int applyBias(NcCam *cam,struct initParam *param,struct camParam *detParam)
{ //This function 1st check if the shutter is open. If it is it will close it.
    //If the shutter was open it will be reopen after the bias frame are taken.
    //The the read out mode is probed. If the RO mode is set to EM and the gain <1000
    //The LM mode is set for the bias processing method. If gain>=1000 the Pc procissing
    //mode will be set. If the conventional mode is set the LM method is used.

    int ro_mode,VerHz,HorHz;
    enum Ampli	ncAmpliNo;
    enum ShutterMode shutterMode;
    int toReOpen=0;
    char Ampli[32];
    if (ncCamGetShutterMode(*cam,1,&shutterMode)!=0)
    {   log.writetoVerbose("Unable to get the shutter mode");
        return -1;
    }

    else if (shutterMode!=2)
    {   toReOpen=1;//reopen the the shutter after the acquisition of the bias
        if (ncCamSetShutterMode(*cam,CLOSE)!=0)
        {
            log.writetoVerbose("Unable to set the shutter mode");
            return -1;
        }

    }
    if (isInAcq==1)
    {
        log.writetoVerbose("Unable to start the bias acquisition because an exposition is in progress. Please stop the acquisition first");
        return -2;
    }
    else
    {
       if ( ncCamGetCurrentReadoutMode(*cam,&ro_mode,&ncAmpliNo,Ampli,&VerHz,&HorHz)!=0)
       {    log.writetoVerbose("Unable to get the readout mode");
           return -1;
       }
       if (ro_mode<4 || ro_mode>=12)
       {
           //Em mode
           if (ncCamCreateBias(*cam,param->nbBiasEm,BIAS_DEFAULT)!=0)
           {    log.writetoVerbose("Unable to start the bias acquisition");
               return -1;
           }
           if (detParam->gain>=1000)
           {
               if (ncCamSetProcType(*cam,PC,0)!=0)
               {
                   log.writeto("Unable to set the processing type of the bias");
                   return -1;
               }
           }
           else if (detParam->gain<1000)
           {
               if (ncCamSetProcType(*cam,LM,0)!=0)
               {
                   log.writeto("Unable to set the processing type of the bias");
                   return -1;
               }
           }
       }
       else
       {
           if (ncCamCreateBias(*cam,param->nbBiasConv,BIAS_DEFAULT)!=0)
           {    log.writetoVerbose("Unable to start the bias acquisition");
               return -1;
           }
           if (ncCamSetProcType(*cam,LM,0)!=0)
           {
               log.writeto("Unable to set the processing type of the bias");
               return -1;
           }
       }

    }
    if (toReOpen==1)
    {
        if (ncCamSetShutterMode(*cam,OPEN)!=0)
        {
            log.writetoVerbose("Unable to set the shutter mode");
            return -1;
        }
    }
    biasOK=1;
    return 0;
}

int get_inc(struct initParam *param)
{   char buf[256];
    std::string command = "python "+param->pathPython+"last_file.py ";
    std::string path=param->racinePath+create_name();
    path=path.substr(0,path.size()-1);//remove last caracter "_"

    path+='/';
    command+=path;
    char com[200];
    const char * Com = command.c_str();
    std::cout<<Com<<std::endl;
    sprintf(com,Com);



    FILE *file = popen(com,"r");

    while (fgets(buf, sizeof(buf), file) != 0)
    {

    }
    pclose(file);
    if (atol(buf)<0)
    {
        return 1;
    }
    else
    {
        return atol(buf);
    }


}

double getPosRot(void)
{
    char buf[256];
    std::string command = "position 1";
    char com[200];
    const char * Com = command.c_str();
    sprintf(com,Com);
    FILE *file = popen(com,"r");

    while (fgets(buf, sizeof(buf), file) != 0)
    {

    }
    pclose(file);
    return atof(buf);
}

double getPosBras(void)
{
    char buf[256];
    std::string command = "position 2";
    char com[200];
    const char * Com = command.c_str();
    sprintf(com,Com);
    FILE *file = popen(com,"r");

    while (fgets(buf, sizeof(buf), file) != 0)
    {

    }
    pclose(file);
    return atof(buf);
}
int preACQ(struct camParam *cam)
{
    if (developpement==0)
    {
        cam->rotAngle = getPosRot();
        cam->rotBras = getPosBras();
    }



    return 0;
}

std::string decToString(double dec){

    int hour,min;
    std::string time;
    hour = (int)dec;
    min = (int)(60*(dec-hour));
    double sec = 60*(60*(dec-hour)-min);
    time = std::to_string(hour)+":"+std::to_string(min)+":"+std::to_string(sec);
    return time;
}
