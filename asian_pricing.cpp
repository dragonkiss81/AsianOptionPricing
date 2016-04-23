#include<iostream>
#include<fstream>
#include<math.h>
#define NEW2D(H, W, TYPE) (TYPE **)new2d(H, W, sizeof(TYPE))
using namespace std;

// Input for Asian American Barrier Pricing
double S,X,H,t,v,r,n,k;

// BOPM and Black-Scholes Model parameter
double delta_t ,r_bar ,u ,d ,p;


double ***Create3DArray(int a, int b, int c) {
    double ***A = new double**[a];
    for (int i = 0 ; i < a ; i++) {
        A[i] = new double*[b];
        for (int j = 0 ; j < b ; j++) {
            A[i][j] = new double[c];
        }
    }
    return A;
}


double AvgMax(int j, int i){
    double A = (1 - pow(u,j-i+1)) / (1-u);
    double B = pow(u,j-i)*d*((1-pow(d,i))/(1-d));
    return (S * A + S * B)/(j+1);
}
double AvgMin(int j, int i){
    double A = (1 - pow(d,i+1)) / (1-d);
    double B = pow(d,i)*u*((1-pow(u,j-i))/(1-u));
    return (S * A + S * B)/(j+1);
}
double InterpoStates(int j, int i, int m){
     return ((k-m)/k)*AvgMin(j,i) + (m/k)*AvgMax(j,i);
}


double RunAvgAu(int cur_node, int j, int i){
    return ((j+1)*cur_node + S*pow(u,j+1-i)*pow(d,i)) / (j+2);
}
double RunAvgAd(int cur_node, int j, int i){
    return ((j+1)*cur_node + S*pow(u,j-i)*pow(d,i+1)) / (j+2);
}


int FindFootL(double A, int j, int i){
    int foot_l = (int) floor( (A - AvgMin(j,i))/ ((AvgMax(j,i)-AvgMin(j,i))/k ) );
    if     (foot_l<0)  { return 0; }
    else if(foot_l>k-1){ return k-1; }
    else               { return foot_l; }
}
double FindInterpoXu(double Au, double*** asianMTree, int j, int i, int l){
    if(asianMTree[j+1][i][l] == asianMTree[j+1][i][l+1]) return 0;

    double x = (Au - asianMTree[j+1][i][l+1])/(asianMTree[j+1][i][l] - asianMTree[j+1][i][l+1]);
    if     (x>1){ return 1; }
    else if(x<0){ return 0; }
    else        { return x; }
}
double FindInterpoXd(double Ad, double*** asianMTree, int j, int i, int l){
    if(asianMTree[j+1][i+1][l] == asianMTree[j+1][i+1][l+1]) return 0;

    double x = (Ad - asianMTree[j+1][i+1][l+1])/(asianMTree[j+1][i+1][l] - asianMTree[j+1][i+1][l+1]);
    if     (x>1){ return 1; }
    else if(x<0){ return 0; }
    else        { return x; }
}
double InterpoCu(double*** asianCTree, double x, int l, int j, int i){
     return x*asianCTree[j+1][i][l] + (1-x)*asianCTree[j+1][i][l+1];
}
double InterpoCd(double*** asianCTree, double x, int l, int j, int i){
     return x*asianCTree[j+1][i+1][l] + (1-x)*asianCTree[j+1][i+1][l+1];
}


double AsianOptions(){
    delta_t = t / n;
    r_bar = r*delta_t;
    u = exp(v*sqrt(delta_t));
    d = 1/u;
    p = (exp(r_bar)-d)/(u-d);
    int node_num = n+1;

    double ***asianMTree = Create3DArray(node_num, node_num, k+1);
    double ***asianCTree = Create3DArray(node_num, node_num, k+1);

    for(int j=0; j<node_num; j++){
        for(int i=0; i<node_num; i++){
            for(int m=0; m<=k; m++){
                asianMTree[j][i][m] = InterpoStates(j, i, m);
            }
        }
    }

    double Au, Ad, interpo_xu, interpo_xd, Cu, Cd, asianC, americanC;
    int foot_lu, foot_ld;
    for(int j=node_num-1; j>=0; j--){
        for(int i=0; i<=j; i++){

            americanC = S*pow(u,j-i)*pow(d,i) - X;

            for(int m=0; m<=k; m++){
                Au = RunAvgAu(asianMTree[j][i][m], j, i);
                Ad = RunAvgAd(asianMTree[j][i][m], j, i);
                if(j==node_num-1){
                    Cu = fmax(Au - X, 0);
                    Cd = fmax(Ad - X, 0);
                }
                else{
                    foot_lu = FindFootL(Au, j, i);
                    interpo_xu = FindInterpoXu(Au, asianMTree, j, i, foot_lu);
                    Cu = InterpoCu(asianCTree, interpo_xu, foot_lu, j, i);

                    foot_ld = FindFootL(Ad, j, i);
                    interpo_xd = FindInterpoXd(Ad, asianMTree, j, i, foot_ld);
                    Cd = InterpoCd(asianCTree, interpo_xd, foot_ld, j, i);
                }

                asianC = (p*Cu + (1-p)*Cd)/exp(r_bar);

                if(americanC + X > H){
                    asianCTree[j][i][m] = 0;
                }
                else if(americanC > asianC){
                    asianCTree[j][i][m] = americanC;
                }
                else{
                    asianCTree[j][i][m] = asianC;
                }
            }
            cout<<asianCTree[j][i][3]<<" ";
        }
        cout<<endl;
    }

    return asianCTree[0][0][0];
}

int main(){
    ifstream fin("test.txt");
    fin>>S>>X>>H>>t>>v>>r>>n>>k;

    cout<<AsianOptions()<<endl;

    return 0;
}

