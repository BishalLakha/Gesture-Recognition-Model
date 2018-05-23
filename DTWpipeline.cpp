

#include <iostream>
#include "GRT.h"
#include "Serial.h"

#include <string>
#include <sstream>

#include <windows.h>

using namespace std;
using namespace GRT;

CSerial serial;
GestureRecognitionPipeline pipeline;



std::vector<float> parseData(char* data)
{
    //std::string input = ">4.576,2.34567,56.789,98.7654";
    std::string input = data;
    std::istringstream ss(input);
    std::string token;

    std::vector<float> parsedData;

    while(std::getline(ss, token, ','))
    {
        float num = strtof((token).c_str(),0);
        parsedData.push_back(num);
    }

    return parsedData;

}

int main()
{

//Create a new GestureRecognitionPipeline

 DTW dtw;
//Add a moving average filter (with a buffer size of 5 and for a 3 dimensional signal) as a pre-processing module to the start of the pipeline
    pipeline.addPreProcessingModule( MovingAverageFilter(5,3) );

//Set the classifier at the core of the pipeline, in this case we are using an Adaptive Naive Bayes Classifier
    pipeline.setClassifier( dtw );

//Add a class label timeout filter to the end of the pipeline (with a timeout value of 1 second), this will filter the predicted class output from the ANBC algorithm
    pipeline.addPostProcessingModule( ClassLabelTimeoutFilter(1000) );

//Load Train Data
//You can comment this section for real time
    LabelledTimeSeriesClassificationData trainingData;

    if(!trainingData.load( "trainData.txt" ))
    {
        cout << "Failed to load training data!\n";
        return EXIT_FAILURE;
    }

//load test data
//You can comment this section for real time
    LabelledTimeSeriesClassificationData testData ;

    if(!testData.load( "testData.txt" ))
    {
        cout << "Failed to load test data!\n";
        return EXIT_FAILURE;
    }

    //Train the pipeline
        trainingData.printStats();

    //Train the pipeline using the training data
    //You can comment this section for real time
        if( !pipeline.train( trainingData ) )
        {
            cout << "ERROR: Failed to train the pipeline!\n";
            return EXIT_FAILURE;
        }


        //Save the pipeline to a file
        //You can comment this for real time prediction
        if( !pipeline.save( "DTWpipeline.txt" ) )
        {
            cout << "ERROR: Failed to save the pipeline!\n";
            return EXIT_FAILURE;
        }

    //Load the pipeline from a file
    //You can use this section and comment training ,saving and test section for real time prediction
    if( !pipeline.load( "DTWpipeline.txt" ) )
    {
        cout << "ERROR: Failed to load the pipeline!\n";
        return EXIT_FAILURE;
    }


    //Test the pipeline using the test data
    //Comment this portion if you don't want to test
    if( !pipeline.test( testData ) )
    {
        cout << "ERROR: Failed to test the pipeline!\n";
        return EXIT_FAILURE;
    }

    //Print some stats about the testing
    cout << "Test Accuracy: " << pipeline.getTestAccuracy() << endl;

//Starting Real time


    if (serial.Open(3, 9600))
    {
        cout<<"Starting RealTime Prediction"<<endl;
        cout << "Port opened successful" << endl;
        cout<<"X Y Z values received from Gyroscope "<<endl;
        Sleep(1000);
        while(1)
        {
            MatrixDouble sample;
            sample.resize(50,3);

            for(int i = 0; i< 50; i++)
            {
                Sleep(300);
                char* lpBuffer = new char[500];
                int nBytesRead = serial.ReadData(lpBuffer, 500);
                // cout<<lpBuffer;

                //Try parsing data
                std::vector<float> data = parseData(lpBuffer);

                sample[i][0]=data[2];
                sample[i][1]=data[3];
                sample[i][2]=data[4];
                cout<<"X"<<i<<": "<<data[2]<<" ; "<<"Y"<<i<<": "<<data[3]<<" ; "<<"Z"<<i<<": "<<data[4]<<endl;
            }
//Perform the prediction
            bool predictionSuccess = pipeline.predict( sample );
//Predicted class label from the pipeline
            UINT predictedClassLabel = pipeline.getPredictedClassLabel();

            cout<<"Prediction Success:"<<predictionSuccess<<endl<<"Predicted ClassLabel: "<<predictedClassLabel<<endl;
            cout<<"Another run? (y/n)"<<endl;
            char ch;
            cin>>ch;
            if (ch == 'n')
            {
                break;
            }

        }
    }
    else
    {
        cout << "Port Failed :(" << endl;
    }
}



