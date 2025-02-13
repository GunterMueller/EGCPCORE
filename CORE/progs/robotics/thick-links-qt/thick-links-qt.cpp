/*
 *  thick-links-qt.cpp
 *  
 *  Author: Ching-Hsiang (Tom) Hsu
 *  Email: chhsu@nyu.edu
 *  Modified: Ching-Hsiang (Tom) Hsu, Oct. 31, 2016
 */


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <stdlib.h>

#include "QuadTree.h"
#include "PriorityQueue.h"
#include "Graph.h"
#include "Timer.h"
#include "Polygon.h"
#include "MainWindow.h"
#include "Color.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QDir>

#define mw_out (*window)

static MainWindow *window;
static int runCount=1;

using namespace std;


// GLOBAL INPUT Parameters ========================================
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
string cfgName("T-room4.cfg");
char cfgNameList[200][200];
int numEg = 0;
string inputDir("inputs"); 		// Path for input files
string workingDir;
string fileName("T-room3.txt"); 		// Input file name


FILE *fp;

Box* boxA;				// start box (containing alpha)
Box* boxB;				// goal box (containing beta)
double boxWidth = 512;			// Initial box width
double boxHeight = 512;			// Initial box height
int windowPosX = 320;			// X Position of Window
int windowPosY = 20;			// Y Position of Window
double alpha[4] = { 125, 115, 355, 5 };		// start configuration
double beta[4] = { 230, 470, 275, 265 };	// goal configuration
// length of 2 links
double L1 = 140;
double L2 = 139;
double R0 = 0;			// will be set to max(L1,L2)
// thickness of robot
double thickness = 10;
double epsilon = 4;		// resolution parameter
int QType = 2;			// Search strategy
int interactive = 0;			// Run interactively?
//    Yes (0) or No (1)
int seed = 11;				// seed for random number generator
// (Could also be used for BFS, etc)
double deltaX = 0;		// x-translation of input environment
double deltaY = 0;		// y-translation of input environment
double scale = 1;		// scaling of input environment

bool noPath(true);			// True means there is "No path.
bool hideBox(true);
bool hideBoxBoundary(true);  		// don't draw box boundary
bool showTrace(false);
bool showPath(true);
bool showFilledObstacles(false);
bool showRobot(true);
bool safeAngle(false);
bool runAnim(true);
bool pauseAnim(false);
bool replayAnim(false);
bool inPoly;

int crossingOption = 0; //  Crossing Option    0: original  1: non-crossing
double bandwidth = 0;

bool verboseOption = false;	// don't print various statistics
int twoStrategyOption = 0;  //  Two-Strategy Option    0: original 1: smarter
int sizeOfPhiB = 0;

bool ompl(false); // for reading ompl configuration

vector<Box*> PATH;
vector<Box*> allLeaf;
vector<Set*> allSet;
QuadTree* QT;

int totalSteps = 0;
int currentStep = 0;
int stepIncrease = 0;
int currentPathStep = 0;

int freeCount = 0;
int stuckCount = 0;
int mixCount = 0;
int mixSmallCount = 0;
int renderCount = 0;

stringstream ssout;
stringstream ssoutLastTime;
stringstream ssTemp;
stringstream ssInfo;

bool leafBoxesDrawed = false;
int firstPolygonClockwise = 0;

// color coding variable ========================================
Color clr_totalFREE(0, 1, 0, 0.5);     // green
Color clr_partialFREE(0.25, 1, 0.25, 0.5); // dark green
Color clr_MIXED(1, 1, 0, 0.5); // yellow
Color clr_STUCK(1, 0, 0, 0.5); // red
Color clr_EPS(0.5, 0.5, 0.5, 1); // grey
Color clr_UNKNOWN(1, 1, 0, 0.5); // white

Color clr_start(1,0,1,0.5);    // purple
Color clr_goal(0.4,0,0.4,0.5); // dark purple
Color clr_robot(0.7,0,0.7,0.5); // purple

Color clr_path(0.5,0,0,1); // dark red
Color clr_obstacle(0,0,1,1); // blue
Color clr_boundary(1,1,1,1); // black


extern vector<int> expansions;
extern vector<Polygon> polygons;
extern vector<int> srcInPolygons;

extern int renderSteps;
extern bool step;
extern int animationSpeed;
extern int animationSpeedScale;
extern int animationSpeedScaleBox;

