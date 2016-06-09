//
//  Dg.cpp
//  Wires
//
//  Created by Nick Porcino on 7/19/14.
//  Copyright (c) 2014 PlanetIx. All rights reserved.
//

#include "Dg.h"
#include "LabText/TextScanner.h"

typedef float M44f;
using namespace std;

//to do, the evaluator should take a cache validity check which could be a time cache so that if the output is cached at time,
//just return it. The recursion of the value method will take care of lazy early outs.

void foo() {
    Dg dg;
    dg.addNode("xform1");
    dg.addAttribute("xform1", "xformOut");
    dg.addNode("xform2");
    dg.addAttribute("xform2", "xformOut");
    dg.addNode("xformFinal");
    dg.addAttribute("xformFinal", "xformL");
    dg.addAttribute("Xformfinal", "xfromR");
    dg.addAttribute("xformFinal", "xformOut");
    dg.connectAttribute("xform1", "xformOut", "xformFinal", "xformL");
    dg.connectAttribute("xform2", "xformOut", "xformFinal", "xformR");
    dg.setEvaluator("xformFinal", "xformOut", std::function<void(Dg&)>([](Dg& dg) {
        M44f l,r;
        dg.value<M44f>("xformFinal", "xformL", l);
        dg.value<M44f>("xformFinal", "xformR", r);
        dg.setValue<M44f>("xformFinal", "xformOut", l * r);
    }));
}

void Dg::connect(const string &from, const string &to) {
    pair<string, string> key(from, to);
    if (find_pair(_connections, key) == _connections.end()) {
        _connections.insert(key);
        _reverseConnections.insert(pair<string, string>(to, from));
    }
}

void Dg::connectAttribute(const string &from, const string& fromAttr, const string &to, const string& toAttr) {
    pair<string, string> key(attrKey(from, fromAttr), attrKey(to, toAttr));
    if (find_pair(_connections, key) == _connections.end()) {
        _connections.insert(key);
        _reverseConnections.insert(pair<string, string>(attrKey(to, toAttr), attrKey(from, fromAttr)));
    }
}

set<string> Dg::roots() const {
    set<string> result;
    for (auto n : _connections) {
        auto el = _reverseConnections.equal_range(n.first);
        if (el.first == el.second)
            result.insert(n.first);  // no reverse connections means a root
    }
    return result;
}
set<string> Dg::terminals() const {
    set<string> result;
    for (auto n : _reverseConnections) {
        auto el = _connections.equal_range(n.first);
        if (el.first == el.second)
            result.insert(n.first);  // no forward connections means a terminal
    }
    return result;
}
set<string> Dg::attributes(const string& node) const {
    set<string> result;
    auto el = _nodeAttributes.equal_range(node);
    for (auto i = el.first; i != el.second; ++i)
        result.insert(i->second);
    return result;
}

void Dg::addNode(const string &nodeName) {
    /// @TODO should throw if it already exists
    if (_nodes.find(nodeName) == _nodes.end())
        _nodes.insert(nodeName);
}

void Dg::addAttribute(const string &nodeName, const string &attrName) {
    string key = attrKey(nodeName, attrName);
    if (_attributes.find(key) != _attributes.end())
        return;
    
    pair<string, string> kv(nodeName, attrName);
    _nodeAttributes.insert(kv);
    AttrRecord* ar = new AttrRecord();
    ar->node = nodeName;
    ar->name = attrName;
    _attributes[key] = ar;
}

void Dg::setEvaluator(const std::string& nodeName, const std::string& attrName, std::function<void(Dg&)> evalFn) {
    std::string key = attrKey(nodeName, attrName);
    auto it = _attributes.find(key);
    AttrRecord *record = 0;
    if (it == _attributes.end()) {
        record = new AttrRecord();
        _attributes[key] = record;
    }
    else
        record = it->second;
    record->evaluator = evalFn;
}

int Dg::addObserver(const std::string &nodeName, const std::string &attrName, std::function<void (Dg &)> callbackFn) {
    static int unique = 0;
    _observers.insert(pair<string, ObserverRecord*>(attrKey(nodeName, attrName), new ObserverRecord(unique++, callbackFn)));
    return 0;
}

vector<string> Dg::pred(const string& node) const {
    vector<string> ret;
    auto preds = _reverseConnections.equal_range(node);
    for (auto pred = preds.first; pred != preds.second; ++pred)
        ret.push_back(pred->second);
    return ret;
}

vector<string> Dg::succ(const string& node) const {
    vector<string> ret;
    auto succs = _connections.equal_range(node);
    for (auto succ = succs.first; succ != succs.second; ++succ)
        ret.push_back(succ->second);
    return ret;
}

void Dg::reportInToOut(const string& node, int indent) {
    char buff[indent+1];
    for (int i = 0; i < indent; ++i)
        buff[i] = ' ';
    buff[indent] = '\0';
    printf("%s%s\n", buff, node.c_str());
    auto nodes = _connections.equal_range(node);
    for (auto i = nodes.first; i != nodes.second; ++i)
        reportInToOut(i->second, indent + 3);
}

void Dg::reportOutToIn(const string& node, int indent) {
    char buff[indent+1];
    for (int i = 0; i < indent; ++i)
        buff[i] = ' ';
    buff[indent] = '\0';
    printf("%s%s\n", buff, node.c_str());
    auto nodes = _reverseConnections.equal_range(node);
    for (auto i = nodes.first; i != nodes.second; ++i)
        reportOutToIn(i->second, indent + 3);
}

void Dg::reportAttributes(const string& node, int indent) {
    char buff[indent+1];
    for (int i = 0; i < indent; ++i)
        buff[i] = ' ';
    buff[indent] = '\0';
    auto nodes = _nodeAttributes.equal_range(node);
    if (nodes.first != nodes.second) {
        printf("%s%s\n", buff, node.c_str());
        for (auto i = nodes.first; i != nodes.second; ++i) {
            bool reported = true;
            auto key = attrKey(i->first, i->second);
            auto attr = _attributes.find(key);
            if (attr != _attributes.end()) {
                AttrRecord* rec = attr->second;
                type_index type = rec->data->type();
                if      (type == typeid(int))    { int v;    value<int>(attr->first, v);    printf("%s   %s:%d\n", buff, i->second.c_str(), v); }
                else if (type == typeid(float))  { float v;  value<float>(attr->first, v);  printf("%s   %s:%f\n", buff, i->second.c_str(), v); }
                else if (type == typeid(string)) { string v; value<string>(attr->first, v); printf("%s   %s:%s\n", buff, i->second.c_str(), v.c_str()); }
                else reported = false;
            }
            else
                reported = false;
            
            if (!reported)
                printf("%s   %s\n", buff, i->second.c_str());
            
            //reportAttributes(i->second, indent + 3);
        }
    }
}

void Dg::report() {
    printf(">> Dg nodes\n");
    for (auto n : _nodes)
        printf("   %s\n", n.c_str());
    
    printf("----- Dg in -> out ------------\n");
    set<string> rootVec = roots();
    for (auto root : rootVec)
        reportInToOut(root, 0);
    
    printf("----- Dg out -> in ------------\n");
    set<string> leafVec = terminals();
    for (auto leaf : leafVec)
        reportOutToIn(leaf, 0);

    bool titled = false;
    for (auto n : _nodes) {
        auto attrs = _nodeAttributes.equal_range(n);
        if (attrs.first != attrs.second) {
            if (!titled) {
                printf("----- Attributes on nodes ------\n");
                titled = true;
            }
            reportAttributes(n, 0);
        }
    }
    printf("================================\n");
}

