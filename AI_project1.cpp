//
//  main.cpp
//  AI_Project1
//
//  Created by David Slakter on 11/3/18.
//  Copyright © 2018 David Slakter. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

//data structure representing a node in the search tree
struct Node {
    int heuristicSum;
    int nodeDepth;
    vector<char> actions;
    tuple<int, int> emptyTileCoord;
    vector<vector<int>> puzzleData;
    
    //checks equivalency between the puzzleData states of nodes
    bool operator==(const Node &other){
        if (this->puzzleData == other.puzzleData){
            return true;
        }
        else{
            return false;
        }
    }
    
    //returns a new node with tile swapped.
    //If tile cannot be swapped, a nullptr is returned
    Node* swapTile(tuple<int, int> tile, const Node* currNode){
        if (get<0>(tile) < 0 || get<0>(tile) >= puzzleData[0].size() || get<1>(tile) < 0 || get<1>(tile) >= puzzleData.size()){
          return nullptr;
        }
        else{
            Node* nodeptr = new Node();
            for (size_t i = 0; i < currNode->puzzleData.size(); i++){
                vector<int> row;
                for(size_t j = 0; j < currNode->puzzleData[0].size(); j++){
                    row.push_back(currNode->puzzleData[i][j]);
                }
                nodeptr->puzzleData.push_back(row);
            }
            
            nodeptr->emptyTileCoord = tile;
            nodeptr->puzzleData[get<0>(emptyTileCoord)][get<1>(emptyTileCoord)] = currNode->puzzleData[get<0>(tile)][get<1>(tile)];
            nodeptr->puzzleData[get<0>(tile)][get<1>(tile)] = currNode->puzzleData[get<0>(emptyTileCoord)][get<1>(emptyTileCoord)];
            
            return nodeptr;
        
        }
       
    
    }
    
    //calculates the heuristic value f(n) = g(n) + h(n)
    //f(n) = nodeDepth + deltaX + deltaY
    //of a node for a given goal node.
    
    void updateHeuristicSum(Node* goalNode){
        heuristicSum = 0;
        for (size_t i = 0; i < puzzleData.size(); i++){
            for(size_t j = 0; j < puzzleData[0].size(); j++){
                int currTile = puzzleData[i][j];
                
                for (size_t k = 0; k < goalNode->puzzleData.size(); k++){
                    for (size_t m  = 0; m < goalNode->puzzleData[0].size(); m++){
                        if (goalNode->puzzleData[k][m] == currTile && (k != get<0>(emptyTileCoord) || m != get<1>(emptyTileCoord))){
                            int xDiff = abs((int)j - (int)m);
                            int yDiff = abs((int)i - (int)k);
                            heuristicSum += xDiff + yDiff;
                        }
                    }
                }
            }
        }
        heuristicSum += nodeDepth;
    }
};

//sorts a vector of nodes by their heuristic sums
bool sortFunction (Node* a, Node* b) { return (a->heuristicSum > b->heuristicSum); }