// External Routines ========================================
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void parseExampleList();
void parseExampleFile();
void parseConfigFile(Box*);
void run();
void genEmptyTree();
int checkClockwise(Polygon p);

extern int fileProcessor(string inputfile);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//find path using simple heuristic:
//use distance to beta as key in PQ, see dijkstraQueue
template<typename Cmp>
bool findPath(Box* a, QuadTree* QT, int& ct) {
    bool isPath = false;
    vector<Box*> toReset;
    a->dist2Source = 0;
    dijkstraQueue<Cmp> dijQ;
    dijQ.push(a);
    toReset.push_back(a);
    Box::Order tempOrder;
    if (beta[2] > beta[3]) {
        tempOrder = Box::GT;
    } else {
        tempOrder = Box::LT;
    }
    while (!dijQ.empty()) {

        Box* current = dijQ.extract();
        current->visited = true;

        // if current is MIXED, try expand it and push the children that is
        // ACTUALLY neighbors of the source set (set containing alpha) into the dijQ again
        if (current->status == Box::MIXED) {
            vector<Box*> cldrn;
            if (QT->expand(current, cldrn)) {
                ++ct;
                for (int i = 0; i < (int) cldrn.size(); ++i) {

                    // go through neighbors of each child to see if it's in source set
                    // if yes, this child go into the dijQ
                    bool isNeighborOfSourceSet = false;
                    for (int j = 0; j < 5 && !isNeighborOfSourceSet; ++j) {
                        for (vector<Box*>::iterator iter = cldrn[i]->Nhbrs[j].begin(); iter < cldrn[i]->Nhbrs[j].end(); ++iter) {
                            Box* n = *iter;
                            if (n->dist2Source == 0) {
                                isNeighborOfSourceSet = true;
                                break;
                            }
                        }
                    }

                    if (isNeighborOfSourceSet) {
                        switch (cldrn[i]->getStatus()) {
                        //if it's FREE, also insert to source set
                        case Box::FREE:
                            cldrn[i]->dist2Source = 0;
                            dijQ.push(cldrn[i]);
                            toReset.push_back(cldrn[i]);
                            break;
                        case Box::MIXED:
                            dijQ.push(cldrn[i]);
                            toReset.push_back(cldrn[i]);
                            break;
                        case Box::STUCK:
                            //cerr << "inside FindPath: STUCK case not treated" << endl;
                            break;
                        case Box::UNKNOWN:
                            //cerr << "inside FindPath: UNKNOWN case not treated" << endl;
                            break;
                        }
                    }
                }
            }
            if (current->shouldSplit2D && current->height / 2 >= epsilon
                    && current->width / 2 >= epsilon) {
                dijQ.push(current);
                toReset.push_back(current);
            }

            continue;
        }

        //found path!
        if (current->status == Box::FREE && current->contains(beta[0], beta[1], beta[2], beta[3])) {
            if (crossingOption) {
                if ((current->order == Box::ALL || current->order == tempOrder) && current->dist2Source == 0) {
                    isPath = true;
                    break;
                }
            } else {
                isPath = true;
                break;
            }

        }

        if (current->status == Box::FREE) {
            // if current is not MIXED, then must be FREE
            // go through it's neighbors and add FREE and MIXED ones to dijQ
            // also add FREE ones to source set
            for (int i = 0; i < 5; ++i) {
                for (vector<Box*>::iterator iter = current->Nhbrs[i].begin();
                        iter < current->Nhbrs[i].end(); ++iter) {
                    Box* neighbor = *iter;
                    if (!neighbor->visited && neighbor->dist2Source == -1 && (neighbor->status == Box::FREE || neighbor->status == Box::MIXED)) {
                        if (neighbor->status == Box::FREE) {
                            neighbor->dist2Source = 0;
                        }
                        dijQ.push(neighbor);
                        toReset.push_back(neighbor);
                    }
                }
            }
        }
    }

    //these two fields are also used in dijkstraShortestPath
    // need to reset
    for (int i = 0; i < (int) toReset.size(); ++i) {
        toReset[i]->visited = false;
        toReset[i]->dist2Source = -1;
    }

    return isPath;
}

