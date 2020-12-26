#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <numeric>
#include <cassert>
#include <cmath>

using namespace std;

#define ATTR_ROOT -2
#define LABEL_ROOT "root"

#define ATTR_LEAF -1
#define LABEL_PLUS "+"
#define LABEL_MINUS "-"

template <typename T>
ostream & operator<<(ostream& out, const vector<T>& elements){
    for(auto i =0;i<elements.size();i++){
        out<<elements[i];
        if(i != elements.size()-1){
            out<<" ";
        }
    }

    return out;
}

#define range(set) set.begin(),set.end()

using num = long long;
constexpr num num_max = numeric_limits<num>::max();

struct Sample {
    vector<string> attributes;
    bool result;

    const string& operator[](num index) const{
        return attributes[index];
    }

    bool  operator()() const{
        return result;
    }
};

ostream & operator<<(ostream& out, const Sample& sample){
    out<<"{"<<sample.attributes<<" | "<<(sample.result ? "true": "false")<<"}";
    return out;
}

template <typename T>
std::unordered_set<T> ident(num size){
    unordered_set<T> out;
    num counter=0;

    for(num i=0;i<size;i++)out.insert(counter++);

    return out;
}

struct SampleData{
    vector<Sample> samples;
    unordered_set<num> attributes;
    num positive;
    num negative;

    void ban(num attr){
        auto err =attributes.find(attr);
        if(err != attributes.end()){
            attributes.erase(err);
        }
    }

    SampleData() = default;

    SampleData(initializer_list<Sample> samples): SampleData(vector<Sample>(range(samples))){}

    SampleData(vector<Sample> samples) :
        SampleData(std::move(samples),ident<num>(samples[0].attributes.size())){}

    SampleData(vector<Sample> samples, unordered_set<num> attributes):
    samples(std::move(samples)), attributes(std::move(attributes)){
        assert(!this->samples.empty());

        this->positive = std::accumulate(range(this->samples),0,[](num value,const Sample &cur){
           return value + cur.result;
        });
        this->negative = this->samples.size() - positive;
    }

    Sample& operator[](num index){
        return this->samples[index];
    }

    num operator()() const{
        return positive;
    }
    num  operator!() const{
        return negative;
    }
};


ostream & operator<<(ostream& out, const SampleData& sample){
    out<<"{"<<endl;
    out<<"true: "<<sample()<<endl;
    out<<"false: "<<!sample<<endl;
    out<<"available: "<<vector<num>(range(sample.attributes))<<endl;
    for(auto& u: sample.samples) out<<u<<endl;
    out<<"}";

    return out;
}

using SampleDisjoint = unordered_map<string,SampleData>;

ostream & operator<<(ostream& out, const SampleDisjoint & dj) {
    for(auto& e : dj){
        out<<e.first<<": "<<e.second<<endl;
    }

    return out;
}
struct Node {
    using NodePtr = Node*;

    num attr;
    string label;

    vector<NodePtr> children;

    Node(num attr,string  label): Node(attr,std::move(label),vector<NodePtr>()){}
    Node(num attr,string  label,vector<NodePtr> children):
        attr(attr),label(label),children(std::move(children)){
        cout<<attr<<" - "<<"("<<label<<")"<<endl;
    }

    ~Node(){
        while(!children.empty()){
            auto node = children[children.size()-1];
            children.pop_back();
            delete node;
        }
    }
};

double entropy(num p, num q){
    if(p==0 || q ==0){
        return 0;
    }
    double a = (double )p/(double )(p+q);
    double b = (double )q/(double )(p+q);

    return -(a*log2(a) + b*log2(b));
}

double entropy(const SampleDisjoint& sets) {
    double out= 0.0;

    num total=0;

    for(auto& s : sets) {
        auto set = s.second;
        total += set.samples.size();
    }

    for(auto& s : sets){
        auto set= s.second;
        double t = (double )(set.positive+set.negative)/(double)total;
        double p = entropy(set.positive,set.negative);

        out += t*p;
    }

    return out;
}

SampleDisjoint disjoint(const SampleData& data, num attr){
    unordered_map<string,vector<Sample>> d;

    for(auto& e : data.samples){
        d[e[attr]].push_back(e);
    }

    SampleDisjoint disjointSet;

    for(const auto& p : d){
        disjointSet[p.first] = p.second;
    }

    return disjointSet;
}

vector<Node::NodePtr> id3Plus(const SampleData& data) {
    if (data.negative == 0) {
        return {new Node(ATTR_LEAF, LABEL_PLUS)};
    }
    if (data.positive == 0) {
        return {new Node(ATTR_LEAF, LABEL_MINUS)};
    }

    num bestAttr = -1;
    num bestEntropy = num_max;
    SampleDisjoint bestDisjoint;

    for (auto &attr: data.attributes) {
        auto dis = disjoint(data, attr);
        auto e = entropy(dis);

        if (e < bestEntropy) {
            bestAttr = attr;
            bestEntropy = e;
            bestDisjoint = std::move(dis);
        }
    }

    vector<Node::NodePtr> children;
    for(auto& set: bestDisjoint){
        set.second.ban(bestAttr);
        children.push_back(new Node(bestAttr, set.first, id3Plus(set.second)));
    }

    return children;
}

Node::NodePtr id3(const SampleData& data){
    return new Node(ATTR_ROOT,LABEL_ROOT,id3Plus(data));
}

int main(int argc, char** argv){
    SampleData data
    {
            {{"a1","b1","c1"},true},
            {{"a1","b2","c2"},true},
            {{"a2","b1","c2"},false},
            {{"a2","b2","c1"},true},
            {{"a2","b1","c1"},false},
            {{"a2","b2","c2"},true},
    };

    id3(data);
}