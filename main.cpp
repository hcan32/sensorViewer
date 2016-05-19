#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <dirent.h>
#include <vector>
#include <iostream>
#include "compNat.hpp"
#include <fstream>

#define BUTTON_WIDTH 400
#define BUTTON_HEIGHT 50

#define FRAME_WIDTH 720
#define FRAME_HEIGHT 480

#define MAX_LASER_GAP 30 // 30 metre
#define LASER_WIDTH 720
#define LASER_HEIGHT 240

using namespace cv;
using namespace std;

bool play = true , stop = false , beginning = false;

vector<string> split(string str, char delimiter) {
	vector<string> internal;
	stringstream ss(str);
	string tok;
	while (getline(ss, tok, delimiter)) {
		internal.push_back(tok);
	}
	return internal;
}



void drawHistogram(string curData, Mat &currentFrame){
	vector<string> splitCurData = split(curData, ';');
	vector<double> gaps;
	//cout << "size : " << splitCurData.size() << endl;
	//cout << "last : " << splitCurData.size() - (45 * 4);

	double currentGap = 0;
	for (int i = 180; i < splitCurData.size() - (45 * 4); i++){
		currentGap = atof(splitCurData[i].c_str());
		if (0 && currentGap == 1.0){
			currentGap = 30;
			gaps.push_back(currentGap);
		}
		gaps.push_back(currentGap / 1000.0);
	}
	for (int i = 0; i < gaps.size(); i++){
		line(currentFrame, Point(i, LASER_HEIGHT), Point(i, LASER_HEIGHT - ((gaps[i] * LASER_HEIGHT) / MAX_LASER_GAP)), Scalar(255, 255, 255), 1);
	}

}




void callBackFunc(int event, int x, int y, int flags, void* userdata)
{
    if (event == EVENT_LBUTTONDOWN){
        cout << "Clicked x: "<<x<<" y : "<<y<<endl;
        if( (x>10 && x<(BUTTON_WIDTH / 5) -10 )  && ( y < FRAME_HEIGHT && y> FRAME_HEIGHT - BUTTON_HEIGHT ) ){
            cout << "PLAY\n";
            play = true;
            stop = false;
        }
        else if( ( x>(BUTTON_WIDTH / 5) + 10 && x< 2*(BUTTON_WIDTH / 5 ) - 10)  && ( y < FRAME_HEIGHT && y> FRAME_HEIGHT - BUTTON_HEIGHT ) ){
            cout << "STOP\n";
            play = false;
            stop = true;
        }
        else if( ( x>2*(BUTTON_WIDTH / 5) + 10 && x< 3*(BUTTON_WIDTH / 5 ) - 10)  && ( y < FRAME_HEIGHT && y> FRAME_HEIGHT - BUTTON_HEIGHT ) ){
            cout << "BEGINNING\n";
            play = false;
            beginning = true;
        }
    }

}


void drawBar(Mat &curFrame){
    rectangle(curFrame,cv::Point(0,FRAME_HEIGHT-BUTTON_HEIGHT),cv::Point(FRAME_WIDTH, FRAME_HEIGHT),cv::Scalar(255, 255, 255),CV_FILLED);
    // play button
    circle(curFrame,Point( FRAME_WIDTH / 6 ,FRAME_HEIGHT-(BUTTON_HEIGHT / 2)  ), (BUTTON_HEIGHT / 2) - 5, cv::Scalar(0, 255, 0),2);

}