int main(int argc, char* argv[]) {


    bool foundFiles = false;
    workingDir = QDir::currentPath().toStdString();

    // Test if the build directory is thick-links. If so,
    // the path to the current working directory will
    // include /thick-links
    int indexOfDesiredDir = workingDir.rfind("/thick-links-qt/");
    if (indexOfDesiredDir != (int)std::string::npos) {
        workingDir = workingDir.substr(0, indexOfDesiredDir + 5);

        // Set current working directory to thick-links
        QDir::setCurrent(workingDir.c_str());

        foundFiles = true;
    }

    // Test if program was downloaded from Github, and is the build directory.
    // Downloading it from Github will result in the folder having the name
    // /thick-links-master instead of /thick-links
    if (!foundFiles &&
        (indexOfDesiredDir = workingDir.rfind("/thick-links-qt/")) != (int)std::string::npos) {
        workingDir = workingDir.substr(0, indexOfDesiredDir + 12);

        // Set current working directory to /thick-links-master
        QDir::setCurrent(workingDir.c_str());

        foundFiles = true;
    }

    // Test if a build directory (/build-thick-links-...) was created. This directory
    // will reside in the same directory as /thick-links or /thick-links-qt
    if (!foundFiles &&
        (indexOfDesiredDir = workingDir.rfind("/build-thick-links-qt")) != (int)std::string::npos) {
        QDir dir(workingDir.substr(0, indexOfDesiredDir).c_str());

        if (dir.exists("thick-links-qt/thick-links-qt.pro")) {                  // Test if /v2-links exists
            workingDir = workingDir.substr(0, indexOfDesiredDir) + "/thick-links-qt";

            // Set current working directory to thick-links
            QDir::setCurrent(workingDir.c_str());

            foundFiles = true;
        } else if (dir.exists("thick-links-qt/thick-links-qt.pro")) {  // Test if /thick-links-qt exists
            workingDir = workingDir.substr(0, indexOfDesiredDir) + "/thick-links-qt";

            // Set current working directory to thick-links
            QDir::setCurrent(workingDir.c_str());

            foundFiles = true;
        }
    }

    // Neither /thick-links nor /thick-links-qt could be found
    if (!foundFiles) {
        std::cerr << std::endl << "!! WARNING !!\n"
        << "The program may not work correctly or at all because the folder "
        "containing the program's files cannot be found.\n"
        "Make sure that the program is inside of a folder named \"thick-links-qt\".\n"
        "If it is not, rename the folder to \"thick-links-qt\" before running again.\n";
    }

    // Allow creation of Qt GUI
    QApplication app(argc, argv);
    fp = fopen("debug.txt", "w");
    if(fp == NULL) return 0;







    //testing
    parseExampleList();
    parseExampleFile();
    window = new MainWindow();
    run();
    window->show();







    return app.exec();
}

void genEmptyTree() {

    Box* root = new Box(boxWidth / 2, boxHeight / 2, boxWidth, boxHeight);

    Box::r0 = R0;
    Box::l1 = L1;
    Box::l2 = L2;
    Box::thickness = thickness;

    Box::pAllLeaf = &allLeaf;

    if (!allLeaf.empty() && allLeaf.size() != 0) {
        for (vector<Box*>::iterator it = allLeaf.begin(); it != allLeaf.end();
                ++it) {
            delete *it;
        }
    }

    if (!allSet.empty() && allSet.size() != 0) {
        for (vector<Set*>::iterator it = allSet.begin(); it != allSet.end();
                ++it) {
            delete *it;
        }
    }

    allLeaf.clear();
    allLeaf.push_back(root);
    allSet.clear();

    parseConfigFile(root);

    root->updateStatus();

    if (QT) delete (QT);
    QT = new QuadTree(root, epsilon, QType); // Note that seed keeps changing!
}


