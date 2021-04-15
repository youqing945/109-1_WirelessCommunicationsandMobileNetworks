#include <iostream>
#include <cmath>
#include <ctime> 
#include <iomanip>
using namespace std;

int main(){

    int s[3] = {1, 5, 10};
    int q = 0;
    int arrival, inqueue, block;
    double p_arr, p_dep;
   
    srand(time(NULL));

    for(int i = 0; i < 2; i++){  //0: Q=0 //1: Q=s
        if(i == 0) cout << "> Q = 0" <<endl;
        else cout<< "> Q = S" << endl;
        cout << "    Erlang |  S = 1   |  S = 5   |  S = 10  " << endl;
        cout << "===========|==========|==========|==========" << endl;

        for(double lambda = 0.01; lambda <= 10; lambda *= 10.0){
            p_arr = 1 - exp(lambda * (-0.001));
            for(double mu = 0.01; mu <= 10.24; mu *= 4.0){
                cout << setw(11) << fixed << setprecision(6) <<lambda / mu ;
                for(int j = 0; j < 3; j++){ //N=1, 5, 10
                    arrival = 0;
                    inqueue = 0;
                    block = 0;
                    if(i == 0) q = 0; //Q=0
                    else q = s[j]; //Q=S

                    for(int systime = 0; systime < 10000000; systime++){
                        double r_arr = (double) rand() / (RAND_MAX + 1.0);
                        double r_dep = (double) rand() / (RAND_MAX + 1.0);
                        p_dep = 1 - exp(inqueue * mu * (-0.001));
                        //arrive
                        if(r_arr <= p_arr && r_dep > p_dep){
                            if(inqueue < (s[j] + q)){
                                inqueue++;
                                arrival++;
                            }
                            else if(inqueue == (s[j] + q)){
                                block++;
                                arrival++;
                            }
                        }
                        //depart
                        else if(r_arr > p_arr && r_dep <= p_dep){ 
                            if(inqueue > 0){
                                inqueue--;
                            }
                        }
                        //arrive and depart
                        else if(r_arr <= p_arr && r_dep <= p_dep){ 
                            arrival++; 
                        }
                    }
                    cout << "| " << setw(8) << fixed << setprecision(6) << (double)block/(double)arrival << " ";
                }
                cout << endl;
            }
        }
        cout << endl;
    }

    return 0;
}
