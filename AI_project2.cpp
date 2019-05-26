//
//  main.cpp
//  AIProject2
//
//  Created by David Slakter on 11/29/18.
//  Copyright © 2018 David Slakter. All rights reserved.
//

#include <iostream>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <math.h>

#define inputLayerSize 784;
#define hiddenLayerSize 50;
#define outputLayerSize 5;

using namespace std;

double sigmoid(double x){
     return 1 / (1 + exp(-x));
}

double sigmoidDerivative(double x){
    return sigmoid(x) * (1 - sigmoid(x));
}

class Neuron;

struct link {
    double weight;
    double gradient;
};

class Neuron {

    public:
    double value;
    int neuronIndex;
    
    
    vector<link> outLinks;
    
    Neuron(int index): neuronIndex(index){}
    
    void updateHiddenGradient(double realValue, vector<Neuron*> prevLayer){
        double error = realValue - value;
        for(size_t i = 0; i < prevLayer.size(); i++){
            prevLayer[i]->outLinks[neuronIndex].gradient = error * sigmoidDerivative(prevLayer[i]->outLinks[neuronIndex].weight);
        }
    }

    void updateInputGradient(const vector<Neuron*> outputLayer, const vector<Neuron*> inputLayer){
        //neuron is on the hidden layer, carry over the gradient from the output layer
        double prevGradientSum = 0.0;
        for (size_t i = 0; i < outputLayer.size(); i++){
            prevGradientSum += outLinks[i].gradient * outLinks[i].weight;
        }
        for (size_t k = 0;  k < inputLayer.size(); k++){
            inputLayer[k]->outLinks[neuronIndex].gradient = sigmoidDerivative(inputLayer[k]->outLinks[neuronIndex].weight)*prevGradientSum;
        }
        
    }
    void updateWeights(vector<Neuron*> prevLayer){
        for (size_t i = 0; i < prevLayer.size(); i++){
            double prevValue = prevLayer[i]->value;
            prevLayer[i]->outLinks[neuronIndex].weight += prevLayer[i]->outLinks[neuronIndex].gradient*prevValue;
        }
    }
};

class Network {
    
    vector<Neuron*> inputNeurons;
    vector<Neuron*> hiddenNeurons;
    vector<Neuron*> outputNeurons;
    
    Neuron* biasNeuron;
    
    public:
    double lastAvgError;
    double currentCorrectGuesses;
    bool training;
    vector<vector<int>> cMatrix;
    
    Network(): currentCorrectGuesses(0) {
        
        //initialize the neurons in each layer
        int ils = inputLayerSize
        for (int k = 0; k < ils; k++) {
            Neuron *newNeuron = new Neuron(k);
            inputNeurons.push_back(newNeuron);
            
        }
        biasNeuron = new Neuron(-1);
        biasNeuron->value = -1.0;
        inputNeurons.push_back(biasNeuron);
        
        
        int hls = hiddenLayerSize;
        for (int j = 0; j < hls; j++){
            Neuron* hiddenNeuron = new Neuron(j);
            hiddenNeurons.push_back(hiddenNeuron);
        }
        int ols = outputLayerSize;
        for (int i = 0; i < ols; i++){
            Neuron* outputNeuron = new Neuron(i);
            outputNeurons.push_back(outputNeuron);
            vector<int> cRow;
            for (int l = 0; l < ols; l++){
                cRow.push_back(0);
            }
            cMatrix.push_back(cRow);
        }
        
        //initialize the weights and links from the input and hidden layer
        for (Neuron* iNeuron: inputNeurons){
            for (size_t j = 0; j < hiddenNeurons.size(); j++){
                link newLink;
                //set the weight to be random float number between 0 and 1
                newLink.weight = (rand() % 100)/100.0;
                iNeuron->outLinks.push_back(newLink);
            }
        }
        
        for (Neuron* hNeuron: hiddenNeurons){
            for (size_t i = 0; i < outputNeurons.size(); i++){
                link newLink;
                newLink.weight = (rand() % 100)/100.0;
                hNeuron->outLinks.push_back(newLink);
            }
        }
    
    }
    
    
    //feed the values in through the forward direction of the network, update the hidden and output values accordingly
    void feedForward(const vector<int> &values){
        for (size_t k = 1; k < values.size(); k++) {
            inputNeurons[k]->value = values[k];
        }
        int hls = hiddenLayerSize;
        
        for (int j = 0; j < hls; j++){
            double sum = 0.0;
            for (size_t a = 0; a < inputNeurons.size(); a++){
                double weight = inputNeurons[a]->outLinks[j].weight;
                sum += inputNeurons[a]->value*weight;
            }
            hiddenNeurons[j]->value = sigmoid(sum);
            
        }
        int ols = outputLayerSize;
        for (int i = 0; i < ols; i++){
            double sum = 0.0;
            for (size_t a = 0; a < hiddenNeurons.size(); a++){
                double weight = hiddenNeurons[a]->outLinks[i].weight;
                sum += hiddenNeurons[a]->value*weight;
            }
            outputNeurons[i]->value = sigmoid(sum);
        }
    }
    