void run() {
    runCount++;

    ssoutLastTime.str("");
    ssoutLastTime << ssout.str();
    ssout.str("");
    ssout << "Run No. " << runCount << ":" << endl;

    currentStep = 0;
    currentPathStep = 0;
    leafBoxesDrawed = false;


    if (L1 > L2) {
        R0 = L1;
    } else {
        R0 = L2;
    }

    genEmptyTree();

    if (bandwidth > 180) {
        bandwidth = 180;
    }
    if (bandwidth < 0) {
        bandwidth = 0;
    }

    while (alpha[2] >= 360) {
        alpha[2] -= 360;
    }
    while (alpha[2] < 0) {
        alpha[2] += 360;
    }
         while (alpha[3] >= 360) {
        alpha[3] -= 360;
    }
    while (alpha[3] < 0) {
        alpha[3] += 360;
    }
    while (beta[2] >= 360) {
        beta[2] -= 360;
    }
    while (beta[2] < 0) {
        beta[2] += 360;
    }
        while (beta[3] >= 360) {
        beta[3] -= 360;
    }
    while (beta[3] < 0) {
        beta[3] += 360;
    }

    Timer t;

    t.start();
    noPath = false;	// initially, pretend we have found path
    int ct = 0;	// number of times a node is expanded

    if (QType == 0 || QType == 1) {
        boxA = QT->getBox(alpha[0], alpha[1], alpha[2], alpha[3], ct);
        if (!boxA || (crossingOption && angleDistance(alpha[2], alpha[3]) <= bandwidth)) {
            noPath = true;
            mw_out << "Start Configuration is not free\n";
        }

        boxB = QT->getBox(beta[0], beta[1], beta[2], beta[3], ct);
        if (!boxB || (crossingOption && angleDistance(beta[2], beta[3]) <= bandwidth)) {
            noPath = true;
            mw_out << "Goal Configuration is not free\n";
        }

        // In the following loop, "noPath" should really mean "hasPath"
        //	Otherwise, we should pre-initialize "noPath" to true
        //	before entering loop...
        while (!noPath && !QT->isConnected(boxA, boxB)) {
            if (!QT->expand()) {
                noPath = true;
            }
            ++ct;
        }
    } else if (QType == 2 || QType == 3 || QType == 4) {
        boxA = QT->getBox(alpha[0], alpha[1], alpha[2], alpha[3], ct);
        if (!boxA || (crossingOption && angleDistance(alpha[2], alpha[3]) <= bandwidth)) {
            noPath = true;
            mw_out << "Start Configuration is not free\n";
        }

        boxB = QT->getBox(beta[0], beta[1], beta[2], beta[3], ct);
        if (!boxB || (crossingOption && angleDistance(beta[2], beta[3]) <= bandwidth)) {
            noPath = true;
            mw_out << "Goal Configuration is not free\n";
        }
        if (!noPath) {
            if (QType == 2) {
                noPath = !findPath<DistCmp>(boxA, QT, ct);
            } else if (QType == 3) {
                noPath = !findPath<DistPlusSizeCmp>(boxA, QT, ct);
            } else if (QType == 4) {
                noPath = !findPath<VorCmp>(boxA, QT, ct);
            }
        }
    }
    t.stop();

    if (!noPath) {
        Graph graph;
        PATH.clear();
        PATH = graph.dijkstraShortestPath(boxA);
    }

    if (!noPath)
        mw_out << ">>      ----->>  Path Found !\n";
    else
        mw_out << ">>      ----->>  No Path !\n";
    mw_out << ">>\n";
    mw_out << ">>      ----->>  Time used: " << t.getElapsedTimeInMilliSec() << " ms\n";
    mw_out << ">>\n";
    mw_out << ">>      ----->>  Search Type: ";
    switch (QType) {
    case 0:
        mw_out << "Random Strategy\n";
        break;
    case 1:
        mw_out << "BFS Strategy\n";
        break;
    case 2:
        mw_out << "Greedy Strategy\n";
        break;
    case 3:
        mw_out << "Dist+Size Strategy\n";
        break;
    case 4:
        mw_out << "Voronoi Strategy\n";
        break;
    }
    mw_out << ">>\n";

    if (verboseOption) {
        cout << "    Expanded " << ct << " times\n" << endl;
        cout << "    total Free boxes: " << freeCount << "\n";
        cout << "    total Stuck boxes: " << stuckCount << "\n";
        cout << "    total Mixed boxes smaller than epsilon: " << mixSmallCount << endl;
        cout << "    total Mixed boxes bigger than epsilon: " << mixCount - mixSmallCount << endl;
    }

    totalSteps = allLeaf.size();
    freeCount = stuckCount = mixCount = mixSmallCount = 0;
    mw_out << "################## END of RUN #################\n";
}// run


int skip_comment_line(std::ifstream & in) {
    int c;

    do {
        c = in.get();
        while (c == '#') {
            do {	// ignore the rest of this line
                c = in.get();
            } while (c != '\n');
            c = in.get(); // now, reach the beginning of the next line
        }
    } while (c == ' ' || c == '\t' || c == '\n'); //ignore white spaces and newlines

    if (c == EOF)
        mw_out << "unexpected end of file.\n" ;

    in.putback(c); // this is non-white and non-comment char!
    return c;
}  //skip_comment_line

