/**
* This file is part of ORB-SLAM2.
*
* Copyright (C) 2014-2016 Raúl Mur-Artal <raulmur at unizar dot es> (University of Zaragoza)
* For more information see <https://github.com/raulmur/ORB_SLAM2>
*
* ORB-SLAM2 is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM2 is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ORB-SLAM2. If not, see <http://www.gnu.org/licenses/>.
*/


#include<iostream>
#include<algorithm>
#include<fstream>
#include<iomanip>
#include<chrono>

#include<opencv2/core/core.hpp>
//#include<opencv2/opencv.hpp>

#include<System.h>
#include<sys/time.h>

using namespace std;

int main(int argc, char **argv)
{
    if(argc != 6)
    {
        cerr << endl << "Usage: ./stereo_euroc path_to_vocabulary path_to_settings path_to_left_folder path_to_right_folder path_to_times_file" << endl;
        return 1;
    }

#if 1   
    int nImages_rt = 1000;
    cv::Mat frame_rt;
    double timestamp_rt = 0;
    cv::VideoCapture cap0("rkisp device=/dev/video6 io-mode=1 ! video/x-raw,format=NV12,width=800,height=448,framerate=30/1 ! videoconvert ! appsink ", cv::CAP_GSTREAMER);
    cv::VideoCapture cap1("rkisp device=/dev/video1 io-mode=1 ! video/x-raw,format=NV12,width=800,height=448,framerate=30/1 ! videoconvert ! appsink ", cv::CAP_GSTREAMER);

   if (! cap0.isOpened()){
        cout<<"Connection Failed!\n";
        return -1;
    }
   if (! cap1.isOpened()){
        cout<<"Connection Failed!\n";
   }
   struct timeval tv_rt;
   gettimeofday(&tv_rt,NULL);
#endif


    // Retrieve paths to images
    vector<string> vstrImageLeft;
    vector<string> vstrImageRight;
    vector<double> vTimeStamp;
/*
    LoadImages(string(argv[3]), string(argv[4]), string(argv[5]), vstrImageLeft, vstrImageRight, vTimeStamp);

    if(vstrImageLeft.empty() || vstrImageRight.empty())
    {
        cerr << "ERROR: No images in provided path." << endl;
        return 1;
    }

    if(vstrImageLeft.size()!=vstrImageRight.size())
    {
        cerr << "ERROR: Different number of left and right images." << endl;
        return 1;
    }
*/

    cv::Mat im_rtLeft, im_rtRight;

    cap0.read(im_rtLeft);
    cap1.read(im_rtRight);
    //cv::imshow("camera0",im_rtLeft);       
    //cv::imshow("camera1",im_rtRight);
    //cv::waitKey(0);
	
    // Read rectification parameters
    cv::FileStorage fsSettings(argv[2], cv::FileStorage::READ);
    if(!fsSettings.isOpened())
    {
        cerr << "ERROR: Wrong path to settings" << endl;
        return -1;
    }

    cv::Mat K_l, K_r, P_l, P_r, R_l, R_r, D_l, D_r;
    fsSettings["LEFT.K"] >> K_l;
    fsSettings["RIGHT.K"] >> K_r;

    fsSettings["LEFT.P"] >> P_l;
    fsSettings["RIGHT.P"] >> P_r;

    fsSettings["LEFT.R"] >> R_l;
    fsSettings["RIGHT.R"] >> R_r;

    fsSettings["LEFT.D"] >> D_l;
    fsSettings["RIGHT.D"] >> D_r;

    int rows_l = fsSettings["LEFT.height"];
    int cols_l = fsSettings["LEFT.width"];
    int rows_r = fsSettings["RIGHT.height"];
    int cols_r = fsSettings["RIGHT.width"];

    if(K_l.empty() || K_r.empty() || P_l.empty() || P_r.empty() || R_l.empty() || R_r.empty() || D_l.empty() || D_r.empty() ||
            rows_l==0 || rows_r==0 || cols_l==0 || cols_r==0)
    {
        cerr << "ERROR: Calibration parameters to rectify stereo are missing!" << endl;
        return -1;
    }

    cv::Mat M1l,M2l,M1r,M2r;
    cv::initUndistortRectifyMap(K_l,D_l,R_l,P_l.rowRange(0,3).colRange(0,3),cv::Size(cols_l,rows_l),CV_32F,M1l,M2l);
    cv::initUndistortRectifyMap(K_r,D_r,R_r,P_r.rowRange(0,3).colRange(0,3),cv::Size(cols_r,rows_r),CV_32F,M1r,M2r);


    //const int nImages = vstrImageLeft.size();

    // Create SLAM system. It initializes all system threads and gets ready to process frames.
    ORB_SLAM2::System SLAM(argv[1],argv[2],ORB_SLAM2::System::STEREO,true);

    // Vector for tracking time statistics
    vector<float> vTimesTrack;
    //vTimesTrack.resize(nImages);
    vTimesTrack.resize(nImages_rt);

    cout << endl << "-------" << endl;
    cout << "Start processing sequence ..." << endl;
    cout << "IIImages in the sequence: " << nImages_rt << endl << endl;

    // Main loop
    //cv::Mat imLeft, imRight,
    cv::Mat imLeftRect, imRightRect;

    for(int ni=0; ni<nImages_rt; ni++)
    {
        // Read left and right images from file
        //imLeft = cv::imread(vstrImageLeft[ni],CV_LOAD_IMAGE_UNCHANGED);
        //imRight = cv::imread(vstrImageRight[ni],CV_LOAD_IMAGE_UNCHANGED);
        //cap0>>im_rtLeft;
	//cap1>>im_rtRight;
        cap0.read(im_rtLeft);
	cap1.read(im_rtRight);
        //cv::imshow("camera0",im_rtLeft);       
        //cv::imshow("camera1",im_rtRight);
        //cv::waitKey(100);
        cout<<"This is the "<<ni<<"images"<<endl;	
        //cap0.read(im_rtLeft);
	//cap1.read(im_rtRight);
        //cv::imshow("camera0",im_rtLeft);       
        //cv::imshow("camera1",im_rtRight);
        //cv::waitKey(1000);
	//continue;
        //usleep(10000000);

        if(im_rtLeft.empty())
        {
            cerr << endl << "Failed to load image at: "
                 << string(vstrImageLeft[ni]) << endl;
            return 1;
        }

        if(im_rtRight.empty())
        {
            cerr << endl << "Failed to load image at: "
                 << string(vstrImageRight[ni]) << endl;
            return 1;
        }
	//cv::waitKey(0);

        cv::remap(im_rtLeft,imLeftRect,M1l,M2l,cv::INTER_LINEAR);
        cv::remap(im_rtRight,imRightRect,M1r,M2r,cv::INTER_LINEAR);

        //double tframe = vTimeStamp[ni];
        double tframe_rt = timestamp_rt+1;

#ifdef COMPILEDWITHC11
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
#else
        std::chrono::monotonic_clock::time_point t1 = std::chrono::monotonic_clock::now();
#endif

        // Pass the images to the SLAM system
        SLAM.TrackStereo(imLeftRect,imRightRect,tframe_rt);

#ifdef COMPILEDWITHC11
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
#else
        std::chrono::monotonic_clock::time_point t2 = std::chrono::monotonic_clock::now();
#endif

        double ttrack= std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1).count();

        vTimesTrack[ni]=ttrack;

        // Wait to load the next frame
	/*
        double T=0;
        if(ni<nImages-1)
            T = vTimeStamp[ni+1]-tframe;
        else if(ni>0)
            T = tframe-vTimeStamp[ni-1];

        if(ttrack<T)
            usleep((T-ttrack)*1e6);
	*/
	usleep(10000);
    }

    // Stop all threads
    SLAM.Shutdown();

    // Tracking time statisticsi
/*
    sort(vTimesTrack.begin(),vTimesTrack.end());
    float totaltime = 0;
    for(int ni=0; ni<nImages; ni++)
    {
        totaltime+=vTimesTrack[ni];
    }
    cout << "-------" << endl << endl;
    cout << "median tracking time: " << vTimesTrack[nImages/2] << endl;
    cout << "mean tracking time: " << totaltime/nImages << endl;
*/
    // Save camera trajectory
    SLAM.SaveTrajectoryTUM("CameraTrajectory.txt");

    return 0;
}