//returns the finished node which contains the finished actions to reach the goal node.
Node* aStarSearch(Node* startNode, Node* goalNode, unsigned long &nodesGenerated){
    vector<Node*> explored;
    vector<Node*> frontier;
    
    frontier.push_back(startNode);
    
    while (!frontier.empty()){
            size_t n = frontier.size() - 1;
            //check to see if node is a goal node
            if (*frontier[n] == *goalNode){
                break;
            }
            explored.push_back(frontier[n]);

            
            //expand chosen node;
            Node* rightNode = frontier[n]->swapTile(make_tuple(get<0>(frontier[n]->emptyTileCoord), get<1>(frontier[n]->emptyTileCoord)  + 1), frontier[n]);
            Node* leftNode = frontier[n]->swapTile(make_tuple(get<0>(frontier[n]->emptyTileCoord), get<1>(frontier[n]->emptyTileCoord)  - 1), frontier[n]);
            Node* upNode = frontier[n]->swapTile(make_tuple(get<0>(frontier[n]->emptyTileCoord) - 1, get<1>(frontier[n]->emptyTileCoord)), frontier[n]);
            Node* downNode = frontier[n]->swapTile(make_tuple(get<0>(frontier[n]->emptyTileCoord) + 1, get<1>(frontier[n]->emptyTileCoord)), frontier[n]);
            
            //update new nodes;
            for (size_t i = 0; i < frontier[n]->actions.size(); i++){
                
                if(rightNode != nullptr){
                    rightNode->actions.push_back(frontier[n]->actions[i]);
                }
                if(leftNode != nullptr){
                    leftNode->actions.push_back(frontier[n]->actions[i]);
                }
                if(upNode != nullptr){
                    upNode->actions.push_back(frontier[n]->actions[i]);
                }
                if(downNode != nullptr){
                    downNode->actions.push_back(frontier[n]->actions[i]);
                }
                
            }
            frontier.pop_back();
        
            vector<Node*> resultingNodes;
            if (rightNode != nullptr){
                rightNode->actions.push_back('R');
                rightNode->nodeDepth = frontier[n]->nodeDepth + 1;
                rightNode->updateHeuristicSum(goalNode);
                resultingNodes.push_back(rightNode);
            }
            if (leftNode != nullptr){
                leftNode->actions.push_back('L');
                leftNode->nodeDepth = frontier[n]->nodeDepth + 1;
                leftNode->updateHeuristicSum(goalNode);
                resultingNodes.push_back(leftNode);
            }
            if (upNode != nullptr){
                upNode->actions.push_back('U');
                upNode->nodeDepth = frontier[n]->nodeDepth + 1;
                upNode->updateHeuristicSum(goalNode);
                resultingNodes.push_back(upNode);
            }
            if (downNode != nullptr){
                downNode->actions.push_back('D');
                downNode->nodeDepth = frontier[n]->nodeDepth + 1;
                downNode->updateHeuristicSum(goalNode);
                resultingNodes.push_back(downNode);
            }
            
            
            for (Node* rNode: resultingNodes){
            
                for (Node* fNode: frontier){
                    if (*rNode == *fNode){
                        break;
                    }
                }
                for (Node* eNode: explored){
                    if (*rNode == *eNode){
                        break;
                    }
                }
                frontier.push_back(rNode);
            }
        
        
            //sort the frontier to have the nodes with low heuristic
            //functions at the end of the stack
            sort(frontier.begin(), frontier.end(), sortFunction);
    }
    nodesGenerated = frontier.size() + 1;
    //delete all nodes stored on the heap
    for (Node* n: explored){
        if (n != frontier.back() && n != startNode){
            delete n;
        }
    }
    for(Node* n: frontier){
        if(n != frontier.back() && n != startNode){
            delete n;
        }
    }
    
    //return solution
    return frontier.back();
}

int main() {
    unsigned long nodesGenerated = 0;
    Node* startNode;
    Node* goalNode;
    
    startNode = new Node();
    goalNode = new Node();

    //read input file
    string line;
    string inputFileSource;
    cout << "specify inputfile path: ";
    cin >> inputFileSource;
    ifstream inputFile(inputFileSource);
    int lineCount = 0;
    if (inputFile.is_open()){
        while (getline(inputFile, line)) {
            if (lineCount < 3){
                vector<int> row;
                for (int i = 0; i < line.size() - 1; i++){
                    if (line[i] != ' '){
                        row.push_back(line[i] - '0');
                    }
                    if (line[i] - '0' == 0){
                        startNode->emptyTileCoord = make_tuple(lineCount, i/2);
                    }
                }
                startNode->puzzleData.push_back(row);
            }
            else if (lineCount > 3){
                vector<int> row;
                for (int i = 0; i < line.size(); i++){
                    if (line[i] != ' ' && line[i] != 13){
                        row.push_back(line[i] - '0');
                    }
                }
                goalNode->puzzleData.push_back(row);
            }
            lineCount++;
        }
        inputFile.close();
    }
    else{
        cout << "couldn't read the file" << endl;
        exit(1);
    }
    
    startNode->updateHeuristicSum(goalNode);
    startNode->nodeDepth = 0;
    
    Node* finishedNode = aStarSearch(startNode, goalNode, nodesGenerated);
    
    
    //construct output file
    ofstream outputFile;
    string outputFileName;
    cout << "specify output file path: ";
    cin >> outputFileName;
    outputFile.open(outputFileName);
    for(size_t i = 0; i < startNode->puzzleData.size(); i++){
        for(size_t j = 0; j < startNode->puzzleData[0].size(); j++){
            outputFile << startNode->puzzleData[i][j] << ' ';
        }
        outputFile << "\n";
    }
    outputFile << "\n";
    for(size_t i = 0; i < goalNode->puzzleData.size(); i++){
        for(size_t j = 0; j < goalNode->puzzleData[0].size(); j++){
            outputFile << goalNode->puzzleData[i][j] << ' ';
        }
        outputFile << "\n";
    }
    outputFile << "\n";
    outputFile << finishedNode->nodeDepth << endl;
    outputFile << nodesGenerated << endl;
    for (char A: finishedNode->actions){
        outputFile << A << ' ';
    }
    
    //cleanup
    delete finishedNode;
    delete startNode;
    outputFile.close();
    
    
    return 0;
}