// skips '\' followed by '\n'
// 	NOTE: this assumes a very special file format (e.g., our BigInt File format)
// 	in which the only legitimate appearance of '\' is when it is followed
// 	by '\n' immediately!
int skip_backslash_new_line(std::istream & in) {
    int c = in.get();

    while (c == '\\') {
        c = in.get();

        if (c == '\n')
            c = in.get();
        else
// assuming the very special file format noted above!
            mw_out
                    << "continuation line \\ must be immediately followed by new line.\n";
    } //while
    return c;
} //skip_backslash_new_line

char cfgPath[200], sptr[200], tmp[200];
void parseExampleList(){

    //    8/24/2016: Tom
    //               new way to parse files in a folder
    string cfgDir = workingDir + "/" + inputDir;
    QDir tmpDir(cfgDir.c_str());
    QStringList fileList = tmpDir.entryList();
    while(!fileList.empty()){
        QString fileName_l = fileList.front();
        strcpy(sptr, fileName_l.toStdString().c_str());
        int len = strlen(sptr);
        if(len > 4 && sptr[len-1] == 'g' && sptr[len-2] == 'f' && sptr[len-3] == 'c' && sptr[len-4] == '.'){
            strcpy(cfgNameList[numEg], sptr);
            ++numEg;
        }
        fileList.pop_front();
    }

//    8/24/2016: Tom
//               old way to parse files in a folder
//    sprintf(cfgPath, "ls -R > tmpList");
//    system(cfgPath);
//    sprintf(cfgPath, "tmpList");
//    FILE *fptr = fopen(cfgPath, "r");
//    if(fptr == NULL) return ;
//    while(fgets(tmp, 200, fptr) != NULL){
//        char *sptr = strtok(tmp, " \n");
//        while(sptr != NULL){
//            int len = strlen(sptr);
//            if(len > 4 && sptr[len-1] == 'g' && sptr[len-2] == 'f' && sptr[len-3] == 'c' && sptr[len-4] == '.'){
//                strcpy(cfgNameList[numEg], sptr);
//                ++numEg;
//            }
//            sptr = strtok(NULL, " \n");
//        }
//    }
//    sprintf(cfgPath, "rm -rf tmpList");
//    system(cfgPath);
}

