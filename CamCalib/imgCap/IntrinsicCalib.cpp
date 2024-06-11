#include <stdio.h>
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <conio.h>

#define cameraIndex 0

//#define UsingChessboardDist

int CHECKERBOARD_DIST = 25;// unit:mm

int CHECKERBOARD[2]{ 9 - 1,7 - 1 };// Defining the dimensions of checkerboard

// @brief List all  available camera (max : 10)
void findAvailableCameras(std::string ResultPath ) {
	std::vector<int> available_cameras;
	for (int i = 0; i < 10; ++i) {
		cv::VideoCapture cap(i);
		if (!cap.isOpened()) {
			continue;
		}
		else {
			available_cameras.push_back(i);
			cap.release();
		}
	}
	if (!available_cameras.empty()) {
		std::cout << "cameraAvailable:" << std::endl;
		for (int camera : available_cameras) {
			std::cout << "camera " << camera << std::endl;
		}
	}
	else {
		std::cout << "Can't find any camera" << std::endl;
	}
}

// @brief Captrue images
// @param camIndex :The camera Index
// @param calibImgsNum :The number of images
// @param calibImgPath :The path of images
// @return :The state of function
int ImgCap(int camIndex,int calibImgsNum , std::string calibImgPath){

	cv::VideoCapture video_capture;

	video_capture.open(camIndex);

	if (video_capture.isOpened()){

		video_capture.set(cv::CAP_PROP_FRAME_WIDTH, 1920);//640

		video_capture.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);//480

	}

	if (!video_capture.isOpened()){

		std::cout << "failed to capture!" << std::endl;

		return -1;
	}

	std::string imgName;

	int f = 1;

	int ch;

	while (f <= calibImgsNum){

		cv::Mat frame;

		video_capture >> frame;

		if (frame.empty()){

			break;

		}
		cv::namedWindow("win", 0);

		cv::imshow("win", frame);

		if (_kbhit()){

			ch = _getch();

			if (ch == 65) {

				imgName = std::to_string(f++);

				cv::imwrite(calibImgPath + imgName + ".jpg", frame);

				std::cout << "Capture "<< imgName<<"th image" << std::endl;

			}

		}

		char key = cv::waitKey(30);

	}

}

bool IntrinsicCalib(std::string calibImgPath , std::string ResultPath){

	std::vector<std::vector<cv::Point3f> > objpoints;// Creating vector to store vectors of 3D points for each checkerboard image

	std::vector<std::vector<cv::Point2f> > imgpoints;// Creating vector to store vectors of 2D points for each checkerboard image

	std::vector<cv::Point3f> objp;// Defining the world coordinates for 3D points

#ifndef UsingChessboardDist
	for (int i{ 0 }; i < CHECKERBOARD[1]; i++){

		for (int j{ 0 }; j < CHECKERBOARD[0]; j++){

			objp.push_back(cv::Point3f(j, i, 0));

		}

	}
#else
	for (int i{ 0 }; i < CHECKERBOARD[1] * CHECKERBOARD_DIST; i = i + CHECKERBOARD_DIST){

		for (int j{ 0 }; j < CHECKERBOARD[0] * CHECKERBOARD_DIST; j = j + CHECKERBOARD_DIST) {

			objp.push_back(cv::Point3f(j, i, 0));

		}

	}

#endif

	
	std::vector<cv::String> images;// Extracting path of individual image stored in a given directory

	std::string path = calibImgPath + "*.jpg";	// Path of the folder containing checkerboard images

	cv::glob(path, images);

	cv::Mat frame, gray;

	std::vector<cv::Point2f> corner_pts;// vector to store the pixel coordinates of detected checker board corners 

	bool success;

	int k = 0;

	for (const auto& imagePath : images) {

		std::cout << imagePath << std::endl;

	}

	for (int i{ 0 }; i < images.size(); i++){

		frame = cv::imread(images[i]);

		cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

		
		
		success = cv::findChessboardCorners(gray, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, // If desired number of corners are found in the image then success = true  
			
		cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE);// Finding checker board corners

		if (success){

			cv::TermCriteria criteria(cv::TermCriteria::MAX_ITER + cv::TermCriteria::EPS, 30, 0.001);

			// refining pixel coordinates for given 2d points.
			cv::cornerSubPix(gray, corner_pts, cv::Size(11, 11), cv::Size(-1, -1), criteria);

			// Displaying the detected corner points on the checker board
			cv::drawChessboardCorners(frame, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, success);

			objpoints.push_back(objp);

			imgpoints.push_back(corner_pts);
		}
		else{
			
			std::cout << "not success " << std::endl;

			std::cout << images[i] <<"can't detect chessboard " << std::endl;

		}

		cv::imshow("Image", frame);

		std::string imgName;

		imgName = std::to_string(k++);

		cv::imwrite(ResultPath + imgName + ".jpg", frame);

		cv::waitKey(15);

	}
	
	cv::destroyAllWindows();

	cv::Mat cameraMatrix, distCoeffs, R, T;

	double RPE;

	RPE = cv::calibrateCamera(objpoints, imgpoints, cv::Size(gray.rows, gray.cols), cameraMatrix, distCoeffs, R, T);

	std::cout << "RPE : " << RPE << std::endl;

	std::cout << "cameraMatrix : " << cameraMatrix << std::endl;

	std::cout << "distCoeffs : " << distCoeffs << std::endl;

	std::cout << "Rotation vector : " << R << std::endl;

	std::cout << "Translation vector : " << T << std::endl;

	std::ofstream ofs;

	ofs.open(ResultPath + "CamParam.txt");

	if (!ofs.is_open()) {

		std::cout << "Failed to open file.\n";
	}

	else {
	
		ofs << "cameraMatrix : " << cameraMatrix << "\n";
		
		ofs << "distCoeffs : " << distCoeffs << "\n";
		
		ofs << "Rotation vector : " << R << "\n";
		
		ofs << "Translation vector : " << T << "\n";
		
		ofs.close();
	
	}

	return true;

} 

int main() {
	//findAvailableCameras();

	int calibImgsNum = 15;

	int camIndex = cameraIndex;

	std::string calibImgPath = ".//ImgSet//";

	int imgCapState = ImgCap(camIndex , calibImgsNum , calibImgPath);

	if (imgCapState == -1){

		return -1;

	} 

	std::string ResultPath = ".//Calib_Result//";

	bool calibState = IntrinsicCalib(calibImgPath, ResultPath);

	if (calibState == false) {

		return -1;

	}

	return 0;

}