int main(int argc, char *argv[])
{
    ifstream imuFile;
    string imuData;
    imuFile.open("imu.txt");


/*
    while( getline(imuFile,imuData) ){
        //imuFile >> imuData;


        string currentAccelerator = imuData.substr(firstIndex+4,secondIndex-firstIndex);
        string currentGyroscope = imuData.substr(secondIndex+4,thirdIndex-(secondIndex+4));
        vector<string> acceleratorValues = split(currentAccelerator,',');
        vector<string> gyroscopeValues = split(currentGyroscope,',');

        for(int i=0 ; i<acceleratorValues.size();i++){
            cout << "   Deger " << i << " :" << acceleratorValues[i] << " , ";
        }

        cout << endl;

        for(int i=0 ; i<gyroscopeValues.size();i++){
            cout << "   Deger " << i << " :" << gyroscopeValues[i] << " , ";
        }

        getchar();
    }
    return 0;
*/
    char *leftDirectoryName = "./left/";
    char *rightDirectoryName = "./right/";
    char *searchType = ".png";
    DIR *leftDir,*rightDir;
    vector<String> leftFileNames,rightFileNames;
    struct dirent *leftEnt,*rightEnt;
    if ( ( leftDir = opendir (leftDirectoryName) ) != NULL && (rightDir = opendir (rightDirectoryName) ) != NULL ) {
      /* print all the files and directories within directory */
      while ( (leftEnt = readdir (leftDir)) != NULL &&  (rightEnt = readdir (rightDir)) != NULL ) {
        if(strstr(leftEnt->d_name,searchType)!=NULL && strstr(rightEnt->d_name,searchType)!=NULL){
            //printf ("%s\n", ent->d_name);
            leftFileNames.push_back(string(leftEnt->d_name));
            rightFileNames.push_back(string(rightEnt->d_name));
        }
      }
      closedir (leftDir);
      closedir (rightDir);
    }else {
      perror ("");
      return -1;
    }
    sort(leftFileNames.begin(),leftFileNames.end(),compareNat);
    sort(rightFileNames.begin(),rightFileNames.end(),compareNat);

	ifstream myReadFile;
	myReadFile.open("laser.txt");
	string currentData;
	int lineCounter = 0;
	char key = 0;

    Mat leftCurrentFrame,rightCurrentFrame;
    string currentGPS = "NO GPS";
	while ( !myReadFile.eof() && key !='q' && getline(imuFile,imuData) && lineCounter < leftFileNames.size()-2 ){
		myReadFile >> currentData;
		lineCounter++;

		if (lineCounter> 1){

            int gpsIndex = imuData.find(", 1,");
            int firstIndex = imuData.find(", 3,") ;
            int secondIndex = imuData.find(", 4,");
            int thirdIndex = imuData.find(", 5,");
            int endIndex = imuData.find(", 6,");

            if(gpsIndex != -1)
                currentGPS = imuData.substr(gpsIndex+4,firstIndex-(gpsIndex+4));
            string currentAccelerator = imuData.substr(firstIndex+4,secondIndex-(firstIndex+4));
            string currentGyroscope = imuData.substr(secondIndex+4,thirdIndex-(secondIndex+4));
            string currentMagnetometer;

            if(gpsIndex == -1)
                currentMagnetometer = imuData.substr(thirdIndex+4,imuData.size());
            else
                currentMagnetometer = imuData.substr(thirdIndex+4,endIndex -(thirdIndex+4));
            //vector<string> acceleratorValues = split(currentAccelerator,',');
            //vector<string> gyroscopeValues = split(currentGyroscope,',');
			Mat graphData = Mat::zeros(LASER_HEIGHT, LASER_WIDTH, CV_8UC3);
			drawHistogram(currentData, graphData);
			string leftFilePath = string(leftDirectoryName) + leftFileNames[lineCounter];
			string rightFilePath  = string(rightDirectoryName) + rightFileNames[lineCounter];
            leftCurrentFrame = imread(leftFilePath);
            rightCurrentFrame = imread(rightFilePath);
            //resize(currentFrame , currentFrame , Size(currentFrame.cols / rate ,currentFrame.rows / rate) );
            cout << "LEFT : " << leftFilePath << endl;
            cout << "RIGHT : " << rightFilePath << endl;
            resize( leftCurrentFrame , leftCurrentFrame , Size(FRAME_WIDTH / 2,FRAME_HEIGHT /2) );
            resize( rightCurrentFrame , rightCurrentFrame , Size(FRAME_WIDTH / 2,FRAME_HEIGHT/2) );
            //Mat concatFrame ;

            Mat imuGraph = Mat::zeros(150, LASER_WIDTH, CV_8UC3);

            putText(imuGraph,"Accelerator : " + currentAccelerator,Point(50,30),FONT_HERSHEY_PLAIN,1.0,Scalar(255,255,255));
            putText(imuGraph,"Gyroscope   : " + currentGyroscope,Point(50,60),FONT_HERSHEY_PLAIN,1.0,Scalar(255,255,255));
            putText(imuGraph,"Magnetometer   : " + currentMagnetometer,Point(50,90),FONT_HERSHEY_PLAIN,1.0,Scalar(255,255,255));
            putText(imuGraph,"GPS         : " + currentGPS,Point(50,120),FONT_HERSHEY_PLAIN,1.0,Scalar(255,255,255));

            hconcat(leftCurrentFrame,rightCurrentFrame,leftCurrentFrame);
            vconcat(graphData,leftCurrentFrame,leftCurrentFrame);
            vconcat(leftCurrentFrame,imuGraph,leftCurrentFrame);
            //imshow("Left",leftCurrentFrame);
            //imshow("Right",rightCurrentFrame);
            //imshow("Graph",graphData);
            imshow("CONCAT",leftCurrentFrame);
		}
		key = waitKey(500);
	}
	myReadFile.close();
    imuFile.close();

    return 0 ;


    char quitKey = 0;
    Mat frame ;



    //Mat playButtons = imread("playerIcon.jpg");
    //resize(playButtons,playButtons,Size(BUTTON_WIDTH,BUTTON_HEIGHT));
    //int playWidth = playButtons.cols;
    //int playHeight = playButtons.rows;

    namedWindow("Frame");
    setMouseCallback("Frame", callBackFunc);
    int i=0;
    while(quitKey!='q' && i < leftFileNames.size()){
        if(play){
            cout << leftFileNames[i]<<endl;
            string filePath = string(leftDirectoryName) + leftFileNames[i];
            leftCurrentFrame = imread(filePath);
            int width = leftCurrentFrame.cols;
            int height = leftCurrentFrame.rows;
            //playButtons.copyTo(currentFrame.rowRange(height-playHeight,height).colRange(0,playWidth));
            //drawBar(currentFrame);
            imshow("Frame",leftCurrentFrame);
            quitKey = waitKey(200);
            i++;
        }else if(stop){
            imshow("Frame",leftCurrentFrame);
            quitKey = waitKey(200);
        }else if(beginning){
            i = 0 ;
            beginning = false;
            play = true;
        }
    }


    return 0;
}