void parseExampleFile() {

    sprintf(cfgPath, "%s/%s", inputDir.c_str(), cfgName.c_str());
    FILE *fptr = fopen(cfgPath, "r");
    if (fptr == NULL) return ;

    while (fgets(tmp, 200, fptr) != NULL){
        char *sptr = strtok(tmp, "=: \t");

        // comments
        if (strcmp(sptr, "#") == 0) {
            continue;
        }

        if (strcmp(sptr, "interactive") == 0) {
            sptr = strtok(NULL, "=: \t");
            interactive = atoi(sptr);
        }

        // start configuration
        if (strcmp(sptr, "startX") == 0) {
            sptr = strtok(NULL, "=: \t");
            alpha[0] = atof(sptr);
        }
        if (strcmp(sptr, "startY") == 0) {
            sptr = strtok(NULL, "=: \t");
            alpha[1] = atof(sptr);
        }
        if (strcmp(sptr, "startTheta1") == 0) {
            sptr = strtok(NULL, "=: ");
            alpha[2] = atof(sptr);
        }
        if (strcmp(sptr, "startTheta2") == 0) {
            sptr = strtok(NULL, "=: \t");
            alpha[3] = atof(sptr);
        }

        // goal configuration
        if (strcmp(sptr, "goalX") == 0) {
            sptr = strtok(NULL, "=: \t");
            beta[0] = atof(sptr);
        }
        if (strcmp(sptr, "goalY") == 0) {
            sptr = strtok(NULL, "=: \t");
            beta[1] = atof(sptr);
        }
        if (strcmp(sptr, "goalTheta1") == 0) {
            sptr = strtok(NULL, "=: \t");
            beta[2] = atof(sptr);
        }
        if (strcmp(sptr, "goalTheta2") == 0) {
            sptr = strtok(NULL, "=: \t");
            beta[3] = atof(sptr);
        }

        if (strcmp(sptr, "epsilon") == 0) {
            sptr = strtok(NULL, "=: \t");
            epsilon = atof(sptr);
        }

        if (strcmp(sptr, "len1") == 0) {
            sptr = strtok(NULL, "=: \t");
            L1 = atof(sptr);
        }
        if (strcmp(sptr, "len2") == 0) {
            sptr = strtok(NULL, "=: \t");
            L2 = atof(sptr);
        }
        if (strcmp(sptr, "thickness") == 0) {
            sptr = strtok(NULL, "=: \t");
            thickness = atof(sptr);
        }

        if (strcmp(sptr, "inputDir") == 0) {
            sptr = strtok(NULL, "=: #\t");
            inputDir = sptr;
        }
        if (strcmp(sptr, "fileName") == 0) {
            sptr = strtok(NULL, "=: #\t");
            fileName = sptr;
        }

        // box width and height
        if (strcmp(sptr, "boxWidth") == 0) {
            sptr = strtok(NULL, "=: \t");
            boxWidth = atof(sptr);
        }
        if (strcmp(sptr, "boxHeight") == 0) {
            sptr = strtok(NULL, "=: \t");
            boxHeight = atof(sptr);
        }

        // windows position
        if (strcmp(sptr, "windowPosX") == 0) {
            sptr = strtok(NULL, "=: \t");
            windowPosX = atoi(sptr);
        }
        if (strcmp(sptr, "windowPosY") == 0) {
            sptr = strtok(NULL, "=: \t");
            windowPosY = atoi(sptr);
        }

        if (strcmp(sptr, "Qtype") == 0) {
            sptr = strtok(NULL, "=: \t");
            QType = atoi(sptr);
        }

        if (strcmp(sptr, "seed") == 0) {
            sptr = strtok(NULL, "=: \t");
            seed = atoi(sptr);
        }

        if (strcmp(sptr, "step") == 0) {
            sptr = strtok(NULL, "=: \t");
            step = atoi(sptr);
        }

        if (strcmp(sptr, "xtrans") == 0) {
            sptr = strtok(NULL, "=: \t");
            deltaX = atof(sptr);
        }

        if (strcmp(sptr, "ytrans") == 0) {
            sptr = strtok(NULL, "=: \t");
            deltaY = atof(sptr);
        }

        if (strcmp(sptr, "scale") == 0) {
            sptr = strtok(NULL, "=: \t");
            scale = atof(sptr);
        }

        if (strcmp(sptr, "verbose") == 0) {
            sptr = strtok(NULL, "=: \t");
            verboseOption = atoi(sptr);
        }

        if(strcmp(sptr, "non-crossing") == 0) {
            sptr = strtok(NULL, "=: \t");
            crossingOption = atoi(sptr);
        }
        if(strcmp(sptr, "bandwidth") == 0) {
            sptr = strtok(NULL, "=: \t");
            bandwidth = atof(sptr);
        }
        if (strcmp(sptr, "ompl") == 0) {
            sptr = strtok(NULL, "=: \t");
            ompl = atoi(sptr);
        }
        if (strcmp(sptr, "safeAngle") == 0) {
            sptr = strtok(NULL, "=: \t");
            safeAngle = atoi(sptr);
        }


        if (strcmp(sptr, "animationSpeed") == 0) {
            sptr = strtok(NULL, "=: \t");
            animationSpeed = atoi(sptr);
        }
        if (strcmp(sptr, "animationSpeedScale") == 0) {
            sptr = strtok(NULL, "=: \t");
            animationSpeedScale = atoi(sptr);
        }
        if (strcmp(sptr, "animationSpeedScaleBox") == 0) {
            sptr = strtok(NULL, "=: \t");
            animationSpeedScaleBox = atoi(sptr);
        }
    }

    // OMPL
    if(ompl){
        double degToRad = PI/180.0f;
        alpha[0] = alpha[0] - L1*cos((alpha[2])*degToRad)*0.5f;
        alpha[1] = alpha[1] - L1*sin((alpha[2])*degToRad)*0.5f;
        beta[0] = beta[0] - L1*cos((beta[2])*degToRad)*0.5f;
        beta[1] = beta[1] - L1*sin((beta[2])*degToRad)*0.5f;
    }
}

