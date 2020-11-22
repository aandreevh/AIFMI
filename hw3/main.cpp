#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <unordered_set>

template <typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& vec){
    for(auto& v :vec){
        out<<v<<" ";
    }

    return out;
}

using num = long long;
#define NUM_MAX INT64_MAX

using coord = std::pair<num,num>;
using world = std::vector<coord>;
using links = std::vector<num>;

template<typename T>
void shuffle(std::vector<T>& arr){
for(num i=0;i<arr.size();i++){
    auto e = rand()%arr.size();
    std::swap(arr[i],arr[e]);
}
}

world w;

world genWorld(num n, num min, num max){
    world out(n);

    for(auto& c: out) c = {rand()%(max-min+1) +min,rand()%(max-min+1) +min};

    return out;
}

struct dna{
 num cost;
 links data;
};

using generation = std::vector<dna>;

struct dnaCmp{
    bool operator()(const dna& l,const dna& r){
        return l.cost < r.cost;
    }
};

std::ostream& operator<<(std::ostream& out, const dna& d){
    out<<"Cost: "<<d.cost<<std::endl;
    out<<"Path: "<<d.data;

    return out;
}


/*
 * dist is squared as square root function is increasing
 *  and won't change the solution
 */
int dist(coord a, coord b){
    return (a.first-b.first)*(a.first-b.first) + (a.second-b.second)*(a.second-b.second);
}

dna& chcost(dna& dn,num index, num mul){
    auto ileft = index ==0 ? 0 : dn.data[index-1];
    auto iright = index==dn.data.size()-1? 0 : dn.data[index+1];

    auto delta = 0;
    delta += dist(w[ileft],w[dn.data[index]]);
    delta += dist(w[iright],w[dn.data[index]]);

    dn.cost += mul*delta;

    return dn;
}


//swaps a mutation for a dna
dna& mutate(dna& dn, num u, num v){
    chcost(dn,u,-1);
    chcost(dn,v,-1);

    std::swap(dn.data[u],dn.data[v]);

    chcost(dn,u,1);
    chcost(dn,v,1);

    return dn;
}

//approaches a local minima of a dna cost
bool reduceCost(dna& dn){
    auto base = dn.cost;

    auto cache = links(dn.data.size());
    auto flag = false;

    for(num i =0;i<dn.data.size();i++){
        num minVal = NUM_MAX;
        num count=0;
        for(num j =0;j<dn.data.size();j++){
            if(i == j)continue;

            mutate(dn,i,j);
            auto val =  base- dn.cost;
            mutate(dn,i,j);
            if(val <= 0)continue;

            if(minVal >val){
                count=0;
                cache[count++]=j;
                minVal = val;
            }else if(minVal == val){
                cache[count++]=j;
            }
        }
        if(count ==0){
            continue;
        }
        auto pick = rand() % count;
        mutate(dn,i,cache[pick]);
        flag = true;
    }
    return flag;
}

//finds a local minima for a dna
dna& minimizeDNA(dna& dn, num cleanCount = NUM_MAX){
    auto counter =0;
    while((counter++)<cleanCount && reduceCost(dn)){}

    return dn;
}

//recalculates the cost of a dna
dna& recalculateCost(dna& out){
    out.cost = 0;

    out.cost += dist(w[0], w[out.data[0]]);
    out.cost += dist(w[0], w[out.data[out.data.size() - 1]]);

    for(num i =1;i<out.data.size();i++){
        out.cost += dist(w[out.data[i - 1]], w[out.data[i]]);
    }

    return out;
}

//generates an identity dna: 1,2,3,...,n
dna ident(){
    dna out={
            0,
            links(w.size()-1),
    };

    num counter= 0;
    for(auto& a : out.data) a =++counter;

    recalculateCost(out);
    return out;
}

//finds a NP solution to the problem, used for testing for low values of n
num NPfind(world& w){
 auto dn = ident();

 num minCost = dn.cost;
 while (std::next_permutation(dn.data.begin(),dn.data.end())){
     recalculateCost(dn);
     if(minCost > dn.cost){
         minCost = dn.cost;
     }
 }

    return minCost;
}

template<typename T,typename A, typename B>
dna startGenerations(T base, A pred , B generator){
    auto generation = base;
    auto genCount = 0;
    while(pred(generation)){
        std::cout<<"GENERATION "<<(genCount++)<<" :"<<generation[0].cost<<std::endl;
        generator(generation);
    }

    return generation[0];
}

auto defaultPredicate(num histCount){
      struct _struct{
          num cost=NUM_MAX,count=0,hist;
          _struct(num h):hist(h) {}

        bool operator()(const std::vector<dna>& data) {
            auto last = data[0].cost;
            if (cost > last) {
                cost = last;
                count = 1;
            } else if (cost == last) {
                count++;
            }
            return count != hist;
        }

    } out(histCount+1);

    return out;
}

template <typename T>
auto defaultGeneration(T crossover){

    return [crossover](std::vector<dna>& data){
        std::vector<dna> x,y;
        num counter = 0;
        for(auto & d : data){
            if((counter++) %2 ==0){
                x.push_back(d);
            }else{
                y.push_back(d);
            }
        }

        shuffle(x);
        shuffle(y);

        for(num i=0;i< std::min(x.size(),y.size());i++){
            data.push_back(crossover(x[i],y[i]));
            data.push_back(crossover(y[i],x[i]));
        }

        std::sort(data.begin(),data.end(),dnaCmp());
        auto half = data.size()/2;
        data = std::vector<dna>(data.begin(),data.begin()+half);

    };
}

std::vector<dna> genBase(num n){
    std::vector<dna> bases;

    dna d2 = ident();
   // minimizeDNA(d2);
    for(num i=0;i<n;i++){
        dna d= d2;
        mutate(d,rand()%(d.data.size()),rand()%(d.data.size()));
        bases.push_back(d);
    }

    std::sort(bases.begin(),bases.end(),dnaCmp());
    return bases;
}


dna mergeCrossover(dna a, dna b){
    std::unordered_set<num> av;
    num n = a.data.size();

    for(num i=1;i<=n;i++){
        av.insert(i);
    }

    dna out;
    out.data = links(n);

    auto p0 = a.data.size()>>1;
    for(num i =0;i<p0;i++){
        out.data[i] = a.data[i];
        av.erase(av.find(a.data[i]));
    }
    num counter = p0;
    for(num i=p0;i < n;i++){
            if(av.find(b.data[i]) == av.end()){
                continue;
            }
            out.data[counter++]=b.data[i];
            av.erase(av.find(b.data[i]));
    }
    links rest(av.begin(),av.end());
    shuffle(rest);

    for(auto& e : rest)out.data[counter++]=e;

    auto f = rand()%out.data.size();
    auto s = rand()%out.data.size();
    std::swap(out.data[f],out.data[s]);
    recalculateCost(out);
   // minimizeDNA(out,100);
    return out;
}
int main(){
    num n =100;

    //World generation always same for same function input
    srand(0);
    w = genWorld(n,-10*n,10*n);

    std::cout << startGenerations(
            genBase(n),
            defaultPredicate(1000),
            defaultGeneration(mergeCrossover)) << std::endl;

    if(w.size() < 13){
        //NP is in reasonable time
        std::cout<<"MIN COST NP: "<<NPfind(w)<<std::endl;
    }

    return 0;
}