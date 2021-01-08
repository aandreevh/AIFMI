#include<iostream>
#include <vector>
#include <cmath>
#include <random>
using namespace std;

#define MIN -0.05
#define MAX  0.05

std::random_device rd;
std::mt19937 e2(rd());
std::uniform_real_distribution<double> dist(MIN, MAX);


template<typename T>
ostream & operator<<(ostream& out, const vector<T>& v){
    for(auto& e : v){
        out<<e<<" ";
    }
    return out;
}
double generate(){
    return dist(e2);
}

double sigmoid(double v){
    return 1/(1+exp(-v));
}

class NN{
    vector<int> layers;
    vector<vector<double>> hidden;

    vector<vector<double>> cache;
    vector<vector<double>> errors;

    double step;

    void init(){

        for(int i=0;i<layers.size()-1;i++){
            int l = layers[i]*layers[i+1];
            hidden.push_back(vector<double>(l));
            for(int j=0;j<l;j++){
                hidden[i][j] = generate();
            }
        }

        for(int i=0;i<layers.size();i++){
                cache.push_back(vector<double>(layers[i]));
                errors.push_back(vector<double>(layers[i]));
        }

    }

    void update(const vector<double>& input){
        if(input.size() != layers[0]){
            throw __LINE__;
        }

        cache[0] = input;

        for(int i=1;i<layers.size();i++){
            for(int j=0;j<layers[i];j++) {
                double sum =0;
                for(int k =0;k<layers[i-1];k++){
                    sum +=conn(i-1,k,j)*cache[i-1][k];
                }
                cache[i][j] = sigmoid(sum);
            }
        }
    }

    void updateErrors(const vector<double>& expected){
        auto last = layers.size()-1;

        if(expected.size() != layers[last]){
            throw __LINE__;
        }

        for(auto i=0;i<layers[last];i++){
            errors[last][i]= cache[last][i]*(1-cache[last][i])*(expected[i]-cache[last][i]);
        }

        for(int layer = layers.size()-2;layer >=0;layer--){
            for(auto i=0;i<layers[layer];i++){
                double sum =0;
                for(auto j=0;j<layers[layer+1];j++){
                    sum += conn(layer,i,j)*errors[layer+1][j];
                }
                errors[layer][i] = cache[layer][i]*(1-cache[layer][i])*sum;
            }
        }
    }

    void updateWeights(){
        for(int i=0;i<layers.size()-1;i++){

            for(int p=0;p<layers[i];p++){
                for(int q=0;q<layers[i+1];q++){
                    conn(i,p,q) = conn(i,p,q)+step*cache[i][p]*errors[i+1][q];
                }
            }
        }
    }
public:

    double& conn(int layer,int i,int j){
        if(layer >= layers.size()-1 || i>=layers[layer] || j >= layers[layer+1]){
            throw __LINE__;
        }

        return hidden[layer][i*layers[layer+1]+j];
    }


    NN(double step,vector<int> ns):step(step),layers(std::forward<vector<int>>(ns)){
        if(layers.size() <2){
            throw __LINE__;
        }
        init();
    }

    vector<double> operator()(vector<double> input){
        update(input);
        return cache[layers.size()-1];
    }

    void train(vector<double> input, vector<double> expected){
        update(input);
        updateErrors(expected);
        updateWeights();
    }
};

void orNN(int n=100000,double step=0.05) {
    NN nn(step, {2,4,4,1});

    for (int p = 0; p < n; p++) {
        nn.train({1,1},{1});
        nn.train({0,1},{1});
        nn.train({0,0},{0});
        nn.train({1,0},{1});
    }
    cout<<"OR:"<<endl;
    cout<<"1 1: "<<nn({1,1})<<endl;
    cout<<"0 1: "<<nn({0,1})<<endl;
    cout<<"1 0: "<<nn({1,0})<<endl;
    cout<<"0 0: "<<nn({0,0})<<endl;
}

void andNN(int n=100000,double step=0.05) {
    NN nn(step, {2,4,4,1});

    for (int p = 0; p < n; p++) {
        nn.train({1,1},{1});
        nn.train({0,1},{0});
        nn.train({0,0},{0});
        nn.train({1,0},{0});
    }
    cout<<"AND:"<<endl;
    cout<<"1 1: "<<nn({1,1})<<endl;
    cout<<"0 1: "<<nn({0,1})<<endl;
    cout<<"1 0: "<<nn({1,0})<<endl;
    cout<<"0 0: "<<nn({0,0})<<endl;
}

void xorNN(int n=100000,double step=0.05) {
    NN nn(step, {2,4,4,1});

    for (int p = 0; p < n; p++) {
        nn.train({1,1},{0});
        nn.train({0,1},{1});
        nn.train({0,0},{1});
        nn.train({1,0},{0});
    }
    cout<<"XOR:"<<endl;
    cout<<"1 1: "<<nn({1,1})<<endl;
    cout<<"0 1: "<<nn({0,1})<<endl;
    cout<<"1 0: "<<nn({1,0})<<endl;
    cout<<"0 0: "<<nn({0,0})<<endl;
}
int main(){
    orNN();

    cout<<endl;

    andNN();

    cout<<endl;

    xorNN();
}