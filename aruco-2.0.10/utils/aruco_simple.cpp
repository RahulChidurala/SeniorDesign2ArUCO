#include <cv.h>
#include "opencv2/opencv.hpp"
#include "aruco.h"

using namespace cv;
using namespace aruco;
using namespace std;
//get the closest marker later based on z or something

static float leftXThreshold = -0.10;
static float rightXThreshold = 0.10;

static int getDirectionFromTvec(Mat Tvec){
	float x, y, z;
	x = Tvec.ptr< float >(0)[0];
	y = Tvec.ptr< float >(0)[1];
	z = Tvec.ptr< float >(0)[2];

	if (x <= leftXThreshold){
		return 1;
	}
	if (x >= rightXThreshold){
		return 2;
	}

	//default return num for Stop
	return 0;
}

static String getJSON(vector<Marker> markers){
	int length = markers.size();

	//make sure the there is a detected marker
	if (length == 0){
		return "";
	}

	String jsonString = "{ \"Markers\": [";



	for (int i = 0; i < length; i++){
		jsonString = jsonString + "{ \"ID\": " + to_string(markers[i].id) + ", "
			+ "\"T\": { " + "\"x\": " + to_string(markers[i].Tvec.ptr< float >(0)[0]) + ", "
			+ "\"y\": " + to_string(markers[i].Tvec.ptr< float >(0)[1]) + ", "
			+ "\"z\": " + to_string(markers[i].Tvec.ptr< float >(0)[2]) + "}, "
			+ "\"R\": { " + "\"x\": " + to_string(markers[i].Rvec.ptr< float >(0)[0]) + ", "
			+ "\"y\": " + to_string(markers[i].Rvec.ptr< float >(0)[1]) + ", "
			+ "\"z\": " + to_string(markers[i].Rvec.ptr< float >(0)[2]) + "}}";
		
		if (i != length - 1){
			jsonString = jsonString + ",";
		}
	}

	jsonString = jsonString + "] }";

	return jsonString;
}

int main(int, char**)
{
	bool calculateLimits = false;

	//Images for direction arrows
	Mat leftArrow = imread("../../resources/LeftArrow.png", CV_LOAD_IMAGE_UNCHANGED);
	Mat rightArrow = imread("../../resources/RightArrow.png", CV_LOAD_IMAGE_UNCHANGED);
	Mat stop = imread("../../resources/Stop.png", CV_LOAD_IMAGE_UNCHANGED);
	Mat stop1 = imread("../../resources/Stop1.jpg", CV_LOAD_IMAGE_UNCHANGED);
	
	if (!leftArrow.data)                              // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	VideoCapture cap(0); // open the default camera
	if (!cap.isOpened())  // check if we succeeded
		return -1;
	Mat edges;
	namedWindow("edges", 1);

	MarkerDetector MDetector;
	vector<Marker> Markers;
	CameraParameters CamParam;
	CamParam.readFromXMLFile("out_camera_calibration.yml");

	//temp frame to get size from camera image
	Mat frameTemp;

	//approx 7.25 in square markers => 0.18415 m square markers
	//approx 4 in => 0.102 m temporary small marker
	float MarkerSize = 0.18415;
	cap >> frameTemp;

	//Get the limits of the window for prototyping atleast
	float Rxmin, Rxmax, Rymin, Rymax, Rzmin, Rzmax;
	float Txmin, Txmax, Tymin, Tymax, Tzmin, Tzmax;

	CamParam.resize(frameTemp.size());
	int imageNumber = 0;

	for (;;)
	{
		Mat frame;
		cap >> frame; // get a new frame from camera

		MDetector.detect(frame, Markers, CamParam, MarkerSize);
		//for each marker, draw info and its boundaries in the image
		for (unsigned int i = 0; i<Markers.size(); i++) {
			if (calculateLimits){
				for (unsigned int j = 0; j < 3; j++){
					float tempPointT = Markers[i].Tvec.ptr< float >(0)[j];
					float tempPointR = Markers[i].Rvec.ptr< float >(0)[j];
					if (j == 0){
						if (tempPointT < Txmin){
							Txmin = tempPointT;
						}
						else if (tempPointT > Txmax){
							Txmax = tempPointT;
						}

						if (tempPointR < Rxmin){
							Rxmin = tempPointR;
						}
						else if (tempPointR > Rxmax){
							Rxmax = tempPointR;
						}

					}
					else if (j == 1){
						if (tempPointT < Tymin){
							Tymin = tempPointT;
						}
						else if (tempPointT > Tymax){
							Tymax = tempPointT;
						}

						if (tempPointR < Rymin){
							Rymin = tempPointR;
						}
						else if (tempPointR > Rymax){
							Rymax = tempPointR;
						}
					}
					else if (j == 2){
						if (tempPointT < Tzmin){
							Tzmin = tempPointT;
						}
						else if (tempPointT > Tzmax){
							Tzmax = tempPointT;
						}

						if (tempPointR < Rzmin){
							Rzmin = tempPointR;
						}
						else if (tempPointR > Rzmax){
							Rzmax = tempPointR;
						}
					}

				}
			}
			
			//do it for multiple images
			imageNumber = getDirectionFromTvec(Markers[i].Tvec);
			
			/*
			cout << "ID " << Markers[i].id << endl;
			cout << "T " << Markers[i].Tvec.ptr< float >(0)[0] << " " << Markers[i].Tvec.ptr< float >(0)[1] << " " << Markers[i].Tvec.ptr< float >(0)[2] << endl;
			cout << "R " << Markers[i].Rvec.ptr< float >(0)[0] << " " << Markers[i].Rvec.ptr< float >(0)[1] << " " << Markers[i].Rvec.ptr< float >(0)[2] << endl;
			cout << endl;*/

			cout << getJSON(Markers) << endl;
			Markers[i].draw(frame, Scalar(0, 0, 255), 2);
			CvDrawingUtils::draw3dAxis(frame, Markers[i], CamParam);
			//CvDrawingUtils::draw3dCube(frame, Markers[i], CamParam, false);
		}

		/*cvtColor(frame, frame, COLOR_BGR2GRAY);
		GaussianBlur(frame, frame, Size(7, 7), 1.5, 1.5);
		Canny(frame, frame, 0, 30, 3);*/
		imshow("edges", frame);

		switch (imageNumber)
		{
			case 0:
				imshow("Directions", stop);
				break;
			case 1:
				imshow("Directions", leftArrow);
				break;
			case 2:
				imshow("Directions", rightArrow);
				break;
			default:
				imshow("Directions", stop1);
				break;
		}
		//opencv waitkey(s)
		//keeps image on console for 30ms before refreshing
		//returns -1 if no key is pressed, else returns keycode
		if (waitKey(30) >= 0){
			cout << "Rmin: (" << Rxmin << ", " << Rymin << ", " << Rzmin << ")" << endl;
			cout << "Rmax: (" << Rxmax << ", " << Rymax << ", " << Rzmax << ")" << endl;
			cout << "Tmin: (" << Txmin << ", " << Tymin << ", " << Tzmin << ")" << endl;
			cout << "Tmax: (" << Txmax << ", " << Tymax << ", " << Tzmax << ")" << endl; 
			waitKey(0);
			break;
		}
	}
	// the camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}