    //calculate the deviation of the output from the real values
    void calculateError(vector<float> realValues){
        double currentAvgError = 0.0;
        double maxValue = outputNeurons[0]->value;
        size_t maxValueInd = 0;
         for (size_t i = 0; i < outputNeurons.size(); i++){
             double error = realValues[i] - outputNeurons[i]->value;
             double squaredError = error*error;
             currentAvgError += squaredError;
            cout << outputNeurons[i]->value << endl;
            if (outputNeurons[i]->value > maxValue){
                maxValue = outputNeurons[i]->value;
                maxValueInd = i;
            }
        }
        currentAvgError = 0.5*currentAvgError;
    
        
        
        cout << maxValueInd << endl;
        
        //if network is currently testing, check and record if the network was right. update the confusion matrix;
        if (!training){
            size_t trueIndex = 0;
            for (size_t i = 0; i < realValues.size(); i++){
                if (realValues[i] == 1) {
                    trueIndex = i;
                }
            }
            if (trueIndex == maxValueInd){
                currentCorrectGuesses += 1;
            }
            cMatrix[trueIndex][maxValueInd] += 1;
        }
        
        
        //check if the average error changed a certain amount from the last iteration, if it didnt end training
        if (abs(lastAvgError - currentAvgError) > 0.00000001){
            lastAvgError = currentAvgError;
        }
        else{
            training = false;
        }
        
    }

    //update the gradients weights in the reverse order of the network
    void backProp(vector<float> realValues){
        for (size_t i = 0; i < outputNeurons.size(); i++){
            outputNeurons[i]->updateHiddenGradient(realValues[i], hiddenNeurons);
        }
        for (size_t n = 0; n < outputNeurons.size(); n++) {
            outputNeurons[n]->updateWeights(hiddenNeurons);
        }
        for (size_t j = 0; j < hiddenNeurons.size(); j++){
            hiddenNeurons[j]->updateInputGradient(outputNeurons, inputNeurons);
        }
        for (size_t n = 0; n < hiddenNeurons.size(); n++) {
            hiddenNeurons[n]->updateWeights(inputNeurons);
        }
        
    }

};

//read in new 784 pixel values from file
void updatePixelValues(vector<int> &pixelValues, FILE* file){
    short c;
    pixelValues.clear();
    for (int k = 0; k < 784; k++){
        c = getc(file);
        pixelValues.push_back(c);
    }
}

int main() {

    vector<int> pixelValues;
    
    ifstream trainLabels;
    ifstream testLabels;
    trainLabels.open("train_labels.txt");
    testLabels.open("test_labels.txt");
    
    if (!trainLabels){
        cout << "could not open train labels" << endl;
    }
    if (!testLabels){
        cout << "could not open test labels" << endl;
    }
    float zeroLabel;
    float oneLabel;
    float twoLabel;
    float threeLabel;
    float fourLabel;
    
     FILE *fp1;
     char file1[20];
     /* ask questions */
     cout << "Enter training images name: " << endl;
     gets(file1);
    if((fp1 = fopen(file1, "rb")) == NULL) {
        printf("cannot open %s\n", file1);
        exit(1);
    }
    
    printf("TRAINING...\n");
    updatePixelValues(pixelValues, fp1);

    
    int trainingCounter = 0;
    
    Network neuralNetwork = Network();
    neuralNetwork.training = true;
    
    while (neuralNetwork.training) {

        //update true values from the labels file
        trainLabels >> skipws >> zeroLabel >> oneLabel >> twoLabel >> threeLabel >> fourLabel;
        
        vector<float> realLabels;
        realLabels.push_back(zeroLabel);
        realLabels.push_back(oneLabel);
        realLabels.push_back(twoLabel);
        realLabels.push_back(threeLabel);
        realLabels.push_back(fourLabel);
        
        neuralNetwork.feedForward(pixelValues);
        cout << "train: " << trainingCounter << endl;
        neuralNetwork.calculateError(realLabels);
        neuralNetwork.backProp(realLabels);
    
        updatePixelValues(pixelValues, fp1);
        
        trainingCounter++;
        
        //reached max training number
        if (trainingCounter >= 28000){
            neuralNetwork.training = false;
        }
    }
    
    
    FILE *fp2;
    char file2[20];
    cout << "Training done." << endl;
    cout << "Enter testing images name: " << endl;
    gets(file2);
    if((fp2 = fopen(file2, "rb")) == NULL) {
        printf("cannot open %s\n", file2);
        exit(1);
    }
    
    
    
    int testCounter = 0;
    while (testCounter < 100){
        updatePixelValues(pixelValues, fp2);
        testLabels >> skipws >> zeroLabel >> oneLabel >> twoLabel >> threeLabel >> fourLabel;
        
        vector<float> realLabels;
        realLabels.push_back(zeroLabel);
        realLabels.push_back(oneLabel);
        realLabels.push_back(twoLabel);
        realLabels.push_back(threeLabel);
        realLabels.push_back(fourLabel);
        
        neuralNetwork.feedForward(pixelValues);
        cout << "test: " << testCounter << endl;
        neuralNetwork.calculateError(realLabels);
        
        testCounter++;
    }
    
    cout << "Confusion Matrix: " << endl;
    for (size_t i = 0; i < neuralNetwork.cMatrix.size(); i++){
        for (size_t j = 0; j < neuralNetwork.cMatrix[0].size(); j++){
            cout << neuralNetwork.cMatrix[i][j] << ' ';
        }
        cout << endl;
    }
    
    
    //calculates the average correct over the testing samples
    float avgCorrect = neuralNetwork.currentCorrectGuesses/(float)testCounter;
    cout << "average correct " << avgCorrect << endl;

    trainLabels.close();
    testLabels.close();
    fclose(fp1);
    
}
   

