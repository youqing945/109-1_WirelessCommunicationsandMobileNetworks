#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<stdlib.h>
#include<time.h>
#include<math.h>
using namespace std;

#define U 0
#define R 1
#define D 2
#define L 3 //direction
#define bs1 0
#define bs2 1
#define bs3 2
#define bs4 3
#define Pt 100
#define Pmin 10
#define vcar 10
#define T 15 //Threshold
#define E 13 //Entropy
#define DELTA 1
#define TOTALTIME 2000
int bsl[4][2] = {{360, 680},{660, 658},{330, 350},{640, 310}};


typedef struct cartype
{
    int x = 0;
    int y = 0;
    int dir = U;
    int bs = bs1;
    int bsmin = bs1;
    double p = 0;
    double pformin = 0;
} cartype;

void Simulate(int input_lambda, int policy);
cartype generateNewCar(int l, int s);
double getP(int x, int y);
cartype updateP(cartype curcar, int policy);

//====================================================

int main(){
    /*
    int a, b;
    cout << "[lambda]" << endl;
    cout << "1: 1/2 car/sec" << endl;
    cout << "2: 1/3 car/sec" << endl;
    cout << "3: 1/5 car/sec" << endl;
    cout << "Choose lambda: ";
    cin >> a;
    cout << "[Policy]" << endl;
    cout << "1: Best      | Pnew > Pold" << endl;
    cout << "2: Threshold | Pnew > Pold & Pold < T" << endl;
    cout << "3: Entropy   | Pnew > Pold + E" << endl;
    cout << "4: My Policy | Pold < T & Pnew > Pold + E" << endl;
    cout << "Choose lambda: ";
    cin >> b;
    */

    cout << "Simulating..." << endl;

    srand(time(NULL));

    for(int i = 1; i <= 3; i++){     //which lambda
        for(int j = 1; j <= 4; j++){ //which policy
            Simulate(i, j);
        }
    }

    cout << "All simulating end. Please check the files. \n";
    return 0;
}

//===================================================

void Simulate(int input_lambda, int policy){
    //Status update.
    cout << "Simulating lambda = ";
    if(input_lambda == 1) cout << "1/2 using ";
    else if(input_lambda == 2) cout << "1/3 using ";
    else cout << "1/5 using ";
    if(policy == 1) cout << "Best...\n";
    else if(policy == 2) cout << "T...\n";
    else if(policy == 3) cout << "E...\n";
    else cout << "MyP...\n";

    //Get the file ready.
    string hfilename = "H_";
    if(input_lambda == 1) hfilename += "1-2_";
    else if(input_lambda == 2) hfilename += "1-3_";
    else hfilename += "1-5_";
    if(policy == 1) hfilename += "Best.csv";
    else if(policy == 2) hfilename += "T.csv";
    else if(policy == 3) hfilename += "E.csv";
    else hfilename += "MyP.csv";
    fstream fileHandoff(hfilename, fstream::out);

    vector<cartype> allcar;
    double Pminatthispolicy[TOTALTIME][2];
    double Paveatthispolicy[TOTALTIME][2];
    double Pave = 0;
    double Pavemin = 0;
    int handoffall = 0;

    //poisson
    double poisson;
    if(input_lambda == 1) poisson = 1 - exp(-((double)1/2) * DELTA);
    else if(input_lambda == 2) poisson = 1 - exp(-((double)1/3) * DELTA);
    else poisson = 1 - exp(-((double)1/5) * DELTA);

    //Simulate for TOTALTIME.
    for(int t = 0; t < TOTALTIME; t++){

        //Generate a new car.
        for(int i = 100; i <= 900; i = i + 100){
            for(int j = 0; j < 4; j++){
                double r = (double) rand() / (RAND_MAX + 1.0);
                if(r < poisson)
                    allcar.push_back(generateNewCar(i, j));
            }
        }

        int handoff = 0;
        int handoffmin = 0;

        //Calculate all cars in vector.
        vector<int> waitfordelete;
        for(int i = 0; i < allcar.size(); i++){
            //1. If the car is at intersection, random its way.
            if(allcar[i].x % 100 == 0 && allcar[i].y % 100 == 0){
                //srand(time(NULL));
                double nextway = (double) rand() / (RAND_MAX + 1.0);
                if(nextway < 0.2) {
                    allcar[i].dir++; //turn right 20%
                    if(allcar[i].dir > 3) allcar[i].dir = 0;
                }
                else if(nextway >= 0.2 && nextway < 0.4) {
                    allcar[i].dir--; //turn left 20%
                    if(allcar[i].dir < 0) allcar[i].dir = 3;
                }
            }
            //2. Go according to the speed.
            if(allcar[i].dir == U) allcar[i].y = allcar[i].y + vcar;
            else if(allcar[i].dir == R) allcar[i].x = allcar[i].x + vcar;
            else if(allcar[i].dir == D) allcar[i].y = allcar[i].y - vcar;
            else if(allcar[i].dir == L) allcar[i].x = allcar[i].x - vcar;
            //3. Check if out of range. If so, delete it from the vector and skip step 4.
            if(allcar[i].x < 0 || allcar[i].x > 1000 || allcar[i].y < 0 || allcar[i].y > 1000){
                waitfordelete.push_back(i);
            }
            //4. Calculate new P and check if handoff happen.
            else{
                int prebs = allcar[i].bs;
                int prebsmin = allcar[i].bsmin;
                allcar[i] = updateP(allcar[i], policy);
                if(prebs != allcar[i].bs){
                    //Handoff happen.
                    handoff++;
                }
                if(prebsmin != allcar[i].bsmin){
                    handoffmin++;
                }
            }
        }
        //Delete from previous step 3.
        for(int i = waitfordelete.size()-1; i >= 0; i--){
            allcar.erase(allcar.begin() + waitfordelete[i]);
        }
        waitfordelete.clear();

        //Calculate data that Paveatthispolicy needs.
        Paveatthispolicy[t][0] = Pminatthispolicy[t][0] = allcar.size();
        Paveatthispolicy[t][1] = Pminatthispolicy[t][1] = 0;
        for(int i = 0; i < allcar.size(); i++) {
            Paveatthispolicy[t][1] += allcar[i].p;
            Pminatthispolicy[t][1] += allcar[i].pformin;
        }
        Paveatthispolicy[t][1] = Paveatthispolicy[t][1] / allcar.size();
        Pminatthispolicy[t][1] = Pminatthispolicy[t][1] / allcar.size();



        //Put handoff times in csv.
        fileHandoff << handoff << "\n";

        handoffall += handoff;

    } //End of a sec.

    //Calculate Pave.
    double temp = 0;
    for(int i = 0; i < TOTALTIME; i++) {
        Pave += (Paveatthispolicy[i][0] * Paveatthispolicy[i][1]);
        temp += Paveatthispolicy[i][0];
    }
    Pave = Pave / temp;

    //Calculate Pmin.
    temp = 0;
    for(int i = 0; i < TOTALTIME; i++) {
        Pavemin += (Pminatthispolicy[i][0] * Pminatthispolicy[i][1]);
        temp += Pminatthispolicy[i][0];
    }
    Pavemin = Pavemin / temp;

    fileHandoff.close();
    allcar.clear();
    cout << "Simulating end. Pave";
    if(policy == 1) cout << "(AVGmax)";
    cout << " = " << Pave << ", Pmin = " << Pavemin << ", handoff = " << handoffall << endl;
    return;
}

