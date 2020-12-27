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
#include <fstream>

using namespace std;
#define DELIMITER ","

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
            out<<",";
        }
    }

    return out;
}

#define range(set) set.begin(),set.end()
#define first(set,n) set.begin(),set.begin()+n
#define last(set,n) set.begin()+n,set.begin()+set.size()

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

    num positive;
    num negative;

    vector<NodePtr> children;

    Node(num attr,string  label): Node(attr,std::move(label),vector<NodePtr>()){}
    Node(num attr,string  label,vector<NodePtr> children):
        attr(attr),label(label),children(std::move(children)){
        /*cout<<attr<<" - "<<"("<<label<<")"<<endl;*/
    }

    ~Node(){
        while(!children.empty()){
            auto node = children.back();
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
        double t = (double)(set.samples.size())/(double)total;
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
       auto dsample = SampleData(p.second,data.attributes);
       dsample.ban(attr);
       disjointSet[p.first] = dsample;
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
    double bestEntropy = numeric_limits<double>::max();
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
        if(set.second.attributes.empty()){
            return {new Node(ATTR_LEAF,
                    set.second.positive > set.second.negative ?
                    LABEL_PLUS : LABEL_MINUS)};
        }else{
            auto nd = new Node(bestAttr, set.first, id3Plus(set.second));

            nd->positive = set.second.positive;
            nd->negative = set.second.negative;

            children.push_back(nd);
        }

    }

    return children;
}

Node::NodePtr id3(const SampleData& data){
    return new Node(ATTR_ROOT,LABEL_ROOT,id3Plus(data));
}

vector<string> tokenize(string in,const string& delimiter){
        vector<string> out;
        string token;
        size_t pos =-1;

        while((pos = in.find(delimiter)) != string::npos){
            out.push_back(in.substr(0,pos));
            in.erase(0,pos + delimiter.length());
        }

        if(!in.empty()){
            out.push_back(in);
        }

return out;
}

bool parseResult(string res){
    transform(range(res),res.begin(), ::tolower);
    return  res == "yes" || res=="true";
}

bool valid(const vector<string>& data){
    for(auto& d: data){
        if(d == "?")return false;
    }

    return true;
}
istream& operator>>(istream& in, vector<Sample>& samples){
    const auto delimiter = ",";
    char buffer[2048];
    samples.clear();

    while(in.getline(buffer,sizeof(buffer))){
        string data(buffer);
        vector<string> tokens = tokenize(data, delimiter);
        if(!valid(tokens))continue;

        vector<string> attr(tokens.begin(),tokens.begin()+(tokens.size()-1));
        string resultRaw = tokens[tokens.size()-1];
        bool res = parseResult(resultRaw);
        samples.push_back({attr,res});
    }

    return in;
}

bool resolveLeafLabel(string label){
    return label == "+";
}

bool validateSample(Node::NodePtr dTr,const Sample& sample){
    if(dTr->attr == ATTR_LEAF){
        return resolveLeafLabel(dTr->label) == sample.result;
    }

    for(auto& e : dTr->children){
        if(e->attr == ATTR_LEAF || sample[e->attr] == e->label){
            return validateSample(e,sample);
        }
    }

    return dTr->positive > dTr->negative ? sample.result : !sample.result;
}

template<typename T>
void shuffle(vector<T> vec){
for(auto i=0;i<vec.size();i++){
    swap(vec[i],vec[rand()%vec.size()]);
}
}

double matchDensity(Node::NodePtr  root, vector<Sample>& validationSet){
    double out=0.0;
    double delta = (double)1/(double)validationSet.size();

    for(auto& s : validationSet){
        if(validateSample(root,s)){
           out+=delta;
        }
    }

    return out;
}
int main(int argc, char** argv){
    const double MAX_ERR = 0.10;

    if(argc != 2){
        cout<<"Please set data file.";
        return 1;
    }

    vector<Sample> inputSamples;

    ifstream  in(argv[1]);
    in >> inputSamples;

    shuffle(inputSamples);

    auto root = id3(SampleData(inputSamples));
    double target =matchDensity(root, inputSamples);
    delete root;


    for(auto k=1;k<inputSamples.size()-1;k++){
        vector<Sample> learnData(first(inputSamples,k));
        vector<Sample> validationData(last(inputSamples,k+1));

        root = id3(learnData);

        double cur=(matchDensity(root,validationData)*validationData.size()
                +matchDensity(root,learnData)*learnData.size())/(double)(inputSamples.size());

        cout<<k<<" : "<<cur<<endl;
        if(target - cur <MAX_ERR){
            cout<<"K: "<<k<<endl;
            delete root;
            return 0;
        }
        delete root;
    }

    cout<<"K: "<<inputSamples.size()<<" | density: "<<target<<endl;

}