void parseConfigFile(Box* b) {

    inPoly = false;
    polygons.clear();
    srcInPolygons.clear();

    std::stringstream ss;
    ss << inputDir << "/" << fileName;	// create full file name
    std::string s = ss.str();

    int nPt, nPolygons;	// previously, nPolygons was misnamed as nFeatures

    char checkFileName[100];
    sprintf(checkFileName, "%s", fileName.c_str());
    int lenCheckFileName = strlen(checkFileName);

    // read .raw file
    if(checkFileName[lenCheckFileName-1] == 'w' && checkFileName[lenCheckFileName-1] == 'a' && checkFileName[lenCheckFileName-1] == 'r'){
    }
    // read .txt file
    else{
        fileProcessor(s);	// this will clean the input and put in

        ifstream ifs("output-tmp.txt");
        if (!ifs) {
            cerr << "cannot open input file" << endl;
            exit(1);
        }

        // First, get to the beginning of the first token:
        skip_comment_line(ifs);

        ifs >> nPt;

        //Read input points:
        vector<double> pts(nPt * 2);
        for (int i = 0; i < nPt; ++i) {
            ifs >> pts[i * 2] >> pts[i * 2 + 1];
        }

        //Read input polygons:
        ifs >> nPolygons;
        string temp;
        std::getline(ifs, temp);
        for (int i = 0; i < nPolygons; ++i) {
            Polygon tempPolygon;
            string s;
            std::getline(ifs, s);
            stringstream sss(s);
            vector<Corner*> ptVec;
            set<int> ptSet;
            while (sss) {
                int pt;
                /// TODO:
                sss >> pt;
                pt -= 1;	//1 based array
                if (ptSet.find(pt) == ptSet.end()) {
                    ptVec.push_back(new Corner(pts[pt * 2] * scale + deltaX, pts[pt * 2 + 1] * scale + deltaY));

                    b->addCorner(ptVec.back());
                    b->vorCorners.push_back(ptVec.back());
                    ptSet.insert(pt);
                    if (ptVec.size() > 1) {
                        Wall* w = new Wall(ptVec[ptVec.size() - 2], ptVec[ptVec.size() - 1]);
                        b->addWall(w);
                        b->vorWalls.push_back(w);
                    }
                }	//if
                //new pt already appeared, a loop is formed.
                //should only happen on first and last pt
                else {
                    if (ptVec.size() > 1) {
                        Wall* w = new Wall(ptVec[ptVec.size() - 1], ptVec[0]);
                        b->addWall(w);
                        b->vorWalls.push_back(w);
                        break;
                    }
                }
            }		// while(ss)
            tempPolygon.corners = ptVec;
            tempPolygon.corners.push_back(ptVec[0]);
            polygons.push_back(tempPolygon);
            if (pointInPolygon(alpha[0], alpha[1], tempPolygon)) {
                srcInPolygons.push_back(1);
            }
            else {
                srcInPolygons.push_back(0);
                inPoly = true;
            }
            if (i == 0) {		// if first polygon is clockwise, set globalMark
                firstPolygonClockwise = checkClockwise(tempPolygon);
            }
        }		//for i=0 to nPolygons
        ifs.close();
        std::remove("output-tmp.txt");
    }

    if (true) {
        mw_out << "input file name = " << s <<"\n";
        mw_out << "nPt=" << nPt<<"\n";
        mw_out << "nPolygons=" << nPolygons <<"\n";
    }
}		//parseConfigFile


int checkClockwise(Polygon p) {
    if (p.corners.size() == 0) {
        return 0;
    }

    double maxX = -100000;
    int maxI = -1;
    int prevI = -1;
    int nextI = -1;

    for (int i = 0; (unsigned)i < p.corners.size(); i++) {
        if (p.corners[i]->x > maxX) {
            maxX = p.corners[i]->x;
            maxI = i;
            if (maxI == 0) {
                prevI = p.corners.size() - 2;
            } else {
                prevI = maxI - 1;
            }

            if ((unsigned)maxI == p.corners.size() - 2) {
                nextI = 0;
            } else {
                nextI = maxI + 1;
            }
        }
    }
    if ((p.corners[nextI]->y - p.corners[maxI]->y) / (p.corners[nextI]->x - p.corners[maxI]->x == 0 ?
        -0.01 : (p.corners[nextI]->x - p.corners[maxI]->x)) - (p.corners[prevI]->y - p.corners[maxI]->y)
        / (p.corners[prevI]->x - p.corners[maxI]->x == 0 ? -0.01 : (p.corners[prevI]->x - p.corners[maxI]->x))
        > 0) {

        return 1;
    } else {
        return 2;
    }
}