cartype generateNewCar(int l, int s){
    cartype newcar;
    if(s == U || s == D) {     //Up & Down
        newcar.x = l;
        if(s == U){  //at Up
            newcar.y = 1000;
            newcar.dir = D;
        }
        else{   //at Down
            newcar.y = 0;
            newcar.dir = U;
        }    
    }
    else {  //Left & Right
        newcar.y = l;
        if(s == L){  //at Left
            newcar.x = 0;
            newcar.dir = R;
        }
        else{   //at Right
            newcar.x = 1000;
            newcar.dir = L;
        }
    }
    //Get recent bs.
    for(int i = 0; i < 4; i++){
        double p = getP(newcar.x-bsl[i][0], newcar.y-bsl[i][1]);
        if(p > newcar.p){
            newcar.p = p;
            newcar.pformin = p;
            newcar.bs = i;
            newcar.bsmin = i;
        }
    }

    return newcar;
}

double getP(int x, int y){
    double ans = Pt - 33 - 20 * log10(sqrt(pow(x, 2) + pow(y, 2)));
    return ans;
}

cartype updateP(cartype curcar, int policy){
    //1. Update current P.
    curcar.p = getP(curcar.x-bsl[curcar.bs][0], curcar.y-bsl[curcar.bs][1]);
    curcar.pformin = getP(curcar.x-bsl[curcar.bsmin][0], curcar.y-bsl[curcar.bsmin][1]);
    //2. Check if need to change bs according to policy.
    for(int i = 0; i < 4; i++){
        double p = getP(curcar.x-bsl[i][0], curcar.y-bsl[i][1]);
        //1: Best      | Pnew > Pold
        if(policy == 1){
            if(p > curcar.p){
                curcar.p = p;
                curcar.bs = i;
            }
            if(p > curcar.pformin && curcar.pformin < Pmin){
                curcar.pformin = p;
                curcar.bsmin = i;
            }
        }
        //2: Threshold | Pnew > Pold & Pold < T
        else if(policy == 2){
            if(p > curcar.p && curcar.p < T){
                curcar.p = p;
                curcar.bs = i;
            }
            if(p > curcar.pformin && curcar.pformin < T && curcar.pformin < Pmin){
                curcar.pformin = p;
                curcar.bsmin = i;
            }
        }
        //3: Entropy   | Pnew > Pold + E
        else if(policy == 3){
            if(p > (curcar.p + E)){
                curcar.p = p;
                curcar.bs = i;
            }
            if(p > (curcar.pformin + E) && curcar.pformin < Pmin){
                curcar.pformin = p;
                curcar.bsmin = i;
            }
        }
        //4: My Policy | Pold < T & Pnew > Pold + E
        else{
            if(curcar.p < T && p > (curcar.p + E)){
                curcar.p = p;
                curcar.bs = i;
            }
            if(curcar.pformin < T && p > (curcar.pformin + E) && curcar.pformin < Pmin){
                curcar.pformin = p;
                curcar.bsmin = i;
            }
        }
    }
    return curcar;
}