#include <iostream>
#include "draw.hpp"
#include <time.h>
#include <vector>
#include <fstream>
#include <unordered_map>
using namespace std;

static vector<color> SET_COLORS{
        {255,0,0},
        {0,255,0},
        {0,0,255},
        {127,0,127},
        {127,127,0},
        {0,127,127},
        {64,64,64},
        {0,0,0},
        {255,127,0},
        {127,255,0},
};

using pvec = vector<dpos>;

void normalize(pvec& v){
    const double min_spacing = 1;
    const double min_outscope = .1;

    double minX = numeric_limits<double>::max();
    double minY = numeric_limits<double>::max();
    double maxX = numeric_limits<double>::min();
    double maxY = numeric_limits<double>::min();

    for(auto& p:v){
        minX = min(p.first,minX);
        minY = min(p.second,minY);

        maxX = max(p.first,maxX);
        maxY = max(p.second,maxY);
    }

    if(maxY -minY < min_spacing){
        maxY = minY+min_spacing;
    }

    double w = maxX-minX;
    double h = maxY-minX;

    minX -=min_outscope*w;
    maxX +=min_outscope*w;

    minY -=min_outscope*h;
    maxY +=min_outscope*h;


    for(auto& p: v){
        p ={(p.first-minX)/(maxX-minX), (p.second-minY)/(maxY-minY)};
    }
}
using disjoint = unordered_map<int,int>;

vector<dpos> genKs(int k, const vector<dpos>& pos){
    double minX = numeric_limits<double>::max();
    double minY = numeric_limits<double>::max();
    double maxX = numeric_limits<double>::min();
    double maxY = numeric_limits<double>::min();

    for(auto& p:pos){
        minX = min(p.first,minX);
        minY = min(p.second,minY);

        maxX = max(p.first,maxX);
        maxY = max(p.second,maxY);
    }

    vector<dpos> ks;

    for(int i=0;i<k;i++){
        double rX = minX +((double) rand() / (RAND_MAX))*(maxX-minX);
        double rY = minY +((double) rand() / (RAND_MAX))*(maxY-minY);
        ks.push_back({rX,rY});
    }

    return ks;
}

bool isDiff(const disjoint& pA,const disjoint& pB){
for(auto& u : pA){
    if(pB.find(u.first)->second != u.second){
        return true;
    }
}
    return false;
}

disjoint split(const vector<dpos>& pos,const vector<dpos>& kpos){
    disjoint dj;
    int counter =0;
    for(auto& p : pos){
        double mdist = numeric_limits<double>::max();
        int c = -1;
        for(auto i=0;i<kpos.size();i++){
            auto temp = dist(p,kpos[i]);
            if(temp < mdist){
                mdist = temp;
                c=i;
            }
        }
        dj[counter++]=c;
    }

    return dj;
}
void fix(const vector<dpos>& pos, vector<dpos>& ks, const disjoint & djs){
 vector<dpos> sums(ks.size());
 vector<int> counts(ks.size());

 for(auto& u : djs){
     sums[u.second] ={sums[u.second].first+ pos[u.first].first,
                      sums[u.second].second+ pos[u.first].second};
     counts[u.second]++;
 }

 int counter=0;
 for(auto& u: ks){
     u = {sums[counter].first/(double)counts[counter],
          sums[counter].second/(double)counts[counter]};
     counter++;
 }
}

pair<disjoint, vector<dpos>> order(const vector<dpos>& pos, int k){
    auto ks = genKs(k,pos);
    auto dd =split(pos,ks);

    fix(pos,ks,dd);
    auto dd2 = split(pos,ks);
    fix(pos, ks,dd2);

    while(isDiff(dd,dd2)){
        dd = dd2;
        dd2=split(pos,ks);
        fix(pos,ks,dd2);
    }

    return {dd2,ks};
}


void drawPoints(img& ii,vector<dpos> pos, unordered_map<int,int>& sets){
    normalize(pos);
    int counter=0;
    for(auto& u: pos){
        ii.draw(u,SET_COLORS[sets[counter++]]);
    }
}

vector<dpos> read(const string& fname){
    ifstream in(fname);
    vector<dpos> out;
    double p,q;
    while(in>>p && in>>q){
        out.push_back({p,q});
    }

    return out;
}

double cost(const vector<dpos>& ptz,const pair<disjoint,vector<dpos>>& v){
    double out =0;
        for(auto& p : v.first){
           out+= dist(ptz[p.first],v.second[p.second]);
        }

        return out;
}

int main(int argc, const char** argv){

    string fname;
    int K ;

    cin>>fname>>K;

    vector<dpos> ptz = read(fname);

    double mcost=numeric_limits<double>::max();
    disjoint minDisjoint;

    const  auto itCount = 150;

    for(auto i=0;i<itCount;i++){
        auto sets = order(ptz,K);
        double tmp = cost(ptz,sets);
        if(tmp < mcost){
            mcost = tmp;
            minDisjoint = std::move(sets.first);
        }
    }


    img ii(1024,1024,{255,255,255});
    drawPoints(ii,ptz,minDisjoint);

    generateBitmapImage(ii,"out.bmp");
}
